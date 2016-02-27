#include <iostream>
#include "itrbase.h"
#include "itrdevice.h"
#include "itrvision.h"
#include "itrsystem.h"
#include "ix264.h"
#include "h264_imx.h"
#include "config.h"
#include "iocontrol.h"

using namespace std;

itr_device::ICamera *camera=NULL;
itrx264::ix264 compress;
H264_imx h264_imx;
IOControl ioControl;
const void *imgCompressData;
int imgLength;
S32 width, height;

eState state=CAPTURE;
bool Init(int argc, char *argv[])
{
    itr_math::MathObjStandInit();
    const int npara=17;
    string IP;
    S32 cameraTunnel=0;
    U32 cameraID = 0;
    S32 tport=0,rport=0;
    F32 fps=0;
    if (argc < npara)
    {
        printf("Help!\n");
        return false;
    }
    else
    {
        for (int i = 1; i < argc;)
        {
            if (argv[i][0] == '-')
            {
                if (strcmp(argv[i], "-ip") == 0)
                {
                    IP = argv[i + 1];
                    i += 2;
                }
                else if (strcmp(argv[i], "-tport") == 0)
                {
                    sscanf(argv[i + 1], "%d", &tport);
                    i+=2;
                }
                else if (strcmp(argv[i], "-rport") == 0)
                {
                    sscanf(argv[i + 1], "%d", &rport);
                    i+=2;
                }
                else if (strcmp(argv[i], "-res") == 0)
                {
                    sscanf(argv[i + 1], "%d", &width);
                    sscanf(argv[i + 2], "%d", &height);
                    i += 3;
                }
                else if (strcmp(argv[i], "-cameraid") == 0)
                {
                    sscanf(argv[i+1], "%d",&cameraID);
                    sscanf(argv[i+2], "%d",&cameraTunnel);
                    i+=3;
                }
                else if (strcmp(argv[i], "-cameratype") == 0)
                {
                    if(strcmp(argv[i+1],"v4l2")==0)
                    {
                        camera=new itr_device::v4linux;
                    }
                    else if(strcmp(argv[i+1],"asi")==0)
                    {
                        //camera=new itr_device::AsiCamera;
                    }
                    i+=2;
                }
                else if (strcmp(argv[i], "-fps") == 0)
                {
                    sscanf(argv[i+1], "%f",&fps);
                    i+=2;
                }
                else
                {
                    i++;
                }
            }
        }
    }
    if (camera!=NULL)
    {
        camera->Open(cameraID, width, height, 2);
        camera->SetTunnel(cameraTunnel);
        compress.Open(width, height, fps);
        h264_imx.Open(width,height,fps);
		h264_imx.SetQuality(15);
    }
    else return false;


    ioControl.Init(IP,rport,tport);
    ioControl.SetControlState(&state);

    return true;
}

int main(int argc, char *argv[])
{
    Init(argc, argv);
    U8 *data[4];
    S32 stride[4];

    stride[0]=width;
    stride[1]=width/2;
    stride[2]=width/2;
    stride[3]=0;
    int size=width*height;
    U8* pic=new U8[size*3/2];
    U8 sendbuffer[18];
    itr_vision::MeanShift *meanShift=NULL;
    Matrix img(height,width);
    RectangleF rect(140,100,40,40);
    TimeClock tc;
    Log log;
    log.enablePrint();
    log.enableTime();
    while (state != EXIT)
    {
		printf("========Begin=======\n");
        tc.Tick();
        ioControl.CheckIncomingData();
        printf("State:%d\n",state);
        if( state == IDLE)
        {
            sleep(1);
            continue;
        }
        camera->FetchFrame(pic,size*3/2,NULL);
        log.log("get image");
        data[0]=pic;
        data[1]=pic+size;
        data[2]=pic+size+size/4;
        data[3]=NULL;
        ////compress.Compress(data, stride,&imgCompressData , &imgLength);
        U8 compressData[65535];
        h264_imx.Compress(pic, compressData, imgLength);
        log.log("x264");
			S32* pimg = (S32*)img.GetData();
			itr_vision::ColorConvert::yuv420p2rgb(pimg,pic,width,height);
		log.log("yuv2rgb");
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
			for(int i=0;i<height;i++)
			{
				for(int j=0;j<width;j++)
				{
					U8 r,g,b;
					r = (*pimg&0xff0000)>>16;
					g = (*pimg&0xff00)>>8;
					b = (*pimg&0xff);
					int a =((r&0xf0)<<4)|((g&0xf0))|((b&0xf0)>>4);
					img(i,j) = a;
					pimg++;
				}

			}
            if(meanShift!=NULL)
            {
                meanShift->Go(img,rect);
				printf("%f %f\n",rect.X,rect.Y);
            }
            else
            {
                meanShift = new itr_vision::MeanShift;
                rect.X = 140;
                rect.Y = 100;
                meanShift->Init(img,rect,itr_vision::MeanShift::IMG_RGB);
            }
        }
        log.log("track");
        itr_container::ByteStream bs((void *) sendbuffer);
        bs.setU8(state);
        bs.setU8(0x1f);
        bs.setU8(50);
        bs.setU16((U16) ((rect.X+rect.Width/2) * 10000 / width));
        bs.setU16((U16) ((rect.Y+rect.Height/2)* 10000 / height));
        bs.setU8((U8) (rect.Width * 250 / width));
        bs.setU8((U8) (rect.Height * 250 / height));
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
