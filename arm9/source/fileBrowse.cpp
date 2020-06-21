#include "fileBrowse.hpp"
#include <algorithm>
#include <dirent.h>
#include <fat.h>
#include <strings.h>
#include <unistd.h>

#include "colors.hpp"
#include "config.hpp"
#include "configMenu.hpp"
#include "flashcard.hpp"
#include "graphics.hpp"
#include "i18n.hpp"
#include "i18n_ext.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "cardSaves.hpp"
#include "cartSaves.hpp"
#include "sound.hpp"
#include "utils.hpp"
#include "version.hpp"

#define ENTRIES_PER_SCREEN 11
#define ENTRY_PAGE_LENGTH 10
#define copyBufSize 0x8000

char fatLabel[14];
char sdLabel[14];
u32 copyBuf[copyBufSize];

struct topMenuItem {
	std::string name;
	bool valid;
};

bool nameEndsWith(const std::string &name, const std::vector<std::string> &extensionList) {
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

void getDirectoryContents(std::vector<DirEntry> &dirContents, const std::vector<std::string> &extensionList) {
	struct stat st;

	dirContents.clear();

	DIR *pdir = opendir(".");

	if(pdir == NULL) {
		printText("Unable to open the directory.", 0, 0, false, true);
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
	char path[PATH_MAX];
	getcwd(path, PATH_MAX);

	// Print path
	drawRectangle(0, 0, 256, 16, CLEAR, false, true);
	printTextMaxW(path, 250, 1, 4, 0, false, true);

	// Print directory listing
	for(int i=0;i < ENTRIES_PER_SCREEN; i++) {
		// Clear row
		drawRectangle(10, (i+1)*16, 246, 16, CLEAR, false, true);

		if(i < ((int)dirContents.size() - startRow)) {
			std::u16string name = StringUtils::UTF8toUTF16(dirContents[i + startRow].name);

			// Trim to fit on screen
			bool addEllipsis = false;
			while(getTextWidth(name) > 227) {
				name = name.substr(0, name.length()-1);
				addEllipsis = true;
			}
			if(addEllipsis)	name += StringUtils::UTF8toUTF16("...");

			printText(name, 10, i*16+16, false, true);
		}
	}
}

bool showTopMenuOnExit = true, noCardMessageSet = false, noCartMessageSet = false;
int tmCurPos = 0, tmScreenOffset = 0, tmSlot1Offset = 0, tmSlot2Offset = 0;

void updateDriveLabel(bool fat) {
	char *label = fat ? fatLabel : sdLabel;
	fatGetVolumeLabel(fat ? "fat" : "sd", label);
	for(int i = strlen(label) - 1; i > 0; i--) {
		if(label[i] != ' ') {
			label[i + 1] = '\0';
			break;
		}
	}
}

void drawSdText(int i, bool valid) {
	char str[20];
	updateDriveLabel(false);
	snprintf(str, sizeof(str), "sd: (%s)", sdLabel[0] == '\0' ? "SD Card" : sdLabel);
	printTextTinted(str, valid ? TextColor::white : TextColor::red, 10, (i+1)*16, false, true);
}

void drawFatText(int i, bool valid) {
	char str[21];
	updateDriveLabel(true);
	snprintf(str, sizeof(str), "fat: (%s)", fatLabel[0] == '\0' ? "Flashcard" : fatLabel);
	printTextTinted(str, valid ? TextColor::white : TextColor::red, 10, (i+1)*16, false, true);
}

void drawSlot1Text(int i, bool valid) {
	char slot1Text[34];
	snprintf(slot1Text, sizeof(slot1Text), "Slot-1: (%s) [%s]", REG_SCFG_MC == 0x11 ? "No card inserted" : slot1Name, slot1ID);
	printTextTinted(slot1Text, valid ? TextColor::white : TextColor::red, 10, (i+1)*16, false, true);
}

void drawSlot2Text(int i, bool valid) {
	char slot2Text[34];
	snprintf(slot2Text, sizeof(slot2Text), "Slot-2: (%s) [%s]", *(u8*)(0x080000B2) != 0x96 ? "No cart inserted" : slot2Name, slot2ID);
	printTextTinted(slot2Text, valid ? TextColor::white : TextColor::red, 10, (i+1)*16, false, true);
}

bool updateSlot1Text(int &cardWait, bool valid) {
	if(REG_SCFG_MC == 0x11) {
		disableSlot1();
		cardWait = 30;
		if(!noCardMessageSet) {
			drawRectangle(10, ((tmSlot1Offset-tmScreenOffset)+1)*16, 246, 16, CLEAR, false, true);
			printTextTinted("Slot-1: (No card inserted)", TextColor::red, 10, ((tmSlot1Offset-tmScreenOffset)+1)*16, false, true);
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
			valid = isValidDSTid(slot1ID);
			drawRectangle(10, ((tmSlot1Offset-tmScreenOffset)+1)*16, 246, 16, CLEAR, false, true);
			drawSlot1Text(tmSlot1Offset-tmScreenOffset, valid);
			noCardMessageSet = false;
			return valid;
		}
	}
	return valid;
}

bool updateSlot2Text(int &cardWait, bool valid) {
	if(*(u8*)(0x080000B2) != 0x96) {
		cardWait = 30;
		if(!noCartMessageSet) {
			drawRectangle(10, ((tmSlot2Offset-tmScreenOffset)+1)*16, 246, 16, CLEAR, false, true);
			printTextTinted("Slot-2: (No cart inserted)", TextColor::red, 10, ((tmSlot2Offset-tmScreenOffset)+1)*16, false, true);
			noCartMessageSet = true;
			return false;
		}
	}
	if(cardWait > 0) {
		cardWait--;
	} else if(cardWait == 0) {
		cardWait--;
		if(updateCartInfo()) {
			valid = isValidGBATid(slot2ID);
			drawRectangle(10, ((tmSlot2Offset-tmScreenOffset)+1)*16, 246, 16, CLEAR, false, true);
			drawSlot2Text(tmSlot2Offset-tmScreenOffset, valid);
			noCartMessageSet = false;
			return valid;
		}
	}
	return valid;
}

void showTopMenu(std::vector<topMenuItem> topMenuContents) {
	for(unsigned i=0;i<ENTRIES_PER_SCREEN;i++) {
		// Clear row
		drawRectangle(10, (i+1)*16, 246, 16, CLEAR, false, true);

		if(i<topMenuContents.size()) {
			if(topMenuContents[i+tmScreenOffset].name == "fat:")	drawFatText(i, topMenuContents[i+tmScreenOffset].valid);
			else if(topMenuContents[i+tmScreenOffset].name == "sd:")	drawSdText(i, topMenuContents[i+tmScreenOffset].valid);
			else if(topMenuContents[i+tmScreenOffset].name == "card:")	drawSlot1Text(i, topMenuContents[i+tmScreenOffset].valid);
			else if(topMenuContents[i+tmScreenOffset].name == "cart:")	drawSlot2Text(i, topMenuContents[i+tmScreenOffset].valid);
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

				printTextTinted(name, topMenuContents[i+tmScreenOffset].valid ? TextColor::white : TextColor::red, 10, i*16+16, false, true);
			}
		}
	}
}

std::string topMenuSelect(void) {
	int pressed, held;
	touchPosition touch;

	// Draw backgrounds
	drawImageDMA(0, 0, boxBgTop, true, false);
	drawImageDMA(0, 0, listBg, false, false);

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, true, true);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Print version number
	printTextScaled(VER_NUMBER, wideScreen ? 0.8 : 1, 1, 256-getTextWidthScaled(VER_NUMBER, wideScreen ? 0.8 : 1)-1, 176, true, true);

	if(!flashcardFound())	updateCardInfo();

	std::vector<topMenuItem> topMenuContents;

	if(flashcardFound())	topMenuContents.push_back({"fat:", true});
	if(sdFound())	topMenuContents.push_back({"sd:", true});
	if(!flashcardFound())	topMenuContents.push_back({"card:", false});
	if(!flashcardFound())	tmSlot1Offset = topMenuContents.size()-1;
	if(flashcardFound())	topMenuContents.push_back({"cart:", false});
	if(flashcardFound())	tmSlot2Offset = topMenuContents.size()-1;

	FILE* favs = fopen((mainDrive() + ":/_nds/pkmn-chest/favorites.lst").c_str(), "rb");

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
	if(flashcardFound())	topMenuContents[tmSlot2Offset].valid = updateSlot2Text(cardWait, topMenuContents[tmSlot2Offset].valid);

	// Show topMenuContents
	showTopMenu(topMenuContents);

	while(1) {
		// Clear old cursors
		drawRectangle(0, 16, 10, 176, CLEAR, false, true);

		// Draw cursor
		drawRectangle(3, (tmCurPos-tmScreenOffset)*16+24, 4, 3, LIGHT_GRAY, false, true);

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
			} else if(flashcardFound()) {
				if(tmScreenOffset <= tmSlot2Offset) {
					topMenuContents[tmSlot2Offset].valid = updateSlot2Text(cardWait, topMenuContents[tmSlot2Offset].valid);
				}
			}
		} while(!held);

		if(held & KEY_UP) {
			if(tmCurPos > 0)	tmCurPos -= 1;
			else	tmCurPos = topMenuContents.size()-1;
		} else if(held & KEY_DOWN) {
			if(tmCurPos < (int)topMenuContents.size()-1)	tmCurPos += 1;
			else	tmCurPos = 0;
		} else if(held & KEY_LEFT) {
			tmCurPos -= ENTRY_PAGE_LENGTH;
			if(tmCurPos < 0)	tmCurPos = 0;
		} else if(held & KEY_RIGHT) {
			tmCurPos += ENTRY_PAGE_LENGTH;
			if(tmCurPos > (int)topMenuContents.size()-1)	tmCurPos = topMenuContents.size()-1;
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
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				printTextCentered(i18n::localize(Config::getLang("lang"), "dumpingSave"), 0, 50, false, false);
				dumpSlot1();
				showTopMenuOnExit = 1;
				return cardSave;
			} else if(topMenuContents[tmCurPos].name == "cart:" && topMenuContents[tmSlot2Offset].valid) {
				Sound::play(Sound::click);
				drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				printTextCentered(i18n::localize(Config::getLang("lang"), "dumpingSave"), 0, 50, false, false);
				dumpSlot2();
				showTopMenuOnExit = 1;
				return cartSave;
			} else if(topMenuContents[tmCurPos].valid) {
				Sound::play(Sound::click);
				DIR *pdir = opendir(topMenuContents[tmCurPos].name.c_str());
				if(pdir) {
					closedir(pdir);
					chdir(topMenuContents[tmCurPos].name.c_str());
					return "";
				} else {
					showTopMenuOnExit = 1;
					return topMenuContents[tmCurPos].name;
				}
			}
		} else if(pressed & KEY_X) {
			Sound::play(Sound::click);
			if(topMenuContents[tmCurPos].name != "fat:"
			&& topMenuContents[tmCurPos].name != "sd:"
			&& topMenuContents[tmCurPos].name != "card:"
			&& topMenuContents[tmCurPos].name != "cart:") {
				if(Input::getBool(i18n::localize(Config::getLang("lang"), "remove"), i18n::localize(Config::getLang("lang"), "cancel"))) {
					topMenuContents.erase(topMenuContents.begin()+tmCurPos);

					FILE* out = fopen((mainDrive() + ":/_nds/pkmn-chest/favorites.lst").c_str(), "wb");

					if(out) {
						for(int i=0;i<(int)topMenuContents.size();i++) {
							if(topMenuContents[i].name != "fat:"
							&& topMenuContents[i].name != "sd:"
							&& topMenuContents[i].name != "card:"
							&& topMenuContents[i].name != "cart:") {
								fwrite((topMenuContents[i].name+"\n").c_str(), 1, (topMenuContents[i].name+"\n").size(), out);
							}
						}
						fclose(out);
					}
				}
				if(tmCurPos > (int)topMenuContents.size()-1) {
					tmCurPos = topMenuContents.size()-1;
					tmScreenOffset = std::max(tmCurPos - ENTRIES_PER_SCREEN + 1, 0);
				}
				showTopMenu(topMenuContents);
			}
		} else if(pressed & KEY_START) {
			return "%EXIT%";
		} else if(pressed & KEY_SELECT) {
			configMenu();
			drawImageDMA(0, 0, listBg, false, false);
			showTopMenu(topMenuContents);
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<std::min(ENTRIES_PER_SCREEN, (int)topMenuContents.size());i++) {
				if(touch.py > (i+1)*16 && touch.py < (i+2)*16) {
					tmCurPos = tmScreenOffset + i;
					goto selection;
				}
			}
		}

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
			drawRectangle(0, 0, 256, 16, CLEAR, false, true);

			// Print the path to the currently selected file
			std::u16string path = StringUtils::UTF8toUTF16(topMenuContents[tmCurPos].name);
			path = path.substr(0, path.find_last_of(StringUtils::UTF8toUTF16("/"))+1); // Cut to just the path
			printTextMaxW(path, 250, 1, 4, 0, false, true);
		}
	}
}

std::string browseForFile(const std::vector<std::string>& extensionList, bool accessSubdirectories, bool canChooseDirs) {
	char startPath[PATH_MAX];
	if(!accessSubdirectories)	getcwd(startPath, PATH_MAX);
	int pressed, held, screenOffset = 0, fileOffset = 0;
	touchPosition touch;
	std::vector<DirEntry> dirContents;

	// Draw background
	drawImageDMA(0, 0, listBg, false, false);

	getDirectoryContents(dirContents, extensionList);
	showDirectoryContents(dirContents, screenOffset);

	while(1) {
		// Clear old cursors
		drawRectangle(0, 16, 10, 176, CLEAR, false, true);

		// Draw cursor
		drawRectangle(3, (fileOffset-screenOffset)*16+24, 4, 3, LIGHT_GRAY, false, true);


		// Power saving loop. Only poll the keys once per frame and sleep the CPU if there is nothing else to do
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(fileOffset > 0)	fileOffset -= 1;
			else	fileOffset = dirContents.size()-1;
		} else if(held & KEY_DOWN) {
			if(fileOffset < (int)dirContents.size()-1)	fileOffset += 1;
			else	fileOffset = 0;
		} else if(held & KEY_LEFT) {
			fileOffset -= ENTRY_PAGE_LENGTH;
			if(fileOffset < 0)	fileOffset = 0;
		} else if(held & KEY_RIGHT) {
			fileOffset += ENTRY_PAGE_LENGTH;
			if(fileOffset > (int)dirContents.size()-1)	fileOffset = dirContents.size()-1;
		} else if(pressed & KEY_A) {
			selection:
			DirEntry* entry = &dirContents.at(fileOffset);
			if(entry->isDirectory && !canChooseDirs) {
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
			} else {
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
		} else if(pressed & KEY_Y && accessSubdirectories) { // accessSubdirectory is to make it not trigger except in save selection
			if(dirContents[fileOffset].isDirectory || loadSave(dirContents[fileOffset].name)) {
				Sound::play(Sound::click);
				char path[PATH_MAX];
				getcwd(path, PATH_MAX);

				FILE* favs = fopen((mainDrive() + ":/_nds/pkmn-chest/favorites.lst").c_str(), "ab");

				if(favs) {
					fwrite((path+dirContents[fileOffset].name+"\n").c_str(), 1, (path+dirContents[fileOffset].name+"\n").size(), favs);
					fclose(favs);
				}
			}
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(int i=0;i<std::min(ENTRIES_PER_SCREEN, (int)dirContents.size());i++) {
				if(touch.py > (i+1)*16 && touch.py < (i+2)*16) {
					fileOffset = screenOffset + i;
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
	}
}

std::string browseForSave(void) {
	if(showTopMenuOnExit) {
		showTopMenuOnExit = 0;
		std::string str = topMenuSelect();
		if(str != "")	return str;
	}

	// Draw backgrounds
	drawImageDMA(0, 0, boxBgTop, true, false);
	drawImageDMA(0, 0, listBg, false, false);

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, true, true);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Print version number
	printTextScaled(VER_NUMBER, wideScreen ? 0.8 : 1, 1, 256-getTextWidthScaled(VER_NUMBER, wideScreen ? 0.8 : 1)-1, 176, true, true);

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
			return -1;
		}

		FILE* destinationFile = fopen(destinationPath, "wb");
			fseek(destinationFile, 0, SEEK_SET);

		off_t offset = 0;
		drawOutline(5, 39, 247, 18, DARKERER_GRAY, false, true);
		while(1) {
			drawRectangle(((offset < fsize ? (float)offset/fsize : 1)*226)+6, 40, 19, 16, LIGHT_GRAY, false, true);
			// Copy file to destination path
			int numr = fread(copyBuf, 2, copyBufSize, sourceFile);
			fwrite(copyBuf, 2, numr, destinationFile);
			offset += copyBufSize;

			if(offset > fsize) {
				fclose(sourceFile);
				fclose(destinationFile);

				drawRectangle(4, 39, 248, 18, CLEAR, false, true);
				return 1;
			}
		}
		drawRectangle(4, 39, 248, 18, CLEAR, false, true);
		return -1;
	} else {
		closedir(isDir);
		return -2;
	}
}
