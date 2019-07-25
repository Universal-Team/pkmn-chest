#ifndef MANAGER_H
#define MANAGER_H

#include <nds.h>

#include "graphics.h"
#include "Sav.hpp"

extern bool topScreen;
extern int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID, 
arrowMode, pokemonSheetScale, pokemonSheetSize;
extern std::vector<int> menuIconID;
extern std::string savePath;
extern std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgBottom, boxBgTop, boxButton, fileBrowseBg, infoBox, menuBg, menuButton, menuButtonBlue, menuIconSheet, optionsBg, pokemonSheet, shiny, summaryBg, types;
extern ImageData ballSheetData, bankBoxData, boxBgBottomData, boxBgTopData, boxButtonData, fileBrowseBgData, infoBoxData, menuBgData, menuButtonData, menuButtonBlueData, menuIconSheetData, optionsBgData, pokemonSheetData, shinyData, summaryBgData, typesData;

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
 * Returns an std::pair<int, int> of the spritesheet position
 */
std::pair<int, int> getPokemonPosition(std::shared_ptr<PKX> pkm);

/*
 * Gets the position of a Pokémon in the spritesheet
 * int species is the National dex number of the Pokémon
 * int alternativeForm is the alternative form of the Pokémon
 * int gender is the gender of the Pokémon
 * Returns an std::pair<int, int> of the spritesheet position
 */
std::pair<int, int> getPokemonPosition(int species, int alternativeForm = 0, int gender = 0);

// Loads the images into RAM and creates the sprites
void loadGraphics(void);

// Draws both boxes and the backgrounds
void drawBoxScreen(void);

// Draws a box
void drawBox(bool top);

// Draws the info for the Pokémon provided
void drawPokemonInfo(std::shared_ptr<PKX> pkm);

// Fills the held Pokémon sprite
void setHeldPokemon(std::shared_ptr<PKX> pkm);

// Main Pokémon box mangaement loop
void manageBoxes(void);

#endif
