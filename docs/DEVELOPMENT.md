# Development notes

## Scope

The checked-in C++ code builds the custom `fmmp-plague-trace.dll` curse adapter and its seven regression executables. It expects the matching v0.5.22 MPQ shipped in the Release asset. This repository alone does not recreate the complete randomized Frankenstein archive or the upstream mod's other DLLs.

The DLL's historical filename is retained for plugin identity and compatibility. Its displayed name is **Frankenstein Curse Overhaul**, version **0.5.22**.

## Build

Use Windows, the x64 MSVC C++ toolchain, and a Windows SDK with the resource compiler. From an x64 Native Tools Command Prompt, run:

```bat
tools\build.cmd
```

The script compiles the plugin and runs the seven regression suites. Outputs are placed in `build/` and are ignored by Git.

| Test | Coverage |
| --- | --- |
| `virus_test` | Infection-before-explosion ordering, nested death chains, duplicate death guards |
| `ramp_test` | Independent poison rates, compounding growth, refresh/expiry, native poison coexistence |
| `pulse_test` | Immediate curse application and single-pulse guards |
| `curses_test` | Stacking behavior, vulnerability, Decrepify progression, procs, frame pump |
| `multitarget_test` | Independent multi-target progression, fire target delivery, cooldowns, skeleton procs |
| `regen_timer_test` | Single native regeneration chain, pending-timer detection, cycle/expiry guards |
| `soul_harvest_test` | Distinct drops, room and radius checks, healing, duplicate pickup protection, independent stack expiry |

## Native integration

- Native hooks are specific to the tested D2R build. Expected byte sequences are checked before binding.
- Stat layers use the current 32-bit ABI, including stack-passed layer arguments.
- Skeleton ownership is resolved through the native minion AI rather than assuming the general projectile-owner fields are populated.
- Timed curse progression runs through the server-frame pump. It must not add a second successor to the native health-regeneration callback.
- Fire uses one marked-target native missile hit per pulse; its consumed target tag prevents duplicate delivery.
- A startup probe exercises isolated stat-list aggregation, wide layers, the owner lookup, the compiled growth formula, the compiled Soul Harvest healing formula, and native pending/expired/unrelated timer records.

Do not replace the native hit handler with manual health subtraction or suppress native healing to mask a timer bug. Mock-based tests and startup probes establish limited properties; record actual combat feedback separately.

The Soul Harvest server hook queues a drop from a marked corpse and creates the pickup on a server frame. It tracks resource bonuses in basis points and keeps expiry per pickup frame. Matching skill and description formulas live in the shipped MPQ. The custom test simulates the native stat lists, missile creation, and pickup callback.

## Releasing

Keep game archives and compiled bundles out of Git history. Build or prepare the matching complete mod environment, run the checks, and attach its portable ZIP and SHA-256 file to a GitHub Release.

The current v0.5.22 bundle's identities are recorded in `release.json`. Twelve upstream companion plugins contain exact data/dependency hash checks; changing the MPQ requires updating and validating those guard values while preserving their executable sections. Compiling only this custom DLL does not perform that archive/plugin adaptation.

No local absolute installation path is required by the source build script. The mod-scoped technical name `FMMP-Experimental-R67` is intentional and must remain consistent.
