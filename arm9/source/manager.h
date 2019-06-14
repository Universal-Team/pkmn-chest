#ifndef MANAGER_H
#define MANAGER_H

#include "sav/Sav.hpp"

struct XYCoords {
	int x;
	int y;
};

extern int arrowID;
extern int shinyID;

void drawBoxScreen(void);

void drawPokemonInfo(std::shared_ptr<PKX> pkm);

#endif
