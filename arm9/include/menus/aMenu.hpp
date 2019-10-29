#ifndef MENU_A
#define MENU_A

#include <string>
#include <vector>

#include "lang.hpp"

extern std::vector<Label> aMenuButtons, aMenuEmptySlotButtons, aMenuTopBarButtons;

// Shows the A menu
int aMenu(int pkmPos, std::vector<Label>& buttons, int buttonMode);

#endif
