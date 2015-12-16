#include <iostream>
#include "itrbase.h"
#include "itrdevice.h"
#include "itrvision.h"
#include "itrsystem.h"
#include "ix264.h"
#include "config.h"
#include "iocontrol.h"

using namespace std;

itr_device::ICamera *camera=NULL;
itrx264::ix264 compress;
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
    }
    else return false;


    ioControl.Init(IP,rport,tport,&imgCompressData,&imgLength);
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
    U8 sspbuffer[18];
    itr_vision::MeanShift *meanShift=NULL;
    Matrix img(height,width);
    RectangleF rect(140,100,40,40);
    TimeClock tc;
    while (state != EXIT)
    {
        tc.Tick();
        ioControl.CheckIncomingData();
        if( state == IDLE)
        {
            sleep(1);
            continue;
        }
        camera->FetchFrame(pic,size*3/2,NULL);

        data[0]=pic;
        data[1]=pic+size;
        data[2]=pic+size+size/4;
        data[3]=NULL;
        compress.Compress(data, stride,&imgCompressData , &imgLength);
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
            for (int i = 0; i < height; ++i)
            {
                for (int j = 0; j < width; ++j)
                {
                    img(i,j)=pic[i*width+j];
                }
            }
            if(meanShift!=NULL)
            {
                meanShift->Go(img,rect);
            }
            else
            {
                meanShift = new itr_vision::MeanShift;
                rect.X = 140;
                rect.Y = 100;
                meanShift->Init(img,rect,itr_vision::MeanShift::IMG_GRAY);
            }
        }

        sspbuffer[0]=0x40;
        sspbuffer[1]=state;
        float fps = 1000.0/tc.Tick();
        memcpy(sspbuffer+2,&fps,4);
        float x=rect.X+rect.Width*0.5;
        memcpy(sspbuffer+6,&x,4);
        float y=rect.Y+rect.Height*0.5;
        memcpy(sspbuffer+10,&y,4);
        float Area=1600;
        memcpy(sspbuffer+14,&Area,4);
//        printf("Pos:%f %f\n",rect.X,rect.Y);
        ioControl.SendData(sspbuffer,18);
    }
    compress.Close();
    camera->Close();
    delete pic;
    delete camera;

    return 0;
}