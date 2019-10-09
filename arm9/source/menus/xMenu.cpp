#include "xMenu.hpp"

#include "banks.hpp"
#include "cardSaves.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "configMenu.hpp"
#include "flashcard.hpp"
#include "input.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "party.hpp"
#include "sound.hpp"
#include "trainer.hpp"

std::vector<std::pair<int, int>> xMenuButtons = {
	{2,  24}, {130,  24},
	{2,  72}, {130,  72},
	{2, 120}, {130, 120},
};

void savePrompt(void) {
	// Draw background
	drawRectangle(0, 0, 256, 32, LIGHT_GRAY, false);
	drawRectangle(0, 32, 256, 144, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	printTextTinted(Lang::saveMsgChest, GRAY, 5, 0, false, true);
	if(Input::getBool(Lang::save, Lang::discard)) {
		if(Config::backupAmount != 0) Banks::bank->backup();
		Banks::bank->save();
	}

	drawRectangle(5, 33, 246, 16, DARK_GRAY, false);
	drawRectangle(0, 0, 256, 32, LIGHT_GRAY, false);
	if(savePath == cardSave)	printTextTinted(Lang::saveMsgCard, GRAY, 5, 0, false, true);
	else	printTextTinted(Lang::saveMsgSave, GRAY, 5, 0, false, true);

	if(Input::getBool(Lang::save, Lang::discard)) {
		// Re-encrypt the box data
		save->cryptBoxData(false);
		// Save changes to save file
		saveChanges(savePath);
		// Reload save
		loadSave(savePath);
		save->cryptBoxData(true);
		if(savePath == cardSave) {
			drawRectangle(0, 32, 256, 32, DARK_GRAY, false);
			updateCardInfo();
			if(!restoreSave()) {
				drawRectangle(0, 0, 256, 192, DARK_GRAY, true);
				drawBox(true);
			}
		}
	}
}

void drawXMenuButtons(unsigned menuSelection) {
	Lang::xMenuText[3] = save->otName();

	for(unsigned i=0;i<xMenuButtons.size();i++) {
		drawImage(xMenuButtons[i].first, xMenuButtons[i].second, menuButtonData.width, menuButtonData.height, menuSelection == i ? menuButtonBlue : menuButton, false);
		printText(Lang::xMenuText[i], xMenuButtons[i].first+47, xMenuButtons[i].second+14, false);
		oamSetAlpha(&oamSub, menuIconID[i], menuSelection == i ? 8 : 15);
		updateOam();
	}
}

bool xMenu(void) {
	// Hide bottom sprites
	for(unsigned i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	setSpriteVisibility(bottomArrowID, false);
	updateOam();

	// Make bottom arrow red
	fillSpriteImage(bottomArrowID, arrowRed);

	// Draw background
	if(sdFound())	drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);
	else {
		drawRectangle(0, 0, 256, 16, BLACK, false);
		drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
		drawRectangle(0, 176, 256, 16, BLACK, false);
	}

	// Enable sprites and set positions
	for(unsigned i=0;i<menuIconID.size();i++) {
		setSpritePosition(menuIconID[i], xMenuButtons[i].first+3, xMenuButtons[i].second+6);
		setSpriteVisibility(menuIconID[i], true);
	}
	updateOam();

	drawXMenuButtons(-1);

	bool iconDirection = true;
	int pressed, menuSelection = -1, selectedOption = -1, iconOffset = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			if(menuSelection != -1) {
				if(iconDirection) {
					if(iconOffset < 6)	iconOffset++;
					else if(iconOffset < 12)	iconOffset++;
					else	iconDirection = false;
				} else {
					if(iconOffset > -6)	iconOffset--;
					else	iconDirection = true;
				}
				if(iconOffset < 7) {
					setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6-(iconOffset/3));
					updateOam();
				}
			}
		} while(!pressed);

		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection -= 2;
			}
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < (int)xMenuButtons.size()-2) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection += 2;
			}
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection--;
			}
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2)) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection++;
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0; i<xMenuButtons.size();i++) {
				if(touch.px >= xMenuButtons[i].first && touch.px <= xMenuButtons[i].first+menuButtonData.width && touch.py >= xMenuButtons[i].second && touch.py <= xMenuButtons[i].second+menuButtonData.height) {
					selectedOption = i;
				}
			}
			if(menuSelection != -1) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].first+3, xMenuButtons[menuSelection].second+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection = -1;
			}
		} else if(pressed & KEY_A) {
			selectedOption = menuSelection;
		}

		if(pressed & KEY_B || pressed & KEY_X) {
			Sound::play(Sound::back);
			// Reset arrow color
			fillSpriteImage(bottomArrowID, arrowMode ? arrowBlue : arrowRed);
			setSpriteVisibility(topScreen ? topArrowID : bottomArrowID, true);
			// Hide menu icons
			for(int i=0;i<6;i++) {
				setSpriteVisibility(menuIconID[i], false);
			}
			updateOam();
			drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);
			drawBox(false);
			break;
		}

		if(selectedOption != -1) {
			// Hide menu icons
			iconOffset = 0;
			iconDirection = true;
			for(int i=0;i<6;i++) {
				setSpriteVisibility(menuIconID[i], false);
			}
			updateOam();
			Sound::play(Sound::click);
			switch(selectedOption) {
				case 0: // Party
					manageParty();
					break;
				case 1: // Options
					configMenu();
					break;
				case 3: // Trainer
					showTrainerCard();

					// Hide arrow
					setSpriteVisibility(bottomArrowID, false);
					updateOam();
					break;
				case 4: // Save
					savePrompt();
					break;
				case 5:
					savePrompt();
					// Hide remaining sprites
					for(unsigned i=30;i<getSpriteAmount();i++) {
						setSpriteVisibility(i, false);
					}
					updateOam();
					return 0;
			}

			// Redraw menu
			if(sdFound())	drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);
			else {
				drawRectangle(0, 0, 256, 16, BLACK, false);
				drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
				drawRectangle(0, 176, 256, 16, BLACK, false);
			}
			for(unsigned i=0;i<menuIconID.size();i++) {
				setSpritePosition(menuIconID[i], xMenuButtons[i].first+3, xMenuButtons[i].second+6);
				setSpriteVisibility(menuIconID[i], true);
			}
			updateOam();
			drawXMenuButtons(menuSelection);

			selectedOption = -1;
		}

		drawXMenuButtons(menuSelection);
	}
	return 1;
}
