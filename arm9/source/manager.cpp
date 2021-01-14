#include "manager.hpp"

#include "PKFilter.hpp"
#include "PKX.hpp"
#include "aMenu.hpp"
#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "filter.hpp"
#include "flashcard.hpp"
#include "gui.hpp"
#include "i18n.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "loading.hpp"
#include "party.hpp"
#include "sort.hpp"
#include "sound.hpp"
#include "utils.hpp"
#include "xMenu.hpp"

#include <dirent.h>

#define WIDE_SCALE (Graphics::wideScreen ? 0.8f : 1)

bool topScreen, inParty;
Sprite arrow[2], heldPokemonSprites[2], keyboardSprite;
std::vector<std::unique_ptr<Sprite>> boxSprites[2], menuSprites, partySprites;
int currentSaveBox, currentBankBox, arrowMode = 0, boxTitleX = 0, boxTitleY = 0, pkmnX = 0, pkmnY = 0;
std::string savePath;
Image arrowImg[3], ball[BALL_COUNT], bankBox, boxBgTop, boxButton, infoBox, itemIcon, keyboardKey, listBg, menuBg,
	menuButton, menuButtonBlue, party, search, setToSelf, shiny;
std::vector<Image> types;
FILE *pokemonGFX;
std::shared_ptr<pksm::PKFilter> filter = std::make_shared<pksm::PKFilter>();

struct HeldPkm {
	std::unique_ptr<pksm::PKX> pkm;
	int position, x, y;
};

int currentBox(void) { return topScreen ? currentBankBox : currentSaveBox; }

std::unique_ptr<pksm::PKX> currentPokemon(int x, int y) {
	if(topScreen)
		return Banks::bank->pkm(currentBox(), (y * 6) + x);
	else if(inParty)
		return save->pkm((y * 2) + x);
	else
		return save->pkm(currentBox(), (y * 6) + x);
}

int getPokemonIndex(const pksm::PKX &pkm) {
	return getPokemonIndex(pkm.species(), pkm.alternativeForm(), pkm.gender(), pkm.egg());
}

int getPokemonIndex(pksm::Species species, u16 alternativeForm, pksm::Gender gender, bool egg) {
	if(species > pksm::Species::Genesect) {
		return 0;
	} else if(egg) {
		return 651;
	} else if(alternativeForm > 0) {
		switch(species) {
			case pksm::Species::Unown:
				return 651 + alternativeForm;
			case pksm::Species::Castform:
				return 678 + alternativeForm;
			case pksm::Species::Deoxys:
				return 681 + alternativeForm;
			case pksm::Species::Burmy:
				return 684 + alternativeForm;
			case pksm::Species::Wormadam:
				return 686 + alternativeForm;
			case pksm::Species::Shellos:
			case pksm::Species::Gastrodon:
				return u16(species) + 266 + alternativeForm;
			case pksm::Species::Rotom:
				return 690 + alternativeForm;
			case pksm::Species::Giratina:
				return 695 + alternativeForm;
			case pksm::Species::Shaymin:
				return 696 + alternativeForm;
			case pksm::Species::Basculin:
				return 698 + alternativeForm;
			case pksm::Species::Darmanitan:
				return 699 + alternativeForm;
			case pksm::Species::Deerling:
				return 700 + alternativeForm;
			case pksm::Species::Sawsbuck:
				return 703 + alternativeForm;
			case pksm::Species::Meloetta:
				return 708 + alternativeForm;
			case pksm::Species::Tornadus:
			case pksm::Species::Thundurus:
				return u16(species) + 68 + alternativeForm;
			case pksm::Species::Landorus:
			case pksm::Species::Kyurem:
				return u16(species) + 66 + alternativeForm;
			case pksm::Species::Keldeo:
				return 714 + alternativeForm;
			default:
				break;
		}
	} else if(species == pksm::Species::Unfezant && gender == pksm::Gender::Female) {
		return 698;
	} else if((species == pksm::Species::Frillish || species == pksm::Species::Jellicent) &&
			  gender == pksm::Gender::Female) {
		return u16(species) + 115;
	}

	// Non-alternate form, return dex number
	return u16(species);
}

Image loadPokemonSprite(int index) {
	fseek(pokemonGFX, (index * 0x42C) + 4, SEEK_SET);
	return Image(pokemonGFX);
}

void resetPokemonSpritesPos(bool top) {
	// Reset Pokémon sprite positions
	for(int y = 0; y < 5; y++) {
		for(int x = 0; x < 6; x++) {
			if(top) {
				boxSprites[top][(y * 6) + x]->position(Graphics::wideScreen ? 20 + (x * (24 * 0.8)) : 8 + (x * 24),
													   32 + (y * 24));
			} else {
				boxSprites[top][(y * 6) + x]->position(pkmnX + (x * 24), pkmnY + (y * 24));
			}
			boxSprites[top][(y * 6) + x]->visibility(false);
			boxSprites[top][(y * 6) + x]->priority(3);
		}
	}
}

void fillArrow(int arrowMode) {
	arrow[1].drawImage(0, 0, arrowImg[arrowMode], WIDE_SCALE, 1);
	arrow[0].drawImage(0, 0, arrowImg[arrowMode]);
}

void loadTypes(pksm::Language lang) {
	std::string path        = "/i18n/" + StringUtils::toLower(i18n::langString(lang)) + "/types/";
	pksm::Language tempLang = (access(("nitro:/" + path + "/0.gfx").c_str(), F_OK) == 0) ? lang : pksm::Language::ENG;
	path                    = "/i18n/" + StringUtils::toLower(i18n::langString(tempLang)) + "/types/";
	types.clear();
	for(int i = 0; i < 17; i++) {
		types.emplace_back(std::vector<std::string>({Config::getString("themeDir") + path + std::to_string(i) + ".gfx",
													 "nitro:/" + path + std::to_string(i) + ".gfx"}));
	}
}

void initSprites(void) {
	// TODO
	// Sprite::scale(1, true, WIDE_SCALE, 1); // Set rotation index 1 to 80% if Graphics::wideScreen

	// Pokémon Sprites
	for(int y = 0; y < 5; y++) {
		for(int x = 0; x < 6; x++) {
			boxSprites[true].push_back(std::make_unique<Sprite>(
				true, SpriteSize_16x32, SpriteColorFormat_Bmp,
				Graphics::wideScreen ? 20 + (x * (24 * 0.8)) : 8 + (x * 24), 32 + (y * 24), 3, -1, 1));
			boxSprites[false].push_back(std::make_unique<Sprite>(
				false, SpriteSize_16x32, SpriteColorFormat_Bmp,
				Graphics::wideScreen ? 20 + (x * (24 * 0.8)) : 8 + (x * 24), 32 + (y * 24), 3));
		}
	}

	// Menu icon sprites
	for(int i = 0; i < 6; i++) {
		menuSprites.push_back(
			std::make_unique<Sprite>(false, SpriteSize_32x32, SpriteColorFormat_Bmp, 0, 0, 2, -1, 15, false, false));
	}

	// Party sprites
	for(int i = 0; i < 6; i++) {
		Sprite(false, SpriteSize_32x32, SpriteColorFormat_Bmp, 0, 0, 2, -1, 15, false, false);
	}

	// Arrow sprites
	arrow[false] = Sprite(false, SpriteSize_16x16, SpriteColorFormat_Bmp, 24, 36, 0);
	arrow[true]  = Sprite(true, SpriteSize_16x16, SpriteColorFormat_Bmp, 24, 36, 0);

	// Sprites for held pokemon when moving
	heldPokemonSprites[false] =
		Sprite(false, SpriteSize_32x32, SpriteColorFormat_Bmp, 0, 0, 1, -1, 15, -1, false, false);
	heldPokemonSprites[true] = Sprite(true, SpriteSize_32x32, SpriteColorFormat_Bmp, 0, 0, 1, -1, 15, -1, false, false);

	// Keyboard button sprite
	keyboardSprite = Sprite(true, SpriteSize_32x32, SpriteColorFormat_Bmp, 0, 0, 1, -1, 15, -1, false, false);
}

void loadGraphics(void) {
	// Load images into RAM
	arrowImg[0] = Image({Config::getString("themeDir") + "/graphics/arrowBlue.gfx", "nitro:/graphics/arrowBlue.gfx"});
	arrowImg[1] = Image({Config::getString("themeDir") + "/graphics/arrowRed.gfx", "nitro:/graphics/arrowRed.gfx"});
	arrowImg[2] =
		Image({Config::getString("themeDir") + "/graphics/arrowYellow.gfx", "nitro:/graphics/arrowYellow.gfx"});
	for(int i = 0; i < BALL_COUNT; i++) {
		ball[i] = Image({Config::getString("themeDir") + "/graphics/ball/" + std::to_string(i) + ".gfx",
						 "nitro:/graphics/ball/" + std::to_string(i) + ".gfx"});
	}
	boxBgTop  = Image({Config::getString("themeDir") +
                          (Graphics::wideScreen ? "/graphics/boxBgTopWide.gfx" : "/graphics/boxBgTop.gfx"),
                      Graphics::wideScreen ? "nitro:/graphics/boxBgTopWide.gfx" : "nitro:/graphics/boxBgTop.gfx"});
	boxButton = Image({Config::getString("themeDir") + "/graphics/boxButton.gfx", "nitro:/graphics/boxButton.gfx"});
	infoBox   = Image({Config::getString("themeDir") + "/graphics/infoBox.gfx", "nitro:/graphics/infoBox.gfx"});
	itemIcon  = Image({Config::getString("themeDir") + "/graphics/item.gfx", "nitro:/graphics/item.gfx"});
	keyboardKey =
		Image({Config::getString("themeDir") + "/graphics/keyboardKey.gfx", "nitro:/graphics/keyboardKey.gfx"});
	listBg     = Image({Config::getString("themeDir") + "/graphics/listBg.gfx", "nitro:/graphics/listBg.gfx"});
	menuBg     = Image({Config::getString("themeDir") + "/graphics/menuBg.gfx", "nitro:/graphics/menuBg.gfx"});
	menuButton = Image({Config::getString("themeDir") + "/graphics/menuButton.gfx", "nitro:/graphics/menuButton.gfx"});
	menuButtonBlue =
		Image({Config::getString("themeDir") + "/graphics/menuButtonBlue.gfx", "nitro:/graphics/menuButtonBlue.gfx"});
	party     = Image({Config::getString("themeDir") + "/graphics/party.gfx", "nitro:/graphics/party.gfx"});
	search    = Image({Config::getString("themeDir") + "/graphics/search.gfx", "nitro:/graphics/search.gfx"});
	setToSelf = Image({Config::getString("themeDir") + "/graphics/setToSelf.gfx", "nitro:/graphics/setToSelf.gfx"});
	shiny     = Image({Config::getString("themeDir") + "/graphics/shiny.gfx", "nitro:/graphics/shiny.gfx"});

	// Open Pokémon combo gfx file
	if(pokemonGFX)
		fclose(pokemonGFX);
	pokemonGFX = fopen((Config::getString("themeDir") + "/graphics/pokemon.combo.gfx").c_str(), "rb");
	if(!pokemonGFX) {
		pokemonGFX = fopen("nitro:/graphics/pokemon.combo.gfx", "rb");
	}

	// Fill menu icon sprites
	Image menuIconSheet(
		{Config::getString("themeDir") + "/graphics/menuIconSheet.gfx", "nitro:/graphics/menuIconSheet.gfx"});
	for(int i = 1; i < 6; i++) { // 0 (party) skiped until I know what to replace it with
		menuSprites[i]->drawImageSegment(0, 0, 0, i * 32, 32, 32, menuIconSheet);
	}

	// Fill arrows
	fillArrow(arrowMode);
}

void drawBoxScreen(void) {
	// Draws backgrounds
	boxBgTop.draw(0, 0, true, 3);
	infoBox.drawSegmentSpecial(164, 2, 0, 0, infoBox.width(), infoBox.height() - 16, true, 3, WIDE_SCALE, 1.0f);
	Graphics::drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);

	// Clear text
	Graphics::drawRectangle(0, 0, 256, 192, CLEAR, true, true);
	Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Show bottom arrow
	arrow[topScreen].visibility(true);

	// Move the arrow back to 24, 36
	arrow[topScreen].position(pkmnX + 16, pkmnY + 4);

	// Draw the boxes and Pokémon
	drawBox(true);
	drawBox(false);

	// Draw first Pokémon's info
	drawPokemonInfo(*save->pkm(currentBox(), 0));
}

std::string boxBgPath(bool top, int box) {
	if(top)
		return (Graphics::wideScreen ? "/graphics/box/chestWide.gfx" : "/graphics/box/chest.gfx");
	std::string game;
	switch(save->version()) {
		case pksm::GameVersion::S: // Sapphire
		case pksm::GameVersion::R: // Ruby
			game = "rs";
			break;
		case pksm::GameVersion::E: // Emerald
			game = "e";
			break;
		case pksm::GameVersion::FR: // FireRed
		case pksm::GameVersion::LG: // LeafGreen
			game = box < 12 ? "rs" : "frlg";
			break;
		case pksm::GameVersion::D: // Diamond
		case pksm::GameVersion::P: // Pearl
			game = "dp";
			break;
		case pksm::GameVersion::Pt: // Platinum
			game = box < 16 ? "dp" : "pt";
			break;
		case pksm::GameVersion::HG: // HeartGold
		case pksm::GameVersion::SS: // SoulSilver
			game = box < 16 ? "dp" : "hgss";
			break;
		case pksm::GameVersion::W: // White
		case pksm::GameVersion::B: // Black
			game = "bw";
			break;
		case pksm::GameVersion::W2: // White 2
		case pksm::GameVersion::B2: // Black 2
		default:
			game = box < 16 ? "bw" : "b2w2";
			break;
	}
	return "/graphics/box/" + game + "/" + std::to_string(box) + ".gfx";
}

void drawBox(bool top) {
	// Load box image
	bankBox = Image({Config::getString("themeDir") + boxBgPath(top), "nitro:" + boxBgPath(top)});

	for(int i = 0; i < 30; i++) {
		// Fill Pokémon Sprites
		std::unique_ptr<pksm::PKX> tempPkm = (top ? Banks::bank->pkm(currentBankBox, i) : save->pkm(currentSaveBox, i));
		if(tempPkm->species() != pksm::Species::None) {
			boxSprites[top][i]->drawImage(0, 0, loadPokemonSprite(getPokemonIndex(*tempPkm)));
			boxSprites[top][i]->visibility(true);
			if(!(partyShown && arrowMode == 0))
				boxSprites[top][i]->alpha((*tempPkm == *filter) ? 15 : 8);
			if(tempPkm->heldItem())
				boxSprites[top][i]->drawImage(17, 32 - itemIcon.height(), itemIcon);
		} else {
			boxSprites[top][i]->visibility(false);
		}
	}
	Sprite::update(true);
	Sprite::update(false);

	// Draw box image
	bankBox.draw((top && Graphics::wideScreen) ? 17 : 5, 15, top, 3);

	// Print box name
	Gui::font.print((top ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)), boxTitleX,
					top ? 20 : boxTitleY, top, 2, Alignment::left, 110 * WIDE_SCALE, WIDE_SCALE);

	if(!top) {
		search.draw(boxButton.width() + 5, 192 - search.height(), false, 3);
		boxButton.draw(0, 192 - boxButton.height(), false, 3);
		Gui::font.print(i18n::localize(Config::getLang("lang"), "party"), 4, 192 - boxButton.height() + 4, false, 3,
						Alignment::center);
	}
}

void drawPokemonInfo(const pksm::PKX &pkm) {
	// Clear previous draw
	Graphics::drawRectangle(164, 2, infoBox.width(), infoBox.height() - 16, CLEAR, true, true);

	if(pkm.species() > pksm::Species::None && pkm.species() <= pksm::Species::Genesect) {
		// Show shiny star if applicable
		if(pkm.shiny())
			shiny.drawSpecial(170 + (69 * WIDE_SCALE), 45, true, 2, WIDE_SCALE);
		else
			Graphics::drawRectangle(170 + (69 * WIDE_SCALE), 45, shiny.width() * WIDE_SCALE, shiny.height(), 0, true,
									true);

		// Print Pokédex number
		char str[9];
		snprintf(str, sizeof(str), "%s%.3i", i18n::localize(Config::getLang("lang"), "dexNo").c_str(),
				 u16(pkm.species()));
		Gui::font.print(str, 170, 8, true, 2, Alignment::left, TextColor::gray, WIDE_SCALE);

		// Print name
		if(pkm.nicknamed())
			Gui::font.print(pkm.nickname(), 170, 25, true, 2, Alignment::left, 80 * WIDE_SCALE,
							(pkm.gender() ? (pkm.gender() == pksm::Gender::Female ? TextColor::red : TextColor::gray)
										  : TextColor::blue),
							WIDE_SCALE);
		else
			Gui::font.print(i18n::species(Config::getLang("lang"), pkm.species()), 170, 25, true, 2, Alignment::left,
							80 * WIDE_SCALE,
							(pkm.gender() ? (pkm.gender() == pksm::Gender::Female ? TextColor::red : TextColor::gray)
										  : TextColor::blue),
							WIDE_SCALE);

		// Draw types
		types[u8(pkm.type1())].drawSpecial(170, 42 - ((types[u8(pkm.type1())].height() - 12) / 2), true, 2, WIDE_SCALE);
		if(pkm.type1() != pkm.type2()) {
			types[u8(pkm.type1())].drawSpecial(170 + (35 * WIDE_SCALE),
											   42 - ((types[u8(pkm.type2())].height() - 12) / 2), true, 2, WIDE_SCALE,
											   1.0f, 4);
		}

		// Print Level
		Gui::font.print(i18n::localize(Config::getLang("lang"), "lv") + std::to_string(pkm.level()), 170, 57, true, 2,
						Alignment::left, WIDE_SCALE, TextColor::gray);
	}
}

void setHeldPokemon(const pksm::PKX &pkm) {
	if(pkm.species() != pksm::Species::None) {
		Image image = loadPokemonSprite(getPokemonIndex(pkm));
		heldPokemonSprites[true].drawImage(0, 0, image);
		heldPokemonSprites[false].drawImage(0, 0, image);
	}
}

void manageBoxes(void) {
	switch(save->generation()) {
		case pksm::Generation::THREE:
			pkmnX     = 7;
			pkmnY     = 26;
			boxTitleX = -44;
			boxTitleY = 18;
			break;
		case pksm::Generation::FOUR:
			pkmnX     = 10;
			pkmnY     = 30;
			boxTitleX = -42;
			boxTitleY = 19;
			break;
		default:
		case pksm::Generation::FIVE:
			pkmnX     = 8;
			pkmnY     = 32;
			boxTitleX = -44;
			boxTitleY = 20;
			break;
	}

	// Set starting values in filter
	if(filter->gender() == pksm::Gender::INVALID)
		filter->gender(pksm::Gender::Genderless);
	if(filter->ball() == pksm::Ball::INVALID)
		filter->ball(pksm::Ball::Poke);
	if(filter->ability() == pksm::Ability::INVALID)
		filter->ability(pksm::Ability::None);

	resetPokemonSpritesPos(true);
	resetPokemonSpritesPos(false);
	drawBoxScreen();
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
			held    = keysDownRepeat();
		} while(!held);

		if(!inParty) {
			if(held & KEY_UP) {
				if(arrowY > (topScreen ? -1 : -2))
					arrowY--;
			} else if(held & KEY_DOWN) {
				if(arrowY < 5)
					arrowY++;
			}
			if(held & KEY_LEFT && arrowY != -1) {
				if(arrowX > 0) {
					arrowX--;
				} else if(!topScreen && partyShown) {
					arrowX  = 1;
					inParty = true;

					arrowY = (arrowY < 3 ? arrowY / 2 : 2);
				} else {
					arrowX = 5;
				}
			} else if(held & KEY_RIGHT && arrowY != -1) {
				if(arrowX < 5) {
					arrowX++;
				} else if(!topScreen && partyShown) {
					arrowX  = 0;
					inParty = true;

					arrowY = (arrowY < 3 ? arrowY / 2 : 2);
				} else {
					arrowX = 0;
				}
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
				else if(topScreen)
					currentBankBox = Banks::bank->boxes() - 1;
				else
					currentSaveBox = save->maxBoxes() - 1;
				drawBox(topScreen);
				if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
					for(unsigned i = 0; i < heldPokemon.size(); i++) {
						boxSprites[heldPokemonScreen][heldPokemon[i].position]->visibility(false);
					}
				}
				goto afterInput;
			} else if(held & KEY_R) {
			switchBoxRight:
				if((topScreen ? currentBankBox < Banks::bank->boxes() - 1 : currentSaveBox < save->maxBoxes() - 1))
					(topScreen ? currentBankBox : currentSaveBox)++;
				else
					(topScreen ? currentBankBox : currentSaveBox) = 0;
				drawBox(topScreen);
				if(!heldMode && currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
					for(unsigned i = 0; i < heldPokemon.size(); i++) {
						boxSprites[heldPokemonScreen][heldPokemon[i].position]->visibility(false);
					}
				}
				goto afterInput;
			}
		} else { // Cursor movement in party tray
			if(held & KEY_UP) {
				if(arrowY == 6)
					arrowY = 2;
				else if(arrowY > 0)
					arrowY--;
				else
					arrowY = 6;
			} else if(held & KEY_DOWN) {
				if(arrowY < 2)
					arrowY++;
				else if(arrowY == 2)
					arrowY = 6;
				else
					arrowY = 0;
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
						arrowX  = 5;
						inParty = false;
						arrowY  = (arrowY < 2 ? arrowY * 2 : 3);
					}
				} else if(held & KEY_RIGHT) {
					if(arrowX == 0) {
						arrowX++;
					} else {
						arrowX  = 0;
						inParty = false;
						arrowY  = (arrowY < 2 ? arrowY * 2 : 3);
					}
				}
			}
		}

		if(pressed & KEY_A) {
		selection:
			Sound::play(Sound::click);
			if(arrowY == -1) {
				if(heldPokemon.size() == 0)
					aMenu(-1, -1, aMenuTopBarButtons, 1);
			} else if(arrowY < 5) {
				if(heldPokemon.size() > 0) {
					if(heldPokemon[0].position == (arrowY * 6) + arrowX && heldPokemonBox == currentBox() &&
					   heldPokemonScreen == topScreen) {
					// If in the held Pokémon's previous spot, just put held Pokémon back down
					returnPokemon:
						for(unsigned i = 0; i < heldPokemon.size(); i++) {
							if(heldInParty)
								partySprites[heldPokemon[i].position]->visibility(heldPokemon[i].pkm->species() !=
																				  pksm::Species::None);
							else if(!heldPokemonScreen)
								boxSprites[heldPokemonScreen][heldPokemon[i].position]->visibility(
									heldPokemon[i].pkm->species() != pksm::Species::None);
							else {
								boxSprites[heldPokemonScreen][heldPokemon[i].position]->drawImage(
									0, 0, loadPokemonSprite(getPokemonIndex(*heldPokemon[i].pkm)));
							}
						}
						heldPokemonSprites[topScreen].visibility(false);
						heldPokemon.clear();
						heldPokemonBox = -1;
						if(partyShown && !topScreen)
							moveParty(arrowMode, heldPokemon.size() > 0);
					} else if(!heldMode || currentPokemon(arrowX, arrowY)->species() == pksm::Species::None) {
						int canPlace = true;
						for(unsigned i = 0; i < heldPokemon.size(); i++) {
							if(heldPokemon[i].x - heldPokemon[0].x > 5 - arrowX)
								canPlace = false;
							if(heldPokemon[i].y - heldPokemon[0].y > 4 - arrowY)
								canPlace = false;
							if(currentBox() == heldPokemonBox && topScreen == heldPokemonScreen) {
								for(unsigned j = 0; j < heldPokemon.size(); j++) {
									if(((arrowY + heldPokemon[j].y) * 6) + arrowX + heldPokemon[j].x ==
									   heldPokemon[i].position) {
										canPlace = false;
										break;
									}
								}
							}
							if(!canPlace)
								break;
						}
						if(canPlace) {
							for(unsigned i = 0; i < heldPokemon.size(); i++) {
								if(topScreen || save->availableSpecies().count(heldPokemon[i].pkm->species()) != 0) {
									// If not copying / there isn't a Pokémon at the new spot, move Pokémon
									// Save the Pokémon at the cursor's postion to a temp variable
									std::unique_ptr<pksm::PKX> tempPkm;
									if(currentPokemon(arrowX + heldPokemon[i].x, arrowY + heldPokemon[i].y)
										   ->species() != pksm::Species::None)
										tempPkm = currentPokemon(arrowX + heldPokemon[i].x, arrowY + heldPokemon[i].y);
									else
										tempPkm = save->emptyPkm();
									// Write the held Pokémon to the cursor position
									if(topScreen) {
										Banks::bank->pkm(*heldPokemon[i].pkm, currentBox(),
														 ((arrowY + heldPokemon[i].y) * 6) + arrowX + heldPokemon[i].x);
									} else if(inParty) {
										save->pkm(*save->transfer(*heldPokemon[i].pkm),
												  ((arrowY + heldPokemon[i].y) * 2) + arrowX + heldPokemon[i].x);
										save->dex(*heldPokemon[i].pkm);
									} else {
										save->pkm(*save->transfer(*heldPokemon[i].pkm), currentBox(),
												  ((arrowY + heldPokemon[i].y) * 6) + arrowX + heldPokemon[i].x, false);
										save->dex(*heldPokemon[i].pkm);
									}
									// If not copying, write the cursor position's previous Pokémon to the held
									// Pokémon's old spot
									if(!heldMode) {
										if(heldPokemonScreen) {
											Banks::bank->pkm(*tempPkm, heldPokemonBox, heldPokemon[i].position);
										} else if(heldInParty) {
											save->pkm(*save->transfer(*tempPkm), heldPokemon[i].position);
											save->dex(*tempPkm);
										} else {
											save->pkm(*save->transfer(*tempPkm), heldPokemonBox,
													  heldPokemon[i].position, false);
											save->dex(*tempPkm);
										}
									}
								}
							}
							save->fixParty();

							// Hide the moving Pokémon
							if(topScreen) {
								heldPokemonSprites[topScreen].position(-32, -32);
							} else {
								heldPokemonSprites[topScreen].visibility(false);
							}

							// Update the box(es) for the moved Pokémon
							drawBox(topScreen);
							if(heldPokemonScreen != topScreen)
								drawBox(heldPokemonScreen);
							drawPokemonInfo(*currentPokemon(arrowX, arrowY));
							if(partyShown)
								fillPartySprites();

							// Not holding a Pokémon anymore
							heldPokemon.clear();
							heldPokemonBox = -1;

							// Move the party tray back
							if(partyShown) {
								moveParty(arrowMode, heldPokemon.size() > 0);
								if(arrowMode == 0 && !inParty) {
									if(topScreen) {
										topScreen = false;
										arrow[topScreen].position(-16, -16);
										heldPokemonSprites[topScreen].position(-32, -32);
										arrow[topScreen].visibility(true);
										heldPokemonSprites[topScreen].visibility(heldPokemon.size() > 0);
									}
									inParty = true;
									arrowX = 0, arrowY = 0;
								}
							}
						}
					}
				} else if(arrowMode == 2) {
					bankBox =
						Image({Config::getString("themeDir") + boxBgPath(topScreen), "nitro:" + boxBgPath(topScreen)});
					int startX = arrowX, startY = arrowY;
					int x = topScreen ? (Graphics::wideScreen ? 22 : 8) : pkmnX, y = topScreen ? 32 : pkmnY;
					float scale = topScreen ? WIDE_SCALE : 1;
					if(inParty)
						Graphics::drawOutline(PARTY_TRAY_X + (partySpritePos[(startY * 2) + startX].first),
											  PARTY_TRAY_Y + (partySpritePos[(startY * 2) + startX].second) + 8, 32, 24,
											  DARKER_GRAY, topScreen, true);
					else
						Graphics::drawOutline(x + (startX * 24 * scale), y + 8 + (startY * 24),
											  (((arrowX + 1) - startX) * 24) + 8, 24, WHITE, topScreen, false);
					while(1) {
						do {
							swiWaitForVBlank();
							scanKeys();
							pressed = keysDown();
							held    = keysDownRepeat();
						} while(!held);

						if(held & KEY_UP && arrowY > 0)
							arrowY--;
						else if(held & KEY_DOWN && arrowY < (inParty ? 2 : 4))
							arrowY++;
						if(held & KEY_LEFT && arrowX > 0)
							arrowX--;
						else if(held & KEY_RIGHT && arrowX < (inParty ? 1 : 5))
							arrowX++;
						if(pressed & KEY_A) {
						yellowSelection:
							if(inParty)
								party.draw(PARTY_TRAY_X, PARTY_TRAY_Y, false, 2);
							else
								bankBox.drawSegment((Graphics::wideScreen && topScreen) ? 17 : 5, 15 + 20, 0, 20,
													bankBox.width(), bankBox.height() - 20, topScreen, 3);
							for(int y = std::min(startY, arrowY); y < std::max(startY, arrowY) + 1; y++) {
								for(int x = std::min(startX, arrowX); x < std::max(startX, arrowX) + 1; x++) {
									heldPokemon.push_back({currentPokemon(x, y), (y * (inParty ? 2 : 6)) + x,
														   x - std::min(startX, arrowX), y - std::min(startY, arrowY)});
									if(inParty)
										partySprites[(y * 2) + x]->visibility(false);
									boxSprites[topScreen][(y * 6) + x]->visibility(false);
								}
							}
							heldPokemonSprites[true].clear();
							heldPokemonSprites[true].drawImage(0, 0, keyboardKey);
							Gui::font.print(StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())),
											24 - (Gui::font.calcWidth(std::to_string(heldPokemon.size())) / 2), 16,
											heldPokemonSprites[true]);
							heldPokemonSprites[false].clear();
							heldPokemonSprites[false].drawImage(0, 0, keyboardKey);
							Gui::font.print(StringUtils::UTF8toUTF16(std::to_string(heldPokemon.size())),
											24 - (Gui::font.calcWidth(std::to_string(heldPokemon.size())) / 2), 16,
											heldPokemonSprites[false]);
							heldPokemonSprites[topScreen].visibility(true);
							heldPokemonSprites[topScreen].update();
							heldPokemonBox    = currentBox();
							heldPokemonScreen = topScreen;
							heldInParty       = inParty;
							arrowX            = std::min(startX, arrowX);
							arrowY            = std::min(startY, arrowY);
							break;
						} else if(pressed & KEY_B) {
							if(inParty)
								party.draw(PARTY_TRAY_X, PARTY_TRAY_Y, false, 2);
							else
								bankBox.drawSegment((Graphics::wideScreen && topScreen) ? 17 : 5, 15 + 20, 0, 20,
													bankBox.width(), bankBox.height() - 20, topScreen, 3);
							drawPokemonInfo(*currentPokemon(arrowX, arrowY));
							break;
						} else if(pressed & KEY_TOUCH) {
							touchRead(&touch);
							for(int x = 0; x < 6; x++) {
								for(int y = 0; y < 5; y++) {
									if(touch.px > 16 + (x * 24) && touch.px < 16 + ((x + 1) * 24) &&
									   touch.py > 40 + (y * 24) && touch.py < 40 + ((y + 1) * 24)) {
										if(arrowX == x && arrowY == y && topScreen == false)
											goto yellowSelection;
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
								for(int i = 0; i < 6; i++) {
									if(touch.px >= PARTY_TRAY_X + partyX + partySpritePos[i].first &&
									   touch.px <= PARTY_TRAY_X + partyX + partySpritePos[i].first + 32 &&
									   touch.py >= PARTY_TRAY_Y + partyY + partySpritePos[i].second &&
									   touch.py <= PARTY_TRAY_Y + partyY + partySpritePos[i].second + 32) {
										if(arrowX == i - ((i / 2) * 2) && arrowY == i / 2 && inParty == true)
											goto yellowSelection;
										else {
											if(inParty) {
												arrowX = i - ((i / 2) * 2);
												arrowY = i / 2;
											}
										}
									}
								}
							}
						}

						if(inParty) {
							party.draw(PARTY_TRAY_X, PARTY_TRAY_Y, false, 2);
							Graphics::drawOutline(
								PARTY_TRAY_X +
									(partySpritePos[(std::min(startY, arrowY) * 2) + std::min(startX, arrowX)].first),
								PARTY_TRAY_Y +
									(partySpritePos[(std::min(startY, arrowY) * 2) + std::min(startX, arrowX)].second) +
									8,
								(std::max(arrowX - startX, startX - arrowX) == 0 ? 32 : 72),
								((std::max(arrowY - startY, startY - arrowY) + 1) * 24 +
								 (std::max(arrowY - startY, startY - arrowY)) * 8 +
								 (std::max(arrowX - startX, startX - arrowX) == 0 ? 0 : 8)),
								DARKER_GRAY, topScreen, true);
							arrow[topScreen].position(
								PARTY_TRAY_X + partySpritePos[(arrowY * 2) + (arrowX)].first + partyX + 16,
								PARTY_TRAY_Y + partySpritePos[(arrowY * 2) + (arrowX)].second + partyY + 4);
						} else {
							int x = topScreen ? (Graphics::wideScreen ? 22 : 8) : pkmnX, y = topScreen ? 32 : pkmnY;
							float scale = topScreen ? WIDE_SCALE : 1;
							bankBox.drawSegment((Graphics::wideScreen && topScreen) ? 17 : 5, 0, 20, 15 + 20,
												bankBox.width(), bankBox.height() - 22, topScreen, 3);
							Graphics::drawOutline(
								x + (std::min(startX, arrowX) * (24 * scale)), y + 8 + (std::min(startY, arrowY) * 24),
								((std::max(arrowX - startX, startX - arrowX) + 1) * (24 * scale)) + 8,
								((std::max(arrowY - startY, startY - arrowY) + 1) * 24), WHITE, topScreen, false);
							arrow[topScreen].position((arrowX * 24 * scale) + x + 16, (arrowY * 24) + y + 4);
						}
						arrow[topScreen].update();
					}
				} else if(currentPokemon(arrowX, arrowY)->species() != pksm::Species::None) {
					int temp = 1;
					if(arrowMode == 1 || (temp = aMenu(arrowX, arrowY, aMenuButtons, 0))) {
						// If no pokemon is currently held and there is one at the cursor, pick it up
						heldPokemon.push_back(
							{currentPokemon(arrowX, arrowY)->clone(), (arrowY * (inParty ? 2 : 6)) + arrowX, 0, 0});
						heldPokemonBox    = currentBox();
						heldPokemonScreen = topScreen;
						heldInParty       = inParty;
						heldMode          = temp - 1; // false = move, true = copy
						setHeldPokemon(*heldPokemon[0].pkm);

						moveParty(arrowMode, heldPokemon.size() > 0);

						if(!heldMode) {
							if(heldInParty)
								partySprites[heldPokemon[0].position]->visibility(false);
							else
								boxSprites[heldPokemonScreen][heldPokemon[0].position]->visibility(false);
						}
						heldPokemonSprites[topScreen].visibility(true);
						drawPokemonInfo(*heldPokemon[0].pkm);
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
						arrow[true].visibility(false);
					}
					arrowX = 0, arrowY = 0;
					inParty = true;
					drawPokemonInfo(*save->pkm(0));
				} else {
					if(inParty) {
						arrowX  = 0;
						arrowY  = 5;
						inParty = false;
					}
					drawPokemonInfo(*save->emptyPkm());
				}
			}
		} else if(pressed & KEY_B) {
			if(heldPokemon.size() > 0) {
				Sound::play(Sound::back);

				// Reset postion
				inParty   = heldInParty;
				arrowX    = heldPokemon[0].position - ((heldPokemon[0].position / 6) * 6);
				arrowY    = heldPokemon[0].position / 6;
				topScreen = heldPokemonScreen;
				if(!inParty) {
					(topScreen ? currentBankBox : currentSaveBox) = heldPokemonBox;
					drawBox(topScreen);
				}

				heldPokemonSprites[true].visibility(false);
				heldPokemonSprites[false].visibility(false);
				arrow[!topScreen].visibility(false);
				arrow[topScreen].visibility(true);

				drawPokemonInfo(*currentPokemon(arrowX, arrowY));
				goto returnPokemon;
			} else if(partyShown) {
				Sound::play(Sound::back);
				goto toggleParty;
			}
		} else if(pressed & KEY_X && heldPokemon.size() == 0 && !partyShown) {
			Sound::play(Sound::click);
			Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			if(!xMenu())
				break;
		} else if(pressed & KEY_Y && !partyShown) {
			if(heldPokemon.size() == 0) {
			filter:
				arrow[topScreen].visibility(false);
				arrow[topScreen].update();
				if(Input::getBool(i18n::localize(Config::getLang("lang"), "filter"),
								  i18n::localize(Config::getLang("lang"), "sort"))) {
					Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					changeFilter(filter);
				} else {
					Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					sortMenu(topScreen);
				}

				// Redraw
				arrow[topScreen].visibility(!topScreen);
				Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				Graphics::drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				drawPokemonInfo(*currentPokemon(arrowX, arrowY));
				if(topScreen)
					drawBox(topScreen);
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(!(partyShown && arrowMode == 0 && heldPokemon.size() == 0)) {
				for(int x = 0; x < 6; x++) {
					for(int y = 0; y < 5; y++) {
						if(touch.px > 16 + (x * 24) && touch.px < 16 + ((x + 1) * 24) && touch.py > 40 + (y * 24) &&
						   touch.py < 40 + ((y + 1) * 24)) {
							if(arrowX == x && arrowY == y && topScreen == false)
								goto selection;
							else {
								if(topScreen) {
									topScreen = false;
									arrow[true].visibility(false);
									arrow[false].visibility(true);
									if(heldPokemon.size() > 0) {
										heldPokemonSprites[true].visibility(false);
										heldPokemonSprites[false].visibility(true);
									}
								}
								inParty = false;
								arrowX  = x;
								arrowY  = y;
							}
						}
					}
				}
				if(touch.px > 26 && touch.px < 141 && touch.py > 19 && touch.py < 37) { // Box title
					arrowY = -1;
					goto selection;
				} else if(touch.px > 6 && touch.px < 26 && touch.py > 19 && touch.py < 37) { // Switch to previous box
					goto switchBoxLeft;
				} else if(touch.px > 141 && touch.px < 161 && touch.py > 19 && touch.py < 37) { // Switch to next box
					goto switchBoxRight;
				} else if(touch.px >= boxButton.width() + 5 && touch.px <= boxButton.width() + 5 + search.width() &&
						  touch.py >= 192 - search.height()) { // Filter button
					goto filter;
				}
			}
			if(partyShown) {
				for(int i = 0; i < 6; i++) {
					if(touch.px >= PARTY_TRAY_X + partyX + partySpritePos[i].first &&
					   touch.px <= PARTY_TRAY_X + partyX + partySpritePos[i].first + 32 &&
					   touch.py >= PARTY_TRAY_Y + partyY + partySpritePos[i].second &&
					   touch.py <= PARTY_TRAY_Y + partyY + partySpritePos[i].second + 32) {
						if(arrowX == i - ((i / 2) * 2) && arrowY == i / 2 && inParty == true)
							goto selection;
						else {
							if(topScreen) {
								topScreen = false;
								arrow[true].visibility(false);
								arrow[false].visibility(true);
								heldPokemonSprites[true].visibility(false);
								heldPokemonSprites[false].visibility(true);
							}
							inParty = true;
							arrowX  = i - ((i / 2) * 2);
							arrowY  = i / 2;
						}
					}
				}
			}
			if(touch.px <= boxButton.width() && touch.py >= 192 - boxButton.height()) {
				goto toggleParty;
			}
		}

	afterInput:

		if(arrowY == -2) {
			// If the Arrow Y is at -2, switch to the top screen
			arrowY    = 4;
			topScreen = true;
			arrow[topScreen].visibility(false);
			if(heldPokemon.size()) {
				heldPokemonSprites[false].visibility(false);
			}
		} else if(arrowY == 5 && topScreen) {
			// If the Arrow Y is at 5, switch to the bottom screen
			arrowY    = -1;
			topScreen = false;
			arrow[true].visibility(false);
			arrow[false].visibility(true);
			if(heldPokemon.size()) {
				heldPokemonSprites[true].visibility(false);
				heldPokemonSprites[false].visibility(true);
			}
		}

		if(pressed & KEY_SELECT && heldPokemon.size() == 0) {
			if(arrowMode < 2)
				arrowMode++;
			else
				arrowMode = 0;

			fillArrow(arrowMode);

			if(partyShown) {
				if(arrowMode == 0) {
					if(topScreen) {
						topScreen = false;
						arrow[topScreen].position(-16, -16);
						heldPokemonSprites[topScreen].position(-32, -32);
						arrow[topScreen].visibility(true);
						heldPokemonSprites[topScreen].visibility(true);
					}
					inParty = true;
					arrowX = 0, arrowY = 0;
				}
				moveParty(arrowMode, heldPokemon.size() > 0);
			}
		}

		if((held & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT | KEY_L | KEY_R | KEY_TOUCH)) && heldPokemon.size() == 0) {
			// If the cursor is moved and we're not holding a Pokémon, draw the new one
			if(arrowY != -1 && arrowY < 5)
				drawPokemonInfo(*currentPokemon(arrowX, arrowY));
			else
				drawPokemonInfo(*save->emptyPkm());
		}

		if(inParty && arrowY < 3) {
			// Move it to the party tray if there
			arrow[topScreen].position(PARTY_TRAY_X + partySpritePos[(arrowY * 2) + (arrowX)].first + partyX + 16,
									  PARTY_TRAY_Y + partySpritePos[(arrowY * 2) + (arrowX)].second + partyY + 4);
			if(heldPokemon.size())
				heldPokemonSprites[topScreen].position(
					PARTY_TRAY_X + partySpritePos[(arrowY * 2) + (arrowX)].first + partyX + 8,
					PARTY_TRAY_Y + partySpritePos[(arrowY * 2) + (arrowX)].second + partyY);
		} else if(arrowY == -1) {
			// If the Arrow Y is at -1 (box title), draw it in the middle
			arrow[topScreen].position(90, 16);
			if(heldPokemon.size())
				heldPokemonSprites[topScreen].position(82, 12);
		} else if(arrowY < 5) {
			int x = topScreen ? 8 : pkmnX, y = topScreen ? 32 : pkmnY;
			// If in the main box, move it to the spot in the box it's at
			float scale = topScreen ? WIDE_SCALE : 1;
			int offset  = scale == 1 ? 0 : 12;
			arrow[topScreen].position((arrowX * 24 * scale) + x + 16 + offset, (arrowY * 24) + y + 4);
			if(heldPokemon.size())
				heldPokemonSprites[topScreen].position((arrowX * 24 * scale) + x + 8 + offset, (arrowY * 24) + y);
		} else {
			// Or move it to Party button
			arrow[topScreen].position(Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), "party")) + 6,
									  191 - boxButton.height());
			if(heldPokemon.size())
				heldPokemonSprites[topScreen].position(
					Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), "party")) - 2,
					191 - boxButton.height() - 4);
		}
		arrow[topScreen].update();
	}
}
