#ifndef PARTY_HPP
#define PARTY_HPP

#include <utility>

#define PARTY_TRAY_X 20
#define PARTY_TRAY_Y 48

const std::pair<int, int> partySpritePos[] = {
	{4,  0}, {44,  8},
	{4, 32}, {44, 40},
	{4, 64}, {44, 72},
};

extern bool partyShown;
extern int partyX, partyY;

void fillPartySprites(void);
void toggleParty(void);
void moveParty(int arrowMode, bool holdingPokemon);

#endif
