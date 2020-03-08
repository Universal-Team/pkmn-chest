#include "configMenu.hpp"
#include <dirent.h>

#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "gui.hpp"
#include "i18n.hpp"
#include "input.hpp"
#include "manager.hpp"
#include "misc.hpp"
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
	{4, 144,          "theme"}, // Theme
};

std::vector<Label> textChestFile {
	{12, 32,    "new"}, // New
	{12, 48, "rename"}, // Rename
	{12, 64, "delete"}, // Delete
	{12, 80, "change"}, // Change
};

std::vector<std::string> songs = {"off", "center1", "center3", "center4", "center5", "twinleafTown", "elmLab", "oakLab", "gameCorner", "theme"};

std::vector<std::string> optionsText;

const std::vector<std::string> langNames = {"Bruh", "Deutsch", "English", "Español", "Français", "Bahasa Indonesia", "Italiano", "Lietuvių", "Polski", "Português", "Русский", "日本語", "한국"};
const std::vector<Language> guiLangs = {Language::BRH, Language::GER, Language::ENG, Language::SPA, Language::FRE, Language::IND, Language::ITA, Language::LIT, Language::POL, Language::POR, Language::RUS, Language::JPN, Language::KOR};

void drawChestFileMenu(void) {
	// Draw background
	drawImageDMA(0, 0, listBg, false, false);
	printText(i18n::localize(Config::getLang("lang"), "options"), 4, 0, false, false);

	// Get bank name
	optionsText[0] = Banks::bank->name();

	// Print text
	printText(i18n::localize(Config::getLang("lang"), textCP1Labels[0].label)+": "+optionsText[0], textCP1Labels[0].x, textCP1Labels[0].y, false, false);
	for(unsigned i=0;i<textChestFile.size();i++) {
		printText(i18n::localize(Config::getLang("lang"), textChestFile[i].label), textChestFile[i].x, textChestFile[i].y, false, false);
	}
}

void chestFileMenu(void) {
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	drawChestFileMenu();

	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, textChestFile[0].x+getTextWidth(i18n::localize(Config::getLang("lang"), textChestFile[0].label)), textChestFile[0].y-6);
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
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<textChestFile.size();i++) {
				if(touch.px >= textChestFile[i].x && touch.px <= textChestFile[i].x+getTextWidth(i18n::localize(Config::getLang("lang"), textChestFile[i].label)) && touch.py >= textChestFile[i].y && touch.py <= textChestFile[i].y+16) {
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
					std::string str = Input::getLine(12);
					if(str != "") {
						savePrompt();
						Config::setString("chestFile", str);
						Config::save();
						Banks::loadBank(str);
					}
					drawBox(true);
					break;
				} case 1: { // Rename
					std::string str = Input::getLine(12);
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
						Gui::warn(i18n::localize(Config::getLang("lang"), "cantDeleteCurrentChest"));
					}
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
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
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					break;
				}
			}
			drawChestFileMenu();
			setSpriteVisibility(arrowID, false, true);
		}

		setSpritePosition(arrowID, false, textChestFile[selection].x+getTextWidth(i18n::localize(Config::getLang("lang"), textChestFile[selection].label)), textChestFile[selection].y-6);
		updateOam();
	}
}

void drawConfigMenu(void) {
	// Draw background
	drawImageDMA(0, 0, listBg, false, false);
	printText(i18n::localize(Config::getLang("lang"), "options"), 4, 0, false, false);

	if(optionsText.size() < textCP1Labels.size()) {
		optionsText.resize(textCP1Labels.size());
	}

	// Set variable text
	optionsText[0] = Banks::bank->name();
	optionsText[1] = std::to_string(Banks::bank->boxes());
	int currentLang = std::distance(guiLangs.begin(), std::find(guiLangs.begin(), guiLangs.end(), Config::getLang("lang")));
	optionsText[2] = langNames[currentLang];
	if(Config::getInt("backupAmount") == 0)
		optionsText[3] = i18n::localize(Config::getLang("lang"), "unlimited");
	else
		optionsText[3] = std::to_string(Config::getInt("backupAmount"));
	std::string songName = Config::getString("music");
	if(songName == "")	songName = "off";
	optionsText[4] = i18n::localize(Config::getLang("lang"), songName);
	optionsText[5] = Config::getBool("playSfx") ? i18n::localize(Config::getLang("lang"), "yes") : i18n::localize(Config::getLang("lang"), "no");
	optionsText[6] = Config::getBool("keyboardDirections") ? "4" : "8";
	optionsText[7] = Config::getBool("keyboardGroupAmount") ? "ABCD" : "ABC.";
	optionsText[8] = Config::getString("themeDir").substr(Config::getString("themeDir").find_last_of("/") == std::string::npos ? 0 : Config::getString("themeDir").find_last_of("/")+1);

	// Print text
	for(unsigned i=0;i<textCP1Labels.size();i++) {
		printText(i18n::localize(Config::getLang("lang"), textCP1Labels[i].label)+":", textCP1Labels[i].x, textCP1Labels[i].y, false, false);
	}
	for(unsigned i=0;i<optionsText.size();i++) {
		printText(optionsText[i], textCP1Labels[i].x+getTextWidth(i18n::localize(Config::getLang("lang"), textCP1Labels[i].label))+11, textCP1Labels[i].y, false, false);
	}
}

void configMenu(void) {
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	drawConfigMenu();

	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, textCP1Labels[0].x+getTextWidth(i18n::localize(Config::getLang("lang"), textCP1Labels[0].label))+12+getTextWidth(optionsText[0]), textCP1Labels[0].y-6);
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
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i=0;i<optionsText.size();i++) {
				if(touch.px >= textCP1Labels[i].x+getTextWidth(i18n::localize(Config::getLang("lang"), textCP1Labels[i].label))+11 && touch.px < textCP1Labels[i].x+getTextWidth(i18n::localize(Config::getLang("lang"), textCP1Labels[i].label))+11+getTextWidth(optionsText[i]) && touch.py >= textCP1Labels[i].y && touch.py <= textCP1Labels[i].y+16) {
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
					int currentLang = std::distance(guiLangs.begin(), std::find(guiLangs.begin(), guiLangs.end(), Config::getLang("lang")));
					if(pressed & KEY_LEFT) {
						if(currentLang > 0) {
							Config::setLang("lang", guiLangs[currentLang-1]);
						} else {
							Config::setLang("lang", guiLangs[guiLangs.size()-1]);
						}
					} else if(pressed & KEY_RIGHT) {
						if(currentLang < (int)guiLangs.size()-1) {
							Config::setLang("lang", guiLangs[currentLang+1]);
						} else {
							Config::setLang("lang", guiLangs[0]);
						}
					} else {
						int num = selectItem(currentLang, 0, langNames.size(), langNames);
						if(num != -1) {
							Config::setLang("lang", guiLangs[num]);
						}
					}

					i18n::exit();
					i18n::init(Config::getLang("lang"));
					loadTypes(Config::getLang("lang"));
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
					int current = 0;
					for(unsigned int i=0;i<songs.size();i++) {
						if(songs[i] == Config::getString("music")) {
							current = i;
							break;
						}
					}

					if(pressed & KEY_LEFT) {
						if(current > 0)	Config::setString("music", songs[current-1]);
						else	Config::setString("music", songs[songs.size()-1]);
					} else if(pressed & KEY_RIGHT) {
						if(current < (int)songs.size()-1)	Config::setString("music", songs[current+1]);
						else	Config::setString("music", songs[0]);
					} else {
						std::vector<std::string> names;
						for(auto str : songs) {
							names.push_back(i18n::localize(Config::getLang("lang"), str));
						}
						Config::setString("music", songs[selectItem(current, 0, names.size(), names)]);
					}
					if(Config::getString("music") == "theme") {
						Sound::load((Config::getString("themeDir")+"/sound.msl").c_str());
					} else {
						Sound::load(("nitro:/sound/"+Config::getString("music")+".msl").c_str());
					}
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
				} case 8: { // Theme
					char startPath[PATH_MAX];
					getcwd(startPath, PATH_MAX);

					chdir(sdFound() ? "sd:/_nds/pkmn-chest/themes" : "fat:/_nds/pkmn-chest/themes");
					std::string themePath = browseForFile({}, false, true);

					if(themePath != "") {
						Config::setString("themeDir", (sdFound() ? "sd:/_nds/pkmn-chest/themes/" : "fat:/_nds/pkmn-chest/themes/") + themePath);
						Colors::load();
						loadGraphics();
						loadFont();
						if(Config::getString("music") == "theme") {
							Sound::load((Config::getString("themeDir")+"/sound.msl").c_str());
						}
					}
					chdir(startPath);
					// Clear text
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					break;
				}
			}
			drawConfigMenu();
			setSpriteVisibility(arrowID, false, true);
		}

		setSpritePosition(arrowID, false, textCP1Labels[selection].x+getTextWidth(i18n::localize(Config::getLang("lang"), textCP1Labels[selection].label))+12+getTextWidth(optionsText[selection]), textCP1Labels[selection].y-6);
		updateOam();
	}
}
