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
	for(unsigned i = 0; i < partySprites.size(); i++) {
		if(save->pkm(i)->species() == pksm::Species::None) {
			partySprites[i]->visibility(false);
		} else {
			Image image = loadPokemonSprite(getPokemonIndex(*save->pkm(i)));
			partySprites[i]->drawImage(0, 0, image);
			partySprites[i]->visibility(true);
			if(save->pkm(i)->heldItem())
				partySprites[i]->drawImage(17, 32 - itemIcon.height(), itemIcon);
		}
	}
	partySprites[0]->update();
}

void fadeSprites(int alpha) {
	for(int i = 0; i < 30; i++) {
		boxSprites[false][i]->alpha(alpha);
	}
}

void enableWindows(void) {
	windowEnableSub(WINDOW_0);
	bgWindowEnable(6, WINDOW_OUT);
	bgWindowEnable(7, WINDOW_0);
	bgWindowEnable(7, WINDOW_OUT);
	oamWindowEnable(&oamSub, WINDOW_OUT);
	windowSetBoundsSub(WINDOW_0, 0, 192 - boxButton.height(), 150, boxButton.height());
}

void showParty(void) {
	// Set up windows
	enableWindows();
	bgSetScroll(6, 0, -120);
	bgUpdate();

	// Draw party tray background
	party.draw(PARTY_TRAY_X, PARTY_TRAY_Y, false, 2);

	// Fill sprites
	fillPartySprites();

	// Fade out sprites
	fadeSprites(arrowMode == 0 ? 6 : 15);

	// Scroll up
	partyY = 120, partyX = (arrowMode == 0 ? 0 : 150);
	while(partyY > 0) {
		partyY -= 6;
		bgSetScroll(6, -partyX, -partyY);
		for(int j = 0; j < 6; j++) {
			partySprites[j]->position(PARTY_TRAY_X + partySpritePos[j].first + partyX,
									  std::min(PARTY_TRAY_Y + partySpritePos[j].second + partyY, 192));
		}
		partySprites[0]->update();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Align to y = 0
	partyY = 0;
	for(int j = 0; j < 6; j++) {
		partySprites[j]->position(PARTY_TRAY_X + partySpritePos[j].first + partyX,
								  PARTY_TRAY_Y + partySpritePos[j].second + partyY);
	}
	bgSetScroll(6, -partyX, -partyY);
	bgUpdate();
	windowDisableSub(WINDOW_0);
}

void hideParty(void) {
	enableWindows();

	// Scroll down
	while(partyY < 130) {
		partyY += 6;
		bgSetScroll(6, -partyX, -partyY);
		for(int j = 0; j < 6; j++) {
			partySprites[j]->position(PARTY_TRAY_X + partySpritePos[j].first + partyX,
									  std::min(PARTY_TRAY_Y + partySpritePos[j].second + partyY, 192));
		}
		partySprites[0]->update();
		bgUpdate();
		swiWaitForVBlank();
	}

	// Reset sprite alpha
	fadeSprites(15);

	// Disable windows
	windowDisableSub(WINDOW_0);
	bgSetScroll(6, 0, 0);
	bgUpdate();

	// Clear party tray
	Graphics::drawRectangle(PARTY_TRAY_X, PARTY_TRAY_Y, party.width(), party.height(), CLEAR, false, true);

	// Hide party sprites
	for(int i = 0; i < 6; i++) {
		partySprites[i]->visibility(false);
	}
	partySprites[0]->update();
}

void toggleParty(void) {
	arrow[false].visibility(false);
	if(partyShown) {
		hideParty();
	} else {
		showParty();
	}
	partyShown = !partyShown;
	arrow[false].visibility(true);
}

void moveParty(int arrowMode, bool holdingPokemon) {
	if(!partyShown)
		return;
	arrow[false].visibility(false);
	if(arrowMode == 0 && !holdingPokemon) {
		fadeSprites(6);
		while(partyX > 0) {
			partyX -= 8;
			bgSetScroll(6, -partyX, -partyY);
			for(int j = 0; j < 6; j++) {
				partySprites[j]->position(PARTY_TRAY_X + partySpritePos[j].first + partyX,
										  PARTY_TRAY_Y + partySpritePos[j].second);
			}
			partySprites[0]->update();
			bgUpdate();
			swiWaitForVBlank();
		}
	} else {
		while(partyX < 150) {
			partyX += 8;
			bgSetScroll(6, -partyX, -partyY);
			for(int j = 0; j < 6; j++) {
				partySprites[j]->position(PARTY_TRAY_X + partySpritePos[j].first + partyX,
										  PARTY_TRAY_Y + partySpritePos[j].second);
			}
			partySprites[0]->update();
			bgUpdate();
			swiWaitForVBlank();
		}
		fadeSprites(15);
	}
	arrow[false].visibility(!topScreen);
	arrow[false].update();
}
