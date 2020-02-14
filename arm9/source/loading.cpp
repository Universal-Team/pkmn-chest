#include "loading.hpp"
#include <nds.h>
#include <vector>

#include "flashcard.hpp"
#include "colors.hpp"
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

	const time_t current = time(NULL);
	
	
	if(gmtime(&current)->tm_mon + 1 == PersonalData->birthMonth && gmtime(&current)->tm_mday == PersonalData->birthDay) {
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
	} else if(gmtime(&current)->tm_mon == 2 && gmtime(&current)->tm_mday == 31) {
		// Replace the red on the chest with a trans flag on March 31 🏳️‍⚧️
		logo.palette[1] = 0xF32D; // Light blue
		logo.palette[3] = 0xDEBC; // Pink
		logo.palette[5] = 0xFBDE; // White
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
	oamSet(&oamSub, 127, 112, 80, 0, 15, SpriteSize_32x32, SpriteColorFormat_Bmp, logoGfx, 0, false, false, false, false, false);
	oamUpdate(&oamSub);
	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(60), loadingAnimation);
}

void hideLoadingLogo(void) {
	oamClearSprite(&oamSub, 127);
	oamUpdate(&oamSub);
	timerStop(0);
}
