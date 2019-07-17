#ifndef MANAGER_H
#define MANAGER_H

#include <nds.h>

#include "graphics.h"
#include "Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern bool topScreen;
extern int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID, 
arrowMode, pokemonSheetScale, pokemonSheetSize;
extern std::string savePath;
extern std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, menuButton, shiny, pokemonSheet, stripes, types;
extern ImageData ballSheetData, bankBoxData, menuButtonData, pokemonSheetData, shinyData, stripesData, typesData;

// Returns the current box for the cursor's current screen
int currentBox(void);

/* 
 * Returns the Pokémon in the provided slot
 * on the cursor's current screen
 */
std::shared_ptr<PKX> currentPokemon(int slot);

/*
 * Gets the position of a Pokémon in the spritesheet
 * int dexNumber is the National dex number of the Pokémon
 * Returns an XYCoords of the spritesheet position
 */
XYCoords getPokemonPosition(std::shared_ptr<PKX> pkm);

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
