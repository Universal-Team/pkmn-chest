#include "loader.h"
#include <dirent.h>
#include <fstream>
#include <unistd.h>

#include "config.h"
#include "fileBrowse.h"
#include "flashcard.h"

static std::string saveFileName;

std::shared_ptr<Sav> save;

bool loadSave(std::string savePath) {
	saveFileName = savePath;
	std::ifstream in(savePath);
	u32 size;
	char* saveData = nullptr;
	if(in.good()) {
		in.seekg(0, in.end);
		size = in.tellg();
		in.seekg(0, in.beg);
		saveData = new char[size];
		in.read(saveData, size);
	} else {
		saveFileName = "";
		in.close();
		return false;
	}
	in.close();
	save = Sav::getSave((u8*)saveData, size);
	delete[] saveData;
	if(!save) {
		saveFileName = "";
		return false;
	}
	return true;
}

void saveChanges(std::string savePath) {
	// Make backups
	if(Config::backupAmount != 0) {
		std::string saveFile = savePath.substr(savePath.find_last_of("/")+1);
		for(unsigned i=Config::backupAmount;i>1;i--) {
			char path1[PATH_MAX];
			char path2[PATH_MAX];
			snprintf(path1, sizeof(path1), "%s:/_nds/pkmn-chest/backups/%s.bak%i", sdFound() ? "sd" : "fat", saveFile.c_str(), i);
			snprintf(path2, sizeof(path2), "%s:/_nds/pkmn-chest/backups/%s.bak%i", sdFound() ? "sd" : "fat", saveFile.c_str(), i-1);
			if(access(path1, F_OK) == 0)	remove(path1);
			if(access(path2, F_OK) == 0)	rename(path2, path1);
		}

		char path[PATH_MAX];
		snprintf(path, sizeof(path), "%s:/_nds/pkmn-chest/backups/%s.bak1", sdFound() ? "sd" : "fat", saveFile.c_str());
		fcopy(savePath.c_str(), path);
	}

	save->resign();
	// No need to check size; if it was read successfully, that means that it has the correct size
	std::fstream out(savePath, std::fstream::in | std::fstream::out);
	out.write((char*)save->rawData(), save->getLength());
	out.close();
}
