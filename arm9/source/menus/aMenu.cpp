#include "aMenu.hpp"
#include <dirent.h>

#include "banks.hpp"
#include "colors.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "summary.hpp"
#include "sound.hpp"

std::vector<Label> aMenuButtons = {
	{170,  16,    "move"}, // Move
	{170,  42,    "edit"}, // Edit
	{170,  68,    "copy"}, // Copy
	{170,  94, "release"}, // Release
	{170, 120,    "dump"}, // Dump
	{170, 146,    "back"}, // Back
};
std::vector<Label> aMenuEmptySlotButtons = {
	{170, 16, "inject"}, // Inject
	{170, 42, "create"}, // Create
	{170, 68,   "back"}, // Back
};
std::vector<Label> aMenuTopBarButtons = {
	{170,  16,      "jump"}, // Jump
	{170,  42,    "rename"}, // Rename
	{170,  68,      "swap"}, // Swap
	{170,  94, "wallpaper"}, // Wallpaper
	{170, 120,   "dumpBox"}, // Dump box
	{170, 146,      "back"}, // Back
};

void drawAMenuButtons(std::vector<Label>& buttons, int buttonMode) {
	for(unsigned i=0;i<buttons.size();i++) {
		drawImage(buttons[i].x, buttons[i].y, boxButton, false, true);
		printTextMaxW(Lang::get(buttons[i].label), 80, 1, buttons[i].x+4, buttons[i].y+4, false);
	}
}

int aMenu(int pkmPos, std::vector<Label>& buttons, int buttonMode) {
	setSpritePosition(arrowID, false, buttons[0].x+getTextWidthMaxW(Lang::get(buttons[0].label), 80)+4, buttons[0].y);
	setSpriteVisibility(arrowID, true, false);
	setSpriteVisibility(arrowID, false, true);
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
					setSpriteVisibility(arrowID, false, false);
					setSpriteVisibility(arrowID, true, true);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);
				return 1;
			} else if(menuSelection == 1) { // Edit
				if(topScreen)	Banks::bank->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentSaveBox, pkmPos, false);

				// Redraw screen
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 2) { // Copy
				if(topScreen) {
					setSpriteVisibility(arrowID, false, false);
					setSpriteVisibility(arrowID, true, true);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);
				return 2;
			} else if(menuSelection == 3) { // Release
				if(Input::getBool(Lang::get("release"), Lang::get("cancel"))) {
					if(topScreen)	Banks::bank->pkm(save->emptyPkm(), currentBankBox, pkmPos);
					else	save->pkm(save->emptyPkm(), currentSaveBox, pkmPos, false);
					drawPokemonInfo(save->emptyPkm());
					drawBox(topScreen);
					goto back;
				}
				drawAMenuButtons(buttons, buttonMode);
			} else if(menuSelection == 4) { // Dump
				char path[256];
				if(currentPokemon(pkmPos)->alternativeForm())
					snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%i-%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", currentPokemon(pkmPos)->species(), currentPokemon(pkmPos)->alternativeForm(), currentPokemon(pkmPos)->nickname().c_str(), currentPokemon(pkmPos)->checksum(), currentPokemon(pkmPos)->encryptionConstant(), currentPokemon(pkmPos)->genNumber());
				else
					snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", currentPokemon(pkmPos)->species(), currentPokemon(pkmPos)->nickname().c_str(), currentPokemon(pkmPos)->checksum(), currentPokemon(pkmPos)->encryptionConstant(), currentPokemon(pkmPos)->genNumber());
				FILE* out = fopen(path, "wb");
				if(out) {
					fwrite(currentPokemon(pkmPos)->rawData(), 1, 136, out);
					fclose(out);
				}
			} else if(menuSelection == 5) { // Back
				back:
				if(topScreen) {
					setSpriteVisibility(arrowID, false, false);
					setSpriteVisibility(arrowID, true, true);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);
				break;
			}
		} else if(optionSelected && buttonMode == 1) { // Top bar
			optionSelected = false;
			if(menuSelection == 0) { // Jump
				// Clear buttons
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);

				// Select a box
				int num = selectBox(topScreen ? currentBankBox : currentSaveBox);

				// Clear mini boxes
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);

				if(num == -1 || num == (topScreen ? currentBankBox : currentSaveBox)) { // If B was pressed or the box wasn't changed
					drawAMenuButtons(buttons, buttonMode);
				} else { // If a new box was selected
					(topScreen ? currentBankBox : currentSaveBox) = num;
					drawBox(topScreen);
					goto back;
				}
			} else if(menuSelection == 1) { // Rename
				// Hide arrow sprite
				setSpriteVisibility(arrowID, false, false);
				updateOam();
				std::string newName = Input::getLine(topScreen ? 16 : 8);
				if(newName != "") {
					if(topScreen)	Banks::bank->boxName(newName, currentBankBox);
					else	save->boxName(currentSaveBox, newName);
				}

				// Redraw screen
				setSpriteVisibility(arrowID, false, true);
				updateOam();
				drawAMenuButtons(buttons, buttonMode);
				drawBox(topScreen);
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
				drawBox(true);
				drawBox(false);
			} else if(menuSelection == 3) { // Wallpaper
				if(!topScreen) {
					// Hide sprites
					for(int i=0;i<30;i++)	setSpriteVisibility(i, false, false);
					setSpriteVisibility(arrowID, false, false);
					updateOam();

					// Get new wallpaper
					int num = selectWallpaper(save->boxWallpaper(currentSaveBox));
					if(num != -1)	save->boxWallpaper(currentSaveBox, num);

					// Redraw screen
					setSpriteVisibility(arrowID, false, true);
					drawRectangle(170, 0, 86, 192, CLEAR, false, true);
					drawAMenuButtons(buttons, buttonMode);
					drawBox(false);
				}
			} else if(menuSelection == 4) { // Dump box
				char path[PATH_MAX];
				snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str());
				mkdir(path, 0777);

				for(int i=0;i<30;i++) {
					if(currentPokemon(i)->species() != 0) {
						if(currentPokemon(i)->alternativeForm())
							snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s/%i-%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str(), currentPokemon(i)->species(), currentPokemon(i)->alternativeForm(), currentPokemon(i)->nickname().c_str(), currentPokemon(i)->checksum(), currentPokemon(i)->encryptionConstant(), currentPokemon(i)->genNumber());
						else
							snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/out/%s/%i - %s - %x%lx.pk%i", sdFound() ? "sd" : "fat", topScreen ? Banks::bank->boxName(currentBankBox).c_str() : save->boxName(currentSaveBox).c_str(), currentPokemon(i)->species(), currentPokemon(i)->nickname().c_str(), currentPokemon(i)->checksum(), currentPokemon(i)->encryptionConstant(), currentPokemon(i)->genNumber());
						FILE* out = fopen(path, "wb");
						if(out) {
							fwrite(currentPokemon(i)->rawData(), 1, 136, out);
							fclose(out);
						}
					}
				}
			} else if(menuSelection == 5) { // Back
				goto back;
			}
		} else if(optionSelected && buttonMode == 2) { // Empty slot
			optionSelected = false;
			if(menuSelection == 0) { // Inject
				// Hide sprites
				for(int i=0;i<30;i++) {
					setSpriteVisibility(i, false, false);
				}
				setSpriteVisibility(arrowID, false, false);
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
					FILE* in = fopen(fileName.c_str(), "rb");
					if(in) {
						fseek(in, 0, SEEK_END);
						u8 buffer[136];
						fread(buffer, 1, sizeof(buffer), in);
						if(topScreen)	Banks::bank->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentBankBox, pkmPos);
						else	save->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos, false);
						fclose(in);
					}
				}

				// Reset & redraw screen
				chdir(path);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));

				if(!topScreen)	setSpriteVisibility(arrowID, false, true);
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
				pkm->metMonth(gmtime(&current)->tm_mon + 1);
				pkm->metYear(gmtime(&current)->tm_year - 100);
				pkm->metLevel(1);

				if(topScreen)	Banks::bank->pkm(showPokemonSummary(pkm), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(pkm), currentSaveBox, pkmPos, false);

				// Redraw screen
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
				drawPokemonInfo(currentPokemon(pkmPos));
				goto back;
			} else if(menuSelection == 2) {
				goto back;
			}
		}

		setSpritePosition(arrowID, false, buttons[menuSelection].x+getTextWidthMaxW(Lang::get(buttons[menuSelection].label), 80)+4, buttons[menuSelection].y);
		updateOam();
	}
	return false;
}
