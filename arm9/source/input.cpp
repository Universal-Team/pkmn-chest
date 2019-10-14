#include "input.hpp"
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <string>

#include "config.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "langStrings.hpp"
#include "manager.hpp"
#include "sound.hpp"

#define u8u16(str) StringUtils::UTF8toUTF16(str)

char16_t iskatakana(char16_t c) { return c >= 0x30A1; }
char16_t tokatakana(char16_t c) {
	if(c >= 0x3041 && c <= 0x3096)	return c + 96;
	return c;
}
char16_t tohiragana(char16_t c) {
	if(c >= 0x30A1 && c <= 0x30F6)	return c - 96;
	return c;
}
char16_t nextcharver(char16_t c) {
	bool katakana = iskatakana(c);
	if(katakana)	c -= 96;
	if((c >= 0x3041 && c <= 0x3061 && c % 2)
	|| (c == 0x3066 || c == 0x3068)
	|| (c >= 0x306F && c <= 0x307C && (c % 3 != 2))
	|| (c >= 0x3083 && c <= 0x3087 && c % 2)) {
		c++;
	} else if(c == 0x3063) {
		c += 2;
	} else if((c >= 0x3042 && c <= 0x3062 && !(c % 2))
		   || (c == 0x3064 || c == 0x3065 || c == 0x3067 || c == 0x3069)
		   || (c >= 0x3084 && c <= 0x3088 && !(c % 2))) {
		c--;
	} else if(c >= 0x3071 && c <= 0x307D && (c % 3 == 2)) {
		c -= 2;
	}
	return katakana ? c+96 : c;
}

struct Key {
	std::string character;
	int x;
	int y;
};

struct Key16 {
	std::u16string character;
	int x;
	int y;
};

bool caps = false, enter = false, katakana = false;
int changeLayout = -1, loadedLayout = -1, xPos = 0, shift = 0, kanaMode = 0;;
ImageData keyboardData;
std::u16string string;
std::vector<u16> keyboard;

Key keys123[] = {
	{"1", 34,  0},	{"2", 68,   0},	{"3", 102,  0},
	{"4", 34, 34},	{"5", 68,  34},	{"6", 102, 34},
	{"7", 34, 68},	{"8", 68,  68},	{"9", 102, 68},
					{"0", 68, 102},
};
Key keysABC[] = {
	{"@#/&", 34,  0},	{"abc",   68,   0},	{"def",  102,   0},
	{"ghi",  34, 34},	{"jkl",   68,  34},	{"mno",  102,  34},	{" ", 136, 34},
	{"pqrs", 34, 68},	{"tuv",   68,  68},	{"wxyz", 102,  68},
						{"'\"()", 68, 102},	{".,?!", 102, 102},
};

std::vector<Key16> keysDPadABC = {
	{u8u16("abc."), 112,  40}, //			abc.
	{u8u16("def!"), 140,  52}, //		wxyz	def!
	{u8u16("ghi,"), 152,  80}, //	tuv;			ghi,
	{u8u16("jkl?"), 140, 108}, //		pqrs	jkl?
	{u8u16("mno:"), 112, 120}, //			mno:
	{u8u16("pqrs"),  84, 108},
	{u8u16("tuv;"),  72,  80},
	{u8u16("wxyz"),  84,  52},
};
std::vector<Key16> keysDPadABC2 = {
	{u8u16("1234"), 112,  40}, //			1234
	{u8u16("5678"), 140,  52}, //		…•~@	5678
	{u8u16("90)("), 152,  80}, //	#%+*			90)(
	{u8u16("=-_|"), 140, 108}, //		♂♀/\	|=_-
	{u8u16("‘’”“"), 112, 120}, //			‘’”“
	{u8u16("♂♀/\\"), 84, 108},
	{u8u16("#%*+"),  72,  80},
	{u8u16("…・~@"), 84,  52},
};
std::vector<Key16> keysDPadABC3 = {
	{u8u16("abcd"), 112,  60}, //		abcd
	{u8u16("efgh"), 132,  80}, //	mnop	efgh
	{u8u16("ijkl"), 112, 100}, //		ijkl
	{u8u16("mnop"),  92,  80},
	{u8u16("qrst"), 112,  60}, //		qrst
	{u8u16("uvwx"), 132,  80}, //	!?)(	uvwx
	{u8u16("yz.,"), 112, 100}, //		yz.,
	{u8u16("!?)("),  92,  80},
};
std::vector<Key16> keysDPadAIU = {
	{u8u16("あいうえお"), 112,  40}, //	　　　　　　　　　　あいうえお
	{u8u16("かきくけこ"), 140,  52}, //	　　　　　や　ゆ　よ　　　　　かきくけこ
	{u8u16("さしすせそ"), 152,  80}, //	まみむめも　　　　　　　　　　　　　　　さしすせそ
	{u8u16("たちつてと"), 140, 108}, //	　　　　　はひふへほ　　　　　たちつてと
	{u8u16("なにぬねの"), 112, 120}, //	　　　　　　　　　　なにぬねの
	{u8u16("はひふへほ"),  84, 108},
	{u8u16("まみむめも"),  72,  80},
	{u8u16("や ゆ よ"),   84,  52},
};
std::vector<Key16> keysDPadAIU2 = {
	{u8u16("ぁぃぅぇぉ"), 112,  40}, //	　　　　　　　　　　ぁぃぅぇぉ
	{u8u16("がぎぐげご"), 140,  52}, //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	{u8u16("ざじずぜぞ"), 152,  80}, //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	{u8u16("だぢっでど"), 140, 108}, //	　　　　　ばびぶべぼ　　　　　だぢっでど
	{u8u16("らりるれろ"), 112, 120}, //	　　　　　　　　　　らりるれろ
	{u8u16("ばびぶべぼ"),  84, 108},
	{u8u16("わをんー"),    72,  80},
	{u8u16("ゃ ゅ ょ"),    84,  52},
};
std::vector<Key16> keysDPadAIU3 = {
	{u8u16("ぁぃゔぇぉ"), 112,  40}, //	　　　　　　　　　　ぁぃゔぇぉ
	{u8u16("がぎぐげご"), 140,  52}, //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	{u8u16("ざじずぜぞ"), 152,  80}, //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	{u8u16("だぢづでど"), 140, 108}, //	　　　　　ぱぴぷぺぽ　　　　　だぢづでど
	{u8u16("らりるれろ"), 112, 120}, //	　　　　　　　　　　らりるれろ
	{u8u16("ぱぴぷぺぽ"),  84, 108},
	{u8u16("わをんー"),    72,  80},
	{u8u16("ゃ ゅ ょ"),    84,  52},
};

Key16 keysAIU[] = {
	{u8u16("あいうえお"), 34,  0},	{u8u16("かきくけこ"), 68,   0},	{u8u16("さしすせそ"), 102,   0},
	{u8u16("たちつてと"), 34, 34},	{u8u16("なにぬねの"), 68,  34},	{u8u16("はひふへほ"), 102,  34},	{u8u16(" "), 136, 34},
	{u8u16("まみむめも"), 34, 68},	{u8u16("や ゆ よ"),  68,  68},	{u8u16("らりるれろ"), 102,  68},
								  {u8u16("わをんー"),   68, 102},
};
Key keysSpecialKana[] = {
	{"shft",  34, 102},
	{"hika", 102, 102},
	{"bksp", 136,   0},
	{"spce", 136,  34},
	{"entr", 136,  68},
	{"entr", 136, 102},
	{"123",    0,   0},
	{"ABC",    0,  34},
	{"AIU",    0,  68},
	{"QWE",    0, 102},
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
	string = u8u16(""), caps = false, shift = false, enter = false, katakana = false, changeLayout = -1, kanaMode = 0;
}

void whileHeld(void) {
	while(keysHeld()) {
		swiWaitForVBlank();
		scanKeys();
	}
}

void drawKeyboard(int layout) {
	if(loadedLayout != layout) {
		keyboard.clear();
		ImageData prevData = keyboardData;
		int prevLayout = loadedLayout;
		if(layout == 3) {
			keyboardData = loadPng("nitro:/graphics/keyboardQWE.png", keyboard);
			loadedLayout = 3;
			xPos = 0;
		} else {
			keyboardData = loadPng("nitro:/graphics/keyboardKana.png", keyboard);
			loadedLayout = layout;
		}
		if(prevLayout != -1)	drawRectangle(0, 192-prevData.height-16, 256, prevData.height+16, BLACK, false);
	}
	drawRectangle(0, 192-keyboardData.height-16, 256, keyboardData.height+16, BLACK, false);
	drawImage(xPos, 192-keyboardData.height, keyboardData.width, keyboardData.height, keyboard, false);

	if(layout == 0) {
		for(unsigned i=0;i<(sizeof(keys123)/sizeof(keys123[0]));i++) {
			printTextTinted(keys123[i].character, GRAY, xPos+keys123[i].x+16-(getTextWidth(keys123[i].character)/2), 192-keyboardData.height+keys123[i].y+8, false, true);
		}
	} else if(layout == 1) {
		for(unsigned i=0;i<(sizeof(keysABC)/sizeof(keysABC[0]));i++) {
			printTextTinted(keysABC[i].character, GRAY, xPos+keysABC[i].x+16-(getTextWidth(keysABC[i].character)/2), 192-keyboardData.height+keysABC[i].y+8, false, true);
		}
		printTextTinted("a/A", GRAY, xPos+keysSpecialKana[0].x+16-(getTextWidth("a/A")/2), 192-keyboardData.height+keysSpecialKana[0].y+8, false, true);
	} else if(layout == 2) {
		for(unsigned i=0;i<(sizeof(keysAIU)/sizeof(keysAIU[0]));i++) {
			std::u16string str;
			str += (katakana ? tokatakana(keysAIU[i].character[0]) : keysAIU[i].character[0]);
			printTextTinted(str, GRAY, xPos+keysAIU[i].x+16-(getTextWidth(str)/2), 192-keyboardData.height+keysAIU[i].y+8, false, true);
		}
		printTextTinted(katakana ? "ｯﾞﾟ" : "っﾞﾟ", GRAY, xPos+keysSpecialKana[0].x+16-(getTextWidth(katakana ? "ｯﾞﾟ" : "っﾞﾟ")/2), 192-keyboardData.height+keysSpecialKana[0].y+8, false, true);
		printTextTinted(katakana ? "あ" : "ア", GRAY, xPos+keysSpecialKana[1].x+16-(getTextWidth(katakana ? "あ" : "ア")/2), 192-keyboardData.height+keysSpecialKana[1].y+8, false, true);
	} else if(layout == 3) {
		for(unsigned i=0;i<(sizeof(keysQWE)/sizeof(keysQWE[0]));i++) {
			std::string str;
			str += (caps||shift ? toupper(keysQWE[i].character[0]) : keysQWE[i].character[0]);
			printText(str, xPos+keysQWE[i].x+8-(getTextWidth(str)/2), 192-keyboardData.height+keysQWE[i].y, false);
		}
	}
}

void processInputABC(u16 held, unsigned maxLength) {
	u32 prevHeld = 0;
	while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
		do {
			swiWaitForVBlank();
			scanKeys();
			held = keysDownRepeat();
		} while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT) && keysHeld() == prevHeld);
		prevHeld = keysHeld();

		int direction = -1;
		switch(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			case KEY_UP:
				direction = 0;
				break;
			case KEY_UP | KEY_RIGHT:
				direction = 1;
				break;
			case KEY_RIGHT:
				direction = 2;
				break;
			case KEY_RIGHT | KEY_DOWN:
				direction = 3;
				break;
			case KEY_DOWN:
				direction = 4;
				break;
			case KEY_DOWN | KEY_LEFT:
				direction = 5;
				break;
			case KEY_LEFT:
				direction = 6;
				break;
			case KEY_LEFT | KEY_UP:
				direction = 7;
				break;
		}

		if(direction != -1) {
			bool upper = keysHeld() & KEY_R;
			Key16 *key = (keysHeld() & KEY_L) ? &keysDPadABC2[direction] : &keysDPadABC[direction];

			fillSpriteImageScaled(keyboardSpriteID, false, 0, 0, keyboardKeyData.width, keyboardKeyData.height, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, key->x, key->y);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::pair<int, int> offsets[] = {{16, 0}, {24, 8}, {16, 16}, {8, 8}};
			for(unsigned i=0;i<key->character.size();i++) {
				std::u16string str = key->character.substr(i, 1);
				if(upper)	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
				fillSpriteText(keyboardSpriteID, false, str, WHITE, offsets[i].first-(getTextWidth(str)/2), offsets[i].second, false);
			}
		}

		if(held & (KEY_A | KEY_B | KEY_X | KEY_Y)) {
			int key = -1;
			switch(held & (KEY_A | KEY_B | KEY_X | KEY_Y)) {
				case KEY_X:
					key = 0;
					break;
				case KEY_A:
					key = 1;
					break;
				case KEY_B:
					key = 2;
					break;
				case KEY_Y:
					key = 3;
					break;
			}

			if(direction != -1 && key != -1 && key < (int)keysDPadABC[direction].character.size() && string.size() < maxLength) {
				Key16 *selectedKey = (keysHeld() & KEY_L) ? &keysDPadABC2[direction] : &keysDPadABC[direction];
				char16_t c = selectedKey->character[key];
				string += (keysHeld() & KEY_R) ? toupper(c) : c;
				break;
			}
		}
	}
}

void processInputABCNoDiagonal(u16 held, unsigned maxLength) {
	while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
		u32 prevHeld = 0;
		do {
			swiWaitForVBlank();
			scanKeys();
			held = keysDownRepeat();
		} while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT) && keysHeld() == prevHeld);

		int direction = -1;
		switch(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			case KEY_UP:
				direction = 0;
				break;
			case KEY_RIGHT:
				direction = 1;
				break;
			case KEY_DOWN:
				direction = 2;
				break;
			case KEY_LEFT:
				direction = 3;
				break;
		}

		if(direction != -1) {
			bool upper = keysHeld() & KEY_R;
			Key16 *key = &keysDPadABC3[direction + ((keysHeld() & KEY_L) ? 4 : 0)];

			fillSpriteImageScaled(keyboardSpriteID, false, 0, 0, keyboardKeyData.width, keyboardKeyData.height, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, key->x, key->y);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::pair<int, int> offsets[] = {{16, 0}, {24, 8}, {16, 16}, {8, 8}};
			for(unsigned i=0;i<key->character.size();i++) {
				std::u16string str = key->character.substr(i, 1);
				if(upper)	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
				fillSpriteText(keyboardSpriteID, false, str, WHITE, offsets[i].first-(getTextWidth(str)/2), offsets[i].second, false);
			}
		}

		if(held & (KEY_A | KEY_B | KEY_X | KEY_Y)) {
			int key = -1;
			switch(held & (KEY_A | KEY_B | KEY_X | KEY_Y)) {
				case KEY_X:
					key = 0;
					break;
				case KEY_A:
					key = 1;
					break;
				case KEY_B:
					key = 2;
					break;
				case KEY_Y:
					key = 3;
					break;
			}

			if(direction != -1 && key != -1 && key < (int)keysDPadABC[direction].character.size() && string.size() < maxLength) {
				Key16 *selectedKey = &keysDPadABC3[direction + ((keysHeld() & KEY_L) ? 4 : 0)];
				char16_t c = selectedKey->character[key];
				string += (keysHeld() & KEY_R) ? toupper(c) : c;
				break;
			}
		}
	}
}

void processInputAIU(u16 held, unsigned maxLength) {
	u16 pressed;
	while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
		u32 prevHeld = 0;
		do {
			swiWaitForVBlank();
			scanKeys();
			held = keysDownRepeat();
			pressed = keysDown();
		} while(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT) && keysHeld() == prevHeld);

		if(pressed & KEY_L) {
			if(kanaMode < 2) kanaMode++;
			else kanaMode = 0;
		}

		int direction = -1;
		switch(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			case KEY_UP:
				direction = 0;
				break;
			case KEY_UP | KEY_RIGHT:
				direction = 1;
				break;
			case KEY_RIGHT:
				direction = 2;
				break;
			case KEY_RIGHT | KEY_DOWN:
				direction = 3;
				break;
			case KEY_DOWN:
				direction = 4;
				break;
			case KEY_DOWN | KEY_LEFT:
				direction = 5;
				break;
			case KEY_LEFT:
				direction = 6;
				break;
			case KEY_LEFT | KEY_UP:
				direction = 7;
				break;
		}

		if(direction != -1) {
			bool katakana = keysHeld() & KEY_R;
			Key16 *key = (kanaMode == 0) ? &keysDPadAIU[direction] : (kanaMode == 1) ? &keysDPadAIU2[direction] : &keysDPadAIU3[direction];

			fillSpriteImageScaled(keyboardSpriteID, false, 0, 0, keyboardKeyData.width, keyboardKeyData.height, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, key->x, key->y);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::u16string str; str += katakana ? tokatakana(key->character[0]) : key->character[0];
			fillSpriteText(keyboardSpriteID, false, str, WHITE, 16-(getTextWidth(str)/2), 8, false);
		}

		if(held & (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_START)) {
			int key = -1;
			switch(held & (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_START)) {
				case KEY_START:
					key = 0;
					break;
				case KEY_Y:
					key = 1;
					break;
				case KEY_X:
					key = 2;
					break;
				case KEY_A:
					key = 3;
					break;
				case KEY_B:
					key = 4;
					break;
			}

			if(direction != -1 && key != -1 && key < (int)keysDPadAIU[direction].character.size() && string.size() < maxLength) {
				Key16 *selectedKey = (kanaMode == 0) ? &keysDPadAIU[direction] : (kanaMode == 1) ? &keysDPadAIU2[direction] : &keysDPadAIU3[direction];
				char16_t c = selectedKey->character[key];
				string += (keysHeld() & KEY_R) ? tokatakana(c) : c;
				break;
			}
		}
	}
}

void processTouch123(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a number key was pressed
		for(unsigned i=0;i<(sizeof(keys123)/sizeof(keys123[0]));i++) {
			if((touch.px > keys123[i].x+xPos-2 && touch.px < keys123[i].x+xPos+34) && (touch.py > keys123[i].y+(192-keyboardData.height)-2 && touch.py < keys123[i].y+34+(192-keyboardData.height))) {
				drawRectangle(keys123[i].x+xPos, keys123[i].y+(192-keyboardData.height), 32, 32, (keys123[i].character == " " ? GRAY : DARK_GRAY), false);
				whileHeld();
				string += u8u16(keys123[i].character);
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboardData.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboardData.height), 32, 64, GRAY, false);
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
		xPos = 256-keyboardData.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchABC(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if an ABC key was pressed
		for(unsigned i=0;i<(sizeof(keysABC)/sizeof(keysABC[0]));i++) {
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
	for(unsigned i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboardData.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboardData.height), 32, 64, GRAY, false);
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
		xPos = 256-keyboardData.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchAIU(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a kana key was pressed
		for(unsigned i=0;i<(sizeof(keysAIU)/sizeof(keysAIU[0]));i++) {
			if((touch.px > keysAIU[i].x+xPos-2 && touch.px < keysAIU[i].x+xPos+34) && (touch.py > keysAIU[i].y+(192-keyboardData.height)-2 && touch.py < keysAIU[i].y+34+(192-keyboardData.height))) {
				drawRectangle(keysAIU[i].x+xPos, keysAIU[i].y+(192-keyboardData.height), 32, 32, (keysAIU[i].character == u8u16(" ") ? GRAY : DARK_GRAY), false);
				u16 c = '\0';
				while(keysHeld() & KEY_TOUCH) {
					if(touch.px < keysAIU[i].x+xPos) {
						c = keysAIU[i].character[1];
						break;
					} else if(touch.py < keysAIU[i].y+(192-keyboardData.height)) {
						c = keysAIU[i].character[2];
						break;
					} else if(touch.px > keysAIU[i].x+xPos+32) {
						c = keysAIU[i].character[3];
						break;
					} else if(touch.py > keysAIU[i].y+(192-keyboardData.height)+32) {
						c = keysAIU[i].character[4];
						break;
					}
					swiWaitForVBlank();
					scanKeys();
					touchRead(&touch);
				}
				if(c == '\0')	c = keysAIU[i].character[0];
				string += katakana ? tokatakana(c) : c;
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboardData.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboardData.height-16, false);
					for(int i=0;i<10 && keysHeld() & KEY_TOUCH;i++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboardData.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "shft") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);
				
				char16_t c = string[string.length()-1];
				string = string.substr(0, string.length()-1);
				string += nextcharver(c);

				whileHeld();
			} else if(keysSpecialKana[i].character == "hika") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboardData.height), 32, 32, DARK_GRAY, false);

				katakana = !katakana;
				drawKeyboard(loadedLayout);

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
		xPos = 256-keyboardData.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchQWE(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(unsigned i=0;i<(sizeof(keysQWE)/sizeof(keysQWE[0]));i++) {
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
			string += u8u16(key.character);
			shift = false;
			printText(string, 0, 192-keyboardData.height-16, false);
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialQWE)/sizeof(keysSpecialQWE[0]));i++) {
		if((touch.px > keysSpecialQWE[i].x-2 && touch.px < keysSpecialQWE[i].x+18) && (touch.py > keysSpecialQWE[i].y+(192-keyboardData.height)-2 && touch.py < keysSpecialQWE[i].y+18+(192-keyboardData.height))) {
			if(keysSpecialQWE[i].character == "bksp") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboardData.height), 16, 16, DARK_GRAY, false);
				string = string.substr(0, string.length()-1);
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
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

std::string Input::getLine(unsigned maxLength) {
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
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
				printText(string+u8u16("_"), 0, 192-keyboardData.height-16, false);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
				printText(string, 0, 192-keyboardData.height-16, false);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!keysHeld());
			if(caps) drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboardData.height), 16, 16, GRAY, false);

		if((loadedLayout == 3 ? held : pressed) & KEY_TOUCH) {
			touchRead(&touch);
			if(loadedLayout == 0)			processTouch123(touch, maxLength);
			else if(loadedLayout == 1)	processTouchABC(touch, maxLength);
			else if(loadedLayout == 2)	processTouchAIU(touch, maxLength);
			else if(loadedLayout == 3)	processTouchQWE(touch, maxLength);
			
			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);
			// Print string
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboardData.height-16, false);

			// If caps lock / shift are on, highlight the key
			if(caps)	drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboardData.height), 16, 16, GRAY, false);
			if(shift)	drawRectangle(keysSpecialQWE[2+shift].x, keysSpecialQWE[2+shift].y+(192-keyboardData.height), 26, 16, GRAY, false);
		} else if(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			if(loadedLayout == 2)	processInputAIU(held, maxLength);
			else if(loadedLayout == 3)	processInputABCNoDiagonal(held, maxLength); // for now just use the simple one on qwerty
			else	processInputABC(held, maxLength);

			// Hide sprite
			setSpriteVisibility(keyboardSpriteID, false, false);
			updateOam();

			// Print string
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboardData.height-16, false);
		} else if(held & KEY_B) {
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboardData.height-16, false);
		} else if(held & KEY_Y) {
			string += u8u16(" ");
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboardData.height-16, false);
		}
		if(held & KEY_START || enter) {
			Sound::play(Sound::click);
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
	return StringUtils::UTF16toUTF8(string);
}

int Input::getInt() { return Input::getInt(-1); }

// Returns -1 if nothing entered
int Input::getInt(unsigned max) {
	char str[4];
	__itoa(max, str, 10);
	unsigned maxLength = strlen(str);
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
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
				printText(string+u8u16("_"), 0, 192-keyboardData.height-16, false);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
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
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboardData.height-16, false);
		} else if(held & KEY_B) {
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboardData.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboardData.height-16, false);
		}
		if(held & KEY_START || enter) {
			Sound::play(Sound::click);
			loadedLayout = -1; // So it doesn't draw the extra rectangle if drawing QWERTY next
			break;
		}
	}
	if(string == u8u16("")) return -1;
	unsigned i = std::stoi(StringUtils::UTF16toUTF8(string));
	if(i > max)	return max;
	return i;
}

bool Input::getBool() { return getBool(Lang::yes, Lang::no); }
bool Input::getBool(std::string optionTrue, std::string optionFalse) {
	// Draw rectangles
	drawRectangle(38, 66, 180, 60, DARKER_GRAY, false);
	drawRectangle(48, 76, 70, 40, LIGHT_GRAY, false);
	drawRectangle(138, 76, 70, 40, LIGHT_GRAY, false);
	drawOutline(38, 66, 180, 60, BLACK, false);

	// Print text
	printTextCenteredTintedMaxW(optionFalse, 60, 1, GRAY, -45, 88, false, true);
	printTextCenteredTintedMaxW(optionTrue, 60, 1, GRAY, 45, 88, false, true);

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
					Sound::play(Sound::back);
					return false;
				} else if(touch.px > 138 && touch.px < 208) {
					Sound::play(Sound::click);
					return true;
				}
			}
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return false;
		}
	}
}
