#ifndef SORT_HPP
#define SORT_HPP

enum class SortType {
	NONE,
	DEX,
	SPECIESNAME,
	FORM,
	TYPE1,
	TYPE2,
	HP,
	ATK,
	DEF,
	SATK,
	SDEF,
	SPE,
	HPIV,
	ATKIV,
	DEFIV,
	SATKIV,
	SDEFIV,
	SPEIV,
	NATURE,
	LEVEL,
	TID,
	HIDDENPOWER,
	FRIENDSHIP,
	NICKNAME,
	OTNAME,
	SHINY
};

void sortMenu(bool top);

#endif