//---------------------------------------------------------------------------

#define RTS_STACKSIZE 15
//#define RTS_ADAPTER_COMPLY

#include <windows.h>
#include <math>
#include "ts.h"
#include "vl80k_aux.h"

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


/*FreeAnimation *FindAnim(Cabin *cab,wchar_t *name){
 for(UINT i=0;i<cab->NumAnim;i++)
  if(!lstrcmpiW(name,cab->Anims[i].name))
   return &cab->Anims[i];
 return NULL;
};

FreeAnimation *FindAnim(const Locomotive *loco,wchar_t *name){
 for(USHORT i=0;i<loco->NumAnim;i++)
  if(!lstrcmpiW(name,loco->Anims[i].name))
   return &loco->Anims[i];
 return NULL;
};*/


//---------------------------------------------------------------------------



bool IsHP(UINT Position){
 if(
  Position==0||Position%4==1
 )
  return true;
 return false;
};

UINT IsLocoOn(ElectricEngine *eng,Cabin *cab){
 UINT LocoOn=0;
 if(eng->var[10]<BATTERY_DEADLINE)
  return 0;
 if(!cab->SwitchSub(0,0))
  LocoOn|=1;
 if(!cab->SwitchSub(0,2))
  LocoOn|=2;
 if(!cab->SwitchSub(0,1))
  LocoOn|=4;
 if(cab->Switch(18))
  LocoOn|=8;
 if(cab->Switch(14))
  LocoOn|=16;
 return LocoOn;
};

inline float ComputeForce(UINT &Throttle,float &VelMax,float &diff){
 float q=0.0,d=0.0;
 /*switch(Throttle){
  case 1:q=295000.0;d=-65000;break;
  case 2:q=455000.0;d=-80000;break;
  case 3:q=535000.0;d=-80000;break;
  case 4:q=540500.0;d=-70000;break;
  case 5:q=580000.0;d=-65000;break;
  case 6:q=630000.0;d=-65000;break;
  case 7:q=755000.0;d=-65000;break;
  case 8:q=835000.0;d=-75000;break;
  case 9:q=900000.0;d=-75000;break;
  case 10:q=942000.0;d=-75000;break;
  case 11:q=1024000.0;d=-80000;break;
  case 12:q=1100000.0;d=-85000;break;
  case 13:q=1115000.0;d=-85000;break;
  case 14:q=1100000.0;d=-85000;break;
  case 15:q=1100000.0;d=-85000;break;
  case 16:q=1125000.0;d=-85000;break;
  case 17:q=1125000.0;d=-75000;break;
  case 18:q=1125000.0;d=-65000;break;
  case 19:q=1100000.0;d=-50025;break;
  case 20:q=1100000.0;d=-50000;break;
  case 21:q=1100000.0;d=-45000;break;
  case 22:q=1100000.0;d=-40000;break;
  case 23:q=1100000.0;d=-37500;break;
  case 24:q=1100000.0;d=-35000;break;
  case 25:q=1100000.0;d=-33750;break;
  case 26:q=1100000.0;d=-32500;break;
  case 27:q=1100000.0;d=-32500;break;
  case 28:q=1100000.0;d=-32500;break;
  case 29:q=1100000.0;d=-32500;break;
  case 30:q=1100000.0;d=-30000;break;
  case 31:q=1100000.0;d=-27500;break;
  case 32:q=1100000.0;d=-25000;break;
  case 33:q=1150000.0;d=-22500;break;
 };*/
 if(VelMax<1.0)
  q=100000.0/VelMax;
 else
  q=100000.0/(VelMax*VelMax);
 if(q<0.0)q=0.0;

 return q;
};


void SwitchGV(const ElectricLocomotive *loco,ElectricEngine *eng,UINT State){
 FreeAnimation *anim;
 if(!loco||!eng)
  return;
 if(eng->MainSwitch==(unsigned char)State)
  return;
 //UINT *Flags=(UINT *)&eng->var[0];
 //UINT *BackSecFlags=NULL;
 //if(loco->NumSlaves)
  //BackSecFlags=(UINT *)&loco->Slaves[0]->locoA->var[0];

 if(eng->MainSwitch!=signed(State))
  if(loco->Flags&4)
   if(eng->sound)
    eng->sound->PostTrigger(103);
 eng->MainSwitch=State;

 anim=loco->FindAnim(L"GV");
 if(anim)
  anim->AnimateTo=State;
};

void SwitchLights(const Locomotive *loco,int Condition){
 switch(Condition){
  case 20:
   loco->SwitchLight(0,false,0.0,0);
   loco->SwitchLight(1,false,0.0,0);
   loco->SwitchLight(2,false,0.0,0);
   loco->SwitchLight(3,false,0.0,0);
   loco->SwitchLight(4,false,0.0,0);
   loco->SwitchLight(5,false,0.0,0);
   loco->SwitchLight(6,false,0.0,0);
   loco->SwitchLight(7,false,0.0,0);
  break;
  case 0:
   loco->SwitchLight(0,false,0.0,0);
   loco->SwitchLight(1,false,0.0,0);
   loco->SwitchLight(6,false,0.0,0);
   loco->SwitchLight(7,false,0.0,0);
   /*
   loco->lights[0].Flags&=~8;
   loco->lights[1].Flags&=~8;
   loco->lights[2].Flags&=~8;
   loco->lights[3].Flags&=~8;
   loco->lights[4].Flags&=~8;
   loco->lights[5].Flags&=~8;
   loco->lights[6].Flags&=~8;
   loco->lights[7].Flags&=~8;
   */
  break;
  case 1:
   loco->SwitchLight(0,true,0.0,0);
   loco->SwitchLight(1,false,0.0,0);
   loco->SwitchLight(6,true,0.0,0);
   loco->SwitchLight(7,false,0.0,0);
   /*
   loco->lights[0].Flags|=8;
   loco->lights[1].Flags&=~8;
   loco->lights[2].Flags|=8;
   loco->lights[3].Flags|=8;
   loco->lights[4].Flags&=~8;
   loco->lights[5].Flags&=~8;
   loco->lights[6].Flags|=8;
   loco->lights[7].Flags&=~8;
   */
  break;
  case 2:
   loco->SwitchLight(0,false,0.0,0);
   loco->SwitchLight(1,true,0.0,0);
   loco->SwitchLight(6,false,0.0,0);
   loco->SwitchLight(7,true,0.0,0);
   /*
   loco->lights[0].Flags&=~8;
   loco->lights[1].Flags|=8;
   loco->lights[2].Flags&=~8;
   loco->lights[3].Flags&=~8;
   loco->lights[4].Flags|=8;
   loco->lights[5].Flags|=8;
   loco->lights[6].Flags&=~8;
   loco->lights[7].Flags|=8;
   */
  break;
  case 3:
   loco->SwitchLight(14,true,0.0,0);
   //loco->lights[14].Flags|=8;
  break;
  case 4:
   loco->SwitchLight(14,false,0.0,0);
   //loco->lights[14].Flags&=~8;
  break;
  //Left
  case 5:
   loco->SwitchLight(2,true,0.0,0xffffae5f);
  break;
  case 6:
   loco->SwitchLight(2,true,0.0,0xf0ff0000);
  break;
  case 7:
   loco->SwitchLight(2,false,0.0,0);
  break;
  //Right
  case 8:
   loco->SwitchLight(3,true,0.0,0xffffae5f);
  break;
  case 9:
   loco->SwitchLight(3,true,0.0,0xf0ff0000);
  break;
  case 10:
   loco->SwitchLight(3,false,0.0,0);
  break;
 };
};

/*

 Stack Variables

 1 - unsigned long Flags
  1bite
   1bit - is on PP
   2bit - Compressor On
   3bit - protection incurred
   4bit - MV1
   5bit - MV2
   6bit - Compressor enabled(for slave section)
   7bit - enable train pipe pressure maintenance
   8bit - battery charging

  2bite
   1bit - brake applied(SMS)
   2bit - MV stream started
   3bit - Sanding
   4bit - battery down
   5bit - RZ
   6bit - enable radiostation on start



 2 - throttle switch counter

 3 - [VACANT]

 4 - main timer

 5 - Sanding timer

 6 - EPK Timer

 7 - Previous Signal Aspect

 8 - EPK State

 9 - EPK Timer2(blinkers)

 10 - Battery Charge

 11 - Battery Charging Rate

 12 - previous force

 13 - previous line voltage

 14 - RZ timer

 15 - Wheelslip blinker timer

 Sound Triggers

 4,5 - sander on/off
 14 - brake applied
 54 - brake released
 45 - MV1 start
 46 - MV1 release
 56,57 - EPK sound on,off
 101 - Compressor start
 102 - Compressor release
 103 - GV
 104 - radio on
 105 - radio off
 106 - EPK reset
 107,108 - door open/close


*/


extern "C" bool __export Init
 (ElectricEngine *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 UINT *Flags=(UINT *)&eng->var[0];
 Cabin *cab=eng->cab;
 loco->HandbrakeValue=0.0;
 eng->HandbrakePercent=0;
 eng->DynamicBrakePercent=0;
 eng->Sanding=0;
 eng->BrakeSystemsEngaged=1;
 eng->BrakeForce=0.0;
 eng->var[0]=0;
 eng->var[1]=0;
 eng->ChargingRate=0;
 eng->TrainPipeRate=0;
 eng->var[3]=GetTickCount();
 eng->var[4]=0.0;
 eng->AuxilaryRate=0.0;
 eng->var[5]=0.0;
 eng->var[6]=0.0;
 eng->var[7]=0.0;
 eng->var[10]=40.0;
 eng->var[12]=0.0;
 eng->var[13]=0.0;
 switch(State&0xFF){
  case 0:
   //Loco is static, without wagons
   loco->MainResPressure=3.0;
   if(!((State>>8)&1)){
    cab->Switches[3].SetState=1;
    cab->Switches[3].State=1;
   }else{
    loco->LocoFlags|=1;
   };
   loco->TrainPipePressure=4.0;
   loco->AuxiliaryPressure=5.2;
   eng->UR=4.0;
   eng->Power=0;
   eng->Force=0;
   loco->BrakeCylinderPressure=2.0;
   eng->IndependentBrakeValue=4.0;
   eng->TrainPipeRate=0.0;
   eng->ThrottlePosition=0;
   eng->Reverse=0;
   eng->ALSNOn=0;
   eng->MainSwitch=0;
   eng->Panto=0;
  break;
  case 1:
   //Loco is static, with wagons
   if((State>>8)&1){
    loco->LocoFlags|=1;
   };
   cab->SetSwitch(3,2,true);
   cab->SetSwitch(0,0,0,true);
   cab->SetSwitch(0,1,0,true);
   cab->SetSwitch(0,2,0,true);
   loco->MainResPressure=7.0;
   loco->TrainPipePressure=5.0;
   loco->AuxiliaryPressure=5.2;
   eng->UR=4.0;
   eng->Power=0;
   eng->Force=0;
   loco->BrakeCylinderPressure=0.0;
   eng->IndependentBrakeValue=4.0;
   eng->TrainPipeRate=0.0;
   eng->ThrottlePosition=0;
   eng->Reverse=0;
   eng->ALSNOn=0;
   eng->MainSwitch=0;
   eng->Panto=0;
   *Flags|=32<<8;
  break;
  case 2:
  case 3:
   //Loco is moving
   if((State>>8)&1){
    loco->LocoFlags|=1;
    if((loco->Velocity<0.0) ^ (loco->LibParam==1)){
     cab->SetSwitch(1,1,false);
    }else{
     cab->SetSwitch(1,3,false);
    };
   };
   if((loco->Velocity<0.0) ^ (loco->LibParam==1)){
     eng->Reverse=-1;
   }else{
     eng->Reverse=1;
   };
   if(loco->Flags&1)
    eng->Reverse=-eng->Reverse;

   cab->SetSwitch(2,1,true);
   cab->SetSwitch(3,1,true);
   cab->SetSwitch(9,1,true);
   cab->SetSwitch(10,1,true);
   cab->SetSwitch(11,1,true);
   cab->SetSwitch(12,1,true);
   cab->SetSwitch(13,1,true);
   cab->SetSwitch(14,1,true);
   cab->SetSwitch(15,1,true);
   cab->SetSwitch(18,1,true);
   cab->SetSwitch(19,1,true);
   cab->SetSwitch(21,1,true);
   cab->SetSwitch(23,1,true);
   cab->SetSwitch(0,0,0,true);
   cab->SetSwitch(0,1,0,true);
   cab->SetSwitch(0,2,0,true);
   loco->MainResPressure=8.5;
   loco->TrainPipePressure=5.2;
   loco->AuxiliaryPressure=5.0;
   eng->UR=5.0;
   eng->Power=0;
   eng->Force=0;
   loco->BrakeCylinderPressure=0.0;
   eng->IndependentBrakeValue=0.0;
   eng->TrainPipeRate=0.0;
   eng->ThrottlePosition=0;
   eng->ALSNOn=0;
   eng->MainSwitch=1;
   eng->Panto=1;
   eng->var[13]=25000.0;
   *Flags|=32<<8;
  break;
 };

 /*
 //Getting track items
 TrackItemsItem *tritems;
 UINT Count;
 loco->GetTrackItems(7,40000.0,tritems,Count);
 for(UINT i=0;i<Count;i++){
  if(tritems[i].obj->Type==TIT_PLATF)
   MessageBoxW(NULL,((PlatformItem *)tritems[i].obj)->PlatformName,L"Platform",MB_OK);
 };
 */

 return true;

};

extern "C" void __export  ChangeLoco
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{

 if(!Prev)
  loco->LocoFlags|=1;
 else if(!Prev->locoA->cab->Switch(18)||Prev->locoA->cab->SwitchSub(0,0)||
  Prev->locoA->cab->SwitchSub(0,1)
 ){
  if(!Prev->locoA->cab->Switch(1))
   loco->LocoFlags|=1;
 };

 /*if(!(loco->LocoFlags&1)&&loco->locoA->sound){
  ULONG *Flags=(ULONG *)&loco->locoA->var[0];

 };*/

};

extern "C" void __export  LostMaster
(Locomotive *loco,const Locomotive *Prev,unsigned long State)
{
 UINT &Flags=*(UINT *)&loco->locoA->var[0];
 Flags&=~1272;
};


extern "C" bool __export CanWorkWith(const Locomotive *loco,const wchar_t *Type){

 if(!lstrcmpiW(Type,L"vl80k"))
  return true;

 return false;
};


extern "C" bool __export  CanSwitch(const ElectricLocomotive *loco,const ElectricEngine *eng,
        unsigned int SwitchID,unsigned int SetState)
{

 switch(SwitchID){
  case 0:
   eng->var[1]=0;
   if(eng->sound)
    eng->sound->PostTrigger(16);
  break;
  case 1:
   UINT *Flags;
   Flags=(UINT *)&eng->var[0];
   if(SetState>3&&((*Flags&1)||!eng->ThrottlePosition))
    return false;
   if(eng->ThrottlePosition&&SetState<3)
    return false;
   if(eng->cab->Switch(0)>1&&SetState<3)
    return false;
   if(eng->sound)
    eng->sound->PostTrigger(15);
  break;
  case 2:
   if(eng->sound)
    eng->sound->PostTrigger(17);
  break;
  case 3:
   if(eng->sound)
    eng->sound->PostTrigger(17);
  break;
  case 7:
   if(SetState&&eng->sound)
    eng->sound->PostTrigger(48);
  break;
  case 42:
   if(SetState&&eng->sound)
    eng->sound->PostTrigger(48);
  break;
 };

 if(eng->sound){
  if(SwitchID>7&&SwitchID!=42)
   eng->sound->PostTrigger(26);
 };

 return true;

};


extern "C" void __export Switched(const ElectricLocomotive *loco,ElectricEngine *eng,
        unsigned int SwitchID,unsigned int PrevState)
{

 ElectricLocomotive *BackSec=NULL;
 if(loco->NumSlaves)
  BackSec=(ElectricLocomotive *)loco->Slaves[0];
 UINT *Flags=(UINT *)&eng->var[0],*BackSecFlags;
 if(BackSec)
  BackSecFlags=(UINT *)&BackSec->loco->var[0];
 FreeAnimation *anim;
 UINT iv,LocoOn=IsLocoOn(eng,eng->cab);

 if(SwitchID==28)
  SwitchID=6;
 else if(SwitchID==29)
  SwitchID=5;
 else if(SwitchID==32)
  SwitchID=27;
 else if(SwitchID==34)
  SwitchID=25;
 else if(SwitchID==39)
  SwitchID=35;
 else if(SwitchID==40)
  SwitchID=36;
 else if(SwitchID==42)
  SwitchID=7;


 switch(SwitchID){
  case 0:
   if(!(loco->LocoFlags&1))
    break;
   if((LocoOn&13)!=13)
    break;
   if(eng->cab->Switches[1].State>3)
    break;
   if(eng->cab->Switches[0].State<3){
    eng->var[1]=GetTickCount();
   }else if(eng->cab->Switches[0].State==3){
    eng->var[1]=0;
    if(PrevState==4&&eng->ThrottlePosition){
     eng->ThrottlePosition--;
    };
   }else if(eng->cab->Switches[0].State==6){
    eng->var[1]=0;
    if(PrevState==5&&eng->ThrottlePosition<33){
     eng->ThrottlePosition++;
    };
   }else if(eng->cab->Switches[0].State==7){
    eng->var[1]=GetTickCount();
   }else
    eng->var[1]=0;

   if(BackSec)
    iv=IsLocoOn(BackSec->loco,BackSec->cab);

   if(IsHP(eng->ThrottlePosition)){
    *Flags&=~1;
    if(BackSec&&((iv&5)==5))
     *BackSecFlags&=~1;
   }else{
    *Flags|=1;
    if(BackSec&&((iv&5)==5))
     *BackSecFlags|=1;
   };

   if(BackSec&&((iv&5)==5))
    BackSec->loco->ThrottlePosition=eng->ThrottlePosition;
  break;
  case 1:
   if(!(loco->LocoFlags&1))
    break;
   switch(eng->cab->Switches[1].State){
    case 0:eng->Reverse=0;break;
    case 2:eng->Reverse=0;break;
    case 1:eng->Reverse=loco->LibParam==0?-1:1;break;
    case 3:eng->Reverse=loco->LibParam==0?1:-1;break;
   };
   //if(loco->Flags&1)
    //eng->Reverse=-eng->Reverse;
   if(BackSec&&((iv&5)==5)){
    BackSec->loco->Reverse=eng->Reverse;
   };
  break;
  case 2:
   if(eng->sound)
    eng->sound->PostTrigger(17);
   if(!(loco->LocoFlags&1))
    return;
   switch(eng->cab->Switches[2].State){
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
   if(BackSec)
    BackSec->loco->IndependentBrakeValue=eng->IndependentBrakeValue;
  break;
  case 3:
  break;
  case 4:

  break;
  case 5:
   iv=eng->cab->Switch(5)||eng->cab->Switch(29)?10:11;
   if((LocoOn&13)!=13)iv=11;
   if(eng->sound)
    eng->sound->PostTrigger(iv);
   if(loco->sound)
    loco->sound->PostTrigger(iv);
  break;
  case 6:
   iv=eng->cab->Switch(6)||eng->cab->Switch(28)?8:9;
   if((LocoOn&13)!=13)iv=9;
   if(eng->sound)
    eng->sound->PostTrigger(iv);
   if(loco->sound)
    loco->sound->PostTrigger(iv);
  break;
  case 7:
   eng->var[5]=0.0;
  break;
  case 8:

  break;
  case 16:
   if(eng->cab->Switches[16].State&&(LocoOn&1))
    SwitchLights((Locomotive *)loco,2);
   else
    if(eng->cab->Switches[17].State&&(LocoOn&1))
     SwitchLights((Locomotive *)loco,1);
    else
     SwitchLights((Locomotive *)loco,0);
  break;
  case 17:
   if(eng->cab->Switch(17)&&(LocoOn&1))
    if(eng->cab->Switch(16))
     SwitchLights((Locomotive *)loco,2);
    else
     SwitchLights((Locomotive *)loco,1);
   else
    SwitchLights((Locomotive *)loco,0);
  break;
  case 18:
   if((loco->LocoFlags&1)&&((LocoOn&13)!=13)){
    SwitchGV(loco,eng,0);
    //eng->Panto=0;
    if(BackSec){
     SwitchGV(BackSec,BackSec->loco,0);
     //BackSec->loco->Panto=0;
    };
   };
   /*if(BackSec){
    if((LocoOn&13)==13)
     BackSec->SwitchLight(14,true);
    else
     BackSec->SwitchLight(14,false);
   };*/
  break;
  case 22:
   if(!(loco->LocoFlags&1))
    return;
   if((LocoOn&13)!=13)
    return;
   if(eng->ThrottlePosition){
    *Flags|=4;
    return;
   };
   if(eng->cab->Switches[23].State){
    SwitchGV(loco,eng,1);
    *Flags&=~4;
    if(BackSec)SwitchGV(BackSec,BackSec->loco,1);
   };
  break;
  case 23:
   if(!(loco->LocoFlags&1))
    return;
   if((LocoOn&13)!=13)
    return;
   if(!eng->cab->Switches[23].State){
    SwitchGV(loco,eng,0);
    if(BackSec)SwitchGV(BackSec,BackSec->loco,0);
   };
  break;
  case 24:
   anim=eng->cab->FindAnim(L"wiper");
   //anim=&eng->cab->Anims[0];
   if(anim){
    if(eng->cab->Switches[24].State&&(LocoOn&1))
     anim->Flags|=1;
    else
     anim->Flags&=~1;
   };
   anim=loco->FindAnim(L"wiper");
   if(anim){
    if(eng->cab->Switches[24].State&&(LocoOn&1))
     anim->Flags|=1;
    else
     anim->Flags&=~1;
   };
  break;
  case 25:
   if(eng->cab->NumTexChanges){
    iv=1;
    if(!eng->cab->Switch(25)&&!eng->cab->Switch(34))
     iv=0;
    if(!(LocoOn&1))
     iv=0;
    eng->cab->Tex->SetState=iv;
   };
  break;
  case 26:
   if(eng->cab->NumLights>2){
    iv=eng->cab->Switches[26].State;
    if(!(LocoOn&1))iv=0;
    eng->cab->Light[1].State=iv;
    eng->cab->Light[2].State=iv;
   };
  break;
  case 27:
   if(eng->cab->NumLights){
    iv=eng->cab->Switch(32);
    iv+=eng->cab->Switch(27)*2;
    if(!(LocoOn&1))iv=0;
    eng->cab->SetLightState(0,iv,0,iv<2?1.0:0.7);
   };
  break;
  case 35:
   if((LocoOn&1)&&eng->cab->Switch(35)){
    if(eng->cab->Switch(39))
     SwitchLights(loco,5);
    else
     SwitchLights(loco,6);
   }else
    SwitchLights(loco,7);
  break;
  case 36:
   if((LocoOn&1)&&eng->cab->Switch(36)){
    if(eng->cab->Switch(40))
     SwitchLights(loco,8);
    else
     SwitchLights(loco,9);
   }else
    SwitchLights(loco,10);
  break;
  case 200:
   Switched(loco,eng,16,0);
   Switched(loco,eng,17,0);
   Switched(loco,eng,24,0);
   Switched(loco,eng,25,0);
   Switched(loco,eng,26,0);
   Switched(loco,eng,27,0);
   Switched(loco,eng,35,0);
   Switched(loco,eng,36,0);
   if(eng->cab->SwitchSub(0,0)||!(LocoOn&1)){
     if(eng->sound) eng->sound->PostTrigger(105);
   }else{
     if(eng->sound) eng->sound->PostTrigger(104);
   };
  break;
 };
};



extern "C" void __export Run
 (ElectricEngine *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{

 ElectricLocomotive *BackSec=NULL;
 if(loco->NumSlaves&&(loco->LocoFlags&1))
  BackSec=(ElectricLocomotive *)loco->Slaves[0];

 UINT ctime;
 UINT LocoOn=0,BackOn,v1;
 Cabin *cab=eng->cab,*cabBack=NULL;
 /*
 LocoOn
 1bite
  1bit - battery on
  2bit - charging on
  3bit - control circuits
  4bit - control circuits switch
  5bit - compressor is on

  if the battery charge is low LocoOn==0
 */

 UINT *Flags=(UINT *)&eng->var[0],*BackSecFlags;
 float Pressure,Compressor=1.0,Vel;
 if(BackSec){
  BackSecFlags=(UINT *)&BackSec->loco->var[0];
  *BackSecFlags&=~32;
  cabBack=BackSec->cab;
  BackOn=IsLocoOn(BackSec->loco,cabBack);
 };

 if(loco->LocoFlags&1){
  eng->Power=0;
  eng->Force=0;
  if(loco->NumEngines){
   eng->EngineForce[0]=0;
   eng->EngineVoltage[0]=0;
   eng->EngineCurrent[0]=0;
  };
  if(BackSec){
   BackSec->loco->Power=0;
   BackSec->loco->Force=0;
   BackSec->loco->BrakeForce=0.0;
   if(BackSec->NumEngines){
    BackSec->loco->EngineForce[0]=0;
    BackSec->loco->EngineVoltage[0]=0;
    BackSec->loco->EngineCurrent[0]=0;
   };
  };
 };

 LocoOn=IsLocoOn(eng,cab);


 if(loco->LocoFlags&1){

  //Rasing pantographs on both sections
  v1=0; eng->Panto=0;
  if((LocoOn&1)&&cab->Switch(21)){
   v1|=4;
   if(cab->Switch(20))
    v1|=loco->LibParam?1:2;
   if(cab->Switch(19))
    v1|=loco->LibParam?2:1;
  };
  if(cabBack){
   BackSec->loco->Panto=0;
   if((BackOn&1)&&cabBack->Switch(21)){
    if(cabBack->Switch(20))
     v1|=BackSec->LibParam?1:2;
    if(cabBack->Switch(19))
     v1|=BackSec->LibParam?2:1;
   };
   if(BackOn&1)
    BackSec->loco->Panto|=v1&3;
  };
  eng->Panto|=v1&3;

  //Enabling MV1,MV2,MV3,MV4
  if(cab->Switch(13))
   *Flags|=8;
  else
   *Flags&=~8;
  if(cab->Switch(12))
   *Flags|=16;
  else
   *Flags&=~16;
  if(BackSec){
   if(cab->Switch(11))
    *BackSecFlags|=8;
   else
    *BackSecFlags&=~8;
   if(cab->Switch(10))
    *BackSecFlags|=16;
   else
    *BackSecFlags&=~16;
  };
 };

 


 //Swtching GV off, if the circuits are off
 if(((LocoOn&5)!=5)&&eng->MainSwitch){
  SwitchGV(loco,eng,0);
  if(BackSec)
   SwitchGV(BackSec,BackSec->loco,0);
 };

 //Determining power source: catenary or socket
 float LineVoltage=loco->LineVoltage;
 bool CurrentOn=LineVoltage>19000,ExtSrc=false;
 if(!CurrentOn&&!eng->MainSwitch){
  if(eng->ExternalPowerSource){
   ExtSrc=true;
   LineVoltage=eng->ExternalPowerSourceVoltage;
   CurrentOn=true;
   Compressor*=eng->ExternalPowerSource/40000.0;
  };
 };

 //GV off due to low voltage or DC
 if(eng->MainSwitch&&((!loco->LineFreq||!CurrentOn)||LineVoltage>30000.0)&&loco->PantoRaised){
  SwitchGV(loco,eng,0);
  //if(BackSec)SwitchGV(BackSec,BackSec->loco,0);
  *Flags|=4;
 };

 //GV off due to line voltage drop or raise
 if(eng->MainSwitch&&
  ((eng->var[13]<19000.0&&LineVoltage>19000.0)||(eng->var[13]>19000.0&&LineVoltage<19000.0)))
 {
  SwitchGV(loco,eng,0);
  if(BackSec)SwitchGV(BackSec,BackSec->loco,0);
 };
 eng->var[13]=loco->LineVoltage;

 //GV off due to open doors in VVK
 if(eng->MainSwitch&&(cab->IsDoorOpened(5)||cab->IsDoorOpened(6)||cab->IsDoorOpened(7))){
  SwitchGV(loco,eng,0);
  *Flags|=4;
 };

 //Battery usage and charging
 eng->var[11]=0.0;
 if(LocoOn&1){
  *Flags&=~2048;
  if(loco->IsLightOn(0))
   eng->var[11]-=0.011;
  if(loco->IsLightOn(1))
   eng->var[11]-=0.024;
  if(cab->Switch(16))
   eng->var[11]-=0.001;
  if(cab->Switch(17))
   eng->var[11]-=0.0006;
  if(cab->Switch(27))
   eng->var[11]-=0.001;
  else if(cab->Switch(33))
   eng->var[11]-=0.0006;
  if(cab->Switch(30))
   eng->var[11]-=0.04;
  else if(cab->Switch(31))
   eng->var[11]-=0.028;
  if(cab->Switch(41)||cab->Switch(43))
   eng->var[11]-=0.008;
  if(cab->Switch(44))
   eng->var[11]-=0.01;
  if(cab->Switch(45))
   eng->var[11]-=0.01;
  if(cab->Switch(46))
   eng->var[11]-=0.01;
  if(cab->Switch(47))
   eng->var[11]-=0.01;

  if(LocoOn&4){
   eng->var[11]-=0.002;
   if((LocoOn&8)||eng->MainSwitch){
    eng->var[11]-=0.004;
    if(cab->Switch(9))
     eng->var[11]-=0.0008;
   };
  };

  if((LocoOn&2)){
   if(eng->MainSwitch&&CurrentOn){
    if(eng->var[10]<69.0||(*Flags&128)){
     if(eng->var[10]>74.2)
      *Flags&=~128;
     else{
      *Flags|=128;
      eng->var[11]=(74.2-eng->var[10])*0.004;
      if(eng->var[11]>0.15)
       eng->var[11]=0.15;
      if(eng->var[11]<0.0)
       eng->var[11]=0.0;
     };
    };
   }else if(ExtSrc&&eng->var[10]<62.0){
    eng->var[11]+=0.002*(62.0-eng->var[10]);
    if(eng->var[10]>61.0)
     eng->var[11]=0.0;
   };
  };
 }else if((LocoOn&2)&&eng->ExternalPowerSource&&eng->var[10]<62.0){
  eng->var[11]=0.005*(62.0-eng->var[10])/40.0;
 };
 eng->var[10]+=eng->var[11]*time;
 if(eng->var[10]<BATTERY_DEADLINE&&!(*Flags&2048)){
  Switched(loco,eng,200,0);
  *Flags|=2048;
 };

 if(*Flags&(32<<8)){
  if(eng->sound)
   eng->sound->PostTrigger(104);
  *Flags&=~(32<<8);
 };

 //AuxLibrary test
 AuxLibrary *lib_test=loco->GetAuxLibrary(L"AUX_TEST");
 if(lib_test){
  AuxLib_Context *lib_test_ctx =(AuxLib_Context *)lib_test->var;
  lib_test_ctx->On=LocoOn&1;   
 };


 //Автонабор/сброс
 if((loco->LocoFlags&1)&&((LocoOn&13)==13)&&eng->var[1]>1){
  ctime=GetTickCount()-eng->var[1];
  if(ctime>500.0&&eng->cab->SwitchSet(1)<4){
   if(ctime>1000.0)
    eng->var[1]=GetTickCount();
   else
    eng->var[1]+=ctime;
   switch(eng->cab->Switches[0].State){
    case 1:
     if(eng->ThrottlePosition)
      eng->ThrottlePosition--;
     else
      eng->var[1]=0;
    break;
    case 2:
     if(eng->ThrottlePosition)
      eng->ThrottlePosition--;
     else
      eng->var[1]=0;
    break;
    case 0:
     SwitchGV(loco,eng,0);
     if(eng->ThrottlePosition)
      eng->ThrottlePosition--;
     if(BackSec)
      SwitchGV(BackSec,BackSec->loco,eng->MainSwitch);
    break;
    case 7:
     if(eng->ThrottlePosition<33)
      eng->ThrottlePosition++;
    break;
   };

   if(IsHP(eng->ThrottlePosition)){
    *Flags&=~1;
    if(BackSec)
     *BackSecFlags&=~1;
   }else{
    *Flags|=1;
    if(BackSec)
     *BackSecFlags|=1;
   };

   if(BackSec&&((BackOn&5)==5)){
    BackSec->loco->ThrottlePosition=eng->ThrottlePosition;
   };
  };
 };

 //Возврат главного контроллера
 switch(cab->Switch(0)){
  case 0:
   if(!GetAsyncKeyState(cab->Switches[0].Key[2])&&!GetAsyncKeyState(VK_LBUTTON))
    cab->SetSwitch(0,1,false);
  break;
  case 7:
   if(!GetAsyncKeyState(cab->Switches[0].Key[0])&&!GetAsyncKeyState(VK_LBUTTON))
    cab->SetSwitch(0,6,false);
  break;
 };


 eng->MainResRate=0.0;
 if(loco->MainResPressure>2.4){
  if(cab->Switches[3].State<4)
   eng->MainResRate=-5e-4*loco->MainResPressure;
 };

 if(CurrentOn&&(eng->MainSwitch||ExtSrc)){

   if(BackSec){
    if(((LocoOn&29)==29))
     *BackSecFlags|=32;
   };

   //Compressor
   if((*Flags&32)||(((LocoOn&29)==29)&&(loco->LocoFlags&1))){
    if(!(*Flags&2)&&loco->MainResPressure<6.0){
     *Flags|=2;
     if(eng->sound)
      eng->sound->PostTrigger(101);
    };
    if(*Flags&2)
     if(loco->MainResPressure>9.0){
      *Flags&=~2;
      if(eng->sound)
       eng->sound->PostTrigger(102);
     }else{
      eng->MainResRate=0.04*(11.0-loco->MainResPressure);
      if(!loco->LineVoltage&&ExtSrc&&eng->ExternalPowerSource<40000.0){
       eng->MainResRate*=eng->ExternalPowerSource/40000.0;
      };
     };
   }else if(*Flags&2){
    if(eng->sound)
     eng->sound->PostTrigger(102);
    *Flags&=~2;
   };


  if(loco->LocoFlags&1){
   //Throttle
   if(eng->Reverse&&(cab->Switch(15)||ExtSrc)&&cab->Switch(0)>1)
   {

     float Velocity=loco->Velocity*eng->Reverse;
     float VelMax=-3.525+eng->ThrottlePosition*0.245;
     float SetForce=0.0;
     int shunt = 0;

     if(eng->cab->Switches[1].State>3){
      shunt =eng->cab->Switches[1].State-3;
     };

     if(Velocity==VelMax)Velocity+=0.1;
     Velocity=Velocity-VelMax;
     if(Velocity<=0.01)
      SetForce=400000.0;
     else{
      SetForce = (340000.0*eng->ThrottlePosition)/
                (Velocity*Velocity*pow(0.978,eng->ThrottlePosition)) +
                shunt*468750.0/Velocity;
     };
     SetForce*=eng->Reverse*(LineVoltage/26000.0);

     eng->Force=eng->var[12];
     if(eng->Force<SetForce){
      eng->Force+=(10000.0+(SetForce-eng->Force)*0.6)*time;
      if(eng->Force>SetForce)
       eng->Force=SetForce;
     }else if(eng->Force>SetForce){
      eng->Force-=30000.0*time;
      if(eng->Force<SetForce)
       eng->Force=SetForce;
     };
     eng->Power=eng->ThrottlePosition*28.8*(LineVoltage/26000.0)*fabs(eng->Force/TR_CURRENT_C)*4.0;

     if(!loco->LineVoltage&&ExtSrc){
      if(eng->Power&&eng->Power>eng->ExternalPowerSource)
       eng->Force*=eng->ExternalPowerSource/eng->Power;
       if(*Flags&2){
        eng->MainResRate*=0.55;
        eng->Power*=0.45;
        eng->Force*=0.45;
       };
     };

     if(loco->NumEngines){
      eng->EngineForce[0]=eng->Force/4.0;
      eng->EngineVoltage[0]=eng->ThrottlePosition*26.5*(LineVoltage/26000.0);
      eng->EngineCurrent[0]=fabs(eng->Force/TR_CURRENT_C);
      if(eng->EngineCurrent[0])
       eng->EngineCurrent[0]+=50.0;
      if(eng->cab->Switches[1].State>3)
       eng->EngineCurrent[0]*=1.0+(eng->cab->Switches[1].State-3)*0.05;
      //if(eng->Wheelslip)
       //eng->EngineCurrent[0]*=1.01+float(GetTickCount()%10)/200.0;
      if(eng->EngineCurrent[0]>1400.0||loco->BrakeCylinderPressure>3.0){
       SwitchGV(loco,eng,0);
       *Flags|=4;
       eng->Force=0;
       eng->Power=0;
       eng->EngineForce[0]=0;
       eng->EngineVoltage[0]=0;
       eng->EngineCurrent[0]=0;
       if(BackSec)
        SwitchGV(BackSec,BackSec->loco,0);
      };
     };

   };
  };
 }else{
   if(*Flags&2){
    *Flags&=~2;
    if(eng->sound)
     eng->sound->PostTrigger(102);
   };
 };
 eng->PowerConsuming=eng->Power;
 if((LocoOn&12)==12)
  eng->PowerConsuming+=500.0;
 if(*Flags&2)
  eng->PowerConsuming+=40000;
 eng->var[12]=eng->Force;
 

 if(BackSec){
  if(BackSec->loco->MainSwitch){
   BackSec->loco->Power=eng->Power;
   BackSec->loco->Force=eng->Force;
   if(BackSec->NumEngines){
    BackSec->loco->EngineForce[0]=eng->EngineForce[0];
    BackSec->loco->EngineCurrent[0]=eng->EngineCurrent[0];
    BackSec->loco->EngineVoltage[0]=eng->EngineVoltage[0];
   };
  };
 };

 if(loco->NumEngines){
  if(eng->sound){
   eng->sound->Var2[0]=(eng->EngineCurrent[0]/800.0)*100.0;
  };
  if(loco->sound){
   loco->sound->Var2[0]=(eng->EngineCurrent[0]/800.0)*100.0;
  };
  if(eng->EngineVoltage[0]>875.0){
   *Flags|=4096;
   eng->var[14]+=time*(eng->EngineVoltage[0]-875.0)*0.02;
   if(eng->var[14]>90.0){
    SwitchGV(loco,eng,0);
    if(BackSec) SwitchGV(BackSec,BackSec->loco,0);
    *Flags|=4;
   };
  }else{
   *Flags&=~4096;
   eng->var[14]=0.0;
  };
  eng->EngineForce[1]=eng->EngineForce[0];
  eng->EngineCurrent[1]=eng->EngineCurrent[0];
  eng->EngineVoltage[1]=eng->EngineVoltage[0];
  eng->EngineForce[2]=eng->EngineForce[0];
  eng->EngineCurrent[2]=eng->EngineCurrent[0];
  eng->EngineVoltage[2]=eng->EngineVoltage[0];
  eng->EngineForce[3]=eng->EngineForce[0];
  eng->EngineCurrent[3]=eng->EngineCurrent[0];
  eng->EngineVoltage[3]=eng->EngineVoltage[0];
 };


 //Loco brake
 if(eng->IndependentBrakeValue>loco->MainResPressure)
  eng->IndependentBrakeValue=loco->MainResPressure;
 if(eng->IndependentBrakeValue>loco->IndependentBrakePressure)
  eng->MainResRate-=0.02;

 //Train brake
 eng->TrainPipeRate=0.0;
 if(loco->LocoFlags&1){
  if(BackSec)*BackSecFlags&=~64;
  switch(cab->Switches[3].State){
   case 0:
    if(eng->var[5]<45.0){
     if(!cab->Switches[3].SetState)
      eng->UR+=BRAKE_UR_RATE_CHARGE*time;
     if(eng->UR>loco->MainResPressure)
      eng->UR=loco->MainResPressure;
     if(loco->TrainPipePressure<eng->UR)
      eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)*1.5;
     if(BackSec){
      BackSec->loco->UR=eng->UR;
      *BackSecFlags|=64;
     };
    };
   break;
   case 1:
    if(eng->var[5]<45.0){
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
      eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)/BRAKE_PIPE_RATE_CHARGE;
      if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
       eng->TrainPipeRate=-BRAKE_PIPE_RATE;
     };
     if(BackSec){
      BackSec->loco->UR=eng->UR;
      *BackSecFlags|=64;
     };
    };
   break;
   case 2:
    if(eng->UR>loco->MainResPressure)
     eng->UR=loco->MainResPressure;
    if(loco->TrainPipePressure>eng->UR)
     eng->TrainPipeRate=eng->UR-loco->TrainPipePressure;
    if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
     eng->TrainPipeRate=-BRAKE_PIPE_RATE;
    if(eng->TrainPipeRate>PIPE_DISCHARGE_SLOW)
     eng->TrainPipeRate=PIPE_DISCHARGE_SLOW;
    if(BackSec)
     BackSec->loco->UR=eng->UR;
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
    if(BackSec)
     BackSec->loco->UR=eng->UR;
   break;
   case 4:
    if(cab->Switches[3].SetState!=4)
     break;
    eng->UR-=0.3*time;
    if(eng->UR>loco->MainResPressure)
     eng->UR=loco->MainResPressure;
    if(eng->UR<0)
     eng->UR=0;
    eng->TrainPipeRate=-0.25;
    if(BackSec)
     BackSec->loco->UR=eng->UR;
    //if(eng->TrainPipeRate<-BRAKE_PIPE_RATE)
     //eng->TrainPipeRate=-BRAKE_PIPE_RATE;
   break;
   case 5:
    eng->UR+=BRAKE_PIPE_EMERGENCY*1.2*time;
    if(eng->UR>loco->MainResPressure)
     eng->UR=loco->MainResPressure;
    if(eng->UR<0)
     eng->UR=0;
    eng->TrainPipeRate=BRAKE_PIPE_EMERGENCY;
    if(BackSec)
     BackSec->loco->UR=eng->UR;
   break;
  };

  //EPK
  if(eng->Reverse && (LocoOn&13==13) &&
     (cab->Switch(9)||cab->Switch(38)) && (loco->Velocity>0.1||loco->Velocity<-0.1))
  {
   UINT Aspect=eng->cab->Signal.Aspect[0];
   if(Aspect!=unsigned(eng->var[6]) || eng->var[7]==2.0)
   {
    if(Aspect>unsigned(eng->var[6]) && Aspect!=SIGASP_BLOCK_OBSTRUCTED && Aspect>SIGASP_STOP_AND_PROCEED){
     eng->var[7]=0.0;
     eng->var[8]=0.0;
     if(eng->sound)
      eng->sound->PostTrigger(106);
    }else{
     if(eng->var[7]<2.0){
      eng->var[7]=2.0;
      eng->var[8]=0.01;
      if(Aspect==SIGASP_STOP_AND_PROCEED || Aspect==SIGASP_RESTRICTING)
       eng->var[5]=35.0-time*0.5;
      else
       eng->var[5]=30.0;
     };
     if(!eng->var[5]){
      eng->var[7]=0.0;
      eng->var[8]=0.0;
      loco->PostTriggerBoth(57);
     }else{
      if(eng->var[5]<35.0&&eng->var[5]+time>=35.0)
       loco->PostTriggerBoth(56);
      eng->var[5]+=time;
      if(eng->var[5]>45.0){
       eng->TrainPipeRate=BRAKE_PIPE_EMERGENCY;
      };
     };
    };
   }else if(Aspect>=SIGASP_APPROACH_1&&Aspect<SIGASP_CLEAR_1){
    eng->var[5]+=time;
    if(eng->var[5]>30.0){
     eng->var[8]=0.01;
     eng->var[7]=2.0;
    };
   }else if(Aspect<SIGASP_APPROACH_1 || Aspect==SIGASP_BLOCK_OBSTRUCTED){
    eng->var[5]+=time;
    if(eng->var[5]>30.0){
     if(eng->var[7]<1.0)
      eng->var[7]=1.0;
     if(eng->var[5]>=35.0&&eng->var[5]-time<35.0)
      loco->PostTriggerBoth(56);
     if(eng->var[5]>45.0){
      eng->TrainPipeRate=BRAKE_PIPE_EMERGENCY;
     };
    }else{
     if(eng->var[7])
      loco->PostTriggerBoth(57);
     eng->var[7]=0.0;
    };
   }else{
    if(eng->var[7])
     loco->PostTriggerBoth(57);
    eng->var[5]=0.0;
    eng->var[7]=0.0;
   };
   eng->var[6]=Aspect;
  }else{
   if(eng->var[7])
    loco->PostTriggerBoth(57);
   eng->var[5]=0.0;
   eng->var[7]=0.0;
  };

  //Sanding
  if(((LocoOn&13)==13)&&
   (eng->cab->Switches[8].State||eng->cab->Switches[4].State)
  )
   eng->Sanding=1;
  else
   eng->Sanding=0;
  if(BackSec){
   BackSec->loco->Sanding=eng->Sanding;
  };

 }else{
  if((*Flags&64)){
   if(eng->UR>loco->MainResPressure)
    eng->UR=loco->MainResPressure;
   if(eng->UR>loco->TrainPipePressure)
    eng->UR-=0.002*time;
   if(loco->TrainPipePressure<eng->UR)
    eng->TrainPipeRate=(eng->UR-loco->TrainPipePressure)/BRAKE_PIPE_RATE_CHARGE;
  };
 };

 if(eng->sound){
  if((loco->BrakeCylinderPressure>0.0||loco->IndependentBrakePressure>0.0)&&!(*Flags&256)){
   eng->sound->PostTrigger(14);
   if(loco->sound)
    loco->sound->PostTrigger(14);
   *Flags|=256;
  };
  if((*Flags&256)&&!loco->BrakeCylinderPressure&&!loco->IndependentBrakePressure){
   eng->sound->PostTrigger(54);
   if(loco->sound)
    loco->sound->PostTrigger(54);
   *Flags&=~256;
  };
 };

 //----Sounds adjustment:
 //Sanding
 if(eng->Sanding&&!(*Flags&1024)){
  *Flags|=1024;
  if(eng->sound)
   eng->sound->PostTrigger(4);
 }else if(!eng->Sanding&&(*Flags&1024)){
  *Flags&=~1024;
  if(eng->sound)
   eng->sound->PostTrigger(5);
 };
 //Turning on and off MV stream
 if(*Flags&512){
  if((!(*Flags&8)&&!(*Flags&16))||!eng->MainSwitch||!CurrentOn){
   if(eng->sound)
    eng->sound->PostTrigger(46);
   if(loco->sound)
    loco->sound->PostTrigger(46);
   *Flags&=~512;
  };
 }else{
  if(((*Flags&8)||(*Flags&16))&&eng->MainSwitch&&CurrentOn){
   if(eng->sound)
    eng->sound->PostTrigger(45);
   if(loco->sound)
    loco->sound->PostTrigger(45);
   *Flags|=512;
  };
 };
 Pressure=loco->BrakeCylinderPressure>loco->IndependentBrakePressure?
          loco->BrakeCylinderPressure:
          loco->IndependentBrakePressure;
 Pressure/=6.0;
 Vel=fabs(loco->Velocity);
 if(Vel<1.5)
  Pressure+=0.5*(1.5-Vel)*fabs(loco->Acceleration)/0.08;
 if(Vel<0.2)
  Pressure*=Vel*5;
 if(eng->sound){
  eng->sound->Var4[0]=Compressor;
  eng->sound->Var5[0]=Pressure;
 };
 if(loco->sound){
  loco->sound->Var4[0]=Compressor;
 };

 //Setting up the displays
 if((State>>8)&1){

  //ЩР
  if(LocoOn&1){
   if(cab->SwitchSub(0,4)){
    if(LocoOn&4){
     if(eng->MainSwitch&&CurrentOn&&eng->var[11]>=0.0)
      cab->SetSwitch(0,6,80.0,false);
     else
      cab->SetSwitch(0,6,eng->var[10],false);
    }else
     cab->SetSwitch(0,6,0.0,false);
   }else
    cab->SetSwitch(0,6,eng->var[10],false);
   cab->SetSwitch(0,7,eng->var[11]*10.0,false);
  }else{
   cab->SetSwitch(0,6,0.0,false);
   cab->SetSwitch(0,7,0.0,false);
  };

  cab->SetDisplayValue(0,Vel);
  if(eng->Wheelslip)
   cab->SetDisplayValue(0,Vel*(1.0+float(GetTickCount()%10)/50.0));

  if(loco->LineVoltage&&eng->MainSwitch)
   cab->SetDisplayValue(1,loco->LineVoltage);
  else
   cab->SetDisplayValue(1,0);

  if(loco->NumEngines)
   cab->SetDisplayValue(2,eng->EngineVoltage[0]);
  else
   cab->SetDisplayValue(2,eng->ThrottlePosition*40.0);

  if(BackSec){
   if(BackSec->NumEngines)
    cab->SetDisplayValue(3,BackSec->loco->EngineCurrent[0]);
   else
    cab->SetDisplayValue(3,BackSec->loco->Force/TR_CURRENT_C);
  }else
   cab->SetDisplayValue(3,0);

  if(loco->NumEngines)
   cab->SetDisplayValue(4,eng->EngineCurrent[0]);
  else
   cab->SetDisplayValue(4,eng->Force/TR_CURRENT_C);

  if(eng->ThrottlePosition>17)
   cab->SetDisplayValue(5,eng->ThrottlePosition+5);
  else if(eng->ThrottlePosition>13)
   cab->SetDisplayValue(5,eng->ThrottlePosition+2);
  else if(eng->ThrottlePosition)
   cab->SetDisplayValue(5,eng->ThrottlePosition+1);
  else
   cab->SetDisplayValue(5,0);

  cab->SetDisplayValue(7,loco->MainResPressure);
  cab->SetHint(7,-1);
  if(loco->MainResPressure<6.0)
   cab->SetHint(7,0);
  if(eng->MainResRate>0.0)
   cab->SetHint(7,1);

  cab->SetDisplayValue(
   6,
   loco->BrakeCylinderPressure>loco->IndependentBrakePressure?
   loco->BrakeCylinderPressure:
   loco->IndependentBrakePressure
  );
  if(!cab->DisplayValue(6))
   cab->SetHint(6,0);
  else
   cab->SetHint(6,-1);

  cab->SetDisplayValue(8,loco->TrainPipePressure);

  cab->SetDisplayValue(9,eng->UR);

  if(((LocoOn&13)==13)&&(loco->LocoFlags&1)){
   eng->cab->Displays[10].State=!(eng->MainSwitch&&CurrentOn&&(*Flags&24));
   eng->cab->Displays[11].State=!(eng->MainSwitch&&CurrentOn&&(*Flags&24));
   eng->cab->Displays[12].State=!(*Flags&1);
   eng->cab->Displays[13].State=!(eng->MainSwitch&&CurrentOn&&(*Flags&24));
   eng->cab->Displays[14].State=!eng->MainSwitch;
   if(BackSec){
    eng->cab->Displays[15].State=!(BackSec->loco->MainSwitch&&CurrentOn&&(*BackSecFlags&24));
    eng->cab->Displays[16].State=!(BackSec->loco->MainSwitch&&CurrentOn&&(*BackSecFlags&24));
    eng->cab->Displays[17].State=!(*BackSecFlags&1);
    eng->cab->Displays[18].State=!(BackSec->loco->MainSwitch&&CurrentOn&&(*BackSecFlags&24));
    eng->cab->Displays[19].State=!BackSec->loco->MainSwitch;
   }else{
    eng->cab->Displays[15].State=1;
    eng->cab->Displays[16].State=1;
    eng->cab->Displays[17].State=0;
    eng->cab->Displays[18].State=1;
    eng->cab->Displays[19].State=1;
   };
   if(eng->cab->Switch(9)||eng->cab->Switch(38))
    eng->ALSNOn=1;
   else
    eng->ALSNOn=0;
   if(eng->cab->Switch(37))
    eng->ALSNOn=0xFFFF;

   if(CurrentOn&&eng->MainSwitch&&(*Flags&24))
    cab->SetDisplayState(20,0);
   else
    cab->SetDisplayState(20,1);
   if(eng->MainSwitch&&eng->cab->Switches[15].State&&
        (!BackSec||BackSec->loco->MainSwitch)
   )
    eng->cab->Displays[21].State=1;
   else
    eng->cab->Displays[21].State=0;
   if(eng->var[11]<=0.0)
    eng->cab->Displays[22].State=1;
   else
    eng->cab->Displays[22].State=0;
   if(*Flags&4100)
    cab->SetDisplayState(23,1);
   else
    cab->SetDisplayState(23,0);
   if(eng->Wheelslip){
    cab->SetDisplayState(25,eng->var[15]>0.5?0:1);
    eng->var[15]+=time;
    if(eng->var[15]>1.0)
     eng->var[15]-=1.0;
   }else if(eng->Sanding&&loco->SandLeft>0.0){
    cab->SetDisplayState(25,1);
    eng->var[15]=0.0;
   }else{
    eng->var[15]=0.0;
    cab->SetDisplayState(25,0);
   };
  }else{
   eng->cab->Displays[10].State=0;
   eng->cab->Displays[11].State=0;
   eng->cab->Displays[12].State=0;
   eng->cab->Displays[13].State=0;
   eng->cab->Displays[14].State=0;
   eng->cab->Displays[15].State=0;
   eng->cab->Displays[16].State=0;
   eng->cab->Displays[17].State=0;
   eng->cab->Displays[18].State=0;
   eng->cab->Displays[19].State=0;
   eng->cab->Displays[20].State=0;
   eng->cab->Displays[21].State=0;
   eng->cab->Displays[22].State=0;
   eng->cab->Displays[23].State=0;
   eng->cab->Displays[25].State=0;
   eng->ALSNOn=0;
  };

  eng->cab->SetDisplayValue(29,5.0);
  if(LocoOn&1)
   eng->cab->SetDisplayValue(30,eng->var[10]);
  else
   eng->cab->SetDisplayValue(30,0.0);
  
  cab->SetHint(12,cab->DisplayState(12));
  cab->SetHint(17,cab->DisplayState(17));


  if((loco->LocoFlags&1)&&((LocoOn&13)==13)&&cab->Switch(0)<2)
   eng->cab->Displays[24].State=1;
  else
   eng->cab->Displays[24].State=0;

  //eng->cab->Displays[23].State=*Flags&4;

  //PSS
  if(eng->var[7]){
   if(eng->var[7]<2.0){
    eng->cab->Displays[26].State=1;
    eng->cab->Displays[27].State=1;
    eng->cab->Displays[28].State=0;
   }else{
    eng->var[8]+=time;
    if(eng->var[8]>2.0)eng->var[8]-=2.0;
    eng->cab->Displays[26].State=eng->var[8]>1.0?0:1;
    eng->cab->Displays[27].State=0;
    eng->cab->Displays[28].State=eng->var[8]>1.0?0:1;
   };
  }else{
   eng->cab->Displays[26].State=0;
   eng->cab->Displays[27].State=0;
   eng->cab->Displays[28].State=0;
  };

 };

};
