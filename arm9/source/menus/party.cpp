#include "party.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "summary.hpp"
#include "sound.hpp"

#define PARTY_TRAY_X 20
#define PARTY_TRAY_Y 48

bool partyShown = false;
int partyX = 0, partyY = 0;

const std::pair<int, int> partySpritePos[] = {
	{4,  0}, {44,  8},
	{4, 32}, {44, 40},
	{4, 64}, {44, 72},
};

void showParty(void) {
	// Set up windows
	windowEnableSub(WINDOW_0);
	bgWindowEnable(bg2Sub, WINDOW_0);
	bgWindowEnable(bg3Sub, WINDOW_0);
	bgWindowEnable(bg3Sub, WINDOW_OUT);
	oamWindowEnable(&oamSub, WINDOW_0);
	windowSetBoundsSub(WINDOW_0, 0, 0, 255, 192-boxButton.height);
	bgSetScroll(bg2Sub, 0, -120);
	bgUpdate();

	// Draw party tray background
	drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);

	// Fill sprites
	for(int i=0;i<6;i++) {
		Image image = loadPokemonSprite(getPokemonIndex(save->pkm(i)));
		fillSpriteImage(partyIconID[i], false, 32, 0, 0, image);
		setSpriteVisibility(partyIconID[i], false, true);
	}

	// Fade out sprites
	for(int i=0;i<30;i++) {
		setSpriteAlpha(i, false, 4);
	}

	// Scroll up
	partyY = 120, partyX = (arrowMode == 0 ? 0 : 150);
	while(partyY > 0) {
		partyY -= 6;
		bgSetScroll(bg2Sub, -partyX, -partyY);
		for(int j=0;j<6;j++) {
			setSpritePosition(partyIconID[j], false, PARTY_TRAY_X + partySpritePos[j].first + partyX, PARTY_TRAY_Y + partySpritePos[j].second + partyY);
		}
		updateOam();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Align to y = 0
	partyY = 0;
	for(int j=0;j<6;j++) {
		setSpritePosition(partyIconID[j], false, PARTY_TRAY_X + partySpritePos[j].first + partyX, PARTY_TRAY_Y + partySpritePos[j].second + partyY);
	}
	bgSetScroll(bg2Sub, -partyX, -partyY);
	bgUpdate();
}

void hideParty(void) {
	// Scroll down
	while(partyY < 130) {
		partyY += 6;
		bgSetScroll(bg2Sub, -partyX, -partyY);
		for(int j=0;j<6;j++) {
			setSpritePosition(partyIconID[j], false, PARTY_TRAY_X + partySpritePos[j].first + partyX, std::min(PARTY_TRAY_Y + partySpritePos[j].second + partyY, 192));
		}
		updateOam();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Reset sprite alpha
	for(int i=0;i<30;i++) {
		setSpriteAlpha(i, false, 15);
	}

	// Disable windows
	windowDisableSub(WINDOW_0);
	bgSetScroll(bg2Sub, 0, 0);
	bgUpdate();

	// Clear party tray
	drawRectangle(PARTY_TRAY_X, PARTY_TRAY_Y, party.width, party.height, CLEAR, false, true);

	// Hide party sprites
	for(int i=0;i<6;i++) {
		setSpriteVisibility(partyIconID[i], false, false);
	}
	updateOam();
}

void toggleParty(void) {
	if(partyShown) {
		hideParty();
	} else {
		showParty();
	}
	partyShown = !partyShown;
}

void moveParty(void) {
	if(arrowMode == 0) {
		while(partyX > 0) {
			partyX -= 8;
			bgSetScroll(bg2Sub, -partyX, -partyY);
			for(int j=0;j<6;j++) {
				setSpritePosition(partyIconID[j], false, PARTY_TRAY_X + partySpritePos[j].first + partyX, PARTY_TRAY_Y + partySpritePos[j].second);
			}
			updateOam();
			bgUpdate();
			swiWaitForVBlank();
		}
	} else {
		while(partyX < 150) {
			partyX += 8;
			bgSetScroll(bg2Sub, -partyX, -partyY);
			for(int j=0;j<6;j++) {
				setSpritePosition(partyIconID[j], false, PARTY_TRAY_X + partySpritePos[j].first + partyX, PARTY_TRAY_Y + partySpritePos[j].second);
			}
			updateOam();
			bgUpdate();
			swiWaitForVBlank();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
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
		drawImage(partyButtons[i].x, partyButtons[i].y, selection == i ? menuButtonBlue : menuButton, false, false);
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
	drawImageDMA(0, 0, menuBg, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

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
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
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
				drawImageDMA(0, 0, menuBg, false, false);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);

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
