#include "aMenu.hpp"

#include "PKX.hpp"
#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "gui.hpp"
#include "i18n.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "party.hpp"
#include "random.hpp"
#include "sound.hpp"
#include "summary.hpp"

#include <dirent.h>

#define pkmPos(x, y) ((y * (inParty ? 2 : 6)) + x)

std::vector<Label> aMenuButtons = {
	{170, 16, "move"},    // Move
	{170, 42, "edit"},    // Edit
	{170, 68, "copy"},    // Copy
	{170, 94, "release"}, // Release
	{170, 120, "dump"},   // Dump
	{170, 146, "back"},   // Back
};
std::vector<Label> aMenuEmptySlotButtons = {
	{170, 16, "inject"}, // Inject
	{170, 42, "create"}, // Create
	{170, 68, "back"},   // Back
};
std::vector<Label> aMenuTopBarButtons = {
	{170, 16, "jump"},      // Jump
	{170, 42, "rename"},    // Rename
	{170, 68, "swap"},      // Swap
	{170, 94, "wallpaper"}, // Wallpaper
	{170, 120, "dumpBox"},  // Dump box
	{170, 146, "back"},     // Back
};

constexpr char pathFormat[][51] = {
	"%s:/_nds/pkmn-chest/out/%s%03i-%02i - %s - %x%lx%s",
	"%s:/_nds/pkmn-chest/out/%s%03i-%02i -\n%s -\n%x%lx%s",
	"%s:/_nds/pkmn-chest/out/%s%03i - %s - %x%lx%s",
	"%s:/_nds/pkmn-chest/out/%s%03i -\n%s -\n%x%lx%s",
};

std::string getPkxOutputPath(const pksm::PKX &pkm, const std::string &boxName = "", bool insertNewlines = false) {
	char path[PATH_MAX];
	if(pkm.alternativeForm()) {
		snprintf(path,
				 sizeof(path),
				 pathFormat[insertNewlines],
				 mainDrive().c_str(),
				 (boxName == "" ? boxName : boxName + (insertNewlines ? "/\n" : "/")).c_str(),
				 u16(pkm.species()),
				 pkm.alternativeForm(),
				 pkm.nickname().c_str(),
				 pkm.checksum(),
				 pkm.encryptionConstant(),
				 pkm.extension().data());
	} else {
		snprintf(path,
				 sizeof(path),
				 pathFormat[2 + insertNewlines],
				 mainDrive().c_str(),
				 (boxName == "" ? boxName : boxName + (insertNewlines ? "/\n" : "/")).c_str(),
				 u16(pkm.species()),
				 pkm.nickname().c_str(),
				 pkm.checksum(),
				 pkm.encryptionConstant(),
				 pkm.extension().data());
	}

	return path;
}

void drawAMenuButtons(std::vector<Label> &buttons, int buttonMode) {
	for(unsigned i = 0; i < buttons.size(); i++) {
		drawImage(buttons[i].x, buttons[i].y, boxButton, false, true);
		printTextMaxW(i18n::localize(Config::getLang("lang"), buttons[i].label),
					  80,
					  1,
					  buttons[i].x + 4,
					  buttons[i].y + 4,
					  false,
					  true);
	}
}

int aMenu(int pkmX, int pkmY, std::vector<Label> &buttons, int buttonMode) {
	setSpritePosition(arrowID,
					  false,
					  buttons[0].x + getTextWidthMaxW(i18n::localize(Config::getLang("lang"), buttons[0].label), 80) +
						  4,
					  buttons[0].y);
	setSpritePosition(arrowID, true, -16, -16); // Move top arrow off screen
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
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(menuSelection > 0)
				menuSelection--;
		} else if(held & KEY_DOWN) {
			if(menuSelection < (int)buttons.size() - 1)
				menuSelection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);

			for(unsigned i = 0; i < buttons.size(); i++) {
				if(touch.px >= buttons[i].x && touch.px <= buttons[i].x + 64 && touch.py >= buttons[i].y &&
				   touch.py <= buttons[i].y + 25) {
					menuSelection  = i;
					optionSelected = true;
				}
			}
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			goto back;
		}

		if(optionSelected)
			Sound::play(Sound::click);
		if(optionSelected && buttonMode == 0) { // A Pokémon
			optionSelected = false;
			if(menuSelection == 0) { // Move
				if(topScreen) {
					setSpriteVisibility(arrowID, false, false);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);
				return 1;
			} else if(menuSelection == 1) { // Edit
				if(partyShown) {
					// Hide party Pokémon
					for(unsigned int i = 0; i < partyIconID.size(); i++) {
						setSpriteVisibility(partyIconID[i], false, false);
					}
					updateOam();
				}
				if(topScreen)
					Banks::bank->pkm(
						showPokemonSummary(*currentPokemon(pkmX, pkmY)), currentBankBox, pkmPos(pkmX, pkmY));
				else if(inParty)
					save->pkm(showPokemonSummary(*currentPokemon(pkmX, pkmY)), pkmPos(pkmX, pkmY));
				else
					save->pkm(
						showPokemonSummary(*currentPokemon(pkmX, pkmY)), currentSaveBox, pkmPos(pkmX, pkmY), false);

				// Redraw screen
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawBox(false);
				if(topScreen)
					drawBox(topScreen);
				drawPokemonInfo(*currentPokemon(pkmX, pkmY));
				drawAMenuButtons(buttons, buttonMode);
				if(partyShown) {
					drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
					fillPartySprites();
				}
			} else if(menuSelection == 2) { // Copy
				if(topScreen) {
					setSpriteVisibility(arrowID, false, false);
				}
				updateOam();
				drawRectangle(170, 0, 86, 192, CLEAR, false, true);
				return 2;
			} else if(menuSelection == 3) { // Release
				if(partyShown) {
					// Hide party Pokémon
					for(unsigned int i = 0; i < partyIconID.size(); i++) {
						setSpriteVisibility(partyIconID[i], false, false);
					}
					updateOam();
				}
				if(Input::getBool(i18n::localize(Config::getLang("lang"), "release"),
								  i18n::localize(Config::getLang("lang"), "cancel"))) {
					if(topScreen)
						Banks::bank->pkm(*save->emptyPkm(), currentBankBox, pkmPos(pkmX, pkmY));
					else if(inParty)
						save->pkm(*save->emptyPkm(), pkmPos(pkmX, pkmY));
					else
						save->pkm(*save->emptyPkm(), currentSaveBox, pkmPos(pkmX, pkmY), false);
					drawPokemonInfo(*save->emptyPkm());
					drawBox(topScreen);

					if(partyShown) {
						save->fixParty();
						drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
						fillPartySprites();
					}
					goto back;
				}
				drawAMenuButtons(buttons, buttonMode);
				if(partyShown) {
					drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
					fillPartySprites();
				}
			} else if(menuSelection == 4) { // Dump
				if(partyShown) {
					// Hide party Pokémon
					for(unsigned int i = 0; i < partyIconID.size(); i++) {
						setSpriteVisibility(partyIconID[i], false, false);
					}
					updateOam();
				}

				FILE *out = fopen(getPkxOutputPath(*currentPokemon(pkmX, pkmY)).c_str(), "wb");
				if(out) {
					fwrite(currentPokemon(pkmX, pkmY)->rawData(), 1, 136, out);
					fclose(out);
				}

				setSpriteVisibility(arrowID, false, false);
				updateOam();

				// Get formatted path for prompt
				char str[PATH_MAX];
				snprintf(str,
						 sizeof(str),
						 i18n::localize(Config::getLang("lang"), "dumpedTo").c_str(),
						 getPkxOutputPath(*currentPokemon(pkmX, pkmY), "", true).c_str());

				Gui::prompt(str, i18n::localize(Config::getLang("lang"), "ok"));

				drawAMenuButtons(buttons, buttonMode);
				setSpriteVisibility(arrowID, false, true);
				updateOam();

				if(partyShown) {
					drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
					fillPartySprites();
				}
			} else if(menuSelection == 5) { // Back
			back:
				if(topScreen) {
					setSpriteVisibility(arrowID, false, false);
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

				if(num == -1 ||
				   num == (topScreen ? currentBankBox : currentSaveBox)) { // If B was pressed or the box wasn't changed
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
					if(topScreen)
						Banks::bank->boxName(newName, currentBankBox);
					else
						save->boxName(currentSaveBox, newName);
				}

				// Redraw screen
				setSpriteVisibility(arrowID, false, true);
				updateOam();
				drawAMenuButtons(buttons, buttonMode);
				drawBox(topScreen);
			} else if(menuSelection == 2) { // Swap
				std::vector<std::unique_ptr<pksm::PKX>> tempBox;
				bool shouldCopy[30];
				// Copy save Pokémon to a buffer
				for(int i = 0; i < 30; i++) {
					if(save->pkm(currentSaveBox, i)->species() != pksm::Species::None)
						tempBox.push_back(save->pkm(currentSaveBox, i));
					else
						tempBox.push_back(save->emptyPkm());
				}

				// Copy bank Pokémon to the save and add it to the Pokédex
				for(int i = 0; i < 30; i++) {
					if(Banks::bank->pkm(currentBankBox, i)->species() != pksm::Species::None) {
						if(save->availableSpecies().count(Banks::bank->pkm(currentBankBox, i)->species()) != 0) {
							save->pkm(*save->transfer(*Banks::bank->pkm(currentBankBox, i)), currentSaveBox, i, false);
							save->dex(*Banks::bank->pkm(currentBankBox, i));
							shouldCopy[i] = true;
						} else {
							shouldCopy[i] = false;
						}
					} else {
						save->pkm(*save->emptyPkm(), currentSaveBox, i, false);
						shouldCopy[i] = true;
					}
				}

				// Copy the save Pokémon from their buffer to the bank
				for(int i = 0; i < 30; i++) {
					if(shouldCopy[i]) {
						Banks::bank->pkm(*tempBox[i], currentBankBox, i);
					}
				}

				// Update the boxes
				drawBox(true);
				drawBox(false);
			} else if(menuSelection == 3) { // Wallpaper
				if(!topScreen) {
					// Hide sprites
					for(int i = 0; i < 30; i++)
						setSpriteVisibility(i, false, false);
					setSpriteVisibility(arrowID, false, false);
					updateOam();

					// Get new wallpaper
					int num = selectWallpaper(save->boxWallpaper(currentSaveBox));
					if(num != -1)
						save->boxWallpaper(currentSaveBox, num);

					// Redraw screen
					setSpriteVisibility(arrowID, false, true);
					drawRectangle(170, 0, 86, 192, CLEAR, false, true);
					drawAMenuButtons(buttons, buttonMode);
					drawBox(false);
				}
			} else if(menuSelection == 4) { // Dump box
				char path[PATH_MAX];
				snprintf(path,
						 sizeof(path),
						 "%s:/_nds/pkmn-chest/out/%s",
						 mainDrive().c_str(),
						 topScreen ? Banks::bank->boxName(currentBankBox).c_str()
								   : save->boxName(currentSaveBox).c_str());
				mkdir(path, 0777);

				for(int y = 0; y < 5; y++) {
					for(int x = 0; x < 6; x++) {
						if(currentPokemon(x, y)->species() != pksm::Species::None) {
							FILE *out = fopen(getPkxOutputPath(*currentPokemon(x, y),
															   topScreen ? Banks::bank->boxName(currentBankBox)
																		 : save->boxName(currentSaveBox))
												  .c_str(),
											  "wb");
							if(out) {
								fwrite(currentPokemon(x, y)->rawData(), 1, 136, out);
								fclose(out);
							}
						}
					}
				}

				setSpriteVisibility(arrowID, false, false);
				updateOam();

				// Get formatted path for prompt
				char outputPath[PATH_MAX], str[PATH_MAX];
				snprintf(outputPath,
						 sizeof(outputPath),
						 "%s:/_nds/pkmn-chest/\nout/%s",
						 mainDrive().c_str(),
						 (topScreen ? Banks::bank->boxName(currentBankBox) : save->boxName(currentSaveBox)).c_str());
				snprintf(str, sizeof(str), i18n::localize(Config::getLang("lang"), "dumpedTo").c_str(), outputPath);

				Gui::prompt(str, i18n::localize(Config::getLang("lang"), "ok"));

				drawAMenuButtons(buttons, buttonMode);
				setSpriteVisibility(arrowID, false, true);
				updateOam();
			} else if(menuSelection == 5) { // Back
				goto back;
			}
		} else if(optionSelected && buttonMode == 2) { // Empty slot
			optionSelected = false;
			if(menuSelection == 0) { // Inject
				// Hide sprites
				for(int i = 0; i < 30; i++) {
					setSpriteVisibility(i, false, false);
				}
				if(partyShown) {
					// Hide party Pokémon
					for(unsigned int i = 0; i < partyIconID.size(); i++) {
						setSpriteVisibility(partyIconID[i], false, false);
					}
				}
				setSpriteVisibility(arrowID, false, false);
				updateOam();

				// Save path and chane to /_nds/pkmn-chest/in
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);
				chdir((mainDrive() + ":/_nds/pkmn-chest/in").c_str());

				// Get a pk4/5
				std::string fileName = browseForFile({"pk3", "pk4", "pk5", "pk6", "pk7"}, false);

				// If the fileName isn't blank, inject the Pokémon
				if(fileName != "") {
					pksm::Generation gen;
					switch(fileName[fileName.size() - 1]) {
						case '3':
							gen = pksm::Generation::THREE;
							break;
						case '4':
							gen = pksm::Generation::FOUR;
							break;
						case '5':
							gen = pksm::Generation::FIVE;
							break;
						case '6':
							gen = pksm::Generation::SIX;
							break;
						case '7':
							switch(fileName[fileName.size() - 2]) {
								case 'k':
								case 'K':
									gen = pksm::Generation::SEVEN;
									break;
								case 'b': // Not currently supported
								case 'B':
									gen = pksm::Generation::LGPE;
									break;
								default:
									gen = pksm::Generation::UNUSED;
									break;
							}
							break;
						case '8': // Not currently supported
							gen = pksm::Generation::EIGHT;
							break;
						default:
							gen = pksm::Generation::UNUSED;
							break;
					}

					if(gen != pksm::Generation::UNUSED) {
						FILE *in = fopen(fileName.c_str(), "rb");

						fseek(in, 0, SEEK_END);
						int size = ftell(in);
						fseek(in, 0, SEEK_SET);

						if(in) {
							u8 buffer[size];
							fread(buffer, 1, sizeof(buffer), in);
							if(topScreen)
								Banks::bank->pkm(*pksm::PKX::getPKM(gen, buffer), currentBankBox, pkmPos(pkmX, pkmY));
							else if(inParty)
								save->pkm(*save->transfer(*pksm::PKX::getPKM(gen, buffer)), pkmPos(pkmX, pkmY));
							else
								save->pkm(*save->transfer(*pksm::PKX::getPKM(gen, buffer)),
										  currentSaveBox,
										  pkmPos(pkmX, pkmY),
										  false);
							fclose(in);
						}
					}
				}

				// Reset & redraw screen
				chdir(path);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				if(topScreen)
					drawBox(topScreen);
				drawPokemonInfo(*currentPokemon(pkmX, pkmY));

				if(partyShown) {
					drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
					fillPartySprites();
				}

				if(!topScreen)
					setSpriteVisibility(arrowID, false, true);
				updateOam();
				goto back;
			} else if(menuSelection == 1) { // Create
				if(partyShown) {
					// Hide party Pokémon
					for(unsigned int i = 0; i < partyIconID.size(); i++) {
						setSpriteVisibility(partyIconID[i], false, false);
					}
					updateOam();
				}
				std::unique_ptr<pksm::PKX> pkm = save->emptyPkm()->clone();
				pkm->TID(save->TID());
				pkm->SID(save->SID());
				pkm->language(getSafeLanguage(pkm->generation(), Config::getLang("lang")));
				pkm->otName(save->otName());
				pkm->otGender(save->gender());
				pkm->ball(pksm::Ball::Poke);
				pkm->encryptionConstant(pksm::randomNumber(1, 0xFFFFFFFF));
				pkm->version(save->version());
				switch(pkm->version()) {
					case pksm::GameVersion::R:
					case pksm::GameVersion::S:
					case pksm::GameVersion::E:
						pkm->metLocation(0x0010); // Route 101 (RSE)
						break;
					case pksm::GameVersion::FR:
					case pksm::GameVersion::LG:
						pkm->metLocation(0x0065); // Route 1 (FRLG)
						break;
					case pksm::GameVersion::HG:
					case pksm::GameVersion::SS:
						pkm->metLocation(0x0095); // Route 1 (HGSS)
						break;
					case pksm::GameVersion::D:
					case pksm::GameVersion::P:
					case pksm::GameVersion::Pt:
						pkm->metLocation(0x0010); // Route 201 (DPPt)
						break;
					case pksm::GameVersion::B:
					case pksm::GameVersion::W:
					case pksm::GameVersion::B2:
					case pksm::GameVersion::W2:
						pkm->metLocation(0x000E); // Route 1 (BWB2W2)
						break;
					case pksm::GameVersion::X:
					case pksm::GameVersion::Y:
						pkm->metLocation(0x0008); // Route 1 (XY)
						break;
					case pksm::GameVersion::OR:
					case pksm::GameVersion::AS:
						pkm->metLocation(0x00CC); // Route 101 (ORAS)
						break;
					case pksm::GameVersion::SN:
					case pksm::GameVersion::MN:
					case pksm::GameVersion::US:
					case pksm::GameVersion::UM:
						pkm->metLocation(0x0006); // Route 1 (SMUSUM)
						break;
					case pksm::GameVersion::GP:
					case pksm::GameVersion::GE:
						pkm->metLocation(0x0003); // Route 1 (LGPE)
						break;
					case pksm::GameVersion::SW:
					case pksm::GameVersion::SH:
						pkm->metLocation(0x000C); // Route 1 (SWSH)
						break;
					default:
						break;
				}
				pkm->fixMoves();
				pkm->PID(pksm::PKX::getRandomPID(pkm->species(),
												 pkm->gender(),
												 pkm->version(),
												 pkm->nature(),
												 pkm->alternativeForm(),
												 pkm->abilityNumber(),
												 pkm->PID(),
												 pkm->generation()));
				pkm->metDate(Date::today());
				pkm->metLevel(pkm->generation() <= pksm::Generation::THREE ? 5 : 1);
				if(pkm->generation() == pksm::Generation::THREE) {
					pkm->level(5); // There was no level 1 before gen 4
					std::string name = i18n::species(Config::getLang("lang"), pkm->species());
					pkm->nickname(StringUtils::toUpper(name));
				}

				if(topScreen)
					Banks::bank->pkm(showPokemonSummary(*pkm), currentBankBox, pkmPos(pkmX, pkmY));
				else if(inParty)
					save->pkm(showPokemonSummary(*pkm), pkmPos(pkmX, pkmY));
				else
					save->pkm(showPokemonSummary(*pkm), currentSaveBox, pkmPos(pkmX, pkmY), false);

				// Redraw screen
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawBox(false);
				if(topScreen)
					drawBox(topScreen);
				drawPokemonInfo(*currentPokemon(pkmX, pkmY));
				if(partyShown) {
					save->fixParty();
					drawImage(PARTY_TRAY_X, PARTY_TRAY_Y, party, false, true);
					fillPartySprites();
				}
				goto back;
			} else if(menuSelection == 2) {
				goto back;
			}
		}

		setSpritePosition(
			arrowID,
			false,
			buttons[menuSelection].x +
				getTextWidthMaxW(i18n::localize(Config::getLang("lang"), buttons[menuSelection].label), 80) + 4,
			buttons[menuSelection].y);
		updateOam();
	}
	return false;
}
