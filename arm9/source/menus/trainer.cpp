#include "trainer.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "sound.hpp"

struct Text {
	int x;
	int y;
	char text[14];
};

Text textTP1[] {
	{4, 16},
	{4, 32},
	{4, 48},
	{4, 64},
	{4, 80},
	{4, 96},
	{4, 112},
};

std::vector<std::string> trainerText = {"name", "trainerID", "secretID", "money", "bp", "badges", "playTime"};

void drawTrainerCard(void) {
	// Draw background
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	printText(Lang::get("trainerInfo"), 4, 0, false);

	// Print labels
	for(unsigned i=0;i<sizeof(textTP1)/sizeof(textTP1[0]);i++) {
		printText(Lang::get(trainerText[i])+":", textTP1[i].x, textTP1[i].y, false);
	}

	// Set info text
	snprintf(textTP1[0].text, sizeof(textTP1[0].text), "%s", save->otName().c_str());
	snprintf(textTP1[1].text, sizeof(textTP1[1].text), "%.5i", save->TID());
	snprintf(textTP1[2].text, sizeof(textTP1[2].text), "%.5i", save->SID());
	snprintf(textTP1[3].text, sizeof(textTP1[3].text), "$%li", save->money());
	snprintf(textTP1[4].text, sizeof(textTP1[4].text), "%li", save->BP());
	snprintf(textTP1[5].text, sizeof(textTP1[5].text), "%i", save->badges());
	snprintf(textTP1[6].text, sizeof(textTP1[6].text), "%i:%i:%i", save->playedHours(), save->playedMinutes(), save->playedSeconds());
	
	// Print info
	printTextTinted(textTP1[0].text, (save->gender() ? RED_TEXT : BLUE_TEXT), textTP1[0].x+getTextWidth(Lang::get(trainerText[0]))+8, textTP1[0].y, false);
	for(unsigned i=1;i<(sizeof(textTP1)/sizeof(textTP1[0]));i++) {
		printText(textTP1[i].text, textTP1[i].x+getTextWidth(Lang::get(trainerText[i]))+8, textTP1[i].y, false);
	}
}

void showTrainerCard(void) {
	// Draw the trainer info
	drawImageDMA(0, 0, listBg, false, false);
	drawTrainerCard();

	// Move arrow to first option
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, textTP1[0].x+getTextWidth(Lang::get(trainerText[0]))+8+getTextWidth(textTP1[0].text), textTP1[0].y-6);
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
			if(selection < (int)(sizeof(textTP1)/sizeof(textTP1[0])-1))	selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)	column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)	column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textTP1)/sizeof(textTP1[0]));i++) {
				if(touch.px >= textTP1[i].x+getTextWidth(Lang::get(trainerText[i]))+8 && touch.px <= textTP1[i].x+getTextWidth(Lang::get(trainerText[i]))+8+getTextWidth(textTP1[i].text) && touch.py >= textTP1[i].y && touch.py <= textTP1[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			switch(selection) {
				case 0: {
					std::string name = Input::getLine(7);
					if(name != "")	save->otName(name);
					save->gender(Input::getBool(Lang::get("female"), Lang::get("male")));
					break;
				} case 1: {
					int num = Input::getInt(65535);
					if(num != -1)	save->TID(num);
					break;
				} case 2: {
					int num = Input::getInt(65535);
					if(num != -1)	save->SID(num);
					break;
				} case 3: {
					int num = Input::getInt(9999999);
					if(num != -1)	save->money(num);
					break;
				} case 4: {
					int num = Input::getInt(9999);
					if(num != -1)	save->BP(num);
					break;
				} case 6: {
					int num = Input::getInt(999);
					if(num != -1)	save->playedHours(num);
					num = Input::getInt(59);
					if(num != -1)	save->playedMinutes(num);
					num = Input::getInt(59);
					if(num != -1)	save->playedSeconds(num);
					break;
				}
			}
			drawTrainerCard();
			setSpriteVisibility(arrowID, false, true);
		}

		setSpritePosition(arrowID, false, textTP1[selection].x+getTextWidth(Lang::get(trainerText[selection]))+8+getTextWidth(textTP1[selection].text), textTP1[selection].y-6);
		updateOam();
	}
}