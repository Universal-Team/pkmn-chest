#include "summary.h"
#include "graphics/colors.h"
#include "manager.h"

XYCoords getPokeballPosition(u8 ball) {
	if(ball > 25)	return {0, 0};
	XYCoords xy;
	xy.y = (ball/9)*15;
	xy.x = (ball-((ball/9)*9))*15;
	return xy;
}

void showPokemonSummary(std::shared_ptr<PKX> pkm) {
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
	char str[16];
	snprintf(str, sizeof(str), "%i", pkm->species());
	printText(str, 64, 8, false);
	printText(pkm->nickname(), 64, 24, false);
	snprintf(str, sizeof(str), "%i", pkm->species());
	printTextTinted(pkm->otName(), (pkm->otGender() ? 0x801F : 0xFC00), 64, 56, false);
	snprintf(str, sizeof(str), "%li", pkm->formatTID());
	printText(str, 64, 72, false);
	snprintf(str, sizeof(str), "%li", pkm->experience());
	printText(str, 64, 88, false);
	snprintf(str, sizeof(str), "%i", pkm->heldItem());
	printText(str, 64, 104, false);

	// Draw types
	drawImageFromSheet(64, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), false);
	if(pkm->type1() != pkm->type2())
		drawImageFromSheet(99, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), false);
}