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

	int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonBox = -1;
	u16 hDown = 0;
	bool topScreen = false;
	while(1) {
		do {
		swiWaitForVBlank();
		scanKeys();
		hDown = keysDownRepeat();
		} while(!hDown);

		if(hDown & KEY_UP) {
			if(arrowY > (topScreen ? -1 : -2))	arrowY--;
		} else if(hDown & KEY_DOWN) {
			if(arrowY < (topScreen ? 5 : 4))	arrowY++;
		}
		if(hDown & KEY_LEFT && arrowY != -1) {
			if(arrowX > 0)	arrowX--;
		} else if(hDown & KEY_RIGHT && arrowY != -1) {
			if(arrowX < 5)	arrowX++;
		}
		if(hDown & KEY_LEFT && arrowY == -1) {
			goto switchBoxLeft;
		} else if(hDown & KEY_RIGHT && arrowY == -1) {
			goto switchBoxRight;
		}
		if(hDown & KEY_L) {
			switchBoxLeft:
			if(currentBox > 0)	currentBox--;
			else currentBox = save->maxBoxes()-1;
			drawBox();
			if(currentBox == heldPokemonBox)
				setSpriteVisibility(heldPokemon, false);
		} else if(hDown & KEY_R) {
			switchBoxRight:
			if(currentBox < save->maxBoxes()-1)	currentBox++;
			else currentBox = 0;
			drawBox();
			if(currentBox == heldPokemonBox)
				setSpriteVisibility(heldPokemon, false);
		}
		if(hDown & KEY_A) {
			if(heldPokemon != -1) {
				if(save->pkm(currentBox, (arrowY*6)+arrowX)->species() != 0)
					setSpriteVisibility(heldPokemon, true);
				setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
			}

			if(heldPokemon == (arrowY*6)+arrowX) {
				heldPokemon = -1;
				heldPokemonBox = -1;
			} else if(save->pkm(currentBox, (arrowY*6)+arrowX)->species() != 0) {
				heldPokemon = (arrowY*6)+arrowX;
				heldPokemonBox = currentBox;
				setHeldPokemon(save->pkm(currentBox, heldPokemon)->species());
				setSpriteVisibility(heldPokemon, false);
				setSpriteVisibility((topScreen ? topHeldPokemonID : bottomHeldPokemonID), true);
				drawPokemonInfo(save->pkm(currentBox, heldPokemon));
			}
		}

		if((hDown & KEY_UP || hDown & KEY_DOWN || hDown & KEY_LEFT || hDown & KEY_RIGHT || hDown & KEY_L || hDown & KEY_R) && heldPokemon == -1) {
			drawPokemonInfo(save->pkm(currentBox, (arrowY*6)+arrowX));
		}

		if(arrowY == -2) {
			arrowY = 4;
			topScreen = true;
			setSpriteVisibility(bottomArrowID, false);
			setSpriteVisibility(topArrowID, true);
			if(heldPokemon != -1) {
				setSpriteVisibility(bottomHeldPokemonID, false);
				setSpriteVisibility(topHeldPokemonID, true);
			}
		} else if(arrowY == 5) {
			arrowY = -1;
			topScreen = false;
			setSpriteVisibility(bottomArrowID, true);
			setSpriteVisibility(topArrowID, false);
			if(heldPokemon != -1) {
				setSpriteVisibility(bottomHeldPokemonID, true);
				setSpriteVisibility(topHeldPokemonID, false);
			}

		}

		if(arrowY == -1) {
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), 90, 16);
			if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), 82, 12);
		} else {
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), (arrowX*24)+24, (arrowY*24)+36);
			if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), (arrowX*24)+16, (arrowY*24)+32);
		}
		updateOam();
	}

	while(1) {

	}

	return 0;
}