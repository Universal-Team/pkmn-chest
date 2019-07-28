#include "loading.h"
#include <nds.h>
#include <vector>

#include "graphics.h"

int angle = 0;
int angleChange = 163;
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
	dmaCopyWords(0, logo.data(), logoGfx, 2048);
}

void showLoadingLogo(void) {
	angle = 0;
	oamSet(&oamSub, 100, 112, 80, 0, 15, SpriteSize_32x32, SpriteColorFormat_Bmp, logoGfx, 0, false, false, false, false, false);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, loadingAnimation);
}

void hideLoadingLogo(void) {
	oamClearSprite(&oamSub, 100);
	oamUpdate(&oamSub);
	irqSet(IRQ_VBLANK, NULL);
}