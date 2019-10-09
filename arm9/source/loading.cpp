#include "loading.hpp"
#include <nds.h>
#include <vector>

#include "flashcard.hpp"
#include "colors.hpp"
#include "graphics.hpp"

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
	
	std::vector<u16> logo;
	loadBmp("nitro:/graphics/icon.bmp", logo);
	if(!(rand() % 8192)) { // Full odds shiny chest ;P
		for(unsigned i=0;i<logo.size();i++) {
			if(logo[i] == 0x801F || logo[i] == 0x8C9F || logo[i] == 0x8018)	logo[i] |= RGB::BLUE & GRAY;
		}
	}
	dmaCopyWords(0, logo.data(), logoGfx, 2048);
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