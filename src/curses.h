#pragma once
#include "ramp.h"
#include "pulse.h"
#include <algorithm>
#include <cstddef>
namespace Curses {
constexpr int MasteryStat = 408, LordStat = 409, DecrepStart = 411, DecrepSecond = 412, Frost = 413,
              HitTime = 414, GroundTime = 415, GroundX = 416, GroundY = 417, SkeletonTime = 419,
              RandomStat = 420, LightningTime = 422;
constexpr int SkeletonClass = 363;
constexpr int ClockMissile = 755, FireStarted = 421, NativeFireTag = 423, FireMissile = 752,
              FireInterval = 75;
using ApplyFn = void*(__fastcall*)(void*);
using FlagFn = void*(__fastcall*)(void*, int);
using CheckFn = int(__fastcall*)(void*, int);
using GameFn = void*(__fastcall*)(void*);
using OwnerFn = void*(__fastcall*)(void*, void*);
using MinionOwnerFn = void*(__fastcall*)(void*);
using FindFn = void*(__fastcall*)(void*, int, unsigned);
using ListIntFn = int(__fastcall*)(void*);
using BaseGetFn = int(__fastcall*)(void*, int, unsigned);
using UpdateFn = void(__fastcall*)(void*, const char*, int);
using CoordFn = int(__fastcall*)(void*);
using CastFn = int(__fastcall*)(void*, int, int, void*, int);
using RegisterFn = void(__fastcall*)(void*, void*, int, int, int, int, int, void*, const short*,
                                     const short*, int);
using RecordFn = void*(__fastcall*)(unsigned char, int);
using EvalFn = int(__fastcall*)(unsigned char, void*, int, int, int);
using TriggerFn = int(__fastcall*)(void*, int, void*, void*, void*, std::uintptr_t);
using MissileTickFn = int(__fastcall*)(void*, void*);
using MissileHitFn = int(__fastcall*)(void*, void*, void*, int);
inline MissileHitFn NativeMissileHit = nullptr;
using DamageFn = void(__fastcall*)(void*, void*, void*, void*);
inline DamageFn OriginalDamage = nullptr;
inline unsigned DamageLogs = 0, OwnedDamageLogs = 0;
inline DamageFn OriginalHitLeech = nullptr;
using FrameFn = void(__fastcall*)(void*);
using TimerFn = void*(__fastcall*)(void*);
inline TimerFn UnitTimer = nullptr;
using MeleeFn = int(__fastcall*)(void*, void*, void*, int, unsigned char);
using DamageForFn = void*(__fastcall*)(void*, void*);
using ReachFn = int(__fastcall*)(void*);
using InRangeFn = int(__fastcall*)(void*, void*, int, void*);
inline ApplyFn OriginalApply = nullptr;
inline FlagFn OriginalFlag = nullptr;
inline CheckFn OriginalCheck = nullptr;
inline GameFn Game = nullptr;
inline OwnerFn Owner = nullptr;
inline FindFn Find = nullptr;
inline MinionOwnerFn MinionOwner = nullptr;
inline ListIntFn ListOwnerId = nullptr, ListOwnerType = nullptr, ListLevel = nullptr;
inline BaseGetFn BaseGet = nullptr;
inline UpdateFn Update = nullptr;
inline CoordFn X = nullptr, Y = nullptr;
inline CastFn Cast = nullptr;
inline RegisterFn Register = nullptr;
inline MissileTickFn OriginalMissileTick = nullptr, OriginalGroundTick = nullptr;
inline RecordFn Record = nullptr;
inline EvalFn Evaluate = nullptr;
inline TriggerFn OriginalTrigger = nullptr;
inline FrameFn OriginalFrame = nullptr;
inline MeleeFn OriginalMelee = nullptr;
inline void* ObservedGame = nullptr;
inline DamageForFn DamageFor = nullptr;
inline ReachFn Reach = nullptr;
inline InRangeFn InRange = nullptr;
inline unsigned Logs = 0, LordLogs = 0;
inline unsigned ClockLogs = 0, DecrepLogs = 0, DecrepApplyLogs = 0, FireLogs = 0;
inline unsigned FireHitLogs = 0;
inline unsigned FireAuditTarget = 0;
inline thread_local unsigned ProcDepth = 0;
inline int ResolveFireHit(void* game, void* missile) noexcept;
inline int MissileTick(void* game, void* missile, MissileTickFn original) noexcept {
    const unsigned id = missile ? static_cast<unsigned*>(missile)[1] : 0;
    const bool own = (id >= 752 && id <= 754) ||
                     (id == FireMissile && Ramp::GetUnit(missile, NativeFireTag, 0) > 0);
    if (own)
        ++ProcDepth;
    const int result = original(game, missile);
    if (own)
        --ProcDepth;
    return result;
}
inline int __fastcall OnMissileTick(void* game, void* missile) noexcept {
    if (missile && static_cast<unsigned*>(missile)[1] == ClockMissile)
        return 2;
    if (missile && static_cast<unsigned*>(missile)[1] == FireMissile)
        return ResolveFireHit(game, missile);
    return MissileTick(game, missile, OriginalMissileTick);
}
inline int __fastcall OnGroundTick(void* game, void* missile) noexcept {
    return MissileTick(game, missile, OriginalGroundTick);
}
struct ClockTarget {
    unsigned id = 0, cls = 0;
};
struct ClockGroup {
    void* game = nullptr;
    unsigned owner = 0, lastFrame = 0, count = 0;
    bool hasFrame = false;
    std::array<ClockTarget, 4096> targets{};
};
inline std::array<ClockGroup, 16> Clocks{};
inline void* ClockGame = nullptr;
inline unsigned ClockEpochFrame = 0;
struct Curse {
    void* caster;
    void* target;
    int skill, level, duration, stat, value, state;
    void* remove;
};
static_assert(offsetof(Curse, state) == 0x24 && offsetof(Curse, remove) == 0x28);
struct Context {
    void* target;
    int state;
    bool stack;
    Context* previous;
};
inline thread_local Context* Active = nullptr;
inline bool Player(void* p) noexcept {
    return p && *static_cast<unsigned*>(p) == 0;
}
inline bool Monster(void* p) noexcept {
    return p && *static_cast<unsigned*>(p) == 1;
}
inline bool OurSkill(int s) noexcept {
    return s == 66 || s == 76 || s == 77 || s == 81 || s == 82 || s == 86 || s == 87 || s == 91 ||
           s == 444;
}
inline int Read(void* game, void* list, int stat) noexcept {
    return Ramp::Get(Ramp::Version(game), list, stat, 0);
}
inline void Write(void* game, void* list, int stat, int value) noexcept {
    Ramp::Set(Ramp::Version(game), list, stat, value, 0);
}
inline bool RegenPending(void* game, void* unit) noexcept {
    if (!UnitTimer || !unit)
        return false;
    void* timer = UnitTimer(unit);
    unsigned count = 0;
    // Native scheduler/iterator layout: type +0, frame +4, next +0x38.
    while (timer && count++ < 4096) {
        const auto* raw = static_cast<const unsigned char*>(timer);
        int frame = 0;
        void* next = nullptr;
        std::memcpy(&frame, raw + 4, 4);
        std::memcpy(&next, raw + 0x38, 8);
        if (raw[0] == 3 && (frame == -1 || frame >= static_cast<int>(Ramp::Frame(game))))
            return true;
        timer = next;
    }
    return timer != nullptr; // Fail closed on an unexpectedly long/cyclic list.
}
inline void* Caster(void* game, void* list) noexcept {
    return list ? Find(game, ListOwnerType(list), static_cast<unsigned>(ListOwnerId(list)))
                : nullptr;
}
inline void* __fastcall OnFlag(void* unit, int flag) noexcept {
    if (flag == 0x20 && Active && Active->stack && Active->target == unit)
        return Ramp::OriginalList(unit, Active->state);
    return OriginalFlag(unit, flag);
}
inline int __fastcall OnCheck(void* unit, int state) noexcept {
    if (state == 57 && Active && Active->stack && Active->target == unit)
        return 0;
    return OriginalCheck(unit, state);
}
inline void RegisterOne(void* game, const Curse& curse, short event, short function) noexcept {
    const short events[4] = {event, -1, -1, -1}, functions[4] = {function, 0, 0, 0};
    Register(game, curse.target, curse.skill, curse.level, curse.skill, 1, curse.state,
             curse.caster, events, functions, 4);
}
inline void EnsureClock(void* game, void* owner, void* unit) noexcept;
inline void ApplyFirePulse(void* game, void* owner, void* unit, int level) noexcept;
inline void* __fastcall OnApply(void* raw) noexcept {
    auto* c = static_cast<Curse*>(raw);
    if (!c || !OurSkill(c->skill) || !c->caster || !Monster(c->target))
        return OriginalApply(raw);
    void* game = Game(c->target);
    if (!game)
        return OriginalApply(raw);
    void* owner = c->caster;
    if (!Player(owner))
        owner = Owner(game, owner);
    Context context{c->target, c->state, Player(owner) && Ramp::GetUnit(owner, MasteryStat, 0) > 0,
                    Active};
    Active = &context;
    void* result = OriginalApply(raw);
    Active = context.previous;
    if (c->skill == 87 && Ramp::Log && DecrepApplyLogs++ < 48) {
        char message[136]{};
        std::snprintf(message, sizeof(message),
                      "Decrepify apply: unit=%u state=%d level=%d duration=%d attached=%d",
                      static_cast<unsigned*>(c->target)[2], c->state, c->level, c->duration,
                      result ? 1 : 0);
        Ramp::Log(message);
    }
    if (!result)
        return result;
    if (c->skill == 81)
        RegisterOne(game, *c, 5, 4); // Confuse + native melee Iron Maiden.
    if (c->skill == 76 && c->state == 776 && Player(owner))
        RegisterOne(game, *c, 10, 33); // Soul Harvest death event.
    if (c->skill == 86) {
        RegisterOne(game, *c, 10, 33); // Attract death explosion.
    }
    const unsigned now = Ramp::Frame(game);
    if (c->skill == 87 && !Read(game, result, DecrepStart)) {
        Write(game, result, DecrepStart, static_cast<int>(now + 1));
        Write(game, result, DecrepSecond, -1);
    }
    if (c->skill == 82 && !Read(game, result, GroundTime)) {
        Write(game, result, GroundTime, static_cast<int>(now + 1));
        Write(game, result, GroundX, X(c->target));
        Write(game, result, GroundY, Y(c->target));
    }
    // Fire and Decrepify run from the server-frame pump, never HP-regen timers.
    if (c->skill == 66 || c->skill == 77 || c->skill == 444 || c->skill == 87 || c->skill == 82)
        EnsureClock(game, owner, c->target);
    if (c->skill == 82)
        ApplyFirePulse(game, owner, c->target, c->level);
    if (Ramp::Log && Logs++ < 128) {
        char m[160]{};
        std::snprintf(m, sizeof(m), "Curse applied: unit=%u skill=%d mastery=%d",
                      static_cast<unsigned*>(c->target)[2], c->skill, context.stack ? 1 : 0);
        Ramp::Log(m);
    }
    return result;
}
inline bool NeedsTick(void* unit) noexcept {
    if (!Monster(unit) || Ramp::GetUnit(unit, 6, 0) <= 0)
        return false;
    for (int state : {58, 60})
        if (void* l = Ramp::OriginalList(unit, state))
            if (Ramp::Skill(l) == (state == 58 ? 82 : 87))
                return true;
    return false;
}
inline bool NeedsClock(void* unit) noexcept {
    if (!Monster(unit) || Ramp::GetUnit(unit, 6, 0) <= 0)
        return false;
    if (NeedsTick(unit))
        return true;
    for (int state : {9, 19})
        if (void* list = Ramp::OriginalList(unit, state)) {
            const int skill = Ramp::Skill(list);
            if (skill == 66 || skill == 77 || skill == 444)
                return true;
        }
    return false;
}
inline int DecrepStrength(unsigned elapsed, int factor = 100) noexcept {
    const std::uint64_t gain =
        std::min(elapsed / 25, 30U) * 3ULL * std::clamp(factor, 100, 100000) / 100;
    return static_cast<int>(std::min<std::uint64_t>(90, gain));
}
inline int EvalCalc4(void* game, void* caster, int skill, int level, int fallback) noexcept {
    if (!Player(caster) || !Record || !Evaluate)
        return fallback;
    const auto version = Ramp::Version(game);
    void* record = Record(version, skill);
    if (!record)
        return fallback;
    int formula = -1;
    std::memcpy(&formula, static_cast<unsigned char*>(record) + 0x19c, sizeof(formula));
    return formula < 0 ? fallback : Evaluate(version, caster, formula, skill, level);
}
inline int GrowthFactor(void* game, void* caster, int level) noexcept {
    return std::clamp(EvalCalc4(game, caster, 87, level, 100), 100, 100000);
}
inline void* CreateAt(void* game, void* owner, int missileId, int skill, int level, int x, int y,
                      int tx, int ty, void* origin = nullptr, int range = 0) noexcept {
    // Engine creation prefix verified against 3.3.93847; remaining fields zero.
    alignas(16) unsigned char raw[0x100]{};
    auto* p = reinterpret_cast<Pulse::Params*>(raw);
    p->flags = 0x21;
    p->owner = owner;
    p->origin = origin;
    p->missile = missileId;
    p->x = x;
    p->y = y;
    p->targetX = tx;
    p->targetY = ty;
    p->skill = skill;
    p->level = level;
    if (range > 0) {
        p->flags |= 0x8000U;
        p->range = range;
    }
    return Pulse::OnCreate(game, raw);
}
inline ClockGroup* GroupFor(void* game, unsigned owner) noexcept {
    const unsigned now = Ramp::Frame(game);
    if (ClockGame != game || now < ClockEpochFrame) {
        for (auto& g : Clocks) {
            g.game = nullptr;
            g.owner = 0;
            g.count = 0;
            g.hasFrame = false;
        }
        ClockGame = game;
    }
    ClockEpochFrame = now;
    ClockGroup* free = nullptr;
    for (auto& g : Clocks) {
        if (g.game == game && g.owner == owner)
            return &g;
        if (!g.count && !free)
            free = &g;
    }
    if (free) {
        *free = ClockGroup{};
        free->game = game;
        free->owner = owner;
    }
    return free;
}
inline void EnsureClock(void* game, void* owner, void* unit) noexcept {
    if (!Player(owner) || !Monster(unit))
        return;
    auto* group = GroupFor(game, static_cast<unsigned*>(owner)[2]);
    if (!group)
        return;
    ClockTarget target{static_cast<unsigned*>(unit)[2], static_cast<unsigned*>(unit)[1]};
    bool found = false;
    for (unsigned i = 0; i < group->count; ++i)
        if (group->targets[i].id == target.id && group->targets[i].cls == target.cls) {
            found = true;
            break;
        }
    if (!found && group->count < group->targets.size())
        group->targets[group->count++] = target;
    if (!found && Ramp::Log && ClockLogs++ < 64) {
        char m[144]{};
        std::snprintf(m, sizeof(m), "Curse server tracking: owner=%u unit=%u class=%u targets=%u",
                      group->owner, target.id, target.cls, group->count);
        Ramp::Log(m);
    }
}
inline void Pump(void* game) noexcept {
    if (!game)
        return;
    const unsigned now = Ramp::Frame(game);
    if (ClockGame == game && now < ClockEpochFrame) {
        for (auto& g : Clocks) {
            g.game = nullptr;
            g.count = 0;
            g.hasFrame = false;
        }
    }
    if (ClockGame == game)
        ClockEpochFrame = now;
    for (auto& group : Clocks) {
        if (group.game != game || !group.count || (group.hasFrame && group.lastFrame == now))
            continue;
        group.hasFrame = true;
        group.lastFrame = now;
        unsigned keep = 0;
        const unsigned before = group.count;
        for (unsigned i = 0; i < before; ++i) {
            const auto target = group.targets[i];
            void* unit = Find(game, 1, target.id);
            if (unit && static_cast<unsigned*>(unit)[1] == target.cls && NeedsClock(unit)) {
                group.targets[keep++] = target;
                Ramp::TimedTick(game, unit);
            }
        }
        group.count = keep;
        if (Ramp::Log && ClockLogs++ < 96) {
            char m[144]{};
            std::snprintf(m, sizeof(m),
                          "Curse server frame: owner=%u frame=%u tracked=%u active=%u", group.owner,
                          now, before, keep);
            Ramp::Log(m);
        }
    }
}
inline void __fastcall OnFrame(void* game) noexcept {
    ObservedGame = game;
    OriginalFrame(game);
    Pump(game);
}
inline bool FilterFire(void* game, void* raw) noexcept {
    auto* p = static_cast<Pulse::Params*>(raw);
    if (!p)
        return true;
    // This helper no longer creates native Blaze state or any ground missiles.
    if (p->skill == 446 && p->missile == 67)
        return false;
    if (p->missile != FireMissile || p->skill != 446)
        return true;
    void* unit = Monster(p->owner) ? p->owner : (Monster(p->origin) ? p->origin : nullptr);
    if (!unit || Ramp::GetUnit(unit, 6, 0) <= 0)
        return false;
    void* mark = Ramp::OriginalList(unit, 58);
    if (!mark || Ramp::Skill(mark) != 82)
        return false;
    void* caster = Caster(game, mark);
    if (!Player(caster))
        return false;
    const unsigned now = Ramp::Frame(game),
                   last = static_cast<unsigned>(Read(game, mark, GroundTime));
    if (Read(game, mark, FireStarted) && now >= last && now - last < FireInterval)
        return false;
    Write(game, mark, FireStarted, 1);
    Write(game, mark, GroundTime, static_cast<int>(now));
    // A four-frame native Fire Bolt crosses the marked target and ends in its
    // small built-in impact flash. Collision kills it after its single hit.
    p->owner = caster;
    p->origin = unit;
    p->flags |= 0x8000U;
    p->range = 4;
    return true;
}
inline void TagFire(void*, void* raw, void* missile) noexcept {
    auto* p = static_cast<Pulse::Params*>(raw);
    if (p && p->missile == FireMissile && p->skill == 446 && Monster(p->origin)) {
        // Store a stable unit ID, never a pointer into a monster that may die before
        // the next frame. Zero is the consumed/no-target sentinel.
        Ramp::SetUnit(missile, NativeFireTag,
                      static_cast<int>(static_cast<unsigned*>(p->origin)[2] + 1U), 0);
    }
}
inline int ResolveFireHit(void* game, void* missile) noexcept {
    const unsigned tag = static_cast<unsigned>(Ramp::GetUnit(missile, NativeFireTag, 0));
    Ramp::SetUnit(missile, NativeFireTag, 0, 0);
    if (!tag || !NativeMissileHit)
        return 2;
    void* target = Find(game, 1, tag - 1U);
    if (!Monster(target) || Ramp::GetUnit(target, 6, 0) <= 0)
        return 2;
    void* mark = Ramp::OriginalList(target, 58);
    if (!mark || Ramp::Skill(mark) != 82)
        return 2;
    void* owner = Owner(game, missile);
    if (!Player(owner) || Caster(game, mark) != owner)
        return 2;
    const int before = Ramp::GetUnit(target, 6, 0), fireMin = Ramp::GetUnit(missile, 48, 0),
              fireMax = Ramp::GetUnit(missile, 49, 0), regenBefore = Ramp::GetUnit(target, 74, 0);
    // Native Fire Bolt collision can be missed when its entire short flight is
    // spawned inside a monster. Pass the known marked target to the engine's
    // complete missile-hit handler instead. It keeps native resistance, block,
    // death credit and impact signaling, without a fabricated damage structure.
    ++ProcDepth;
    const int result = NativeMissileHit(game, missile, target, 0);
    --ProcDepth;
    // Sample one target across pulses; avoid synchronous logging for whole packs.
    if (!FireAuditTarget)
        FireAuditTarget = tag;
    if (Ramp::Log && FireAuditTarget == tag && FireHitLogs < 4) {
        ++FireHitLogs;
        void* after = Find(game, 1, tag - 1U);
        const int hpAfter = after == target ? Ramp::GetUnit(after, 6, 0) : 0;
        char m[260]{};
        std::snprintf(
            m, sizeof(m),
            "Element fire sample: unit=%u frame=%u interval=75 min256=%d max256=%d hpBefore256=%d hpAfter256=%d regen256=%d result=%d",
            tag - 1U, Ramp::Frame(game), fireMin, fireMax, before, hpAfter, regenBefore, result);
        Ramp::Log(m);
    }
    // Exactly one attempt, even if blocked or if this callback is repeated.
    return 2;
}
inline void ApplyFirePulse(void* game, void* owner, void* unit, int level) noexcept {
    if (!Player(owner))
        return;
    const int x = X(unit), y = Y(unit);
    ++ProcDepth;
    void* fire = CreateAt(game, owner, FireMissile, 446, level, x - 1, y, x + 1, y, unit);
    --ProcDepth;
    if (!fire && Ramp::Log && FireLogs++ < 4)
        Ramp::Log("Element fire creation skipped or failed (cooldown/expired target).");
}
inline void Tick(void* game, void* unit) noexcept {
    if (!Monster(unit) || Ramp::GetUnit(unit, 6, 0) <= 0)
        return;
    const unsigned now = Ramp::Frame(game);
    if (void* l = Ramp::OriginalList(unit, 60))
        if (Ramp::Skill(l) == 87) {
            const int stored = Read(game, l, DecrepStart);
            const unsigned start = stored > 0 ? static_cast<unsigned>(stored - 1) : now;
            const unsigned seconds = now >= start ? (now - start) / 25 : 0;
            if (Read(game, l, DecrepSecond) != static_cast<int>(seconds)) {
                const int strength =
                    DecrepStrength(now >= start ? now - start : 0,
                                   GrowthFactor(game, Caster(game, l), ListLevel(l)));
                for (int stat : {67, 68, 25})
                    Write(game, l, stat, -strength);
                // Apply the full curse strength even to an initially physical-immune unit.
                Write(game, l, 36, -strength);
                Write(game, l, DecrepSecond, static_cast<int>(seconds));
                Update(unit, "local curse progression", 0);
                if (Ramp::Log && DecrepLogs++ < 128) {
                    char m[220]{};
                    std::snprintf(
                        m, sizeof(m),
                        "Decrepify tick: unit=%u seconds=%u strength=%d listPhysical=%d totalPhysical=%d start=%d",
                        static_cast<unsigned*>(unit)[2], seconds, strength, Read(game, l, 36),
                        Ramp::GetUnit(unit, 36, 0), stored);
                    Ramp::Log(m);
                }
            }
        }
    if (void* l = Ramp::OriginalList(unit, 58))
        if (Ramp::Skill(l) == 82) {
            const unsigned last = static_cast<unsigned>(Read(game, l, GroundTime));
            if (now >= last && now - last >= FireInterval)
                if (void* caster = Caster(game, l))
                    ApplyFirePulse(game, caster, unit, ListLevel(l));
        }
}
inline void ElementHit(void* game, void* unit) noexcept {
    if (ProcDepth || !Monster(unit) || Ramp::GetUnit(unit, 6, 0) <= 0)
        return;
    void* l = Ramp::OriginalList(unit, 58);
    if (!l || Ramp::Skill(l) != 82)
        return;
    const unsigned now = Ramp::Frame(game), last = static_cast<unsigned>(Read(game, l, HitTime));
    if (last && now < last + 5)
        return;
    void* caster = Caster(game, l);
    if (!Player(caster))
        return;
    Write(game, l, HitTime, static_cast<int>(now));
    const int level = ListLevel(l), stacks = Read(game, l, Frost) + 1;
    Write(game, l, Frost, stacks < 3 ? stacks : 0);
    ++ProcDepth;
    if (stacks >= 3) {
        const int x = X(unit), y = Y(unit);
        constexpr int dirs[32][2] = {
            {32, 0},    {31, 6},    {30, 12},  {27, 18},  {23, 23},   {18, 27},   {12, 30},
            {6, 31},    {0, 32},    {-6, 31},  {-12, 30}, {-18, 27},  {-23, 23},  {-27, 18},
            {-30, 12},  {-31, 6},   {-32, 0},  {-31, -6}, {-30, -12}, {-27, -18}, {-23, -23},
            {-18, -27}, {-12, -30}, {-6, -31}, {0, -32},  {6, -31},   {12, -30},  {18, -27},
            {23, -23},  {27, -18},  {30, -12}, {31, -6}};
        for (const auto& d : dirs)
            CreateAt(game, caster, 753, 447, level, x, y, x + d[0], y + d[1]);
    }
    const unsigned lastLightning = static_cast<unsigned>(Read(game, l, LightningTime));
    if (!lastLightning || now >= lastLightning + 25) {
        Write(game, l, LightningTime, static_cast<int>(now));
        Cast(caster, 448, level, unit, 0);
    }
    --ProcDepth;
}
inline int LordChance(int level) noexcept {
    return level > 0 ? std::min(100, 1 + level) : 0;
}
inline void* SkeletonOwner(void* game, void* unit) noexcept {
    // Player summons store ownership in their AI control, not in the general
    // unit owner fields used by projectiles and other temporary units.
    if (MinionOwner) {
        void* result = MinionOwner(unit);
        if (Player(result))
            return result;
    }
    void* result = Owner(game, unit);
    if (Player(result))
        return result;
    // Same native owner fields read by SUNIT_GetOwner, without its flag gate.
    unsigned type = 6, id = 0;
    std::memcpy(&type, static_cast<unsigned char*>(unit) + 0xe8, 4);
    std::memcpy(&id, static_cast<unsigned char*>(unit) + 0xec, 4);
    return type == 0 ? Find(game, 0, id) : nullptr;
}
inline int LordLevel(void* game, void* player) noexcept {
    const int passive = std::clamp(Ramp::GetUnit(player, LordStat, 0), 0, 1000);
    if (Record && Evaluate) {
        if (void* record = Record(Ramp::Version(game), 72)) {
            int formula = 0;
            std::memcpy(&formula, static_cast<unsigned char*>(record) + 0x190, 4);
            return std::max(
                passive,
                std::clamp(Evaluate(Ramp::Version(game), player, formula, 72, 1), 0, 1000));
        }
    }
    return passive;
}
inline void SkeletonHit(void* game, void* skeleton, void* target) noexcept {
    if (ProcDepth || !Monster(skeleton) || static_cast<unsigned*>(skeleton)[1] != SkeletonClass ||
        !Monster(target) || Ramp::GetUnit(target, 6, 0) <= 0)
        return;
    void* player = SkeletonOwner(game, skeleton);
    if (!Player(player))
        return;
    const int level = LordLevel(game, player), chance = LordChance(level);
    if (Ramp::Log && LordLogs++ < 64) {
        char m[144]{};
        std::snprintf(m, sizeof(m), "Plague Lord melee: skeleton=%u owner=%u level=%d chance=%d",
                      static_cast<unsigned*>(skeleton)[2], static_cast<unsigned*>(player)[2], level,
                      chance);
        Ramp::Log(m);
    }
    if (!chance)
        return;
    const unsigned now = Ramp::Frame(game),
                   last = static_cast<unsigned>(Ramp::GetUnit(skeleton, SkeletonTime, 0));
    if (last && now < last + 25)
        return;
    unsigned seed = static_cast<unsigned>(Ramp::GetUnit(skeleton, RandomStat, 0));
    if (!seed)
        seed = static_cast<unsigned*>(skeleton)[2] ^ now ^ 0x9e3779b9U;
    seed = seed * 1664525U + 1013904223U;
    Ramp::SetUnit(skeleton, RandomStat, static_cast<int>(seed), 0);
    if ((seed >> 8) % 100 >= static_cast<unsigned>(chance))
        return;
    Ramp::SetUnit(skeleton, SkeletonTime, static_cast<int>(now), 0);
    constexpr int skills[] = {66, 77, 76, 81, 82, 86, 87, 91};
    const int skill = skills[(seed >> 20) % 8];
    ++ProcDepth;
    Cast(player, skill, level, target, 0);
    --ProcDepth;
    if (Ramp::Log && Logs++ < 256) {
        char m[128]{};
        std::snprintf(m, sizeof(m), "Plague Lord: skeleton=%u curse=%d level=%d chance=%d",
                      static_cast<unsigned*>(skeleton)[2], skill, level, chance);
        Ramp::Log(m);
    }
}
inline void AddAttractHit(void* game, void* defender, void* damage) noexcept {
    if (!game || !damage || ProcDepth || !Monster(defender))
        return;
    auto* raw = static_cast<unsigned char*>(damage);
    unsigned short flags = 0;
    int physical = 0, total = 0;
    std::memcpy(&flags, raw + 4, sizeof(flags));
    std::memcpy(&total, raw + 0x134, sizeof(total));
    if (!(flags & 1) || (flags & 0x22) || total <= 0)
        return;
    void* mark = Ramp::OriginalList(defender, 57);
    if (!mark || Ramp::Skill(mark) != 86)
        return;
    const int level = ListLevel(mark);
    if (level < 1)
        return;
    const int resist = std::clamp(Ramp::GetUnit(defender, 36, 0), -100, 100);
    if (resist == 100)
        return;
    const int base = 50 + 15 * (std::min(level, 1000) - 1);
    const int bonus = std::clamp(EvalCalc4(game, Caster(game, mark), 86, level, base), 0, 100000);

    // The engine has already resolved resistance here. Scale only the added
    // physical component, then update both fields before native HP subtraction.
    std::memcpy(&physical, raw + 0x18, sizeof(physical));
    const int scaled = static_cast<int>(std::int64_t{bonus} * 256 * (100 - resist) / 100);
    const int extra = std::min(scaled, std::numeric_limits<int>::max() - std::max(physical, total));
    physical += extra;
    total += extra;
    std::memcpy(raw + 0x18, &physical, sizeof(physical));
    std::memcpy(raw + 0x134, &total, sizeof(total));
}
inline void __fastcall OnHitLeech(void* game, void* attacker, void* defender,
                                  void* damage) noexcept {
    if (attacker && attacker != defender)
        AddAttractHit(game, defender, damage);
    OriginalHitLeech(game, attacker, defender, damage);
}
inline void __fastcall OnDamage(void* game, void* attacker, void* defender, void* damage) noexcept {
    unsigned short flags = 0;
    if (damage)
        std::memcpy(&flags, static_cast<unsigned char*>(damage) + 4, 2);
    if (game && !ProcDepth && Monster(attacker) && Monster(defender)) {
        void* owner = SkeletonOwner(game, attacker);
        const bool owned = Player(owner);
        if (Ramp::Log && (DamageLogs++ < 16 || (owned && OwnedDamageLogs++ < 48))) {
            char m[190]{};
            std::snprintf(m, sizeof(m),
                          "Damage audit: source=%u class=%u target=%u flags=%u playerOwner=%u",
                          static_cast<unsigned*>(attacker)[2], static_cast<unsigned*>(attacker)[1],
                          static_cast<unsigned*>(defender)[2], flags,
                          owned ? static_cast<unsigned*>(owner)[2] : 0);
            Ramp::Log(m);
        }
        if (static_cast<unsigned*>(attacker)[1] == SkeletonClass && (flags & 1) && !(flags & 32))
            SkeletonHit(game, attacker, defender);
    }
    OriginalDamage(game, attacker, defender, damage);
}

} // namespace Curses
