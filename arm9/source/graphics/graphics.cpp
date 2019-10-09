#include "graphics.hpp"
#include "lodepng.hpp"
#include "tonccpy.h"

#define WHITE 0xFFFF

std::vector<Sprite> sprites;
std::vector<char> fontTiles;
std::vector<char> fontWidths;
std::vector<u16> fontMap;
u16 tileSize, tileWidth, tileHeight;
std::u16string newline = StringUtils::UTF8toUTF16("»");

int getCharIndex(char16_t c) {
	int spriteIndex = 0;
	int left = 0;
	int mid = 0;
	int right = fontMap.size();

	while(left <= right) {
		mid = left + ((right - left) / 2);
		if(fontMap[mid] == c) {
			spriteIndex = mid;
			break;
		}

		if(fontMap[mid] < c) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}
	return spriteIndex;
}

void initGraphics(void) {
	// Initialize video mode
	videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

	// initialize all the VRAM banks
	vramSetPrimaryBanks(VRAM_A_MAIN_BG,
						VRAM_B_MAIN_SPRITE,
						VRAM_C_SUB_BG,
						VRAM_D_SUB_SPRITE);

	// Init oam with 1D mapping 128 byte boundary and no external palette support
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);

	// Init for background
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(3);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(3);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	// Set main background as target for sprite transparency
	REG_BLDCNT_SUB = 1<<11;
}

void loadFont(void) {
	FILE* font = fopen("nitro:/graphics/font.nftr", "rb");

	// Get file size
	fseek(font, 0, SEEK_END);
	u32 fileSize = ftell(font);

	// Load font info
	fseek(font, 0x30, SEEK_SET);
	u32 chunkSize;
	fread(&chunkSize, 4, 1, font);
	fseek(font, 0x34, SEEK_SET);
	tileWidth = fgetc(font);
	fseek(font, 0x35, SEEK_SET);
	tileHeight = fgetc(font);
	fseek(font, 0x36, SEEK_SET);
	fread(&tileSize, 2, 1, font);

	// Load character glyphs
	int tileAmount = ((chunkSize-0x10)/tileSize);
	fontTiles = std::vector<char>(tileSize*tileAmount);
	fseek(font, 0x3C, SEEK_SET);
	fread(fontTiles.data(), tileSize, tileAmount, font);

	// Fix top row
	for(int i=0;i<tileAmount;i++) {
		fontTiles[i*tileSize] = 0;
		fontTiles[i*tileSize+1] = 0;
		fontTiles[i*tileSize+2] = 0;
	}

	// Load character widths
	fseek(font, 0x24, SEEK_SET);
	u32 locHDWC;
	fread(&locHDWC, 4, 1, font);
	fseek(font, locHDWC-4, SEEK_SET);
	fread(&chunkSize, 4, 1, font);
	fseek(font, 8, SEEK_CUR);
	fontWidths = std::vector<char>(3*tileAmount);
	fread(fontWidths.data(), 3, tileAmount, font);

	// Load character maps
	fontMap = std::vector<u16>(tileAmount);
	fseek(font, 0x28, SEEK_SET);
	u32 locPAMC, mapType;
	fread(&locPAMC, 4, 1, font);

	while(locPAMC < fileSize) {
		u16 firstChar, lastChar;
		fseek(font, locPAMC, SEEK_SET);
		fread(&firstChar, 2, 1, font);
		fread(&lastChar, 2, 1, font);
		fread(&mapType, 4, 1, font);
		fread(&locPAMC, 4, 1, font);

		switch(mapType) {
			case 0: {
				u16 firstTile;
				fread(&firstTile, 2, 1, font);
				for(unsigned i=firstChar;i<=lastChar;i++) {
					fontMap[firstTile+(i-firstChar)] = i;
				}
				break;
			} case 1: {
				for(int i=firstChar;i<=lastChar;i++) {
					u16 tile;
					fread(&tile, 2, 1, font);
					fontMap[tile] = i;
				}
				break;
			} case 2: {
				u16 groupAmount;
				fread(&groupAmount, 2, 1, font);
				for(int i=0;i<groupAmount;i++) {
					u16 charNo, tileNo;
					fread(&charNo, 2, 1, font);
					fread(&tileNo, 2, 1, font);
					fontMap[tileNo] = charNo;
				}
				break;
			}
		}
	}
	fclose(font);
}

ImageData loadBmp16(std::string path, std::vector<u16> &imageBuffer) {
	FILE* file = fopen(path.c_str(), "rb");
	ImageData imageData = {0, 0};

	if(file) {
		// Get width and height on image
		char buffer[4];
		fseek(file, 0x12, SEEK_SET); // Width
		fread(buffer, 4, 1, file);
		imageData.width = *(int*)&buffer[0];
		fseek(file, 0x16, SEEK_SET); // Height
		fread(buffer, 4, 1, file);
		imageData.height = *(int*)&buffer[0];

		// Load palette
		u32 palTemp[16];
		u16 pal[16];
		fseek(file, 0x89, SEEK_SET);
		fread(palTemp, 4, 16, file);
		for(int i=0;i<16;i++) {
			pal[i] = ((palTemp[i]>>27)&31) | ((palTemp[i]>>19)&31)<<5 | ((palTemp[i]>>11)&31)<<10 | 1<<15;
		}

		// Load pixels
		fseek(file, 0xA, SEEK_SET); // Get pixel start location
		fseek(file, (u8)fgetc(file), SEEK_SET); // Seek to pixel start location
		u8 bmpImageBuffer[imageData.width*imageData.height];
		fread(bmpImageBuffer, 1, imageData.width*imageData.height, file);
		for(int y=imageData.height-1; y>=0; y--) {
			u8* src = bmpImageBuffer+y*(imageData.width/2);
			for(unsigned x=0;x<imageData.width;x+=2) {
				u8 val = *(src++);
				if(pal[val>>4] == 0xfc1f) { // First nibble
					imageBuffer.push_back(0<<15);
				} else {
					imageBuffer.push_back(pal[val>>4]);
				}
				if(pal[val&0xF] == 0xfc1f) { // Second nibble
					imageBuffer.push_back(0<<15);
				} else {
					imageBuffer.push_back(pal[val&0xF]);
				}
			}
		}
	}
	fclose(file);
	return imageData;
}

ImageData loadBmp(std::string path, std::vector<u16> &imageBuffer) {
	FILE* file = fopen(path.c_str(), "rb");

	// Get width and height on image
	ImageData imageData;
	char buffer[4];
	fseek(file, 0x12, SEEK_SET); // Width
	fread(buffer, 4, 1, file);
	imageData.width = *(int*)&buffer[0];
	fseek(file, 0x16, SEEK_SET); // Height
	fread(buffer, 4, 1, file);
	imageData.height = *(int*)&buffer[0];

	if(file) {
		// Start loading
		fseek(file, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(file) + 0xe;
		fseek(file, pixelStart, SEEK_SET);
		u16 bmpImageBuffer[imageData.width*imageData.height];
		fread(bmpImageBuffer, 2, imageData.width*imageData.height, file);
		for(int y=imageData.height-1; y>=0; y--) {
			u16* src = bmpImageBuffer+y*imageData.width;
			for(unsigned x=0;x<imageData.width;x++) {
				u16 val = *(src++);
				if(val == 0xfc1f) { // If a pixel is magenta (#ff00ff)
					imageBuffer.push_back(0<<15); // Save it as a transparent pixel
				} else {
					imageBuffer.push_back(((val>>10)&31) | (val&(31)<<5) | (val&(31))<<10 | BIT(15));
				}
			}
		}
	}
	fclose(file);
	return imageData;
}

ImageData loadPng(std::string path, std::vector<u16> &imageBuffer) {
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, path);
	for(unsigned i=0;i<image.size()/4;i++) {
		imageBuffer.push_back(ARGB16(image[(i*4)+3], image[i*4]>>3, image[(i*4)+1]>>3, image[(i*4)+2]>>3));
	}

	ImageData imageData;
	imageData.width = width;
	imageData.height = height;
	return imageData;
}

void drawImage(int x, int y, int w, int h, std::vector<u16> &imageBuffer, bool top) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			if(imageBuffer[(i*w)+j]>>15 != 0) { // Do not render transparent pixel
				(top ? BG_GFX : BG_GFX_SUB)[(y+i)*256+j+x] = imageBuffer[(i*w)+j];
			}
		}
	}
}

void drawImageFromSheet(int x, int y, int w, int h, std::vector<u16> &imageBuffer, int imageWidth, int xOffset, int yOffset, bool top) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			if(imageBuffer[((i+yOffset)*imageWidth)+j+xOffset]>>15 != 0) { // Do not render transparent pixel
				(top ? BG_GFX : BG_GFX_SUB)[((y+i)*256)+j+x] = imageBuffer[((i+yOffset)*imageWidth)+j+xOffset];
			}
		}
	}
}

void drawImageFromSheetScaled(int x, int y, int w, int h, double scaleX, double scaleY, std::vector<u16> &imageBuffer, int imageWidth, int xOffset, int yOffset, bool top) {
	if(scaleX == 1 && scaleY == 1)	drawImageFromSheet(x, y, w, h, imageBuffer, imageWidth, xOffset, yOffset, top);
	else {
		std::vector<u16> buffer;
		for(int i=0;i<h;i++) {
			for(int j=0;j<w;j++) {
				buffer.push_back(imageBuffer[((i+yOffset)*imageWidth)+j+xOffset]);
			}
		}
		drawImageScaled(x, y, w, h, scaleX, scaleY, buffer, top);
	}
}

void drawImageScaled(int x, int y, int w, int h, double scaleX, double scaleY, std::vector<u16> &imageBuffer, bool top) {
	if(scaleX == 1 && scaleY == 1)	drawImage(x, y, w, h, imageBuffer, top);
	else {
		for(int i=0;i<(h*scaleY);i++) {
			for(int j=0;j<(w*scaleX);j++) {
				if(imageBuffer[(((int)(i/scaleY))*w)+(j/scaleX)]>>15 != 0) { // Do not render transparent pixel
					(top ? BG_GFX : BG_GFX_SUB)[(y+i)*256+x+j] = imageBuffer[(((int)(i/scaleY))*w)+(j/scaleX)];
				}
			}
		}
	}
}

void drawImageTinted(int x, int y, int w, int h, u16 color, std::vector<u16> &imageBuffer, bool top) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			if(imageBuffer[(i*w)+j]>>15 != 0) { // Do not render transparent pixel
				(top ? BG_GFX : BG_GFX_SUB)[(y+i)*256+j+x] = color & imageBuffer[(i*w)+j];
			}
		}
	}
}

void drawOutline(int x, int y, int w, int h, int color, bool top) {
	h+=y;
	if(y>=0 && y<192)	dmaFillHalfWords(((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15), (top ? BG_GFX : BG_GFX_SUB)+((y*256)+(x < 0 ? 0 : x)), (x+w > 256 ? w+(256-x-w) : w*2));
	for(y++;y<(h-1);y++) {
		if(y>=0 && y<192 && x>0)	(top ? BG_GFX : BG_GFX_SUB)[(y)*256+x] = ((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15);
		if(y>=0 && y<192 && x+w<256)	(top ? BG_GFX : BG_GFX_SUB)[(y)*256+x+w-1] = ((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15);
	}
	if(y>=0 && y<192)	dmaFillHalfWords(((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15), (top ? BG_GFX : BG_GFX_SUB)+((y*256)+(x < 0 ? 0 : x)), (x+w > 256 ? w+(256-x-w) : w*2));
}

void drawRectangle(int x, int y, int w, int h, int color, bool top) { drawRectangle(x, y, w, h, color, color, top); }
void drawRectangle(int x, int y, int w, int h, int color1, int color2, bool top) {
	for(int i=0;i<h;i++) {
		dmaFillHalfWords((i%2 ? color1 : color2) | BIT(15), (top ? BG_GFX : BG_GFX_SUB)+((y+i)*256+x), w*2);
	}
}

int initSprite(SpriteSize spriteSize, bool top) {
	Sprite sprite = {0, spriteSize, SpriteColorFormat_Bmp, -1, 15, 0, 0, top};
	sprites.push_back(sprite);

	int id = sprites.size()-1;

	// Allocate memory for graphics
	sprites[id].gfx = oamAllocateGfx((top ? &oamMain : &oamSub), sprites[id].size, sprites[id].format);

	return id;
}

void fillSpriteColor(int id, u16 color) {
	int size = 0;
	switch(sprites[id].size) {
		default:
			size = 0; // I'm lazy
			break;
		case SpriteSize_16x16:
			size = 16*16*2;
			break;
		case SpriteSize_32x32:
			size = 32*32*2;
			break;
	}

	toncset16(sprites[id].gfx, color, size);
}

void fillSpriteImage(int id, std::vector<u16> &imageBuffer, int size) {
	tonccpy(sprites[id].gfx, imageBuffer.data(), size*2);
}

void fillSpriteImage(int id, int x, int y, int w, int h, std::vector<u16> &imageBuffer) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites[id].gfx[((y+i)*32)+(x+j)] = imageBuffer[((i)*w)+j];
		}
	}
}

void fillSpriteFromSheet(int id, std::vector<u16> &imageBuffer, int w, int h, int imageWidth, int xOffset, int yOffset) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites[id].gfx[(i*w)+j] = imageBuffer[((i+yOffset)*imageWidth)+j+xOffset];
		}
	}
}

void fillSpriteFromSheetScaled(int id, double scale, std::vector<u16> &imageBuffer, int w, int h, int imageWidth, int xOffset, int yOffset) {
	if(scale == 1)	fillSpriteFromSheet(id, imageBuffer, w, h, imageWidth, xOffset, yOffset);
	else {
		u16 ws = w*(u16)scale;
		scale = 1/scale;
		for(double i=0;i<h;i+=scale) {
			u16 ii=i;
			u16 is=i/scale;
			for(double j=0;j<w;j+=scale) {
				u16 jj=j;
				u16 js=j/scale;
				sprites[id].gfx[(is*ws)+js] = imageBuffer[((ii+yOffset)*imageWidth)+jj+xOffset];
			}
		}
	}
}

void fillSpriteFromSheetTinted(int id, std::vector<u16> &imageBuffer, u16 color, int w, int h, int imageWidth, int xOffset, int yOffset) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites[id].gfx[(i*w)+j] = color & imageBuffer[((i+yOffset)*imageWidth)+j+xOffset];
		}
	}
}

void fillSpriteText(int id, std::string text, u16 color, int xPos, int yPos, bool invert) { fillSpriteText(id, StringUtils::UTF8toUTF16(text), color, xPos, yPos, invert); };

void fillSpriteText(int id, std::u16string text, u16 color, int xPos, int yPos, bool invert) {
	u16 pallet[4] = {
		0,
		(u16)(color & (invert ? 0xBDEF : 0xFBDE)),
		(u16)(color & (invert ? 0xFBDE : 0xBDEF)),
		0
	};
	for(unsigned c=0;c<text.size();c++) {
		int t = getCharIndex(text[c]);
		std::vector<u16> image;
		for(int i=0;i<tileSize;i++) {
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>6 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>4 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>2 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)] & 3]);
		}

		xPos += fontWidths[t*3];
		fillSpriteImage(id, xPos, yPos, tileWidth, tileHeight, image);
		xPos += fontWidths[(t*3)+1];
	}
}

void prepareSprite(int id, int x, int y, int priority) {
	oamSet(
	(sprites[id].top ? &oamMain : &oamSub),	// Main/Sub display
	id,	// Oam entry to set
	x, y,	// Position
	priority, // Priority
	sprites[id].paletteAlpha, // Alpha for bmp sprite
	sprites[id].size,
	sprites[id].format,
	sprites[id].gfx,
	sprites[id].rotationIndex,
	false, // Don't double the sprite size for rotation
	false, // Don't hide the sprite
	false, false, // vflip, hflip
	false // Apply mosaic
	);
	sprites[id].x = x;
	sprites[id].y = y;
}

void updateOam(void) {
	oamUpdate(&oamSub);
	oamUpdate(&oamMain);
}

void setSpritePosition(int id, int x, int y) {
	oamSetXY((sprites[id].top ? &oamMain : &oamSub), id, x, y);
	sprites[id].x = x;
	sprites[id].y = y;
}

void setSpritePriority(int id, int priority) { oamSetPriority((sprites[id].top ? &oamMain : &oamSub), id, priority); }
void setSpriteVisibility(int id, int show) { oamSetHidden((sprites[id].top ? &oamMain : &oamSub), id, !show); }
Sprite getSpriteInfo(int id) { return sprites[id]; }
unsigned getSpriteAmount(void) { return sprites.size(); }

void printText(std::string text, int xPos, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), WHITE, xPos, yPos, top, invert); }
void printText(std::u16string text, int xPos, int yPos, bool top, bool invert) { printTextTinted(text, WHITE, xPos, yPos, top, invert); }
void printTextCentered(std::string text, int xOffset, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), WHITE, ((256-getTextWidth(StringUtils::UTF8toUTF16(text)))/2)+xOffset, yPos, top, invert); }
void printTextCentered(std::u16string text, int xOffset, int yPos, bool top, bool invert) { printTextTinted(text, WHITE, ((256-getTextWidth(text))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTinted(std::string text, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), color, ((256-getTextWidth(StringUtils::UTF8toUTF16(text)))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTinted(std::u16string text, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTinted(text, color, ((256-getTextWidth(text))/2)+xOffset, yPos, top, invert); }
void printTextTinted(std::string text, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), color, xPos, yPos, top, invert); }

void printTextTinted(std::u16string text, u16 color, int xPos, int yPos, bool top, bool invert) {
	int x=xPos;
	u16 pallet[4] = {
		0,
		(u16)(color & (invert ? 0xBDEF : 0xFBDE)),
		(u16)(color & (invert ? 0xFBDE : 0xBDEF)),
		0
	};
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == newline[0]) {
			x = xPos;
			yPos += tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		std::vector<u16> image;
		for(int i=0;i<tileSize;i++) {
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>6 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>4 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>2 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)] & 3]);
		}

		x += fontWidths[t*3];
		if(x > 256) {
			x = xPos+fontWidths[t*3];
			yPos += tileHeight;
		}
		drawImage(x, yPos, tileWidth, tileHeight, image, top);
		x += fontWidths[(t*3)+1];
	}
}

void printTextAbsW(std::string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), w/getTextWidth(text), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextAbsW(std::u16string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, w/getTextWidth(text), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextMaxW(std::string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextMaxW(std::u16string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextScaled(std::string text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, WHITE, xPos, yPos, top, invert); }
void printTextScaled(std::u16string text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, scaleX, scaleY, WHITE, xPos, yPos, top, invert); }
void printTextCenteredAbsW(std::string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), w/getTextWidth(text), scaleY, WHITE, ((256-w)/2)+xOffset, yPos, top, invert); }
void printTextCenteredAbsW(std::u16string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, w/getTextWidth(text), scaleY, WHITE, ((256-w)/2)+xOffset, yPos, top, invert); }
void printTextCenteredMaxW(std::string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, ((256-getTextWidthMaxW(text, w))/2)+xOffset, yPos, top, invert); }
void printTextCenteredMaxW(std::u16string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, ((256-getTextWidthMaxW(text, w))/2)+xOffset, yPos, top, invert); }
void printTextCenteredScaled(std::string text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, WHITE, ((256-getTextWidthScaled(text, scaleX))/2)+xOffset, yPos, top, invert); }
void printTextCenteredScaled(std::u16string text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, scaleX, scaleY, WHITE, ((256-getTextWidthScaled(text, scaleX))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedAbsW(std::string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), w/getTextWidth(text), scaleY, color, ((256-w)/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedAbsW(std::u16string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, w/getTextWidth(text), scaleY, color, ((256-w)/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedMaxW(std::string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, color, ((256-getTextWidthMaxW(text, w))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedMaxW(std::u16string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, color, ((256-getTextWidthMaxW(text, w))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedScaled(std::string text, double scaleX, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, color, ((256-getTextWidth(text))/2)+xOffset, yPos, top, invert); }
void printTextCenteredTintedScaled(std::u16string text, double scaleX, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert) { printTextTintedScaled(text, scaleX, scaleY, color, ((256-getTextWidth(text))/2)+xOffset, yPos, top, invert); }
void printTextTintedAbsW(std::string text, double w, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), w/getTextWidth(text), scaleY, color, xPos, yPos, top, invert); }
void printTextTintedAbsW(std::u16string text, double w,  double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, w/getTextWidth(text), scaleY, color, xPos, yPos, top, invert); }
void printTextTintedMaxW(std::string text, double w, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, color, xPos, yPos, top, invert); }
void printTextTintedMaxW(std::u16string text, double w,  double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, color, xPos, yPos, top, invert); }
void printTextTintedScaled(std::string text, double scaleX, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, color, xPos, yPos, top, invert); }

void printTextTintedScaled(std::u16string text, double scaleX, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert) {
	if(scaleX == 1 && scaleY == 1) {
		printTextTinted(text, color, xPos, yPos, top, invert);
		return;
	}

	int x=xPos;
	u16 pallet[4] = {
		0,
		(u16)(color & (invert ? 0xBDEF : 0xFBDE)),
		(u16)(color & (invert ? 0xFBDE : 0xBDEF)),
		0
	};
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == newline[0]) {
			x = xPos;
			yPos += tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		std::vector<u16> image;
		for(int i=0;i<tileSize;i++) {
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>6 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>4 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)]>>2 & 3]);
			image.push_back(pallet[fontTiles[i+(t*tileSize)] & 3]);
		}

		x += fontWidths[t*3];
		if(x > 256) {
			x = xPos+fontWidths[t*3];
			yPos += tileHeight;
		}
		drawImageScaled(x, yPos, tileWidth, tileHeight, scaleX, scaleY, image, top);
		x += fontWidths[(t*3)+1]*scaleX;
	}
}

int getTextWidthMaxW(std::string text, int w) { return std::min(w, getTextWidth(StringUtils::UTF8toUTF16(text))); }
int getTextWidthMaxW(std::u16string text, int w) { return std::min(w, getTextWidth(text)); }
int getTextWidthScaled(std::string text, double scale) { return getTextWidth(StringUtils::UTF8toUTF16(text))*scale; }
int getTextWidthScaled(std::u16string text, double scale) { return getTextWidth(text)*scale; }
int getTextWidth(std::string text) { return getTextWidth(StringUtils::UTF8toUTF16(text)); }
int getTextWidth(std::u16string text) {
	int textWidth = 0;
	for(unsigned c=0;c<text.size();c++) {
		textWidth += fontWidths[(getCharIndex(text[c])*3)+2];
	}
	return textWidth;
}
