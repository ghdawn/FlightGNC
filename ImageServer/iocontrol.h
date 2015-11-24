//
// Created by ghdawn on 15-11-24.
//

#ifndef IMAGESERVER_IOCONTROL_H
#define IMAGESERVER_IOCONTROL_H

#include "config.h"


class IOControl
{
public:
    void Init(string IP,int ReceivePort,int TransmitPort,const void** imgCompressData, int* imgLength);
    void SetControlState(eState *state);
    void CheckIncomingData();
    void SendData(U8* data,S32 length);
    ~IOControl();
private:

    class SSPSend;
    class SSPReceive;
    itr_system::Udp::UdpPackage udpPackage;
    SSPSend* sendobj;
    SSPReceive* receiveObj;
    itr_protocol::StandSerialProtocol sspUdp;
    itr_system::Udp* udp;

    char RecBuf[MaxRecLength];
    char SendBuf[MaxSendLength];
};
#endif //IMAGESERVER_IOCONTROL_H
