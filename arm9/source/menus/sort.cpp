#include "sort.hpp"

#include "PKFilter.hpp"
#include "PKX.hpp"
#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "i18n.hpp"
#include "i18n_ext.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "sound.hpp"

std::vector<SortType> sortTypes;
std::vector<std::string> sortText = {
	"none",   "dexNo", "speciesName", "form",   "type1",      "type2",     "hp",          "attack",  "defense",
	"spAtk",  "spDef", "speed",       "hpIV",   "attackIV",   "defenseIV", "spAtkIV",     "spDefIV", "speedIV",
	"nature", "level", "trainerID",   "hpType", "friendship", "name",      "origTrainer", "shiny"};

bool sortPokemonFilter(const std::unique_ptr<pksm::PKX> &pkm1, const std::unique_ptr<pksm::PKX> &pkm2) {
	for(const auto &type : sortTypes) {
		switch(type) {
			case SortType::DEX:
				if(pkm1->species() < pkm2->species())
					return true;
				if(pkm2->species() < pkm1->species())
					return false;
				break;
			case SortType::FORM:
				if(pkm1->alternativeForm() < pkm2->alternativeForm())
					return true;
				if(pkm2->alternativeForm() < pkm1->alternativeForm())
					return false;
				break;
			case SortType::TYPE1:
				if(pkm1->type1() < pkm2->type1())
					return true;
				if(pkm2->type1() < pkm1->type1())
					return false;
				break;
			case SortType::TYPE2:
				if(pkm1->type2() < pkm2->type2())
					return true;
				if(pkm2->type2() < pkm1->type2())
					return false;
				break;
			case SortType::HP:
				if(pkm1->stat(pksm::Stat::HP) < pkm2->stat(pksm::Stat::HP))
					return true;
				if(pkm2->stat(pksm::Stat::HP) < pkm1->stat(pksm::Stat::HP))
					return false;
				break;
			case SortType::ATK:
				if(pkm1->stat(pksm::Stat::ATK) < pkm2->stat(pksm::Stat::ATK))
					return true;
				if(pkm2->stat(pksm::Stat::ATK) < pkm1->stat(pksm::Stat::ATK))
					return false;
				break;
			case SortType::DEF:
				if(pkm1->stat(pksm::Stat::DEF) < pkm2->stat(pksm::Stat::DEF))
					return true;
				if(pkm2->stat(pksm::Stat::DEF) < pkm1->stat(pksm::Stat::DEF))
					return false;
				break;
			case SortType::SATK:
				if(pkm1->stat(pksm::Stat::SPATK) < pkm2->stat(pksm::Stat::SPATK))
					return true;
				if(pkm2->stat(pksm::Stat::SPATK) < pkm1->stat(pksm::Stat::SPATK))
					return false;
				break;
			case SortType::SDEF:
				if(pkm1->stat(pksm::Stat::SPDEF) < pkm2->stat(pksm::Stat::SPDEF))
					return true;
				if(pkm2->stat(pksm::Stat::SPDEF) < pkm1->stat(pksm::Stat::SPDEF))
					return false;
				break;
			case SortType::SPE:
				if(pkm1->stat(pksm::Stat::SPDEF) < pkm2->stat(pksm::Stat::SPD))
					return true;
				if(pkm2->stat(pksm::Stat::SPDEF) < pkm1->stat(pksm::Stat::SPD))
					return false;
				break;
			case SortType::NATURE:
				if(pkm1->nature() < pkm2->nature())
					return true;
				if(pkm2->nature() < pkm1->nature())
					return false;
				break;
			case SortType::LEVEL:
				if(pkm1->level() < pkm2->level())
					return true;
				if(pkm2->level() < pkm1->level())
					return false;
				break;
			case SortType::TID:
				if(pkm1->TID() < pkm2->TID())
					return true;
				if(pkm2->TID() < pkm1->TID())
					return false;
				break;
			case SortType::HPIV:
				if(pkm1->iv(pksm::Stat::HP) < pkm2->iv(pksm::Stat::HP))
					return true;
				if(pkm2->iv(pksm::Stat::HP) < pkm1->iv(pksm::Stat::HP))
					return false;
				break;
			case SortType::ATKIV:
				if(pkm1->iv(pksm::Stat::ATK) < pkm2->iv(pksm::Stat::ATK))
					return true;
				if(pkm2->iv(pksm::Stat::ATK) < pkm1->iv(pksm::Stat::ATK))
					return false;
				break;
			case SortType::DEFIV:
				if(pkm1->iv(pksm::Stat::DEF) < pkm2->iv(pksm::Stat::DEF))
					return true;
				if(pkm2->iv(pksm::Stat::DEF) < pkm1->iv(pksm::Stat::DEF))
					return false;
				break;
			case SortType::SATKIV:
				if(pkm1->iv(pksm::Stat::SPATK) < pkm2->iv(pksm::Stat::SPATK))
					return true;
				if(pkm2->iv(pksm::Stat::SPATK) < pkm1->iv(pksm::Stat::SPATK))
					return false;
				break;
			case SortType::SDEFIV:
				if(pkm1->iv(pksm::Stat::SPDEF) < pkm2->iv(pksm::Stat::SPDEF))
					return true;
				if(pkm2->iv(pksm::Stat::SPDEF) < pkm1->iv(pksm::Stat::SPDEF))
					return false;
				break;
			case SortType::SPEIV:
				if(pkm1->iv(pksm::Stat::SPD) < pkm2->iv(pksm::Stat::SPD))
					return true;
				if(pkm2->iv(pksm::Stat::SPD) < pkm1->iv(pksm::Stat::SPD))
					return false;
				break;
			case SortType::HIDDENPOWER:
				if(pkm1->hpType() < pkm2->hpType())
					return true;
				if(pkm2->hpType() < pkm1->hpType())
					return false;
				break;
			case SortType::FRIENDSHIP:
				if(pkm1->currentFriendship() < pkm2->currentFriendship())
					return true;
				if(pkm2->currentFriendship() < pkm1->currentFriendship())
					return false;
				break;
			case SortType::NICKNAME:
				if(pkm1->nickname() < pkm2->nickname())
					return true;
				if(pkm2->nickname() < pkm1->nickname())
					return false;
				break;
			case SortType::SPECIESNAME:
				if(i18n::species(Config::getLang("lang"), pkm1->species()) <
				   i18n::species(Config::getLang("lang"), pkm2->species()))
					return true;
				if(i18n::species(Config::getLang("lang"), pkm2->species()) <
				   i18n::species(Config::getLang("lang"), pkm1->species()))
					return false;
				break;
			case SortType::OTNAME:
				if(pkm1->otName() < pkm2->otName())
					return true;
				if(pkm2->otName() < pkm1->otName())
					return false;
				break;
			case SortType::SHINY:
				if(pkm1->shiny() && !pkm2->shiny())
					return true;
				if(pkm2->shiny() && !pkm1->shiny())
					return false;
				break;
			default:
				break;
		}
	}
	return false;
}

void sortPokemon(bool top) {
	std::vector<std::unique_ptr<pksm::PKX>> sortPkm;
	while(!sortTypes.empty() && sortTypes.back() == SortType::NONE) {
		sortTypes.pop_back();
	}
	if(std::find(sortTypes.begin(), sortTypes.end(), SortType::DEX) == sortTypes.end()) {
		sortTypes.push_back(SortType::DEX);
	}
	if(top) {
		for(int i = 0; i < Banks::bank->boxes() * 30; i++) {
			if(Banks::bank->pkm(i / 30, i % 30)->species() != pksm::Species::None) {
				sortPkm.push_back(Banks::bank->pkm(i / 30, i % 30));
			}
		}
	} else {
		for(int i = 0; i < save->maxSlot(); i++) {
			if(save->pkm(i / 30, i % 30)->species() != pksm::Species::None) {
				sortPkm.push_back(save->pkm(i / 30, i % 30));
			}
		}
	}

	std::stable_sort(sortPkm.begin(), sortPkm.end(), sortPokemonFilter);

	if(top) {
		for(unsigned i = 0; i < sortPkm.size(); i++) {
			Banks::bank->pkm(*sortPkm[i], i / 30, i % 30);
		}
		for(int i = sortPkm.size(); i < Banks::bank->boxes() * 30; i++) {
			Banks::bank->pkm(*save->emptyPkm(), i / 30, i % 30);
		}
	} else {
		for(unsigned i = 0; i < sortPkm.size(); i++) {
			save->pkm(*sortPkm[i], i / 30, i % 30, false);
		}
		for(int i = sortPkm.size(); i < save->maxSlot(); i++) {
			save->pkm(*save->emptyPkm(), i / 30, i % 30, false);
		}
	}
}
void drawSortMenu(void) {
	// Clear screen
	drawImageDMA(0, 0, listBg, false, false);
	printText(i18n::localize(Config::getLang("lang"), "sort"), 4, 0, false, true);

	// Print items
	for(unsigned i = 0; i < sortTypes.size(); i++) {
		printText(i18n::localize(Config::getLang("lang"), "filter") + " " + std::to_string(i + 1) + ": " +
					  i18n::localize(Config::getLang("lang"), sortText[int(sortTypes[i])]),
				  4,
				  16 + (i * 16),
				  false,
				  true);
	}

	drawImage(253 - boxButton.width, 189 - boxButton.height, boxButton, false, false);
	printTextMaxW(i18n::localize(Config::getLang("lang"), "sort"),
				  boxButton.width - 8,
				  1,
				  260 - boxButton.width,
				  193 - boxButton.height,
				  false,
				  true);
}

void sortMenu(bool top) {
	sortTypes = {SortType::NONE, SortType::NONE, SortType::NONE, SortType::NONE, SortType::NONE};
	drawSortMenu();

	// Set arrow position
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID,
					  false,
					  4 +
						  getTextWidth(i18n::localize(Config::getLang("lang"), "filter") + " " + std::to_string(1) +
									   ": " + i18n::localize(Config::getLang("lang"), sortText[int(sortTypes[0])])) +
						  2,
					  10);
	// Hide all Pokémon sprites
	for(int i = 0; i < 30; i++) {
		setSpriteVisibility(i, false, false);
	}
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			break;
		} else if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < (int)sortTypes.size())
				selection++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < sortTypes.size(); i++) {
				if(touch.px <= 4 +
						   getTextWidth(i18n::localize(Config::getLang("lang"), "filter") + " " +
										std::to_string(selection + 1) + ": " +
										i18n::localize(Config::getLang("lang"), sortText[int(sortTypes[selection])])) &&
				   touch.py >= 16 + (i * 16) && touch.py <= 16 + ((i + 1) * 16)) {
					selection      = i;
					optionSelected = true;
					break;
				}
			}
			if(touch.px >= 253 - boxButton.width && touch.py >= 189 - boxButton.height) {
				selection      = sortTypes.size();
				optionSelected = true;
			}
		}

		if(optionSelected) {
			optionSelected = false;
			Sound::play(Sound::click);
			if(selection < (int)sortTypes.size()) {
				std::vector<std::string> sortTextLocalized;
				for(unsigned i = 0; i < sortText.size(); i++) {
					sortTextLocalized.push_back(i18n::localize(Config::getLang("lang"), sortText[i]));
				}
				sortTypes[selection] = selectItem(sortTypes[selection], 0, sortTextLocalized.size(), sortTextLocalized);
			} else {
				sortPokemon(top);
				break;
			}
			drawSortMenu();
		}

		// Move cursor
		if(selection < (int)sortTypes.size())
			setSpritePosition(
				arrowID,
				false,
				4 +
					getTextWidth(i18n::localize(Config::getLang("lang"), "filter") + " " +
								 std::to_string(selection + 1) + ": " +
								 i18n::localize(Config::getLang("lang"), sortText[int(sortTypes[selection])])) +
					2,
				(16 * (selection) + 10));
		else
			setSpritePosition(arrowID,
							  false,
							  260 - boxButton.width + getTextWidth(i18n::localize(Config::getLang("lang"), "sort")) + 2,
							  191 - boxButton.height - 3);
		updateOam();
	}
}
