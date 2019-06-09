#include <cstdio>
#include <ctype.h>
#include <dirent.h>
#include <fat.h>
#include <fstream>
#include <iostream>
#include <nds.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "fileBrowse.h"
#include "graphics/graphics.h"

int main() {
	initGraphics();
	keysSetRepeat(25,5);

	if(!fatInitDefault()) {
		// Draws the bottom screen red if fatInitDefault() fails
		drawRectange(0, 0, 256, 192, RGB15(0xff, 0, 0), false);
		while(1) swiWaitForVBlank();
	}

	// Some test rectangles
	drawRectange(10, 0, 100, 100, RGB15(0xff, 0xff, 0xff), true);
	drawRectange(50, 50, 10, 10, RGB15(0xff, 0xff, 0), true);

	std::vector<u16> testPng;
	ImageData pngData = loadPng("sd:/test.png", testPng);
	std::vector<u16> testBmp;
	ImageData bmpData = loadBmp("sd:/test.bmp", testBmp);

	drawImage(10, 50, pngData.width, pngData.height, testPng, false);
	drawImage(70, 30, bmpData.width, bmpData.height, testBmp, true);

	// std::vector<std::string> extensionList;
	// extensionList.push_back(".sav");
	// browseForFile(extensionList);

	while(1) {

	}

	return 0;
}