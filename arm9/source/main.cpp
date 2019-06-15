#include <fat.h>

#include "fileBrowse.h"
#include "graphics/graphics.h"
#include "loader.h"
#include "manager.h"
#include "nitrofs.h"

int main(int argc, char **argv) {
	initGraphics();
	keysSetRepeat(25,5);

	if(!fatInitDefault()) {
		// Draws the bottom screen red if fatInitDefault() fails
		drawRectangle(0, 0, 256, 192, BGR15(0, 0, 0xff), false);
		while(1) swiWaitForVBlank();
	}

	if(!nitroFSInit(argv[0])) {
		// Draws the bottom screen blue if nitroFSInit() fails
		drawRectangle(0, 0, 256, 192, BGR15(0xff, 0, 0), false);
		while(1) swiWaitForVBlank();
	}
	loadFont();

	std::vector<std::string> extensionList;
	extensionList.push_back(".sav");
	if(load(browseForFile(extensionList))) {
		drawRectangle(0, 0, 256, 192, BGR15(0, 255, 0), true);
	} else {
		drawRectangle(0, 0, 256, 192, BGR15(0, 0, 255), true);
	}
	currentBox = save->currentBox();

	drawBoxScreen();

	int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonX = 0, heldPokemonY = 0;
	u16 hDown = 0;
	while(1) {
		do {
		swiWaitForVBlank();
		scanKeys();
		hDown = keysDownRepeat();
		} while(!hDown);

		if(hDown & KEY_UP) {
			if(arrowY > 0)	arrowY--;
		} else if(hDown & KEY_DOWN) {
			if(arrowY < 4)	arrowY++;
		}
		if(hDown & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
		} else if(hDown & KEY_RIGHT) {
			if(arrowX < 5)	arrowX++;
		}
		if(hDown & KEY_L) {
			if(currentBox > 0)	currentBox--;
			else currentBox = save->maxBoxes()-1;
			drawBox();
		} else if(hDown & KEY_R) {
			if(currentBox < save->maxBoxes()-1)	currentBox++;
			else currentBox = 0;
			drawBox();
		}
		if(hDown & KEY_A) {
			if(heldPokemon != -1) {
				setSpritePosition(heldPokemon, heldPokemonX, heldPokemonY);
				setSpritePriority(heldPokemon, 2);
			}

			if(heldPokemon == (arrowY*6)+arrowX) {
				setSpritePriority(heldPokemon, 2);
				heldPokemon = -1;
			} else {
				heldPokemon = (arrowY*6)+arrowX;
				heldPokemonX = getSpriteInfo(heldPokemon).x;
				heldPokemonY = getSpriteInfo(heldPokemon).y;
				setSpritePriority(heldPokemon, 1);
				drawPokemonInfo(save->pkm(currentBox, heldPokemon));
			}
		}

		if((hDown & KEY_UP || hDown & KEY_DOWN || hDown & KEY_LEFT || hDown & KEY_RIGHT || hDown & KEY_L || hDown & KEY_R) && heldPokemon == -1) {
			drawPokemonInfo(save->pkm(currentBox, (arrowY*6)+arrowX));
		}

		if(heldPokemon != -1)	setSpritePosition(heldPokemon, (arrowX*24)+16, (arrowY*24)+32);
		setSpritePosition(arrowID, (arrowX*24)+24, (arrowY*24)+36);
		updateOam();
	}

	while(1) {

	}

	return 0;
}