#include "cartSaves.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "lang.hpp"

char slot2Name[13];
char slot2ID[5];

const std::vector<std::string> goodTids = {
	"AXP", // Sapphire
	"AXV", // Ruby
	"BPE", // Emerald
	"BPG", // Leaf Green
	"BPR", // Fire Red
};

bool isValidGBATid(char* tid) {
	for(unsigned i=0;i<goodTids.size(); i++) {
		if(strncmp(tid, goodTids[i].c_str(), 3) == 0)	return true;
	}
	return false;
}

bool updateCartInfo(void) {
	strcpy(slot2Name, (char*)0x080000A0);
	strcpy(slot2ID, (char*)0x080000AC);

	return true;
}

void dumpSlot2(void) {
	// Check that its the right save type
	if(strcmp("FLASH1M_V103", (char*)0x08654F00)) {
		FILE *file = fopen(cartSave, "wb");
		if(file) {
			for (int i = 0; i < 2; i++) {
				// FLASH - must be opened by register magic, then blind copy
				// we need to wait a few cycles before the hardware reacts!
				*(u8*)0x0A005555 = 0xAA;
				swiDelay(10);
				*(u8*)0x0A002AAA = 0x55;
				swiDelay(10);
				*(u8*)0x0A005555 = 0xB0;
				swiDelay(10);
				*(u8*)0x0A000000 = (u8)i;
				swiDelay(10);

				sysSetBusOwners(true, true);
				fwrite((void*)0x0A000000, 1, 0x10000, file);
			}
			fclose(file);
		}
	}
}

bool restoreSlot2(void) {
	// Check that its the right save type
	if(strcmp("FLASH1M_V103", (char*)0x08654F00)) {
		FILE *file = fopen(cartSave, "wb");
		if(file) {
			// FLASH - must be opened by register magic, erased and then rewritten
			// FIXME: currently, you can only write "all or nothing"
			for (int i = 0; i < 2; i++) {
				*(u8*)0x0A005555 = 0xAA;
				swiDelay(10);
				*(u8*)0x0A002AAA = 0x55;
				swiDelay(10);
				*(u8*)0x0A005555 = 0xB0;
				swiDelay(10);
				*(u8*)0x0A000000 = (u8)i;
				swiDelay(10);

				u8 *tmpdst = (u8*)0x0A000000;
				sysSetBusOwners(true, true);
				for (u32 i = 0; i < 0x10000; i++, tmpdst++) {
					// we need to wait a few cycles before the hardware reacts!
					*(u8*)0x0a005555 = 0xaa;
					swiDelay(10);
					*(u8*)0x0a002aaa = 0x55;
					swiDelay(10);
					*(u8*)0x0a005555 = 0xA0; // write byte command
					swiDelay(10);

					u8 src = fgetc(file);

					*tmpdst = src;
					while(*tmpdst != src)	swiDelay(10);
				}
			}
			fclose(file);
			return 1;
		}
	}
	return 0;
}
