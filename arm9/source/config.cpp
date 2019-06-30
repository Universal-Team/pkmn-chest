#include "config.h"
#include "common/inifile.h"
#include "flashcard.h"

std::string Config::chestFile;
int Config::keyboardLayout, Config::keyboardXPos, Config::lang;

void Config::loadConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	Config::chestFile = ini.GetString("chest", "file", "pkmn-chest_1");
	Config::keyboardLayout = ini.GetInt("keyboard", "layout", 0);
	Config::keyboardXPos = ini.GetInt("keyboard", "xPos", 0);
	Config::lang = ini.GetInt("language", "lang", 1);
}

void Config::saveConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	ini.SetString("chest", "file", Config::chestFile);
	ini.SetInt("keyboard", "layout", Config::keyboardLayout);
	ini.SetInt("keyboard", "xPos", Config::keyboardXPos);
	ini.SetInt("language", "lang", Config::lang);
	ini.SaveIniFile(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
}
