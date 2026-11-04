#pragma once
#include <D2RLPlugin/api.h>
#include "curses.h"
#include <cstring>
namespace NativeProbe {
struct Cell {
    std::uint64_t key;
    int value, pad;
};
static_assert(sizeof(Cell) == 16);
inline int Meta = 0, Wide = 0, Aggregate = 0, OwnerId = 0, GrowthFormula = -1, RegenTimer = 0;
inline void Init(unsigned char* list, Cell* cell) noexcept {
    const unsigned type = 6;
    const std::uint64_t one = 1;
    std::memcpy(list + 8, &type, 4);
    std::memcpy(list + 0x30, &cell, 8);
    std::memcpy(list + 0x38, &one, 8);
    std::memcpy(list + 0x40, &one, 8);
}
inline bool Run() noexcept {
    __try {
        // Isolated, preallocated lists: no game units, allocator, saves or globals
        // are mutated. Layout and parent-cache offsets verified in this Core build.
        alignas(16) unsigned char child[0x200]{}, parent[0x200]{}, unit[0x200]{};
        Cell meta{std::uint64_t(411) << 32, 1, 0};
        Init(child, &meta);
        Ramp::Set(3, child, 411, 306, 0);
        Meta = Ramp::Get(3, child, 411, 0);
        if (Meta != 306)
            return false;
        constexpr unsigned layer = 0x12340000U;
        meta.key = (std::uint64_t(411) << 32) | layer;
        meta.value = 1;
        Ramp::Set(3, child, 411, 7654321, layer);
        Wide = Ramp::Get(3, child, 411, layer);
        if (Wide != 7654321)
            return false;
        // Physical resistance is a real gameplay stat. The same native setter must
        // propagate a child curse's delta into the aggregate seen by UnitGet.
        Cell curse{std::uint64_t(36) << 32, -10, 0}, base{std::uint64_t(36) << 32, 50, 0},
            total{std::uint64_t(36) << 32, 40, 0};
        std::memset(child, 0, sizeof(child));
        Init(child, &curse);
        Init(parent, &base);
        const unsigned extended = 0x80000000U, type = 1;
        std::memcpy(parent + 0x1c, &extended, 4);
        Cell* totals = &total;
        const std::uint64_t one = 1;
        std::memcpy(parent + 0xa8, &totals, 8);
        std::memcpy(parent + 0xb0, &one, 8);
        std::memcpy(parent + 0xb8, &one, 8);
        unsigned char* parentPtr = parent;
        std::memcpy(child + 0x78, &parentPtr, 8);
        std::memcpy(unit, &type, 4);
        std::memcpy(unit + 0x88, &parentPtr, 8);
        unit[0x1bd] = 3;
        Ramp::Set(3, child, 36, -20, 0);
        Aggregate = Ramp::GetUnit(unit, 36, 0);
        if (Ramp::Get(3, child, 36, 0) != -20 || Aggregate != 30)
            return false;
        // Exercise the actual native AI-owner function and native game-unit lookup
        // with isolated preallocated objects, including empty general owner fields.
        alignas(16) unsigned char game[0x4000]{}, pet[0x200]{}, player[0x200]{},
            monsterData[0x100]{}, ai[0x100]{};
        const unsigned monsterType = 1, playerId = 1;
        std::memcpy(pet, &monsterType, 4);
        std::memcpy(player + 8, &playerId, 4);
        unsigned char* md = monsterData;
        unsigned char* control = ai;
        unsigned char* gp = game;
        unsigned char* pp = player;
        std::memcpy(pet + 0x10, &md, 8);
        std::memcpy(monsterData + 0x30, &control, 8);
        std::memcpy(ai + 0x60, &gp, 8);
        std::memcpy(ai + 0x68, &playerId, 4);
        std::memcpy(game + 0x2238, &pp, 8);
        game[0x106] = 3;
        pet[0x1bd] = 3;
        player[0x1bd] = 3;
        void* owner = Curses::MinionOwner(pet);
        if (owner == player)
            OwnerId = 1;
        alignas(8) unsigned char timer[0x50]{};
        unsigned char* timerPtr = timer;
        std::memcpy(pet + 0x148, &timerPtr, 8);
        timer[0] = 3;
        const unsigned currentFrame = 100, futureFrame = 101, expiredFrame = 99;
        std::memcpy(game + 0x170, &currentFrame, 4);
        std::memcpy(timer + 4, &futureFrame, 4);
        if (!Curses::RegenPending(game, pet))
            return false;
        std::memcpy(timer + 4, &expiredFrame, 4);
        if (Curses::RegenPending(game, pet))
            return false;
        std::memcpy(timer + 4, &futureFrame, 4);
        timer[0] = 12;
        if (Curses::RegenPending(game, pet))
            return false;
        RegenTimer = 1;
        void* record = Curses::Record(3, 87);
        if (!record)
            return false;
        std::memcpy(&GrowthFormula, static_cast<unsigned char*>(record) + 0x19c, 4);
        if (GrowthFormula < 0)
            return false;
        void* attract = Curses::Record(3, 86);
        if (!attract)
            return false;
        int attractFormula = -1;
        std::memcpy(&attractFormula, static_cast<unsigned char*>(attract) + 0x19c, 4);
        if (attractFormula < 0)
            return false;
        void* soul = Curses::Record(3, 76);
        if (!soul)
            return false;
        int soulFormula = -1;
        std::memcpy(&soulFormula, static_cast<unsigned char*>(soul) + 0x19c, 4);
        if (soulFormula < 0)
            return false;
        return OwnerId == 1 && RegenTimer == 1;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}
inline void __cdecl OnTables(const D2RL::PluginContext* context,
                             const D2RL::Lifecycle::DataTablesLoadedEvent*, void*) noexcept {
    const bool ok = Run();
    char message[200]{};
    std::snprintf(
        message, sizeof(message),
        "Native stat ABI probe %s: metadata=%d wideLayer=%d aggregatePhysical=%d minionOwner=%d compiledGrowthFormula=%d regenTimer=%d",
        ok ? "PASS" : "FAIL", Meta, Wide, Aggregate, OwnerId, GrowthFormula, RegenTimer);
    if (ok)
        context->api->logInfo(context, message);
    else
        context->api->logError(context, message);
}
inline void __cdecl OnMonsterDeath(const D2RL::PluginContext*,
                                   const D2RL::Lifecycle::MonsterDeathEvent* event,
                                   void*) noexcept {
    if (!event || event->structSize < D2RL::Lifecycle::MonsterDeathEventRequiredSize ||
        event->monster.type != D2RL::Lifecycle::UnitType::Monster)
        return;
    void* game = SoulHarvest::Session;
    if (!game || game != Curses::ObservedGame)
        return;
    void* corpse = Curses::Find(game, 1, event->monster.id);
    if (!Curses::Monster(corpse) || static_cast<unsigned*>(corpse)[1] != event->monster.classId)
        return;
    void* mark = Ramp::OriginalList(corpse, SoulHarvest::CurseState);
    if (!mark || Ramp::Skill(mark) != SoulHarvest::Skill)
        return;
    // Snapshot the drop now. The next server-frame hook creates the pickup.
    SoulHarvest::QueueDrop(game, corpse, Curses::Caster(game, mark), Curses::ListLevel(mark));
}
inline bool Register(const D2RL::PluginContext* context) noexcept {
    if (context->api->apiSize < D2RL::PluginApiQueryServiceSize || !context->api->queryService)
        return false;
    const void* service = nullptr;
    if (context->api->queryService(context, D2RL::ServiceId::Lifecycle, 1, &service) !=
            D2RL::ServiceQueryResult::Success ||
        !service)
        return false;
    auto* lifecycle = static_cast<const D2RL::LifecycleService*>(service);
    if (!D2RL::HasLifecycleServiceField(lifecycle, D2RL::LifecycleServiceSize) ||
        !lifecycle->registerDataTablesLoadedListener || !lifecycle->registerMonsterDeathListener)
        return false;
    D2RL::Lifecycle::DataTablesLoadedListener listener{
        D2RL::Lifecycle::DataTablesLoadedListenerSize, 0, &OnTables, nullptr};
    D2RL::Lifecycle::ListenerHandle handle = 0;
    if (lifecycle->registerDataTablesLoadedListener(context, &listener, &handle) !=
        D2RL::Lifecycle::Result::Success)
        return false;
    D2RL::Lifecycle::MonsterDeathListener deaths{D2RL::Lifecycle::MonsterDeathListenerSize, 0,
                                                 &OnMonsterDeath, nullptr};
    return lifecycle->registerMonsterDeathListener(context, &deaths, &handle) ==
           D2RL::Lifecycle::Result::Success;
}
} // namespace NativeProbe
