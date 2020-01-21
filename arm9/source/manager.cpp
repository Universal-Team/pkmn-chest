#include "manager.hpp"
#include <dirent.h>

#include "aMenu.hpp"
#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "filter.hpp"
#include "flashcard.hpp"
#include "input.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "loading.hpp"
#include "party.hpp"
#include "PKFilter.hpp"
#include "PKX.hpp"
#include "sort.hpp"
#include "sound.hpp"
#include "xMenu.hpp"

bool topScreen, inParty;
int arrowID = 126, currentSaveBox, currentBankBox, heldPokemonID = 125, keyboardSpriteID = 124, arrowMode = 0;
std::vector<int> menuIconID, partyIconID;
std::string savePath;
Image arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgTop, boxButton, infoBox, keyboardKey, listBg, menuBg, menuButton, menuButtonBlue, party, search, setToSelf, shiny;
std::vector<Image> types;
FILE* pokemonGFX;
std::shared_ptr<PKFilter> filter = std::make_shared<PKFilter>();

struct HeldPkm {
	std::shared_ptr<PKX> pkm;
	int position, x, y;
};

int currentBox(void) {
	return topScreen ? currentBankBox : currentSaveBox;
}

std::shared_ptr<PKX> currentPokemon(int x, int y) {
	if(topScreen)	return Banks::bank->pkm(currentBox(), (y*6)+x);
	else if(inParty)	return save->pkm((y*2)+x);
	else	return save->pkm(currentBox(), (y*6)+x);
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

Image loadPokemonSprite(int dexNo) {
	Image image = {0, 0};
	u16 palCount, palOfs;
	if(pokemonGFX) {
		fseek(pokemonGFX, (dexNo*0x42C)+4, SEEK_SET);
		fread(&image.width, 1, 2, pokemonGFX);
		fread(&image.height, 1, 2, pokemonGFX);
		image.bitmap = std::vector<u8>(image.width*image.height);
		fread(image.bitmap.data(), 1, image.width*image.height, pokemonGFX);
		fread(&palCount, 1, 2, pokemonGFX);
		fread(&palOfs, 1, 2, pokemonGFX);
		image.palette = std::vector<u16>(palCount);
		fread(image.palette.data(), 2, palCount, pokemonGFX);
	}

	return image;
}

void fillArrow(int arrowMode) {
	if(arrowMode == 0) {
		fillSpriteImage(arrowID, false, 16, 0, 0, arrowRed);
		fillSpriteImage(arrowID, true, 16, 0, 0, arrowRed);
	} else if(arrowMode == 1) {
		fillSpriteImage(arrowID, false, 16, 0, 0, arrowBlue);
		fillSpriteImage(arrowID, true, 16, 0, 0, arrowBlue);
	} else {
		fillSpriteImage(arrowID, false, 16, 0, 0, arrowYellow);
		fillSpriteImage(arrowID, true, 16, 0, 0, arrowYellow);
	}
}

void initSprites(void) {
	// Pokémon Sprites
	for(int y=0;y<5;y++) {
		for(int x=0;x<6;x++) {
			initSprite(true, SpriteSize_32x32);
			initSprite(false, SpriteSize_32x32);
			prepareSprite((y*6)+x,  true, 8+(x*24), 32+(y*24), 3);
			prepareSprite((y*6)+x, false, 8+(x*24), 32+(y*24), 3);
		}
	}

	// Menu icon sprites
	for(int i=0;i<6;i++) {
		int id = initSprite(false, SpriteSize_32x32);
		prepareSprite(id, false, 0, 0, 0);
		setSpriteVisibility(id, false, false);
		menuIconID.push_back(id);
	}

	// Party sprites
	for(int i=0;i<6;i++) {
		int id = initSprite(false, SpriteSize_32x32);
		prepareSprite(id, false, 0, 0, 2);
		setSpriteVisibility(id, false, false);
		partyIconID.push_back(id);
	}

	// Bottom arrow sprite
	initSprite(false, SpriteSize_16x16, arrowID);
	prepareSprite(arrowID, false, 24, 36, 0);
	setSpriteVisibility(arrowID, false, false);

	// Top arrow sprite
	initSprite(true, SpriteSize_16x16, arrowID);
	prepareSprite(arrowID, true, 24, 36, 0);
	setSpriteVisibility(arrowID, true, false);

	// Bottom sprite for moving pokemon
	initSprite(false, SpriteSize_32x32, heldPokemonID);
	prepareSprite(heldPokemonID, false, 0, 0, 1);
	setSpriteVisibility(heldPokemonID, false, false);

	// Top sprite for moving pokemon
	initSprite(true, SpriteSize_32x32, heldPokemonID);
	prepareSprite(heldPokemonID, true, 0, 0, 1);
	setSpriteVisibility(heldPokemonID, true, false);

	// Keyboard button sprite
	initSprite(false, SpriteSize_32x32, keyboardSpriteID);
	prepareSprite(keyboardSpriteID, false, 0, 0, 0);
	setSpriteVisibility(keyboardSpriteID, false, false);
}

void loadGraphics(void) {
	// Load images into RAM
	arrowBlue = loadImage("/graphics/arrowBlue.gfx");
	arrowRed = loadImage("/graphics/arrowRed.gfx");
	arrowYellow = loadImage("/graphics/arrowYellow.gfx");
	ballSheet = loadImage("/graphics/ballSheet.gfx");
	boxBgTop = loadImage("/graphics/boxBgTop.gfx");
	boxButton = loadImage("/graphics/boxButton.gfx");
	infoBox = loadImage("/graphics/infoBox.gfx");
	keyboardKey = loadImage("/graphics/keyboardKey.gfx");
	listBg = loadImage("/graphics/listBg.gfx");
	menuBg = loadImage("/graphics/menuBg.gfx");
	menuButton = loadImage("/graphics/menuButton.gfx");
	menuButtonBlue = loadImage("/graphics/menuButtonBlue.gfx");
	Image menuIconSheet = loadImage("/graphics/menuIconSheet.gfx");
	party = loadImage("/graphics/party.gfx");
	search = loadImage("/graphics/search.gfx");
	setToSelf = loadImage("/graphics/setToSelf.gfx");
	shiny = loadImage("/graphics/shiny.gfx");

	// Open Pokémon combo gfx file
	if(pokemonGFX)	fclose(pokemonGFX);
	pokemonGFX = fopen((Config::getString("themeDir")+"/graphics/pokemon.combo.gfx").c_str(), "rb");
	if(!pokemonGFX) {
		pokemonGFX = fopen("nitro:/graphics/pokemon.combo.gfx", "rb");
	}

	// Fill menu icon sprites
	for(int i=1;i<6;i++) { // 0 (party) skiped until I know what to replace it with
		fillSpriteSegment(menuIconID[i], false, 32, 0, 0, 32, 32, menuIconSheet, 0, i*32);
	}

	// Fill arrow sprites
	fillArrow(0);
}

void drawBoxScreen(void) {
	// Draws backgrounds
	drawImageDMA(0, 0, boxBgTop, true, false);
	drawImageSegment(164, 2, infoBox.width, infoBox.height-16, infoBox, 0, 0, true, false);
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, true, true);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Show bottom arrow
	setSpriteVisibility(arrowID, false, true);

	// Move the arrow back to 24, 36
	setSpritePosition(arrowID, false, 24, 36);

	// Draw the boxes and Pokémon
	drawBox(true);
	drawBox(false);

	// Draw first Pokémon's info
	drawPokemonInfo(save->pkm(currentBox(), 0));
}

std::string boxBgPath(bool top, int box) {
	if(top)	return "/graphics/box/chest.gfx";
	std::string game;
	switch(save->game) {
		default:
		case Game::RS:
			game = "rs";
			break;
		case Game::E:
			game = "e";
		case Game::FRLG:
			game = box < 12 ? "rs" : "lgfr";
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
	return "/graphics/box/"+game+"/"+std::to_string(box)+".gfx";
}

void drawBox(bool top) {
	// Load box image
	bankBox = loadImage(boxBgPath(top));

	for(int i=0;i<30;i++) {
		// Fill Pokémon Sprites
		std::shared_ptr<PKX> tempPkm = (top ? Banks::bank->pkm(currentBankBox, i) : save->pkm(currentSaveBox, i));
		if(tempPkm->species() != 0) {
			Image image = loadPokemonSprite(getPokemonIndex(tempPkm));
			fillSpriteImage(i, top, 32, 0, 0, image);
			setSpriteVisibility(i, top, true);
			if(!(partyShown && arrowMode == 0))
				setSpriteAlpha(i, top, (*tempPkm == *filter) ? 15 : 8);
		} else {
			setSpriteVisibility(i, top, false);
		}
	}
	updateOam();

	// Draw box image
	drawImage(5, save->generation() == Generation::THREE ? 40 : 15, bankBox, top, false);
	// TODO: Add top bar for gen 3, it has additional palettes so it has to be split

	// Print box name
	printTextCenteredTintedMaxW((top ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)), 110, 1, TextColor::gray, -44, 20, top, false);

	if(!top) {
		drawImage(boxButton.width+5, 192-search.height, search, false, false);
		drawImage(0, 192-boxButton.height, boxButton, false, false);
		printTextMaxW(Lang::get("party"), boxButton.width-8, 1, 4, 192-boxButton.height+4, false, false);
	}
}

void drawPokemonInfo(std::shared_ptr<PKX> pkm) {
	// Clear previous draw
	drawImageSegment(164, 2, infoBox.width, infoBox.height-16, infoBox, 0, 0, true, false);
	drawRectangle(164, 2, infoBox.width, infoBox.height-16, CLEAR, true, true);

	if(pkm->species() > 0 && pkm->species() < 650) {
		// Show shiny star if applicable
		if(pkm->shiny())	drawImage(239, 45, shiny, true, false);

		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "%s%.3i", Lang::get("dexNo").c_str(), pkm->species());
		printTextTinted(str, TextColor::gray, 170, 8, true, true);

		// Print name
		if(pkm->nicknamed())	printTextTintedMaxW(pkm->nickname(), 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? TextColor::red : TextColor::gray) : TextColor::blue), 170, 25, true, true);
		else	printTextTintedMaxW(Lang::species[pkm->species()], 80, 1, (pkm->gender() ? (pkm->gender() == 1 ? TextColor::red : TextColor::gray) : TextColor::blue), 170, 25, true, true);

		// Draw types
		int type = (pkm->generation() < Generation::FIVE && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1();
		drawImage(170, 42-((types[type].height-12)/2), types[type], true, false);
		if(pkm->type1() != pkm->type2()) {
			type = (pkm->generation() < Generation::FIVE && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2();
			drawImage(205, 42-((types[type].height-12)/2), types[type], true, false, 4);
		}

		// Print Level
		printTextTinted(Lang::get("lv")+std::to_string(pkm->level()), TextColor::gray, 170, 57, true, true);
	}
}

void setHeldPokemon(std::shared_ptr<PKX> pkm) {
	if(pkm->species() != 0) {
		Image image = loadPokemonSprite(getPokemonIndex(pkm));
		fillSpriteImage(heldPokemonID, false, 32, 0, 0, image);
		fillSpriteImage(heldPokemonID, true, 32, 0, 0, image);
	}
}

void manageBoxes(void) {
	filter->ball(1);
	int arrowX = 0, arrowY = 0, heldPokemonBox = -1;
	std::vector<HeldPkm> heldPokemon;
	bool heldPokemonScreen = false, heldInParty = false, heldMode = false;
	topScreen = false, arrowMode = 0;
	u16 pressed, held;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(!inParty) {
			if(held & KEY_UP) {	
				if(arrowY > (topScreen ? -1 : -2))	arrowY--;
			} else if(held & KEY_DOWN) {
				if(arrowY < 5)	arrowY++;
			}
			if(held & KEY_LEFT && arrowY != -1) {
				if(arrowX > 0) {
					arrowX--;
				} else if(!topScreen && partyShown) {
					arrowX = 1;
					inParty = true;

					arrowY = (arrowY < 3 ? arrowY/2 : 2);
				} else {
					arrowX = 5;
				}
			} else if(held & KEY_RIGHT && arrowY != -1) {
				if(arrowX < 5) {
					arrowX++;
				} else if(!topScreen && partyShown) {
					arrowX = 0;
					inParty = true;

					arrowY = (arrowY < 3 ? arrowY/2 : 2);
				} else {
					arrowX = 0;
				}
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
						setSpriteVisibility(heldPokemon[i].position, heldPokemonScreen, false);
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
						setSpriteVisibility(heldPokemon[i].position, heldPokemonScreen, false);
					}
				}
			}
		} else { // Cursor movement in party tray
			if(held & KEY_UP) {
				if(arrowY == 6)	arrowY = 2;
				else if(arrowY > 0)	arrowY--;
				else	arrowY = 6;
			} else if(held & KEY_DOWN) {
				if(arrowY < 2)	arrowY++;
				else if(arrowY == 2)	arrowY = 6;
				else	arrowY = 0;
			}
			if(arrowMode == 0 && heldPokemon.size() == 0) { // Party tray is over the box
				if(held & KEY_LEFT) {
					if(arrowY > 0 || arrowX % 2) {
						if(arrowX % 2) {
							arrowX--;
						} else {
							arrowX++;
							arrowY--;
						}
					}
				} else if(held & KEY_RIGHT) {
					if(arrowY < 3) {
						if(arrowX % 2 && arrowY == 2) {
							arrowY = 6;
						} else if(arrowX % 2) {
							arrowX--;
							arrowY++;
						} else {
							arrowX++;
						}
					} else {
						arrowX = 0, arrowY = 0;
					}
				}
			} else { // Party box is to the right
				if(held & KEY_LEFT) {
					if(arrowX == 1) {
						arrowX--;
					} else {
						arrowX = 5;
						inParty = false;
						arrowY = (arrowY < 2 ? arrowY*2 : 3);
					}
				} else if(held & KEY_RIGHT) {
					if(arrowX == 0) {
						arrowX++;
					} else {
						arrowX = 0;
						inParty = false;
						arrowY = (arrowY < 2 ? arrowY*2 : 3);
					}
				}
			}
		}

		if(pressed & KEY_A) {
			selection:
			Sound::play(Sound::click);
			if(arrowY == -1) {
				if(heldPokemon.size() == 0)	aMenu(-1, -1, aMenuTopBarButtons, 1);
			} else if(arrowY < 5) {
				if(heldPokemon.size() > 0) {
					if(heldPokemon[0].position == (arrowY*6)+arrowX && heldPokemonBox == currentBox() && heldPokemonScreen == topScreen) {
						// If in the held Pokémon's previous spot, just put held Pokémon back down
						for(unsigned i=0;i<heldPokemon.size();i++) {
							if(heldInParty)	setSpriteVisibility(partyIconID[heldPokemon[i].position], heldPokemonScreen, heldPokemon[i].pkm->species());
							else	setSpriteVisibility(heldPokemon[i].position, heldPokemonScreen, heldPokemon[i].pkm->species());
						}
						setSpriteVisibility(heldPokemonID, topScreen, false);
						heldPokemon.clear();
						heldPokemonBox = -1;
						if(partyShown && !topScreen)	moveParty(arrowMode, heldPokemon.size() > 0);
					} else if(!heldMode || currentPokemon(arrowX, arrowY)->species() == 0) {
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
								if(topScreen || save->availableSpecies().count(heldPokemon[i].pkm->species()) != 0) {
									// If not copying / there isn't a Pokémon at the new spot, move Pokémon
									// Save the Pokémon at the cursor's postion to a temp variable
									std::shared_ptr<PKX> tempPkm;
									if(currentPokemon(arrowX+heldPokemon[i].x, arrowY+heldPokemon[i].y)->species() != 0)	tempPkm = currentPokemon(arrowX+heldPokemon[i].x, arrowY+heldPokemon[i].y);
									else	tempPkm = save->emptyPkm();
									// Write the held Pokémon to the cursor position
									if(topScreen) {
										Banks::bank->pkm(heldPokemon[i].pkm, currentBox(), ((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x);
									} else if(inParty) {
										save->pkm(save->transfer(heldPokemon[i].pkm), ((arrowY+heldPokemon[i].y)*2)+arrowX+heldPokemon[i].x);
										save->dex(heldPokemon[i].pkm);
									} else {
										save->pkm(save->transfer(heldPokemon[i].pkm), currentBox(), ((arrowY+heldPokemon[i].y)*6)+arrowX+heldPokemon[i].x, false);
										save->dex(heldPokemon[i].pkm);
									}
									// If not copying, write the cursor position's previous Pokémon to the held Pokémon's old spot
									if(!heldMode) {
										if(heldPokemonScreen) {
											Banks::bank->pkm(tempPkm, heldPokemonBox, heldPokemon[i].position);
										} else if(heldInParty) {
											save->pkm(save->transfer(tempPkm), heldPokemon[i].position);
											save->dex(tempPkm);
										} else {
											save->pkm(save->transfer(tempPkm), heldPokemonBox, heldPokemon[i].position, false);
											save->dex(tempPkm);
										}
									}
								}
							}
							save->fixParty();

							// Hide the moving Pokémon
							setSpriteVisibility(heldPokemonID, topScreen, false);
							
							// Update the box(es) for the moved Pokémon
							drawBox(topScreen);
							if(heldPokemonScreen != topScreen)	drawBox(heldPokemonScreen);
							drawPokemonInfo(currentPokemon(arrowX, arrowY));
							if(partyShown)	fillPartySprites();

							// Not holding a Pokémon anymore
							heldPokemon.clear();
							heldPokemonBox = -1;

							// Move the party tray back
							if(partyShown) {
								moveParty(arrowMode, heldPokemon.size() > 0);
								if(arrowMode == 0 && !inParty) {
									if(topScreen) {
										topScreen = false;
										setSpriteVisibility(arrowID, true, false);
										setSpriteVisibility(heldPokemonID, true, false);
										setSpriteVisibility(arrowID, false, true);
										setSpriteVisibility(heldPokemonID, false, heldPokemon.size() > 0);
									}
									inParty = true;
									arrowX = 0, arrowY = 0;
								}
							}
						}
					}
				} else if(arrowMode == 2) {
					bankBox = loadImage(boxBgPath(topScreen));
					int startX = arrowX, startY = arrowY;
					if(inParty)	drawOutline(PARTY_TRAY_X+(partySpritePos[(startY*2)+startX].first), PARTY_TRAY_Y+(partySpritePos[(startY*2)+startX].second)+8, 32, 24, DARKER_GRAY, topScreen, true);
					else	drawOutline(8+(startX*24), 40+(startY*24), (((arrowX+1)-startX)*24)+8, 24, WHITE, topScreen, false);
					while(1) {
						do {
							swiWaitForVBlank();
							scanKeys();
							pressed = keysDown();
							held = keysDownRepeat();
						} while(!held);

						if(held & KEY_UP && arrowY > 0)		arrowY--;
						else if(held & KEY_DOWN && arrowY < (inParty ? 2 : 4))	arrowY++;
						if(held & KEY_LEFT && arrowX > 0)	arrowX--;
						else if(held & KEY_RIGHT && arrowX < (inParty ? 1 : 5))	arrowX++;
						if(pressed & KEY_A) {
							yellowSelection:
							if(inParty)	drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
							else	drawImageSegment(5, 15+20, bankBox.width, bankBox.height-20, bankBox, 0, 20, topScreen, false);
							for(int y=std::min(startY, arrowY);y<std::max(startY,arrowY)+1;y++) {
								for(int x=std::min(startX, arrowX);x<std::max(startX,arrowX)+1;x++) {
									heldPokemon.push_back({currentPokemon(x, y), (y*(inParty ? 2 : 6))+x, x-std::min(startX, arrowX), y-std::min(startY, arrowY)});
									if(inParty)	setSpriteVisibility(partyIconID[(y*2)+x], topScreen, false);
									else	setSpriteVisibility((y*6)+x, topScreen, false);
								}
							}
							fillSpriteColor(heldPokemonID, true, 0); // Fill the sprite with transparency
							fillSpriteImage(heldPokemonID, true, 32, 16, 16, keyboardKey);
							fillSpriteText(heldPokemonID, true, StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())), TextColor::white, 24-(getTextWidth(std::to_string(heldPokemon.size()))/2), 16);
							fillSpriteColor(heldPokemonID, false, 0); // Fill the sprite with transparency
							fillSpriteImage(heldPokemonID, false, 32, 16, 16, keyboardKey);
							fillSpriteText(heldPokemonID, false, StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())), TextColor::white, 24-(getTextWidth(std::to_string(heldPokemon.size()))/2), 16);
							setSpriteVisibility(heldPokemonID, topScreen, true);
							updateOam();
							heldPokemonBox = currentBox();
							heldPokemonScreen = topScreen;
							heldInParty = inParty;
							arrowX = std::min(startX, arrowX);
							arrowY = std::min(startY, arrowY);
							updateOam();
							break;
						} else if(pressed & KEY_B) {
							if(inParty)	drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
							else	drawImageSegment(5, 15+20, bankBox.width, bankBox.height-20, bankBox, 0, 20, topScreen, false);
							drawPokemonInfo(currentPokemon(arrowX, arrowY));
							break;
						} else if(pressed & KEY_TOUCH) {
							touchRead(&touch);
							for(int x=0;x<6;x++) {
								for(int y=0;y<5;y++) {
									if(touch.px > 16+(x*24) && touch.px < 16+((x+1)*24) && touch.py > 40+(y*24) && touch.py < 40+((y+1)*24)) {
										if(arrowX == x && arrowY == y && topScreen == false)	goto yellowSelection;
										else {
											if(!topScreen && !inParty) {
												arrowX = x;
												arrowY = y;
											}
										}
									}
								}
							}
							if(partyShown) {
								for(int i=0;i<6;i++) {
									if(touch.px >= PARTY_TRAY_X+partyX+partySpritePos[i].first && touch.px <= PARTY_TRAY_X+partyX+partySpritePos[i].first+32
									&& touch.py >= PARTY_TRAY_Y+partyY+partySpritePos[i].second && touch.py <= PARTY_TRAY_Y+partyY+partySpritePos[i].second+32) {
										if(arrowX == i-((i/2)*2) && arrowY == i/2 && inParty == true)	goto yellowSelection;
										else {
											if(inParty) {
												arrowX = i-((i/2)*2);
												arrowY = i/2;
											}
										}
									}
								}
							}
						}

						if(inParty) {
							drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
							drawOutline(PARTY_TRAY_X+(partySpritePos[(std::min(startY, arrowY)*2)+std::min(startX, arrowX)].first),
										PARTY_TRAY_Y+(partySpritePos[(std::min(startY, arrowY)*2)+std::min(startX, arrowX)].second)+8,
										(std::max(arrowX-startX, startX-arrowX) == 0 ? 32 : 72),
										((std::max(arrowY-startY, startY-arrowY)+1)*24 + (std::max(arrowY-startY, startY-arrowY))*8 + (std::max(arrowX-startX, startX-arrowX) == 0 ? 0 : 8)), DARKER_GRAY, topScreen, true);
							setSpritePosition(arrowID, topScreen, PARTY_TRAY_X+partySpritePos[(arrowY*2)+(arrowX)].first+partyX+16, PARTY_TRAY_Y+partySpritePos[(arrowY*2)+(arrowX)].second+partyY+4);
						} else {
							drawImageSegment(5, 15+20, bankBox.width, bankBox.height-20, bankBox, 0, 20, topScreen, false);
							drawOutline(8+(std::min(startX, arrowX)*24), 40+(std::min(startY, arrowY)*24), ((std::max(arrowX-startX, startX-arrowX)+1)*24)+8, ((std::max(arrowY-startY, startY-arrowY)+1)*24), WHITE, topScreen, false);
							setSpritePosition(arrowID, topScreen, (arrowX*24)+24, (arrowY*24)+36);
						}
						updateOam();
					}
				} else if(currentPokemon(arrowX, arrowY)->species() != 0) {
					int temp = 1;
					if(arrowMode == 1 || (temp = aMenu(arrowX, arrowY, aMenuButtons, 0))) {
						// If no pokemon is currently held and there is one at the cursor, pick it up
						heldPokemon.push_back({currentPokemon(arrowX, arrowY)->clone(), (arrowY*(inParty ? 2 : 6))+arrowX, 0, 0});
						heldPokemonBox = currentBox();
						heldPokemonScreen = topScreen;
						heldInParty = inParty;
						heldMode = temp-1; // false = move, true = copy
						setHeldPokemon(heldPokemon[0].pkm);

						moveParty(arrowMode, heldPokemon.size() > 0);

						if(!heldMode) {
							if(heldInParty)	setSpriteVisibility(partyIconID[heldPokemon[0].position], heldPokemonScreen, false);
							else	setSpriteVisibility(heldPokemon[0].position, heldPokemonScreen, false);
						}
						setSpriteVisibility(heldPokemonID, topScreen, true);
						drawPokemonInfo(heldPokemon[0].pkm);
					}
				} else if(arrowMode == 0) {
					aMenu(arrowX, arrowY, aMenuEmptySlotButtons, 2);
				}
			} else {
				toggleParty:
				toggleParty();
				if(partyShown) {
					if(topScreen) {
						topScreen = false;
						setSpriteVisibility(arrowID, true, false);
					}
					arrowX = 0, arrowY = 0;
					inParty = true;
					drawPokemonInfo(save->pkm(0));
				} else {
					if(inParty) {
						arrowX = 0;
						arrowY = 5;
						inParty = false;
					}
					drawPokemonInfo(save->emptyPkm());
				}
			}
		} else if(pressed & KEY_B) {
			if(partyShown) {
				Sound::play(Sound::back);
				goto toggleParty;
			}
		} else if(pressed & KEY_Y && !partyShown) {
			if(heldPokemon.size() == 0) {
				filter:
				if(Input::getBool(Lang::get("filter"), Lang::get("sort"))) {
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					changeFilter(filter);
				} else {
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					sortMenu(topScreen);
				}

				// Redraw
				setSpriteVisibility(arrowID, false, !topScreen);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				drawPokemonInfo(currentPokemon(arrowX, arrowY));
				if(topScreen)	drawBox(topScreen);
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(!(partyShown && arrowMode == 0 && heldPokemon.size() == 0)) {
				for(int x=0;x<6;x++) {
					for(int y=0;y<5;y++) {
						if(touch.px > 16+(x*24) && touch.px < 16+((x+1)*24) && touch.py > 40+(y*24) && touch.py < 40+((y+1)*24)) {
							if(arrowX == x && arrowY == y && topScreen == false)	goto selection;
							else {
								if(topScreen) {
									topScreen = false;
									setSpriteVisibility(arrowID, true, false);
									setSpriteVisibility(heldPokemonID, true, false);
									setSpriteVisibility(arrowID, false, true);
									setSpriteVisibility(heldPokemonID, false, heldPokemon.size() > 0);
								}
								inParty = false;
								arrowX = x;
								arrowY = y;
							}
						}
					}
				}
				if((touch.px > 26 && touch.px < 141 && touch.py > 19 && touch.py < 37)) {
					arrowY = -1;
					goto selection;
				} else if(touch.px >= boxButton.width+5 && touch.px <= boxButton.width+5+search.width && touch.py >= 192-search.height) {
					goto filter;
				}
			}
			if(partyShown) {
				for(int i=0;i<6;i++) {
					if(touch.px >= PARTY_TRAY_X+partyX+partySpritePos[i].first && touch.px <= PARTY_TRAY_X+partyX+partySpritePos[i].first+32
					&& touch.py >= PARTY_TRAY_Y+partyY+partySpritePos[i].second && touch.py <= PARTY_TRAY_Y+partyY+partySpritePos[i].second+32) {
						if(arrowX == i-((i/2)*2) && arrowY == i/2 && inParty == true)	goto selection;
						else {
							if(topScreen) {
								topScreen = false;
								setSpriteVisibility(arrowID, true, false);
								setSpriteVisibility(heldPokemonID, true, false);
								setSpriteVisibility(arrowID, false, true);
								setSpriteVisibility(heldPokemonID, false, heldPokemon.size() > 0);
							}
							inParty = true;
							arrowX = i-((i/2)*2);
							arrowY = i/2;
						}
					}
				}
			}
			if(touch.px <= boxButton.width && touch.py >= 192-boxButton.height) {
				goto toggleParty;
			}
		}

		if(pressed & KEY_X && heldPokemon.size() == 0 && !partyShown) {
			Sound::play(Sound::click);
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			if(!xMenu())	break;
		}

		if(arrowY == -2) {
			// If the Arrow Y is at -2, switch to the top screen
			arrowY = 4;
			topScreen = true;
			setSpriteVisibility(arrowID, false, false);
			setSpriteVisibility(arrowID, true, true);
			if(heldPokemon.size()) {
				setSpriteVisibility(heldPokemonID, false, false);
				setSpriteVisibility(heldPokemonID, true, true);
			}
		} else if(arrowY == 5 && topScreen) {
			// If the Arrow Y is at 5, switch to the bottom screen
			arrowY = -1;
			topScreen = false;
			setSpriteVisibility(arrowID, false, true);
			setSpriteVisibility(arrowID, true, false);
			if(heldPokemon.size()) {
				setSpriteVisibility(heldPokemonID, false, true);
				setSpriteVisibility(heldPokemonID, true, false);
			}

		}

		if(pressed & KEY_SELECT && heldPokemon.size() == 0) {
			if(arrowMode < 2)	arrowMode++;
			else	arrowMode = 0;

			fillArrow(arrowMode);

			if(partyShown) {
				if(arrowMode == 0) {
					if(topScreen) {
						topScreen = false;
						setSpriteVisibility(arrowID, true, false);
						setSpriteVisibility(heldPokemonID, true, false);
						setSpriteVisibility(arrowID, false, true);
						setSpriteVisibility(heldPokemonID, false, heldPokemon.size() > 0);
					}
					inParty = true;
					arrowX = 0, arrowY = 0;
				}
				moveParty(arrowMode, heldPokemon.size() > 0);
			}
		}

		if((held & KEY_UP || held & KEY_DOWN || held & KEY_LEFT || held & KEY_RIGHT || held & KEY_L || held & KEY_R || held & KEY_TOUCH) && heldPokemon.size() == 0) {
			// If the cursor is moved and we're not holding a Pokémon, draw the new one
			if(arrowY != -1)	drawPokemonInfo(currentPokemon(arrowX, arrowY));
			else	drawPokemonInfo(save->emptyPkm());
		}

		if(inParty && arrowY < 3) {
			// Move it to the party tray if there
			setSpritePosition(arrowID, topScreen, PARTY_TRAY_X+partySpritePos[(arrowY*2)+(arrowX)].first+partyX+16, PARTY_TRAY_Y+partySpritePos[(arrowY*2)+(arrowX)].second+partyY+4);
			if(heldPokemon.size())	setSpritePosition(heldPokemonID, topScreen, PARTY_TRAY_X+partySpritePos[(arrowY*2)+(arrowX)].first+partyX+8, PARTY_TRAY_Y+partySpritePos[(arrowY*2)+(arrowX)].second+partyY);
		} else if(arrowY == -1) {
			// If the Arrow Y is at -1 (box title), draw it in the middle
			setSpritePosition(arrowID, topScreen, 90, 16);
			if(heldPokemon.size())	setSpritePosition(heldPokemonID, topScreen, 82, 12);
		} else if(arrowY < 5) {
			// If in the main box, move it to the spot in the box it's at
			setSpritePosition(arrowID, topScreen, (arrowX*24)+24, (arrowY*24)+36);
			if(heldPokemon.size())	setSpritePosition(heldPokemonID, topScreen, (arrowX*24)+16, (arrowY*24)+32);
		} else {
			// Or move it to Party button
			setSpritePosition(arrowID, topScreen, getTextWidth(Lang::get("party"))+6, 191-boxButton.height);
			if(heldPokemon.size())	setSpritePosition(heldPokemonID, topScreen, getTextWidth(Lang::get("party"))-2, 191-boxButton.height-4);
		}
		updateOam();
	}
}
