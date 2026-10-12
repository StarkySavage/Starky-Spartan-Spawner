# Component behavior for reviewers

## Native bridge

`src/halosandbox_native_v2.c`, `src/phase1_profile.h`, and `src/halosandbox_hook.S` build `halosandbox_native.dll`. It is loaded into the game by UE4SS/Lua. It validates the exact supported simulation DLL file hash and code signatures, installs an in-process dispatch hook and calls game functions on the simulation thread. This intentionally involves native pointers, executable memory protection changes and an assembly register-saving stub.

It implements actor, weapon and vehicle spawning; AI controls; equipment/team/squad management; companion maintenance; and the AI-ignore observer flag. Lua issues commands and reads results through files beside the native DLL. Logs such as `ai_trace.txt` and `native_result.txt` are local files, not remote telemetry.

The original derivative retains some older implementation code. Retention does not mean all original actions remain enabled: the command gate rejects excluded operations, and Pelican/automatic preload menu paths are disabled. Review the gate and actual exported entry points rather than inferring active features from function names alone.

A diagnostic vectored exception handler remains in the shipped AI28 code. It recognizes one known loader-allocation fault and writes bounded game-context diagnostic data to `AI24-loader-crash.bin` in the mod's native folder. It does not suppress the exception or upload the output. Crash data/log files are excluded from this repository. Reviewers should treat user-generated crash captures as potentially sensitive and request them separately only when necessary.

## Controller helper

`src/halosandbox_controller.c` builds `halosandbox_controller.dll`. It loads a Windows XInput library, polls controller state and uses `SendInput` for menu keys when the game has focus. It reads `menu_state.txt` and creates a controller worker thread. This input synthesis is deliberate menu functionality. It does not install a driver or service.

## Lua

`mod/HaloSandbox/Scripts/main.lua` and `catalog.lua` are the exact AI28 shipped Lua files. They provide the menu, asset catalog, spawn queues, escorts, mission hooks and native bridge calls. The current menu label is `JOIN LOCAL SQUAD ORDERS`; internal function/variable names can still refer to the earlier search experiment.

## Capacity script

`Apply-Capacity-Fix.ps1` modifies the user's on-disk `HaloSimulation_tag_release.dll` only on an exact hash match. It changes five fields: tag capacity from 800 to 1280 MiB, total reservation and matching pointer bounds. It creates a verified backup and supports reversal. No game DLL is in this repository. No automatic script execution occurs during compilation.

## Scenario tools

`scripts/build_scenarios.py` and `scenario_reader.py` read game-owned serialized mission data, add reserved empty squads and repack using external retoc. They do not change mission diplomacy, armor or existing squad records. The native code separately supplies a missing player-human alliance on the identified Two Betrayals and Keyes scenarios, preserving existing alliance records.

## Network and system scope

The reviewed C sources and Lua scripts do not implement network downloads or telemetry. Build scripts launch the locally supplied compiler/retoc and write local outputs. Tool downloads, Git/GitHub publication and UE4SS itself are separate activities/components. This describes the code's intended and inspected behavior; it is not an antivirus clearance or a guarantee of runtime safety.

