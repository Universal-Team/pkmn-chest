# ![Logo](https://raw.githubusercontent.com/Universal-Team/pkmn-chest/master/nitrofiles/graphics/icon.png) Pokémon Chest
<p align="center">
  <a href="https://dev.azure.com/Universal-Team/Builds/_build?definitionId=4">
  <img src="https://dev.azure.com/Universal-Team/Builds/_apis/build/status/Universal-Team.pkmn-chest?branchName=master">
  </a>
</p>

A Pokémon Bank for 4<sup>th</sup> and 5<sup>th</sup> generation Pokémon games for the Nintendo DS(i).

## Compiling

Compiling this app requires [devkitPRO](https://devkitpro.org)'s devkitARM and libnds.
These can be installed by getting [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman), then running:
`sudo dkp-pacman -S default-arm7 devkitARM devkitarm-rules libnds libfat-nds`<br/>
<sup>(command may be slightly different depending on your OS)</sup>

## Credits
### Main Developers
- [Epicpkmn11](https://github.com/Epicpkmn11): GUI code, porting PKSM's save and bank management code
### Others
- [devkitPro](https://github.com/devkitPro), [WinterMute](https://github.com/WinterMute): devkitARM, libnds, and libfat.
- [Flagbrew](https://github.com/FlagBrew): [PKSM](https://github.com/FlagBrew/PKSM)'s save and bank management code
- [edo9300](https://github.com/edo9300): [ndsi-savedumper](https://github.com/edo9300/ndsi-savedumper)'s gamecard save dumping and injecting code
- [RocketRobz](https://github.com/RocketRobz): Adding flashcard and SD being used together and code from [TWiLight Menu++](https://github.com/DS-Homebrew/TWiLightMenu)
- [VoltZ](https://github.com/SuperSaiyajinVoltZ): Idea to make a Pokémon Bank for the DS(i)
