#include "keyboard.h"
#include "graphics/graphics.h"
#include "manager.h"
#include <string>
#include <stdio.h>
#include <ctype.h>

struct Key {
	std::string character;
	int x;
	int y;
};

Key keys[] = {
	{"1", 0, 0}, {"2", 20, 0}, {"3", 40, 0}, {"4", 60, 0}, {"5", 80, 0}, {"6", 100, 0}, {"7", 120, 0}, {"8", 140, 0}, {"9", 160, 0}, {"0", 180, 0}, {"-", 200, 0}, {"=", 220, 0},
	{"q", 10, 18}, {"w", 30, 18}, {"e", 50, 18}, {"r", 70, 18}, {"t", 90, 18}, {"y", 110, 18}, {"u", 130, 18}, {"i", 150, 18}, {"o", 170, 18}, {"p", 190, 18}, {"[", 210, 18}, {"]", 230, 18},
	{"a", 20, 36}, {"s", 40, 36}, {"d", 60, 36}, {"f", 80, 36}, {"g", 100, 36}, {"h", 120, 36}, {"j", 140, 36}, {"k", 160, 36}, {"l", 180, 36}, {";", 200, 36}, {"'", 220, 36},
	{"z", 28, 54}, {"x", 48, 54}, {"c", 68, 54}, {"v", 88, 54}, {"b", 108, 54}, {"n", 128, 54}, {"m", 148, 54}, {",", 168, 54}, {".", 188, 54}, {"/", 208, 54},

};
Key modifierKeys[] = {
	{"bksp", 240, 0},	// Backspace
	{"caps", 0, 36},	// Caps Lock
	{"entr", 240, 36},	// Enter
	{"lsft", 0, 54},	// Left Shift
	{"rsft", 228, 54},	// Right Shift
};

std::string Input::getString() {
	// Hide bottom sprites below the keyboard
	for(int i=12;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	updateOam();

	drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARK_BLUE, false);
	drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
	int hDown;
	touchPosition touch;
	std::string string;
	int keyDownDelay = 10;
	bool caps = false, shift = false, enter = false;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			hDown = keysDownRepeat();
			if(keyDownDelay > 0) {
				keyDownDelay--;
			} else if(keyDownDelay == 0) {
				keyDownDelay--;
				drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
				if(caps) drawRectangle(modifierKeys[1].x, modifierKeys[1].y+(192-keyboardData.height), 16, 16, 0x8260, false);
			}
		} while(!hDown);
		if(keyDownDelay > 0) {
			drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
			if(caps) drawRectangle(modifierKeys[1].x, modifierKeys[1].y+(192-keyboardData.height), 16, 16, 0x8006, false);
		}
		keyDownDelay = 10;

		// 
		if(hDown & KEY_TOUCH) {
			touchRead(&touch);
			// Check if a regular key was pressed
			for(uint i=0;i<(sizeof(keys)/sizeof(keys[0]));i++) {
				if((touch.px > keys[i].x-2 && touch.px < keys[i].x+18) && (touch.py > keys[i].y+(192-keyboardData.height)-2 && touch.py < keys[i].y+18+(192-keyboardData.height))) {
					drawRectangle(keys[i].x, keys[i].y+(192-keyboardData.height), 16, 16, 0x8006, false);
					char c = keys[i].character[0];
					string += (shift || caps ? toupper(c) : c);
					shift = false;
					printText(string, 0, 192-keyboardData.height-16, false);
					break;
				}
			}
			// Check if space was pressed
			Key key = {" ", 68, 72};
			if((touch.px > key.x-2 && touch.px < key.x+100) && (touch.py > key.y+(192-keyboardData.height)-2 && touch.py < key.y+18+(192-keyboardData.height))) {
				drawRectangle(key.x, key.y+(192-keyboardData.height), 98, 16, 0x8006, false);
				string += key.character;
				shift = false;
				printText(string, 0, 192-keyboardData.height-16, false);
			}
			// Check if a modifier key was pressed
			for(uint i=0;i<(sizeof(modifierKeys)/sizeof(modifierKeys[0]));i++) {
				if((touch.px > modifierKeys[i].x-2 && touch.px < modifierKeys[i].x+18) && (touch.py > modifierKeys[i].y+(192-keyboardData.height)-2 && touch.py < modifierKeys[i].y+18+(192-keyboardData.height))) {
					drawRectangle(modifierKeys[i].x, modifierKeys[i].y+(192-keyboardData.height), 16, 16, 0x8006, false);
					if(modifierKeys[i].character == "bksp") {
						string = string.substr(0, string.length()-1);
						drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARK_BLUE, false);
						printText(string, 0, 192-keyboardData.height-16, false);
					} else if(modifierKeys[i].character == "caps") {
						caps = !caps;
						if(caps) drawRectangle(modifierKeys[i].x, modifierKeys[i].y+(192-keyboardData.height), 16, 16, 0x8006, false);
					} else if(modifierKeys[i].character == "entr") {
						enter = true;
					} else if(modifierKeys[i].character == "lsft") {
						shift = !shift;
						if(shift) {
							drawRectangle(modifierKeys[i].x, modifierKeys[i].y+(192-keyboardData.height), 24, 16, 0x8260, false);
							keyDownDelay = -1;
						} else {
							keyDownDelay = 0;
						}
					} else if(modifierKeys[i].character == "rsft") {
						shift = !shift;
						if(shift) {
							drawRectangle(modifierKeys[i].x, modifierKeys[i].y+(192-keyboardData.height), 24, 16, 0x8260, false);
							keyDownDelay = -1;
						} else {
							keyDownDelay = 0;
						}
					}
					break;
				}
			}
		}

		if(hDown & KEY_START || enter) {
			drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARK_BLUE, false);
			drawBox(false);
			break;
		}
	}
	return string;
}