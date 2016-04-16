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
    int size=config.encoderWidth*config.encoderHeight;
    U8* pic=new U8[size*3/2];
    U8 sendbuffer[20];
    itr_vision::MeanShift *meanShift=NULL;
    Matrix img_origin(config.encoderHeight, config.encoderWidth);
    Matrix img(config.height,config.width);
    RectangleF rect(config.targetx-config.targetWidth/2,config.targety-config.targetHeight/2,config.targetWidth,config.targetHeight);
    TimeClock tcDelay,tcFre;
    U16 counter = 0;
    Log log;
    log.enablePrint();
    log.enableTime();

    KalmanFilter kf;
    kf.Init(4);
    F32 data[24]= {1,0,1,0,
                   0,1,0,1,
                   0,0,1,0,
                   0,0,0,1,
                   1,0,0,0,
                   0,1,0,0
    };
    kf.F_x.CopyFrom(data);
    kf.F_n.SetDiag(1);

    Matrix H(2, 4), R(2, 2), Q(4, 4);
    H.CopyFrom(data+16);
    Q.SetDiag(3.0);
    R.SetDiag(12.0);
    Vector z(2), X(4), n(4);

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
            config.confidence = 0;
            if (meanShift!=NULL)
            {
                delete meanShift;
                meanShift = NULL;
            }
        }
        if (state == TRACK)
        {

            S32 *pimgI = (S32 *) img_origin.GetData();
            itr_vision::ColorConvert::yuv420p2rgb(pimgI, pic, config.encoderWidth, config.encoderHeight);
            itr_vision::Scale::SubSampling(img_origin, img, img.GetCol()/config.encoderWidth );
            meanShift->ChangeFormat(img);
            if (meanShift != NULL)
            {
                meanShift->Go(img, rect);
                z[0]=rect.X;
                z[1]=rect.Y;
                X=kf.UpdateModel(Q,n);
                X = kf.UpdateMeasure(H, R, z);
                rect.X=X[0];
                rect.Y=X[1];
                printf("%f %f\n", rect.X, rect.Y);
                config.confidence = 80;
                if(rect.X<0)
                {
                    rect.X = 0;
                    config.confidence = 1;
                }
                else if(rect.X>config.width)
                {
                    rect.X = config.width;
                    config.confidence = 1;
                }
                if(rect.Y<0)
                {
                    rect.Y = 0;
                    config.confidence = 1;
                }
                else if(rect.Y>config.height)
                {
                    rect.Y = config.height;
                    config.confidence = 1;
                }
            }
            else
            {
                meanShift = new itr_vision::MeanShift;
                meanShift->Init(img, rect, itr_vision::MeanShift::IMG_RGB);
                kf.x[0]=rect.X;
                kf.x[1]=rect.Y;
                kf.x[2]=0;
                kf.x[3]=0;
            }

        }
		else
		{
                rect.X = config.targetx - config.targetWidth/2;
                rect.Y = config.targety - config.targetHeight/2;
                rect.Width = config.targetWidth;
                rect.Height = config.targetHeight;
                printf("%f %f %f %f\n",rect.X,rect.Y,rect.Width,rect.Height);
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


