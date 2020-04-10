#include "i18n_ext.hpp"
#include "../core/source/i18n/i18n_internal.hpp"
#include "i18n.hpp"

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
