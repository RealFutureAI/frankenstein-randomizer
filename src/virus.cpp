#include <D2RLPlugin/api.h>
#include "ramp.h"
#include "ramp_native.h"
#include "pulse.h"
#include "pulse_native.h"
#include "curses.h"
#include "curses_native.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <windows.h>

namespace Virus {
struct Identity { unsigned type=0xffffffffU; unsigned id=0xffffffffU; };
struct Packet { void* game=nullptr; int event=0; int level=0; int skill=77; Identity corpse{},killer{},owner{}; };
struct Chain { std::array<Packet,4096> packets{}; unsigned size=0; unsigned next=0; };
using EventFn=int(__fastcall*)(void*,int,void*,void*,void*,int,int,std::uintptr_t,std::uintptr_t);
using FindFn=void*(__fastcall*)(void*,int,unsigned);
using CastFn=void(__fastcall*)(void*,int,int,void*,int);
static EventFn Original=nullptr;
static FindFn Find=nullptr;
static CastFn Cast=nullptr;
static const D2RL::PluginContext* Context=nullptr;
static thread_local Chain* Active=nullptr;
static unsigned Logs=0;
static void Log(const char* text) noexcept {if(Context && Context->api && Context->api->logInfo)Context->api->logInfo(Context,text);}
static bool ReadUnit(void* unit,Identity& out) noexcept {
 __try {if(!unit)return false;const auto p=static_cast<const unsigned*>(unit);out={p[0],p[2]};return out.type<=5;} __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool ReadOwner(std::uintptr_t pointer,Identity& out) noexcept {
 __try {if(!pointer)return false;const auto p=reinterpret_cast<const unsigned*>(pointer);out={p[0],p[1]};return out.type<=5;} __except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
static bool Enqueue(Chain& chain,const Packet& packet) noexcept {
 for(unsigned i=0;i<chain.size;++i){const auto& p=chain.packets[i];if(p.game==packet.game && p.corpse.type==packet.corpse.type && p.corpse.id==packet.corpse.id && p.skill==packet.skill)return true;}
 if(chain.size==chain.packets.size()){Log("Virus chain queue reached its 4096-corpse guard.");return false;}
 chain.packets[chain.size++]=packet;return true;
}
static int Process(const Packet& p) noexcept {
 void* corpse=Find(p.game,static_cast<int>(p.corpse.type),p.corpse.id);
 void* caster=Find(p.game,static_cast<int>(p.owner.type),p.owner.id);
 void* killer=Find(p.game,static_cast<int>(p.killer.type),p.killer.id);
 if(!corpse || !caster)return 0;
 if(!killer)killer=caster;
 // Apply the wide mark first. Its immediate poison pulse uses the player's
 // learned Terror/virus damage. Nested kills wait until the native proc lock
 // has been released, so explosion kills can continue the same chain.
 if(p.skill==77)Cast(caster,444,p.level,corpse,0);
 const unsigned owner[2]={p.owner.type,p.owner.id};
 const int result=Original(p.game,p.event,corpse,killer,nullptr,p.skill,p.level,0,reinterpret_cast<std::uintptr_t>(owner));
 if(Logs++<500){char text[192]{};std::snprintf(text,sizeof(text),"Virus death: corpse=%u level=%d infectionSkill=444 explosionResult=%d",p.corpse.id,p.level,result);Log(text);}
 return result;
}
static int __fastcall OnEvent(void* game,int event,void* source,void* target,void* damage,int skill,int level,std::uintptr_t a8,std::uintptr_t a9) noexcept {
 if(skill==70)return 1; // Resolved melee completion owns Plague Lord.
 if(skill==82){if(event==1 || event==2)Curses::ElementHit(game,source);return 1;}
 if(skill!=66 && skill!=86 && skill!=77 && skill!=444)return Original(game,event,source,target,damage,skill,level,a8,a9);
 Packet packet{};packet.game=game;packet.event=event;packet.level=level;packet.skill=skill==444?77:skill;
 if(!Find || !Cast || !ReadUnit(source,packet.corpse) || packet.corpse.type!=1 || !ReadOwner(a9,packet.owner) || packet.owner.type!=0 || level<1){
  return Original(game,event,source,target,damage,skill==444?77:skill,level,a8,a9);
 }
 if(!ReadUnit(target,packet.killer))packet.killer=packet.owner;
 if(Active){return Enqueue(*Active,packet)?1:0;}
 Chain chain{};
 Enqueue(chain,packet);Active=&chain;int first=0;
 while(chain.next<chain.size){const unsigned index=chain.next++;const int result=Process(chain.packets[index]);if(index==0)first=result;}
 Active=nullptr;return first;
}
}

#ifndef VIRUS_TEST
#include "native_probe.h"
extern "C" {__declspec(dllexport) void** FMMP_Curse_ObservedGame=&Curses::ObservedGame;}
static constexpr D2RL::PluginInfo Info={D2RL::PluginInfoSize,4,"fmmp-plague-trace","Frankenstein Curse Overhaul","0.5.13","Local mod customization","Native death-event adapter for corpse explosion infection chains",D2RL::PluginFlags::Server|D2RL::PluginFlags::NativeHooks|D2RL::PluginFlags::ModScopedOnly,{}};
D2RL_PLUGIN_EXPORT const D2RL::PluginInfo* D2RLoaderGetPluginInfo() noexcept {return &Info;}
D2RL_PLUGIN_EXPORT bool D2RLoaderLoadPlugin(const D2RL::PluginContext* c) noexcept {
 if(!c || c->contextSize<offsetof(D2RL::PluginContext,activeMod)+sizeof(c->activeMod) || !c->api || c->api->apiSize<offsetof(D2RL::PluginApi,installInlineHook)+sizeof(c->api->installInlineHook) || !c->api->installInlineHook || !c->api->checkExpectedBytes || !c->activeMod || std::strcmp(c->activeMod,"FMMP-Experimental-R67"))return false;
 const unsigned char eventBytes[]={0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x6c,0x24,0x10,0x48,0x89,0x74,0x24,0x20};
 const unsigned char castBytes[]={0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x6c,0x24,0x18,0x48,0x89,0x74,0x24,0x20,0x57,0x41,0x56,0x41,0x57,0x48,0x83,0xec,0x70};
 const unsigned char findBytes[]={0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x18,0x57,0x48,0x83,0xec,0x20};
 if(!c->api->checkExpectedBytes(c,0x5896e0,castBytes,sizeof(castBytes)) || !c->api->checkExpectedBytes(c,0x48fe80,findBytes,sizeof(findBytes)))return false;
 Virus::Context=c;Virus::Find=reinterpret_cast<Virus::FindFn>(c->exeBase+0x48fe80);Virus::Cast=reinterpret_cast<Virus::CastFn>(c->exeBase+0x5896e0);
 const auto check=c->api->checkExpectedBytes;
 if(!check(c,RampNative::RegenRva,RampNative::RegenBytes,sizeof(RampNative::RegenBytes)) ||
    !check(c,RampNative::HitRva,RampNative::HitBytes,sizeof(RampNative::HitBytes)) ||
    !check(c,RampNative::ListRva,RampNative::ListBytes,sizeof(RampNative::ListBytes)) ||
    !check(c,RampNative::GetStatRva,RampNative::GetStatBytes,sizeof(RampNative::GetStatBytes)) ||
    !check(c,RampNative::SetStatRva,RampNative::SetStatBytes,sizeof(RampNative::SetStatBytes)) ||
    !check(c,RampNative::GetSkillRva,RampNative::GetSkillBytes,sizeof(RampNative::GetSkillBytes)))return false;
 if(!check(c,PulseNative::CreateRva,PulseNative::CreateBytes,sizeof(PulseNative::CreateBytes)) || !check(c,PulseNative::GetUnitRva,PulseNative::GetUnitBytes,sizeof(PulseNative::GetUnitBytes)) || !check(c,PulseNative::SetUnitRva,PulseNative::SetUnitBytes,sizeof(PulseNative::SetUnitBytes)))return false;
 Ramp::GetUnit=reinterpret_cast<Ramp::UnitGetFn>(c->exeBase+PulseNative::GetUnitRva);
 Ramp::SetUnit=reinterpret_cast<Ramp::UnitSetFn>(c->exeBase+PulseNative::SetUnitRva);
 Ramp::Get=reinterpret_cast<Ramp::GetFn>(c->exeBase+RampNative::GetStatRva);
 Ramp::Set=reinterpret_cast<Ramp::SetFn>(c->exeBase+RampNative::SetStatRva);
 Ramp::Skill=reinterpret_cast<Ramp::SkillFn>(c->exeBase+RampNative::GetSkillRva);
 Ramp::Log=&Virus::Log;
 auto checkCurse=[c](std::uint64_t rva,const unsigned char* bytes,unsigned size){return c->api->checkExpectedBytes(c,rva,bytes,size);};
 if(!checkCurse(CurseNative::ApplyRva,CurseNative::ApplyBytes,sizeof(CurseNative::ApplyBytes)))return false;
 if(!checkCurse(CurseNative::FlagRva,CurseNative::FlagBytes,sizeof(CurseNative::FlagBytes)))return false;
 if(!checkCurse(CurseNative::CheckRva,CurseNative::CheckBytes,sizeof(CurseNative::CheckBytes)))return false;
 if(!checkCurse(CurseNative::GameRva,CurseNative::GameBytes,sizeof(CurseNative::GameBytes)))return false;
 if(!checkCurse(CurseNative::OwnerRva,CurseNative::OwnerBytes,sizeof(CurseNative::OwnerBytes)))return false;
 if(!checkCurse(CurseNative::OwnerIdRva,CurseNative::OwnerIdBytes,sizeof(CurseNative::OwnerIdBytes)))return false;
 if(!checkCurse(CurseNative::OwnerTypeRva,CurseNative::OwnerTypeBytes,sizeof(CurseNative::OwnerTypeBytes)))return false;
 if(!checkCurse(CurseNative::LevelRva,CurseNative::LevelBytes,sizeof(CurseNative::LevelBytes)))return false;
 if(!checkCurse(CurseNative::BaseGetRva,CurseNative::BaseGetBytes,sizeof(CurseNative::BaseGetBytes)))return false;
 if(!checkCurse(CurseNative::UpdateRva,CurseNative::UpdateBytes,sizeof(CurseNative::UpdateBytes)))return false;
 if(!checkCurse(CurseNative::XRva,CurseNative::XBytes,sizeof(CurseNative::XBytes)))return false;
 if(!checkCurse(CurseNative::YRva,CurseNative::YBytes,sizeof(CurseNative::YBytes)))return false;
 if(!checkCurse(CurseNative::RegisterRva,CurseNative::RegisterBytes,sizeof(CurseNative::RegisterBytes)))return false;
 if(!checkCurse(CurseNative::ExpiryRva,CurseNative::ExpiryBytes,sizeof(CurseNative::ExpiryBytes)))return false;
 if(!checkCurse(CurseNative::ScheduleRva,CurseNative::ScheduleBytes,sizeof(CurseNative::ScheduleBytes)))return false;
 Curses::Game=reinterpret_cast<Curses::GameFn>(c->exeBase+CurseNative::GameRva);
 Curses::Owner=reinterpret_cast<Curses::OwnerFn>(c->exeBase+CurseNative::OwnerRva);
 if(!checkCurse(CurseNative::MinionOwnerRva,CurseNative::MinionOwnerBytes,sizeof(CurseNative::MinionOwnerBytes)) || !checkCurse(CurseNative::MinionOwnerTailRva,CurseNative::MinionOwnerTailBytes,sizeof(CurseNative::MinionOwnerTailBytes)))return false;
 Curses::MinionOwner=reinterpret_cast<Curses::MinionOwnerFn>(c->exeBase+CurseNative::MinionOwnerRva);
 Curses::ListOwnerId=reinterpret_cast<Curses::ListIntFn>(c->exeBase+CurseNative::OwnerIdRva);
 Curses::ListOwnerType=reinterpret_cast<Curses::ListIntFn>(c->exeBase+CurseNative::OwnerTypeRva);
 Curses::ListLevel=reinterpret_cast<Curses::ListIntFn>(c->exeBase+CurseNative::LevelRva);
 Curses::BaseGet=reinterpret_cast<Curses::BaseGetFn>(c->exeBase+CurseNative::BaseGetRva);
 Curses::Update=reinterpret_cast<Curses::UpdateFn>(c->exeBase+CurseNative::UpdateRva);
 Curses::X=reinterpret_cast<Curses::CoordFn>(c->exeBase+CurseNative::XRva);
 Curses::Y=reinterpret_cast<Curses::CoordFn>(c->exeBase+CurseNative::YRva);
 Curses::Register=reinterpret_cast<Curses::RegisterFn>(c->exeBase+CurseNative::RegisterRva);
 if(!checkCurse(CurseNative::UnitTimerRva,CurseNative::UnitTimerBytes,sizeof(CurseNative::UnitTimerBytes)))return false;
 Curses::UnitTimer=reinterpret_cast<Curses::TimerFn>(c->exeBase+CurseNative::UnitTimerRva);
 Ramp::RegenPending=&Curses::RegenPending;
 if(!checkCurse(CurseNative::MissileHitRva,CurseNative::MissileHitBytes,sizeof(CurseNative::MissileHitBytes)))return false;
 Curses::NativeMissileHit=reinterpret_cast<Curses::MissileHitFn>(c->exeBase+CurseNative::MissileHitRva);
 if(!checkCurse(CurseNative::MissileTickRva,CurseNative::MissileTickBytes,sizeof(CurseNative::MissileTickBytes)))return false;
 if(!checkCurse(CurseNative::GroundTickRva,CurseNative::GroundTickBytes,sizeof(CurseNative::GroundTickBytes)))return false;
 if(!checkCurse(CurseNative::FrameRva,CurseNative::FrameBytes,sizeof(CurseNative::FrameBytes)))return false;
 if(!checkCurse(CurseNative::DamageRva,CurseNative::DamageBytes,sizeof(CurseNative::DamageBytes)))return false;
 if(!checkCurse(CurseNative::RecordRva,CurseNative::RecordBytes,sizeof(CurseNative::RecordBytes)))return false;
 if(!checkCurse(CurseNative::EvalTailRva,CurseNative::EvalTailBytes,sizeof(CurseNative::EvalTailBytes)))return false;
 const auto* serverDo=reinterpret_cast<const std::uintptr_t*>(c->exeBase+CurseNative::ServerDoTableRva);
 if(serverDo[30]!=c->exeBase+0x55c840)return false;
 Curses::Record=reinterpret_cast<Curses::RecordFn>(c->exeBase+CurseNative::RecordRva);
 Curses::Evaluate=reinterpret_cast<Curses::EvalFn>(c->exeBase+CurseNative::EvalRva);
 Pulse::FilterCreate=&Curses::FilterFire;
 Pulse::AfterCreate=&Curses::TagFire;
 Curses::Find=Virus::Find;Curses::Cast=reinterpret_cast<Curses::CastFn>(c->exeBase+0x5896e0);
 Ramp::SetExpiry=reinterpret_cast<Ramp::ExpireFn>(c->exeBase+CurseNative::ExpiryRva);
 Ramp::Schedule=reinterpret_cast<Ramp::ScheduleFn>(c->exeBase+CurseNative::ScheduleRva);
 Ramp::ExtraTick=&Curses::Tick;Ramp::NeedsTick=&Curses::NeedsTick;
 Ramp::BeforeRegen=&Curses::Pump;

 auto install=[c](std::uint64_t rva,const unsigned char* bytes,unsigned size,void* target,void** original){
  D2RL::InlineHookRegistration hook{};hook.registrationSize=sizeof(hook);hook.rva=rva;hook.expected=bytes;hook.expectedSize=size;hook.target=target;hook.original=original;return c->api->installInlineHook(c,&hook);
 };
 if(!install(CurseNative::ApplyRva,CurseNative::ApplyBytes,sizeof(CurseNative::ApplyBytes),reinterpret_cast<void*>(&Curses::OnApply),reinterpret_cast<void**>(&Curses::OriginalApply)))return false;
 if(!install(CurseNative::FlagRva,CurseNative::FlagBytes,sizeof(CurseNative::FlagBytes),reinterpret_cast<void*>(&Curses::OnFlag),reinterpret_cast<void**>(&Curses::OriginalFlag)))return false;
 if(!install(CurseNative::CheckRva,CurseNative::CheckBytes,sizeof(CurseNative::CheckBytes),reinterpret_cast<void*>(&Curses::OnCheck),reinterpret_cast<void**>(&Curses::OriginalCheck)))return false;
 if(!install(CurseNative::MissileTickRva,CurseNative::MissileTickBytes,sizeof(CurseNative::MissileTickBytes),reinterpret_cast<void*>(&Curses::OnMissileTick),reinterpret_cast<void**>(&Curses::OriginalMissileTick)))return false;
 if(!install(CurseNative::GroundTickRva,CurseNative::GroundTickBytes,sizeof(CurseNative::GroundTickBytes),reinterpret_cast<void*>(&Curses::OnGroundTick),reinterpret_cast<void**>(&Curses::OriginalGroundTick)))return false;
 if(!install(CurseNative::FrameRva,CurseNative::FrameBytes,sizeof(CurseNative::FrameBytes),reinterpret_cast<void*>(&Curses::OnFrame),reinterpret_cast<void**>(&Curses::OriginalFrame)))return false;
 if(!install(CurseNative::DamageRva,CurseNative::DamageBytes,sizeof(CurseNative::DamageBytes),reinterpret_cast<void*>(&Curses::OnDamage),reinterpret_cast<void**>(&Curses::OriginalDamage)))return false;
 if(!install(0x42f300,eventBytes,sizeof(eventBytes),reinterpret_cast<void*>(&Virus::OnEvent),reinterpret_cast<void**>(&Virus::Original)) ||
    !install(RampNative::ListRva,RampNative::ListBytes,sizeof(RampNative::ListBytes),reinterpret_cast<void*>(&Ramp::OnList),reinterpret_cast<void**>(&Ramp::OriginalList)) ||
    !install(RampNative::HitRva,RampNative::HitBytes,sizeof(RampNative::HitBytes),reinterpret_cast<void*>(&Ramp::OnHit),reinterpret_cast<void**>(&Ramp::OriginalHit)) ||
    !install(RampNative::RegenRva,RampNative::RegenBytes,sizeof(RampNative::RegenBytes),reinterpret_cast<void*>(&Ramp::OnRegen),reinterpret_cast<void**>(&Ramp::OriginalRegen)) ||
    !install(PulseNative::CreateRva,PulseNative::CreateBytes,sizeof(PulseNative::CreateBytes),reinterpret_cast<void*>(&Pulse::OnCreate),reinterpret_cast<void**>(&Pulse::OriginalCreate)))return false;
 if(!NativeProbe::Register(c))return false;
 Virus::Log("Curse overhaul 0.5.13 active: balance update: zero-start 3-point Decrepify growth; reduced curse/element damage; cold/lightning, curse synergies, native AI skeleton owner, wide stat layers and corpse-free summons retained.");return true;
}
D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {Virus::Context=nullptr;Ramp::Log=nullptr;Curses::ObservedGame=nullptr;}
#endif
