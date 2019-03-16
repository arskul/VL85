//---------------------------------------------------------------------------

#include <windows.h>
#include <math>
#include <stdio>
/*#define RTS_NODIRECTOBJLINKS*/
#include <ts.h>

#define CURRENT_Q 430.0
#define CURRENT_Q_BF 0.0025
#define TIME_THROTTLE_SWITCH1 1.0
#define TIME_THROTTLE_SWITCH2 2.0

#define BRAKE_STR_RATE 1.8
#define BRAKE_MR_RATIO    0.005
#define BRAKE_PIPE_RATE_CHARGE 1.8
#define BRAKE_UR_RATE_CHARGE   0.25
#define BRAKE_PIPE_RATE 0.5
#define BRAKE_PIPE_APPL_RATE   0.25
#define BRAKE_PIPE_EMERGENCY -1.2
#define PIPE_DISCHARGE_SLOW  -0.005

#define FORCE_SHIFT 30000.0
#define BRAKE_FORCE_SHIFT 10000.0
#define CURRENT_SHUNT_Q 1.2

//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------


extern "C" void Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState);
extern "C" void __export  LostMaster
        (Locomotive *loco,const Locomotive *Prev,unsigned long State);


//---------------------------------------------------------------------------

/*
 Stack Variables

 0 - unsigned long Flags
  1bite
   1bit - low voltage
   2bit - emergency brake incurred
   3bit - protection incurred
   4bit - compressor on
   5bit - alert started
   6bit -
   7bit - enable train pipe pressure maintenance
   8bit - init radiostation

  2bite
   1bit - convertor on
   2bit - MV on
   3bit - MV on high

 1 - throttle switch timer

 2 - set throttle position
   1 - off
   2 - S
   3 - SP
   4 - P1
   5 - P2
   6 - +1
   7 - -1
   8 - temp on
   9 - temp off
   10 - SHUNTS
   11 - start EDT
   12 - EDT+
   13 - EDT-
   14 - EDT S
   15 - EDT SP
   16 - EDT 0
   17 - stop EDT

 3 - [VACANT]

 4 - FlagsAsync
  1bite
   1bit - switch gv off
   2bit - switch gv on
   3,4bit - raise 1,2 panto
   5,6bit - lower 3,4 panto
   7bit - convertor on
   8bit - temp(position)
  2bite
   1bit - reset throttle set
   2bit - compressor on
   3bit - sanding on
   4bit - enable train pipe charge
   5bit - turn vents on
   6bit - turn vents on high
   7bit - TED 1-2 off
   8bit - TED 3-4 off
  3bite
   1bit - compressor manual on
   2bit - No S
   3bit - shutdown P2

 5 - Traction current limit

 6 - throttle lamp blink timer

 7 - shunting position

 8 - dynamic brake

 9 - previous force/brake force

 10 - resistors heat amount

 11 - EDT position

 12 - EDT current limit

 13 - previous brake force

 14 - current speed limit

 15 - EPK state
  0 - free
  1 - signal change (non-critical)
  2 - approaching red
  3 - overspeed
  4 - approaching red
  5 - approaching signal limit

 16 - EPK timer

 17 - previous signal aspect

 Sound Triggers

 4,5 - sander on/off
 14 - brake applied
 23 - BV switching
 54 - brake released
 56,57 - EPK sound on,off
 101 - Compressor start
 102 - Compressor release
 103 - BV
 104 - radio on
 105 - radio off
 106 - MV on
 107 - MV off
 108 - button press
 109 - MV high started
 110 - MV high stop
 111 - EPK release

 LocoOn
  1bite
   1bit - battery on
   2bit - KU on
   3bit - converter on
   4bit - EPK on
*/


extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 //UINT *Flags=(UINT *)&eng->var[0];
 UINT *AsyncFlags=(UINT *)&eng->var[4];
 Cabin *cab=loco->Cab();
 loco->HandbrakeValue=1.0;
 eng->HandbrakePercent=100;
 eng->DynamicBrakePercent=0;
 eng->Sanding=0;
 eng->BrakeSystemsEngaged=1;
 eng->BrakeForce=0.0;
 eng->var[0]=0;
 eng->ChargingRate=0;
 eng->TrainPipeRate=0;
 eng->var[1]=0.0;
 eng->var[2]=0.0;
 eng->UR=0.0;
 eng->var[4]=0.0;
 eng->var[5]=0.0;
 eng->var[6]=0.0;
 eng->var[7]=0.0;
 eng->var[8]=0.0;
 eng->var[9]=0.0;
 eng->var[10]=0.0;
 eng->var[11]=0.0;
 eng->var[12]=0.0;
 eng->var[13]=0.0;
 eng->var[14]=25.0;
 eng->var[15]=0.0;
 eng->var[16]=0.0;
 eng->var[17]=0.0;
 eng->AuxilaryRate=0.0;
 loco->MainResPressure=1.2;

 switch(State&0xFF){
  case 1:
   eng->UR=5.5;
   eng->HandbrakePercent=0.0;
   loco->IndependentBrakePressure=3.0;
   loco->TrainPipePressure=5.5;
   loco->AuxiliaryPressure=5.3;
   loco->BrakeCylinderPressure=0.0;
   loco->MainResPressure=7.8;
   eng->Panto=3;
   eng->MainSwitch=1;
   eng->IndependentBrakeValue=3.0;
   *AsyncFlags|=588;
   cab->SetSwitch(0,1,true);
   cab->SetSwitch(1,1,true);
   cab->SetSwitch(22,0,true);
   cab->SetSwitch(24,0,true);
   cab->SetSwitch(25,0,true);
   cab->SetSwitch(26,10,true);
   cab->SetSwitch(27,0,true);
   cab->SetSwitch(30,0,true);
   cab->SetSwitch(32,8,true);
   cab->SetSwitch(115,0,true);
   cab->SetSwitch(122,1,true);
   cab->SetSwitch(123,1,true);
   cab->SetSwitch(124,1,true);
   cab->SetSwitch(125,1,true);
   cab->SetSwitch(127,1,true);
   if(State&(1<<8)){
    cab->SetSwitch(132,1,true);
    cab->SetSwitch(116,0,true);
    cab->SetSwitch(117,0,true);
   };
  break;
  case 2:
  case 3:
   eng->UR=5.5;
   eng->HandbrakePercent=0.0;
   loco->IndependentBrakePressure=0.0;
   loco->TrainPipePressure=5.5;
   loco->AuxiliaryPressure=5.3;
   loco->BrakeCylinderPressure=0.0;
   loco->MainResPressure=9.2;
   eng->Panto=3;
   eng->MainSwitch=1;
   eng->IndependentBrakeValue=0.0;
   *AsyncFlags|=4684;
   cab->SetSwitch(0,1,true);
   cab->SetSwitch(1,1,true);
   cab->SetSwitch(22,0,true);
   cab->SetSwitch(24,0,true);
   cab->SetSwitch(25,0,true);
   cab->SetSwitch(26,10,true);
   cab->SetSwitch(27,0,true);
   cab->SetSwitch(28,0,true);
   cab->SetSwitch(30,0,true);
   cab->SetSwitch(32,8,true);
   cab->SetSwitch(115,0,true);
   
   cab->SetSwitch(122,1,true);
   cab->SetSwitch(123,1,true);
   cab->SetSwitch(124,1,true);
   cab->SetSwitch(125,1,true);
   cab->SetSwitch(127,1,true);
   if(State&(1<<8)){
    cab->SetSwitch(38,2,true);
    cab->SetSwitch(132,1,true);
    cab->SetSwitch(116,0,true);
    cab->SetSwitch(117,0,true);
   };
   cab->SetDisplayState(14,1);
  break;
 };

 return true;
};



UINT IsLocoOn(const ElectricLocomotive *loco,ULONG Flags){
 UINT res=1;
 //KU
 if(loco->Cab()->Switch(132))
  res|=2;
 //convertor
 if(Flags&256)
  res|=4;
 if(loco->Cab()->Switch(131)&&loco->Cab()->Switch(118))
  res|=8;
 return res;
};

void SwitchBV(const Locomotive *loco,ElectricEngine *eng,bool BVOn){
 if((eng->MainSwitch!=0)!=BVOn){
  eng->MainSwitch=BVOn?1:0;
  loco->PostTriggerBoth(103);
 };
};

void SwitchLights(const Locomotive *loco,Engine *eng,UINT State){
 Cabin *cab=loco->Cab();
 if(loco->LibParam==0){
  switch(State){
   case 0:
    loco->SwitchLight(0,false,0.0,0);
    loco->SwitchLight(1,false,0.0,0);
    loco->SwitchLight(6,false,0.0,0);
    loco->SwitchLight(7,false,0.0,0);
   break;
   case 1:
    loco->SwitchLight(0,true,0.0,0);
    loco->SwitchLight(1,false,0.0,0);
    loco->SwitchLight(6,true,0.0,0);
    loco->SwitchLight(7,false,0.0,0);
   break;
   case 2:
    loco->SwitchLight(0,false,0.0,0);
    loco->SwitchLight(1,true,0.0,0);
    loco->SwitchLight(6,false,0.0,0);
    loco->SwitchLight(7,true,0.0,0);
   break;
   case 3:
    loco->SwitchLight(2,false,0.0,0);
    loco->SwitchLight(14,false,0.0,0);
   break;
   case 4:
    loco->SwitchLight(2,false,0.0,0);
    loco->SwitchLight(14,true,0.0,0);
   break;
   case 5:
    loco->SwitchLight(2,true,0.0,0);
    loco->SwitchLight(14,false,0.0,0);
   break;
   case 6:
    loco->SwitchLight(3,false,0.0,0);
    loco->SwitchLight(15,false,0.0,0);
   break;
   case 7:
    loco->SwitchLight(3,false,0.0,0);
    loco->SwitchLight(15,true,0.0,0);
   break;
   case 8:
    loco->SwitchLight(3,true,0.0,0);
    loco->SwitchLight(15,false,0.0,0);
   break;
  };
 }else if(loco->LibParam==1){
  switch(State){
   case 0:
    loco->SwitchLight(2,false,0.0,0);
    loco->SwitchLight(3,false,0.0,0);
    loco->SwitchLight(4,false,0.0,0);
    loco->SwitchLight(5,false,0.0,0);
   break;
   case 1:
    loco->SwitchLight(2,true,0.0,0);
    loco->SwitchLight(3,false,0.0,0);
    loco->SwitchLight(4,true,0.0,0);
    loco->SwitchLight(5,false,0.0,0);
   break;
   case 2:
    loco->SwitchLight(2,false,0.0,0);
    loco->SwitchLight(3,true,0.0,0);
    loco->SwitchLight(4,false,0.0,0);
    loco->SwitchLight(5,true,0.0,0);
   break;
   case 3:
    loco->SwitchLight(6,false,0.0,0);
    loco->SwitchLight(8,false,0.0,0);
   break;
   case 4:
    loco->SwitchLight(6,false,0.0,0);
    loco->SwitchLight(8,true,0.0,0);
   break;
   case 5:
    loco->SwitchLight(6,true,0.0,0);
    loco->SwitchLight(8,false,0.0,0);
   break;
   case 6:
    loco->SwitchLight(7,false,0.0,0);
    loco->SwitchLight(9,false,0.0,0);
   break;
   case 7:
    loco->SwitchLight(7,false,0.0,0);
    loco->SwitchLight(9,true,0.0,0);
   break;
   case 8:
    loco->SwitchLight(7,true,0.0,0);
    loco->SwitchLight(9,false,0.0,0);
   break;
  };
 };
 switch(State){
  case 9:
   cab->SetLightState(0,0,0,-1.0);
   cab->SetLightState(1,0,0,-1.0);
  break;
  case 10:
   cab->SetLightState(0,1,0x90bbaa40,0.8);
   cab->SetLightState(1,1,0x90bbaa40,0.8);
  break;
  case 11:
   cab->SetLightState(0,1,0xaaddddcc,0.5);
   cab->SetLightState(1,1,0xaaddddcc,0.5);
  break;
  case 12:
   cab->ChangeTexture(0,0);
   cab->ChangeTexture(1,0);
   cab->ChangeTexture(2,0);
   cab->ChangeTexture(3,0);
  break;
  case 13:
   cab->ChangeTexture(0,1);
   cab->ChangeTexture(1,1);
   cab->ChangeTexture(2,1);
   cab->ChangeTexture(3,1);
  break;
 };
};



void AsyncSwitch(const ElectricLocomotive *loco,UINT UnitID,UINT Command,bool On){
 const ElectricLocomotive *unit=NULL;
 ElectricEngine *eng=(ElectricEngine *)loco->Eng();
 switch(UnitID){
  case 3:
   if(loco->NumSlaves==1)
    unit=(ElectricLocomotive *)loco->SlaveLoco(0);
   else if(loco->NumSlaves==2)
    unit=(ElectricLocomotive *)loco->SlaveLoco(1);
   else if(loco->NumSlaves>=3)
    unit=(ElectricLocomotive *)loco->SlaveLoco(2);
  break;
  case 2:
   if(loco->NumSlaves==2)
    unit=(ElectricLocomotive *)loco->SlaveLoco(0);
   else if(loco->NumSlaves>=3)
    unit=(ElectricLocomotive *)loco->SlaveLoco(1);
  break;
  case 1:
   if(loco->NumSlaves>=3)
    unit=(ElectricLocomotive *)loco->SlaveLoco(0);
  break;
  case 0:
   unit=loco;
  break;
 };
 if(unit){
  UINT &AsyncFlags=*(UINT *)&unit->Eng()->var[4];
  switch(Command){
   case 0:
    //Control SU
    if(On){
     LostMaster((ElectricLocomotive *)unit,NULL,0);
    }else{
     Switched(loco,eng,10,0);
     Switched(loco,eng,11,0);
     Switched(loco,eng,12,0);
     Switched(loco,eng,24,0);
     Switched(loco,eng,25,0);
     Switched(loco,eng,27,0);
     Switched(loco,eng,28,0);
     Switched(loco,eng,30,0);
     Switched(loco,eng,31,0);
     Switched(loco,eng,34,0);
     Switched(loco,eng,36,0);
    };
   break;
   case 1:
    //Sanding
    if(On)
     AsyncFlags|=1024;
    else
     AsyncFlags&=~1024;
   break;
   case 2:
    //Panto
    if(On)
     AsyncFlags|=48;
    else{
     Switched(loco,eng,24,0);
     Switched(loco,eng,25,0);
    };
   break;
   case 3:
    //BV
    if(On)
     AsyncFlags|=2;
   break;
   case 4:
    //turn EDT off
    if(On)
     unit->Eng()->var[2]=17.0;
   break;
   case 5:
    //BV off
    if(On)
     AsyncFlags|=1;
   break;
   case 6:
    //Heaters 1

   break;
   case 7:
    //TED1-2 off
    if(On)
     AsyncFlags|=64<<8;
    else
     AsyncFlags&=~(64<<8);
   break;
   case 8:
    //HCH lights

   break;
   case 9:
    //TED2-3 off
    if(On)
     AsyncFlags|=128<<8;
    else
     AsyncFlags&=~(128<<8);
   break;
   case 10:
    //valve heaters

   break;
   case 11:
    //compressor off
    if(!On){
     Switched(loco,eng,30,0);
    }else
     AsyncFlags&=~512;
   break;
   case 12:
    //auxiliary machines off

   break;
   case 13:
    //vents off
    if(On)
     AsyncFlags&=~(48<<8);
    else
     Switched(loco,eng,28,0);
   break;
   case 14:
    //control TS
    if(On){
     //unit->locoA->var[2]=1.0;
    }else{
     Switched(loco,eng,36,0);
    };
   break;
  };
 };
};


bool ThreeSectionOK(const ElectricLocomotive *loco,ElectricEngine *eng){
 Cabin *cab=loco->Cab();
 if(cab->Switch(34))
  return false;
 if(cab->Switch(55)||cab->Switch(111))
  return false;
 if(loco->NumSlaves>2){
  if(!cab->Switch(58)&&!cab->Switch(114))
   return false;
  if(cab->Switch(56)||cab->Switch(112))
   return false;
  if(cab->Switch(57)||cab->Switch(113))
   return false;
 }else if(loco->NumSlaves!=2){
  return false;
 }else{
  if(cab->Switch(57)||cab->Switch(113))
   return false;
  if(cab->Switch(58)||cab->Switch(114))
   return false;
 };

 return true;
};



extern "C" void __export  ChangeLoco
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 if(Prev){
  if(!Prev->Eng()->Reverse)
   if(!Prev->Cab()->Switch(132) ||
        (Prev->Cab()->Switch(55)+Prev->Cab()->Switch(58)==2))
   loco->LocoFlags|=1;
 }else
  loco->LocoFlags|=1;
};




extern "C" void __export  LostMaster
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 //UINT &Flags=*(UINT *)&loco->locoA->var[0];
 Engine *eng=loco->Eng();
 UINT &AsyncFlags=*(UINT *)&eng->var[4];
 AsyncFlags&=~14016;
 AsyncFlags&=~(2<<16);
 eng->var[11]=0.0;
 if(eng->ThrottlePosition>36)
  eng->ThrottlePosition-=9;
 //Flags&=~1272;
};



extern "C" bool __export CanWorkWith(const Locomotive *loco,const wchar_t *Type){
 if(!lstrcmpiW(Type,L"vl10k"))
  return true;

 return false;
};


extern "C" bool __export  CanSwitch(const ElectricLocomotive *loco,const ElectricEngine *eng,
        unsigned int SwitchID,unsigned int SetState)
{
 Cabin *cab=loco->Cab();
 if(SwitchID==33 || SwitchID==38){
  if(!(loco->LocoFlags&1))
   return false;
  if(eng->ThrottlePosition)
   return false;
  if(eng->var[2]!=0.0)
   return false;
  if(eng->var[11]!=0.0)
   return false;
  if(!cab->Switch(132))
   return false;
  if(SwitchID==33){
   if(cab->SwitchSet(38))
    return false;
  }else if(SwitchID==38){
   if(cab->SwitchSet(33))
    return false;
  };
  loco->PostTriggerCab(15);
 }else if(SwitchID==0){
  loco->PostTriggerCab(17);
 }else if(SwitchID==1){
  if(SetState==1)
   Switched(loco,(ElectricEngine *)eng,1,0);
  loco->PostTriggerCab(18);
 }else if(SwitchID==11){
  if(eng->ThrottlePosition)
   return false;
  if(eng->var[2]!=0.0 && eng->var[2]!=11.0 && eng->var[2]!=16.0)
   return false;
  if(eng->var[11]!=0.0)
   return false;
  loco->PostTriggerCab(26);
 }else if(SwitchID==17 || SwitchID==18){
  if(SetState)
   loco->PostTriggerCab(48);
 }else if((SwitchID>=2 && SwitchID<=16 && SwitchID!=11)||SwitchID==37){
  if(SetState)
   loco->PostTriggerCab(108);
 }else if((SwitchID>=19 && SwitchID<33) || (SwitchID>33&&SwitchID<=36) ||
        (SwitchID>=39&&SwitchID<=114) )
 {
  loco->PostTriggerCab(26);
 }else if((SwitchID>=135 && SwitchID<=138)||(SwitchID>=121 && SwitchID<=132)){
  loco->PostTriggerCab(108);
 }else if(SwitchID==119 || SwitchID==117 || SwitchID==116 || SwitchID==120){
  loco->PostTriggerCab(18);
 };

 return true;
};



extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
 UINT &Flags=*(UINT *)&eng->var[0];
 Cabin *cab=loco->Cab();
 UINT i,id;
 int rev;
 UINT BVOn=0,Panto=0;
 UINT LocoOn=IsLocoOn(loco,Flags);
 UINT ConvOn=0,SetPos=0;
 bool ThrottleReset=false,NoS=false;
 FreeAnimation *anim;

 if(SwitchID>=55 && SwitchID<=114){
  if(LocoOn&2){
   SetPos=(SwitchID-55)/4;
   BVOn=SwitchID-55-SetPos*4;
   if(!SetPos || !cab->Switch(55+BVOn))
    AsyncSwitch(loco,BVOn,SetPos,cab->Switch(SwitchID));
  };
  return;
 };

 switch(SwitchID){
  case 33:
   if(cab->Switch(33)<2)
    rev=0;
   else{
    if(loco->LibParam==1)
     rev=1;
    else
     rev=-1;
   };
   if(!cab->Switch(55))
    eng->Reverse=rev;
  break;
  case 38:
   if(cab->Switch(38)<2)
    rev=0;
   else{
    if(loco->LibParam==1)
     rev=-1;
    else
     rev=1;
   };
   if(!cab->Switch(55))
    eng->Reverse=rev;
  break;
  case 22:
   if(loco->LocoFlags&1)
    if(LocoOn&2)
     if(cab->Switch(22))
      BVOn=2;
   if(BVOn==2)
    if(!cab->Switch(55)){
     //Switching gv off
     UINT &AsyncFlags=*(UINT *)&eng->var[4];
     AsyncFlags|=1;
    };
  break;
  case 23:
   if(loco->LocoFlags&1)
    if(LocoOn&2)
     if(cab->Switch(23))
      if(!cab->Switch(22))
       BVOn=1;
   if(BVOn==1)
    if(!cab->Switch(55)&&!cab->Switch(75)){
     //Switching gv on
      UINT &AsyncFlags=*(UINT *)&eng->var[4];
     AsyncFlags|=2;
    };
  break;
  case 24:
  case 25:
   //Panto
   if(loco->LocoFlags&1)
    if(LocoOn&2){
     if(SwitchID==24){
      if(!cab->Switch(24))
       Panto=1;
      else
       Panto=3;
     }else{
      if(!cab->Switch(25))
       Panto=2;
      else
       Panto=4;
     };
    };
   if(Panto){
    if((loco->LibParam==1) ^ (loco->Flags&1)){
     if(Panto==1 || Panto==3)
      Panto++;
     else
      Panto--;
    };
    UINT &AsyncFlags=*(UINT *)&eng->var[4];
    if(!cab->Switch(55)&&!cab->Switch(63))
     AsyncFlags|=1<<(Panto+1);
   };
  break;
  case 1:
   //if(eng->sound)
    //eng->sound->PostTrigger(17);
   //Loco brake
   if(cab->Switch(117)){
    return;
   };
   switch(cab->Switch(1)){
    case 0:
     eng->IndependentBrakeValue=0.0;
    break;
    case 1:
     eng->IndependentBrakeValue=loco->IndependentBrakePressure;
    break;
    case 2:
     if(eng->IndependentBrakeValue<1.0)
      eng->IndependentBrakeValue=1.0;
    break;
    case 3:
     if(eng->IndependentBrakeValue<2.0)
      eng->IndependentBrakeValue=2.0;
    break;
    case 4:
     if(eng->IndependentBrakeValue<3.0)
      eng->IndependentBrakeValue=3.0;
    break;
    case 5:
     if(eng->IndependentBrakeValue<4.0)
      eng->IndependentBrakeValue=4.0;
    break;
   };
  break;
  case 117:
   if(cab->Switch(117)){
    eng->IndependentBrakeValue=loco->IndependentBrakePressure;
    return;
   }else
    Switched(loco,eng,1,0);
  break;
  case 27:
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(27))
     ConvOn=1;
    else
     ConvOn=2;
   };
   if(ConvOn)
    if(!cab->Switch(55)){
     UINT &AsyncFlags=*(UINT *)&eng->var[4];
     if(ConvOn==1)
      AsyncFlags|=64;
     else
      AsyncFlags&=~64;
    };
  break;
  case 10:
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(10))
     SetPos=8;
    else
     SetPos=9;
    ThrottleReset=true;
   };
   if(SetPos){
    if(!cab->Switch(55) && !cab->Switch(111)){
     UINT &AsyncFlags=*(UINT *)&eng->var[4];
     if(SetPos==8)
      AsyncFlags|=128;
     else
      AsyncFlags&=~128;
     if(ThrottleReset)
      AsyncFlags|=256;
    };
   };
  break;
  case 3:
   //S
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     if(cab->Switch(36)){
      if(cab->Switch(3))
       SetPos=2;
     }else{
      NoS=true;
      if(cab->Switch(3) && !cab->Switch(111))
       SetPos=3;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55)){
     eng->var[2]=SetPos;
    };
   };
  break;
  case 4:
   //SP
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     if(cab->Switch(36)){
      if(cab->Switch(4))
       SetPos=3;
     }else{
      NoS=true;
      if(cab->Switch(4))
       SetPos=4;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55) && !cab->Switch(111)){
     if(NoS){
      if(eng->ThrottlePosition>16)
       eng->var[2]=SetPos;
     }else{
      if(eng->ThrottlePosition>0)
       eng->var[2]=SetPos;
     };
    };
   };
  break;
  case 5:
   //P1
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     if(cab->Switch(36)){
      if(cab->Switch(5))
       SetPos=4;
     }else{
      NoS=true;
      if(cab->Switch(5))
       SetPos=5;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55) && !cab->Switch(111)){
     if(NoS){
      if(eng->ThrottlePosition>27)
       eng->var[2]=SetPos;
     }else{
      if(eng->ThrottlePosition>16)
       eng->var[2]=SetPos;
     };
    };
   };
  break;
  case 6:
   //P2
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     if(cab->Switch(36)){
      if(cab->Switch(6) && ThreeSectionOK(loco,eng))
       SetPos=5;
     }else{
      NoS=true;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55) && !cab->Switch(111)){
     if(eng->ThrottlePosition>27)
      eng->var[2]=SetPos;
    };
   };
  break;
  case 7:
   //-
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(7)){
     if(!cab->Switch(11)){
      SetPos=7;
      ThrottleReset=true;
     }else{
      SetPos=13;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55)){
     if(SetPos==7){
      if(!cab->Switch(111)){
       if(eng->ThrottlePosition!=17 &&   //eng->ThrottlePosition &&
          eng->ThrottlePosition!=28 && eng->ThrottlePosition!=38 &&
          !eng->var[2])
       {
        eng->var[2]=SetPos;
       };
       UINT &AsyncFlags=*(UINT *)&eng->var[4];
       if(ThrottleReset)
        AsyncFlags|=256;
      };
     }else if(SetPos==13){
      if(!cab->Switch(71))
       if(eng->var[11]>0.0 && eng->var[11]!=1.0 && eng->var[11]!=16.0)
        eng->var[2]=SetPos;
     };
    };
   };
  break;
  case 8:
   //+
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(8)){
     if(!cab->Switch(11)){
      SetPos=6;
      ThrottleReset=true;
     }else{
      SetPos=12;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55)){
     if(SetPos==6){
      if(!cab->Switch(111)){
       UINT &AsyncFlags=*(UINT *)&eng->var[4];
       if(((eng->ThrottlePosition!=16 && eng->ThrottlePosition!=27 &&
          eng->ThrottlePosition!=36 && eng->ThrottlePosition!=45) ||
         eng->var[7]>0.0) && !eng->var[2])
        eng->var[2]=SetPos;
       if(ThrottleReset)
        AsyncFlags|=256;
      };
     }else if(SetPos==12){
      if(!cab->Switch(71)){
       if(eng->var[11]>0.0 && eng->var[11]!=15.0 && eng->var[11]!=30.0)
        eng->var[2]=SetPos;
      };
     };
    };
   };
  break;
  case 9:
   //SH
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     if(cab->Switch(9)){
      SetPos=10;
      ThrottleReset=true;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55) && !cab->Switch(111)){
     UINT &AsyncFlags=*(UINT *)&eng->var[4];
     if((eng->ThrottlePosition==16 || eng->ThrottlePosition==27 ||
        eng->ThrottlePosition==36 || eng->ThrottlePosition==45) &&
      !eng->var[2])
       eng->var[2]=SetPos;
     if(ThrottleReset)
      AsyncFlags|=256;
    };
   };
  break;
  case 2:
   //0
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(2)){
     if(!cab->Switch(11)){
      SetPos=1;
     }else{
      SetPos=16;
     };
    };
   };
   if(SetPos){
    if(!cab->Switch(55)){
     if(SetPos==1){
      if(!cab->Switch(111))
       eng->var[2]=SetPos;
     }else{
      if(!cab->Switch(71))
       eng->var[2]=SetPos;
     };
    };
   };
  break;
  case 36:
   //No S
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(36)){
     NoS=true;
    };
   };
   if(!cab->Switch(55)&&!cab->Switch(111)){
    UINT &AsyncFlags=*(UINT *)&eng->var[4];
    if(NoS)
     AsyncFlags|=2<<16;
    else
     AsyncFlags&=~(2<<16);
   };
  break;
  case 30:
  case 31:
   //Compressor
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(30))
     BVOn=1;
    else
     BVOn=2;
    if(cab->Switch(31))
     ConvOn=1;
    else
     ConvOn=2;
   };
   if(BVOn || ConvOn){
    if(!cab->Switch(55) && !cab->Switch(99)){
     UINT &AsyncFlags=*(UINT *)&eng->var[4];
     if(BVOn==1)
      AsyncFlags|=512;
     else if(BVOn==2)
      AsyncFlags&=~512;
     if(ConvOn==1)
      AsyncFlags|=1<<16;
     else if(ConvOn==2)
      AsyncFlags&=~(1<<16);
    };
   };
  break;
  case 13:
   if((loco->LocoFlags&1)&&(LocoOn&2)&&cab->Switch(13)){
    loco->PostTriggerBoth(8);
   }else{
    loco->PostTriggerBoth(9);
   };
  break;
  case 14:
   if((loco->LocoFlags&1)&&(LocoOn&2)&&cab->Switch(14)){
    loco->PostTriggerBoth(10);
   }else{
    loco->PostTriggerBoth(11);
   };
  break;
  case 12:
   //Sanding
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(12))
     BVOn=1;
    else
     BVOn=2;
   };
   if(BVOn){
    UINT &AsyncFlags=*(UINT *)&eng->var[4];
    if((BVOn==1 || cab->Switch(59)) && !cab->Switch(55))
     AsyncFlags|=1024;
    else
     AsyncFlags&=~1024;
   };
  break;
  case 15:
   //Release brake
   if(LocoOn&2){
    if(cab->Switch(15)){
     eng->EngineFlags|=1;
     eng->IndependentBrakeValue=0.0;
    };
   };
  break;
  case 42:
  case 123:
  case 50:
   //front light left
   if(cab->Switch(123) && !cab->Switch(42)){
    if(!cab->Switch(50))
     SwitchLights(loco,eng,5);
    else
     SwitchLights(loco,eng,4);
   }else{
    SwitchLights(loco,eng,3);
   };
  break;
  case 43:
  case 124:
  case 51:
   //front light right
   if(cab->Switch(124) && !cab->Switch(43)){
    if(!cab->Switch(51))
     SwitchLights(loco,eng,8);
    else
     SwitchLights(loco,eng,7);
   }else{
    SwitchLights(loco,eng,6);
   };
  break;
  case 44:
  case 125:
  case 52:
   //cabin light
   if(cab->Switch(125) && !cab->Switch(44)){
    if(cab->Switch(52))
     SwitchLights(loco,eng,11);
    else
     SwitchLights(loco,eng,10);
   }else{
    SwitchLights(loco,eng,9);
   };
  break;
  case 137:
  case 138:
   //headlight
   if(cab->Switch(137)){
    if(!cab->Switch(138))
     SwitchLights(loco,eng,2);
    else
     SwitchLights(loco,eng,1);
   }else{
    SwitchLights(loco,eng,0);
   };
  break;
  case 135:
   if(cab->Switch(135))
    cab->SetDisplayState(14,1);
  break;
  case 136:
   if(cab->Switch(136))
    cab->SetDisplayState(14,0);
  break;
  case 28:
  case 29:
   //Vents
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(29))
     BVOn=2;
    else if(!cab->Switch(28))
     BVOn=1;
    else
     BVOn=3;
   };
   if(BVOn){
    UINT &AsyncFlags=*(UINT *)&eng->var[4];
    if(!cab->Switch(55)&&!cab->Switch(107)){
     AsyncFlags&=~12288;
     if(BVOn==2)
      AsyncFlags|=8192;
     else if(BVOn==1)
      AsyncFlags|=4096;
    };
   };
  break;
  case 45:
  case 127:
   //equipment lighting
   if(!cab->Switch(45) && cab->Switch(127)){
    SwitchLights(loco,eng,13);
   }else{
    SwitchLights(loco,eng,12);
   };
  break;
  case 39:
   //left wiper
   BVOn=cab->Switch(39);
   anim=cab->FindAnim(L"wiperleft");
   if(anim){
    if(BVOn)
     anim->Flags&=~1;
    else
     anim->Flags|=1;
   };
   anim=loco->FindAnim(L"wiperleft");
   if(anim){
    if(BVOn)
     anim->Flags&=~1;
    else
     anim->Flags|=1;
   };
  break;
  case 40:
   //right wiper
   BVOn=cab->Switch(40);
   anim=cab->FindAnim(L"wiperright");
   if(anim){
    if(BVOn)
     anim->Flags&=~1;
    else
     anim->Flags|=1;
   };
   anim=loco->FindAnim(L"wiperright");
   if(anim){
    if(BVOn)
     anim->Flags&=~1;
    else
     anim->Flags|=1;
   };
  break;
  case 11:
   //Throttle/brake switch
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(!cab->Switch(11)){
     SetPos=17;
    }else{
     SetPos=11;
    };
   };
   if(SetPos){
    if(!cab->Switch(55)&&!cab->Switch(71)){
     eng->var[2]=SetPos;
    };
   };
  break;
  case 37:
   //S rec
   if((loco->LocoFlags&1)&&(LocoOn&2)){
    if(cab->Switch(37)&&cab->Switch(11)){
     SetPos=14;
    };
   };
   if(SetPos){
    if(!cab->Switch(55)&&!cab->Switch(71)){
     if(eng->var[11]<16.0){
      if(eng->var[2]==11.0)
       eng->var[1]-=3.0;
      eng->var[2]=SetPos;
     };
    };
   };
  break;
  case 17:
  case 18:
   eng->var[15]=0.0;
   eng->var[16]=0.0;
  break;

 };

 //Distributing event to slave locos
 if(SwitchID==33 || SwitchID==38 || SwitchID==37 || SwitchID==36 ||
    (SwitchID>=22 && SwitchID<=25) ||
        SwitchID==1 || (SwitchID>=27 && SwitchID<=31) ||
         (SwitchID==15 && cab->Switch(15) && (LocoOn&2)) ||
        (SwitchID>=2 && SwitchID<=12))
 {
  Locomotive *slave; UINT PantoR;
  Engine *slaveeng;
  for(UINT i=0;i<loco->NumSlaves;i++){
   if(i>3)
    break;
   switch(loco->NumSlaves){
    case 1: id=3; break;
    case 2: id=i+2; break;
    default: id=i+1; break;
   };
   slave=loco->SlaveLoco(i);
   slaveeng=slave->Eng();
   UINT &AsyncFlags=*(UINT *)&slaveeng->var[4];
   switch(SwitchID){
    case 33:
    case 38:
     if(!cab->Switch(55+id))
      slaveeng->Reverse=rev;
    break;
    case 22:
    case 23:
     if(!cab->Switch(55+id)&&!cab->Switch(75+id)){
      if(BVOn==1)
       AsyncFlags|=2;
      else if(BVOn==2)
       AsyncFlags|=1;
     }else{
      AsyncFlags|=1;
     };
    break;
    case 24:
    case 25:
     if(!cab->Switch(55+id)&&!cab->Switch(63+id)){
      if(Panto){
       PantoR=Panto;
       if(slave->Flags&1){
        if(PantoR==1 || PantoR==3)
         PantoR++;
        else
         PantoR--;
       };
       AsyncFlags|=1<<(PantoR+1);
      };
     };
    break;
    case 1:
     if(!cab->Switch(117))
      slaveeng->IndependentBrakeValue=eng->IndependentBrakeValue;
     else if(slave->Cab()->Switch(117))
      eng->IndependentBrakeValue=slaveeng->IndependentBrakeValue;
    break;
    case 27:
     if(!cab->Switch(55+id)){
      if(ConvOn==1)
       AsyncFlags|=64;
      else if(ConvOn==2)
       AsyncFlags&=~64;
     };
    break;
    case 10:
     if(!cab->Switch(55+id)){
      if(SetPos==8)
       AsyncFlags|=128;
      else if(SetPos==9)
       AsyncFlags&=~128;
      if(ThrottleReset)
       AsyncFlags|=256;
     };
    break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 11:
    case 37:
     if(!cab->Switch(55+id)){
      if(SetPos){
       if(SetPos>=1 && SetPos<11){
        if(cab->Switch(111+id))
         break;
       }else if(SetPos>=11 && SetPos<=17){
        if(cab->Switch(71+id))
         break;
       };
       switch(SetPos){
        case 3:
         if(slaveeng->ThrottlePosition>0 || NoS)
          slaveeng->var[2]=SetPos;
        break;
        case 4:
         if(slaveeng->ThrottlePosition>16)
          slaveeng->var[2]=SetPos;
        break;
        case 5:
         if(slaveeng->ThrottlePosition>27)
          slaveeng->var[2]=SetPos;
        break;
        case 6:
         if(((slaveeng->ThrottlePosition!=16 && slaveeng->ThrottlePosition!=27 &&
              slaveeng->ThrottlePosition!=36) || slaveeng->var[7]>0.0) &&
            !slaveeng->var[2])
          slaveeng->var[2]=SetPos;
        break;
        case 7:
         if(slaveeng->ThrottlePosition!=17 && slaveeng->ThrottlePosition!=28
           && slaveeng->ThrottlePosition!=37 && !slaveeng->var[2])
          slaveeng->var[2]=SetPos;
        break;
        case 10:
         if((slaveeng->ThrottlePosition==16 || slaveeng->ThrottlePosition==27 ||
             slaveeng->ThrottlePosition==36 || slaveeng->ThrottlePosition==45 )
           && !slaveeng->var[2])
          slaveeng->var[2]=SetPos;
        break;
        case 12:
         if(slaveeng->var[11]>0.0 && slaveeng->var[11]!=15.0 && slaveeng->var[11]!=30.0)
          slaveeng->var[2]=SetPos;
        break;
        case 13:
         if(slaveeng->var[11]>0.0 && slaveeng->var[11]!=1.0 && slaveeng->var[11]!=16.0)
          slaveeng->var[2]=SetPos;
        break;
        case 14:
         if(slaveeng->var[11]<16.0){
          if(slaveeng->var[2]==11.0)
           slaveeng->var[1]-=2.0;
          slaveeng->var[2]=SetPos;
         };
        break;
        default:
         slaveeng->var[2]=SetPos;
        break;
       };
      };
      if(ThrottleReset)
       AsyncFlags|=256;
     };
    break;
    case 30:
    case 31:
     if(!cab->Switch(55+id) && !cab->Switch(99+id)){
      if(BVOn==1)
       AsyncFlags|=512;
      else if(BVOn==2)
       AsyncFlags&=~512;
      if(ConvOn==1)
       AsyncFlags|=1<<16;
      else if(ConvOn==2)
       AsyncFlags&=~(1<<16);
     };
    break;
    case 12:
     if(!cab->Switch(55+id)){
      if(BVOn==1 || cab->Switch(59+id))
       AsyncFlags|=1024;
      else if(BVOn==2)
       AsyncFlags&=~1024;
     };
    break;
    case 15:
     slaveeng->IndependentBrakeValue=0.0;
     slaveeng->EngineFlags|=1;
    break;
    case 28:
    case 29:
     if(BVOn){
      if(!cab->Switch(55+id)&&!cab->Switch(107+id)){
       AsyncFlags&=~12288;
       if(BVOn==2)
        AsyncFlags|=8192;
       else if(BVOn==1)
        AsyncFlags|=4096;
      };
     };
    break;
    case 36:
     if(!cab->Switch(55+id)&&!cab->Switch(111+id)){
      if(NoS)
       AsyncFlags|=2<<16;
      else
      AsyncFlags&=~(2<<16);
     };
    break;
   };
  };
 };
};




UINT ApproachRed(Engine *eng,float SigDist,float Vel){
 UINT res=0;
     if(SigDist<500.0){
      if(Vel>5.55)
       res=1;
      if(SigDist<300.0){
       if(Vel>5.55)
        res=2;
       if(SigDist<250.0){
        if(Vel>4.0 && res<1)
         res=1;
        if(SigDist<150.0){
         if(Vel>4.0)
          res=2;
         if(SigDist<100.0){
          if(Vel>2.7 && res<1)
           res=1;
          if(SigDist<100.0){
           if(Vel>2.7)
            res=2;
           else if(Vel>1.4 && res<1)
            res=1;
           if(SigDist<20.0)
            if(Vel>1.4)
             res=2;
          };
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
 if(!(loco->LocoFlags&1))
  return;

 Engine *eng=loco->Eng();
 //unsigned long &Flags=*(unsigned long *)&eng->var[0];

 float Limit=GetLimit(Route.Limit,Signal.Limit,Event.Limit);
 //float Vel=fabs(loco->Velocity);
 eng->var[14]=Limit*3.6;
 if(Limit<6.945){
  Limit=6.945;
  eng->var[14]=25.0;
 };


/*
  if(Asp==SIGASP_STOP_AND_PROCEED){
    if((Signal.Distance<50.0 && Vel>3.0) || (Signal.Distance<250.0 && Vel>6.0)){
     eng->var[16]=76;
     Flags|=32;
    }else if(eng->var[11]<2.0){
     if((Vel>3.0 && Signal.Distance<150.0) ||(Vel>6.0 && Signal.Distance<350.0)){
      eng->var[11]=3.0;
      eng->var[10]=20;
     };
    };
   };
  if(Limit>0.0){
   if(Vel>Limit && eng->var[11]<2.0){
    eng->var[11]=3.0;
    eng->var[10]=30;
   };
   if(Vel>Limit+4){
    if(Signal.Limit==Limit){
     if(Vel>Limit+7){
      eng->var[10]=76;
      Flags|=32;
     };
    }else{
     eng->var[10]=76;
     Flags|=32;
    };
   };
  };
  if(Vel>ER9T_MAX_VELOCITY){
   eng->var[10]=76;
   Flags|=32;
  };
 };*/
};


bool SwitchPosition(ElectricEngine *eng,bool Fast,bool NoS,UINT SetPos,float Current){
 if(SetPos>eng->ThrottlePosition){
  if(Current<eng->var[5]){
   if(Fast){
    eng->ThrottlePosition+=3;
    if(eng->ThrottlePosition>SetPos)
     eng->ThrottlePosition=SetPos;
   }else
    eng->ThrottlePosition++;
   if(NoS){
    if(eng->ThrottlePosition<=16)
     eng->ThrottlePosition=17;
   };
   return true;
  };
 }else if(SetPos<eng->ThrottlePosition){
  if(Fast){
   if(eng->ThrottlePosition-SetPos>3)
    eng->ThrottlePosition-=3;
   else
    eng->ThrottlePosition--;
  }else
   eng->ThrottlePosition--;
  if(NoS){
   if(eng->ThrottlePosition<=16)
    eng->ThrottlePosition=0;
  };
  return true;
 };
 return false;
};

void WriteThrottlePosition(wchar_t *buf,Engine *eng){
 buf[0]=0;
 int Pos=eng->ThrottlePosition;
 if(Pos>0){
  if(Pos>=1 && Pos<=16){
   swprintf(buf,L"Ñ %02i",Pos);
  }else if(Pos>=17 && Pos<=27){
   Pos-=16;
   swprintf(buf,L"ÑÏ %02i",Pos);
  }else if(Pos>=28 && Pos<=36){
   Pos-=27;
   swprintf(buf,L"Ï1 %02i",Pos);
  }else if(Pos>=37 && Pos<=45){
   Pos-=36;
   swprintf(buf,L"Ï2 %02i",Pos);
  };
  Pos=eng->ThrottlePosition;
  if(Pos==16 || Pos==27 ||Pos==36 || Pos==45 ){
   switch(int(eng->var[7])){
    case 1: lstrcatW(buf,L"\nÎÏ1"); break;
    case 2: lstrcatW(buf,L"\nÎÏ2"); break;
    case 3: lstrcatW(buf,L"\nÎÏ3"); break;
    case 4: lstrcatW(buf,L"\nÎÏ4"); break;
    default: lstrcatW(buf,L"\nÕÏ"); break;
   };
  };
 }else if(eng->var[11]>0.0){
  Pos=eng->var[11];
  if(Pos>=1 && Pos<=15){
   swprintf(buf,L"Ò2 %02i",Pos);
  }else if(Pos>=16 && Pos<=30){
   Pos-=15;
   swprintf(buf,L"Ò1 %02i",Pos);
  };
 };
};


extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 wchar_t buf[32];
 UINT &Flags=*(UINT *)&eng->var[0];
 UINT &AsyncFlags=*(UINT *)&eng->var[4];
 UINT LocoOn=IsLocoOn(loco,Flags);
 Cabin *cab=loco->Cab();
 SMSObject *soundExt=loco->SoundEng(),
           *soundCab=loco->SoundCab();
 bool CurrentOn=false,UpdateSlaves=false,AllowPipeCharging=false;

 eng->MainResRate=0.0;

 //Pantographs
 if(AsyncFlags&48){
  eng->Panto&=~((AsyncFlags&48)>>4);
  AsyncFlags&=~((AsyncFlags&48)>>2);
  AsyncFlags&=~48;
 };
 eng->Panto|=(AsyncFlags>>2)&3;

 //BV
 if((AsyncFlags&2)&&(!eng->Reverse||!eng->ThrottlePosition)){
  //switching BV on
  SwitchBV(loco,eng,true);
  AsyncFlags&=~2;
  Flags&=~4;
 };
 if(AsyncFlags&1){
  //switching BV off
  SwitchBV(loco,eng,false);
  AsyncFlags&=~1;
 };

 Flags|=1;
 if(loco->LineVoltage && eng->MainSwitch){
  if(loco->LineVoltage>2000.0 && loco->LineVoltage<4500.0 && !loco->LineFreq){
   CurrentOn=true;
  }else{
   //Switching gv off
   SwitchBV(loco,eng,false);
  };
  if(loco->LineVoltage>2500.0)
   Flags&=~1;
 };

 //Double-checking three-section P2
 if((eng->ThrottlePosition>36)&&(loco->LocoFlags&1)){
  if(!ThreeSectionOK(loco,eng)){
   for(UINT i=0;i<loco->NumSlaves;i++){
    UINT &SlaveAsyncFlags=*(UINT *)&loco->SlaveLoco(i)->Eng()->var[4];
    SlaveAsyncFlags|=4<<16;
   };
   if(eng->ThrottlePosition>36)
    eng->ThrottlePosition-=9;
  };
 }else if(AsyncFlags&(4<<16)){
  if(eng->ThrottlePosition>36)
   eng->ThrottlePosition-=9;
  AsyncFlags&=~(4<<16);
 };
 

 //convertor
 if(AsyncFlags&64)
  Flags|=256;
 else
  Flags&=~256;

 //Compressor
 if(CurrentOn && (AsyncFlags&512)){
  if(!(Flags&8)){
   if((loco->MainResPressure<6.0)||(AsyncFlags&(1<<16))){
    Flags|=8;
    loco->PostTriggerBoth(101);
   };
  }else{
   if(loco->MainResPressure>9.1 && !(AsyncFlags&(1<<16))){
    Flags&=~8;
    loco->PostTriggerBoth(102);
   };
  };
 }else if(Flags&8){
  Flags&=~8;
  loco->PostTriggerBoth(102);
 };
 if(Flags&8){
  eng->MainResRate=(12.1-loco->MainResPressure)*0.015;
 };

 //Sanding
 if((AsyncFlags&1024)&&loco->SandLeft){
  if(!eng->Sanding){
   eng->Sanding=1;
   loco->PostTriggerBoth(4);
  };
 }else{
  if(eng->Sanding){
   eng->Sanding=0;
   loco->PostTriggerBoth(5);
  };
 };

 //Vents
 float Var1=0.0;
 if((AsyncFlags&12288)&&CurrentOn){
  if(!(Flags&512)){
   if(AsyncFlags&8192){
    Flags|=1536;
    loco->PostTriggerBoth(106);
    loco->PostTriggerBoth(109);
   }else{
    Flags|=512;
    loco->PostTriggerBoth(106);
   };
  }else{
   if((AsyncFlags&8192)&&!(Flags&1024)){
    Flags|=1024;
    loco->PostTriggerBoth(109);
   }else if(!(AsyncFlags&8192)&&(Flags&1024)){
    Flags&=~1024;
    loco->PostTriggerBoth(110);
   };
   Var1=fabs(eng->Force)/5000.0;
   if(Flags&1024){
    Var1+=100.0;
   }else if(eng->Reverse){
    if(eng->ThrottlePosition>0 && eng->ThrottlePosition<16)
     Var1*=3.0+(16-eng->ThrottlePosition)*0.1125;
    else if(eng->ThrottlePosition>16 && eng->ThrottlePosition<27)
     Var1*=3.0+(27-eng->ThrottlePosition)*0.3;
    else if(eng->ThrottlePosition>27 && eng->ThrottlePosition<36)
     Var1*=3.0+(36-eng->ThrottlePosition)*0.31;
    else if(eng->var[11]>0.0 && eng->var[2]!=17.0)
     Var1=fabs(eng->BrakeForce)/1500.0;
   };
  };
 }else{
  if(Flags&1024){
   Flags&=~1536;
   loco->PostTriggerBoth(110);
   loco->PostTriggerBoth(107);
  }else if(Flags&512){
   Flags&=~512;
   loco->PostTriggerBoth(107);
  };
 };
 if(soundCab){
  float Var1p=soundCab->Var1[0];
  if(Var1p<Var1){
   Var1p+=25.0*time;
   if(Var1p<Var1)
    Var1=Var1p;
  }else if(Var1p>Var1){
   Var1p-=25.0*time;
   if(Var1p>Var1)
    Var1=Var1p;
  };
  soundCab->Var1[0]=Var1;
 };
 if(soundExt)
  soundExt->Var1[0]=Var1;

 //Throttle position set
 UINT SetPos=eng->var[2];
 bool SpeedCutoff=false;
 if(SetPos){
  bool TempOn=(AsyncFlags&128)==128;
  bool NoS =(AsyncFlags&(2<<16));
  const float SwitchTime=TempOn?TIME_THROTTLE_SWITCH2:TIME_THROTTLE_SWITCH1;
  const float BrakeSwitchTime=1.0;
  float Current=eng->Force/CURRENT_Q;
  if(loco->NumEngines>=4){
   if(eng->EngineCurrent[0]>eng->EngineCurrent[2])
    Current=eng->EngineCurrent[0];
   else
    Current=eng->EngineCurrent[2];
  }else{
   if(eng->var[7]>0.0)
    Current*=CURRENT_SHUNT_Q;
  };
  eng->var[1]+=time;
  switch(SetPos){
   case 1: //0
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(TempOn){
     SpeedCutoff=true;
     if(eng->var[1]>=SwitchTime*2.0)
      eng->ThrottlePosition=0;
    }else{
     if(eng->ThrottlePosition && eng->var[1]>=SwitchTime){
      SwitchPosition(eng,TempOn,NoS,0,Current);
      eng->var[1]-=SwitchTime;
      /*if(TempOn){
       if(eng->ThrottlePosition<=3)
        eng->ThrottlePosition=0;
       else
        eng->ThrottlePosition-=3;
      }else
       eng->ThrottlePosition--;*/
     };
    };
    if((!eng->ThrottlePosition)||(AsyncFlags&256)){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 2: //S
    if(eng->var[11]>0.0 || NoS){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(eng->ThrottlePosition==0)
     SetPos=1;
    else if(eng->ThrottlePosition>16)
     SetPos=16;
    else
     SetPos=16;
    if(eng->var[1]>=SwitchTime){
     if(SwitchPosition(eng,TempOn,NoS,SetPos,Current))
      eng->var[1]=0.0;
    };
    if((eng->ThrottlePosition==SetPos)||(AsyncFlags&256)){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 3: //SP
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(eng->ThrottlePosition<17)
     SetPos=17;
    else if(eng->ThrottlePosition>27)
     SetPos=27;
    else
     SetPos=27;
    if(eng->var[1]>=SwitchTime){
     if(SwitchPosition(eng,TempOn,NoS,SetPos,Current))
      eng->var[1]=0.0;
    };
    if((eng->ThrottlePosition==SetPos)||(AsyncFlags&256)){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 4: //P1
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(eng->ThrottlePosition<28)
     SetPos=28;
    else
     SetPos=36;
    if(eng->var[1]>=SwitchTime){
     if(SwitchPosition(eng,TempOn,NoS,SetPos,Current))
      eng->var[1]=0.0;
    };
    if((eng->ThrottlePosition==SetPos)||(AsyncFlags&256)){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 5: //P2
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(eng->ThrottlePosition<37)
     SetPos=37;
    else
     SetPos=45;
    if(eng->var[1]>=SwitchTime){
     if(SwitchPosition(eng,TempOn,NoS,SetPos,Current))
      eng->var[1]=0.0;
    };
    if((eng->ThrottlePosition==SetPos)||(AsyncFlags&256)){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 6: //+1
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    if(eng->var[7]>0.0){
     eng->var[7]=int(eng->var[7])-1;
     eng->var[1]=0.0;
     eng->var[2]=0.0;
     break;
    };
    /*if(SetPos>45){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
     break;
    };*/
    SetPos=eng->ThrottlePosition+1;
    if(eng->var[1]>=SwitchTime){
     SwitchPosition(eng,TempOn,NoS,SetPos,Current);
    };
    if(eng->ThrottlePosition==SetPos){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 7: //-1
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    eng->var[7]=0.0;
    if(!eng->ThrottlePosition)
     SetPos=0;
    else
     SetPos=eng->ThrottlePosition-1;
    if(eng->var[1]>=SwitchTime){
     SwitchPosition(eng,TempOn,NoS,SetPos,Current);
    };
    if(eng->ThrottlePosition==SetPos){
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 10: //SH
    if(eng->var[11]>0.0){
     eng->var[2]=0.0;
     break;
    };
    if(Current<=eng->var[5]){
     SetPos=int(eng->var[7])+1;
     if(SetPos>4)
      SetPos=4;
     eng->var[7]=SetPos;
    };
    eng->var[1]=0.0;
    eng->var[2]=0.0;
   break;
   case 11: //start EDT
    if(eng->ThrottlePosition){
     eng->var[7]=0.0;
     if(eng->var[1]>=SwitchTime){
      SwitchPosition(eng,TempOn,NoS,0,Current);
      eng->var[1]-=SwitchTime;
     };
     break;
    };
    if(eng->var[1]>=4.0 || eng->var[11]>0.0){
     eng->var[11]=1.0;
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 17: //stop EDT
    if(eng->var[1]>=BrakeSwitchTime){
     eng->var[11]=0.0;
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 12: //EDT +
    if(eng->var[1]>=BrakeSwitchTime && Current<eng->var[12]){
     if(TempOn){
      eng->var[11]+=3.0;
      if(eng->var[11]>30.0)
       eng->var[11]=30.0;
      else if(eng->var[11]==16.0 || eng->var[11]==17.0 || eng->var[11]==18.0)
       eng->var[11]=15.0;
     }else{
      if(eng->var[11]!=15.0 && eng->var[11]!=30.0)
       eng->var[11]+=1.0;
     };
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 13: //EDT -
    if(eng->var[1]>=BrakeSwitchTime){
     if(TempOn){
      eng->var[11]-=3.0;
      if(eng->var[11]<1.0)
       eng->var[11]=1.0;
      else if(eng->var[11]==15.0 || eng->var[11]==14.0 || eng->var[11]==13.0)
       eng->var[11]=16.0;
     }else{
      if(eng->var[11]!=16.0 && eng->var[11]!=1.0)
       eng->var[11]-=1.0;
     };
     eng->var[1]=0.0;
     eng->var[2]=0.0; 
    };
   break;
   case 14: //Srec
    if(eng->var[1]>=2.0 && Current<eng->var[12]){
     eng->var[11]=16.0;
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;
   case 16: //EDT 0
    if(eng->var[1]>=4.0){
     eng->var[11]=0.0;
     eng->var[1]=0.0;
     eng->var[2]=0.0;
    };
   break;

  };
 };
 AsyncFlags&=~256;

 //Interconnection type (S,SP,P)
 UINT Intercon=0;
 if(eng->ThrottlePosition>=1 && eng->ThrottlePosition<=16)
  Intercon=1;
 else if(eng->ThrottlePosition>=17 && eng->ThrottlePosition<=27)
  Intercon=2;
 else if(eng->ThrottlePosition>=28 && eng->ThrottlePosition<=36)
  Intercon=3;
 else if(eng->ThrottlePosition>=37 && eng->ThrottlePosition<=45)
  Intercon=4;
 else if(eng->var[11]>0.0 && eng->var[11]<=15.0)
  Intercon=5;
 else if(eng->var[11]>=16.0 && eng->var[11]<=30.0)
  Intercon=6;

 //Throttle calculation
 eng->Force=0.0;
 eng->BrakeForce=0.0;
 eng->PowerConsuming=0.0;
 if(loco->NumEngines>=4){
  eng->EngineForce[0]=0.0;
  eng->EngineForce[1]=0.0;
  eng->EngineForce[2]=0.0;
  eng->EngineForce[3]=0.0;
  eng->EngineCurrent[0]=0.0;
  eng->EngineCurrent[1]=0.0;
  eng->EngineCurrent[2]=0.0;
  eng->EngineCurrent[3]=0.0;
  eng->EngineVoltage[0]=0.0;
  eng->EngineVoltage[1]=0.0;
  eng->EngineVoltage[2]=0.0;
  eng->EngineVoltage[3]=0.0;
 };
 if(CurrentOn && (LocoOn&4) && eng->Reverse && eng->ThrottlePosition && !SpeedCutoff){
  float Force=0.0,VelMax,Velocity,Current,CurrentQ=1.0,
        LineQ=loco->LineVoltage/3000.0,HPVoltage=0.0;
  bool TEDOff=false;
  switch(Intercon){
   case 1:VelMax=-3.125+eng->ThrottlePosition*0.245; break;
   case 2:VelMax=-2.625+eng->ThrottlePosition*0.225; break;
   case 3:VelMax=-15.525+eng->ThrottlePosition*0.636; break;
   case 4:VelMax=-15.525+eng->ThrottlePosition*0.636; break;
  };

  Velocity=loco->Velocity*eng->Reverse;
  Velocity=Velocity-VelMax;
  if(Velocity>0.0){
   switch(Intercon){
    case 1:
     Force = (150000.0+9000.0*eng->ThrottlePosition)/(pow(Velocity,2.18+(16-eng->ThrottlePosition)*0.06));
     //EngVoltage = 23.4*eng->ThrottlePosition*LineQ;
     HPVoltage=375.0*LineQ;
     //HPVoltage*=1.0+(16-eng->ThrottlePosition)*1.1;
    break;
    case 2:
     Force = (150000.0+10000.0*eng->ThrottlePosition)/(pow(Velocity,2.49-eng->ThrottlePosition*0.02));
     //EngVoltage = 375.0+34.1*(eng->ThrottlePosition-16)*LineQ;
     HPVoltage=750.0*LineQ;
    break;
    case 3:
     Force = (250000.0+22000.0*eng->ThrottlePosition)/(pow(Velocity,2.16-eng->ThrottlePosition*0.01));
     //EngVoltage = 750.0+83.3*(eng->ThrottlePosition-27)*LineQ;
     HPVoltage=1500.0*LineQ;
    break;
    case 4:
     Force = (250000.0+22000.0*eng->ThrottlePosition)/(pow(Velocity,2.16-eng->ThrottlePosition*0.01));
     //EngVoltage = 1500.0+34.1*(eng->ThrottlePosition-36)*LineQ;
     HPVoltage=1500.0*LineQ;
    break;
   };
   Force*=5.0;
  }else
   Force=800000.0;

  //Shunts
  if(eng->var[7]>0.0){
   int Shunt=eng->var[7];
   CurrentQ=CURRENT_SHUNT_Q;
   switch(Shunt){
    case 1: Force*=1.25; break;
    case 2: Force*=1.8;  break;
    case 3: Force*=2.4;  break;
    case 4: Force*=2.8;  break;
   };
  };
  Force*=LineQ;
  
  //smoothing force
  if(Force){
   //TED turned off
   if(AsyncFlags&(192<<8)){
    TEDOff=true;
    if(((AsyncFlags>>8)&192)==192)
     Force=0.0;
    else{
     switch(Intercon){
      case 2: Force*=0.75; break;
      case 3: Force*=0.5; break;
      case 4: Force*=0.5; break;
     };
    };
   };
   eng->var[9]*=eng->Reverse;
   if(Force>eng->var[9]){
    eng->var[9]+=FORCE_SHIFT*time;
    if(eng->var[9]<Force)
     Force=eng->var[9];
   }else if(Force<eng->var[9]){
    eng->var[9]-=FORCE_SHIFT*time;
    if(eng->var[9]>Force)
     Force=eng->var[9];
   };
  };
  Current=Force/CURRENT_Q;
  Current*=CurrentQ;
  Force*=eng->Reverse;

  //Calculating engine current and force
  if(Force && loco->NumEngines>=4){
   //First bogie
   if(!(AsyncFlags&(64<<8))){
    eng->PowerConsuming+=HPVoltage*Current*2.0;
    if(TEDOff){
     switch(Intercon){
      case 1:
       Current*=2.0;
       eng->EngineCurrent[0]=Current;
       eng->EngineCurrent[1]=Current;
       eng->EngineForce[0]=Force*0.55;
       eng->EngineForce[1]=Force*0.45;
      break;
      case 2:
       Current*=1.5;
       eng->EngineCurrent[0]=Current;
       eng->EngineCurrent[1]=Current;
       eng->EngineForce[0]=Force*0.55;
       eng->EngineForce[1]=Force*0.45;
      break;
      case 3:
       Current*=2.0;
       eng->EngineCurrent[0]=Current*1.1;
       eng->EngineCurrent[1]=Current*0.9;
       eng->EngineForce[0]=Force*0.5;
       eng->EngineForce[1]=Force*0.5;
       Current*=1.1;//eng->EngineCurrent[0];
      break;
      case 4:
       Current*=1.5;
       eng->EngineCurrent[0]=Current*1.05;
       eng->EngineCurrent[1]=Current*0.95;
       eng->EngineForce[0]=Force*0.5;
       eng->EngineForce[1]=Force*0.5;
       Current*=1.1;//eng->EngineCurrent[0];
      break;
     };
    }else{
     switch(Intercon){
      case 1:
       eng->EngineCurrent[0]=Current;
       eng->EngineCurrent[1]=Current;
       eng->EngineForce[0]=Force*0.29;
       eng->EngineForce[1]=Force*0.24;
      break;
      case 2:
       eng->EngineCurrent[0]=Current*1.05;
       eng->EngineCurrent[1]=Current*1.05;
       eng->EngineForce[0]=Force*0.27;
       eng->EngineForce[1]=Force*0.24;
      break;
      case 3:
       eng->EngineCurrent[0]=Current*1.1;
       eng->EngineCurrent[1]=Current;
       eng->EngineForce[0]=Force*0.25;
       eng->EngineForce[1]=Force*0.25;
      break;
      case 4:
       eng->EngineCurrent[0]=Current*1.05;
       eng->EngineCurrent[1]=Current;
       eng->EngineForce[0]=Force*0.25;
       eng->EngineForce[1]=Force*0.25;
      break;
     };
    };
   };
   //Second bogie
   if(!(AsyncFlags&(128<<8))){
    eng->PowerConsuming+=HPVoltage*Current*2.0;
    if(TEDOff){
     switch(Intercon){
      case 1:
       Current*=2.0;
       eng->EngineCurrent[2]=Current;
       eng->EngineCurrent[3]=Current;
       eng->EngineForce[2]=Force*0.55;
       eng->EngineForce[3]=Force*0.45;
      break;
      case 2:
       Current*=1.5;
       eng->EngineCurrent[2]=Current;
       eng->EngineCurrent[3]=Current;
       eng->EngineForce[2]=Force*0.55;
       eng->EngineForce[3]=Force*0.45;
      break;
      case 3:
       Current*=2.0;
       eng->EngineCurrent[2]=Current*1.05;
       eng->EngineCurrent[3]=Current*0.95;
       eng->EngineForce[2]=Force*0.5;
       eng->EngineForce[3]=Force*0.5;
       Current*=1.05;
      break;
      case 4:
       Current*=1.5;
       eng->EngineCurrent[2]=Current*1.05;
       eng->EngineCurrent[3]=Current*0.95;
       eng->EngineForce[2]=Force*0.5;
       eng->EngineForce[3]=Force*0.5;
       Current*=1.05;
      break;
     };
    }else{
     switch(Intercon){
      case 1:
       eng->EngineCurrent[2]=Current;
       eng->EngineCurrent[3]=Current;
       eng->EngineForce[2]=Force*0.24;
       eng->EngineForce[3]=Force*0.23;
      break;
      case 2:
       eng->EngineCurrent[2]=Current*0.95;
       eng->EngineCurrent[3]=Current*0.95;
       eng->EngineForce[2]=Force*0.27;
       eng->EngineForce[3]=Force*0.24;
       Current*=1.05;
      break;
      case 3:
       eng->EngineCurrent[2]=Current*0.98;
       eng->EngineCurrent[3]=Current*0.95;
       eng->EngineForce[2]=Force*0.25;
       eng->EngineForce[3]=Force*0.25;
       Current*=1.1;
      break;
      case 4:
       eng->EngineCurrent[2]=Current*0.99;
       eng->EngineCurrent[3]=Current*0.97;
       eng->EngineForce[2]=Force*0.25;
       eng->EngineForce[3]=Force*0.25;
       Current*=1.1;
      break;
     };
    };
   };
   if(eng->Wheelslip&8){
    eng->EngineCurrent[3]*=1.0+float(GetTickCount()%10)*0.02;
    Current=eng->EngineCurrent[3];
   };
   if(eng->Wheelslip&4){
    eng->EngineCurrent[2]*=1.0+float(GetTickCount()%10)*0.02;
    Current=eng->EngineCurrent[2];
   };
   if(eng->Wheelslip&2){
    eng->EngineCurrent[1]*=1.0+float(GetTickCount()%10)*0.02;
    Current=eng->EngineCurrent[1];
   };
   if(eng->Wheelslip&1){
    eng->EngineCurrent[0]*=1.0+float(GetTickCount()%10)*0.02;
    Current=eng->EngineCurrent[0];
   };
  };

  //high current
  if(Current>800.0 || loco->BrakeCylinderPressure>0.2 || eng->var[10]>1000.0){
   //SwitchBV
   SwitchBV(loco,eng,false);
   if(Current>800.0)
    Flags|=4;
   Force=0.0;
   Current=0.0;
  };
  eng->Force=Force;

  if(soundExt)
   soundExt->Var2[0]=Current/5.0;
  if(soundCab)
   soundCab->Var2[0]=Current/5.0;
 }else{
  if(soundExt)
   soundExt->Var2[0]=0.0;
  if(soundCab)
   soundCab->Var2[0]=0.0;
 };

 //Brake force calculation
 if(CurrentOn && (LocoOn&4) && eng->Reverse && eng->var[11]>0.0){
  float Force=0.0,VelMax,Velocity,Current,CurrentQ=1.0;
  switch(Intercon){
   case 5:VelMax=22.7-pow(eng->var[11]-1.0,0.68)*2.25; break;
   case 6:VelMax=8.6-pow(eng->var[11]-16.0,0.48)*1.20; break;
  };

  Velocity=loco->Velocity*eng->Reverse;
  Velocity=Velocity-VelMax;
  if(Velocity>0.0){
   switch(Intercon){
    case 5:
     Force = (1200.0+(eng->var[11]-1.0)*800.0)*(pow(Velocity,0.55+(eng->var[11]-1.0)*0.03));
    break;
    case 6:
     Force = (1500.0+(eng->var[11]-16.0)*2200.0)*(pow(Velocity,0.68-(eng->var[11]-16.0)*0.02));
    break;
   };
   /*if(Force<0.0)
    Force=0.0;*/
   Force*=5.0;
  }else
   Force=0.0;
  Force*=loco->LineVoltage/3000.0;

  //EDT is being shut down
  if(eng->var[2]==16.0){
   Force*=1.0-eng->var[1]/4.0;
   if(Force<0.0)
    Force=0.0;
   eng->var[13]=Force;
  };

  //smoothing brake force
  if(Force||eng->var[13]!=0.0){
   //TED turned off
   if(AsyncFlags&(192<<8)){
    if(((AsyncFlags>>8)&192)==192){
     Force=0.0;
    }else{
     switch(Intercon){
      case 5: Force*=0.5;  break;
      case 6: Force*=0.8; CurrentQ=2.0; break;
     };
    };
   };
   if(Force>eng->var[13]){
    eng->var[13]+=BRAKE_FORCE_SHIFT*time;
    if(eng->var[13]<Force)
     Force=eng->var[13];
   }else if(Force<eng->var[13]){
    eng->var[13]-=BRAKE_FORCE_SHIFT*time;
    if(eng->var[13]>Force)
     Force=eng->var[13];
   };
  };
  Current=Force*CurrentQ*CURRENT_Q_BF;

  if(loco->NumEngines>=4){
   if(!((AsyncFlags>>8)&64)){
    eng->EngineCurrent[0]=-Current;
    eng->EngineCurrent[1]=-Current;
   };
   if(!((AsyncFlags>>8)&128)){
    eng->EngineCurrent[2]=-Current;
    eng->EngineCurrent[3]=-Current;
   };
  };

  //high current
  if(Current>500.0){
   SwitchBV(loco,eng,false);
   Flags|=4;
   Force=0.0;
   Current=0.0;
  };

  if(soundExt)
   soundExt->Var2[0]=Current/4.0;
  if(soundCab)
   soundCab->Var2[0]=Current/4.0;

  eng->BrakeForce=Force;
 };

 eng->var[9]=eng->Force;
 eng->var[13]=eng->BrakeForce;

 //Consuming power
 if(Flags&8)
  eng->PowerConsuming+=35000.0;
 //if(CurrentOn)
  //if(LocoOn&2)

 //resistor overheat
 if(!eng->Force || eng->ThrottlePosition==16 || eng->ThrottlePosition==27 ||
        eng->ThrottlePosition==36 || eng->ThrottlePosition==45)
 {
  float CurrentQ=(50.0-AirTemperature)/30.0;
  if(Flags&1024)
   eng->var[10]-=4.5*CurrentQ*time;
  else if(Flags&512)
   eng->var[10]-=1.8*CurrentQ*time;
  else
   eng->var[10]-=0.2*CurrentQ*time;
  if(eng->var[10]<0.0)
   eng->var[10]=0.0;
 }else{
  float CurrentQ;
  switch(Intercon){
   case 1:
    CurrentQ=4.35-eng->ThrottlePosition*0.238;
   break;
   case 2:
    CurrentQ=4.12-(eng->ThrottlePosition-16)*0.305;
   break;
   case 3:
    CurrentQ=3.60-(eng->ThrottlePosition-27)*0.308;
   break;
   case 4:
    CurrentQ=3.10-(eng->ThrottlePosition-36)*0.308;
   break;
  };
  CurrentQ*=fabs(eng->Force)/200000.0;
  if(AirTemperature>20.0)
   CurrentQ*=1.0+(AirTemperature-20.0)/20.0;
  else
   CurrentQ*=1.0-(20.0-AirTemperature)/100.0;
  if(Flags&1024)
   CurrentQ*=0.5;
  else if(!(Flags&512))
   CurrentQ*=6.0;
  eng->var[10]+=CurrentQ*time;
 };


 //EPK
 if((loco->LocoFlags&1)&&((LocoOn&10)==10)){
  UINT Asp=cab->Signal.Aspect[0],PrevAsp=eng->var[17];
  float Vel=loco->Velocity,SigDist=cab->Signal.Distance;
  if(Vel<0.0) Vel=-Vel;
  if(Asp==SIGASP_BLOCK_OBSTRUCTED)
   Asp=SIGASP_STOP;
  bool Moving=(Vel>0.1)&&(eng->Reverse!=0);
  float Limit=eng->var[14];
  if(Moving && Limit>0.0){
   Limit/=3.6;
   if(Vel>Limit){
    if(eng->var[16]<30.0)
     eng->var[16]=30.0;
    eng->var[15]=3.0;
    if(Vel>Limit+6.0)
     eng->var[16]=60.0;
   };
  };
  if(Asp!=PrevAsp){
   if(PrevAsp>Asp){
    //Signal change to lower
    if(Asp>=SIGASP_APPROACH_1){
     if(eng->var[15]<2.0 || int(eng->var[15])==5){
      eng->var[15]=1.0;
      eng->var[16]=30.0;
     };
    }else{
     eng->var[15]=2.0;
     eng->var[16]=35.0;
    };
   }else{
    //Signal changed to higher
    if(eng->var[15]<3.0){
     eng->var[15]=0.0;
     loco->PostTriggerCab(111);
    };
   };
  };
  if(Moving){
   if(Asp<SIGASP_APPROACH_1){
    if(Asp==SIGASP_STOP_AND_PROCEED){
     eng->var[15]=2.0;
     //Approaching red signal
     UINT warn=ApproachRed(eng,SigDist,Vel);
     switch(warn){
      case 1:
       if(eng->var[16]<35.0)
        eng->var[16]=35.0;
       eng->var[15]=4.0;
      break;
      case 2:
       eng->var[16]=60.0;
      break;
     };
    };
    if(eng->var[15]<2.0 || int(eng->var[15])==5)
     eng->var[15]=2.0;
   }else if(cab->Signal.SpeedLimit>=0.0){
    if(eng->var[15]<1.0){
     if(Vel>cab->Signal.SpeedLimit){
      eng->var[15]=5.0;
      if(eng->var[16]<15.0)
       eng->var[16]=15.0;
     };
    }else if(int(eng->var[15])==5){
     if(Vel<=cab->Signal.SpeedLimit){
      eng->var[15]=0.0;
      loco->PostTriggerCab(111);
     };
    };
   };
  };
  
  if(eng->var[15]>0.0){
   if(eng->var[16]>=35.0){
    if(!(Flags&16)){
     Flags|=16;
     loco->PostTriggerCab(56);
    };
    if(eng->var[16]>45.0){
     Flags|=2;
    };
   };
   eng->var[16]+=time;
  }else{
   eng->var[16]=0.0;
  };
  if(Flags&16){
   if(eng->var[15]<1.0 || eng->var[16]<35.0){
    loco->PostTriggerCab(57);
    Flags&=~16;
   };
  };
  eng->var[17]=Asp;
  if(!loco->Velocity){
   Flags&=~2;
   eng->var[15]=0.0;
  };
 }else{
  eng->var[15]=0.0;
  eng->var[16]=0.0;
  eng->var[17]=0.0;
  Flags&=~2;
  if(Flags&16){
   loco->PostTriggerCab(57);
   Flags&=~16;
  };
 };

 //handbrake
 eng->HandbrakePercent=cab->GetSwitchFrame(115)*25.0;

 //Loco brake
 if(eng->IndependentBrakeValue>loco->MainResPressure)
  eng->IndependentBrakeValue=loco->MainResPressure;
 if(eng->IndependentBrakeValue>loco->IndependentBrakePressure)
  eng->MainResRate-=0.02;

 //Train brake
 eng->TrainPipeRate=0.0;
 if((loco->LocoFlags&1)&&!cab->Switch(116)){
  UpdateSlaves=true;
  if(Flags&2){
   eng->UR+=BRAKE_PIPE_EMERGENCY*time;
   if(eng->UR<0.0)
    eng->UR=0.0;
   eng->TrainPipeRate=BRAKE_PIPE_EMERGENCY;
  }else{
   switch(cab->Switch(0)){
    case 0:
     //if(eng->var[5]<45.0){
      if(!cab->SwitchSet(0))
       eng->UR+=BRAKE_UR_RATE_CHARGE*time;
      if(eng->UR>loco->MainResPressure)
       eng->UR=loco->MainResPressure;
      if(loco->TrainPipePressure<eng->UR)
       eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)/BRAKE_PIPE_RATE_CHARGE;
      AllowPipeCharging=true;
      //if(BackSec){
       //BackSec->loco->var[2]=eng->var[2];
       //*BackSecFlags|=64;
      //};
     //};
    break;
    case 1:
     //if(eng->var[5]<45.0){
      if(eng->UR<5.0){
       float rate=(loco->MainResPressure-eng->UR)*2.0;
       if(rate<0.0)rate=0.0;
       if(rate>BRAKE_UR_RATE_CHARGE)rate=BRAKE_UR_RATE_CHARGE;
       eng->UR+=rate*time;
      }else
       if(loco->BrakeCylinderPressure>0.0&&(eng->UR-loco->TrainPipePressure)<0.1)
        eng->UR+=0.15*time;
      if(eng->UR>loco->MainResPressure)
       eng->UR=loco->MainResPressure;
      if(eng->UR>loco->TrainPipePressure)
       eng->UR-=0.003*time;
      if(loco->TrainPipePressure<eng->UR-0.01){
       eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)/BRAKE_PIPE_RATE_CHARGE;
      }else if(loco->TrainPipePressure>eng->UR){
       eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure);
       if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
        eng->TrainPipeRate=-BRAKE_PIPE_RATE;
      };
      AllowPipeCharging=true;
      //if(BackSec){
      // BackSec->loco->var[2]=eng->var[2];
      // *BackSecFlags|=64;
      //};
     //};
    break;
    case 2:
     if(eng->UR>loco->MainResPressure)
      eng->UR=loco->MainResPressure;
     if(loco->TrainPipePressure>eng->UR)
      eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
     if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
      eng->TrainPipeRate=-BRAKE_PIPE_RATE;
     else if(eng->TrainPipeRate>PIPE_DISCHARGE_SLOW)
      eng->TrainPipeRate=PIPE_DISCHARGE_SLOW;
     //if(BackSec)
      //BackSec->loco->UR=eng->UR;
    break;
    case 3:
     if(eng->UR>loco->MainResPressure)
      eng->UR=loco->MainResPressure;
     if(loco->TrainPipePressure>eng->UR)
      eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
     else if(eng->UR-loco->TrainPipePressure>0.1)
      eng->TrainPipeRate=0.05;
     if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
      eng->TrainPipeRate=-BRAKE_PIPE_RATE;
     //if(BackSec)
     // BackSec->loco->var[2]=eng->var[2];
    break;
    case 4:
     if(cab->SwitchSet(0)!=4)
      break;
     eng->UR-=0.2*time;
     if(eng->UR>loco->MainResPressure)
      eng->UR=loco->MainResPressure;
     if(eng->UR<0)
      eng->UR=0;
     eng->TrainPipeRate=-BRAKE_PIPE_APPL_RATE;
     //if(BackSec)
      //BackSec->loco->var[2]=eng->var[2];
    break;
    case 5:
     eng->UR+=BRAKE_PIPE_EMERGENCY*1.2*time;
     if(eng->UR>loco->MainResPressure)
      eng->UR=loco->MainResPressure;
     if(eng->UR<0)
      eng->UR=0;
     eng->TrainPipeRate=BRAKE_PIPE_EMERGENCY;
     //if(BackSec)
      //BackSec->loco->var[2]=eng->var[2];
    break;
   };
  };
 }else{
  if(AsyncFlags&2048){
   if(eng->UR>loco->MainResPressure)
    eng->UR=loco->MainResPressure;
   if(eng->UR>loco->TrainPipePressure)
    eng->UR-=0.001*time;
   if(loco->TrainPipePressure<eng->UR)
    eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)/BRAKE_PIPE_RATE_CHARGE;
   AsyncFlags&=~2048;
  };
 };


 //Uploading some values to slave locomotives, if necessary
 if((loco->LocoFlags&1) && (LocoOn&2)){
  float NewVal;
  NewVal=cab->GetSwitchFrame(26);
  if(NewVal<=1.0)
   NewVal*=200.0;
  else if(NewVal>=9.0)
   NewVal=600.0+(NewVal-9.0)*100.0;
  else
   NewVal=200.0+(NewVal-1.0)*50.0;
  if(NewVal!=eng->var[5])
   UpdateSlaves=true;
  eng->var[5]=NewVal;
  NewVal=cab->GetSwitchFrame(32);
  if(NewVal<=1.0)
   NewVal*=100.0;
  else if(NewVal>=7.0)
   NewVal=400.0+(NewVal-7.0)*100.0;
  else
   NewVal=100.0+(NewVal-1.0)*50.0;
  if(NewVal!=eng->var[12])
   UpdateSlaves=true;
  eng->var[12]=NewVal;
  if(UpdateSlaves){
   Locomotive *slave; UINT id;
   Engine *slaveeng;
   for(UINT i=0;i<loco->NumSlaves;i++){
    if(i>3)
     break;
    switch(loco->NumSlaves){
     case 1: id=3; break;
     case 2: id=i+2; break;
     default: id=i+1; break;
    };
    slave=loco->SlaveLoco(i);
    slaveeng=slave->Eng();
    UINT &SlaveAsyncFlags=*(UINT *)&slaveeng->var[4];
    if(!cab->Switch(55+id)){
     slaveeng->var[5]=eng->var[5];
     slaveeng->var[12]=eng->var[12];
    };
    if(AllowPipeCharging)
     SlaveAsyncFlags|=2048;
    if(!cab->Switch(116))
     slaveeng->UR=eng->UR;
   };
  };
 };


 

 //Updating displays if player is in cabin
 if((State>>8)&1){
  if((LocoOn&1)&&cab->Switch(131)){
   cab->SetDisplayState(4,1);
   swprintf(buf,L"%0.0f",fabs(loco->Velocity)*3.6);
   cab->SetScreenLabel(4,0,buf);
   swprintf(buf,L"%0.0f",eng->var[14]);
   cab->SetScreenLabel(4,1,buf);
   cab->SetDisplayState(15,1);
   swprintf(buf,L"êì%0iïê%0i",int(eng->CurrentMilepost),int(eng->CurrentMilepost*10)%10);
   cab->SetScreenLabel(15,0,buf);
   cab->SetScreenState(16,0,-1);
   if(LocoOn&8){
    UINT EPKState=eng->var[15];
    if(EPKState && eng->var[16]>30.0){
     switch(EPKState){
      case 1:
      case 5:
       if(!(int(eng->var[16])%2))
        cab->SetScreenState(16,0,0);
      break;
      case 2:
       cab->SetScreenState(16,0,0);
      break;
      case 3:
       if(!(int(eng->var[16]*3.0)%2))
        cab->SetScreenState(16,0,0);
      break;
      case 4:
       if(!(int(eng->var[16]*1.5)%2))
        cab->SetScreenState(16,0,0);
      break;
     };
    };
   };
   if(Flags&2)
    cab->SetScreenState(16,1,0);
   else
    cab->SetScreenState(16,1,-1);
   eng->ALSNOn=1;
  }else{
   cab->SetDisplayState(4,0);
   cab->SetDisplayState(15,0);
   cab->SetScreenState(16,0,-1);
   cab->SetScreenState(16,1,-1);
   eng->ALSNOn=0;
  };

  //traction panel lamps
  if((loco->LocoFlags&1)&&(LocoOn&2)){
   UINT Seq=eng->var[2];
   eng->var[6]+=time;
   if(eng->var[6]>=1.0)
    eng->var[6]-=1.0;
   bool Blink=eng->var[6]-int(eng->var[6])>0.5;
   if(cab->Switch(11))
    cab->SetDisplayState(9,0);
   else
    cab->SetDisplayState(9,1);
   if(Intercon==4 && (eng->ThrottlePosition==45 || !Blink)){
    cab->SetDisplayState(5,1);
   }else
    cab->SetDisplayState(5,0);
   if(Intercon==3 && (eng->ThrottlePosition==36 || !Blink)){
    cab->SetDisplayState(6,1);
   }else
    cab->SetDisplayState(6,0);
   if(Intercon==2 && (eng->ThrottlePosition==27 || !Blink)){
    cab->SetDisplayState(7,1);
   }else
    cab->SetDisplayState(7,0);
   if(Intercon==1 && (eng->ThrottlePosition==16 || !Blink)){
    cab->SetDisplayState(8,1);
   }else
    cab->SetDisplayState(8,0);
   if(cab->Switch(10))
    cab->SetDisplayState(10,1);
   else
    cab->SetDisplayState(10,0);
   if(Seq==6 || (Seq>0 && Seq<6 && Seq-1>=Intercon && eng->var[1]-int(eng->var[1])>0.5)){
    cab->SetDisplayState(12,1);
   }else
    cab->SetDisplayState(12,0);
   if(Seq==7 || (Seq>0 && Seq<6 && Seq-1<Intercon && eng->var[1]-int(eng->var[1])>0.5)){
    cab->SetDisplayState(13,1);
   }else
    cab->SetDisplayState(13,0);
   if(eng->var[7]>0.0)
    cab->SetDisplayState(11,1);
   else
    cab->SetDisplayState(11,0);
  }else{
   cab->SetDisplayState(9,0);
   cab->SetDisplayState(5,0);
   cab->SetDisplayState(6,0);
   cab->SetDisplayState(7,0);
   cab->SetDisplayState(8,0);
   cab->SetDisplayState(10,0);
   cab->SetDisplayState(11,0);
   cab->SetDisplayState(12,0);
   cab->SetDisplayState(13,0);
  };

  cab->SetDisplayValue(0,eng->UR);
  cab->SetDisplayValue(1,loco->TrainPipePressure);
  cab->SetDisplayValue(2,loco->MainResPressure);
  cab->SetDisplayValue(3,
   loco->BrakeCylinderPressure>loco->IndependentBrakePressure?
    loco->BrakeCylinderPressure:
    loco->IndependentBrakePressure);

  //Display
  if(cab->DisplayState(14)){
   UINT id;
   Locomotive *slave;
   Engine *slaveeng;
   int CurValue; bool BVOn=true,Wheelslip=false,LowVoltage=false,
        TZ=false,RP=false,VV=true,VN=true,SH=false,TR=false;
   for(UINT i=0;i<loco->NumSlaves;i++){
    if(i>3)
     break;
    switch(loco->NumSlaves){
     case 1: id=3; break;
     case 2: id=i+2; break;
     default: id=i+1; break;
    };
    slave=loco->SlaveLoco(i);
    slaveeng=slave->Eng();
    //UINT &SlaveAsyncFlags=*(UINT *)&slave->locoA->var[4];
    UINT &SlaveFlags=*(UINT *)&slaveeng->var[0];
    if(cab->Switch(55+id)){
     cab->SetScreenState(14,id,-1);
     cab->SetScreenState(14,8+id,-1);
     cab->SetScreenState(14,12+id,-1);
     cab->SetScreenState(14,31+id,-1);
     cab->SetScreenState(14,35+id,-1);
    }else{
     cab->SetScreenState(14,id,0);
     cab->SetScreenState(14,8+id,0);
     cab->SetScreenState(14,12+id,0);
     cab->SetScreenState(14,31+id,-1);
     cab->SetScreenState(14,35+id,0);
     if(slave->NumEngines>=4)
      CurValue=slaveeng->EngineCurrent[0];
     else
      CurValue=fabs(slaveeng->Force)/CURRENT_Q;
     if(CurValue<=999 && CurValue>=-999.0){
      swprintf(buf,L"%03i",CurValue);
      cab->SetScreenLabel(14,8+id,buf);
     }else
      cab->SetScreenLabel(14,8+id,L"E99");
     if(slave->NumEngines>=4)
      CurValue=slaveeng->EngineCurrent[2];
     else
      CurValue=fabs(slaveeng->Force)/CURRENT_Q;
     if(CurValue<=999 && CurValue>=-999.0){
      swprintf(buf,L"%03i",CurValue);
      cab->SetScreenLabel(14,35+id,buf);
     }else
      cab->SetScreenLabel(14,35+id,L"E99");
     WriteThrottlePosition(buf,slaveeng);
     cab->SetScreenLabel(14,12+id,buf);
     if(!((ElectricEngine *)slaveeng)->MainSwitch)
      BVOn=false;
     else{
      if(SlaveFlags&1)
       LowVoltage=true;
     };
     if(slaveeng->Reverse){
      if(slaveeng->Force||slaveeng->var[11]>0.0){
       cab->SetScreenState(14,31+id,0);
       if(slaveeng->var[7]>0.0)
        SH=true;
      };
     };
     if(slaveeng->Wheelslip)
      Wheelslip=true;
     if(slave->BrakeCylinderPressure>0.2 || slave->IndependentBrakePressure>0.2)
      TZ=true;
     if(SlaveFlags&4)
      RP=true;
     if(!(SlaveFlags&1024))
      VV=false;
     if(!(SlaveFlags&512))
      VN=false;
     if(slaveeng->var[11]>0.0)
      TR=true;
    };
   };

   if(loco->NumSlaves<3){
    cab->SetScreenState(14,1,-1);
    cab->SetScreenState(14,9,-1);
    cab->SetScreenState(14,13,-1);
    cab->SetScreenState(14,32,-1);
    cab->SetScreenState(14,36,-1);
   };
   if(loco->NumSlaves<2){
    cab->SetScreenState(14,2,-1);
    cab->SetScreenState(14,10,-1);
    cab->SetScreenState(14,14,-1);
    cab->SetScreenState(14,33,-1);
    cab->SetScreenState(14,37,-1);
   };
   if(loco->NumSlaves<1){
    cab->SetScreenState(14,3,-1);
    cab->SetScreenState(14,11,-1);
    cab->SetScreenState(14,15,-1);
    cab->SetScreenState(14,34,-1);
    cab->SetScreenState(14,38,-1);
   };
   if(cab->Switch(55)){
    cab->SetScreenState(14,0,-1);
    cab->SetScreenState(14,8,-1);
    cab->SetScreenState(14,12,-1);
    cab->SetScreenState(14,31,-1);
    cab->SetScreenState(14,35,-1);
   }else{
    cab->SetScreenState(14,0,0);
    cab->SetScreenState(14,8,0);
    cab->SetScreenState(14,12,0);
    cab->SetScreenState(14,31,-1);
    cab->SetScreenState(14,35,0);
    if(loco->NumEngines>=4)
     CurValue=eng->EngineCurrent[0];
    else
     CurValue=fabs(eng->Force)/CURRENT_Q;
    if(CurValue<=999 && CurValue>=-999.0){
     swprintf(buf,L"%03i",CurValue);
     cab->SetScreenLabel(14,8,buf);
    }else
     cab->SetScreenLabel(14,8,L"E99");
    if(loco->NumEngines>=4)
     CurValue=eng->EngineCurrent[2];
    if(CurValue<=999 && CurValue>=-999.0){
     swprintf(buf,L"%03i",CurValue);
     cab->SetScreenLabel(14,35,buf);
    }else
     cab->SetScreenLabel(14,35,L"E99");
    WriteThrottlePosition(buf,eng);
    cab->SetScreenLabel(14,12,buf);
    if(!eng->MainSwitch)
      BVOn=false;
    else{
     if(Flags&1)
      LowVoltage=true;
    };
    if(eng->Reverse){
     if(eng->Force||eng->var[11]>0.0){
      cab->SetScreenState(14,31,0);
      if(eng->var[7]>0.0)
       SH=true;
     };
    };
    if(eng->Wheelslip)
     Wheelslip=true;
    if(loco->BrakeCylinderPressure>0.2 || loco->IndependentBrakePressure>0.2)
     TZ=true;
    if(Flags&4)
     RP=true;
    if(!(Flags&1024))
     VV=false;
    if(!(Flags&512))
     VN=false;
    if(eng->var[11]>0.0 || cab->Switch(11))
     TR=true;
   };

   if((loco->LocoFlags&1) &&!BVOn && !cab->Switch(22))
    cab->SetScreenState(14,16,0);
   else
    cab->SetScreenState(14,16,-1);
   if(loco->TrainPipePressure>=5.0)
    cab->SetScreenState(14,18,0);
   else
    cab->SetScreenState(14,18,-1);
   if(Wheelslip)
    cab->SetScreenState(14,19,0);
   else
    cab->SetScreenState(14,19,-1);

   if(TZ)
    cab->SetScreenState(14,23,0);
   else
    cab->SetScreenState(14,23,-1);
   if(RP)
    cab->SetScreenState(14,24,0);
   else
    cab->SetScreenState(14,24,-1);
   if(LowVoltage)
    cab->SetScreenState(14,25,0);
   else
    cab->SetScreenState(14,25,-1);
   if(VV){
    cab->SetScreenState(14,26,-1);
    cab->SetScreenState(14,27,0);
   }else if(VN){
    cab->SetScreenState(14,26,0);
    cab->SetScreenState(14,27,-1);
   }else{
    cab->SetScreenState(14,26,-1);
    cab->SetScreenState(14,27,-1);
   };
   if(TR)
    cab->SetScreenState(14,28,0);
   else
    cab->SetScreenState(14,28,-1);
   if(VN)
    cab->SetScreenState(14,29,0);
   else
    cab->SetScreenState(14,29,-1);
   if(SH)
    cab->SetScreenState(14,30,0);
   else
    cab->SetScreenState(14,30,-1);

  };
 };
};
