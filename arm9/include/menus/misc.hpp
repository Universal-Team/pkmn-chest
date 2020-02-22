#ifndef MENU_MISC
#define MENU_MISC

#include "Sav.hpp"

// Shows the form selection menu
int selectForm(int dexNo, int currentForm);

// Shows a list of items in a vector to select from
int selectItem(int current, std::set<int> validItems, const std::vector<std::string> &items);

// Shows a list of items in a vector to select from
int selectItem(int current, int start, int max, const std::vector<std::string> &items);

// Shows a list of items in a vector to select from
std::string selectItem(int current, const std::vector<std::string> &items);

// Shows the move editing menu
void selectMoves(PKX &pkm);

// Shows the nature selection menu
int selectNature(int currentNature);

// Shows the Pokéball selection menu
int selectPokeball(int currentBall);

// Shows the Wallpaper selection menu
int selectWallpaper(int currentWallpaper);

// Shows the box selection menu
int selectBox(int currentBox);

// Shows the origin editing menu
void selectOrigin(PKX &pkm);

// Shows the stat editing menu
void selectStats(PKX &pkm);

// Shows the type selection menu
int selectHPType(int current);

#endif
