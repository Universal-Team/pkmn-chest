#include "manager.h"
#include "graphics/graphics.h"
#include "loader.h"

std::vector<u16> arrow, bankBox, shiny, spriteSheet, stripes, types;
ImageData bankBoxData, spriteSheetData, stripesData, typesData;
int arrowID;
int shinyID;

XYCoords getPokemonPosition(int dexNumber) {
	XYCoords xy;
	xy.y = (dexNumber/16)*32;
	xy.x = (dexNumber-((dexNumber/16)*16))*32;
	return xy;
}

void drawBoxScreen(void) {
	bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	spriteSheetData = loadPng("nitro:/graphics/spriteSheet.png", spriteSheet);
	stripesData = loadPng("nitro:/graphics/stripes.png", stripes);
	typesData = loadPng("nitro:/graphics/types.png", types);
	loadPng("nitro:/graphics/arrow.png", arrow);
	loadPng("nitro:/graphics/shiny.png", shiny);

	// Draws the BGs
	drawRectangle(0, 0, 256, 192, BGR15(0x66, 0, 0), true);
	drawRectangle(0, 0, 256, 192, BGR15(0x66, 0, 0), false);
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, false);
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, true);

	// The Button.
	drawRectangle(170, 120, 68, 30, BGR15(0x63, 0x65, 0x73), false);

	// Print Pokemon info and box names
	drawPokemonInfo(save->pkm(0));
	printTextTinted(save->boxName(save->currentBox()), DARK_GRAY, 60, 20, true);
	printTextTinted(save->boxName(save->currentBox()), DARK_GRAY, 60, 20, false);

	// Pokémon Sprites
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, false);
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, true);
	arrowID = initSprite(SpriteSize_16x16, false);
	shinyID = initSprite(SpriteSize_16x16, true); // 8x8 wasn't working

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

	// Arrow & Shiny icon
	fillSpriteImage(arrowID, arrow);
	prepareSprite(arrowID, 24, 36, 0);
	fillSpriteImage(shinyID, shiny);
	prepareSprite(shinyID, 239, 52, 0);
	setSpriteVisibility(shinyID, save->pkm(0)->shiny());

	updateOam();
}

void drawPokemonInfo(std::shared_ptr<PKX> pkm) {
	// Show shiny star if applicable
	setSpriteVisibility(shinyID, pkm->shiny());

	// Clear previous draw
	drawRectangle(170, 0, 86, 192, BGR15(0x66, 0, 0), true);

	// Draw dashed lines
	drawImage(170, 30, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 60, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 90, stripesData.width, stripesData.height, stripes, true);

	if(pkm->species() > 0 && pkm->species() < 650) {
		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "No.%.3i", pkm->species());
		printTextTinted(str, 0xCE73, 170, 2, true);

		// Print nickname
		printText(pkm->nickname(), 170, 14, true);

		// Draw types
		drawImageFromSheet(170, 33, 32, 12, types, 32, 0, (pkm->type1()*12), true);
		if(pkm->type1() != pkm->type2())
			drawImageFromSheet(205, 33, 32, 12, types, 32, 0, (pkm->type2()*12), true);

		// Print Level
		snprintf(str, sizeof(str), "Lv.%i", pkm->level());
		printText(str, 170, 46, true);
	}
}
