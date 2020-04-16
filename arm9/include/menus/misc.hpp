#ifndef MENU_MISC
#define MENU_MISC

#include "Nature.hpp"
#include "Sav.hpp"
#include "Type.hpp"

// Shows the form selection menu
int selectForm(const PKX &pkm);

// Shows a list of items in a vector to select from
std::string selectItem(int current, const std::vector<std::string> &strings);

// Shows a list of items in a vector to select from
template <typename T>
T selectItem(T current, const std::set<T> &validItems, const std::vector<std::string>& strings) {
	std::vector<std::string> availableItems;
	for(unsigned int i=0;i<strings.size();i++) {
		if(validItems.count(T(i)) != 0) {
			availableItems.push_back(strings[i]);
		}
	}

	if((int)current < 0)
		current = T(0);
	else if((int)current > (int)availableItems.size()-1)
		current = T(availableItems.size()-1);
	std::string selection = selectItem((int)current, availableItems);

	for(unsigned int i=0;i<strings.size();i++) {
		if(strings[i] == selection)	return T(i);
	}
	return current;
}

// Shows a list of items in a vector to select from
template <typename T>
T selectItem(T current, int start, int max, const std::vector<std::string> &strings) {
	if((int)current < start || (int)current > max)	current = T(start);
	std::string selection = selectItem((int)current, std::vector<std::string>(&strings[start], &strings[max]));

	for(unsigned int i=0;i<strings.size();i++) {
		if(strings[i] == selection)	return T(i);
	}

	return current;
}

// Shows the move editing menu
void selectMoves(PKX &pkm);

// Shows the nature selection menu
Nature selectNature(Nature currentNature);

// Shows the Pokéball selection menu
Ball selectPokeball(Ball currentBall);

// Shows the Wallpaper selection menu
int selectWallpaper(int currentWallpaper);

// Shows the box selection menu
int selectBox(int currentBox);

// Shows the origin editing menu
void selectOrigin(PKX &pkm);

// Shows the stat editing menu
void selectStats(PKX &pkm);

// Shows the type selection menu
Type selectHPType(Type type);

#endif
