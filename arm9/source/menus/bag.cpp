#include "bag.hpp"
#include <strings.h>

#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "sound.hpp"

#define entriesPerScreen 11

int getMaxItem(int pouchIndex) {
	for(int i=save->pouches()[pouchIndex].second;i>0;i--) {
		if(save->item(save->pouches()[pouchIndex].first, i)->id() != 0) {
			return std::min(i+1, save->pouches()[pouchIndex].second);
		}
	}
	return 0;
}

void drawBag(Pouch pouch, int maxItem, int screenPos) {
	// Clear screen
	if(sdFound())	drawImageDMA(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, false);
	else {
		drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
		drawRectangle(145, 0, 111, 88, LIGHT_GRAY, false);
	}

	printText(save->pouchName(pouch), 4, 1, false);

	// Draw search icon
	drawImage(256-searchData.width, 16, searchData.width, searchData.height, search, false);

	// Print items
	for(int i=0;i<std::min(entriesPerScreen, maxItem+1);i++) {
		printTextMaxW(Lang::items[save->item(pouch, screenPos+i)->id()], 110, 1, 4, 17+(i*16), false);
		printText(std::to_string(save->item(pouch, screenPos+i)->count()), 120, 17+(i*16), false);
	}
}

void editBag(void) {
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, 4+getTextWidth(Lang::items[save->item(save->pouches()[0].first, 0)->id()]), 15);
	updateOam();

	int maxItem = getMaxItem(0);
	drawBag(save->pouches()[0].first, maxItem, 0);

	bool optionSelected = false;
	int held, pressed, column = 0, selection = 0, selectedPouch = 0, screenPos = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(pressed & KEY_A) {
			if(save->pouches()[selectedPouch].first != Pouch::KeyItem) {
				Sound::play(Sound::click);
				optionSelected = true;
			}
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			break;
		} else if(held & KEY_Y) {
			search:
			std::string str = Input::getLine(-1);
			if(str != "") {
				for(int i=0;i<save->pouches()[selectedPouch].second;i++) {
					if(strncasecmp(str.c_str(), Lang::items[save->item(save->pouches()[selectedPouch].first, i)->id()].c_str(), str.size()) == 0) {
						selection = i;
						// Redraw if not going to scroll
						if((selection >= screenPos) && (selection < screenPos+entriesPerScreen)) {
							drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
						}
						break;
					}
				}
			} else {
				drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
			}
		} else if(held & KEY_UP) {
			if(selection > 0) selection--;
			else	selection = maxItem;
		} else if(held & KEY_DOWN) {
			if(selection < maxItem)	selection++;
			else	selection = 0;
		} else if(held & KEY_LEFT) {
			if(column == 0) {
				selection -= entriesPerScreen-1;
			} else {
				column = 0;
			}

			if(selection < 0)	selection = 0;
		} else if(held & KEY_RIGHT) {
			if(column == 1) {
				selection += entriesPerScreen-1;
			} else {
				column = 1;
			}
			if(selection > maxItem)	selection = maxItem;
		} else if(held & KEY_L) {
			if(selectedPouch > 0)	selectedPouch--;
			else	selectedPouch = save->pouches().size()-1;
			selection = 0;
			screenPos = 0;
			maxItem = getMaxItem(selectedPouch);
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
		} else if(held & KEY_R) {
			if(selectedPouch < (int)save->pouches().size()-1)	selectedPouch++;
			else	selectedPouch = 0;
			selection = 0;
			screenPos = 0;
			maxItem = getMaxItem(selectedPouch);
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.px >= 256-searchData.width && touch.py >= 16 && touch.py <= 16+searchData.height) {
				goto search;
			}
			for(int i=0;i<entriesPerScreen;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(Lang::items[save->item(save->pouches()[selectedPouch].first, selection)->id()]) && touch.py >= 17+(i*16) && touch.py <= 17+((i+1)*16)) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				} else if(touch.px >= 4 && touch.px <= 120+getTextWidth(std::to_string(save->item(save->pouches()[selectedPouch].first, selection)->count())) && touch.py >= 17+(i*16) && touch.py <= 17+((i+1)*16)) {
					column = 1;
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
				std::unique_ptr<Item> item = save->item(save->pouches()[selectedPouch].first, selection);

				// Create list of valid items
				int currentItem = 0;
				std::vector<std::string> validItems;
				validItems.push_back(Lang::items[0]);
				for(unsigned i=0;i<save->validItems()[save->pouches()[selectedPouch].first].size();i++) {
					validItems.push_back(Lang::items[save->validItems()[save->pouches()[selectedPouch].first][i]]);
					if(save->validItems()[save->pouches()[selectedPouch].first][i] == item->id()) {
						currentItem = i+1;
					}
				}

				int num = selectItem(currentItem, 0, validItems.size(), validItems);
				if(num != item->id()) {
					// If `None` was selected, bring all items down by one
					if(num == 0) {
						for(int i=selection+1;i<save->pouches()[selectedPouch].second;i++) {
							save->item(*save->item(save->pouches()[selectedPouch].first, i), save->pouches()[selectedPouch].first, i-1);
						}
						// And clear the last one
						Item4 emptyItem;
						save->item(emptyItem, save->pouches()[selectedPouch].first, save->pouches()[selectedPouch].second);
						maxItem--;
					} else {
						// Convert back from the valid item list to the real item list
						for(unsigned i=0;i<Lang::items.size();i++) {
							if(Lang::items[i] == validItems[num]) {
								num = i;
								break;
							}
						}

						item->id(num);
						if(item->count() == 0)	item->count(1);
						save->item(*item, save->pouches()[selectedPouch].first, selection);
						if(selection == maxItem)	maxItem = std::min(maxItem+1, save->pouches()[selectedPouch].second);
					}
				}
			} else {
				int num = Input::getInt(999);
				if(num != -1) {
					std::unique_ptr<Item> item = save->item(save->pouches()[selectedPouch].first, selection);
					item->count(num);
					save->item(*item, save->pouches()[selectedPouch].first, selection);
				}
			}
			setSpriteVisibility(arrowID, false, true);
			updateOam();
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
		}

		// Scroll screen if needed
		if(selection < screenPos) {
			screenPos = selection;
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
		} else if(selection > screenPos + entriesPerScreen - 1) {
			screenPos = selection - entriesPerScreen + 1;
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos);
		}


		// Move cursor
		if(column == 0)	setSpritePosition(arrowID, false, 4+getTextWidth(Lang::items[save->item(save->pouches()[selectedPouch].first, selection)->id()]), (16*(selection-screenPos)+15));
		else if(column == 1)	setSpritePosition(arrowID, false, 120+getTextWidth(std::to_string(save->item(save->pouches()[selectedPouch].first, selection)->count())), (16*(selection-screenPos)+15));
		updateOam();
	}
}