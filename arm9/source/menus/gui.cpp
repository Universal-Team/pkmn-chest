#include "gui.hpp"

#include "sound.hpp"

Font Gui::font;

void Gui::prompt(int message, const std::string &confirm) { prompt(std::to_string(message), confirm); }

void Gui::prompt(std::string message, const std::string &confirm) {
	// Get height
	int lines = 1;
	for(unsigned int c = 0; c < message.length(); c++) {
		if(message[c] == '\n') {
			lines++;
		}
	}

	// Draw backgruond
	Graphics::drawRectangle(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, DARKER_GRAY, false, true);
	Graphics::drawOutline(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, BLACK, false, true);

	int okWidth = font.calcWidth(confirm);

	// Draw confirm button
	Graphics::drawRectangle(233 - okWidth - 8, 96 + (8 * lines) - 18, okWidth + 8, 18, LIGHT_GRAY, false, true);
	Graphics::drawOutline(233 - okWidth - 8, 96 + (8 * lines) - 18, okWidth + 8, 18, BLACK, false, true);

	// Print text
	font.print(message, 23, 96 - (8 * (lines)), false, 2, Alignment::left, 0, TextColor::white);
	font.print(confirm, 233 - okWidth - 4, 96 + (8 * lines) - 17, false, 2, Alignment::left, 0, TextColor::gray);

	int pressed;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!pressed);

		if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.py > 96 + (8 * lines) - 18 && touch.py < 96 + (8 * lines)) {
				if(touch.px > 233 - okWidth - 8 && touch.px < 233) {
					goto exit;
				}
			}
		} else if(pressed & (KEY_A | KEY_B)) {
		exit:
			Sound::play(Sound::click);
			Graphics::drawRectangle(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, CLEAR, false, true);
			return;
		}
	}
}

void Gui::warn(int message, const std::string &confirm) { warn(std::to_string(message), confirm); }

void Gui::warn(std::string message, const std::string &confirm) {
	// Get height
	int lines = 2;
	for(unsigned int c = 0; c < message.length(); c++) {
		if(message[c] == '\n') {
			lines++;
		}
	}

	// Draw backgruond
	Graphics::drawRectangle(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, DARKER_GRAY, false, true);
	Graphics::drawOutline(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, DARK_RED, false, true);

	int okWidth = font.calcWidth(confirm);

	// Draw confirm button
	Graphics::drawRectangle(233 - okWidth - 8, 96 + (8 * lines) - 18, okWidth + 8, 18, LIGHT_GRAY, false, true);
	Graphics::drawOutline(233 - okWidth - 8, 96 + (8 * lines) - 18, okWidth + 8, 18, BLACK, false, true);

	// Print text
	font.print(i18n::localize(Config::getLang("lang"), "warning"), 23, 96 - (8 * lines), false, 2, Alignment::left, 0,
			   TextColor::red);
	font.print(message, 23, 96 - (8 * (lines)) + 16, false, 2, Alignment::left, 0, TextColor::white);
	font.print(confirm, 233 - okWidth - 4, 96 + (8 * lines) - 17, false, 2, Alignment::left, 0, TextColor::gray);

	int pressed;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
		} while(!pressed);

		if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.py > 96 + (8 * lines) - 18 && touch.py < 96 + (8 * lines)) {
				if(touch.px > 233 - okWidth - 8 && touch.px < 233) {
					goto exit;
				}
			}
		} else if(pressed & (KEY_A | KEY_B)) {
		exit:
			Sound::play(Sound::click);
			Graphics::drawRectangle(20, 96 - (8 * lines) - 3, 215, (16 * lines) + 5, CLEAR, false, true);
			return;
		}
	}
}
