#include "ramp.h"
#include <cassert>
#include <map>

struct List {
    unsigned char pad[0x24]{};
    float expiry = 500;
    int skill = 77;
    std::map<int, int> stats;
};

static List poison, pest, virus;
static bool hasPoison = false, hasPest = true, hasVirus = true, rejected = false;
static unsigned unit[] = {1, 0, 101, 0}, missile[] = {3, 750, 501, 0};
alignas(8) static unsigned char game[0x180]{};
static int rate = 512, nativeTicks = 0, scheduled = 0;

static void* __fastcall ListFor(void*, int state) {
    if (state == 2)
        return hasPoison ? &poison : nullptr;
    if (state == 9)
        return hasPest ? &pest : nullptr;
    if (state == 19)
        return hasVirus ? &virus : nullptr;
    return nullptr;
}

static int __fastcall Get(unsigned char, void* p, int stat, unsigned) {
    return static_cast<List*>(p)->stats[stat];
}

static void __fastcall Set(unsigned char, void* p, int stat, int value, unsigned) {
    static_cast<List*>(p)->stats[stat] = value;
}

static int __fastcall Skill(void* p) {
    return static_cast<List*>(p)->skill;
}

static void __fastcall Expire(void* p, int frame) {
    static_cast<List*>(p)->expiry = float(frame);
}

static void __fastcall Schedule(void*, void*, int, int, int, int, int) {
    ++scheduled;
}

static int __fastcall Hit(void*, void*, void*) {
    Ramp::OnList(unit, 2);
    if (!rejected) {
        hasPoison = true;
        poison.stats[74] = -rate;
        poison.expiry = 500;
    }
    return 1;
}

static void __fastcall Regen(void*, void*, int, int) {
    ++nativeTicks;
}

static void Advance(unsigned frame) {
    *reinterpret_cast<unsigned*>(game + 0x170) = frame;
    Ramp::OnRegen(game, unit, 0, 0);
}

int main() {
    Ramp::OriginalList = &ListFor;
    Ramp::Get = &Get;
    Ramp::Set = &Set;
    Ramp::Skill = &Skill;
    Ramp::OriginalHit = &Hit;
    Ramp::OriginalRegen = &Regen;
    Ramp::SetExpiry = &Expire;
    Ramp::Schedule = &Schedule;
    game[0x106] = 3;
    pest.skill = 66;
    assert(Ramp::OnHit(game, missile, nullptr) == 1);
    assert(virus.stats[403] == 512 && virus.stats[74] == -512 && poison.stats[74] == 0);
    Advance(25);
    assert(virus.stats[74] == -614);
    missile[1] = 749;
    rate = 1024;
    Ramp::OnHit(game, missile, nullptr);
    assert(pest.stats[74] == -1024 && virus.stats[74] == -614 && poison.stats[74] == 0);
    Advance(50);
    assert(virus.stats[74] == -737 && pest.stats[74] == -1024);
    missile[1] = 750;
    rate = 512;
    Ramp::OnHit(game, missile, nullptr);
    assert(virus.stats[404] == 0 && virus.stats[74] == -737);
    Advance(75);
    assert(virus.stats[74] == -885 && pest.stats[74] == -1024);
    // Replacing one mark does not leave its damage behind or stop the other.
    hasPest = false;
    Advance(100);
    assert(virus.stats[74] == -1062);
    Advance(500);
    assert(virus.stats[74] == 0 && virus.stats[403] == 0);
    poison.stats[74] = -2000;
    rejected = true;
    Ramp::OnHit(game, missile, nullptr);
    assert(virus.stats[403] == 0 && poison.stats[74] == -2000);
    assert(Ramp::IncreasedRate(1000, 10) == 6192);
    assert(Ramp::IncreasedRate(1000000000, 1000) == std::numeric_limits<int>::max());
    assert(scheduled >= 4 && nativeTicks == 5);
    std::puts(
        "PASS: separate Pestfluch/Virus rates, compounded growth, refresh keeps ramp, curse removal independence, own expiry, stronger external poison preserved, overflow bounded, regen scheduled.");
}
