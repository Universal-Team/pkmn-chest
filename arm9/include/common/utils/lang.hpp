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
	enum Language : int {de, en, es, fr, it, lt, pt, ru, jp, ko, br};
	// In game text strings
	extern std::vector<std::string> abilities, games, items, locations4, locations5, moves, natures, species;

	void load(int lang);
	std::string get(const std::string &key);
}

#endif
