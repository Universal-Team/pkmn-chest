#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace Config {
	extern std::string chestFile;
	extern bool playSfx;
	extern int backupAmount, keyboardLayout, keyboardXPos, lang, music;

	void loadConfig();
	void saveConfig();
}

#endif
