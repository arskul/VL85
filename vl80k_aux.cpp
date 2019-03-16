//---------------------------------------------------------------------------

#define RTS_ADAPTER_COMPLY
#define RTS_STACKSIZE sizeof(AuxLib_Context)/4+1

#include <windows.h>
#include <stdio.h>

#include <vl80k_aux.h>
#include <ts.h>



//---------------------------------------------------------------------------

#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
        return 1;
}
//---------------------------------------------------------------------------


wchar_t StartMsg[256];


extern "C" bool __export Init
 (AuxLibrary *eng,ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 AuxLib_Context *ctx=(AuxLib_Context *)eng->var;
 ctx->LightID[0]=eng->GetParameter(L"AuxTest_Light1",0.0);
 ctx->LightID[1]=eng->GetParameter(L"AuxTest_Light2",0.0);
 ctx->LightID[2]=eng->GetParameter(L"AuxTest_Light3",0.0);
 ctx->SwitchID  =eng->GetParameter(L"AuxTest_SwitchID",0.0);
 if(!ctx->LightID[0] && !ctx->LightID[1] && !ctx->LightID[2]){
  MessageBox(NULL,"Missing AUX_LIB parameters from eng-file","Failed to init library",MB_OK);
  return false;
 };

 swprintf(StartMsg,L"Welcome aboard VL80k\r\nOutside temperature is %0.00f",AirTemperature);
 loco->Eng()->ShowMessage(GMM_HINT,StartMsg);

 return true;
};



extern "C" void __export Switched(const ElectricLocomotive *loco,AuxLibrary *eng,
        unsigned int SwitchID,unsigned int PrevState)
{
 AuxLib_Context *ctx=(AuxLib_Context *)eng->var;
 if(ctx->On && SwitchID==ctx->SwitchID){
  Cabin *cab=eng->Loco()->Cab();
  ctx->StartedSound=cab->Switch(SwitchID);
  if(eng->sound){
   if(ctx->StartedSound){
    wchar_t *hint=eng->GetParameterAsString(L"AuxTest_ShowHint",NULL);
    if(hint)
     loco->Eng()->ShowMessage(GMM_HINT,hint);
    eng->PostTrigger(eng->sound->GetNamedTrigger(L"Aux on"));
   }else
    eng->PostTrigger(eng->sound->GetNamedTrigger(L"Aux off"));
  };
 };
};



extern "C" void __export Run
 (AuxLibrary *eng,const ElectricLocomotive *loco,unsigned long State,
        float time,float AirTemperature)
{
 AuxLib_Context *ctx=(AuxLib_Context *)eng->var;
 Cabin *cab=loco->Cab();
 if(ctx->On && cab->Switch(ctx->SwitchID)){
  ctx->Timer+=time;
  if(ctx->Timer>1.0)
   ctx->Timer-=1.0;
  if(ctx->Timer<0.5){
   loco->SwitchLight(ctx->LightID[0],true);
   loco->SwitchLight(ctx->LightID[1],false);
  }else{
   loco->SwitchLight(ctx->LightID[0],false);
   loco->SwitchLight(ctx->LightID[1],true);
  };
  if(ctx->Timer*3.0>=2.0)
   loco->SwitchLight(ctx->LightID[2],true);
  else
   loco->SwitchLight(ctx->LightID[2],false);
 };
 if(!ctx->On && ctx->StartedSound){
  if(eng->sound)
   eng->PostTrigger(eng->sound->GetNamedTrigger(L"Aux off"));
  ctx->StartedSound=false;
 };
};
