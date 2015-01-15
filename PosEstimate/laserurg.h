#ifndef LASERURG_H
#define LASERURG_H

typedef void (*Process)(int *data, int length);

///初始化串口和波特率
bool LaserInit(char *dev, int baudrate);

void LaserSetProcess(Process process);

void LaserStart();

void LaserStop();

#endif // LASERURG_H
