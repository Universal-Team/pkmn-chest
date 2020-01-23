#ifndef GUI_HPP
#define GUI_HPP

#include "lang.hpp"

namespace Gui {
	void prompt(std::string message, const std::string &confirm = Lang::get("ok"));
	void warn(std::string message, const std::string &confirm = Lang::get("ok"));
}

#endif
