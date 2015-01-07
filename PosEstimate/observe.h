#ifndef OBSERVE_H
#define OBSERVE_H
#include "itrbase.h"
#include "itrvision.h"

class Observe
{
public:
    void Init();
    Vector PosEstimate(F32 Ix,F32 Iy,S32 height,Vector GPS, Vector att);
    static const F32 EarthR=6371004;
    itr_vision::CameraInterCalc interCalc;
    Matrix Ccb;
private:
    F32 MetertoLatDegree(F32 dm);
    F32 MetertoLonDegree(F32 lat,F32 dm);

};


#endif
