//---------------------------------------------------------------------------

#define RTS_STACKSIZE 15 // поправить по необходимости
//#define RTS_ADAPTER_COMPLY

#include <windows.h>
//#include <math>
#include "ts.h"

#define BRAKE_STR_RATE 1.8
#define TR_CURRENT_C 272.0
#define BRAKE_MR_RATIO    0.005
#define BRAKE_PIPE_RATE_CHARGE 2.5
#define BRAKE_UR_RATE_CHARGE   0.3
#define BRAKE_PIPE_RATE 0.4
#define BRAKE_PIPE_EMERGENCY -1.2
#define PIPE_DISCHARGE_SLOW -0.005
#define UR_DISCHARGE2     0.003

#define BATTERY_DEADLINE 9.7

#pragma argsused

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------

/*
Stack variables
0 - unsigned long Flags
        1 - vsp compressor
        2 - compressor
        3 - MV1
        4 - MV2
        5 - MV3
        6 - MV4
        7 - battery state
1 - throttle position
2 - speed position
3 - EPK timer
4 - EPK state
5 - Battery Charge(V)

*/

extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
        UINT *Flags=(UINT *)&eng->var[0];
        Cabin *cab=eng->cab;

        eng->var[5] = 50.0
        switch (State&0xFF)
        {
                case 0:
                        setSwitch (85, 0, true)
                        break;
                case 1:
                        setSwitch (85, 0, true)
                        break;
                case 2:
                        setSwitch (85, 1, true)
                        break;
                case 3:
                        setSwitch (85, 1, true)
                        break;
        }
}

extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
        switch (SwitchID)
        {
                case 85:
                        Flags|=128;
                        break;
        
                default:
                        break;
        }
}

extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
        if Flags&128
        cab->SetDisplayValue(11, eng->var[5]);
        else
        cab->SetDisplayValue(11, 0);
        
}
