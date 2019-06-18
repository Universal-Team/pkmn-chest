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

#include "flashcard.h"
#include "graphics/colors.h"
#include "graphics/graphics.h"
#include "saves/cardSaves.h"
#include "utils.hpp"

#define ENTRIES_PER_SCREEN 11
#define ENTRY_PAGE_LENGTH 10

char path[PATH_MAX];

struct DirEntry {
	std::string name;
	bool isDirectory;
};

bool nameEndsWith(const std::string& name, const std::vector<std::string> extensionList) {
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

#define verNumber "v0.1"

bool showDriveSelectOnBoot = true;

uint driveSelect_cursorPosition = 0;

std::string driveSelect(void) {
	int pressed = 0;

	// Clear top screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, true);

	// Print version number
	printText(verNumber, 256-getTextWidth(StringUtils::UTF8toUTF16(verNumber)), 176, true);

	// Draw background
	drawRectangle(0, 0, 256, 15, BLACK, false);
	drawRectangle(0, 16, 256, 1, WHITE, false);
	drawRectangle(0, 17, 256, 175, DARK_GRAY, false);

	updateCardInfo();

	std::vector<std::string> drives;
	char slot1Text[28];
	int slot1TextY = 0;

	// TODO: Implement favorite saves and add them to this
	if(flashcardFound())	drives.push_back("fat:");
	if(sdFound())	drives.push_back("sd:");
	drives.push_back("card:");
	
	// Show drives
	for(uint i=0;i<drives.size();i++) {
		if(drives[i] != "card:")	printText(drives[i], 10, ((i+1)*16), false);
		else {
			slot1TextY = ((i+1)*16);
			if(REG_SCFG_MC == 0x11) {
				printText("Slot-1: No card inserted", 10, slot1TextY, false);
			} else {
				snprintf(slot1Text, sizeof(slot1Text), "Slot-1: %s [%s]", gamename, gameid);
				printText(slot1Text, 10, slot1TextY, false);
			}
		}
	}

	int cardWait = 30;
	bool noCardMessageSet = 0;
	while(1) {
		// Clear old cursors
		drawRectangle(0, 17, 10, 175, DARK_GRAY, false);

		// Show cursor
		drawRectangle(3, driveSelect_cursorPosition*16+24, 4, 3, WHITE, false);

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDownRepeat();

			if(REG_SCFG_MC == 0x11) {
				disableSlot1();
				cardWait = 30;
				if(!noCardMessageSet) {
					drawRectangle(10, slot1TextY, 200, 16, DARK_GRAY, false);
					printText("Slot-1: No card inserted", 10, slot1TextY, false);
					noCardMessageSet = 1;
				}
			}
			if(cardWait > 0) {
				cardWait--;
			} else if(cardWait == 0) {
				cardWait--;
				enableSlot1();
				if(updateCardInfo()) {
					snprintf(slot1Text, sizeof(slot1Text), "Slot-1: %s [%s]", gamename, gameid);
					drawRectangle(10, slot1TextY, 200, 16, DARK_GRAY, false);
					printText(slot1Text, 10, slot1TextY, false);
					noCardMessageSet = 0;
				}
			}
		} while(!pressed);

		if(pressed & KEY_UP)	driveSelect_cursorPosition--;
		else if(pressed & KEY_DOWN)	driveSelect_cursorPosition++;

		if(driveSelect_cursorPosition < 0) driveSelect_cursorPosition = drives.size()-1;
		if(driveSelect_cursorPosition > drives.size()-1) driveSelect_cursorPosition = 0;

		if(pressed & KEY_A) {
			if(drives[driveSelect_cursorPosition] == "fat:") {
				chdir("fat:/");
			} else if(drives[driveSelect_cursorPosition] == "sd:") {
				chdir("sd:/");
			} else if(drives[driveSelect_cursorPosition] == "card:") {
				dumpSave();
				return cardSave;
			}
			return "";
		}
	}
}

std::string browseForFile(const std::vector<std::string>& extensionList) {
	{
		std::string drive = driveSelect();
		if(drive != "")	return drive;
	}

	int pressed = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	bool drawFullScreen = false;
	std::vector<DirEntry> dirContents;

	getDirectoryContents(dirContents, extensionList);
	showDirectoryContents(dirContents, screenOffset);

	// Clear top screen
	drawRectangle(0, 0, 256, 192, DARK_GRAY, true);

	// Print version number
	printText(verNumber, 256-getTextWidth(StringUtils::UTF8toUTF16(verNumber)), 176, true);

	while(1) {
		// Clear old cursors
		drawRectangle(0, 17, 10, 175, DARK_GRAY, false);

		// Show cursor
		drawRectangle(3, (fileOffset-screenOffset)*16+24, 4, 3, WHITE, false);


		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDownRepeat();
		} while(!pressed);

		if(pressed & KEY_UP)	fileOffset -= 1;
		else if(pressed & KEY_DOWN)	fileOffset += 1;
		else if(pressed & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		} else if(pressed & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			drawFullScreen = true;
		}

		if(fileOffset < 0) {
			// Wrap around to bottom of list unless left was pressed
			fileOffset = drawFullScreen ? 0 : dirContents.size()-1;
			drawFullScreen = true;
		} else if(fileOffset >((int)dirContents.size() - 1)) {
			// Wrap around to top of list unless right was pressed
			fileOffset = drawFullScreen ? dirContents.size()-1 : 0;
			drawFullScreen = true;
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
		}
		if(fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
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

		if(pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if(entry->isDirectory) {
				// Enter selected directory
				chdir(entry->name.c_str());
				getDirectoryContents(dirContents, extensionList);
				screenOffset = 0;
				fileOffset = 0;
				showDirectoryContents(dirContents, screenOffset);
			} else {
				// Return the chosen file
				return entry->name;
			}
		}

		if(pressed & KEY_B) {
			// Go up a directory
			if((strcmp (path, "sd:/") == 0) || (strcmp (path, "fat:/") == 0)) {
				std::string drive = driveSelect();
				if(drive != "")	return drive;
			} else {
				chdir("..");
			}
			getDirectoryContents(dirContents, extensionList);
			screenOffset = 0;
			fileOffset = 0;
			showDirectoryContents(dirContents, screenOffset);
		}
	}
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
			u32 copyBuf[0x8000];

			// Copy file to destination path
			numr = fread(copyBuf, 2, sizeof(copyBuf), sourceFile);
			fwrite(copyBuf, 2, numr, destinationFile);
			offset += sizeof(copyBuf);

			if(offset > fsize) {
				fclose(sourceFile);
				fclose(destinationFile);

				for(int i = 0; i < 30; i++) swiWaitForVBlank();

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
