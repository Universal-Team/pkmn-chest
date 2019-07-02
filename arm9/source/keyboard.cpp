#include "keyboard.h"
#include <string>
#include <stdio.h>
#include <ctype.h>

#include "config.h"
#include "colors.h"
#include "graphics.h"
#include "langStrings.h"
#include "manager.h"

struct Key {
	std::string character;
	int x;
	int y;
};

bool caps = false, enter = false;
int changeLayout = -1, loadedLayout = -1, xPos = 0, shift = 0;
ImageData keyboardData;
std::string string;
std::vector<u16> keyboard;

Key keys123[] = {
	{"1", 34,  0},	{"2", 68,   0},	{"3", 102,  0},
	{"4", 34, 34},	{"5", 68,  34},	{"6", 102, 34},
	{"7", 34, 68},	{"8", 68,  68},	{"9", 102, 68},
					{"0", 68, 102},
};
Key keysABC[] = {
	{"@#/&_", 34,   0},	{"abc",   68,   0},	{"def",  102,   0},
	{"ghi",   34,  34},	{"jkl",   68,  34},	{"mno",  102,  34},	{" ", 136, 34},
	{"pqrs",  34,  68},	{"tuv",   68,  68},	{"wxyz", 102,  68},
						{"'\"()", 68, 102},	{".,?!", 102, 102},
};
Key keysSpecialKana[] = {
	{"shft",  34, 102},
	{"bksp", 136,   0},
	{"spce", 136,  34},
	{"entr", 136,  68},
	{"entr", 136, 102},
	{"123",    0,   0},
	{"ABC",    0,  34},
	{"AIU",    0,  68},
	{"QWE",   0, 102},
};

Key keysQWE[] = {
	{"1", 0, 0}, {"2", 20, 0}, {"3", 40, 0}, {"4", 60, 0}, {"5", 80, 0}, {"6", 100, 0}, {"7", 120, 0}, {"8", 140, 0}, {"9", 160, 0}, {"0", 180, 0}, {"-", 200, 0}, {"=", 220, 0},
	{"q", 10, 18}, {"w", 30, 18}, {"e", 50, 18}, {"r", 70, 18}, {"t", 90, 18}, {"y", 110, 18}, {"u", 130, 18}, {"i", 150, 18}, {"o", 170, 18}, {"p", 190, 18}, {"[", 210, 18}, {"]", 230, 18},
	{"a", 20, 36}, {"s", 40, 36}, {"d", 60, 36}, {"f", 80, 36}, {"g", 100, 36}, {"h", 120, 36}, {"j", 140, 36}, {"k", 160, 36}, {"l", 180, 36}, {";", 200, 36}, {"'", 220, 36},
	{"z", 30, 54}, {"x", 50, 54}, {"c", 70, 54}, {"v", 90, 54}, {"b", 110, 54}, {"n", 130, 54}, {"m", 150, 54}, {",", 170, 54}, {".", 190, 54}, {"/", 210, 54},
};
Key keysSpecialQWE[] = {
	{"bksp", 240, 0},	// Backspace
	{"caps", 0, 36},	// Caps Lock
	{"entr", 240, 36},	// Enter
	{"lsft", 0, 54},	// Left Shift
	{"rsft", 230, 54},	// Right Shift
	{"mode", 50, 72},	// Change mode
};

void clearVars(void) {
	string = "", caps = false, shift = false, enter = false, changeLayout = -1;
}

void whileHeld(void) {
	while(keysHeld()) {
		swiWaitForVBlank();
		scanKeys();
	}
}

void drawKeyboard(int layout) {
	keyboard.clear();
	if(loadedLayout != layout) {
		ImageData prevData = keyboardData;
		int prevLayout = loadedLayout;
		switch(layout) {
			default:
			case 0:
				keyboardData = loadPng("nitro:/graphics/keyboard123.png", keyboard);
				loadedLayout = 0;
				xPos = Config::keyboardXPos;
				break;
			case 1:
				keyboardData = loadPng("nitro:/graphics/keyboardABC.png", keyboard);
				loadedLayout = 1;
				xPos = Config::keyboardXPos;
				break;
			case 2:
				keyboardData = loadPng("nitro:/graphics/keyboardABC.png", keyboard);
				loadedLayout = 1;
				xPos = Config::keyboardXPos;
				break;
			case 3:
				keyboardData = loadPng("nitro:/graphics/keyboardQWE.png", keyboard);
				loadedLayout = 3;
				xPos = 0;
				break;
		}
		if(prevLayout != -1)	drawRectangle(0, 192-prevData.height-16, 256, prevData.height+16, DARKER_GRAY, false);
	}
	drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, DARKER_GRAY, false);
	drawImage(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);
}

void processTouch123(touchPosition touch, uint maxLength) {
	if(string.length() < maxLength) {
		// Check if a number key was pressed
		for(uint i=0;i<(sizeof(keys123)/sizeof(keys123[0]));i++) {
			if((touch.px > keys123[i].x+xPos-2 && touch.px < keys123[i].x+xPos+34) && (touch.py > keys123[i].y+(192-keyboardData.height)-2 && touch.py < keys123[i].y+34+(192-keyboardData.height))) {
				drawRectangle(keys123[i].x+xPos, keys123[i].y+(192-keyboardData.height), 32, 32, (keys123[i].character == " " ? GRAY : DARK_GRAY), false);
				whileHeld();
				string += keys123[i].character;
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(uint i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboardData.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[3].x+xPos, keysSpecialKana[3].y+(192-keyboardData.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "123") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == "ABC") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == "AIU") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == "QWE") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos+keyboardData.width) {
		drawRectangle(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, DARKER_GRAY, false);
		xPos = 256-keyboardData.width;
	} else if(touch.px < xPos) {
		drawRectangle(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, DARKER_GRAY, false);
		xPos = 0;
	}
}

void processTouchABC(touchPosition touch, uint maxLength) {
	if(string.length() < maxLength) {
		// Check if an ABC key was pressed
		for(uint i=0;i<(sizeof(keysABC)/sizeof(keysABC[0]));i++) {
			if((touch.px > keysABC[i].x+xPos-2 && touch.px < keysABC[i].x+xPos+34) && (touch.py > keysABC[i].y+(192-keyboardData.height)-2 && touch.py < keysABC[i].y+34+(192-keyboardData.height))) {
				drawRectangle(keysABC[i].x+xPos, keysABC[i].y+(192-keyboardData.height), 32, 32, (keysABC[i].character == " " ? GRAY : DARK_GRAY), false);
				char c = '\0';
				while(keysHeld() & KEY_TOUCH) {
					if(touch.px < keysABC[i].x+xPos) {
						c = keysABC[i].character[1];
						break;
					} else if(touch.py < keysABC[i].y+(192-keyboardData.height)) {
						c = keysABC[i].character[2];
						break;
					} else if(touch.px > keysABC[i].x+xPos+32) {
						c = keysABC[i].character[3];
						break;
					} else if(touch.py > keysABC[i].y+(192-keyboardData.height)+32) {
						c = keysABC[i].character[4];
						break;
					}
					swiWaitForVBlank();
					scanKeys();
					touchRead(&touch);
				}
				if(c == '\0')	c = keysABC[i].character[0];
				string += c;
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(uint i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboardData.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[3].x+xPos, keysSpecialKana[3].y+(192-keyboardData.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "shft") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
				
				char c = string[string.length()-1];
				string = string.substr(0, string.length()-1);
				string += isupper(c) ? tolower(c) : toupper(c);

				whileHeld();
			} else if(keysSpecialKana[i].character == "123") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == "ABC") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == "AIU") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == "QWE") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos+keyboardData.width) {
		drawRectangle(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, DARKER_GRAY, false);
		xPos = 256-keyboardData.width;
	} else if(touch.px < xPos) {
		drawRectangle(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, DARKER_GRAY, false);
		xPos = 0;
	}
}

void processTouchQWE(touchPosition touch, uint maxLength) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(uint i=0;i<(sizeof(keysQWE)/sizeof(keysQWE[0]));i++) {
			if((touch.px > keysQWE[i].x-2 && touch.px < keysQWE[i].x+18) && (touch.py > keysQWE[i].y+(192-keyboardData.height)-2 && touch.py < keysQWE[i].y+18+(192-keyboardData.height))) {
				drawRectangle(keysQWE[i].x, keysQWE[i].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
				char c = keysQWE[i].character[0];
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
	for(uint i=0;i<(sizeof(keysSpecialQWE)/sizeof(keysSpecialQWE[0]));i++) {
		if((touch.px > keysSpecialQWE[i].x-2 && touch.px < keysSpecialQWE[i].x+18) && (touch.py > keysSpecialQWE[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialQWE[i].y+18+(192-keyboardData.height))) {
			if(keysSpecialQWE[i].character == "bksp") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
				string = string.substr(0, string.length()-1);
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
				printText(string, 0, 192-keyboardData.height-16, false);
			} else if(keysSpecialQWE[i].character == "caps") {
				caps = !caps;
				if(caps) drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 16, 16, GRAY, false);
				whileHeld();
			} else if(keysSpecialQWE[i].character == "entr") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 16, 16, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialQWE[i].character == "lsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 1;
				}
			} else if(keysSpecialQWE[i].character == "rsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 2;
				}
			} else if(keysSpecialQWE[i].character == "mode") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 16, 16, GRAY, false);
				whileHeld();
				changeLayout = 1;
			}
			break;
		}
	}
}

std::string Input::getLine() { return Input::getLine(-1); }

std::string Input::getLine(uint maxLength) {
	clearVars();
	drawKeyboard(Config::keyboardLayout);
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
			if(caps) drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboardData.height), 16, 16, GRAY, false);

		if((loadedLayout == 3 ? held : pressed) & KEY_TOUCH) {
			touchRead(&touch);
			if(loadedLayout == 0)			processTouch123(touch, maxLength);
			else if(loadedLayout == 1)	processTouchABC(touch, maxLength);
			else if(loadedLayout == 3)	processTouchQWE(touch, maxLength);
			
			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);
			// Print string
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			printText(string + (cursorBlink ? "_" : ""), 0, 192-keyboardData.height-16, false);

			// If caps lock / shift are on, highlight the key
			if(caps)	drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboardData.height), 16, 16, GRAY, false);
			if(shift)	drawRectangle(keysSpecialQWE[2+shift].x, keysSpecialQWE[2+shift].y+(192-keyboardData.height), 26, 16, GRAY, false);
		} else if(held & KEY_B) {
			if(loadedLayout < 3)	drawRectangle(keysSpecialKana[1].x+xPos, keysSpecialKana[1].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
			else	drawRectangle(keysSpecialQWE[0].x, keysSpecialQWE[0].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			printText(string, 0, 192-keyboardData.height-16, false);
		}
		if(held & KEY_START || enter) {
			Config::keyboardLayout = loadedLayout;
			if(loadedLayout < 3)	Config::keyboardXPos = xPos;
			Config::saveConfig();
			break;
		} else if (pressed & KEY_SELECT) {
			if(loadedLayout < 3)	drawKeyboard(loadedLayout+1);
			else	drawKeyboard(0);
		} else if(changeLayout != -1) {
			drawKeyboard(changeLayout);
			changeLayout = -1;
		}
	}
	return string;
}

int Input::getInt() { return Input::getInt(-1); }

// Returns -1 if nothing entered
int Input::getInt(uint max) {
	char str[4];
	itoa(max, str, 10);
	uint maxLength = strlen(str);
	clearVars();
	drawKeyboard(0);
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
			if(caps) drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboardData.height), 16, 16, GRAY, false);

		if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			processTouch123(touch, maxLength);
			
			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);
			// Print string
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			printText(string + (cursorBlink ? "_" : ""), 0, 192-keyboardData.height-16, false);
		} else if(held & KEY_B) {
			drawRectangle(keysSpecialKana[1].x+xPos, keysSpecialKana[1].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, DARKER_GRAY, false);
			printText(string, 0, 192-keyboardData.height-16, false);
		}
		if(held & KEY_START || enter) {
			loadedLayout = -1; // So it doesn't draw the extra rectangle if drawing QWERTY next
			break;
		}
	}
	if(string == "") return -1;
	uint i = atoi(string.c_str());
	if(i > max)	return max;
	return i;
}

bool Input::getBool() { return getBool(Lang::yes, Lang::no); }
bool Input::getBool(std::string optionTrue, std::string optionFalse) {
	// Draw rectangles
	drawRectangle(38, 66, 180, 60, DARKER_GRAY, false);
	drawRectangle(48, 76, 70, 40, LIGHT_GRAY, false);
	drawRectangle(138, 76, 70, 40, LIGHT_GRAY, false);

	// Print text
	printTextCenteredTintedMaxW(optionFalse, 60, 1, DARK_GRAY, -45, 88, false);
	printTextCenteredTintedMaxW(optionTrue, 60, 1, DARK_GRAY, 45, 88, false);

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
			if(touch.py > 76 && touch.py < 116) {
				if(touch.px > 48 && touch.px < 118) {
					return false;
				} else if(touch.px > 138 && touch.px < 208) {
					return true;
				}
			}
		} else if(pressed & KEY_A) {
			return true;
		} else if(pressed & KEY_B) {
			return false;
		}
	}
}
