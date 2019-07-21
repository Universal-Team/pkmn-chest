#include <fat.h>

#include "banks.hpp"
#include "colors.h"
#include "config.h"
#include "fileBrowse.h"
#include "flashcard.h"
#include "graphics.h"
#include "lang.h"
#include "langStrings.h"
#include "loader.h"
#include "manager.h"
#include "nitrofs.h"
#include "sound.h"

int angle = 0;
int angleChange = 170;
u16* logoGfx;

void loadLogo(void) {
	swiWaitForVBlank();
	logoGfx = oamAllocateGfx(&oamSub, SpriteSize_32x32, SpriteColorFormat_Bmp);
	std::vector<u16> logo;
	loadBmp("nitro:/graphics/icon.bmp", logo);
	dmaCopyWords(0, logo.data(), logoGfx, 2048);

	oamSet(&oamSub, 100, 112, 80, 0, 15, SpriteSize_32x32, SpriteColorFormat_Bmp, logoGfx, 0, false, false, false, false, false);
	oamUpdate(&oamSub);
}

void loadingAnimation(void) {
	oamRotateScale(&oamSub, 0, angle, (1 << 8), (1<<8));
	angle -= angleChange;
	oamUpdate(&oamSub);
}

int main(int argc, char **argv) {
	initGraphics();
	keysSetRepeat(25,5);
	sysSetCardOwner(BUS_OWNER_ARM9); // Give ARM9 access to Slot-1 (for dumping/injecting saves)
	defaultExceptionHandler();
	scanKeys(); // So it doesn't open the SD if A is held
	srand(time(NULL));
	if(!(rand() % 100))	angleChange *= 2;
	if(!(rand() % 128))	angleChange *= -1;

	drawRectangle(0, 0, 256, 192, DARKER_GRAY, true);
	drawRectangle(0, 0, 256, 192, DARKER_GRAY, false);

	// Init filesystem
	if(!fatInitDefault()) {
		// Prints error if fatInitDefault() fails
		consoleDemoInit();
		printf("fatInitDefault() failed...");
		while(1) swiWaitForVBlank();
	}

	// Make directories
	mkdir(sdFound() ? "sd:/_nds" : "fat:/_nds", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest" : "fat:/_nds/pkmn-chest", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/backups" : "fat:/_nds/pkmn-chest/backups", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/in" : "fat:/_nds/pkmn-chest/out", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/out" : "fat:/_nds/pkmn-chest/out", 0777);

	// Try to init NitroFS from argv provided to the game when it was launched
	if(!nitroFSInit(argv[0])) {
		// If that fails, try to init NitroFS on 'pkmn-chest.nds'
		if(!nitroFSInit("pkmn-chest.nds")) {
			if(!nitroFSInit("/_nds/pkmn-chest/pkmn-chest.nds")) {
				// Prints error if nitroFSInit() fails
				consoleDemoInit();
				printf("nitroFSInit() failed...\n\n");
				printf("If you're using a cia install,\n");
				printf("you'll need:\n\n");
				printf("sdmc:/_nds/pkmn-chest/\n");
				printf("                  pkmn-chest.nds");
				while(1) swiWaitForVBlank();
			}
		}
	}
	loadLogo();
	irqSet(IRQ_VBLANK, loadingAnimation);

	loadFont();
	Config::loadConfig();
	Lang::loadLangStrings(Config::lang);
	printTextCentered(Lang::loading, 0, 32, false);

	Banks::init();
	Sound::init();
	loadGraphics();

	irqSet(IRQ_VBLANK, NULL);
	oamClearSprite(&oamSub, 100);
	oamFreeGfx(&oamSub, logoGfx);
	oamUpdate(&oamSub);

	while(1) {
		if(!loadSave(savePath = browseForSave())) {
			drawRectangle(20, 20, 216, 152, 0xCC00, true);
			printTextCentered(Lang::invalidSave, 0, 24, true);
			for(int i=0;i<120;i++)	swiWaitForVBlank();
			continue;
		}
		currentSaveBox = save->currentBox();
		currentBankBox = 0;
		// Decrypt the box data
		save->cryptBoxData(true);

		drawBoxScreen();
		manageBoxes();
	}

	return 0;
}