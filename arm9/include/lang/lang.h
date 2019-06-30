#ifndef LANG_H
#define LANG_H

#include <string>
#include <vector>

namespace Lang {
	extern std::vector<std::string> balls, items, locations4, locations5, moves, natures, species;
	void loadLangStrings(int lang);
}

#endif
