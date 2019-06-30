#include "loader.h"
#include <fstream>

static bool saveIsFile;
static std::string saveFileName;

std::shared_ptr<Sav> save;

bool loadSave(std::string savePath) {
	saveIsFile   = true;
	saveFileName = savePath;
	std::ifstream in(savePath);
	u32 size;
	char* saveData = nullptr;
	if(in.good()) {
		in.seekg(0, in.end);
		size     = in.tellg();
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
	save->resign();
	if(saveIsFile) {
		// No need to check size; if it was read successfully, that means that it has the correct size
		std::fstream out(savePath, std::fstream::in | std::fstream::out);
		out.write((char*)save->rawData(), save->getLength());
		out.close();
	}
}
