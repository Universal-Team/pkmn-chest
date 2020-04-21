#include "i18n_ext.hpp"
#include "../core/source/i18n/i18n_internal.hpp"
#include "i18n.hpp"
#include "json.hpp"

const std::string &i18n::pouch(Language lang, Sav::Pouch pouch) {
	switch(pouch) {
		case Sav::Pouch::NormalItem:
			return localize(lang, "ITEMS");
			break;
		case Sav::Pouch::KeyItem:
			return localize(lang, "KEY_ITEMS");
			break;
		case Sav::Pouch::TM:
			return localize(lang, "BALLS");
			break;
		case Sav::Pouch::Mail:
			return localize(lang, "MAIL");
			break;
		case Sav::Pouch::Medicine:
			return localize(lang, "MEDICINE");
			break;
		case Sav::Pouch::Berry:
			return localize(lang, "BERRIES");
			break;
		case Sav::Pouch::Ball:
			return localize(lang, "BALLS");
			break;
		case Sav::Pouch::Battle:
			return localize(lang, "BATTLE_ITEMS");
			break;
		case Sav::Pouch::Candy:
			return localize(lang, "CANDIES");
			break;
		case Sav::Pouch::ZCrystals:
			return localize(lang, "ZCRYSTALS");
			break;
		case Sav::Pouch::Treasure:
			return localize(lang, "TREASURES");
			break;
		case Sav::Pouch::Ingredient:
			return localize(lang, "INGREDIENTS");
			break;
		case Sav::Pouch::PCItem:
			return localize(lang, "PC_ITEMS");
			break;
		case Sav::Pouch::RotomPower:
			return localize(lang, "ROTOM_POWERS");
			break;
		case Sav::Pouch::CatchingItem:
			return localize(lang, "CATCHING_ITEMS");
			break;
		default:
			return emptyString;
			break;
	}
}

std::unordered_map<Language, nlohmann::json> gui;

// load function for json
void load(Language lang, const std::string& name, nlohmann::json& json) {
	std::string path = io::exists(_PKSMCORE_LANG_FOLDER + i18n::folder(lang) + name) ? _PKSMCORE_LANG_FOLDER + i18n::folder(lang) + name
																					 : _PKSMCORE_LANG_FOLDER + i18n::folder(Language::ENG) + name;

	FILE* values = fopen(path.c_str(), "rt");
	if(values) {
		json = nlohmann::json::parse(values, nullptr, false);
		fclose(values);
	}
}

void i18n::initGui(Language lang) {
	nlohmann::json j;
	load(lang, "/gui.json", j);
	
	gui.insert_or_assign(lang, std::move(j));
}

void i18n::exitGui(Language lang) { gui.erase(lang); }

const std::string &i18n::localize(Language lang, const std::string &v) {
	checkInitialized(lang);
	auto it = gui.find(lang);
	if(it != gui.end()) {
		if(!it->second.contains(v)) {
			it->second[v] = "MISSING: " + v;
		}
		return it->second[v].get_ref<const std::string&>();
	}
	return emptyString;
}
