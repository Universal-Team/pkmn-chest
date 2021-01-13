#ifndef GUI_HPP
#define GUI_HPP

#include "colors.hpp"
#include "config.hpp"
#include "graphics.hpp"
#include "i18n_ext.hpp"

#include <nds.h>

struct Label {
	int x;
	int y;
	std::string label;
};

namespace Gui {
	extern Font font;

	void prompt(int message, const std::string &confirm = i18n::localize(Config::getLang("lang"), "ok"));
	void prompt(std::string message, const std::string &confirm = i18n::localize(Config::getLang("lang"), "ok"));

	void warn(int message, const std::string &confirm = i18n::localize(Config::getLang("lang"), "ok"));
	void warn(std::string message, const std::string &confirm = i18n::localize(Config::getLang("lang"), "ok"));
}

#endif
