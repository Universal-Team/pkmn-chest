#include "manager.h"
#include "banks.hpp"
#include "common/banks.hpp"
#include "graphics/graphics.h"
#include "loader.h"

std::vector<u16> arrow, bankBox, shiny, spriteSheet, stripes, types;
ImageData bankBoxData, spriteSheetData, stripesData, typesData;
int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID;
bool topScreen;

int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int slot) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), slot);
	else	return save->pkm(currentBox(), slot);
}

XYCoords getPokemonPosition(int dexNumber) {
	XYCoords xy;
	xy.y = (dexNumber/16)*32;
	xy.x = (dexNumber-((dexNumber/16)*16))*32;
	return xy;
}

void loadGraphics(void) {
	// Load images into RAM
	bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	spriteSheetData = loadPng("nitro:/graphics/spriteSheet.png", spriteSheet);
	stripesData = loadPng("nitro:/graphics/stripes.png", stripes);
	typesData = loadPng("nitro:/graphics/types.png", types);
	loadPng("nitro:/graphics/arrow.png", arrow);
	loadPng("nitro:/graphics/shiny.png", shiny);

	// Init Pokémon Sprites
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, false);
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, true);
	// Prepare their locations
	for(int y=0;y<5;y++) {
		for(int x=0;x<6;x++) {
			prepareSprite((y*6)+x, 8+(x*24), 32+(y*24), 2);
			prepareSprite(((y*6)+x)+30, 8+(x*24), 32+(y*24), 2);
		}
	}

	// Prepare bottom arrow sprite
	bottomArrowID = initSprite(SpriteSize_16x16, false);
	fillSpriteImage(bottomArrowID, arrow);
	prepareSprite(bottomArrowID, 24, 36, 0);
	setSpriteVisibility(bottomArrowID, false);

	// Prepare top arrow sprite
	topArrowID = initSprite(SpriteSize_16x16, true);
	fillSpriteImage(topArrowID, arrow);
	prepareSprite(topArrowID, 24, 36, 0);
	setSpriteVisibility(topArrowID, false);

	// Prepare bottom sprite for moving pokemon
	bottomHeldPokemonID = initSprite(SpriteSize_32x32, false);
	prepareSprite(bottomHeldPokemonID, 0, 0, 1);
	setSpriteVisibility(bottomHeldPokemonID, false);

	// Prepare top sprite for moving pokemon
	topHeldPokemonID = initSprite(SpriteSize_32x32, true);
	prepareSprite(topHeldPokemonID, 0, 0, 1);
	setSpriteVisibility(topHeldPokemonID, false);

	// Prepare shiny sprite
	shinyID = initSprite(SpriteSize_16x16, true); // 8x8 wasn't working
	fillSpriteImage(shinyID, shiny);
	prepareSprite(shinyID, 239, 52, 0);
	setSpriteVisibility(shinyID, false);
}

void drawBoxScreen(void) {
	// Draws backgrounds
	drawRectangle(0, 0, 256, 192, DARK_BLUE, true);
	drawRectangle(0, 0, 256, 192, DARK_BLUE, false);

	// Show bottom arrow
	setSpriteVisibility(bottomArrowID, true);

	// Draw the boxes and Pokémon
	drawBox(true);
	drawBox(false);

	// Draw first Pokémon's info
	drawPokemonInfo(save->pkm(currentBox(), 0));
}

void drawBox(bool top) {
	// Draw box images
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, top);

	if(top) {
		// Print box names
		printTextCenteredTinted(Banks::bank->boxName(currentBankBox), DARK_GRAY, -44, 20, true);

		for(int i=0;i<30;i++) {
			// Show/Hide Pokémon sprites for bank box
			if(Banks::bank->pkm(currentBankBox, i)->species() == 0)
				setSpriteVisibility(i+30, false);
			else
				setSpriteVisibility(i+30, true);
		}
		updateOam();

		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(Banks::bank->pkm(currentBankBox, i)->species() != 0) {
				XYCoords xy = getPokemonPosition(Banks::bank->pkm(currentBankBox, i)->species());
				fillSpriteFromSheet(i+30, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
			}
		}
	} else {
		// Print box names
		printTextCenteredTinted(save->boxName(currentSaveBox), DARK_GRAY, -44, 20, false);

		for(int i=0;i<30;i++) {
			// Show/Hide Pokémon sprites for save box
			if(save->pkm(currentSaveBox, i)->species() == 0)
				setSpriteVisibility(i, false);
			else
				setSpriteVisibility(i, true);
		}
		updateOam();

		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(save->pkm(currentSaveBox, i)->species() != 0) {
				XYCoords xy = getPokemonPosition(save->pkm(currentSaveBox, i)->species());
				fillSpriteFromSheet(i, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
			}
		}
	}
}

void drawPokemonInfo(std::shared_ptr<PKX> pkm) {
	// Clear previous draw
	drawRectangle(170, 0, 86, 192, DARK_BLUE, true);

	// Draw dashed lines
	drawImage(170, 30, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 60, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 90, stripesData.width, stripesData.height, stripes, true);

	if(pkm->species() > 0 && pkm->species() < 650) {
		// Show shiny star if applicable
		setSpriteVisibility(shinyID, pkm->shiny());

		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "No.%.3i", pkm->species());
		printTextTinted(str, 0xCE73, 170, 2, true);

		// Print nickname
		printText(pkm->nickname(), 170, 14, true);

		// Draw types
		drawImageFromSheet(170, 33, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), true);
		if(pkm->type1() != pkm->type2())
			drawImageFromSheet(205, 33, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), true);

		// Print Level
		snprintf(str, sizeof(str), "Lv.%i", pkm->level());
		printText(str, 170, 46, true);
	} else {
		// Hide shiny star
		setSpriteVisibility(shinyID, false);
	}
}

void setHeldPokemon(int id) {
	if(id != 0) {
		XYCoords xy = getPokemonPosition(id);
		fillSpriteFromSheet(bottomHeldPokemonID, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
		fillSpriteFromSheet(topHeldPokemonID, spriteSheet, 32, 32, spriteSheetData.width, xy.x, xy.y);
	}
}
