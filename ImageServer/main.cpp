#include <iostream>
#include <fstream>
#include "itrbase.h"
#include "itrdevice.h"
#include "itrvision.h"
#include "itrsystem.h"
#include "ix264.h"
#include "h264_imx.h"
#include "configure.h"
#include "iocontrol.h"

using namespace std;

itr_device::ICamera *camera=new itr_device::v4linux;
itrx264::ix264 compress;
H264_imx h264_imx;
IOControl ioControl;
int imgLength;
U8 compressData[MaxSendLength];
Configure config;
eState state=CAPTURE;
bool Init(int argc, char *argv[])
{
    itr_math::MathObjStandInit();
    if (argc > 1)
    {
        config.Parse(argc, argv);
    }
    else
    {
        ifstream fin("config");
        string str;
        getline(fin,str);
        config.Parse(str);
    }
    if (camera!=NULL)
    {
        camera->Open(config.cameraID, config.encoderWidth, config.encoderHeight, 2);
        camera->SetTunnel(config.cameraTunnel);
        h264_imx.Open(config.encoderWidth, config.encoderHeight,config.fps);
		h264_imx.SetQuality(config.encoderQuality);
    }
    else return false;

    ioControl.Init(config.IP,config.receivePort,config.transmitPort);
	ioControl.SetConfigure(&config);
    ioControl.SetControlState(&state);

    return true;
}

int main(int argc, char *argv[])
{
    Init(argc, argv);
    int size=config.width*config.height;
    U8* pic=new U8[size*3/2];
    U8 sendbuffer[20];
    itr_vision::MeanShift *meanShift=NULL;
    Matrix img_origin(config.encoderHeight,config.encoderWidth);
    Matrix img(config.height,config.width);
    RectangleF rect(config.targetx,config.targety,config.targetWidth,config.targetHeight);
    TimeClock tcDelay,tcFre;
    U16 counter = 0;
    Log log;
    log.enablePrint();
    log.enableTime();
    while (state != EXIT)
    {
		printf("========Begin=======\n");
        ioControl.CheckIncomingData();
        printf("State:%d\n",state);
        if( state == IDLE)
        {
            sleep(1);
            continue;
        }
        camera->FetchFrame(pic,size*3/2,NULL);
        float fps = 1000.0f/tcFre.Tick();
        tcDelay.Tick();
        log.log("get image");
        h264_imx.Compress(pic, compressData, imgLength);
        log.log("x264");
        const U8 OPTBIT = 0x1f;
        if (state != TRACK)
        {
            if (meanShift!=NULL)
            {
                delete meanShift;
                meanShift = NULL;
            }
        }
        if (state == TRACK)
        {
            S32 *pimg = (S32 *) img_origin.GetData();
            itr_vision::ColorConvert::yuv420p2rgb(pimg, pic, config.width, config.height);
            itr_vision::Scale::SubSampling(img_origin, img, config.encoderWidth / config.width);
            log.log("yuv2rgb");
            pimg = (S32 *) img.GetData();
            for (int i = 0; i < config.height; i++)
            {
                for (int j = 0; j < config.width; j++)
                {
                    U8 r, g, b;
                    r = (U8) ((*pimg & 0xff0000) >> 16);
                    g = (U8) ((*pimg & 0xff00) >> 8);
                    b = (U8) (*pimg & 0xff);
                    int a = ((r & 0xf0) << 4) | ((g & 0xf0)) | ((b & 0xf0) >> 4);
                    img(i, j) = a;
                    pimg++;
                }
            }
            if (meanShift != NULL)
            {
                meanShift->Go(img, rect);
                printf("%f %f\n", rect.X, rect.Y);
                config.confidence = 80;
                if(rect.X<0)
                {
                    rect.X = 0;
                    config.confidence = 0;
                }
                else if(rect.X>config.width)
                {
                    rect.X = config.width;
                    config.confidence = 0;
                }
                if(rect.Y<0)
                {
                    rect.Y = 0;
                    config.confidence = 0;
                }
                else if(rect.Y>config.height)
                {
                    rect.Y = config.height;
                    config.confidence = 0;
                }
            }
            else
            {
                meanShift = new itr_vision::MeanShift;
                meanShift->Init(img, rect, itr_vision::MeanShift::IMG_RGB);
            }

        }
		else
		{
                rect.X = config.targetx;
                rect.Y = config.targety;
                rect.Width = config.targetWidth;
                rect.Height = config.targetHeight;
		}
        log.log("track");
        itr_container::ByteStream bs((void *) sendbuffer);
        bs.setU8(state);
        bs.setU8(OPTBIT | config.userOptBit);
        bs.setU8((U8) config.confidence);
        bs.setU16((U16) ((rect.X+rect.Width/2) * 10000 / config.width));
        bs.setU16((U16) ((rect.Y+rect.Height/2)* 10000 / config.height));
        bs.setU8((U8) (rect.Width * 250 / config.width));
        bs.setU8((U8) (rect.Height * 250 / config.height));
        if(config.userOptBit & (1<<5))
            bs.setU16(counter++);
        if(config.userOptBit & (1<<6))
            bs.setU8((U8) (5 * fps));
        if(config.userOptBit & (1<<7))
            bs.setU8((U8) tcDelay.Tick());
        itr_protocol::StandardExchangePackage sep(0x10);
        sep.setSID(0x05);
        sep.data.assign(sendbuffer,sendbuffer+bs.getLength());
        ioControl.SendData(sep, compressData, imgLength);
        log.log("send");
		printf("========End=======\n\n");
    }
    compress.Close();
    h264_imx.Close();
    camera->Close();
    delete pic;
    delete camera;

    return 0;
}
