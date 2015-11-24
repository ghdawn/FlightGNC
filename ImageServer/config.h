//
// Created by ghdawn on 15-11-24.
//

#ifndef IMAGESERVER_CONFIG_H_H
#define IMAGESERVER_CONFIG_H_H

const int MaxSendLength=65535;
const int MaxRecLength=200;

enum eState
{
    IDLE=0,CAPTURE,TRACK,EXIT
};
#endif //IMAGESERVER_CONFIG_H_H
