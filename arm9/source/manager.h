#ifndef MANAGER_H
#define MANAGER_H

#include "sav/Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern int bottomArrowID, topArrowID, shinyID, currentBox, bottomHeldPokemonID, topHeldPokemonID;

void loadGraphics(void);

void drawBoxScreen(void);

void drawBox(void);

void drawPokemonInfo(std::shared_ptr<PKX> pkm);

void setHeldPokemon(int id);

#endif
