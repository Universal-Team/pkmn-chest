#include <fat.h>

#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "lang.hpp"
#include "lang.hpp"
#include "loader.hpp"
#include "loading.hpp"
#include "manager.hpp"
#include "nitrofs.h"
#include "sound.hpp"

int main(int argc, char **argv) {
	initGraphics();
	keysSetRepeat(25,5);
	sysSetCardOwner(BUS_OWNER_ARM9); // Set ARM9 as Slot-1 owner (for dumping/injecting DS saves)
	sysSetCartOwner(BUS_OWNER_ARM9); // Set ARM9 as Slot-2 owner (for dumping/injecting GBA saves)
	defaultExceptionHandler();
	scanKeys(); // So it doesn't open the SD if A is held
	srand(time(NULL));
	if(!(rand() % 100))	angleChange *= 2;
	if(!(rand() % 128))	angleChange *= -1;

	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, true, false);
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);

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
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/in" : "fat:/_nds/pkmn-chest/in", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/out" : "fat:/_nds/pkmn-chest/out", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/themes" : "fat:/_nds/pkmn-chest/themes", 0777);

	// Try to init NitroFS from argv provided to the game when it was launched
	if(!nitroFSInit(argv[0])) {
		// If that fails, try to init NitroFS on 'pkmn-chest.nds'
		if(!nitroFSInit("pkmn-chest.nds")) {
			if(!nitroFSInit("/_nds/pkmn-chest/pkmn-chest.nds")) {
				// Prints error if nitroFSInit() fails
				consoleDemoInit();
				printf("nitroFSInit() failed...\n\n");
				printf("Please copy pkmn-chest.nds to:\n\n");
				printf("%s:/_nds/pkmn-chest/\n", sdFound() ? (access("/Nintendo 3DS", F_OK) == 0 ? "sdmc" : "sd") : "fat");
				printf("                  pkmn-chest.nds\n\n");
				printf("or launch pkmn-chest using\n\n");
				printf("      TWiLight Menu++ or HBMenu\n\n\n\n\n");
				printf("(Note: TWiLight's Acekard\n");
				printf("        theme needs a copy in ^)\n\n");
				while(1) swiWaitForVBlank();
			}
		}
	}
	loadLoadingLogo();
	showLoadingLogo();

	Config::load();
	Colors::load();
	loadFont();
	Lang::load(Config::getLang("lang"));
	printTextCentered(Lang::get("loading"), 0, 32, false, true);

	Sound::init();
	Banks::init();
	initSprites();
	loadGraphics();

	hideLoadingLogo();

	while(1) {
		savePath = browseForSave();
		if(savePath == "%EXIT%")	break;

		if(!loadSave(savePath)) {
			drawRectangle(20, 20, 216, 152, DARK_RED, true, true);
			printTextCentered(Lang::get("invalidSave"), 0, 24, true, true);
			for(int i=0;i<120;i++)	swiWaitForVBlank();
			drawRectangle(20, 20, 216, 152, CLEAR, true, true);
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