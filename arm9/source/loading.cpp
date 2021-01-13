#include "loading.hpp"

#include "DateTime.hpp"
#include "colors.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"

#include <nds.h>
#include <vector>

int angle       = 0;
int angleChange = 190;
Sprite *logoSprite;

void loadingAnimation(void) {
	if(logoSprite) {
		logoSprite->rotation(angle);
		angle -= angleChange;
		logoSprite->update();
	}
}

void loadLoadingLogo(void) {
	logoSprite = new Sprite(false, SpriteSize_32x32, SpriteColorFormat_Bmp, 128 - 16, 96 - 16, 0, 127, 12, 0);

	Image logo("nitro:/graphics/icon.gfx");

	extern bool useTwlCfg;
	int birthMonth = (useTwlCfg ? *(u8 *)0x02000446 : PersonalData->birthMonth);
	int birthDay   = (useTwlCfg ? *(u8 *)0x02000447 : PersonalData->birthDay);
	Date today     = Date::today();

	if(today.month() == birthMonth && today.day() == birthDay) {
		// On the DS's set birthday color the logo pink 🎂! (Woulda done a cake but that can't be done with just palettes
		// ;P)
		logo.palette()[1]  = 0xDEBF;
		logo.palette()[3]  = 0xE6FF;
		logo.palette()[5]  = 0xE6FF;
		logo.palette()[9]  = 0xB9B8;
		logo.palette()[10] = 0xB998;
		logo.palette()[11] = 0xC1F9;
		logo.palette()[12] = 0xCE5B;
		logo.palette()[13] = 0xD27B;
		logo.palette()[14] = 0xDEFE;
	} else if(today.month() == 3 && today.day() == 31) {
		// Replace the red on the chest with a trans flag on March 31 🏳️‍⚧️
		logo.palette()[1] = 0xF2EA; // Light blue
		logo.palette()[3] = 0xD65C; // Pink
		logo.palette()[5] = 0xF7BD; // White
	}

	if(!(rand() % 8192)) { // Full odds shiny chest ;P
		// Flip red & blue on palettes 1, 3, and 5
		logo.palette()[1] =
			(((logo.palette()[1] >> 10) & 0x1F) | (logo.palette()[1] & 0x83E0) | ((logo.palette()[1] & 0x1F) << 10));
		logo.palette()[3] =
			(((logo.palette()[3] >> 10) & 0x1F) | (logo.palette()[3] & 0x83E0) | ((logo.palette()[3] & 0x1F) << 10));
		logo.palette()[5] =
			(((logo.palette()[5] >> 10) & 0x1F) | (logo.palette()[5] & 0x83E0) | ((logo.palette()[5] & 0x1F) << 10));
	}

	logoSprite->drawImage(0, 0, logo);

	irqSet(IRQ_VBLANK, loadingAnimation);
}

void hideLoadingLogo(void) {
	delete logoSprite;
	irqSet(IRQ_VBLANK, NULL);
}
