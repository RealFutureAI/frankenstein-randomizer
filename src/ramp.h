#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

namespace Ramp {
using ListFn = void*(__fastcall*)(void*, int);
// Current D2RCore uses 32-bit stat layers, including the fifth stack argument.
using GetFn = int(__fastcall*)(unsigned char, void*, int, unsigned);
using SetFn = void(__fastcall*)(unsigned char, void*, int, int, unsigned);
using SkillFn = int(__fastcall*)(void*);
using HitFn = int(__fastcall*)(void*, void*, void*);
using RegenFn = void(__fastcall*)(void*, void*, int, int);
using UnitGetFn = int(__fastcall*)(void*, int, unsigned);
using UnitSetFn = void(__fastcall*)(void*, int, int, unsigned);
using ExpireFn = void(__fastcall*)(void*, int);
using ScheduleFn = void(__fastcall*)(void*, void*, int, int, int, int, int);
inline ListFn OriginalList = nullptr;
inline GetFn Get = nullptr;
inline SetFn Set = nullptr;
inline SkillFn Skill = nullptr;
inline HitFn OriginalHit = nullptr;
inline RegenFn OriginalRegen = nullptr;
inline UnitGetFn GetUnit = nullptr;
inline UnitSetFn SetUnit = nullptr;
inline ExpireFn SetExpiry = nullptr;
inline ScheduleFn Schedule = nullptr;
inline bool (*RegenPending)(void*, void*) noexcept = nullptr;
inline void (*ExtraTick)(void*, void*) noexcept = nullptr;
inline void (*BeforeRegen)(void*) noexcept = nullptr;
inline bool (*NeedsTick)(void*) noexcept = nullptr;
inline void (*Log)(const char*) noexcept = nullptr;
inline unsigned LogCount = 0;
constexpr int RegenStat = 74, BaseStat = 403, StartStat = 404, SecondStat = 405, RateStat = 406,
              EndStat = 410, TransportStat = 418;

// Capture the native poison state before the hit so unchanged applications can be skipped.
struct Snapshot {
    void* unit = nullptr;
    void* list = nullptr;
    int rate = 0;
    std::uint32_t expiry = 0;
};

struct HitContext {
    void* game = nullptr;
    std::array<Snapshot, 2048> targets{};
    unsigned count = 0;
};

inline thread_local HitContext* Active = nullptr;

inline unsigned char Version(void* game) noexcept {
    return *(static_cast<unsigned char*>(game) + 0x106);
}

inline unsigned Frame(void* game) noexcept {
    return *reinterpret_cast<unsigned*>(static_cast<unsigned char*>(game) + 0x170);
}

// Compare raw expiry bits for change detection. The engine stores the frame as a float.
inline std::uint32_t Expiry(void* list) noexcept {
    std::uint32_t value = 0;
    if (list)
        std::memcpy(&value, static_cast<unsigned char*>(list) + 0x24, 4);
    return value;
}

inline unsigned ExpireFrame(void* list) noexcept {
    float value = 0;
    if (list)
        std::memcpy(&value, static_cast<unsigned char*>(list) + 0x24, 4);
    return value > 0 && value < 2147483647.f ? static_cast<unsigned>(value) : 0;
}

inline int IncreasedRate(int base, unsigned seconds) noexcept {
    const double value = double(base) * std::pow(1.20, double(seconds));
    if (!std::isfinite(value) || value >= double(std::numeric_limits<int>::max()))
        return std::numeric_limits<int>::max();
    return static_cast<int>(std::floor(value + 0.5));
}

inline void* __fastcall OnList(void* unit, int state) noexcept {
    void* result = OriginalList(unit, state);
    if (!Active || state != 2 || !unit || *static_cast<unsigned*>(unit) != 1)
        return result;
    for (unsigned i = 0; i < Active->count; ++i)
        if (Active->targets[i].unit == unit)
            return result;
    if (Active->count < Active->targets.size()) {
        auto& s = Active->targets[Active->count++];
        s.unit = unit;
        s.list = result;
        s.expiry = Expiry(result);
        s.rate = result ? Get(Version(Active->game), result, RegenStat, 0) : 0;
    }
    return result;
}

inline int __fastcall OnHit(void* game, void* missile, void* target) noexcept {
    const unsigned id = missile ? static_cast<unsigned*>(missile)[1] : 0;
    if (!game || (id != 749 && id != 750 && id != 751))
        return OriginalHit(game, missile, target);
    if (id != 749 && GetUnit && SetUnit) {
        if (GetUnit(missile, 407, 0))
            return 1;
        SetUnit(missile, 407, 1, 0);
    }
    // The native area hit queries each target through OnList while this context is active.
    HitContext context{};
    context.game = game;
    auto* previous = Active;
    Active = &context;
    const int result = OriginalHit(game, missile, target);
    Active = previous;
    const auto version = Version(game);
    const auto now = Frame(game);
    const int state = id == 749 ? 9 : 19;
    for (unsigned i = 0; i < context.count; ++i) {
        const auto& s = context.targets[i];
        void* poison = OriginalList(s.unit, 2);
        void* curse = OriginalList(s.unit, state);
        if (!poison || !curse)
            continue;
        const int skill = Skill(curse);
        if (state == 9 ? skill != 66 : (skill != 77 && skill != 444))
            continue;
        const int rate = Get(version, poison, RegenStat, 0);
        if (rate >= 0 || rate == std::numeric_limits<int>::min())
            continue;
        if (poison == s.list && rate == s.rate && Expiry(poison) == s.expiry)
            continue;
        const int oldBase = Get(version, curse, BaseStat, 0);
        if (oldBase != -rate) {
            Set(version, curse, BaseStat, -rate, 0);
            Set(version, curse, StartStat, static_cast<int>(now), 0);
            Set(version, curse, SecondStat, 0, 0);
        }
        const unsigned start = static_cast<unsigned>(Get(version, curse, StartStat, 0));
        const int current =
            state == 19 ? IncreasedRate(-rate, now >= start ? (now - start) / 25 : 0) : -rate;
        Set(version, curse, RateStat, current, 0);
        Set(version, curse, RegenStat, -current, 0);
        const unsigned end = ExpireFrame(poison);
        Set(version, curse, EndStat, static_cast<int>(end), 0);
        // Native poison supplies resistance, duration, visuals and kill attribution.
        // Each curse owns its rate so both DoTs can coexist and expire independently.
        Set(version, poison, RegenStat, 0, 0);
        Set(version, poison, TransportStat, 1, 0);
        unsigned longest = end;
        for (int other : {9, 19})
            if (void* list = OriginalList(s.unit, other)) {
                const unsigned e = static_cast<unsigned>(Get(version, list, EndStat, 0));
                if (e > longest && Get(version, list, BaseStat, 0) > 0)
                    longest = e;
            }
        if (SetExpiry && longest > end)
            SetExpiry(poison, static_cast<int>(longest));
        // Seed regeneration only if the native poison hit left no pending regen event.
        if (Schedule) {
            if (!RegenPending || !RegenPending(game, s.unit))
                Schedule(game, s.unit, 3, static_cast<int>(now + 1), 0, 0, 0);
            Schedule(game, s.unit, 12, static_cast<int>(longest), 0, 0, 0);
        }
        if (Log && LogCount++ < 256) {
            char m[170]{};
            std::snprintf(m, sizeof(m),
                          "Independent curse poison: unit=%u skill=%d base=%d expiry=%u",
                          static_cast<unsigned*>(s.unit)[2], skill, -rate, end);
            Log(m);
        }
    }
    return result;
}

inline void TimedTick(void* game, void* unit) noexcept {
    if (game && unit) {
        const auto version = Version(game);
        const auto now = Frame(game);
        for (int state : {9, 19})
            if (void* list = OriginalList(unit, state)) {
                const int base = Get(version, list, BaseStat, 0);
                if (base <= 0)
                    continue;
                const unsigned end = static_cast<unsigned>(Get(version, list, EndStat, 0));
                if (end <= now) {
                    Set(version, list, RegenStat, 0, 0);
                    Set(version, list, BaseStat, 0, 0);
                    continue;
                }
                if (state == 19) {
                    const unsigned start = static_cast<unsigned>(Get(version, list, StartStat, 0)),
                                   seconds = now >= start ? (now - start) / 25 : 0;
                    if (seconds > static_cast<unsigned>(Get(version, list, SecondStat, 0))) {
                        const int rate = IncreasedRate(base, seconds);
                        Set(version, list, RegenStat, -rate, 0);
                        Set(version, list, RateStat, rate, 0);
                        Set(version, list, SecondStat, static_cast<int>(seconds), 0);
                    }
                }
            }
        if (ExtraTick)
            ExtraTick(game, unit);
    }
}

inline void __fastcall OnRegen(void* game, void* unit, int a, int b) noexcept {
    if (BeforeRegen)
        BeforeRegen(game);
    else
        TimedTick(game, unit);
    OriginalRegen(game, unit, a, b);
    // Native regeneration schedules its own successor. Adding another here
    // doubles that event chain every frame: runaway healing and server stalls.
    // Timed curses already advance independently in the server-frame pump.
}
} // namespace Ramp
