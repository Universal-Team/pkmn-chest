#include "lang.h"
#include <fstream>

std::vector<std::string> Lang::balls, Lang::items, Lang::locations4, Lang::locations5, Lang::moves, Lang::natures, Lang::species;

std::string langs[] = { "de", "en", "es", "fr", "it", "jp"};

void Lang::loadLangStrings(int lang) {
	// Clear vectors
	Lang::balls.clear();
	Lang::items.clear();
	Lang::locations4.clear();
	Lang::locations5.clear();
	Lang::moves.clear();
	Lang::natures.clear();
	Lang::species.clear();
	
	// Declare variables
	char path[32];
	std::string line;

	// Fill vectors from files
	snprintf(path, sizeof(path), "nitro:/lang/%s/balls", langs[lang].c_str());
	std::ifstream in(path);
	while(std::getline(in, line)) {
		Lang::balls.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/items", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::items.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/locations4", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::locations4.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/locations5", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::locations5.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/moves", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::moves.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/natures", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::natures.push_back(line);
	}
	in.close();

	snprintf(path, sizeof(path), "nitro:/lang/%s/species", langs[lang].c_str());
	in.open(path);
	while(std::getline(in, line)) {
		Lang::species.push_back(line);
	}
	in.close();
}