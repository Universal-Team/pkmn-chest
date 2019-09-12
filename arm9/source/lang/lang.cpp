#include "lang.h"
#include <fstream>

#include "inifile.h"
#include "langStrings.h"

std::string langs[] = { "de", "en", "es", "fr", "it", "lt", "pt", "ru", "jp", "ko"};

void Lang::loadLangStrings(int lang) {
	// Clear vectors
	Lang::abilities.clear();
	Lang::games.clear();
	Lang::items.clear();
	Lang::locations4.clear();
	Lang::locations5.clear();
	Lang::moves.clear();
	Lang::natures.clear();
	Lang::species.clear();
	
	// Declare variables
	std::string line;

	// Fill vectors from files
	std::ifstream in("nitro:/lang/"+langs[lang]+"/abilities");
	while(std::getline(in, line)) {
		Lang::abilities.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/games");
	while(std::getline(in, line)) {
		Lang::games.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/items");
	while(std::getline(in, line)) {
		Lang::items.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/locations4");
	while(std::getline(in, line)) {
		Lang::locations4.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/locations5");
	while(std::getline(in, line)) {
		Lang::locations5.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/moves");
	while(std::getline(in, line)) {
		Lang::moves.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/natures");
	while(std::getline(in, line)) {
		Lang::natures.push_back(line);
	}
	in.close();

	in.open("nitro:/lang/"+langs[lang]+"/species");
	while(std::getline(in, line)) {
		Lang::species.push_back(line);
	}
	in.close();

	// Load app strings
	CIniFile ini("nitro:/lang/"+langs[lang]+"/app.ini");

	// [aMenu]
	Lang::aMenuText[0] = ini.GetString("aMenu", "move", Lang::aMenuText[0]);
	Lang::aMenuText[1] = ini.GetString("aMenu", "edit", Lang::aMenuText[1]);
	Lang::aMenuText[2] = ini.GetString("aMenu", "copy", Lang::aMenuText[2]);
	Lang::aMenuText[3] = ini.GetString("aMenu", "release", Lang::aMenuText[3]);
	Lang::aMenuText[4] = ini.GetString("aMenu", "dump", Lang::aMenuText[4]);
	Lang::aMenuText[5] = ini.GetString("aMenu", "back", Lang::aMenuText[5]);
	Lang::aMenuEmptySlotText[0] = ini.GetString("aMenu", "inject", Lang::aMenuEmptySlotText[0]);
	Lang::aMenuEmptySlotText[1] = ini.GetString("aMenu", "create", Lang::aMenuEmptySlotText[1]);
	Lang::aMenuEmptySlotText[2] = ini.GetString("aMenu", "back", Lang::aMenuEmptySlotText[2]);
	Lang::aMenuTopBarText[0] = ini.GetString("aMenu", "jump", Lang::aMenuTopBarText[0]);
	Lang::aMenuTopBarText[1] = ini.GetString("options", "rename", Lang::aMenuTopBarText[1]);
	Lang::aMenuTopBarText[2] = ini.GetString("aMenu", "swap", Lang::aMenuTopBarText[2]);
	Lang::aMenuTopBarText[3] = ini.GetString("aMenu", "dumpBox", Lang::aMenuTopBarText[3]);
	Lang::aMenuTopBarText[4] = ini.GetString("aMenu", "back", Lang::aMenuTopBarText[4]);

	// [main]
	Lang::cancel = ini.GetString("main", "cancel", Lang::cancel);
	Lang::chest = ini.GetString("main", "chest", Lang::chest);
	Lang::discard = ini.GetString("main", "discard", Lang::discard);
	Lang::female = ini.GetString("main", "female", Lang::female);
	Lang::hpType = ini.GetString("main", "hpType", Lang::hpType);
	Lang::invalidSave = ini.GetString("main", "invalidSave", Lang::invalidSave);
	Lang::loading = ini.GetString("main", "loading", Lang::loading);
	Lang::male = ini.GetString("main", "male", Lang::male);
	Lang::no = ini.GetString("main", "no", Lang::no);
	Lang::origin = ini.GetString("main", "origin", Lang::origin);
	Lang::release = ini.GetString("aMenu", "release", Lang::release);
	Lang::remove = ini.GetString("main", "remove", Lang::remove);
	Lang::save = ini.GetString("main", "save", Lang::save);
	Lang::yes = ini.GetString("main", "yes", Lang::yes);

	// [options]
	Lang::optionsTextLabels[0] = ini.GetString("options", "chestFile", Lang::optionsTextLabels[0]);
	Lang::optionsTextLabels[1] = ini.GetString("options", "chestSize", Lang::optionsTextLabels[1]);
	Lang::optionsTextLabels[2] = ini.GetString("options", "language", Lang::optionsTextLabels[2]);
	Lang::optionsTextLabels[3] = ini.GetString("options", "backupAmount", Lang::optionsTextLabels[3]);
	Lang::optionsTextLabels[4] = ini.GetString("options", "music", Lang::optionsTextLabels[4]);
	Lang::optionsTextLabels[5] = ini.GetString("options", "soundFX", Lang::optionsTextLabels[5]);
	Lang::optionsText[0] = ini.GetString("options", "new", Lang::optionsText[0]);
	Lang::optionsText[1] = ini.GetString("options", "rename", Lang::optionsText[1]);
	Lang::optionsText[2] = ini.GetString("options", "delete", Lang::optionsText[2]);
	Lang::optionsText[3] = ini.GetString("options", "change", Lang::optionsText[3]);

	// [origin]
	Lang::originLabels[0] = ini.GetString("origin", "metLevel", Lang::originLabels[0]);
	Lang::originLabels[1] = ini.GetString("origin", "metYear", Lang::originLabels[1]);
	Lang::originLabels[2] = ini.GetString("origin", "metMonth", Lang::originLabels[2]);
	Lang::originLabels[3] = ini.GetString("origin", "metDay", Lang::originLabels[3]);
	Lang::originLabels[4] = ini.GetString("origin", "metLocation", Lang::originLabels[4]);
	Lang::originLabels[5] = ini.GetString("origin", "originGame", Lang::originLabels[5]);

	// [songs]
	Lang::songs[0] = ini.GetString("songs", "off", Lang::songs[0]);
	Lang::songs[1] = ini.GetString("songs", "center1", Lang::songs[1]);
	Lang::songs[2] = ini.GetString("songs", "center4", Lang::songs[2]);
	Lang::songs[3] = ini.GetString("songs", "elmLab", Lang::songs[3]);
	Lang::songs[4] = ini.GetString("songs", "oakLab", Lang::songs[4]);
	Lang::songs[5] = ini.GetString("songs", "gameCorner", Lang::songs[5]);

	// Stats
	Lang::statsLabels[0] = ini.GetString("stats", "hp", Lang::statsLabels[0]);
	Lang::statsLabels[1] = ini.GetString("stats", "attack", Lang::statsLabels[1]);
	Lang::statsLabels[2] = ini.GetString("stats", "defense", Lang::statsLabels[2]);
	Lang::statsLabels[3] = ini.GetString("stats", "spAtk", Lang::statsLabels[3]);
	Lang::statsLabels[4] = ini.GetString("stats", "spDef", Lang::statsLabels[4]);
	Lang::statsLabels[5] = ini.GetString("stats", "speed", Lang::statsLabels[5]);
	Lang::statsLabels[6] = ini.GetString("stats", "base", Lang::statsLabels[6]);
	Lang::statsLabels[7] = ini.GetString("stats", "iv", Lang::statsLabels[7]);
	Lang::statsLabels[8] = ini.GetString("stats", "ev", Lang::statsLabels[8]);
	Lang::statsLabels[9] = ini.GetString("stats", "total", Lang::statsLabels[9]);

	// [summary]
	Lang::summaryLabels[0]  = ini.GetString("summary", "dexNo", Lang::summaryLabels[0]);
	Lang::summaryLabels[1]  = ini.GetString("summary", "name", Lang::summaryLabels[1]);
	Lang::summaryLabels[2]  = ini.GetString("summary", "level", Lang::summaryLabels[2]);
	Lang::summaryLabels[3]  = ini.GetString("summary", "ability", Lang::summaryLabels[3]);
	Lang::summaryLabels[4]  = ini.GetString("summary", "nature", Lang::summaryLabels[4]);
	Lang::summaryLabels[5]  = ini.GetString("summary", "shiny", Lang::summaryLabels[5]);
	Lang::summaryLabels[6]  = ini.GetString("summary", "pokerus", Lang::summaryLabels[6]);
	Lang::summaryLabels[7]  = ini.GetString("summary", "origTrainer", Lang::summaryLabels[7]);
	Lang::summaryLabels[8]  = ini.GetString("summary", "trainerID", Lang::summaryLabels[8]);
	Lang::summaryLabels[9]  = ini.GetString("summary", "secretID", Lang::summaryLabels[9]);
	Lang::summaryLabels[10] = ini.GetString("summary", "friendship", Lang::summaryLabels[10]);

	Lang::movesString = ini.GetString("summary", "moves", Lang::movesString);
	Lang::stats = ini.GetString("summary", "stats", Lang::stats);

	// [saveMsg]
	Lang::saveMsgCard = ini.GetString("saveMsg", "saveMsgCard", Lang::saveMsgCard);
	Lang::saveMsgChest = ini.GetString("saveMsg", "saveMsgChest", Lang::saveMsgChest);
	Lang::saveMsgSave = ini.GetString("saveMsg", "saveMsgSave", Lang::saveMsgSave);
	
	// [trainer]
	Lang::trainerText[0] = ini.GetString("summary", "name", Lang::trainerText[0]);
	Lang::trainerText[1] = ini.GetString("summary", "trainerID", Lang::trainerText[1]);
	Lang::trainerText[2] = ini.GetString("summary", "secretID", Lang::trainerText[2]);
	Lang::trainerText[3] = ini.GetString("trainer", "money", Lang::trainerText[3]);
	Lang::trainerText[4] = ini.GetString("trainer", "bp", Lang::trainerText[4]);
	Lang::trainerText[5] = ini.GetString("trainer", "badges", Lang::trainerText[5]);
	Lang::trainerText[6] = ini.GetString("trainer", "playTime", Lang::trainerText[6]);

	// [xMenu]
	Lang::xMenuText[0] = ini.GetString("xMenu", "party", Lang::xMenuText[0]);
	Lang::xMenuText[1] = ini.GetString("xMenu", "options", Lang::xMenuText[1]);
	Lang::xMenuText[4] = ini.GetString("main", "save", Lang::xMenuText[4]);
	Lang::xMenuText[5] = ini.GetString("xMenu", "exit", Lang::xMenuText[5]);
}