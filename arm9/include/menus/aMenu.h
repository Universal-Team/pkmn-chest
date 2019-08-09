#ifndef MENU_A
#define MENU_A

#include <vector>

extern std::vector<std::pair<int, int>> aMenuButtons, aMenuEmptySlotButtons, aMenuTopBarButtons;

// Shows the A menu
int aMenu(int pkmPos, std::vector<std::pair<int, int>>& buttons, int buttonMode);

#endif
