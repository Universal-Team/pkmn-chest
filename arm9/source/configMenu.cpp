#include "configMenu.h"
#include <dirent.h>

#include "banks.hpp"
#include "colors.h"
#include "config.h"
#include "fileBrowse.h"
#include "flashcard.h"
#include "graphics.h"
#include "keyboard.h"
#include "lang.h"
#include "langStrings.h"
#include "manager.h"

struct Text {
	int x;
	int y;
};

Text textCP1Labels[] {
	{20, 20}, // Chest file
	{20, 100}, // Chest size
	{20, 116}, // Language
	{20, 132}, // Backups
};
Text textCP1[] {
	{28, 36}, // New
	{28, 52}, // Rename
	{28, 68}, // Delete
	{28, 84}, // Change
};

std::vector<std::string> optionsText = {"", "", ""};

std::string langNames[] = { "Deutsche", "English", "Español", "Français", "Italiano", "にほんご" };

void drawConfigMenu(void) {
	// Draw background
	drawRectangle(0, 0, 256, 16, BLACK, false);
	drawRectangle(0, 16, 256, 160, DARK_GRAY, false);
	drawRectangle(0, 176, 256, 16, BLACK, false);

	// Set bank size and language text
	char str[4];
	snprintf(str, sizeof(str), "%i", Banks::bank->boxes());
	optionsText[0] = str;
	optionsText[1] = langNames[Config::lang];
	snprintf(str, sizeof(str), "%i", Config::backupAmount);
	optionsText[2] = str;

	// Print text
	for(uint i=0;i<Lang::optionsTextLabels.size();i++) {
		printText(Lang::optionsTextLabels[i], textCP1Labels[i].x, textCP1Labels[i].y, false);
	}
	for(uint i=0;i<(sizeof(textCP1)/sizeof(textCP1[0]));i++) {
		printText(Lang::optionsText[i], textCP1[i].x, textCP1[i].y, false);
	}
	for(uint i=0;i<optionsText.size();i++) {
		printText(optionsText[i], textCP1Labels[i+1].x+getTextWidth(Lang::optionsTextLabels[i+1])+8, textCP1Labels[i+1].y, false);
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
		} else if(selection == 5 && (pressed & KEY_LEFT || pressed & KEY_RIGHT)) {
			optionSelected = true;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Config::saveConfig();
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(uint i=0;i<(sizeof(textCP1)/sizeof(textCP1[0]));i++) {
				if(touch.px >= textCP1[i].x && touch.px <= textCP1[i].x+getTextWidth(Lang::optionsText[i]) && touch.py >= textCP1[i].y && touch.py <= textCP1[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			setSpriteVisibility(bottomArrowID, false);
			updateOam();
			switch(selection) {
				case 0: { // New
					std::string str = Input::getLine();
					if(str != "") {
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
						Config::chestFile = str.substr(0, str.find_last_of("."));
						Config::saveConfig();
						Banks::init();
					}
					chdir(path);
					drawBox(true);
					break;
				} case 4: { // Resize
					int num = Input::getInt(500);
					if(num > 0)	Banks::setBankSize(Config::chestFile, num);
					break;
				} case 5: { // Language
					if(pressed & KEY_LEFT) {
						if(Config::lang > 0)	Config::lang--;
						else	Config::lang = 5;
					} else {
						if(Config::lang < 5)	Config::lang++;
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
