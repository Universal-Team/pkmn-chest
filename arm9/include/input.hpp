#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP
#include <string>

namespace Input {
	std::string getLine();
	std::string getLine(unsigned maxLength);
	int getInt();
	int getInt(unsigned max);
	bool getBool();
	bool getBool(std::string option1, std::string option2);
}

#endif
