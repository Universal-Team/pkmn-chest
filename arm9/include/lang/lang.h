#ifndef LANG_H
#define LANG_H

#include <string>

namespace Lang {
	enum Language {de, en, es, fr, it, lt, pt, ru, jp, ko};

	void loadLangStrings(int lang);
}

#endif
