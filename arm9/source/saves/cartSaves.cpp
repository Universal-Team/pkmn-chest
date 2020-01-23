#include "cartSaves.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "lang.hpp"

#define MAGIC_EEPR 0x52504545
#define MAGIC_SRAM 0x4D415253
#define MAGIC_FLAS 0x53414C46
#define MAGIC_H1M_ 0x5F4D3148

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
	strncpy(slot2Name, (char*)0x080000A0, sizeof(slot2Name)-1);
	slot2Name[sizeof(slot2Name)-1] = '\0';
	strncpy(slot2ID, (char*)0x080000AC, sizeof(slot2ID)-1);
	slot2Name[sizeof(slot2Name)-1] = '\0';

	return true;
}

bool isCorrectType(void) {
	// Search for a save version string in the ROM
	u32 *data = (u32*)0x08000000;

	for(int i=0;i<(0x02000000 >> 2); i++, data++) {
		switch(*data) {
			case MAGIC_EEPR:
				return false;
			case MAGIC_SRAM:
				return false;
			case MAGIC_FLAS:
				if(*(data+1) == MAGIC_H1M_)	return true;
				else return false;
			default:
				break;
		}
	}

	return false;
}

void dumpSlot2(void) {
	// Check that its the right save type
	if(isCorrectType()) {
		std::shared_ptr<u8[]> buffer = std::shared_ptr<u8[]>(new u8[0x20000]);
		u8 *dst = buffer.get();
		for (int bank = 0; bank < 2; bank++) {
			// FLASH - must be opened by register magic, then blind copy
			// we need to wait a few cycles before the hardware reacts!
			*(u8*)0x0A005555 = 0xAA;
			swiDelay(10);
			*(u8*)0x0A002AAA = 0x55;
			swiDelay(10);
			*(u8*)0x0A005555 = 0xB0;
			swiDelay(10);
			*(u8*)0x0A000000 = (u8)bank;
			swiDelay(10);

			u8* src = (u8*)0x0A000000;
			sysSetBusOwners(true, true);
			for(u32 i=0;i < 0x10000;i++) {
				*dst++ = *src++;
			}
		}

		FILE *file = fopen(cartSave, "wb");
		if(file) {
			fwrite(buffer.get(), 1, 0x20000, file);
			fclose(file);
		}
	}
}

bool restoreSlot2(void) {
	// Check that its the right save type
	if(isCorrectType()) {
		FILE *file = fopen(cartSave, "wb");
		if(file) {
			// Draw progress bar outline
			drawOutline(5, 39, 247, 18, DARKERER_GRAY, false, true);

			// FLASH - must be opened by register magic, erased and then rewritten
			// FIXME: currently, you can only write "all or nothing"
			for (int bank = 0; bank < 2; bank++) {
				*(u8*)0x0A005555 = 0xAA;
				swiDelay(10);
				*(u8*)0x0A002AAA = 0x55;
				swiDelay(10);
				*(u8*)0x0A005555 = 0xB0;
				swiDelay(10);
				*(u8*)0x0A000000 = (u8)bank;
				swiDelay(10);

				u8 *dst = (u8*)0x0A000000;
				sysSetBusOwners(true, true);
				for (u32 i = 0; i < 0x10000; i++, dst++) {
					// we need to wait a few cycles before the hardware reacts!
					*(u8*)0x0a005555 = 0xaa;
					swiDelay(10);
					*(u8*)0x0a002aaa = 0x55;
					swiDelay(10);
					*(u8*)0x0a005555 = 0xA0; // write byte command
					swiDelay(10);

					u8 src = fgetc(file);

					*dst = src;
					while(*dst != src)	swiDelay(10);

					// Draw progress bar
					drawRectangle((((float)((0x10000*bank)+i)/0x20000)*245)+6, 40, 1, 16, LIGHT_GRAY, false, true);
				}
			}
			fclose(file);
			return 1;
		}
	} else {
		Lang::get("wrongSaveCart");
	}
	return 0;
}
