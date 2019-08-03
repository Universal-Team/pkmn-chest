#include "menus.h"
#include <dirent.h>
#include <fstream>

#include "banks.hpp"
#include "cardSaves.h"
#include "colors.h"
#include "config.h"
#include "configMenu.h"
#include "fileBrowse.h"
#include "flashcard.h"
#include "graphics.h"
#include "input.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "party.h"
#include "sound.h"
#include "summary.h"
#include "trainer.h"

std::vector<TextPos> aMenuButtons = {
	{170,  16}, // Move
	{170,  42}, // Edit
	{170,  68}, // Copy
	{170,  94}, // Release
	{170, 120}, // Dump
	{170, 146}, // Back
};
std::vector<TextPos> aMenuEmptySlotButtons = {
	{170, 16}, // Inject
	{170, 42}, // Create
	{170, 68}, // Back
};
std::vector<TextPos> aMenuTopBarButtons = {
	{170, 16}, // Jump
	{170, 42}, // Rename
	{170, 68}, // Swap
	{170, 94}, // Dump box
	{170, 120}, // Back
};
std::vector<TextPos> xMenuButtons = {
	{2,  24}, {130,  24},
	{2,  72}, {130,  72},
	{2, 120}, {130, 120},
};

struct FormCount {
	int dexNo;
	int noForms;
} formCounts[] = {
	{201, 27}, // Unown
	{386,  4}, // Deoxys
	{412,  3}, // Burmy
	{413,  3}, // Wormadam
	{422,  2}, // Shellos
	{423,  2}, // Gastrodon
	{479,  6}, // Rotom
	{487,  2}, // Giratina
	{492,  2}, // Shaymin
	{550,  2}, // Basculin
	{555,  2}, // Darmanitan // Not sure if I should have this or not
	{585,  4}, // Deerling
	{586,  4}, // Sawsbuck
	{648,  2}, // Meloetta
	{641,  2}, // Tornadus
	{642,  2}, // Thunderus
	{645,  2}, // Landorus
	{646,  3}, // Kyurem
	{647,  2}, // Keldeo
};

struct Text {
	int x;
	int y;
	char text[32];
} textStatsR1[] {
	{-20, 20}, {-20, 36}, {-20, 52}, {-20, 68}, {-20, 84}, {-20, 100},
}, textStatsR2[] {
	{20, 20}, {20, 36}, {20, 52}, {20, 68}, {20, 84}, {20, 100},
}, textStatsR3[] {
	{60, 20}, {60, 36}, {60, 52}, {60, 68}, {60, 84}, {60, 100},
}, textStatsR4[] {
	{100, 20}, {100, 36}, {100, 52}, {100, 68}, {100, 84}, {100, 100},
};

int pkmLang(void) {
	switch(Config::lang) {
		case 0:
			return 5; // German
		case 1:
		default:
			return 2; // English
		case 2:
			return 6; // Spanish
		case 3:
			return 3; // French
		case 4:
			return 4; // Italian
		case 5:
			return 1; // Japanese
		case 6:
			return 2; // Russian (returns English)
	}
}

std::string aMenuText(int buttonMode, int i) {
	if(buttonMode == 0)	return Lang::aMenuText[i];
	else if(buttonMode == 1)	return Lang::aMenuTopBarText[i];
	else return Lang::aMenuEmptySlotText[i];
}

void drawAMenuButtons(std::vector<TextPos>& buttons, int buttonMode) {
	for(unsigned i=0;i<buttons.size();i++) {
		drawImage(buttons[i].x, buttons[i].y, boxButtonData.width, boxButtonData.height, boxButton, false);
		printTextMaxW(aMenuText(buttonMode, i), 80, 1, buttons[i].x+4, buttons[i].y+4, false);
	}
}

int aMenu(int pkmPos, std::vector<TextPos>& buttons, int buttonMode) {
	setSpritePosition(bottomArrowID, buttons[0].x+getTextWidthMaxW(aMenuText(buttonMode, 0), 80)+4, buttons[0].y);
	setSpriteVisibility(topArrowID, false);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();

	drawAMenuButtons(buttons, buttonMode);

	bool optionSelected = false;
	int held, pressed, menuSelection = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);
		
		if(held & KEY_UP) {
			if(menuSelection > 0)	menuSelection--;
		} else if(held & KEY_DOWN) {
			if(menuSelection < (int)buttons.size()-1)	menuSelection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);

			for(unsigned i=0; i<buttons.size();i++) {
				if(touch.px >= buttons[i].x && touch.px <= buttons[i].x+64 && touch.py >= buttons[i].y && touch.py <= buttons[i].y+25) {
					menuSelection = i;
					optionSelected = true;
				}
			}
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			goto back;
		}

		if(optionSelected) Sound::play(Sound::click);
		if(optionSelected && buttonMode == 0) { // A Pokémon
			optionSelected = false;
			if(menuSelection == 0) { // Move
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				if(sdFound())	drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				else drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
				return 1;
			} else if(menuSelection == 1) { // Edit
				int species = currentPokemon(pkmPos)->species();
				int form = currentPokemon(pkmPos)->alternativeForm();
				if(topScreen)	Banks::bank->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentSaveBox, pkmPos, false);
				
				// Redraw screen
				if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
				bool updateBox = (species == currentPokemon(pkmPos)->species() && form == currentPokemon(pkmPos)->alternativeForm());
				drawBox(false, !updateBox);
				if(topScreen)	drawBox(topScreen, !updateBox);
				drawPokemonInfo(currentPokemon(pkmPos));
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 2) { // Copy
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				if(sdFound())	drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				else drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
				return 2;
			} else if(menuSelection == 3) { // Release
				// Hide sprites below getBool message
				for(int i=7;i<22;i++)
					if(i%6)	setSpriteVisibility(i, false);
				updateOam();
				if(Input::getBool(Lang::release, Lang::cancel)) {
					if(topScreen)	Banks::bank->pkm(save->emptyPkm(), currentBankBox, pkmPos);
					else	save->pkm(save->emptyPkm(), currentSaveBox, pkmPos, false);
					drawBox(false);
					if(topScreen)	drawBox(topScreen);
					if(sdFound())	drawImageFromSheet(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, boxBgBottom, boxBgBottomData.width, 5+bankBoxData.width, 0, false);
					else drawRectangle(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, DARK_GRAY, false);
					drawPokemonInfo(save->emptyPkm());
					goto back;
				}
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				if(sdFound())	drawImageFromSheet(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, boxBgBottom, boxBgBottomData.width, 5+bankBoxData.width, 0, false);
				else	drawRectangle(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, DARK_GRAY, false);
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 4) { // Dump
				char path[256];
				if(currentPokemon(pkmPos)->alternativeForm())
					snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%i-%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", currentPokemon(pkmPos)->species(), currentPokemon(pkmPos)->alternativeForm(), currentPokemon(pkmPos)->nickname().c_str(), currentPokemon(pkmPos)->checksum(), currentPokemon(pkmPos)->encryptionConstant(), currentPokemon(pkmPos)->genNumber());
				else
					snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", currentPokemon(pkmPos)->species(), currentPokemon(pkmPos)->nickname().c_str(), currentPokemon(pkmPos)->checksum(), currentPokemon(pkmPos)->encryptionConstant(), currentPokemon(pkmPos)->genNumber());
				std::ofstream out(path);
				if(out.good())	out.write((char*)currentPokemon(pkmPos)->rawData(), 136);
				out.close();
			} else if(menuSelection == 5) { // Back
				back:
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				if(sdFound())	drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				else	drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
				break;
			}
		} else if(optionSelected && buttonMode == 1) { // Top bar
			optionSelected = false;
			if(menuSelection == 0) { // Jump
				// Clear buttons
				if(sdFound())	drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				else	drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
				
				// Select a box
				int num = selectBox(topScreen ? currentBankBox : currentSaveBox);
				
				// Clear mini boxes
				if(sdFound())	drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				else	drawRectangle(170, 0, 86, 192, DARK_GRAY, false);

				if(num == -1 || num == (topScreen ? currentBankBox : currentSaveBox)) { // If B was pressed or the box wasn't changed
					drawAMenuButtons(buttons, buttonMode);
				} else { // If a new box was selected
					(topScreen ? currentBankBox : currentSaveBox) = num;
					drawBox(topScreen, true);
					break;
				}
			} else if(menuSelection == 1) { // Rename
				// Hide bottom screen sprites
				for(int i=0;i<30;i++) {
					setSpriteVisibility(i, false);
				}
				updateOam();
				std::string newName = Input::getLine(topScreen ? 16 : 8);
				if(newName != "") {
					if(topScreen)	Banks::bank->boxName(newName, currentBankBox);
					else	save->boxName(currentSaveBox, newName);
				}

				// Redraw screen
				if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 2) { // Swap
				std::vector<std::shared_ptr<PKX>> tempBox;
				// Copy save Pokémon to a buffer
				for(int i=0;i<30;i++) {
					if(save->pkm(currentSaveBox, i)->species() != 0)
						tempBox.push_back(save->pkm(currentSaveBox, i));
					else
						tempBox.push_back(save->emptyPkm());
				}

				// Copy bank Pokémon to the save and add it to the Pokédex
				for(int i=0;i<30;i++) {
					if(Banks::bank->pkm(currentBankBox, i)->species() != 0) {
						save->pkm(Banks::bank->pkm(currentBankBox, i), currentSaveBox, i, false);
						save->dex(Banks::bank->pkm(currentBankBox, i));
					} else {
						save->pkm(save->emptyPkm(), currentSaveBox, i, false);
					}
				}

				// Copy the save Pokémon from their buffer to the bank
				for(int i=0;i<30;i++)
					Banks::bank->pkm(tempBox[i], currentBankBox, i);

				// Update the boxes
				drawBox(true, true);
				drawBox(false, true);
			} else if(menuSelection == 3) { // Dump box
				char path[PATH_MAX];
				snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str());
				mkdir(path, 0777);

				for(int i=0;i<30;i++) {
					if(currentPokemon(i)->species() != 0) {
						if(currentPokemon(i)->alternativeForm())
							snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s/%i-%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str(), currentPokemon(i)->species(), currentPokemon(i)->alternativeForm(), currentPokemon(i)->nickname().c_str(), currentPokemon(i)->checksum(), currentPokemon(i)->encryptionConstant(), currentPokemon(i)->genNumber());
						else
							snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s/%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str(), currentPokemon(i)->species(), currentPokemon(i)->nickname().c_str(), currentPokemon(i)->checksum(), currentPokemon(i)->encryptionConstant(), currentPokemon(i)->genNumber());
						std::ofstream out(path);
						if(out.good())	out.write((char*)currentPokemon(i)->rawData(), 136);
						out.close();
					}
				}
			} else if(menuSelection == 4) { // Back
				goto back;
			}
		} else if(optionSelected && buttonMode == 2) { // Empty slot
			optionSelected = false;
			if(menuSelection == 0) { // Inject
				// Hide sprites
				for(int i=0;i<30;i++) {
					setSpriteVisibility(i, false);
				}
				setSpriteVisibility(bottomArrowID, false);
				updateOam();

				// Save path and chane to /_nds/pkmn-chest/in
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);
				chdir(sdFound() ? "sd:/_nds/pkmn-chest/in" : "fat:/_nds/pkmn-chest/in");

				// Get a pk4/5
				std::vector<std::string> extList = {"pk4", "pk5"};
				std::string fileName = browseForFile(extList, false);
				
				// If the fileName isn't blank, inject the Pokémon
				if(fileName != "") {
					std::ifstream in(fileName);
					u8* buffer = 0;
					in.read((char*)buffer, 136);
					if(topScreen)	Banks::bank->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentBankBox, pkmPos);
					else	save->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos, false);
				}
				
				// Reset & redraw screen
				chdir(path);
				if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
				drawBox(false, !topScreen);
				if(topScreen)	drawBox(topScreen, topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));

				if(!topScreen)	setSpriteVisibility(bottomArrowID, true);
				updateOam();
				goto back;
			} else if(menuSelection == 1) { // Create
				std::shared_ptr<PKX> pkm = save->emptyPkm()->clone();
				pkm->TID(save->TID());
				pkm->SID(save->SID());
				pkm->otName(save->otName());
				pkm->otGender(save->gender());
				pkm->ball(4);
				pkm->encryptionConstant((((u32)randomNumbers()) % 0xFFFFFFFF) + 1);
				pkm->version(save->version());
				switch (pkm->version()) {
					case 7:
					case 8:
						pkm->metLocation(0x0095); // Route 1 (HGSS)
						break;
					case 10:
					case 11:
					case 12:
						pkm->metLocation(0x0010); // Route 201 (DPPt)
						break;
					case 20:
					case 21:
					case 22:
					case 23:
						pkm->metLocation(0x000e); // Route 1 (BWB2W2)
						break;
				}
				pkm->fixMoves();
				pkm->PID(PKX::getRandomPID(pkm->species(), pkm->gender(), pkm->version(), pkm->nature(), pkm->alternativeForm(), pkm->abilityNumber(), pkm->PID(), pkm->generation()));
				pkm->language(pkmLang());
				const time_t current = time(NULL);
				pkm->metDay(gmtime(&current)->tm_mday);
				pkm->metMonth(gmtime(&current)->tm_mon);
				pkm->metYear(gmtime(&current)->tm_year - 2000);
				pkm->metLevel(1);

				if(topScreen)	Banks::bank->pkm(showPokemonSummary(pkm), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(pkm), currentSaveBox, pkmPos, false);
				
				// Redraw screen
				if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
				drawBox(false, !topScreen);
				if(topScreen)	drawBox(topScreen, topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));
				goto back;
			} else if(menuSelection == 2) {
				goto back;
			}
		}

		setSpritePosition(bottomArrowID, buttons[menuSelection].x+getTextWidthMaxW(aMenuText(buttonMode, menuSelection), 80)+4, buttons[menuSelection].y);
		updateOam();
	}
	return false;
}

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
		drawImage(xMenuButtons[i].x, xMenuButtons[i].y, menuButtonData.width, menuButtonData.height, menuSelection == i ? menuButtonBlue : menuButton, false);
		printText(Lang::xMenuText[i], xMenuButtons[i].x+47, xMenuButtons[i].y+14, false);
		oamSetAlpha(&oamSub, menuIconID[i], menuSelection == i ? 8 : 15);
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
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Enable sprites and set positions
	for(unsigned i=0;i<menuIconID.size();i++) {
		setSpritePosition(menuIconID[i], xMenuButtons[i].x+3, xMenuButtons[i].y+6);
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
			if(iconDirection) {
				if(iconOffset < 6)	iconOffset++;
				else if(iconOffset < 12)	iconOffset++;
				else	iconDirection = false;
			} else {
				if(iconOffset > -6)	iconOffset--;
				else	iconDirection = true;
			}
			if(iconOffset < 7) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6-(iconOffset/3));
				updateOam();
			}
		} while(!pressed);

		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
			iconOffset = 0;
			iconDirection = true;

			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection -= 2;
			}
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < (int)xMenuButtons.size()-2) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection += 2;
			}
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection--;
			}
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2)) {
				setSpritePosition(menuIconID[menuSelection], xMenuButtons[menuSelection].x+3, xMenuButtons[menuSelection].y+6);
				iconOffset = 0;
				iconDirection = true;

				menuSelection++;
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0; i<xMenuButtons.size();i++) {
				if(touch.px >= xMenuButtons[i].x && touch.px <= xMenuButtons[i].x+menuButtonData.width && touch.py >= xMenuButtons[i].y && touch.py <= xMenuButtons[i].y+menuButtonData.height) {
					selectedOption = i;
				}
			}
			menuSelection = -1;
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
			if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
			else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
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
			else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
			for(unsigned i=0;i<menuIconID.size();i++) {
				setSpritePosition(menuIconID[i], xMenuButtons[i].x+3, xMenuButtons[i].y+6);
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

void drawMiniBoxes(int currentBox) {
	if(currentBox < 0)	currentBox = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1)+currentBox;
	// Clear text
	if(sdFound())	drawImageFromSheet(210, 0, 46, 192, boxBgBottom, boxBgBottomData.width, 210, 0, false);
	else	drawRectangle(210, 0, 46, 192, DARK_GRAY, false);

	for(int i=0;i<5;i++) {
		drawRectangle(170, 10+(i*33), 35, 30, WHITE, false);
		drawOutline(170, 10+(i*33), 35, 30, DARK_GRAY, false);
		for(int j=0;j<30;j++) {
			if((topScreen ? Banks::bank->pkm(currentBox, j)->species() : save->pkm(currentBox, j)->species()) != 0) {
				// Type 1
				int type = topScreen ? Banks::bank->pkm(currentBox, j)->type1() : save->pkm(currentBox, j)->type1();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(173+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*384)+34], false);

				// Type 2
				type = topScreen ? Banks::bank->pkm(currentBox, j)->type2() : save->pkm(currentBox, j)->type2();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(175+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*384)+33], false);
			}
		}
		// Print box number
		printText(std::to_string(currentBox+1), 210, 20+(i*33), false);
		if(currentBox < (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1))	currentBox++;
		else	currentBox = 0;
	}
}

int selectBox(int currentBox) {
	setSpritePosition(bottomArrowID, 205, 14);
	updateOam();
	drawMiniBoxes(currentBox);

	int pressed, held, screenPos = currentBox;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(currentBox > 0)	currentBox--;
			else {
				screenPos = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1)-(currentBox-screenPos-1);
				currentBox = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1);
			}
		} else if(held & KEY_DOWN) {
			if(currentBox < (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1))	currentBox++;
			else {
				screenPos = 0-(currentBox-screenPos+1);
				currentBox = 0;
			}
		} else if(pressed & KEY_A) {
			return currentBox;
		} else if(pressed & KEY_B) {
			return -1;
		}

		// Scroll screen if needed
		if(currentBox < screenPos) {
			screenPos = currentBox;
			drawMiniBoxes(screenPos);
		} else if(currentBox > screenPos+4) {
			screenPos = currentBox-4;
			drawMiniBoxes(screenPos);
		}

		// Move cursor
		setSpritePosition(bottomArrowID, 205, 14+(33*(currentBox-screenPos)));
		updateOam();
	}
}

int selectForm(int dexNo, int currentForm) {
	int altIndex = -1;
	for(unsigned i=0;i<(sizeof(formCounts)/sizeof(formCounts[0]));i++) {
		if(formCounts[i].dexNo == dexNo) {
			altIndex = i;
			break;
		}
	}
	if(altIndex == -1)	return -1; // No alternate forms
	else if(altIndex == 0) { // Unown
		int num = Input::getLine(1)[0];

		if(num == 33)	return 26; // !
		else if(num == 63)	return 27; // ?
		else if(num > 96 && num < 123)	return num-97; // a-z
		else	return -1;
	}

	// Draw background
	drawRectangle(0, 60, 256, 72, DARK_GRAY, false);
	drawOutline(0, 60, 256, 72, LIGHT_GRAY, false);

	// Draw forms
	for(int i=0;i<formCounts[altIndex].noForms;i++) {
		if(sdFound()) {
			std::pair<int, int> xy = getPokemonPosition(dexNo, i);
			drawImageFromSheet((i*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
		} else {
			std::vector<u16> bmp;
			loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(dexNo, i))+".bmp", bmp);
			drawImage((i*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, bmp, false);
		}
	}

	// Move arrow to current form
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (currentForm*32)+(128-((32*formCounts[altIndex].noForms)/2))+28, 84);
	updateOam();

	int pressed, held;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_LEFT) {
			if(currentForm > 0)	currentForm--;
			else	currentForm=formCounts[altIndex].noForms-1;
		} else if(held & KEY_RIGHT) {
			if(currentForm < formCounts[altIndex].noForms-1)	currentForm++;
			else currentForm=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return currentForm;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int i=0;i<5;i++) {
				if(touch.px > (i*32)+(128-((32*formCounts[altIndex].noForms)/2)) && touch.px < (i*32)+(128-((32*formCounts[altIndex].noForms)/2))+32 && touch.py > 72 && touch.py < 104) {
					Sound::play(Sound::click);
					return i;
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (currentForm*32)+(128-((32*formCounts[altIndex].noForms)/2))+28, 84);
		updateOam();
	}
}

void drawMoveList(int screenPos, std::vector<std::string> moveList) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw search icon
	drawImage(256-searchData.width, 0, searchData.width, searchData.height, search, false);

	// Print moves
	for(unsigned i=0;i<std::min(9u, moveList.size());i++) {
		printText(moveList[screenPos+i], 4, 4+(i*20), false);
	}
}

int selectMove(int currentMove) {
	// Set arrow position
	setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[currentMove]), -2);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();

	// Print moves
	std::vector<std::string> moveList = Lang::moves;
	drawMoveList(currentMove, moveList);

	int held, pressed, screenPos = currentMove, newMove = currentMove, entriesPerScreen = 9;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(newMove > 0)	newMove--;
			else	newMove = std::min(save->maxMove(), (int)moveList.size()-1);
		} else if(held & KEY_DOWN) {
			if(newMove < std::min(save->maxMove(), (int)moveList.size()-1))	newMove++;
			else newMove = 0;
		} else if(held & KEY_LEFT) {
			newMove -= entriesPerScreen;
			if(newMove < 0)	newMove = 0;
		} else if(held & KEY_RIGHT) {
			newMove += entriesPerScreen;
			if(newMove > std::min(save->maxMove(), (int)moveList.size()-1))	newMove = std::min(save->maxMove(), (int)moveList.size()-1);
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			for(int i=0;i<save->maxMove();i++) {
				if(moveList[newMove] == Lang::moves[i]) {
					return i;
				}
			}
		} if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return currentMove;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<entriesPerScreen;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(moveList[screenPos+i]) && touch.py >= 4+(i*20) && touch.py <= 4+((i+1)*20)) {
					return screenPos+i;
					break;
				} else if(touch.px >= 256-searchData.width && touch.py <= searchData.height) {
					goto search;
				}
			}
		} else if(pressed & KEY_Y) {
			search:
			std::string str = Input::getLine();
			if(str != "") {
				moveList.clear();
				moveList.push_back("-----");
				for(int i=0;i<save->maxMove();i++) {
					if(strncasecmp(str.c_str(), Lang::moves[i].c_str(), str.length()) == 0) {
						moveList.push_back(Lang::moves[i]);
					}
				}
				newMove = 0;
				screenPos = 0;
			}
			drawMoveList(screenPos, moveList);
		}

		// Scroll screen if needed
		if(newMove < screenPos) {
			screenPos = newMove;
			drawMoveList(screenPos, moveList);
		} else if(newMove > screenPos + entriesPerScreen - 1) {
			screenPos = newMove - entriesPerScreen + 1;
			drawMoveList(screenPos, moveList);
		}

		// Move cursor
		setSpritePosition(bottomArrowID, 4+getTextWidth(moveList[newMove]), (20*(newMove-screenPos)-2));
		updateOam();
	}
}

std::shared_ptr<PKX> selectMoves(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Print moves
	for(int i=0;i<4;i++) {
		printText(Lang::moves[pkm->move(i)], 4, 4+(i*20), false);
	}

	// Set arrow position
	setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[pkm->move(0)]), -2);
	setSpriteVisibility(bottomArrowID, true);
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
			if(selection < 3)	selection++;
		} else if(held & KEY_LEFT) {
			selection = 0;
		} else if(held & KEY_RIGHT) {
			selection = 3;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<4;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(Lang::moves[pkm->move(i)]) && touch.py >= 4+(i*20) && touch.py <= 4+((i+1)*20)) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			int num = selectMove(pkm->move(selection));
			if(num != -1)	pkm->move(selection, num);

			// Clear the screen
			if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
			else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

			// Print moves
			for(int i=0;i<4;i++) {
				printText(Lang::moves[pkm->move(i)], 4, 4+(i*20), false);
			}
		}

		setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[pkm->move(selection)]), (selection*20)-2);
		updateOam();
	}
}

int selectNature(int currentNature) {
	// Clear screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw labels (not a for loop as speed is 3rd)
	{
		int x = -2;
		printTextCenteredTintedMaxW(Lang::statsLabels[1], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[2], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[5], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[3], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[4], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		
		int y = 0;
		printTextTintedScaled(Lang::statsLabels[1], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[2], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[5], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[3], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[4], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
	}

	// Print natures
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			printTextCenteredMaxW(Lang::natures[(y*5)+x], 48, 1, ((x-2)*48), (y*32)+32, false);
		}
	}

	int arrowX = currentNature-((currentNature/5)*5), arrowY = currentNature/5, pressed, held;
	// Move arrow to current nature
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[currentNature], 48)/2)+28, (arrowY*32)+24);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(arrowY > 0)	arrowY--;
			else	arrowY=4;
		} else if(held & KEY_DOWN) {
			if(arrowY < 4)	arrowY++;
			else	arrowY=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return (arrowY*5)+arrowX;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y=0;y<5;y++) {
				for(int x=0;x<5;x++) {
					if(touch.px > (x*48)+8 && touch.px < (x*48)+56 && touch.py > (y*32)+8 && touch.py < (y*32)+56) {
						Sound::play(Sound::click);
						return (y*5)+x;
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[(arrowY*5)+arrowX], 48)/2)+28, (arrowY*32)+24);
		updateOam();
	}
}

int selectPokeball(int currentBall) {
	// Clear screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw Pokéballs
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
				std::pair<int, int> xy = getPokeballPosition((y*5)+x+1);
				drawImageFromSheet((x*48)+24, (y*32)+24, 15, 15, ballSheet, ballSheetData.width, xy.first, xy.second, false);
			}
		}
	}

	currentBall--;
	int arrowX = currentBall-((currentBall/5)*5), arrowY = currentBall/5, pressed, held;
	// Move arrow to current ball
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+40, (arrowY*32)+16);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(arrowY > 0)	arrowY--;
			else	arrowY=4;
		} else if(held & KEY_DOWN) {
			if(arrowY < 4)	arrowY++;
			else	arrowY=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			if(!(save->generation() != Generation::FIVE && (arrowY*5)+arrowX == 24)) {
				Sound::play(Sound::click);
				return (arrowY*5)+arrowX+1;
			}
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y=0;y<5;y++) {
				for(int x=0;x<5;x++) {
					if(touch.px > (x*48)+8 && touch.px < (x*48)+56 && touch.py > (y*32)+8 && touch.py < (y*32)+56) {
						if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
							Sound::play(Sound::click);
							return (y*5)+x+1;
						}
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*48)+40, (arrowY*32)+16);
		updateOam();
	}
}

void drawStatsPage(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw lines
	for(unsigned i=1;i<(sizeof(textStatsR1)/sizeof(textStatsR1[0]));i++) {
		drawRectangle(16, textStatsR1[i].y, 230, 1, LIGHT_GRAY, false);
	}
	drawRectangle(128, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(168, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(208, 4, 1, 112, LIGHT_GRAY, false);

	// Print stat info labels
	int i = pkm->nature();
	printText(Lang::statsLabels[0], 20, textStatsR1[0].y, false);
	printTextTintedMaxW(Lang::statsLabels[1], 80, 1, (i!=0&&i<5         ? RGB::RED : i!=0&&!(i%5)      ? RGB::BLUE : WHITE), 20, textStatsR1[1].y, false);
	printTextTintedMaxW(Lang::statsLabels[2], 80, 1, (i!=6&&i>4&&i<10   ? RGB::RED : i!=6&&!((i-1)%5)  ? RGB::BLUE : WHITE), 20, textStatsR1[2].y, false);
	printTextTintedMaxW(Lang::statsLabels[3], 80, 1, (i!=18&&i>14&&i<20 ? RGB::RED : i!=18&&!((i-3)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[3].y, false);
	printTextTintedMaxW(Lang::statsLabels[4], 80, 1, (i!=24&&i>19       ? RGB::RED : i!=24&&!((i-4)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[4].y, false);
	printTextTintedMaxW(Lang::statsLabels[5], 80, 1, (i!=12&&i>9&&i<15  ? RGB::RED : i!=12&&!((i-2)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[5].y, false);

	// Print column titles
	printTextCenteredMaxW(Lang::statsLabels[6], 30, 1, textStatsR1[0].x, textStatsR1[0].y-16, false);
	printTextCentered(Lang::statsLabels[7], textStatsR2[0].x, textStatsR2[0].y-16, false);
	printTextCentered(Lang::statsLabels[8], textStatsR3[0].x, textStatsR3[0].y-16, false);
	printTextCenteredMaxW(Lang::statsLabels[9], 30, 1, textStatsR4[0].x, textStatsR4[0].y-16, false);

	// Set base stat info
	snprintf(textStatsR1[0].text,  sizeof(textStatsR1[0].text), "%i", pkm->baseHP());
	snprintf(textStatsR1[1].text,  sizeof(textStatsR1[1].text), "%i", pkm->baseAtk());
	snprintf(textStatsR1[2].text,  sizeof(textStatsR1[2].text), "%i", pkm->baseDef());
	snprintf(textStatsR1[3].text,  sizeof(textStatsR1[3].text), "%i", pkm->baseSpa());
	snprintf(textStatsR1[4].text,  sizeof(textStatsR1[4].text), "%i", pkm->baseSpd());
	snprintf(textStatsR1[5].text,  sizeof(textStatsR1[5].text), "%i", pkm->baseSpe());

	// Set & print other stat info and 
	for(unsigned i=0;i<(sizeof(textStatsR1)/sizeof(textStatsR1[0]));i++) {
		snprintf(textStatsR2[i].text,  sizeof(textStatsR2[i].text), "%i", pkm->iv(i));
		snprintf(textStatsR3[i].text,  sizeof(textStatsR3[i].text), "%i", pkm->ev(i));
		snprintf(textStatsR4[i].text,  sizeof(textStatsR4[i].text), "%i", pkm->stat(i));

		printTextCentered(textStatsR1[i].text, textStatsR1[i].x, textStatsR1[i].y, false);
		printTextCentered(textStatsR2[i].text, textStatsR2[i].x, textStatsR2[i].y, false);
		printTextCentered(textStatsR3[i].text, textStatsR3[i].x, textStatsR3[i].y, false);
		printTextCentered(textStatsR4[i].text, textStatsR4[i].x, textStatsR4[i].y, false);
	}
}

std::shared_ptr<PKX> selectStats(std::shared_ptr<PKX> pkm) {
	setSpritePosition(bottomArrowID, 128+(textStatsR2[0].x+(getTextWidth(textStatsR2[0].text)/2)), textStatsR2[0].y-6);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();
	drawStatsPage(pkm);

	bool optionSelected = false;
	int held, pressed, selection = 0, column = 0;
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
			if(selection < 5)	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)	column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textStatsR2)/sizeof(textStatsR2[0]));i++) {
				if(touch.px >= 128+(textStatsR2[selection].x-(getTextWidth(textStatsR2[selection].text)/2)) && touch.px <= 128+(textStatsR2[selection].x+(getTextWidth(textStatsR2[selection].text)/2)) && touch.py >= textStatsR2[i].y && touch.py <= textStatsR2[i].y+16) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i=0;i<(sizeof(textStatsR3)/sizeof(textStatsR3[0]));i++) {
				if(touch.px >= 128+(textStatsR3[selection].x-(getTextWidth(textStatsR3[selection].text)/2)) && touch.px <= 128+(textStatsR3[selection].x+(getTextWidth(textStatsR3[selection].text)/2)) && touch.py >= textStatsR3[i].y && touch.py <= textStatsR3[i].y+16) {
					column = 1;
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = 0;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			if(column == 0) {
				int num = Input::getInt(31);
				if(num != -1)	pkm->iv(selection, num);
			} else {
				int total = 0;
				for(int i=0;i<6;i++) {
					if(i != selection)	total += pkm->ev(i);
				}
				int num = Input::getInt(std::min(510-total, 255));
				if(num != -1)	pkm->ev(selection, num);
			}
			setSpriteVisibility(bottomArrowID, true);
			updateOam();
			drawStatsPage(pkm);
		}

		if(column == 0) {
			setSpritePosition(bottomArrowID, 128+(textStatsR2[selection].x+(getTextWidth(textStatsR2[selection].text)/2)), textStatsR2[selection].y-6);
		} else {
			setSpritePosition(bottomArrowID, 128+(textStatsR3[selection].x+(getTextWidth(textStatsR3[selection].text)/2)), textStatsR3[selection].y-6);
		}
		updateOam();
	}
}
