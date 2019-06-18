#ifndef MANAGER_H
#define MANAGER_H

#include "graphics/graphics.h"
#include <nds.h>
#include "sav/Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID;
extern std::string savePath;
extern std::vector<u16> arrow, bankBox, keyboard, shiny, spriteSheet, stripes, types;
extern ImageData bankBoxData, keyboardData, spriteSheetData, stripesData, typesData;

// Returns the current box for the cursor's current screen
int currentBox(void);

/* 
 * Returns the Pokémon in the provided slot
 * on the cursor's current screen
 */
std::shared_ptr<PKX> currentPokemon(int slot);

// Loads the images into RAM and creates the sprites
void loadGraphics(void);

// Draws both boxes and the backgrounds
void drawBoxScreen(void);

// Draws a box
void drawBox(bool top);

// Draws the info for the Pokémon provided
void drawPokemonInfo(std::shared_ptr<PKX> pkm);

// Fills the held Pokémon sprite
void setHeldPokemon(int dexNum);

// Main Pokémon box mangaement loop
void manageBoxes(void);

#endif
