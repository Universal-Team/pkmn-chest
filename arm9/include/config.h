#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace Config {
	extern std::string chestFile;
	extern int keyboardLayout, keyboardXPos, lang;

	void loadConfig();
	void saveConfig();
}

#endif
