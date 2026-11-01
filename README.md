# Mafia II Co-op (fork of m2o-reborn)

This fork is the starting point for a **2-player co-op campaign mod for Mafia II Classic (Steam, 32-bit)**.
It builds on the archived Mafia 2 Online "reborn" code (Apache-2.0) and the bundled m2sdk.

## Building (Windows)

1. Install Visual Studio 2022 with the **Desktop development with C++** workload, and CMake 3.15+.
2. Run `generate-vs2022.bat` from the repo root. It creates a **32-bit (Win32)** solution in `build/`.
3. Open `build\m2o-reborn.sln`, choose **Release**, and build.
4. Outputs land in `binary/client/` (`m2o-client.dll`, `m2o-launcher.exe`) and `binary/server/`.

Both players must run the identical Steam Classic `mafia2.exe`; all addresses in m2sdk target that build.

## First test run (single PC)

The client boots Mafia II into a free-ride world (it overrides a few game files in memory via `binary/client/files`; your install is not modified).

1. Start `binary\server\m2o-server.exe` (allow it through Windows Firewall when asked).
2. Make sure Steam is running, then start `binary\client\m2o-launcher.exe`. It finds Mafia II through Steam, launches `pc\mafia2.exe` and injects `m2o-client.dll`.
3. In game, press **F5** to connect. F1 toggles the mouse.
4. Logs: `binary\client\debug.log`, crash dumps in `binary\client\exceptions\`.

### Playing across two PCs

Edit `binary\client\server.txt` on the guest PC to the host's address, e.g. `192.168.1.20:27010`. The host forwards/opens **UDP 27010** if connecting over the internet.

## Plan

1. Two players in free roam (on-foot + vehicle sync) - inherited from m2o-reborn.
2. Host-owned NPCs and damage forwarded from the guest.
3. Guest loads the same chapter with mission scripts suppressed; synced cutscene start.
4. One mission fully co-op, then generalise across chapters.

---

<div align="center">
    <a href="https://github.com/mafia2online/m2o-reborn"><img src="https://user-images.githubusercontent.com/2182108/46598345-7a6f3080-caec-11e8-9204-64ab4a8ba591.png" alt="Mafia 2 Online" /></a>
</div>

<div align="center">
    <a href="https://travis-ci.org/mafia2online/m2o-reborn"><img src="https://travis-ci.org/mafia2online/m2o-reborn.svg?branch=master" alt="Build status" /></a>
    <a href="https://ci.appveyor.com/project/inlife/m2o-reborn"><img src="https://ci.appveyor.com/api/projects/status/tquw30xp8rjla33q?svg=true" alt="Build status" /></a>
    <a href="https://discord.gg/xyMepEk"><img src="https://discordapp.com/api/guilds/129213934887501824/embed.png" alt="Discord server" /></a>
    <a href="LICENSE"><img src="https://img.shields.io/github/license/mafia2online/m2o-reborn.svg" alt="license" /></a>
</div>

<br />
<div align="center">
  A multiplayer modification for a steam version of Mafia II.
</div>

<div align="center">
  <sub>
    Built with love using <a href="https://github.com/librg/librg"><strong>librg</strong></a>
    &bull; Brought to you by <a href="https://github.com/inlife">@inlife</a>,
    <a href="https://github.com/tyldar">@tyldar</a>
    and <a href="#contributors">contributors</a>
  </sub>
</div>

## Description

This is a core repository of "Mafia 2 Online" multiplayer modification for Mafia II.

Mafia 2 Online is an open-source project, which is under active development. It is not yet ready to be used by players.

We invite each and everyone to contribute, to add issues regarding bags/ideas/propositions and to share this repo with everyone who could potentially be interested in such development process.

To run the binaries you are **required** to have a steam version of the game installed in your steam library.

## Features

* cross-platform/env project generation
* code style somewhat influenced by [Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b)
* heavy use of single-header styled libraries
* unity builds based projects
* fast compile time orientation
* auto-building and auto-delivery (not fully implemented atm)

## Prerequisites

* visual studio 2017 (project itself isn't version dependant, however multiple static libs inside vendors, are built targeting vs2017, something that hepefully we gonna fix in the future)
* bought and installed steam version of the Mafia II
* running steam client

## Structure

* `binary/` This is a folder which you should use to run client or server.
Contains files and resources and scripts needed fro running them both.
Compiled binaries are automatically copied there after build.
* `build/` Is a git-ignored folder where cmake will generate your project files to.
* `misc/` A set of small things/tools that are used in the repo.
* `projects/` Place where you put your code.
* `vendor/` A folder containing dependencies

## Building

0. Install cmake

```
windows: https://cmake.org/files/v3.9/cmake-3.9.0-win64-x64.msi (**Make sure to add to path**)
linux: using your package manager
macos: brew install cmake
```

1. Clone

```sh
$ git clone https://github.com/mafia2online/m2o-reborn.git
```
2. Generate project

```
$ generate.sh.bat
```

3. Open the solution (Windows)

```
build/m2o-reborn.sln
```

3. Build the stuff (\*nix)

```
$ cd build && make -j4
```

## Contributors

The list of contributors goes there (do not hesitate adding yourself):

* [MyU](https://github.com/myudev)
* [ZaKlaus](https://github.com/zaklaus)

## License

Project is licensed under Apache 2.0 license.
You can check it out [there](LICENSE).
