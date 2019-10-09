#include "config.hpp"
#include <nds/system.h>

#include "inifile.hpp"
#include "flashcard.hpp"
#include "lang.hpp"

std::string Config::chestFile;
bool Config::playSfx;
int Config::backupAmount, Config::keyboardLayout, Config::keyboardXPos, Config::lang, Config::music;

int sysLang() {
	switch(PersonalData->language) {
		case 0:
			return Lang::jp;
		case 1:
		default:
			return Lang::en;
		case 2:
			return Lang::fr;
		case 3:
			return Lang::de;
		case 4:
			return Lang::es;
	}
}

void Config::loadConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	Config::backupAmount = ini.GetInt("backup", "amount", 0);
	Config::chestFile = ini.GetString("chest", "file", "pkmn-chest_1");
	Config::keyboardLayout = ini.GetInt("keyboard", "layout", 0);
	Config::keyboardXPos = ini.GetInt("keyboard", "xPos", 0);
	Config::lang = ini.GetInt("language", "lang", sysLang());
	Config::music = ini.GetInt("sound", "music", 0);
	Config::playSfx = ini.GetInt("sound", "sfx", 0);
}

void Config::saveConfig() {
	CIniFile ini(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
	ini.SetInt("backup", "amount", Config::backupAmount);
	ini.SetString("chest", "file", Config::chestFile);
	ini.SetInt("keyboard", "layout", Config::keyboardLayout);
	ini.SetInt("keyboard", "xPos", Config::keyboardXPos);
	ini.SetInt("language", "lang", Config::lang);
	ini.SetInt("sound", "music", Config::music);
	ini.SetInt("sound", "sfx", Config::playSfx);
	ini.SaveIniFile(sdFound() ? "sd:/_nds/pkmn-chest/config.ini" : "fat:/_nds/pkmn-chest/config.ini");
}
