#ifndef COLORS_HPP
#define COLORS_HPP

#include <nds/ndstypes.h>

#define CLEAR			0x0
#define WHITE			0x1
#define LIGHT_GRAY		0x2
#define GRAY			0x3
#define DARKISH_GRAY	0x4
#define DARK_GRAY		0x5
#define DARKER_GRAY		0x6
#define DARKERER_GRAY	0x7
#define BLACK			0x8
#define RED				0x9
#define DARK_RED		0xA
#define BLUE			0xB
#define DARK_BLUE		0xC

// Text
enum TextColor {
	white = 4,
	gray  = 5,
	red   = 6,
	blue  = 7,
};

namespace Colors {
	void load(void);
}

#endif
