//---------------------------------------------------------------------------

#define RTS_STACKSIZE 15 // поправить по необхоодимости
//#define RTS_ADAPTER_COMPLY

#include <windows.h>
#include <math>
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
1 - unsigned long Flags
 vsp compressor
 compressor
 MV1
 MV2
 MV3
 MV4

*/

extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
        switch (SwitchID)
        {
                case /* constant-expression */:
                        /* code */
                        break;
        
                default:
                        break;
        }
}

extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
        
}