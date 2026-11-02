# Installation, updates, and sharing

## Download the playable package

Use the named `Frankenstein-Randomizer-0.5.13-Portable.zip` asset from the [v0.5.13 release](https://github.com/RealFutureAI/frankenstein-randomizer/releases/tag/v0.5.13). GitHub's “Source code” ZIP and TAR downloads contain this small documentation/source repository, not the playable modpack.

The package is **222,671,939 bytes** and includes the complete configured mod, 19 matching DLL plugins, the map helper, sounds, mod settings, license notices, and the matching clean D2RLoader release.

## Required software

- Your own Diablo II: Resurrected installation with Reign of the Warlock.
- The tested game build is **3.3.93847**.
- **D2RLoader 1.3.1-beta** is already supplied in the portable ZIP.
- A 64-bit Windows environment. The native plugins use the Microsoft Visual C++ runtime normally installed with the game's Windows prerequisites.

The loader executable, core DLL, and loader MPQ in the package were compared with the working installation. Do not replace them with a different release without checking compatibility.

## Install

1. Close D2R and D2RLoader.
2. Back up existing saves and any loader configuration you want to keep.
3. If `mods/FMMP-Experimental-R67` already exists, move that old folder out of the game's active `mods` directory before installing. Keep it as a backup rather than merging different mod versions.
4. Extract the entire portable ZIP into the directory that contains `D2R.exe` and `Data`.
5. Confirm that `FMMP_STARTEN.cmd` and `D2RLoader.exe` sit next to `D2R.exe`.
6. Start `FMMP_STARTEN.cmd` or `D2RLoader.exe`. The supplied global configuration automatically selects `FMMP-Experimental-R67`.
7. Keep mod extensions enabled. If you change the selected mod in the loader menu, apply it and restart when requested.
8. Create a new Reign of the Warlock character for this mod environment.

The package includes clean global loader settings with the required mod selection and matching gameplay options. Your previous hotkeys, UI preferences, and other loader settings are not imported automatically.

Do not rename the technical mod folder or its MPQ. The plugin is scoped to `FMMP-Experimental-R67`.

## Verify the download

The release includes a separate `.sha256` file. The v0.5.13 portable ZIP hash is:

```text
c92db9bc3b1dcb2de01892354adabe8ae816d62138c5a1e3dba2f36eac11365e
```

For example, in PowerShell:

```powershell
Get-FileHash -LiteralPath .\Frankenstein-Randomizer-0.5.13-Portable.zip -Algorithm SHA256
```

After extraction and **before the first launch**, the included `PRUEFEN.ps1` can compare the 50 payload files with `DATEIEN-SHA256.json` without changing them. Configuration files can legitimately change after you run the game or edit options, so this is an initial-installation check rather than a permanent requirement that every config remain unchanged.

## Characters and shared stash

The mod's relative save path is `FMMP_Randomizer/`. D2R resolves it inside each Windows user's own saved-games directory:

```text
Saved Games/Diablo II Resurrected/mods/FMMP_Randomizer/
```

No character, stash, account file, combat log, or debug/cheat settings are distributed. Keep this mod's characters and shared stash separate from your other game/mod environments. Back up your own saves before updates.

## Playing with friends

Use the same release ZIP, game build, and loader version on every machine. D2RLoader provides TCP/IP/LAN multiplayer. The package does not configure a router, create a public game listing, or share your saves with other players.

For the loader's own installation and multiplayer documentation, see [D2RLoader](https://d2rloader.net/docs.html).

## Updating or reverting

Close the game first. Keep a copy of the existing mod folder, loader configuration, and saves. Replace the active mod folder with the new complete release rather than layering new files over unknown old plugins or patches.

To revert, restore the matching older mod/loader setup and a compatible save backup. Save-format compatibility between different game or mod versions is not guaranteed by this repository.

## Troubleshooting

| Problem | Check |
| --- | --- |
| Launcher cannot find `D2R.exe` | The ZIP was probably extracted into a nested folder. Move its contents next to the existing game executable. |
| Wrong or missing mod | Check the selected mod in D2RLoader; keep the exact technical folder name. |
| Curses behave like the old version | Confirm that both the v0.5.13 MPQ and `fmmp-plague-trace.dll` were installed, and that mod extensions are enabled. |
| Plugin refuses to load after an update | Compare game/loader versions with the tested configuration. Native hook addresses are build-specific. |
| File verification differs after playing | Config changes are expected. Verify a freshly extracted copy to distinguish edited settings from a damaged download. |
| Combat regression | Report the release, skill level, difficulty, relevant synergies, other active curses, and exact steps. Attach only relevant logs after reviewing their contents. |

Expected startup checkpoints for the tested package are 19 loaded plugins, 188 compiled tables, and a passing native ABI/timer probe. Known baseline assertions and gameplay limitations are described in the main README.
