#include "pulse.h"
#include <cassert>
#include <map>
static std::map<void*,int> processed;
static unsigned missile[]={3,750,501,0};
static unsigned char game[0x180]{};
static int hits=0,creates=0;
static Pulse::Params seen{};
static void* __fastcall Create(void*,void* raw){seen=*static_cast<Pulse::Params*>(raw);missile[1]=seen.missile;++creates;processed[missile]=0;return missile;}
static int __fastcall Hit(void*,void*,void*){++hits;return 1;}
static int __fastcall Get(void* unit,int stat,unsigned){assert(stat==407);return processed[unit];}
static void __fastcall Set(void* unit,int stat,int value,unsigned){assert(stat==407);processed[unit]=value;}
int main(){
 Pulse::OriginalCreate=&Create;Ramp::OriginalHit=&Hit;Ramp::GetUnit=&Get;Ramp::SetUnit=&Set;
 Pulse::Params p{};p.missile=750;p.flags=0x420;p.x=10;p.y=20;p.targetX=100;p.targetY=200;p.target=reinterpret_cast<void*>(1);
 assert(Pulse::OnCreate(game,&p)==missile);assert(seen.x==100 && seen.y==200 && seen.target==nullptr && seen.flags==0x21);assert(hits==1);
 assert(Ramp::OnHit(game,missile,nullptr)==1 && hits==1);
 p={};p.missile=751;p.flags=0x21;p.x=300;p.y=400;p.targetX=500;p.targetY=600;
 Pulse::OnCreate(game,&p);assert(seen.x==300 && seen.y==400 && hits==2);
 Ramp::OnHit(game,missile,nullptr);assert(hits==2);
 p.missile=749;p.x=17;p.targetX=44;p.flags=0x420;Pulse::OnCreate(game,&p);assert(seen.x==17 && seen.flags==0x420 && hits==2 && creates==3);
 std::puts("PASS: first cast applies immediately at cursor, spread at corpse, next missile update cannot double-hit, original Pestfluch projectile unchanged.");
}
