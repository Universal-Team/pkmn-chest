#include "cardSaves.hpp"
#include "colors.hpp"
#include "graphics.hpp"
#include "lang.hpp"

auxspi_extra card_type = AUXSPI_FLASH_CARD;
sNDSHeader nds;
char gamename[13];
char gameid[5];

std::vector<std::string> goodTids = {
	"ADA", // Diamond
	"APA", // Pearl
	"CPU", // Platinum
	"IPK", // HeartGold
	"IPG", // SoulSilver
	"IRB", // Black
	"IRA", // White
	"IRE", // Black 2
	"IRD", // White 2
};

bool isValidTid(char* tid) {
	for(unsigned i=0;i<goodTids.size(); i++) {
		if(strncmp(tid, goodTids[i].c_str(), 3) == 0)	return true;
	}
	return false;
}

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
	FILE* out = fopen(cardSave, "wb");
	if(out) {
		unsigned char* buffer;
		if(card_type == AUXSPI_INFRARED) {
			int size = auxspi_save_size_log_2(card_type);
			int type = auxspi_save_type(card_type);
			int size_blocks;
			if(size < 16)
				size_blocks = 1;
			else
				size_blocks = 1 << (size - 16);
			u32 LEN = std::min(1 << size, 1 << 16);
			buffer = new unsigned char[LEN*size_blocks];
			auxspi_read_data(0, buffer, LEN*size_blocks, type, card_type);
			fwrite(buffer, 1, LEN*size_blocks, out);
		} else {
			int type = cardEepromGetType();
			int size = cardEepromGetSize();
			buffer = new unsigned char[size];
			cardReadEeprom(0, buffer, size, type);
			fwrite(buffer, 1, size, out);
		}
		delete[] buffer;
		fclose(out);
	}
}

bool restoreSave(void) {
	bool auxspi = card_type == AUXSPI_INFRARED;
	FILE* in = fopen(cardSave, "rb");
	if(in) {
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
				return false;
			}
			LEN = 1 << shift;
			num_blocks = 1 << (size - shift);
		} else {
			type = cardEepromGetType();
			size = cardEepromGetSize();
		}
		fseek(in, 0, SEEK_END);
		length = ftell(in);
		fseek(in, 0, SEEK_SET);
		if(length != (auxspi ? (int)(LEN*num_blocks) : size)) {
			drawRectangle(20, 20, 216, 152, DARK_RED, true, true);
			printTextCentered(Lang::get("wrongSaveSize"), 0, 24, true, true);
			for(int i=0;i<120;i++)	swiWaitForVBlank();
			drawRectangle(20, 20, 216, 152, CLEAR, true, true);
			fclose(in);
			return false;
		}
		if(type == 3) {
			if(auxspi)
				auxspi_erase(card_type);
			else
				cardEepromChipErase();
		}
		if(auxspi){
			buffer = new unsigned char[LEN];
			drawOutline(5, 39, 247, 18, DARKERER_GRAY, false, true);
			for(unsigned int i = 0; i < num_blocks; i++) {
				drawRectangle((((float)i/num_blocks)*245)+6, 40, 1, 16, LIGHT_GRAY, false, true);

				fread(buffer, 1, LEN, in);
				auxspi_write_data(i << shift, buffer, LEN, type, card_type);
			}
			drawRectangle(4, 39, 248, 18, CLEAR, false, true);
		} else {
			int blocks = size / 32;
			int written = 0;
			buffer = new unsigned char[blocks];
			drawOutline(5, 39, 247, 18, DARKERER_GRAY, false, true);
			for(unsigned int i = 0; i < 32; i++) {
				drawRectangle(((i/32)*245)+6, 40, 8, 16, LIGHT_GRAY, false, true);
				fread(buffer, 1, blocks, in);
				cardWriteEeprom(written, buffer, blocks, type);
				written += blocks;
			}
			drawRectangle(4, 39, 248, 18, CLEAR, false, true);
		}
		delete[] buffer;
		fclose(in);
	}
	return true;
}
