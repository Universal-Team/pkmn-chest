#include "i18n_ext.hpp"
#include "../core/source/i18n/i18n_internal.hpp"
#include "i18n.hpp"

const std::string &i18n::pouch(pksm::Language lang, pksm::Sav::Pouch pouch) {
	switch(pouch) {
		case pksm::Sav::Pouch::NormalItem:
			return localize(lang, "ITEMS");
			break;
		case pksm::Sav::Pouch::KeyItem:
			return localize(lang, "KEY_ITEMS");
			break;
		case pksm::Sav::Pouch::TM:
			return localize(lang, "BALLS");
			break;
		case pksm::Sav::Pouch::Mail:
			return localize(lang, "MAIL");
			break;
		case pksm::Sav::Pouch::Medicine:
			return localize(lang, "MEDICINE");
			break;
		case pksm::Sav::Pouch::Berry:
			return localize(lang, "BERRIES");
			break;
		case pksm::Sav::Pouch::Ball:
			return localize(lang, "BALLS");
			break;
		case pksm::Sav::Pouch::Battle:
			return localize(lang, "BATTLE_ITEMS");
			break;
		case pksm::Sav::Pouch::Candy:
			return localize(lang, "CANDIES");
			break;
		case pksm::Sav::Pouch::ZCrystals:
			return localize(lang, "ZCRYSTALS");
			break;
		case pksm::Sav::Pouch::Treasure:
			return localize(lang, "TREASURES");
			break;
		case pksm::Sav::Pouch::Ingredient:
			return localize(lang, "INGREDIENTS");
			break;
		case pksm::Sav::Pouch::PCItem:
			return localize(lang, "PC_ITEMS");
			break;
		case pksm::Sav::Pouch::RotomPower:
			return localize(lang, "ROTOM_POWERS");
			break;
		case pksm::Sav::Pouch::CatchingItem:
			return localize(lang, "CATCHING_ITEMS");
			break;
		default:
			return emptyString;
			break;
	}
}

std::unordered_map<pksm::Language, nlohmann::json> gui;

// load function for json
void i18n::load(pksm::Language lang, const std::string& name, nlohmann::json& json) {
	std::string path = io::exists(_PKSMCORE_LANG_FOLDER + i18n::folder(lang) + name) ? _PKSMCORE_LANG_FOLDER + i18n::folder(lang) + name
																					 : _PKSMCORE_LANG_FOLDER + i18n::folder(pksm::Language::ENG) + name;

	FILE* values = fopen(path.c_str(), "rt");
	if(values) {
		json = nlohmann::json::parse(values, nullptr, false);
		fclose(values);
	}
}

void i18n::initGui(pksm::Language lang) {
	nlohmann::json j;
	load(lang, "/gui.json", j);
	
	gui.insert_or_assign(lang, std::move(j));
}

void i18n::exitGui(pksm::Language lang) { gui.erase(lang); }

const std::string &i18n::localize(pksm::Language lang, const std::string &v) {
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
