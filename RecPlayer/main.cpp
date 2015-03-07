#include <iostream>
#include <sstream>
#include <itrbase.h>
#include "itrsystem.h"
using namespace std;

const int ImagePort = 9032, SensorPort = 9034;
F32 pos_x,pos_y;
struct SensorData
{
    F32 height;
    F32 x, y;
    S32 laserlength;
    S32 laser[780];
};
int imagelenth;
static const int MAX_BUFFER = 65535;
U8 imagebuffer[MAX_BUFFER];
char SendBuf[MAX_BUFFER];
SensorData sensordata;

void SendSensorData();

S32 SendLength;

class SSPSend : public itr_protocol::StandSerialProtocol::SSPDataSendFun {
    S32 Do(U8 *Buffer, S32 Length) {
        memcpy(SendBuf, Buffer, Length);
        memcpy(SendBuf + Length, imagebuffer, imagelenth);
        SendLength = Length + imagelenth;
        return SendLength;
    }
};
itr_protocol::StandSerialProtocol sspUdp;
void SendImageData()
{
    itr_system::Udp _udp;
    itr_system::Udp::UdpPackage udpPackage;


    SendLength=0;
    S32 offset=0;
    U8 tempbuff[100];
    memset(tempbuff,0,sizeof(tempbuff));
    tempbuff[offset++]=0x40;
    tempbuff[offset++]=3;
    offset+=4;
    memcpy(tempbuff+offset,(void*)&pos_x,4);
    offset+=4;
    memcpy(tempbuff+offset,(void*)&pos_y,4);
    offset+=4;
    offset+=4;
    sspUdp.SSPSendPackage(0,tempbuff,offset);
    // 发送结果
    udpPackage.pbuffer=SendBuf;
    udpPackage.port = ImagePort;
    udpPackage.IP = "127.0.0.1";
    udpPackage.len = SendLength;
    _udp.Send(udpPackage);
}
int main(int argc, char **argv)
{
    char filename[100];
    int i;
    sspUdp.Init(0xA5, 0x5A, new SSPSend);
    stringstream str;
    str<<argv[2];
    str>>i;
    for(;;i++)
    {
        sprintf(filename,argv[1],i);
        printf("%s\n",filename);
        FILE* fin=fopen(filename,"r");
        if(fin<=0)
            break;
        fscanf(fin,"%f %f\n",&pos_x,&pos_y);
        fscanf(fin,"%f %f %f\n",&(sensordata.x),&(sensordata.y),&(sensordata.height));
        fscanf(fin,"%d\n",&(sensordata.laserlength));
        for(int i=0;i<sensordata.laserlength;i++)
            fscanf(fin,"%d ",sensordata.laser+i);
        fscanf(fin,"%d\n",&imagelenth);
        for(int i=0;i<imagelenth;i++)
            fscanf(fin,"%c",imagebuffer+i);
        fclose(fin);
        printf("%f %f\n",pos_x,pos_y);
        printf("%f %f %f\n",sensordata.x,sensordata.y,sensordata.height);
        printf("%d\n",sensordata.laserlength);
        SendImageData();
        SendSensorData();
        usleep(50*1000);
    }
    return 0;
}

void SendSensorData()
{
    itr_system::Udp udp;
    itr_system::Udp::UdpPackage package;
    package.pbuffer = (char *) &sensordata;
    package.len = sizeof(sensordata);
    package.IP = "127.0.0.1";
    package.port = SensorPort;
    udp.Send(package);
}