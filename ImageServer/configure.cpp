//
// Created by ghdawn on 16-2-28.
//

#include "configure.h"
#include <cstring>
#include <sstream>
#include <cstdio>

using namespace std;

Configure::Configure()
{
    this->cameraID = 0;
    this->cameraTunnel = 0;
    this->encoderWidth = 320;
    this->encoderHeight = 240;
    this->width = 320;
    this->height = 240;
    this->fps = 30;
    this->IP = "127.0.0.1";
    this->receivePort = 9031;
    this->transmitPort = 9032;
    this->userOptBit = 0x1f;
    this->encoderQuality = 10;
    this->targetx = 160;
    this->targety = 120;
    this->targetWidth = 40;
    this->targetHeight = 40;
    this->confidence = 0;
}

void Configure::Parse(string conf)
{
    stringstream strstream(conf);
    string str;
    while (strstream >> str)
    {
        if (str == "-ip")
        {
            strstream >> IP;
        }
        else if (str == "-tport")
        {
            strstream >> transmitPort;
        } else if (str == "-rport")
        {
            strstream >> receivePort;
        }
        else if (str == "-trackres")
        {
            strstream >> width;
            strstream >> height;
            targetx = width/2;
            targety = height/2;
        }
        else if (str == "-encoderres")
        {
            strstream >> encoderWidth;
            strstream >> encoderHeight;
        }
        else if (str == "-cameraid")
        {
            strstream >> cameraID;
            strstream >> cameraTunnel;
        }
        else if (str == "-fps")
        {
            strstream >> fps;
        }
        else if(str == "-quality")
        {
            strstream >> encoderQuality;
        }
    }
}

void Configure::Parse(int argc, char **argv)
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
                sscanf(argv[i + 1], "%d", &transmitPort);
                i += 2;
            }
            else if (strcmp(argv[i], "-rport") == 0)
            {
                sscanf(argv[i + 1], "%d", &receivePort);
                i += 2;
            }
            else if (strcmp(argv[i], "-trackres") == 0)
            {
                sscanf(argv[i + 1], "%d", &width);
                sscanf(argv[i + 2], "%d", &height);
                i += 3;
            }
            else if (strcmp(argv[i], "-encoderres") == 0)
            {
                sscanf(argv[i + 1], "%d", &encoderWidth);
                sscanf(argv[i + 2], "%d", &encoderHeight);
                i += 3;
            }
            else if (strcmp(argv[i], "-cameraid") == 0)
            {
                sscanf(argv[i + 1], "%d", &cameraID);
                sscanf(argv[i + 2], "%d", &cameraTunnel);
                i += 3;
            }
            else if (strcmp(argv[i], "-fps") == 0)
            {
                sscanf(argv[i + 1], "%d", &fps);
                i += 2;
            }
            else
            {
                i++;
            }
        }
    }
}

string Configure::ToString()
{
    stringstream strstream;
    strstream << "-ip ";
    strstream << IP;
    strstream << " -tport ";
    strstream << transmitPort;
    strstream << " -rport ";
    strstream << receivePort;
    strstream << " -trackres ";
    strstream << width<<' ';
    strstream << height;
    strstream << " -encoderres ";
    strstream << encoderWidth<<' ';
    strstream << encoderHeight;
    strstream << " -cameraid ";
    strstream << cameraID<<' ';
    strstream << cameraTunnel;
    strstream << " -fps ";
    strstream << fps;
    strstream << " -quality ";
    strstream << encoderQuality;
    return strstream.str();
}

void Configure::SetIP(U32 ip)
{
    stringstream str;
	U8* ptr = (U8*)&ip;
    str<<*(ptr+3)<<".";
    str<<*(ptr+2)<<".";
    str<<*(ptr+1)<<".";
    str<<*(ptr+0)<<".";
}







