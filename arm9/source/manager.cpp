#include "manager.h"

#include "banks.hpp"
#include "colors.h"
#include "flashcard.h"
#include "langStrings.h"
#include "loader.h"
#include "loading.h"
#include "aMenu.h"
#include "xMenu.h"
#include "sound.h"

bool topScreen;
int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID, arrowMode = 0;
std::vector<int> menuIconID, partyIconID;
std::string savePath;
std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgBottom, boxBgTop, boxButton, fileBrowseBg, infoBox, menuBg, menuButton, menuButtonBlue, menuIconSheet, optionsBg, pokemonSheet, search, shiny, summaryBg, types;
ImageData ballSheetData, bankBoxData, boxBgBottomData, boxBgTopData, boxButtonData, fileBrowseBgData, infoBoxData, menuBgData, menuButtonData, menuButtonBlueData, menuIconSheetData, optionsBgData, pokemonSheetData, searchData, shinyData, summaryBgData, typesData;

int bankBoxPokemon[30] = {
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
},	saveBoxPokemon[30] = {
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0,
};

int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int slot) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), slot);
	else	return save->pkm(currentBox(), slot);
}

std::pair<int, int> getPokemonPosition(std::shared_ptr<PKX> pkm) {
	return getPokemonPosition(pkm->species(), pkm->alternativeForm(), pkm->gender(), pkm->egg());
}

std::pair<int, int> getPokemonPosition(int species, int alternativeForm, int gender, bool egg) {
	if(species > 649)	return {0, 0};
	else if(egg)	return {352, 1280};
	else if(species == 201) { // Unown
		if(alternativeForm == 0);
		else if(alternativeForm < 5)
			return {384+((alternativeForm-1)*32), 1280};
		else if(alternativeForm < 21)
			return {(alternativeForm-5)*32, 1312};
		else
			return {(alternativeForm-21)*32, 1344};
	} else if(species == 386) { // Deoxys
		if(alternativeForm > 0)
			return {224+((alternativeForm-1)*32), 1344};
	} else if(species == 412) { // Burmy
		if(alternativeForm > 0)
			return {320+((alternativeForm-1)*32), 1344};
	} else if(species == 413) { // Wormadam
		if(alternativeForm > 0)
			return {384+((alternativeForm-1)*32), 1344};
	} else if(species == 422) { // Shellos
		if(alternativeForm == 1)
			return {448, 1344};
	} else if(species == 423) { // Gastrodon
		if(alternativeForm == 1)
			return {480, 1344};
	} else if(species == 479) { // Rotom
		if(alternativeForm > 0)
			return {(alternativeForm-1)*32, 1376};
	} else if(species == 487) { // Giratina
		if(alternativeForm == 1)
			return {160, 1376};
	} else if(species == 492) { // Shaymin
		if(alternativeForm == 1)
			return {192, 1376};
	} else if(species == 521) { // Unfezant
		if(gender == 1)
			return {224, 1376};
	} else if(species == 550) { // Basculin
		if(alternativeForm == 1)
			return {256, 1376};
	} else if(species == 555) { // Darmanitan
		if(alternativeForm == 1)
			return {288, 1376};
	} else if(species == 585) { // Deerling
		if(alternativeForm > 0)
			return {320+((alternativeForm-1)*32), 1376};
	} else if(species == 586) { // Sawsbuck
		if(alternativeForm > 0)
			return {416+((alternativeForm-1)*32), 1376};
	} else if(species == 592) { // Frillish
		if(gender == 1)
			return {0, 1408};
	} else if(species == 593) { // Jellicent
		if(gender == 1)
			return {32, 1408};
	} else if(species == 648) { // Meloetta
		if(alternativeForm == 1)
			return {64, 1408};
	} else if(species == 641) { // Tornadus
		if(alternativeForm == 1)
			return {96, 1408};
	} else if(species == 642) { // Thunderus
		if(alternativeForm == 1)
			return {128, 1408};
	} else if(species == 645) { // Landorus
		if(alternativeForm == 1)
			return {160, 1408};
	} else if(species == 646) { // Kyurem
		if(alternativeForm > 0)
			return {192+((alternativeForm-1)*32), 1408};
	} else if(species == 647) { // Keldeo
		if(alternativeForm == 1)
			return {256, 1408};
	}

	// Non-alternate form, return based on dex number
	return {(species-((species/16)*16))*32, (species/16)*32};
}

int getPokemonIndex(std::shared_ptr<PKX> pkm) {
	return getPokemonIndex(pkm->species(), pkm->alternativeForm(), pkm->gender(), pkm->egg());
}

int getPokemonIndex(int species, int alternativeForm, int gender, bool egg) {
	if(species > 649)	return 0;
	else if(egg)	return 651;
	else if(species == 201) { // Unown
		if(alternativeForm > 0)
			return 651+alternativeForm;
	} else if(species == 386) { // Deoxys
		if(alternativeForm > 0)
			return 578+alternativeForm;
	} else if(species == 412) { // Burmy
		if(alternativeForm > 0)
			return 681+alternativeForm;
	} else if(species == 413) { // Wormadam
		if(alternativeForm > 0)
			return 683+alternativeForm;
	} else if(species == 422) { // Shellos
		if(alternativeForm == 1)
			return 686;
	} else if(species == 423) { // Gastrodon
		if(alternativeForm == 1)
			return 687;
	} else if(species == 479) { // Rotom
		if(alternativeForm > 0)
			return 687+alternativeForm;
	} else if(species == 487) { // Giratina
		if(alternativeForm == 1)
			return 693;
	} else if(species == 492) { // Shaymin
		if(alternativeForm == 1)
			return 694;
	} else if(species == 521) { // Unfezant
		if(gender == 1)
			return 695;
	} else if(species == 550) { // Basculin
		if(alternativeForm == 1)
			return 696;
	} else if(species == 555) { // Darmanitan
		if(alternativeForm == 1)
			return 697;
	} else if(species == 585) { // Deerling
		if(alternativeForm > 0)
			return 697+alternativeForm;
	} else if(species == 586) { // Sawsbuck
		if(alternativeForm > 0)
			return 700+alternativeForm;
	} else if(species == 592) { // Frillish
		if(gender == 1)
			return 704;
	} else if(species == 593) { // Jellicent
		if(gender == 1)
			return 705;
	} else if(species == 648) { // Meloetta
		if(alternativeForm == 1)
			return 706;
	} else if(species == 641) { // Tornadus
		if(alternativeForm == 1)
			return 707;
	} else if(species == 642) { // Thunderus
		if(alternativeForm == 1)
			return 708;
	} else if(species == 645) { // Landorus
		if(alternativeForm == 1)
			return 709;
	} else if(species == 646) { // Kyurem
		if(alternativeForm > 0)
			return 709+alternativeForm;
	} else if(species == 647) { // Keldeo
		if(alternativeForm == 1)
			return 712;
	}

	// Non-alternate form, return dex number
	return species;
}

void loadGraphics(void) {
	// Load images into RAM
	ballSheetData = loadPng("nitro:/graphics/ballSheet.png", ballSheet);
	bankBoxData = loadPng("nitro:/graphics/bankBox.png", bankBox);
	boxButtonData = loadPng("nitro:/graphics/boxButton.png", boxButton);
	infoBoxData = loadPng("nitro:/graphics/infoBox.png", infoBox);
	menuButtonData = loadPng("nitro:/graphics/menuButton.png", menuButton);
	menuButtonBlueData = loadPng("nitro:/graphics/menuButtonBlue.png", menuButtonBlue);
	menuIconSheetData = loadPng("nitro:/graphics/menuIconSheet.png", menuIconSheet);
	searchData = loadPng("nitro:/graphics/search.png", search);
	shinyData = loadPng("nitro:/graphics/shiny.png", shiny);
	typesData = loadPng("nitro:/graphics/types.png", types);
	loadPng("nitro:/graphics/arrowBlue.png", arrowBlue);
	loadPng("nitro:/graphics/arrowRed.png", arrowRed);
	loadPng("nitro:/graphics/arrowYellow.png", arrowYellow);
	if(sdFound()) {
		boxBgBottomData = loadPng("nitro:/graphics/boxBgBottom.png", boxBgBottom);
		boxBgTopData = loadPng("nitro:/graphics/boxBgTop.png", boxBgTop);
		fileBrowseBgData = loadPng("nitro:/graphics/fileBrowseBg.png", fileBrowseBg);
		menuBgData = loadPng("nitro:/graphics/menuBg.png", menuBg);
		optionsBgData = loadPng("nitro:/graphics/optionsBg.png", optionsBg);
		pokemonSheetData = loadPng("nitro:/graphics/pokemonSheet.png", pokemonSheet);
		summaryBgData = loadPng("nitro:/graphics/summaryBg.png", summaryBg);
	}

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

	// Prepare menu icon sprites
	for(int i=0;i<6;i++) {
		int id = initSprite(SpriteSize_32x32, false);
		fillSpriteFromSheet(id, menuIconSheet, 32, 32, menuIconSheetData.width, 0, i*32);
		prepareSprite(id, 0, 0, 0);
		setSpriteVisibility(id, false);
		menuIconID.push_back(id);
	}

	// Prepare party sprites
	for(int i=0;i<6;i++) {
		int id = initSprite(SpriteSize_32x32, false);
		fillSpriteFromSheet(id, menuIconSheet, 32, 32, menuIconSheetData.width, 0, i*32);
		prepareSprite(id, 0, 0, 0);
		setSpriteVisibility(id, false);
		partyIconID.push_back(id);
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
	prepareSprite(shinyID, 239, 45, 0);
	setSpriteVisibility(shinyID, false);
}

void drawBoxScreen(void) {
	// Draws backgrounds
	drawImage(0, 0, boxBgTopData.width, boxBgTopData.height, boxBgTop, true);
	drawImage(164, 2, infoBoxData.width, infoBoxData.height, infoBox, true);
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
	

	// Show bottom arrow
	setSpriteVisibility(bottomArrowID, true);

	// Move the arrow back to 24, 36
	setSpritePosition(bottomArrowID, 24, 36);

	// Draw the boxes and Pokémon
	drawBox(true, true);
	drawBox(false, true);

	// Draw first Pokémon's info
	drawPokemonInfo(save->pkm(currentBox(), 0));
}

void drawBox(bool top, bool reloadPokemon) {
	// Draw box images
	drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, top);

	if(top) {
		// Print box names
		printTextCenteredTinted(Banks::bank->boxName(currentBankBox), GRAY, -44, 20, true, true);

		if(reloadPokemon) {
			if(sdFound()) {
				for(int i=0;i<30;i++) {
					if(Banks::bank->pkm(currentBankBox, i)->species() == 0) {
						setSpriteVisibility(i+30, false);
					} else {
						std::pair<int, int> xy = getPokemonPosition(Banks::bank->pkm(currentBankBox, i));
						fillSpriteFromSheet(i+30, pokemonSheet, 32, 32, pokemonSheetData.width, xy.first, xy.second);
						setSpriteVisibility(i+30, true);
					}
				}
				updateOam();
			} else {
				for(int i=0;i<30;i++) {
					// Hide all Pokémon sprites for bank box
					setSpriteVisibility(i+30, false);
				}
				showLoadingLogo();
				for(int i=0;i<30;i++) {
					// Fill Pokémon Sprites
					if(Banks::bank->pkm(currentBankBox, i)->species() != 0) {
						if(bankBoxPokemon[i] != Banks::bank->pkm(currentBankBox, i)->species()) {
							bankBoxPokemon[i] = Banks::bank->pkm(currentBankBox, i)->species();
							std::vector<u16> bmp;
							loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(Banks::bank->pkm(currentBankBox, i)))+".bmp", bmp);
							fillSpriteImage(i+30, bmp);
						}
						setSpriteVisibility(i+30, true);
						updateOam();
					}
				}
				hideLoadingLogo();
			}
		} else {
			for(int i=0;i<30;i++) {
				// Show/Hide Pokémon sprites for bank box
				if(Banks::bank->pkm(currentBankBox, i)->species() == 0)
					setSpriteVisibility(i+30, false);
				else
					setSpriteVisibility(i+30, true);
			}
			updateOam();
		}

	} else {
		// Print box names
		printTextCenteredTinted(save->boxName(currentSaveBox), GRAY, -44, 20, false, true);

		if(reloadPokemon) {
			if(sdFound()) {
				for(int i=0;i<30;i++) {
					if(save->pkm(currentSaveBox, i)->species() == 0) {
						setSpriteVisibility(i, false);
					} else {
						std::pair<int, int> xy = getPokemonPosition(save->pkm(currentSaveBox, i));
						fillSpriteFromSheet(i, pokemonSheet, 32, 32, pokemonSheetData.width, xy.first, xy.second);
						setSpriteVisibility(i, true);
					}
				}
				updateOam();
			} else {
				for(int i=0;i<30;i++) {
					// Hide all Pokémon sprites for save box
					setSpriteVisibility(i, false);
				}
				showLoadingLogo();
				for(int i=0;i<30;i++) {
					// Fill Pokémon Sprites
					if(save->pkm(currentSaveBox, i)->species() != 0) {
						if(saveBoxPokemon[i] != save->pkm(currentSaveBox, i)->species()) {
							saveBoxPokemon[i] = save->pkm(currentSaveBox, i)->species();
							std::vector<u16> bmp;
							loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(save->pkm(currentSaveBox, i)))+".bmp", bmp);
							fillSpriteImage(i, bmp);
						}
						setSpriteVisibility(i, true);
						updateOam();
					}
				}
				hideLoadingLogo();
			}
		} else {
			for(int i=0;i<30;i++) {
				// Show/Hide Pokémon sprites for save box
				if(save->pkm(currentSaveBox, i)->species() == 0)
					setSpriteVisibility(i, false);
				else
					setSpriteVisibility(i, true);
			}
			updateOam();
		}
	}
}

void drawPokemonInfo(std::shared_ptr<PKX> pkm) {
	// Clear previous draw
	drawImage(164, 2, infoBoxData.width, infoBoxData.height, infoBox, true);

	if(pkm->species() > 0 && pkm->species() < 650) {
		// Show shiny star if applicable
		setSpriteVisibility(shinyID, pkm->shiny());

		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "No.%.3i", pkm->species());
		printTextTinted(str, GRAY, 170, 8, true, true);

		// Print name
		if(pkm->nicknamed())	printTextTintedMaxW(pkm->nickname(), 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? RGB::RED : GRAY) : RGB::BLUE), 170, 25, true, true);
		else	printTextTintedMaxW(Lang::species[pkm->species()], 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? RGB::RED : GRAY) : RGB::BLUE), 170, 25, true, true);


		// Draw types
		drawImageFromSheet(170, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), true);
		if(pkm->type1() != pkm->type2())
			drawImageFromSheet(205, 43, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), true);

		// Print Level
		snprintf(str, sizeof(str), "Lv.%i", pkm->level());
		printTextTinted(str, GRAY, 170, 57, true, true);
	} else {
		// Hide shiny star
		setSpriteVisibility(shinyID, false);
	}
}

void setHeldPokemon(std::shared_ptr<PKX> pkm) {
	if(pkm->species() != 0) {
		if(sdFound()) {
			std::pair<int, int> xy = getPokemonPosition(pkm);
			fillSpriteFromSheet(bottomHeldPokemonID, pokemonSheet, 32, 32, pokemonSheetData.width, xy.first, xy.second);
			fillSpriteFromSheet(topHeldPokemonID, pokemonSheet, 32, 32, pokemonSheetData.width, xy.first, xy.second);
		} else {
			std::vector<u16> bmp;
			loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(pkm))+".bmp", bmp);
			fillSpriteImage(bottomHeldPokemonID, bmp);
			fillSpriteImage(topHeldPokemonID, bmp);
		}
	}
}

void manageBoxes(void) {
	int arrowX = 0, arrowY = 0, heldPokemon = -1, heldPokemonBox = -1;
	bool heldPokemonScreen = false, heldMode = false;
	topScreen = false;
	u16 pressed = 0, held = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			touchRead(&touch);
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
			else	arrowX = 5;
		} else if(held & KEY_RIGHT && arrowY != -1) {
			if(arrowX < 5)	arrowX++;
			else	arrowX = 0;
		}
		if(held & KEY_LEFT && arrowY == -1) {
			goto switchBoxLeft;
		} else if(held & KEY_RIGHT && arrowY == -1) {
			goto switchBoxRight;
		}
		if(held & KEY_L || (touch.px > 6 && touch.px < 26 && touch.py > 19 && touch.py < 37)) {
			switchBoxLeft:
			if(currentBox() > 0)
				(topScreen ? currentBankBox : currentSaveBox)--;
			else if(topScreen) currentBankBox = Banks::bank->boxes()-1;
			else currentSaveBox = save->maxBoxes()-1;
			drawBox(topScreen, true);
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		} else if(held & KEY_R || (touch.px > 141 && touch.px < 161 && touch.py > 19 && touch.py < 37)) {
			switchBoxRight:
			if((topScreen ? currentBankBox < Banks::bank->boxes()-1 : currentSaveBox < save->maxBoxes()-1))
				(topScreen ? currentBankBox : currentSaveBox)++;
			else (topScreen ? currentBankBox : currentSaveBox) = 0;
			drawBox(topScreen, true);
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen)
				setSpriteVisibility(heldPokemon, false);
		}
		if(pressed & KEY_A) {
			selection:
			Sound::play(Sound::click);
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
							drawBox(topScreen, true);
							if(heldPokemonScreen != topScreen)	drawBox(heldPokemonScreen, true);
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
		} else if(pressed & KEY_TOUCH) {
			for(int x=0;x<6;x++) {
				for(int y=0;y<6;y++) {
					if(touch.px > 8+(x*24) && touch.px < 8+((x+1)*24) && touch.py > 32+(y*24) && touch.py < 32+((y+1)*24)) {
						if(arrowX == x && arrowY == y && topScreen == false)	goto selection;
						else {
							if(topScreen) {
								topScreen = false;
								setSpriteVisibility(topArrowID, false);
								setSpriteVisibility(topHeldPokemonID, false);
								setSpriteVisibility(bottomArrowID, true);
								setSpriteVisibility(bottomHeldPokemonID, true);
							}
							arrowX = x;
							arrowY = y;
						}
					}
				}
			}
		}

		if(pressed & KEY_X && heldPokemon == -1) {
			Sound::play(Sound::click);
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

		if((held & KEY_UP || held & KEY_DOWN || held & KEY_LEFT || held & KEY_RIGHT || held & KEY_L || held & KEY_R || held & KEY_TOUCH) && heldPokemon == -1) {
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
