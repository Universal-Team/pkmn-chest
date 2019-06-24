#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <string>

namespace Input {
	std::string getLine();
	std::string getLine(uint maxLength);
	int getInt();
	int getInt(uint max);
	bool getBool();
	bool getBool(std::string option1, std::string option2);
}

#endif
