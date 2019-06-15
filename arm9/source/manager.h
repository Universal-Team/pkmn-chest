#ifndef MANAGER_H
#define MANAGER_H

#include "sav/Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern int bottomArrowID;
extern int topArrowID;
extern int shinyID;
extern int currentBox;

void drawBoxScreen(void);

void drawBox(void);

void drawPokemonInfo(std::shared_ptr<PKX> pkm);

#endif
