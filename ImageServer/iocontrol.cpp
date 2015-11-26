//
// Created by ghdawn on 15-11-24.
//

#include <udp.h>
#include <itrbase.h>
#include "iocontrol.h"

class IOControl::SSPReceive : public itr_protocol::StandSerialProtocol::SSPDataRecFun
{
public:
    void SetControlState(eState *state)
    {
        this->state = state;
    }
    void Do(itr_protocol::StandSerialProtocol *SSP, itr_protocol::StandSerialFrameStruct *SSFS, U8 *Package, S32 PackageLength)
    {
        F32 *a, *b, *c, *d;
        switch (Package[0])
        {
            case 0x41:
                *state = (eState) Package[1];
                break;
            default:
                break;
        }
    }

public:
    eState *state;
};

class IOControl::SSPSend : public itr_protocol::StandSerialProtocol::SSPDataSendFun {
public:
    SSPSend(char* SendBuffer,const void** imgCompressData, int* imgLength)
    {
        sendbuf = SendBuffer;
        img = imgCompressData;
        this->imgLength = imgLength;
    }
    S32 Do(U8 *Buffer, S32 Length) {
        memcpy(sendbuf, Buffer, Length);
        memcpy(sendbuf + Length,  *img,*imgLength);
        int SendLength = Length + *imgLength;
        return SendLength;
    }

private:
    const void** img;
    int* imgLength;
    char* sendbuf;
};

void IOControl::SetControlState(eState *state)
{
    this->receiveObj->SetControlState(state);

}

void IOControl::Init(string IP, int ReceivePort, int TransmitPort,const void** imgCompressData, int* imgLength)
{
    udp=new itr_system::Udp(ReceivePort,false);
    sendobj = new SSPSend(SendBuf,imgCompressData,imgLength);
    receiveObj = new SSPReceive();
    sspUdp.Init(0xA5,0x5A,sendobj);
    sspUdp.AddDataRecFunc(receiveObj, 0);

    udpPackage.port = TransmitPort;
    udpPackage.IP = IP;
    udpPackage.pbuffer=IOControl::SendBuf;
}

IOControl::~IOControl()
{
    delete udp;
    delete sendobj;
    delete receiveObj;
}

void IOControl::CheckIncomingData()
{
    int len;
    len = udp->Receive(RecBuf, MaxRecLength);
    if(len>0)
    {
        sspUdp.ProcessRawByte((U8 *) RecBuf, len);
    }
}

void IOControl::SendData(U8 *data, S32 length)
{
    udpPackage.len = sspUdp.SSPSendPackage(0, data, length);
    udp->Send(udpPackage);
}
