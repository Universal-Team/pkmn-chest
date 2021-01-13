#include "trainer.hpp"

#include "Sav3.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "gui.hpp"
#include "i18n_ext.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "sound.hpp"

struct Text {
	int x;
	int y;
	char text[18];
};

Text textTP1[]{
	{4, 16}, {4, 32}, {4, 48}, {4, 64}, {4, 80}, {4, 96}, {4, 112}, {4, 128}, {4, 144},
};

std::vector<std::string> trainerText = {"name",   "trainerID", "secretID",   "money",     "bp",
										"badges", "playTime",  "rtcInitial", "rtcElapsed"};

void drawTrainerCard(void) {
	// Draw background
	Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	Gui::font.print(i18n::localize(Config::getLang("lang"), "trainerInfo"), 4, 0, false);

	// Print labels
	for(unsigned i = 0;
		i < sizeof(textTP1) / sizeof(textTP1[0]) - (save->generation() == pksm::Generation::THREE ? 0 : 2); i++) {
		Gui::font.print(i18n::localize(Config::getLang("lang"), trainerText[i]) + ":", textTP1[i].x, textTP1[i].y,
						false, true);
	}

	// Set info text
	snprintf(textTP1[0].text, sizeof(textTP1[0].text), "%s", save->otName().c_str());
	snprintf(textTP1[1].text, sizeof(textTP1[1].text), "%.5i", save->TID());
	snprintf(textTP1[2].text, sizeof(textTP1[2].text), "%.5i", save->SID());
	snprintf(textTP1[3].text, sizeof(textTP1[3].text), i18n::localize(Config::getLang("lang"), "moneyFormat").c_str(),
			 save->money());
	snprintf(textTP1[4].text, sizeof(textTP1[4].text), "%li", save->BP());
	snprintf(textTP1[5].text, sizeof(textTP1[5].text), "%i", save->badges());
	snprintf(textTP1[6].text, sizeof(textTP1[6].text), "%i:%i:%i", save->playedHours(), save->playedMinutes(),
			 save->playedSeconds());
	if(save->generation() == pksm::Generation::THREE) {
		pksm::Sav3 *sav3 = (pksm::Sav3 *)save.get();
		snprintf(textTP1[7].text, sizeof(textTP1[7].text), "%i:%i:%i:%i", sav3->rtcInitialDay(), sav3->rtcInitialHour(),
				 sav3->rtcInitialMinute(), sav3->rtcInitialSecond());
		snprintf(textTP1[8].text, sizeof(textTP1[8].text), "%i:%i:%i:%i", sav3->rtcElapsedDay(), sav3->rtcElapsedHour(),
				 sav3->rtcElapsedMinute(), sav3->rtcElapsedSecond());
	}

	// Print info
	Gui::font.print(textTP1[0].text,
					textTP1[0].x + Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[0])) + 8,
					textTP1[0].y, false, 2, Alignment::left, 0, save->gender() ? TextColor::red : TextColor::blue);
	for(unsigned i = 1;
		i < (sizeof(textTP1) / sizeof(textTP1[0])) - (save->generation() == pksm::Generation::THREE ? 0 : 2); i++) {
		Gui::font.print(textTP1[i].text,
						textTP1[i].x + Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[i])) + 8,
						textTP1[i].y, false, 2);
	}
}

void showTrainerCard(void) {
	// Draw the trainer info
	listBg.draw(0, 0, false, 3);
	drawTrainerCard();

	// Move arrow to first option
	arrow[false].visibility(true);
	arrow[false].position(textTP1[0].x + Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[0])) +
							  8 + Gui::font.calcWidth(textTP1[0].text),
						  textTP1[0].y - 6);
	arrow[false].update();

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
			if(selection < (int)(sizeof(textTP1) / sizeof(textTP1[0]) - 1))
				selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)
				column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)
				column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Graphics::drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < (sizeof(textTP1) / sizeof(textTP1[0])); i++) {
				if(touch.px >= textTP1[i].x +
						   Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[i])) + 8 &&
				   touch.px <= textTP1[i].x +
						   Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[i])) + 8 +
						   Gui::font.calcWidth(textTP1[i].text) &&
				   touch.py >= textTP1[i].y && touch.py <= textTP1[i].y + 16) {
					selection      = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			arrow[false].visibility(false);
			arrow[false].update();
			switch(selection) {
				case 0: {
					std::string name = Input::getLine(7);
					if(name != "")
						save->otName(name);
					save->gender(pksm::Gender(Input::getBool(i18n::localize(Config::getLang("lang"), "female"),
															 i18n::localize(Config::getLang("lang"), "male"))));
					break;
				}
				case 1: {
					int num = Input::getInt(65535);
					if(num != -1)
						save->TID(num);
					break;
				}
				case 2: {
					int num = Input::getInt(65535);
					if(num != -1)
						save->SID(num);
					break;
				}
				case 3: {
					int num = Input::getInt(9999999);
					if(num != -1)
						save->money(num);
					break;
				}
				case 4: {
					int num = Input::getInt(9999);
					if(num != -1)
						save->BP(num);
					break;
				}
				case 6: {
					int num = Input::getInt(999);
					if(num != -1)
						save->playedHours(num);
					num = Input::getInt(59);
					if(num != -1)
						save->playedMinutes(num);
					num = Input::getInt(59);
					if(num != -1)
						save->playedSeconds(num);
					break;
				}
				case 7: {
					pksm::Sav3 *sav3 = (pksm::Sav3 *)save.get();
					int num          = Input::getInt(65535);
					if(num != -1)
						sav3->rtcInitialDay(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcInitialHour(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcInitialMinute(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcInitialSecond(num);
					break;
				}
				case 8: {
					pksm::Sav3 *sav3 = (pksm::Sav3 *)save.get();
					int num          = Input::getInt(65535);
					if(num != -1)
						sav3->rtcElapsedDay(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcElapsedHour(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcElapsedMinute(num);
					num = Input::getInt(255);
					if(num != -1)
						sav3->rtcElapsedSecond(num);
					break;
				}
			}
			drawTrainerCard();
			arrow[false].visibility(true);
		}

		arrow[false].position(textTP1[selection].x +
								  Gui::font.calcWidth(i18n::localize(Config::getLang("lang"), trainerText[selection])) +
								  8 + Gui::font.calcWidth(textTP1[selection].text),
							  textTP1[selection].y - 6);
		arrow[false].update();
	}
}