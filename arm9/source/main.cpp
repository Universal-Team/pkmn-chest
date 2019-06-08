#include <cstdio>
#include <ctype.h>
#include <dirent.h>
#include <fat.h>
#include <fstream>
#include <iostream>
#include <nds.h>
#include <string>
#include <vector>

#include "fileBrowse.h"

PrintConsole topScreen;
PrintConsole touchScreen;

void printLine(std::string text, bool onTopScreen) {
	consoleSelect(onTopScreen ? &topScreen : &touchScreen);
	std::cout << text.c_str() << std::endl;
}

int main() {
	touchScreen = *consoleDemoInit();
	videoSetMode(MODE_0_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	consoleInit(&topScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
	
	
	printLine("pkmn-chest", true);
	printLine("by: Universal Team", true);
	printLine("[Very early WIP]", true);

	if(!fatInitDefault()) {
		consoleSelect(&touchScreen);
		printLine("SD init failed", false);
		while(1) swiWaitForVBlank();
	}

	consoleSelect(&touchScreen);

	printLine("Test", false);

	std::vector<std::string> extensionList;
	extensionList.push_back(".sav");
	browseForFile(extensionList);

	while(1) {

	}

	return 0;
}