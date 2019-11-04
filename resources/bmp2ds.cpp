#include <string>
#include <vector>

struct Image {
	uint16_t height;
	uint16_t width;
	std::vector<uint8_t> bitmap;
	std::vector<uint16_t> palette;
};

Image loadBmp16(std::string path, int paletteOffset) {
	FILE* file = fopen(path.c_str(), "rb");
	Image image = {0, 0};

	if(file) {
		// Get width and height on image
		char buffer[4];
		fseek(file, 0x12, SEEK_SET); // Width
		fread(buffer, 4, 1, file);
		image.width = *(int*)&buffer[0];
		fseek(file, 0x16, SEEK_SET); // Height
		fread(buffer, 4, 1, file);
		image.height = *(int*)&buffer[0];

		// Load palette
		uint32_t palTemp[16];
		image.palette = std::vector<uint16_t>(16);
		fseek(file, 0x0E, SEEK_SET);
		fseek(file, (uint8_t)fgetc(file)-2, SEEK_CUR); // Seek to palette start location
		fread(palTemp, 4, 16, file);
		for(int i=0;i<16;i++) {
			image.palette[i] = ((palTemp[i]>>27)&31) | ((palTemp[i]>>19)&31)<<5 | ((palTemp[i]>>11)&31)<<10 | 1<<15;
			if(image.palette[i] == 0xfc1f)	image.palette[i] = 0;
		}

		// Load pixels
		fseek(file, 0xA, SEEK_SET); // Get pixel start location
		fseek(file, (uint8_t)fgetc(file), SEEK_SET); // Seek to pixel start location
		uint8_t bmpImageBuffer[image.width*image.height];
		fread(bmpImageBuffer, 1, image.width*image.height, file);
		for(int y=image.height-1; y>=0; y--) {
			uint8_t* src = bmpImageBuffer+y*(image.width/2);
			for(unsigned x=0;x<image.width;x+=2) {
				uint8_t val = *(src++);
				image.bitmap.push_back(val>>4);  // First nibble
				image.bitmap.push_back(val&0xF); // Second nibble
			}
		}
	}
	fclose(file);
	return image;
}

void offsetPalette(Image &image, int paletteOffset) {
	for(unsigned int i=0;i<image.bitmap.size();i++) {
		image.bitmap[i] += paletteOffset;
	}
}

void exportGfx(std::string path, Image &image) {
	FILE* file = fopen(path.c_str(), "wb");

	if(file) {
		fwrite(".GFX", 1, 4, file);
		fwrite(&image.width, 1, 2, file);
		fwrite(&image.height, 1, 2, file);
		fwrite(image.bitmap.data(), 1, image.bitmap.size(), file);
		fwrite(image.palette.data(), 2, image.palette.size(), file);
	}

	fclose(file);
}

int main(int argc, char *argv[]) {
	int paletteOffset = 0;
	if(argc < 3) {
		printf("usage: bmp2ds in.bmp out.gfx [paletteOffest]\n");
		return 1;
	}

	std::string in = argv[1], out = argv[2];

	if(argc >= 4) {
		paletteOffset = std::stoi(argv[3]);
	}

	Image bmp = loadBmp16(in, paletteOffset);
	offsetPalette(bmp, paletteOffset);
	exportGfx(out, bmp);
}