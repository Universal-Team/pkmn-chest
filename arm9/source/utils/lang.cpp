#include "lang.hpp"
#include "manager.hpp"
#include "json.hpp"

std::vector<std::string> Lang::abilities, Lang::games, Lang::items, Lang::items3, Lang::locations3, Lang::locations4, Lang::locations5, Lang::moves, Lang::natures, Lang::species;
const std::string langs[] = {"br", "de", "en", "es", "fr", "id", "it", "lt", "pt", "ru", "jp", "ko"};
nlohmann::json langJson;

void loadToVector(const std::string &filename, int lang, std::vector<std::string> &vec) {
	// Check if the language has game info
	int tempLang = (access(("nitro:/lang/"+langs[lang]+"/"+filename).c_str(), F_OK) == 0) ? lang : Lang::en;

	char* line = NULL;
	size_t len = 0;
	vec.clear();

	FILE* in = fopen(("nitro:/lang/"+langs[tempLang]+"/"+filename).c_str(), "r");
	if(in) {
		while(__getline(&line, &len, in) != -1) {
			if(line[strlen(line)-1] == '\n')	line[strlen(line)-1] = '\0';
			vec.push_back(line);
		}
		fclose(in);
	}
}

void Lang::load(int lang) {
	// Fill vectors from files
	loadToVector("abilities.txt", lang, Lang::abilities);
	loadToVector("games.txt", lang, Lang::games);
	loadToVector("items.txt", lang, Lang::items);
	loadToVector("items3.txt", lang, Lang::items3);
	loadToVector("locations3.txt", lang, Lang::locations3);
	loadToVector("locations4.txt", lang, Lang::locations4);
	loadToVector("locations5.txt", lang, Lang::locations5);
	loadToVector("moves.txt", lang, Lang::moves);
	loadToVector("natures.txt", lang, Lang::natures);
	loadToVector("species.txt", lang, Lang::species);

	// Load types picture
	int tempLang = (access(("nitro:/lang/"+langs[lang]+"/types/0.gfx").c_str(), F_OK) == 0) ? lang : Lang::en;
	types.clear();
	for(int i=0;i<17;i++) {
		types.push_back(loadImage("/lang/"+langs[tempLang]+"/types/"+std::to_string(i)+".gfx"));
	}

	// Load app strings
	FILE* file = fopen(("nitro:/lang/"+langs[lang]+"/app.json").c_str(), "rt");
	if(file) {
		langJson = nlohmann::json::parse(file, nullptr, false);
		fclose(file);
	}
}

std::string Lang::get(const std::string &key) {
	if(!langJson.contains(key)) {
		return key;
	}
	return langJson.at(key).get_ref<const std::string&>();
}
