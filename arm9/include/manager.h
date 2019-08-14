#ifndef MANAGER_H
#define MANAGER_H

#include <nds.h>

#include "graphics.h"
#include "Sav.hpp"

extern bool topScreen;
extern int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID, arrowMode;
extern std::vector<int> menuIconID, partyIconID;
extern std::string savePath;
extern std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgBottom, boxBgTop, boxButton, fileBrowseBg, infoBox, menuBg, menuButton, menuButtonBlue, menuIconSheet, optionsBg, pokemonSheet, search, shiny, summaryBg, types;
extern ImageData ballSheetData, bankBoxData, boxBgBottomData, boxBgTopData, boxButtonData, fileBrowseBgData, infoBoxData, menuBgData, menuButtonData, menuButtonBlueData, menuIconSheetData, optionsBgData, pokemonSheetData, searchData, shinyData, summaryBgData, typesData;

// Returns the current box for the cursor's current screen
int currentBox(void);

/*
 * Returns the Pokémon in the provided slot
 * on the cursor's current screen
 */
std::shared_ptr<PKX> currentPokemon(int slot);

/*
 * Gets the position of a Pokémon in the spritesheet
 * std::shared_ptr<PKX> pkm is the Pokémon to get the position of
 * Returns a std::pair<int, int> of the spritesheet position
 */
std::pair<int, int> getPokemonPosition(std::shared_ptr<PKX> pkm);

/*
 * Gets the position of a Pokémon in the spritesheet
 * int species is the National dex number of the Pokémon
 * int alternativeForm is the alternative form of the Pokémon
 * int gender is the gender of the Pokémon
 * bool egg is wheher the Pokémon is in an egg
 * Returns a std::pair<int, int> of the spritesheet position
 */
std::pair<int, int> getPokemonPosition(int species, int alternativeForm = 0, int gender = 0, bool egg = false);

/*
 * Gets the bmp number for a Pokémon
 * std::shared_ptr<PKX> pkm is the Pokémon to get the bmp for
 * Returns an int of the bmp to use
 */
int getPokemonIndex(std::shared_ptr<PKX> pkm);

/*
 * Gets the bmp number for a Pokémon
 * int species is the National dex number of the Pokémon
 * int alternativeForm is the alternative form of the Pokémon
 * int gender is the gender of the Pokémon
 * bool egg is wheher the Pokémon is in an egg
 * Returns an int of the bmp to use
 */
int getPokemonIndex(int species, int alternativeForm = 0, int gender = 0, bool egg = false);

// Loads the images into RAM and creates the sprites
void loadGraphics(void);

// Draws both boxes and the backgrounds
void drawBoxScreen(void);

// Draws a box
void drawBox(bool top, bool reloadPokemon = false);

// Draws the info for the Pokémon provided
void drawPokemonInfo(std::shared_ptr<PKX> pkm);

// Fills the held Pokémon sprite
void setHeldPokemon(std::shared_ptr<PKX> pkm);

// Main Pokémon box mangaement loop
void manageBoxes(void);

#endif
