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
		drawRectangle(0, 0, 256, 192, BGR15(0, 0, 0xff), false);
		while(1) swiWaitForVBlank();
	}

	// Some test rectangles
	drawRectangle(10, 0, 100, 100, BGR15(0xff, 0xff, 0xff), true);
	drawRectangle(50, 50, 10, 10, BGR15(0, 0xff, 0xff), true);

	std::vector<u16> testPng;
	ImageData pngData = loadPng("sd:/test.png", testPng);
	std::vector<u16> testBmp;
	ImageData bmpData = loadBmp("sd:/test.bmp", testBmp);

	drawImageScaled(10, 50, pngData.width, pngData.height, 2, testPng, true);
	drawImageTinted(100, 150, pngData.width, pngData.height, 0x83ff, testPng, false);
	double scale = 1;
	int x = 0;
	int y = 0;
	u16 hHeld = 0;
	while(1) {
		do {
		swiWaitForVBlank();
		scanKeys();
		hHeld = keysHeld();
		} while(!hHeld);
		if(hHeld & KEY_X) {
			scale += .1;
		} else if(hHeld & KEY_Y) {
			scale -= .1;
		} else if(hHeld & KEY_A) {
			scale = 1;
		}
		if(hHeld & KEY_UP) {
			y--;
		} else if(hHeld & KEY_DOWN) {
			y++;
		}
		if(hHeld & KEY_RIGHT) {
			x++;
		} else if(hHeld & KEY_LEFT) {
			x--;
		}
		drawRectangle(0, 0, 256, 192, 0, true);
		drawImageScaled(x, y, bmpData.width, bmpData.height, scale, testBmp, true);
	}

	// std::vector<std::string> extensionList;
	// extensionList.push_back(".sav");
	// browseForFile(extensionList);

	while(1) {

	}

	return 0;
}