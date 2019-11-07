#include "configMenu.hpp"
#include <dirent.h>

#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "lang.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "xMenu.hpp"
#include "sound.hpp"

#define getChestFile() (Config::getString("chestFile") == "" ? "pkmn-chest_1" : Config::getString("chestFile"))

std::vector<Label> textCP1Labels {
	{4,  16,      "chestFile"}, // Chest file
	{4,  32,      "chestSize"}, // Chest size
	{4,  48,       "language"}, // Language
	{4,  64,   "backupAmount"}, // Backups
	{4,  80,          "music"}, // Music
	{4,  96,        "soundFX"}, // Sound FX
	{4, 112, "dPadDirections"}, // D-Pad typing directions
	{4, 128,     "dPadGroups"}, // D-Pad typing groups
};

std::vector<Label> textChestFile {
	{12, 32,    "new"}, // New
	{12, 48, "rename"}, // Rename
	{12, 64, "delete"}, // Delete
	{12, 80, "change"}, // Change
};

std::vector<std::string> songs = {"off", "center1", "center4", "center5", "twinleafTown", "elmLab", "oakLab", "gameCorner"};

std::vector<std::string> optionsText;

std::string langNames[] = { "Deutsche", "English", "Español", "Français", "Italiano", "Lietuvių", "Português", "русский", "日本語", "한국", "Bruh"};

void drawChestFileMenu(void) {
	// Draw background
	drawImageDMA(0, 0, 256, 192, listBg, false);
	printText(Lang::get("options"), 4, 0, false);

	// Print text
	printText(Lang::get(textCP1Labels[0].label)+": "+optionsText[0], textCP1Labels[0].x, textCP1Labels[0].y, false);
	for(unsigned i=0;i<textChestFile.size();i++) {
		printText(Lang::get(textChestFile[i].label), textChestFile[i].x, textChestFile[i].y, false);
	}
}

void chestFileMenu(void) {
	drawChestFileMenu();

	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, textChestFile[0].x+getTextWidth(Lang::get(textChestFile[0].label)), textChestFile[0].y-6);
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
			if(selection < (int)(textChestFile.size())-1)	selection++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<(sizeof(textChestFile)/sizeof(textChestFile[0]));i++) {
				if(touch.px >= textChestFile[i].x && touch.px <= textChestFile[i].x+getTextWidth(Lang::get(textChestFile[i].label)) && touch.py >= textChestFile[i].y && touch.py <= textChestFile[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			switch(selection) {
				case 0: { // New
					std::string str = Input::getLine();
					if(str != "") {
						savePrompt();
						Config::setString("chestFile", str);
						Config::save();
						Banks::loadBank(str);
					}
					drawBox(true);
					break;
				} case 1: { // Rename
					std::string str = Input::getLine();
					if(str != "")	Banks::renameBank(getChestFile(), str);
					Config::setString("chestFile", str);
					Config::save();
					break;
				} case 2: { // Delete
					char path[PATH_MAX];
					getcwd(path, PATH_MAX);
					chdir(sdFound() ? "sd:/_nds/pkmn-chest/banks" : "fat:/_nds/pkmn-chest/banks");
					std::vector<std::string> extList;
					extList.push_back(".bnk");
					std::string str = browseForFile(extList, false);
					if(str.substr(0, str.find_last_of(".")) != getChestFile() && str != "")	Banks::removeBank(str.substr(0, str.find_last_of(".")));
					else if(str != "") {
						drawRectangle(20, 20, 216, 152, RGB::DARK_RED, false);
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
					if(str != getChestFile() && str != "") {
						savePrompt();
						Config::setString("chestFile", str.substr(0, str.find_last_of(".")));
						Config::save();
						Banks::init();
					}
					chdir(path);
					drawBox(true);
					break;
				}
			}
			drawChestFileMenu();
			setSpriteVisibility(arrowID, false, true);
		}

		setSpritePosition(arrowID, false, textChestFile[selection].x+getTextWidth(Lang::get(textChestFile[selection].label)), textChestFile[selection].y-6);
		updateOam();
	}
}

void drawConfigMenu(void) {
	// Draw background
	drawImageDMA(0, 0, 256, 192, listBg, false);
	printText(Lang::get("options"), 4, 0, false);

	if(optionsText.size() < textCP1Labels.size()) {
		optionsText.resize(textCP1Labels.size());
	}

	// Set variable text
	optionsText[0] = Banks::bank->name();
	optionsText[1] = std::to_string(Banks::bank->boxes());
	optionsText[2] = langNames[Config::getLang("lang")];
	if(Config::getInt("backupAmount") == 0)
		optionsText[3] = Lang::get("unlimited");
	else
		optionsText[3] = std::to_string(Config::getInt("backupAmount"));
	optionsText[4] = Lang::get(songs[Config::getInt("music")]);
	optionsText[5] = Config::getBool("playSfx") ? Lang::get("yes") : Lang::get("no");
	optionsText[6] = Config::getInt("keyboardDirections") ? "4" : "8";
	optionsText[7] = Config::getInt("keyboardGroupAmount") ? "ABCD" : "ABC.";

	// Print text
	for(unsigned i=0;i<textCP1Labels.size();i++) {
		printText(Lang::get(textCP1Labels[i].label)+":", textCP1Labels[i].x, textCP1Labels[i].y, false);
	}
	for(unsigned i=0;i<optionsText.size();i++) {
		printText(optionsText[i], textCP1Labels[i].x+getTextWidth(Lang::get(textCP1Labels[i].label))+11, textCP1Labels[i].y, false);
	}
}

void configMenu(void) {
	drawConfigMenu();

	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, textCP1Labels[0].x+getTextWidth(Lang::get(textCP1Labels[0].label))+12+getTextWidth(optionsText[0]), textCP1Labels[0].y-6);
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
			if(selection < (int)(optionsText.size())-1)	selection++;
		} else if((selection > 1) && (pressed & KEY_LEFT || pressed & KEY_RIGHT)) {
			optionSelected = true;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			Config::save();
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<optionsText.size();i++) {
				if(touch.px >= textCP1Labels[i].x+getTextWidth(Lang::get(textCP1Labels[i].label))+11 && touch.px < textCP1Labels[i].x+getTextWidth(Lang::get(textCP1Labels[i].label))+11+getTextWidth(optionsText[i]) && touch.py >= textCP1Labels[i].y && touch.py <= textCP1Labels[i].y+16) {
					selection = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			switch(selection) {
				case 0: { // Chest File
					chestFileMenu();
					break;
				} case 1: { // Chest Size
					int num = Input::getInt(sdFound() ? 500 : 50);
					if(num > 0) {
						Banks::setBankSize(getChestFile(), num);
					}
					break;
				} case 2: { // Language
					if(pressed & KEY_LEFT) {
						if(Config::getLang("lang") > 0)	Config::setInt("lang", Config::getLang("lang")-1);
						else	Config::setInt("lang", (sizeof(langNames)/sizeof(langNames[0]))-1);
					} else {
						if(Config::getLang("lang") < (int)(sizeof(langNames)/sizeof(langNames[0]))-1)	Config::setInt("lang", Config::getLang("lang")+1);
						else	Config::setInt("lang", 0);
					}
					Lang::load(Config::getLang("lang"));
					break;
				} case 3: { // Backup Amount
					if(pressed & KEY_LEFT) {
						if(Config::getInt("backupAmount") > 0)	Config::setInt("backupAmount", Config::getInt("backupAmount")-1);
					} else if(pressed & KEY_RIGHT) {
						Config::setInt("backupAmount", Config::getInt("backupAmount")+1);
					} else {
						int num = Input::getInt(9);
						if(num != -1) {
							Config::setInt("backupAmount", num);
						}
					}
					break;
				} case 4: { // Music
					if(pressed & KEY_LEFT) {
						if(Config::getInt("music") > 0)	Config::setInt("music", Config::getInt("music")-1);
						else	Config::setInt("music", songs.size()-1);
					} else {
						if(Config::getInt("music") < (int)songs.size()-1)	Config::setInt("music", Config::getInt("music")+1);
						else	Config::setInt("music", 0);
					}
					Sound::playBgm(Config::getInt("music"));
					break;
				} case 5: { // Sound FX
					Config::setBool("playSfx", !Config::getBool("playSfx"));
					break;
				} case 6: { // D-Pad typing directions
					Config::setBool("keyboardDirections", !Config::getBool("keyboardDirections"));
					break;
				} case 7: { // D-Pad typing grounds
					Config::setBool("keyboardGroupAmount", !Config::getBool("keyboardGroupAmount"));
					break;
				}
			}
			drawConfigMenu();
			setSpriteVisibility(arrowID, false, true);
		}

		setSpritePosition(arrowID, false, textCP1Labels[selection].x+getTextWidth(Lang::get(textCP1Labels[selection].label))+12+getTextWidth(optionsText[selection]), textCP1Labels[selection].y-6);
		updateOam();
	}
}
