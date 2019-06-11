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
#include "nitrofs.h"

int main(int argc, char **argv) {
	initGraphics();
	keysSetRepeat(25,5);

	if(!fatInitDefault()) {
		// Draws the bottom screen red if fatInitDefault() fails
		drawRectangle(0, 0, 256, 192, BGR15(0, 0, 0xff), false);
		while(1) swiWaitForVBlank();
	}

	if(!nitroFSInit(argv[0])) {
		// Draws the bottom screen blue if nitroFSInit() fails
		drawRectangle(0, 0, 256, 192, BGR15(0xff, 0, 0), false);
		while(1) swiWaitForVBlank();
	}

	std::vector<u16> /*testPng,*/ bankBox, stripes, boxName, arrow;
	// ImageData pngData = loadPng("sd:/test.png", testPng);
	ImageData bankBoxData = loadPng("nitro:/pkmn-chest/bankBox.png", bankBox);
	ImageData stripesData = loadPng("nitro:/pkmn-chest/stripes.png", stripes);
	ImageData boxNameData = loadPng("nitro:/pkmn-chest/boxName.png", boxName);
	ImageData arrowData = loadPng("nitro:/pkmn-chest/arrow.png", arrow);

	// std::vector<u16> testBmp;
	// ImageData bmpData = loadBmp("sd:/test.bmp", testBmp);

	//drawImageScaled(10, 50, pngData.width, pngData.height, 2, testPng, true);  // That was for test purpose.
	drawImageScaled(5, 30, bankBoxData.width, bankBoxData.height, 1, bankBox, false);
	//drawImageTinted(100, 150, pngData.width, pngData.height, 0x83ff, testPng, false); // That was for test purpose too.
	drawImageScaled(5, 30, bankBoxData.width, bankBoxData.height, 1, bankBox, true);
	// Stripes for the Text later.
	drawImageScaled(180, 30, stripesData.width, stripesData.height, 1, stripes, true);
	drawImageScaled(180, 60, stripesData.width, stripesData.height, 1, stripes, true);
	drawImageScaled(180, 90, stripesData.width, stripesData.height, 1, stripes, true);
	// Box Bars for the Bank Name.
	drawImageScaled(5, 10, boxNameData.width, boxNameData.height, 1, boxName, true);
	drawImageScaled(5, 10, boxNameData.width, boxNameData.height, 1, boxName, false);
	// Arrow.
	drawImageScaled(7, 30, arrowData.width, arrowData.height, 1, arrow, true);
	// The Button.
	drawRectangle(180, 120, 68, 30, BGR15(0x63, 0x65, 0x73), false);
	
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
		// drawImageScaled(x, y, bmpData.width, bmpData.height, scale, testBmp, true);
	}

	// std::vector<std::string> extensionList;
	// extensionList.push_back(".sav");
	// browseForFile(extensionList);

	while(1) {

	}

	return 0;
}