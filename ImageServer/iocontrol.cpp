//
// Created by ghdawn on 15-11-24.
//

#include <udp.h>
#include <itrbase.h>
#include "iocontrol.h"

class IOControl::SEPReceive : public itr_protocol::OnReceiveSEP
{
public:
    void SetControlState(eState *state)
    {
        this->state = state;
    }
    S32 Do(const itr_protocol::StandardExchangePackage& sep)
    {
        switch (sep.keyword)
        {
            case 0x11:
                *state = (eState) sep.data[0];
                break;
        }
    }
public:
    eState *state;
};


void IOControl::SetControlState(eState *state)
{
    this->receiveObj->SetControlState(state);
}

void IOControl::Init(string IP, int ReceivePort, int TransmitPort)
{
    udp=new itr_system::Udp(ReceivePort,false);
    receiveObj = new SEPReceive;
    protocolSerial.AddReceiveFun(receiveObj);
    udpPackage.port = TransmitPort;
    udpPackage.IP = IP;
    udpPackage.pbuffer=SendBuf;
}

IOControl::~IOControl()
{
    delete udp;
    delete receiveObj;
}

void IOControl::CheckIncomingData()
{
    int len;
    len = udp->Receive(RecBuf, MaxRecLength);
    if(len>0)
    {
        int count = protocolSerial.ProcessByte((U8 *) RecBuf,0, len);
        printf("receive : %d\n",count);
    }
}

void IOControl::SendData(const itr_protocol::StandardExchangePackage& sep,void* imgData,int imgLen)
{
    itr_protocol::StandardExchangeProtocolSerial protocolSerial;
    int len = protocolSerial.FillBuffer(sep, (U8 *) SendBuf);
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
