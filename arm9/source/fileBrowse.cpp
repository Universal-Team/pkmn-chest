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
#include <cstdio>
#include <dirent.h>
#include <nds.h>
#include <string>
#include <unistd.h>
#include <vector>

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 22
#define ENTRIES_START_ROW 2
#define ENTRY_PAGE_LENGTH 10

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
		iprintf("Unable to open the directory.\n");
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
	char path[PATH_MAX];

	getcwd(path, PATH_MAX);

	// Clear the screen
	iprintf("\x1b[2J");

	// Print the path
	if(strlen(path) < SCREEN_COLS) {
		iprintf("%s", path);
	} else {
		iprintf("%s", path + strlen(path) - SCREEN_COLS);
	}

	// Move to 2nd row and print line of dashes
	iprintf("\x1b[1;0H");
	iprintf("--------------------------------");

	// Print directory listing
	for(int i=0;i < ((int)dirContents.size() - startRow) && i < ENTRIES_PER_SCREEN; i++) {
		const DirEntry* entry = &dirContents.at(i + startRow);
		char entryName[SCREEN_COLS + 1];

		// Set row
		iprintf("\x1b[%d;0H", i + ENTRIES_START_ROW);

		if(entry->isDirectory) {
			strncpy(entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 3] = '\0';
			iprintf(" [%s]", entryName);
		} else {
			strncpy(entryName, entry->name.c_str(), SCREEN_COLS);
			entryName[SCREEN_COLS - 1] = '\0';
			iprintf(" %s", entryName);
		}
	}
}

std::string browseForFile(const std::vector<std::string>& extensionList) {
	int pressed = 0;
	int screenOffset = 0;
	int fileOffset = 0;
	std::vector<DirEntry> dirContents;

	getDirectoryContents(dirContents, extensionList);
	showDirectoryContents(dirContents, screenOffset);

	while(true) {
		// Clear old cursors
		for(int i = ENTRIES_START_ROW; i < ENTRIES_PER_SCREEN + ENTRIES_START_ROW; i++) {
			iprintf("\x1b[%d;0H ", i);
		}
		// Show cursor
		iprintf("\x1b[%d;0H*", fileOffset - screenOffset + ENTRIES_START_ROW);

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			scanKeys();
			pressed = keysDownRepeat();
			swiWaitForVBlank();
		} while(!pressed);

		if(pressed & KEY_UP)	fileOffset -= 1;
		if(pressed & KEY_DOWN)	fileOffset += 1;
		if(pressed & KEY_LEFT)	fileOffset -= ENTRY_PAGE_LENGTH;
		if(pressed & KEY_RIGHT)	fileOffset += ENTRY_PAGE_LENGTH;

		if(fileOffset < 0) 	fileOffset = dirContents.size() - 1;		// Wrap around to bottom of list
		if(fileOffset >((int)dirContents.size() - 1))		fileOffset = 0;		// Wrap around to top of list

		// Scroll screen if needed
		if(fileOffset < screenOffset) {
			screenOffset = fileOffset;
			showDirectoryContents(dirContents, screenOffset);
		}
		if(fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			showDirectoryContents(dirContents, screenOffset);
		}

		if(pressed & KEY_A) {
			DirEntry* entry = &dirContents.at(fileOffset);
			if(entry->isDirectory) {
				iprintf("Entering directory\n");
				// Enter selected directory
				chdir(entry->name.c_str());
				getDirectoryContents(dirContents, extensionList);
				screenOffset = 0;
				fileOffset = 0;
				showDirectoryContents(dirContents, screenOffset);
			} else {
				// Clear the screen
				iprintf("\x1b[2J");
				// Return the chosen file
				return entry->name;
			}
		}

		if(pressed & KEY_B) {
			// Go up a directory
			chdir("..");
			getDirectoryContents(dirContents, extensionList);
			screenOffset = 0;
			fileOffset = 0;
			showDirectoryContents(dirContents, screenOffset);
		}
	}
}
