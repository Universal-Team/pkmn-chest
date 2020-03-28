#include "loading.hpp"
#include <nds.h>
#include <vector>

#include "flashcard.hpp"
#include "colors.hpp"
#include "DateTime.hpp"
#include "graphics.hpp"
#include "tonccpy.h"

int angle = 0;
int angleChange = 190;
u16 *logoGfx;

void loadingAnimation(void) {
	oamRotateScale(&oamSub, 0, angle, (1 << 8), (1<<8));
	angle -= angleChange;
	oamUpdate(&oamSub);
}

void loadLoadingLogo(void) {
	logoGfx = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_Bmp);
	
	Image logo;
	logo = loadImage("/graphics/icon.gfx");

	extern bool useTwlCfg;
	int birthMonth = (useTwlCfg ? *(u8*)0x02000446 : PersonalData->birthMonth);
	int birthDay = (useTwlCfg ? *(u8*)0x02000447 : PersonalData->birthDay);
	Date today = Date::today();
	
	
	if(today.month() == birthMonth && today.day() == birthDay) {
		// On the DS's set birthday color the logo pink 🎂! (Woulda done a cake but that can't be done with just palettes ;P)
		logo.palette[1]  = 0xDEBF;
		logo.palette[3]  = 0xE6FF;
		logo.palette[5]  = 0xE6FF;
		logo.palette[9]  = 0xB9B8;
		logo.palette[10] = 0xB998;
		logo.palette[11] = 0xC1F9;
		logo.palette[12] = 0xCE5B;
		logo.palette[13] = 0xD27B;
		logo.palette[14] = 0xDEFE;
	} else if(today.month() == 3 && today.day() == 31) {
		// Replace the red on the chest with a trans flag on March 31 🏳️‍⚧️
		logo.palette[1] = 0xF2EA; // Light blue
		logo.palette[3] = 0xD65C; // Pink
		logo.palette[5] = 0xF7BD; // White
	}

	if(!(rand() % 8192)) { // Full odds shiny chest ;P
		// Flip red & blue on palettes 1, 3, and 5
		logo.palette[1] = (((logo.palette[1] >> 10) & 0x1F) | (logo.palette[1] & 0x83E0) | ((logo.palette[1] & 0x1F) << 10));
		logo.palette[3] = (((logo.palette[3] >> 10) & 0x1F) | (logo.palette[3] & 0x83E0) | ((logo.palette[3] & 0x1F) << 10));
		logo.palette[5] = (((logo.palette[5] >> 10) & 0x1F) | (logo.palette[5] & 0x83E0) | ((logo.palette[5] & 0x1F) << 10));
	}

	for(unsigned int i=0;i<logo.bitmap.size();i++) {
		logoGfx[i] = logo.palette[logo.bitmap[i]];
	}
}

void showLoadingLogo(void) {
	angle = 0;
	oamSet(&oamSub, 127, 96, 64, 0, 15, SpriteSize_32x32, SpriteColorFormat_Bmp, logoGfx, 0, true, false, false, false, false);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, loadingAnimation);
}

void hideLoadingLogo(void) {
	oamClearSprite(&oamSub, 127);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, NULL);
}