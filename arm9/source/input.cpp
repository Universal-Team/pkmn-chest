#include "input.hpp"
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <string>

#include "config.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "manager.hpp"
#include "sound.hpp"

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
	return isKatakana ? c + 96 : c;
}
char16_t tossang(char16_t c) {
	if(c == 0x3131 || c == 0x3137 || c == 0x3142 || c == 0x3145 || c == 0x3148) {
		c++;
	} else if(c == 0x3150 || c == 0x3154) {
		c += 2;
	}
	return c;
}

struct Key {
	std::u16string character;
	int x;
	int y;
};

bool caps = false, enter = false, katakana = false;
int changeLayout = -1, loadedLayout = -1, xPos = 0, shift = 0, kanaMode = 0;;
std::u16string string;
Image keyboard;

constexpr std::u16string_view hangul0  = u"가까.나..다따라.......마바빠.사싸아자짜차카타파하";
constexpr std::u16string_view hangul1  = u"ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎㅗㅜ";
constexpr std::u16string_view hangul2  = u".ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ";
constexpr std::u16string_view hangul31 = u".ㄱㅅ";
constexpr std::u16string_view hangul32 = u".ㅈㅎ";
constexpr std::u16string_view hangul33 = u".ㄱㅁㅂㅅㅌㅎ";
constexpr std::u16string_view hangul34 = u".ㅅ";
constexpr std::u16string_view hangulW1 = u".ㅏㅐㅣ";
constexpr std::u16string_view hangulW2 = u".ㅓㅔㅣ";

const std::vector<std::pair<int, int>> keysDPad4 = {
	{112,  60}, //		abcd
	{132,  80}, //	mnop	efgh
	{112, 100}, //		ijkl
	{ 92,  80},
	{112,  60}, //		qrst
	{132,  80}, //	!?)(	uvwx
	{112, 100}, //		yz.,
	{ 92,  80},
};
const std::vector<std::pair<int, int>> keysDPad8 = {
	{112,  40}, //			abc.
	{140,  52}, //		wxyz	def!
	{152,  80}, //	tuv;			ghi,
	{140, 108}, //		pqrs	jkl?
	{112, 120}, //			mno:
	{ 84, 108},
	{ 72,  80},
	{ 84,  52},
};

const std::vector<std::u16string> keysDPadABC3 = {
	u"abc.", //			abc.
	u"def!", //		wxyz	def!
	u"ghi,", //	tuv;			ghi,
	u"jkl?", //		pqrs	jkl?
	u"mno:", //			mno:
	u"pqrs",
	u"tuv;",
	u"wxyz",
};
const std::vector<std::u16string> keysDPadABC4 = {
	u"abcd", //			abcd
	u"efgh", //		!?)(	efgh
	u"ijkl", //	yz.,			ijkl
	u"mnop", //		uvwx	mnop
	u"qrst", //			qrst
	u"uvwx",
	u"yz.,",
	u"!?)(",
};
const std::vector<std::u16string> keysDPadABCSymbols = {
	u"1234", //			1234
	u"5678", //		…•~@	5678
	u"90)(", //	#%+*			90)(
	u"=-_|", //		♂♀/\	|=_-
	u"‘’”“", //			‘’”“
	u"♂♀/\\",
	u"#%*+",
	u"…・~@",
};
const std::vector<std::u16string> keysDPadAIU = {
	u"あいうえお", //	　　　　　　　　　　あいうえお
	u"かきくけこ", //	　　　　　や　ゆ　よ　　　　　かきくけこ
	u"さしすせそ", //	まみむめも　　　　　　　　　　　　　　　さしすせそ
	u"たちつてと", //	　　　　　はひふへほ　　　　　たちつてと
	u"なにぬねの", //	　　　　　　　　　　なにぬねの
	u"はひふへほ",
	u"まみむめも",
	u"や ゆ よ",
};
const std::vector<std::u16string> keysDPadAIU2 = {
	u"ぁぃぅぇぉ", //	　　　　　　　　　　ぁぃぅぇぉ
	u"がぎぐげご", //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	u"ざじずぜぞ", //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	u"だぢっでど", //	　　　　　ばびぶべぼ　　　　　だぢっでど
	u"らりるれろ", //	　　　　　　　　　　らりるれろ
	u"ばびぶべぼ",
	u"わをんー",
	u"ゃ ゅ ょ",
};
const std::vector<std::u16string> keysDPadAIU3 = {
	u"ぁぃゔぇぉ", //	　　　　　　　　　　ぁぃゔぇぉ
	u"がぎぐげご", //	　　　　　ゃ　ゅ　ょ　　　　　がぎぐげご
	u"ざじずぜぞ", //	わをんー　　　　　　　　　　　　　　　　ざじずぜぞ
	u"だぢづでど", //	　　　　　ぱぴぷぺぽ　　　　　だぢづでど
	u"らりるれろ", //	　　　　　　　　　　らりるれろ
	u"ぱぴぷぺぽ",
	u"わをんー",
	u"ゃ ゅ ょ",
};

const std::vector<Key> keys123 = {
	{u"1", 34,  0},	{u"2", 68,   0},	{u"3", 102,  0},
	{u"4", 34, 34},	{u"5", 68,  34},	{u"6", 102, 34},
	{u"7", 34, 68},	{u"8", 68,  68},	{u"9", 102, 68},
					{u"0", 68, 102},
};
const std::vector<Key> keysABC = {
	{u"@#/&", 34,  0},	{u"abc",   68,   0},	{u"def",  102,   0},
	{u"ghi",  34, 34},	{u"jkl",   68,  34},	{u"mno",  102,  34},	{u" ", 136, 34},
	{u"pqrs", 34, 68},	{u"tuv",   68,  68},	{u"wxyz", 102,  68},
						{u"'\"()", 68, 102},	{u".,?!", 102, 102},
};
const std::vector<Key> keysAIU = {
	{u"あいうえお", 34,  0},	{u"かきくけこ", 68,   0},	{u"さしすせそ", 102,   0},
	{u"たちつてと", 34, 34},	{u"なにぬねの", 68,  34},	{u"はひふへほ", 102,  34},	{u" ", 136, 34},
	{u"まみむめも", 34, 68},	{u"や　ゆ　よ", 68,  68},	{u"らりるれろ", 102,  68},
							  {u"わをんー",   68, 102},
};
const std::vector<Key> keysSpecialKana = {
	{u"shft",  34, 102},
	{u"hika", 102, 102},
	{u"bksp", 136,   0},
	{u"spce", 136,  34},
	{u"entr", 136,  68},
	{u"entr", 136, 102},
	{u"123",    0,   0},
	{u"ABC",    0,  34},
	{u"AIU",    0,  68},
	{u"QWE",    0, 102},
};

const std::vector<Key> keysQWERTY = {
	{u"1",  0,  0}, {u"2", 20,  0}, {u"3", 40,  0}, {u"4", 60,  0}, {u"5",  80,  0}, {u"6", 100,  0}, {u"7", 120,  0}, {u"8", 140,  0}, {u"9", 160,  0}, {u"0", 180,  0}, {u"-", 200,  0}, {u"=", 220,  0},
	{u"q", 10, 18}, {u"w", 30, 18}, {u"e", 50, 18}, {u"r", 70, 18}, {u"t",  90, 18}, {u"y", 110, 18}, {u"u", 130, 18}, {u"i", 150, 18}, {u"o", 170, 18}, {u"p", 190, 18}, {u"[", 210, 18}, {u"]", 230, 18},
	{u"a", 20, 36}, {u"s", 40, 36}, {u"d", 60, 36}, {u"f", 80, 36}, {u"g", 100, 36}, {u"h", 120, 36}, {u"j", 140, 36}, {u"k", 160, 36}, {u"l", 180, 36}, {u";", 200, 36}, {u"'", 220, 36},
	{u"z", 30, 54}, {u"x", 50, 54}, {u"c", 70, 54}, {u"v", 90, 54}, {u"b", 110, 54}, {u"n", 130, 54}, {u"m", 150, 54}, {u",", 170, 54}, {u".", 190, 54}, {u"/", 210, 54},
};
const std::vector<Key> keysKor = {
	{u"1",  0,  0},  {u"2", 20,  0}, {u"3", 40,  0}, {u"4",  60, 0},  {u"5",  80,  0}, {u"6",  100, 0},  {u"7", 120,  0}, {u"8", 140,  0},  {u"9", 160, 0}, {u"0",  180, 0},   {u"-", 200, 0},  {u"=", 220, 0},
	{u"ㅂ", 10, 18}, {u"ㅈ", 30, 18}, {u"ㄷ", 50, 18}, {u"ㄱ", 70, 18}, {u"ㅅ",  90, 18}, {u"ㅛ", 110, 18}, {u"ㅕ", 130, 18}, {u"ㅑ", 150, 18}, {u"ㅐ", 170, 18}, {u"ㅔ", 190, 18},  {u"[", 210, 18}, {u"]", 230, 18},
	{u"ㅁ", 20, 36}, {u"ㄴ", 40, 36}, {u"ㅇ", 60, 36}, {u"ㄹ", 80, 36}, {u"ㅎ", 100, 36}, {u"ㅗ", 120, 36}, {u"ㅓ", 140, 36}, {u"ㅏ", 160, 36}, {u"ㅣ", 180, 36}, {u";",  200, 36}, {u"'", 220,  36},
	{u"ㅋ", 30, 54}, {u"ㅌ", 50, 54}, {u"ㅊ", 70, 54}, {u"ㅍ", 90, 54}, {u"ㅠ", 110, 54}, {u"ㅜ", 130, 54}, {u"ㅡ", 150, 54}, {u",",  170, 54}, {u".", 190, 54}, {u"/",  210, 54},
};
const std::vector<Key> keysSpecialQWERTY = {
	{u"bksp", 240, 0},	// Backspace
	{u"caps", 0, 36},	// Caps Lock
	{u"entr", 240, 36},	// Enter
	{u"lsft", 0, 54},	// Left Shift
	{u"rsft", 230, 54},	// Right Shift
	{u"mode", 50, 72},	// Change mode
};

void clearVars(void) {
	string = u"", caps = false, shift = false, enter = false, katakana = false, changeLayout = -1, kanaMode = 0;
}

void whileHeld(void) {
	while(keysHeld()) {
		swiWaitForVBlank();
		scanKeys();
	}
}

bool touching(const touchPosition &touch, int x, int y, int w, int h) {
	return (touch.px >= x + std::min(w, 0) && touch.px <= x + std::max(w, 0) && touch.py >= y + std::min(h, 0) && touch.py <= y + std::max(h, 0));
}

void backspace(void) {
	char16_t in = string[string.length() - 1];

	int first = -1, second = -1, third = 0;

	if(in >= 0x3131 && in <= 0x3163) {
		first = hangul1.find(in);
	} else if(in >= 0xac00 && in <= 0xd7a3) {
		first = (in - 0xac00) / 588;
		second = (in - (0xac00 + (first * 588))) / 28;
		third = in - (0xac00 + (first * 588) + (second * 28));
	}

	if(third != 0) {
		switch(third) {
			case 2:
			case 3:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 1;
				break;
			case 5:
			case 6:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 4;
				break;
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
				break;
			case 18:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 17;
				break;
			case 20:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 19;
				break;
			default:
				string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28);
				break;
		}
	} else if(second != -1) {
		string[string.length() - 1] = 0x3131 + hangul0.find(0xac00 + ((first) * 588));
	} else if(in == 0x3132 || in == 0x3138 || in == 0x3143 || in == 0x3146 || in == 0x3149) {
		string[string.length() - 1]--;
	} else {
		string = string.substr(0, string.length() - 1);
	}
}

void addToString(char16_t c) {
	c = (shift ? tossang(c) : c);
	c = (shift | caps ? toupper(c) : c);
	c = (katakana ? tokatakana(c) : c);
	char16_t in = string[string.length() - 1];

	int first = -1, second = -1, third = 0;

	if(in >= 0x3131 && in <= 0x3163) {
		first = hangul1.find(in);
	} else if(in >= 0xac00 && in <= 0xd7a3) {
		first = (in - 0xac00) / 588;
		second = (in - (0xac00 + (first * 588))) / 28;
		third = in - (0xac00 + (first * 588) + (second * 28));
	}

	if(first == -1) {
		string += c;
	} else if(second == -1) {
		if(first == 19 && (hangulW1.find(c) != hangulW1.npos)) { // ㅗ
			string[string.length() - 1] += hangulW1.find(c) * 28;
		} else if(first == 20 && (hangulW2.find(c) != hangulW2.npos)) { // ㅜ
			string[string.length() - 1] += hangulW2.find(c) * 28;
		} else if(in >= 0x3131 && in <= 0x314e && c >= 0x314f && c <= 0x3163) {
			string[string.length() - 1] = 0xac00 + ((first) * 588) + ((c - 0x314f) * 28);
		} else if((c == 0x3131 && in == 0x3131) || (c == 0x3137 && in == 0x3137)
				|| (c == 0x3142 && in == 0x3142) || (c == 0x3145 && in == 0x3145)
				|| (c == 0x3148 && in == 0x3148)) {
			string[string.length() - 1]++;
		} else {
			string += c;
		}
	} else if(third == 0) {
		if(second == 8 && (hangulW1.find(c) != hangulW1.npos)) { // ㅗ
			string[string.length() - 1] += hangulW1.find(c) * 28;
		} else if(second == 13 && (hangulW2.find(c) != hangulW2.npos)) { // ㅜ
			string[string.length() - 1] += hangulW2.find(c) * 28;
		} else if(hangul2.find(c) != hangul2.npos) {
			string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + hangul2.find(c);
		} else {
			string += c;
		}
	} else {
		if(c >= 0x314f && c <= 0x3163) {
			switch(third) {
				case 3:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 1;
					string += (0xac00 + (9 * 588) + ((c - 0x314f) * 28));
					break;
				case 5:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 4;
					string += (0xac00 + (12 * 588) + ((c - 0x314f) * 28));
					break;
				case 6:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 4;
					string += (0xac00 + (18 * 588) + ((c - 0x314f) * 28));
					break;
				case 9:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (1 * 588) + ((c - 0x314f) * 28));
					break;
				case 10:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (6 * 588) + ((c - 0x314f) * 28));
					break;
				case 11:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (7 * 588) + ((c - 0x314f) * 28));
					break;
				case 12:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (16 * 588) + ((c - 0x314f) * 28));
					break;
				case 13:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (17 * 588) + ((c - 0x314f) * 28));
					break;
				case 14:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (18 * 588) + ((c - 0x314f) * 28));
					break;
				case 15:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 8;
					string += (0xac00 + (1 * 588) + ((c - 0x314f) * 28));
					break;
				case 18:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28) + 17;
					string += (0xac00 + (9 * 588) + ((c - 0x314f) * 28));
					break;
				default:
					string[string.length() - 1] = 0xac00 + ((first) * 588) + ((second) * 28);
					string += (0xac00 + ((hangul1.find(hangul2[third])) * 588) + ((c - 0x314f) * 28));
					break;
			}
		} else {
			switch(third) {
				case 1:
					if(hangul31.find(c) != hangul31.npos) {
						string[string.length() - 1] += hangul31.find(c);
					} else {
						string += c;
					}
					break;
				case 4:
					if(hangul32.find(c) != hangul32.npos) {
						string[string.length() - 1] += hangul32.find(c);
					} else {
						string += c;
					}
					break;
				case 8:
					if(hangul33.find(c) != hangul33.npos) {
						string[string.length() - 1] += hangul33.find(c);
					} else {
						string += c;
					}
					break;
				case 17:
				case 19:
					if(hangul34.find(c) != hangul34.npos) {
						string[string.length() - 1] += hangul34.find(c);
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
}

void drawKeyboard(int layout) {
	if(loadedLayout != layout) {
		int prevHeight = keyboard.height, prevLayout = loadedLayout;
		loadedLayout = layout;
		if(layout < 3) {
			keyboard = loadImage("/graphics/keyboardKana.gfx");
		} else {
			keyboard = loadImage("/graphics/keyboardQWERTY.gfx");
			xPos = 0;
		}
		if(prevLayout != -1) {
			drawRectangle(0, 192 - prevHeight - 16, 256, prevHeight + 16, 0, false, true);
		}
	}
	drawRectangle(0, 192 - keyboard.height, 256, keyboard.height, DARKERER_GRAY, DARKER_GRAY, false, true);
	drawImage(xPos, 192 - keyboard.height, keyboard, false, true);

	drawRectangle(0, 192 - keyboard.height - 16, 256, 16, DARKERER_GRAY, false, true);
	printText(string, 0, 192 - keyboard.height - 16, false, true);

	if(layout == 0) {
		for(unsigned i=0;i<keys123.size();i++) {
			printTextTinted(keys123[i].character, TextColor::white, xPos + keys123[i].x + 16 - (getTextWidth(keys123[i].character) / 2), 192 - keyboard.height + keys123[i].y + 8, false, true);
		}
	} else if(layout == 1) {
		for(unsigned i=0;i<keysABC.size();i++) {
			printTextTinted(keysABC[i].character, TextColor::white, xPos + keysABC[i].x + 16 - (getTextWidth(keysABC[i].character) / 2), 192 - keyboard.height + keysABC[i].y + 8, false, true);
		}
		printTextTinted("a/A", TextColor::white, xPos + keysSpecialKana[0].x + 16 - (getTextWidth("a/A") / 2), 192 - keyboard.height + keysSpecialKana[0].y + 8, false, true);
	} else if(layout == 2) {
		for(unsigned i=0;i<keysAIU.size();i++) {
			std::u16string str;
			str += (katakana ? tokatakana(keysAIU[i].character[0]) : keysAIU[i].character[0]);
			printTextTinted(str, TextColor::white, xPos + keysAIU[i].x + 16 - (getTextWidth(str) / 2), 192 - keyboard.height + keysAIU[i].y + 8, false, true);
		}
		printTextTinted(katakana ? "ｯﾞﾟ" : "っﾞﾟ", TextColor::white, xPos + keysSpecialKana[0].x + 16 - (getTextWidth(katakana ? "ｯﾞﾟ" : "っﾞﾟ") / 2), 192 - keyboard.height + keysSpecialKana[0].y + 8, false, true);
		printTextTinted(katakana ? "あ" : "ア", TextColor::white, xPos + keysSpecialKana[1].x + 16 - (getTextWidth(katakana ? "あ" : "ア") / 2), 192 - keyboard.height + keysSpecialKana[1].y + 8, false, true);
	} else if(layout == 3) {
		for(unsigned i=0;i<keysQWERTY.size();i++) {
			std::string str;
			str += (caps||shift ? toupper(keysQWERTY[i].character[0]) : keysQWERTY[i].character[0]);
			printText(str, xPos + keysQWERTY[i].x + 8 - (getTextWidth(str) / 2), 192 - keyboard.height + keysQWERTY[i].y, false, true);
		}
	} else if(layout == 4) {
		for(unsigned i=0;i<keysKor.size();i++) {
			std::u16string str;
			str += (shift ? tossang(keysKor[i].character[0]) : keysKor[i].character[0]);
			printText(str, xPos + keysKor[i].x + 8 - (getTextWidth(str) / 2), 192 - keyboard.height + keysKor[i].y, false, true);
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
			const std::u16string *character;
				if(Config::getInt("keyboardDirections")) character = &(Config::getInt("keyboardGroupAmount") ? keysDPadABC4 : keysDPadABC3)[direction + ((keysHeld() & KEY_L) ? 4 : 0)];
				else	character = (keysHeld() & KEY_L) ? &keysDPadABCSymbols[direction] : (Config::getInt("keyboardGroupAmount") ? &keysDPadABC4[direction] : &keysDPadABC3[direction]);
			const std::pair<int, int> *pos = (Config::getInt("keyboardDirections") ? &keysDPad4[direction] : &keysDPad8[direction]);

			fillSpriteImageScaled(keyboardSpriteID, false, 32, 0, 0, 2, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, pos->first, pos->second);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::pair<int, int> offsets[] = {{16, 0}, {24, 8}, {16, 16}, {8, 8}};
			for(unsigned i=0;i<character->size();i++) {
				std::u16string str = character->substr(i, 1);
				if(upper)	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
				fillSpriteText(keyboardSpriteID, false, str, TextColor::white, offsets[i].first-(getTextWidth(str) / 2), offsets[i].second);
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
				addToString((keysHeld() & KEY_R) ? toupper(c) : c);
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
			const std::pair<int, int> *pos = &keysDPad8[direction];

			fillSpriteImageScaled(keyboardSpriteID, false, 32, 0, 0, 2, 2, keyboardKey);
			setSpritePosition(keyboardSpriteID, false, pos->first, pos->second);
			setSpriteVisibility(keyboardSpriteID, false, true);
			updateOam();

			std::u16string str; str += isKatakana ? tokatakana(character[0]) : character[0];
			fillSpriteText(keyboardSpriteID, false, str, TextColor::white, 16-(getTextWidth(str) / 2), 8);
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
				addToString((keysHeld() & KEY_R) ? tokatakana(c) : c);
				break;
			}
		}
	}
}

void processTouchKana(touchPosition touch, unsigned maxLength, const std::vector<Key> &keys) {
	if(string.length() < maxLength) {
		// Check if a letter key was pressed
		for(unsigned i=0;i<keys.size();i++) {
			if(touching(touch, keys[i].x + xPos-2, keys[i].y + (192 - keyboard.height) - 2, 36, 36)) {
				drawRectangle(keys[i].x + xPos, keys[i].y + (192 - keyboard.height), 32, 32, (keys[i].character == u" " ? GRAY : DARK_GRAY), false, true);
				int selection = 0, prevSelection = -1, xOfs = 0, yOfs = 0;
				while(keysHeld() & KEY_TOUCH) {
					if(touching(touch, keys[i].x + xPos, keys[i].y + (192 - keyboard.height), 32, 32)) {
						selection = 0;
						xOfs = 0;
						yOfs = 0;
					} else if(touching(touch, keys[i].x + xPos, keys[i].y + (192 - keyboard.height), -256, 32)) {
						selection = 1;
						xOfs = -32;
						yOfs = 0;
					} else if(touching(touch, keys[i].x + xPos, keys[i].y + (192 - keyboard.height), 32, -256)) {
						selection = 2;
						xOfs = 0;
						yOfs = -32;
					} else if(touching(touch, keys[i].x + xPos + 32, keys[i].y + (192 - keyboard.height), 256, 32)) {
						selection = 3;
						xOfs = 32;
						yOfs = 0;
					} else if(touching(touch, keys[i].x + xPos, keys[i].y + (192 - keyboard.height) + 32, 32, 256)) {
						selection = 4;
						xOfs = 0;
						yOfs = 32;
					}

					if(selection != prevSelection) {
						prevSelection = selection;
						if(selection < (int)keys[i].character.length()) {
							setSpriteVisibility(keyboardSpriteID, false, true);
							setSpritePosition(keyboardSpriteID, false, keys[i].x + xPos + xOfs, keys[i].y + (192 - keyboard.height) + yOfs);
							fillSpriteImageScaled(keyboardSpriteID, false, 32, 0, 0, 2, 2, keyboardKey);
							fillSpriteText(keyboardSpriteID, false, keys[i].character.substr(selection, 1), TextColor::white, 16-(getTextWidth(keys[i].character.substr(selection, 1)) / 2), 8);
						} else {
							setSpriteVisibility(keyboardSpriteID, false, false);
						}
						updateOam();
					}

					swiWaitForVBlank();
					scanKeys();
					touchRead(&touch);
				}
				setSpriteVisibility(keyboardSpriteID, false, false);
				updateOam();

				if(selection < (int)keys[i].character.length()) {
					addToString(keys[i].character[selection]);
				}
				return;
			}
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<keysSpecialKana.size();i++) {
		if(touching(touch, keysSpecialKana[i].x + xPos - 2, keysSpecialKana[i].y + (192 - keyboard.height) - 2, 36, 36)) {
			if(keysSpecialKana[i].character == u"bksp") {
				while(keysHeld() & KEY_TOUCH) {
					drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, GRAY, false, true);
					backspace();
					drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
					printText(string, 0, 192 - keyboard.height-16, false, true);
					for(int j=0;j<10 && keysHeld() & KEY_TOUCH;j++) {
						swiWaitForVBlank();
						scanKeys();
					}
				}
			} else if(keysSpecialKana[i].character == u"entr") {
				drawRectangle(keysSpecialKana[4].x + xPos, keysSpecialKana[4].y + (192 - keyboard.height), 32, 64, GRAY, false, true);
				whileHeld();
				enter = true;
			} else if(keysSpecialKana[i].character == u"shft") {
				if(&keys != &keys123) {
					drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, DARK_GRAY, false, true);

					char16_t c = string[string.length() - 1];
					if(&keys == &keysABC) {
						string[string.length() - 1] = isupper(c) ? tolower(c) : toupper(c);
					} else if(&keys == &keysAIU) {
						string[string.length() - 1] = nextcharver(c);
					}
					whileHeld();
				}
			} else if(keysSpecialKana[i].character == u"hika") {
				drawRectangle(keysSpecialKana[i].x+xPos, keysSpecialKana[i].y+(192-keyboard.height), 32, 32, DARK_GRAY, false, true);

				katakana = !katakana;
				drawKeyboard(loadedLayout);

				whileHeld();
			} else if(keysSpecialKana[i].character == u"123") {
				drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, GRAY, false, true);
				whileHeld();
				changeLayout = 0;
			} else if(keysSpecialKana[i].character == u"ABC") {
				drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, GRAY, false, true);
				whileHeld();
				changeLayout = 1;
			} else if(keysSpecialKana[i].character == u"AIU") {
				drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, GRAY, false, true);
				whileHeld();
				changeLayout = 2;
			} else if(keysSpecialKana[i].character == u"QWE") {
				drawRectangle(keysSpecialKana[i].x + xPos, keysSpecialKana[i].y + (192 - keyboard.height), 32, 32, GRAY, false, true);
				whileHeld();
				changeLayout = 3;
			}
			return;
		}
	}
	if(touch.px > xPos + keyboard.width) {
		xPos = 256 - keyboard.width;
	} else if(touch.px < xPos) {
		xPos = 0;
	}
}

void processTouchQWERTY(touchPosition touch, unsigned maxLength, const std::vector<Key> &keys) {
	if(string.length() < maxLength) {
		// Check if a regular key was pressed
		for(unsigned i=0;i<keys.size();i++) {
			if(touching(touch, keys[i].x-2, keys[i].y + (192 - keyboard.height) - 1, 20, 18)) {
				drawRectangle(keys[i].x, keys[i].y + (192 - keyboard.height), 16, 16, DARK_GRAY, false, true);
				addToString(keys[i].character[0]);
				shift = false;
				printText(string, 0, 192 - keyboard.height - 16, false, true);
				break;
			}
		}
		// Check if space was pressed
		Key space = {u" ", 70, 72};
		if(touching(touch, space.x-2, space.y + (192 - keyboard.height) - 1, 100, 18)) {
			drawRectangle(space.x, space.y + (192 - keyboard.height), 96, 16, DARK_GRAY, false, true);
			addToString(space.character[0]);
			shift = false;
			printText(string, 0, 192 - keyboard.height - 16, false, true);
		}
	}
	// Check if a special key was pressed
	for(unsigned i=0;i<keysSpecialQWERTY.size();i++) {
		if(touching(touch, keysSpecialQWERTY[i].x-2, keysSpecialQWERTY[i].y + (192 - keyboard.height) - 1, 20, 18)) {
			if(keysSpecialQWERTY[i].character == u"bksp") {
				drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 16, 16, DARK_GRAY, false, true);
				backspace();
				drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
				printText(string, 0, 192 - keyboard.height-16, false, true);
			} else if(keysSpecialQWERTY[i].character == u"caps") {
				caps = !caps;
				if(caps) drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 16, 16, GRAY, false, true);
				whileHeld();
			} else if(keysSpecialQWERTY[i].character == u"entr") {
				drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 16, 16, GRAY, false, true);
				whileHeld();
				enter = true;
			} else if(keysSpecialQWERTY[i].character == u"lsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 26, 16, GRAY, false, true);
					whileHeld();
					shift = 1;
				}
			} else if(keysSpecialQWERTY[i].character == u"rsft") {
				if(shift)	shift = 0;
				else {
					drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 26, 16, GRAY, false, true);
					whileHeld();
					shift = 2;
				}
			} else if(keysSpecialQWERTY[i].character == u"mode") {
				drawRectangle(keysSpecialQWERTY[i].x, keysSpecialQWERTY[i].y + (192 - keyboard.height), 16, 16, GRAY, false, true);
				whileHeld();
				changeLayout = loadedLayout + 1;
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
				drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
				printText(string + u"_", 0, 192 - keyboard.height - 16, false, true);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
				printText(string, 0, 192 - keyboard.height-16, false, true);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!keysHeld());
			if(caps) drawRectangle(keysSpecialQWERTY[1].x, keysSpecialQWERTY[1].y + (192 - keyboard.height), 16, 16, GRAY, false, true);

		if((loadedLayout >= 3 ? held : pressed) & KEY_TOUCH) {
			touchRead(&touch);
			if(loadedLayout == 0)		processTouchKana(touch, maxLength, keys123);
			else if(loadedLayout == 1)	processTouchKana(touch, maxLength, keysABC);
			else if(loadedLayout == 2)	processTouchKana(touch, maxLength, keysAIU);
			else if(loadedLayout == 3)	processTouchQWERTY(touch, maxLength, keysQWERTY);
			else if(loadedLayout == 4)	processTouchQWERTY(touch, maxLength, keysKor);

			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);
			// Print string
			drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
			printText(string + (cursorBlink ? u"_" : u""), 0, 192 - keyboard.height-16, false, true);

			// If caps lock / shift are on, highlight the key
			if(caps)	drawRectangle(keysSpecialQWERTY[1].x, keysSpecialQWERTY[1].y + (192 - keyboard.height), 16, 16, GRAY, false, true);
			if(shift)	drawRectangle(keysSpecialQWERTY[2 + shift].x, keysSpecialQWERTY[2 + shift].y + (192 - keyboard.height), 26, 16, GRAY, false, true);
		} else if(keysHeld() & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
			if(loadedLayout == 2)	processInputAIU(held, maxLength);
			else	processInputABC(held, maxLength);

			// Hide sprite
			setSpriteVisibility(keyboardSpriteID, false, false);
			updateOam();

			// Print string
			drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
			printText(string + (cursorBlink ? u"_" : u""), 0, 192 - keyboard.height-16, false, true);
		} else if(held & KEY_B) {
			backspace();
			drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
			printText(string, 0, 192 - keyboard.height-16, false, true);
		} else if(held & KEY_Y) {
			addToString(u' ');
			drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
			printText(string, 0, 192 - keyboard.height-16, false, true);
		}
		if(held & KEY_START || enter) {
			Sound::play(Sound::click);
			int prevLayout = Config::getInt("keyboardLayout"), prevXPos = Config::getInt("keyboardXPos");
			Config::setInt("keyboardLayout", loadedLayout);
			if(loadedLayout < 3)	Config::setInt("keyboardXPos", xPos);
			if(prevLayout != loadedLayout || prevXPos != xPos)	Config::save();
			break;
		} else if (pressed & KEY_SELECT) {
			if(loadedLayout < 4)	drawKeyboard(loadedLayout + 1);
			else	drawKeyboard(0);
		} else if(changeLayout != -1) {
			if(changeLayout > 4)	changeLayout = 0;
			drawKeyboard(changeLayout);
			changeLayout = -1;
		}
	}
	drawRectangle(0, 192 - keyboard.height-16, 256, keyboard.height + 16, CLEAR, false, true);
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
				drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
				printText(string + u"_", 0, 192 - keyboard.height-16, false, true);
			} else if(cursorBlink == 0) {
				drawRectangle(0, 192 - keyboard.height-16, 256, 16, DARKERER_GRAY, false, true);
				printText(string, 0, 192 - keyboard.height-16, false, true);
			} else if(cursorBlink == -30) {
				cursorBlink = 31;
			}
			cursorBlink--;
		} while(!held);

		if(caps) drawRectangle(keysSpecialQWERTY[1].x, keysSpecialQWERTY[1].y + (192 - keyboard.height), 16, 16, GRAY, false, true);

		if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			processTouchKana(touch, maxLength, keys123);

			// Redraw keyboard to cover up highlight
			drawKeyboard(loadedLayout);

			// Print string
			drawRectangle(0, 192 - keyboard.height - 16, 256, 16, DARKERER_GRAY, false, true);
			printText(string + (cursorBlink ? u"_" : u""), 0, 192 - keyboard.height - 16, false, true);
		} else if(held & KEY_B) {
			backspace();
			drawRectangle(0, 192 - keyboard.height - 16, 256, 16, DARKERER_GRAY, false, true);
			printText(string, 0, 192 - keyboard.height - 16, false, true);
		}
		if(held & KEY_START || enter) {
			Sound::play(Sound::click);
			loadedLayout = -1; // So it doesn't draw the extra rectangle if drawing QWERTY next
			break;
		}
	}
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	if(string == u"") return -1;
	unsigned i = std::stoi(StringUtils::UTF16toUTF8(string));
	if(i > max)	return max;
	return i;
}

bool Input::getBool() { return getBool(i18n::localize(Config::getLang("lang"), "yes"), i18n::localize(Config::getLang("lang"), "no")); }
bool Input::getBool(std::string optionTrue, std::string optionFalse) {
	// Draw rectangles
	drawRectangle(38, 65, 180, 61, DARKER_GRAY, false, true);
	drawOutline(38, 65, 180, 61, BLACK, false, true);

	drawRectangle(48, 75, 70, 41, LIGHT_GRAY, false, true);
	drawOutline(48, 75, 70, 41, BLACK, false, true);

	drawRectangle(138, 75, 70, 41, LIGHT_GRAY, false, true);
	drawOutline(138, 75, 70, 41, BLACK, false, true);

	// Print text
	printTextCenteredTintedMaxW(optionFalse, 60, 1, TextColor::gray, -45, 88, false, true);
	printTextCenteredTintedMaxW(optionTrue, 60, 1, TextColor::gray, 45, 88, false, true);

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
			if(touching(touch, 48, 76, 70, 40)) {
				goto no;
			} else if(touching(touch, 138, 76,70, 40)) {
				goto yes;
			}
		} else if(pressed & KEY_A) {
			yes:
			Sound::play(Sound::click);
			drawRectangle(38, 65, 180, 61, CLEAR, false, true);
			return true;
		} else if(pressed & KEY_B) {
			no:
			Sound::play(Sound::back);
			drawRectangle(38, 65, 180, 61, CLEAR, false, true);
			return false;
		}
	}
}
