#include "lang.hpp"
#include "graphics.hpp"
#include "lang.hpp"
#include "manager.hpp"
#include "json.hpp"

std::vector<std::string> Lang::abilities, Lang::games, Lang::items, Lang::locations4, Lang::locations5, Lang::moves, Lang::natures, Lang::species;
std::string langs[] = {"de", "en", "es", "fr", "it", "lt", "pt", "ru", "jp", "ko"};
nlohmann::json langJson;

void loadToVector(std::string path, std::vector<std::string> &vec) {
	char* line = NULL;
	size_t len = 0;
	vec.clear();

	FILE* in = fopen(path.c_str(), "r");
	while(__getline(&line, &len, in) != -1) {
		if(line[strlen(line)-1] == '\n')	line[strlen(line)-1] = '\0';
		vec.push_back(line);
	}
	fclose(in);
}

void Lang::load(int lang) {
	// Check if the language has game info
	int tempLang = (access(("nitro:/lang/"+langs[lang]+"/abilities.txt").c_str(), F_OK) == 0) ? lang : 1;

	// Fill vectors from files
	loadToVector("nitro:/lang/"+langs[tempLang]+"/abilities.txt", Lang::abilities);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/games.txt", Lang::games);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/items.txt", Lang::items);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/locations4.txt", Lang::locations4);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/locations5.txt", Lang::locations5);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/moves.txt", Lang::moves);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/natures.txt", Lang::natures);
	loadToVector("nitro:/lang/"+langs[tempLang]+"/species.txt", Lang::species);

	// Load types picture
	types.clear();
	tempLang = (access(("nitro:/lang/"+langs[lang]+"/types.png").c_str(), F_OK) == 0) ? lang : 1;
	typesData = loadPng("nitro:/lang/"+langs[tempLang]+"/types.png", types);

	// Load app strings
	FILE* file = fopen(("nitro:/lang/"+langs[lang]+"/app.json").c_str(), "rt");
	if(file)	langJson = nlohmann::json::parse(file, nullptr, false);
	fclose(file);
}

std::string Lang::get(const std::string &key) {
	if(!langJson.contains(key)) {
		return "MISSING: " + key;
	}
	return langJson.at(key).get_ref<const std::string&>();
}
