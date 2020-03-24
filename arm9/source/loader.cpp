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
	if(in) {
		fseek(in, 0, SEEK_END);
		u32 size = ftell(in);
		fseek(in, 0, SEEK_SET);
		std::shared_ptr<u8[]> saveData = std::shared_ptr<u8[]>(new u8[size]);
		fread(saveData.get(), 1, size, in);
		fclose(in);
		save = Sav::getSave(saveData, size);
	} else {
		saveFileName = "";
		return false;
	}
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
	DateTime now = DateTime::now();
	snprintf(backupPath,
			 sizeof(backupPath),
			 "%s:/_nds/pkmn-chest/backups/%s/%.4ld%.2d%.2d-%.2d%.2d%.2d.sav.bak",
			 sdFound() ? "sd" : "fat",
			 saveFile.substr(0, saveFile.find_last_of(".")).c_str(),
			 now.year(),
			 now.month(),
			 now.day(),
			 now.hour(),
			 now.minute(),
			 now.second());

	fcopy(savePath.c_str(), backupPath);

	if(Config::getInt("backupAmount") > 0) { // 0 = unlimited
		char savDir[PATH_MAX];
		getcwd(savDir, PATH_MAX);
		chdir(backupDir);
		std::vector<DirEntry> dirContents;
		getDirectoryContents(dirContents, {"bak"});

		if((int)dirContents.size() > Config::getInt("backupAmount"))	remove(dirContents[1].name.c_str()); // index 0 is '..'

		chdir(savDir);
	}

	save->finishEditing();

	FILE* out = fopen(savePath.c_str(), "rb+");
	fwrite(save->rawData().get(), 1, save->getLength(), out);
	fclose(out);

	save->beginEditing();
}
