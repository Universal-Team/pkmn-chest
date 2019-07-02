#include "langStrings.h"

namespace Lang {
	std::vector<std::string>
	// In game text strings
	balls, items, locations4, locations5, moves, natures, species,
	// App strings
	aMenuText = {"Edit", "Move", "Copy", "Release", "Dump", "Back"},
	aMenuEmptySlotText = {"Inject", "Create", "Back"},
	aMenuTopBarText = {"Rename", "Swap", "Dump box", "Back"},
	optionsTextLabels = {"Chest File:", "Chest Size:", "Language:", "Backups:"},
	optionsText = {"New", "Rename", "Delete", "Change"},
	summaryP1Labels = {"Dex No.", "Name", "Ball", "Level", "Nature", "Shiny", "Pokérus", "Orig. Trainer", "Trainer ID", "Secret ID", "Friendship"},
	summaryP2Labels = {"HP.", "Attack", "Defense", "Sp. Atk", "Sp. Def", "Speed", "Base", "IV", "EV", "Total"},
	xMenuText = {"Party", "Options", "", "", "Save", "Exit"};

	std::string
	// [main]
	cancel = "Cancel",
	chest = "Chest",
	discard = "Discard",
	female = "Female",
	invalidSave = "Invalid save file",
	loading = "Loading...",
	male = "Male",
	no = "False",
	release = "Release",
	remove = "Remove",
	save = "Save",
	yes = "True",
	// [saveMsg]
	saveMsgCard = "Would you like to save changes\nto the gamecard?",
	saveMsgChest = "Would you like to save changes\nto the chest?",
	saveMsgSave = "Would you like to save changes\nto the save file?";
}
