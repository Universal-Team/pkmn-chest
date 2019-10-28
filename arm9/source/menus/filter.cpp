#include "filter.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "PKFilter.hpp"
#include "sound.hpp"

std::vector<std::string> filterValues;
std::vector<bool> filterEnabled;
std::vector<bool> filterInversed;
std::vector<std::string> genders = {Lang::male, Lang::female, "Genderless"}; // TODO: Add to lang

void selectMoves(std::shared_ptr<PKFilter> &filter) {
	// Clear screen
	if(sdFound())	drawImageSegmentDMA(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, summaryBgData.width, false);
	else	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);
	printText(Lang::movesString, 4, 1, false);

	// Print moves
	for(int i=0;i<4;i++) {
		printText(filter->moveEnabled(i) ? "√" : "x", 4, 17+(i*16), false);
		printTextMaxW(Lang::moves[filter->move(i)], 100, 1, 20, 17+(i*16), false);
		printText(filter->moveInversed(i) ? "≠" : "=", 120, 17+(i*16), false);
	}

	// Set arrow position
	setSpritePosition(arrowID, false, 4+getTextWidth(filter->moveEnabled(0) ? "√" : "x")+2, 15);
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
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
		} else if(held & KEY_DOWN) {
			if(selection < 3)	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(pressed & KEY_RIGHT) {
			if(column < 2)	column++;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<4;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(filter->moveEnabled(selection) ? "√" : "x") && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 20 && touch.px <= 20+getTextWidth(Lang::moves[filter->move(selection)]) && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 1;
					selection = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 120 && touch.px <= 120+getTextWidth(filter->moveInversed(selection) ? "≠" : "=") && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 2;
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			if(column == 0) {
				filter->moveEnabled(selection, !filter->moveEnabled(selection));
			} else if(column == 1) {
				filter->move(selection, selectItem(filter->move(selection), 0, save->maxMove()+1, Lang::moves));
			} else if(column == 2) {
				filter->moveInversed(selection, !filter->moveInversed(selection));
			}

			// Clear screen
			if(sdFound())	drawImageSegmentDMA(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, summaryBgData.width, false);
			else	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);
			printText(Lang::movesString, 4, 1, false);

			// Print moves
			for(int i=0;i<4;i++) {
				printText(filter->moveEnabled(i) ? "√" : "x", 4, 17+(i*16), false);
				printTextMaxW(Lang::moves[filter->move(i)], 100, 1, 20, 17+(i*16), false);
				printText(filter->moveInversed(i) ? "≠" : "=", 120, 17+(i*16), false);
			}
		}

		// Move cursor
		if(column == 0)	setSpritePosition(arrowID, false, 4+getTextWidth(filter->moveEnabled(selection) ? "√" : "x")+2, (16*(selection)+15));
		else if(column == 1)	setSpritePosition(arrowID, false, 20+getTextWidth(Lang::moves[filter->move(selection)])+2, (16*(selection)+15));
		else if(column == 2)	setSpritePosition(arrowID, false, 120+getTextWidth(filter->moveInversed(selection) ? "≠" : "=")+2, (16*(selection)+15));
		updateOam();
	}
}

void drawFilterMenu(std::shared_ptr<PKFilter> &filter) {
	// Clear screen
	if(sdFound())	drawImageSegmentDMA(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, summaryBgData.width, false);
	else {
		drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);
	}
	printText(Lang::filter, 4, 1, false);

	// Fill filterValues
	filterValues.clear();
	filterValues.push_back(Lang::species[filter->species()]);
	filterValues.push_back(Lang::natures[filter->nature()]);
	filterValues.push_back(Lang::abilities[filter->ability()]);
	filterValues.push_back(genders[filter->gender()]);
	filterValues.push_back(Lang::items[filter->heldItem()]);
	filterValues.push_back(std::to_string(filter->ball()));
	filterValues.push_back(std::to_string(filter->alternativeForm()));
	filterValues.push_back(std::to_string(filter->level()));
	filterValues.push_back("……");
	filterValues.push_back(filter->shiny() ? Lang::yes : Lang::no);

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
	filterEnabled.push_back(filter->moveEnabled(0) | filter->moveEnabled(1) | filter->moveEnabled(2) | filter->moveEnabled(3));
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
	filterInversed.push_back(filter->moveInversed(0) | filter->moveInversed(1) | filter->moveInversed(2) | filter->moveInversed(3));
	filterInversed.push_back(filter->shinyInversed());

	// Print items
	for(unsigned i=0;i<Lang::filterLabels.size();i++) {
		printText(filterEnabled[i] ? "o" : "x", 4, 17+(i*16), false);
		printTextMaxW(Lang::filterLabels[i], 100, 1, 20, 17+(i*16), false);
		printText(filterInversed[i] ? "≠" : "=", 120, 17+(i*16), false);
		printTextMaxW(filterValues[i], 100, 1, 136, 17+(i*16), false);
	}
}

void changeFilter(std::shared_ptr<PKFilter> &filter) {
	drawFilterMenu(filter);

	// Set arrow position
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, 4+getTextWidth(filterEnabled[0] ? "√" : "x")+2, (15));
	// Hide all Pokémon sprites
	for(int i=0;i<30;i++) {
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
			held = keysDownRepeat();
		} while(!held);

		if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			break;
		} else if(held & KEY_UP) {
			if(selection > 0)	selection--;
		} else if(held & KEY_DOWN) {
			if(selection < (int)Lang::filterLabels.size()-1)	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(pressed & KEY_RIGHT) {
			if(column < 2)	column++;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<Lang::filterLabels.size();i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(filter->moveEnabled(selection) ? "√" : "x") && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 120 && touch.px <= 120+getTextWidth(filter->moveInversed(selection) ? "≠" : "=") && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 1;
					selection = i;
					optionSelected = true;
					break;
				}
				if(touch.px >= 136 && touch.px <= 136+getTextWidth(Lang::moves[filter->move(selection)]) && touch.py >= 15+(i*16) && touch.py <= 15+((i+1)*16)) {
					column = 2;
					selection = i;
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
				switch (selection) {
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
						if(filter->moveEnabled(0) | filter->moveEnabled(1) | filter->moveEnabled(2) | filter->moveEnabled(3)) {
							for(int i=0;i<4;i++)	filter->moveEnabled(i, false);
						} else {
							for(int i=0;i<4;i++)	filter->moveEnabled(i, true);
						}
						break;
					case 9:
						filter->shinyEnabled(!filter->shinyEnabled());
						break;
				}
			} else if(column == 1) {
				switch (selection) {
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
						if(filter->moveInversed(0) | filter->moveInversed(1) | filter->moveInversed(2) | filter->moveInversed(3)) {
							for(int i=0;i<4;i++)	filter->moveInversed(i, false);
						} else {
							for(int i=0;i<4;i++)	filter->moveInversed(i, true);
						}
						break;
					case 9:
						filter->shinyInversed(!filter->shinyInversed());
						break;
				}
			} else if(column == 2) {
				switch(selection) {
					case 0: // Species
						filter->species(selectItem(filter->species(), 0, save->maxSpecies(), Lang::species));
						break;
					case 1: // Nature
						filter->nature(selectNature(filter->nature()));
						break;
					case 2: // Ability
						filter->ability(selectItem(filter->ability(), 0, save->maxAbility(), Lang::abilities));
						break;
					case 3: // Gender
						filter->gender(selectItem(filter->gender(), 0, genders.size(), genders));
						break;
					case 4: // Held item
						filter->heldItem(selectItem(filter->heldItem(), 0, save->maxItem(), Lang::items));
						break;
					case 5: // Ball
						filter->ball(selectPokeball(filter->ball()));
						break;
					case 6: // Alt. form
						filter->alternativeForm(Input::getInt(28));
						break;
					case 7: // Level
						filter->level(Input::getInt(100));
						break;
					case 8: // Moves
						selectMoves(filter);
						break;
					case 9: // Shiny
						filter->shiny(!filter->shiny());
						break;
				}
			}
			drawFilterMenu(filter);
			setSpriteVisibility(arrowID, false, true);
			updateOam();
		}

		// Move cursor
		if(column == 0)	setSpritePosition(arrowID, false, 4+getTextWidth(filterEnabled[selection] ? "√" : "x")+2, (16*(selection)+15));
		else if(column == 1)	setSpritePosition(arrowID, false, 120+getTextWidth(filterInversed[selection] ? "≠" : "=")+2, (16*(selection)+15));
		else if(column == 2)	setSpritePosition(arrowID, false, 136+getTextWidth(filterValues[selection])+2, (16*(selection)+15));
		updateOam();
	}
}
