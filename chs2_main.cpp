//---------------------------------------------------------------------------

#include <windows.h>
#include <math>
#include "ts.h"

#define BATTERY_DEADLINE 45.0
#define BATTERY_HIGH     58.0
#define THROTTLE_SWITCH_SPEED 1.8

#define FORCE_RAISE 50000.0
#define FORCE_LOWER 80000.0
#define CURRENT_Q 285.0

#define UR_CHARGE_RATE 0.25
#define UR_DISCHARGE_RATE -0.25
#define UR_EMERGENCY_RATE -1.00
#define UR_DISCHARGE2     0.003
#define PIPE_CHARGE_Q_REL 0.8
#define PIPE_CHARGE_Q     0.7
#define PIPE_CHARGE_Q_4   0.3
#define PIPE_DISCHARGE_SLOW -0.005
#define EPT_RELEASE_RATE  0.75
#define EPT_RELEASE_RATE1 1.0
#define EPT_APPLY_RATE    1.0
#define EPT_MAX           4.0
#define MAX_TRAIN_PIPE_RATE 1.2

#define COLOUR_RED   0xF0FF0000
#define COLOUR_WHITE 0xFFEDBF5E

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------


UINT IsLocoOn(const ElectricEngine *eng,Cabin *cab){
 UINT LocoOn=0;

 if(eng->BatteryCharge>BATTERY_DEADLINE && cab->Switch(180)){
  LocoOn|=1;
  ULONG &Flags=*(unsigned long *)&eng->var[0];
  if(Flags&1){
   LocoOn|=2;
   if(!cab->Switch(9)&&cab->Switch(8))
    LocoOn|=8;
  }else if(Flags&2){
   LocoOn|=4;
   if(!cab->Switch(109)&&cab->Switch(108))
    LocoOn|=8;
  };
 };

 return LocoOn;
};

void SwitchBV(const ElectricLocomotive *loco,ElectricEngine *eng,UINT State){
 if(eng->MainSwitch!=signed(State)){
  eng->MainSwitch=State;
  if(State)
   loco->PostTriggerBoth(101);
  else
   loco->PostTriggerBoth(102);
 };
};

bool IsHP(short Position){
 if(Position==43 || Position==34 || Position==21)
  return true;
 return false;
};

int AspectToALSNState(UINT Aspect){
 switch(Aspect){
  case SIGASP_CLEAR_1:
  case SIGASP_CLEAR_2:
   return 1;
  case SIGASP_APPROACH_1:
  case SIGASP_APPROACH_2:
  case SIGASP_APPROACH_3:
   return 2;
  case SIGASP_STOP_AND_PROCEED:
   return 20;
  case SIGASP_STOP:
   return 4;
  case SIGASP_RESTRICTING:
   return 16;
  case SIGASP_BLOCK_OBSTRUCTED:
   return 8;
 };
 return 0;
};

UINT ApproachRed(float SigDist,float Vel){
 UINT res=0;
 if(SigDist<500.0){
  if(Vel>25.0)
   res=2;
  else if(Vel>16.7)
   res=1;
  if(SigDist<250.0){
   if(Vel>16.7)
    res=2;
   if(Vel>11.1 && res<1)
    res=1;
   if(SigDist<150.0){
    if(Vel>11.1)
     res=2;
    if(SigDist<100.0){
     if(Vel>5.5 && res<1)
      res=1;
     if(SigDist<50.0){
      if(Vel>5.5)
       res=2;
      else if(Vel>2.7 && res<1)
       res=1;
      if(SigDist<20.0){
       if(Vel>1.5)
        res=1;
       if(SigDist<10.0)
        if(Vel>1.5)
         res=2;
      };
     };
    };
   };
  };
 };
 return res;
};

void SwitchLights(const Locomotive *loco,int Condition){
 switch(Condition){
  case 0:
   loco->SwitchLight(0,false);
   loco->SwitchLight(1,false);
   loco->SwitchLight(4,false);
   loco->SwitchLight(5,false);
  break;
  case 1:
   loco->SwitchLight(0,false);
   loco->SwitchLight(1,true);
   loco->SwitchLight(4,true);
   loco->SwitchLight(5,false);
  break;
  case 2:
   loco->SwitchLight(0,true);
   loco->SwitchLight(1,false);
   loco->SwitchLight(4,false);
   loco->SwitchLight(5,true);
  break;
  case 3:
   loco->SwitchLight(6,false);
  break;
  case 4:
   loco->SwitchLight(6,true,0.0,COLOUR_WHITE);
  break;
  case 5:
   loco->SwitchLight(6,true,0.0,COLOUR_RED);
  break;
  case 6:
   loco->SwitchLight(7,false);
  break;
  case 7:
   loco->SwitchLight(7,true,0.0,COLOUR_WHITE);
  break;
  case 8:
   loco->SwitchLight(7,true,0.0,COLOUR_RED);
  break;
  case 10:
   loco->SwitchLight(27,false);
   loco->SwitchLight(28,false);
   loco->SwitchLight(29,false);
   loco->SwitchLight(30,false);
  break;
  case 11:
   loco->SwitchLight(27,false);
   loco->SwitchLight(28,true);
   loco->SwitchLight(29,true);
   loco->SwitchLight(30,false);
  break;
  case 12:
   loco->SwitchLight(27,true);
   loco->SwitchLight(28,false);
   loco->SwitchLight(29,false);
   loco->SwitchLight(30,true);
  break;
  case 13:
   loco->SwitchLight(8,false);
  break;
  case 14:
   loco->SwitchLight(8,true,0.0,COLOUR_WHITE);
  break;
  case 15:
   loco->SwitchLight(8,true,0.0,COLOUR_RED);
  break;
  case 16:
   loco->SwitchLight(31,false);
  break;
  case 17:
   loco->SwitchLight(31,true,0.0,COLOUR_WHITE);
  break;
  case 18:
   loco->SwitchLight(31,true,0.0,COLOUR_RED);
  break;

 };
};

/*

 Stack Variables

  0 - Flags
   1bite
    1bit - front control on
    2bit - back control on
    3,4bit - compressor 1,2 on
    5bit - ventilators on
    6bit - emergency brake incurred
    7bit - reverser emergency
    8bit - missed red
   2bite
    1bit - auxilary compressor
    2bit - 1st compressor
    3bit - 2nd compressor
   3bite
    1,2,3bit - TED 1,2,3 off
    4,5,6bit - TED 4,5,6 off


  1 - ControlFlags   (!USED IN SME!)
   1bite
    1,2bit - panto front/rear raised
    3,4bit - disconnector front/rear on
    5,6bit - compressor 1,2 auto
    7,8bit - compressor 1,2 manual
   2bite
    1bit - BV on
    2bit - BV off
    3bit - ventilators on
    4bit - ventilators stop
    5bit - sanding on


  2 - Sound flags
   1bite
    1bit - EPK started
    2bit - battery turned on

  3 - Throttle switch timer

  4 - Set throttle position (!USED IN SME!)

  5 - EPK timer

  6 - shunting position (!USED IN SME!)

  7 - throttle spinner position

  8 - previous force

  9 - EPT pressure

  10 - EPK state
   0 - off
   1 - approach
   2 - red

  11 - previous signal state

  12 - resistor accumulated heat

 Engine Flags
  1bite
   1bit - master loco


 LocoOn Flags
  1bite
   1bit - battery on
   2bit - front control on
   3bit - back control on
   4bit - EPK on

 Sound triggers
  4,5 - sander on off
  6,7 - vents on/off
  8,9 - tifon
  10,11 - svistok
  15 - reverser
  16 - throttle
  17 - train brake switch
  18 - loco brake switch
  25 - rb
  37 - switch2
  47 - switch
  56,57 - EPK on/off
  100 - throttle switch
  101 - BV on
  102 - BV off
  103,104 - compressor on/off
  105 - auxilary compressor
  106 - one compressor
  107 - two compressors
  108,112 - panto front up/down
  109 - switch valve
  110,111 - panto back up/down
  113 - EPK release
  114,115 - battery on/off

 Sound variables
  Variable2 - current

*/


extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 if(loco->NumEngines!=6)
  return false;
 Cabin *cab=loco->Cab();
 ULONG &Flags=*(unsigned long *)&eng->var[0];
 ULONG &ControlFlags=*(unsigned long *)&eng->var[1];
 loco->MainResPressure=4.7;
 loco->IndependentBrakePressure=4.0;
 loco->AuxiliaryPressure=4.0;
 loco->BrakeCylinderPressure=4.0;
 loco->TrainPipePressure=0.0;
 eng->IndependentBrakeValue=4.0;
 eng->HandbrakePercent=100.0;
 eng->UR=0.0;
 eng->BatteryCharge=55.5;
 eng->var[0]=0.0;
 eng->var[1]=0.0;
 eng->var[2]=0.0;
 eng->var[3]=0.0;
 eng->var[4]=0.0;
 eng->var[5]=0.0;
 eng->var[6]=0.0;
 eng->var[7]=0.0;
 eng->var[8]=0.0;
 eng->var[9]=4.0;
 eng->var[10]=0.0;
 eng->var[11]=0.0;
 eng->var[12]=0.0;

 switch(State&0xFF){
  case 1:
   loco->MainResPressure=9.0;
   loco->IndependentBrakePressure=4.0;
   loco->AuxiliaryPressure=5.0;
   loco->BrakeCylinderPressure=0.0;
   loco->TrainPipePressure=5.2;
   loco->PantoRaised=3;
   loco->HandbrakeValue=0.0;
   eng->IndependentBrakeValue=4.0;
   eng->HandbrakePercent=0.0;
   eng->UR=5.2;
   eng->EPTvalue=0.0;
   eng->MainSwitch=1;
   eng->Panto=3;
   eng->Reverse=0;
   eng->var[9]=0.0;

   cab->SetSwitch(180,1,true);
   cab->SetSwitch(  5,1,true);
   cab->SetSwitch(105,1,true);
   cab->SetSwitch(  6,1,true);
   cab->SetSwitch(106,1,true);
   if((State>>8)&1){
    cab->SetSwitch( 13,0,true);
    cab->SetSwitch(  4,1,true);
    cab->SetSwitch(  7,1,true);
    cab->SetSwitch( 14,0,true);
    cab->SetSwitch( 22,1,true);
    cab->SetSwitch( 24,1,true);
    cab->SetSwitch( 25,0,true);
    cab->SetSwitch( 28,1,true);
    cab->SetSwitch( 30,1,true);
    cab->SetSwitch( 31,0,true);
    eng->Flags|=1;
   };
   Flags=1;
   ControlFlags=63;
  break;
  case 2:
  case 3:
   loco->MainResPressure=9.0;
   loco->IndependentBrakePressure=0.0;
   loco->AuxiliaryPressure=5.0;
   loco->BrakeCylinderPressure=0.0;
   loco->TrainPipePressure=5.2;
   loco->PantoRaised=3;
   loco->HandbrakeValue=0.0;
   eng->IndependentBrakeValue=0.0;
   eng->HandbrakePercent=0.0;
   eng->UR=5.2;
   eng->EPTvalue=0.0;
   eng->MainSwitch=1;
   eng->Panto=3;
   eng->var[9]=0.0;

   eng->Reverse=(loco->Velocity>=0.0 ^ bool(loco->Flags&1))?1:-1;
   cab->SetSwitch(180,1,true);
   cab->SetSwitch(  5,1,true);
   cab->SetSwitch(105,1,true);
   cab->SetSwitch(  6,1,true);
   cab->SetSwitch(106,1,true);
   if((State>>8)&1){
    if(eng->Reverse==1){
     cab->SetSwitch( 13,0,true);
     cab->SetSwitch(  1,4,true);
     cab->SetSwitch(  4,1,true);
     cab->SetSwitch(  7,1,true);
     cab->SetSwitch(  9,0,true);
     cab->SetSwitch( 14,1,true);
     cab->SetSwitch( 22,1,true);
     cab->SetSwitch( 24,1,true);
     cab->SetSwitch( 25,0,true);
     cab->SetSwitch( 28,1,true);
     cab->SetSwitch( 30,1,true);
     cab->SetSwitch( 31,0,true);
     Flags=1;
    }else{
     cab->SetSwitch(113,0,true);
     cab->SetSwitch(101,4,true);
     cab->SetSwitch(104,1,true);
     cab->SetSwitch(107,1,true);
     cab->SetSwitch(109,0,true);
     cab->SetSwitch(114,1,true);
     cab->SetSwitch(122,1,true);
     cab->SetSwitch(124,1,true);
     cab->SetSwitch(125,0,true);
     cab->SetSwitch(128,1,true);
     cab->SetSwitch(130,1,true);
     cab->SetSwitch(131,0,true);
     Flags=2;
    };
    eng->Flags|=1;
   };
   ControlFlags=63;
  break;
 };

 eng->ShowMessage(GMM_POST,L"CHS2 initialized");

 return true;
};


extern "C" void __export  ChangeLoco
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 if(!Prev || !(Prev->Eng()->Flags&1))
  loco->LocoFlags|=1;
};

extern "C" void __export  LostMaster
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{

};

extern "C" void __export  NewSlave
(Locomotive *loco,const Locomotive *Master,unsigned long State)
{

};


extern "C" bool __export CanWorkWith(const Locomotive *loco,const wchar_t *Type){

 if(!lstrcmpiW(Type,L"chs2"))
  return true;
 if(!lstrcmpiW(Type,L"chs2z"))
  return true;
 if(!lstrcmpiW(Type,L"chs2k"))
  return true;

 return false;
};



extern "C" bool __export  CanSwitch(const ElectricLocomotive *loco,const ElectricEngine *eng,
        unsigned int SwitchID,unsigned int SetState)
{
 Cabin *cab=loco->Cab();
 //UINT LocoOn=IsLocoOn(eng,cab);
 switch(SwitchID){
  case 0:
   if(!eng->Reverse || !cab->SwitchSet(1) || cab->SwitchSet(1)==3)
    return false;
   if(cab->SwitchSet(3))
    return false;
   if(cab->SwitchSet(9))
    return false;
   if(int(SetState)>cab->Switch(0)){
    if(eng->ThrottlePosition<1 && SetState>1)
     return false;
    if(eng->ThrottlePosition<21 && SetState>21)
     return false;
    if(eng->ThrottlePosition<34 && SetState>34)
     return false;
   };
  break;
  case 1:
   if(cab->SwitchSet(0) || eng->ThrottlePosition || eng->var[4]>0.0)
    return false;
   if(cab->SwitchSet(101))
    return false;
  break;
  case 2:
   if(eng->Reverse)
    return false;
   if(cab->SwitchSet(1) && cab->SwitchSet(1)!=3)
    return false;
  break;
  case 3:
   if(!IsHP(cab->SwitchSet(0)) || int(eng->var[7])!=cab->SwitchSet(0))
    return false;
  break;

  case 100:
   if(!eng->Reverse || !cab->SwitchSet(101) || cab->SwitchSet(101)==3)
    return false;
   if(cab->SwitchSet(103))
    return false;
   if(cab->SwitchSet(109))
    return false;
   if(int(SetState)>cab->Switch(100)){
    if(eng->ThrottlePosition<1 && SetState>1)
     return false;
    if(eng->ThrottlePosition<21 && SetState>21)
     return false;
    if(eng->ThrottlePosition<34 && SetState>34)
     return false;
   };
  break;
  case 101:
   if(cab->SwitchSet(100) || eng->ThrottlePosition || eng->var[4]>0.0)
    return false;
   if(cab->SwitchSet(1))
    return false;
   //if(!(LocoOn&4))
    //return false;
  break;
  case 102:
   if(eng->Reverse)
    return false;
   if(cab->SwitchSet(101) && cab->SwitchSet(101)!=3)
    return false;
  break;
  case 103:
   if(!IsHP(cab->SwitchSet(100)) || int(eng->var[7])!=cab->SwitchSet(100))
    return false;
  break;
 };

 switch(SwitchID){
  case 0: case 100:
   loco->PostTriggerCab(16);
  break;
  case 7: case 107:
  case 8: case 108:
  case 40: case 140:
  case 41: case 141:
   loco->PostTriggerCab(109);
  break;
  case 10: case 110:
   if(SetState)
    loco->PostTriggerCab(25);
  break;
  case 13: case 113:
  case 14: case 114:
  case 15: case 115:
  case 180:
   loco->PostTriggerCab(37);
  break;
  case 16: case 116:
  case 17: case 117:
  case 18: case 118:
  case 19: case 119:
  case 21: case 121:
  case 22: case 122:
   loco->PostTriggerCab(47);
  break;
 };

 if((SwitchID>=124 && SwitchID<=133)||(SwitchID>=24 && SwitchID<=33))
  loco->PostTriggerCab(47);

 return true;
};


extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
 Cabin *cab=loco->Cab();
 UINT LocoOn=IsLocoOn(eng,cab),state;
 ULONG &Flags=*(unsigned long *)&eng->var[0];
 ULONG &ControlFlags=*(unsigned long *)&eng->var[1];
 FreeAnimation *anim;

 switch(SwitchID){
  case 180:
   if(cab->Switch(180)){
    Switched(loco,eng, 13,0);
    Switched(loco,eng,113,0);
   };
   Switched(loco,eng, 21,0);
   Switched(loco,eng, 26,0);
   Switched(loco,eng, 27,0);
   Switched(loco,eng, 22,0);
   Switched(loco,eng, 32,0);
   Switched(loco,eng, 33,0);
   Switched(loco,eng,121,0);
   Switched(loco,eng,126,0);
   Switched(loco,eng,127,0);
   Switched(loco,eng,122,0);
   Switched(loco,eng,132,0);
   Switched(loco,eng,133,0);
  break;
  case 13:
   //control
   if((LocoOn&1) && !(LocoOn&4)){
    if(!cab->Switch(13)){
     Flags|=1;
     eng->Flags|=1;
     Switched(loco,eng, 11,0);
     Switched(loco,eng, 12,0);
     Switched(loco,eng, 16,0);
     Switched(loco,eng, 17,0);
     Switched(loco,eng, 18,0);
     Switched(loco,eng, 19,0);
     Switched(loco,eng, 22,0);
     Switched(loco,eng, 24,0);
     Switched(loco,eng, 25,0);
     Switched(loco,eng, 28,0);
     Switched(loco,eng, 29,0);
     Switched(loco,eng, 30,0);
     Switched(loco,eng, 31,0);
    }else if(Flags&1){
     Flags&=~1;
     if(cab->Switch(113)){
      eng->Flags&=~1;
      ControlFlags=0;
     }else
      Switched(loco,eng,113,0);
    };
   };
  break;
  case 113:
   if((LocoOn&1) && !(LocoOn&2)){
    if(!cab->Switch(113)){
     Flags|=2;
     eng->Flags|=1;
     Switched(loco,eng,111,0);
     Switched(loco,eng,112,0);
     Switched(loco,eng,116,0);
     Switched(loco,eng,117,0);
     Switched(loco,eng,118,0);
     Switched(loco,eng,119,0);
     Switched(loco,eng,122,0);
     Switched(loco,eng,124,0);
     Switched(loco,eng,125,0);
     Switched(loco,eng,128,0);
     Switched(loco,eng,129,0);
     Switched(loco,eng,130,0);
     Switched(loco,eng,131,0);
    }else if(Flags&2){
     Flags&=~2;
     if(cab->Switch(13)){
      eng->Flags&=~1;
      ControlFlags=0;
     }else
      Switched(loco,eng,13,0);
    };
   };
  break;
  case 1:
   //reverser
   if((LocoOn&3)==3){
    state=cab->Switch(1);
    Flags&=~64;
    switch(state){
     case 1: Flags|=64;
     case 2: eng->Reverse=-1;  break;
     case 3: eng->Reverse=0;  break;
     case 5: Flags|=64;
     case 4: eng->Reverse=1; break;
    };
   };
   loco->PostTriggerCab(15);
  break;
  case 101:
   if((LocoOn&5)==5){
    state=cab->Switch(101);
    Flags&=~64;
    switch(state){
     case 1: Flags|=64;
     case 2: eng->Reverse=1;  break;
     case 3: eng->Reverse=0;  break;
     case 5: Flags|=64;
     case 4: eng->Reverse=-1; break;
    };
   };
   loco->PostTriggerCab(15);
  break;
  case 2:
  case 102:
   //TED switcher
   Flags&=~(0x3F<<16);
   switch(cab->Switch(2)){
    case 3: Flags|=3<<16; break;
    case 4: Flags|=12<<16; break;
    case 5: Flags|=48<<16; break;
   };
   switch(cab->Switch(102)){
    case 3: Flags|=48<<16; break;
    case 4: Flags|=12<<16; break;
    case 5: Flags|=3<<16; break;
   };
   loco->PostTriggerCab(15);
  break;
  case 3:
   //OP
   if(LocoOn&2)
    eng->var[6]=cab->Switch(3);
   loco->PostTriggerCab(15);
  break;
  case 103:
   if(LocoOn&4)
    eng->var[6]=cab->Switch(103);
   loco->PostTriggerCab(15);
  break;
  case 4:
   //Train brake
   loco->PostTriggerCab(17);
   if(cab->Switch(7))
    loco->PostTriggerCab(18);
  break;
  case 104:
   //Train brake
   loco->PostTriggerCab(17);
   if(cab->Switch(107))
    loco->PostTriggerCab(18);
  break;
  case 5:
  case 105:
   //Loco brake
   if(loco->LocoFlags&1){
    UINT st=cab->Switch(105),st1=cab->Switch(5);
    if( st==1 || (st1>st && st1!=1))
     st=st1;
    float nv;
    switch(st){
     case 0: eng->IndependentBrakeValue=0.0; break;
     case 1: eng->IndependentBrakeValue=loco->IndependentBrakePressure; break;
     case 2:
     case 3:
     case 4:
     case 5:
      nv=(st-1)*1.0;
      if(nv>eng->IndependentBrakeValue)
       eng->IndependentBrakeValue=nv;
      else
       eng->IndependentBrakeValue=loco->IndependentBrakePressure;
     break;
    };
   };
   loco->PostTriggerCab(18);
  break;
  case 10: case 110:
   //RB
   if(((LocoOn&2)&&(cab->Switch(10)))||((LocoOn&4)&&(cab->Switch(110)))){
    eng->var[5]=0.0;
    eng->var[10]=0.0;
   };
  break;
  case 11: case 111:
   //whistle
   if(((LocoOn&2)&&(cab->Switch(11)))||((LocoOn&4)&&(cab->Switch(111))))
    loco->PostTriggerBoth(10);
   else
    loco->PostTriggerBoth(11);
  break;
  case 12: case 112:
   //horn
   if(((LocoOn&2)&&(cab->Switch(12)))||((LocoOn&4)&&(cab->Switch(112))))
    loco->PostTriggerBoth(8);
   else
    loco->PostTriggerBoth(9);
  break;
  case 14:
   //EPT
   if(LocoOn&2)
    eng->var[9]=loco->BrakeCylinderPressure;
  break;
  case 114:
   //EPT
   if(LocoOn&4)
    eng->var[9]=loco->BrakeCylinderPressure;
  break;
  case 19:
   //ventilators
   if((LocoOn&2)){
    if(cab->Switch(19))
     ControlFlags|=1024;
    else
     ControlFlags&=~1024;
   };
  break;
  case 119:
   //ventilators
   if(LocoOn&4){
    if(cab->Switch(119))
     ControlFlags|=1024;
    else
     ControlFlags&=~1024;
   };
  break;
  case 20:
   //ventilators off
   if((LocoOn&2)&&cab->Switch(20))
    ControlFlags|=2048;
  break;
  case 120:
   //ventilators off
   if((LocoOn&4)&&cab->Switch(120))
    ControlFlags|=2048;
  break;
  case 22:
   //Panto front
   if(LocoOn&2){
    if(cab->Switch(22))
     ControlFlags|=1;
    else
     ControlFlags&=~1;
   };
  break;
  case 122:
   //Panto front
   if(LocoOn&4){
    if(cab->Switch(122))
     ControlFlags|=2;
    else
     ControlFlags&=~2;
   };
  break;
  case 24:
   //disconnector
   if(LocoOn&2){
    if(cab->Switch(24))
     ControlFlags|=4;
    else
     ControlFlags&=~4;
   };
  break;
  case 124:
   //disconnector
   if(LocoOn&4){
    if(cab->Switch(124))
     ControlFlags|=8;
    else
     ControlFlags&=~8;
   };
  break;
  case 25:
   //compressor 1
   if(LocoOn&2){
    ControlFlags&=~80;
    switch(cab->Switch(25)){
     case 0: ControlFlags|=16;  break;
     case 2: ControlFlags|=64; break;
    };
   };
  break;
  case 125:
   //compressor 1
   if(LocoOn&4){
    ControlFlags&=~160;
    switch(cab->Switch(125)){
     case 0: ControlFlags|=32;  break;
     case 2: ControlFlags|=128; break;
    };
   };
  break;
  case 26:
   //BF left
   state=cab->Switch(26);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,5);
    else if(state==2)
     SwitchLights(loco,4);
   }else
    SwitchLights(loco,3);
  break;
  case 27:
   //BF right
   state=cab->Switch(27);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,8);
    else if(state==2)
     SwitchLights(loco,7);
   }else
    SwitchLights(loco,6);
  break;
  case 126:
   //BF left
   state=cab->Switch(126);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,15);
    else if(state==2)
     SwitchLights(loco,14);
   }else
    SwitchLights(loco,13);
  break;
  case 127:
   //BF right
   state=cab->Switch(127);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,18);
    else if(state==2)
     SwitchLights(loco,17);
   }else
    SwitchLights(loco,16);
  break;
  case 28:
   //Panto 2
   if(LocoOn&2){
    if(cab->Switch(28))
     ControlFlags|=2;
    else
     ControlFlags&=~2;
   };
  break;
  case 128:
   //Panto 2
   if(LocoOn&4){
    if(cab->Switch(128))
     ControlFlags|=1;
    else
     ControlFlags&=~1;
   };
  break;
  case 29:
  case 129:
   //sanding
   if(((LocoOn&2) && cab->Switch(29))||((LocoOn&4) && cab->Switch(129)))
    ControlFlags|=4096;
   else
    ControlFlags&=~4096;
  break;
  case 30:
   //disconnector 2
   if(LocoOn&2){
    if(cab->Switch(30))
     ControlFlags|=8;
    else
     ControlFlags&=~8;
   };
  break;
  case 130:
   //disconnector 2
   if(LocoOn&4){
    if(cab->Switch(130))
     ControlFlags|=4;
    else
     ControlFlags&=~4;
   };
  break;
  case 31:
   //compressor 2
   if(LocoOn&2){
    ControlFlags&=~160;
    switch(cab->Switch(31)){
     case 0: ControlFlags|=32;  break;
     case 2: ControlFlags|=128; break;
    };
   };
  break;
  case 131:
   //compressor 2
   if(LocoOn&4){
    ControlFlags&=~80;
    switch(cab->Switch(131)){
     case 0: ControlFlags|=16;  break;
     case 2: ControlFlags|=64; break;
    };
   };
  break;
  case 32:
   //cabin lights
   state=cab->Switch(32);
   if((LocoOn&1)&&(state!=1)){
    switch(state){
     case 0:
      cab->SwitchLight(0,true);
      cab->SwitchLight(1,false);
      cab->SwitchLight(2,true);
      cab->SwitchLight(3,false);
      cab->SwitchLight(4,true);
     break;
     case 2:
      cab->SwitchLight(0,true);
      cab->SwitchLight(1,true);
      cab->SwitchLight(2,true);
      cab->SwitchLight(3,true);
      cab->SwitchLight(4,true);
     break;
    };
   }else{
    cab->SwitchLight(0,false);
    cab->SwitchLight(1,false);
    cab->SwitchLight(2,false);
    cab->SwitchLight(3,false);
    cab->SwitchLight(4,false);
   };
  break;
  case 132:
   //cabin lights
   state=cab->Switch(132);
   if((LocoOn&1)&&(state!=1)){
    switch(state){
     case 0:
      cab->SwitchLight(5,true);
      cab->SwitchLight(6,false);
      cab->SwitchLight(7,true);
      cab->SwitchLight(8,false);
      cab->SwitchLight(9,true);
     break;
     case 2:
      cab->SwitchLight(5,true);
      cab->SwitchLight(6,true);
      cab->SwitchLight(7,true);
      cab->SwitchLight(8,true);
      cab->SwitchLight(9,true);
     break;
    };
   }else{
    cab->SwitchLight(5,false);
    cab->SwitchLight(6,false);
    cab->SwitchLight(7,false);
    cab->SwitchLight(8,false);
    cab->SwitchLight(9,false);
   };
  break;
  case 33:
   //projector
   state=cab->Switch(33);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,1);
    else if(state==2)
     SwitchLights(loco,2);
   }else
    SwitchLights(loco,0);
  break;
  case 133:
   //projector
   state=cab->Switch(133);
   if((LocoOn&1)&&(state!=1)){
    if(!state)
     SwitchLights(loco,11);
    else if(state==2)
     SwitchLights(loco,12);
   }else
    SwitchLights(loco,10);
  break;
  case 35:
   //BV off
   if((LocoOn&3)==3)
    if(cab->Switch(35))
     ControlFlags|=512;
  break;
  case 36:
   //BV on
   if((LocoOn&3)==3)
    if(cab->Switch(36))
     ControlFlags|=256;
  break;
  case 135:
   //BV off
   if((LocoOn&5)==5)
    if(cab->Switch(135))
     ControlFlags|=512;
  break;
  case 136:
   //BV on
   if((LocoOn&5)==5)
    if(cab->Switch(136))
     ControlFlags|=256;
  break;
  case 40:
   //wiper left
   anim=cab->FindAnim(L"WIPER1A");
   if(anim)
    anim->Switch(cab->Switch(40));
   anim=loco->FindAnim(L"WIPER1A");
   if(anim)
    anim->Switch(cab->Switch(40));
  break;
  case 140:
   //wiper left
   anim=cab->FindAnim(L"WIPER1B");
   if(anim)
    anim->Switch(cab->Switch(140));
   anim=loco->FindAnim(L"WIPER1B");
   if(anim)
    anim->Switch(cab->Switch(140));
  break;
  case 41:
   //wiper right
   anim=cab->FindAnim(L"WIPER2A");
   if(anim)
    anim->Switch(cab->Switch(41));
   anim=loco->FindAnim(L"WIPER2A");
   if(anim)
    anim->Switch(cab->Switch(41));
  break;
  case 141:
   //wiper right
   anim=cab->FindAnim(L"WIPER2B");
   if(anim)
    anim->Switch(cab->Switch(141));
   anim=loco->FindAnim(L"WIPER2B");
   if(anim)
    anim->Switch(cab->Switch(141));
  break;
 };
};


extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 Cabin *cab=loco->Cab();
 UINT LocoOn=IsLocoOn(eng,cab);
 ULONG &Flags=*(unsigned long *)&eng->var[0];
 const ULONG &ControlFlags=*(unsigned long *)&eng->var[1];
 ULONG &SoundFlags=*(unsigned long *)&eng->var[2];
 SMSObject *soundExt=loco->SoundEng(),
           *soundCab=loco->SoundCab();

 double BatteryUsage=0.0;
 const float LocoVelocity=fabs(loco->Velocity);
 eng->PowerConsuming=0.0;
 eng->MainResRate=0.0;
 bool CurrentOn=true;
 if((!(loco->PantoRaised&1)||!(ControlFlags&8)) &&
        (!(loco->PantoRaised&2)||!(ControlFlags&4)))
  CurrentOn=false;
 if(CurrentOn){
  if(!eng->MainSwitch)
   CurrentOn=false;
  else if(loco->LineVoltage>4000.0 || loco->LineFreq>0.0){
   SwitchBV(loco,eng,0);
   CurrentOn=false;
  }else if(loco->LineVoltage<2000.0)
   CurrentOn=false;
 };

 if(bool(LocoOn&1)!=bool(SoundFlags&2)){
  if(LocoOn&1){
   loco->PostTriggerBoth(114);
   SoundFlags|=2;
  }else{
   loco->PostTriggerBoth(115);
   SoundFlags&=~2;
  };
 };

 //Pantographs
 unsigned char WasPanto=eng->Panto;
 if(LocoOn&1){
  if(ControlFlags&1){
   if(loco->MainResPressure>3.5)
    eng->Panto|=2;
  }else
   eng->Panto&=~2;
  if(ControlFlags&2){
   if(loco->MainResPressure>3.5)
    eng->Panto|=1;
  }else
   eng->Panto&=~1;
 }else
  eng->Panto=0;
 if(eng->Panto!=loco->PantoRaised)
  eng->MainResRate-=0.05;
 if(eng->Panto!=WasPanto){
  if((eng->Panto&2)&&!(WasPanto&2))
   loco->PostTriggerBoth(108);
  else if(!(eng->Panto&2)&&(WasPanto&2))
   loco->PostTriggerBoth(112);
  if((eng->Panto&1)&&!(WasPanto&1))
   loco->PostTriggerBoth(110);
  else if(!(eng->Panto&1)&&(WasPanto&1))
   loco->PostTriggerBoth(111);
 };

 //BV
 if(ControlFlags&256)
  if(!eng->MainSwitch && loco->MainResPressure>=4.0 && !eng->ThrottlePosition){
   SwitchBV(loco,eng,1);
   eng->MainResRate-=0.2/time;
  };
 if((ControlFlags&512)||!(LocoOn&1))
  if(eng->MainSwitch)
   SwitchBV(loco,eng,0);

 //Ventilators
 bool VentsOn=Flags&16;
 if(CurrentOn && ((ControlFlags&1024) || !(ControlFlags&2048))){
  if(ControlFlags&1024)
   Flags|=16;
  else if(eng->ThrottlePosition>1 && eng->Reverse)
   Flags|=16;
 }else
  Flags&=~16;
 if(Flags&16){
  if(!VentsOn)
   loco->PostTriggerBoth(6);
  eng->PowerConsuming+=4000.0;
 }else{
  if(VentsOn)
   loco->PostTriggerBoth(7);
 };

 //Compressors
 unsigned char CompressorState=0,CurrentCompressorState=(Flags>>8)&7;
 if(CurrentOn&&(ControlFlags&80)){
  if(((ControlFlags&16)&&(loco->MainResPressure<6.0))||(ControlFlags&64)){
   Flags|=4;
  };
 }else{
  if((ControlFlags&64)&&(eng->BatteryCharge>47.0)){
   Flags|=4;
  }else
   Flags&=~4;
 };
 if(CurrentOn&&(ControlFlags&160)){
  if(((ControlFlags&32)&&(loco->MainResPressure<6.0))||(ControlFlags&128)){
   Flags|=8;
  };
 }else{
  if((ControlFlags&128)&&(eng->BatteryCharge>47.0)){
   Flags|=8;
  }else
   Flags&=~8;
 };
 if(loco->MainResPressure>9.0){
  if(ControlFlags&16)
   Flags&=~4;
  if(ControlFlags&32)
   Flags&=~8;
 };
 if(Flags&12){
  float ComprPower=1.0;
  if((Flags&12)==12)
   ComprPower=2.0;
  if(CurrentOn){
   eng->PowerConsuming+=20000*ComprPower;
   CompressorState=(Flags&12)>>1;
  }else{
   BatteryUsage-=ComprPower*12.2;
   ComprPower*=0.08;
   CompressorState=1;
  };
  eng->MainResRate+=0.034*(9.58-loco->MainResPressure)*ComprPower;
 }else{
  if(eng->MainResRate>2.45)
   eng->MainResRate-=-2e-4*loco->MainResPressure;
 };
 if(CompressorState!=CurrentCompressorState){
  if(!CurrentCompressorState)
   loco->PostTriggerBoth(103);
  else if(!CompressorState)
   loco->PostTriggerBoth(104);
  if(CompressorState){
   if(CompressorState==1)
    loco->PostTriggerBoth(105);
   else if((CompressorState&6)!=6)
    loco->PostTriggerBoth(106);
   else
    loco->PostTriggerBoth(107);
  };
  Flags&=~(7<<8);
  Flags|=(CompressorState&7)<<8;
 };

 //Throttle switch spinner
 float CurrentSpinnerSet=cab->Switch(0);
 if(LocoOn&4)
  CurrentSpinnerSet=cab->Switch(100);
 if(CurrentSpinnerSet!=eng->var[7]){
  if(CurrentSpinnerSet>eng->var[7]){
   eng->var[7]+=THROTTLE_SWITCH_SPEED*time;
   if(CurrentSpinnerSet<=eng->var[7]){
    eng->var[7]=CurrentSpinnerSet;
    eng->var[6]=0.0;
   };
  }else{
   eng->var[7]-=THROTTLE_SWITCH_SPEED*time;
   if(CurrentSpinnerSet>=eng->var[7]){
    eng->var[7]=CurrentSpinnerSet;
    eng->var[6]=0.0;
   };
  };
 };
 if(LocoOn&6)
  eng->var[4]=eng->var[7];
 if(LocoOn&1){
  if(eng->ThrottlePosition!=UINT(eng->var[4]))
   loco->PostTriggerBoth(100);
  eng->ThrottlePosition=eng->var[4];
 };


 //Throttle
 short Intercon=0;
 eng->EngineForce[0]=0.0;
 eng->EngineCurrent[0]=0.0;
 eng->EngineVoltage[0]=0.0;
 eng->EngineForce[1]=0.0;
 eng->EngineCurrent[1]=0.0;
 eng->EngineVoltage[1]=0.0;
 eng->EngineForce[2]=0.0;
 eng->EngineCurrent[2]=0.0;
 eng->EngineVoltage[2]=0.0;
 eng->EngineForce[3]=0.0;
 eng->EngineCurrent[3]=0.0;
 eng->EngineVoltage[3]=0.0;
 eng->EngineForce[4]=0.0;
 eng->EngineCurrent[4]=0.0;
 eng->EngineVoltage[4]=0.0;
 eng->EngineForce[5]=0.0;
 eng->EngineCurrent[5]=0.0;
 eng->EngineVoltage[5]=0.0;

 if(CurrentOn && eng->Reverse){
  eng->Force=0.0;
  const float KSVoltageQ=loco->LineVoltage/3000.0;

  if(eng->ThrottlePosition>1){
   float diff=0.0,VelMax;
   if(eng->ThrottlePosition>=2 && eng->ThrottlePosition<=21){
    Intercon=1;
    diff=(21-eng->ThrottlePosition);
    VelMax=3.320-diff*0.1-pow(diff,3.2)*0.0008;
   }else if(eng->ThrottlePosition>=22 && eng->ThrottlePosition<=34){
    Intercon=2;
    diff=(34-eng->ThrottlePosition);
    VelMax=9.635-diff*0.125-pow(diff,2.0)*0.0380;
   }else if(eng->ThrottlePosition>=35 && eng->ThrottlePosition<=43){
    Intercon=3;
    diff=(43-eng->ThrottlePosition);
    VelMax=13.72-diff*0.425-pow(diff,2.0)*0.0380;
   };
   VelMax=loco->Velocity*eng->Reverse-VelMax;
   if(VelMax>0.0){
    switch(Intercon){
     case 1:
      eng->Force = 3390000.0/(pow(VelMax,1.88+diff*0.002));
     break;
     case 2:
      eng->Force = (4900000.0-diff*110000.0)/pow(VelMax,1.64+diff*0.006);
     break;
     case 3:
      eng->Force = (12960000.0-diff*110000.0)/pow(VelMax,1.77+diff*0.006);
     break;
    };
   }else
    eng->Force=400000.0;
   eng->Force*=KSVoltageQ;
   //Shunts
   if(eng->var[6]>0.0){
    float ShuntQ=1.0;
    switch(int(eng->var[6])){
     case 1:
      ShuntQ=1.20;
     break;
     case 2:
      ShuntQ=1.60;
     break;
     case 3:
      ShuntQ=2.00;
     break;
     case 4:
      ShuntQ=2.5;
     break;
     case 5:
      ShuntQ=3.2;
     break;
    };
    eng->Force*=ShuntQ;
   };
  };

  //Force smoothing
  if(eng->Force || eng->var[8]){
   if(eng->Force>eng->var[8]){
    eng->var[8]+=FORCE_RAISE*time;
    if(eng->Force>eng->var[8])
     eng->Force=eng->var[8];
   }else if(eng->Force<eng->var[8]){
    eng->var[8]-=FORCE_LOWER*time;
    if(eng->Force<eng->var[8])
     eng->Force=eng->var[8];
   };
   eng->var[8]=eng->Force;
  };

  //Shutting TED off
  float Current=eng->Force/CURRENT_Q,ForceQ=1.0,CurrentQ=1.0;
  float Voltage=0.0,TEDVoltageAll=0.0,PowerQ=1.0;
  UINT TEDDisabled=0,TEDEnabled=6;
  switch(Intercon){
   case 1:
    Voltage=500.0-25.0*(21-eng->ThrottlePosition);
    Voltage*=KSVoltageQ;
    TEDVoltageAll=6.0*Voltage;
   break;
   case 2:
    Voltage=1000.0-38.0*(34-eng->ThrottlePosition);
    Voltage*=KSVoltageQ;
    TEDVoltageAll=3.0*Voltage;
   break;
   case 3:
    Voltage=1500.0-55.0*(43-eng->ThrottlePosition);
    Voltage*=KSVoltageQ;
    TEDVoltageAll=2.0*Voltage;
   break;
  };
  if(TEDVoltageAll>0.0)
   PowerQ=loco->LineVoltage/TEDVoltageAll;
  if(Flags&64){
   for(UINT i=0;i<6;i++){
    if(Flags&((1<<i)<<16))
     TEDDisabled++;
   };
  };
  TEDEnabled=6-TEDDisabled;
  if(!TEDEnabled){
   eng->Force=0.0;
   Current=0.0;
   Voltage=0.0;
  }else{
   if(Intercon==1){
    float delta=0.2/(TEDEnabled>1?TEDEnabled-1:1);
    float StartDelta=1.1;
    if(TEDDisabled)
     Current*=6.0/TEDEnabled;
    for(UINT i=0;i<6;i++){
     if(!(Flags&((1<<i)<<16)) || !(Flags&64)){
      if(TEDEnabled>1){
       eng->EngineForce[i]=StartDelta*eng->Force/TEDEnabled;
       StartDelta-=delta;
      }else
       eng->EngineForce[i]=eng->Force/TEDEnabled;
      eng->EngineCurrent[i]=Current;
      eng->EngineVoltage[i]=Voltage*(1.0+TEDDisabled*0.16);
     };
    };
   }else if(Intercon==2){
    for(UINT i=0;i<3;i++){
     ULONG GroupOff=(Flags>>(16+i*2))&3;
     if(!(Flags&64))
      GroupOff=0;
     if((GroupOff&3)!=3){
      if(GroupOff&3){
       CurrentQ=2.0;
       if(GroupOff&1){
        eng->EngineCurrent[i*2+1]=Current*2.0;
        eng->EngineForce[i*2+1]=eng->Force/3.0;
        eng->EngineVoltage[i*2+1]=Voltage*2.0;
       }else{
        eng->EngineCurrent[i*2]=Current*2.0;
        eng->EngineForce[i*2]=eng->Force/3.0;
        eng->EngineVoltage[i*2]=Voltage*2.0;
       };
      }else{
       eng->EngineCurrent[i*2]=Current;
       eng->EngineForce[i*2]=1.05*eng->Force/6.0;
       eng->EngineVoltage[i*2]=Voltage;
       eng->EngineCurrent[i*2+1]=Current;
       eng->EngineForce[i*2+1]=0.95*eng->Force/6.0;
       eng->EngineVoltage[i*2+1]=Voltage;
      };
     }else{
      ForceQ-=1.0/3.0;
     };
    };
    eng->Force*=ForceQ;
    Current*=CurrentQ;
   }else if(Intercon==3){
    for(UINT i=0;i<6;i++){
     if((Flags&(1<<(i+16))) && (Flags&64)){
      ForceQ-=1.0/6.0;
     }else{
      eng->EngineCurrent[i]=Current;
      eng->EngineForce[i]=eng->Force/6.0;
      eng->EngineVoltage[i]=Voltage;
     };
     eng->Force*=ForceQ;
    };
   };
  };
  //Calculating total power usage and resistor power and heating
  if(eng->Force){
   float ResistorPower=0.0,UnitPower;
   for(UINT i=0;i<6;i++){
    UnitPower=eng->EngineCurrent[i]*eng->EngineVoltage[i];
    ResistorPower+=UnitPower*(PowerQ-1.0)*PowerQ;
    eng->PowerConsuming+=UnitPower*PowerQ;
   };
   eng->var[12]+=ResistorPower*time;
  };

  if(soundExt)
   soundExt->Var2[0]=Current/600.0;
  if(soundCab)
   soundCab->Var2[0]=Current/600.0;

  if(Current>700.0){
   SwitchBV(loco,eng,0);
   eng->Force=0.0;
  };
  if(eng->Force && loco->BrakeCylinderPressure>=1.0){
   SwitchBV(loco,eng,0);
   eng->Force=0.0;
  };
  if(eng->var[12]>9.0e8 && eng->Force){
   SwitchBV(loco,eng,0);
   eng->Force=0.0;
  };

  eng->Force*=eng->Reverse;

 }else{
  eng->Force=0.0;
  eng->var[8]=0.0;
  if(soundExt)
   soundExt->Var2[0]=0.0;
  if(soundCab)
   soundCab->Var2[0]=0.0;
 };

 //Resistor cooling
 if(Flags&16)
  eng->var[12]-=420000*time;
 eng->var[12]-=200000*(0.1+LocoVelocity*0.01+(eng->var[12]*6.0e-8+25.0-AirTemperature)*0.03)*time;
 if(eng->var[12]<0.0){
  eng->var[12]=0.0;
 };

 //EPK
 if((loco->LocoFlags&1)&&(LocoOn&8)){
  UINT Aspect=cab->Signal.Aspect[0];
  UINT PrevAspect=eng->var[11];
  const float Vel=LocoVelocity;
  bool Moving=Vel>0.2;
  if(Aspect==SIGASP_STOP_AND_PROCEED){
   if(Moving){
    eng->var[10]=2.0;
    UINT appr=ApproachRed(cab->Signal.Distance,Vel);
    if(appr==1 && eng->var[5]<30.0){
     eng->var[5]=30.0;
    }else if(appr==2){
     eng->var[5]=50.0;
    };
    if(Aspect!=PrevAspect && eng->var[5]<30.0)
     eng->var[5]=30.0;
   };
  }else if(Moving && (Aspect<SIGASP_APPROACH_1 || Aspect==SIGASP_BLOCK_OBSTRUCTED)){
   eng->var[10]=2.0;
   if(Aspect!=PrevAspect && eng->var[5]<30.0)
    eng->var[5]=30.0;
  }else if(Aspect!=PrevAspect){
   if(Aspect<PrevAspect && Moving){
    eng->var[10]=1.0;
    eng->var[5]=30.0;
   }else if(Aspect>PrevAspect && Aspect!=SIGASP_BLOCK_OBSTRUCTED){
    eng->var[10]=0.0;
    eng->var[5]=0.0;
    SoundFlags&=~1;
    loco->PostTriggerCab(113);
   };
  };
  if(!Moving && eng->var[5]<45.0){
   eng->var[10]=0.0;
   eng->var[5]=0.0;
  };

  if(eng->var[10]>0.0 && eng->var[5]>=35.0){
   if(!(SoundFlags&1)){
    loco->PostTriggerCab(56);
    SoundFlags|=1;
   };
  }else{
   if(SoundFlags&1){
    loco->PostTriggerCab(57);
    SoundFlags&=~1;
   };
  };
  if(eng->var[10]>0.0 && eng->var[5]>=45.0){
   Flags|=32;
   eng->TrainPipeRate=UR_EMERGENCY_RATE;
  }else{
   if(!Vel)
    Flags&=~32;
  };
  if(eng->var[10]>0.0)
   eng->var[5]+=time;

  eng->var[11]=Aspect;
 }else{
  eng->var[5]=0.0;
  eng->var[10]=0.0;
  eng->var[11]=0.0;
  Flags&=~160;
  if(SoundFlags&1){
   loco->PostTriggerCab(57);
   SoundFlags&=~1;
  };
 };

 //EPT
 eng->BrakeSystemsEngaged=1;
 if((LocoOn&2)&&!cab->Switch(14))
  eng->BrakeSystemsEngaged|=2;
 else if((LocoOn&4)&&!cab->Switch(114))
  eng->BrakeSystemsEngaged|=2;

 //Locomotive brake
 if(eng->IndependentBrakeValue>loco->MainResPressure)
  eng->IndependentBrakeValue=loco->MainResPressure;
 if(eng->IndependentBrakeValue>loco->IndependentBrakePressure)
  eng->MainResRate-=0.04;

 //Train brake
 int brake=-1,brakeset=-1,EPTset=-1,EPT=-1;
 float EPTVoltage=0.0;
 if(cab->Switch(7)){
  brake=cab->Switch(4);
  brakeset=cab->SwitchSet(4);
 };
 if(cab->Switch(107) && int(cab->Switch(104))>brake){
  brake=cab->Switch(104);
  brakeset=cab->SwitchSet(104);
 };
 if((LocoOn&2) && !(cab->Switch(14)) && cab->Switch(7)){
  EPT=cab->Switch(4);
  EPTset=cab->SwitchSet(4);
 }else if((LocoOn&4) && !(cab->Switch(114)) && cab->Switch(107)){
  EPT=cab->Switch(104);
  EPTset=cab->SwitchSet(104);
 };
 if(Flags&32){
  EPTset=-2;
  EPT=-2;
  brake=-2;
  brakeset=-2;
  eng->TrainPipeRate=UR_EMERGENCY_RATE;
  /*if(EPTset!=-1){
   EPTset=6;
   EPT=6;
  };
  brake=6;
  brakeset=6;*/
 };
 switch(brake){
  case 0:
   if(brakeset)
    break;
   eng->UR+=UR_CHARGE_RATE*time;
   if(eng->UR>loco->MainResPressure)
    eng->UR=loco->MainResPressure;
   eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q_REL;
   if(eng->TrainPipeRate<0.0)
    eng->TrainPipeRate=0.0;
   else if(eng->TrainPipeRate>MAX_TRAIN_PIPE_RATE)
    eng->TrainPipeRate=MAX_TRAIN_PIPE_RATE;
  break;
  case 1:
   if(eng->UR<5.2)
    eng->UR+=UR_CHARGE_RATE*time;
   eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
   if(eng->TrainPipeRate<0.0){
    if(eng->TrainPipeRate>-0.1){
     eng->TrainPipeRate=0.0;
    };
   }else if(eng->TrainPipeRate>MAX_TRAIN_PIPE_RATE)
    eng->TrainPipeRate=MAX_TRAIN_PIPE_RATE;
   if(eng->TrainPipeRate>0.0)
    eng->UR-=UR_DISCHARGE2*time;
  break;
  case -1:
   eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
   if(eng->TrainPipeRate<0.0)
    eng->TrainPipeRate=0.0;
   else if(eng->TrainPipeRate>MAX_TRAIN_PIPE_RATE)
    eng->TrainPipeRate=MAX_TRAIN_PIPE_RATE;
  break;
  case 2:
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>PIPE_DISCHARGE_SLOW)
    eng->TrainPipeRate=PIPE_DISCHARGE_SLOW;
  break;
  case 3:
  case 4:
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>0.0)
    eng->TrainPipeRate*=PIPE_CHARGE_Q_4;
   if(eng->TrainPipeRate>MAX_TRAIN_PIPE_RATE*0.3)
    eng->TrainPipeRate=MAX_TRAIN_PIPE_RATE*0.3;
  break;
  case 5:
   if(brakeset<5)
    break;
   eng->UR+=UR_DISCHARGE_RATE*time;
   if(eng->UR<0.0)
    eng->UR=0.0;
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>0.0)
    eng->TrainPipeRate=0.0;
  break;
  case 6:
   eng->UR+=UR_EMERGENCY_RATE*time;
   if(eng->UR<0.0)
    eng->UR=0.0;
   eng->TrainPipeRate=UR_EMERGENCY_RATE;
  break;
 };
 eng->EPTvalue=-1.0;
 switch(EPT){
  case 0:
   eng->var[9]-=EPT_RELEASE_RATE1*time;
   if(eng->var[9]<0.0)
    eng->var[9]=0.0;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[9];
   EPTVoltage=50.0;
  break;
  case 1:
   if(EPTset>1)
    break;
   eng->var[9]-=EPT_RELEASE_RATE*time;
   if(eng->var[9]<0.0)
    eng->var[9]=0.0;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[9];
   EPTVoltage=40.0;
  break;
  case 2:
  case 3:
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[9];
  break;
  case 4:
   if(EPTset<4)
    break;
   eng->var[9]+=EPT_APPLY_RATE*time;
   if(eng->var[9]>EPT_MAX)
    eng->var[9]=EPT_MAX;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[9];
   EPTVoltage=20.0;
  break;
  case 5:
   eng->var[9]+=EPT_APPLY_RATE*time;
   if(eng->var[9]<loco->BrakeCylinderPressure)
    eng->var[9]=loco->BrakeCylinderPressure;
   if(eng->var[9]>EPT_MAX)
    eng->var[9]=EPT_MAX;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[9];
   EPTVoltage=20.0;
  break;
  case 6:
   eng->var[9]=EPT_MAX;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=EPT_MAX;
   EPTVoltage=25.0;
  break;
 };

 //Handbrake
 eng->HandbrakePercent=(2.0-cab->GetSwitchFrame(6)-cab->GetSwitchFrame(106))*100.0;

 //Sanding
 if((LocoOn&1)&&(ControlFlags&4096)&&loco->SandLeft){
  if(!eng->Sanding)
   loco->PostTriggerBoth(4);
  eng->Sanding=1;
 }else{
  if(eng->Sanding)
   loco->PostTriggerBoth(5);
  eng->Sanding=0;
 };

 //Auxilary power usage
 if(LocoOn&1){
  if(loco->IsLightOn(0))
   BatteryUsage-=3.06;
  if(loco->IsLightOn(27))
   BatteryUsage-=3.06;
  if(loco->IsLightOn(1))
   BatteryUsage-=1.80;
  if(loco->IsLightOn(28))
   BatteryUsage-=1.80;
  if(loco->IsLightOn(31))
   BatteryUsage-=0.75;
  if(loco->IsLightOn(8))
   BatteryUsage-=0.75;
  if(cab->Switch(32)!=1)
   BatteryUsage-=0.8;
  if(cab->Switch(132)!=1)
   BatteryUsage-=0.8;
 };

 //Battery charging/discharging
 if(LocoOn&6)
  BatteryUsage-=1.8;
 if(CurrentOn){
  eng->PowerConsuming+=BatteryUsage*50.0;
  BatteryUsage=(BATTERY_HIGH-eng->BatteryCharge)*1.25;
  if(BatteryUsage>18.5)
   BatteryUsage=18.5;
 };
 if(BatteryUsage)
  eng->BatteryCharge+=BatteryUsage*time*3.5e-5;

 //Updating slave locos
 if(loco->LocoFlags&1){
  Engine *Slave;
  for(UINT i=0;i<loco->NumSlaves;i++){
   Slave=loco->SlaveLoco(i)->Eng();
   Slave->var[4]=eng->var[4];
   ULONG &SlaveControlFlags=*(unsigned long *)&Slave->var[1];
   SlaveControlFlags=ControlFlags;
   Slave->var[6]=eng->var[6];
   Slave->UR=eng->UR;
   Slave->IndependentBrakeValue=eng->IndependentBrakeValue;
   Slave->Reverse=eng->Reverse;
  };
 };
 if(ControlFlags&2816){
  ULONG &UnsetControlFlags=*(unsigned long *)&eng->var[1];
  UnsetControlFlags&=~2816;
 };

 //Displays
 if((State>>8)&1){
  if(!(LocoOn&1)){
   cab->SetSwitch( 34,1,false);
   cab->SetSwitch(134,1,false);
  }else if(eng->MainSwitch){
   cab->SetSwitch( 34,2,false);
   cab->SetSwitch(134,2,false);
  }else{
   cab->SetSwitch( 34,0,false);
   cab->SetSwitch(134,0,false);
  };

  float TZ=loco->BrakeCylinderPressure;
  if(loco->IndependentBrakePressure>TZ)
   TZ=loco->IndependentBrakePressure;
  float Vel=fabs(loco->Velocity*3.6);
  float Voltage=loco->LineVoltage;
  if((!(loco->PantoRaised&1)||!(ControlFlags&8)) &&
        (!(loco->PantoRaised&2)||!(ControlFlags&4)))
   Voltage=0.0;
  float TEDCurrent[6]={eng->EngineCurrent[0],0.0,0.0,
        eng->EngineCurrent[5],0.0,0.0};
  if(Intercon>1){
   TEDCurrent[1]=eng->EngineCurrent[2];
   TEDCurrent[4]=eng->EngineCurrent[3];
  };
  if(Intercon>2){
   TEDCurrent[2]=eng->EngineCurrent[4];
   TEDCurrent[5]=eng->EngineCurrent[1];
  };
  const UINT GameTime=RTSGetInteger(NULL,RTS_VAR_TIME,0);
  float Hours=float(GameTime)/3600.0;
  float Minutes=float(GameTime%3600)/60.0;

  cab->SetDisplayValue(0, Vel);
  cab->SetDisplayValue(1, Hours);
  cab->SetDisplayValue(2, Minutes);
  cab->SetDisplayValue(3, Voltage);
  cab->SetDisplayValue(4, TEDCurrent[0]);
  cab->SetDisplayValue(5, TEDCurrent[1]);
  cab->SetDisplayValue(6, TEDCurrent[2]);
  cab->SetDisplayValue(7, loco->TrainPipePressure);
  cab->SetDisplayValue(8, eng->UR);
  cab->SetDisplayValue(9, loco->MainResPressure);
  cab->SetDisplayValue(10,TZ);

  cab->SetDisplayValue(11,EPTVoltage);
  if(LocoOn&2){
   if(eng->Reverse && eng->ThrottlePosition>1 && !IsHP(eng->ThrottlePosition))
    cab->SetDisplayState(12,1);
   else
    cab->SetDisplayState(12,0);
   if(Flags&4)
    cab->SetDisplayState(13,1);
   else
    cab->SetDisplayState(13,0);
   if(Flags&16)
    cab->SetDisplayState(14,1);
   else
    cab->SetDisplayState(14,0);
   if(eng->Wheelslip)
    cab->SetDisplayState(15,1);
   else
    cab->SetDisplayState(15,0);
   if(Flags&8)
    cab->SetDisplayState(16,1);
   else
    cab->SetDisplayState(16,0);
   if(Flags&16)
    cab->SetDisplayState(17,1);
   else
    cab->SetDisplayState(17,0);
  }else{
   cab->SetDisplayState(12,0);
   cab->SetDisplayState(13,0);
   cab->SetDisplayState(14,0);
   cab->SetDisplayState(15,0);
   cab->SetDisplayState(16,0);
   cab->SetDisplayState(17,0);
  };
  if((eng->BrakeSystemsEngaged&2)&&(LocoOn&2)){
   cab->SetDisplayState(18,0);
   cab->SetDisplayState(19,1);
   cab->SetDisplayState(20,0);
   switch(cab->Switch(4)){
    case 2: case 3: cab->SetDisplayState(18,1); break;
    case 4: case 5: case 6: cab->SetDisplayState(20,1); break;
   };
  }else{
   cab->SetDisplayState(18,0);
   cab->SetDisplayState(19,0);
   cab->SetDisplayState(20,0);
  };
  if(((LocoOn&10)==10)&&(eng->var[10]>0.0 && eng->var[5]>=30.0)){
   cab->SetDisplayState(21,1);
   cab->SetDisplayState(22,1);
   cab->SetDisplayState(23,0);
   switch(int(eng->var[10])){
    case 1:
     cab->SetDisplayFade(21,0.0,0.6);
     cab->SetDisplayFade(22,0.0,0.6);
    break;
    case 2:
     cab->SetDisplayFade(21,0.0,0.0);
     cab->SetDisplayFade(22,0.0,0.0);
    break;
   };
   if(Flags&128)
    cab->SetDisplayState(23,1);
  }else{
   cab->SetDisplayState(21,0);
   cab->SetDisplayState(22,0);
   cab->SetDisplayState(23,0);
  };
  if((LocoOn&1)&&cab->Switch(21))
   cab->SetDisplayState(24,15);
  else
   cab->SetDisplayState(24,0);
  if((LocoOn&10)==10)
   cab->SetDisplayState(25,AspectToALSNState(cab->Signal.Aspect[0]));
  else
   cab->SetDisplayState(25,0);

  cab->SetDisplayValue(50,Vel);
  cab->SetDisplayValue(51,Hours);
  cab->SetDisplayValue(52,Minutes);
  cab->SetDisplayValue(53,Voltage);
  cab->SetDisplayValue(54,TEDCurrent[3]);
  cab->SetDisplayValue(55,TEDCurrent[4]);
  cab->SetDisplayValue(56,TEDCurrent[5]);
  cab->SetDisplayValue(57,loco->TrainPipePressure);
  cab->SetDisplayValue(58,eng->UR);
  cab->SetDisplayValue(59,loco->MainResPressure);
  cab->SetDisplayValue(60,TZ);

  if(LocoOn&4){
   if(eng->Reverse && eng->ThrottlePosition>1 && !IsHP(eng->ThrottlePosition))
    cab->SetDisplayState(62,1);
   else
    cab->SetDisplayState(62,0);
   if(Flags&8)
    cab->SetDisplayState(63,1);
   else
    cab->SetDisplayState(63,0);
   if(Flags&16)
    cab->SetDisplayState(64,1);
   else
    cab->SetDisplayState(64,0);
   if(eng->Wheelslip)
    cab->SetDisplayState(65,1);
   else
    cab->SetDisplayState(65,0);
   if(Flags&4)
    cab->SetDisplayState(66,1);
   else
    cab->SetDisplayState(66,0);
   if(Flags&16)
    cab->SetDisplayState(67,1);
   else
    cab->SetDisplayState(67,0);
  }else{
   cab->SetDisplayState(62,0);
   cab->SetDisplayState(63,0);
   cab->SetDisplayState(64,0);
   cab->SetDisplayState(65,0);
   cab->SetDisplayState(66,0);
   cab->SetDisplayState(67,0);
  };
  cab->SetDisplayValue(61,EPTVoltage);
  if((eng->BrakeSystemsEngaged&2)&&(LocoOn&4)){
   cab->SetDisplayState(68,0);
   cab->SetDisplayState(69,1);
   cab->SetDisplayState(70,0);
   switch(cab->Switch(104)){
    case 2: case 3: cab->SetDisplayState(68,1); break;
    case 4: case 5: case 6: cab->SetDisplayState(70,1); break;
   };
  }else{
   cab->SetDisplayState(68,0);
   cab->SetDisplayState(69,0);
   cab->SetDisplayState(70,0);
  };
  //PSS
  if(((LocoOn&12)==12)&&(eng->var[10]>0.0 && eng->var[5]>=30.0)){
   cab->SetDisplayState(71,1);
   cab->SetDisplayState(72,1);
   cab->SetDisplayState(73,0);
   switch(int(eng->var[10])){
    case 1:
     cab->SetDisplayFade(71,0.0,0.6);
     cab->SetDisplayFade(72,0.0,0.6);
    break;
    case 2:
     cab->SetDisplayFade(71,0.0,0.0);
     cab->SetDisplayFade(72,0.0,0.0);
    break;
   };
   if(Flags&128)
    cab->SetDisplayState(73,1);
  }else{
   cab->SetDisplayState(71,0);
   cab->SetDisplayState(72,0);
   cab->SetDisplayState(73,0);
  };
  if((LocoOn&1)&&cab->Switch(121))
   cab->SetDisplayState(74,15);
  else
   cab->SetDisplayState(74,0);
  if((LocoOn&12)==12)
   cab->SetDisplayState(75,AspectToALSNState(cab->Signal.Aspect[0]));
  else
   cab->SetDisplayState(75,0);

  if(LocoOn&1){
   cab->SetDisplayValue(85,BatteryUsage);
   cab->SetDisplayValue(86,eng->BatteryCharge);
   if(CurrentOn && BatteryUsage>0.0){
    cab->SetDisplayState(87,1);
    cab->SetDisplayState(88,1);
   }else{
    cab->SetDisplayState(87,0);
    cab->SetDisplayState(88,0);
   };
  }else{
   cab->SetDisplayValue(85,0.0);
   cab->SetDisplayValue(86,0.0);
   cab->SetDisplayState(87,0);
   cab->SetDisplayState(88,0);
  };
 };
};


