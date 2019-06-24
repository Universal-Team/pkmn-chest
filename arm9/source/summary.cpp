#include "summary.h"
#include "graphics/colors.h"
#include "keyboard.h"
#include "manager.h"

char dexNo[6];
char expPts[6];
char item[6];
char tid[6];

XYCoords getPokeballPosition(u8 ball) {
	if(ball > 25)	return {0, 0};
	XYCoords xy;
	xy.y = (ball/9)*15;
	xy.x = (ball-((ball/9)*9))*15;
	return xy;
}

void drawPokemonStats(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
	for(int i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}

	// Draw outline
	drawOutline(146, -1, 110, 85, LIGHT_GRAY, false);

	// Print Pokémon name and level
	printTextTinted(pkm->nickname(), (pkm->gender() ? (pkm->gender() == 1 ? 0x801F : WHITE) : 0xFC00), 165, 1, false);
	char level[8];
	snprintf(level, sizeof(level), "Lv. %i", pkm->level());
	printText(level, 152, 16, false);

	// Draw Pokémon and Pokéball
	XYCoords xy = getPokeballPosition(pkm->ball());
	drawImageFromSheet(148, 1, 15, 15, ballSheet, ballSheetData.width, xy.x, xy.y, false);
	xy = getPokemonPosition(pkm->species());
	drawImageFromSheetScaled(169, 16, 32, 32, 2, pokemonSheet, pokemonSheetData.width, xy.x, xy.y, false);

	// Print Pokémon and trainer info labels
	printText("Dex #", 20, 8, false);
	printText("Name", 20, 24, false);
	printText("Type", 20, 40, false);
	printText("OT", 20, 56, false);
	printText("ID #", 20, 72, false);
	printText("Exp.", 20, 88, false);
	printText("Item", 20, 104, false);

	// Print Pokémon and trainer info
	snprintf(dexNo, sizeof(dexNo), "%.3i", pkm->species());
	printText(dexNo, 64, 8, false);
	printText(pkm->nickname(), 64, 24, false);
	printTextTinted(pkm->otName(), (pkm->otGender() ? 0x801F : 0xFC00), 64, 56, false);
	snprintf(tid, sizeof(tid), "%.5li", pkm->formatTID());
	printText(tid, 64, 72, false);
	snprintf(expPts, sizeof(expPts), "%li", pkm->experience());
	printText(expPts, 64, 88, false);
	snprintf(item, sizeof(item), "%i", pkm->heldItem());
	printText(item, 64, 104, false);

	// Draw types
	drawImageFromSheet(64, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), false);
	if(pkm->type1() != pkm->type2())
		drawImageFromSheet(99, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), false);
}

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm) {
	// Draw the Pokémon's info
	drawPokemonStats(pkm);

	// Move arrow to first option
	setSpriteVisibility(topArrowID, false);
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, 64+getTextWidth(dexNo), 2);
	updateOam();

	int held, pressed, selection = 0;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(pressed & KEY_UP) {
			if(selection > 0)	selection--;
		} else if(pressed & KEY_DOWN) {
			if(selection < 5)	selection++;
		} else if(pressed & KEY_A) {
			switch(selection) {
				case 0: {
					int num = Input::getInt(649);
					if(num != -1)	pkm->species(num);
					break;
				} case 1: {
					std::string name = Input::getLine(10);
					if(name != "")	pkm->nickname(name);
					break;
				} case 2: {
					std::string name = Input::getLine(7);
					if(name != "")	pkm->otName(name);
					break;
				} case 3:
					int num = Input::getInt(65535);
					if(num != -1)	pkm->TID(num);
					break;
			}
			drawPokemonStats(pkm);
		} else if(pressed & KEY_B) {
			return pkm;
		}

		switch(selection) {
			case 0:
				setSpritePosition(bottomArrowID, 64+getTextWidth(dexNo), 2);
				break;
			case 1:
				setSpritePosition(bottomArrowID, 64+getTextWidth(pkm->nickname()), 18);
				break;
			case 2:
				setSpritePosition(bottomArrowID, 64+getTextWidth(pkm->otName()), 50);
				break;
			case 3:
				setSpritePosition(bottomArrowID, 64+getTextWidth(tid), 66);
				break;
			case 4:
				setSpritePosition(bottomArrowID, 64+getTextWidth(expPts), 82);
				break;
			case 5:
				setSpritePosition(bottomArrowID, 64+getTextWidth(item), 98);
				break;
		}
		updateOam();
	}
}