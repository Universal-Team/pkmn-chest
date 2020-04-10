#include <fat.h>

#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "gui.hpp"
#include "i18n.hpp"
#include "loader.hpp"
#include "loading.hpp"
#include "manager.hpp"
#include "miscUtils.hpp"
#include "nitrofs.h"
#include "sound.hpp"

bool useTwlCfg = false;

extern std::vector<std::string> songs;

int main(int argc, char **argv) {
	useTwlCfg = (isDSiMode() && (*(u8*)0x02000400 & 0x0F) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0));
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
		while(1)	swiWaitForVBlank();
	}

	// Make directories
	mkdir(sdFound() ? "sd:/_nds" : "fat:/_nds", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest" : "fat:/_nds/pkmn-chest", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/backups" : "fat:/_nds/pkmn-chest/backups", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/in" : "fat:/_nds/pkmn-chest/in", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/out" : "fat:/_nds/pkmn-chest/out", 0777);
	mkdir(sdFound() ? "sd:/_nds/pkmn-chest/themes" : "fat:/_nds/pkmn-chest/themes", 0777);

	// Try to init NitroFS from argv provided to the app when it was launched
	if(!nitroFSInit(argv[0])) {
		// If that fails, try to init NitroFS on 'pkmn-chest.nds'
		if(!nitroFSInit("pkmn-chest.nds") || !nitroFSGood()) {
			// If that fails, try '/_nds/pkmn-chest/pkmn-chest.nds'
			if(!nitroFSInit("/_nds/pkmn-chest/pkmn-chest.nds")) {
				// Print error that nitroFSInit() failed
				consoleDemoInit();
				printf("nitroFSInit() failed...\n\n");
				printf("Please copy pkmn-chest.nds to:\n\n");
				printf("%s:/_nds/pkmn-chest/\n", sdFound() ? (access("/Nintendo 3DS", F_OK) == 0 ? "sdmc" : "sd") : "fat");
				printf("                  pkmn-chest.nds\n\n");
				printf("or launch pkmn-chest using\n\n");
				printf("      TWiLight Menu++ or HBMenu\n\n\n\n\n");
				printf("(Note: TWiLight's Acekard\n");
				printf("        theme needs a copy in ^)\n\n");
				while(1)	swiWaitForVBlank();
			} else if(!nitroFSGood()) {
				// Print error that the NitroFS was out of date
				consoleDemoInit();
				printf("NitroFS is out of date!\n\n");
				printf("Please update pkmn-chest.nds at:\n\n");
				printf("%s:/_nds/pkmn-chest/\n", sdFound() ? (access("/Nintendo 3DS", F_OK) == 0 ? "sdmc" : "sd") : "fat");
				while(1)	swiWaitForVBlank();
			}
		}
	}

	wideScreen = (strcmp(argv[1], "wide") == 0) || keysDown() & KEY_Y;

	loadLoadingLogo();
	showLoadingLogo();

	Config::load();
	Colors::load();
	loadFont();

	i18n::removeCallbacks(i18n::initBall, i18n::exitBall);
	i18n::removeCallbacks(i18n::initForm, i18n::exitForm);
	i18n::removeCallbacks(i18n::initHP, i18n::exitHP);
	i18n::removeCallbacks(i18n::initGeo, i18n::exitGeo);
	i18n::init(Config::getLang("lang"));

	printTextCentered(i18n::localize(Config::getLang("lang"), "loading"), 0, 32, false, true);

	if(Config::getString("music") == "theme") {
		Sound::load((Config::getString("themeDir")+"/sound.msl").c_str());
	} else {
		Sound::load(("nitro:/sound/"+Config::getString("music")+".msl").c_str());
	}

	Banks::init();
	initSprites();
	loadGraphics();
	loadTypes(Config::getLang("lang"));

	hideLoadingLogo();

	while(1) {
		savePath = browseForSave();
		if(savePath == "%EXIT%")	break;

		if(!loadSave(savePath)) {
			Gui::warn(i18n::localize(Config::getLang("lang"), "invalidSave"));
			continue;
		}
		currentSaveBox = save->currentBox();
		currentBankBox = 0;
		// Decrypt the box data
		save->cryptBoxData(true);

		manageBoxes();
	}

	return 0;
}