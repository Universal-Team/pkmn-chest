#include "manager.h"
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
#include "keyboard.h"
#include "langStrings.h"
#include "loader.h"
#include "party.h"
#include "summary.h"
#include "trainer.h"
#include "utils.hpp"

bool topScreen;
int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID,
arrowMode = 0, pokemonSheetScale = 1, pokemonSheetSize = 32;
std::string savePath;
std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, menuButton, shiny, pokemonSheet, stripes, types;
ImageData ballSheetData, bankBoxData, menuButtonData, pokemonSheetData, shinyData, stripesData, typesData;

struct TextPos {
	int x;
	int y;
};

std::vector<TextPos> aMenuButtons = {
	{170,  16}, // Edit
	{170,  41}, // Move
	{170,  66}, // Copy
	{170,  91}, // Release
	{170, 116}, // Dump
	{170, 141}, // Back
};
std::vector<TextPos> aMenuEmptySlotButtons = {
	{170, 16}, // Inject
	{170, 41}, // Create
	{170, 66}, // Back
};
std::vector<TextPos> aMenuTopBarButtons = {
	{170, 16}, // Rename
	{170, 41}, // Swap
	{170, 66}, // Dump box
	{170, 91}, // Back
};
std::vector<TextPos> xMenuButtons = {
	{3,  24}, {131,  24},
	{3,  72}, {131,  72},
	{3, 120}, {131, 120},
};


int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int slot) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), slot);
	else	return save->pkm(currentBox(), slot);
}

XYCoords getPokemonPosition(std::shared_ptr<PKX> pkm) {
	if(pkm->species() > 649)	return {0, 0};
	else if(pkm->species() == 201) { // Unown
		if(pkm->alternativeForm() == 0);
		else if(pkm->alternativeForm() < 5)
			return {384+((pkm->alternativeForm()-1)*32), 1280};
		else if(pkm->alternativeForm() < 21)
			return {(pkm->alternativeForm()-5)*32, 1312};
		else
			return {(pkm->alternativeForm()-21)*32, 1344};
	} else if(pkm->species() == 386) { // Deoxys
		if(pkm->alternativeForm() > 0)
			return {224+((pkm->alternativeForm()-1)*32), 1344};
	} else if(pkm->species() == 412) { // Burmy
		if(pkm->alternativeForm() > 0)
			return {320+((pkm->alternativeForm()-1)*32), 1344};
	} else if(pkm->species() == 413) { // Wormadam
		if(pkm->alternativeForm() > 0)
			return {384+((pkm->alternativeForm()-1)*32), 1344};
	} else if(pkm->species() == 422) { // Shellos
		if(pkm->alternativeForm() == 1)
			return {448, 1344};
	} else if(pkm->species() == 423) { // Gastrodon
		if(pkm->alternativeForm() == 1)
			return {480, 1344};
	} else if(pkm->species() == 479) { // Rotom
		if(pkm->alternativeForm() > 0)
			return {(pkm->alternativeForm()-1)*32, 1376};
	} else if(pkm->species() == 487) { // Giratina
		if(pkm->alternativeForm() == 1)
			return {160, 1376};
	} else if(pkm->species() == 492) { // Shaymin
		if(pkm->alternativeForm() == 1)
			return {192, 1376};
	} else if(pkm->species() == 521) { // Unfezant
		if(pkm->gender() == 1)
			return {224, 1376};
	} else if(pkm->species() == 550) { // Basculin
		if(pkm->alternativeForm() == 1)
			return {256, 1376};
	} else if(pkm->species() == 555) { // Darmanitan
		if(pkm->alternativeForm() == 1)
			return {288, 1376};
	} else if(pkm->species() == 585) { // Deerling
		if(pkm->alternativeForm() > 0)
			return {320+((pkm->alternativeForm()-1)*32), 1376};
	} else if(pkm->species() == 586) { // Sawsbuck
		if(pkm->alternativeForm() > 0)
			return {416+((pkm->alternativeForm()-1)*32), 1376};
	} else if(pkm->species() == 592) { // Frillish
		if(pkm->gender() == 1)
			return {0, 1408};
	} else if(pkm->species() == 593) { // Jellicent
		if(pkm->gender() == 1)
			return {32, 1408};
	} else if(pkm->species() == 648) { // Meloetta
		if(pkm->alternativeForm() == 1)
			return {64, 1408};
	} else if(pkm->species() == 641) { // Tornadus
		if(pkm->alternativeForm() == 1)
			return {96, 1408};
	} else if(pkm->species() == 642) { // Thunderus
		if(pkm->alternativeForm() == 1)
			return {128, 1408};
	} else if(pkm->species() == 645) { // Landorus
		if(pkm->alternativeForm() == 1)
			return {160, 1408};
	} else if(pkm->species() == 646) { // Kyurem
		if(pkm->alternativeForm() > 0)
			return {192+((pkm->alternativeForm()-1)*32), 1408};
	} else if(pkm->species() == 647) { // Keldeo
		if(pkm->alternativeForm() == 1)
			return {256, 1408};
	}

	// Non-alternate form, get based on dex number
	XYCoords xy;
	xy.y = (pkm->species()/16)*pokemonSheetSize;
	xy.x = (pkm->species()-((pkm->species()/16)*16))*pokemonSheetSize;
	return xy;
}

void loadGraphics(void) {
	// Load images into RAM
	ballSheetData = loadPng("nitro:/graphics/ballSheet.png", ballSheet);
	bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	menuButtonData = loadPng("nitro:/graphics/menuButton.png", menuButton);
	pokemonSheetData = loadPng(sdFound() ? "nitro:/graphics/pokemonSheet.png" : "nitro:/graphics/pokemonSheetSmall.png", pokemonSheet);
	shinyData = loadPng("nitro:/graphics/shiny.png", shiny);
	stripesData = loadPng("nitro:/graphics/stripes.png", stripes);
	typesData = loadPng("nitro:/graphics/types.png", types);
	loadPng("nitro:/graphics/arrowBlue.png", arrowBlue);
	loadPng("nitro:/graphics/arrowRed.png", arrowRed);
	loadPng("nitro:/graphics/arrowYellow.png", arrowYellow);

	// Set Pokémon sheet scale and size vars
	pokemonSheetScale = sdFound() ? 1 : 2;
	pokemonSheetSize = sdFound() ? 32 : 16;

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
	fillSpriteImage(bottomArrowID, arrowRed);
	prepareSprite(bottomArrowID, 24, 36, 0);
	setSpriteVisibility(bottomArrowID, false);

	// Prepare top arrow sprite
	topArrowID = initSprite(SpriteSize_16x16, true);
	fillSpriteImage(topArrowID, arrowRed);
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
				XYCoords xy = getPokemonPosition(Banks::bank->pkm(currentBankBox, i));
				fillSpriteFromSheetScaled(i+30, pokemonSheetScale, pokemonSheet, pokemonSheetSize, pokemonSheetSize, pokemonSheetData.width, xy.x, xy.y);
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
				XYCoords xy = getPokemonPosition(save->pkm(currentSaveBox, i));
				fillSpriteFromSheetScaled(i, pokemonSheetScale, pokemonSheet, pokemonSheetSize, pokemonSheetSize, pokemonSheetData.width, xy.x, xy.y);
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
		snprintf(str, sizeof(str), "No.%.3i", pkm->species());
		printTextTinted(str, 0xCE73, 170, 2, true);

		// Print name
		if(pkm->nicknamed())	printTextTintedMaxW(pkm->nickname(), 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? RED_RGB : WHITE) : BLUE_RGB), 170, 14, true);
		else	printTextTintedMaxW(Lang::species[pkm->species()], 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? RED_RGB : WHITE) : BLUE_RGB), 170, 14, true);


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

void setHeldPokemon(std::shared_ptr<PKX> pkm) {
	if(pkm->species() != 0) {
		XYCoords xy = getPokemonPosition(pkm);
		fillSpriteFromSheetScaled(bottomHeldPokemonID, pokemonSheetScale, pokemonSheet, pokemonSheetSize, pokemonSheetSize, pokemonSheetData.width, xy.x, xy.y);
		fillSpriteFromSheetScaled(topHeldPokemonID, pokemonSheetScale, pokemonSheet, pokemonSheetSize, pokemonSheetSize, pokemonSheetData.width, xy.x, xy.y);
	}
}

std::string aMenuText(int buttonMode, int i) {
	if(buttonMode == 0)	return Lang::aMenuText[i];
	else if(buttonMode == 1)	return Lang::aMenuTopBarText[i];
	else return Lang::aMenuEmptySlotText[i];
}

void drawAMenuButtons(std::vector<TextPos>& buttons, int buttonMode) {
	for(uint i=0;i<buttons.size();i++) {
		drawRectangle(buttons[i].x, buttons[i].y, 80, 24, DARKER_GRAY, false);
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

			for(uint i=0; i<buttons.size();i++) {
				if(touch.px >= buttons[i].x && touch.px <= buttons[i].x+64 && touch.py >= buttons[i].y && touch.py <= buttons[i].y+25) {
					menuSelection = i;
					optionSelected = true;
				}
			}
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			goto back;
		}

		if(optionSelected && buttonMode == 0) { // A Pokémon
			optionSelected = false;
			if(menuSelection == 0) { // Edit
				edit:
				if(topScreen)	Banks::bank->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentBankBox, pkmPos);
				else	save->pkm(showPokemonSummary(currentPokemon(pkmPos)), currentSaveBox, pkmPos, false);
				
				// Redraw screen
				drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
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
				drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
				return 1;
			} else if(menuSelection == 2) { // Copy
				if(topScreen) {
					setSpriteVisibility(bottomArrowID, false);
					setSpriteVisibility(topArrowID, true);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
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
					drawRectangle(5+bankBoxData.width, 0, 256-(5+bankBoxData.width), 192, DARK_GRAY, false);
					goto back;
				}
				drawBox(topScreen);
				drawPokemonInfo(save->emptyPkm());
				drawRectangle(5+bankBoxData.width, 66, 256-(5+bankBoxData.width), 60, DARK_GRAY, false);
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
				drawRectangle(170, 0, 86, 192, DARK_GRAY, false);
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
				drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
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
				
				// If the fileName 
				if(fileName != "") {
				std::ifstream in(fileName);
				u8* buffer = 0;
				in.read((char*)buffer, 136);
				if(topScreen)	Banks::bank->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos);
				else	save->pkm(save->emptyPkm()->getPKM(fileName.substr(fileName.size()-1) == "4" ? Generation::FOUR : Generation::FIVE, buffer), currentSaveBox, pkmPos, false);
				}
				
				// Reset 
				chdir(path);
				drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
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

	// Draw X menu background
	drawRectangle(0, 0, 256, 16, BLACK, false);
	drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);
}

void drawXMenuButtons(uint menuSelection) {
	Lang::xMenuText[3] = save->otName();

	for(uint i=0;i<xMenuButtons.size();i++) {
		drawImageTinted(xMenuButtons[i].x, xMenuButtons[i].y, menuButtonData.width, menuButtonData.height, menuSelection == i ? TEAL_RGB : LIGHT_GRAY, menuButton, false);
		printText(Lang::xMenuText[i], xMenuButtons[i].x+47, xMenuButtons[i].y+14, false);
	}
}

bool xMenu(void) {
	// Hide bottom sprites
	for(uint i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	setSpriteVisibility(bottomArrowID, false);
	updateOam();

	// Make bottom arrow red
	fillSpriteImage(bottomArrowID, arrowRed);
	
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
		if(menuSelection == -1 && !(pressed & KEY_TOUCH)) {
			menuSelection = 0;
		} else if(pressed & KEY_UP) {
			if(menuSelection > 1)	menuSelection -= 2;
		} else if(pressed & KEY_DOWN) {
			if(menuSelection < (int)xMenuButtons.size()-2)	menuSelection += 2;
		} else if(pressed & KEY_LEFT) {
			if(menuSelection % 2)	menuSelection--;
		} else if(pressed & KEY_RIGHT) {
			if(!(menuSelection % 2))	menuSelection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(uint i=0; i<xMenuButtons.size();i++) {
				if(touch.px >= xMenuButtons[i].x && touch.px <= xMenuButtons[i].x+menuButtonData.width && touch.py >= xMenuButtons[i].y && touch.py <= xMenuButtons[i].y+menuButtonData.height) {
					selectedOption = i;
				}
			}
			menuSelection = -1;
		} else if(pressed & KEY_A) {
			selectedOption = menuSelection;
		} else if(pressed & KEY_B || pressed & KEY_X) {
			// Reset arrow color
			fillSpriteImage(bottomArrowID, arrowMode ? arrowBlue : arrowRed);
			setSpriteVisibility(topScreen ? topArrowID : bottomArrowID, true);
			updateOam();
			drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
			drawBox(false);
			break;
		}

		if(selectedOption != -1) {
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
					for(uint i=30;i<getSpriteAmount();i++) {
						setSpriteVisibility(i, false);
					}
					updateOam();
					return 0;
			}

			// Redraw menu
			drawRectangle(0, 0, 256, 16, BLACK, false);
			drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
			drawRectangle(0, 176, 256, 16, BLACK, false);
			drawXMenuButtons(1);

			selectedOption = -1;
		}

		drawXMenuButtons(menuSelection);
	}
	return 1;
}

void manageBoxes(void) {
	int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonBox = -1;
	bool heldPokemonScreen = false, heldMode = false;
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
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		} else if(held & KEY_R) {
			switchBoxRight:
			if((topScreen ? currentBankBox < Banks::bank->boxes()-1 : currentSaveBox < save->maxBoxes()-1))
				(topScreen ? currentBankBox : currentSaveBox)++;
			else (topScreen ? currentBankBox : currentSaveBox) = 0;
			drawBox(topScreen);
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		}
		if(pressed & KEY_A) {
			if(arrowY == -1) {
				if(arrowMode == 0 && heldPokemon == -1)
					aMenu(-1, aMenuTopBarButtons, 1);
			} else {
				// Otherwise move Pokémon
				if(heldPokemon != -1) {
					if(heldPokemon == (arrowY*6)+arrowX && heldPokemonBox == currentBox()) {
						// If in the held Pokémon's previous spot, just put it back down
						setSpriteVisibility((heldPokemonScreen ? heldPokemon+30 : heldPokemon), true);
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
						heldPokemon = -1;
						heldPokemonBox = -1;
					} else if(!heldMode || currentPokemon((arrowY*6)+arrowX)->species() == 0) {
						if(topScreen || (heldPokemonScreen ? Banks::bank->pkm(heldPokemonBox, heldPokemon) : save->pkm(heldPokemonBox, heldPokemon))->species() <= save->maxSpecies()) {
							// If not copying / there isn't a Pokémon at the new spot, move Pokémon
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
							// If not copying, write the cursor position's previous Pokémon to the held Pokémon's old spot
							if(!heldMode) {
								if(heldPokemonScreen)	Banks::bank->pkm(tempPkm, heldPokemonBox, heldPokemon);
								else {
									save->pkm(tempPkm, heldPokemonBox, heldPokemon, false);
									save->dex(heldPkm);
								}
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
					}
				} else if(currentPokemon((arrowY*6)+arrowX)->species() != 0) {
					int temp = 1;
					if(arrowMode != 0 || (temp = aMenu((arrowY*6)+arrowX, aMenuButtons, 0))) {
						// If no pokemon is currently held and there is one at the cursor, pick it up
						heldPokemon = (arrowY*6)+arrowX;
						heldPokemonBox = currentBox();
						heldPokemonScreen = topScreen;
						heldMode = temp-1; // false = move, true = copy
						setHeldPokemon(currentPokemon(heldPokemon));
						if(!heldMode)	setSpriteVisibility(heldPokemonScreen ? heldPokemon+30 : heldPokemon, false);
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, true);
						drawPokemonInfo(currentPokemon(heldPokemon));
					}
				} else if(arrowMode == 0) {
					aMenu((arrowY*6)+arrowX, aMenuEmptySlotButtons, 2);
				}
			}
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

		if(pressed & KEY_SELECT && heldPokemon == -1) {
			if(arrowMode < 1)	arrowMode++;
			else	arrowMode = 0;

			if(arrowMode == 0) {
				fillSpriteImage(bottomArrowID, arrowRed);
				fillSpriteImage(topArrowID, arrowRed);
			} else if(arrowMode == 1) {
				fillSpriteImage(bottomArrowID, arrowBlue);
				fillSpriteImage(topArrowID, arrowBlue);
			} else {
				fillSpriteImage(bottomArrowID, arrowYellow);
				fillSpriteImage(topArrowID, arrowYellow);
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
