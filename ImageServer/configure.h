//
// Created by ghdawn on 16-2-28.
//

#ifndef IMAGESERVER_CONFIGURE_H
#define IMAGESERVER_CONFIGURE_H

#include <platform/typedef.h>
#include <string>
using std::string;
const int MaxSendLength=65535;
const int MaxRecLength=200;

enum eState
{
    IDLE=0,CAPTURE,TRACK,EXIT
};

class Configure
{
public:
    Configure();
    void Parse(string conf);
    void Parse(int argc, char **argv);
    string ToString();
    void SetIP(U32 ip);
    string IP;
    int transmitPort,receivePort;
    int width,height;
    int encoderWidth,encoderHeight;
    U32 cameraID,cameraTunnel;
    int fps;
    U8 userOptBit;
    U8 encoderQuality;
    int targetHeight;
    int targetWidth;
    int targetx;
    int targety;
    int confidence;
};


#endif //IMAGESERVER_CONFIGURE_H
