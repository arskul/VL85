//---------------------------------------------------------------------------

#include <windows.h>
#include <math.h>
#include <ts.h>

#define UR_CHARGE_RATE 0.25
#define UR_DISCHARGE_RATE -0.25
#define UR_EMERGENCY_RATE -0.65
#define PIPE_CHARGE_Q   0.4
#define CHARGE_PIPE_VALUE 5.0
#define COMPRESSOR_RATE 0.1
#define EPT_RELEASE_RATE  0.5
#define EPT_RELEASE_RATE1 0.8
#define EPT_APPLY_RATE    0.5
#define COMPRESSOR_CHARGE_RATE 0.1

#define DOOR_OPEN_TIME 5.0
#define DOOR_CHARGE_RATE 0.5

#define AUX_COMPRESSOR_RATE 0.1

#define VV_COST 0.5
#define TP_COST 0.05
#define POS_COST 0.08

#define POS_SWITCH_TIME 0.8
#define POS_SWITCH_TIME_DOWN 0.3

#define ACCELERATION_LO  0.7

#define MAX_EDT_FORCE 50000.0

#define ER9T_MAX_VELOCITY 41.8



//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------


/*

stack variables

 0,1 - Flags
 1bite
  1bit - panto raise
  2bit - engine overload
  3bit - LK
  4bit - EPK sound on
  5bit - enable pipe charging
  6bit - autostop incurred
  7bit - compressor on
  8bit - doors are opening
 2bite
  1bit - lowered acceleration
  2bit - aux.compressor enabled
  3bit - aux.compressor on
  4bit - Vent stream started
  5bit - enable vent
  6bit - broken down



 2 - Main timer

 3 - [VACANT]

 4 - agregates reservoir pressure

 5 - EP BrakeCylinder pressure value

 6 - Door timer

 7 - Set Throttle Position

 8 - Throttle timer

 9 - Dynamic brake force

 10 - EPK timer

 11 - EPK state

 12 - PSS timer

 13 - Previous signal state



 Sound triggers
   8 - tifon start
   9 - tifon end
  10 - svist start
  11 - svist end
  12 - compressor starts
  13 - compressor ends
  14,54 - brake apply/release
  15 - reverser
  16 - throttle
  17 - brake
  37 - light switch

 101 - doors opened
 102 - doors closed
 103,104 - vent on/off
 105 - common switch
 107,108 - epk start,epk end

*/


void SwitchLights(const Locomotive *loco,int State){
 switch(State){
  case 0:
   loco->SwitchLight(0,false);
   loco->SwitchLight(1,false);
   loco->SwitchLight(2,false);
   loco->SwitchLight(3,false);
   loco->SwitchLight(4,false);
   loco->SwitchLight(5,false);
   loco->SwitchLight(6,false);
   loco->SwitchLight(7,false);
   loco->SwitchLight(8,false);
   loco->SwitchLight(9,false);
   loco->SwitchLight(10,false);
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
  case 1:
   loco->SwitchLight(0,true);
   loco->SwitchLight(1,true);
   loco->SwitchLight(2,false);
   loco->SwitchLight(3,false);
   loco->SwitchLight(4,false);
   loco->SwitchLight(5,false);
   loco->SwitchLight(6,false);
   loco->SwitchLight(7,true);
   loco->SwitchLight(8,true);
   loco->SwitchLight(9,false);
   loco->SwitchLight(10,false);
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
  case 2:
   loco->SwitchLight(0,false);
   loco->SwitchLight(1,false);
   loco->SwitchLight(2,false);
   loco->SwitchLight(3,false);
   loco->SwitchLight(4,true);
   loco->SwitchLight(5,false);
   loco->SwitchLight(6,true);
   loco->SwitchLight(7,false);
   loco->SwitchLight(8,false);
   loco->SwitchLight(9,true);
   loco->SwitchLight(10,true);
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
  case 3:
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(13,false);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
  case 4:
   loco->SwitchLight(11,true);
   loco->SwitchLight(12,true);
   loco->SwitchLight(13,false);
   loco->SwitchLight(14,true);
   loco->SwitchLight(15,true);
  break;
  case 5:
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(13,false);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
  case 6:
   loco->SwitchLight(11,false);
   loco->SwitchLight(12,false);
   loco->SwitchLight(13,true);
   loco->SwitchLight(14,false);
   loco->SwitchLight(15,false);
  break;
 };
};


extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 Cabin *cab=loco->Cab();

 eng->Panto=0;
 eng->MainSwitch=0;
 eng->PowerConsuming=0.0;
 eng->var[0]=0.0;
 eng->var[1]=0.0;
 eng->var[2]=0.0;
 eng->UR=0.0;
 eng->var[4]=3.5;
 eng->var[5]=2.5;
 eng->var[7]=0.0;
 eng->var[8]=0.0;
 eng->var[9]=0.0;
 eng->var[10]=0.0;
 eng->var[11]=0.0;
 eng->var[12]=0.0;
 eng->var[13]=0.0;
 eng->Power=0.0;
 eng->Force=0.0;
 eng->BrakeForce=0.0;
 eng->IndependentBrakeValue=0.0;
 loco->IndependentBrakePressure=0.0;
 eng->TrainPipeRate=0.0;
 eng->AuxilaryRate=0.0;
 eng->MainResRate=0.0;
 eng->ChargingRate=0.0;
 eng->HandbrakePercent=100.0;
 eng->DynamicBrakePercent=0.0;
 eng->EPTvalue=-1.0;
 eng->BrakeSystemsEngaged=5;
 eng->ALSNOn=0;
 eng->Sanding=0;
 eng->Reverse=0;
 eng->Wheelslip=0;
 eng->ThrottlePosition=0;

 unsigned long state=State&0xFF;
 if(!state||state==1){
  loco->TrainPipePressure=0.0;
  loco->AuxiliaryPressure=2.5;
  loco->BrakeCylinderPressure=2.5;
  loco->MainResPressure=4.0;
  if(loco->LibParam==0||loco->LibParam==2)
   cab->SetSwitch(2,6,true);
 }else if(state==2 || state==3){
  ULONG *Flags=(ULONG *)&eng->var[0];
  loco->TrainPipePressure=4.5;
  loco->AuxiliaryPressure=4.5;
  loco->BrakeCylinderPressure=0.0;
  loco->ChargingPipePressure=4.5;
  loco->MainResPressure=6.0;
  loco->PantoRaised=1;
  eng->Panto=1;
  eng->MainSwitch=1;
  *Flags|=1;
  eng->UR=4.5;
  eng->var[4]=6.0;
  eng->var[5]=0.0;
  if(loco->LibParam==0||loco->LibParam==2){
   cab->SetSwitch(2,1,true);
   cab->SetSwitch(3,2,true);
   eng->HandbrakePercent=0.0;
   if((State>>8)&1){
    eng->EPTvalue=0.0;
    loco->LocoFlags|=1;
    if(loco->Velocity<0.0 ^ ((loco->Flags&1)^(loco->LibParam==2)))
     cab->SetSwitch(1,1,false);
    else
     cab->SetSwitch(1,3,false);
    cab->SetSwitch(15,1,false);
    cab->SetSwitch(20,1,true);
    cab->SetSwitch(28,2,true);
    cab->SetSwitch(33,1,true);
    cab->SetSwitch(42,1,false);
    cab->SetSwitch(34,1,false);
    cab->SetSwitch(35,1,true);
    cab->SetSwitch(37,1,false);
    cab->SetSwitch(39,1,false);
   };
  };
 };

 return true;
}




extern "C" void __export  ChangeLoco
 (Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 if(loco->LibParam==0||loco->LibParam==2){
  if(!Prev)
   loco->LocoFlags|=1;
  else{
   Cabin *Prevcab=Prev->Cab();
   if(!Prevcab->Switch(1)&&!Prevcab->Switch(11)&&!Prevcab->Switch(20)
   )
    loco->LocoFlags|=1;
  };
 };
};



extern "C" bool __export CanWorkWith
        (const Locomotive *loco,const wchar_t *Type)
{
 if(!lstrcmpiW(Type,L"er9t"))
  return true;
 return false;
};



extern "C" bool __export  CanSwitch(const ElectricLocomotive *loco,const ElectricEngine *eng,
        unsigned int SwitchID,unsigned int SetState)
{

 UINT n;
 Cabin *cab=loco->Cab();

 if(loco->LibParam==0||loco->LibParam==2){
  switch(SwitchID){
   case 0:
    n=cab->SwitchSet(1);
    if(!n||n==2)
     return false;
    loco->PostTriggerCab(16);
   break;
   case 1:
    if(!(loco->LocoFlags&1))
     if(SetState==1||SetState>2)
      return false;
    n=cab->SwitchSet(0);
    if(n!=4)
     return false;
    if(SetState&&cab->SwitchSet(1))
     loco->PostTriggerCab(15);
   break;
   case 2:
    loco->PostTriggerCab(17);
   break;
   case 3:

   break;
   case 22:

   break;
   case 23:

   break;
   case 27:
    loco->PostTriggerCab(37);
   break;
   case 28:
    loco->PostTriggerCab(37);
   break;
   case 29:

   break;
   default:
    if(SwitchID<44)
     loco->PostTriggerCab(105);
   break;
  };
 };

 return true;
};




extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{

 Cabin *cab=loco->Cab();
 ULONG *Flags=(ULONG *)&eng->var[0];
 UINT i;

 if(loco->LibParam==0||loco->LibParam==2){
  switch(SwitchID){
   case 1:
    eng->Reverse=cab->Switch(1);
    if(eng->Reverse){
     eng->Reverse-=2;
     if((loco->Flags&1)^(loco->LibParam==2))
      eng->Reverse=-eng->Reverse;
    };
    for(i=0;i<loco->NumSlaves;i++)
     loco->SlaveLoco(i)->Eng()->Reverse=eng->Reverse;
   break;
   case 3:
    eng->HandbrakePercent=(2-cab->Switch(3))*50.0;
   break;
   case 4:
    if(loco->LocoFlags&1){
     for(i=0;i<loco->NumSlaves;i++){
      ((ElectricEngine *)loco->SlaveLoco(i)->Eng())->MainSwitch=0;
      loco->SlaveLoco(i)->Eng()->var[4]-=0.5;
     };
    };
   break;
   case 5:
    if(cab->Switch(5)&&(loco->LocoFlags&1)&&cab->Switch(20)){
     if(!cab->Switch(4)){
      if(eng->ThrottlePosition)
       return;
      for(i=0;i<loco->NumSlaves;i++){
       Locomotive *Slave=loco->SlaveLoco(i);
       if((Slave->LibParam==1||Slave->LibParam==3)
         &&Slave->Eng()->var[4]>=5.5)
       {
        if(Slave->Eng()->ThrottlePosition){
         Switched(loco,eng,4,0);
         return;
        };
        ((ElectricEngine *)Slave->Eng())->MainSwitch=1;
        Slave->Eng()->var[4]-=0.5;
       };
      };
     };
    };
   break;
   case 9:
    if(loco->LocoFlags&1){
     for(i=0;i<loco->NumSlaves;i++){
      Locomotive *Slave=loco->SlaveLoco(i);
      if(Slave->LibParam==1||Slave->LibParam==3){
       if(cab->Switch(9)&&cab->Switch(20))
        *(ULONG *)&Slave->Eng()->var[0]|=256;
       else
        *(ULONG *)&Slave->Eng()->var[0]&=~256;
      };
     };
    };
   break;
   case 11:
    if((loco->LocoFlags&1)&&!cab->Switch(12)&&cab->Switch(11)){
     for(i=0;i<loco->NumSlaves;i++)
      *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]|=1;
    };
   break;
   case 12:
    if((loco->LocoFlags&1)&&cab->Switch(12)){
     for(i=0;i<loco->NumSlaves;i++)
      *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]&=~1;
    };
   break;
   case 13:
    if(loco->LocoFlags&1){
     for(i=0;i<loco->NumSlaves;i++){
      if(cab->Switch(39)&&cab->Switch(20)&&cab->Switch(13))
       *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]|=512;
      else
       *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]&=~512;
     };
    };
   break;
   case 14:
    if(loco->LocoFlags&1){
     for(i=0;i<loco->NumSlaves;i++){
      if(cab->Switch(20)&&cab->Switch(14))
       loco->SlaveLoco(i)->Eng()->Sanding=1;
      else
       loco->SlaveLoco(i)->Eng()->Sanding=0;
     };
    };
   break;
   case 15:
    Switched(loco,eng,24,0);
    Switched(loco,eng,25,0);
   break;
   case 16:
     if(cab->Switch(16)){
      if(cab->Switch(33))
       loco->PostTriggerEng(8);
     }else
      loco->PostTriggerEng(9);
   break;
   case 17:
    if(cab->Switch(17)){
      if(cab->Switch(33))
       loco->PostTriggerEng(10);
    }else
     loco->PostTriggerEng(11);
   break;
   case 20:
    if(loco->LocoFlags&1){
     if(cab->Switch(20)){
      Switched(loco,eng,35,0);
     }else{
      Switched(loco,eng,4,0);
     };
     Switched(loco,eng,9,0);
     Switched(loco,eng,13,1);
     Switched(loco,eng,14,1);
    };
   break;
   case 23:
    eng->var[10]=0.0;
    eng->var[11]=0.0;
   break;
   case 24:
    if((loco->LocoFlags&1)&&cab->Switch(15)&&cab->Switch(20)
      &&loco->ChargingPipePressure>2.0)
    {
     *Flags|=128;
     eng->var[6]=DOOR_OPEN_TIME;
     for(i=0;i<loco->NumSlaves;i++){
      *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]|=128;
      loco->SlaveLoco(i)->Eng()->var[6]=DOOR_OPEN_TIME;
     };
     if((loco->Flags&1)||loco->LibParam==2){
      if(cab->Switch(24)){
       eng->PostGlobalMessage(GM_DOOR_OPEN,2);
       eng->PostGlobalMessage(GM_SMS_POST,101);
      }else{
       eng->PostGlobalMessage(GM_DOOR_CLOSE,2);
       eng->PostGlobalMessage(GM_SMS_POST,102);
      };
     }else{
      if(cab->Switch(24)){
       eng->PostGlobalMessage(GM_DOOR_OPEN,1);
       eng->PostGlobalMessage(GM_SMS_POST,101);
      }else{
       eng->PostGlobalMessage(GM_DOOR_CLOSE,1);
       eng->PostGlobalMessage(GM_SMS_POST,102);
      };
     };
    };
   break;
   case 25:
    if((loco->LocoFlags&1)&&cab->Switch(15)&&cab->Switch(20)
      &&loco->ChargingPipePressure>2.0)
    {
     *Flags|=128;
     eng->var[6]=DOOR_OPEN_TIME;
     for(i=0;i<loco->NumSlaves;i++){
      *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]|=128;
      loco->SlaveLoco(i)->Eng()->var[6]=DOOR_OPEN_TIME;
     };
     if((loco->Flags&1)||loco->LibParam==2){
      if(cab->Switch(25)){
       eng->PostGlobalMessage(GM_DOOR_OPEN,1);
       eng->PostGlobalMessage(GM_SMS_POST,101);
      }else{
       eng->PostGlobalMessage(GM_DOOR_CLOSE,1);
       eng->PostGlobalMessage(GM_SMS_POST,102);
      };
     }else{
      if(cab->Switch(25)){
       eng->PostGlobalMessage(GM_DOOR_OPEN,2);
       eng->PostGlobalMessage(GM_SMS_POST,101);
      }else{
       eng->PostGlobalMessage(GM_DOOR_CLOSE,2);
       eng->PostGlobalMessage(GM_SMS_POST,102);
      };
     };
    };
   break;
   case 27:
    SwitchLights(loco,cab->Switch(27));
   break;
   case 28:
    if(loco->LocoFlags&1){
     if(!PrevState&&cab->Switch(28)&&cab->Switch(35))
      eng->var[5]=loco->BrakeCylinderPressure;
    };
   break;
   case 29:
    if(!(*Flags&32)){
     eng->var[10]=0.0;
     eng->var[11]=0.0;
    };
   break;
   case 30:
    if(loco->LocoFlags&1){
     if(cab->Switch(30))
      eng->PostGlobalMessage(GM_LIGHT_ON,1);
     else
      eng->PostGlobalMessage(GM_LIGHT_OFF,1);
    };
   break;
   case 31:
    if(cab->Switch(31)){
     cab->SwitchLight(0,1);
     cab->SwitchLight(1,1);
    }else{
     cab->SwitchLight(0,0);
     cab->SwitchLight(1,0);
    };
   break;
   case 32:
    SwitchLights(loco,cab->Switch(32)+5);
   break;
   case 33:
    if(cab->Switch(33)){
     if(cab->Switch(16))
      loco->PostTriggerEng(8);
     if(cab->Switch(17))
      loco->PostTriggerEng(10);
    }else{
     if(cab->Switch(16))
      loco->PostTriggerEng(9);
     if(cab->Switch(17))
      loco->PostTriggerEng(11);
    };
   break;
   case 35:
    if((loco->LocoFlags&1)&&cab->Switch(35)){
     Switched(loco,eng,25,0);
    };
   break;
   case 37:
    if(loco->LocoFlags&1){
     for(i=0;i<loco->NumSlaves;i++){
      if(cab->Switch(37))
       *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]|=4096;
      else
       *(ULONG *)&loco->SlaveLoco(i)->Eng()->var[0]&=~4096;
     };
    };
   break;
   case 39:
    Switched(loco,eng,13,0);
   break;
   case 42:
    for(i=0;i<loco->NumSlaves;i++)
     if(loco->SlaveLoco(i)->LibParam==2||loco->SlaveLoco(i)->LibParam==0)
      SwitchLights(loco->SlaveLoco(i),cab->Switch(42)+3);
   break;
   case 44:
    cab->SetDisplayState(22,cab->Switch(44)+1);
   break;
  };
 };

};




extern "C" void _export SpeedLimit(const Locomotive *loco,
        SpeedLimitDescr Route,SpeedLimitDescr Signal,SpeedLimitDescr Event)
{
 if(loco->LibParam!=0 && loco->LibParam!=2)
  return;
 if(!(loco->LocoFlags&1))
  return;

 Engine *eng=loco->Eng();
 Cabin *cab=loco->Cab();
 unsigned long &Flags=*(unsigned long *)&eng->var[0];

 if(Signal.Limit>12.0)
  Signal.Limit=-1.0;
 else if(!Signal.Limit)
  Signal.Limit=6.0;
 if(Signal.NextLimit>12.0)
  Signal.NextLimit=-1.0;


 float Limit=GetLimit(Route.Limit,Signal.Limit,Event.Limit);
 float Vel=fabs(loco->Velocity);

 if(cab->Switch(26)==1){
  if(Limit>=0.0){
   if(!Signal.NextLimit){
    if((Signal.Distance<50.0 && Vel>3.0) || (Signal.Distance<250.0 && Vel>6.0)){
     eng->var[10]=76;
     Flags|=32;
    }else if(eng->var[11]<2.0){
     if((Vel>3.0 && Signal.Distance<150.0) ||(Vel>6.0 && Signal.Distance<350.0)){
      eng->var[11]=3.0;
      eng->var[10]=20;
     };
    };
   };
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
 }else if(cab->Switch(26)==0){
  if(Limit>=0.0){
   if(Vel>Limit && eng->var[11]<2.0){
    eng->var[11]=3.0;
    eng->var[10]=55;
   };
   if(Vel>Limit+3){
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
 };
};




void RunHead(
 ElectricEngine *eng,const ElectricLocomotive *loco,const float &time,unsigned long &Flags,
 const unsigned long &State)
{
 Cabin *cab=loco->Cab();
 //bool CurrentOn=loco->LineVoltage>19000.0&&loco->LineVoltage<30000.0;

 //EPT is on?
 eng->BrakeSystemsEngaged&=~2;
 if(loco->LocoFlags&1){
  if(cab->Switch(20)&&cab->Switch(35))
   if(cab->Switch(28))
    eng->BrakeSystemsEngaged|=2;
 };

 //ALSN
 eng->ALSNOn=0;
 if(cab->Switch(20)&&cab->Switch(26)<2)
  eng->ALSNOn=1;

 //Main controller
 if(loco->LocoFlags&1){
  int Pos=cab->Switch(0)-4;
  switch(Pos){
   case 1:
    Pos=3;
   break;
   case 2:
    Pos=8;
   break;
   case 3:
    Pos=12;
   break;
   case 4:
    Pos=17;
   break;
   case 5:
    Pos=20;
   break;
  };
  for(UINT i=0;i<loco->NumSlaves;i++){
   loco->SlaveLoco(i)->Eng()->var[7]=Pos;
  };
 };


 eng->EPTvalue=-1.0;
 eng->TrainPipeRate=0.0;
 eng->ChargingRate=(CHARGE_PIPE_VALUE-loco->ChargingPipePressure)*PIPE_CHARGE_Q;
 if(eng->ChargingRate<0.0||loco->ChargingPipePressure>loco->MainResPressure)
  eng->ChargingRate=0.0;

 //EPK
 if((loco->LocoFlags&1)&&loco->Velocity&&eng->Reverse){
  bool SoundOn=(Flags&8);
  Flags&=~8;
  if(fabs(loco->Velocity)<0.05)
   Flags&=~32;
  if(cab->Switch(26)==1){
   UINT Aspect=cab->Signal.Aspect[0];
   if(Aspect!=unsigned(eng->var[13]))
   {
    if(Aspect<unsigned(eng->var[13]) || Aspect==SIGASP_BLOCK_OBSTRUCTED){
     eng->var[10]=30.0;
     if(Aspect==SIGASP_STOP_AND_PROCEED || Aspect==SIGASP_RESTRICTING)
      eng->var[10]=40.0;
     eng->var[11]=1.0;
    }else if(eng->var[11]<3.0){
     eng->var[10]=0.0;
     eng->var[11]=4.0;
     eng->var[12]=0.0;
    };
   };
   if(cab->Signal.Aspect[0]<SIGASP_APPROACH_1){
    eng->var[10]+=time;
    if(eng->var[10]>30.0)
     eng->var[11]=2.0;
   };
   if(eng->var[11]==1.0 || eng->var[11]==3.0)
    eng->var[10]+=time;
   if(eng->var[10]>35.0){
    Flags|=8;
    if(!SoundOn)
     loco->PostTriggerCab(107);
    SoundOn=true;
   };
   if(eng->var[10]>55.0){
    eng->TrainPipeRate=-1.5;
    Flags|=32;
   };
  }else if(cab->Switch(26)==0){
   eng->var[10]+=time;
   if(eng->var[10]>60.0){
    eng->var[11]=2.0;
   };
   if(eng->var[10]>65.0){
    Flags|=8;
    if(!SoundOn)
     loco->PostTriggerCab(107);
    SoundOn=true;
   };
   if(eng->var[10]>75.0){
    eng->TrainPipeRate=-1.5;
    Flags|=32;
   };
  };
  eng->var[13]=cab->Signal.Aspect[0];
  if(SoundOn&&!(Flags&8))
   loco->PostTriggerCab(108);
 }else{
  if(Flags&40){
   loco->PostTriggerCab(108);
   Flags&=~40;
  };
  eng->var[10]=0.0;
  eng->var[11]=0.0;
  eng->var[12]=0.0;
 };

 //Brakes
 if(loco->LocoFlags&1){
  bool EnableTrainPipeCharge=false;
  switch(cab->Switch(2)){
   case 0:
    if(Flags&32)
     break;
    if(cab->SwitchSet(2)!=0)
     break;
    eng->UR+=UR_CHARGE_RATE*time;
    if(eng->UR>loco->MainResPressure)
     eng->UR=loco->MainResPressure;
    eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
    if(eng->TrainPipeRate<0.0)
     eng->TrainPipeRate=0.0;
    eng->var[5]-=EPT_RELEASE_RATE1*time;
    if(eng->var[5]<0.0)
     eng->var[5]=0.0;
    if(eng->BrakeSystemsEngaged&2)
     eng->EPTvalue=eng->var[5];
    EnableTrainPipeCharge=true;
   break;
   case 1:
    if(Flags&32)
     break;
    if(eng->UR<4.5)
     eng->UR+=UR_CHARGE_RATE*time;
    if(eng->UR>loco->MainResPressure)
     eng->UR=loco->MainResPressure;
    eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
    if(eng->TrainPipeRate<0.0)
     eng->TrainPipeRate=0.0;
    eng->var[5]-=EPT_RELEASE_RATE*time;
    if(eng->var[5]<0.0)
     eng->var[5]=0.0;
    if(eng->BrakeSystemsEngaged&2)
     eng->EPTvalue=eng->var[5];
    EnableTrainPipeCharge=true;
   break;
   case 2:
    if(Flags&32)
     break;
    eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
    if(eng->TrainPipeRate>0.0)
     eng->TrainPipeRate=0.0;
    if(eng->BrakeSystemsEngaged&2){
     eng->EPTvalue=eng->var[5];
    };
   break;
   case 3:
    if(Flags&32)
     break;
    eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
    if(eng->TrainPipeRate>0.05)
     eng->TrainPipeRate=0.05;
    else if(eng->TrainPipeRate>0.0&&eng->TrainPipeRate<0.1)
     eng->TrainPipeRate=0.0;
    if(eng->BrakeSystemsEngaged&2){
     eng->EPTvalue=eng->var[5];
    };
   break;
   case 4:
    if(Flags&32)
     break;
    if(cab->SwitchSet(2)<4)
     break;
    if(eng->BrakeSystemsEngaged&2){
     eng->var[5]+=EPT_APPLY_RATE*time;
     if(eng->var[5]>6.0)
      eng->var[5]=6.0;
     eng->EPTvalue=eng->var[5];
    };
    eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
    if(eng->TrainPipeRate>0.0)
     eng->TrainPipeRate=0.0;
   break;
   case 5:
    if(Flags&32)
     break;
    if(cab->SwitchSet(2)<5)
     if(!(eng->BrakeSystemsEngaged&2))
      break;
    eng->UR+=UR_DISCHARGE_RATE*time;
    if(eng->UR<0.0)
     eng->UR=0.0;
    eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
    if(eng->TrainPipeRate>0.0)
     eng->TrainPipeRate=0.0;
    if(eng->BrakeSystemsEngaged&2){
     eng->var[5]+=EPT_APPLY_RATE*time;
     if(eng->var[5]>6.0)
      eng->var[5]=6.0;
     eng->EPTvalue=eng->var[5];
    };
   break;
   case 6:
    eng->ChargingRate=0.0;
    eng->UR-=1.0*time;
    if(eng->UR<0.0)
     eng->UR=0.0;
    eng->TrainPipeRate=-1.0;
   break;
  };

  for(UINT i=0;i<loco->NumSlaves;i++){
   Engine *SlaveEng=loco->SlaveLoco(i)->Eng();
   if(SlaveEng->EPTvalue>eng->EPTvalue)
    eng->EPTvalue=-1.0;
   if(EnableTrainPipeCharge)
    *(ULONG *)&SlaveEng->var[0]|=16;
   else
    *(ULONG *)&SlaveEng->var[0]&=~16;
   SlaveEng->UR=eng->UR;
  };

 };


 if((State>>8)&1){
  bool Voltage=false,RN=false,DoorL=false,DoorR=false,RP=false,VV=false,
       LK=false,RB=false;
  ElectricLocomotive *l;ULONG *Fl;
  for(UINT i=0;i<loco->NumSlaves;i++){
   l=(ElectricLocomotive *)loco->SlaveLoco(i);
   ElectricEngine *SlaveEng=(ElectricEngine *)l->Eng();
   Fl=(ULONG *)&SlaveEng->var[0];
   if(l->LibParam==1||l->LibParam==3){
    if(SlaveEng->MainSwitch)
     Voltage=true;
    else
     VV=true;
    if(!l->PantoRaised||!SlaveEng->MainSwitch)
     RN=true;
    if(*Fl&2)
     RP=true;
    if(*Fl&4)
     LK=true;
    if(SlaveEng->Wheelslip)
     RB=true;
   };
   for(USHORT j=0;j<l->NumDoors;j++){
    ActDoor *d=l->Door(j);
    if(d->GlobalID==1)
     if(d->State>0)
      DoorL=true;
    if(d->GlobalID==2)
     if(d->State>0)
      DoorR=true;
   };
  };

  cab->SetDisplayValue(0,fabs(loco->Velocity));
  cab->SetDisplayValue(1,Voltage?loco->LineVoltage:0);
  cab->SetDisplayValue(2,loco->BrakeCylinderPressure);
  cab->SetDisplayValue(3,eng->UR);
  cab->SetDisplayValue(4,loco->TrainPipePressure);
  cab->SetDisplayValue(5,loco->ChargingPipePressure);
  if((loco->LocoFlags&1)&&cab->Switch(20)){
   cab->SetDisplayState(6,RN);
   cab->SetDisplayState(7,DoorL);
   cab->SetDisplayState(8,DoorR);
   cab->SetDisplayState(9,RP);
   cab->SetDisplayState(10,VV);
   cab->SetDisplayState(11,LK);
   cab->SetDisplayState(12,RB);
   cab->SetDisplayState(13,VV||!loco->LineVoltage);
   cab->SetDisplayState(14,0);
   RP=(eng->BrakeSystemsEngaged&2)&&cab->Switch(28)>1;
   cab->SetDisplayState(15,RP&&loco->BrakeCylinderPressure!=0.0);
   cab->SetDisplayState(16,RP&&(cab->Switch(2)>3));
   cab->SetDisplayState(17,RP&&(cab->Switch(2)==2));
   cab->SetDisplayState(18,(eng->BrakeSystemsEngaged&2)?1:0);
   switch(int(eng->var[11])){
    case 0:
     cab->SetDisplayState(19,0);
    break;
    case 1:
     eng->var[12]+=time;
     if(eng->var[12]>=2.0)
      eng->var[12]-=2.0;
     if(eng->var[12]<1.0)
      cab->SetDisplayState(19,1);
     else
      cab->SetDisplayState(19,0);
    break;
    case 2:
     cab->SetDisplayState(19,1);
    break;
    case 3:
     eng->var[12]+=time;
     if(eng->var[12]>=1.0)
      eng->var[12]-=1.0;
     if(eng->var[12]<0.5)
      cab->SetDisplayState(19,1);
     else
      cab->SetDisplayState(19,0);
    break;
    case 4:
     eng->var[12]+=time;
     if(eng->var[12]>0.5){
      eng->var[11]=0.0;
     }else{
      if(!(int(eng->var[12]*5)%2))
       cab->SetDisplayState(19,1);
      else
       cab->SetDisplayState(19,0);
     };
    break;
   };
  }else{
   cab->SetDisplayState(6,0);
   cab->SetDisplayState(7,0);
   cab->SetDisplayState(8,0);
   cab->SetDisplayState(9,0);
   cab->SetDisplayState(10,0);
   cab->SetDisplayState(11,0);
   cab->SetDisplayState(12,0);
   cab->SetDisplayState(13,0);
   cab->SetDisplayState(14,0);
   cab->SetDisplayState(15,0);
   cab->SetDisplayState(16,0);
   cab->SetDisplayState(17,0);
   cab->SetDisplayState(18,0);
   cab->SetDisplayState(19,0);
  };
  cab->SetDisplayValue(20,0.0);
  cab->SetDisplayValue(21,0.0);
 };

};




void RunMotor(
 ElectricEngine *eng,const ElectricLocomotive *loco,const float &time,unsigned long &Flags)
{
 if(Flags&8192)
  return;

 if((Flags&1)&&eng->var[4]>4.0){
  eng->Panto|=3;
  if(!loco->PantoRaised)
   eng->var[4]-=TP_COST*time;
 }else{
  eng->Panto&=~3;
 };
 if(eng->var[4]<0.0)
  eng->var[4]=0.0;
 if(eng->var[4]<4.0)
  eng->MainSwitch=0;


 Cabin *cab=loco->Cab();
 bool CurrentOn=loco->LineVoltage>19000.0&&loco->LineVoltage<30000.0;
 if(!eng->MainSwitch)
  CurrentOn=false;
 else if(loco->LineVoltage>800.0 && !loco->LineFreq){
  Flags|=8192;
  eng->ShowMessage(GMM_BOX,L"Моторный вагон сгорел");
  CurrentOn=false;
 };


 //Switching LK
 Flags&=~4;
 if(eng->var[7]!=signed(eng->ThrottlePosition)){
  if(eng->var[7]>0.0||eng->ThrottlePosition){
   UINT set=eng->var[7]>0.0?eng->var[7]:0;
   int diff=set>eng->ThrottlePosition?1:-1;
   eng->var[8]+=time;
   if(set==eng->ThrottlePosition){
    eng->var[8]=0.0;
   };
   if(set>eng->ThrottlePosition){
    if(!eng->ThrottlePosition)
     Flags|=4;
    while(eng->var[8]>POS_SWITCH_TIME){
     float MaxForce=60000.0,Force=0.0;
     if(diff>0){
      float Vel=-3.425+(eng->ThrottlePosition+1)*0.685;
      Vel=loco->Velocity*eng->Reverse-Vel;
      if(Vel<0.0)
       Force=100000.0;
      else
       Force=(8000.0+9500.0*(eng->ThrottlePosition+1))/(pow(Vel,1.8-(eng->ThrottlePosition+1)*0.043));
     };
     if(eng->var[4]>2.0&&(diff<0||Force<MaxForce)){
      eng->ThrottlePosition+=diff;
      eng->var[4]-=POS_COST;
      if(set==eng->ThrottlePosition){
       eng->var[8]=0.0;
       break;
      };
     };
     eng->var[8]-=POS_SWITCH_TIME;
    };
   }else if(set<eng->ThrottlePosition){
    while(eng->var[8]>POS_SWITCH_TIME_DOWN){
     eng->ThrottlePosition--;
     eng->var[8]-=POS_SWITCH_TIME_DOWN;
     if(eng->ThrottlePosition==set){
      eng->var[8]=0.0;
      break;
     };
    };
   };
  };
 };

 SMSObject *soundExt=loco->SoundEng();
 eng->EPTvalue=-1.0;
 eng->BrakeForce=0.0;
 eng->BrakeSystemsEngaged=5;
 if(eng->var[7]<0.0||eng->var[9]>0.0){
  //Dynamic brake
  float BrakeForce=0.0;
  if(eng->var[7]<0.0&&!eng->ThrottlePosition){
   float Vel=fabs(loco->Velocity);
   float VelMin=loco->GetParameter(L"EDTVelA",1.5)*(eng->var[7]+loco->GetParameter(L"EDTVelB",4.0));
   float BrakeM = loco->GetParameter(L"EDTForceBase",500.0);
   float EDTPosGain= loco->GetParameter(L"EDTPosGain",2.0);
   float MaxEDTForce= loco->GetParameter(L"EDTMaxForce",50000);

   if(eng->var[7]<0.0&&Vel>0.0&&Vel<3.0&&loco->BrakeCylinderPressure<1.5){
    eng->EPTvalue=1.5;
    eng->BrakeSystemsEngaged|=2;
   };
   if(Vel<VelMin)
    Vel=0;
   else
    Vel-=VelMin;
   BrakeForce=Vel*BrakeM*(1.0-(eng->var[7]+1.0)*EDTPosGain);
   if(BrakeForce<0.0)
    BrakeForce=0.0;
   else if(BrakeForce>MaxEDTForce)
    BrakeForce=MaxEDTForce;
  };
  if(BrakeForce>eng->var[9]){
   eng->var[9]+=BrakeForce*0.1*time;
   if(eng->var[9]>BrakeForce)
    eng->var[9]=BrakeForce;
  }else{
   eng->var[9]-=4000.0*time;
   if(eng->var[9]<0.0)
    eng->var[9]=0.0;
  };
  eng->BrakeForce=eng->var[9];
 };

 //Calculating force
 if(CurrentOn){
  float Force;
  float Vel=-3.425+eng->ThrottlePosition*0.685,Vel1;
  Vel=loco->Velocity*eng->Reverse-Vel;
  if(eng->ThrottlePosition){
   if(Vel<0.0)
    Force=120000.0;
   else
    Force=((8000.0+9500.0*eng->ThrottlePosition)/(pow(Vel,1.8-0.043*eng->ThrottlePosition)));
   Vel1=3.0+eng->ThrottlePosition*0.9;
   if(Vel>Vel1){
    Force*=1.0-(Vel-Vel1)/40.0;
    if(Force<0.0)
     Force=0.0;
   };
  }else{
   Force=0.0;
   eng->Force=0.0;
  };

  if(Flags&256)
   Force*=ACCELERATION_LO;

  if(fabs(eng->Force)>75000.0){
   eng->MainSwitch=0;
   eng->Force=0.0;
   eng->Power=0;
   Force=0.0;
   Flags|=2;
  }else{
   Flags&=~2;
  };

  if(Force){
   if(loco->BrakeCylinderPressure>0.5){
    eng->MainSwitch=0;
    eng->Power=0;
    eng->Force=0.0;
    Force=0.0;
   };
   for(UINT i=0;i<cab->NumDoors;i++){
    ActDoor *d=cab->Door(i);
    if(d->GlobalID==1||d->GlobalID==2){
     if(d->State){
      eng->Power=0.0;
      eng->Force=0.0;
      Force=0.0;
      break;
     };
    };
   };
  };

  float diff=Force-eng->Force*eng->Reverse;
  if(diff>-1.0&&diff<1.0)
   eng->Force=Force*eng->Reverse;
  else
   eng->Force+=diff*0.7*time*eng->Reverse;
 }else{
  eng->Power=0.0;
  eng->Force=0.0;
 };
 if(loco->NumEngines){
  eng->EngineForce[0]=eng->Force/2.0;
  eng->EngineForce[1]=eng->EngineForce[0];
 };
 if(soundExt){
  soundExt->Var2[0]=fabs((eng->Force/35000.0)*100.0);
  soundExt->Var3[0]=(eng->BrakeForce/MAX_EDT_FORCE)*100.0;
 };

 //Ventilators
 if(Flags&2048){
  if(!CurrentOn||!(Flags&4096))
   if(soundExt){
    soundExt->PostTrigger(104);
    Flags&=~2048;
   };
 };
 if((Flags&4096)&&!(Flags&2048)){
  if(soundExt)
   if(CurrentOn){
    soundExt->PostTrigger(103);
    Flags|=2048;
   };
 };


 //Auxiliary compressor
 if((Flags&512)||CurrentOn){
  if(eng->var[4]<5.5)
   Flags|=1024;
  if(eng->var[4]>=7.0)
   Flags&=~1024;
  if(Flags&1024){
   eng->var[4]+=(AUX_COMPRESSOR_RATE*(7.1-eng->var[4])/1.6)*time;
  };
 };

 //Brakes
 if(Flags&16){
  if(eng->UR>loco->MainResPressure)
   eng->UR=loco->MainResPressure;
  eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*PIPE_CHARGE_Q;
  if(eng->TrainPipeRate<0.0)
   eng->TrainPipeRate=0.0;
 };
 eng->ChargingRate=(CHARGE_PIPE_VALUE-loco->ChargingPipePressure)*PIPE_CHARGE_Q;
 if(eng->ChargingRate<0.0||loco->ChargingPipePressure>loco->MainResPressure)
  eng->ChargingRate=0.0;

};





extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{

 unsigned long *Flags=(unsigned long *)&eng->var[0];
 eng->var[2]=float(GetTickCount())/1000.0;
 bool CurrentOn=loco->LineVoltage>19000.0&&loco->LineVoltage<30000.0;
 if(loco->LibParam==0||loco->LibParam==2){
  CurrentOn=false;
  for(UINT i=0;i<loco->NumSlaves;i++)
   if(((ElectricEngine *)loco->SlaveLoco(i)->Eng())->MainSwitch)
    CurrentOn=true;
  if(CurrentOn)
   CurrentOn=loco->LineVoltage>19000.0&&loco->LineVoltage<30000.0;
 }else if(loco->LibParam==1||loco->LibParam==3){
  if(!eng->MainSwitch)
   CurrentOn=false;
 };

 //Compressor
 eng->MainResRate=0.0;
 if(loco->MainResPressure>3.0){
  eng->MainResRate=-0.005;
 };
 if(loco->MainResPressure<5.0){
  if(!(*Flags&64))
   loco->PostTriggerEng(12);
  *Flags|=64;
 }else if(loco->MainResPressure>7.5){
  if(*Flags&64)
   loco->PostTriggerEng(13);
  *Flags&=~64;
 };
 if(!CurrentOn){
  if(*Flags&64)
   loco->PostTriggerEng(13);
  *Flags&=~64;
 };
 if(*Flags&64)
  eng->MainResRate=COMPRESSOR_CHARGE_RATE;

 //Door air usage
 if(*Flags&128){
  float &pres=(float)loco->ChargingPipePressure;
  pres-=DOOR_CHARGE_RATE*time;
  eng->var[6]-=time;
  if(eng->var[6]<0.0)
   *Flags&=~128;
 };


 if(loco->LibParam==0||loco->LibParam==2)
  RunHead(eng,loco,time,*Flags,State);
 else if(loco->LibParam==1||loco->LibParam==3)
  RunMotor(eng,loco,time,*Flags);
};
