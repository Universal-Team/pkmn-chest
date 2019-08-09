#include "summary.h"
#include "colors.h"
#include "flashcard.h"
#include "langStrings.h"
#include "loader.h"
#include "input.h"
#include "manager.h"
#include "misc.h"
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
	{170, 132},
	{170, 162},
};

int summaryPage = 0;

std::pair<int, int> getPokeballPosition(u8 ball) {
	if(ball > 25)	return {0, 0};
	std::pair<int, int> xy;
	xy.second = (ball/9)*15;
	xy.first = (ball-((ball/9)*9))*15;
	return xy;
}

void drawSummaryPage(std::shared_ptr<PKX> pkm) {
	// Clear the screen
	if(sdFound())	drawImage(0, 0, summaryBgData.width, summaryBgData.height, summaryBg, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);
	for(int i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	updateOam();

	// Draw lines
	drawOutline(0, 124, 160, 69, LIGHT_GRAY, false);

	// Print Pokémon name
	printTextTintedMaxW(Lang::species[pkm->species()], 90, 1, (pkm->gender() ? (pkm->gender() == 1 ? RGB::RED : GRAY) : RGB::BLUE), 165, 8, false, true);

	// Draw Pokémon, Pokéball, types, and shiny star (if shiny)
		std::pair<int, int> xy = getPokeballPosition(pkm->ball());
		drawImageFromSheet(148, 8, 15, 15, ballSheet, ballSheetData.width, xy.first, xy.second, false);
	if(sdFound()) {
		xy = getPokemonPosition(pkm);
		drawImageFromSheetScaled(169, 22, 32, 32, 2, pokemonSheet, pokemonSheetData.width, xy.first, xy.second, false);
	} else {
		std::vector<u16> bmp;
		ImageData bmpData = loadBmp16("nitro:/graphics/pokemon/"+std::to_string(getPokemonIndex(pkm))+".bmp", bmp);
		drawImageScaled(169, 22, bmpData.width, bmpData.height, 2, bmp, false);
	}
	drawImageFromSheet(150, 26, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type1() > 8) ? pkm->type1()-1 : pkm->type1())*12), false);
	if(pkm->type1() != pkm->type2())
		drawImageFromSheet(185, 26, 32, 12, types, 32, 0, (((pkm->generation() == Generation::FOUR && pkm->type2() > 8) ? pkm->type2()-1 : pkm->type2())*12), false);
	if(pkm->shiny())	drawImage(150, 45, shinyData.width, shinyData.height, shiny, false);

	// Print Pokémon and trainer info labels
	for(unsigned i=0;i<Lang::summaryLabels.size();i++) {
		printTextMaxW(Lang::summaryLabels[i], textSP1[i].x-8, 1, 4, textSP1[i].y, false);

	}

	// Draw buttons
	drawImage(textSP1[11].x-4, textSP1[11].y-4, boxButtonData.width, boxButtonData.height, boxButton, false);
	drawImage(textSP1[12].x-4, textSP1[12].y-4, boxButtonData.width, boxButtonData.height, boxButton, false);

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
	snprintf(textSP1[11].text, sizeof(textSP1[11].text),"%s", Lang::movesString.c_str());
	snprintf(textSP1[12].text, sizeof(textSP1[12].text),"%s", Lang::stats.c_str());
	for(unsigned i=0;i<(sizeof(textSP1)/sizeof(textSP1[0]));i++) {
		if(i!=7)	// Ball is scaled, OT Name is colored
			printTextMaxW(textSP1[i].text, 80, 1, textSP1[i].x, textSP1[i].y, false);
	}
	printTextTinted(textSP1[7].text, (pkm->otGender() ? RGB::RED : RGB::BLUE), textSP1[7].x, textSP1[7].y, false);

}

std::shared_ptr<PKX> showPokemonSummary(std::shared_ptr<PKX> pkm) {
	// Draw the Pokémon's info
	drawSummaryPage(pkm);

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
			if(selection < (column == 0 ? 10 : 2))	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
			selection = 0;
		} else if(held & KEY_RIGHT) {
			if(column < 1)	column++;
			selection = 0;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			pkm->refreshChecksum();
			return pkm;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textSP1)/sizeof(textSP1[0]))-2;i++) {
				if(touch.px >= textSP1[i].x && touch.px <= textSP1[i].x+getTextWidthMaxW(textSP1[i].text, 80) && touch.py >= textSP1[i].y && touch.py <= textSP1[i].y+16) {
					column = 0;
					selection = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i=11;i<(sizeof(textSP1)/sizeof(textSP1[0]));i++) {
				if(touch.px >= textSP1[i].x-4 && touch.px <= textSP1[i].x-4+boxButtonData.width && touch.py >= textSP1[i].y-4 && touch.py <= textSP1[i].y-4+boxButtonData.height) {
					column = 1;
					selection = i-10;
					optionSelected = true;
					break;
				}
			}
			if(touch.px >= 146 && touch.py >= 16 && touch.py <= 80) {
				column = 1;
				optionSelected = true;
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			if(column == 0) {
				switch(selection) {
					case 0: {
						int num = Input::getInt(save->maxSpecies());
						if(num > 0) {
							pkm->species(num);
							if(!pkm->nicknamed())	pkm->nickname(Lang::species[num]);
							pkm->setAbility(0);
						}
						break;
					} case 1: {
						std::string name = Input::getLine(10);
						if(name != "") {
							pkm->nickname(name);
							pkm->nicknamed(name != Lang::species[pkm->species()]);
						}
						if(pkm->gender() != 2) {
							pkm->gender(Input::getBool(Lang::female, Lang::male));
							pkm->PID(PKX::getRandomPID(pkm->species(), pkm->gender(), pkm->version(), pkm->nature(), pkm->alternativeForm(), pkm->abilityNumber(), pkm->PID(), pkm->generation()));
						}
						break;
					} case 2: {
						int num = selectPokeball(pkm->ball());
						if(num > 0)	pkm->ball(num);
						break;
					} case 3: {
						int num = Input::getInt(100);
						if(num > 0)	pkm->level(num);
						break;
					} case 4: {
						int num = selectNature(pkm->nature());
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
				switch(selection) {
					case 0: {
						int num = selectForm(pkm->species(), pkm->alternativeForm());
						if(num != -1)	pkm->alternativeForm(num);
						break;
					} case 1: {
						pkm = selectMoves(pkm);
						break;
					} case 2: {
						pkm = selectStats(pkm);
						break;
					}
				}
			}
			drawSummaryPage(pkm);
			setSpriteVisibility(bottomArrowID, true);
		}

		if(column == 0) {
			setSpritePosition(bottomArrowID, textSP1[selection].x+getTextWidthMaxW(textSP1[selection].text, 80), textSP1[selection].y-6);
		} else {
			if(selection == 0)	setSpritePosition(bottomArrowID, 230, 30);
			else	setSpritePosition(bottomArrowID, textSP1[selection+10].x+getTextWidthMaxW(textSP1[selection+10].text, 80), textSP1[selection+10].y-6);
		}
		updateOam();
	}
}