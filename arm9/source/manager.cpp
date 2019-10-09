#include "manager.hpp"
#include <dirent.h>

#include "banks.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "loading.hpp"
#include "aMenu.hpp"
#include "xMenu.hpp"
#include "sound.hpp"

bool topScreen;
int bottomArrowID, topArrowID, shinyID, currentSaveBox, currentBankBox, bottomHeldPokemonID, topHeldPokemonID, arrowMode = 0;
std::vector<int> menuIconID, partyIconID;
std::string savePath;
std::vector<u16> arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgTop, boxButton, fileBrowseBg, infoBox, menuBg, menuButton, menuButtonBlue, menuIconSheet, optionsBg, search, shiny, summaryBg, types;
ImageData ballSheetData, bankBoxData, boxBgTopData, boxButtonData, fileBrowseBgData, infoBoxData, menuBgData, menuButtonData, menuButtonBlueData, menuIconSheetData, optionsBgData, searchData, shinyData, summaryBgData, typesData;
FILE* pokemonSheet;

struct HeldPkm {
	std::shared_ptr<PKX> pkm;
	int position, x, y;
};

int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int slot) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), slot);
	else	return save->pkm(currentBox(), slot);
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
	} else if(species == 351) { // Castform
		if(alternativeForm > 0)
			return 678+alternativeForm;
	} else if(species == 386) { // Deoxys
		if(alternativeForm > 0)
			return 681+alternativeForm;
	} else if(species == 412) { // Burmy
		if(alternativeForm > 0)
			return 684+alternativeForm;
	} else if(species == 413) { // Wormadam
		if(alternativeForm > 0)
			return 686+alternativeForm;
	} else if(species == 422) { // Shellos
		if(alternativeForm == 1)
			return 689;
	} else if(species == 423) { // Gastrodon
		if(alternativeForm == 1)
			return 690;
	} else if(species == 479) { // Rotom
		if(alternativeForm > 0)
			return 690+alternativeForm;
	} else if(species == 487) { // Giratina
		if(alternativeForm == 1)
			return 696;
	} else if(species == 492) { // Shaymin
		if(alternativeForm == 1)
			return 697;
	} else if(species == 521) { // Unfezant
		if(gender == 1)
			return 698;
	} else if(species == 550) { // Basculin
		if(alternativeForm == 1)
			return 699;
	} else if(species == 555) { // Darmanitan
		if(alternativeForm == 1)
			return 700;
	} else if(species == 585) { // Deerling
		if(alternativeForm > 0)
			return 700+alternativeForm;
	} else if(species == 586) { // Sawsbuck
		if(alternativeForm > 0)
			return 703+alternativeForm;
	} else if(species == 592) { // Frillish
		if(gender == 1)
			return 707;
	} else if(species == 593) { // Jellicent
		if(gender == 1)
			return 708;
	} else if(species == 648) { // Meloetta
		if(alternativeForm == 1)
			return 709;
	} else if(species == 641) { // Tornadus
		if(alternativeForm == 1)
			return 710;
	} else if(species == 642) { // Thunderus
		if(alternativeForm == 1)
			return 711;
	} else if(species == 645) { // Landorus
		if(alternativeForm == 1)
			return 712;
	} else if(species == 646) { // Kyurem
		if(alternativeForm > 0)
			return 712+alternativeForm;
	} else if(species == 647) { // Keldeo
		if(alternativeForm == 1)
			return 715;
	}

	// Non-alternate form, return dex number
	return species;
}

ImageData loadPokemonSprite(int dexNo, std::vector<u16> &imageBuffer) {
	dexNo *= 714;
	ImageData imageData = {0, 0};

	if(pokemonSheet) {
		// Get width and height on image
		char buffer[4];
		fseek(pokemonSheet, dexNo+0x12, SEEK_SET); // Width
		fread(buffer, 4, 1, pokemonSheet);
		imageData.width = *(int*)&buffer[0];
		fseek(pokemonSheet, dexNo+0x16, SEEK_SET); // Height
		fread(buffer, 4, 1, pokemonSheet);
		imageData.height = *(int*)&buffer[0];

		// Load palette
		u32 palTemp[16];
		u16 pal[16];
		fseek(pokemonSheet, dexNo+0x89, SEEK_SET);
		fread(palTemp, 4, 16, pokemonSheet);
		for(int i=0;i<16;i++) {
			pal[i] = ((palTemp[i]>>27)&31) | ((palTemp[i]>>19)&31)<<5 | ((palTemp[i]>>11)&31)<<10 | 1<<15;
		}

		// Load pixels
		fseek(pokemonSheet, dexNo+0xA, SEEK_SET); // Get pixel start location
		fseek(pokemonSheet, dexNo+(u8)fgetc(pokemonSheet), SEEK_SET); // Seek to pixel start location
		u8 bmpImageBuffer[(imageData.width*imageData.height)/2];
		fread(bmpImageBuffer, 1, sizeof(bmpImageBuffer), pokemonSheet);
		for(int y=imageData.height-1; y>=0; y--) {
			u8* src = bmpImageBuffer+y*(imageData.width/2);
			for(unsigned x=0;x<imageData.width;x+=2) {
				u8 val = *(src++);
				if(pal[val>>4] == 0xfc1f) { // First nibble
					imageBuffer.push_back(0<<15);
				} else {
					imageBuffer.push_back(pal[val>>4]);
				}
				if(pal[val&0xF] == 0xfc1f) { // Second nibble
					imageBuffer.push_back(0<<15);
				} else {
					imageBuffer.push_back(pal[val&0xF]);
				}
			}
		}
	}
	return imageData;
}

void loadGraphics(void) {
	// Load images into RAM
	ballSheetData = loadPng("nitro:/graphics/ballSheet.png", ballSheet);
	boxBgTopData = loadPng("nitro:/graphics/boxBgTop.png", boxBgTop);
	boxButtonData = loadPng("nitro:/graphics/boxButton.png", boxButton);
	fileBrowseBgData = loadPng("nitro:/graphics/fileBrowseBg.png", fileBrowseBg);
	infoBoxData = loadPng("nitro:/graphics/infoBox.png", infoBox);
	menuButtonData = loadPng("nitro:/graphics/menuButton.png", menuButton);
	menuButtonBlueData = loadPng("nitro:/graphics/menuButtonBlue.png", menuButtonBlue);
	menuIconSheetData = loadPng("nitro:/graphics/menuIconSheet.png", menuIconSheet);
	searchData = loadPng("nitro:/graphics/search.png", search);
	shinyData = loadPng("nitro:/graphics/shiny.png", shiny);
	loadPng("nitro:/graphics/arrowBlue.png", arrowBlue);
	loadPng("nitro:/graphics/arrowRed.png", arrowRed);
	loadPng("nitro:/graphics/arrowYellow.png", arrowYellow);
	if(sdFound()) {
		menuBgData = loadPng("nitro:/graphics/menuBg.png", menuBg);
		optionsBgData = loadPng("nitro:/graphics/optionsBg.png", optionsBg);
		summaryBgData = loadPng("nitro:/graphics/summaryBg.png", summaryBg);
	}
	pokemonSheet = fopen("nitro:/graphics/pokemonSheet.bmps", "rb");

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
		prepareSprite(id, 0, 0, 0);
		setSpriteVisibility(id, false);
		partyIconID.push_back(id);
	}

	// Prepare bottom arrow sprite
	bottomArrowID = initSprite(SpriteSize_16x16, false);
	fillSpriteImage(bottomArrowID, arrowRed, 16*16);
	prepareSprite(bottomArrowID, 24, 36, 0);
	setSpriteVisibility(bottomArrowID, false);

	// Prepare top arrow sprite
	topArrowID = initSprite(SpriteSize_16x16, true);
	fillSpriteImage(topArrowID, arrowRed, 16*16);
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
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);
	

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

std::string boxBgPath(bool top, int box) {
	if(top)	return "nitro:/graphics/box/chest.png";
	std::string game;
	switch(save->game) {
		default:
		case Game::DP:
			game = "dp";
			break;
		case Game::Pt:
			game = box < 16 ? "dp" : "pt";
			break;
		case Game::HGSS:
			game = box < 16 ? "dp" : "hgss";
			break;
		case Game::BW:
			game = "bw";
			break;
		case Game::B2W2:
			game = box < 16 ? "bw" : "b2w2";
			break;
	}
	return "nitro:/graphics/box/"+game+"/"+std::to_string(box)+".png";
}

void drawBox(bool top) {
	// Load box image
	bankBox.clear();
	bankBoxData = loadPng(boxBgPath(top), bankBox);

	if(top) {
		// Hide all Pokémon sprites for bank box
		for(int i=0;i<30;i++) {
			setSpriteVisibility(i+30, false);
		}
		updateOam();
		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(Banks::bank->pkm(currentBankBox, i)->species() != 0) {
				std::vector<u16> bmp;
				loadPokemonSprite(getPokemonIndex(Banks::bank->pkm(currentBankBox, i)), bmp);
				fillSpriteImage(i+30, bmp);
				setSpriteVisibility(i+30, true);
			}
		}
		updateOam();

		// Draw box image
		drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, top);

		// Print box name
		printTextCenteredTintedMaxW(Banks::bank->boxName(currentBankBox), 110, 1, GRAY, -44, 20, true, true);
	} else {
		// Hide all Pokémon sprites for save box
		for(int i=0;i<30;i++) {
			setSpriteVisibility(i, false);
		}
		updateOam();
		for(int i=0;i<30;i++) {
			// Fill Pokémon Sprites
			if(save->pkm(currentSaveBox, i)->species() != 0) {
				std::vector<u16> bmp;
				loadPokemonSprite(getPokemonIndex(save->pkm(currentSaveBox, i)), bmp);
				fillSpriteImage(i, bmp);
				setSpriteVisibility(i, true);
			}
		}
		updateOam();

		// Draw box image
		drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, top);

		// Print box name
		printTextCenteredTintedMaxW(save->boxName(currentSaveBox), 110, 1, GRAY, -44, 20, false, true);
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
		drawImageFromSheet(170, 43-(((typesData.height/17)-12)/2), typesData.width, typesData.height/17, types, typesData.width, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*(typesData.height/17)), true);
		if(pkm->type1() != pkm->type2())
			drawImageFromSheet(205, 43-(((typesData.height/17)-12)/2), typesData.width, typesData.height/17, types, typesData.width, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*(typesData.height/17)), true);

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
		std::vector<u16> bmp;
		loadPokemonSprite(getPokemonIndex(pkm), bmp);
		fillSpriteImage(bottomHeldPokemonID, bmp);
		fillSpriteImage(topHeldPokemonID, bmp);
	}
}

void manageBoxes(void) {
	int arrowX = 0, arrowY = 0, heldPokemonBox = -1;
	std::vector<HeldPkm> heldPokemon;
	bool heldPokemonScreen = false, heldMode = false;
	topScreen = false, arrowMode = 0;
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
			drawBox(topScreen);
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
				for(unsigned i=0;i<heldPokemon.size();i++) {
					setSpriteVisibility(heldPokemon[i].position+(heldPokemonScreen ? 30 : 0), false);
				}
			}
		} else if(held & KEY_R || (touch.px > 141 && touch.px < 161 && touch.py > 19 && touch.py < 37)) {
			switchBoxRight:
			if((topScreen ? currentBankBox < Banks::bank->boxes()-1 : currentSaveBox < save->maxBoxes()-1))
				(topScreen ? currentBankBox : currentSaveBox)++;
			else (topScreen ? currentBankBox : currentSaveBox) = 0;
			drawBox(topScreen);
			if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
				for(unsigned i=0;i<heldPokemon.size();i++) {
					setSpriteVisibility(heldPokemon[i].position+(heldPokemonScreen ? 30 : 0), false);
				}
			}
		}
		if(pressed & KEY_A) {
			selection:
			Sound::play(Sound::click);
			if(arrowY == -1) {
				if(heldPokemon.size() == 0)	aMenu(-1, aMenuTopBarButtons, 1);
			} else {
				if(heldPokemon.size() > 0) {
					if(heldPokemon[0].position == (arrowY*6)+arrowX && heldPokemonBox == currentBox() && heldPokemonScreen == topScreen) {
						// If in the held Pokémon's previous spot, just put held Pokémon back down
						for(unsigned i=0;i<heldPokemon.size();i++)
							setSpriteVisibility((heldPokemonScreen ? heldPokemon[i].position+30 : heldPokemon[i].position), heldPokemon[i].pkm->species());
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
						heldPokemon.clear();
						heldPokemonBox = -1;
					} else if(!heldMode || currentPokemon((arrowY*6)+arrowX)->species() == 0) {
						int canPlace = true;
						for(unsigned i=0;i<heldPokemon.size();i++) {
							if(heldPokemon[i].x-heldPokemon[0].x > 5-arrowX)	canPlace = false;
							if(heldPokemon[i].y-heldPokemon[0].y > 4-arrowY)	canPlace = false;
							if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
								for(unsigned j=0;j<heldPokemon.size();j++) {
									if(((arrowY+heldPokemon[j].y)*6)+arrowX+heldPokemon[j].x == heldPokemon[i].position) {
										canPlace = false;
										break;
									}
								}
							}
							if(!canPlace)	break;
						}
						if(canPlace) {
							for(unsigned i=0;i<heldPokemon.size();i++) {
								if(topScreen || (heldPokemonScreen ? Banks::bank->pkm(heldPokemonBox, heldPokemon[i].position) : save->pkm(heldPokemonBox, heldPokemon[i].position))->species() <= save->maxSpecies()) {
									// If not copying / there isn't a Pokémon at the new spot, move Pokémon
									// Save the Pokémon at the cursor's postion to a temp variable
									std::shared_ptr<PKX> tempPkm;
									if(currentPokemon(((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x)->species() != 0)	tempPkm = currentPokemon(((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x);
									else	tempPkm = save->emptyPkm();
									// Write the held Pokémon to the cursor position
									if(topScreen)	Banks::bank->pkm(heldPokemon[i].pkm, currentBox(), ((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x);
									else {
										save->pkm(heldPokemon[i].pkm, currentBox(), ((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x, false);
										save->dex(heldPokemon[i].pkm);
									}
									// If not copying, write the cursor position's previous Pokémon to the held Pokémon's old spot
									if(!heldMode) {
										if(heldPokemonScreen)	Banks::bank->pkm(tempPkm, heldPokemonBox, heldPokemon[i].position);
										else {
											save->pkm(tempPkm, heldPokemonBox, heldPokemon[i].position, false);
											save->dex(tempPkm);
										}
									}
								}
							}
							// Hide the moving Pokémon
							setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, false);
							
							// Update the box(es) for the moved Pokémon
							drawBox(topScreen);
							if(heldPokemonScreen != topScreen)	drawBox(heldPokemonScreen);
							drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));

							// Not holding a Pokémon anymore
							heldPokemon.clear();
							heldPokemonBox = -1;
						}
					}
				} else if(arrowMode == 2) {
					int startX = arrowX, startY = arrowY;
					drawOutline(8+(startX*24), 40+(startY*24), (((arrowX+1)-startX)*24)+8, (((arrowY+1)-startY)*24), WHITE, topScreen);
					while(1) {
						do {
							swiWaitForVBlank();
							scanKeys();
							pressed = keysDown();
							held = keysDownRepeat();
						} while(!held);

						if(held & KEY_UP && arrowY > 0)			arrowY--;
						else if(held & KEY_DOWN && arrowY < 4)	arrowY++;
						if(held & KEY_LEFT && arrowX > 0)		arrowX--;
						else if(held & KEY_RIGHT && arrowX < 5)	arrowX++;
						if(pressed & KEY_A) {
							yellowSelection:
							drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, topScreen);
							printTextCenteredTinted((topScreen ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)), GRAY, -44, 20, topScreen, true);
							for(int y=std::min(startY, arrowY);y<std::max(startY,arrowY)+1;y++) {
								for(int x=std::min(startX, arrowX);x<std::max(startX,arrowX)+1;x++) {
									heldPokemon.push_back({currentPokemon((y*6)+x), (y*6)+x, x-std::min(startX, arrowX), y-std::min(startY, arrowY)});
									setSpriteVisibility((topScreen ? ((y*6)+x)+30 : (y*6)+x), false);
								}
							}
							fillSpriteColor(topHeldPokemonID, 0); // Fill the sprite with transparency
							fillSpriteText(topHeldPokemonID, StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())), GRAY, 32-getTextWidth(std::to_string(heldPokemon.size())), 16, true);
							fillSpriteColor(bottomHeldPokemonID, 0); // Fill the sprite with transparency
							fillSpriteText(bottomHeldPokemonID, StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())), GRAY, 32-getTextWidth(std::to_string(heldPokemon.size())), 16, true);
							setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, true);
							updateOam();
							heldPokemonBox = currentBox();
							heldPokemonScreen = topScreen;
							arrowX = std::min(startX, arrowX);
							arrowY = std::min(startY, arrowY);
							updateOam();
							break;
						} else if(pressed & KEY_B) {
							drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, topScreen);
							printTextCenteredTinted((topScreen ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)), GRAY, -44, 20, topScreen, true);
							drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));
							break;
						} else if(pressed & KEY_TOUCH) {
							touchRead(&touch);
							for(int x=0;x<6;x++) {
								for(int y=0;y<5;y++) {
									if(touch.px > 16+(x*24) && touch.px < 16+((x+1)*24) && touch.py > 40+(y*24) && touch.py < 40+((y+1)*24)) {
										if(arrowX == x && arrowY == y && topScreen == false)	goto yellowSelection;
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

						drawImage(5, 15, bankBoxData.width, bankBoxData.height, bankBox, topScreen);
						printTextCenteredTinted((topScreen ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)), GRAY, -44, 20, topScreen, true);
						drawOutline(8+(std::min(startX, arrowX)*24), 40+(std::min(startY, arrowY)*24), ((std::max(arrowX-startX, startX-arrowX)+1)*24)+8, ((std::max(arrowY-startY, startY-arrowY)+1)*24), WHITE, topScreen);
						setSpritePosition((topScreen ? topArrowID : bottomArrowID), (arrowX*24)+24, (arrowY*24)+36);
						updateOam();
					}
				} else if(currentPokemon((arrowY*6)+arrowX)->species() != 0) {
					int temp = 1;
					if(arrowMode == 1 || (temp = aMenu((arrowY*6)+arrowX, aMenuButtons, 0))) {
						// If no pokemon is currently held and there is one at the cursor, pick it up
						heldPokemon.push_back({currentPokemon((arrowY*6)+arrowX)->clone(), (arrowY*6)+arrowX, 0, 0});
						heldPokemonBox = currentBox();
						heldPokemonScreen = topScreen;
						heldMode = temp-1; // false = move, true = copy
						setHeldPokemon(currentPokemon(heldPokemon[0].position));
						if(!heldMode)	setSpriteVisibility(heldPokemonScreen ? heldPokemon[0].position+30 : heldPokemon[0].position, false);
						setSpriteVisibility(topScreen ? topHeldPokemonID : bottomHeldPokemonID, true);
						drawPokemonInfo(currentPokemon(heldPokemon[0].position));
					}
				} else if(arrowMode == 0) {
					aMenu((arrowY*6)+arrowX, aMenuEmptySlotButtons, 2);
				}
			}
		} else if(pressed & KEY_TOUCH) {
			for(int x=0;x<6;x++) {
				for(int y=0;y<5;y++) {
					if(touch.px > 16+(x*24) && touch.px < 16+((x+1)*24) && touch.py > 40+(y*24) && touch.py < 40+((y+1)*24)) {
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
			if((touch.px > 26 && touch.px < 141 && touch.py > 19 && touch.py < 37)) {
				arrowY = -1;
				goto selection;
			}
		}

		if(pressed & KEY_X && heldPokemon.size() == 0) {
			Sound::play(Sound::click);
			if(!xMenu())	break;
		}

		if(arrowY == -2) {
			// If the Arrow Y is at -2, switch to the top screen
			arrowY = 4;
			topScreen = true;
			setSpriteVisibility(bottomArrowID, false);
			setSpriteVisibility(topArrowID, true);
			if(heldPokemon.size()) {
				setSpriteVisibility(bottomHeldPokemonID, false);
				setSpriteVisibility(topHeldPokemonID, true);
			}
		} else if(arrowY == 5) {
			// If the Arrow Y is at 5, switch to the bottom screen
			arrowY = -1;
			topScreen = false;
			setSpriteVisibility(bottomArrowID, true);
			setSpriteVisibility(topArrowID, false);
			if(heldPokemon.size()) {
				setSpriteVisibility(bottomHeldPokemonID, true);
				setSpriteVisibility(topHeldPokemonID, false);
			}

		}

		if(pressed & KEY_SELECT && heldPokemon.size() == 0) {
			if(arrowMode < 2)	arrowMode++;
			else	arrowMode = 0;

			if(arrowMode == 0) {
				fillSpriteImage(bottomArrowID, arrowRed, 16*16);
				fillSpriteImage(topArrowID, arrowRed, 16*16);
			} else if(arrowMode == 1) {
				fillSpriteImage(bottomArrowID, arrowBlue, 16*16);
				fillSpriteImage(topArrowID, arrowBlue, 16*16);
			} else {
				fillSpriteImage(bottomArrowID, arrowYellow, 16*16);
				fillSpriteImage(topArrowID, arrowYellow, 16*16);
			}
		}

		if((held & KEY_UP || held & KEY_DOWN || held & KEY_LEFT || held & KEY_RIGHT || held & KEY_L || held & KEY_R || held & KEY_TOUCH) && heldPokemon.size() == 0) {
			// If the cursor is moved and we're not holding a Pokémon, draw the new one
			if(arrowY != -1)	drawPokemonInfo(currentPokemon((arrowY*6)+arrowX));
			else	drawPokemonInfo(save->emptyPkm());
		}

		if(arrowY == -1) {
			// If the Arrow Y is at -1 (box title), draw it in the middle
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), 90, 16);
			if(heldPokemon.size())	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), 82, 12);
		} else {
			// Otherwise move it to the spot in the box it's at
			setSpritePosition((topScreen ? topArrowID : bottomArrowID), (arrowX*24)+24, (arrowY*24)+36);
			if(heldPokemon.size())	setSpritePosition((topScreen ? topHeldPokemonID : bottomHeldPokemonID), (arrowX*24)+16, (arrowY*24)+32);
		}
		updateOam();
	}
}
