#include <iostream>
#include "itrbase.h"
#include "itrvision.h"
#include "itrdevice.h"
#include "itrsystem.h"
#include "observe.h"
#include "laserurg.h"

using namespace std;

F32 pos_x, pos_y;
S32 height;
Vector GPS, AttPRY;

const int RecPort = 9033;
bool stop = false;

const U8 IDLE = 0;
const U8 CAPTURE = 1;
const U8 TRACK = 2;
const U8 EXIT = 3;
U8 mode = IDLE;

class RecFunc : public itr_protocol::StandSerialProtocol::SSPDataRecFun {
    void Do(itr_protocol::StandSerialProtocol *SSP, itr_protocol::StandSerialFrameStruct *SSFS, U8 *Package, S32 PackageLength) {
        switch (Package[0]) {
            case 0x41:
                mode = Package[1];
                break;
            case 0x40:
                pos_x = *((F32 *) (Package + 6));
                pos_y = *((F32 *) (Package + 10));
                break;
            case 0x10:
                S16 tmp16;
                tmp16 = *((S16 *) (Package + 3));
                AttPRY[0] = tmp16 * 0.1f;
                tmp16 = *((S16 *) (Package + 5));
                AttPRY[1] = tmp16 * 0.1f;
                tmp16 = *((S16 *) (Package + 7));
                AttPRY[2] = tmp16 * 0.1f;
                break;
            case 0x11:
                S32 tmp32;
                U16 tmpu16;
                tmp32 = *((S32 *) (Package));
                GPS[0] = tmp32 * 1e-7f;
                tmp32 = *((S32 *) (Package + 4));
                GPS[1] = tmp32 * 1e-7f;
                tmpu16 = *((U16 *) (Package + 8));
                GPS[2] = tmpu16 * 0.2f - 1000.0f;
                break;
            default:
                break;
        }
    }
};


char laser_dev[30];
char fc_dev[30];

void OnLaserDataReceive(int *data, int length)
{
    for (int i = -5; i < 5; ++i)
    {
        printf("%d ", data[i + length / 2]);
    }
    cout << endl;
}
void *Image_thread(void *) {
    itr_system::Udp udp(RecPort, false);
    itr_protocol::StandSerialProtocol ssp;
    ssp.Init(0xA5, 0x5A, NULL);
    ssp.AddDataRecFunc(new RecFunc, 0);
    const int RecLength = 50;
    char RecBuf[RecLength];
    while (mode != EXIT)
    {
        int len = udp.Receive(RecBuf, RecLength);
        if (len > 0) {
            ssp.ProcessRawByte((U8 *) RecBuf, RecLength);
//            printf("Get Data\n");
        }

    }
}


void *FC_thread(void *)
{
    itr_protocol::StandSerialProtocol ssp;
    ssp.Init(0xA5, 0x5A, NULL);
    ssp.AddDataRecFunc(new RecFunc, 0);
    itr_system::SerialPort serialPort;
    if (serialPort.Init(fc_dev, 115200) != 0)
    {
        PRINT_DEBUG("No serial port!");
        return NULL;
    }
    const int RecLength = 150;
    U8 RecBuf[RecLength];
    while (mode != EXIT)
    {
        int len = serialPort.Read(RecBuf, RecLength);
        if (len > 0)
        {
            ssp.ProcessRawByte(RecBuf, RecLength);
        }
    }
}


void Init(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
                case 'f':
                    strncpy(fc_dev, &argv[i][2], strlen(argv[i]) - 2);
                    break;
                case 'l':
                    strncpy(laser_dev, &argv[i][2], strlen(argv[i]) - 2);
                    break;
                default:
                    break;
            }
        }
    }
    LaserInit(laser_dev, 115200);
    LaserSetProcess(OnLaserDataReceive);
    LaserStart();
    getchar();
}


bool isOK(F32 value, F32 target)
{
    const F32 eps = 1e-6;
    if (fabs(value - target) > eps)
        return false;
    return true;
}

int main(int argc, char **argv)
{
    Init(argc, argv);
    itr_math::MathObjStandInit();
    Vector result(3);
    GPS.Init(3);
    AttPRY.Init(3);

    pthread_t tidImage, tidFC;
    pthread_create(&tidImage, NULL, Image_thread, (void *) ("Image Data"));
    pthread_create(&tidFC, NULL, FC_thread, (void *) ("FC"));


    Observe obs;
    obs.Init();

    CycleQueue<Vector> posData;
    CycleQueue<F32> lat;
    CycleQueue<F32> lon;
    CycleQueue<F32> alt;
    const S32 Capacity = 500;
    posData.Init(Capacity);
    lat.Init(Capacity);
    lon.Init(Capacity);
    alt.Init(Capacity);
    F32 midlat, midlon, midalt;
    S32 ncount = 0;
    while (mode != EXIT)
    {
        result = obs.PosEstimate(pos_x, pos_y, height, GPS, AttPRY);
        posData.Insert(result);
        lat.Insert(result[0]);
        lon.Insert(result[1]);
        alt.Insert(result[2]);

        S32 length = lat.GetLength();
        itr_math::StatisticsObj->Median(lat.GetBase(), length, midlat);
        itr_math::StatisticsObj->Median(lon.GetBase(), length, midlon);
        itr_math::StatisticsObj->Median(alt.GetBase(), length, midalt);
        ncount = 0;
        result[0] = result[1] = result[2] = 0;
        for (int i = 0; i < length; ++i)
        {
            if (isOK(lat[i], midlat)
                    && isOK(lon[i], midlon)
                    && isOK(alt[i], midalt))
            {
                result[0] += lat[i];
                result[1] += lon[i];
                result[2] += alt[i];
                ++ncount;
            }
        }
        result.Mul(1f / ncount);
        itr_math::helpdebug::PrintVector(result);
        std::cout << std::endl;
        sleep(1);
    }
    cout << "Hello, World!" << endl;
    return 0;
}
