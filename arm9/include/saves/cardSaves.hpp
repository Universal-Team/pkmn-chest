#ifndef CARD_SAVES_HPP
#define CARD_SAVES_HPP
#include "auxspi.hpp"

#define cardSave "sd:/_nds/pkmn-chest/backups/card.sav"

extern auxspi_extra card_type;
extern sNDSHeader nds;
extern char slot1Name[13];
extern char slot1ID[5];

bool isValidDSTid(char *tid);

bool updateCardInfo(void);
bool updateCardInfo(sNDSHeader *nds, char *gameid, char *gamename, auxspi_extra *card_type);

void dumpSlot1(void);
bool restoreSlot1(void);

#endif
