#ifndef CARD_SAVES_HPP
#define CARD_SAVES_HPP
#include "auxspi.hpp"

#define cardSave "sd:/_nds/pkmn-chest/backups/card.sav"

extern auxspi_extra card_type;
extern sNDSHeader nds;
extern char gamename[13];
extern char gameid[5];

bool isValidTid(char* tid);

bool updateCardInfo(void);
bool updateCardInfo(sNDSHeader* nds, char* gameid, char* gamename, auxspi_extra* card_type);

void dumpSave(void);
bool restoreSave(void);

#endif
