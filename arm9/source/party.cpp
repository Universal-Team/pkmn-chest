#include "party.h"
#include "colors.h"
#include "graphics.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "sound.h"
#include "summary.h"

struct button {
	int x;
	int y;
	std::string text;
} partyButtons[] = {
	{2, 24}, {130, 24},
	{2, 72}, {130, 72},
	{2, 120}, {130, 120},
};

void showParty(int selection) {
	for(int i=0;i<6;i++) {
		drawImage(partyButtons[i].x, partyButtons[i].y, menuButtonData.width, menuButtonData.height, selection == i ? menuButtonBlue : menuButton, false);
		if(save->pkm(i)->species() != 0) {
			std::pair<int, int> xy = getPokemonPosition(save->pkm(i));
			drawImageFromSheetScaled(partyButtons[i].x+8, partyButtons[i].y, pokemonSheetSize, pokemonSheetSize, pokemonSheetScale, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
			if(save->pkm(i)->nicknamed())	printText(save->pkm(i)->nickname(), partyButtons[i].x+47, partyButtons[i].y+14, false);
			else	printText(Lang::species[save->pkm(i)->species()], partyButtons[i].x+47, partyButtons[i].y+14, false);
		}
	}
}

void manageParty(void) {
	// Draw background
	drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);

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
		}

		if(pressed & KEY_B) {
			Sound::play(Sound::back);
			break;
		}

		if(selectedOption != -1) {
			Sound::play(Sound::click);
			if(save->pkm(selectedOption)->species() != 0) {
				save->pkm(showPokemonSummary(save->pkm(selectedOption)), selectedOption);

				// Redraw background
				drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);
				
				// Hide arrow
				setSpriteVisibility(bottomArrowID, false);
				updateOam();
			}

			selectedOption = -1;
		}

		showParty(menuSelection);
	}
}
