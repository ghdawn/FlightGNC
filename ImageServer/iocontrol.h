//
// Created by ghdawn on 15-11-24.
//

#ifndef IMAGESERVER_IOCONTROL_H
#define IMAGESERVER_IOCONTROL_H

#include "configure.h"


class IOControl
{
public:
    void Init(string IP,int ReceivePort,int TransmitPort);
    void SetControlState(eState *state);
	void SetConfigure(Configure *conf);
    void CheckIncomingData();
    void SendData(const itr_protocol::StandardExchangePackage& sep, void* imgData=0,int imgLen=0);
    ~IOControl();
private:

    class SEPReceive;
    itr_system::Udp::UdpPackage udpPackage;
    SEPReceive* receiveObj;
    itr_protocol::StandardExchangeProtocolSerial protocolSerial;
    itr_system::Udp* udp;

    U8** imgData;
    int* imgLen;

    char RecBuf[MaxRecLength];
    char SendBuf[MaxSendLength];

    char* memBuf;

};
#endif //IMAGESERVER_IOCONTROL_H
