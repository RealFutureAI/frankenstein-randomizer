#pragma once
#include "ramp.h"
#include <cstddef>
#include <cstdint>

namespace Pulse {
using CreateFn = void*(__fastcall*)(void*, void*);
inline CreateFn OriginalCreate = nullptr;
inline bool (*FilterCreate)(void*, void*) noexcept = nullptr;
inline void (*AfterCreate)(void*, void*, void*) noexcept = nullptr;
inline unsigned Logs = 0;

// Verified native 3.3.93847 creation layout. Range override is read at 0x5c
// only with flags bit 15; unused intervening fields remain opaque.
struct Params {
    unsigned flags;
    unsigned padding;
    void* owner;
    void* origin;
    void* target;
    int missile, x, y, targetX, targetY, loops, velocity, skill, level;
    unsigned char reservedTail[0x18];
    int range;
};

static_assert(offsetof(Params, missile) == 0x20 && offsetof(Params, x) == 0x24
              && offsetof(Params, targetX) == 0x2c && offsetof(Params, skill) == 0x3c
              && offsetof(Params, range) == 0x5c);

inline void* Create(void* game, void* raw) noexcept {
    void* missile = OriginalCreate(game, raw);
    if (missile && AfterCreate)
        AfterCreate(game, raw, missile);
    return missile;
}

inline void* __fastcall OnCreate(void* game, void* raw) noexcept {
    if (FilterCreate && !FilterCreate(game, raw))
        return nullptr;
    auto* p = static_cast<Params*>(raw);
    if (!p || (p->missile != 750 && p->missile != 751))
        return Create(game, raw);
    if (p->missile == 750) {
        p->x = p->targetX;
        p->y = p->targetY;
    }
    // Use the cast location for manual infection, and the corpse location for
    // spread. Absolute position, no origin/distance-driven flight, no target AI.
    p->flags = (p->flags | 0x21U) & ~0x402U;
    p->target = nullptr;
    p->targetX = p->x + 1;
    p->targetY = p->y;
    void* missile = Create(game, raw);
    if (missile)
        Ramp::OnHit(game, missile, nullptr);
    if (Ramp::Log && Logs++ < 256) {
        char message[160]{};
        std::snprintf(message, sizeof(message),
                      "Virus immediate pulse: missile=%d x=%d y=%d created=%d", p->missile, p->x,
                      p->y, missile ? 1 : 0);
        Ramp::Log(message);
    }
    return missile;
}
} // namespace Pulse
