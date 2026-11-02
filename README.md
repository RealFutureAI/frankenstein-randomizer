# Frankenstein Randomizer

A portable **Diablo II: Resurrected** modpack combining **Frankenstein / FMMP Experimental R67**, prebuilt randomized game data, and a custom Necromancer curse overhaul.

The current gameplay customization version is **0.5.13**. The download includes the complete configured mod, its **19 matching plugins**, and the matching **D2RLoader 1.3.1-beta**. You do not need to run ModGen or assemble several mods yourself.

**[Download v0.5.13](https://github.com/RealFutureAI/frankenstein-randomizer/releases/tag/v0.5.13)** · [Installation](docs/INSTALLATION.md) · [Curse reference](docs/CURSES.md) · [Changelog](CHANGELOG.md)

> **Experimental gameplay build.** Automated regression tests and startup checks pass. The latest balance values have not yet received a complete in-game combat test. This is a customized FMMP distribution, maintained independently of its upstream authors.

## What is included?

- The complete, currently configured Frankenstein Randomizer MPQ, including its randomized game tables and existing base-mod content.
- A redesigned Necromancer curse tree with **Plague**, **Infection**, **Elemental Marks**, **Curse Mastery**, and **Plague Lord**.
- Additional effects for Confuse, Attract, Decrepify, Lower Resist, and Life Tap.
- A complete hard-point synergy system for the active curses.
- Custom curse skill icons and recolored overhead effects.
- Normal Skeleton Warriors that can be summoned **without a corpse**.
- All 19 matching mod-scoped DLL plugins, the map-generation helper, mod configuration, sounds, and bundled license notices.
- A clean, matching D2RLoader distribution and a relative-path Windows launcher.

The repository stays small: documentation, release metadata, verification tools, and the custom curse plugin source live in Git. The approximately **223 MB portable package is a Release asset**, rather than a large file in Git history. GitHub's automatically generated “Source code” archives are developer files; use the named portable ZIP to play.

## Requirements

| Component | Tested configuration |
| --- | --- |
| Game | Your own Diablo II: Resurrected installation with Reign of the Warlock |
| Game build | **3.3.93847** |
| Loader | **D2RLoader 1.3.1-beta**, included in the portable ZIP |
| Platform | Windows, 64-bit |
| Technical mod name | **`FMMP-Experimental-R67`** — keep this name unchanged |

The plugin contains native hooks for the tested game build. A newer game or loader version needs a fresh compatibility check. The pack contains no base-game installation, account data, characters, or shared stashes.

## Quick installation

1. Download `Frankenstein-Randomizer-0.5.13-Portable.zip` from the [release page](https://github.com/RealFutureAI/frankenstein-randomizer/releases/tag/v0.5.13).
2. Close D2R and D2RLoader. If the same mod or loader is already installed, back up that installation's configuration and mod folder first. Keep old mod folders outside the active `Mods` folder while installing this complete build.
3. Extract the **entire ZIP directly into your D2R game folder**, next to `D2R.exe` and `Data`. Do not extract it into a second nested game folder.
4. Run **`FMMP_STARTEN.cmd`**, or launch `D2RLoader.exe` directly. The supplied clean loader configuration selects `FMMP-Experimental-R67` automatically.
5. Check the selected mod and keep **mod extensions enabled**. Create a new Reign of the Warlock character for this mod environment.

Expected layout:

```text
Diablo II Resurrected/
├── D2R.exe                         # Your existing game
├── Data/                           # Your existing game data
├── D2RLoader.exe
├── D2RCore.dll
├── d2rloader.mpq
├── FMMP_STARTEN.cmd
├── PRUEFEN.ps1
├── DATEIEN-SHA256.json
├── d2rloader/config/d2rloader.toml
└── mods/FMMP-Experimental-R67/
    ├── FMMP-Experimental-R67.mpq
    └── d2rloader/
        ├── metadata.json
        ├── config/
        ├── plugins/
        ├── sounds/
        └── licenses/
```

The portable ZIP's short installation notes are in German; the instructions in this repository are in English. See [the full installation guide](docs/INSTALLATION.md) for updates, checksums, saves, and multiplayer.

## Necromancer curse overhaul

All damage figures below are **before equipment modifiers and enemy resistances**. `L` means effective skill level, starting at 1. Native fixed-point rounding and native immunity/slow limits still apply.

| Skill | Main behavior in v0.5.13 |
| --- | --- |
| **Plague** | A poison curse dealing `25 + 12.5 × (L − 1)` damage per second before synergies. Marked deaths trigger an explosion based on 20% of corpse base life. |
| **Infection** | A small initial curse with `12.5 + 5 × (L − 1)` starting poison DPS. Damage compounds by 20% each full second. On death, a 33% corpse-base-life explosion spreads Infection over a larger area. |
| **Confuse** | Confuses enemies and adds Iron Maiden-style reflection of physical **melee** damage. |
| **Attract** | Draws enemies to the marked target. Adds physical vulnerability of 5 percentage points per skill level and a death explosion starting at 60% of corpse base life. |
| **Decrepify** | Starts at **0%** strength and gains **3 percentage points each second**, before growth synergies, up to a computed 90% maximum. |
| **Lower Resist** | Reduces fire, cold, lightning, poison, physical, and magic resistance. Its previous base curve is halved; hard-point synergies add 0.75% each. |
| **Life Tap** | Native-style Life Tap healing, starting at **25%**, with healing synergies. |
| **Elemental Marks** | Automatic compact fire pulses, a larger Frost Nova after every third registered hit, and Chain Lightning on qualifying hits. |
| **Curse Mastery** | A one-point passive allowing different supported curses to coexist on the same enemy. Reapplying the same curse refreshes it rather than creating duplicate copies. |
| **Plague Lord** | Normal Skeleton Warriors have `min(100, 1 + L)%` chance on a successful physical hit to cast one of the eight active curses. Successful procs have a one-second cooldown per warrior. |

Poison damage from Plague and Infection is **flat damage**, rather than a percentage of the living target's maximum health. Death explosions use **corpse base life**, not the monster's scaled current maximum health.

### Elemental Marks

| Element | Base damage before synergies | Trigger |
| --- | --- | --- |
| Fire | `9.375 + 3.75 × (L − 1)` per pulse | Immediately on application, then at most once every **3 seconds per target** |
| Cold | `37.5 + 15 × (L − 1)` per Frost Nova | Every **third registered hit** |
| Lightning | `0.25` to `25 + 7.5 × (L − 1)` | Qualifying hits, with a **1-second cooldown per target** |

Fire uses a short impact effect instead of leaving persistent burning ground. Frost Nova uses a larger 32-ray pattern; its base cold duration is `8 + 0.8 × (L − 1)` seconds before resistance and difficulty adjustments. Chain Lightning has a maximum of **five hits total**, or four additional jumps, and may jump to enemies that are not cursed.

The elemental effects do not recursively trigger themselves. The precise registered-hit rules, native limits, and implementation notes are documented in [the curse reference](docs/CURSES.md).

### Hard-point synergies

Only invested skill points count. Item-granted skill levels do not add synergy points. The percentage is a **relative bonus to the listed effect per hard point**, added across the listed sources.

| Recipient | Bonus per hard point | Sources | Effect increased |
| --- | ---: | --- | --- |
| Plague | 10% | Infection, Corpse Explosion, Poison Explosion | Poison damage |
| Infection | 10% | Plague, Corpse Explosion, Poison Explosion | Poison damage |
| Life Tap | 10% | Confuse, Attract | Healing |
| Confuse | 15% | Elemental Marks, Attract | Melee reflection |
| Attract | 8% | Confuse, Decrepify, Infection | Death-explosion damage |
| Decrepify | 5% | Confuse, Attract, Lower Resist | Growth rate |
| Lower Resist | 0.75% | Plague, Infection, Elemental Marks | Resistance reduction |
| Elemental Marks | 15% | Confuse, Lower Resist | All three elemental damage types |

**Curse Mastery and Plague Lord neither receive nor provide synergies.**

## Save separation and playing with friends

Each player's characters and shared stash are stored in their own Windows profile under:

```text
Saved Games/Diablo II Resurrected/mods/FMMP_Randomizer/
```

The package includes no saves. Use new characters for this mod environment and keep backups of your own saves. Everyone in a multiplayer session should use the **same release, game build, and loader version**. Use D2RLoader's TCP/IP/LAN multiplayer features; this repository does not host a game server or manage network access for you.

## Validation and current limitations

- All **3,388 MPQ members** were checked when packaging the current balance update.
- The package's **50 payload files** have recorded SHA-256 hashes, and the ZIP was checked for CRC errors and file-hash mismatches.
- All **six custom regression suites** pass: death/infection chains, poison ramps, immediate curse pulses, curse behavior, multi-target behavior, and regeneration-timer handling.
- Startup completed with **19 plugins loaded** and **188 tables compiled**.
- Native probes cover 32-bit stat layers, aggregate resistance updates, skeleton-owner resolution, the compiled Decrepify growth formula, and pending-regeneration-timer detection.

These checks do **not** replace combat testing. The latest balance changes and the fire/regen correction still need broader gameplay feedback. Three pre-existing baseline assertions remain during table startup; the customization introduces no additional startup assertions in the tested setup.

Overhead curse colors are customized. Blue Decrepify and cyan Lower Resist were confirmed visible in-game. The underlying overhead motion still uses the native Confuse question-mark animation: fully distinct custom symbol silhouettes are not yet working. The skill icons themselves are separate custom assets.

## Source and development

`src/` contains the current **custom curse adapter** and its six regression tests. It does not contain the full Frankenstein base-mod source or regenerate the randomized MPQ. The vendored D2RLoader plugin SDK keeps its included MIT license.

To build on Windows, open an **x64 Native Tools Command Prompt for Visual Studio** with the C++ compiler and Windows SDK available, then run:

```bat
tools\build.cmd
```

Outputs go to the ignored `build/` directory. The custom DLL must be used with the matching v0.5.13 mod data and tested game build. Native addresses are build-specific. See [development notes](docs/DEVELOPMENT.md).

## Credits and upstream projects

- **Frankenstein / FMMP Experimental R67** and its contributors: the base mod and bundled systems.
- [D2RLoader](https://d2rloader.net/) and its contributors: loader, runtime, and plugin SDK.
- [D2ModGen](https://github.com/mapron/d2modgen): randomizer tooling used to prepare the game data.
- **RuffnecKk MapSense** and its contributors: map integration and the bundled map-generation helper.
- The authors of the other bundled FMMP plugins and third-party dependencies. Their notices are preserved in the portable package.

This is an independent customization, not an official Blizzard, FMMP, D2RLoader, or D2ModGen release. Upstream software, game assets, and bundled dependencies retain their respective copyrights and license terms. See [NOTICE.md](NOTICE.md).
