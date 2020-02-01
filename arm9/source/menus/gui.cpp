#include "gui.hpp"
#include "graphics.hpp"
#include "sound.hpp"

void Gui::prompt(std::string message, const std::string &confirm) {
	// Get height
	int lines = 1;
	for(unsigned int c=0;c<message.length();c++) {
		if(message[c] == '\n') {
			lines++;
		}
	}

	// Draw backgruond
	drawRectangle(20, 96-(8*lines)-3, 215, (16*lines)+5, DARKER_GRAY, false, true);
	drawOutline(20, 96-(8*lines)-3, 215, (16*lines)+5, BLACK, false, true);

	int okWidth = getTextWidth(confirm);

	// Draw confirm button
	drawRectangle(233-okWidth-8, 96+(8*lines)-18, okWidth+8, 18, LIGHT_GRAY, false, true);
	drawOutline(233-okWidth-8, 96+(8*lines)-18, okWidth+8, 18, BLACK, false, true);

	// Print text
	printTextTinted(message, TextColor::white, 23, 96-(8*(lines)), false, true);
	printTextTinted(confirm, TextColor::gray, 233-okWidth-4, 96+(8*lines)-17, false, true);

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
			if(touch.py > 96+(8*lines)-18 && touch.py < 96+(8*lines)) {
				if(touch.px > 233-okWidth-8 && touch.px < 233) {
					goto exit;
				}
			}
		} else if(pressed & (KEY_A | KEY_B)) {
			exit:
			Sound::play(Sound::click);
			drawRectangle(20, 96-(8*lines)-3, 215, (16*lines)+5, CLEAR, false, true);
			return;
		}
	}
}

void Gui::warn(std::string message, const std::string &confirm) {
	// Get height
	int lines = 2;
	for(unsigned int c=0;c<message.length();c++) {
		if(message[c] == '\n') {
			lines++;
		}
	}

	// Draw backgruond
	drawRectangle(20, 96-(8*lines)-3, 215, (16*lines)+5, DARKER_GRAY, false, true);
	drawOutline(20, 96-(8*lines)-3, 215, (16*lines)+5, DARK_RED, false, true);

	int okWidth = getTextWidth(confirm);

	// Draw confirm button
	drawRectangle(233-okWidth-8, 96+(8*lines)-18, okWidth+8, 18, LIGHT_GRAY, false, true);
	drawOutline(233-okWidth-8, 96+(8*lines)-18, okWidth+8, 18, BLACK, false, true);

	// Print text
	printTextTinted(i18n::localize(Config::getLang("lang"), "warning"), TextColor::red, 23, 96-(8*lines), false, true);
	printTextTinted(message, TextColor::white, 23, 96-(8*(lines))+16, false, true);
	printTextTinted(confirm, TextColor::gray, 233-okWidth-4, 96+(8*lines)-17, false, true);

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
			if(touch.py > 96+(8*lines)-18 && touch.py < 96+(8*lines)) {
				if(touch.px > 233-okWidth-8 && touch.px < 233) {
					goto exit;
				}
			}
		} else if(pressed & (KEY_A | KEY_B)) {
			exit:
			Sound::play(Sound::click);
			drawRectangle(20, 96-(8*lines)-3, 215, (16*lines)+5, CLEAR, false, true);
			return;
		}
	}
}
