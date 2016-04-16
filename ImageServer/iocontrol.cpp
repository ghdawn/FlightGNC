//
// Created by ghdawn on 15-11-24.
//

#include <udp.h>
#include <itrbase.h>
#include "iocontrol.h"
#include "configure.h"
#include <fstream>
#include <sys/mman.h>
#include <fcntl.h>

using namespace std;

class IOControl::SEPReceive : public itr_protocol::OnReceiveSEP
{
public:
    void SetControlState(eState *state)
    {
        this->state = state;
    }
    void SetConfigure(Configure *config)
    {
        configure = config;
    }
    S32 Do(const itr_protocol::StandardExchangePackage& sep)
    {
		if (sep.getTID()!= 5)
			return 0;

        switch (sep.keyword)
        {
            case 0x11:
                *state = (eState) sep.data[0];
                break;
            case 0x12:
            {
                itr_container::ByteStream bs((void *) &sep.data[0]);
                configure->targetx = bs.getU16() * configure->width / 10000;
                configure->targety = bs.getU16() * configure->height / 10000;
                configure->targetWidth = bs.getU8() * configure->width / 250;
                configure->targetHeight = bs.getU8() * configure->height/ 250;
            }
                break;
            case 0x13:
            {
                itr_container::ByteStream bs((void *) &sep.data[0]);
                configure->targetx += bs.getS16() * configure->width / 10000;
                configure->targety += bs.getS16() * configure->height / 10000;
                configure->targetWidth += bs.getS8() * configure->width / 100;
                configure->targetHeight += bs.getS8() * configure->height/ 100;
            }
                break;
            case 0x14:
            {
                itr_container::ByteStream bs((void *) &sep.data[0]);
                configure->userOptBit = bs.getU8();
                configure->encoderQuality = bs.getU8();
                configure->encoderWidth = bs.getU16();
                configure->encoderHeight = bs.getU16();
                configure->width = bs.getU16();
                configure->height = bs.getU16();
                configure->cameraID = bs.getU8();
                configure->cameraTunnel = bs.getU8();
                configure->receivePort = bs.getU16();
                configure->SetIP(bs.getU32());
                configure->transmitPort = bs.getU16();
				ofstream fout("config");
				fout<<configure->ToString()<<endl;
				fout.close();
                *state = EXIT;
            }
                break;
        }
		return 0;
    }
public:
    eState *state;
    Configure *configure;
};


void IOControl::SetControlState(eState *state)
{
    this->receiveObj->SetControlState(state);
}
void IOControl::SetConfigure(Configure *conf)
{
	this->receiveObj->SetConfigure(conf);
}

void IOControl::Init(string IP, int ReceivePort, int TransmitPort)
{
    udp=new itr_system::Udp(ReceivePort,false);
    receiveObj = new SEPReceive;
    protocolSerial.AddReceiveFun(receiveObj);
    udpPackage.port = TransmitPort;
    udpPackage.IP = IP;
    udpPackage.pbuffer=SendBuf;

    const char *filepath = "/dev/shm/SPRepeaterLite_ShareFile";
    int fd;
    if ((fd = open(filepath, O_CREAT | O_RDWR, (mode_t)00700)) == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    memBuf= (char *) mmap(NULL, 32*1024*1024, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (memBuf == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
}

IOControl::~IOControl()
{
    delete udp;
    delete receiveObj;
}

void IOControl::CheckIncomingData()
{
    int len;
    int count = 0;
    while (1)
    {
        len = udp->Receive(RecBuf, MaxRecLength);
        if(len<=0)
            break;
        count += protocolSerial.ProcessByte((U8 *) RecBuf,0, len);
    }
    printf("receive : %d\n",count);
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

    int *length= (int *) memBuf;
    if(*length == 0)
    {
        MemoryCopy(memBuf+4,udpPackage.pbuffer,udpPackage.len);
        *length = udpPackage.len;
    }
    else
    {
        perror("Skip one frame");
    }
    printf("%d\n",udp->Send(udpPackage));
}
