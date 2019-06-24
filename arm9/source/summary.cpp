#include "summary.h"
#include "graphics/colors.h"
#include "keyboard.h"
#include "manager.h"

char dexNo[6];
char ball[6];
char level[6];
char nature[6];
char shinyText[6];
char pkrs[6];
char tid[6];
char sid[6];
char frnd[6];

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

	// Draw lines
	drawOutline(146, -1, 110, 85, LIGHT_GRAY, false);
	drawRectangle(0, 124, 150, 1, LIGHT_GRAY, false);

	// Print Pokémon name
	printTextTinted(pkm->nickname(), (pkm->gender() ? (pkm->gender() == 1 ? 0x801F : WHITE) : 0xFC00), 165, 1, false);

	// Draw Pokémon Pokéball, types, and shiny star (if shiny)
	XYCoords xy = getPokeballPosition(pkm->ball());
	drawImageFromSheet(148, 1, 15, 15, ballSheet, ballSheetData.width, xy.x, xy.y, false);
	xy = getPokemonPosition(pkm->species());
	drawImageFromSheetScaled(169, 16, 32, 32, 2, pokemonSheet, pokemonSheetData.width, xy.x, xy.y, false);
	drawImageFromSheet(150, 18, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), false);
	if(pkm->type1() != pkm->type2())
		drawImageFromSheet(185, 18, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), false);
	if(pkm->shiny())	drawImage(150, 32, shinyData.width, shinyData.height, shiny, false);

	// Print Pokémon and trainer info labels
	printText("Dex #", 20, 4, false);
	printText("Name", 20, 20, false);
	printText("Ball", 20, 36, false);
	printText("Level", 20, 52, false);
	printText("Nature", 20, 68, false);
	printText("Shiny", 20, 84, false);
	printText("Pkrs", 20, 100, false);
	printText("Orig. Trainer", 20, 124, false);
	printText("Trainer ID", 20, 140, false);
	printText("Secret ID", 20, 156, false);
	printText("Friendship", 20, 172, false);

	// Print Pokémon and trainer info
	snprintf(dexNo, sizeof(dexNo), "%.3i", pkm->species());
	printText(dexNo, 64, 4, false);
	printText(pkm->nickname(), 64, 20, false);
	snprintf(ball, sizeof(ball), "%i", pkm->ball());
	printText(ball, 64, 36, false);
	snprintf(level, sizeof(level), "%i", pkm->level());
	printText(level, 64, 52, false);
	snprintf(nature, sizeof(nature), "%i", pkm->nature());
	printText(nature, 64, 68, false);
	snprintf(shinyText, sizeof(shinyText), "%s", pkm->shiny() ? "Yes" : "No");
	printText(shinyText, 64, 84, false);
	snprintf(pkrs, sizeof(pkrs), "%s", pkm->pkrsDays() ? "Yes" : "No");
	printText(pkrs, 64, 100, false);
	printTextTinted(pkm->otName(), (pkm->otGender() ? 0x801F : 0xFC00), 96, 124, false);
	snprintf(tid, sizeof(tid), "%.5i", pkm->TID());
	printText(tid, 96, 140, false);
	snprintf(sid, sizeof(sid), "%.5i", pkm->SID());
	printText(sid, 96, 156, false);
	snprintf(frnd, sizeof(frnd), "%i", pkm->otFriendship());
	printText(frnd, 96, 172, false);
}

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm) {
	// Draw the Pokémon's info
	drawPokemonStats(pkm);

	// Move arrow to first option
	setSpriteVisibility(topArrowID, false);
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, 64+getTextWidth(dexNo), -2);
	updateOam();

	int held, pressed, selection = 0;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
		} else if(held & KEY_DOWN) {
			if(selection < 10)	selection++;
		} else if(pressed & KEY_A) {
			setSpriteVisibility(bottomArrowID, false);
			switch(selection) {
				case 0: {
					int num = Input::getInt(649);
					if(num > 0)	pkm->species(num);
					break;
				} case 1: {
					std::string name = Input::getLine(10);
					if(name != "")	pkm->nickname(name);
					if(pkm->gender() != 2)	pkm->gender(Input::getBool("Female", "Male"));
					break;
				} case 2: {
					int num = Input::getInt(25);
					if(num > 0)	pkm->ball(num);
					break;
				} case 3: {
					int num = Input::getInt(100);
					if(num > 0)	pkm->level(num);
					break;
				} case 4: {
					int num = Input::getInt(); // TODO: Add limit and proper selection
					if(num != -1)	pkm->nature(num);
					break;
				} case 5: {
					pkm->shiny(!pkm->shiny());
					break;
				} case 6: {
					pkm->pkrs(pkm->pkrs() ? 0 : 0xF4);
					break;
				} case 7: {
					std::string name = Input::getLine(7);
					if(name != "")	pkm->otName(name);
					pkm->otGender(Input::getBool("Female", "Male"));
					break;
				} case 8: {
					int num = Input::getInt(65535);
					if(num != -1)	pkm->TID(num);
					break;
				} case 9: {
					int num = Input::getInt(65535);
					if(num != -1)	pkm->SID(num);
					break;
				} case 10: {
					int num = Input::getInt(255);
					if(num != -1)	pkm->otFriendship(num);
					break;
				}
			}
			setSpriteVisibility(bottomArrowID, true);
			drawPokemonStats(pkm);
		} else if(pressed & KEY_B) {
			return pkm;
		}

		switch(selection) {
			case 0:
				setSpritePosition(bottomArrowID, 64+getTextWidth(dexNo), -2);
				break;
			case 1:
				setSpritePosition(bottomArrowID, 64+getTextWidth(pkm->nickname()), 14);
				break;
			case 2:
				setSpritePosition(bottomArrowID, 64+getTextWidth(ball), 30);
				break;
			case 3:
				setSpritePosition(bottomArrowID, 64+getTextWidth(level), 46);
				break;
			case 4:
				setSpritePosition(bottomArrowID, 64+getTextWidth(nature), 62);
				break;
			case 5:
				setSpritePosition(bottomArrowID, 64+getTextWidth(shinyText), 78);
				break;
			case 6:
				setSpritePosition(bottomArrowID, 64+getTextWidth(pkrs), 94);
				break;
			case 7:
				setSpritePosition(bottomArrowID, 96+getTextWidth(pkm->otName()), 118);
				break;
			case 8:
				setSpritePosition(bottomArrowID, 96+getTextWidth(tid), 136);
				break;
			case 9:
				setSpritePosition(bottomArrowID, 96+getTextWidth(sid), 150);
				break;
			case 10:
				setSpritePosition(bottomArrowID, 96+getTextWidth(frnd), 166);
				break;
		}
		updateOam();
	}
}