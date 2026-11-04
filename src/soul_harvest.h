#pragma once

#include "curses.h"

namespace SoulHarvest {
constexpr int Skill = 76;
constexpr int CurseState = 776;
constexpr int BuffState = 775;
constexpr unsigned OrbMissile = 81; // Native Sanctuary ground effect, available in both renderers.
constexpr int PickupRadius = 9;
constexpr int OrbLifetime = 500;
constexpr int OrbHealStat = 424;
constexpr int OrbBonusStat = 425;
constexpr unsigned BuffFrames = 15 * 25;
constexpr int MaxBonus =
    3000000; // Safety bound in basis points for the native fixed-point resource stats.

using RoomFn = void*(__fastcall*)(void*);
inline RoomFn Room = nullptr;
using RefreshFn = void(__fastcall*)(void*);
inline RefreshFn Refresh = nullptr;
using GfxStateFn = void(__fastcall*)(void*, int, int);
using PlayerDataFn = void*(__fastcall*)(void*);
using SyncMissileFn = void(__fastcall*)(void*, void*, void*, int);
inline GfxStateFn MarkGfxDirty = nullptr;
inline PlayerDataFn PlayerData = nullptr;
inline SyncMissileFn SyncMissile = nullptr;
inline unsigned DropLogs = 0, PickupLogs = 0, SyncLogs = 0;

// Pickups in the same frame share one bucket because their bonuses expire together.
struct ExpiryBucket {
    unsigned frame = 0;
    int bonus = 0;
};

struct PlayerBuff {
    unsigned playerId = 0;
    unsigned checkedFrame = 0;
    int total = 0;
    std::array<ExpiryBucket, BuffFrames> expirations{};
};

// Capture the coordinates and healing while the corpse and curse owner are still available.
struct PendingDrop {
    unsigned corpseId;
    unsigned casterId;
    int x, y, level, heal;
};

inline void* Session = nullptr;
inline unsigned LastFrame = 0;
inline bool PlayersCached = false;
inline std::array<unsigned, 32> PlayerIds{};
inline unsigned PlayerCount = 0;
inline std::array<PlayerBuff, 32> Buffs{};
inline std::array<PendingDrop, 1024> Drops{};
inline unsigned DropCount = 0;
inline constexpr std::uint64_t RoomRva = 0x34b440;
inline constexpr unsigned char RoomBytes[] = {0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x48, 0x8b, 0xd9};
inline constexpr std::uint64_t PlayerBucketRva = 0x49260f;
inline constexpr unsigned char PlayerBucketBytes[] = {0x41, 0x8b, 0xc0, 0x83, 0xe0, 0x7f,
                                                      0x48, 0x05, 0x46, 0x04, 0x00, 0x00};
inline constexpr std::uint64_t RefreshRva = 0x438970;
inline constexpr unsigned char RefreshBytes[] = {0x40, 0x53, 0x48, 0x83, 0xec, 0x20,
                                                 0xb2, 0x01, 0x48, 0x8b, 0xd9};
inline constexpr std::uint64_t GfxStateRva = 0x3355a0;
inline constexpr unsigned char GfxStateBytes[] = {0x48, 0x89, 0x5c, 0x24, 0x10, 0x55, 0x56,
                                                  0x41, 0x56, 0x48, 0x83, 0xec, 0x30};
inline constexpr std::uint64_t PlayerDataRva = 0x34b240, PlayerClientFieldRva = 0x5005f6;
inline constexpr unsigned char PlayerDataBytes[] = {0x40, 0x53, 0x48, 0x83, 0xec,
                                                    0x20, 0x48, 0x8b, 0xd9};
inline constexpr unsigned char PlayerClientFieldBytes[] = {0x4c, 0x8b, 0xb0, 0x98,
                                                           0x00, 0x00, 0x00};
inline constexpr std::uint64_t SyncMissileRva = 0x537bc0;
inline constexpr unsigned char SyncMissileBytes[] = {
    0x40, 0x55, 0x53, 0x57, 0x41, 0x56, 0x41, 0x57, 0x48, 0x8d, 0xac, 0x24, 0x50, 0xff, 0xff, 0xff};

inline unsigned UnitId(const void* unit) noexcept {
    return static_cast<const unsigned*>(unit)[2];
}

inline void ResetIfNewGame(void* game, unsigned frame) noexcept {
    if (game != Session || frame < LastFrame) {
        Session = game;
        PlayersCached = false;
        DropCount = 0;
        for (auto& buff : Buffs)
            buff = {};
    }
    if (frame != LastFrame)
        PlayersCached = false;
    LastFrame = frame;
}

inline bool LivingPlayer(void* unit) noexcept {
    if (!Curses::Player(unit))
        return false;
    const unsigned mode = static_cast<unsigned*>(unit)[3];
    return mode != 0 && mode != 17 && Ramp::GetUnit(unit, 6, 0) > 0;
}

inline void CachePlayerIds(void* game) noexcept {
    if (PlayersCached)
        return;
    PlayersCached = true;
    PlayerCount = 0;
    auto* raw = static_cast<unsigned char*>(game);
    // The native unit lookup selects one of these 128 player buckets by GUID.
    for (unsigned bucket = 0; bucket < 128 && PlayerCount < PlayerIds.size(); ++bucket) {
        void* unit = nullptr;
        std::memcpy(&unit, raw + 0x2230 + bucket * sizeof(unit), sizeof(unit));
        for (unsigned links = 0; unit && links < PlayerIds.size() && PlayerCount < PlayerIds.size();
             ++links) {
            if (LivingPlayer(unit))
                PlayerIds[PlayerCount++] = UnitId(unit);
            auto* next = static_cast<unsigned char*>(unit) + 0x158;
            std::memcpy(&unit, next, sizeof(unit));
        }
    }
}

inline void SyncOrbToPlayer(void* game, void* orb, void* player) noexcept {
    if (!PlayerData || !SyncMissile)
        return;
    const unsigned layer = UnitId(player) + 1U; // Layer zero still stores the healing amount.
    if (!layer || Ramp::GetUnit(orb, OrbHealStat, layer))
        return;
    void* data = PlayerData(player);
    if (!data)
        return;
    void* client = nullptr;
    std::memcpy(&client, static_cast<unsigned char*>(data) + 0x98, sizeof(client));
    if (!client)
        return;

    // This orb is created on the server without a corresponding client cast.
    // Send the engine's own missile packet, which carries the server missile ID.
    SyncMissile(client, game, orb, 0);
    Ramp::SetUnit(orb, OrbHealStat, 1, layer);
    if (Ramp::Log && SyncLogs++ < 24) {
        char text[120]{};
        std::snprintf(text, sizeof(text), "Soul Harvest visual sync: orb=%u player=%u", UnitId(orb),
                      UnitId(player));
        Ramp::Log(text);
    }
}

// Resource percentages use basis points so the native resource calculation retains fractions.
inline int BaseHealing(int level) noexcept {
    return 100 + 30 * std::clamp(level, 1, 1000);
}

inline int MaximumBonus(int level) noexcept {
    return 50 + 8 * std::clamp(level, 1, 1000);
}

// Defer missile creation to the server frame and mark this corpse as already queued.
inline void QueueDrop(void* game, void* corpse, void* caster, int level) noexcept {
    if (!game || !Curses::Monster(corpse) || !Curses::Player(caster) || level < 1)
        return;
    ResetIfNewGame(game, Ramp::Frame(game));
    void* mark = Ramp::OriginalList(corpse, CurseState);
    if (!mark || Ramp::Skill(mark) != Skill || Curses::Caster(game, mark) != caster)
        return;
    // Both native death paths can report the same corpse. Reserve its drop only once.
    if (Curses::Read(game, mark, OrbBonusStat) || DropCount == Drops.size())
        return;
    Curses::Write(game, mark, OrbBonusStat, 1);
    const int healing =
        std::clamp(Curses::EvalCalc4(game, caster, Skill, level, BaseHealing(level)), 1, 10000);
    Drops[DropCount++] = {UnitId(corpse),    UnitId(caster), Curses::X(corpse),
                          Curses::Y(corpse), level,          healing};
}

inline void FlushDrops(void* game) noexcept {
    const unsigned count = DropCount;
    for (unsigned i = 0; i < count; ++i) {
        const auto drop = Drops[i];
        void* caster = Curses::Find(game, 0, drop.casterId);
        if (!Curses::Player(caster))
            continue;
        void* orb = Curses::CreateAt(game, caster, OrbMissile, Skill, drop.level, drop.x, drop.y,
                                     drop.x + 1, drop.y, nullptr, OrbLifetime);
        if (!orb)
            continue;
        Ramp::SetUnit(orb, OrbHealStat, drop.heal, 0);
        Ramp::SetUnit(orb, OrbBonusStat, MaximumBonus(drop.level), 0);
        if (Ramp::Log && DropLogs++ < 64) {
            char text[120]{};
            std::snprintf(text, sizeof(text), "Soul Harvest orb: corpse=%u orb=%u level=%d",
                          drop.corpseId, UnitId(orb), drop.level);
            Ramp::Log(text);
        }
    }
    // Preserve any entries queued by a reentrant callback during native creation.
    for (unsigned i = count; i < DropCount; ++i)
        Drops[i - count] = Drops[i];
    DropCount -= count;
}

inline void SetResource(void* player, int stat, int maxStat, int oldValue, int heal) noexcept {
    const int maximum = Ramp::GetUnit(player, maxStat, 0);
    if (maximum <= 0)
        return;
    const auto next = std::int64_t{oldValue} + std::int64_t{maximum} * heal / 10000;
    Ramp::SetUnit(player, stat, static_cast<int>(std::clamp<std::int64_t>(next, 0, maximum)), 0);
}

inline void ApplyStats(void* game, void* player, void* list, int bonus, int heal) noexcept {
    const int life = Ramp::GetUnit(player, 6, 0);
    const int mana = Ramp::GetUnit(player, 8, 0);
    // Derive a percentage of the character's current, unbuffed maximum. The
    // native max-resource stats use 256ths, so odd half-percent stacks keep
    // their precision. Only the temporary list receives the flat deltas.
    for (const int stat : {7, 9}) {
        Curses::Write(game, list, stat, 0);
        const int base = std::max(0, Ramp::GetUnit(player, stat, 0));
        const auto gain = std::int64_t{base} * std::clamp(bonus, 0, MaxBonus) / 10000;
        Curses::Write(
            game, list, stat,
            static_cast<int>(std::min<std::int64_t>(gain, std::numeric_limits<int>::max() - base)));
    }
    SetResource(player, 6, 7, life, heal);
    SetResource(player, 8, 9, mana, heal);
    if (Refresh)
        Refresh(player); // Same refresh used after a native warcry buff.
    if (MarkGfxDirty)
        MarkGfxDirty(player, BuffState, 1); // Resend the changed state list to the client.
    Curses::Update(player, "Soul Harvest", 0);
}

inline bool Collect(void* game, void* player, unsigned frame, int heal, int bonus) noexcept {
    PlayerBuff* slot = nullptr;
    for (auto& buff : Buffs) {
        if (buff.total && buff.playerId == UnitId(player)) {
            slot = &buff;
            break;
        }
        if (!buff.total && !slot)
            slot = &buff;
    }
    if (!slot)
        return false;

    const unsigned expiry = frame + BuffFrames;
    void* list = Ramp::OriginalList(player, BuffState);
    if (list) {
        Ramp::SetExpiry(list, static_cast<int>(expiry));
        Ramp::Schedule(game, player, 12, static_cast<int>(expiry), 0, 0, 0);
    } else {
        // A non-curse state gets its own timed stat list without replacing active curses.
        Curses::Curse request{player, player, Skill,     1,      static_cast<int>(BuffFrames),
                              -1,     0,      BuffState, nullptr};
        list = Curses::OriginalApply(&request);
    }
    if (!list)
        return false;

    if (!slot->total) {
        slot->playerId = UnitId(player);
        slot->checkedFrame = frame;
    }
    auto& bucket = slot->expirations[frame % BuffFrames];
    if (bucket.bonus && bucket.frame != expiry) {
        slot->total -= bucket.bonus;
        bucket = {};
    }
    bucket.frame = expiry;
    bucket.bonus += bonus;
    slot->total += bonus;
    ApplyStats(game, player, list, slot->total, heal);
    if (Ramp::Log && PickupLogs++ < 64) {
        char text[176]{};
        std::snprintf(
            text, sizeof(text),
            "Soul Harvest pickup: player=%u stackBp=%d totalBp=%d maxHp256=%d maxMana256=%d",
            slot->playerId, bonus, slot->total, Ramp::GetUnit(player, 7, 0),
            Ramp::GetUnit(player, 9, 0));
        Ramp::Log(text);
    }
    return true;
}

inline void TickBuffs(void* game) noexcept {
    const unsigned frame = Ramp::Frame(game);
    ResetIfNewGame(game, frame);
    for (auto& buff : Buffs) {
        if (!buff.total)
            continue;
        void* player = Curses::Find(game, 0, buff.playerId);
        if (!LivingPlayer(player)) {
            buff = {};
            continue;
        }
        const unsigned elapsed = std::min(frame - buff.checkedFrame, BuffFrames);
        const int oldTotal = buff.total;
        for (unsigned offset = elapsed; offset > 0; --offset) {
            const unsigned tick = frame - offset + 1;
            auto& bucket = buff.expirations[tick % BuffFrames];
            if (!bucket.bonus || bucket.frame > frame)
                continue;
            buff.total -= bucket.bonus;
            bucket = {};
        }
        buff.checkedFrame = frame;
        if (buff.total != oldTotal) {
            if (void* list = Ramp::OriginalList(player, BuffState))
                ApplyStats(game, player, list, buff.total, 0);
        }
    }
}

inline int __fastcall OnMissileTick(void* game, void* missile) noexcept {
    if (!game || !missile || static_cast<unsigned*>(missile)[1] != OrbMissile)
        return Curses::OnMissileTick(game, missile);

    const int heal = Ramp::GetUnit(missile, OrbHealStat, 0);
    const int bonus = Ramp::GetUnit(missile, OrbBonusStat, 0);
    void* room = heal > 0 && bonus > 0 ? Room(missile) : nullptr;
    if (room) {
        const unsigned frame = Ramp::Frame(game);
        ResetIfNewGame(game, frame);
        CachePlayerIds(game);
        const int x = Curses::X(missile), y = Curses::Y(missile);
        for (unsigned i = 0; i < PlayerCount; ++i) {
            void* player = Curses::Find(game, 0, PlayerIds[i]);
            if (!LivingPlayer(player) || Room(player) != room)
                continue;
            SyncOrbToPlayer(game, missile, player);
            const auto dx = std::int64_t{Curses::X(player)} - x;
            const auto dy = std::int64_t{Curses::Y(player)} - y;
            if (dx < -PickupRadius || dx > PickupRadius || dy < -PickupRadius ||
                dy > PickupRadius || dx * dx + dy * dy > PickupRadius * PickupRadius)
                continue;
            if (!Collect(game, player, frame, heal, bonus))
                continue;
            Ramp::SetUnit(missile, OrbBonusStat, 0, 0);
            return Curses::NativeMissileHit(game, missile, nullptr, 1);
        }
    }
    return Curses::OriginalMissileTick(game, missile);
}

inline void __fastcall OnFrame(void* game) noexcept {
    // Expire individual stacks before the native list expires on the last stack.
    if (game)
        TickBuffs(game);
    Curses::OnFrame(game);
    if (game)
        FlushDrops(game);
}
} // namespace SoulHarvest
