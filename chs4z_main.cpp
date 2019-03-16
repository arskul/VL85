//---------------------------------------------------------------------------

#include <windows.h>
/*#define RTS_NODIRECTOBJLINKS*/
#include <ts.h>
#include <math>

#define THROTTLE_SWITCH_TIME 0.63
#define FORCE_SMOOTH_INCR  30000.0
#define FORCE_SMOOTH_DECR  50000.0

#define CURRENT_Q 175.0

#define UR_CHARGE_RATE 0.15
#define UR_DISCHARGE_RATE -0.25
#define UR_EMERGENCY_RATE -1.00
#define UR_DISCHARGE2     0.003
#define PIPE_CHARGE_Q_REL 0.8
#define PIPE_CHARGE_Q     0.5
#define PIPE_DISCHARGE_SLOW -0.005
#define CHARGE_PIPE_VALUE  6.0
#define CHARGE_PIPE_VALUE1 4.0
#define EPT_RELEASE_RATE  0.75
#define EPT_RELEASE_RATE1 1.0
#define EPT_APPLY_RATE    1.0
#define EPT_MAX           4.0
#define AUXC_RATE         0.02

#define LIGHT_RED   0xf0ff0000
#define LIGHT_WHITE 0xf0ffffff

#define LIGHTINT_GREEN  0xaa00ff00
#define LIGHTINT_DARK   0xaa909090
#define LIGHTINT_BRIGHT 0xbb909090

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

/*
Stack Variables

 0,1 - unsigned long Flags
  1bite
   1bit - protection incurred
   2bit - low line voltage
   3bit - high line voltage
   4bit - compressor1 on
   5bit - compressor2 on
   6bit - has voltage on TP
   7bit - emergency brake incurred
   8bit - overheat
  2bite
   1bit - tifon on
   2bit - svist on
   3bit - sand on
   4bit - compressor stream started
   5bit - vent stream started
   6bit - transformer on
  3bite
   1bit - EPK off
   2bit - alert on
   3bit - ground
   4bit - just switched GV on

 2 - throttle switch timer

 3 - [VACANT]

 4 - Shunt

 5 - previous force

 6 - overheat timer

 7 - EPK state

 8 - previous signal aspect

 9 - EPK timer

 10 - current speed limit

 11 - battery voltage

 12 - ZU pressure

 13 - throttle switch
   1 - +
   2 - +1
   3 - -1
   4 - -

 14,15 - compressor1,compressor2
  0 - off
  1 - auto
  2 - manual
  3 - ob

 16 - EPT value

 17 - ventilators mode
  0 - 0
  1 - off
  2 - 1+2
  3 - TR220
  4 - TR440

 18 - ground timer

LocoOn
 1bite
  1bit - battery on
  2bit - front control on
  3bit - back control on
  4bit - back cabin on
  5bit - EPK on
  6bit - EPK active

Sound Triggers
 4,5 - start/stop sand
 6,7 - start/stop vent
 8,9 - tifon
 10,11 - svist
 12,13 - start/stop compressor
 15 - reverse
 17 - switch brake valve
 18 - loco brake
 23,24 - GV on/off
 25,26 - transformator on/off
 37 - chik1
 45,46 - TP up/down
 47 - paketnik
 56,57 - start/stop alarm
 101 - start air release
 102 - kontaktor
 103 - RB
 104 - kontroller1
 105 - km
 106 - epk release

 Variable1 - vents volume
 Variable2 - compressor volume
 Variable3 - current to max 
*/


extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 Cabin *cab=loco->Cab();
 UINT &Flags=*(UINT *)&eng->var[0];

 loco->HandbrakeValue=1.0;
 loco->MainResPressure=4.0;
 loco->IndependentBrakePressure=2.0;
 loco->AuxiliaryPressure=2.0;
 loco->BrakeCylinderPressure=2.0;
 loco->TrainPipePressure=0.0;
 loco->ChargingPipePressure=0.0;

 eng->var[0]=0.0;
 eng->var[1]=0.0;
 eng->var[2]=0.0;
 eng->UR=0.0;
 eng->var[4]=0.0;
 eng->var[5]=0.0;
 eng->var[6]=0.0;
 eng->var[7]=0.0;
 eng->var[8]=0.0;
 eng->var[9]=0.0;
 eng->var[10]=-1.0;
 eng->var[11]=48.0;
 eng->var[12]=8.0;
 eng->var[13]=0.0;
 eng->var[14]=0.0;
 eng->var[15]=0.0;
 eng->var[16]=0.0;
 eng->var[17]=0.0;
 eng->var[18]=0.0;

 eng->IndependentBrakeValue=2.0;
 eng->HandbrakePercent=100.0;
 eng->EPTvalue=-1.0;

 if(!loco->NumEngines)
  return false;

 switch(State&0xFF){
  case 1: //stopped, with wagons
   loco->HandbrakeValue=0;
   loco->MainResPressure=10.0;
   loco->IndependentBrakePressure=4.0;
   eng->IndependentBrakeValue=4.0;
   loco->AuxiliaryPressure=5.1;
   loco->BrakeCylinderPressure=0.0;
   loco->TrainPipePressure=5.2;
   loco->ChargingPipePressure=9.0;
   loco->PantoRaised=1;
   eng->Panto=1;
   Flags=32;
   eng->UR=5.2;

   cab->SetSwitch(201,1,true);
   cab->SetSwitch(21,1,true);
   cab->SetSwitch(2,1,true);
   cab->SetSwitch(3,1,true);
   cab->SetSwitch(5,1,true);
   cab->SetSwitch(80,0,true);
   cab->SetSwitch(81,0,true);
   cab->SetSwitch(102,1,true);
   cab->SetSwitch(103,1,true);
   cab->SetSwitch(105,1,true);
   cab->SetSwitch(180,0,true);
   cab->SetSwitch(181,0,true);
  break;
  case 2:
  case 3:
   loco->HandbrakeValue=0;
   loco->MainResPressure=10.0;
   loco->IndependentBrakePressure=0.0;
   eng->IndependentBrakeValue=0.0;
   loco->AuxiliaryPressure=5.1;
   loco->BrakeCylinderPressure=0.0;
   loco->TrainPipePressure=5.2;
   loco->ChargingPipePressure=9.0;
   eng->UR=5.2;
   eng->var[14]=1.0;
   eng->var[15]=1.0;
   eng->MainSwitch=1;
   
   cab->SetSwitch(201,1,true);
   cab->SetSwitch(5,1,true);
   cab->SetSwitch(105,1,true);

   if(loco->Velocity>=0.0 ^ bool(loco->Flags&1)){
    loco->PantoRaised=1;
    eng->Panto=1;
    Flags=56;
    eng->Reverse=1;
    cab->SetSwitch(1,3,true);
    cab->SetSwitch(2,1,true);
    cab->SetSwitch(3,1,true);
    cab->SetSwitch(4,1,true);
    cab->SetSwitch(15,2,true);
    cab->SetSwitch(17,1,true);
    cab->SetSwitch(20,2,true);
    cab->SetSwitch(21,1,true);
    cab->SetSwitch(22,0,true);
    cab->SetSwitch(33,0,true);
    cab->SetSwitch(34,0,true);
   }else{
    cab->SetSwitch(202,0,true);
    loco->PantoRaised=2;
    eng->Panto=2;
    Flags=56;
    eng->Reverse=-1;
    cab->SetSwitch(101,3,true);
    cab->SetSwitch(102,1,true);
    cab->SetSwitch(103,1,true);
    cab->SetSwitch(104,1,true);
    cab->SetSwitch(115,2,true);
    cab->SetSwitch(117,1,true);
    cab->SetSwitch(120,2,true);
    cab->SetSwitch(121,1,true);
    cab->SetSwitch(122,0,true);
    cab->SetSwitch(133,0,true);
    cab->SetSwitch(134,0,true);
   };
  break;
 };

 return true;
};


ULONG IsLocoOn(const ElectricLocomotive *loco,ULONG Flags){
 ULONG res=0;
 ElectricEngine *eng=(ElectricEngine *)loco->Eng();
 Cabin *cab=loco->Cab();
 if(eng->var[11]>45.0 && cab->Switch(201)){
  res|=1;
  if(cab->Switch(202)){
   if(cab->Switch(17)){
    res|=2;
    if(cab->Switch(4))
     res|=16;
    if(!cab->Switch(35))
     res|=32;
   };
  }else{
   res|=8;
   if(cab->Switch(117)){
    res|=4;
    if(cab->Switch(104))
     res|=16;
    if(!cab->Switch(135))
     res|=32;
   };
  };
 };
 return res;
};

void SwitchLights(const Locomotive *loco,Engine *eng,UINT State){
 Cabin *cab=loco->Cab();
 switch(State){
  case 0:
   loco->SwitchLight(0,false);
   loco->SwitchLight(1,false);
   loco->SwitchLight(2,false);
  break;
  case 1:
   loco->SwitchLight(0,true);
   loco->SwitchLight(1,true);
   loco->SwitchLight(2,false);
  break;
  case 2:
   loco->SwitchLight(0,true);
   loco->SwitchLight(1,false);
   loco->SwitchLight(2,true);
  break;
  case 3:
   loco->SwitchLight(5,true,0,LIGHT_RED);
  break;
  case 4:
   loco->SwitchLight(5,false);
  break;
  case 5:
   loco->SwitchLight(5,true,0,LIGHT_WHITE);
  break;
  case 6:
   loco->SwitchLight(4,true,0,LIGHT_RED);
  break;
  case 7:
   loco->SwitchLight(4,false);
  break;
  case 8:
   loco->SwitchLight(4,true,0,LIGHT_WHITE);
  break;
  case 12:
   loco->SwitchLight(9,false);
   loco->SwitchLight(10,false);
   loco->SwitchLight(11,false);
  break;
  case 13:
   loco->SwitchLight(9,true);
   loco->SwitchLight(10,true);
   loco->SwitchLight(11,false);
  break;
  case 14:
   loco->SwitchLight(9,true);
   loco->SwitchLight(10,false);
   loco->SwitchLight(11,true);
  break;
  case 15:
   loco->SwitchLight(8,true,0,LIGHT_RED);
  break;
  case 16:
   loco->SwitchLight(8,false);
  break;
  case 17:
   loco->SwitchLight(8,true,0,LIGHT_WHITE);
  break;
  case 18:
   loco->SwitchLight(12,true,0,LIGHT_RED);
  break;
  case 19:
   loco->SwitchLight(12,false);
  break;
  case 20:
   loco->SwitchLight(12,true,0,LIGHT_WHITE);
  break;

  case 21:
   cab->SetLightState(0,1,LIGHTINT_GREEN,-1.0,2.0,2.0);
  break;
  case 22:
   cab->SetLightState(0,0,0,-1.0);
  break;
  case 23:
   cab->SetLightState(0,0,0,-1.0);
  break;
  case 24:
   cab->SetLightState(0,1,LIGHTINT_BRIGHT,-1.0,4.5,3.0);
  break;
  case 25:
   cab->SetLightState(0,1,LIGHTINT_DARK,-1.0,2.0,2.0);
  break;
  case 26:
   cab->SetLightState(0,1,LIGHTINT_GREEN,-1.0,2.0,2.0);
  break;
  case 27:
   cab->SetLightState(1,1,LIGHTINT_GREEN,-1.0,2.0,2.0);
  break;
  case 28:
   cab->SetLightState(1,0,0,-1.0);
  break;
  case 29:
   cab->SetLightState(1,0,0,-1.0);
  break;
  case 30:
   cab->SetLightState(1,1,LIGHTINT_BRIGHT,-1.0,4.5,3.0);
  break;
  case 31:
   cab->SetLightState(1,1,LIGHTINT_DARK,-1.0,2.0,2.0);
  break;
  case 32:
   cab->SetLightState(1,1,LIGHTINT_GREEN,-1.0,2.0,2.0);
  break;
 };
};

void SwitchGV(const ElectricLocomotive *loco,bool On){
 ElectricEngine *eng=(ElectricEngine *)loco->Eng();
 if(On!=bool(eng->MainSwitch)){
  if(On){
   UINT &Flags=*(UINT *)&eng->var[0];
   eng->MainSwitch=1;
   loco->PostTriggerBoth(23);
   Flags|=8<<16;
  }else{
   eng->MainSwitch=0;
   loco->PostTriggerBoth(24);
  };
 };
};

inline bool IsHP(ElectricEngine *eng){
 if(!(eng->ThrottlePosition%5) || eng->ThrottlePosition==32)
  return true;
 return false;
};

inline bool ShuntsAllowed(const ElectricEngine *eng){
 if(eng->ThrottlePosition>=26 && eng->ThrottlePosition<=32)
  return true;
 return false;
};


extern "C" void __export  ChangeLoco
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 loco->LocoFlags|=1;
};


extern "C" bool __export CanWorkWith(const Locomotive *loco,const wchar_t *Type){
 if(!lstrcmpiW(Type,L"chs4z")){
  return true;
 };
 if(!lstrcmpiW(Type,L"chs4")){
  //loco->LibParam=1;
  return true;
 };

 return false;
};

extern "C" bool __export  CanSwitch(const ElectricLocomotive *loco,const ElectricEngine *eng,
        unsigned int SwitchID,unsigned int SetState)
{
 UINT Set;
 Cabin *cab=loco->Cab();
 if(SwitchID==100 || SwitchID==0){
  //Throttle controller
  Set=cab->Switch(SwitchID);
  if(Set<=1 || Set>=12){
   //switch normal
   if(SetState>=2&&SetState<12){
    if(Set<2){
     cab->SetSwitch(SwitchID,12,false);
     SetState=12;
    }else{
     cab->SetSwitch(SwitchID,1,false);
     SetState=1;
    };
   };
  }else{
   //shunts
   if(SetState<5||SetState>10){
    return false;
   };
  };
  if(SetState && SetState!=14)
   cab->SetSwitchFixedState(SwitchID,SetState,false);
  loco->PostTriggerCab(104);
 }else if(SwitchID==200){
  //Shunt dummy switcher
  ULONG LocoOn=IsLocoOn(loco,*(unsigned long *)&eng->var[0]);
  if(SetState==1){
   if((LocoOn&1) && (LocoOn&6))
    eng->var[13]=0.0;
   cab->SetSwitchFixedState(0,10,true);
   cab->SetSwitchFixedState(100,10,true);
  }else{
   if((LocoOn&1) && (LocoOn&6))
    eng->var[4]=0.0;
   cab->SetSwitchFixedState(0,12,true);
   cab->SetSwitchFixedState(100,12,true);
  };
 }else if(SwitchID==1 || SwitchID==101){
  if(eng->ThrottlePosition)
   return false;
  if(SwitchID==1)
   if(cab->SwitchSet(101))
    return false;
  if(SwitchID==101)
   if(cab->SwitchSet(1))
    return false;
  loco->PostTriggerCab(15);
 };

 if(SwitchID==2 || SwitchID==3 || SwitchID==102 || SwitchID==103 ||
    (SwitchID>=33 && SwitchID<=35) || (SwitchID>=133 && SwitchID<=135))
 {
  loco->PostTriggerCab(17);
  
 }else if(SwitchID==6 || SwitchID==106){
  if(SetState)
   loco->PostTriggerCab(103);
 }else if((SwitchID>=7&&SwitchID<=13)||(SwitchID>=107&&SwitchID<=113)||
          SwitchID==32 || SwitchID==132 ||
          SwitchID==201 || SwitchID==202 || SwitchID==203)
 {
  loco->PostTriggerCab(37);
 }else if((SwitchID>=14&&SwitchID<=28)||(SwitchID>=29&&SwitchID<=31) ||
          (SwitchID>=114&&SwitchID<=128)||(SwitchID>=129&&SwitchID<=131))
 {
  loco->PostTriggerCab(47);
 }else if(SwitchID==29 || SwitchID==129){
  loco->PostTriggerCab(104);
 };
 
 
 return true;
};




extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
 UINT &Flags=*(UINT *)&eng->var[0];
 Cabin *cab=loco->Cab();
 ULONG LocoOn=IsLocoOn(loco,Flags);
 UINT st;
 int rev;

 switch(SwitchID){
  case 201:
   Switched(loco,eng,202,0);
   Switched(loco,eng,24,0);
   Switched(loco,eng,25,0);
   Switched(loco,eng,26,0);
   Switched(loco,eng,27,0);
   Switched(loco,eng,124,0);
   Switched(loco,eng,125,0);
   Switched(loco,eng,126,0);
   Switched(loco,eng,127,0);
  break;
  case 202:
   if((LocoOn&8)){
    Switched(loco,eng,107,0);
    Switched(loco,eng,109,0);
    Switched(loco,eng,110,0);
    Switched(loco,eng,115,0);
    Switched(loco,eng,117,0);
    Switched(loco,eng,119,0);
    Switched(loco,eng,120,0);
   }else{
    Switched(loco,eng,7,0);
    Switched(loco,eng,9,0);
    Switched(loco,eng,10,0);
    Switched(loco,eng,15,0);
    Switched(loco,eng,17,0);
    Switched(loco,eng,19,0);
    Switched(loco,eng,20,0);
   };
   if(eng->ThrottlePosition&&eng->Reverse&&eng->MainSwitch){
    SwitchGV(loco,false);
    Flags|=2;
   };
  break;
  
  case 0:
   //Controller
   if((LocoOn&19)==19){
    eng->var[4]=0.0;
    st=cab->Switch(0);
    switch(st){
     case 0:  //+
      eng->var[13]=1.0;
      eng->var[2]=0.0;
     break;
     case 1:  //+1
      if(PrevState>1){
       eng->var[13]=2.0;
       eng->var[2]=0.0;
      }else
       eng->var[13]=0.0;
     break;
     case 12: //0
      eng->var[13]=0.0;
     break;
     case 13: //-1
      if(PrevState<13){
       eng->var[13]=3.0;
       eng->var[2]=0.0;
      }else
       eng->var[13]=0.0;
     break;
     case 14: //-
      eng->var[13]=4.0;
      eng->var[2]=0.0;
     break;
     case 5: case 6: case 7: case 8: case 9: case 10:
      if(!eng->var[13] && ShuntsAllowed(eng))
       eng->var[4]=10-st;
      else
       eng->var[4]=0.0;
     break;
    };
   };
  break;
  case 100:
   //Controller
   if((LocoOn&21)==21){
    eng->var[4]=0.0;
    st=cab->Switch(100);
    switch(st){
     case 0:  //+
      eng->var[13]=1.0;
      eng->var[2]=0.0;
     break;
     case 1:  //+1
      if(PrevState>1){
       eng->var[13]=2.0;
       eng->var[2]=0.0;
      };
     break;
     case 12: //0
      eng->var[13]=0.0;
     break;
     case 13: //-1
      if(PrevState<13){
       eng->var[13]=3.0;
       eng->var[2]=0.0;
      };
     break;
     case 14: //-
      eng->var[13]=4.0;
      eng->var[2]=0.0;
     break;
     case 5: case 6: case 7: case 8: case 9: case 10:
      if(!eng->var[13] && ShuntsAllowed(eng))
       eng->var[4]=10-st;
      else
       eng->var[4]=0.0;
     break;
    };
   };
  break;
  case 1:
   //Reverse
   if((LocoOn&3)==3){
    rev=cab->Switch(1);
    if(rev)
     eng->Reverse=rev-2;
    else
     eng->Reverse=0;
   };
  break;
  case 101:
   //Reverse
   if((LocoOn&5)==5){
    rev=cab->Switch(101);
    if(rev)
     eng->Reverse=2-rev;
    else
     eng->Reverse=0;
   };
  break;
  case 17:
   //Upravlenie
   st=cab->Switch(17);
   if((LocoOn&1)&&!(LocoOn&8)){
    if(st){
     if(st==2 && (!eng->ThrottlePosition) && loco->MainResPressure>5.5){
      SwitchGV(loco,true);
      Flags&=~7;
      Flags&=~(4<<16);
     };
    }else{
     SwitchGV(loco,false);
    };
   };
  break;
  case 117:
   //Upravlenie
   st=cab->Switch(117);
   if((LocoOn&9)==9){
    if(st){
     if(st==2 && (!eng->ThrottlePosition) && loco->MainResPressure>5.5){
      SwitchGV(loco,true);
      Flags&=~7;
      Flags&=~(4<<16);
     };
    }else{
     SwitchGV(loco,false);
    };
   };
  break;
  case 2:
  case 33:
   //Train brake
   if(!cab->Switch(33)){
    st=cab->Switch(2);
    if((st==5||st==6)&& loco->TrainPipePressure>0.5)
     loco->PostTriggerBoth(101);
   };
  break;
  case 102:
  case 133:
   //Train brake
   if(!cab->Switch(133)){
    st=cab->Switch(102);
    if((st==5||st==6)&& loco->TrainPipePressure>0.5)
     loco->PostTriggerBoth(101);
   };
  break;
  case 3:
  case 34:
  case 103:
  case 134:
   //Loco brake
   rev=-1;
   if(!cab->Switch(34))
    rev=cab->Switch(3);
   if(!cab->Switch(134)){
    int rev1=cab->Switch(103);
    if(rev1>rev)
     rev=rev1;
   };
   switch(rev){
    case 0:
     eng->IndependentBrakeValue=0.0;
     if(loco->IndependentBrakePressure>0.0)
      loco->PostTriggerBoth(18);
    break;
    case 1:
     eng->IndependentBrakeValue=loco->IndependentBrakePressure;
    break;
    case 2:
    case 3:
    case 4:
    case 5:
     if(eng->IndependentBrakeValue<(rev-1)*1.0)
      eng->IndependentBrakeValue=(rev-1)*1.0;
     if(eng->IndependentBrakeValue<loco->MainResPressure &&
        eng->IndependentBrakeValue>loco->IndependentBrakePressure)
      loco->PostTriggerBoth(18);
    break;
   };
  break;
  case 11:
   //Bail-off button
   if((LocoOn&3)==3){
    if(cab->Switch(11)){
     eng->EngineFlags|=3;
     eng->var[16]=0.0;
    };
   };
  break;
  case 111:
   //Bail-off button
   if((LocoOn&5)==5){
    if(cab->Switch(111)){
     eng->EngineFlags|=3;
     eng->var[16]=0.0;
    };
   };
  break;
  case 15:
   //compressor1
   if(!(LocoOn&8)){
    st=cab->Switch(15);
    if(st==0)
     eng->var[14]=3;
    else
     eng->var[14]=st-1;
   };
  break;
  case 20:
   //compressor2
   if(!(LocoOn&8)){
    st=cab->Switch(20);
    if(st==0)
     eng->var[15]=3;
    else
     eng->var[15]=st-1;
   };
  break;
  case 115:
   //compressor1
   if(LocoOn&8){
    st=cab->Switch(115);
    if(st==0)
     eng->var[15]=3;
    else
     eng->var[15]=st-1;
   };
  break;
  case 120:
   //compressor2
   if(LocoOn&8){
    st=cab->Switch(120);
    if(st==0)
     eng->var[14]=3;
    else
     eng->var[14]=st-1;
   };
  break;
  case 28:
   //GV off
   if(!(LocoOn&8)){
    if(cab->Switch(28))
     if(eng->MainSwitch)
      SwitchGV(loco,false);
   };
  break;
  case 128:
   //GV off
   if(LocoOn&8){
    if(cab->Switch(128))
     if(eng->MainSwitch)
      SwitchGV(loco,false);
   };
  break;
  case 9:
  case 12:
   //tifon
   if(!(LocoOn&8)){
    if((LocoOn&1)&&(cab->Switch(9)||cab->Switch(12))){
     Flags|=256;
     loco->PostTriggerBoth(8);
    }else if(Flags&256){
     loco->PostTriggerBoth(9);
     Flags&=~256;
    };
   };
  break;
  case 109:
  case 112:
   //tifon
   if(LocoOn&8){
    if((LocoOn&1)&&(cab->Switch(109)||cab->Switch(112))){
     Flags|=256;
     loco->PostTriggerBoth(8);
    }else if(Flags&256){
     loco->PostTriggerBoth(9);
     Flags&=~256;
    };
   };
  break;
  case 10:
  case 13:
   //svist
   if(!(LocoOn&8)){
    if((LocoOn&1)&&(cab->Switch(10)||cab->Switch(13))){
     Flags|=512;
     loco->PostTriggerBoth(10);
    }else if(Flags&512){
     loco->PostTriggerBoth(11);
     Flags&=~512;
    };
   };
  break;
  case 110:
  case 113:
   //svist
   if(LocoOn&8){
    if((LocoOn&1)&&(cab->Switch(110)||cab->Switch(113))){
     Flags|=512;
     loco->PostTriggerBoth(10);
    }else if(Flags&512){
     loco->PostTriggerBoth(11);
     Flags&=~512;
    };
   };
  break;
  case 7:
   //PBZ
   if(!(LocoOn&8)){
    if(((LocoOn&3)==3)&&cab->Switch(7)&&loco->SandLeft){
     Flags|=1024;
     eng->Sanding=1;
     loco->PostTriggerBoth(4);
    }else if(Flags&1024){
     loco->PostTriggerBoth(5);
     Flags&=~1024;
     eng->Sanding=0;
    };
   };
  break;
  case 107:
   //PBZ
   if(LocoOn&8){
    if(((LocoOn&5)==5)&&cab->Switch(107)&&loco->SandLeft){
     Flags|=1024;
     eng->Sanding=1;
     loco->PostTriggerBoth(4);
    }else if(Flags&1024){
     loco->PostTriggerBoth(5);
     Flags&=~1024;
     eng->Sanding=0;
    };
   };
  break;
  case 6:
   //RB
   if(!(LocoOn&8)){
    if((LocoOn&1)&&cab->Switch(6)){
     if(!(Flags&64)||fabs(loco->Velocity)<0.1){
      eng->var[7]=0.0;
      eng->var[9]=0.0;
     }else{
      loco->PostTriggerBoth(57);
     };
    };
   };
  break;
  case 106:
   //RB
   if(LocoOn&8){
    if((LocoOn&1)&&cab->Switch(106)){
     if(!(Flags&64)||fabs(loco->Velocity)<0.1){
      eng->var[7]=0.0;
      eng->var[9]=0.0;
     }else{
      loco->PostTriggerBoth(57);
     };
    };
   };
  break;
  case 19:
   //ventilators
   if(!(LocoOn&8)){
    if((LocoOn&1)){
     st=cab->Switch(19);
     if(!st)
      eng->var[17]=1.0;
     else if(st==1)
      eng->var[17]=0.0;
     else
      eng->var[17]=st;
    };
   };
  break;
  case 119:
   //ventilators
   if(LocoOn&8){
    if((LocoOn&1)){
     st=cab->Switch(119);
     if(!st)
      eng->var[17]=1.0;
     else if(st==1)
      eng->var[17]=0.0;
     else
      eng->var[17]=st;
    };
   };
  break;
  case 24:
   //BFL
   if(LocoOn&1){
    SwitchLights(loco,eng,3+cab->Switch(24));
   }else{
    SwitchLights(loco,eng,4);
   };
  break;
  case 25:
   //BFR
   if(LocoOn&1){
    SwitchLights(loco,eng,6+cab->Switch(25));
   }else{
    SwitchLights(loco,eng,7);
   };
  break;
  case 26:
   //Projector
   if(LocoOn&1){
    SwitchLights(loco,eng,0+cab->Switch(26));
   }else{
    SwitchLights(loco,eng,0);
   };
  break;
  case 124:
   //BFLB
   if(LocoOn&1){
    SwitchLights(loco,eng,15+cab->Switch(124));
   }else{
    SwitchLights(loco,eng,16);
   };
  break;
  case 125:
   //BFRB
   if(LocoOn&1){
    SwitchLights(loco,eng,18+cab->Switch(125));
   }else{
    SwitchLights(loco,eng,19);
   };
  break;
  case 126:
   //Projector back
   if(LocoOn&1){
    SwitchLights(loco,eng,12+cab->Switch(126));
   }else{
    SwitchLights(loco,eng,12);
   };
  break;
  case 27:
   //Cabin light
   if(LocoOn&1){
    SwitchLights(loco,eng,21+cab->Switch(27));
   }else{
    SwitchLights(loco,eng,23);
   };
   if((LocoOn&1) && (cab->Switch(27)<2 || cab->Switch(127)<2)){
    cab->ChangeTexture(0,1);
    cab->ChangeTexture(1,1);
   }else{
    cab->ChangeTexture(0,0);
    cab->ChangeTexture(1,0);
   };
  break;
  case 127:
   //Cabin light back
   if(LocoOn&1){
    SwitchLights(loco,eng,27+cab->Switch(127));
   }else{
    SwitchLights(loco,eng,29);
   };
   if((LocoOn&1) && (cab->Switch(27)<2 || cab->Switch(127)<2)){
    cab->ChangeTexture(0,1);
    cab->ChangeTexture(1,1);
   }else{
    cab->ChangeTexture(0,0);
    cab->ChangeTexture(1,0);
   };
  break;
 };
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
      if(SigDist<10.0)
       if(Vel>1.5)
        res=2;
     };
    };
   };
  };
 };
 return res;
};


extern "C" void _export SpeedLimit(const Locomotive *loco,
        SpeedLimitDescr Route,SpeedLimitDescr Signal,SpeedLimitDescr Event)
{
 Engine *eng=loco->Eng();
 float Limit=GetLimit(Route.Limit,Signal.Limit,Event.Limit);
 eng->var[10]=Limit;
 if(Limit<6.945){
  eng->var[10]=7.0;
 };
};



extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 UINT &Flags=*(UINT *)&eng->var[0];
 ULONG LocoOn=IsLocoOn(loco,Flags);
 Cabin *cab=loco->Cab();
 SMSObject *soundExt=loco->SoundEng(),
           *soundCab=loco->SoundCab();
 float BatteryCurrent=0.0;
 bool CurrentOn=false;
 eng->PowerConsuming=0.0;

 //Pantographs
 UCHAR HadRaised=eng->Panto;
 eng->Panto=0;
 if(LocoOn&1){
  if(cab->Switch(21)||cab->Switch(116)){
   eng->Panto|=1;
   if(!(HadRaised&1)){
    if(loco->MainResPressure>4.0)
     loco->PostTriggerBoth(45);
    else
     eng->Panto&=~1;
   };
  }else{
   if(HadRaised&1)
    loco->PostTriggerBoth(46);
  };
  if(cab->Switch(16)||cab->Switch(121)){
   eng->Panto|=2;
   if(!(HadRaised&2)){
    if(loco->MainResPressure>4.0)
     loco->PostTriggerBoth(45);
    else
     eng->Panto&=~2;
   };
  }else{
   if(HadRaised&2)
    loco->PostTriggerBoth(46);
  };
 }else{
  if(HadRaised)
   loco->PostTriggerBoth(46);
 };
 HadRaised=eng->Panto;

 if(eng->MainSwitch && loco->LineVoltage>19000.0)
  CurrentOn=true;

 if(eng->MainSwitch && loco->PantoRaised){
  if(loco->LineVoltage<19000.0){
   SwitchGV(loco,false);
   Flags|=2;
  }else if(loco->LineVoltage>29000.0){
   SwitchGV(loco,false);
   Flags|=4;
  }else if(loco->LineVoltage>200.0 && !loco->LineFreq){
   //DC
   SwitchGV(loco,false);
   Flags|=1;
  };
 };
 if(eng->MainSwitch)
  if((!CurrentOn&&(Flags&32))||(CurrentOn&&!(Flags&32))){
   SwitchGV(loco,false);
   Flags|=2;
  };
 if(loco->LineVoltage>19000.0)
  Flags|=32;
 else
  Flags&=~32;
 if(!eng->MainSwitch)
  CurrentOn=false;

 //Transformer
 if(CurrentOn){
  if(!(Flags&8192)){
   Flags|=8192;
   loco->PostTriggerBoth(25);
  };
 }else{
  if(Flags&8192){
   Flags&=~8192;
   loco->PostTriggerBoth(26);
  };
 };

 //Throttle reset when EPK off
 if((LocoOn&32) && !(LocoOn&16) && eng->Reverse)
  eng->var[13]=4;

 //Throttle switch
 if((eng->var[13]>0.0) && (LocoOn&1)){
  eng->var[2]+=time;
  UINT ThrottleSwitch=eng->var[13];
  bool WasOn=eng->ThrottlePosition;
  switch(ThrottleSwitch){
   case 1: //+
    if(eng->ThrottlePosition>=32){
     eng->var[13]=0.0;
     break;
    };
    if(!(LocoOn&6)){
     eng->var[13]=0.0;
     break;
    };
    if(eng->var[2]>THROTTLE_SWITCH_TIME){
     eng->var[2]-=THROTTLE_SWITCH_TIME;
     if((LocoOn&17)==17)
      eng->ThrottlePosition++;
    };
   break;
   case 2: //+1
    if(eng->ThrottlePosition>=32){
     eng->var[13]=0.0;
     break;
    };
    if(eng->var[2]>THROTTLE_SWITCH_TIME){
     if((LocoOn&17)==17)
      eng->ThrottlePosition++;
     eng->var[13]=0.0;
    };
   break;
   case 3: //-1
    if(!eng->ThrottlePosition){
     eng->var[13]=0.0;
     break;
    };
    if(eng->var[2]>THROTTLE_SWITCH_TIME){
     if((LocoOn&17)==17)
      eng->ThrottlePosition--;
     eng->var[13]=0.0;
    };
   break;
   case 4: //-
    if(!eng->ThrottlePosition){
     eng->var[13]=0.0;
     break;
    };
    if(!(LocoOn&6)){
     eng->var[13]=0.0;
     break;
    };
    if(eng->var[2]>THROTTLE_SWITCH_TIME){
     eng->var[2]-=THROTTLE_SWITCH_TIME;
     eng->ThrottlePosition--;
    };
   break;
  };
  
  if((LocoOn&17)!=17){
   eng->var[13]=0;
  }else{
   if(eng->Reverse && WasOn!=bool(eng->ThrottlePosition))
    loco->PostTriggerBoth(102);
   if(eng->var[2]-time<0.1 && eng->var[2]>0.1)
    loco->PostTriggerBoth(105);
  };
 };

 //Compressor
 if(loco->MainResPressure>4.0)
  eng->MainResRate=-5e-4*(loco->MainResPressure-4.0);
 else
  eng->MainResRate=0.0;
 if(loco->PantoRaised!=eng->Panto)
  eng->MainResRate-=0.08;
 if(Flags&(8<<16)){
  eng->MainResRate-=0.2/time;
  Flags&=~(8<<16);
 };
 //compressor1
 if((LocoOn&1)&&CurrentOn){
  UINT ComprOn=eng->var[14];
  bool On=bool(Flags&8);
  if(!On){
   switch(ComprOn){
    case 1:
     if(loco->MainResPressure<7.0)
      On=true;
    break;
    case 2:
     On=true;
    break;
   };
   if(On){
    Flags|=8;
   };
  }else{
   eng->MainResRate+=0.025*(11.21-loco->MainResPressure);
   eng->PowerConsuming+=35000.0;
   switch(ComprOn){
    case 0:
     On=false;
    break;
    case 1:
     if(loco->MainResPressure>10.0){
      On=false;
     };
    break;
   };
  };
  if(!On){
   Flags&=~8;
  };
 }else{
  if(Flags&8){
   Flags&=~8;
  };
 };
 //compressor2
 if((LocoOn&1)&&CurrentOn){
  UINT ComprOn=eng->var[15];
  bool On=bool(Flags&16);
  if(!On){
   switch(ComprOn){
    case 1:
     if(loco->MainResPressure<7.0)
      On=true;
    break;
    case 2:
     On=true;
    break;
   };
   if(On){
    Flags|=16;
   };
  }else{
   eng->MainResRate+=0.025*(11.21-loco->MainResPressure);
   eng->PowerConsuming+=35000.0;
   switch(ComprOn){
    case 0:
     On=false;
    break;
    case 1:
     if(loco->MainResPressure>10.0){
      On=false;
     };
    break;
   };
  };
  if(!On){
   Flags&=~16;
  };
 }else{
  if(Flags&16){
   Flags&=~16;
  };
 };
 //compressor sounds
 if((Flags&24)&&!(Flags&2048)){
  Flags|=2048;
  loco->PostTriggerBoth(12);
 }else if(!(Flags&24)&&(Flags&2048)){
  Flags&=~2048;
  loco->PostTriggerBoth(13);
 };
 if((Flags&2048)){
  float sound=0.0;
  if(Flags&8)
   sound+=1.0;
  if(Flags&16)
   sound+=1.0;
  if(soundCab)
   soundCab->Var2[0]=sound;
  if(soundExt)
   soundExt->Var2[0]=sound;
 };

 //Auxilary compressor
 UCHAR AuxC=0;
 if(LocoOn&1){
  if(LocoOn&2){
   switch(cab->Switch(22)){
    case 2: AuxC=1; break;
    case 3: AuxC=2; break;
    case 4: AuxC=3; break;
   };
  }else if(LocoOn&4){
   switch(cab->Switch(122)){
    case 2: AuxC=2; break;
    case 3: AuxC=1; break;
    case 4: AuxC=3; break;
   };
  };
  float AuxCPower=(AuxC&1)+((AuxC&2)>>1);
  if(AuxCPower>0.0){
   float AuxCRate=((5.6+AuxCPower*0.4)-loco->MainResPressure)*AUXC_RATE*AuxCPower;
   if(AuxCRate>0.0)
    eng->MainResRate+=AuxCRate;
  };
 };

 //Ventilators
 if((LocoOn&1)&&CurrentOn){
  bool On=bool(Flags&4096);
  UINT VentMode=eng->var[17];
  if(!On){
   switch(VentMode){
    case 0:
     if(fabs(eng->Force)>0.0)
      On=true;
    break;
    case 2:
    case 3:
    case 4:
     On=true;
    break;
   };
   if(On){
    loco->PostTriggerBoth(6);
    Flags|=4096;
   };
  }else{
   switch(VentMode){
    case 1: On=false; break;
    case 0:
     if(!eng->Force)
      On=false;
     break;
   };
   if(!On){
    loco->PostTriggerBoth(7);
    Flags&=~4096;
   };
  };
  if(Flags&4096){
   float vol=0.0;
   if(VentMode==0 || VentMode==2){
    vol=0.85;
    eng->PowerConsuming+=10000.0;
   }else if(VentMode==3 || VentMode==4){
    vol=1.5;
    eng->PowerConsuming+=15000.0;
   };
   if(soundCab)
    soundCab->Var1[0]=vol;
   if(soundExt)
    soundExt->Var1[0]=vol;
  };
 }else if(Flags&4096){
  loco->PostTriggerBoth(7);
  Flags&=~4096;
 };


 //Throttle
 if(CurrentOn && eng->MainSwitch && eng->Reverse && ((LocoOn&17)==17)){
  eng->Force=0.0;
  if(eng->ThrottlePosition){
   float VelMax=-10.783+0.285*eng->ThrottlePosition; //pow(,1.03)
   float Velocity=loco->Velocity*eng->Reverse-VelMax;
   if(Velocity<0.01)Velocity=0.01;
   eng->Force = (291184.0)/(pow(Velocity,4.60-0.057*eng->ThrottlePosition))*10000.0;
   if(eng->var[4]>0.0)
    eng->Force*=1.05+eng->var[4]*0.15;
   eng->Force*=loco->LineVoltage/25000.0;
  };

  //Force smoothing
  if(eng->Force>eng->var[5]){
   eng->var[5]+=FORCE_SMOOTH_INCR*time;
   if(eng->Force>eng->var[5])
    eng->Force=eng->var[5];
  }else if(eng->Force<eng->var[5]){
   eng->var[5]-=FORCE_SMOOTH_DECR*time;
   if(eng->Force<eng->var[5])
    eng->Force=eng->var[5];
  };

  float Current=eng->Force/CURRENT_Q;
  if(eng->ThrottlePosition)
   Current+=eng->ThrottlePosition*4.69+33.0;
  if(eng->var[4]>0.0)
   Current*=1.0+eng->var[4]*0.05;
  if(Current>1800.0){
   SwitchGV(loco,false);
   Flags|=1;
   Current=0.0;
   eng->Force=0.0;
  }else if(eng->Force>0.0 && loco->BrakeCylinderPressure>1.5){
   SwitchGV(loco,false);
   Current=0.0;
   eng->Force=0.0;
  };
  //Autosand
  bool AutoSandOn=false;
  if((LocoOn&2)&&!cab->Switch(22)&&!cab->Switch(7))
   AutoSandOn=true;
  else if((LocoOn&4)&&!cab->Switch(122)&&!cab->Switch(107))
   AutoSandOn=true;
  if(AutoSandOn){
   if(Current>1200.0 && loco->SandLeft){
    if(!(Flags&1024)){
     eng->Sanding=1;
     loco->PostTriggerBoth(4);
     Flags|=1024;
    };
   }else{
    if(Flags&1024){
     eng->Sanding=0;
     Flags&=~1024;
     loco->PostTriggerBoth(5); 
    };
   };
  };

  float EngineForce=eng->Force/loco->NumEngines;
  float EngineVoltage=eng->ThrottlePosition*24.0*loco->LineVoltage/25000.0;
  for(UINT i=0;i<loco->NumEngines;i++){
   eng->EngineCurrent[i]=Current;
   eng->EngineForce[i]=EngineForce;
   eng->EngineVoltage[i]=EngineVoltage;
   eng->PowerConsuming+=Current*EngineVoltage;
  };
  if(soundCab)
   soundCab->Var3[0]=Current/1000.0;
  if(soundExt)
   soundExt->Var3[0]=Current/1000.0;

  //Ground
  if(EngineVoltage>715.0){
   Flags|=4<<16;
  };
  if(Flags&(4<<16)){
   eng->var[18]+=(EngineVoltage-600.0)*0.3*time;
   if(eng->var[18]>2000.0){
    SwitchGV(loco,false);
    Flags|=1;
    Current=0.0;
    eng->Force=0.0;
   }else if(eng->var[18]<0.0){
    eng->var[18]=0.0;
    Flags&=~(4<<16);
   };
  };

  eng->var[5]=eng->Force;
  eng->Force*=eng->Reverse;
 }else{
  eng->Force=0.0;
  eng->var[5]=0.0;
  eng->var[18]=0.0;
  //Autosand
  if(eng->Sanding){
   bool AutoSandOn=false;
   if((LocoOn&2)&&!cab->Switch(22)&&!cab->Switch(7))
    AutoSandOn=true;
   else if((LocoOn&4)&&!cab->Switch(122)&&!cab->Switch(107))
    AutoSandOn=true;
   if(AutoSandOn)
    eng->Sanding=0;
  };
  for(UINT i=0;i<loco->NumEngines;i++){
   eng->EngineCurrent[i]=0.0;
   eng->EngineForce[i]=0.0;
   eng->EngineVoltage[i]=0.0;
  };
  if(soundCab)
   soundCab->Var3[0]=0.0;
  if(soundExt)
   soundExt->Var3[0]=0.0;
 };

 //EPK
 if(((LocoOn&49)==49)){
  UINT Aspect=cab->Signal.Aspect[0],PrevAspect=eng->var[8];
  const float Vel=fabs(loco->Velocity);
  bool Moving=Vel>0.1;
  bool Release=false;
  if(!Moving)
   Flags&=~64;
  //if(Aspect==SIGASP_BLOCK_OBSTRUCTED)
   //Aspect=SIGASP_STOP;
  if(Flags&(1<<16)){
   eng->var[7]=4.0;
   eng->var[9]=30.0;
  }else if(Aspect==SIGASP_STOP_AND_PROCEED && Moving){
   eng->var[7]=3.0;
   if(PrevAspect!=SIGASP_STOP_AND_PROCEED)
    eng->var[9]=36.0;
   UINT appr=ApproachRed(cab->Signal.Distance,Vel);
   if(appr==1 && eng->var[9]<30.0){
    eng->var[9]=30.0;
   }else if(appr==2){
    eng->var[9]=50.0;
   };
  }else if(Moving && (Aspect==SIGASP_STOP || Aspect==SIGASP_BLOCK_OBSTRUCTED ||
           Aspect==SIGASP_RESTRICTING))
  {
   eng->var[7]=3.0;
  }else if(Moving && (Aspect>=SIGASP_APPROACH_1 && Aspect<=SIGASP_CLEAR_1))
  {
   if(cab->Signal.SpeedLimit>=0.0 && eng->var[7]<1.0){
    if(Vel>cab->Signal.SpeedLimit){
     eng->var[7]=1.0;
     if(eng->var[9]<15.0)
      eng->var[9]=15.0;
    }else if(eng->var[7]==1.0){
     eng->var[7]=0.0;
     Release=true;
    };
   };
  }else if(Aspect>PrevAspect){
   eng->var[7]=0.0;
   Release=true;
  };
  if(Moving && Aspect<PrevAspect){
   if(eng->var[7]<=1.0){
    eng->var[7]=1.0;
    if(eng->var[9]<30.0)
     eng->var[9]=30.0;
   };
  };
  if(eng->var[10]>0.0 && Vel>eng->var[10] &&
        UINT(eng->var[7])!=4 && eng->var[9]<44.0)
  {
   eng->var[7]=5.0;
   if(eng->var[9]<30.0)
    eng->var[9]=30.0;
   Release=false;
  };
  if(!Moving && eng->var[7]>0.0 && UINT(eng->var[7])!=4)
   eng->var[7]=0.0;

  if(Release){
   loco->PostTriggerBoth(106);
   Flags&=~(2<<16);
  };
  if(eng->var[7]>0.0){
   eng->var[9]+=time;
   if(eng->var[9]>=36.0){
    if(!(Flags&(2<<16))){
     Flags|=2<<16;
     loco->PostTriggerBoth(56);
    };
   }else{
    if(Flags&(2<<16)){
     Flags&=~(2<<16);
     loco->PostTriggerBoth(57);
    };
   };
   if(eng->var[9]>=44.0)
    Flags|=64;
  }else{
   if(Flags&(2<<16)){
    Flags&=~(2<<16);
    loco->PostTriggerBoth(57);
   };
   eng->var[9]=0.0;
  };;

  eng->var[8]=Aspect;
  Flags&=~(1<<16);
 }else{
  eng->var[7]=0.0;
  eng->var[8]=0.0;
  eng->var[9]=0.0;
  Flags|=1<<16;
  Flags&=~64;
  if(Flags&(2<<16)){
   Flags&=~(2<<16);
   loco->PostTriggerBoth(57);
  };
 };

 //Handbrake
 eng->HandbrakePercent=(1.0-cab->GetSwitchFrame(5))*50.0 +
  (1.0-cab->GetSwitchFrame(105))*50.0;
 //Locomotive brake
 if(eng->IndependentBrakeValue>loco->MainResPressure)
  eng->IndependentBrakeValue=loco->MainResPressure;
 if(eng->IndependentBrakeValue>loco->IndependentBrakePressure)
  eng->MainResRate-=0.018;

  
 //Train brake
 eng->BrakeSystemsEngaged=5;
 if(LocoOn&1)
  if(cab->Switch(203))
   eng->BrakeSystemsEngaged|=2;
  /*if(LocoOn&96){
   eng->BrakeSystemsEngaged=1;
   if(LocoOn&256)
    eng->BrakeSystemsEngaged|=2;*/
 int brake=-1,brakeset=-1;
 if(!cab->Switch(33)){
  brake=cab->Switch(2);
  brakeset=cab->SwitchSet(2);
 };
 if(!cab->Switch(133)){
  int brake1=cab->Switch(102);
  int brakeset1=cab->SwitchSet(102);
  if(brake1>brake)
   brake=brake1;
  if(brakeset1>brakeset)
   brakeset=brakeset1;
 };
 eng->ChargingRate=(CHARGE_PIPE_VALUE-loco->ChargingPipePressure)*PIPE_CHARGE_Q;
 /*if(brakeset>1 && brakeset<6){
  eng->ChargingRate=(CHARGE_PIPE_VALUE1-loco->ChargingPipePressure)*PIPE_CHARGE_Q;
  if(eng->ChargingRate<0.0)
   eng->ChargingRate=0.0;
 };*/
 if(Flags&64)
  brake=6;
 if(eng->UR>loco->MainResPressure)
  eng->UR=loco->MainResPressure;
 switch(brake){
  case 0:
   if(brakeset!=0)
    break;
   eng->UR+=UR_CHARGE_RATE*time;
   if(eng->UR>loco->MainResPressure)
    eng->UR=loco->MainResPressure;
   eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q_REL;
   if(eng->TrainPipeRate<0.0)
    eng->TrainPipeRate=0.0;
   else if(eng->TrainPipeRate>1.2)
    eng->TrainPipeRate=1.2;
   eng->var[16]-=EPT_RELEASE_RATE1*time;
   if(eng->var[16]<0.0)
    eng->var[16]=0.0;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[16];
  break;
  case 1:
   if(eng->UR<5.2)
    eng->UR+=UR_CHARGE_RATE*time;
   eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
   if(eng->TrainPipeRate<0.0){
    if(eng->TrainPipeRate>-0.1){
     eng->TrainPipeRate=0.0;
    };
   }else if(eng->TrainPipeRate>1.2)
    eng->TrainPipeRate=1.2;
   if(eng->TrainPipeRate>0.0)
    eng->UR-=UR_DISCHARGE2*time;
   eng->var[16]-=EPT_RELEASE_RATE*time;
   if(eng->var[16]<0.0)
    eng->var[16]=0.0;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[16];
  break;
  case 2:
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>PIPE_DISCHARGE_SLOW)
    eng->TrainPipeRate=PIPE_DISCHARGE_SLOW;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[16];
  break;
  case 3:
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>0.0)
    eng->TrainPipeRate*=0.05;
   if(eng->TrainPipeRate>0.02)
    eng->TrainPipeRate=0.02;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=eng->var[16];
  break;
  case 4:
   if(brakeset<4)
    break;
   if(eng->BrakeSystemsEngaged&2){
    eng->var[16]+=EPT_APPLY_RATE*time;
    if(eng->var[16]>EPT_MAX)
     eng->var[16]=EPT_MAX;
    eng->EPTvalue=eng->var[16];
   };
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>0.0)
    eng->TrainPipeRate=0.0;
  break;
  case 5:
   if(brakeset<5)
    if(!(eng->BrakeSystemsEngaged&2))
     break;
   eng->UR+=UR_DISCHARGE_RATE*time;
   if(eng->UR<0.0)
    eng->UR=0.0;
   eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
   if(eng->TrainPipeRate>0.0)
    eng->TrainPipeRate=0.0;
   eng->var[16]+=EPT_APPLY_RATE*time;
   if(eng->var[16]<loco->BrakeCylinderPressure)
    eng->var[16]=loco->BrakeCylinderPressure;
   if(eng->var[16]>EPT_MAX)
    eng->var[16]=EPT_MAX;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=-1.0;
  break;
  case 6:
   eng->UR+=UR_EMERGENCY_RATE*time;
   if(eng->UR<0.0)
    eng->UR=0.0;
   eng->var[16]=EPT_MAX;
   if(eng->BrakeSystemsEngaged&2)
    eng->EPTvalue=EPT_MAX;
   eng->TrainPipeRate=UR_EMERGENCY_RATE;
  break;
 };

 //Sanding
 if(Flags&1024){
  if(!loco->SandLeft||!eng->Sanding){
   Flags&=~1024;
   loco->PostTriggerBoth(5);
  };
 };



 if((State>>8)&1){
  float val;

  //Lamps and indicators
  for(UINT i=14;i<=37;i++)
   cab->SetDisplayState(i,0);
  for(UINT i=64;i<=87;i++)
   cab->SetDisplayState(i,0);
  if(LocoOn&1){
   if(!(LocoOn&8)){
    //front cabin active
    if(eng->MainSwitch)
     cab->SetSwitch(29,1,false);
    else if(LocoOn&1)
     cab->SetSwitch(29,3,false);
    if(cab->Switch(32))
     eng->ALSNOn=0xFFFF;
    else if((LocoOn&48)==48)
     eng->ALSNOn=1;
    else
     eng->ALSNOn=0;

    //Lamps
    if(eng->Sanding)
     cab->SetDisplayState(14,1);
    if(AuxC)
     cab->SetDisplayState(15,1);
    if(loco->BrakeCylinderPressure>0.0 || loco->IndependentBrakePressure>0.0)
     cab->SetDisplayState(17,1);
    if(Flags&128)
     cab->SetDisplayState(18,1);
    if(Flags&(4<<16))
     cab->SetDisplayState(20,1);
    if(Flags&7)
     cab->SetDisplayState(21,1);
    if(!CurrentOn){
     cab->SetDisplayState(22,1);
     cab->SetDisplayState(27,1);
     cab->SetDisplayState(28,1);
     cab->SetDisplayState(29,1);
     cab->SetDisplayState(30,1);
    };
    if(eng->var[13]>0.0 && eng->var[2]>0.1 && eng->var[2]<0.5)
     cab->SetDisplayState(24,1);
    if(!eng->ThrottlePosition)
     cab->SetDisplayState(25,1);
    if((eng->BrakeSystemsEngaged&2)&&!cab->Switch(33)){
     UINT b=cab->Switch(2);
     if(b==4)
      cab->SetDisplayState(35,1);
     else if(b==3||b==2)
      cab->SetDisplayState(36,1);
     else if(b==1)
      cab->SetDisplayState(37,1);
    };

   }else{
    //Back cabin active
    if(eng->MainSwitch)
     cab->SetSwitch(129,1,false);
    else if(LocoOn&1)
     cab->SetSwitch(129,3,false);
    if(cab->Switch(132))
     eng->ALSNOn=0xFFFF;
    else if((LocoOn&48)==48)
     eng->ALSNOn=1;
    else
     eng->ALSNOn=0;

    //Lamps
    if(eng->Sanding)
     cab->SetDisplayState(64,1);
    if(AuxC)
     cab->SetDisplayState(65,1);
    if(loco->BrakeCylinderPressure>0.0 || loco->IndependentBrakePressure>0.0)
     cab->SetDisplayState(67,1);
    if(Flags&128)
     cab->SetDisplayState(68,1);
    if(Flags&(4<<16))
     cab->SetDisplayState(70,1);
    if(Flags&7)
     cab->SetDisplayState(71,1);
    if(!CurrentOn){
     cab->SetDisplayState(72,1);
     cab->SetDisplayState(77,1);
     cab->SetDisplayState(78,1);
     cab->SetDisplayState(79,1);
     cab->SetDisplayState(80,1);
    };
    if(eng->var[13]>0.0 && eng->var[2]>0.1 && eng->var[2]<0.45)
     cab->SetDisplayState(74,1);
    if(!eng->ThrottlePosition)
     cab->SetDisplayState(75,1);
    if((eng->BrakeSystemsEngaged&2)&&!cab->Switch(133)){
     UINT b=cab->Switch(102);
     if(b==4)
      cab->SetDisplayState(85,1);
     else if(b==3||b==2)
      cab->SetDisplayState(86,1);
     else if(b==1)
      cab->SetDisplayState(87,1);
    };

   };
  }else{
   cab->SetSwitch(29,2,false);
   cab->SetSwitch(129,2,false);
   eng->ALSNOn=0;
  };

  if(LocoOn&2){
   if(eng->MainSwitch)
    cab->SetDisplayValue(5,loco->LineVoltage*0.001);
   else
    cab->SetDisplayValue(5,0.0);
   cab->SetDisplayValue(6,eng->ThrottlePosition);
   cab->SetDisplayValue(7,eng->EngineVoltage[0]);
   cab->SetDisplayValue(8,eng->EngineCurrent[0]);
   cab->SetDisplayValue(9,eng->EngineCurrent[3]);
   if(eng->var[7]>0.0 && eng->var[9]>30.0){
    UINT EPKState=eng->var[7];
    switch(EPKState){
     case 1:
      cab->SetDisplayState(38,1);
      cab->SetDisplayState(39,1);
      cab->SetDisplayFade(38,-1.0,0.5);
      cab->SetDisplayFade(39,-1.0,0.5);
     break;
     case 3:
     case 4:
      cab->SetDisplayState(38,1);
      cab->SetDisplayState(39,1);
      cab->SetDisplayFade(38,-1.0,0.0);
      cab->SetDisplayFade(39,-1.0,0.0);
     break;
     case 5:
      cab->SetDisplayState(38,1);
      cab->SetDisplayState(39,1);
      cab->SetDisplayFade(38,-1.0,0.3);
      cab->SetDisplayFade(39,-1.0,0.3);
     break;
    };
   }else{
    cab->SetDisplayState(38,0);
    cab->SetDisplayState(39,0);
   };
  }else{
   cab->SetDisplayValue(5,0.0);
   cab->SetDisplayValue(7,0.0);
   cab->SetDisplayValue(8,0.0);
   cab->SetDisplayValue(9,0.0);
   cab->SetDisplayState(38,0);
   cab->SetDisplayState(39,0);
   cab->SetDisplayState(40,0);
  };

  if(LocoOn&4){
   if(eng->MainSwitch)
    cab->SetDisplayValue(55,loco->LineVoltage*0.001);
   else
    cab->SetDisplayValue(55,0.0);
   cab->SetDisplayValue(56,eng->ThrottlePosition);
   cab->SetDisplayValue(57,eng->EngineVoltage[0]);
   cab->SetDisplayValue(58,eng->EngineCurrent[3]);
   cab->SetDisplayValue(59,eng->EngineCurrent[0]);
   if(eng->var[7]>0.0 && eng->var[9]>30.0){
    UINT EPKState=eng->var[7];
    switch(EPKState){
     case 1:
      cab->SetDisplayState(88,1);
      cab->SetDisplayState(89,1);
      cab->SetDisplayFade(88,-1.0,0.5);
      cab->SetDisplayFade(89,-1.0,0.5);
     break;
     case 3:
     case 4:
      cab->SetDisplayState(88,1);
      cab->SetDisplayState(89,1);
      cab->SetDisplayFade(88,-1.0,0.0);
      cab->SetDisplayFade(89,-1.0,0.0);
     break;
     case 5:
      cab->SetDisplayState(88,1);
      cab->SetDisplayState(89,1);
      cab->SetDisplayFade(88,-1.0,0.3);
      cab->SetDisplayFade(89,-1.0,0.3);
     break;
    };
   }else{
    cab->SetDisplayState(88,0);
    cab->SetDisplayState(89,0);
   };
  }else{
   cab->SetDisplayValue(55,0.0);
   cab->SetDisplayValue(57,0.0);
   cab->SetDisplayValue(58,0.0);
   cab->SetDisplayValue(59,0.0);
   cab->SetDisplayState(88,0);
   cab->SetDisplayState(89,0);
   cab->SetDisplayState(90,0);
  };

  if(LocoOn&1){
   cab->SetDisplayValue(1,50.0);
   cab->SetDisplayValue(2,eng->var[11]);
   cab->SetDisplayValue(3,BatteryCurrent);
   cab->SetDisplayValue(4,BatteryCurrent);
   cab->SetDisplayValue(51,50.0);
   cab->SetDisplayValue(52,eng->var[11]);
   cab->SetDisplayValue(53,BatteryCurrent);
   cab->SetDisplayValue(54,BatteryCurrent);
  }else{
   cab->SetDisplayValue(1,0.0);
   cab->SetDisplayValue(2,0.0);
   cab->SetDisplayValue(3,0.0);
   cab->SetDisplayValue(4,0.0);
   cab->SetDisplayValue(51,0.0);
   cab->SetDisplayValue(52,0.0);
   cab->SetDisplayValue(53,0.0);
   cab->SetDisplayValue(54,0.0);
  };

  val=loco->Velocity;
  if(val<0.0)val=-val;
  cab->SetDisplayValue(0,val);
  cab->SetDisplayValue(50,val);
  cab->SetDisplayValue(10,loco->MainResPressure);
  cab->SetDisplayValue(60,loco->MainResPressure);
  if(loco->BrakeCylinderPressure>loco->IndependentBrakePressure)
   val=loco->BrakeCylinderPressure;
  else
   val=loco->IndependentBrakePressure;
  cab->SetDisplayValue(11,val);
  cab->SetDisplayValue(61,val);
  cab->SetDisplayValue(12,eng->UR);
  cab->SetDisplayValue(62,eng->UR);
  cab->SetDisplayValue(13,loco->TrainPipePressure);
  cab->SetDisplayValue(63,loco->TrainPipePressure);
 };
};






