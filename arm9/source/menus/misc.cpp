#include "misc.h"

#include "banks.hpp"
#include "colors.h"
#include "config.h"
#include "flashcard.h"
#include "input.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "summary.h"
#include "sound.h"

struct FormCount {
	int dexNo;
	int noForms;
} formCounts[] = {
	{201, 27}, // Unown
	{386,  4}, // Deoxys
	{412,  3}, // Burmy
	{413,  3}, // Wormadam
	{422,  2}, // Shellos
	{423,  2}, // Gastrodon
	{479,  6}, // Rotom
	{487,  2}, // Giratina
	{492,  2}, // Shaymin
	{550,  2}, // Basculin
	{555,  2}, // Darmanitan // Not sure if I should have this or not
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
} textStatsR1[] {
	{-20, 20}, {-20, 36}, {-20, 52}, {-20, 68}, {-20, 84}, {-20, 100},
}, textStatsR2[] {
	{20, 20}, {20, 36}, {20, 52}, {20, 68}, {20, 84}, {20, 100},
}, textStatsR3[] {
	{60, 20}, {60, 36}, {60, 52}, {60, 68}, {60, 84}, {60, 100},
}, textStatsR4[] {
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
		case 6:
			return 2; // Russian (returns English)
	}
}

void drawMiniBoxes(int currentBox) {
	if(currentBox < 0)	currentBox = (topScreen ? Banks::bank->boxes()-1 : save->maxBoxes()-1)+currentBox;
	// Clear text
	if(sdFound())	drawImageFromSheet(210, 0, 46, 192, boxBgBottom, boxBgBottomData.width, 210, 0, false);
	else	drawRectangle(210, 0, 46, 192, DARK_GRAY, false);

	for(int i=0;i<5;i++) {
		drawRectangle(170, 10+(i*33), 35, 30, WHITE, false);
		drawOutline(170, 10+(i*33), 35, 30, DARK_GRAY, false);
		for(int j=0;j<30;j++) {
			if((topScreen ? Banks::bank->pkm(currentBox, j)->species() : save->pkm(currentBox, j)->species()) != 0) {
				// Type 1
				int type = topScreen ? Banks::bank->pkm(currentBox, j)->type1() : save->pkm(currentBox, j)->type1();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(173+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*384)+34], false);

				// Type 2
				type = topScreen ? Banks::bank->pkm(currentBox, j)->type2() : save->pkm(currentBox, j)->type2();
				if(((topScreen ? Banks::bank->pkm(currentBox, j)->generation() : save->pkm(currentBox, j)->generation()) == Generation::FOUR) && type > 8)	type--;
				drawRectangle(175+((j-((j/6)*6))*5), 13+((j/6)*5)+(i*33), 2, 4, types[(type*384)+33], false);
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
		if(sdFound()) {
			std::pair<int, int> xy = getPokemonPosition(dexNo, i);
			drawImageFromSheet((i*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
		} else {
			std::vector<u16> bmp;
			loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(dexNo, i))+".bmp", bmp);
			drawImage((i*32)+(128-((32*formCounts[altIndex].noForms)/2)), 80, 32, 32, bmp, false);
		}
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

void drawMoveList(int screenPos, std::vector<std::string> moveList) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw search icon
	drawImage(256-searchData.width, 0, searchData.width, searchData.height, search, false);

	// Print moves
	for(unsigned i=0;i<std::min(9u, moveList.size()-screenPos);i++) {
		printText(moveList[screenPos+i], 4, 4+(i*20), false);
	}
}

int selectMove(int currentMove) {
	// Set arrow position
	setSpritePosition(bottomArrowID, 4+getTextWidth(Lang::moves[currentMove]), -2);
	setSpriteVisibility(bottomArrowID, true);
	updateOam();

	// Print moves
	std::vector<std::string> moveList(&Lang::moves[0], &Lang::moves[save->maxMove()+1]);
	drawMoveList(currentMove, moveList);

	int held, pressed, screenPos = currentMove, newMove = currentMove, entriesPerScreen = 9;
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
			else	newMove = std::min(save->maxMove(), (int)moveList.size()-1);
		} else if(held & KEY_DOWN) {
			if(newMove < std::min(save->maxMove(), (int)moveList.size()-1))	newMove++;
			else newMove = 0;
		} else if(held & KEY_LEFT) {
			newMove -= entriesPerScreen;
			if(newMove < 0)	newMove = 0;
		} else if(held & KEY_RIGHT) {
			newMove += entriesPerScreen;
			if(newMove > std::min(save->maxMove(), (int)moveList.size()-1))	newMove = std::min(save->maxMove(), (int)moveList.size()-1);
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			for(int i=0;i<save->maxMove()+1;i++) {
				if(moveList[newMove] == Lang::moves[i]) {
					return i;
				}
			}
		} if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return currentMove;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<entriesPerScreen;i++) {
				if(touch.px >= 4 && touch.px <= 4+getTextWidth(moveList[screenPos+i]) && touch.py >= 4+(i*20) && touch.py <= 4+((i+1)*20)) {
					return screenPos+i;
					break;
				} else if(touch.px >= 256-searchData.width && touch.py <= searchData.height) {
					goto search;
				}
			}
		} else if(pressed & KEY_Y) {
			search:
			std::string str = Input::getLine();
				moveList.clear();
				if(str != "")	moveList.push_back("-----");
				for(int i=0;i<save->maxMove()+1;i++) {
					if(strncasecmp(str.c_str(), Lang::moves[i].c_str(), str.length()) == 0) {
						moveList.push_back(Lang::moves[i]);
					}
				}
				newMove = 0;
				screenPos = 0;
			drawMoveList(screenPos, moveList);
		}

		// Scroll screen if needed
		if(newMove < screenPos) {
			screenPos = newMove;
			drawMoveList(screenPos, moveList);
		} else if(newMove > screenPos + entriesPerScreen - 1) {
			screenPos = newMove - entriesPerScreen + 1;
			drawMoveList(screenPos, moveList);
		}

		// Move cursor
		setSpritePosition(bottomArrowID, 4+getTextWidth(moveList[newMove]), (20*(newMove-screenPos)-2));
		updateOam();
	}
}

std::shared_ptr<PKX> selectMoves(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

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
			int num = selectMove(pkm->move(selection));
			if(num != -1)	pkm->move(selection, num);

			// Clear the screen
			if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
			else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

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
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

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

	int arrowX = currentNature-((currentNature/5)*5), arrowY = currentNature/5, pressed, held;
	// Move arrow to current nature
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[currentNature], 48)/2)+28, (arrowY*32)+24);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(arrowY > 0)	arrowY--;
			else	arrowY=4;
		} else if(held & KEY_DOWN) {
			if(arrowY < 4)	arrowY++;
			else	arrowY=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return (arrowY*5)+arrowX;
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
		setSpritePosition(bottomArrowID, (arrowX*48)+(getTextWidthMaxW(Lang::natures[(arrowY*5)+arrowX], 48)/2)+28, (arrowY*32)+24);
		updateOam();
	}
}

int selectPokeball(int currentBall) {
	// Clear screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

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
	int arrowX = currentBall-((currentBall/5)*5), arrowY = currentBall/5, pressed, held;
	// Move arrow to current ball
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, (arrowX*48)+40, (arrowY*32)+16);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(arrowY > 0)	arrowY--;
			else	arrowY=4;
		} else if(held & KEY_DOWN) {
			if(arrowY < 4)	arrowY++;
			else	arrowY=0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)	arrowX--;
			else	arrowX=4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)	arrowX++;
			else arrowX=0;
		} else if(pressed & KEY_A) {
			if(!(save->generation() != Generation::FIVE && (arrowY*5)+arrowX == 24)) {
				Sound::play(Sound::click);
				return (arrowY*5)+arrowX+1;
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
		setSpritePosition(bottomArrowID, (arrowX*48)+40, (arrowY*32)+16);
		updateOam();
	}
}

void drawStatsPage(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Draw lines
	for(unsigned i=1;i<(sizeof(textStatsR1)/sizeof(textStatsR1[0]));i++) {
		drawRectangle(16, textStatsR1[i].y, 230, 1, LIGHT_GRAY, false);
	}
	drawRectangle(128, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(168, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(208, 4, 1, 112, LIGHT_GRAY, false);

	// Print stat info labels
	int i = pkm->nature();
	printText(Lang::statsLabels[0], 20, textStatsR1[0].y, false);
	printTextTintedMaxW(Lang::statsLabels[1], 80, 1, (i!=0&&i<5         ? RGB::RED : i!=0&&!(i%5)      ? RGB::BLUE : WHITE), 20, textStatsR1[1].y, false);
	printTextTintedMaxW(Lang::statsLabels[2], 80, 1, (i!=6&&i>4&&i<10   ? RGB::RED : i!=6&&!((i-1)%5)  ? RGB::BLUE : WHITE), 20, textStatsR1[2].y, false);
	printTextTintedMaxW(Lang::statsLabels[3], 80, 1, (i!=18&&i>14&&i<20 ? RGB::RED : i!=18&&!((i-3)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[3].y, false);
	printTextTintedMaxW(Lang::statsLabels[4], 80, 1, (i!=24&&i>19       ? RGB::RED : i!=24&&!((i-4)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[4].y, false);
	printTextTintedMaxW(Lang::statsLabels[5], 80, 1, (i!=12&&i>9&&i<15  ? RGB::RED : i!=12&&!((i-2)%5) ? RGB::BLUE : WHITE), 20, textStatsR1[5].y, false);

	// Print column titles
	printTextCenteredMaxW(Lang::statsLabels[6], 30, 1, textStatsR1[0].x, textStatsR1[0].y-16, false);
	printTextCentered(Lang::statsLabels[7], textStatsR2[0].x, textStatsR2[0].y-16, false);
	printTextCentered(Lang::statsLabels[8], textStatsR3[0].x, textStatsR3[0].y-16, false);
	printTextCenteredMaxW(Lang::statsLabels[9], 30, 1, textStatsR4[0].x, textStatsR4[0].y-16, false);

	// Set base stat info
	snprintf(textStatsR1[0].text,  sizeof(textStatsR1[0].text), "%i", pkm->baseHP());
	snprintf(textStatsR1[1].text,  sizeof(textStatsR1[1].text), "%i", pkm->baseAtk());
	snprintf(textStatsR1[2].text,  sizeof(textStatsR1[2].text), "%i", pkm->baseDef());
	snprintf(textStatsR1[3].text,  sizeof(textStatsR1[3].text), "%i", pkm->baseSpa());
	snprintf(textStatsR1[4].text,  sizeof(textStatsR1[4].text), "%i", pkm->baseSpd());
	snprintf(textStatsR1[5].text,  sizeof(textStatsR1[5].text), "%i", pkm->baseSpe());

	// Set & print other stat info and
	for(unsigned i=0;i<(sizeof(textStatsR1)/sizeof(textStatsR1[0]));i++) {
		snprintf(textStatsR2[i].text,  sizeof(textStatsR2[i].text), "%i", pkm->iv(i));
		snprintf(textStatsR3[i].text,  sizeof(textStatsR3[i].text), "%i", pkm->ev(i));
		snprintf(textStatsR4[i].text,  sizeof(textStatsR4[i].text), "%i", pkm->stat(i));

		printTextCentered(textStatsR1[i].text, textStatsR1[i].x, textStatsR1[i].y, false);
		printTextCentered(textStatsR2[i].text, textStatsR2[i].x, textStatsR2[i].y, false);
		printTextCentered(textStatsR3[i].text, textStatsR3[i].x, textStatsR3[i].y, false);
		printTextCentered(textStatsR4[i].text, textStatsR4[i].x, textStatsR4[i].y, false);
	}
}

std::shared_ptr<PKX> selectStats(std::shared_ptr<PKX> pkm) {
	setSpritePosition(bottomArrowID, 128+(textStatsR2[0].x+(getTextWidth(textStatsR2[0].text)/2)), textStatsR2[0].y-6);
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
			if(selection < 5)	selection++;
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
			for(unsigned i=0;i<(sizeof(textStatsR2)/sizeof(textStatsR2[0]));i++) {
				if(touch.px >= 128+(textStatsR2[selection].x-(getTextWidth(textStatsR2[selection].text)/2)) && touch.px <= 128+(textStatsR2[selection].x+(getTextWidth(textStatsR2[selection].text)/2)) && touch.py >= textStatsR2[i].y && touch.py <= textStatsR2[i].y+16) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i=0;i<(sizeof(textStatsR3)/sizeof(textStatsR3[0]));i++) {
				if(touch.px >= 128+(textStatsR3[selection].x-(getTextWidth(textStatsR3[selection].text)/2)) && touch.px <= 128+(textStatsR3[selection].x+(getTextWidth(textStatsR3[selection].text)/2)) && touch.py >= textStatsR3[i].y && touch.py <= textStatsR3[i].y+16) {
					column = 1;
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = 0;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			if(column == 0) {
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

		if(column == 0) {
			setSpritePosition(bottomArrowID, 128+(textStatsR2[selection].x+(getTextWidth(textStatsR2[selection].text)/2)), textStatsR2[selection].y-6);
		} else {
			setSpritePosition(bottomArrowID, 128+(textStatsR3[selection].x+(getTextWidth(textStatsR3[selection].text)/2)), textStatsR3[selection].y-6);
		}
		updateOam();
	}
}
