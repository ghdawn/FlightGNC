#ifndef IOCONTROL_H
#define IOCONTROL_H
#include "itrbase.h"
#include "itrsystem.h"
#include "QThread"
const int ServerPort=9031;
const int ClientPort=9032;

class RecThread: public QThread
{
    Q_OBJECT
       protected:
        void run()
        {
            while(true)
            {
                int length=udp->Receive(RecBuf,65535);
                if(length>0)
                {
                    emit OnDataReceive(RecBuf,length);
                }

            }
        }
signals:
     void OnDataReceive(char* buffer,int length);
public:
        void Init()
        {
            udp = new itr_system::Udp(ClientPort,true);
        }

private:
        char RecBuf[65535];
        itr_system::Udp *udp;

};

class SSPReceivefuc:public itr_protocol::StandSerialProtocol::SSPDataRecFun
{
public:
    void setpara(S32* Mode,F32 *X,F32 *Y,F32 *A,F32 *F)
    {
        mode = Mode;
        pos_x = X;
        pos_y = Y;
        Area = A;
        fps = F;
    }
    F32 *pos_x,*pos_y,*Area,*fps;
    S32 *mode;

    void Do(itr_protocol::StandSerialProtocol *SSP, itr_protocol::StandSerialFrameStruct *SSFS,U8 *Package,S32 PackageLength)
    {
            F32 *X,*Y,*A,*FPS;
            switch(Package[0])
            {
            case 0x40:
                *mode=Package[1];
                FPS=(F32*)&Package[2];
                X=(F32*)&Package[6];
                Y=(F32*)&Package[10];
                A=(F32*)&Package[14];
                *pos_x=*X;
                *pos_y=*Y;
                *Area=*A;
                *fps=*FPS;
                break;

            }
    }
};

class SSPSend: public itr_protocol::StandSerialProtocol::SSPDataSendFun
{
public:
    itr_system::Udp udp;
    itr_system::Udp::UdpPackage udppackage;
    SSPSend()
    {
        udppackage.IP="192.168.199.251";
        udppackage.port=ServerPort;
    }
    S32 Do(U8* Buffer,S32 Length)
    {
        udppackage.pbuffer=(char*)Buffer;
        udppackage.len=Length;
        udppackage.port=ServerPort;
        udp.Send(udppackage);
    }
};


#endif // IOCONTROL_H
