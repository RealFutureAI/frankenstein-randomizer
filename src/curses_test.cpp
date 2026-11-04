#include "curses.h"
#include <cassert>
#include <map>
#include <vector>
struct Unit {
    unsigned type = 1, txt = 0, id = 100, mode = 0;
    int x = 50, y = 60;
    std::map<int, int> stats;
};
struct List {
    int skill = 0, level = 3;
    std::map<int, int> stats;
};
static Unit player, monster, skeleton, clock;
static List mark, pest, viral;
static std::map<int, List*> marks;
alignas(8) static unsigned char game[0x180]{};
static int originalChecks = 0, registrations = 0, created = 0, casts = 0, updates = 0;
static int lastSkill = 0;
static bool masteryExpected = false;
static void* __fastcall ListFor(void*, int state) {
    auto i = marks.find(state);
    return i != marks.end() ? i->second : nullptr;
}
static int __fastcall Get(unsigned char, void* p, int stat, unsigned) {
    return static_cast<List*>(p)->stats[stat];
}
static void __fastcall Set(unsigned char, void* p, int stat, int v, unsigned) {
    static_cast<List*>(p)->stats[stat] = v;
}
static int __fastcall UnitGet(void* p, int stat, unsigned) {
    return static_cast<Unit*>(p)->stats[stat];
}
static void __fastcall UnitSet(void* p, int stat, int v, unsigned) {
    static_cast<Unit*>(p)->stats[stat] = v;
}
static int __fastcall Skill(void* p) {
    return static_cast<List*>(p)->skill;
}
static int __fastcall Level(void* p) {
    return static_cast<List*>(p)->level;
}
static int __fastcall OwnerId(void*) {
    return int(player.id);
}
static int __fastcall OwnerType(void*) {
    return 0;
}
static void* __fastcall Owner(void*, void*) {
    return &player;
}
static void* __fastcall Find(void*, int type, unsigned id) {
    if (type == 3 && id == clock.id)
        return &clock;
    if (type == 1 && id == monster.id)
        return &monster;
    return type == 0 && id == player.id ? &player : nullptr;
}
static void* __fastcall Game(void*) {
    return game;
}
static void* __fastcall Flag(void*, int) {
    return &pest;
}
static int __fastcall Check(void*, int) {
    ++originalChecks;
    return 1;
}
static int __fastcall X(void* u) {
    return static_cast<Unit*>(u)->x;
}
static int __fastcall Y(void* u) {
    return static_cast<Unit*>(u)->y;
}
static void __fastcall Update(void*, const char*, int) {
    ++updates;
}
static void __fastcall Schedule(void*, void*, int, int, int, int, int) {
}
static void __fastcall Register(void*, void* target, int skill, int, int, int group, int state,
                                void* caster, const short* events, const short* funcs, int count) {
    assert(target == &monster && caster == &player && group == 1 && count == 4);
    assert((skill == 81 && state == 59 && events[0] == 5 && funcs[0] == 4) ||
           (skill == 86 && state == 57 && events[0] == 10 && funcs[0] == 33));
    ++registrations;
}
static void* __fastcall Apply(void* raw) {
    auto* c = static_cast<Curses::Curse*>(raw);
    assert((Curses::OnFlag(c->target, 32) == marks[c->state]) == masteryExpected);
    assert(Curses::OnCheck(c->target, 57) == (masteryExpected ? 0 : 1));
    mark.skill = c->skill;
    marks[c->state] = &mark;
    return &mark;
}
static void* __fastcall Create(void*, void* raw) {
    auto* p = static_cast<Pulse::Params*>(raw);
    assert(p->owner == &player);
    assert(p->missile == 752 || p->missile == 753 || p->missile == 755);
    ++created;
    return p->missile == 755 ? &clock : nullptr;
}
static int __fastcall Cast(void* caster, int skill, int, void* target, int) {
    assert(caster == &player && target == &monster);
    ++casts;
    lastSkill = skill;
    return 1;
}
static int __fastcall MissileTick(void* g, void*) {
    Curses::ElementHit(g, &monster);
    return 42;
}
static int __fastcall ClockNativeTick(void*, void*) {
    return 42;
}
static void Frame(unsigned n) {
    *reinterpret_cast<unsigned*>(game + 0x170) = n;
}
static unsigned char attractRecord[0x200]{};
static void* __fastcall Record(unsigned char, int skill) {
    assert(skill == 86);
    return attractRecord;
}
static int __fastcall Evaluate(unsigned char, void* owner, int formula, int skill, int level) {
    assert(owner == &player && formula == 17 && skill == 86 && level == 3);
    return 100; // Level 3 base 80 plus the mock caster's synergy.
}
static int appliedTotal = 0, appliedPhysical = 0, hitCalls = 0;
static void __fastcall HitLeech(void*, void*, void*, void* damage) {
    auto* raw = static_cast<unsigned char*>(damage);
    std::memcpy(&appliedPhysical, raw + 0x18, 4);
    std::memcpy(&appliedTotal, raw + 0x134, 4);
    ++hitCalls;
}
static void Hit(unsigned short flags, int total, int physical) {
    unsigned char raw[0x160]{};
    std::memcpy(raw + 4, &flags, 2);
    std::memcpy(raw + 0x18, &physical, 4);
    std::memcpy(raw + 0x134, &total, 4);
    Curses::OnHitLeech(game, &player, &monster, raw);
}
// Bind stand-ins for the engine callbacks so curse state and resolved hit effects can be inspected.
int main() {
    assert(Curses::DecrepStrength(24) == 0 && Curses::DecrepStrength(25) == 3 &&
           Curses::DecrepStrength(375) == 45 && Curses::DecrepStrength(400) == 48 &&
           Curses::DecrepStrength(750) == 90 && Curses::DecrepStrength(4000) == 90);
    player.type = 0;
    player.id = 1;
    monster.stats[6] = 1000;
    skeleton.txt = 363;
    skeleton.id = 77;
    clock.type = 3;
    clock.txt = 755;
    clock.id = 500;
    Ramp::OriginalList = &ListFor;
    Ramp::Get = &Get;
    Ramp::Set = &Set;
    Ramp::Skill = &Skill;
    Ramp::GetUnit = &UnitGet;
    Ramp::SetUnit = &UnitSet;
    Ramp::Schedule = &Schedule;
    Curses::OriginalApply = &Apply;
    Curses::OriginalFlag = &Flag;
    Curses::OriginalCheck = &Check;
    Curses::Game = &Game;
    Curses::Owner = &Owner;
    Curses::Find = &Find;
    Curses::ListOwnerId = &OwnerId;
    Curses::ListOwnerType = &OwnerType;
    Curses::ListLevel = &Level;
    Curses::X = &X;
    Curses::Y = &Y;
    Curses::Update = &Update;
    Curses::BaseGet = &UnitGet;
    Curses::Register = &Register;
    Curses::Cast = &Cast;
    Pulse::OriginalCreate = &Create;
    Pulse::FilterCreate = &Curses::FilterFire;
    Curses::Curse c{&player, &monster, 81, 3, 500, -1, 0, 59, nullptr};
    marks[59] = &viral;
    Frame(100);
    Curses::OnApply(&c);
    assert(registrations == 1 && Curses::Active == nullptr);
    player.stats[408] = 1;
    masteryExpected = true;
    marks[59] = &viral;
    Curses::OnApply(&c);
    assert(registrations == 2 && Curses::Active == nullptr);
    c.skill = 86;
    c.state = 57;
    marks[57] = &viral;
    Curses::OnApply(&c);
    assert(registrations == 3 && mark.stats[36] == 0);
    Curses::OriginalHitLeech = &HitLeech;
    Hit(1, 25 * 256, 10 * 256);
    assert(appliedTotal == 105 * 256 && appliedPhysical == 90 * 256);
    int formula = 17;
    std::memcpy(attractRecord + 0x19c, &formula, 4);
    Curses::Record = &Record;
    Curses::Evaluate = &Evaluate;
    monster.stats[36] = 50;
    Hit(1, 25 * 256, 0);
    assert(appliedTotal == 75 * 256 && appliedPhysical == 50 * 256);
    Hit(3, 25 * 256, 0);
    assert(appliedTotal == 25 * 256 && appliedPhysical == 0);
    Hit(33, 25 * 256, 0);
    assert(appliedTotal == 25 * 256 && appliedPhysical == 0);
    Hit(0, 25 * 256, 0);
    assert(appliedTotal == 25 * 256 && appliedPhysical == 0);
    Hit(1, 0, 0);
    assert(appliedTotal == 0 && appliedPhysical == 0);
    monster.stats[36] = 100;
    Hit(1, 25 * 256, 0);
    assert(appliedTotal == 25 * 256 && appliedPhysical == 0);
    monster.stats[36] = 0;
    marks.erase(57);
    Hit(1, 25 * 256, 0);
    assert(appliedTotal == 25 * 256 && hitCalls == 8);
    Curses::Record = nullptr;
    Curses::Evaluate = nullptr;
    marks.clear();
    mark = {};
    mark.skill = 87;
    marks[60] = &mark;
    mark.stats[411] = 101;
    mark.stats[412] = -1;
    Frame(100);
    Curses::Tick(game, &monster);
    assert(mark.stats[67] == 0 && mark.stats[36] == 0);
    Frame(125);
    Curses::Tick(game, &monster);
    assert(mark.stats[67] == -3);
    Frame(300);
    Curses::Tick(game, &monster);
    assert(mark.stats[67] == -24 && mark.stats[25] == -24);
    Frame(500);
    Curses::Tick(game, &monster);
    assert(mark.stats[67] == -48);
    monster.stats[36] = 100;
    Frame(525);
    Curses::Tick(game, &monster);
    assert(mark.stats[36] == -51);
    marks.clear();
    mark = {};
    mark.skill = 82;
    marks[58] = &mark;
    mark.stats[415] = 50;
    mark.stats[416] = 50;
    mark.stats[417] = 60;
    Frame(110);
    Curses::Tick(game, &monster);
    assert(created == 0);
    monster.x = 51;
    Frame(120);
    Curses::Tick(game, &monster);
    assert(created == 0);
    Frame(125);
    Curses::Tick(game, &monster);
    assert(created == 1);
    for (unsigned f : {125, 130}) {
        Frame(f);
        Curses::ElementHit(game, &monster);
    }
    assert(created == 1 && mark.stats[413] == 2 && casts == 1 && lastSkill == 448);
    Frame(135);
    Curses::ElementHit(game, &monster);
    assert(created == 33 && mark.stats[413] == 0 && casts == 1 && lastSkill == 448);
    Curses::ElementHit(game, &monster);
    assert(created == 33 && casts == 1);
    for (unsigned f : {140, 145}) {
        Frame(f);
        Curses::ElementHit(game, &monster);
    }
    assert(created == 33 && mark.stats[413] == 2);
    Frame(150);
    Curses::ElementHit(game, &monster);
    assert(created == 65 && mark.stats[413] == 0 && casts == 2);
    casts = 1;
    Curses::ProcDepth = 1;
    Frame(200);
    Curses::ElementHit(game, &monster);
    assert(casts == 1);
    Curses::ProcDepth = 0;
    assert(Curses::LordChance(0) == 0 && Curses::LordChance(1) == 2 &&
           Curses::LordChance(20) == 21 && Curses::LordChance(200) == 100);
    player.stats[409] = 99;
    Frame(300);
    Curses::SkeletonHit(game, &skeleton, &monster);
    assert(casts == 2 && lastSkill != 71 && lastSkill != 72);
    Curses::SkeletonHit(game, &skeleton, &monster);
    assert(casts == 2);
    Frame(325);
    Curses::SkeletonHit(game, &skeleton, &monster);
    assert(casts == 3);
    skeleton.txt = 364;
    Frame(350);
    Curses::SkeletonHit(game, &skeleton, &monster);
    assert(casts == 3);
    Curses::OriginalMissileTick = &MissileTick;
    Curses::OriginalGroundTick = &MissileTick;
    unsigned missile[] = {3, 753, 500, 0};
    Frame(400);
    assert(Curses::OnMissileTick(game, missile) == 42 && casts == 3 && Curses::ProcDepth == 0);
    missile[1] = 752;
    assert(Curses::OnGroundTick(game, missile) == 42 && casts == 3 && Curses::ProcDepth == 0);
    missile[1] = 754;
    assert(Curses::OnMissileTick(game, missile) == 42 && casts == 3 && Curses::ProcDepth == 0);
    missile[1] = 600;
    assert(Curses::OnMissileTick(game, missile) == 42 && casts == 4 && Curses::ProcDepth == 0);
    marks.clear();
    mark = {};
    mark.skill = 87;
    marks[60] = &mark;
    mark.stats[411] = 601;
    mark.stats[412] = -1;
    monster.stats[36] = 0;
    Ramp::ExtraTick = &Curses::Tick;
    Curses::OriginalMissileTick = &ClockNativeTick;
    const int previousCreated = created;
    Frame(600);
    Curses::EnsureClock(game, &player, &monster);
    assert(created == previousCreated && Curses::GroupFor(game, player.id)->count == 1);
    Curses::EnsureClock(game, &player, &monster);
    assert(created == previousCreated);
    // No controller missile is created: the server-frame pump drives all effects.
    Curses::Pump(game);
    assert(mark.stats[67] == 0);
    Frame(625);
    Curses::Pump(game);
    assert(mark.stats[67] == -3);
    Frame(650);
    Curses::Pump(game);
    assert(mark.stats[67] == -6);
    const int previousUpdates = updates;
    Curses::Pump(game);
    assert(updates == previousUpdates);
    marks.clear();
    mark = {};
    mark.skill = 82;
    marks[58] = &mark;
    mark.stats[415] = 625;
    mark.stats[416] = monster.x;
    mark.stats[417] = monster.y;
    const int beforeFire = created;
    Frame(700);
    Curses::Pump(game);
    assert(created == beforeFire + 1);
    ++monster.x;
    Frame(710);
    Curses::Pump(game);
    assert(created == beforeFire + 1);
    viral = {};
    viral.skill = 77;
    viral.stats[403] = 512;
    viral.stats[404] = 700;
    viral.stats[410] = 1000;
    marks[19] = &viral;
    Frame(725);
    Curses::Pump(game);
    assert(viral.stats[74] == -614 && created == beforeFire + 1);
    Frame(775);
    Curses::Pump(game);
    assert(created == beforeFire + 2 && viral.stats[74] == -885);
    monster.stats[6] = 0;
    Frame(776);
    Curses::Pump(game);
    assert(Curses::GroupFor(game, 1)->count == 0);
    monster.stats[6] = 1000;
    marks.clear();
    Frame(777);
    Curses::Pump(game);
    assert(Curses::GroupFor(game, 1)->count == 0);
    std::puts(
        "PASS: mastery; Attract flat physical hit bonus, synergy and hit guards; full Decrepify reduction on immunes; elemental cooldowns; skeleton procs; fire cadence and expiry guards.");
}
