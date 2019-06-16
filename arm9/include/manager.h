#ifndef MANAGER_H
#define MANAGER_H

#include "sav/Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID;
extern bool topScreen;

int currentBox(void);

std::shared_ptr<PKX> currentPokemon(int slot);

void loadGraphics(void);

void drawBoxScreen(void);

void drawBox(bool top);

void drawPokemonInfo(std::shared_ptr<PKX> pkm);

void setHeldPokemon(int id);

#endif
