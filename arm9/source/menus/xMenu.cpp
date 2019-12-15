#include "xMenu.hpp"

#include "bag.hpp"
#include "banks.hpp"
#include "cardSaves.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "configMenu.hpp"
#include "flashcard.hpp"
#include "input.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "party.hpp"
#include "sound.hpp"
#include "trainer.hpp"

std::vector<Label> xMenuButtons = {
	{2,  24, "party"}, {130,  24, "options"},
	{2,  72,   "bag"}, {130,  72,        ""},
	{2, 120,  "save"}, {130, 120,    "exit"},
};

void savePrompt(void) {
	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw background
	drawImageDMA(0, 0, menuBg, false, false);
	drawRectangle(0, 0, 256, 33, LIGHT_GRAY, false, false);
	drawRectangle(0, 33, 256, 1, BLACK, false, false, false);
	drawRectangle(0, 34, 256, 2, DARK_GRAY, false, false);
	drawRectangle(0, 36, 256, 1, DARKERER_GRAY, false, false);

	printTextTinted(Lang::get("saveMsgChest"), GRAY_TEXT, 5, 0, false, true);
	if(Input::getBool(Lang::get("save"), Lang::get("discard"))) {
		drawRectangle(0, 0, 256, 32, CLEAR, false, true);
		printTextTinted(Lang::get("reopenIfFrozen"), GRAY_TEXT, 5, 0, false, true);
		Banks::bank->backup();
		Banks::bank->save();
	}

	drawRectangle(0, 0, 256, 32, CLEAR, false, true);
	if(savePath == cardSave)	printTextTinted(Lang::get("saveMsgCard"), GRAY_TEXT, 5, 0, false, true);
	else	printTextTinted(Lang::get("saveMsgSave"), GRAY_TEXT, 5, 0, false, true);

	if(Input::getBool(Lang::get("save"), Lang::get("discard"))) {
		drawRectangle(0, 0, 256, 32, CLEAR, false, true);
		printTextTinted(Lang::get("reopenIfFrozen"), GRAY_TEXT, 5, 0, false, true);
		// Re-encrypt the box data
		save->cryptBoxData(false);
		// Save changes to save file
		saveChanges(savePath);
		// Reload save
		loadSave(savePath);
		save->cryptBoxData(true);
		if(savePath == cardSave) {
			updateCardInfo();
			restoreSave();
		}
	}

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
}

void drawXMenuButtons(unsigned menuSelection) {
	xMenuButtons[3].label = save->otName();

	for(unsigned i=0;i<xMenuButtons.size();i++) {
		drawImage(xMenuButtons[i].x, xMenuButtons[i].y, menuSelection == i ? menuButtonBlue : menuButton, false, false);
		printText((i==3) ? xMenuButtons[i].label : Lang::get(xMenuButtons[i].label), xMenuButtons[i].x+47, xMenuButtons[i].y+14, false, true);
		setSpriteAlpha(menuIconID[i], false, menuSelection == i ? 8 : 15);
		updateOam();
	}
}

bool xMenu(void) {
	// Hide bottom sprites
	for(unsigned i=0;i<30;i++) {
		setSpriteVisibility(i, false, false);
	}
	setSpriteVisibility(arrowID, false, false);
	updateOam();

	// Make bottom arrow red
	fillArrow(0);

	// Draw background
	drawImageDMA(0, 0, menuBg, false, false);

	// Enable sprites and set positions
	for(unsigned i=0;i<menuIconID.size();i++) {
		setSpritePosition(menuIconID[i], false, xMenuButtons[i].x+3, xMenuButtons[i].y+6);
		setSpriteVisibility(menuIconID[i], false, true);
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
					setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6-(iconOffset/3));
					updateOam();
				}
			}
		} while(!pressed);

		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1) {
				setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection -= 2;
			}
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < (int)xMenuButtons.size()-2) {
				setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection += 2;
			}
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2) {
				setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection--;
			}
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2)) {
				setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection++;
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0; i<xMenuButtons.size();i++) {
				if(touch.px >= xMenuButtons[i].x && touch.px <= xMenuButtons[i].x+menuButton.width && touch.py >= xMenuButtons[i].y && touch.py <= xMenuButtons[i].y+menuButton.height) {
					selectedOption = i;
				}
			}
			if(menuSelection != -1) {
				setSpritePosition(menuIconID[menuSelection], false, xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
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
			fillArrow(arrowMode);
			setSpriteVisibility(arrowID, topScreen, true);
			// Hide menu icons
			for(int i=0;i<6;i++) {
				setSpriteVisibility(menuIconID[i], false, false);
			}
			updateOam();
			drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			drawBox(false);
			break;
		}

		if(selectedOption != -1) {
			// Hide menu icons
			iconOffset = 0;
			iconDirection = true;
			for(int i=0;i<6;i++) {
				setSpriteVisibility(menuIconID[i], false, false);
			}
			updateOam();
			Sound::play(Sound::click);
			switch(selectedOption) {
				case 1: // Options
					configMenu();
					break;
				case 2: // Bag
					editBag();
					break;
				case 3: // Trainer
					showTrainerCard();

					// Hide arrow
					setSpriteVisibility(arrowID, false, false);
					updateOam();
					break;
				case 4: // Save
					savePrompt();
					break;
				case 5: // Exit
					savePrompt();
					// Hide remaining sprites
					for(unsigned i=0;i<128;i++) {
						setSpriteVisibility(i, false, false);
					}
					for(unsigned i=0;i<128;i++) {
						setSpriteVisibility(i, true, false);
					}
					updateOam();
					return 0;
			}

			// Redraw menu
			drawImage(0, 0, menuBg, false, false);
			for(unsigned i=0;i<menuIconID.size();i++) {
				setSpritePosition(menuIconID[i], false, xMenuButtons[i].x+3, xMenuButtons[i].y+6);
				setSpriteVisibility(menuIconID[i], false, true);
			}
			updateOam();
			drawXMenuButtons(menuSelection);

			selectedOption = -1;
		}

		drawXMenuButtons(menuSelection);
	}
	return 1;
}
