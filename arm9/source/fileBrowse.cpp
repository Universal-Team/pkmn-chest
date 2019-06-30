/*-----------------------------------------------------------------
 Copyright(C) 2005 - 2017
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or(at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/

#include "fileBrowse.h"
#include <algorithm>
#include <dirent.h>
#include <fat.h>
#include <fstream>
#include <unistd.h>

#include "flashcard.h"
#include "colors.h"
#include "graphics.h"
#include "keyboard.h"
#include "loader.h"
#include "cardSaves.h"
#include "utils.hpp"

#define ENTRIES_PER_SCREEN 11
#define ENTRY_PAGE_LENGTH 10
#define copyBufSize 0x8000

char path[PATH_MAX];
char fatLabel[12];
char sdLabel[12];
u32 copyBuf[copyBufSize];

struct DirEntry {
	std::string name;
	bool isDirectory;
};

struct topMenuItem {
	std::string name;
	bool valid;
};

bool nameEndsWith(const std::string& name, const std::vector<std::string> extensionList) {
	if(name.substr(0, 2) == "._") return false;

	if(name.size() == 0) return false;

	if(extensionList.size() == 0) return true;

	for(int i = 0; i <(int)extensionList.size(); i++) {
		const std::string ext = extensionList.at(i);
		if(strcasecmp(name.c_str() + name.size() - ext.size(), ext.c_str()) == 0) return true;
	}
	return false;
}

bool dirEntryPredicate(const DirEntry& lhs, const DirEntry& rhs) {
	if(!lhs.isDirectory && rhs.isDirectory) {
		return false;
	}
	if(lhs.isDirectory && !rhs.isDirectory) {
		return true;
	}
	return strcasecmp(lhs.name.c_str(), rhs.name.c_str()) < 0;
}

void getDirectoryContents(std::vector<DirEntry>& dirContents, const std::vector<std::string> extensionList) {
	struct stat st;

	dirContents.clear();

	DIR *pdir = opendir(".");

	if(pdir == NULL) {
		printText("Unable to open the directory.", 0, 0, false);
	} else {
		while(true) {
			DirEntry dirEntry;

			struct dirent* pent = readdir(pdir);
			if(pent == NULL) break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory =(st.st_mode & S_IFDIR) ? true : false;

			if(dirEntry.name.compare(".") != 0 &&(dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
				dirContents.push_back(dirEntry);
			}
		}
		closedir(pdir);
	}
	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

void getDirectoryContents(std::vector<DirEntry>& dirContents) {
	std::vector<std::string> extensionList;
	getDirectoryContents(dirContents, extensionList);
}

void showDirectoryContents(const std::vector<DirEntry>& dirContents, int startRow) {
	getcwd(path, PATH_MAX);

	// Draw background
	drawRectangle(0, 0, 256, 15, BLACK, false);
	drawRectangle(0, 16, 256, 1, WHITE, false);
	drawRectangle(0, 17, 256, 175, DARK_GRAY, false);
	
	// Print path
	printText(path, 0, 0, false);

	// Print directory listing
	for(int i=0;i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		std::u16string name = StringUtils::UTF8toUTF16(dirContents[i + startRow].name);

		// Trim to fit on screen
		bool addEllipsis = false;
		while(getTextWidth(name) > 227) {
			name = name.substr(0, name.length()-1);
			addEllipsis = true;
		}
		if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

		printText(name, 10, i*16+16, false);
	}
}

#define verNumber "v0.5"

bool showTopMenuOnExit = true, noCardMessageSet = false;
int tmCurPos = 0, tmScreenOffset = 0, tmSlot1Offset = 0;

void updateDriveLabel(bool fat) {
	if (fat) {
		fatGetVolumeLabel("fat", fatLabel);
		for (int i = 0; i < 12; i++) {
			if (((fatLabel[i] == ' ') && (fatLabel[i+1] == ' ') && (fatLabel[i+2] == ' '))
			|| ((fatLabel[i] == ' ') && (fatLabel[i+1] == ' '))
			|| (fatLabel[i] == ' ')) {
				fatLabel[i] = '\0';
				break;
			}
		}
	} else {
		fatGetVolumeLabel("sd", sdLabel);
		for (int i = 0; i < 12; i++) {
			if (((sdLabel[i] == ' ') && (sdLabel[i+1] == ' ') && (sdLabel[i+2] == ' '))
			|| ((sdLabel[i] == ' ') && (sdLabel[i+1] == ' '))
			|| (sdLabel[i] == ' ')) {
				sdLabel[i] = '\0';
				break;
			}
		}
	}
}

void drawSdText(int i, bool valid) {
	char str[19];
	updateDriveLabel(false);
	snprintf(str, sizeof(str), "sd: (%s)", sdLabel[0] == '\0' ? "SD Card" : sdLabel);
	printTextTinted(str, valid ? WHITE : RED_RGB, 10, (i+1)*16, false);
}

void drawFatText(int i, bool valid) {
	char str[20];
	updateDriveLabel(true);
	snprintf(str, sizeof(str), "fat:/ (%s)", fatLabel[0] == '\0' ? "Flashcard" : fatLabel);
	printTextTinted(str, valid ? WHITE : RED_RGB, 10, (i+1)*16, false);
}

void drawSlot1Text(int i, bool valid) {
	char slot1Text[34];
	snprintf(slot1Text, sizeof(slot1Text), "Slot-1: (%s) [%s]", REG_SCFG_MC == 0x11 ? "No card inserted" : gamename, gameid);
	printTextTinted(slot1Text, valid ? WHITE : RED_RGB, 10, (i+1)*16, false);
}

bool updateSlot1Text(int &cardWait, bool valid) {
	if(REG_SCFG_MC == 0x11) {
		disableSlot1();
		cardWait = 30;
		if(!noCardMessageSet) {
			drawRectangle(10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, 200, 16, DARK_GRAY, false);
			printText("Slot-1: (No card inserted)", 10, ((tmSlot1Offset-tmScreenOffset)+1)*16, false);
			noCardMessageSet = true;
			return false;
		}
	}
	if(cardWait > 0) {
		cardWait--;
	} else if(cardWait == 0) {
		cardWait--;
		enableSlot1();
		if(updateCardInfo()) {
			valid = isValidTid(gameid);
			drawRectangle(10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, 200, 16, DARK_GRAY, false);
			drawSlot1Text(tmSlot1Offset-tmScreenOffset, valid);
			noCardMessageSet = false;
			return valid;
		}
	}
	return valid;
}

void showTopMenu(std::vector<topMenuItem> topMenuContents) {
	// Draw background
	drawRectangle(0, 0, 256, 15, BLACK, false);
	drawRectangle(0, 16, 256, 1, WHITE, false);
	drawRectangle(0, 17, 256, 175, DARK_GRAY, false);

	for(uint i=0;i<topMenuContents.size() && i<ENTRIES_PER_SCREEN;i++) {
		if(topMenuContents[i+tmScreenOffset].name == "fat:")	drawFatText(i, topMenuContents[i+tmScreenOffset].valid);
		else if(topMenuContents[i+tmScreenOffset].name == "sd:")	drawSdText(i, topMenuContents[i+tmScreenOffset].valid);
		else if(topMenuContents[i+tmScreenOffset].name == "card:")	drawSlot1Text(i, topMenuContents[i+tmScreenOffset].valid);
		else {
			std::u16string name = StringUtils::UTF8toUTF16(topMenuContents[i+tmScreenOffset].name);

			// Trim to fit on screen
			bool addEllipsis = false;
			while(getTextWidth(name) > 227) {
				name = name.substr(0, name.length()-1);
				addEllipsis = true;
			}
			if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

			printTextTinted(name, topMenuContents[i+tmScreenOffset].valid ? WHITE : RED_RGB, 10, i*16+16, false);
		}
	}
}

std::string topMenuSelect(void) {
	int pressed = 0, held = 0;

	// Clear top screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, true);

	// Print version number
	printText(verNumber, 256-getTextWidth(verNumber), 176, true);

	updateCardInfo();

	std::vector<topMenuItem> topMenuContents;

	if(flashcardFound())	topMenuContents.push_back({"fat:", true});
	if(sdFound())	topMenuContents.push_back({"sd:", true});
	topMenuContents.push_back({"card:", false});
	tmSlot1Offset = topMenuContents.size()-1;

	std::ifstream favs(sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst");
	std::string line;
	while(std::getline(favs, line)) {
		topMenuContents.push_back({line, (access(line.c_str(), F_OK) == 0)});
	}

	int cardWait = 0;
	topMenuContents[tmSlot1Offset].valid = updateSlot1Text(cardWait, topMenuContents[tmSlot1Offset].valid);
	
	// Show topMenuContents
	showTopMenu(topMenuContents);

	bool drawFullScreen = false;
	while(1) {
		// Clear old cursors
		drawRectangle(0, 17, 10, 175, DARK_GRAY, false);

		// Show cursor
		drawRectangle(3, (tmCurPos-tmScreenOffset)*16+24, 4, 3, WHITE, false);

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();

			if(tmScreenOffset <= tmSlot1Offset) {
				topMenuContents[tmSlot1Offset].valid = updateSlot1Text(cardWait, topMenuContents[tmSlot1Offset].valid);
			};

		} while(!held);

		if(held & KEY_UP)	tmCurPos -= 1;
		else if(held & KEY_DOWN)	tmCurPos += 1;
		else if(held & KEY_LEFT) {
			tmCurPos -= ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		} else if(held & KEY_RIGHT) {
			tmCurPos += ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		} else if(pressed & KEY_A) {
			if(topMenuContents[tmCurPos].name == "fat:") {
				chdir("fat:/");
				return "";
			} else if(topMenuContents[tmCurPos].name == "sd:") {
				chdir("sd:/");
				return "";
			} else if(topMenuContents[tmCurPos].name == "card:" && topMenuContents[tmSlot1Offset].valid) {
				dumpSave();
				showTopMenuOnExit = 1;
				return cardSave;
			} else if(topMenuContents[tmCurPos].valid) {
				showTopMenuOnExit = 1;
				return topMenuContents[tmCurPos].name;
			}
		} else if(pressed & KEY_X) {
			if((topMenuContents[tmCurPos].name != "fat:") && (topMenuContents[tmCurPos].name != "sd:") && (topMenuContents[tmCurPos].name != "card:")) {
				if(Input::getBool("Remove", "Cancel")) {
					std::ofstream out(sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst");

					std::string line;
					for(int i=0;i<(int)topMenuContents.size();i++) {
						if(i != tmCurPos && topMenuContents[i].name != "fat:" && topMenuContents[i].name != "sd:" && topMenuContents[i].name != "card:") {
							out << topMenuContents[i].name << std::endl;
						}
					}

					out.close();

					topMenuContents.erase(topMenuContents.begin()+tmCurPos);
				}
				showTopMenu(topMenuContents);
				drawFullScreen = true; // Stay at the bottom of the list
			}
		}

		if(tmCurPos < 0) {
			// Wrap around to bottom of list unless left was pressed
			tmCurPos = drawFullScreen ? 0 : topMenuContents.size()-1;
			drawFullScreen = true;
		} else if(tmCurPos > (int)topMenuContents.size()-1) {
			// Wrap around to top of list unless right was pressed
			tmCurPos = drawFullScreen ? topMenuContents.size()-1 : 0;
			drawFullScreen = true;
		}

		// Scroll screen if needed
		if(tmCurPos < tmScreenOffset) {
			tmScreenOffset = tmCurPos;
			if(drawFullScreen) {
				showTopMenu(topMenuContents);
			} else {
				// Copy old entries down
				for(int i=ENTRIES_PER_SCREEN-1;i>0;i--) {
					dmaCopyWords(0, BG_GFX_SUB+(((i*16)+1)*256), BG_GFX_SUB+((((i+1)*16)+1)*256), 16*256*2);
				}
				drawRectangle(10, 17, 246, 16, DARK_GRAY, false); // DARK_GRAY out previous top entry
				drawRectangle(3, 40, 4, 3, DARK_GRAY, false); // DARK_GRAY out previous cursor mark

				// Draw new entry
				if(topMenuContents[tmCurPos].name == "fat:")	drawFatText(0, topMenuContents[tmCurPos].valid);
				else if(topMenuContents[tmCurPos].name == "sd:")	drawSdText(0, topMenuContents[tmCurPos].valid);
				else if(topMenuContents[tmCurPos].name == "card:") drawSlot1Text(0, topMenuContents[tmCurPos].valid);
				else {
					std::u16string name = StringUtils::UTF8toUTF16(topMenuContents[tmScreenOffset].name);

					// Trim to fit on screen
					bool addEllipsis = false;
					while(getTextWidth(name) > 227) {
						name = name.substr(0, name.length()-1);
						addEllipsis = true;
					}
					if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

					printText(name, 10, 16, false);
				}
			}
		} else if(tmCurPos > tmScreenOffset + ENTRIES_PER_SCREEN - 1) {
			tmScreenOffset = tmCurPos - ENTRIES_PER_SCREEN + 1;
			if(drawFullScreen) {
				showTopMenu(topMenuContents);
			} else {
				dmaCopyWords(0, BG_GFX_SUB+(33*256), BG_GFX_SUB+(17*256), 160*256*2); // Copy old entries up
				drawRectangle(10, ENTRIES_PER_SCREEN*16, 246, 16, DARK_GRAY, false); // DARK_GRAY out previous bottom entry
				drawRectangle(3, 168, 4, 3, DARK_GRAY, false); // DARK_GRAY out previous cursor mark
				drawRectangle(3, (tmCurPos-tmScreenOffset)*16+24, 4, 3, WHITE, false); // Draw new cursor mark

				// Draw new entry
				if(topMenuContents[tmCurPos].name == "fat:")	drawFatText(ENTRIES_PER_SCREEN*16, topMenuContents[tmCurPos].valid);
				else if(topMenuContents[tmCurPos].name == "sd:")	drawSdText(ENTRIES_PER_SCREEN*16, topMenuContents[tmCurPos].valid);
				else if(topMenuContents[tmCurPos].name == "card:")	drawSlot1Text(ENTRIES_PER_SCREEN*16, topMenuContents[tmCurPos].valid);
				else {
					std::u16string name = StringUtils::UTF8toUTF16(topMenuContents[tmScreenOffset+ENTRIES_PER_SCREEN-1].name);

					// Trim to fit on screen
					bool addEllipsis = false;
					while(getTextWidth(name) > 227) {
						name = name.substr(0, name.length()-1);
						addEllipsis = true;
					}
					if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

					printText(name, 10, ENTRIES_PER_SCREEN*16, false);
				}
			}
		}
		drawFullScreen = 0;
	}
}

std::string browseForFile(const std::vector<std::string>& extensionList, bool directoryNavigation) {
	int pressed = 0, held = 0, screenOffset = 0, fileOffset = 0;
	bool drawFullScreen = false;
	std::vector<DirEntry> dirContents;

	getDirectoryContents(dirContents, extensionList);
	showDirectoryContents(dirContents, screenOffset);

	while(1) {
		// Clear old cursors
		drawRectangle(0, 17, 10, 175, DARK_GRAY, false);

		// Show cursor
		drawRectangle(3, (fileOffset-screenOffset)*16+24, 4, 3, WHITE, false);


		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP)	fileOffset -= 1;
		else if(held & KEY_DOWN)	fileOffset += 1;
		else if(held & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		} else if(held & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		}

		if(fileOffset < 0) {
			// Wrap around to bottom of list unless left was pressed
			fileOffset = drawFullScreen ? 0 : dirContents.size()-1;
			drawFullScreen = true;
		} else if(fileOffset > ((int)dirContents.size()-1)) {
			// Wrap around to top of list unless right was pressed
			fileOffset = drawFullScreen ? dirContents.size()-1 : 0;
			drawFullScreen = true;
		} else if(pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if(entry->isDirectory && directoryNavigation) {
				// Enter selected directory
				chdir(entry->name.c_str());
				getDirectoryContents(dirContents, extensionList);
				screenOffset = 0;
				fileOffset = 0;
				showDirectoryContents(dirContents, screenOffset);
			} else if(!entry->isDirectory) {
				// Return the chosen file
				return entry->name;
			}
		} else if(pressed & KEY_B && directoryNavigation) {
			// Go up a directory
			if((strcmp (path, "sd:/") == 0) || (strcmp (path, "fat:/") == 0)) {
				std::string str = topMenuSelect();
				if(str != "")	return str;
			} else {
				chdir("..");
			}
			getDirectoryContents(dirContents, extensionList);
			screenOffset = 0;
			fileOffset = 0;
			showDirectoryContents(dirContents, screenOffset);
		} else if(pressed & KEY_B && !directoryNavigation) {
			return "";
		} else if(pressed & KEY_Y && !dirContents[fileOffset].isDirectory && directoryNavigation) {
			if(loadSave(dirContents[fileOffset].name)) {
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);
				std::ofstream favs(sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst", std::fstream::app);
				favs << path << dirContents[fileOffset].name << std::endl;
				favs.close();
			}
		}

		// Scroll screen if needed
		if(fileOffset < screenOffset) {
			screenOffset = fileOffset;
			if(drawFullScreen) {
				showDirectoryContents(dirContents, screenOffset);
			} else {
				// Copy old entries down
				for(int i=ENTRIES_PER_SCREEN-1;i>0;i--) {
					dmaCopyWords(0, BG_GFX_SUB+(((i*16)+1)*256), BG_GFX_SUB+((((i+1)*16)+1)*256), 16*256*2);
				}
				drawRectangle(10, 17, 246, 16, DARK_GRAY, false); // DARK_GRAY out previous top entry
				drawRectangle(3, 40, 4, 3, DARK_GRAY, false); // DARK_GRAY out previous cursor mark

				std::u16string name = StringUtils::UTF8toUTF16(dirContents[screenOffset].name);

				// Trim to fit on screen
				bool addEllipsis = false;
				while(getTextWidth(name) > 227) {
					name = name.substr(0, name.length()-1);
					addEllipsis = true;
				}
				if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

				// Draw new entry
				printText(name, 10, 16, false);
			}
		} else if(fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			if(drawFullScreen) {
				showDirectoryContents(dirContents, screenOffset);
			} else {
				dmaCopyWords(0, BG_GFX_SUB+(33*256), BG_GFX_SUB+(17*256), 160*256*2); // Copy old entries up
				drawRectangle(10, ENTRIES_PER_SCREEN*16, 246, 16, DARK_GRAY, false); // DARK_GRAY out previous bottom entry
				drawRectangle(3, 168, 4, 3, DARK_GRAY, false); // DARK_GRAY out previous cursor mark
				drawRectangle(3, (fileOffset-screenOffset)*16+24, 4, 3, WHITE, false); // Draw new cursor mark

				std::u16string name = StringUtils::UTF8toUTF16(dirContents[screenOffset+ENTRIES_PER_SCREEN-1].name);

				// Trim to fit on screen
				bool addEllipsis = false;
				while(getTextWidth(name) > 227) {
					name = name.substr(0, name.length()-1);
					addEllipsis = true;
				}
				if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

				// Draw new entry
				printText(name, 10, ENTRIES_PER_SCREEN*16, false);
			}
		}
		drawFullScreen = false;
	}
}

std::string browseForSave(void) {
	if(showTopMenuOnExit) {
		showTopMenuOnExit = 0;
		std::string str = topMenuSelect();
		if(str != "")	return str;
	}

	// Clear top screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, true);

	// Print version number
	printText(verNumber, 256-getTextWidth(verNumber), 176, true);

	std::vector<std::string> extensionList;
	extensionList.push_back("sav");
	char sav[6];
	for(int i=1;i<10;i++) {
		snprintf(sav, sizeof(sav), "sav%i", i);
		extensionList.push_back(sav);
	}
	return browseForFile(extensionList, true);
}


int fcopy(const char *sourcePath, const char *destinationPath) {
	DIR *isDir = opendir(sourcePath);
	
	if(isDir == NULL) {
		closedir(isDir);

		// Source path is a file
		FILE* sourceFile = fopen(sourcePath, "rb");
		off_t fsize = 0;
		if(sourceFile) {
			fseek(sourceFile, 0, SEEK_END);
			fsize = ftell(sourceFile);			// Get source file's size
			fseek(sourceFile, 0, SEEK_SET);
		} else {
			fclose(sourceFile);
			return -1;
		}

	    FILE* destinationFile = fopen(destinationPath, "wb");
			fseek(destinationFile, 0, SEEK_SET);

		off_t offset = 0;
		int numr;
		while(1)
		{
			drawRectangle(((offset < fsize ? (double)offset/fsize : 1)*227)+5, 33, 19, 16, LIGHT_GRAY, false);
			// Copy file to destination path
			numr = fread(copyBuf, 2, copyBufSize, sourceFile);
			fwrite(copyBuf, 2, numr, destinationFile);
			offset += copyBufSize;


			if(offset > fsize) {
				fclose(sourceFile);
				fclose(destinationFile);

				return 1;
				break;
			}
		}

		return -1;
	} else {
		closedir(isDir);
		return -2;
	}
}
