#include "party.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "summary.hpp"
#include "sound.hpp"

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
		drawImage(partyButtons[i].x, partyButtons[i].y, selection == i ? menuButtonBlue : menuButton, false);
		if(save->pkm(i)->species() != 0) {
			if(save->pkm(i)->nicknamed())	printText(save->pkm(i)->nickname(), partyButtons[i].x+47, partyButtons[i].y+14, false);
			else	printText(Lang::species[save->pkm(i)->species()], partyButtons[i].x+47, partyButtons[i].y+14, false);
		}
	}
}

void fillPartySprites(void) {
	// Fill sprites and set positions
	for(unsigned i=0;i<partyIconID.size();i++) {
		if(save->pkm(i)->species() == 0) {
			setSpriteVisibility(partyIconID[i], false, false);
		} else {
			Image image = loadPokemonSprite(getPokemonIndex(save->pkm(i)));
			fillSpriteImage(partyIconID[i], false, 32, 0, 0, image);
			setSpritePosition(partyIconID[i], false, partyButtons[i].x+8, partyButtons[i].y);
			setSpriteVisibility(partyIconID[i], false, true);
			updateOam();
		}
	}
}

void manageParty(void) {
	// Draw background
	drawImageDMA(0, 0, menuBg, false);

	showParty(-1);
	fillPartySprites();

	int held, pressed, menuSelection = -1, selectedOption = -1;
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
			for(unsigned i=0; i<(sizeof(partyButtons)/sizeof(partyButtons[0]));i++) {
				if(touch.px >= partyButtons[i].x && touch.px <= partyButtons[i].x+menuButton.width && touch.py >= partyButtons[i].y && touch.py <= partyButtons[i].y+menuButton.height) {
					selectedOption = i;
				}
			}
			menuSelection = -1;
		} else if(pressed & KEY_A) {
			selectedOption = menuSelection;
		}

		if(pressed & KEY_B) {
			for(unsigned i=0;i<partyIconID.size();i++) {
				setSpriteVisibility(partyIconID[i], false, false);
			}
			updateOam();
			Sound::play(Sound::back);
			break;
		}

		if(selectedOption != -1) {
			for(unsigned i=0;i<partyIconID.size();i++) {
				setSpriteVisibility(partyIconID[i], false, false);
			}
			updateOam();
			Sound::play(Sound::click);
			if(save->pkm(selectedOption)->species() != 0) {
				save->pkm(showPokemonSummary(save->pkm(selectedOption)), selectedOption);

				// Redraw background
				drawImageDMA(0, 0, menuBg, false);

				// Show sprites
				showParty(menuSelection);
				fillPartySprites();
				
				// Hide arrow
				setSpriteVisibility(arrowID, false, false);
				updateOam();
			}

			selectedOption = -1;
		}

		showParty(menuSelection);
	}
}
