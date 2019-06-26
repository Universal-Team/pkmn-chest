#include "party.h"
#include "graphics/colors.h"
#include "graphics/graphics.h"
#include "loader.h"
#include "manager.h"
#include "summary.h"

struct button {
	int x;
	int y;
	std::string text;
} partyButtons[] = {
	{3, 24}, {131, 24},
	{3, 72}, {131, 72},
	{3, 120}, {131, 120},
};

void showParty(int selection) {
	for(int i=0;i<6;i++) {
		XYCoords xy = getPokemonPosition(save->pkm(i)->species());
		drawImageTinted(partyButtons[i].x, partyButtons[i].y, menuButtonData.width, menuButtonData.height, selection == i ? TEAL_RGB : LIGHT_GRAY, menuButton, false);
		drawImageFromSheet(partyButtons[i].x+8, partyButtons[i].y, 32, 32, pokemonSheet, pokemonSheetData.width, xy.x, xy.y, false);
		printText(save->pkm(i)->nickname(), partyButtons[i].x+47, partyButtons[i].y+14, false);
	}
}

void manageParty(void) {
	// Draw background
	drawRectangle(0, 0, 256, 16, BLACK, false);
	drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	showParty(-1);

	int held = 0, pressed = 0, menuSelection = -1, selectedOption = -1;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			menuSelection = 0;
		} else if(held & KEY_UP) {
			if(menuSelection > 1)	menuSelection -= 2;
		} else if(held & KEY_DOWN) {
			if(menuSelection < 4)	menuSelection += 2;
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2)	menuSelection--;
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2))	menuSelection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(uint i=0; i<(sizeof(partyButtons)/sizeof(partyButtons[0]));i++) {
				if(touch.px >= partyButtons[i].x && touch.px <= partyButtons[i].x+menuButtonData.width && touch.py >= partyButtons[i].y && touch.py <= partyButtons[i].y+menuButtonData.height) {
					selectedOption = i;
				}
			}
			menuSelection = -1;
		} else if(pressed & KEY_A) {
			selectedOption = menuSelection;
		} else if(pressed & KEY_B) {
			break;
		}

		if(selectedOption != -1) {
			save->pkm(showPokemonSummary(save->pkm(selectedOption)), selectedOption);

			// Redraw background
			drawRectangle(0, 0, 256, 16, BLACK, false);
			drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
			drawRectangle(0, 176, 256, 16, BLACK, false);
			
			// Hide arrow
			setSpriteVisibility(bottomArrowID, false);
			updateOam();

			selectedOption = -1;
		}

		showParty(menuSelection);
	}
}
