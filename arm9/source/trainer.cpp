#include "trainer.h"
#include "colors.h"
#include "graphics.h"
#include "input.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "sound.h"

struct Text {
	int x;
	int y;
	char text[14];
};

Text textTP1[] {
	{4, 14},
	{4, 30},
	{4, 46},
	{4, 62},
	{4, 78},
	{4, 94},
	{4, 110},
};

void drawTrainerCard(void) {
	// Draw background
	drawImage(0, 0, optionsBgData.width, optionsBgData.height, optionsBg, false);

	// Print labels
	for(uint i=0;i<sizeof(textTP1)/sizeof(textTP1[0]);i++) {
		printTextTinted(Lang::trainerText[i]+":", DARK_GRAY, textTP1[i].x, textTP1[i].y, false);
	}

	// Set info text
	snprintf(textTP1[0].text,  sizeof(textTP1[0].text), "%s", save->otName().c_str());
	snprintf(textTP1[1].text,  sizeof(textTP1[1].text), "%.5i", save->TID());
	snprintf(textTP1[2].text,  sizeof(textTP1[2].text), "%.5i", save->SID());
	snprintf(textTP1[3].text,  sizeof(textTP1[3].text), "%li", save->money());
	snprintf(textTP1[4].text,  sizeof(textTP1[4].text), "%li", save->BP());
	snprintf(textTP1[5].text,  sizeof(textTP1[5].text), "%i", save->badges());
	snprintf(textTP1[6].text,  sizeof(textTP1[6].text), "%i:%i:%i", save->playedHours(), save->playedMinutes(), save->playedSeconds());
	
	// Print info
	printTextTinted(textTP1[0].text, (save->gender() ? RED_RGB : BLUE_RGB), textTP1[0].x+getTextWidth(Lang::trainerText[0])+8, textTP1[0].y, false);
	for(uint i=1;i<(sizeof(textTP1)/sizeof(textTP1[0]));i++) {
			printTextTinted(textTP1[i].text, DARK_GRAY, textTP1[i].x+getTextWidth(Lang::trainerText[i])+8, textTP1[i].y, false);
	}
}

void showTrainerCard(void) {
	// Draw the trainer info
	drawTrainerCard();

	// Move arrow to first option
	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, textTP1[0].x+getTextWidth(Lang::trainerText[0])+8+getTextWidth(textTP1[0].text), textTP1[0].y-6);
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
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(uint i=0;i<(sizeof(textTP1)/sizeof(textTP1[0]));i++) {
				if(touch.px >= textTP1[i].x+getTextWidth(Lang::trainerText[i])+8 && touch.px <= textTP1[i].x+getTextWidth(Lang::trainerText[i])+8+getTextWidth(textTP1[i].text) && touch.py >= textTP1[i].y && touch.py <= textTP1[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			switch(selection) {
				case 0: {
					std::string name = Input::getLine(7);
					if(name != "")	save->otName(name);
					save->gender(Input::getBool(Lang::female, Lang::male));
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
			setSpriteVisibility(bottomArrowID, true);
		}

		setSpritePosition(bottomArrowID, textTP1[selection].x+getTextWidth(Lang::trainerText[selection])+8+getTextWidth(textTP1[selection].text), textTP1[selection].y-6);
		updateOam();
	}
}