# Build instructions

Verified on Windows x64 on 2026-10-11. These commands only write to this repository's ignored `build/` directory. They do not install the mod or run the game.

## Requirements

- Python 3.12 (standard library only for the build scripts).
- **Zig 0.16.0**, Windows x86_64, from the [official Zig downloads](https://ziglang.org/download/#release-0.16.0). Extract the full distribution, keeping its `lib` directory beside `zig.exe`. Do not substitute a development or newer compiler for the documented build.
- For optional scenario reproduction: `retoc.exe` as supplied with **Blamforge 0.7.0**, and your own supported Steam CU4 game installation. [Blamforge source](https://github.com/ionicether/blamforge), [retoc source](https://github.com/trumank/retoc). The tested retoc binary SHA256 is recorded in `review/verification.json`.

Open PowerShell in the repository root. Replace the example tool and game paths below with yours.

## Build both DLLs

```powershell
python scripts/build.py --zig "C:\Tools\zig-0.16.0\zig.exe"
```

The script checks the compiler version and runs these commands (using the supplied Zig executable):

```powershell
zig cc -target x86_64-windows-gnu -c src/halosandbox_hook.S -o build/halosandbox_hook.obj
zig cc -target x86_64-windows-gnu -fms-extensions -O2 -Werror -Wno-deprecated-declarations -shared -o build/halosandbox_native.dll src/halosandbox_native_v2.c build/halosandbox_hook.obj -lkernel32 -luser32 -lxinput1_4 -lbcrypt
zig cc -target x86_64-windows-gnu -O2 -Werror -shared -o build/halosandbox_controller.dll src/halosandbox_controller.c -lkernel32 -luser32
```

The assembly object is built from source; no precompiled object is needed. `phase1_profile.h` is included by the native source. Zig supplies the GNU target headers/runtime/import libraries; the verified build does not need Visual Studio or a separately installed Windows SDK.

Outputs:

- `build/halosandbox_native.dll`
- `build/halosandbox_controller.dll`
- `build/build-hashes.json`
- `build/package/Meteorite/Binaries/Win64/ue4ss/Mods/HaloSandbox/` with DLLs, Lua source and `enabled.txt`.

Lua scripts are interpreted by UE4SS; there is no Lua compilation step. UE4SS is not required merely to compile the DLLs. It is required to use the mod in-game.

**Binary comparison:** the AI28 native DLL rebuilt with this recipe has matching `.text`, `.rdata`, `.data`, `.pdata`, `.tls` and `.reloc` sections compared with the uploaded DLL. Build IDs/debug paths and whole-file hashes can differ. The controller in AI28 is the unchanged upstream MSVC-target binary; this documented GNU-target rebuild uses the supplied source but is **not byte-identical**. See [provenance](PROVENANCE.md). Rebuilt binaries have not replaced the author's installed mod and have not been separately gameplay-tested.

## Reproduce the scenario packages

```powershell
python scripts/build_scenarios.py --retoc "C:\Tools\Blamforge-0.7.0\retoc.exe" --game-folder "E:\SteamLibrary\steamapps\common\Halo Campaign Evolved"
```

The script reads only stock `pakchunk510-Windows.utoc` (E10) and `pakchunk440-Windows.utoc` (Maw/D40), checks original scenario hashes, appends empty squad definitions, adjusts serialized lengths and export bulk sizes, packs new containers, and reads them back to verify the output. It also verifies exact reversal to the original scenario bytes.

Outputs are under `build/scenarios/package/Meteorite/Content/Paks/~mods/`. Original game files are never modified. The empty companion `.pak` is reconstructed from the textual hex fixture; it contains only empty mount/index/footer data. The compiled scenario data is excluded from Git because it incorporates game-owned content.

E10 has 191 original squads plus 12 enemy and 4 independent-human squads (207 total). Maw has 194 original squads plus 20 reserved squads (214 total). The serialized bulk/export chunks were compared against the AI28 public packages; see the verification report. Container identifiers or layout may differ when retoc repacks them.

To assemble a local mod-only distribution, merge the two generated `Meteorite` trees in an empty staging folder and include `Apply-Capacity-Fix.ps1` and the installation instructions. Do not bundle UE4SS or a patched game DLL. These instructions do not upload anything to Nexus.

## Capacity patch

`Apply-Capacity-Fix.ps1` is readable PowerShell source, not a compiled executable. It is not run by either build script. Review its exact five edits and hashes before using it. The forward, already-patched, reverse, and unsupported-file paths were tested on disposable DLL copies during AI28 packaging. Installation and restoration commands are in `INSTALL.txt`.

