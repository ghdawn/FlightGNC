#include <iostream>
#include "itrbase.h"
#include "itrvision.h"
#include "itrsystem.h"
#include "observe.h"

F32 pos_x, pos_y;
Vector GPS, AttPRY;

class RecFunc : public itr_protocol::StandSerialProtocol::SSPDataRecFun {
    void Do(itr_protocol::StandSerialProtocol *SSP, itr_protocol::StandSerialFrameStruct *SSFS, U8 *Package, S32 PackageLength) {
        switch (Package[0]) {
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

const int RecPort = 9033;
bool stop = false;

void *Image_thread(void *) {
    itr_system::Udp udp(RecPort, false);
    itr_protocol::StandSerialProtocol ssp;
    ssp.Init(0xA5, 0x5A, NULL);
    ssp.AddDataRecFunc(new RecFunc, 0);
    const int RecLength = 50;
    char RecBuf[RecLength];
    while (!stop) {
        int len = udp.Receive(RecBuf, RecLength);
        if (len > 0) {
            ssp.ProcessRawByte((U8 *) RecBuf, RecLength);
            printf("Get Data\n");
        }

    }
}

void *Laser_thread(void *);

void *FC_thread(void *);

using namespace std;


int main() {
    itr_math::MathObjStandInit();

    pthread_t tidImage, tidLaser, tidFC;

    pthread_create(&tidImage, NULL, Image_thread, (void *) ("Image Data"));
//    pthread_create(&tidLaser, NULL, Laser_thread, (void *) ("Laser"));
//    pthread_create(&tidFC, NULL, FC_thread, (void *)( "FC" ));

    Vector tmp(3);
    GPS.Init(3);
    AttPRY.Init(3);
    Observe obs;
    obs.Init();
    char c = 0;
    while (c == 0) {
        tmp = obs.PosEstimate(pos_x, pos_y, 1000, GPS, AttPRY);
        itr_math::helpdebug::PrintVector(tmp);
        std::cout << std::endl;
        sleep(1);
    }
    cout << "Hello, World!" << endl;
    return 0;
}