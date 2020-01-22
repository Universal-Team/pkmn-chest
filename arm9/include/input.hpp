#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include <string>

#include "lang.hpp"

namespace Input {
	std::string getLine();
	std::string getLine(unsigned maxLength);

	int getInt();
	int getInt(unsigned max);

	bool getBool();
	bool getBool(std::string option1, std::string option2);

	void prompt(std::string message, const std::string &confirm = Lang::get("ok"));
}

#endif
