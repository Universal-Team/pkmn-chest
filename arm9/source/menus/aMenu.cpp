#include "aMenu.h"
#include <dirent.h>
#include <fstream>

#include "banks.hpp"
#include "colors.h"
#include "fileBrowse.h"
#include "flashcard.h"
#include "graphics.h"
#include "input.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "misc.h"
#include "summary.h"
#include "sound.h"


std::vector<std::pair<int, int>> aMenuButtons = {
	{170,  16}, // Move
	{170,  42}, // Edit
	{170,  68}, // Copy
	{170,  94}, // Release
	{170, 120}, // Dump
	{170, 146}, // Back
};
std::vector<std::pair<int, int>> aMenuEmptySlotButtons = {
	{170, 16}, // Inject
	{170, 42}, // Create
	{170, 68}, // Back
};
std::vector<std::pair<int, int>> aMenuTopBarButtons = {
	{170, 16}, // Jump
	{170, 42}, // Rename
	{170, 68}, // Swap
	{170, 94}, // Dump box
	{170, 120}, // Back
};

std::string aMenuText(int buttonMode, int i) {
	if(buttonMode == 0)	return Lang::aMenuText[i];
	else if(buttonMode == 1)	return Lang::aMenuTopBarText[i];
	else return Lang::aMenuEmptySlotText[i];
}

void drawAMenuButtons(std::vector<std::pair<int, int>>& buttons, int buttonMode) {
	for(unsigned i=0;i<buttons.size();i++) {
		drawImage(buttons[i].first, buttons[i].second, boxButtonData.width, boxButtonData.height, boxButton, false);
		printTextMaxW(aMenuText(buttonMode, i), 80, 1, buttons[i].first+4, buttons[i].second+4, false);
	}
}

int aMenu(int pkmPos, std::vector<std::pair<int, int>>& buttons, int buttonMode) {
	setSpritePosition(bottomArrowID, buttons[0].first+getTextWidthMaxW(aMenuText(buttonMode, 0), 80)+4, buttons[0].second);
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
				if(touch.px >= buttons[i].first && touch.px <= buttons[i].first+64 && touch.py >= buttons[i].second && touch.py <= buttons[i].second+25) {
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

		setSpritePosition(bottomArrowID, buttons[menuSelection].first+getTextWidthMaxW(aMenuText(buttonMode, menuSelection), 80)+4, buttons[menuSelection].second);
		updateOam();
	}
	return false;
}
