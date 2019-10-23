#include <stdio.h>
#include <vector>

int increaseAmount = 23;
uint16_t tileSize;

int amountToIncrease(bool tile, bool width) {
	int out = 0;
	if(tile)	out += increaseAmount*tileSize;
	if(width)	out += increaseAmount*3;
	while(out%4)	out++;
	return out;
}

int main() {
	// Get file size
	FILE* in = fopen("in.nftr", "rb");
	if(!in) {
		printf("in.nftr not found!\n");
		return 1;
	}
	fseek(in, 0, SEEK_END);
	uint32_t fileSize = ftell(in);
	fseek(in, 0, SEEK_SET);

	// Read file to array
	std::vector<uint8_t> font(fileSize);
	fread(font.data(), 1, fileSize, in);
	fclose(in);

	// Load font info
	tileSize = *(uint16_t*)(font.data()+0x36);

	*(uint32_t*)(font.data()+0x30) += amountToIncrease(true, false); // Increase chunk size
	uint32_t chunkSize = *(uint32_t*)(font.data()+0x30);

	printf("Increase: %d\n", amountToIncrease(true, false));
	printf("Increase: %d\n", amountToIncrease(false, true));
	printf("Increase: %d\n", amountToIncrease(true, true));

	uint32_t locHDWC = *(uint32_t*)(font.data()+0x24);

	// Add blank character glyphs
	int tileAmount = ((chunkSize-0x10)/tileSize);
	printf("%d:%x\n", tileAmount, tileAmount);
	font.insert(font.begin()+locHDWC-8, amountToIncrease(true, false), 0);
	locHDWC += amountToIncrease(true, false);

	*(uint32_t*)(font.data()+locHDWC+2) += increaseAmount; // Increase max character

	// Increase character widths offset
	*(uint32_t*)(font.data()+0x24) = locHDWC;

	uint32_t locPAMC = *(uint32_t*)(font.data()+0x28) + amountToIncrease(true, false);

	*(uint32_t*)(font.data()+locHDWC-4) += amountToIncrease(false, true); // Increase chunk size
	chunkSize = *(uint32_t*)(font.data()+locHDWC-4);

	// Add blank character widths
	font.insert(font.begin()+locPAMC-8, amountToIncrease(false, true), 0);
	locPAMC += amountToIncrease(false, true);

	*(uint32_t*)(font.data()+0x28) = locPAMC;

	// Increase character maps offsets
	while(locPAMC < font.size()) {
		const uint8_t* fontp = font.data()+locPAMC;
		printf("Map: %d, %x\n", *(uint32_t*)(font.data()+locPAMC+4), *(uint32_t*)(font.data()+locPAMC+8));
		*(uint32_t*)(font.data()+locPAMC+8) += amountToIncrease(true, true);
		locPAMC = *(uint32_t*)(font.data()+locPAMC+8);

		uint16_t firstChar = *(uint16_t*)fontp;
		fontp += 2;
		uint16_t lastChar = *(uint16_t*)fontp;
		fontp += 2;
		uint32_t mapType = *(uint32_t*)fontp;
		fontp += 4;
		locPAMC = *(uint32_t*)fontp;
		fontp += 4;

		switch(mapType) {
			case 2: {
				uint16_t groupAmount = *(uint16_t*)fontp;
				fontp += 2;
				for(int i=0;i<groupAmount;i++) {
					uint16_t charNo = *(uint16_t*)fontp;
					fontp += 2;
					uint16_t tileNo = *(uint16_t*)fontp;
					fontp += 2;
					// printf("%x: %x\n", charNo, tileNo);
				}
				break;
			}
		}
	}

	// Write new size to header
	*(uint32_t*)(font.data()+8) = font.size();

	// Write output
	FILE* out = fopen("out.nftr", "wb");
	fwrite(font.data(), 1, font.size(), out);
	fclose(out);
}