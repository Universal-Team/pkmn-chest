#include "bag.hpp"
#include <strings.h>

#include "colors.hpp"
#include "config.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "i18n.hpp"
#include "input.hpp"
#include "Item.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "sound.hpp"

#define entriesPerScreen 11

int getMaxItem(int pouchIndex) {
	for(int i=save->pouches()[pouchIndex].second-1;i>0;i--) {
		if(save->item(save->pouches()[pouchIndex].first, i)->id() != 0) {
			return std::min(i+1, save->pouches()[pouchIndex].second);
		}
	}
	return 0;
}

void drawBag(Sav::Pouch pouch, int maxItem, int screenPos, bool background) {
	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw background
	if(background) {
		drawImageDMA(0, 0, listBg, false, false);

		// Draw search icon
		drawImage(256-search.width, 0, search, false, false);

		// Draw pouch buttons
		for(unsigned i=0;i<save->pouches().size();i++) {
			drawImageScaled(170, (104-(10*save->pouches().size()))+i*(20), 1, (float)20/boxButton.height, boxButton, false, false);
			printTextMaxW(save->pouchName(Config::getLang("lang"), save->pouches()[i].first), boxButton.width-8, 1, 174, (104-(10*save->pouches().size()))+i*(20)+2, false, false);
		}
	}

	printText(save->pouchName(Config::getLang("lang"), pouch), 4, 0, false, true);

	// Print items
	for(int i=0;i<std::min(entriesPerScreen, maxItem+1);i++) {
		printTextMaxW(i18n::item(Config::getLang("lang"), save->item(pouch, screenPos+i)->id()), 127, 1, 30, 16+(i*16), false, true);
		printText(std::to_string(save->item(pouch, screenPos+i)->count()), 4, 16+(i*16), false, true);
	}
}

void editBag(void) {
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, 4+getTextWidth(std::to_string(save->item(save->pouches()[0].first, 0)->count()))+2, 10);
	updateOam();

	int maxItem = getMaxItem(0);
	drawBag(save->pouches()[0].first, maxItem, 0, true);

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
			if(save->pouches()[selectedPouch].first != Sav::Pouch::KeyItem) {
				Sound::play(Sound::click);
				optionSelected = true;
			}
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			break;
		} else if(held & KEY_Y) {
			search:
			std::string str = Input::getLine(-1);
			if(str != "") {
				for(int i=0;i<save->pouches()[selectedPouch].second;i++) {
					if(strncasecmp(str.c_str(), i18n::item(Config::getLang("lang"), save->item(save->pouches()[selectedPouch].first, i)->id()).c_str(), str.size()) == 0) {
						selection = i;
						break;
					}
				}
			}
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos, true);
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
			changePouch:
			selection = 0;
			screenPos = 0;
			maxItem = getMaxItem(selectedPouch);
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos, false);
		} else if(held & KEY_R) {
			if(selectedPouch < (int)save->pouches().size()-1)	selectedPouch++;
			else	selectedPouch = 0;
			goto changePouch;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.px >= 256-search.width && touch.py <= search.height) {
				goto search;
			}
			for(int i=0;i<std::min(entriesPerScreen, maxItem+1);i++) {
				if(touch.px <= 30 && touch.py >= 16+(i*16) && touch.py <= 16+((i+1)*16)) {
					column = 0;
					selection = screenPos+i;
					if(save->pouches()[selectedPouch].first != Sav::Pouch::KeyItem) {
						optionSelected = true;
					}
					break;
				} else if(touch.px > 30 && touch.px <= 160 && touch.py >= 16+(i*16) && touch.py <= 16+((i+1)*16)) {
					column = 1;
					selection = screenPos+i;
					if(save->pouches()[selectedPouch].first != Sav::Pouch::KeyItem) {
						optionSelected = true;
					}
					break;
				}
			}
			for(unsigned i=0;i<save->pouches().size();i++) {
				if(touch.px >= 170 && touch.py >= (104-(10*save->pouches().size()))+i*(20) && touch.py <= (104-(10*save->pouches().size()))+(i+1)*(20)) {
					selectedPouch = i;
					goto changePouch;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			if(column == 0) {
				int num = Input::getInt(save->item(save->pouches()[selectedPouch].first, selection)->maxCount());
				if(num != -1) {
					std::unique_ptr<Item> item = save->item(save->pouches()[selectedPouch].first, selection);
					item->count(num);
					save->item(*item, save->pouches()[selectedPouch].first, selection);
				}
			} else if(column == 1) {
				std::unique_ptr<Item> item = save->item(save->pouches()[selectedPouch].first, selection);

				// Create list of valid items
				int currentItem = 0;
				std::vector<std::string> validItems;
				validItems.push_back(i18n::item(Config::getLang("lang"), 0));
				for(unsigned i=0;i<save->validItems()[save->pouches()[selectedPouch].first].size();i++) {
					validItems.push_back(i18n::item(Config::getLang("lang"), save->validItems()[save->pouches()[selectedPouch].first][i]));
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
						for(unsigned i=0;i<i18n::rawItems(Config::getLang("lang")).size();i++) {
							if(i18n::item(Config::getLang("lang"), i) == validItems[num]) {
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
			}
			setSpriteVisibility(arrowID, false, true);
			updateOam();
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos, true);
		}

		// Scroll screen if needed
		if(selection < screenPos) {
			screenPos = selection;
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos, false);
		} else if(selection > screenPos + entriesPerScreen - 1) {
			screenPos = selection - entriesPerScreen + 1;
			drawBag(save->pouches()[selectedPouch].first, maxItem, screenPos, false);
		}

		// Move cursor
		if(column == 0)	setSpritePosition(arrowID, false, 4+getTextWidth(std::to_string(save->item(save->pouches()[selectedPouch].first, selection)->count()))+2, (16*(selection-screenPos)+10));
		else if(column == 1)	setSpritePosition(arrowID, false, 30+getTextWidth(i18n::item(Config::getLang("lang"), save->item(save->pouches()[selectedPouch].first, selection)->id()))+2, (16*(selection-screenPos)+10));
		updateOam();
	}
}