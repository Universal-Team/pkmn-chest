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

#include "fileBrowse.hpp"
#include <algorithm>
#include <dirent.h>
#include <fat.h>
#include <strings.h>
#include <unistd.h>

#include "flashcard.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "langStrings.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "cardSaves.hpp"
#include "sound.hpp"
#include "utils.hpp"

#define ENTRIES_PER_SCREEN 11
#define ENTRY_PAGE_LENGTH 10
#define copyBufSize 0x8000

char path[PATH_MAX];
char fatLabel[12];
char sdLabel[12];
u32 copyBuf[copyBufSize];

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
			if(pent == NULL)	break;

			stat(pent->d_name, &st);
			dirEntry.name = pent->d_name;
			dirEntry.isDirectory = (st.st_mode & S_IFDIR) ? true : false;

			if(dirEntry.name.compare(".") != 0 && (dirEntry.isDirectory || nameEndsWith(dirEntry.name, extensionList))) {
				dirContents.push_back(dirEntry);
			}
		}
		closedir(pdir);
	}
	sort(dirContents.begin(), dirContents.end(), dirEntryPredicate);
}

void getDirectoryContents(std::vector<DirEntry>& dirContents) {
	getDirectoryContents(dirContents, {});
}

void showDirectoryContents(const std::vector<DirEntry>& dirContents, int startRow) {
	getcwd(path, PATH_MAX);

	// Print path
	drawImageFromSheet(0, 0, fileBrowseBgData.width, 17, fileBrowseBg, fileBrowseBgData.width, 0, 0, false);
	printTextMaxW(path, 250, 1, 5, 0, false);

	// Print directory listing
	for(int i=0;i < ENTRIES_PER_SCREEN; i++) {
		// Clear row
		drawImageFromSheet(10, i*16+16, 246, 16, fileBrowseBg, fileBrowseBgData.width, 10, i*16+16, false);

		if(i < ((int)dirContents.size() - startRow)) {
			std::u16string name = StringUtils::UTF8toUTF16(dirContents[i + startRow].name);

			// Trim to fit on screen
			bool addEllipsis = false;
			while(getTextWidth(name) > 227) {
				name = name.substr(0, name.length()-1);
				addEllipsis = true;
			}
			if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

			printTextTinted(name, GRAY, 10, i*16+16, false, true);
		}
	}
}

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
	printTextTinted(str, valid ? GRAY : RGB::RED, 10, (i+1)*16, false, true);
}

void drawFatText(int i, bool valid) {
	char str[20];
	updateDriveLabel(true);
	snprintf(str, sizeof(str), "fat: (%s)", fatLabel[0] == '\0' ? "Flashcard" : fatLabel);
	printTextTinted(str, valid ? GRAY : RGB::RED, 10, (i+1)*16, false, true);
}

void drawSlot1Text(int i, bool valid) {
	char slot1Text[34];
	snprintf(slot1Text, sizeof(slot1Text), "Slot-1: (%s) [%s]", REG_SCFG_MC == 0x11 ? "No card inserted" : gamename, gameid);
	printTextTinted(slot1Text, valid ? GRAY : RGB::RED, 10, (i+1)*16, false, true);
}

bool updateSlot1Text(int &cardWait, bool valid) {
	if(REG_SCFG_MC == 0x11) {
		disableSlot1();
		cardWait = 30;
		if(!noCardMessageSet) {
			drawImageFromSheet(10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, 200, 16, fileBrowseBg, fileBrowseBgData.width, 10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, false);
			printTextTinted("Slot-1: (No card inserted)", GRAY, 10, ((tmSlot1Offset-tmScreenOffset)+1)*16, false, true);
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
			drawImageFromSheet(10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, 200, 16, fileBrowseBg, fileBrowseBgData.width, 10, ((tmSlot1Offset-tmScreenOffset)+1)*16+1, false);
			drawSlot1Text(tmSlot1Offset-tmScreenOffset, valid);
			noCardMessageSet = false;
			return valid;
		}
	}
	return valid;
}

void showTopMenu(std::vector<topMenuItem> topMenuContents) {
	for(unsigned i=0;i<ENTRIES_PER_SCREEN;i++) {
		// Clear row
		drawImageFromSheet(10, i*16+16, 246, 16, fileBrowseBg, fileBrowseBgData.width, 10, i*16+16, false);

		if(i<topMenuContents.size()) {
			if(topMenuContents[i+tmScreenOffset].name == "fat:")	drawFatText(i, topMenuContents[i+tmScreenOffset].valid);
			else if(topMenuContents[i+tmScreenOffset].name == "sd:")	drawSdText(i, topMenuContents[i+tmScreenOffset].valid);
			else if(topMenuContents[i+tmScreenOffset].name == "card:")	drawSlot1Text(i, topMenuContents[i+tmScreenOffset].valid);
			else {
				std::u16string name = StringUtils::UTF8toUTF16(topMenuContents[i+tmScreenOffset].name);
				name = name.substr(name.find_last_of(StringUtils::UTF8toUTF16("/"))+1); // Remove path to the file

				// Trim to fit on screen
				bool addEllipsis = false;
				while(getTextWidth(name) > 227) {
					name = name.substr(0, name.length()-1);
					addEllipsis = true;
				}
				if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

				printTextTinted(name, topMenuContents[i+tmScreenOffset].valid ? GRAY : RGB::RED, 10, i*16+16, false, true);
			}
		}
	}
}

std::string topMenuSelect(void) {
	int pressed = 0, held = 0;
	touchPosition touch;

	// Clear screens
	drawImage(0, 0, boxBgTopData.width, boxBgTopData.height, boxBgTop, true);
	drawImage(0, 0, fileBrowseBgData.width, fileBrowseBgData.height, fileBrowseBg, false);

	// Print version number
	printText(VER_NUMBER, 256-getTextWidth(VER_NUMBER)-1, 176, true);

	updateCardInfo();

	std::vector<topMenuItem> topMenuContents;

	if(flashcardFound())	topMenuContents.push_back({"fat:", true});
	if(sdFound())	topMenuContents.push_back({"sd:", true});
	if(!flashcardFound())	topMenuContents.push_back({"card:", false});
	if(!flashcardFound())	tmSlot1Offset = topMenuContents.size()-1;

	FILE* favs = fopen((sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst"), "rb");

	if(favs) {
		char* line = NULL;
		size_t len = 0;

		while(__getline(&line, &len, favs) != -1) {
			line[strlen(line)-1] = '\0'; // Remove newline
			topMenuContents.push_back({line, (access(line, F_OK) == 0)});
		}
	}

	int cardWait = 0;
	if(!flashcardFound())	topMenuContents[tmSlot1Offset].valid = updateSlot1Text(cardWait, topMenuContents[tmSlot1Offset].valid);

	// Show topMenuContents
	showTopMenu(topMenuContents);

	bool bigJump = false;
	while(1) {
		// Clear old cursors
		drawImageFromSheet(0, 17, 10, 175, fileBrowseBg, fileBrowseBgData.width, 0, 17, false);

		// Draw cursor
		drawRectangle(3, (tmCurPos-tmScreenOffset)*16+24, 4, 3, DARK_GRAY, false);

		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();

			if(!flashcardFound()) {
				if(tmScreenOffset <= tmSlot1Offset) {
					topMenuContents[tmSlot1Offset].valid = updateSlot1Text(cardWait, topMenuContents[tmSlot1Offset].valid);
				}
			}

		} while(!held);

		if(held & KEY_UP)	tmCurPos -= 1;
		else if(held & KEY_DOWN)	tmCurPos += 1;
		else if(held & KEY_LEFT) {
			tmCurPos -= ENTRY_PAGE_LENGTH;
			bigJump = true;
		} else if(held & KEY_RIGHT) {
			tmCurPos += ENTRY_PAGE_LENGTH;
			bigJump = true;
		} else if(pressed & KEY_A) {
			selection:
			if(topMenuContents[tmCurPos].name == "fat:") {
				chdir("fat:/");
				return "";
			} else if(topMenuContents[tmCurPos].name == "sd:") {
				chdir("sd:/");
				return "";
			} else if(topMenuContents[tmCurPos].name == "card:" && topMenuContents[tmSlot1Offset].valid) {
				Sound::play(Sound::click);
				dumpSave();
				showTopMenuOnExit = 1;
				return cardSave;
			} else if(topMenuContents[tmCurPos].valid) {
				Sound::play(Sound::click);
				showTopMenuOnExit = 1;
				return topMenuContents[tmCurPos].name;
			}
		} else if(pressed & KEY_X) {
			Sound::play(Sound::click);
			if((topMenuContents[tmCurPos].name != "fat:") && (topMenuContents[tmCurPos].name != "sd:") && (topMenuContents[tmCurPos].name != "card:")) {
				if(Input::getBool(Lang::remove, Lang::cancel)) {
					topMenuContents.erase(topMenuContents.begin()+tmCurPos);

					FILE* out = fopen((sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst"), "wb");

					if(out) {
						for(int i=0;i<(int)topMenuContents.size();i++) {
							if(topMenuContents[i].name != "fat:" && topMenuContents[i].name != "sd:" && topMenuContents[i].name != "card:") {
								fwrite((topMenuContents[i].name+"\n").c_str(), 1, (topMenuContents[i].name+"\n").size(), out);
							}
						}
					}

					fclose(out);
				}
				if(tmCurPos > (int)topMenuContents.size()-1) {
					tmCurPos = topMenuContents.size()-1;
					tmScreenOffset = std::max(tmCurPos - ENTRIES_PER_SCREEN + 1, 0);
				}
				showTopMenu(topMenuContents);
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<std::min(ENTRIES_PER_SCREEN, (int)topMenuContents.size());i++) {
				if(touch.py > (i+1)*16 && touch.py < (i+2)*16) {
					tmCurPos = i;
					goto selection;
				}
			}
		}

		if(tmCurPos < 0) {
			// Wrap around to bottom of list unless left was pressed
			tmCurPos = bigJump ? 0 : topMenuContents.size()-1;
		} else if(tmCurPos > (int)topMenuContents.size()-1) {
			// Wrap around to top of list unless right was pressed
			tmCurPos = bigJump ? topMenuContents.size()-1 : 0;
		}
		bigJump = false;

		// Scroll screen if needed
		if(tmCurPos < tmScreenOffset) {
			tmScreenOffset = tmCurPos;
			showTopMenu(topMenuContents);
		} else if(tmCurPos > tmScreenOffset + ENTRIES_PER_SCREEN - 1) {
			tmScreenOffset = tmCurPos - ENTRIES_PER_SCREEN + 1;
			showTopMenu(topMenuContents);
		}

		if(held & KEY_UP || held & KEY_DOWN || held & KEY_LEFT || held & KEY_RIGHT || pressed & KEY_X) {
			// Clear the path area of the screen
			drawImage(0, 0, fileBrowseBgData.width, 17, fileBrowseBg, false);

			// Print the path to the currently selected file
			std::u16string path = StringUtils::UTF8toUTF16(topMenuContents[tmCurPos].name);
			path = path.substr(0, path.find_last_of(StringUtils::UTF8toUTF16("/"))+1); // Cut to just the path
			printTextMaxW(path, 250, 1, 5, 0, false);
		}
	}
}

std::string browseForFile(const std::vector<std::string>& extensionList, bool accessSubdirectories) {
	char startPath[PATH_MAX];
	if(!accessSubdirectories)	getcwd(startPath, PATH_MAX);
	int pressed = 0, held = 0, screenOffset = 0, fileOffset = 0;
	touchPosition touch;
	bool bigJump = false;
	std::vector<DirEntry> dirContents;

	getDirectoryContents(dirContents, extensionList);
	showDirectoryContents(dirContents, screenOffset);

	while(1) {
		// Clear old cursors
		drawImageFromSheet(0, 17, 10, 175, fileBrowseBg, fileBrowseBgData.width, 0, 17, false);

		// Draw cursor
		drawRectangle(3, (fileOffset-screenOffset)*16+24, 4, 3, DARK_GRAY, false);


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
			bigJump = true;
		} else if(held & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			bigJump = true;
		}

		if(fileOffset < 0) {
			// Wrap around to bottom of list unless left was pressed
			fileOffset = bigJump ? 0 : dirContents.size()-1;
			bigJump = true;
		} else if(fileOffset > ((int)dirContents.size()-1)) {
			// Wrap around to top of list unless right was pressed
			fileOffset = bigJump ? dirContents.size()-1 : 0;
			bigJump = true;
		} else if(pressed & KEY_A) {
			selection:
			DirEntry* entry = &dirContents.at(fileOffset);
			if(entry->isDirectory) {
				// Don't go up directory if in the start directory
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);
				if(!accessSubdirectories && entry->name == ".." && (strcmp(startPath, path) == 0))	continue;

				// Enter selected directory
				chdir(entry->name.c_str());
				getDirectoryContents(dirContents, extensionList);
				screenOffset = 0;
				fileOffset = 0;
				showDirectoryContents(dirContents, screenOffset);
			} else if(!entry->isDirectory) {
				Sound::play(Sound::click);
				// Return the chosen file
				return entry->name;
			}
		} else if(pressed & KEY_B) {
			// Don't go up directory if in the start directory
			char path[PATH_MAX];
			getcwd(path, PATH_MAX);
			if(!accessSubdirectories && (strcmp(startPath, path) == 0)) {
				Sound::play(Sound::back);
				return "";
			}

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
		} else if(pressed & KEY_Y && !dirContents[fileOffset].isDirectory && accessSubdirectories) { // accessSubdirectory check is a hack to make it not trigger except in save selection
			if(loadSave(dirContents[fileOffset].name)) {
				Sound::play(Sound::click);
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);

				FILE* favs = fopen((sdFound() ? "sd:/_nds/pkmn-chest/favorites.lst" : "fat:/_nds/pkmn-chest/favorites.lst"), "ab");

				if(favs) {
					fwrite((path+dirContents[fileOffset].name+"\n").c_str(), 1, (path+dirContents[fileOffset].name+"\n").size(), favs);
				}

				fclose(favs);
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<std::min(ENTRIES_PER_SCREEN, (int)dirContents.size());i++) {
				if(touch.py > (i+1)*16 && touch.py < (i+2)*16) {
					fileOffset = i;
					goto selection;
				}
			}
		}

		// Scroll screen if needed
		if(fileOffset < screenOffset) {
			screenOffset = fileOffset;
			showDirectoryContents(dirContents, screenOffset);
		} else if(fileOffset > screenOffset + ENTRIES_PER_SCREEN - 1) {
			screenOffset = fileOffset - ENTRIES_PER_SCREEN + 1;
			showDirectoryContents(dirContents, screenOffset);
		}
		bigJump = false;
	}
}

std::string browseForSave(void) {
	if(showTopMenuOnExit) {
		showTopMenuOnExit = 0;
		std::string str = topMenuSelect();
		if(str != "")	return str;
	}

	// Clear screens
	drawImage(0, 0, boxBgTopData.width, boxBgTopData.height, boxBgTop, true);
	drawImage(0, 0, fileBrowseBgData.width, fileBrowseBgData.height, fileBrowseBg, false);

	// Print version number
	printText(VER_NUMBER, 256-getTextWidth(VER_NUMBER)-1, 176, true);

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
		while(1) {
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
