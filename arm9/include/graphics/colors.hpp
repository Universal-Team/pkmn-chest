#ifndef COLORS_HPP
#define COLORS_HPP

#define CLEAR			0x0
#define WHITE			0x1
#define DARKISH_GRAY	0x2
#define LIGHT_GRAY		0x3
// FILLER CLEAR			0x4
#define BLACK			0x5
#define GRAY			0x6
#define DARK_GRAY		0x7
// FILLER CLEAR			0x8
#define RED				0x9
#define DARK_RED		0xA
#define DARKER_GRAY		0xB
// FILLER CLEAR			0xC
#define BLUE			0xD
#define DARK_BLUE		0xE
#define DARKERER_GRAY	0xF

// Text
#define WHITE_TEXT 0
#define GRAY_TEXT 1
#define RED_TEXT 2
#define BLUE_TEXT 3

namespace Colors {
	void load(void);
}

#endif
