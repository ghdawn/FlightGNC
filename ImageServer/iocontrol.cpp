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
//    this->receiveObj->SetControlState(state);

}

void IOControl::Init(string IP, int ReceivePort, int TransmitPort)
{
    udp=new itr_system::Udp(ReceivePort,false);

    udpPackage.port = TransmitPort;
    udpPackage.IP = IP;
    udpPackage.pbuffer=SendBuf;
}

IOControl::~IOControl()

{
    delete udp;
}

void IOControl::CheckIncomingData()
{
    int len;
    len = udp->Receive(RecBuf, MaxRecLength);
    itr_protocol::StandardExchangeProtocolSerial protocolSerial;
    if(len>0)
    {
        protocolSerial.processByte((U8 *) RecBuf,0, len);
    }
}

void IOControl::SendData(const itr_protocol::StandardExchangePackage& sep,void* imgData,int imgLen)
{
    itr_protocol::StandardExchangeProtocolSerial protocolSerial;
    int len = protocolSerial.fillBuffer(sep, (U8 *) SendBuf);
    itr_protocol::ComboPackage cp;
    cp.F0 = 'E';
    cp.F1 = 'P';
    cp.length = (U32) len;
    MemoryCopy(cp.data,SendBuf,len);
    len = cp.writeTo((U8 *) SendBuf, 0);
    udpPackage.len = len;
    if (imgData!=0 && imgLen!=0)
    {
        cp.F0 = 'X';
        cp.F1 = '4';
        cp.length = (U32) imgLen;
        MemoryCopy(cp.data, (void *) imgData, imgLen);
        len = cp.writeTo((U8 *) SendBuf, len);
    }
    udpPackage.len += len;
    printf("\n%d\n",udp->Send(udpPackage));
}
