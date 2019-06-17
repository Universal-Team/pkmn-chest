#include <fat.h>

#include "common/banks.hpp"
#include "fileBrowse.h"
#include "graphics/graphics.h"
#include "keyboard.h"
#include "loader.h"
#include "manager.h"
#include "flashcard.h"
#include "nitrofs.h"

int main(int argc, char **argv) {
	initGraphics();
	keysSetRepeat(25,5);

	// Init filesystem
	if(!fatInitDefault()) {
		// Prints error if fatInitDefault() fails
		consoleDemoInit();
		printf("fatInitDefault() failed...");
		while(1) swiWaitForVBlank();
	}

	// Make directories
	mkdir(sdFound() ? "sd:/_nds" : "fat:/_nds", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest" : "fat:/_nds/pkmn-chest", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/backups" : "fat:/_nds/pkmn-chest/backups", 0777);

	// Try to init NitroFS from argv provided to the game when it was launched
	if(!nitroFSInit(argv[0])) {
		// If that fails, try to init NitroFS on 'pkmn-chest.nds'
		if(!nitroFSInit("pkmn-chest.nds")) {
			// Prints error if nitroFSInit() fails
			consoleDemoInit();
			printf("nitroFSInit() failed...");
			while(1) swiWaitForVBlank();
		}
	}
	loadFont();
	Banks::init();

	drawRectangle(0, 0, 256, 192, DARK_BLUE, true);
	drawRectangle(0, 0, 256, 192, DARK_BLUE, false);
	printTextCentered("Loading...", 32, 0, false);
	
	loadGraphics();

	std::vector<std::string> extensionList;
	extensionList.push_back(".sav");
	std::string savePath;
	while(1) {
		if(!loadSave(savePath = browseForFile(extensionList))) {
			drawRectangle(20, 20, 216, 152, 0xCC00, true);
			printText("Invalid save file", 25, 25, true);
			for(int i=0;i<120;i++)	swiWaitForVBlank();
			continue;
		}
		currentSaveBox = save->currentBox();
		currentBankBox = 0;
		save->cryptBoxData(true);

		drawBoxScreen();

		int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonBox = -1;
		bool heldPokemonScreen = false;
		topScreen = false;
		u16 hDown = 0;
		while(1) {
			do {
				swiWaitForVBlank();
				scanKeys();
				hDown = keysDownRepeat();
			} while(!hDown);

			if(hDown & KEY_UP) {
				if(arrowY > (topScreen ? -1 : -2))	arrowY--;
			} else if(hDown & KEY_DOWN) {
				if(arrowY < (topScreen ? 5 : 4))	arrowY++;
			}
			if(hDown & KEY_LEFT && arrowY != -1) {
				if(arrowX > 0)	arrowX--;
			} else if(hDown & KEY_RIGHT && arrowY != -1) {
				if(arrowX < 5)	arrowX++;
			}
			if(hDown & KEY_LEFT && arrowY == -1) {
				goto switchBoxLeft;
			} else if(hDown & KEY_RIGHT && arrowY == -1) {
				goto switchBoxRight;
			}
			if(hDown & KEY_L) {
				switchBoxLeft:
				if(currentBox() > 0)
					(topScreen ? currentBankBox : currentSaveBox)--;
				else if(topScreen) currentBankBox = Banks::bank->boxes()-1;
				else currentSaveBox = save->maxBoxes()-1;
				drawBox(topScreen);
				if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
					setSpriteVisibility(heldPokemon, false);
			} else if(hDown & KEY_R) {
				switchBoxRight:
				if((topScreen ? currentBankBox < Banks::bank->boxes()-1 : currentSaveBox < save->maxBoxes()-1))
					(topScreen ? currentBankBox : currentSaveBox)++;
				else (topScreen ? currentBankBox : currentSaveBox) = 0;
				drawBox(topScreen);
				if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
					setSpriteVisibility(heldPokemon, false);
			}
			if(hDown & KEY_A) {
				if(heldPokemon != -1) {
					if(heldPokemon == (arrowY*6)+arrowX && heldPokemonBox == currentBox()) {
						// If in the held Pokémon's previous spot, just put it back down
						setSpriteVisibility(heldPokemon, true);
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
						heldPokemon = -1;
						heldPokemonBox = -1;
					} else {
						// If the new spot has a Pokémon, swap it with the held one
						// Save the Pokémon at the cursor's postion to a temp variable
						std::shared_ptr<PKX> heldPkm = (heldPokemonScreen ? Banks::bank->pkm(heldPokemonBox, heldPokemon) : save->pkm(heldPokemonBox, heldPokemon));
						std::shared_ptr<PKX> tempPkm;
						if(currentPokemon((arrowY*6)+arrowX)->species() != 0)	tempPkm = currentPokemon((arrowY*6)+arrowX);
						else	tempPkm = save->emptyPkm();
						// Write the held Pokémon to the cursor position
						if(topScreen)	Banks::bank->pkm(heldPkm, currentBox(), (arrowY*6)+arrowX);
						else {
							save->pkm(heldPkm, currentBox(), (arrowY*6)+arrowX, false);
							save->dex(heldPkm);
						}
						// Write the cursor position's previous Pokémon to the held Pokémon's old spot
						if(heldPokemonScreen)	Banks::bank->pkm(tempPkm, heldPokemonBox, heldPokemon);
						else {
							save->pkm(tempPkm, heldPokemonBox, heldPokemon, false);
							save->dex(heldPkm);
						}
						// Hide the moving Pokémon
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
						
						// Update the box(es) for the moved Pokémon
						drawBox(topScreen);
						if(heldPokemonScreen != topScreen)	drawBox(heldPokemonScreen);
						drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));

						// Not holding a Pokémon anymore
						heldPokemon = -1;
						heldPokemonBox = -1;
					}
				} else {
					if(currentPokemon((arrowY*6)+arrowX)->species() != 0) {
						// If no pokemon is currently held and there is one at the cursor, pick it up
						heldPokemon = (arrowY*6)+arrowX;
						heldPokemonBox = currentBox();
						heldPokemonScreen = topScreen;
						setHeldPokemon(currentPokemon(heldPokemon)->species());
						setSpriteVisibility(heldPokemonScreen ? heldPokemon+30 : heldPokemon, false);
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, true);
						drawPokemonInfo(currentPokemon(heldPokemon));
					}
				}
			}

			if(hDown & KEY_SELECT) {
				std::string newName = Input::getLine(topScreen ? 16 : 8);
				if(topScreen)	Banks::bank->boxName(newName, currentBankBox);
				else	save->boxName(currentSaveBox, newName);
				drawBox(topScreen);
			}

			if(hDown & KEY_START) {
				// Hide all sprites
				for(uint i=0;i<getSpriteAmount();i++) {
					setSpriteVisibility(i, false);
				}
				updateOam();

				// Draw message box asking whether to save
				drawRectangle(20, 20, 216, 152, 0x8006, true);
				printText("Do you want to save changes?", 25, 25, true);
				printText("A: Yes   B: No", 25, 57, true);
				
				// Scan for A/B to decide whether to save
				do {
					swiWaitForVBlank();
					scanKeys();
					hDown = keysDown();
				} while(!(hDown & KEY_A || hDown & KEY_B));
				if(hDown & KEY_A) {
					// Save changes to save file
					save->cryptBoxData(false);
					saveChanges(savePath);

					// Save changes to bank
					Banks::bank->save();
				} else {
					// Reload the bank
					Banks::bank->load(Banks::bank->boxes());
				}

				break;
			}

			if(arrowY == -2) {
				arrowY = 4;
				topScreen = true;
				setSpriteVisibility(bottomArrowID, false);
				setSpriteVisibility(topArrowID, true);
				if(heldPokemon != -1) {
					setSpriteVisibility(bottomHeldPokemonID, false);
					setSpriteVisibility(topHeldPokemonID, true);
				}
			} else if(arrowY == 5) {
				arrowY = -1;
				topScreen = false;
				setSpriteVisibility(bottomArrowID, true);
				setSpriteVisibility(topArrowID, false);
				if(heldPokemon != -1) {
					setSpriteVisibility(bottomHeldPokemonID, true);
					setSpriteVisibility(topHeldPokemonID, false);
				}

			}

			if((hDown & KEY_UP || hDown & KEY_DOWN || hDown & KEY_LEFT || hDown & KEY_RIGHT || hDown & KEY_L || hDown & KEY_R) && heldPokemon == -1) {
				if(arrowY != -1)	drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));
				else	drawPokemonInfo(save->emptyPkm());
			}

			if(arrowY == -1) {
				setSpritePosition((topScreen ? topArrowID : bottomArrowID), 90, 16);
				if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), 82, 12);
			} else {
				setSpritePosition((topScreen ? topArrowID : bottomArrowID), (arrowX*24)+24, (arrowY*24)+36);
				if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), (arrowX*24)+16, (arrowY*24)+32);
			}
			updateOam();
		}
	}

	return 0;
}