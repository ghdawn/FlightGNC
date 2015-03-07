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
const void *imgCompressData;
int imgLength;
S32 width, height;
S32 cameraID, cameraTunnel;
string IP;
S32 tport=0,rport=0;
F32 fps=0;
S32 SendLength;
const int MaxSendLength=65535;
const int MaxRecLength=200;
char RecBuf[MaxRecLength];
char SendBuf[MaxSendLength];

enum eState
{
    IDLE=0,CAPTURE,TRACK,EXIT
};
eState state=CAPTURE;

class SSPReceiveFunc : public itr_protocol::StandSerialProtocol::SSPDataRecFun
{
    void Do(itr_protocol::StandSerialProtocol *SSP, itr_protocol::StandSerialFrameStruct *SSFS, U8 *Package, S32 PackageLength)
    {
        F32 *a, *b, *c, *d;
        switch (Package[0])
        {
            case 0x41:
                state = (eState) Package[1];
                break;
            case 0x42:

                break;
            case 0x44:

            case 0x43:

            case 0x45:

            default:
                break;
        }
    }
};

class SSPSend : public itr_protocol::StandSerialProtocol::SSPDataSendFun {
    S32 Do(U8 *Buffer, S32 Length) {
        memcpy(SendBuf, Buffer, Length);
        memcpy(SendBuf + Length,  imgCompressData,imgLength);
        SendLength = Length + imgLength;
        itr_system::Udp::UdpPackage udpPackage;
        udpPackage.pbuffer=SendBuf;
        udpPackage.port = tport;
        udpPackage.IP = IP;
        udpPackage.len = SendLength;
        udp->Send(udpPackage);
        return SendLength;
    }
};

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

    udp=new itr_system::Udp(rport,false);
    sspUdp.SetDataSendFunc(new SSPSend);
    sspUdp.AddDataRecFunc(new SSPReceiveFunc, 0);
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
    int len;
    while (state != EXIT)
    {
        if(len=udp->Receive(RecBuf, MaxRecLength))
        {
            sspUdp.ProcessRawByte((U8 *) RecBuf, len);
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
        int rc = compress.Compress(data, stride,&imgCompressData , &imgLength);
        sspbuffer[0]=0x40;
        sspbuffer[1]=1;
        sspUdp.SSPSendPackage(0, sspbuffer, 18);
    }
    compress.Close();
    camera->Close();
    delete pic;
    delete camera;
    delete udp;
    return 0;
}