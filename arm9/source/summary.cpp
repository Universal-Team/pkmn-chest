#include "summary.h"
#include "colors.h"
#include "langStrings.h"
#include "loader.h"
#include "input.h"
#include "manager.h"
#include "menus.h"
#include "sound.h"

struct Text {
	int x;
	int y;
	char text[32];
};

Text textSP1[] {
	{64, 4},
	{64, 20},
	{64, 36},
	{64, 52},
	{64, 68},
	{64, 84},
	{64, 100},
	{96, 124},
	{96, 140},
	{96, 156},
	{96, 172},
};
Text textSP2r1[] {
	{-20, 20}, {-20, 36}, {-20, 52}, {-20, 68}, {-20, 84}, {-20, 100},
};
Text textSP2r2[] {
	{20, 20}, {20, 36}, {20, 52}, {20, 68}, {20, 84}, {20, 100},
};
Text textSP2r3[] {
	{60, 20}, {60, 36}, {60, 52}, {60, 68}, {60, 84}, {60, 100},
};
Text textSP2r4[] {
	{100, 20}, {100, 36}, {100, 52}, {100, 68}, {100, 84}, {100, 100},
};

int summaryPage = 0;

std::pair<int, int> getPokeballPosition(u8 ball) {
	if(ball > 25)	return {0, 0};
	std::pair<int, int> xy;
	xy.second = (ball/9)*15;
	xy.first = (ball-((ball/9)*9))*15;
	return xy;
}

void drawSummaryP1(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	drawImage(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, false);
	for(int i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}

	// Draw lines
	drawRectangle(0, 124, 256, 1, LIGHT_GRAY, false);

	// Print Pokémon name
	printTextTintedMaxW(Lang::species[pkm->species()], 90, 1, (pkm->gender() ? (pkm->gender() == 1 ? RED_RGB : DARK_GRAY) : BLUE_RGB), 165, 8, false);

	// Draw Pokémon, Pokéball, types, and shiny star (if shiny)
	std::pair<int, int> xy = getPokeballPosition(pkm->ball());
	drawImageFromSheet(148, 8, 15, 15, ballSheet, ballSheetData.width, xy.first, xy.second, false);
	xy = getPokemonPosition(pkm);
	drawImageFromSheetScaled(169, 22, pokemonSheetSize, pokemonSheetSize, 2*pokemonSheetScale, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
	drawImageFromSheet(150, 26, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), false);
	if(pkm->type1() != pkm->type2())
		drawImageFromSheet(185, 26, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), false);
	if(pkm->shiny())	drawImage(150, 45, shinyData.width, shinyData.height, shiny, false);

	// Print Pokémon and trainer info labels
	for(unsigned i=0;i<Lang::summaryP1Labels.size();i++) {
		printTextMaxW(Lang::summaryP1Labels[i], textSP1[i].x-8, 1, 4, textSP1[i].y, false);

	}

	// Print Pokémon and trainer info
	snprintf(textSP1[0].text,  sizeof(textSP1[0].text), "%.3i", pkm->species());
	if(pkm->nicknamed())	snprintf(textSP1[1].text,  sizeof(textSP1[1].text), "%s", pkm->nickname().c_str());
	else	snprintf(textSP1[1].text,  sizeof(textSP1[1].text), "%s", Lang::species[pkm->species()].c_str());
	snprintf(textSP1[2].text,  sizeof(textSP1[2].text), "%s", Lang::balls[pkm->ball()].c_str());
	snprintf(textSP1[3].text,  sizeof(textSP1[3].text), "%i", pkm->level());
	snprintf(textSP1[4].text,  sizeof(textSP1[4].text), "%s", Lang::natures[pkm->nature()].c_str());
	snprintf(textSP1[5].text,  sizeof(textSP1[5].text), "%s", pkm->shiny() ? Lang::yes.c_str() : Lang::no.c_str());
	snprintf(textSP1[6].text,  sizeof(textSP1[6].text), "%s", pkm->pkrs() ? Lang::yes.c_str() : Lang::no.c_str());
	snprintf(textSP1[7].text,  sizeof(textSP1[7].text), "%s", pkm->otName().c_str());
	snprintf(textSP1[8].text,  sizeof(textSP1[8].text), "%.5i", pkm->TID());
	snprintf(textSP1[9].text,  sizeof(textSP1[9].text), "%.5i", pkm->SID());
	snprintf(textSP1[10].text, sizeof(textSP1[10].text),"%i", pkm->otFriendship());
	for(unsigned i=0;i<(sizeof(textSP1)/sizeof(textSP1[0]));i++) {
		if(i!=7)	// Ball is scaled, OT Name is colored
			printTextMaxW(textSP1[i].text, 80, 1, textSP1[i].x, textSP1[i].y, false);
	}
	printTextTinted(textSP1[7].text, (pkm->otGender() ? RED_RGB : BLUE_RGB), textSP1[7].x, textSP1[7].y, false);
}

void drawSummaryP2(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	drawImage(0, 0, boxBgBottomData.width, boxBgBottomData.height, boxBgBottom, false);

	// Draw lines
	for(unsigned i=1;i<(sizeof(textSP2r1)/sizeof(textSP2r1[0]));i++) {
		drawRectangle(16, textSP2r1[i].y, 230, 1, LIGHT_GRAY, false);
	}
	drawRectangle(128, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(168, 4, 1, 112, LIGHT_GRAY, false);
	drawRectangle(208, 4, 1, 112, LIGHT_GRAY, false);

	// Print stat info labels
	int i = pkm->nature();
	printText(Lang::summaryP2Labels[0], 20, textSP2r1[0].y, false);
	printTextTinted(Lang::summaryP2Labels[1], (i!=0&&i<5         ? RED_RGB : i!=0&&!(i%5)      ? BLUE_RGB : WHITE), 20, textSP2r1[1].y, false);
	printTextTinted(Lang::summaryP2Labels[2], (i!=6&&i>4&&i<10   ? RED_RGB : i!=6&&!((i-1)%5)  ? BLUE_RGB : WHITE), 20, textSP2r1[2].y, false);
	printTextTinted(Lang::summaryP2Labels[3], (i!=18&&i>14&&i<20 ? RED_RGB : i!=18&&!((i-3)%5) ? BLUE_RGB : WHITE), 20, textSP2r1[3].y, false);
	printTextTinted(Lang::summaryP2Labels[4], (i!=24&&i>19       ? RED_RGB : i!=24&&!((i-4)%5) ? BLUE_RGB : WHITE), 20, textSP2r1[4].y, false);
	printTextTinted(Lang::summaryP2Labels[5], (i!=12&&i>9&&i<15  ? RED_RGB : i!=12&&!((i-2)%5) ? BLUE_RGB : WHITE), 20, textSP2r1[5].y, false);

	// Print column titles
	printTextCenteredMaxW(Lang::summaryP2Labels[6], 30, 1, textSP2r1[0].x, textSP2r1[0].y-16, false);
	printTextCentered(Lang::summaryP2Labels[7], textSP2r2[0].x, textSP2r2[0].y-16, false);
	printTextCentered(Lang::summaryP2Labels[8], textSP2r3[0].x, textSP2r3[0].y-16, false);
	printTextCenteredMaxW(Lang::summaryP2Labels[9], 30, 1, textSP2r4[0].x, textSP2r4[0].y-16, false);

	// Set base stat info
	snprintf(textSP2r1[0].text,  sizeof(textSP2r1[0].text), "%i", pkm->baseHP());
	snprintf(textSP2r1[1].text,  sizeof(textSP2r1[1].text), "%i", pkm->baseAtk());
	snprintf(textSP2r1[2].text,  sizeof(textSP2r1[2].text), "%i", pkm->baseDef());
	snprintf(textSP2r1[3].text,  sizeof(textSP2r1[3].text), "%i", pkm->baseSpa());
	snprintf(textSP2r1[4].text,  sizeof(textSP2r1[4].text), "%i", pkm->baseSpd());
	snprintf(textSP2r1[5].text,  sizeof(textSP2r1[5].text), "%i", pkm->baseSpe());

	// Set & print other stat info and 
	for(unsigned i=0;i<(sizeof(textSP2r1)/sizeof(textSP2r1[0]));i++) {
		snprintf(textSP2r2[i].text,  sizeof(textSP2r2[i].text), "%i", pkm->iv(i));
		snprintf(textSP2r3[i].text,  sizeof(textSP2r3[i].text), "%i", pkm->ev(i));
		snprintf(textSP2r4[i].text,  sizeof(textSP2r4[i].text), "%i", pkm->stat(i));

		printTextCentered(textSP2r1[i].text, textSP2r1[i].x, textSP2r1[i].y, false);
		printTextCentered(textSP2r2[i].text, textSP2r2[i].x, textSP2r2[i].y, false);
		printTextCentered(textSP2r3[i].text, textSP2r3[i].x, textSP2r3[i].y, false);
		printTextCentered(textSP2r4[i].text, textSP2r4[i].x, textSP2r4[i].y, false);
	}
}

void drawSummaryPage(std::shared_ptr<PKX> pkm) {
	if(summaryPage == 0) {
		drawSummaryP1(pkm);
	} else {
		drawSummaryP2(pkm);
	}
}

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm) {
	// Draw the Pokémon's info
	drawSummaryP1(pkm);

	// Move arrow to first option
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, textSP1[0].x+getTextWidthMaxW(textSP1[0].text, 80), textSP1[0].y-6);
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
			if(selection < (summaryPage == 0 ? 10 : 5))	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)	column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			summaryPage = 0;
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(summaryPage == 0) {
				for(unsigned i=0;i<(sizeof(textSP1)/sizeof(textSP1[0]));i++) {
					if(touch.px >= textSP1[i].x && touch.px <= textSP1[i].x+getTextWidthMaxW(textSP1[i].text, 80) && touch.py >= textSP1[i].y && touch.py <= textSP1[i].y+16) {
						column = 0;
						selection = i;
						optionSelected = true;
						break;
					}
				}
				if(touch.px >= 146 && touch.py >= 16 && touch.py <= 80) {
					column = 1;
					optionSelected = true;
				}
			} else {
				for(unsigned i=0;i<(sizeof(textSP2r2)/sizeof(textSP2r2[0]));i++) {
					if(touch.px >= 128+(textSP2r2[selection].x-(getTextWidth(textSP2r2[selection].text)/2)) && touch.px <= 128+(textSP2r2[selection].x+(getTextWidth(textSP2r2[selection].text)/2)) && touch.py >= textSP2r2[i].y && touch.py <= textSP2r2[i].y+16) {
						column = 0;
						selection = i;
						optionSelected = true;
						break;
					}
				}
				for(unsigned i=0;i<(sizeof(textSP2r3)/sizeof(textSP2r3[0]));i++) {
					if(touch.px >= 128+(textSP2r3[selection].x-(getTextWidth(textSP2r3[selection].text)/2)) && touch.px <= 128+(textSP2r3[selection].x+(getTextWidth(textSP2r3[selection].text)/2)) && touch.py >= textSP2r3[i].y && touch.py <= textSP2r3[i].y+16) {
						column = 1;
						selection = i;
						optionSelected = true;
						break;
					}
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			if(summaryPage == 0) {
				if(column == 0) {
					switch(selection) {
						case 0: {
							int num = Input::getInt(save->maxSpecies());
							if(num > 0)	pkm->species(num);
							break;
						} case 1: {
							std::string name = Input::getLine(10);
							if(name != "") {
								pkm->nickname(name);
								pkm->nicknamed(true);
							}
							if(pkm->gender() != 2)	pkm->gender(Input::getBool(Lang::female, Lang::male));
							break;
						} case 2: {
							int num = selectPokeball();
							if(num > 0)	pkm->ball(num);
							break;
						} case 3: {
							int num = Input::getInt(100);
							if(num > 0)	pkm->level(num);
							break;
						} case 4: {
							int num = selectNature();
							if(num != -1)	pkm->nature(num);
							break;
						} case 5: {
							pkm->shiny(!pkm->shiny());
							break;
						} case 6: {
							pkm->pkrs(pkm->pkrs() ? 0 : 0xF4);
							break;
						} case 7: {
							std::string name = Input::getLine(7);
							if(name != "")	pkm->otName(name);
							pkm->otGender(Input::getBool(Lang::female, Lang::male));
							break;
						} case 8: {
							int num = Input::getInt(65535);
							if(num != -1)	pkm->TID(num);
							break;
						} case 9: {
							int num = Input::getInt(65535);
							if(num != -1)	pkm->SID(num);
							break;
						} case 10: {
							int num = Input::getInt(255);
							if(num != -1)	pkm->otFriendship(num);
							break;
						}
					}
				} else {
					int num = selectForm(pkm->species());
					if(num != -1)	pkm->alternativeForm(num);
				}
			} else {
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
			}
			drawSummaryPage(pkm);
			setSpriteVisibility(bottomArrowID, true);
		}

		if(pressed & KEY_L) {
			if(summaryPage > 0)	summaryPage--;
			column = 0, selection = 0;
			drawSummaryPage(pkm);
		} else if(pressed & KEY_R) {
			if(summaryPage < 1)	summaryPage++;
			column = 0, selection = 0;
			drawSummaryPage(pkm);
		}

		if(summaryPage == 0) {
			if(column == 0) {
				setSpritePosition(bottomArrowID, textSP1[selection].x+getTextWidthMaxW(textSP1[selection].text, 80), textSP1[selection].y-6);
			} else {
				setSpritePosition(bottomArrowID, 230, 30);
			}
		} else {
			if(column == 0) {
				setSpritePosition(bottomArrowID, 128+(textSP2r2[selection].x+(getTextWidth(textSP2r2[selection].text)/2)), textSP2r2[selection].y-6);
			} else {
				setSpritePosition(bottomArrowID, 128+(textSP2r3[selection].x+(getTextWidth(textSP2r3[selection].text)/2)), textSP2r3[selection].y-6);
			}
		}
		updateOam();
	}
}