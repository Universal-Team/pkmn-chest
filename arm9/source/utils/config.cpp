#include "config.hpp"

#include "flashcard.hpp"
#include "i18n.hpp"
#include "json.hpp"

#include <nds/system.h>

nlohmann::json configJson;

pksm::Language sysLang() {
	extern bool useTwlCfg;
	switch(useTwlCfg ? *(u8 *)0x02000406 : PersonalData->language) {
		case 0:
			return pksm::Language::JPN;
		case 1:
		default:
			return pksm::Language::ENG;
		case 2:
			return pksm::Language::FRE;
		case 3:
			return pksm::Language::GER;
		case 4:
			return pksm::Language::ITA;
		case 5:
			return pksm::Language::SPA;
		case 6:
			return pksm::Language::CHS;
		case 7:
			return pksm::Language::KOR;
	}
}

void Config::load() {
	FILE *file = fopen((mainDrive() + ":/_nds/pkmn-chest/config.json").c_str(), "rb");
	if(file) {
		configJson = nlohmann::json::parse(file, nullptr, false);
		fclose(file);
	}
}

void Config::save() {
	FILE *file = fopen((mainDrive() + ":/_nds/pkmn-chest/config.json").c_str(), "wb");
	if(file) {
		fwrite(configJson.dump(1, '\t').c_str(), 1, configJson.dump(1, '\t').size(), file);
		fclose(file);
	}
}

bool Config::getBool(const std::string &key) {
	if(!configJson.contains(key) || !configJson[key].is_boolean()) {
		return false;
	}
	return configJson.at(key).get_ref<const bool &>();
}
void Config::setBool(const std::string &key, bool v) { configJson[key] = v; }

int Config::getInt(const std::string &key) {
	if(!configJson.contains(key) || !configJson[key].is_number()) {
		return 0;
	}
	return configJson.at(key).get_ref<const int64_t &>();
}
void Config::setInt(const std::string &key, int v) { configJson[key] = v; }

std::string Config::getString(const std::string &key) {
	if(!configJson.contains(key) || !configJson[key].is_string()) {
		return "";
	}
	return configJson.at(key).get_ref<const std::string &>();
}
void Config::setString(const std::string &key, const std::string &v) { configJson[key] = v; }

pksm::Language Config::getLang(const std::string &key) {
	if(!configJson.contains(key) || !configJson[key].is_string()) {
		return sysLang();
	}
	return i18n::langFromString(configJson.at(key).get_ref<const std::string &>());
}
void Config::setLang(const std::string &key, pksm::Language lang) { configJson[key] = i18n::langString(lang); }
