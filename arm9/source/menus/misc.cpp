#include "misc.hpp"

#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "flashcard.hpp"
#include "input.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "summary.hpp"
#include "sound.hpp"

struct FormCount {
	int dexNo;
	int noForms;
} formCounts[] = {
	{201, 27}, // Unown
	{351,  4}, // Castform
	{386,  4}, // Deoxys
	{412,  3}, // Burmy
	{413,  3}, // Wormadam
	{422,  2}, // Shellos
	{423,  2}, // Gastrodon
	{479,  6}, // Rotom
	{487,  2}, // Giratina
	{492,  2}, // Shaymin
	{550,  2}, // Basculin
	{555,  2}, // Darmanitan
	{585,  4}, // Deerling
	{586,  4}, // Sawsbuck
	{648,  2}, // Meloetta
	{641,  2}, // Tornadus
	{642,  2}, // Thunderus
	{645,  2}, // Landorus
	{646,  3}, // Kyurem
	{647,  2}, // Keldeo
};

struct Text {
	int x;
	int y;
	char text[32];
} textStatsC1[] {
	{-20, 20}, {-20, 36}, {-20, 52}, {-20, 68}, {-20, 84}, {-20, 100},
}, textStatsC2[] {
	{20, 20}, {20, 36}, {20, 52}, {20, 68}, {20, 84}, {20, 100},
}, textStatsC3[] {
	{60, 20}, {60, 36}, {60, 52}, {60, 68}, {60, 84}, {60, 100},
}, textStatsC4[] {
	{100, 20}, {100, 36}, {100, 52}, {100, 68}, {100, 84}, {100, 100},
};

int pkmLang(void) {
	switch(Config::lang) {
		case 0:
			return 5; // German
		case 1:
		default:
			return 2; // English
		case 2:
			return 6; // Spanish
		case 3:
			return 3; // French
		case 4:
			return 4; // Italian
		case 5:
			return 1; // Japanese
	}
}

void drawMiniBoxes(int currentBox) {
	if(currentBox < 0)	currentBox = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1)+currentBox;
	// Clear text
	drawRectangle(210, 0, 46, 192, DARKERER_GRAY, DARKER_GRAY, false);

	for(int i=0;i<5;i++) {
		drawRectangle(170, 10+(i*33), 35, 30, WHITE, false);
		drawOutline(170, 10+(i*33), 35, 30, DARK_GRAY, false);
		for(int j=0;j<30;j++) {
			if((topScreen ? Banks::bank->pkm(currentBox, j)->species() : save->pkm(currentBox, j)->species()) != 0) {
				// Type 1
				int type = topScreen ? Banks::bank->pkm(currentBox, j)->type1() : save->pkm(currentBox, j)->type1();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(173+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*(typesData.width*(typesData.height/17)))+typesData.width+1], false);

				// Type 2
				type = topScreen ? Banks::bank->pkm(currentBox, j)->type2() : save->pkm(currentBox, j)->type2();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(175+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*(typesData.width*(typesData.height/17)))+typesData.width+1], false);
			}
		}
		// Print box number
		printText(std::to_string(currentBox+1), 210, 20+(i*33), false);
		if(currentBox < (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1))	currentBox++;
		else	currentBox = 0;
	}
}

int selectBox(int currentBox) {
	setSpritePosition(bottomArrowID, 205, 14);
	updateOam();
	drawMiniBoxes(currentBox);

	int pressed, held, screenPos = currentBox;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(currentBox > 0)	currentBox--;
			else {
				screenPos = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1)-(currentBox-screenPos-1);
				currentBox = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1);
			}
		} else if(held & KEY_DOWN) {
			if(currentBox < (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1))	currentBox++;
			else {
				screenPos = 0-(currentBox-screenPos+1);
				currentBox = 0;
			}
		} else if(pressed & KEY_A) {
			return currentBox;
		} else if(pressed & KEY_B) {
			return -1;
		}

		// Scroll screen if needed
		if(currentBox < screenPos) {
			screenPos = currentBox;
			drawMiniBoxes(screenPos);
		} else if(currentBox > screenPos+4) {
			screenPos = currentBox-4;
			drawMiniBoxes(screenPos);
		}

		// Move cursor
		setSpritePosition(bottomArrowID, 205, 14+(33*(currentBox-screenPos)));
		updateOam();
	}
}

int selectForm(int dexNo, int currentForm) {
	int altIndex = -1;
	for(unsigned i=0;i<(sizeof(formCounts)/sizeof(formCounts[0]));i++) {
		if(formCounts[i].dexNo == dexNo) {
			altIndex = i;
			break;
		}
	}
	if(altIndex == -1)	return -1; // No alternate forms
	else if(altIndex == 0) { // Unown
		int num = Input::getLine(1)[0];

		if(num == 33)	return 26; // !
		else if(num == 63)	return 27; // ?
		else if(num > 96 && num < 123)	return num-97; // a-z
		else	return -1;
	}

	// Draw background
	drawRectangle(0, 60, 256, 72, DARK_GRAY, false);
	drawOutline(0, 60, 256, 72, LIGHT_GRAY, false);

	// Draw forms
	for(int i=0;i<formCounts[altIndex].noForms;i++) {
		std::vector<u16> bmp;
		loadPokemonSprite(getPokemonIndex(dexNo, i), bmp);
		drawImage((i*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, bmp, false);
	}

	// Move arrow to current form
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (currentForm*32)+(128-((32*formCounts[altIndex].noForms)/2))+28, 84);
	updateOam();

	int pressed, held;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_LEFT) {
			if(currentForm > 0)	currentForm--;
			else	currentForm=formCounts[altIndex].noForms-1;
		} else if(held & KEY_RIGHT) {
			if(currentForm < formCounts[altIndex].noForms-1)	currentForm++;
			else currentForm=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return currentForm;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int i=0;i<5;i++) {
				if(touch.px > (i*32)+(128-((32*formCounts[altIndex].noForms)/2)) && touch.px < (i*32)+(128-((32*formCounts[altIndex].noForms)/2))+32 && touch.py > 72 && touch.py < 104) {
					Sound::play(Sound::click);
					return i;
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (currentForm*32)+(128-((32*formCounts[altIndex].noForms)/2))+28, 84);
		updateOam();
	}
}

void drawItemList(int screenPos, std::vector<std::string> itemList) {
	// Clear the screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Draw search icon
	drawImage(256-searchData.width, 0, searchData.width, searchData.height, search, false);

	// Print moves
	for(unsigned i=0;i<std::min(9u, itemList.size()-screenPos);i++) {
		printText(itemList[screenPos+i], 4, 4+(i*20), false);
	}
}

int selectItem(int current, int start, int max, std::vector<std::string> &items) {
	if(current < start || current > max)	current = start;
	// Set arrow position
	setSpritePosition(bottomArrowID, 4+getTextWidth(items[current]), -2);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();

	// Print moves
	std::vector<std::string> itemList(&items[start], &items[max]);
	drawItemList(current-start, itemList);

	int held, pressed, screenPos = current-start, newMove = current-start, entriesPerScreen = 9;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(newMove > 0)	newMove--;
			else	newMove = std::min(max-1, (int)itemList.size()-1);
		} else if(held & KEY_DOWN) {
			if(newMove < std::min(max-1, (int)itemList.size()-1))	newMove++;
			else newMove = 0;
		} else if(held & KEY_LEFT) {
			newMove -= entriesPerScreen;
			if(newMove < 0)	newMove = 0;
		} else if(held & KEY_RIGHT) {
			newMove += entriesPerScreen;
			if(newMove > std::min(max-1, (int)itemList.size()-1))	newMove = std::min(max, (int)itemList.size()-1);
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			for(int i=0;i<max;i++) {
				if(itemList[newMove] == items[i]) {
					return i;
				}
			}
		} if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return current;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<entriesPerScreen;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(itemList[screenPos+i]) && touch.py >= 4+(i*20) && touch.py <= 4+((i+1)*20)) {
					for(int j=0;j<max;j++) {
						if(itemList[screenPos+i] == items[j]) {
							return j;
						}
					}
					break;
				} else if(touch.px >= 256-searchData.width && touch.py <= searchData.height) {
					goto search;
				}
			}
		} else if(pressed & KEY_Y) {
			search:
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			std::string str = Input::getLine();
				itemList.clear();
				if(str != "")	itemList.push_back("-----");
				for(int i=0;i<max;i++) {
					if(strncasecmp(str.c_str(), items[i].c_str(), str.length()) == 0) {
						itemList.push_back(items[i]);
					}
				}
				newMove = 0;
				screenPos = 0;
			drawItemList(screenPos, itemList);
			setSpriteVisibility(bottomArrowID, true);
			updateOam();
		}

		// Scroll screen if needed
		if(newMove < screenPos) {
			screenPos = newMove;
			drawItemList(screenPos, itemList);
		} else if(newMove > screenPos + entriesPerScreen - 1) {
			screenPos = newMove - entriesPerScreen + 1;
			drawItemList(screenPos, itemList);
		}

		// Move cursor
		setSpritePosition(bottomArrowID, 4+getTextWidth(itemList[newMove]), (20*(newMove-screenPos)-2));
		updateOam();
	}
}

std::shared_ptr<PKX> selectMoves(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Print moves
	for(int i=0;i<4;i++) {
		printText(Lang::moves[pkm->move(i)], 4, 4+(i*20), false);
	}

	// Set arrow position
	setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[pkm->move(0)]), -2);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0;
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
		} else if(held & KEY_LEFT) {
			selection = 0;
		} else if(held & KEY_RIGHT) {
			selection = 3;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<4;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(Lang::moves[pkm->move(i)]) && touch.py >= 4+(i*20) && touch.py <= 4+((i+1)*20)) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			pkm->move(selection, selectItem(pkm->move(selection), 0, save->maxMove()+1, Lang::moves));

			// Clear the screen
			drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

			// Print moves
			for(int i=0;i<4;i++) {
				printText(Lang::moves[pkm->move(i)], 4, 4+(i*20), false);
			}
		}

		setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[pkm->move(selection)]), (selection*20)-2);
		updateOam();
	}
}

int selectNature(int currentNature) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Draw labels (not a for loop as speed is 3rd)
	{
		int x = -2;
		printTextCenteredTintedMaxW(Lang::statsLabels[1], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[2], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[5], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[3], 48, 1, RGB::BLUE, ((x++)*48), 4, false);
		printTextCenteredTintedMaxW(Lang::statsLabels[4], 48, 1, RGB::BLUE, ((x++)*48), 4, false);

		int y = 0;
		printTextTintedScaled(Lang::statsLabels[1], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[2], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[5], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[3], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
		printTextTintedScaled(Lang::statsLabels[4], 0.8, 0.8, RGB::RED, 1, ((y++)*32)+22, false);
	}

	// Print natures
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			printTextCenteredMaxW(Lang::natures[(y*5)+x], 48, 1, ((x-2)*48), (y*32)+32, false);
		}
	}

	int arrowX = currentNature-((currentNature/5)*5), selection = currentNature/5, pressed, held;
	// Move arrow to current nature
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[currentNature], 48)/2)+28, (selection*32)+24);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
			else	selection=4;
		} else if(held & KEY_DOWN) {
			if(selection < 4)	selection++;
			else	selection=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return (selection*5)+arrowX;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y=0;y<5;y++) {
				for(int x=0;x<5;x++) {
					if(touch.px > (x*48)+8 && touch.px < (x*48)+56 && touch.py > (y*32)+8 && touch.py < (y*32)+56) {
						Sound::play(Sound::click);
						return (y*5)+x;
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[(selection*5)+arrowX], 48)/2)+28, (selection*32)+24);
		updateOam();
	}
}

int selectPokeball(int currentBall) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Draw Pokéballs
	for(int y=0;y<5;y++) {
		for(int x=0;x<5;x++) {
			if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
				std::pair<int, int> xy = getPokeballPosition((y*5)+x+1);
				drawImageFromSheet((x*48)+24, (y*32)+24, 15, 15, ballSheet, ballSheetData.width, xy.first, xy.second, false);
			}
		}
	}

	currentBall--;
	int arrowX = currentBall-((currentBall/5)*5), selection = currentBall/5, pressed, held;
	// Move arrow to current ball
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+40, (selection*32)+16);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
			else	selection=4;
		} else if(held & KEY_DOWN) {
			if(selection < 4)	selection++;
			else	selection=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			if(!(save->generation() != Generation::FIVE && (selection*5)+arrowX == 24)) {
				Sound::play(Sound::click);
				return (selection*5)+arrowX+1;
			}
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y=0;y<5;y++) {
				for(int x=0;x<5;x++) {
					if(touch.px > (x*48)+8 && touch.px < (x*48)+56 && touch.py > (y*32)+8 && touch.py < (y*32)+56) {
						if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
							Sound::play(Sound::click);
							return (y*5)+x+1;
						}
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*48)+40, (selection*32)+16);
		updateOam();
	}
}

int selectWallpaper(int currentWallpaper) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Draw wallpapers
	for(int y=0;y<4;y++) {
		for(int x=0;x<6;x++) {
			std::string path = boxBgPath(false, (y*6)+x);
			std::vector<u16> img;
			ImageData imgData = loadPng(path, img);
			drawImageScaled((x*36)+28, (y*36)+28, imgData.width, imgData.height, 0.125, 0.125, img, false);
		}
	}

	int arrowX = currentWallpaper-((currentWallpaper/6)*6), selection = currentWallpaper/6, pressed, held;
	// Move arrow to current wallpaper
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*36)+44, (selection*36)+20);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
			else	selection=3;
		} else if(held & KEY_DOWN) {
			if(selection < 3)	selection++;
			else	selection=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=5;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 5)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return (selection*6)+arrowX;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y=0;y<4;y++) {
				for(int x=0;x<6;x++) {
					if(touch.px > (x*36)+20 && touch.px < (x*36)+56 && touch.py > (y*36)+20 && touch.py < (y*36)+56) {
						if(!(save->generation() != Generation::FIVE && (y*5)+x == 24)) {
							Sound::play(Sound::click);
							return (y*6)+x;
						}
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(bottomArrowID, (arrowX*36)+44, (selection*36)+20);
		updateOam();
	}
}

void drawOriginPage(std::shared_ptr<PKX> pkm, std::vector<std::string> &varText) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Print text
	varText = { 
		std::to_string(pkm->metLevel()),
		std::to_string(pkm->metYear()+2000),
		std::to_string(pkm->metMonth()),
		std::to_string(pkm->metDay()),
		pkm->gen4() ? (pkm->metLocation() > Lang::locations4.size() ? "" : Lang::locations4[pkm->metLocation()])
		: (pkm->metLocation() > Lang::locations5.size() ? "" : Lang::locations5[pkm->metLocation()]),
		Lang::games[pkm->version()],
	};
	for(unsigned i=0;i<Lang::originLabels.size();i++) {
		printText(Lang::originLabels[i]+": "+varText[i], 4, 4+(i*20), false);
	}
}

std::shared_ptr<PKX> selectOrigin(std::shared_ptr<PKX> pkm) {
	std::vector<std::string> varText;
	drawOriginPage(pkm, varText);

	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::originLabels[0]+": "+varText[0]), -2);
	updateOam();

	bool optionSelected = false;
	int pressed, held, selection = 0;
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
			if(selection < (int)Lang::originLabels.size()-1)	selection++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(unsigned i=0;i<Lang::originLabels.size();i++) {
				if(touch.py > 4+(i*20) && touch.py < 4+(i+1)*20) {
					selection = i;
					optionSelected = true;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			switch(selection) {
				case 0: { // Level
					int num = Input::getInt(100);
					if(num != -1)	pkm->metLevel(num);
					break;
				} case 1: { // Year
					int num = Input::getInt(2099);
					if(num != -1) {
						if(num < 2000)	pkm->metYear(std::min(num, 99));
						else	pkm->metYear(num-2000);

						if(pkm->metYear()%4 && pkm->metMonth() == 2 && pkm->metDay() > 28) {
							pkm->metDay(28);
						}
					}
					break;
				} case 2: { // Month
					int num = Input::getInt(12);
					if(num > 0) {
						pkm->metMonth(num);
						if(num == 2 && pkm->metDay() > (pkm->metYear()%4 ? 28 : 29)) {
							pkm->metDay(pkm->metYear()%4 ? 28 : 29);
						} else if((num == 4 || num == 6 || num == 9 || num == 11) && pkm->metDay() > 30) {
							pkm->metDay(30);
						}
					}
					break;
				} case 3: { // Day
					int num;
					switch(pkm->metMonth()) {
						case 2:
							num = Input::getInt(pkm->metYear()%4 ? 28 : 29);
							break;
						case 4:
						case 6:
						case 9:
						case 11:
							num = Input::getInt(30);
							break;
						default:
							num = Input::getInt(31);
							break;
					}
					if(num != -1)	pkm->metDay(num);
					break;
				} case 4: { // Location
					pkm->metLocation(selectItem(pkm->metLocation(), 0, pkm->gen4() ? Lang::locations4.size() : Lang::locations5.size(), pkm->gen4() ? Lang::locations4 : Lang::locations5));
					break;
				} case 5: { // Game
					pkm->version(selectItem(pkm->version(), 0, Lang::games.size(), Lang::games));
					break;
				}
			}
			drawOriginPage(pkm, varText);
			setSpriteVisibility(bottomArrowID, true);
			updateOam();
		}

		// Move arrow
		setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::originLabels[selection]+": "+varText[selection]), (selection*20)-2);
		updateOam();
	}
}

void drawStatsPage(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false);

	// Draw lines
	for(unsigned i=1;i<(sizeof(textStatsC1)/sizeof(textStatsC1[0]));i++) {
		drawRectangle(16, textStatsC1[i].y, 230, 1, LIGHT_GRAY, false);
	}
	drawRectangle(128, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(168, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(208, 4, 1, 112, LIGHT_GRAY, false);

	// Print stat info labels
	int i = pkm->nature();
	printText(Lang::statsLabels[0], 20, textStatsC1[0].y, false);
	printTextTintedMaxW(Lang::statsLabels[1], 80, 1, (i!=0&&i<5         ? RGB::RED : i!=0&&!(i%5)      ? RGB::BLUE : WHITE), 20, textStatsC1[1].y, false);
	printTextTintedMaxW(Lang::statsLabels[2], 80, 1, (i!=6&&i>4&&i<10   ? RGB::RED : i!=6&&!((i-1)%5)  ? RGB::BLUE : WHITE), 20, textStatsC1[2].y, false);
	printTextTintedMaxW(Lang::statsLabels[3], 80, 1, (i!=18&&i>14&&i<20 ? RGB::RED : i!=18&&!((i-3)%5) ? RGB::BLUE : WHITE), 20, textStatsC1[3].y, false);
	printTextTintedMaxW(Lang::statsLabels[4], 80, 1, (i!=24&&i>19       ? RGB::RED : i!=24&&!((i-4)%5) ? RGB::BLUE : WHITE), 20, textStatsC1[4].y, false);
	printTextTintedMaxW(Lang::statsLabels[5], 80, 1, (i!=12&&i>9&&i<15  ? RGB::RED : i!=12&&!((i-2)%5) ? RGB::BLUE : WHITE), 20, textStatsC1[5].y, false);

	// Print column titles
	printTextCenteredMaxW(Lang::statsLabels[6], 30, 1, textStatsC1[0].x, textStatsC1[0].y-16, false);
	printTextCentered(Lang::statsLabels[7], textStatsC2[0].x, textStatsC2[0].y-16, false);
	printTextCentered(Lang::statsLabels[8], textStatsC3[0].x, textStatsC3[0].y-16, false);
	printTextCenteredMaxW(Lang::statsLabels[9], 30, 1, textStatsC4[0].x, textStatsC4[0].y-16, false);

	// Set base stat info
	snprintf(textStatsC1[0].text,  sizeof(textStatsC1[0].text), "%i", pkm->baseHP());
	snprintf(textStatsC1[1].text,  sizeof(textStatsC1[1].text), "%i", pkm->baseAtk());
	snprintf(textStatsC1[2].text,  sizeof(textStatsC1[2].text), "%i", pkm->baseDef());
	snprintf(textStatsC1[3].text,  sizeof(textStatsC1[3].text), "%i", pkm->baseSpa());
	snprintf(textStatsC1[4].text,  sizeof(textStatsC1[4].text), "%i", pkm->baseSpd());
	snprintf(textStatsC1[5].text,  sizeof(textStatsC1[5].text), "%i", pkm->baseSpe());

	// Set & print other stat info and
	for(unsigned i=0;i<(sizeof(textStatsC1)/sizeof(textStatsC1[0]));i++) {
		snprintf(textStatsC2[i].text,  sizeof(textStatsC2[i].text), "%i", pkm->iv(i));
		snprintf(textStatsC3[i].text,  sizeof(textStatsC3[i].text), "%i", pkm->ev(i));
		snprintf(textStatsC4[i].text,  sizeof(textStatsC4[i].text), "%i", pkm->stat(i));

		printTextCentered(textStatsC1[i].text, textStatsC1[i].x, textStatsC1[i].y, false);
		printTextCentered(textStatsC2[i].text, textStatsC2[i].x, textStatsC2[i].y, false);
		printTextCentered(textStatsC3[i].text, textStatsC3[i].x, textStatsC3[i].y, false);
		printTextCentered(textStatsC4[i].text, textStatsC4[i].x, textStatsC4[i].y, false);
	}

	// Draw Hidden Power type
	printText(Lang::hpType, 20, 118, false);
	drawImageFromSheet(24+getTextWidth(Lang::hpType), 120, typesData.width, typesData.height/17, types, typesData.width, 0, (pkm->hpType()+1)*(typesData.height/17), false);

}

std::shared_ptr<PKX> selectStats(std::shared_ptr<PKX> pkm) {
	setSpritePosition(bottomArrowID, 128+(textStatsC2[0].x+(getTextWidth(textStatsC2[0].text)/2)), textStatsC2[0].y-6);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();
	drawStatsPage(pkm);

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
			if(selection < 6)	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)	column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textStatsC2)/sizeof(textStatsC2[0]));i++) {
				if(touch.px >= 128+(textStatsC2[selection].x-(getTextWidth(textStatsC2[selection].text)/2)) && touch.px <= 128+(textStatsC2[selection].x+(getTextWidth(textStatsC2[selection].text)/2)) && touch.py >= textStatsC2[i].y && touch.py <= textStatsC2[i].y+16) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i=0;i<(sizeof(textStatsC3)/sizeof(textStatsC3[0]));i++) {
				if(touch.px >= 128+(textStatsC3[selection].x-(getTextWidth(textStatsC3[selection].text)/2)) && touch.px <= 128+(textStatsC3[selection].x+(getTextWidth(textStatsC3[selection].text)/2)) && touch.py >= textStatsC3[i].y && touch.py <= textStatsC3[i].y+16) {
					column = 1;
					selection = i;
					optionSelected = true;
					break;
				}
			}
			if(touch.px > 24+getTextWidth(Lang::hpType) && touch.px < 24+getTextWidth(Lang::hpType)+typesData.width && touch.py > 120 && touch.py < 132) {
				selection = 6;
				optionSelected = true;
			}
		}

		if(optionSelected) {
			optionSelected = 0;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			if(selection == 6) {
				int num = Input::getInt(15); // TODO: Add proper selector
				if(num != -1)	pkm->hpType(num);
			} else if(column == 0) {
				int num = Input::getInt(31);
				if(num != -1)	pkm->iv(selection, num);
			} else {
				int total = 0;
				for(int i=0;i<6;i++) {
					if(i != selection)	total += pkm->ev(i);
				}
				int num = Input::getInt(std::min(510-total, 255));
				if(num != -1)	pkm->ev(selection, num);
			}
			setSpriteVisibility(bottomArrowID, true);
			updateOam();
			drawStatsPage(pkm);
		}

		if(selection == 6) { // Hidden Power type
			setSpritePosition(bottomArrowID, 25+getTextWidth(Lang::hpType)+typesData.width, 112);
		} else if(column == 0) {
			setSpritePosition(bottomArrowID, 128+(textStatsC2[selection].x+(getTextWidth(textStatsC2[selection].text)/2)), textStatsC2[selection].y-6);
		} else {
			setSpritePosition(bottomArrowID, 128+(textStatsC3[selection].x+(getTextWidth(textStatsC3[selection].text)/2)), textStatsC3[selection].y-6);
		}
		updateOam();
	}
}
