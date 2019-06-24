#ifndef CONFIG_H
#define CONFIG_H

namespace Config {
	extern int keyboardLayout;
	extern int keyboardXPos;

	void loadConfig();
	void saveConfig();
}

#endif
