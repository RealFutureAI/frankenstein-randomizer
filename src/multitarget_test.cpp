#include "curses.h"
#include <cassert>
#include <map>
struct List {int skill=0,level=3;std::map<int,int> stats;};
struct Unit {unsigned type=1,cls=0,id=0,mode=0;int x=100,y=200;std::map<int,int> stats;std::map<int,List> lists;};
static Unit player,skeleton,clock,fire,npcs[4];
alignas(8) static unsigned char game[0x180]{};
static unsigned char skillRecord[0x2ec]{};
static unsigned char decrepRecord[0x2ec]{};static int decrepFactor=100;
static int clockCreates=0,fireCreates=0,casts=0,updates=0,frames=0,regens=0,melees=0;
static int timersScheduled=0;
static int formulaLevel=99;static bool killOnDamage=false;
static int fireHits=0;
static int __fastcall FireHit(void* g,void* projectile,void* target,int forced){
 assert(g==game && projectile==&fire && target==&npcs[3] && forced==0 && Curses::ProcDepth==1);
 assert(fire.stats[423]==0);++fireHits;static_cast<Unit*>(target)->stats[6]-=fire.stats[48];return 2;
}
static unsigned char pendingDamage[0x160]{};
static void __fastcall Damage(void*,void*,void*,void*){++melees;if(killOnDamage)npcs[1].stats[6]=0;}
static void __fastcall ServerFrame(void*){++frames;}
static void __fastcall Regen(void*,void*,int,int){++regens;}
static void* __fastcall Find(void*,int type,unsigned id){if(type==0 && id==player.id)return &player;if(type==3 && id==clock.id && clockCreates)return &clock;for(auto& u:npcs)if(type==1 && u.id==id)return &u;return nullptr;}
static int __fastcall UGet(void* p,int stat,unsigned){return static_cast<Unit*>(p)->stats[stat];}
static void __fastcall USet(void* p,int stat,int v,unsigned){static_cast<Unit*>(p)->stats[stat]=v;}
static int __fastcall Get(unsigned char,void* p,int stat,unsigned){return static_cast<List*>(p)->stats[stat];}
static void __fastcall Set(unsigned char,void* p,int stat,int v,unsigned){static_cast<List*>(p)->stats[stat]=v;}
static void* __fastcall ListFor(void* p,int state){auto& lists=static_cast<Unit*>(p)->lists;auto i=lists.find(state);return i==lists.end()?nullptr:&i->second;}
static int __fastcall Skill(void* p){return static_cast<List*>(p)->skill;}
static int __fastcall Level(void* p){return static_cast<List*>(p)->level;}
static int __fastcall OwnerId(void*){return int(player.id);}
static int __fastcall OwnerType(void*){return 0;}
static void* __fastcall Owner(void*,void* unit){return unit==&skeleton?nullptr:&player;}
static void* __fastcall MinionOwner(void* unit){return unit==&skeleton?&player:nullptr;}
static void* __fastcall Game(void*){return game;}
static int __fastcall X(void* p){return static_cast<Unit*>(p)->x;}
static int __fastcall Y(void* p){return static_cast<Unit*>(p)->y;}
static void __fastcall Update(void*,const char*,int){++updates;}
static void __fastcall Schedule(void*,void*,int,int,int,int,int){++timersScheduled;}
static void* __fastcall Apply(void* raw){auto* c=static_cast<Curses::Curse*>(raw);auto& list=static_cast<Unit*>(c->target)->lists[c->state];list.skill=c->skill;list.level=c->level;return &list;}
static void* __fastcall Create(void*,void* raw){auto* p=static_cast<Pulse::Params*>(raw);if(p->missile==755){++clockCreates;return &clock;}if(p->missile==752){assert(p->owner==&player && p->origin && static_cast<Unit*>(p->origin)->type==1 && p->skill==446 && p->range==4 && (p->flags&0x8000));++fireCreates;return &fire;}assert(p->missile!=67);return nullptr;}
static int __fastcall NativeTick(void*,void*){return 42;}
static int __fastcall FireTick(void*,void*){assert(Curses::ProcDepth==1);return 43;}
static int __fastcall Cast(void* owner,int skill,int level,void* target,int){assert(static_cast<Unit*>(target)->stats[6]>0);assert(owner==&player && level==99 && skill!=71 && skill!=72);++casts;return 1;}
static void* __fastcall Record(unsigned char,int id){assert(id==72 || id==87);return id==87?decrepRecord:skillRecord;}
static int __fastcall Evaluate(unsigned char,void* caster,int formula,int skill,int level){assert(caster==&player);if(skill==87){assert(formula==321 && level==3);return decrepFactor;}assert(formula==123 && skill==72 && level==1);return formulaLevel;}
static void Frame(unsigned n){*reinterpret_cast<unsigned*>(game+0x170)=n;}
int main(){
 game[0x106]=3;player.type=0;player.id=1;clock.type=3;clock.cls=755;clock.id=901;fire.type=3;fire.cls=752;fire.id=902;skeleton.type=1;skeleton.cls=363;skeleton.id=900;skeleton.mode=1;
 for(unsigned i=0;i<4;++i){npcs[i].id=10+i;npcs[i].cls=100+i;npcs[i].stats[6]=10000;}
 Ramp::GetUnit=&UGet;Ramp::SetUnit=&USet;Ramp::Get=&Get;Ramp::Set=&Set;Ramp::OriginalList=&ListFor;Ramp::Skill=&Skill;Ramp::Schedule=&Schedule;Ramp::ExtraTick=&Curses::Tick;
 Curses::Game=&Game;Curses::Owner=&Owner;Curses::MinionOwner=&MinionOwner;Curses::Find=&Find;Curses::OriginalApply=&Apply;Curses::BaseGet=&UGet;Curses::X=&X;Curses::Y=&Y;Curses::Update=&Update;Curses::ListOwnerId=&OwnerId;Curses::ListOwnerType=&OwnerType;Curses::ListLevel=&Level;Curses::OriginalMissileTick=&NativeTick;Curses::OriginalFrame=&ServerFrame;Ramp::BeforeRegen=&Curses::Pump;Ramp::OriginalRegen=&Regen;
 Pulse::OriginalCreate=&Create;Pulse::FilterCreate=&Curses::FilterFire;Pulse::AfterCreate=&Curses::TagFire;
 *reinterpret_cast<int*>(decrepRecord+0x19c)=321;Curses::Record=&Record;Curses::Evaluate=&Evaluate;
 Frame(100);for(auto& u:npcs){Curses::Curse c{&player,&u,87,3,500,-1,0,60,nullptr};assert(Curses::OnApply(&c));}
 assert(clockCreates==0 && Curses::GroupFor(game,1)->count==4 && timersScheduled==0);
 Frame(125);Curses::OnFrame(game);for(auto& u:npcs)assert(u.lists[60].stats[67]==-3);
 decrepFactor=180;Frame(150);Curses::OnFrame(game);for(auto& u:npcs)assert(u.lists[60].stats[67]==-10);
 decrepFactor=580;Frame(175);Curses::OnFrame(game);for(auto& u:npcs)assert(u.lists[60].stats[67]==-52);decrepFactor=100;
 Frame(200);Ramp::OnRegen(game,&npcs[0],0,0);assert(regens==1);for(auto& u:npcs)assert(u.lists[60].stats[67]==-12);
 const int oldUpdates=updates;Curses::OnFrame(game);assert(updates==oldUpdates);
 npcs[0].stats[6]=0;Frame(225);Curses::OnFrame(game);assert(Curses::GroupFor(game,1)->count==3);for(unsigned i=1;i<4;++i)assert(npcs[i].lists[60].stats[67]==-15);
 Frame(300);for(auto& u:npcs){u.stats[6]=10000;Curses::Curse c{&player,&u,82,3,500,-1,0,58,nullptr};Curses::OnApply(&c);}
 assert(fireCreates==4 && clockCreates==0 && timersScheduled==0);
 // Every target receives an immediate pulse, without a Blaze cast/state.
 for(auto& u:npcs)assert(u.lists[58].stats[421]==1);
 assert(fire.stats[423]==int(npcs[3].id+1));
 Curses::NativeMissileHit=&FireHit;fire.stats[48]=fire.stats[49]=3200;
 const int initialHp=npcs[3].stats[6];
 assert(Curses::OnMissileTick(game,&fire)==2 && fireHits==1 && npcs[3].stats[6]==initialHp-3200 && Curses::ProcDepth==0);
 assert(Curses::OnMissileTick(game,&fire)==2 && fireHits==1); // No duplicate damage.
 fire.stats[423]=npcs[3].id+1;npcs[3].stats[6]=0;assert(Curses::OnMissileTick(game,&fire)==2 && fireHits==1);npcs[3].stats[6]=initialHp;
 fire.stats[423]=9999;assert(Curses::OnMissileTick(game,&fire)==2 && fireHits==1); // Removed target.
 auto savedMark=npcs[3].lists[58];npcs[3].lists.erase(58);fire.stats[423]=npcs[3].id+1;
 assert(Curses::OnMissileTick(game,&fire)==2 && fireHits==1);npcs[3].lists[58]=savedMark; // Expired curse.
 Curses::OriginalMissileTick=&NativeTick;fire.cls=58;assert(Curses::OnMissileTick(game,&fire)==42 && fireHits==1);fire.cls=752; // Native spell forwarded.
 Frame(310);for(auto& u:npcs)++u.x;Curses::OnFrame(game);assert(fireCreates==4);
 Curses::Curse again{&player,&npcs[0],82,3,500,-1,0,58,nullptr};Curses::OnApply(&again);assert(fireCreates==4);
 Pulse::Params params{};params.missile=752;params.owner=&npcs[2];params.skill=446;params.level=3;
 assert(Pulse::OnCreate(game,&params)==nullptr && fireCreates==4);
 Frame(375);
 Pulse::OnCreate(game,&params);assert(params.owner==&player && fireCreates==5);
 npcs[2].lists.erase(58);params.owner=&npcs[2];assert(Pulse::OnCreate(game,&params)==nullptr && fireCreates==5);
 params.missile=67;params.owner=&npcs[0];assert(!Curses::FilterFire(game,&params));
 params.missile=67;params.skill=46;params.owner=&npcs[2];params.flags=0x21;params.range=0;assert(Curses::FilterFire(game,&params));assert(params.owner==&npcs[2] && params.flags==0x21 && params.range==0); // Original Blaze untouched.
 params.missile=58;params.skill=36;assert(Curses::FilterFire(game,&params));assert(params.range==0 && params.owner==&npcs[2]); // Original Fire Bolt untouched.
 // Plague Lord reads the real skill formula and triggers without any pet event
 // registration or passive stat cache, including existing summoned warriors.
 *reinterpret_cast<int*>(skillRecord+0x190)=123;Curses::Record=&Record;Curses::Evaluate=&Evaluate;Curses::Cast=&Cast;Curses::OriginalDamage=&Damage;pendingDamage[4]=1;
 // Stationary targets keep receiving pulses; movement added no extra ones.
 for(unsigned n:{375U,450U,525U,600U}){Frame(n);Curses::OnFrame(game);}
 assert(fireCreates==17); // Three still-marked stationary targets, four refreshes.
 npcs[1].lists.erase(58);Frame(675);Curses::OnFrame(game);assert(fireCreates==19);
 assert(Owner(game,&skeleton)==nullptr && Curses::SkeletonOwner(game,&skeleton)==&player);
 assert(player.stats[409]==0);Frame(1000);Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==1);
 Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==1);
 Frame(1025);Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==2);
 pendingDamage[4]=0;Frame(1050);Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==2);
 pendingDamage[4]=1;skeleton.cls=364;Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==2);
 skeleton.cls=363;Curses::ProcDepth=1;Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==2);Curses::ProcDepth=0;
 pendingDamage[4]=33;Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==2);
 pendingDamage[4]=1;killOnDamage=true;Frame(1075);Curses::OnDamage(game,&skeleton,&npcs[1],pendingDamage);assert(casts==3 && npcs[1].stats[6]==0 && melees==8);
 formulaLevel=0;player.stats[409]=20;assert(Curses::LordLevel(game,&player)==20);
 std::puts("PASS: four-target Decrepify; immediate fire pulses on all targets; three-second cap across movement/recasts; no ground missile or Blaze state; player attribution; expiry/death guards; elemental self-proc suppression; original Fire Bolt/Blaze unaffected; stationary pulses; no forced regen timers; skeleton procs and native forwarding preserved.");
}
