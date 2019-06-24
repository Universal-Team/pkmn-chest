#include "config.h"
#include "common/inifile.h"
#include "flashcard.h"

int Config::keyboardLayout;
int Config::keyboardXPos;

void Config::loadConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	Config::keyboardLayout = ini.GetInt("pkmn-chest", "keyboardLayout", 0);
	Config::keyboardXPos = ini.GetInt("pkmn-chest", "keyboardXPos", 0);
}

void Config::saveConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	ini.SetInt("pkmn-chest", "keyboardLayout", Config::keyboardLayout);
	ini.SetInt("pkmn-chest", "keyboardXPos", Config::keyboardXPos);
	ini.SaveIniFile(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
}
