#include <fstream>
#include "saves/cardSaves.h"
#include "graphics/graphics.h"

auxspi_extra card_type = AUXSPI_FLASH_CARD;
sNDSHeader nds;
char gamename[13];
char gameid[5];

bool updateCardInfo(void) {
	// nds.gameCode[0] = 0;
	// nds.gameTitle[0] = 0;
	return updateCardInfo(&nds, &gameid[0], &gamename[0], &card_type);
}

bool updateCardInfo(sNDSHeader* nds, char* gameid, char* gamename, auxspi_extra* card_type) {
	cardReadHeader((uint8*)nds);
	*card_type = auxspi_has_extra();
	int type = cardEepromGetType();
	int size = cardEepromGetSize();
	if(type == 999 || size < 1) {
		return false;
	}
	memcpy(gameid, nds->gameCode, 4);
	gameid[4] = 0x00;
	memcpy(gamename, nds->gameTitle, 12);
	gamename[12] = 0x00;
	return true;
}

void dumpSave(void) {
	std::ofstream output("sd:/_nds/pkmn-chest/backups/card.sav", std::ofstream::binary);
	if(output.is_open()) {
		unsigned char* buffer;
		if(card_type == AUXSPI_INFRARED) {
			int size = auxspi_save_size_log_2(card_type);
			int size_blocks = 1 << std::max(0, (int8(size) - 18));
			int type = auxspi_save_type(card_type);
			if(size < 16)
				size_blocks = 1;
			else
				size_blocks = 1 << (size - 16);
			u32 LEN = std::min(1 << size, 1 << 16);
			buffer = new unsigned char[LEN*size_blocks];
			auxspi_read_data(0, buffer, LEN*size_blocks, type, card_type);
			output.write((char*)buffer, LEN*size_blocks);
		} else {
			int type = cardEepromGetType();
			int size = cardEepromGetSize();
			buffer = new unsigned char[size];
			cardReadEeprom(0, buffer, size, type);
			output.write((char*)buffer, size);
		}
		delete[] buffer;
	}
	output.close();
}

void restoreSave(auxspi_extra card_type, char gameid[]) {
	// sprintf(txt, "Press A to load %s.sav.\n", gameid);
	// WriteMessage(txt, true);
	// WriteMessage("Press X to manually select the file to load (use this also if garbage text is displayed)\n");
	// NameUpdate(gameid, false);
	bool auxspi = card_type == AUXSPI_INFRARED;
	std::ifstream input("/_nds/pkmn-chest/backups/card.sav", std::ifstream::binary);
	if(input.is_open()) {
		unsigned char* buffer;
		int size;
		int type;
		int length;
		unsigned int num_blocks = 0, shift = 0, LEN = 0;
		if(auxspi) {
			size = auxspi_save_size_log_2(card_type);
			type = auxspi_save_type(card_type);
			switch(type) {
			case 1:
				shift = 4; // 16 bytes
				break;
			case 2:
				shift = 5; // 32 bytes
				break;
			case 3:
				shift = 8; // 256 bytes
				break;
			default:
				return;
			}
			LEN = 1 << shift;
			num_blocks = 1 << (size - shift);
		} else {
			type = cardEepromGetType();
			size = cardEepromGetSize();
		}
		input.seekg(0, input.end);
		length = input.tellg();
		input.seekg(0, input.beg);
		if(length != (auxspi ? (int)(LEN*num_blocks) : size)) {
			// WriteMessage("The size of the loaded file doesn't match the size of the save for the cartridge!\n", true);
			// WriteMessage("Press A to continue\n");
			// wait();
			input.close();
			return;
		}
		if(type == 3) {
			// WriteMessage("The savefile in the cartridge has to be cleared, press A to continue, B to cancel\n", true);
			// if(wait(true) == KEY_B) {
				// input.close();
				// return;
			// }
			// WriteMessage("Deleting the previous savefile\n", true);
			if(auxspi)
				auxspi_erase(card_type);
			else
				cardEepromChipErase();
			// WriteMessage("Savefile deleted\n");
		}
		// WriteMessage("Savefile loaded, press A to write it in the cartridge, B to cancel\n", true);
		// if(wait(true) == KEY_B) {
		// 	input.close();
		// 	return;
		// }
		if(auxspi){
			buffer = new unsigned char[LEN];
			int step = num_blocks / 32;
			for(unsigned int i = 0; i < num_blocks; i++) {
				if(i % step == 0)
					printText("#", (i/step)*16, 32, false);

				input.read((char*)buffer, LEN);
				auxspi_write_data(i << shift, buffer, LEN, type, card_type);
			}
		} else {
			int blocks = size / 32;
			int written = 0;
			buffer = new unsigned char[blocks];
			for(unsigned int i = 0; i < 32; i++) {
				printText("#", (i)*16, 32, false);
				input.read((char*)buffer, blocks);
				cardWriteEeprom(written, buffer, blocks, type);
				written += blocks;
			}
		}
		delete[] buffer;
		// WriteMessage("Savefile successfully written!\n");
	} else {
		// WriteMessage("Savefile not found!\n");
	}
	input.close();
	// WriteMessage("Press A to continue\n");
	// wait();
}