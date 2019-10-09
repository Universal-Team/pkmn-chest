#ifndef MENU_MISC
#define MENU_MISC

#include "Sav.hpp"

// Returns the language in the correct order for a Pokémon
int pkmLang(void);

// Shows the form selection menu
int selectForm(int dexNo, int currentForm);

// Shows a list of items in a vector to select from
int selectItem(int current, int start, int max, std::vector<std::string> &items);

// Shows the move editing menu
std::shared_ptr<PKX> selectMoves(std::shared_ptr<PKX> pkm);

// Shows the nature selection menu
int selectNature(int currentNature);

// Shows the Pokéball selection menu
int selectPokeball(int currentBall);

// Shows the Wallpaper selection menu
int selectWallpaper(int currentWallpaper);

// Shows the box selection menu
int selectBox(int currentBox);

// Shows the origin editing menu
std::shared_ptr<PKX> selectOrigin(std::shared_ptr<PKX> pkm);

// Shows the stat editing menu
std::shared_ptr<PKX> selectStats(std::shared_ptr<PKX> pkm);

#endif
