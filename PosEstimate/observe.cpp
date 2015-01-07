#include "observe.h"

F32 Observe::MetertoLatDegree(F32 dm)
{
    return RAD2ANG(dm/EarthR);
}

F32 Observe::MetertoLonDegree(F32 lat,F32 dm)
{
    return RAD2ANG(dm/(EarthR*cos(ANG2RAD(lat))));
}

void Observe::Init()
{
    interCalc.SetPara(0.04, 0.01, 0.01, 300, 300);
    Ccb.Init(4, 4);
    Ccb.SetDiag(1.0);
}

Vector Observe::PosEstimate(F32 Ix, F32 Iy, S32 height, const Vector &GPS, const Vector &AttPRY)
{
    Vector PixelPoint(3),CameraPoint(4),cameraPoint(3);
    PixelPoint[0]=Ix;
    PixelPoint[1]=Iy;
    PixelPoint[2]=1;
    interCalc.CalcP2C(PixelPoint, height / 1000.0f, CameraPoint);
    cameraPoint.CopyFrom(CameraPoint.GetData());
    itr_math::helpdebug::PrintVector(cameraPoint);

    Matrix R2(3,3),t2(3,1);
    Ccb.CopyTo(0, 0, 3, 3, R2.GetData());
    Ccb.CopyTo(0,3,1,3,t2.GetData());

//    itr_math::helpdebug::PrintMatrix(Ccb);
//    itr_math::helpdebug::PrintMatrix(R2);
//    itr_math::helpdebug::PrintMatrix(t2);

    F32 sinpitch,sinroll,sinyaw;
    F32 cospitch,cosroll,cosyaw;
    itr_math::NumericalObj->Cos(AttPRY[0], cospitch);
    itr_math::NumericalObj->Cos(AttPRY[1], cosroll);
    itr_math::NumericalObj->Cos(AttPRY[2], cosyaw);
    itr_math::NumericalObj->Sin(AttPRY[0], sinpitch);
    itr_math::NumericalObj->Sin(AttPRY[1], sinroll);
    itr_math::NumericalObj->Sin(AttPRY[2], sinyaw);
    F32 Rdata[]={cospitch*cosyaw,sinroll*sinpitch*cosyaw-cosroll*sinyaw,cosroll*sinpitch*cosyaw+sinroll*sinyaw,
            cospitch*sinyaw,sinroll*sinpitch*sinyaw+cosroll*cosyaw,cosroll*sinpitch*sinyaw-sinroll*cosyaw,
            -sinpitch,sinroll*cospitch,cosroll*cospitch};
    Matrix R(3,3,Rdata);
    itr_math::helpdebug::PrintMatrix(R);
    Vector temp = R * R2 * cameraPoint;
    temp[0]= MetertoLatDegree(temp[0]);
    temp[1] = MetertoLonDegree(GPS[1], temp[1]);
    temp[0] += GPS[1];
    temp[1] += GPS[0];
    return temp;
}