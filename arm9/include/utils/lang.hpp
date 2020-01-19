#ifndef LANG_HPP
#define LANG_HPP
#include <string>
#include <vector>

// Position & label struct
struct Label {
	int x;
	int y;
	std::string label;
};

namespace Lang {
	enum Language : int {br, de, en, es, fr, id, it, lt, pt, ru, jp, ko};
	// In game text strings
	extern std::vector<std::string> abilities, games, items, locations4, locations5, moves, natures, species;

	void load(int lang);
	std::string get(const std::string &key);
}

#endif
