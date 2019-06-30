#include "langStrings.h"

namespace Lang {
	std::vector<std::string>
	// In game text strings
	balls, items, locations4, locations5, moves, natures, species,
	// App strings
	optionsTextLabels = {"Chest File:", "Chest Size:", "Language:"},
	optionsText = {"New", "Rename", "Delete", "Change", "", ""},
	xMenuText = {"Party", "Options", "", "", "Save", "Exit"};

	std::string
	// [main]
	invalidSave = "Invalid save file",
	loading = "Loading...";
}
