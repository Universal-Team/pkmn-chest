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
	logo = loadImage("nitro:/graphics/icon.gfx");
	if(!(rand() % 8192)) { // Full odds shiny chest ;P
		for(unsigned i=0;i<logo.palette.size();i++) {
			if(logo.palette[i] == 0x801F || logo.palette[i] == 0x8C9F || logo.palette[i] == 0x8018)	logo.palette[i] |= RGB::BLUE & GRAY;
		}
	}
	for(int i=0;i<32;i++) {
		for(int j=0;j<32;j++) {
			logoGfx[(i*32)+j] = logo.palette[logo.bitmap[((i)*32)+j]] | BIT(15);
		}
	}
}

void showLoadingLogo(void) {
	angle = 0;
	oamSet(&oamSub, 127, 112, 80, 0, 15, SpriteSize_32x32, SpriteColorFormat_Bmp, logoGfx, 0, false, false, false, false, false);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, loadingAnimation);
}

void hideLoadingLogo(void) {
	oamClearSprite(&oamSub, 127);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, NULL);
}