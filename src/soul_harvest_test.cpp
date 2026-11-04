#include "soul_harvest.h"
#include <cassert>
#include <map>

struct Unit {
    unsigned type = 1, classId = 0, id = 0, mode = 1;
    unsigned char native[0x160]{};
    std::map<int, int> stats;
    std::map<std::pair<int, unsigned>, int> layeredStats;
    int x = 20, y = 10, room = 1;
};
struct StatList {
    int skill = SoulHarvest::Skill;
    std::map<int, int> stats;
};

alignas(8) static unsigned char game[0x3630]{};
static Unit player, corpse, secondCorpse, orb;
static int rooms[3]{};
static StatList mark, secondMark, buff;
static bool buffActive = false;
static int spawned = 0, consumed = 0;
static unsigned visualSyncs = 0, stateUpdates = 0;
static int client = 0;
alignas(8) static unsigned char playerData[0xa0]{};

static void Frame(unsigned frame) {
    std::memcpy(game + 0x170, &frame, sizeof(frame));
}
static int __fastcall X(void* unit) {
    return static_cast<Unit*>(unit)->x;
}
static int __fastcall Y(void* unit) {
    return static_cast<Unit*>(unit)->y;
}
static void* __fastcall Room(void* unit) {
    return &rooms[static_cast<Unit*>(unit)->room];
}
static void* __fastcall Find(void*, int type, unsigned id) {
    return type == 0 && id == player.id ? &player : nullptr;
}
static int __fastcall OwnerType(void*) {
    return 0;
}
static int __fastcall OwnerId(void*) {
    return static_cast<int>(player.id);
}
static int __fastcall Skill(void* list) {
    return static_cast<StatList*>(list)->skill;
}
static void* __fastcall List(void* unit, int state) {
    if (unit == &corpse && state == SoulHarvest::CurseState)
        return &mark;
    if (unit == &secondCorpse && state == SoulHarvest::CurseState)
        return &secondMark;
    if (unit == &player && state == SoulHarvest::BuffState && buffActive)
        return &buff;
    return nullptr;
}
static void* __fastcall Apply(void* raw) {
    auto& request = *static_cast<Curses::Curse*>(raw);
    assert(request.caster == &player && request.target == &player);
    assert(request.state == SoulHarvest::BuffState && request.duration == 375);
    buffActive = true;
    return &buff;
}
static int __fastcall Get(unsigned char, void* list, int stat, unsigned) {
    return static_cast<StatList*>(list)->stats[stat];
}
static void __fastcall Set(unsigned char, void* list, int stat, int value, unsigned) {
    static_cast<StatList*>(list)->stats[stat] = value;
}
static int __fastcall UnitGet(void* raw, int stat, unsigned layer) {
    auto& unit = *static_cast<Unit*>(raw);
    if (layer)
        return unit.layeredStats[{stat, layer}];
    if (raw == &player && (stat == 7 || stat == 9)) {
        const int base = stat == 7 ? 1000 * 256 : 500 * 256;
        return base + (buffActive ? buff.stats[stat] : 0);
    }
    return unit.stats[stat];
}
static void __fastcall UnitSet(void* raw, int stat, int value, unsigned layer) {
    auto& unit = *static_cast<Unit*>(raw);
    if (layer)
        unit.layeredStats[{stat, layer}] = value;
    else
        unit.stats[stat] = value;
}
static void __fastcall Update(void*, const char*, int) {
}
static void* __fastcall PlayerData(void* unit) {
    assert(unit == &player);
    return playerData;
}
static void __fastcall SyncMissile(void* recipient, void* currentGame, void* missile, int moving) {
    assert(recipient == &client && currentGame == game && missile == &orb && moving == 0);
    ++visualSyncs;
}
static void __fastcall MarkGfxDirty(void* unit, int state, int active) {
    assert(unit == &player && state == SoulHarvest::BuffState && active == 1);
    ++stateUpdates;
}
static void __fastcall SetExpiry(void* list, int frame) {
    assert(list == &buff && frame >= 475);
}
static void __fastcall Schedule(void*, void* unit, int event, int, int, int, int) {
    assert(unit == &player && event == 12);
}
static void* __fastcall Create(void*, void* raw) {
    const auto& request = *static_cast<Pulse::Params*>(raw);
    assert(request.owner == &player && !request.origin &&
           request.missile == SoulHarvest::OrbMissile);
    assert(request.x == corpse.x && request.y == corpse.y && (request.flags & 0x8000U) &&
           request.range == 500);
    ++spawned;
    orb.layeredStats.clear(); // A new native missile has no recipients yet.
    return &orb;
}
static int __fastcall NativeTick(void*, void*) {
    return 1;
}
static int __fastcall NativeHit(void*, void* missile, void* target, int forced) {
    assert(missile == &orb && !target && forced == 1);
    ++consumed;
    return 2;
}

// Exercise the drop queue and pickup hook against native-style room and resource mocks.
int main() {
    player.type = 0;
    player.id = 1;
    player.stats[6] = 400 * 256;
    player.stats[8] = 100 * 256;
    orb.type = 3;
    orb.classId = SoulHarvest::OrbMissile;
    void* first = &player;
    std::memcpy(game + 0x2238, &first, sizeof(first));
    Curses::X = &X;
    Curses::Y = &Y;
    Curses::Find = &Find;
    Curses::ListOwnerId = &OwnerId;
    Curses::ListOwnerType = &OwnerType;
    Curses::OriginalApply = &Apply;
    Curses::Update = &Update;
    Curses::OriginalMissileTick = &NativeTick;
    Curses::NativeMissileHit = &NativeHit;
    Pulse::OriginalCreate = &Create;
    Ramp::OriginalList = &List;
    Ramp::Skill = &Skill;
    Ramp::Get = &Get;
    Ramp::Set = &Set;
    Ramp::GetUnit = &UnitGet;
    Ramp::SetUnit = &UnitSet;
    Ramp::SetExpiry = &SetExpiry;
    Ramp::Schedule = &Schedule;
    SoulHarvest::Room = &Room;
    SoulHarvest::PlayerData = &PlayerData;
    SoulHarvest::SyncMissile = &SyncMissile;
    SoulHarvest::MarkGfxDirty = &MarkGfxDirty;
    void* recipient = &client;
    std::memcpy(playerData + 0x98, &recipient, sizeof(recipient));

    assert(SoulHarvest::BaseHealing(1) == 130 && SoulHarvest::BaseHealing(20) == 700);
    assert(SoulHarvest::MaximumBonus(1) == 58 && SoulHarvest::MaximumBonus(20) == 210);
    Frame(99);
    SoulHarvest::QueueDrop(game, &corpse, &player, 1);
    SoulHarvest::QueueDrop(game, &corpse, &player, 1);
    assert(SoulHarvest::DropCount == 1 && spawned == 0);
    SoulHarvest::FlushDrops(game);
    assert(spawned == 1);
    player.x = 30;
    assert(SoulHarvest::OnMissileTick(game, &orb) == 1 && !consumed && !buffActive &&
           visualSyncs == 1);
    assert(SoulHarvest::OnMissileTick(game, &orb) == 1 && visualSyncs == 1);
    player.x = 29; // At the new pickup boundary, beyond the old radius.
    player.room = 2;
    assert(SoulHarvest::OnMissileTick(game, &orb) == 1 && !consumed);
    player.room = 1;
    Frame(100);
    assert(SoulHarvest::OnMissileTick(game, &orb) == 2 && consumed == 1 && visualSyncs == 1 &&
           stateUpdates == 1);
    assert(buff.stats[7] == 1484 && buff.stats[9] == 742);
    assert(player.stats[6] == 400 * 256 + (1000 * 256 + 1484) * 130 / 10000);
    assert(player.stats[8] == 100 * 256 + (500 * 256 + 742) * 130 / 10000);
    assert(SoulHarvest::OnMissileTick(game, &orb) == 1 && consumed == 1);

    Frame(101);
    buffActive =
        false; // Native stat list temporarily disappears, but the first stack is still valid.
    buff.stats.clear();
    SoulHarvest::TickBuffs(game);
    Frame(200);
    mark.stats[SoulHarvest::OrbBonusStat] = 0;
    SoulHarvest::QueueDrop(game, &corpse, &player, 2);
    SoulHarvest::FlushDrops(game);
    assert(SoulHarvest::OnMissileTick(game, &orb) == 2 && consumed == 2);
    assert(buff.stats[7] == 3174 && buff.stats[9] == 1587);
    Frame(200);
    mark.stats[SoulHarvest::OrbBonusStat] = 0;
    SoulHarvest::QueueDrop(game, &corpse, &player, 1);
    SoulHarvest::FlushDrops(game);
    assert(SoulHarvest::OnMissileTick(game, &orb) == 2 && consumed == 3);
    assert(buff.stats[7] == 4659 && buff.stats[9] == 2329);
    // The oldest stack expires at frame 475. Both later stacks remain until frame 575.
    Frame(474);
    SoulHarvest::TickBuffs(game);
    assert(buff.stats[7] == 4659);
    player.stats[6] = 1000 * 256 + 4659;
    Frame(475);
    SoulHarvest::TickBuffs(game);
    assert(buff.stats[7] == 3174 && buff.stats[9] == 1587 && player.stats[6] == 1000 * 256 + 3174);
    Frame(575);
    SoulHarvest::TickBuffs(game);
    assert(buff.stats[7] == 0 && buff.stats[9] == 0 && player.stats[6] == 1000 * 256);
    assert(visualSyncs == 3 && stateUpdates == 5);
    secondCorpse.id = 22;
    mark.stats[SoulHarvest::OrbBonusStat] = 0;
    SoulHarvest::QueueDrop(game, &corpse, &player, 1);
    SoulHarvest::QueueDrop(game, &secondCorpse, &player, 1);
    SoulHarvest::QueueDrop(game, &secondCorpse, &player, 1);
    assert(SoulHarvest::DropCount == 2);
    SoulHarvest::FlushDrops(game);
    assert(SoulHarvest::DropCount == 0 && spawned == 5);
    std::puts(
        "PASS: one orb per marked corpse, duplicate death and pickup guards, wider player pickup, display sync, healing and independent stack expiry.");
}
