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
	{170,  16}, // Edit
	{170,  42}, // Move
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
	{170, 16}, // Rename
	{170, 42}, // Swap
	{170, 68}, // Dump box
	{170, 94}, // Back
};
std::vector<TextPos> xMenuButtons = {
	{2,  24}, {130,  24},
	{2,  72}, {130,  72},
	{2, 120}, {130, 120},
};

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
	setSpritePosition(bottomArrowID, buttons[0].x+getTextWidth(aMenuText(buttonMode, 0))+4, buttons[0].y);
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
			if(menuSelection == 0) { // Edit
				edit:
				if(topScreen)	Banks::bank->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentSaveBox, pkmPos, false);
				
				// Redraw screen
				drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 1) { // Move
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				return 1;
			} else if(menuSelection == 2) { // Copy
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
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
					drawImageFromSheet(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, boxBgBottom, boxBgBottomData.width, 5+bankBoxData.width, 0, false);
					drawPokemonInfo(save->emptyPkm());
					goto back;
				}
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawImageFromSheet(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, boxBgBottom, boxBgBottomData.width, 5+bankBoxData.width, 0, false);
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
				drawImageFromSheet(170, 0, 86, 192, boxBgBottom, boxBgBottomData.width, 170, 0, false);
				break;
			}
		} else if(optionSelected && buttonMode == 1) { // Top bar
			optionSelected = false;
			if(menuSelection == 0) { // Rename
				// If the arrow is on the box title, rename it
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
				drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 1) { // Swap
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
				drawBox(true);
				drawBox(false);
			} else if(menuSelection == 2) { // Dump box
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
			} else if(menuSelection == 3) { // Back
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
					if(topScreen)	Banks::bank->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos);
					else	save->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos, false);
				}
				
				// Reset & redraw screen
				chdir(path);
				drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));

				if(!topScreen)	setSpriteVisibility(bottomArrowID, true);
				updateOam();
				goto back;
			} else if(menuSelection == 1) { // Create
				goto edit;
			} else if(menuSelection == 2) {
				goto back;
			}
		}

		setSpritePosition(bottomArrowID, buttons[menuSelection].x+getTextWidth(aMenuText(buttonMode, menuSelection))+4, buttons[menuSelection].y);
		updateOam();
	}
	return false;
}

void savePrompt(void) {
	// Draw background
	drawRectangle(0, 0, 256, 32, LIGHT_GRAY, false);
	drawRectangle(0, 32, 256, 144, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	printTextTinted(Lang::saveMsgChest, DARK_GRAY, 5, 0, false);
	if(Input::getBool(Lang::save, Lang::discard)) {
		if(Config::backupAmount != 0) Banks::bank->backup();
		Banks::bank->save();
	}

	drawRectangle(5, 33, 246, 16, DARK_GRAY, false);
	drawRectangle(0, 0, 256, 32, LIGHT_GRAY, false);
	if(savePath == cardSave)	printTextTinted(Lang::saveMsgCard, DARK_GRAY, 5, 0, false);
	else	printTextTinted(Lang::saveMsgSave, DARK_GRAY, 5, 0, false);

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
	drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);

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
			drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
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
			drawImage(0, 0, menuBgData.width, menuBgData.height, menuBg, false);
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

int selectForm(int dexNo) {
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
	std::shared_ptr<PKX> tempPkm = save->emptyPkm();
	tempPkm->species(dexNo);
	// for(int y=0;y<5;y++) {
		for(int x=0;x<formCounts[altIndex].noForms;x++) {
			tempPkm->alternativeForm(x);
			std::pair<int, int> xy = getPokemonPosition(tempPkm);
			drawImageFromSheet((x*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
		}
	// }

	// Move arrow to first form
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (128-((32*formCounts[altIndex].noForms)/2))+28, 84);
	updateOam();

	int arrowX = 0, pressed, held;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=formCounts[altIndex].noForms-1;
		} else if(held & KEY_RIGHT) {
			if(arrowX < formCounts[altIndex].noForms-1)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return arrowX;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int x=0;x<5;x++) {
				if(touch.px > (x*32)+(128-((32*formCounts[altIndex].noForms)/2)) && touch.px < (x*32)+(128-((32*formCounts[altIndex].noForms)/2))+32 && touch.py > 72 && touch.py < 104) {
					Sound::play(Sound::click);
					return x;
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*32)+(128-((32*formCounts[altIndex].noForms)/2))+28, 84);
		updateOam();
	}
}

int selectNature(void) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw labels (not a for loop as speed is 3rd)
	{
		int x = -2;
		printTextCenteredTintedMaxW(Lang::summaryP2Labels[1], 48, 1, BLUE_RGB, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::summaryP2Labels[2], 48, 1, BLUE_RGB, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::summaryP2Labels[5], 48, 1, BLUE_RGB, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::summaryP2Labels[3], 48, 1, BLUE_RGB, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::summaryP2Labels[4], 48, 1, BLUE_RGB, ((x++)*48), 4, false);
		
		int y = 0;
		printTextTintedScaled(Lang::summaryP2Labels[1], 0.8, 0.8, RED_RGB, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::summaryP2Labels[2], 0.8, 0.8, RED_RGB, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::summaryP2Labels[5], 0.8, 0.8, RED_RGB, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::summaryP2Labels[3], 0.8, 0.8, RED_RGB, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::summaryP2Labels[4], 0.8, 0.8, RED_RGB, 1, ((y++)*32)+22, false);
	}

	// Print natures
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			printTextCenteredMaxW(Lang::natures[(y*5)+x], 48, 1, ((x-2)*48), (y*32)+32, false);
		}
	}

	// Move arrow to first nature
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (getTextWidthMaxW(Lang::natures[0], 48)/2)+28, 24);
	updateOam();

	int arrowX = 0, arrowY = 0, pressed, held;
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

int selectPokeball(void) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw Pokéballs
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
				std::pair<int, int> xy = getPokeballPosition((y*5)+x+1);
				drawImageFromSheet((x*48)+24, (y*32)+24, 15, 15, ballSheet, ballSheetData.width, xy.first, xy.second, false);
			}
		}
	}

	// Move arrow to first ball
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, 40, 16);
	updateOam();

	int arrowX = 0, arrowY = 0, pressed, held;
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
