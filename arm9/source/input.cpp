#include "input.hpp"
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <string>

#include "config.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "lang.hpp"
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
	bool isKatakana = iskatakana(c);
	if(isKatakana)	c -= 96;
	if((c >= 0x3041 && c <= 0x3061 && c % 2 && c != 0x3045)
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
	} else if(c == 0x3045) {
		c = 0x3094;
	} else if(c == 0x3094) {
		c = 0x3046;
	}
	return isKatakana ? c+96 : c;
}
char16_t tossang(char16_t c) {
	if(c == 0x3131 || c == 0x3137 || c == 0x3142 || c == 0x3145 || c == 0x3148) {
		c++;
	} else if(c == 0x3150 || c == 0x3154) {
		c += 2;
	}
	return c;
}

std::u16string hangul0  = u8u16("가까.나..다따라.......마바빠.사싸아자짜차카타파하");
std::u16string hangul1  = u8u16("ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎㅗㅜ");
std::u16string hangul2  = u8u16(".ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ");
std::u16string hangul31 = u8u16(".ㄱㅅ");
std::u16string hangul32 = u8u16(".ㅈㅎ");
std::u16string hangul33 = u8u16(".ㄱㅁㅂㅅㅌㅎ");
std::u16string hangul34 = u8u16(".ㅅ");
std::u16string hangulW1 = u8u16(".ㅏㅐㅣ");
std::u16string hangulW2 = u8u16(".ㅓㅔㅣ");

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
std::u16string string;
Image keyboard;

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

std::vector<std::pair<int, int>> keysDPad4 = {
	{112,  60}, //		abcd
	{132,  80}, //	mnop	efgh
	{112, 100}, //		ijkl
	{ 92,  80},
	{112,  60}, //		qrst
	{132,  80}, //	!?)(	uvwx
	{112, 100}, //		yz.,
	{ 92,  80},
};
std::vector<std::pair<int, int>> keysDPad8 = {
	{112,  40}, //			abc.
	{140,  52}, //		wxyz	def!
	{152,  80}, //	tuv;			ghi,
	{140, 108}, //		pqrs	jkl?
	{112, 120}, //			mno:
	{ 84, 108},
	{ 72,  80},
	{ 84,  52},
};

std::vector<std::u16string> keysDPadABC3 = {
	u8u16("abc."), //			abc.
	u8u16("def!"), //		wxyz	def!
	u8u16("ghi,"), //	tuv;			ghi,
	u8u16("jkl?"), //		pqrs	jkl?
	u8u16("mno:"), //			mno:
	u8u16("pqrs"),
	u8u16("tuv;"),
	u8u16("wxyz"),
};
std::vector<std::u16string> keysDPadABC4 = {
	u8u16("abcd"), //			abcd
	u8u16("efgh"), //		!?)(	efgh
	u8u16("ijkl"), //	yz.,			ijkl
	u8u16("mnop"), //		uvwx	mnop
	u8u16("qrst"), //			qrst
	u8u16("uvwx"),
	u8u16("yz.,"),
	u8u16("!?)("),
};
std::vector<std::u16string> keysDPadABCSymbols = {
	u8u16("1234"), //			1234
	u8u16("5678"), //		…•~@	5678
	u8u16("90)("), //	#%+*			90)(
	u8u16("=-_|"), //		♂♀/\	|=_-
	u8u16("‘’”“"), //			‘’”“
	u8u16("♂♀/\\"),
	u8u16("#%*+"),
	u8u16("…・~@"),
};
std::vector<std::u16string> keysDPadAIU = {
	u8u16("あいうえお"), //	　　　　　　　　　　あいうえお
	u8u16("かきくけこ"), //	　　　　　や　ゆ　よ　　　　　かきくけこ
	u8u16("さしすせそ"), //	まみむめも　　　　　　　　　　　　　　　さしすせそ
	u8u16("たちつてと"), //	　　　　　はひふへほ　　　　　たちつてと
	u8u16("なにぬねの"), //	　　　　　　　　　　なにぬねの
	u8u16("はひふへほ"),
	u8u16("まみむめも"),
	u8u16("や ゆ よ"),
};
std::vector<std::u16string> keysDPadAIU2 = {
	u8u16("ぁぃぅぇぉ"), //	　　　　　　　　　　ぁぃぅぇぉ
	u8u16("がぎぐげご"), //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	u8u16("ざじずぜぞ"), //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	u8u16("だぢっでど"), //	　　　　　ばびぶべぼ　　　　　だぢっでど
	u8u16("らりるれろ"), //	　　　　　　　　　　らりるれろ
	u8u16("ばびぶべぼ"),
	u8u16("わをんー"),
	u8u16("ゃ ゅ ょ"),
};
std::vector<std::u16string> keysDPadAIU3 = {
	u8u16("ぁぃゔぇぉ"), //	　　　　　　　　　　ぁぃゔぇぉ
	u8u16("がぎぐげご"), //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	u8u16("ざじずぜぞ"), //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	u8u16("だぢづでど"), //	　　　　　ぱぴぷぺぽ　　　　　だぢづでど
	u8u16("らりるれろ"), //	　　　　　　　　　　らりるれろ
	u8u16("ぱぴぷぺぽ"),
	u8u16("わをんー"),
	u8u16("ゃ ゅ ょ"),
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
	{"1",  0,  0}, {"2", 20,  0}, {"3", 40,  0}, {"4", 60,  0}, {"5",  80,  0}, {"6", 100,  0}, {"7", 120,  0}, {"8", 140,  0}, {"9", 160,  0}, {"0", 180,  0}, {"-", 200,  0}, {"=", 220,  0},
	{"q", 10, 18}, {"w", 30, 18}, {"e", 50, 18}, {"r", 70, 18}, {"t",  90, 18}, {"y", 110, 18}, {"u", 130, 18}, {"i", 150, 18}, {"o", 170, 18}, {"p", 190, 18}, {"[", 210, 18}, {"]", 230, 18},
	{"a", 20, 36}, {"s", 40, 36}, {"d", 60, 36}, {"f", 80, 36}, {"g", 100, 36}, {"h", 120, 36}, {"j", 140, 36}, {"k", 160, 36}, {"l", 180, 36}, {";", 200, 36}, {"'", 220, 36},
	{"z", 30, 54}, {"x", 50, 54}, {"c", 70, 54}, {"v", 90, 54}, {"b", 110, 54}, {"n", 130, 54}, {"m", 150, 54}, {",", 170, 54}, {".", 190, 54}, {"/", 210, 54},
};
Key16 keysKor[] = {
	{u8u16("1"),  0,  0},  {u8u16("2"), 20,  0}, {u8u16("3"), 40,  0}, {u8u16("4"),  60, 0},  {u8u16("5"),  80,  0}, {u8u16("6"),  100, 0},  {u8u16("7"), 120,  0}, {u8u16("8"), 140,  0},  {u8u16("9"), 160, 0}, {u8u16("0"),  180, 0},   {u8u16("-"), 200, 0},  {u8u16("="), 220, 0},
	{u8u16("ㅂ"), 10, 18}, {u8u16("ㅈ"), 30, 18}, {u8u16("ㄷ"), 50, 18}, {u8u16("ㄱ"), 70, 18}, {u8u16("ㅅ"),  90, 18}, {u8u16("ㅛ"), 110, 18}, {u8u16("ㅕ"), 130, 18}, {u8u16("ㅑ"), 150, 18}, {u8u16("ㅐ"), 170, 18}, {u8u16("ㅔ"), 190, 18},  {u8u16("["), 210, 18}, {u8u16("]"), 230, 18},
	{u8u16("ㅁ"), 20, 36}, {u8u16("ㄴ"), 40, 36}, {u8u16("ㅇ"), 60, 36}, {u8u16("ㄹ"), 80, 36}, {u8u16("ㅎ"), 100, 36}, {u8u16("ㅗ"), 120, 36}, {u8u16("ㅓ"), 140, 36}, {u8u16("ㅏ"), 160, 36}, {u8u16("ㅣ"), 180, 36}, {u8u16(";"),  200, 36}, {u8u16("'"), 220,  36},
	{u8u16("ㅋ"), 30, 54}, {u8u16("ㅌ"), 50, 54}, {u8u16("ㅊ"), 70, 54}, {u8u16("ㅍ"), 90, 54}, {u8u16("ㅠ"), 110, 54}, {u8u16("ㅜ"), 130, 54}, {u8u16("ㅡ"), 150, 54}, {u8u16(","),  170, 54}, {u8u16("."), 190, 54}, {u8u16("/"),  210, 54},
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
		int prevHeight = keyboard.height, prevLayout = loadedLayout;
		loadedLayout = layout;
		if(layout < 3) {
			keyboard = loadImage("nitro:/graphics/keyboardKana.gfx");
		} else {
			keyboard = loadImage("nitro:/graphics/keyboardQWE.gfx");
			xPos = 0;
		}
		if(prevLayout != -1) {
			drawRectangle(0, 192-prevHeight-16, 256, prevHeight+16, BLACK, false);

		}
	}
	drawRectangle(0, 192-keyboard.height, 256, keyboard.height, DARKERER_GRAY, DARKER_GRAY, false);
	drawImage(xPos, 192-keyboard.height, keyboard.width, keyboard.height, keyboard, false);

	drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
	printText(string, 0, 192-keyboard.height-16, false);

	if(layout == 0) {
		for(unsigned i=0;i<(sizeof(keys123)/sizeof(keys123[0]));i++) {
			printTextTinted(keys123[i].character, GRAY_TEXT, xPos+keys123[i].x+16-(getTextWidth(keys123[i].character)/2), 192-keyboard.height+keys123[i].y+8, false);
		}
	} else if(layout == 1) {
		for(unsigned i=0;i<(sizeof(keysABC)/sizeof(keysABC[0]));i++) {
			printTextTinted(keysABC[i].character, GRAY_TEXT, xPos+keysABC[i].x+16-(getTextWidth(keysABC[i].character)/2), 192-keyboard.height+keysABC[i].y+8, false);
		}
		printTextTinted("a/A", GRAY_TEXT, xPos+keysSpecialKana[0].x+16-(getTextWidth("a/A")/2), 192-keyboard.height+keysSpecialKana[0].y+8, false);
	} else if(layout == 2) {
		for(unsigned i=0;i<(sizeof(keysAIU)/sizeof(keysAIU[0]));i++) {
			std::u16string str;
			str += (katakana ? tokatakana(keysAIU[i].character[0]) : keysAIU[i].character[0]);
			printTextTinted(str, GRAY_TEXT, xPos+keysAIU[i].x+16-(getTextWidth(str)/2), 192-keyboard.height+keysAIU[i].y+8, false);
		}
		printTextTinted(katakana ? "ｯﾞﾟ" : "っﾞﾟ", GRAY_TEXT, xPos+keysSpecialKana[0].x+16-(getTextWidth(katakana ? "ｯﾞﾟ" : "っﾞﾟ")/2), 192-keyboard.height+keysSpecialKana[0].y+8, false);
		printTextTinted(katakana ? "あ" : "ア", GRAY_TEXT, xPos+keysSpecialKana[1].x+16-(getTextWidth(katakana ? "あ" : "ア")/2), 192-keyboard.height+keysSpecialKana[1].y+8, false);
	} else if(layout == 3) {
		for(unsigned i=0;i<(sizeof(keysQWE)/sizeof(keysQWE[0]));i++) {
			std::string str;
			str += (caps||shift ? toupper(keysQWE[i].character[0]) : keysQWE[i].character[0]);
			printText(str, xPos+keysQWE[i].x+8-(getTextWidth(str)/2), 192-keyboard.height+keysQWE[i].y, false);
		}
	} else if(layout == 4) {
		for(unsigned i=0;i<(sizeof(keysKor)/sizeof(keysKor[0]));i++) {
			std::u16string str;
			str += (shift ? tossang(keysKor[i].character[0]) : keysKor[i].character[0]);
			printText(str, xPos+keysKor[i].x+8-(getTextWidth(str)/2), 192-keyboard.height+keysKor[i].y, false);
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
		if(Config::getInt("keyboardDirections")) {
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
		} else {
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
		}

		if(direction != -1) {
			bool upper = keysHeld() & KEY_R;
			std::u16string *character;
				if(Config::getInt("keyboardDirections")) character = &(Config::getInt("keyboardGroupAmount") ? keysDPadABC4 : keysDPadABC3)[direction + ((keysHeld() & KEY_L) ? 4 : 0)];
				else	character = (keysHeld() & KEY_L) ? &keysDPadABCSymbols[direction] : (Config::getInt("keyboardGroupAmount") ? &keysDPadABC4[direction] : &keysDPadABC3[direction]);
			std::pair<int, int> *pos = (Config::getInt("keyboardDirections") ? &keysDPad4[direction] : &keysDPad8[direction]);

			fillSpriteImageScaled(keyboardSpriteID, false, 0, 0, 16, 16, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, pos->first, pos->second);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::pair<int, int> offsets[] = {{16, 0}, {24, 8}, {16, 16}, {8, 8}};
			for(unsigned i=0;i<character->size();i++) {
				std::u16string str = character->substr(i, 1);
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

			if(direction != -1 && key != -1 && key < (int)keysDPadABC3[direction].size() && string.size() < maxLength) {
				std::u16string character;
				if(Config::getInt("keyboardDirections")) character = (Config::getInt("keyboardGroupAmount") ? keysDPadABC4 : keysDPadABC3)[direction + ((keysHeld() & KEY_L) ? 4 : 0)];
				else	character = (keysHeld() & KEY_L) ? keysDPadABCSymbols[direction] : (Config::getInt("keyboardGroupAmount") ? keysDPadABC4[direction] : keysDPadABC3[direction]);

				char16_t c = character[key];
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
			bool isKatakana = keysHeld() & KEY_R;
			std::u16string character = (kanaMode == 0) ? keysDPadAIU[direction] : (kanaMode == 1) ? keysDPadAIU2[direction] : keysDPadAIU3[direction];
			std::pair<int, int> *pos = &keysDPad8[direction];

			fillSpriteImageScaled(keyboardSpriteID, false, 0, 0, 16, 16, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, pos->first, pos->second);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::u16string str; str += isKatakana ? tokatakana(character[0]) : character[0];
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

			if(direction != -1 && key != -1 && key < (int)keysDPadAIU[direction].size() && string.size() < maxLength) {
				std::u16string character = (kanaMode == 0) ? keysDPadAIU[direction] : (kanaMode == 1) ? keysDPadAIU2[direction] : keysDPadAIU3[direction];

				char16_t c = character[key];
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
			if((touch.px > keys123[i].x+xPos-2 && touch.px < keys123[i].x+xPos+34) && (touch.py > keys123[i].y+(192-keyboard.height)-2 && touch.py < keys123[i].y+34+(192-keyboard.height))) {
				drawRectangle(keys123[i].x+xPos, keys123[i].y+(192-keyboard.height), 32, 32, (keys123[i].character == " " ? GRAY : DARK_GRAY), false);
				whileHeld();
				string += u8u16(keys123[i].character);
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialKana)/sizeof(keysSpecialKana[0]));i++) {
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboard.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboard.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboard.height-16, false);
					for(int j=0;j<10 && keysHeld() & KEY_TOUCH;j++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboard.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "123") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == "ABC") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == "AIU") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == "QWE") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos+keyboard.width) {
		xPos = 256-keyboard.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchABC(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if an ABC key was pressed
		for(unsigned i=0;i<(sizeof(keysABC)/sizeof(keysABC[0]));i++) {
			if((touch.px > keysABC[i].x+xPos-2 && touch.px < keysABC[i].x+xPos+34) && (touch.py > keysABC[i].y+(192-keyboard.height)-2 && touch.py < keysABC[i].y+34+(192-keyboard.height))) {
				drawRectangle(keysABC[i].x+xPos, keysABC[i].y+(192-keyboard.height), 32, 32, (keysABC[i].character == " " ? GRAY : DARK_GRAY), false);
				char c = '\0';
				while(keysHeld() & KEY_TOUCH) {
					if(touch.px < keysABC[i].x+xPos) {
						c = keysABC[i].character[1];
						break;
					} else if(touch.py < keysABC[i].y+(192-keyboard.height)) {
						c = keysABC[i].character[2];
						break;
					} else if(touch.px > keysABC[i].x+xPos+32) {
						c = keysABC[i].character[3];
						break;
					} else if(touch.py > keysABC[i].y+(192-keyboard.height)+32) {
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
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboard.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboard.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboard.height-16, false);
					for(int j=0;j<10 && keysHeld() & KEY_TOUCH;j++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboard.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "shft") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, DARK_GRAY, false);
				
				char c = string[string.length()-1];
				string = string.substr(0, string.length()-1);
				string += isupper(c) ? tolower(c) : toupper(c);

				whileHeld();
			} else if(keysSpecialKana[i].character == "123") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == "ABC") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == "AIU") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == "QWE") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos+keyboard.width) {
		xPos = 256-keyboard.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchAIU(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a kana key was pressed
		for(unsigned i=0;i<(sizeof(keysAIU)/sizeof(keysAIU[0]));i++) {
			if((touch.px > keysAIU[i].x+xPos-2 && touch.px < keysAIU[i].x+xPos+34) && (touch.py > keysAIU[i].y+(192-keyboard.height)-2 && touch.py < keysAIU[i].y+34+(192-keyboard.height))) {
				drawRectangle(keysAIU[i].x+xPos, keysAIU[i].y+(192-keyboard.height), 32, 32, (keysAIU[i].character == u8u16(" ") ? GRAY : DARK_GRAY), false);
				u16 c = '\0';
				while(keysHeld() & KEY_TOUCH) {
					if(touch.px < keysAIU[i].x+xPos) {
						c = keysAIU[i].character[1];
						break;
					} else if(touch.py < keysAIU[i].y+(192-keyboard.height)) {
						c = keysAIU[i].character[2];
						break;
					} else if(touch.px > keysAIU[i].x+xPos+32) {
						c = keysAIU[i].character[3];
						break;
					} else if(touch.py > keysAIU[i].y+(192-keyboard.height)+32) {
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
		if((touch.px > keysSpecialKana[i].x+xPos-2 && touch.px < keysSpecialKana[i].x+xPos+34) && (touch.py > keysSpecialKana[i].y+(192-keyboard.height)-2 && touch.py < keysSpecialKana[i].y+34+(192-keyboard.height))) {
			if(keysSpecialKana[i].character == "bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
					string = string.substr(0, string.length()-1);
					drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
					printText(string, 0, 192-keyboard.height-16, false);
					for(int j=0;j<10 && keysHeld() & KEY_TOUCH;j++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == "entr") {
				drawRectangle(keysSpecialKana[4].x+xPos, keysSpecialKana[4].y+(192-keyboard.height), 32, 64, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == "shft") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, DARK_GRAY, false);
				
				char16_t c = string[string.length()-1];
				string = string.substr(0, string.length()-1);
				string += nextcharver(c);

				whileHeld();
			} else if(keysSpecialKana[i].character == "hika") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, DARK_GRAY, false);

				katakana = !katakana;
				drawKeyboard(loadedLayout);

				whileHeld();
			} else if(keysSpecialKana[i].character == "123") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == "ABC") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == "AIU") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == "QWE") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, GRAY, false);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos+keyboard.width) {
		xPos = 256-keyboard.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchQWE(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(unsigned i=0;i<(sizeof(keysQWE)/sizeof(keysQWE[0]));i++) {
			if((touch.px > keysQWE[i].x-2 && touch.px < keysQWE[i].x+18) && (touch.py > keysQWE[i].y+(192-keyboard.height)-2 && touch.py < keysQWE[i].y+18+(192-keyboard.height))) {
				drawRectangle(keysQWE[i].x, keysQWE[i].y+(192-keyboard.height), 16, 16, DARK_GRAY, false);
				char c = keysQWE[i].character[0];
				string += (shift || caps ? toupper(c) : c);
				shift = false;
				printText(string, 0, 192-keyboard.height-16, false);
				break;
			}
		}
		// Check if space was pressed
		Key key = {" ", 70, 72};
		if((touch.px > key.x-2 && touch.px < key.x+100) && (touch.py > key.y+(192-keyboard.height)-2 && touch.py < key.y+18+(192-keyboard.height))) {
			drawRectangle(key.x, key.y+(192-keyboard.height), 96, 16, DARK_GRAY, false);
			string += u8u16(key.character);
			shift = false;
			printText(string, 0, 192-keyboard.height-16, false);
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialQWE)/sizeof(keysSpecialQWE[0]));i++) {
		if((touch.px > keysSpecialQWE[i].x-2 && touch.px < keysSpecialQWE[i].x+18) && (touch.py > keysSpecialQWE[i].y+(192-keyboard.height)-2 && touch.py < keysSpecialQWE[i].y+18+(192-keyboard.height))) {
			if(keysSpecialQWE[i].character == "bksp") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, DARK_GRAY, false);
				string = string.substr(0, string.length()-1);
				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string, 0, 192-keyboard.height-16, false);
			} else if(keysSpecialQWE[i].character == "caps") {
				caps = !caps;
				if(caps) drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
				whileHeld();
			} else if(keysSpecialQWE[i].character == "entr") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialQWE[i].character == "lsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 1;
				}
			} else if(keysSpecialQWE[i].character == "rsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 2;
				}
			} else if(keysSpecialQWE[i].character == "mode") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
				whileHeld();
				changeLayout = 4;
			}
			break;
		}
	}
}

void processTouchKor(touchPosition touch, unsigned maxLength) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(unsigned i=0;i<(sizeof(keysKor)/sizeof(keysKor[0]));i++) {
			if((touch.px > keysKor[i].x-2 && touch.px < keysKor[i].x+18) && (touch.py > keysKor[i].y+(192-keyboard.height)-2 && touch.py < keysKor[i].y+18+(192-keyboard.height))) {
				drawRectangle(keysKor[i].x, keysKor[i].y+(192-keyboard.height), 16, 16, DARK_GRAY, false);
				char16_t c = (shift ? tossang(keysKor[i].character[0]) : keysKor[i].character[0]);
				char16_t in = string[string.length()-1];

				int first = -1, second = -1, third = 0;

				if(in >= 0x3131 && in <= 0x3163) {
					first = hangul1.find(in);
				} else if(in >= 0xac00 && in <= 0xd7a3) {
					first = (in-0xac00)/588;
					second = (in-(0xac00+(first*588)))/28;
					third = in-(0xac00+(first*588)+(second*28));
				}

				if(first == -1) {
					string += c;
				} else if(second == -1) {
					if(first == 19 && (hangulW1.find(c) != hangulW1.npos)) { // ㅗ
						string[string.length()-1] += hangulW1.find(c)*28;
					} else if(first == 20 && (hangulW2.find(c) != hangulW2.npos)) { // ㅜ
						string[string.length()-1] += hangulW2.find(c)*28;
					} else if(in >= 0x3131 && in <= 0x314e && c >= 0x314f && c <= 0x3163) {
						string[string.length()-1] = 0xac00 + ((first)*588) + ((c-0x314f)*28);
					} else if((c == 0x3131 && in == 0x3131) || (c == 0x3137 && in == 0x3137)
						   || (c == 0x3142 && in == 0x3142) || (c == 0x3145 && in == 0x3145)
						   || (c == 0x3148 && in == 0x3148)) {
						string[string.length()-1]++;
					} else {
						string += c;
					}
				} else if(third == 0) {
					if(second == 8 && (hangulW1.find(c) != hangulW1.npos)) { // ㅗ
						string[string.length()-1] += hangulW1.find(c)*28;
					} else if(second == 13 && (hangulW2.find(c) != hangulW2.npos)) { // ㅜ
						string[string.length()-1] += hangulW2.find(c)*28;
					} else if(hangul2.find(c) != hangul2.npos) {
						string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + hangul2.find(c);
					} else {
						string += c;
					}
				} else {
					if(c >= 0x314f && c <= 0x3163) {
						switch(third) {
							case 3:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 1;
								string += (0xac00 + (9*588) + ((c-0x314f)*28));
								break;
							case 5:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 4;
								string += (0xac00 + (12*588) + ((c-0x314f)*28));
								break;
							case 6:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 4;
								string += (0xac00 + (18*588) + ((c-0x314f)*28));
								break;
							case 9:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (1*588) + ((c-0x314f)*28));
								break;
							case 10:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (6*588) + ((c-0x314f)*28));
								break;
							case 11:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (7*588) + ((c-0x314f)*28));
								break;
							case 12:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (16*588) + ((c-0x314f)*28));
								break;
							case 13:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (17*588) + ((c-0x314f)*28));
								break;
							case 14:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (18*588) + ((c-0x314f)*28));
								break;
							case 15:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
								string += (0xac00 + (1*588) + ((c-0x314f)*28));
								break;
							case 18:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 17;
								string += (0xac00 + (9*588) + ((c-0x314f)*28));
								break;
							default:
								string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28);
								string += (0xac00 + ((hangul1.find(hangul2[third]))*588) + ((c-0x314f)*28));
								break;
						}
					} else {
						switch(third) {
							case 1:
								if(hangul31.find(c) != hangul31.npos) {
									string[string.length()-1] += hangul31.find(c);
								} else {
									string += c;
								}
								break;
							case 4:
								if(hangul32.find(c) != hangul32.npos) {
									string[string.length()-1] += hangul32.find(c);
								} else {
									string += c;
								}
								break;
							case 8:
								if(hangul33.find(c) != hangul33.npos) {
									string[string.length()-1] += hangul33.find(c);
								} else {
									string += c;
								}
								break;
							case 17:
							case 19:
								if(hangul34.find(c) != hangul34.npos) {
									string[string.length()-1] += hangul34.find(c);
								} else {
									string += c;
								}
								break;
							default:
								string += c;
								break;
						}
					}
				}

				shift = false;
				printText(string, 0, 192-keyboard.height-16, false);
				break;
			}
		}
		// Check if space was pressed
		Key key = {" ", 70, 72};
		if((touch.px > key.x-2 && touch.px < key.x+100) && (touch.py > key.y+(192-keyboard.height)-2 && touch.py < key.y+18+(192-keyboard.height))) {
			drawRectangle(key.x, key.y+(192-keyboard.height), 96, 16, DARK_GRAY, false);
			string += u8u16(key.character);
			shift = false;
			printText(string, 0, 192-keyboard.height-16, false);
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<(sizeof(keysSpecialQWE)/sizeof(keysSpecialQWE[0]));i++) {
		if((touch.px > keysSpecialQWE[i].x-2 && touch.px < keysSpecialQWE[i].x+18) && (touch.py > keysSpecialQWE[i].y+(192-keyboard.height)-2 && touch.py < keysSpecialQWE[i].y+18+(192-keyboard.height))) {
			if(keysSpecialQWE[i].character == "bksp") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, DARK_GRAY, false);

				char16_t in = string[string.length()-1];

				int first = -1, second = -1, third = 0;

				if(in >= 0x3131 && in <= 0x3163) {
					first = hangul1.find(in);
				} else if(in >= 0xac00 && in <= 0xd7a3) {
					first = (in-0xac00)/588;
					second = (in-(0xac00+(first*588)))/28;
					third = in-(0xac00+(first*588)+(second*28));
				}

				if(third != 0) {
					switch(third) {
						case 2:
						case 3:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 1;
							break;
						case 5:
						case 6:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 4;
							break;
						case 9:
						case 10:
						case 11:
						case 12:
						case 13:
						case 14:
						case 15:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 8;
							break;
						case 18:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 17;
							break;
						case 20:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28) + 19;
							break;
						default:
							string[string.length()-1] = 0xac00 + ((first)*588) + ((second)*28);
							break;
					}
				} else if(second != -1) {
					string[string.length()-1] = 0x3131+hangul0.find(0xac00+((first)*588));
				} else if(in == 0x3132 || in == 0x3138 || in == 0x3143 || in == 0x3146 || in == 0x3149) {
					string[string.length()-1]--;
				} else {
					string = string.substr(0, string.length()-1);
				}

				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string, 0, 192-keyboard.height-16, false);
			} else if(keysSpecialQWE[i].character == "caps") {
				caps = !caps;
				if(caps) drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
				whileHeld();
			} else if(keysSpecialQWE[i].character == "entr") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
				whileHeld();
				enter = true;
			} else if(keysSpecialQWE[i].character == "lsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 1;
				}
			} else if(keysSpecialQWE[i].character == "rsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 26, 16, GRAY, false);
					whileHeld();
					shift = 2;
				}
			} else if(keysSpecialQWE[i].character == "mode") {
				drawRectangle(keysSpecialQWE[i].x, keysSpecialQWE[i].y+(192-keyboard.height), 16, 16, GRAY, false);
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
	drawKeyboard(Config::getInt("keyboardLayout"));
	int held, pressed, cursorBlink = 30;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			held = keysDownRepeat();
			pressed = keysDown();
			if(cursorBlink == 30) {
				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string+u8u16("_"), 0, 192-keyboard.height-16, false);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string, 0, 192-keyboard.height-16, false);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!keysHeld());
			if(caps) drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboard.height), 16, 16, GRAY, false);

		if((loadedLayout == 3 ? held : pressed) & KEY_TOUCH) {
			touchRead(&touch);
			if(loadedLayout == 0)			processTouch123(touch, maxLength);
			else if(loadedLayout == 1)	processTouchABC(touch, maxLength);
			else if(loadedLayout == 2)	processTouchAIU(touch, maxLength);
			else if(loadedLayout == 3)	processTouchQWE(touch, maxLength);
			else if(loadedLayout == 4)	processTouchKor(touch, maxLength);
			
			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);
			// Print string
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboard.height-16, false);

			// If caps lock / shift are on, highlight the key
			if(caps)	drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboard.height), 16, 16, GRAY, false);
			if(shift)	drawRectangle(keysSpecialQWE[2+shift].x, keysSpecialQWE[2+shift].y+(192-keyboard.height), 26, 16, GRAY, false);
		} else if(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			if(loadedLayout == 2)	processInputAIU(held, maxLength);
			else	processInputABC(held, maxLength);

			// Hide sprite
			setSpriteVisibility(keyboardSpriteID, false, false);
			updateOam();

			// Print string
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboard.height-16, false);
		} else if(held & KEY_B) {
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboard.height-16, false);
		} else if(held & KEY_Y) {
			string += u8u16(" ");
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboard.height-16, false);
		}
		if(held & KEY_START || enter) {
			Sound::play(Sound::click);
			int prevLayout = Config::getInt("keyboardLayout"), prevXPos = Config::getInt("keyboardXPos");
			Config::setInt("keyboardLayout", loadedLayout);
			if(loadedLayout < 3)	Config::setInt("keyboardXPos", xPos);
			if(prevLayout != loadedLayout || prevXPos != xPos)	Config::save();
			break;
		} else if (pressed & KEY_SELECT) {
			if(loadedLayout < 4)	drawKeyboard(loadedLayout+1);
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
				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string+u8u16("_"), 0, 192-keyboard.height-16, false);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
				printText(string, 0, 192-keyboard.height-16, false);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!held);
			if(caps) drawRectangle(keysSpecialQWE[1].x, keysSpecialQWE[1].y+(192-keyboard.height), 16, 16, GRAY, false);

		if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			processTouch123(touch, maxLength);
			
			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);

			// Print string
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string + (cursorBlink ? u8u16("_") : u8u16("")), 0, 192-keyboard.height-16, false);
		} else if(held & KEY_B) {
			string = string.substr(0, string.length()-1);
			drawRectangle(0, 192-keyboard.height-16, 256, 16, BLACK, false);
			printText(string, 0, 192-keyboard.height-16, false);
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

bool Input::getBool() { return getBool(Lang::get("yes"), Lang::get("no")); }
bool Input::getBool(std::string optionTrue, std::string optionFalse) {
	// Draw rectangles
	drawRectangle(38, 65, 180, 61, DARKER_GRAY, false);
	drawRectangle(48, 75, 70, 41, LIGHT_GRAY, false);
	drawOutline(48, 75, 70, 41, BLACK, false);
	drawRectangle(138, 75, 70, 41, LIGHT_GRAY, false);
	drawOutline(138, 75, 70, 41, BLACK, false);
	drawOutline(38, 65, 180, 61, BLACK, false);

	// Print text
	printTextCenteredTintedMaxW(optionFalse, 60, 1, GRAY_TEXT, -45, 88, false);
	printTextCenteredTintedMaxW(optionTrue, 60, 1, GRAY_TEXT, 45, 88, false);

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
