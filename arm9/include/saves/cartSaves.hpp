#ifndef CART_SAVES_HPP
#define CART_SAVES_HPP

#define cartSave "/_nds/pkmn-chest/backups/cart.sav"

extern char slot2Name[13];
extern char slot2ID[5];

bool isValidGBATid(char *tid);

bool updateCartInfo(void);

void dumpSlot2(void);
bool restoreSlot2(void);

#endif
