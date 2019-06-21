#include "keyboard.h"
#include "graphics/colors.h"
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

std::string string;
int keyboardLayout = 0;
bool caps = false, shift = false, enter = false;

Key keysKanaABC[] = {
	{"@#/&_", 34,   0},	{"abc",   68,   0},	{"def",  102,   0},
	{"ghi",   34,  34},	{"jkl",   68,  34},	{"mno",  102,  34},	{" ", 136, 34},
	{"pqrs",  34,  68},	{"tuv",   68,  68},	{"wxyz", 102,  68},
						{"'\"()", 68, 102},	{".,?!", 102, 102},
};
Key keysKanaSpecial[] = {
	{"shft",  34, 102},
	{"bksp", 136,   0},
	{"spce", 136,  34},
	{"entr", 136,  68},
	{"entr", 136, 102},
};

Key keys[] = {
	{"1", 0, 0}, {"2", 20, 0}, {"3", 40, 0}, {"4", 60, 0}, {"5", 80, 0}, {"6", 100, 0}, {"7", 120, 0}, {"8", 140, 0}, {"9", 160, 0}, {"0", 180, 0}, {"-", 200, 0}, {"=", 220, 0},
	{"q", 10, 18}, {"w", 30, 18}, {"e", 50, 18}, {"r", 70, 18}, {"t", 90, 18}, {"y", 110, 18}, {"u", 130, 18}, {"i", 150, 18}, {"o", 170, 18}, {"p", 190, 18}, {"[", 210, 18}, {"]", 230, 18},
	{"a", 20, 36}, {"s", 40, 36}, {"d", 60, 36}, {"f", 80, 36}, {"g", 100, 36}, {"h", 120, 36}, {"j", 140, 36}, {"k", 160, 36}, {"l", 180, 36}, {";", 200, 36}, {"'", 220, 36},
	{"z", 30, 54}, {"x", 50, 54}, {"c", 70, 54}, {"v", 90, 54}, {"b", 110, 54}, {"n", 130, 54}, {"m", 150, 54}, {",", 170, 54}, {".", 190, 54}, {"/", 210, 54},
};
Key keysSpecialQwerty[] = {
	{"bksp", 240, 0},	// Backspace
	{"caps", 0, 36},	// Caps Lock
	{"entr", 240, 36},	// Enter
	{"lsft", 0, 54},	// Left Shift
	{"rsft", 230, 54},	// Right Shift
};

void clearVars(void) {
	string == "", keyboardLayout = 0, caps = false,
	shift = false, enter = false;
}

void whileHeld(void) {
	while(keysHeld()) {
		swiWaitForVBlank();
		scanKeys();
	}
}

void processTouchQwerty(touchPosition touch, uint maxLength) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(uint i=0;i<(sizeof(keys)/sizeof(keys[0]));i++) {
			if((touch.px > keys[i].x-2 && touch.px < keys[i].x+18) && (touch.py > keys[i].y+(192-keyboardData.height)-2 && touch.py < keys[i].y+18+(192-keyboardData.height))) {
				drawRectangle(keys[i].x, keys[i].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
				char c = keys[i].character[0];
				string += (shift || caps ? toupper(c) : c);
				shift = false;
				printText(string, 0, 192-keyboardData.height-16, false);
				break;
			}
		}
		// Check if space was pressed
		Key key = {" ", 70, 72};
		if((touch.px > key.x-2 && touch.px < key.x+100) && (touch.py > key.y+(192-keyboardData.height)-2 && touch.py < key.y+18+(192-keyboardData.height))) {
			drawRectangle(key.x, key.y+(192-keyboardData.height), 96, 16, DARK_GRAY, false);
			string += key.character;
			shift = false;
			printText(string, 0, 192-keyboardData.height-16, false);
		}
	}
	// Check if a special key was pressed
	for(uint i=0;i<(sizeof(keysSpecialQwerty)/sizeof(keysSpecialQwerty[0]));i++) {
		if((touch.px > keysSpecialQwerty[i].x-2 && touch.px < keysSpecialQwerty[i].x+18) && (touch.py > keysSpecialQwerty[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialQwerty[i].y+18+(192-keyboardData.height))) {
			if(keysSpecialQwerty[i].character == "bksp") {
				drawRectangle(keysSpecialQwerty[i].x, keysSpecialQwerty[i].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
				whileHeld();
				string = string.substr(0, string.length()-1);
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
				printText(string, 0, 192-keyboardData.height-16, false);
			} else if(keysSpecialQwerty[i].character == "caps") {
				caps = !caps;
				if(caps) drawRectangle(keysSpecialQwerty[i].x, keysSpecialQwerty[i].y+(192-keyboardData.height), 16, 16, GRAY, false);
				whileHeld();
			} else if(keysSpecialQwerty[i].character == "entr") {
				if(caps) drawRectangle(keysSpecialQwerty[i].x, keysSpecialQwerty[i].y+(192-keyboardData.height), 16, 16, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialQwerty[i].character == "lsft") {
				shift = !shift;
				if(shift)	drawRectangle(keysSpecialQwerty[i].x, keysSpecialQwerty[i].y+(192-keyboardData.height), 26, 16, GRAY, false);
				whileHeld();
			} else if(keysSpecialQwerty[i].character == "rsft") {
				shift = !shift;
				if(shift)	drawRectangle(keysSpecialQwerty[i].x, keysSpecialQwerty[i].y+(192-keyboardData.height), 26, 16, GRAY, false);
				whileHeld();
			}
			break;
		}
	}
}

void processTouchKana(touchPosition touch, uint maxLength) {
	if(string.length() < maxLength) {
		// Check if an ABC key was pressed
		for(uint i=0;i<(sizeof(keysKanaABC)/sizeof(keysKanaABC[0]));i++) {
			if((touch.px > keysKanaABC[i].x-2 && touch.px < keysKanaABC[i].x+34) && (touch.py > keysKanaABC[i].y+(192-keyboardData.height)-2 && touch.py < keysKanaABC[i].y+34+(192-keyboardData.height))) {
				drawRectangle(keysKanaABC[i].x, keysKanaABC[i].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
				char c = '\0';
				while(keysHeld() & KEY_TOUCH) {
					if(touch.px < keysKanaABC[i].x) {
						c = keysKanaABC[i].character[1];
						break;
					} else if(touch.py < keysKanaABC[i].y+(192-keyboardData.height)) {
						c = keysKanaABC[i].character[2];
						break;
					} else if(touch.px > keysKanaABC[i].x+32) {
						c = keysKanaABC[i].character[3];
						break;
					} else if(touch.py > keysKanaABC[i].y+(192-keyboardData.height)+32) {
						c = keysKanaABC[i].character[4];
						break;
					}
					swiWaitForVBlank();
					scanKeys();
					touchRead(&touch);
				}
				if(c == '\0')	c = keysKanaABC[i].character[0];
				string += (shift || caps ? toupper(c) : c);
				shift = false;
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(uint i=0;i<(sizeof(keysKanaSpecial)/sizeof(keysKanaSpecial[0]));i++) {
		if((touch.px > keysKanaSpecial[i].x-2 && touch.px < keysKanaSpecial[i].x+34) && (touch.py > keysKanaSpecial[i].y+(192-keyboardData.height)-2 && touch.py < keysKanaSpecial[i].y+34+(192-keyboardData.height))) {
			if(keysKanaSpecial[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysKanaSpecial[i].x, keysKanaSpecial[i].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysKanaSpecial[i].character == "entr") {
				drawRectangle(keysKanaSpecial[3].x, keysKanaSpecial[3].y+(192-keyboardData.height), 32, 64, DARK_GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysKanaSpecial[i].character == "shft") {
				drawRectangle(keysKanaSpecial[i].x, keysKanaSpecial[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				
				char c = string[string.length()-1];
				string = string.substr(0, string.length()-1);
				string += isupper(c) ? tolower(c) : toupper(c);

				whileHeld();
			}
			return;
		}
	}
}

std::string Input::getLine() { return Input::getLine(-1); }

std::string Input::getLine(uint maxLength) {
	clearVars();
	// Hide sprites below the keyboard
	for(int i=0;i<30;i++) {
		setSpriteVisibility(i, false);
	}
	updateOam();

	drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARKER_GRAY, false);
	drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
	int held, pressed, cursorBlink = 30;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			held = keysDownRepeat();
			pressed = keysDown();
			if(cursorBlink == 30) {
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
				printText(string+"_", 0, 192-keyboardData.height-16, false);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
				printText(string, 0, 192-keyboardData.height-16, false);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!held);
			if(caps) drawRectangle(keysSpecialQwerty[1].x, keysSpecialQwerty[1].y+(192-keyboardData.height), 16, 16, GRAY, false);

		if((keyboardLayout ? held : pressed) & KEY_TOUCH) {
			touchRead(&touch);
			if(keyboardLayout == 0)			processTouchKana(touch, maxLength);
			else if(keyboardLayout == 1)	processTouchQwerty(touch, maxLength);
			
			drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			printText(string + (cursorBlink ? "_" : ""), 0, 192-keyboardData.height-16, false);
		} else if(held & KEY_B) {
			if(keyboardLayout == 0)			drawRectangle(keysKanaSpecial[1].x, keysKanaSpecial[1].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
			else if(keyboardLayout == 1)	drawRectangle(keysSpecialQwerty[0].x, keysSpecialQwerty[0].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			if(!shift)	printText(string, 0, 192-keyboardData.height-16, false);
		} else if (pressed & KEY_SELECT) {
			keyboard.clear();
			if(keyboardLayout == 0) {
				keyboardData = loadPng("nitro:/graphics/keyboardQwerty.png", keyboard);
				keyboardLayout = 1;
			} else if(keyboardLayout == 1) {
				keyboardData = loadPng("nitro:/graphics/keyboardKana.png", keyboard);
				keyboardLayout = 0;
			}
			drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARKER_GRAY, false);
			drawImage(0, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
		}
		if(held & KEY_START || enter) {
			drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARK_GRAY, false);
			drawBox(false);
			break;
		}
	}
	return string;
}