#include "graphics.hpp"
#include "tonccpy.h"

#define WHITE 0xFFFF

std::vector<Sprite> spritesMain(128), spritesSub(128);
int maxSpriteMain = 0, maxSpriteSub = 0;
std::vector<char> fontTiles;
std::vector<char> fontWidths;
std::vector<u16> fontMap;
u16 tileSize, tileWidth, tileHeight;

#define sprites(top) (top ? spritesMain : spritesSub)
#define maxSprite(top) (top ? maxSpriteMain : maxSpriteSub)

int getCharIndex(char16_t c) {
	// Try a binary search
	int left = 0;
	int right = fontMap.size();

	while(left <= right) {
		int mid = left + ((right - left) / 2);
		if(fontMap[mid] == c) {
			return mid;
		}

		if(fontMap[mid] < c) {
			left = mid + 1;
		} else {
			right = mid - 1;
		}
	}

	// If that doesn't find the char, do a linear search
	for(unsigned int i=0;i<fontMap.size();i++) {
		if(fontMap[i] == c)	return i;
	}
	return 0;
}

void initGraphics(void) {
	// Initialize video mode
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	// initialize all the VRAM banks
	vramSetPrimaryBanks(VRAM_A_MAIN_BG,
						VRAM_B_MAIN_SPRITE,
						VRAM_C_SUB_BG,
						VRAM_D_SUB_SPRITE);

	// Init oam with 1D mapping 128 byte boundary and no external palette support
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);

	// Init for background
	int id = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(id, 3);
	id = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(id, 3);

	u16 palette[] = {0, 0xFBDE, 0xBDEF, // WHITE_TEXT
					 0, 0x8C63, 0xCA52, // GRAY_TEXT
					 0, (u16)(0x801F & 0xFBDE), (u16)(0x801F & 0xBDEF), // RED_TEXT
					 0, (u16)(0xFC00 & 0xFBDE), (u16)(0xFC00 & 0xBDEF)}; // BLUE_TEXT
	tonccpy(BG_PALETTE, &palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, &palette, sizeof(palette));

	// Set main background as target for sprite transparency
	REG_BLDCNT = 1<<11;
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

Image loadImage(std::string path) {
	Image image = {0, 0, {}, {}, 0};
	FILE *file = fopen(path.c_str(), "rb");
	if(file) {
		fseek(file, 4, SEEK_SET);
		fread(&image.width, 1, 2, file);
		fread(&image.height, 1, 2, file);
		image.bitmap = std::vector<u8>(image.width*image.height);
		fread(image.bitmap.data(), 1, image.width*image.height, file);
		u16 palCount;
		fread(&palCount, 1, 2, file);
		fread(&image.palOfs, 1, 2, file);
		image.palette = std::vector<u16>(palCount);
		fread(image.palette.data(), 2, palCount, file);
		fclose(file);
	}

	return image;
}

void copyPalette(const Image &image, bool top, int paletteOffset = 0) {
	tonccpy((top ? BG_PALETTE : BG_PALETTE_SUB)+image.palOfs+paletteOffset, image.palette.data(), image.palette.size()*2);
}

void drawImageDMA(int x, int y, int w, int h, const Image &image, bool top) {
	copyPalette(image, top);
	for(int i=0;i<h;i++) {
		dmaCopyHalfWords(0, image.bitmap.data()+(i*w), (u8*)(top ? BG_GFX : BG_GFX_SUB)+((y+i)*256)+x, w);
	}
}

void drawImage(int x, int y, int w, int h, const Image &image, bool top, int paletteOffset) {
	copyPalette(image, top, paletteOffset);
	u8 *dst = (u8*)(top ? BG_GFX : BG_GFX_SUB);
	u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			if(pal[image.bitmap[(i*w)+j]] != 0) { // Do not render transparent pixel
				dst[(y+i)*256+j+x] = image.bitmap[(i*w)+j] + paletteOffset;
			}
		}
	}
}

void drawImageSegmentDMA(int x, int y, int w, int h, const Image &image, int imageWidth, bool top) {
	copyPalette(image, top);
	for(int i=0;i<h;i++) {
		dmaCopyHalfWords(0, image.bitmap.data()+(i*imageWidth), (u8*)(top ? BG_GFX : BG_GFX_SUB)+((y+i)*256)+x, w);
	}
}

void drawImageSegment(int x, int y, int w, int h, const Image &image, int imageWidth, int xOffset, int yOffset, bool top) {
	copyPalette(image, top);
	u8* dst = (u8*)(top ? BG_GFX : BG_GFX_SUB);
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			if(image.bitmap[((i+yOffset)*imageWidth)+j+xOffset] != 0) { // Do not render transparent pixel
				dst[((y+i)*256)+j+x] = image.bitmap[((i+yOffset)*imageWidth)+j+xOffset];
			}
		}
	}
}

void drawImageSegmentScaled(int x, int y, int w, int h, double scaleX, double scaleY, const Image &image, int imageWidth, int xOffset, int yOffset, bool top) {
	if(scaleX == 1 && scaleY == 1)	drawImageSegment(x, y, w, h, image, imageWidth, xOffset, yOffset, top);
	else {
		copyPalette(image, top);
		Image buffer = {(u16)w, (u16)h, {}, image.palette};
		for(int i=0;i<h;i++) {
			for(int j=0;j<w;j++) {
				buffer.bitmap.push_back(image.bitmap[((i+yOffset)*imageWidth)+j+xOffset]);
			}
		}
		drawImageScaled(x, y, w, h, scaleX, scaleY, buffer, top);
	}
}

void drawImageScaled(int x, int y, int w, int h, double scaleX, double scaleY, const Image &image, bool top) {
	if(scaleX == 1 && scaleY == 1)	drawImage(x, y, w, h, image, top);
	else {
		copyPalette(image, top);
		u8* dst = (u8*)(top ? BG_GFX : BG_GFX_SUB);
		for(int i=0;i<(h*scaleY);i++) {
			for(int j=0;j<(w*scaleX);j++) {
				if(image.bitmap[(((int)(i/scaleY))*w)+(j/scaleX)] != 0) { // Do not render transparent pixel
					dst[(y+i)*256+x+j] = image.bitmap[(((int)(i/scaleY))*w)+(j/scaleX)];
				}
			}
		}
	}
}

void drawOutline(int x, int y, int w, int h, int color, bool top) {
	u8* dst = (u8*)(top ? BG_GFX : BG_GFX_SUB);
	h+=y;
	if(y>=0 && y<192)	dmaFillHalfWords(((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15), dst+((y*256)+(x < 0 ? 0 : x)), (x+w > 256 ? w+(256-x-w) : w));
	for(y++;y<(h-1);y++) {
		if(y>=0 && y<192 && x>0)	dst[(y)*256+x] = ((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15);
		if(y>=0 && y<192 && x+w<256)	dst[(y)*256+x+w-1] = ((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15);
	}
	if(y>=0 && y<192)	dmaFillHalfWords(((color>>10)&0x1f) | ((color)&(0x1f<<5)) | (color&0x1f)<<10 | BIT(15), dst+((y*256)+(x < 0 ? 0 : x)), (x+w > 256 ? w+(256-x-w) : w));
}

void drawRectangle(int x, int y, int w, int h, int color, bool top) { drawRectangle(x, y, w, h, color, color, top); }
void drawRectangle(int x, int y, int w, int h, int color1, int color2, bool top) {
	for(int i=0;i<h;i++) {
		dmaFillHalfWords(((i%2) ? color1 : color2) | ((i%2) ? color1 : color2) << 8, (u8*)(top ? BG_GFX : BG_GFX_SUB)+((y+i)*256+x), w);
	}
}

int initSprite(bool top, SpriteSize spriteSize, int id, int rotationIndex) {
	if(id == -1)	id = maxSprite(top)++;

	Sprite sprite = {0, spriteSize, SpriteColorFormat_Bmp, rotationIndex, 15, 0, 0};
	sprites(top)[id] = sprite;

	// Allocate memory for graphics
	sprites(top)[id].gfx = oamAllocateGfx((top ? &oamMain : &oamSub), sprites(top)[id].size, sprites(top)[id].format);

	return id;
}

void fillSpriteColor(int id, bool top, u8 color) {
	int size = 0;
	switch(sprites(top)[id].size) {
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

	toncset16(sprites(top)[id].gfx, color, size);
}

void fillSpriteImage(int id, bool top, int x, int y, int w, int h, const Image &image, int spriteW) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites(top)[id].gfx[((y+i)*spriteW)+(x+j)] = image.palette[image.bitmap[((i)*w)+j]];
		}
	}
}

void fillSpriteImageScaled(int id, bool top, int x, int y, int w, int h, double scale, const Image &image) {
	if(scale == 1)	fillSpriteImage(id, top, x, y, w, h, image);
	else {
		for(int i=0;i<(h*scale);i++) {
			for(int j=0;j<(w*scale);j++) {
				// if(image.palette[image.bitmap[(((int)(i/scale))*w)+(j/scale)]]>>15 != 0) { // Do not render transparent pixel
					sprites(top)[id].gfx[(y+i)*32+x+j] = image.palette[image.bitmap[(((int)(i/scale))*w)+(j/scale)]];
				// }
			}
		}
	}
}

void fillSpriteSegment(int id, bool top, const Image &image, int w, int h, int imageWidth, int xOffset, int yOffset) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites(top)[id].gfx[(i*w)+j] = image.palette[image.bitmap[((i+yOffset)*imageWidth)+j+xOffset]];
		}
	}
}

// update this
void fillSpriteSegmentScaled(int id, bool top, double scale, const Image &image, int w, int h, int imageWidth, int xOffset, int yOffset) {
	if(scale == 1)	fillSpriteSegment(id, top, image, w, h, imageWidth, xOffset, yOffset);
	else {
		u16 ws = w*(u16)scale;
		scale = 1/scale;
		for(double i=0;i<h;i+=scale) {
			u16 ii=i;
			u16 is=i/scale;
			for(double j=0;j<w;j+=scale) {
				u16 jj=j;
				u16 js=j/scale;
				sprites(top)[id].gfx[(is*ws)+js] = image.palette[image.bitmap[((ii+yOffset)*imageWidth)+jj+xOffset]];
			}
		}
	}
}

// void fillSpriteSegmentTinted(int id, bool top, const Image &image, int palette, int w, int h, int imageWidth, int xOffset, int yOffset) {
// 	for(int i=0;i<h;i++) {
// 		for(int j=0;j<w;j++) {
// 			sprites(top)[id].gfx[(i*w)+j] = (color & image.palette[image.bitmap[((i+yOffset)*imageWidth)+j+xOffset]]);
// 		}
// 	}
// }

void fillSpriteText(int id, bool top, const std::string &text, int palette, int xPos, int yPos, bool invert) { fillSpriteText(id, top, StringUtils::UTF8toUTF16(text), palette, xPos, yPos, invert); };

void fillSpriteText(int id, bool top, const std::u16string &text, int palette, int xPos, int yPos, bool invert) {
	for(unsigned c=0;c<text.size();c++) {
		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>6 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>4 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>2 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)] & 3);
		}

		xPos += fontWidths[t*3];
		for(int i=0;i<tileHeight;i++) {
			for(int j=0;j<tileWidth;j++) {
				if(image.palette[image.bitmap[((i)*tileWidth)+j]]>>15 != 0) { // Do not render transparent pixel
					sprites(top)[id].gfx[((yPos+i)*32)+(xPos+j)] = image.palette[image.bitmap[((i)*tileWidth)+j]];
				}
			}
		}
		xPos += fontWidths[(t*3)+1];
	}
}

void prepareSprite(int id, bool top, int x, int y, int priority) {
	oamSet(
		(top ? &oamMain : &oamSub),	// Main/Sub display
		id,	// Oam entry to set
		x, y,	// Position
		priority, // Priority
		sprites(top)[id].paletteAlpha, // Alpha for bmp sprite
		sprites(top)[id].size,
		sprites(top)[id].format,
		sprites(top)[id].gfx,
		sprites(top)[id].rotationIndex,
		false, // Don't double the sprite size for rotation
		false, // Don't hide the sprite
		false, false, // vflip, hflip
		false // Apply mosaic
	);
	sprites(top)[id].x = x;
	sprites(top)[id].y = y;
}

void updateOam(void) {
	oamUpdate(&oamSub);
	oamUpdate(&oamMain);
}

void setSpritePosition(int id, bool top, int x, int y) {
	oamSetXY((top ? &oamMain : &oamSub), id, x, y);
	sprites(top)[id].x = x;
	sprites(top)[id].y = y;
}

void setSpriteAlpha(int id, bool top, int alpha) { oamSetAlpha((top ? &oamMain : &oamSub), id, alpha); }
void setSpritePriority(int id, bool top, int priority) { oamSetPriority((top ? &oamMain : &oamSub), id, priority); }
void setSpriteVisibility(int id, bool top, int show) { oamSetHidden((top ? &oamMain : &oamSub), id, !show); }
Sprite getSpriteInfo(int id, bool top) { return sprites(top)[id]; }
unsigned getSpriteAmount(bool top) { return maxSprite(top); }

void printText(const std::string &text, int xPos, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), WHITE, xPos, yPos, top, invert); }
void printText(const std::u16string &text, int xPos, int yPos, bool top, bool invert) { printTextTinted(text, WHITE, xPos, yPos, top, invert); }
void printTextCentered(const std::string &text, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTinted(StringUtils::UTF8toUTF16(text), WHITE, xOffset, yPos, top, invert); }
void printTextCentered(const std::u16string &text, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTinted(text, WHITE, xOffset, yPos, top, invert); }
void printTextCenteredTinted(const std::string &text, int palette, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTinted(StringUtils::UTF8toUTF16(text), palette, xOffset, yPos, top, invert); }
void printTextCenteredTinted(std::u16string text, int palette, int xOffset, int yPos, bool top, bool invert) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTinted(text.substr(0, text.find('\n')), palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i++*16), top, invert);
		text = text.substr(text.find('\n')+1);
	}
	printTextTinted(text.substr(0, text.find('\n')), palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i*16), top, invert);
}
void printTextTinted(const std::string &text, int palette, int xPos, int yPos, bool top, bool invert) { printTextTinted(StringUtils::UTF8toUTF16(text), palette, xPos, yPos, top, invert); }

void printTextTinted(const std::u16string &text, int palette, int xPos, int yPos, bool top, bool invert) {
	int x=xPos;
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == 0xA) {
			yPos += tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap[(i*4)]   = (palette*3 + (fontTiles[i+(t*tileSize)]>>6 & 3));
			image.bitmap[(i*4)+1] = (palette*3 + (fontTiles[i+(t*tileSize)]>>4 & 3));
			image.bitmap[(i*4)+2] = (palette*3 + (fontTiles[i+(t*tileSize)]>>2 & 3));
			image.bitmap[(i*4)+3] = (palette*3 + (fontTiles[i+(t*tileSize)]    & 3));
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

void printTextMaxW(const std::string &text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextMaxW(const std::u16string &text, double w, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, WHITE, xPos, yPos, top, invert); }
void printTextCenteredMaxW(const std::string &text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedMaxW(StringUtils::UTF8toUTF16(text), w, scaleY, WHITE, xOffset, yPos, top, invert); }
void printTextCenteredMaxW(const std::u16string &text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedMaxW(text, w, scaleY, WHITE, xOffset, yPos, top, invert); }
void printTextCenteredTintedMaxW(const std::string &text, double w, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedMaxW(StringUtils::UTF8toUTF16(text), w, scaleY, palette, xOffset, yPos, top, invert); }
void printTextCenteredTintedMaxW(std::u16string text, double w, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTintedScaled(text.substr(0, text.find('\n')), std::min(1.0, w/getTextWidth(text.substr(0, text.find('\n')))), scaleY, palette, ((256-getTextWidthMaxW(text.substr(0, text.find('\n')), w))/2)+xOffset, yPos+(i++*(16*scaleY)), top, invert);
		text = text.substr(text.find('\n')+1);
	}
	printTextTintedScaled(text.substr(0, text.find('\n')), std::min(1.0, w/getTextWidth(text.substr(0, text.find('\n')))), scaleY, palette, ((256-getTextWidthMaxW(text.substr(0, text.find('\n')), w))/2)+xOffset, yPos+(i*(16*scaleY)), top, invert);
}
void printTextTintedMaxW(const std::string &text, double w, double scaleY, int palette, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(1.0, w/getTextWidth(text)), scaleY, palette, xPos, yPos, top, invert); }
void printTextTintedMaxW(const std::u16string &text, double w,  double scaleY, int palette, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, std::min(1.0, w/getTextWidth(text)), scaleY, palette, xPos, yPos, top, invert); }

void printTextScaled(const std::string &text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, WHITE, xPos, yPos, top, invert); }
void printTextScaled(const std::u16string &text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(text, scaleX, scaleY, WHITE, xPos, yPos, top, invert); }
void printTextCenteredScaled(const std::string &text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, WHITE, xOffset, yPos, top, invert); }
void printTextCenteredScaled(const std::u16string &text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedScaled(text, scaleX, scaleY, WHITE, xOffset, yPos, top, invert); }
void printTextCenteredTintedScaled(const std::string &text, double scaleX, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert) { printTextCenteredTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, palette, xOffset, yPos, top, invert); }
void printTextCenteredTintedScaled(std::u16string text, double scaleX, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTintedScaled(text.substr(0, text.find('\n')), scaleX, scaleY, palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i++*(16*scaleY)), top, invert);
		text = text.substr(text.find('\n')+1);
	}
	printTextTintedScaled(text.substr(0, text.find('\n')), scaleX, scaleY, palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i*(16*scaleY)), top, invert);
}
void printTextTintedScaled(const std::string &text, double scaleX, double scaleY, int palette, int xPos, int yPos, bool top, bool invert) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, palette, xPos, yPos, top, invert); }

void printTextTintedScaled(const std::u16string &text, double scaleX, double scaleY, int palette, int xPos, int yPos, bool top, bool invert) {
	if(scaleX == 1 && scaleY == 1) {
		printTextTinted(text, palette, xPos, yPos, top, invert);
		return;
	}

	int x=xPos;
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == 0xA) {
			x = xPos;
			yPos += tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap[(i*4)]   = (palette*3 + (fontTiles[i+(t*tileSize)]>>6 & 3));
			image.bitmap[(i*4)+1] = (palette*3 + (fontTiles[i+(t*tileSize)]>>4 & 3));
			image.bitmap[(i*4)+2] = (palette*3 + (fontTiles[i+(t*tileSize)]>>2 & 3));
			image.bitmap[(i*4)+3] = (palette*3 + (fontTiles[i+(t*tileSize)]    & 3));
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

int getTextWidthMaxW(const std::string &text, int w) { return std::min(w, getTextWidth(StringUtils::UTF8toUTF16(text))); }
int getTextWidthMaxW(const std::u16string &text, int w) { return std::min(w, getTextWidth(text)); }
int getTextWidthScaled(const std::string &text, double scale) { return getTextWidth(StringUtils::UTF8toUTF16(text))*scale; }
int getTextWidthScaled(const std::u16string &text, double scale) { return getTextWidth(text)*scale; }
int getTextWidth(const std::string &text) { return getTextWidth(StringUtils::UTF8toUTF16(text)); }
int getTextWidth(const std::u16string &text) {
	int textWidth = 0;
	for(unsigned c=0;c<text.size();c++) {
		textWidth += fontWidths[(getCharIndex(text[c])*3)+2];
	}
	return textWidth;
}