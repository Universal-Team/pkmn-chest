#include <cstdint>
#include <math.h>
#include <string>
#include <string.h>
#include <vector>

struct Image {
	uint32_t height;
	uint32_t width;
	std::vector<uint8_t> bitmap;
	std::vector<uint16_t> palette;
};

Image loadBmp16(std::string path, int paletteOffset, int paletteCount) {
	FILE* file = fopen(path.c_str(), "rb");
	Image image = {0, 0};

	if(file) {
		// Get width and height on image
		fseek(file, 0x12, SEEK_SET);
		fread(&image.width, 4, 1, file); // Width
		fread(&image.height, 4, 1, file); // Height
		fseek(file, 2, SEEK_CUR); // Skip color planes
		uint16_t bitDepth;
		fread(&bitDepth, 2, 1, file); // Bit depth
		uint32_t numColors;
		fseek(file, 0x2E, SEEK_SET); // Skip to palette count
		fread(&numColors, 4, 1, file); // Palette count
		if(numColors == 0)	numColors = pow(2, bitDepth);

		// Load palette
		uint32_t palTemp[numColors];
		image.palette = std::vector<uint16_t>(numColors);
		fseek(file, 0x0E, SEEK_SET);
		fseek(file, (uint8_t)fgetc(file)-2, SEEK_CUR); // Seek to palette start location
		fread(palTemp, 4, numColors, file);
		for(int i=0;i<numColors;i++) {
			int r = round((((palTemp[i]>>24)&0xff)*31)/255.0);
			int g = round((((palTemp[i]>>16)&0xff)*31)/255.0);
			int b = round((((palTemp[i]>>8)&0xff)*31)/255.0);
			image.palette[i] = r | g<<5 | b<<10 | 1<<15;
			if(i > 0 && palTemp[i] == palTemp[i-1] && i > paletteCount) {
				image.palette.resize(i);
				break;
			}
			if(image.palette[i] == 0xfc1f)	image.palette[i] = 0;
		}

		// for(unsigned int i=0;i<image.palette.size();i++) {
		// 	printf("%x|", image.palette[i]);
		// }
		// printf("\n");

		int ppb = 1.0 / bitDepth * 8; // Pixels Per Byte
		int rowWidth = ((bitDepth*image.width+31)/32)*4;

		// Load pixels
		fseek(file, 0xA, SEEK_SET); // Get pixel start location
		int pixelStart = (uint8_t)fgetc(file);
		// printf("W: %d, rW: %d, H: %d, P: 0x%x\n", image.width, rowWidth, image.height, pixelStart);
		fseek(file, pixelStart, SEEK_SET); // Seek to pixel start location
		uint8_t bmpImageBuffer[image.height*rowWidth];
		fread(bmpImageBuffer, 1, image.height*rowWidth, file);
		for(int y=image.height-1; y>=0; y--) {
			// if(ppb == 1)image.bitmap.push_back(0);
			uint8_t* src = bmpImageBuffer+(y*rowWidth);
			for(unsigned int x=0;x<image.width;x+=ppb) {
				uint8_t val = *(src++);
				switch(ppb) {
					case 1:
						image.bitmap.push_back(val);
						break;
					case 2:
						image.bitmap.push_back(val>>4); // First nibble
						// printf("%x", val>>4);
						if(!(image.width%2 && x == image.width-1)) {
							image.bitmap.push_back(val&0xF); // Second nibble
							// printf("%x", val&0xF);
						}
						break;
					case 8:
						image.bitmap.push_back((val >> 7) & 1); // First bit
						image.bitmap.push_back((val >> 6) & 1); // Second bit
						image.bitmap.push_back((val >> 5) & 1); // Third bit
						image.bitmap.push_back((val >> 4) & 1); // Fourth bit
						image.bitmap.push_back((val >> 3) & 1); // Fifth bit
						image.bitmap.push_back((val >> 2) & 1); // Sixth bit
						image.bitmap.push_back((val >> 1) & 1); // Seventh bit
						image.bitmap.push_back((val >> 0) & 1); // Eighth bit
						break;
				}
			}
			// printf("|\n");
		}
		fclose(file);
	}
	return image;
}

void offsetPalette(Image &image, int paletteOffset) {
	for(unsigned int i=0;i<image.bitmap.size();i++) {
		image.bitmap[i] += paletteOffset;
	}
}

void exportGfx(std::string path, Image &image, int paletteOffset) {
	FILE* file = fopen(path.c_str(), "wb");

	if(file) {
		fwrite(".GFX", 1, 4, file);
		fwrite(&image.width, 1, 2, file);
		fwrite(&image.height, 1, 2, file);
		fwrite(image.bitmap.data(), 1, image.bitmap.size(), file);
		uint16_t palSize = image.palette.size();
		fwrite(&palSize, 1, 2, file);
		fwrite(&paletteOffset, 1, 2, file);
		fwrite(image.palette.data(), 2, image.palette.size(), file);
		fclose(file);
	}
}

int main(int argc, char *argv[]) {
	int paletteOffset = 0, paletteCount = 0;
	if(argc < 3) {
		printf("usage: bmp2ds in.bmp out.gfx [-o paletteOffest] [-c paletteCount]\n");
		return 1;
	}

	std::string in = argv[1], out = argv[2];

	for(int i=0;i<argc;i++) {
		if(strcmp(argv[i], "-o") == 0) {
			if(i+1 < argc) {
				paletteOffset = std::stoi(argv[i+1], nullptr, 16);
			}
		} else if(strcmp(argv[i], "-c") == 0) {
			if(i+1 < argc) {
				paletteCount = std::stoi(argv[i+1], nullptr, 16);
			}
		}
	}

	Image bmp = loadBmp16(in, paletteOffset, paletteCount);
	offsetPalette(bmp, paletteOffset);
	exportGfx(out, bmp, paletteOffset);
}
