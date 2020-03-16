#include "graphics.hpp"
#include "config.hpp"
#include "imgcpy.h"
#include "tonccpy.h"

std::vector<Sprite> spritesMain(128), spritesSub(128);
int maxSpriteMain = 0, maxSpriteSub = 0;
std::vector<char> fontTiles;
std::vector<char> fontWidths;
std::vector<u16> fontMap;
char16_t questionMark = 0;
u16 tileSize, tileWidth, tileHeight;
int bg3Main, bg2Main, bg3Sub, bg2Sub, bg1Sub;
bool wideScreen = false;

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
	return questionMark;
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
	bg3Main = bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Main, 2);

	bg3Sub = bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	bg2Sub = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(bg2Sub, 2);

	// Set main background as target for sprite transparency
	REG_BLDCNT = 1<<11;
	REG_BLDCNT_SUB = 1<<11;
}

void loadFont(Language lang) {
	bool chinese = lang == Language::CHS || lang == Language::CHT;
	FILE *file = fopen((Config::getString("themeDir") + "/graphics/" + (chinese ? "fontChinese.nftr" : "font.nftr")).c_str(), "rb");
	if(!file) {
		file = fopen((std::string("nitro:/graphics/") + (chinese ? "fontChinese.nftr" : "font.nftr")).c_str(), "rb");
	}

	if(file) {
		// Get file size
		fseek(file, 0, SEEK_END);
		u32 fileSize = ftell(file);

		// Skip font info
		fseek(file, 0x14, SEEK_SET);
		fseek(file, fgetc(file)-1, SEEK_CUR);

		// Load glyph info
		u32 chunkSize;
		fread(&chunkSize, 4, 1, file);
		tileWidth = fgetc(file);
		tileHeight = fgetc(file);
		fread(&tileSize, 2, 1, file);

		// Load character glyphs
		int tileAmount = ((chunkSize-0x10)/tileSize);
		fontTiles = std::vector<char>(tileSize*tileAmount);
		fseek(file, 4, SEEK_CUR);
		fread(fontTiles.data(), tileSize, tileAmount, file);

		// Fix top row
		for(int i=0;i<tileAmount;i++) {
			fontTiles[i*tileSize] = 0;
			fontTiles[i*tileSize+1] = 0;
			fontTiles[i*tileSize+2] = 0;
		}

		// Load character widths
		fseek(file, 0x24, SEEK_SET);
		u32 locHDWC;
		fread(&locHDWC, 4, 1, file);
		fseek(file, locHDWC-4, SEEK_SET);
		fread(&chunkSize, 4, 1, file);
		fseek(file, 8, SEEK_CUR);
		fontWidths = std::vector<char>(3*tileAmount);
		fread(fontWidths.data(), 3, tileAmount, file);

		// Load character maps
		fontMap = std::vector<u16>(tileAmount);
		fseek(file, 0x28, SEEK_SET);
		u32 locPAMC, mapType;
		fread(&locPAMC, 4, 1, file);

		while(locPAMC < fileSize) {
			u16 firstChar, lastChar;
			fseek(file, locPAMC, SEEK_SET);
			fread(&firstChar, 2, 1, file);
			fread(&lastChar, 2, 1, file);
			fread(&mapType, 4, 1, file);
			fread(&locPAMC, 4, 1, file);

			switch(mapType) {
				case 0: {
					u16 firstTile;
					fread(&firstTile, 2, 1, file);
					for(unsigned i=firstChar;i<=lastChar;i++) {
						fontMap[firstTile+(i-firstChar)] = i;
					}
					break;
				} case 1: {
					for(int i=firstChar;i<=lastChar;i++) {
						u16 tile;
						fread(&tile, 2, 1, file);
						fontMap[tile] = i;
					}
					break;
				} case 2: {
					u16 groupAmount;
					fread(&groupAmount, 2, 1, file);
					for(int i=0;i<groupAmount;i++) {
						u16 charNo, tileNo;
						fread(&charNo, 2, 1, file);
						fread(&tileNo, 2, 1, file);
						fontMap[tileNo] = charNo;
					}
					break;
				}
			}
		}
		fclose(file);
	}

	questionMark = getCharIndex('?');
}

Image loadImage(const std::string &path) {
	Image image = {0, 0, {}, {}, 0};
	FILE *file = fopen((Config::getString("themeDir")+path).c_str(), "rb");
	if(!file) {
		file = fopen(("nitro:"+path).c_str(), "rb");
	}

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

u8 *gfxPointer(bool top, bool layer) {
	if(top) {
		if(layer) {
			return (u8*)bgGetGfxPtr(bg2Main);
		} else {
			return (u8*)bgGetGfxPtr(bg3Main);
		}
	} else {
		if(layer) {
			return (u8*)bgGetGfxPtr(bg2Sub);
		} else {
			return (u8*)bgGetGfxPtr(bg3Sub);
		}
	}
}

void drawImage(int x, int y, const Image &image, bool top, bool layer, int paletteOffset) {
	copyPalette(image, top, paletteOffset);
	u8 *dst = gfxPointer(top, layer);
	u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
	for(int i=0;i<image.height;i++) {
		imgcpy(dst+((y+i)*256+x), image.bitmap.data()+((i*image.width)), pal, image.width, paletteOffset);
	}
}

void drawImageDMA(int x, int y, const Image &image, bool top, bool layer) {
	copyPalette(image, top);
	for(int i=0;i<image.height;i++) {
		dmaCopyHalfWords(0, image.bitmap.data()+(i*image.width), gfxPointer(top, layer)+((y+i)*256)+x, image.width);
	}
}

void drawImageScaled(int x, int y, float scaleX, float scaleY, const Image &image, bool top, bool layer, int paletteOffset) {
	if(scaleX == 1 && scaleY == 1)	drawImage(x, y, image, top, layer);
	else {
		copyPalette(image, top, paletteOffset);
		u8* dst = gfxPointer(top, layer);
		u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
		u8 buffer[(int)(image.width*scaleX)];
		for(int i=0;i<(image.height*scaleY);i++) {
			for(int j=0;j<(image.width*scaleX);j++) {
				buffer[j] = image.bitmap[(((int)(i/scaleY))*image.width)+(j/scaleX)];
			}
			imgcpy(dst+(y+i)*256+x, buffer, pal, (int)(image.width*scaleX), paletteOffset);
		}
	}
}

void drawImageSegment(int x, int y, int w, int h, const Image &image, int xOffset, int yOffset, bool top, bool layer) {
	copyPalette(image, top);
	u8* dst = gfxPointer(top, layer);
	u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
	for(int i=0;i<h;i++) {
		imgcpy(dst+((y+i)*256+x), image.bitmap.data()+(((yOffset+i)*image.width)+xOffset), pal, w, 0);
	}
}

void drawImageSegmentDMA(int x, int y, int w, int h, const Image &image, int xOffset, int yOffset, bool top, bool layer) {
	copyPalette(image, top);
	for(int i=0;i<h;i++) {
		dmaCopyHalfWords(0, image.bitmap.data()+((yOffset+i)*image.width)+xOffset, gfxPointer(top, layer)+((y+i)*256)+x, w);
	}
}

void drawImageSegmentScaled(int x, int y, int w, int h, float scaleX, float scaleY, const Image &image, int xOffset, int yOffset, bool top, bool layer) {
	if(scaleX == 1 && scaleY == 1)	return drawImageSegment(x, y, w, h, image, xOffset, yOffset, top, layer);

	// u8* dst = gfxPointer(top, layer);
	// u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
	// for(int i=0;i<h;i++) {
	// 	imgcpy(dst+((y+i)*256+x), image.bitmap.data()+(((yOffset+i)*image.width)+xOffset), pal, w, 0);
	// }

	copyPalette(image, top);
	u8* dst = gfxPointer(top, layer);
	u16 *pal = (top ? BG_PALETTE : BG_PALETTE_SUB);
	u8 buffer[(int)(image.width*scaleX)];
	for(int i=0;i<(h*scaleY);i++) {
		for(int j=0;j<(w*scaleX);j++) {
			buffer[j] = image.bitmap[(((int)((yOffset+i)/scaleY))*image.width)+((xOffset+j)/scaleX)];
		}
		imgcpy(dst+(y+i)*256+x, buffer, pal, (int)(image.width*scaleX), 0);
	}
}

void drawOutline(int x, int y, int w, int h, u8 color, bool top, bool layer) {
	u8* dst = gfxPointer(top, layer);
	h+=y;
	if(y>=0 && y<192)	toncset(dst+((y*256)+(x < 0 ? 0 : x)), color, (x+w > 256 ? w+(256-x-w) : w));
	for(y++;y<(h-1);y++) {
		if(y>=0 && y<192 && x>0)	toncset(dst+((y)*256+x), color, 1);
		if(y>=0 && y<192 && x+w<256)	toncset(dst+((y)*256+x+w-1), color, 1);
	}
	if(y>=0 && y<192)	toncset(dst+((y*256)+(x < 0 ? 0 : x)), color, (x+w > 256 ? w+(256-x-w) : w));
}

void drawRectangle(int x, int y, int w, int h, u8 color, bool top, bool layer) { drawRectangle(x, y, w, h, color, color, top, layer); }
void drawRectangle(int x, int y, int w, int h, u8 color1, u8 color2, bool top, bool layer) {
	u8 *dst = gfxPointer(top, layer);
	for(int i=0;i<h;i++) {
		toncset(dst+((y+i)*256+x), ((i%2) ? color1 : color2), w);
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

void fillSpriteColor(int id, bool top, u16 color) {
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

void fillSpriteImage(int id, bool top, int spriteW, int x, int y, const Image &image, bool skipAlpha) {
	for(int i=0;i<image.height;i++) {
		for(int j=0;j<image.width;j++) {
			if(!(skipAlpha && image.palette[image.bitmap[(i*image.width)+j]] == 0)) {
				sprites(top)[id].gfx[((y+i)*spriteW)+(x+j)] = image.palette[image.bitmap[(i*image.width)+j]-image.palOfs];
			}
		}
	}
}

void fillSpriteImageScaled(int id, bool top, int spriteW, int x, int y, float scaleX, float scaleY, const Image &image) {
	if(scaleX == 1 && scaleY == 1)	fillSpriteImage(id, top, spriteW, x, y, image);
	else {
		for(int i=0;i<(image.height*scaleY);i++) {
			for(int j=0;j<(image.width*scaleX);j++) {
				sprites(top)[id].gfx[(y+i)*spriteW+x+j] = image.palette[image.bitmap[(((int)(i/scaleY))*image.width)+(j/scaleX)]-image.palOfs];
			}
		}
	}
}

void fillSpriteSegment(int id, bool top, int spriteW, int x, int y, int w, int h, const Image &image, int xOffset, int yOffset) {
	for(int i=0;i<h;i++) {
		for(int j=0;j<w;j++) {
			sprites(top)[id].gfx[((y+i)*spriteW)+x+j] = image.palette[image.bitmap[((i+yOffset)*image.width)+j+xOffset]];
		}
	}
}

void fillSpriteText(int id, bool top, const std::string &text, TextColor palette, int xPos, int yPos) { fillSpriteText(id, top, StringUtils::UTF8toUTF16(text), palette, xPos, yPos); };

void fillSpriteText(int id, bool top, const std::u16string &text, TextColor palette, int xPos, int yPos) {
	for(unsigned c=0;c<text.size();c++) {
		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {0, 0, 0, 0}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>6 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>4 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)]>>2 & 3);
			image.bitmap.push_back(fontTiles[i+(t*tileSize)] & 3);
		}

		tonccpy(image.palette.data(), BG_PALETTE+(int(palette)*4), 8);

		xPos += fontWidths[t*3];
		fillSpriteImage(id, top, 32, xPos, yPos, image, true);
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
		false, // Don't float the sprite size for rotation
		false, // Don't hide the sprite
		false, false, // vflip, hflip
		false // Apply mosaic
	);
	sprites(top)[id].x = x;
	sprites(top)[id].y = y;
}

void setSpriteScale(int rotationIndex, bool top, float sx, float sy) {
	oamRotateScale((top ? &oamMain : &oamSub), rotationIndex, 0, (1 / sx) * (1 << 8), (1 / sy) * (1 << 8));
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

void printText(const std::string &text, int xPos, int yPos, bool top, bool layer) { printTextTinted(StringUtils::UTF8toUTF16(text), TextColor::white, xPos, yPos, top, layer); }
void printText(const std::u16string &text, int xPos, int yPos, bool top, bool layer) { printTextTinted(text, TextColor::white, xPos, yPos, top, layer); }
void printTextCentered(const std::string &text, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTinted(StringUtils::UTF8toUTF16(text), TextColor::white, xOffset, yPos, top, layer); }
void printTextCentered(const std::u16string &text, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTinted(text, TextColor::white, xOffset, yPos, top, layer); }
void printTextCenteredTinted(const std::string &text, TextColor palette, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTinted(StringUtils::UTF8toUTF16(text), palette, xOffset, yPos, top, layer); }
void printTextCenteredTinted(std::u16string text, TextColor palette, int xOffset, int yPos, bool top, bool layer) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTinted(text.substr(0, text.find('\n')), palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i++*16), top, layer);
		text = text.substr(text.find('\n')+1);
	}
	printTextTinted(text.substr(0, text.find('\n')), palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i*16), top, layer);
}
void printTextTinted(const std::string &text, TextColor palette, int xPos, int yPos, bool top, bool layer) { printTextTinted(StringUtils::UTF8toUTF16(text), palette, xPos, yPos, top, layer); }

void printTextTinted(const std::u16string &text, TextColor palette, int xPos, int yPos, bool top, bool layer) {
	int x=xPos;
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == '\n') {
			x = xPos;
			yPos += tileHeight;
			continue;
		}

		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap[(i*4)]   = (fontTiles[i+(t*tileSize)]>>6 & 3);
			image.bitmap[(i*4)+1] = (fontTiles[i+(t*tileSize)]>>4 & 3);
			image.bitmap[(i*4)+2] = (fontTiles[i+(t*tileSize)]>>2 & 3);
			image.bitmap[(i*4)+3] = (fontTiles[i+(t*tileSize)]    & 3);
		}

		if(x > 256) {
			x = xPos;
			yPos += tileHeight;
		}
		drawImage(x + fontWidths[t*3], yPos, image, top, layer, int(palette)*4);
		x += fontWidths[(t*3)+2];
	}
}

void printTextMaxW(const std::string &text, float w, float scaleY, int xPos, int yPos, bool top, bool layer, float maxScale) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(std::min(1.0f, maxScale), w/getTextWidth(text)), scaleY, TextColor::white, xPos, yPos, top, layer); }
void printTextMaxW(const std::u16string &text, float w, float scaleY, int xPos, int yPos, bool top, bool layer, float maxScale) { printTextTintedScaled(text, std::min(std::min(1.0f, maxScale), w/getTextWidth(text)), scaleY, TextColor::white, xPos, yPos, top, layer); }
void printTextCenteredMaxW(const std::string &text, float w, float scaleY, int xOffset, int yPos, bool top, bool layer, float maxScale) { printTextCenteredTintedMaxW(StringUtils::UTF8toUTF16(text), w, scaleY, TextColor::white, xOffset, yPos, top, layer, maxScale); }
void printTextCenteredMaxW(const std::u16string &text, float w, float scaleY, int xOffset, int yPos, bool top, bool layer, float maxScale) { printTextCenteredTintedMaxW(text, w, scaleY, TextColor::white, xOffset, yPos, top, layer, maxScale); }
void printTextCenteredTintedMaxW(const std::string &text, float w, float scaleY, TextColor palette, int xOffset, int yPos, bool top, bool layer, float maxScale) { printTextCenteredTintedMaxW(StringUtils::UTF8toUTF16(text), w, scaleY, palette, xOffset, yPos, top, layer, maxScale); }
void printTextCenteredTintedMaxW(std::u16string text, float w, float scaleY, TextColor palette, int xOffset, int yPos, bool top, bool layer, float maxScale) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTintedScaled(text.substr(0, text.find('\n')), std::min(std::min(1.0f, maxScale), w/getTextWidth(text.substr(0, text.find('\n')))), scaleY, palette, ((256-getTextWidthMaxW(text.substr(0, text.find('\n')), w))/2)+xOffset, yPos+(i++*(16*scaleY)), top, layer);
		text = text.substr(text.find('\n')+1);
	}
	printTextTintedScaled(text.substr(0, text.find('\n')), std::min(std::min(1.0f, maxScale), w/getTextWidth(text.substr(0, text.find('\n')))), scaleY, palette, ((256-getTextWidthMaxW(text.substr(0, text.find('\n')), w))/2)+xOffset, yPos+(i*(16*scaleY)), top, layer);
}
void printTextTintedMaxW(const std::string &text, float w, float scaleY, TextColor palette, int xPos, int yPos, bool top, bool layer, float maxScale) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), std::min(std::min(1.0f, maxScale), w/getTextWidth(text)), scaleY, palette, xPos, yPos, top, layer); }
void printTextTintedMaxW(const std::u16string &text, float w,  float scaleY, TextColor palette, int xPos, int yPos, bool top, bool layer, float maxScale) { printTextTintedScaled(text, std::min(std::min(1.0f, maxScale), w/getTextWidth(text)), scaleY, palette, xPos, yPos, top, layer); }

void printTextScaled(const std::string &text, float scaleX, float scaleY, int xPos, int yPos, bool top, bool layer) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, TextColor::white, xPos, yPos, top, layer); }
void printTextScaled(const std::u16string &text, float scaleX, float scaleY, int xPos, int yPos, bool top, bool layer) { printTextTintedScaled(text, scaleX, scaleY, TextColor::white, xPos, yPos, top, layer); }
void printTextCenteredScaled(const std::string &text, float scaleX, float scaleY, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, TextColor::white, xOffset, yPos, top, layer); }
void printTextCenteredScaled(const std::u16string &text, float scaleX, float scaleY, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTintedScaled(text, scaleX, scaleY, TextColor::white, xOffset, yPos, top, layer); }
void printTextCenteredTintedScaled(const std::string &text, float scaleX, float scaleY, TextColor palette, int xOffset, int yPos, bool top, bool layer) { printTextCenteredTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, palette, xOffset, yPos, top, layer); }
void printTextCenteredTintedScaled(std::u16string text, float scaleX, float scaleY, TextColor palette, int xOffset, int yPos, bool top, bool layer) {
	int i = 0;
	while(text.find('\n') != text.npos) {
		printTextTintedScaled(text.substr(0, text.find('\n')), scaleX, scaleY, palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i++*(16*scaleY)), top, layer);
		text = text.substr(text.find('\n')+1);
	}
	printTextTintedScaled(text.substr(0, text.find('\n')), scaleX, scaleY, palette, ((256-getTextWidth(text.substr(0, text.find('\n'))))/2)+xOffset, yPos+(i*(16*scaleY)), top, layer);
}
void printTextTintedScaled(const std::string &text, float scaleX, float scaleY, TextColor palette, int xPos, int yPos, bool top, bool layer) { printTextTintedScaled(StringUtils::UTF8toUTF16(text), scaleX, scaleY, palette, xPos, yPos, top, layer); }

void printTextTintedScaled(const std::u16string &text, float scaleX, float scaleY, TextColor palette, int xPos, int yPos, bool top, bool layer) {
	if(scaleX == 1 && scaleY == 1) {
		printTextTinted(text, palette, xPos, yPos, top, layer);
		return;
	}

	int x=xPos;
	for(unsigned c=0;c<text.size();c++) {
		if(text[c] == '\n') {
			x = xPos;
			yPos += tileHeight*scaleY;
			continue;
		}

		int t = getCharIndex(text[c]);
		Image image = {tileWidth, tileHeight, {}, {}, 0};
		for(int i=0;i<tileSize;i++) {
			image.bitmap[(i*4)]   = (fontTiles[i+(t*tileSize)]>>6 & 3);
			image.bitmap[(i*4)+1] = (fontTiles[i+(t*tileSize)]>>4 & 3);
			image.bitmap[(i*4)+2] = (fontTiles[i+(t*tileSize)]>>2 & 3);
			image.bitmap[(i*4)+3] = (fontTiles[i+(t*tileSize)]    & 3);
		}

		if(x > 256) {
			x = xPos;
			yPos += tileHeight;
		}
		drawImageScaled(x + fontWidths[t*3], yPos, scaleX, scaleY, image, top, layer, int(palette)*4);
		x += fontWidths[(t*3)+2]*scaleX;
	}
}

int getTextWidthMaxW(const std::string &text, int w) { return std::min(w, getTextWidth(StringUtils::UTF8toUTF16(text))); }
int getTextWidthMaxW(const std::u16string &text, int w) { return std::min(w, getTextWidth(text)); }
int getTextWidthScaled(const std::string &text, float scale) { return getTextWidth(StringUtils::UTF8toUTF16(text))*scale; }
int getTextWidthScaled(const std::u16string &text, float scale) { return getTextWidth(text)*scale; }
int getTextWidth(const std::string &text) { return getTextWidth(StringUtils::UTF8toUTF16(text)); }
int getTextWidth(const std::u16string &text) {
	int textWidth = 0;
	for(unsigned c=0;c<text.size();c++) {
		textWidth += fontWidths[(getCharIndex(text[c])*3)+2];
	}
	return textWidth;
}
