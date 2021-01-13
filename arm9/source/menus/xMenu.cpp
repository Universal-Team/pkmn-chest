#include "xMenu.hpp"

#include "bag.hpp"
#include "banks.hpp"
#include "cardSaves.hpp"
#include "cartSaves.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "configMenu.hpp"
#include "flashcard.hpp"
#include "gui.hpp"
#include "i18n_ext.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "party.hpp"
#include "sound.hpp"
#include "trainer.hpp"

// clang-format off
std::vector<Label> xMenuButtons = {
	{2,  24, "party"}, {130,  24, "options"},
	{2,  72,   "bag"}, {130,  72,        ""},
	{2, 120,  "save"}, {130, 120,    "exit"},
};
// clang-format on

void savePrompt(void) {
	// Clear text
	Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw background
	menuBg.draw(0, 0, false, 3);
	Graphics::drawRectangle(0, 0, 256, 33, LIGHT_GRAY, false, false);
	Graphics::drawRectangle(0, 33, 256, 1, BLACK, false, false, false);
	Graphics::drawRectangle(0, 34, 256, 2, DARK_GRAY, false, false);
	Graphics::drawRectangle(0, 36, 256, 1, DARKERER_GRAY, false, false);

	Gui::font.print(i18n::localize(Config::getLang("lang"), "saveMsgChest"), 5, 0, false, 2, Alignment::left, 0,
					TextColor::gray);
	if(Input::getBool(i18n::localize(Config::getLang("lang"), "save"),
					  i18n::localize(Config::getLang("lang"), "discard"))) {
		Graphics::drawRectangle(0, 0, 256, 32, CLEAR, false, true);
		Gui::font.print(i18n::localize(Config::getLang("lang"), "reopenIfFrozen"), 5, 0, false, 2, Alignment::left, 0,
						TextColor::gray);
		Banks::bank->backup();
		Banks::bank->save();
	}

	Graphics::drawRectangle(0, 0, 256, 32, CLEAR, false, true);
	if(savePath == cardSave)
		Gui::font.print(i18n::localize(Config::getLang("lang"), "saveMsgCard"), 5, 0, false, 2, Alignment::left, 0,
						TextColor::gray);
	else
		Gui::font.print(i18n::localize(Config::getLang("lang"), "saveMsgSave"), 5, 0, false, 2, Alignment::left, 0,
						TextColor::gray);

	if(Input::getBool(i18n::localize(Config::getLang("lang"), "save"),
					  i18n::localize(Config::getLang("lang"), "discard"))) {
		Graphics::drawRectangle(0, 0, 256, 32, CLEAR, false, true);
		Gui::font.print(i18n::localize(Config::getLang("lang"), "reopenIfFrozen"), 5, 0, false, 2, Alignment::left, 0,
						TextColor::gray);
		// Re-encrypt the box data
		save->cryptBoxData(false);
		// Save changes to save file
		saveChanges(savePath);
		// Reload save
		loadSave(savePath);
		save->cryptBoxData(true);
		if(savePath == cardSave) {
			updateCardInfo();
			restoreSlot1();
		} else if(savePath == cartSave) {
			restoreSlot2();
		}
	}

	// Clear text
	Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
}

void drawXMenuButtons(unsigned menuSelection) {
	xMenuButtons[3].label = save->otName();

	for(unsigned i = 0; i < xMenuButtons.size(); i++) {
		(menuSelection == i ? menuButtonBlue : menuButton).draw(xMenuButtons[i].x, xMenuButtons[i].y, false, 2);
		if(i != 0) // skipping 0 (party) until I replace it with something
			Gui::font.print((i == 3) ? xMenuButtons[i].label
									 : i18n::localize(Config::getLang("lang"), xMenuButtons[i].label),
							xMenuButtons[i].x + 47, xMenuButtons[i].y + 14, false, true);
		menuSprites[i].alpha(menuSelection == i ? 8 : 15);
	}
	menuSprites[0].update();
}

bool xMenu(void) {
	// Hide bottom sprites
	for(unsigned i = 0; i < 30; i++) {
		boxSprites[false][i].visibility(false);
	}
	arrow[false].visibility(false);
	arrow[false].update();

	// Make arrow red
	fillArrow(0);

	// Draw background
	menuBg.draw(0, 0, false, 3);

	// Enable sprites and set positions
	for(unsigned i = 0; i < menuSprites.size(); i++) {
		menuSprites[i].position(xMenuButtons[i].x + 3, xMenuButtons[i].y + 6);
		menuSprites[i].visibility(true);
	}
	menuSprites[0].update();

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
					if(iconOffset < 6)
						iconOffset++;
					else if(iconOffset < 12)
						iconOffset++;
					else
						iconDirection = false;
				} else {
					if(iconOffset > -6)
						iconOffset--;
					else
						iconDirection = true;
				}
				if(iconOffset < 7) {
					menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
														xMenuButtons[menuSelection].y + 6 - (iconOffset / 3));
					menuSprites[menuSelection].update();
				}
			}
		} while(!pressed);

		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1) {
				menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
													xMenuButtons[menuSelection].y + 6);
				iconOffset    = 0;
				iconDirection = true;

				menuSelection -= 2;
			}
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < (int)xMenuButtons.size() - 2) {
				menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
													xMenuButtons[menuSelection].y + 6);
				iconOffset    = 0;
				iconDirection = true;

				menuSelection += 2;
			}
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2) {
				menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
													xMenuButtons[menuSelection].y + 6);
				iconOffset    = 0;
				iconDirection = true;

				menuSelection--;
			}
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2)) {
				menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
													xMenuButtons[menuSelection].y + 6);
				iconOffset    = 0;
				iconDirection = true;

				menuSelection++;
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < xMenuButtons.size(); i++) {
				if(touch.px >= xMenuButtons[i].x && touch.px <= xMenuButtons[i].x + menuButton.width() &&
				   touch.py >= xMenuButtons[i].y && touch.py <= xMenuButtons[i].y + menuButton.height()) {
					selectedOption = i;
				}
			}
			if(menuSelection != -1) {
				menuSprites[menuSelection].position(xMenuButtons[menuSelection].x + 3,
													xMenuButtons[menuSelection].y + 6);
				iconOffset    = 0;
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
			if(!topScreen)
				arrow[false].visibility(true);
			// Hide menu icons
			for(int i = 0; i < 6; i++) {
				menuSprites[i].visibility(false);
			}
			menuSprites[0].update();
			Graphics::drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
			Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			drawBox(false);
			break;
		}

		if(selectedOption != -1) {
			// Hide menu icons
			iconOffset    = 0;
			iconDirection = true;
			for(int i = 0; i < 6; i++) {
				menuSprites[i].visibility(false);
			}
			menuSprites[0].update();
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
					arrow[false].visibility(false);
					arrow[false].update();
					break;
				case 4: // Save
					savePrompt();
					break;
				case 5: // Exit
					savePrompt();
					// Hide remaining sprites
					for(unsigned i = 0; i < boxSprites[true].size(); i++) {
						boxSprites[false][i].visibility(false);
						boxSprites[true][i].visibility(false);
					}
					Sprite::update(true);
					Sprite::update(false);
					return 0;
			}

			// Redraw menu
			menuBg.draw(0, 0, false, 3);
			for(unsigned i = 0; i < menuSprites.size(); i++) {
				menuSprites[i].position(xMenuButtons[i].x + 3, xMenuButtons[i].y + 6);
				menuSprites[i].visibility(true);
			}
			menuSprites[0].update();
			drawXMenuButtons(menuSelection);

			selectedOption = -1;
		}

		drawXMenuButtons(menuSelection);
	}
	return 1;
}
