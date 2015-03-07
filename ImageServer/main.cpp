#include <iostream>
#include <string>
#include "itrbase.h"
#include "itrdevice.h"
#include "itrsystem.h"
#include "ix264.h"
using namespace std;

itr_device::ICamera *camera=NULL;
itrx264::ix264 compress;
itr_system::Udp *udp=NULL;
itr_protocol::StandSerialProtocol sspUdp;

S32 width, height;
S32 cameraID, cameraTunnel;
string IP;
S32 tport=0,rport=0;
F32 fps=0;

enum eState
{
    IDLE,CAPTURE,TRACK,EXIT
};
eState state;
bool Init(int argc, char *argv[])
{
    itr_math::MathObjStandInit();
    const int npara=17;
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
                if (strcmp(argv[i], "IP") == 0)
                {
                    IP = argv[i + 1];
                    i += 2;
                }
                else if (strcmp(argv[i], "tport") == 0)
                {
                    sscanf(argv[i + 1], "%d", &tport);
                    i+=2;
                }
                else if (strcmp(argv[i], "rport") == 0)
                {
                    sscanf(argv[i + 1], "%d", &rport);
                    i+=2;
                }
                else if (strcmp(argv[i], "res") == 0)
                {
                    sscanf(argv[i + 1], "%d", &width);
                    sscanf(argv[i + 2], "%d", &height);
                    i += 3;
                }
                else if (strcmp(argv[i], "cameraid") == 0)
                {
                    sscanf(argv[i+1], "%d",&cameraID);
                    sscanf(argv[i+2], "%d",&cameraTunnel);
                    i+=3;
                }
                else if (strcmp(argv[i], "cameratype") == 0)
                {
                    if(strcmp(argv[i+1],"v4l2"))
                    {
                        camera=new itr_device::v4linux;
                    }
                    else if(strcmp(argv[i+1],"asi"))
                    {
                        //camera=new itr_device::AsiCamera;
                    }
                    i+=2;
                }
                else if (strcmp(argv[i], "fps") == 0)
                {
                    sscanf(argv[i+1], "%f",&fps);
                    i+=2;
                }
            }
            else
            {
                i++;
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

    udp=new itr_system::Udp(rport,false);

    return true;
}

int main(int argc, char *argv[])
{
    Init(argc, argv);
    const BUFSIZE=200;
    char buf[BUFSIZE];
    U8 *data[4];
    S32 stride[4];
    U8* pic;
    const void *imgCompressData;
    int imgLength;
    stride[0]=width;
    stride[1]=width/2;
    stride[2]=width/2;
    stride[3]=0;
    int size=width*height;

    while (state != EXIT)
    {
        if(udp->Receive(buf, BUFSIZE))
        {

        }
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
        int rc = compress.Compress(data, stride,&imgCompressData , &imgLength);  //前两位是压缩后长度

        TimeClock tc;
        tc.Tick();



    }
    compress.Close();
    camera->Close();
    return 0;
}