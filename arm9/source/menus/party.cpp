#include "party.hpp"

#include "PKX.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "sound.hpp"
#include "summary.hpp"

bool partyShown = false;
int partyX = 0, partyY = 0;

void fillPartySprites(void) {
	for(unsigned i = 0; i < partyIconID.size(); i++) {
		if(save->pkm(i)->species() == pksm::Species::None) {
			setSpriteVisibility(partyIconID[i], false, false);
		} else {
			Image image = loadPokemonSprite(getPokemonIndex(*save->pkm(i)));
			fillSpriteImage(partyIconID[i], false, 32, 0, 0, image);
			setSpriteVisibility(partyIconID[i], false, true);
			if(save->pkm(i)->heldItem())
				fillSpriteImage(partyIconID[i], false, 32, 17, 32 - itemIcon.height, itemIcon, true);
		}
	}
	updateOam();
}

void fadeSprites(int alpha) {
	for(int i = 0; i < 30; i++) {
		setSpriteAlpha(i, false, alpha);
	}
}

void enableWindows(void) {
	windowEnableSub(WINDOW_0);
	bgWindowEnable(bg2Sub, WINDOW_OUT);
	bgWindowEnable(bg3Sub, WINDOW_0);
	bgWindowEnable(bg3Sub, WINDOW_OUT);
	oamWindowEnable(&oamSub, WINDOW_OUT);
	windowSetBoundsSub(WINDOW_0, 0, 192 - boxButton.height, 150, boxButton.height);
}

void showParty(void) {
	// Set up windows
	enableWindows();
	bgSetScroll(bg2Sub, 0, -120);
	bgUpdate();

	// Draw party tray background
	drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);

	// Fill sprites
	fillPartySprites();

	// Fade out sprites
	fadeSprites(arrowMode == 0 ? 6 : 15);

	// Scroll up
	partyY = 120, partyX = (arrowMode == 0 ? 0 : 150);
	while(partyY > 0) {
		partyY -= 6;
		bgSetScroll(bg2Sub, -partyX, -partyY);
		for(int j = 0; j < 6; j++) {
			setSpritePosition(partyIconID[j],
							  false,
							  PARTY_TRAY_X + partySpritePos[j].first + partyX,
							  std::min(PARTY_TRAY_Y + partySpritePos[j].second + partyY, 192));
		}
		updateOam();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Align to y = 0
	partyY = 0;
	for(int j = 0; j < 6; j++) {
		setSpritePosition(partyIconID[j],
						  false,
						  PARTY_TRAY_X + partySpritePos[j].first + partyX,
						  PARTY_TRAY_Y + partySpritePos[j].second + partyY);
	}
	bgSetScroll(bg2Sub, -partyX, -partyY);
	bgUpdate();
	windowDisableSub(WINDOW_0);
}

void hideParty(void) {
	enableWindows();

	// Scroll down
	while(partyY < 130) {
		partyY += 6;
		bgSetScroll(bg2Sub, -partyX, -partyY);
		for(int j = 0; j < 6; j++) {
			setSpritePosition(partyIconID[j],
							  false,
							  PARTY_TRAY_X + partySpritePos[j].first + partyX,
							  std::min(PARTY_TRAY_Y + partySpritePos[j].second + partyY, 192));
		}
		updateOam();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Reset sprite alpha
	fadeSprites(15);

	// Disable windows
	windowDisableSub(WINDOW_0);
	bgSetScroll(bg2Sub, 0, 0);
	bgUpdate();

	// Clear party tray
	drawRectangle(PARTY_TRAY_X, PARTY_TRAY_Y, party.width, party.height, CLEAR, false, true);

	// Hide party sprites
	for(int i = 0; i < 6; i++) {
		setSpriteVisibility(partyIconID[i], false, false);
	}
	updateOam();
}

void toggleParty(void) {
	setSpriteVisibility(arrowID, false, false);
	if(partyShown) {
		hideParty();
	} else {
		showParty();
	}
	partyShown = !partyShown;
	setSpriteVisibility(arrowID, false, true);
}

void moveParty(int arrowMode, bool holdingPokemon) {
	if(!partyShown)
		return;
	setSpriteVisibility(arrowID, false, false);
	if(arrowMode == 0 && !holdingPokemon) {
		fadeSprites(6);
		while(partyX > 0) {
			partyX -= 8;
			bgSetScroll(bg2Sub, -partyX, -partyY);
			for(int j = 0; j < 6; j++) {
				setSpritePosition(partyIconID[j],
								  false,
								  PARTY_TRAY_X + partySpritePos[j].first + partyX,
								  PARTY_TRAY_Y + partySpritePos[j].second);
			}
			updateOam();
			bgUpdate();
			swiWaitForVBlank();
		}
	} else {
		while(partyX < 150) {
			partyX += 8;
			bgSetScroll(bg2Sub, -partyX, -partyY);
			for(int j = 0; j < 6; j++) {
				setSpritePosition(partyIconID[j],
								  false,
								  PARTY_TRAY_X + partySpritePos[j].first + partyX,
								  PARTY_TRAY_Y + partySpritePos[j].second);
			}
			updateOam();
			bgUpdate();
			swiWaitForVBlank();
		}
		fadeSprites(15);
	}
	setSpriteVisibility(arrowID, false, !topScreen);
	updateOam();
}
