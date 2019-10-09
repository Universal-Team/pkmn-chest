#include "loader.hpp"
#include <dirent.h>
#include <unistd.h>

#include "config.hpp"
#include "fileBrowse.hpp"
#include "flashcard.hpp"

static std::string saveFileName;

std::shared_ptr<Sav> save;

bool loadSave(std::string savePath) {
	save = nullptr;
	saveFileName = savePath;
	FILE* in = fopen(savePath.c_str(), "rb");
	u8* saveData = nullptr;
	u32 size;
	if(in) {
		fseek(in, 0, SEEK_END);
		size = ftell(in);
		fseek(in, 0, SEEK_SET);
		saveData = new u8[size];
		fread(saveData, 1, size, in);
	} else {
		saveFileName = "";
		fclose(in);
		return false;
	}
	fclose(in);
	save = Sav::getSave(saveData, size);
	delete[] saveData;
	if(!save) {
		saveFileName = "";
		return false;
	}
	return true;
}

void saveChanges(std::string savePath) {
	// Make backup
	std::string saveFile = savePath.substr(savePath.find_last_of("/")+1);
	char backupDir[PATH_MAX];
	snprintf(backupDir, sizeof(backupDir), "%s:/_nds/pkmn-chest/backups/%s", sdFound() ? "sd" : "fat", saveFile.substr(0, saveFile.find_last_of(".")).c_str());
	mkdir(backupDir, 0777);

	char backupPath[PATH_MAX];
	const time_t current = time(NULL);
	snprintf(backupPath, sizeof(backupPath), "%s:/_nds/pkmn-chest/backups/%s/%.4d%.2d%.2d-%.2d%.2d%.2d.sav.bak", sdFound() ? "sd" : "fat", saveFile.substr(0, saveFile.find_last_of(".")).c_str(), gmtime(&current)->tm_year+1900, gmtime(&current)->tm_mon+1, gmtime(&current)->tm_mday, gmtime(&current)->tm_hour, gmtime(&current)->tm_min, gmtime(&current)->tm_sec);

	fcopy(savePath.c_str(), backupPath);

	if(Config::backupAmount > 0) { // 0 = unlimited
		char savDir[PATH_MAX];
		getcwd(savDir, PATH_MAX);
		chdir(backupDir);
		std::vector<DirEntry> dirContents;
		getDirectoryContents(dirContents, {"bak"});

		if((int)dirContents.size() > Config::backupAmount)	remove(dirContents[1].name.c_str()); // index 0 is '..'

		chdir(savDir);
	}

	save->resign();
	// No need to check size; if it was read successfully, that means that it has the correct size
	FILE* out = fopen(savePath.c_str(), "rb+");
	fwrite(save->rawData(), 1, save->getLength(), out);
	fclose(out);
}
