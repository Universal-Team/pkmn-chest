#include "configMenu.h"
#include <dirent.h>

#include "banks.hpp"
#include "colors.h"
#include "config.h"
#include "fileBrowse.h"
#include "flashcard.h"
#include "graphics.h"
#include "input.h"
#include "lang.h"
#include "langStrings.h"
#include "manager.h"
#include "menus.h"
#include "sound.h"

struct Text {
	int x;
	int y;
};

Text textCP1Labels[] {
	{4, 14}, // Chest file
	{4, 94}, // Chest size
	{4, 110}, // Language
	{4, 126}, // Backups
	{4, 142}, // Music
	{4, 158}, // Sound FX
};
Text textCP1[] {
	{12, 30}, // New
	{12, 46}, // Rename
	{12, 62}, // Delete
	{12, 78}, // Change
};

std::vector<std::string> optionsText = {"", "", "", "", ""}; // Placeholders to be filled

std::string langNames[] = { "Deutsche", "English", "Español", "Français", "Italiano", "にほんご", "русский"};

void drawConfigMenu(void) {
	// Draw background
	if(sdFound())	drawImage(0, 0, optionsBgData.width, optionsBgData.height, optionsBg, false);
	else	drawRectangle(0, 0, 256, 192, DARK_GRAY, false);

	// Set variable text
	char str[16];
	snprintf(str, sizeof(str), "%i", Banks::bank->boxes());
	optionsText[0] = str;
	optionsText[1] = langNames[Config::lang];
	snprintf(str, sizeof(str), "%i", Config::backupAmount);
	optionsText[2] = str;
	snprintf(str, sizeof(str), "%i", Config::music);
	optionsText[3] = Lang::songs[Config::music];
	optionsText[4] = Config::playSfx ? Lang::yes : Lang::no;

	// Print text
	for(unsigned i=0;i<Lang::optionsTextLabels.size();i++) {
		printTextTinted(Lang::optionsTextLabels[i]+":", GRAY, textCP1Labels[i].x, textCP1Labels[i].y, false, true);
	}
	for(unsigned i=0;i<(sizeof(textCP1)/sizeof(textCP1[0]));i++) {
		printTextTinted(Lang::optionsText[i], GRAY, textCP1[i].x, textCP1[i].y, false, true);
	}
	printTextTinted(Banks::bank->name(), GRAY, textCP1Labels[0].x+getTextWidth(Lang::optionsTextLabels[0])+8, textCP1Labels[0].y, false, true);
	for(unsigned i=0;i<optionsText.size();i++) {
		printTextTinted(optionsText[i], GRAY, textCP1Labels[i+1].x+getTextWidth(Lang::optionsTextLabels[i+1])+8, textCP1Labels[i+1].y, false, true);
	}

}

void configMenu(void) {
	drawConfigMenu();

	setSpriteVisibility(bottomArrowID, true);
	setSpritePosition(bottomArrowID, textCP1[0].x+getTextWidth(Lang::optionsText[0]), textCP1[0].y-6);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)	selection--;
		} else if(held & KEY_DOWN) {
			if(selection < (int)(Lang::optionsText.size()+optionsText.size())-1)	selection++;
		} else if((selection == 5 || selection == 7) && (pressed & KEY_LEFT || pressed & KEY_RIGHT)) {
			optionSelected = true;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			Config::saveConfig();
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textCP1)/sizeof(textCP1[0]));i++) {
				if(touch.px >= textCP1[i].x && touch.px <= textCP1[i].x+getTextWidth(Lang::optionsText[i]) && touch.py >= textCP1[i].y && touch.py <= textCP1[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i=0;i<optionsText.size();i++) {
				if(touch.px >= textCP1Labels[i+1].x+getTextWidth(Lang::optionsTextLabels[i+1])+8 && touch.px < textCP1Labels[i+1].x+getTextWidth(Lang::optionsTextLabels[i+1])+8+getTextWidth(optionsText[i]) && touch.py >= textCP1Labels[i+1].y && touch.py <= textCP1Labels[i+1].y+16) {
					selection = i+(sizeof(textCP1)/sizeof(textCP1[0]));
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			switch(selection) {
				case 0: { // New
					std::string str = Input::getLine();
					if(str != "") {
						savePrompt();
						Config::chestFile = str;
						Config::saveConfig();
						Banks::loadBank(str);
					}
					drawBox(true);
					break;
				} case 1: { // Rename
					std::string str = Input::getLine();
					if(str != "")	Banks::renameBank(Config::chestFile, str);
					Config::chestFile = str;
					Config::saveConfig();
					break;
				} case 2: { // Delete
					char path[PATH_MAX];
					getcwd(path, PATH_MAX);
					chdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks");
					std::vector<std::string> extList;
					extList.push_back(".bnk");
					std::string str = browseForFile(extList, false);
					if(str.substr(0, str.find_last_of(".")) != Config::chestFile && str != "")	Banks::removeBank(str.substr(0, str.find_last_of(".")));
					else if(str != "") {
						drawRectangle(20, 20, 216, 152, 0xCC00, false);
						printTextCentered("You can not delete", 0, 24, false);
						printTextCentered("the current bank.", 0, 40, false);
						for(int i=0;i<120;i++)	swiWaitForVBlank();
					}
					chdir(path);
					break;
				} case 3: { // Change
					char path[PATH_MAX];
					getcwd(path, PATH_MAX);
					chdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks");
					std::vector<std::string> extList;
					extList.push_back(".bnk");
					std::string str = browseForFile(extList, false);
					if(str != Config::chestFile && str != "") {
						savePrompt();
						Config::chestFile = str.substr(0, str.find_last_of("."));
						Config::saveConfig();
						Banks::init();
					}
					chdir(path);
					drawBox(true, true);
					break;
				} case 4: { // Resize
					int num = Input::getInt(500);
					if(num > 0)	Banks::setBankSize(Config::chestFile, num);
					break;
				} case 5: { // Language
					if(pressed & KEY_LEFT) {
						if(Config::lang > 0)	Config::lang--;
						else	Config::lang = 6;
					} else {
						if(Config::lang < 6)	Config::lang++;
						else	Config::lang = 0;
					}
					Lang::loadLangStrings(Config::lang);
					break;
				} case 6: { // Backup Amount
					int num = Input::getInt(9);
					if(num != -1) {
						Config::backupAmount = num;
						Config::saveConfig();
					}
					break;
				} case 7: { // Music
					if(pressed & KEY_LEFT) {
						if(Config::music > 0)	Config::music--;
						else	Config::music = 6;
					} else {
						if(Config::music < 6)	Config::music++;
						else	Config::music = 0;
					}
					Sound::playBgm(Config::music);
					break;
				} case 8: { // Sound FX
					Config::playSfx = !Config::playSfx;
					Config::saveConfig();
					break;
				}
			}
			drawConfigMenu();
			setSpriteVisibility(bottomArrowID, true);
		}

		if(selection < (int)Lang::optionsText.size())	setSpritePosition(bottomArrowID, textCP1[selection].x+getTextWidth(Lang::optionsText[selection]), textCP1[selection].y-6);
		else	setSpritePosition(bottomArrowID, textCP1Labels[selection-3].x+getTextWidth(Lang::optionsTextLabels[selection-3])+8+getTextWidth(optionsText[selection-4]), textCP1Labels[selection-3].y-6);
		updateOam();
	}
}
