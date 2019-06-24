#include "manager.h"
#include "banks.hpp"
#include "common/banks.hpp"
#include "graphics/colors.h"
#include "graphics/graphics.h"
#include "keyboard.h"
#include "loader.h"
#include "saves/cardSaves.h"
#include "summary.h"
#include "utils.hpp"

bool topScreen;
int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID;
std::string savePath;
std::vector<u16> arrow, ballSheet, bankBox, menuButton, shiny, pokemonSheet, stripes, types;
ImageData ballSheetData, bankBoxData, menuButtonData, pokemonSheetData, stripesData, typesData;

int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int slot) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), slot);
	else	return save->pkm(currentBox(), slot);
}

XYCoords getPokemonPosition(int dexNumber) {
	if(dexNumber > 649)	return {0, 0};
	XYCoords xy;
	xy.y = (dexNumber/16)*32;
	xy.x = (dexNumber-((dexNumber/16)*16))*32;
	return xy;
}

void loadGraphics(void) {
	// Load images into RAM
	ballSheetData = loadPng("nitro:/graphics/ballSheet.png", ballSheet);
	bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	menuButtonData = loadPng("nitro:/graphics/menuButton.png", menuButton);
	pokemonSheetData = loadPng("nitro:/graphics/pokemonSheet.png", pokemonSheet);
	stripesData = loadPng("nitro:/graphics/stripes.png", stripes);
	typesData = loadPng("nitro:/graphics/types.png", types);
	loadPng("nitro:/graphics/arrow.png", arrow);
	loadPng("nitro:/graphics/shiny.png", shiny);

	// Init Pokémon Sprites
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, false);
	for(int i=0;i<30;i++)	initSprite(SpriteSize_32x32, true);
	// Prepare their locations
	for(int y=0;y<5;y++) {
		for(int x=0;x<6;x++) {
			prepareSprite((y*6)+x, 8+(x*24), 32+(y*24), 2);
			prepareSprite(((y*6)+x)+30, 8+(x*24), 32+(y*24), 2);
		}
	}

	// Prepare bottom arrow sprite
	bottomArrowID = initSprite(SpriteSize_16x16, false);
	fillSpriteImage(bottomArrowID, arrow);
	prepareSprite(bottomArrowID, 24, 36, 0);
	setSpriteVisibility(bottomArrowID, false);

	// Prepare top arrow sprite
	topArrowID = initSprite(SpriteSize_16x16, true);
	fillSpriteImage(topArrowID, arrow);
	prepareSprite(topArrowID, 24, 36, 0);
	setSpriteVisibility(topArrowID, false);

	// Prepare bottom sprite for moving pokemon
	bottomHeldPokemonID = initSprite(SpriteSize_32x32, false);
	prepareSprite(bottomHeldPokemonID, 0, 0, 1);
	setSpriteVisibility(bottomHeldPokemonID, false);

	// Prepare top sprite for moving pokemon
	topHeldPokemonID = initSprite(SpriteSize_32x32, true);
	prepareSprite(topHeldPokemonID, 0, 0, 1);
	setSpriteVisibility(topHeldPokemonID, false);

	// Prepare shiny sprite
	shinyID = initSprite(SpriteSize_16x16, true); // 8x8 wasn't working
	fillSpriteImage(shinyID, shiny);
	prepareSprite(shinyID, 239, 52, 0);
	setSpriteVisibility(shinyID, false);
}

void drawBoxScreen(void) {
	// Draws backgrounds
	drawRectangle(0, 0, 256, 192, DARK_GRAY, true);
	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Show bottom arrow
	setSpriteVisibility(bottomArrowID, true);

	// Move the arrow back to 24, 36
	setSpritePosition(bottomArrowID, 24, 36);

	// Draw the boxes and Pokémon
	drawBox(true);
	drawBox(false);

	// Draw first Pokémon's info
	drawPokemonInfo(save->pkm(currentBox(), 0));
}

void drawBox(bool top) {
	// Draw box images
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, top);

	if(top) {
		// Print box names
		printTextCenteredTinted(Banks::bank->boxName(currentBankBox), DARK_GRAY, -44, 20, true);

		for(int i=0;i<30;i++) {
			// Show/Hide Pokémon sprites for bank box
			if(Banks::bank->pkm(currentBankBox, i)->species() == 0)
				setSpriteVisibility(i+30, false);
			else
				setSpriteVisibility(i+30, true);
		}
		updateOam();

		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(Banks::bank->pkm(currentBankBox, i)->species() != 0) {
				XYCoords xy = getPokemonPosition(Banks::bank->pkm(currentBankBox, i)->species());
				fillSpriteFromSheet(i+30, pokemonSheet, 32, 32, pokemonSheetData.width, xy.x, xy.y);
			}
		}
	} else {
		// Print box names
		printTextCenteredTinted(save->boxName(currentSaveBox), DARK_GRAY, -44, 20, false);

		for(int i=0;i<30;i++) {
			// Show/Hide Pokémon sprites for save box
			if(save->pkm(currentSaveBox, i)->species() == 0)
				setSpriteVisibility(i, false);
			else
				setSpriteVisibility(i, true);
		}
		updateOam();

		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(save->pkm(currentSaveBox, i)->species() != 0) {
				XYCoords xy = getPokemonPosition(save->pkm(currentSaveBox, i)->species());
				fillSpriteFromSheet(i, pokemonSheet, 32, 32, pokemonSheetData.width, xy.x, xy.y);
			}
		}
	}
}

void drawPokemonInfo(std::shared_ptr<PKX> pkm) {
	// Clear previous draw
	drawRectangle(170, 0, 86, 192, DARK_GRAY, true);

	// Draw dashed lines
	drawImage(170, 30, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 60, stripesData.width, stripesData.height, stripes, true);
	drawImage(170, 90, stripesData.width, stripesData.height, stripes, true);

	if(pkm->species() > 0 && pkm->species() < 650) {
		// Show shiny star if applicable
		setSpriteVisibility(shinyID, pkm->shiny());

		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "#%.3i", pkm->species());
		printTextTinted(str, 0xCE73, 170, 2, true);

		// Print nickname
		printText(pkm->nickname(), 170, 14, true);

		// Draw types
		drawImageFromSheet(170, 33, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), true);
		if(pkm->type1() != pkm->type2())
			drawImageFromSheet(205, 33, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), true);

		// Print Level
		snprintf(str, sizeof(str), "Lv.%i", pkm->level());
		printText(str, 170, 46, true);
	} else {
		// Hide shiny star
		setSpriteVisibility(shinyID, false);
	}
}

void setHeldPokemon(int dexNum) {
	if(dexNum != 0) {
		XYCoords xy = getPokemonPosition(dexNum);
		fillSpriteFromSheet(bottomHeldPokemonID, pokemonSheet, 32, 32, pokemonSheetData.width, xy.x, xy.y);
		fillSpriteFromSheet(topHeldPokemonID, pokemonSheet, 32, 32, pokemonSheetData.width, xy.x, xy.y);
	}
}

void savePrompt(void) {
	// Draw background
	drawRectangle(0, 0, 256, 32, WHITE, false);
	drawRectangle(0, 32, 256, 144, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	printTextTinted("Would you like to save changes", DARK_GRAY, 5, 0, false);
	printTextTinted("to the chest?", DARK_GRAY, 5, 16, false);
	drawRectangle(200, 33, 56, 16, BLACK, false);
	printText("Yes", 205, 32, false);
	drawRectangle(200, 50, 56, 16, BLACK, false);
	printText("No", 205, 50, false);
	bool menuSelection = 0, colorSelection = 0, savingSave = 0;
	int pressed;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!pressed);

		if(pressed & KEY_A) {
			saveYes:
			drawRectangle(0, 0, 256, 32, WHITE, false);
			printTextTinted("Saving...", DARK_GRAY, 5, 0, false);
			if(!savingSave) {
				if(!menuSelection)	Banks::bank->save();
				drawRectangle(0, 0, 256, 32, WHITE, false);
				printTextTinted("Would you like to save changes", DARK_GRAY, 5, 0, false);
				if(savePath == cardSave)
					printTextTinted("to the gamecard?", DARK_GRAY, 5, 16, false);
				else
					printTextTinted("to the save?", DARK_GRAY, 5, 16, false);
				savingSave = 1;
			} else {
				if(!menuSelection) {
					// Re-encrypt the box data
					save->cryptBoxData(false);
					// Save changes to save file
					saveChanges(savePath);
					// Reload save
					loadSave(savePath);
					save->cryptBoxData(true);
					if(savePath == cardSave) {
						drawRectangle(0, 32, 256, 48, DARK_GRAY, false);
						updateCardInfo();
						if(!restoreSave()) {
							drawRectangle(0, 0, 256, 192, DARK_GRAY, true);
							drawBox(true);
						}
					}
				}
				break;
			}
		} else if(pressed & KEY_B) {
			saveNo:
			if(!savingSave) {
				drawRectangle(0, 0, 256, 32, WHITE, false);
				printTextTinted("Would you like to save changes", DARK_GRAY, 5, 0, false);
				if(savePath == cardSave)
					printTextTinted("to the gamecard?", DARK_GRAY, 5, 16, false);
				else
					printTextTinted("to the save?", DARK_GRAY, 5, 16, false);
				savingSave = 1;
			} else {
				break;
			}
		} else if(pressed & KEY_UP) {
			if(menuSelection)	menuSelection = 0;
			colorSelection = 1;
		} else if(pressed & KEY_DOWN) {
			if(!menuSelection)	menuSelection = 1;
			colorSelection = 1;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.px >= 200 && touch.py >= 32 && touch.py <= 48) {
				goto saveYes;
			} else if(touch.px >= 200 && touch.py >= 49 && touch.py <= 65) {
				goto saveNo;
			}
		}
		if(colorSelection) {
			drawRectangle(200, 33, 56, 16, !menuSelection ? TEAL & LIGHT_GRAY : BLACK, false);
			printText("Yes", 205, 33, false);
			drawRectangle(200, 50, 56, 16, menuSelection ? TEAL & LIGHT_GRAY : BLACK, false);
			printText("No", 205, 50, false);
		}
	}
	// Draw X menu background
	drawRectangle(0, 0, 256, 16, BLACK, false);
	drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);
}

void drawXMenuButtons(int menuSelection) {
	drawImageTinted(3, 24, menuButtonData.width, menuButtonData.height, menuSelection == 0 ? TEAL_RGB : LIGHT_GRAY, menuButton, false);
	drawImageTinted(131, 24, menuButtonData.width, menuButtonData.height, menuSelection == 1 ? TEAL_RGB : LIGHT_GRAY, menuButton, false);
	drawImageTinted(3, 72, menuButtonData.width, menuButtonData.height, menuSelection == 2 ? TEAL_RGB : LIGHT_GRAY, menuButton, false);
	printText("Options", 50, 40, false);
	printText("Save", 178, 40, false);
	printText("Exit", 50, 88, false);
}

bool xMenu(void) {
	// Hide bottom sprites
	for(uint i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	setSpriteVisibility(bottomArrowID, false);
	updateOam();
	
	// Draw background
	drawRectangle(0, 0, 256, 16, BLACK, false);
	drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	drawXMenuButtons(-1);

	int pressed, menuSelection = -1, selectedOption = -1;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!pressed);
		if(menuSelection == -1 && (pressed & KEY_UP || pressed & KEY_DOWN || pressed & KEY_LEFT || pressed & KEY_RIGHT)) {
			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1)	menuSelection -= 2;
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < 1)	menuSelection += 2;
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2)	menuSelection--;
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2))	menuSelection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			XYCoords menuButtons[] = {{3, 24}, {131, 24}, {3, 72}};
			for(uint i=0; i<(sizeof(menuButtons)/sizeof(menuButtons[0]));i++) {
				if(touch.px >= menuButtons[i].x && touch.px <= menuButtons[i].x+menuButtonData.width && touch.py >= menuButtons[i].y && touch.py <= menuButtons[i].y+menuButtonData.height) {
					selectedOption = i;
				}
			}
			menuSelection = -1;
		} else if(pressed & KEY_A) {
			selectedOption = menuSelection;
		} else if(pressed & KEY_B || pressed & KEY_X) {
			if(!topScreen)	setSpriteVisibility(bottomArrowID, true);
			drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
			drawBox(false);
			break;
		}

		if(selectedOption != -1) {
			// Flash selected option
			drawXMenuButtons(selectedOption);
			for(int i=0;i<6;i++)	swiWaitForVBlank();
			drawXMenuButtons(-1);
			for(int i=0;i<6;i++)	swiWaitForVBlank();
			drawXMenuButtons(selectedOption);
			for(int i=0;i<6;i++)	swiWaitForVBlank();

			if(selectedOption == 0) {
			} else if(selectedOption == 1) {
				savePrompt();
			} else if(selectedOption == 2) {
				savePrompt();
				// Hide remaining sprites
				for(uint i=30;i<getSpriteAmount();i++) {
					setSpriteVisibility(i, false);
				}
				updateOam();
				return 0;
			}
			selectedOption = -1;
		}

		drawXMenuButtons(menuSelection);
	}
	return 1;
}

void manageBoxes(void) {
	int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonBox = -1;
	bool heldPokemonScreen = false;
	topScreen = false;
	u16 pressed = 0, held = 0;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(arrowY > (topScreen ? -1 : -2))	arrowY--;
		} else if(held & KEY_DOWN) {
			if(arrowY < (topScreen ? 5 : 4))	arrowY++;
		}
		if(held & KEY_LEFT && arrowY != -1) {
			if(arrowX > 0)	arrowX--;
		} else if(held & KEY_RIGHT && arrowY != -1) {
			if(arrowX < 5)	arrowX++;
		}
		if(held & KEY_LEFT && arrowY == -1) {
			goto switchBoxLeft;
		} else if(held & KEY_RIGHT && arrowY == -1) {
			goto switchBoxRight;
		}
		if(held & KEY_L) {
			switchBoxLeft:
			if(currentBox() > 0)
				(topScreen ? currentBankBox : currentSaveBox)--;
			else if(topScreen) currentBankBox = Banks::bank->boxes()-1;
			else currentSaveBox = save->maxBoxes()-1;
			drawBox(topScreen);
			if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		} else if(held & KEY_R) {
			switchBoxRight:
			if((topScreen ? currentBankBox < Banks::bank->boxes()-1 : currentSaveBox < save->maxBoxes()-1))
				(topScreen ? currentBankBox : currentSaveBox)++;
			else (topScreen ? currentBankBox : currentSaveBox) = 0;
			drawBox(topScreen);
			if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		}
		if(pressed & KEY_A) {
			if(arrowY == -1) {
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
				drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
				drawBox(false);
				if(topScreen)	drawBox(topScreen);
			} else {
				// Otherwise move Pokémon
				if(heldPokemon != -1) {
					if(heldPokemon == (arrowY*6)+arrowX && heldPokemonBox == currentBox()) {
						// If in the held Pokémon's previous spot, just put it back down
						setSpriteVisibility((heldPokemonScreen ? heldPokemon+30 : heldPokemon), true);
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
		}

		if(pressed & KEY_Y && heldPokemon == -1) {
			if(topScreen)
				Banks::bank->pkm(showPokemonSummary(currentPokemon((arrowY*6)+arrowX)), currentBankBox, (arrowY*6)+arrowX);
			else
				save->pkm(showPokemonSummary(currentPokemon((arrowY*6)+arrowX)), currentSaveBox, (arrowY*6)+arrowX, false);
			drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
			drawBox(false);
			if(topScreen)	drawBox(topScreen);
			drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));
			setSpriteVisibility(topArrowID, topScreen);
			setSpriteVisibility(bottomArrowID, !topScreen);
			setSpritePosition(bottomArrowID, arrowX, arrowY);
			updateOam();
		}

		if(pressed & KEY_X && heldPokemon == -1) {
			if(!xMenu())	break;
		}

		if(arrowY == -2) {
			// If the Arrow Y is at -2, switch to the top screen
			arrowY = 4;
			topScreen = true;
			setSpriteVisibility(bottomArrowID, false);
			setSpriteVisibility(topArrowID, true);
			if(heldPokemon != -1) {
				setSpriteVisibility(bottomHeldPokemonID, false);
				setSpriteVisibility(topHeldPokemonID, true);
			}
		} else if(arrowY == 5) {
			// If the Arrow Y is at 5, switch to the bottom screen
			arrowY = -1;
			topScreen = false;
			setSpriteVisibility(bottomArrowID, true);
			setSpriteVisibility(topArrowID, false);
			if(heldPokemon != -1) {
				setSpriteVisibility(bottomHeldPokemonID, true);
				setSpriteVisibility(topHeldPokemonID, false);
			}

		}

		if((held & KEY_UP || held & KEY_DOWN || held & KEY_LEFT || held & KEY_RIGHT || held & KEY_L || held & KEY_R) && heldPokemon == -1) {
			// If the cursor is moved and we're not holding a Pokémon, draw the new one
			if(arrowY != -1)	drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));
			else	drawPokemonInfo(save->emptyPkm());
		}

		if(arrowY == -1) {
			// If the Arrow Y is at -1 (box title), draw it in the middle
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), 90, 16);
			if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), 82, 12);
		} else {
			// Otherwise move it to the spot in the box it's at
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), (arrowX*24)+24, (arrowY*24)+36);
			if(heldPokemon != -1)	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), (arrowX*24)+16, (arrowY*24)+32);
		}
		updateOam();
	}
}
