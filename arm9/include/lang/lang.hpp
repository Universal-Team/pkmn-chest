#ifndef LANG_HPP
#define LANG_HPP
#include <string>

namespace Lang {
	enum Language {de, en, es, fr, it, lt, pt, ru, jp, ko};

	void loadLangStrings(int lang);
}

#endif
