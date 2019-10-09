#ifndef LANG_STRINGS_HPP
#define LANG_STRINGS_HPP
#include <string>
#include <vector>

namespace Lang {
	extern std::vector<std::string>
	// In game text strings
	abilities, games, items, locations4, locations5, moves, natures, species,
	// App strings
	aMenuText,
	aMenuEmptySlotText,
	aMenuTopBarText,
	optionsTextLabels,
	optionsText,
	originLabels,
	songs,
	summaryLabels,
	statsLabels,
	trainerText,
	xMenuText;

	extern std::string
	// [main]
	cancel,
	chest,
	discard,
	female,
	hpType,
	invalidSave,
	loading,
	male,
	movesString,
	no,
	origin,
	stats,
	release,
	remove,
	save,
	unlimited,
	yes,
	// [saveMsg]
	saveMsgCard,
	saveMsgChest,
	saveMsgSave;
}

#endif
