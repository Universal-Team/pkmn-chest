# ![Logo](https://raw.githubusercontent.com/Universal-Team/pkmn-chest/master/resources/icon.png) Pokémon Chest
<p align="center">
  <a href="https://dev.azure.com/Universal-Team/Builds/_build?definitionId=4">
    <img src="https://dev.azure.com/Universal-Team/Builds/_apis/build/status/Universal-Team.pkmn-chest?branchName=master">
  </a>
  <a href="https://crowdin.com/project/pkmn-chest">
    <img src="https://badges.crowdin.net/pkmn-chest/localized.svg">
  </a>
</p>

A Pokémon Bank for the 3<sup>rd</sup> through 5<sup>th</sup> generation Pokémon games for the Nintendo DS(i).

## Compiling
### Setting up your environment
Compiling this app requires [devkitPro](https://devkitpro.org)'s devkitARM and libnds.
These can be installed by getting [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman), then running:
`sudo dkp-pacman -S nds-dev`<br/>
<sup>(command may be slightly different depending on your OS)</sup>

### Cloning the repo
To clone the repo run `git clone --recursive https://github.com/Universal-Team/pkmn-chest.git` (Note the `--recursive`, that is needed to clone the submodules)
If you already have the repo without the submodules you can run `git submodule update --init --recursive` to get the core submodule and the memecrypto submodule inside it.

### Compiling
Simply run `make` in the pkmn-chest source directory
- (Note: `g++` is also required currently to build the bmp2ds grapics converter)


## Contact
The best way to talk to us is to join [our Discord](https://discord.gg/KDJCfGF).
If you want to report a bug you can do so [here](https://github.com/Universal-Team/pkmn-chest/issues/new/choose).
You can also talk on the [GBAtemp thread](https://gbatemp.net/threads/release-pkmn-chest-a-pokemon-bank-for-the-nintendo-ds-i.549249/).

## Credits
### Main Developers
- [Pk11](https://github.com/Epicpkmn11): GUI code, porting PKSM's save and bank management code
### Translators
- [antoine62](https://github.com/antoine62): French
- [Chips](https://github.com/Ch1p5): Portuguese, some Spanish and French
- [cimo95](https://github.com/cimo95): Indonesian
- [David Pires](https://github.com/DavidPires): Portuguese
- [edo9300](https://github.com/edo9300): Italian
- [Pk11](https://github.com/Epicpkmn11): English and Japanese
- [Extocine](https://twitter.com/@ExtocineN): Russian
- [lemonnade0](https://steamcommunity.com/profiles/76561198276444028): Lithuanian
- [NightScript](https://github.com/NightYoshi370/): French
- [R-YaTian](https://github.com/R-YaTian): Chinese
- [StackZ](https://github.com/SuperSaiyajinStackZ): German
- [whatter](https://github.com/whatter): Polish
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
