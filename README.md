# ![Logo](https://raw.githubusercontent.com/Universal-Team/pkmn-chest/master/resources/icon.png) Pokémon Chest
<p align="center">
  <img src="https://img.shields.io/badge/License-GPLv3-informational.svg" alt="License: GPLv3">
  <a href="https://gbatemp.net/threads/release-pkmn-chest-a-pokemon-bank-for-the-nintendo-ds-i.549249/">
    <img src="https://img.shields.io/badge/GBAtemp-thread-blue" alt="GBAtemp thread">
  </a>
  <a href="https://discord.gg/KDJCfGF">
    <img src="https://img.shields.io/badge/Discord-%23pkmn--chest-7289DA" alt="Discord Server: #pkmn-chest">
  </a>
  <a href="https://github.com/Universal-Team/pkmn-chest/actions?query=workflow%3A%22Build+pkmn-chest%22">
    <img src="https://github.com/Universal-Team/pkmn-chest/workflows/Build%20pkmn-chest/badge.svg" alt="Build status on GitHub Actions">
  </a>
  <a href="https://crowdin.com/project/pkmn-chest">
    <img src="https://badges.crowdin.net/pkmn-chest/localized.svg" alt="Translation status on Crowdin">
  </a>
</p>

A Pokémon Bank for the 3<sup>rd</sup> through 5<sup>th</sup> generation Pokémon games for the Nintendo DS(i).

<p align="center">
  <img src="https://universal-team.net/images/pkmn-chest/top-menu-1.png" alt="The top menu (top screen)">
  <img src="https://universal-team.net/images/pkmn-chest/box-1.png" alt="In the boxes (top screen)">
  <img src="https://universal-team.net/images/pkmn-chest/box-1.png" alt="Editing a Pokémon (top screen)">
  <br>
  <img src="https://universal-team.net/images/pkmn-chest/top-menu-2.png" alt="The top menu (bottom screen)">
  <img src="https://universal-team.net/images/pkmn-chest/box-2.png" alt="In the boxes (bottom screen)">
    <img src="https://universal-team.net/images/pkmn-chest/summary.png" alt="Editing a Pokémon (bottom screen)">
</p>

## Table of contents
- [Building](#building)
  - [Setting up your environment](#setting-up-your-environment)
  - [Cloning the repo](#cloning-the-repo)
  - [Compiling](#compiling)
- [Contact](#contact)
- [Translating](#translating)
- [Credits](#credits)
  - [Main Developers](#main-developers)
  - [Translators](#translators)
  - [Others](#others)
  - [Music](#music)

## Building
### Setting up your environment
Compiling this app requires [devkitPro](https://devkitpro.org)'s devkitARM and libnds.
These can be installed by getting [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman), then running:
`sudo dkp-pacman -S nds-dev`<br>
<sup>(command may be slightly different depending on your OS)</sup>

You will also need the [bmp2ds](https://github.com/Universal-Team/bmp2ds/releases/latest) graphics converter in your PATH named `bmp2ds`.

### Cloning the repo
To clone the repo run `git clone --recursive https://github.com/Universal-Team/pkmn-chest.git` (Note the `--recursive`, that is needed to clone the submodules)

If you already have the repo without the submodules you can run `git submodule update --init --recursive` to get the core submodule and the memecrypto submodule inside it.

### Compiling
Simply run `make` in the pkmn-chest source directory

### Important notes (10/05/2025)
Some recent major refactorings to the devkitPro toolchain for the nds family of systems have made it impossible to compile this app with the most recent versions of devkitARM and libnds.

Fortunately, there are some archives of legacy compiling environments that can be run through Docker, an engine that isolates library-level dependencies. **This is currently the most straightforward way for compiling the app until it is ported to the newer version of these libraries.**

The latest environment that can compile the app is the one from the 04 of November of 2025, [and can be accessed through Dockerhub](https://hub.docker.com/layers/devkitpro/devkitarm/20241104/images/sha256-847dcaaa096a8c0b5a24f23ab81dedd94d4ad07c714cc5b0f09fe78d2c42ceef).

On the Docker CLI, the following command template may be used to obtain and run the compiling environment:
```bash
sudo docker run -it\
  --name pkmnchest\
  --rm\
  -v /path/to/repo:/pkmn-chest:rw\
  -v /path/to/bmp2ds-linux:/usr/bin/bmp2ds:ro\
  devkitpro/devkitarm:20241104\
  bash
```

Alternatively to mounting the bmp2ds binary directly (in which case it would need to be the linux binary), you can also install the program inside the environment through the terminal.

There might be some other issues when compiling depending on the C/C++ version you have installed, but changing the source code according to the compiler's suggestions (e.g.: changing every instance of 'std::find_if' to 'std::ranges::find_if') seems to work for now.

## Contact
The best way to talk to us is to join [our Discord](https://discord.gg/KDJCfGF). If you want to report a bug or request a feature you can do so [in a GitHub Issue](https://github.com/Universal-Team/pkmn-chest/issues/new/choose). You can also talk on the [GBAtemp thread](https://gbatemp.net/threads/release-pkmn-chest-a-pokemon-bank-for-the-nintendo-ds-i.549249/).

## Translating
[Crowdin](https://crowdin.com) is the preferred place for translating pkmn-chest as it manages keeping the languages synchronized and provides useful features when translating like screenshots for context. You can join the pkmn-chest project with [this invite link](https://crwd.in/pkmn-chest). If you don't want to use Crowdin you can send an updated `app.json` file on our Discord or the GBAtemp thread (linked above) or in an Issue here, direct Pull Requests editing the `app.json` files are not preferred since that messes with Crowdin's syncing.

If you'd like to help translate pkmn-chest to a language that isn't yet supported, or for a regional variant to be created for an existing language, please either [make a feature request issue](https://github.com/Universal-Team/pkmn-chest/issues/new/choose) requesting the language be added, ask on our Discord or the GBAtemp thread (linked above), or send Pk11 a message on Crowdin.

## Credits
### Main Developers
- [Pk11](https://github.com/Epicpkmn11): Main developer
### Translators
- [antoine62](https://github.com/antoine62): French
- [5antos](https://github.com/5antos): Portuguese, some Spanish and French
- [cimo95](https://github.com/cimo95): Indonesian
- [David Pires](https://github.com/DavidPires): Portuguese
- [edo9300](https://github.com/edo9300): Italian
- [Pk11](https://github.com/Epicpkmn11): English and Japanese
- [Extocine](https://twitter.com/@ExtocineN): Russian
- [lemonnade0](https://steamcommunity.com/profiles/76561198276444028): Lithuanian
- [NightScript](https://github.com/NightYoshi370/): French
- [R-YaTian](https://github.com/R-YaTian): Chinese
- [StackZ](https://github.com/SuperSaiyajinStackZ): German
- [Viktor Varga](https://github.com/vargaviktor): Hungarian
- [whatter](https://github.com/whatter): Polish
- [XxPhoenix1996xX](https://github.com/XxPhoenix1996xX): Spanish
### Others
- [devkitPro](https://github.com/devkitPro), [WinterMute](https://github.com/WinterMute): devkitARM, libnds, and libfat.
- [edo9300](https://github.com/edo9300): [ndsi-savedumper](https://github.com/edo9300/ndsi-savedumper)'s gamecard save dumping and injecting code
- [Flagbrew](https://github.com/FlagBrew), [piepie62](https://github.com/piepie62): [PKSM](https://github.com/FlagBrew/PKSM), [PKSM-Core](https://github.com/FlagBrew/PKSM-Core)'s save and bank management code
- [Kurt](https://github.com/kwsch): [PKHeX](https://github.com/kwsch/PKHeX) which was a major reference for the generation 3 code
- Pokedoc: [savegame-manager](https://code.google.com/archive/p/savegame-manager)'s GBA save dumping and injecting
- [RocketRobz](https://github.com/RocketRobz): Adding flashcard and SD being used together and code from [TWiLight Menu++](https://github.com/DS-Homebrew/TWiLightMenu)
- [StackZ](https://github.com/SuperSaiyajinStackZ): Idea to make a Pokémon Bank for the DS(i)
- [TotallyNotGuy](https://github.com/TotallyNotGuy): Finding & making graphics and the Pokémon Center generation 4 song
### Music
- [Pokémon Center (Gen 1)](https://modarchive.org/module.php?181718), [Elm's Lab](https://modarchive.org/module.php?181711), [Oak's Lab](https://modarchive.org/module.php?181717), [Game Corner](https://modarchive.org/module.php?181756), and [Twinleaf Town](https://modarchive.org/module.php?178770).
