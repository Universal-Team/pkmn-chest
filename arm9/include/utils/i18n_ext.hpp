#ifndef I18N_EXT_HPP
#define I18N_EXT_HPP

#include "Sav.hpp"

namespace i18n {
	const std::string &pouch(Language lang, Sav::Pouch);

	void initGui(Language lang);
	void exitGui(Language lang);
	const std::string &localize(Language lang, const std::string &v);
}

#endif
