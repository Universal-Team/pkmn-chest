#include "summary.h"
#include "graphics/colors.h"
#include "keyboard.h"
#include "manager.h"

struct Button {
	int x;
	int y;
	char text[11];
};

Button buttons[] {
	{64, 4},
	{64, 20},
	{64, 36},
	{64, 52},
	{64, 68},
	{64, 84},
	{64, 100},
	{64, 124},
	{96, 140},
	{96, 156},
	{96, 172},
};

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
	printText("Dex #", 20, buttons[0].y, false);
	printText("Name", 20, buttons[1].y, false);
	printText("Ball", 20, buttons[2].y, false);
	printText("Level", 20, buttons[3].y, false);
	printText("Nature", 20, buttons[4].y, false);
	printText("Shiny", 20, buttons[5].y, false);
	printText("Pkrs", 20, buttons[6].y, false);
	printText("Orig. Trainer", 20, buttons[7].y, false);
	printText("Trainer ID", 20, buttons[8].y, false);
	printText("Secret ID", 20, buttons[9].y, false);
	printText("Friendship", 20, buttons[10].y, false);

	// Print Pokémon and trainer info
	snprintf(buttons[0].text,  sizeof(buttons[0].text), "%.3i", pkm->species());
	snprintf(buttons[1].text,  sizeof(buttons[1].text), "%s", pkm->nickname().c_str());
	snprintf(buttons[2].text,  sizeof(buttons[2].text), "%i", pkm->ball());
	snprintf(buttons[3].text,  sizeof(buttons[3].text), "%i", pkm->level());
	snprintf(buttons[4].text,  sizeof(buttons[4].text), "%i", pkm->nature());
	snprintf(buttons[5].text,  sizeof(buttons[5].text), "%s", pkm->shiny() ? "Yes" : "No");
	snprintf(buttons[6].text,  sizeof(buttons[6].text), "%s", pkm->pkrsDays() ? "Yes" : "No");
	snprintf(buttons[7].text,  sizeof(buttons[7].text), "%s", pkm->otName().c_str());
	snprintf(buttons[8].text,  sizeof(buttons[8].text), "%.5i", pkm->TID());
	snprintf(buttons[9].text,  sizeof(buttons[9].text), "%.5i", pkm->SID());
	snprintf(buttons[10].text, sizeof(buttons[10].text),"%i", pkm->otFriendship());
	for(uint i=0;i<(sizeof(buttons)/sizeof(buttons[0]));i++) {
		if(i!=7)	// OT Name is colored
			printText(buttons[i].text, buttons[i].x, buttons[i].y, false);
		else
			printTextTinted(buttons[i].text, (pkm->otGender() ? 0x801F : 0xFC00), buttons[i].x, buttons[i].y, false);
	}
}

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm) {
	// Draw the Pokémon's info
	drawPokemonStats(pkm);

	// Move arrow to first option
	setSpriteVisibility(topArrowID, false);
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, buttons[0].x+getTextWidth(buttons[0].text), buttons[0].y-6);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0;
	touchPosition touch;
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
			optionSelected = true;
		} else if(pressed & KEY_B) {
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(uint i=0;i<(sizeof(buttons)/sizeof(buttons[0]));i++) {
				if(touch.px >= buttons[i].x && touch.px <= buttons[i].x+getTextWidth(buttons[i].text) && touch.py >= buttons[i].y && touch.py <= buttons[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
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
		}

		setSpritePosition(bottomArrowID, buttons[selection].x+getTextWidth(buttons[selection].text), buttons[selection].y-6);
		updateOam();
	}
}