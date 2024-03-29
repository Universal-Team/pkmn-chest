#include "filter.hpp"

#include "PKFilter.hpp"
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

std::vector<std::string> filterValues;
std::vector<bool> filterEnabled;
std::vector<bool> filterInversed;
std::array<std::string, 3> genders       = {"male", "female", "unknown"};
std::array<std::string, 10> filterLabels = {
	"species", "nature", "ability", "gender", "item", "ball", "form", "level", "moves", "shiny"};

void selectMoves(std::shared_ptr<pksm::PKFilter> filter) {
	// Clear screen
	drawImageDMA(0, 0, listBg, false, false);
	printText(i18n::localize(Config::getLang("lang"), "moves"), 4, 0, false, false);

	// Print moves
	for(int i = 0; i < 4; i++) {
		printText(filter->moveEnabled(i) ? "o" : "x", 4, 16 + (i * 16), false, false);
		printTextMaxW(i18n::move(Config::getLang("lang"), filter->move(i)), 100, 1, 20, 16 + (i * 16), false, false);
		printText(filter->moveInversed(i) ? "≠" : "=", 120, 16 + (i * 16), false, false);
	}

	// Set arrow position
	setSpritePosition(arrowID, false, 4 + getTextWidth(filter->moveEnabled(0) ? "o" : "x") + 2, 15);
	setSpriteVisibility(arrowID, false, true);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0, column = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < 3)
				selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)
				column--;
		} else if(pressed & KEY_RIGHT) {
			if(column < 2)
				column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < 4; i++) {
				if(touch.px >= 4 && touch.px <= 4 + getTextWidth(filter->moveEnabled(selection) ? "o" : "x") &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 0;
					selection      = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 20 &&
				   touch.px <= 20 + getTextWidth(i18n::move(Config::getLang("lang"), filter->move(selection))) &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 1;
					selection      = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 120 && touch.px <= 120 + getTextWidth(filter->moveInversed(selection) ? "≠" : "=") &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 2;
					selection      = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			Sound::play(Sound::click);
			if(column == 0) {
				filter->moveEnabled(selection, !filter->moveEnabled(selection));
			} else if(column == 1) {
				filter->move(selection,
							 selectItem(filter->move(selection),
										save->availableMoves(),
										i18n::rawMoves(Config::getLang("lang"))));
			} else if(column == 2) {
				filter->moveInversed(selection, !filter->moveInversed(selection));
			}

			// Clear screen
			drawImageDMA(0, 0, listBg, false, false);
			printText(i18n::localize(Config::getLang("lang"), "moves"), 4, 0, false, false);

			// Print moves
			for(int i = 0; i < 4; i++) {
				printText(filter->moveEnabled(i) ? "o" : "x", 4, 16 + (i * 16), false, false);
				printTextMaxW(
					i18n::move(Config::getLang("lang"), filter->move(i)), 100, 1, 20, 16 + (i * 16), false, false);
				printText(filter->moveInversed(i) ? "≠" : "=", 120, 16 + (i * 16), false, false);
			}
		}

		// Move cursor
		if(column == 0)
			setSpritePosition(arrowID,
							  false,
							  4 + getTextWidth(filter->moveEnabled(selection) ? "o" : "x") + 2,
							  (16 * (selection) + 10));
		else if(column == 1)
			setSpritePosition(arrowID,
							  false,
							  20 + getTextWidth(i18n::move(Config::getLang("lang"), filter->move(selection))) + 2,
							  (16 * (selection) + 10));
		else if(column == 2)
			setSpritePosition(arrowID,
							  false,
							  120 + getTextWidth(filter->moveInversed(selection) ? "≠" : "=") + 2,
							  (16 * (selection) + 10));
		updateOam();
	}
}

void drawFilterMenu(const std::shared_ptr<pksm::PKFilter> filter) {
	// Clear screen
	drawImageDMA(0, 0, listBg, false, false);
	printText(i18n::localize(Config::getLang("lang"), "filter"), 4, 0, false, false);

	// Fill filterValues
	filterValues.clear();
	filterValues.push_back(i18n::species(Config::getLang("lang"), filter->species()));
	filterValues.push_back(i18n::nature(Config::getLang("lang"), filter->nature()));
	filterValues.push_back(i18n::ability(Config::getLang("lang"), filter->ability()));
	filterValues.push_back(i18n::localize(Config::getLang("lang"), genders[u8(filter->gender())]));
	filterValues.push_back(i18n::item(Config::getLang("lang"), filter->heldItem()));
	filterValues.push_back(i18n::ball(Config::getLang("lang"), filter->ball()));
	filterValues.push_back(std::to_string(filter->alternativeForm()));
	filterValues.push_back(std::to_string(filter->level()));
	filterValues.push_back("……");
	filterValues.push_back(filter->shiny() ? i18n::localize(Config::getLang("lang"), "yes")
										   : i18n::localize(Config::getLang("lang"), "no"));

	// Fill filterEnabled
	filterEnabled.clear();
	filterEnabled.push_back(filter->speciesEnabled());
	filterEnabled.push_back(filter->natureEnabled());
	filterEnabled.push_back(filter->abilityEnabled());
	filterEnabled.push_back(filter->genderEnabled());
	filterEnabled.push_back(filter->heldItemEnabled());
	filterEnabled.push_back(filter->ballEnabled());
	filterEnabled.push_back(filter->alternativeFormEnabled());
	filterEnabled.push_back(filter->levelEnabled());
	filterEnabled.push_back(filter->moveEnabled(0) | filter->moveEnabled(1) | filter->moveEnabled(2) |
							filter->moveEnabled(3));
	filterEnabled.push_back(filter->shinyEnabled());

	// Fill filterInversed
	filterInversed.clear();
	filterInversed.push_back(filter->speciesInversed());
	filterInversed.push_back(filter->natureInversed());
	filterInversed.push_back(filter->abilityInversed());
	filterInversed.push_back(filter->genderInversed());
	filterInversed.push_back(filter->heldItemInversed());
	filterInversed.push_back(filter->ballInversed());
	filterInversed.push_back(filter->alternativeFormInversed());
	filterInversed.push_back(filter->levelInversed());
	filterInversed.push_back(filter->moveInversed(0) | filter->moveInversed(1) | filter->moveInversed(2) |
							 filter->moveInversed(3));
	filterInversed.push_back(filter->shinyInversed());

	// Print items
	for(unsigned i = 0; i < filterLabels.size(); i++) {
		printText(filterEnabled[i] ? "o" : "x", 4, 16 + (i * 16), false, false);
		printTextMaxW(
			i18n::localize(Config::getLang("lang"), filterLabels[i]), 100, 1, 20, 16 + (i * 16), false, false);
		printText(filterInversed[i] ? "≠" : "=", 120, 16 + (i * 16), false, false);
		printTextMaxW(filterValues[i], 100, 1, 136, 16 + (i * 16), false, false);
	}
}

void changeFilter(std::shared_ptr<pksm::PKFilter> filter) {
	drawFilterMenu(filter);

	// Set arrow position
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, 4 + getTextWidth(filterEnabled[0] ? "o" : "x") + 2, 10);
	// Hide all Pokémon sprites
	for(int i = 0; i < 30; i++) {
		setSpriteVisibility(i, false, false);
	}
	updateOam();

	bool optionSelected = false;
	int held, pressed, column = 0, selection = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			break;
		} else if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < (int)filterLabels.size() - 1)
				selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)
				column--;
		} else if(pressed & KEY_RIGHT) {
			if(column < 2)
				column++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < filterLabels.size(); i++) {
				if(touch.px >= 4 && touch.px <= 4 + getTextWidth(filter->moveEnabled(selection) ? "o" : "x") &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 0;
					selection      = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 120 && touch.px <= 120 + getTextWidth(filter->moveInversed(selection) ? "≠" : "=") &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 1;
					selection      = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 136 &&
				   touch.px <= 136 + getTextWidth(i18n::move(Config::getLang("lang"), filter->move(selection))) &&
				   touch.py >= 15 + (i * 16) && touch.py <= 15 + ((i + 1) * 16)) {
					column         = 2;
					selection      = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			if(column == 0) {
				switch(selection) {
					case 0:
						filter->speciesEnabled(!filter->speciesEnabled());
						break;
					case 1:
						filter->natureEnabled(!filter->natureEnabled());
						break;
					case 2:
						filter->abilityEnabled(!filter->abilityEnabled());
						break;
					case 3:
						filter->genderEnabled(!filter->genderEnabled());
						break;
					case 4:
						filter->heldItemEnabled(!filter->heldItemEnabled());
						break;
					case 5:
						filter->ballEnabled(!filter->ballEnabled());
						break;
					case 6:
						filter->alternativeFormEnabled(!filter->alternativeFormEnabled());
						break;
					case 7:
						filter->levelEnabled(!filter->levelEnabled());
						break;
					case 8:
						if(filter->moveEnabled(0) | filter->moveEnabled(1) | filter->moveEnabled(2) |
						   filter->moveEnabled(3)) {
							for(int i = 0; i < 4; i++)
								filter->moveEnabled(i, false);
						} else {
							for(int i = 0; i < 4; i++)
								filter->moveEnabled(i, true);
						}
						break;
					case 9:
						filter->shinyEnabled(!filter->shinyEnabled());
						break;
				}
			} else if(column == 1) {
				switch(selection) {
					case 0:
						filter->speciesInversed(!filter->speciesInversed());
						break;
					case 1:
						filter->natureInversed(!filter->natureInversed());
						break;
					case 2:
						filter->abilityInversed(!filter->abilityInversed());
						break;
					case 3:
						filter->genderInversed(!filter->genderInversed());
						break;
					case 4:
						filter->heldItemInversed(!filter->heldItemInversed());
						break;
					case 5:
						filter->ballInversed(!filter->ballInversed());
						break;
					case 6:
						filter->alternativeFormInversed(!filter->alternativeFormInversed());
						break;
					case 7:
						filter->levelInversed(!filter->levelInversed());
						break;
					case 8:
						if(filter->moveInversed(0) | filter->moveInversed(1) | filter->moveInversed(2) |
						   filter->moveInversed(3)) {
							for(int i = 0; i < 4; i++)
								filter->moveInversed(i, false);
						} else {
							for(int i = 0; i < 4; i++)
								filter->moveInversed(i, true);
						}
						break;
					case 9:
						filter->shinyInversed(!filter->shinyInversed());
						break;
				}
			} else if(column == 2) {
				switch(selection) {
					case 0: { // Species
						filter->species(selectItem(
							filter->species(), save->availableSpecies(), i18n::rawSpecies(Config::getLang("lang"))));
						break;
					}
					case 1: { // Nature
						pksm::Nature nature = selectNature(filter->nature());
						if(nature != pksm::Nature::INVALID)
							filter->nature(nature);
						break;
					}
					case 2: { // Ability
						filter->ability(selectItem(filter->ability(),
												   save->availableAbilities(),
												   i18n::rawAbilities(Config::getLang("lang"))));
						break;
					}
					case 3: { // Gender
						std::vector<std::string> genderList(genders.size());
						for(unsigned int i = 0; i < genders.size(); i++) {
							genderList[i] = i18n::localize(Config::getLang("lang"), genders[i]);
						}
						filter->gender(selectItem(filter->gender(), 0, genderList.size(), genderList));
						break;
					}
					case 4: { // Held item
						filter->heldItem(selectItem<int>(
							filter->heldItem(), save->availableItems(), i18n::rawItems(Config::getLang("lang"))));
						break;
					}
					case 5: { // Ball
						pksm::Ball ball = selectPokeball(filter->ball());
						if(ball != pksm::Ball::INVALID)
							filter->ball(ball);
						break;
					}
					case 6: { // Alt. form
						filter->alternativeForm(Input::getInt(28));
						break;
					}
					case 7: { // Level
						filter->level(Input::getInt(100));
						break;
					}
					case 8: { // Moves
						selectMoves(filter);
						break;
					}
					case 9: { // Shiny
						filter->shiny(!filter->shiny());
						break;
					}
				}
			}
			drawFilterMenu(filter);
			setSpriteVisibility(arrowID, false, true);
			updateOam();
		}

		// Move cursor
		if(column == 0)
			setSpritePosition(
				arrowID, false, 4 + getTextWidth(filterEnabled[selection] ? "o" : "x") + 2, (16 * (selection) + 10));
		else if(column == 1)
			setSpritePosition(
				arrowID, false, 120 + getTextWidth(filterInversed[selection] ? "≠" : "=") + 2, (16 * (selection) + 10));
		else if(column == 2)
			setSpritePosition(arrowID, false, 136 + getTextWidth(filterValues[selection]) + 2, (16 * (selection) + 10));
		updateOam();
	}
}
