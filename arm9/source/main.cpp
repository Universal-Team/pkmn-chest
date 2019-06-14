#include <fat.h>

#include "fileBrowse.h"
#include "graphics/graphics.h"
#include "loader.h"
#include "nitrofs.h"


struct XYCoords {
	int x;
	int y;
};

XYCoords getPokemonPosition(int dexNumber) {
	XYCoords xy;
	xy.y = (dexNumber/16)*32;
	xy.x = (dexNumber-((dexNumber/16)*16))*32;
	return xy;
}

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

	std::vector<u16> spriteSheet, bankBox, stripes, arrow, shiny;
	ImageData spriteSheetData = loadPng("nitro:/graphics/spriteSheet.png", spriteSheet);
	ImageData bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	ImageData stripesData = loadPng("nitro:/graphics/stripes.png", stripes);
	ImageData shinyData = loadPng("nitro:/graphics/shiny.png", shiny);
	loadPng("nitro:/graphics/arrow.png", arrow);

	// Draws the BGs.
	drawRectangle(0, 0, 256, 192, BGR15(0xff, 0, 0), true);
	drawRectangle(0, 0, 256, 192, BGR15(0xff, 0, 0), false);
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, false);
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, true);
	// Stripes for the Text later.
	drawImage(180, 30, stripesData.width, stripesData.height, stripes, true);
	drawImage(180, 60, stripesData.width, stripesData.height, stripes, true);
	drawImage(180, 90, stripesData.width, stripesData.height, stripes, true);
	// The Button.
	drawRectangle(180, 120, 68, 30, BGR15(0x63, 0x65, 0x73), false);
	
	// Shiny Icon.
	drawImage(180, 120, shinyData.width, shinyData.height, shiny, true);
	// First Row.
	// drawImage(0, 40, 32, 32, spriteSheet, true);  // That was for test purpose.

	printText(save->pkm(0)->nickname(), 180, 15, true);
	printTextTinted(save->boxName(save->currentBox()), DARK_GRAY, 60, 20, true);
	printTextTinted(save->boxName(save->currentBox()), DARK_GRAY, 60, 20, false);

	// drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, false);
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, true);


	// Pokémon Sprites
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, false);
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, true);
	int arrowID = initSprite(SpriteSize_16x16, false);

	for(int y=0;y<5;y++) {
		for(int x=0;x<6;x++) {
			prepareSprite((y*6)+x, 8+(x*24), 32+(y*24), 2);
			prepareSprite(((y*6)+x)+30, 8+(x*24), 32+(y*24), 2);
		}
	}

	for(int i=0;i<6;i++) {
		XYCoords xy = getPokemonPosition(save->pkm(i)->species());
		fillSpriteFromSheet(i, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
		fillSpriteFromSheet(i+30, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
		updateOam();
	}

	// Arrow
	fillSpriteImage(arrowID, arrow);
	prepareSprite(arrowID, 24, 36, 0);

	updateOam();

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
			}
		}
		
		if(heldPokemon != -1)	setSpritePosition(heldPokemon, (arrowX*24)+16, (arrowY*24)+32);
		setSpritePosition(arrowID, (arrowX*24)+24, (arrowY*24)+36);
		updateOam();
	}

	while(1) {

	}

	return 0;
}