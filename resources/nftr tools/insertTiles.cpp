#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

uint8_t tileWidth;
uint8_t tileHeight;
uint16_t tileSize;

std::pair<int, int> loadBmp16(std::string path, std::vector<uint8_t> &imageBuffer) {
	FILE* file = fopen(path.c_str(), "rb");
	std::pair<int, int> imageData = std::pair<int, int>(0, 0);

	if(file) {
		// Get width and height on image
		char buffer[4];
		fseek(file, 0x12, SEEK_SET); // Width
		fread(buffer, 4, 1, file);
		imageData.first = *(int*)&buffer[0];
		fseek(file, 0x16, SEEK_SET); // Height
		fread(buffer, 4, 1, file);
		imageData.second = *(int*)&buffer[0];

		// Load pixels
		fseek(file, 0xA, SEEK_SET); // Get pixel start location
		fseek(file, (uint8_t)fgetc(file), SEEK_SET); // Seek to pixel start location
		uint8_t bmpImageBuffer[imageData.first*imageData.second];
		fread(bmpImageBuffer, 1, imageData.first*imageData.second, file);
		for(int y=imageData.second-1; y>=0; y--) {
			uint8_t* src = bmpImageBuffer+y*(imageData.first/2);
			for(unsigned x=0;x<imageData.first;x+=2) {
				uint8_t val = *(src++);
				imageBuffer.push_back((val>>4)&3);
				imageBuffer.push_back(val&0xF);
			}
		}
	} else {
		printf("in.bmp not found!");
		return imageData;
	}
	fclose(file);

	return imageData;
}

std::vector<uint8_t> convertToTiles(std::vector<uint8_t> imageBuffer, int imageWidth, int imageHeight) {
	std::vector<uint8_t> tileBuffer;

	for(int y=0;y<imageHeight;y+=tileHeight) {
		for(int x=0;x<imageWidth;x+=tileWidth) {
			for(int tY=0;tY<tileHeight;tY++) {
				for(int tX=0;tX<tileWidth;) {
					tileBuffer.push_back(imageBuffer[((y+tY)*imageWidth)+x+tX++] << 6 |
										 imageBuffer[((y+tY)*imageWidth)+x+tX++] << 4 |
										 imageBuffer[((y+tY)*imageWidth)+x+tX++] << 2 |
										 imageBuffer[((y+tY)*imageWidth)+x+tX++]);
				}
			}
			for(int i=(tileWidth*tileHeight)/4;i<tileSize;i++) {
				tileBuffer.push_back(0);
			}
		}
	}

	return tileBuffer;
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
	uint32_t chunkSize = *(uint32_t*)(font.data()+0x30);
	tileWidth = *(font.data()+0x34);
	tileHeight = *(font.data()+0x35);
	tileSize = *(uint16_t*)(font.data()+0x36);

	std::vector<uint8_t> imageBuffer;
	std::pair<int, int> imageInfo = loadBmp16("in.bmp", imageBuffer);
	if(imageInfo.first == 0)	return 2;

	std::vector<uint8_t> tileBuffer = convertToTiles(imageBuffer, imageInfo.first, imageInfo.second);

	memcpy(font.data()+0x3C, tileBuffer.data(), chunkSize-0x10);

	FILE* out = fopen("out.nftr", "wb");
	fwrite(font.data(), 1, font.size(), out);
}