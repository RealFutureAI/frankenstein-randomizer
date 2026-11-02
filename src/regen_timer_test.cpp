#include "curses.h"
#include <cassert>
alignas(8) static unsigned char game[0x180]{},a[0x50]{},b[0x50]{};
static void* head=nullptr;
static int queued=1,nativeCalls=0,health=1000;
static void* __fastcall Timer(void*){return head;}
static bool Needs(void*) noexcept{return true;}
static void __fastcall Schedule(void*,void*,int event,int,int,int,int){assert(event==3);++queued;}
static void __fastcall Regen(void* g,void* u,int,int){++nativeCalls;++health;Schedule(g,u,3,0,0,0,0);}
static void Frame(unsigned n){std::memcpy(game+0x170,&n,4);}
static void SetTimer(unsigned char* t,int type,int frame,void* next){t[0]=static_cast<unsigned char>(type);std::memcpy(t+4,&frame,4);std::memcpy(t+0x38,&next,8);}
int main(){
 Curses::UnitTimer=&Timer;int unit=0;Frame(100);
 assert(!Curses::RegenPending(game,&unit));
 head=a;SetTimer(a,12,150,nullptr);assert(!Curses::RegenPending(game,&unit));
 SetTimer(a,3,101,nullptr);assert(Curses::RegenPending(game,&unit));
 SetTimer(a,3,99,nullptr);assert(!Curses::RegenPending(game,&unit));
 SetTimer(a,12,150,b);SetTimer(b,3,-1,nullptr);assert(Curses::RegenPending(game,&unit));
 SetTimer(a,12,150,a);assert(Curses::RegenPending(game,&unit)); // Bounded/cyclic guard.
 Ramp::OriginalRegen=&Regen;Ramp::Schedule=&Schedule;Ramp::NeedsTick=&Needs;
 Ramp::BeforeRegen=+[](void*) noexcept {};
 // Native regen schedules one successor. The old wrapper adds a second,
 // so this assertion fails on frame one with the previous implementation.
 // Across three seconds the corrected wrapper must keep exactly one chain,
 // with precisely native-rate healing, despite NeedsTick being true.
 for(unsigned f=1;f<=75;++f){Frame(f);const int due=queued;queued=0;for(int i=0;i<due;++i)Ramp::OnRegen(game,&unit,0,0);assert(queued==1);}
 assert(nativeCalls==75 && health==1075);
 std::puts("PASS: regen timers do not multiply; exactly 75 native callbacks and native-rate healing over 75 frames; pending/expired/recurring/unrelated timer detection and cycle guard.");
}
