# Starky Spartan Spawner

Source and build documentation for the **AI28 Nexus update** of Starky Spartan Spawner for Halo Campaign Evolved, Steam CU4 build `2026.08.11.1121610.2`.

This repository is provided for source review. It contains the native C/assembly code, Lua menu and runtime scripts, the reversible capacity patch, and scripts that reproduce the scenario modifications from a user's own game installation. It contains no game DLLs, game assets, UE4SS binaries, compiled mod DLLs, personal configuration, logs, saves, or credentials.

## Start here

- [Build both DLLs and scenario packages](docs/BUILD.md)
- [Component behavior and review guide](docs/REVIEW.md)
- [Release provenance and verification limits](docs/PROVENANCE.md)
- [AI28 file hashes](review/ai28-release-hashes.json)
- [Local verification results](review/verification.json)
- [Installation instructions](docs/INSTALL.txt)
- [Changelog](docs/CHANGELOG.txt)

## Features

Faction/rank/loadout AI spawning, x1/x10 quantities, weapon and vehicle spawning, AI pause/resume, Marine and Johnson escorts, catch-up teleport, and Battle Observer. Manual Marines offer Follow, Free Combat, or **Join Local Squad Orders**, male/female selection, and 15 explicit weapon choices plus Mixed Weapons. Energy swords and the unfinished DMR are excluded from manual Marine choices.

Join Local Squad Orders is not autonomous Search and Destroy. Marines can inherit mission routines, wait at doors, defend positions, or follow the player. Initial nearby-enemy awareness runs once per spawn; actual behavior depends on mission AI/navigation.

## Scope

The native code checks the supported simulation DLL hash/signatures. The capacity patch supports only the documented original and patched CU4 hashes, backs up the DLL, and can reverse the five changes. E10 and Maw scenario edits add reserved spawn squads; they can conflict with other overrides of those scenarios. This public version preserves stock bonus-mission armor and does not contain the personal Soppo/Mark IV adaptation.

Solo gameplay was tested by the mod author. Not every newly exposed Marine weapon combination or co-op behavior has been tested. The public E10 companion was structurally verified but not separately gameplay-tested. Compiling successfully is not a security certification or a substitute for Nexus review.

## Credits and license

Based on **Halo Sandbox / Halo Campagne Revolved**, copyright 2026 **Ghost7134**, under the included [MIT license](LICENSE). Original copyright notices remain in the source. Modifications were developed for Starky with OpenAI Codex assistance. The controller source/binary originated in the original Steam distribution; see provenance for build differences.

UE4SS is a separate prerequisite: <https://github.com/UE4SS-RE/RE-UE4SS>. Game content belongs to its respective owners and is not licensed by this repository's MIT notice. The scenario scripts require the owner's local game files.

