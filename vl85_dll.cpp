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
        1 - vsp compressor  1
        2 - compressor 2
        3 - MV1 4
        4 - MV2 8
        5 - MV3 16
        6 - MV4 32
        7 - battery state 64
1 - throttle position
2 - speed position
3 - EPK timer
4 - EPK state
5 - Battery Charge (V)
6 - 32L reservoir pressure
7 - 150L reservoir pressure

*/

extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
        UINT *Flags=(UINT *)&eng->var[0];
        Cabin *cab=eng->cab;

        eng->var[5] = 50.0;
        eng->var[6] = 0.0;
        eng->var[7] = 0.0;
        loco->MainResPressure = 0.0;

        setSwitch (94, 0, true);
        setSwitch (105, 1, true);
        setSwitch (106, 1, true);
        setSwitch (107, 1, true);
        setSwitch (108, 1, true);
        switch (State&0xFF)
        {
                //холодный
                case 0:
                        setSwitch (85, 0, true);
                        break;
                //включён(под составом)
                case 1:
                        setSwitch (85, 1, true);
                        break;
                //движется резервом
                case 2:
                        setSwitch (85, 1, true);
                        break;
                //движется с составом
                case 3:
                        setSwitch (85, 1, true);
                        break;
        }
}

extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
        switch (SwitchID)
        {

                case 85:
                // Battery ON
                        Flags|=64;
                        break;
                case 94:
                        if (Flags&64) {
                                if (!(cab->Switch(105))||!(cab->Switch(106))||\
                                !(cab->Switch(107))||!(cab->Switch(108)))
                                {
                                        Flags|=1;
                                }
                        }
                        
        
                default:
                        break;
        }
}

extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
        //отключение вспомогательного компрессора (при неверном положении кранов)()
        if ((cab->Switch(105))&&(cab->Switch(106))&&(cab->Switch(107))&&(cab->Switch(108)))
                Flags&=!1;
        //отключение вспомогательного компрессора (по тумблеру)
        if (cab->Switch(94))
                Flags&=!1;

        //каждую секунду +0,019
        if (Flag&1)
        {
                eng->var[6]+=0.019*time;
        }

        //отключение вспом. компрессора "помощником"
        if (eng->var[6]>6.0)
        {
             Flags&=!1;
             SetSwitch(94,0, true);  
        }

        // battery voltmeter
        if (Flags&64)
                cab->SetDisplayValue(11, eng->var[5]);
        else
                cab->SetDisplayValue(11, 0);
        //та ша
        //какой-то костыль от Теда (не понятно)
        if((State>>8)&1)
                float val;
        //манометр ТЦ
        if(loco->BrakeCylinderPressure>loco->IndependentBrakePressure)
                val=loco->BrakeCylinderPressure;
        else
                val=loco->IndependentBrakePressure;
        cab->SetDisplayValue(6,val);
        //манометр ГР
        cab->SetDisplayValue(7, loco->MainResPressure);
        cab->SetDisplayValue(8, loco->TrainPipePressure);
        cab->SetDisplayValue(9, eng->UR);
        //манометр резервуара ЦУ
        cab->SetDisplayValue(13, eng->var[7]);
        //манометр резервуара ТП
        cab->SetDisplayValue(14, eng->var[6]);
        
}