#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#include <nds.h>
#include <string>
#include <vector>

#include "utils.hpp"

// Converts 3 0-255 (0x0-0xff) values to a BGR15 value
#define BGR15(b, g, r)   ((b)|((g)<<5)|((r)<<10))

struct ImageData {
	unsigned width;
	unsigned height;
};

struct Sprite {
	u16* gfx;
	SpriteSize size;
	SpriteColorFormat format;
	int rotationIndex;
	int paletteAlpha;
	int x;
	int y;
};

extern std::vector<u16> font;

// Initializes the screens for drawing
void initGraphics(void);

// Loads the font into RAM
void loadFont(void);

/*
 * Loads a .bmp image into a vector of raw pixel data
 * std::string path is the path of the .bmp file
 * std::vector<u16> &imageBuffer is the vector to load the raw pixel data into
 * Returns an ImageData with the Width and Height of the image
 */
ImageData loadBmp(std::string path, std::vector<u16> &imageBuffer);

/*
 * Loads a 16 color .bmp image into a vector of raw 16 bit pixel data
 * std::string path is the path of the .bmp file
 * std::vector<u16> &imageBuffer is the vector to load the raw pixel data into
 * Returns an ImageData with the Width and Height of the image
 */
ImageData loadBmp16(std::string path, std::vector<u16> &imageBuffer);

/*
 * Loads a .png image into a vector of raw pixel data
 * std::string path is the path of the .png file
 * std::vector<u16> &imageBuffer is the vector to load the raw pixel data into
 * Returns an ImageData with the Width and Height of the image
 */
ImageData loadPng(std::string path, std::vector<u16> &imageBuffer);

/*
 * Draws an image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * std::vector<u16> &imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImage(int x, int y, int w, int h, std::vector<u16> &imageBuffer, bool top);

/*
 * Draws an image to the screen from a portion of a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * std::vector<u16> &imageBuffer is the raw pixel data
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageFromSheet(int x, int y, int w, int h, std::vector<u16> &imageBuffer, int imageWidth, int xOffset, int yOffset, bool top);

/*
 * Draws a scaled image to the screen from a portion of a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * double scale is the Scale to draw the X at
 * double scale is the Scale to draw the Y at
 * std::vector<u16> &imageBuffer is the raw pixel data
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageFromSheetScaled(int x, int y, int w, int h, double scaleX, double scaleY, std::vector<u16> &imageBuffer, int imageWidth, int xOffset, int yOffset, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * double scaleX is the Scale to draw the X at
 * double scaleY is the Scale to draw the Y at
 * std::vector<u16> &imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageScaled(int x, int y, int w, int h, double scaleX, double scaleY, std::vector<u16> &imageBuffer, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * u16 color is the color to tint the image
 * std::vector<u16> &imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageTinted(int x, int y, int w, int h, u16 color, std::vector<u16> &imageBuffer, bool top);

/*
 * Draws a rectangle outline of a given size at a given position
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * int color is the color in RGB15
 * bool top is whether to draw on the top or bottom screen
 */
void drawOutline(int x, int y, int w, int h, int color, bool top);

/*
 * Draws a rectangle of a given size at a given position
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * int color is the color in RGB15
 * bool top is whether to draw on the top or bottom screen
 */
void drawRectangle(int x, int y, int w, int h, int color, bool top);

/*
 * Draws a rectangle of a given size at a given position
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * int color1 is the first color in RGB15
 * int color2 is the second color in RGB15
 * bool top is whether to draw on the top or bottom screen
 */
void drawRectangle(int x, int y, int w, int h, int color1, int color2, bool top);

/*
 * Creates a vector item and allocated VRAM for a sprite
 * SpriteSize spriteSize is the dimensions of the sprite
 * https://libnds.devkitpro.org/sprite_8h.html#a1b3e231e628b18808e49a2f94c96b1ea
 * bool top is whether the sprite should be on the top or bottom screen
 * returns the id of the sprite
 */
int initSprite(bool top, SpriteSize spriteSize, int id = -1, int rotationIndex = -1);

/*
 * Fills a sprite with a solid color
 * int id is the id of the sprite
 * u16 color is the BGR15 color to fill with
 */
void fillSpriteColor(int id, bool top, u16 color);

/*
 * Fills a sprite with raw pixel data from a vector
 * int id is the id of the sprite
 * size is the width * height of the image
 * std::vector<u16> &imageBuffer is the raw pixel data
 */
void fillSpriteImage(int id, bool top, std::vector<u16> &imageBuffer, int size = 32*32);

/*
 * Fills a sprite with raw pixel data from a vector
 * int id is the id of the sprite
 * int x is the x position to draw at
 * int y is the y positoin to draw at
 * int w is the width of the image
 * int h is the height of the image
 * std::vector<u16> &imageBuffer is the raw pixel data
 */
void fillSpriteImage(int id, int x, int y, int w, int h, std::vector<u16> &imageBuffer);

/*
 * Fills a sprite with scaled raw pixel data from a vector
 * int id is the id of the sprite
 * bool top is whether it's on the top or bottom screen
 * int x is the x position to draw at
 * int y is the y positoin to draw at
 * int w is the width of the image
 * int h is the height of the image
 * size is the width * height of the image
 * std::vector<u16> &imageBuffer is the raw pixel data
 */
void fillSpriteImageScaled(int id, bool top, int x, int y, int w, int h, double scale, std::vector<u16> &imageBuffer);

/*
 * Fills a sprite with raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * std::vector<u16> &imageBuffer is the raw pixel data
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteFromSheet(int id, bool top, std::vector<u16> &imageBuffer, int w, int h, int imageWidth, int xOffset, int yOffset);

/*
 * Fills a sprite with scaled raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * double scale is the Scale to draw the image at
 * std::vector<u16> &imageBuffer is the raw pixel data
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteFromSheetScaled(int id, bool top, double scale, std::vector<u16> &imageBuffer, int w, int h, int imageWidth, int xOffset, int yOffset);

/*
 * Fills a sprite with tinted raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * std::vector<u16> &imageBuffer is the raw pixel data
 * u16 color is the color to tint the pixels
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteFromSheetTinted(int id, bool top, std::vector<u16> &imageBuffer, u16 color, int w, int h, int imageWidth, int xOffset, int yOffset);

/**
 * Fills a sprite with text
 * int id is the sprite to print to
 * std::string test is the text that will be printed
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool invert is whether to swap the colors
 */
void fillSpriteText(int id, bool top, std::string text, u16 color, int xPos, int yPos, bool invert = false);

/**
 * Fills a sprite with text
 * int id is the sprite to print to
 * std::u16string test is the text that will be printed
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool invert is whether to swap the colors
 */
void fillSpriteText(int id, bool top, std::u16string text, u16 color, int xPos, int yPos, bool invert = false);

/*
 * Sets the sprite to be drawn
 * int id is the id of the sprite
 * int x is the X position of the sprite
 * int y is the Y position of the sprite
 * int priority is when the sprite will be drawn (0-3, lower is on top)
 */
void prepareSprite(int id, bool top, int x, int y, int priority);

// Updates the OAM to show updates sprite properties
void updateOam(void);

/*
 * Moves a sprite to a new position
 * int id is the id of the sprite
 * int x is the X position of the sprite
 * int y is the Y position of the sprite
 */
void setSpritePosition(int id, bool top, int x, int y);

/*
 * Changes a sprite's priority
 * int id is the id of the sprite
 * int priority is when the sprite will be drawn (0-3, lower is on top)
 */
void setSpritePriority(int id, bool top, int priority);

/*
 * Changes the visibilty of a sprite
 * int id is the id of the sprite
 * int show is whether the sprite will be visible
 */
void setSpriteVisibility(int id, bool top, int show);

/*
 * Gets info on a sprite
 * int id is the id of the sprite
 * returns the sprite's vector item
 */
Sprite getSpriteInfo(int id, bool top);

/*
 * Gets the total amount of sprites
 * returns the total amount of sprites
 */
unsigned getSpriteAmount(bool top);

// Get the index in the UV coordinate array where the letter appears
unsigned int getFontSpriteIndex(const u16 letter);

/**
 * Prints text to the a screen
 * std::string test is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printText(std::string text, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen
 * std::u16string test is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printText(std::u16string text, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text centered on a screen
 * std::u16string test is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCentered(std::string text, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints text centered on a screen
 * std::u16string test is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCentered(std::u16string text, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text centered on a screen
 * u16 color is the color to tint the text
 * std::string test is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTinted(std::string text, u16 color, int xOffset, int yPos, bool top, bool invert = false);
/**
 * Prints colored text centered on a screen
 * u16 color is the color to tint the text
 * std::string test is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int y is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTinted(std::u16string text, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen
 * u16 color is the color to tint the text
 * std::string test is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTinted(std::string text, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen
 * u16 color is the color to tint the text
 * std::u16string test is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTinted(std::u16string text, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a set width
 * std::string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextAbsW(std::string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a set width
 * std::u16string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextAbsW(std::u16string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a max width
 * std::string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextMaxW(std::string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a max width
 * std::u16string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextMaxW(std::u16string text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen at a scaled size
 * std::string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextScaled(std::string text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen at a scaled size
 * std::u16string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextScaled(std::u16string text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a set width
 * std::string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredAbsW(std::string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a set width
 * std::u16string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredAbsW(std::u16string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a set width
 * std::string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredMaxW(std::string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a max width
 * std::u16string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredMaxW(std::u16string text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen at a scaled size
 * std::string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredScaled(std::string text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen at a scaled size
 * std::u16string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredScaled(std::u16string text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a set width
 * std::string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedAbsW(std::string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a set width
 * std::u16string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedAbsW(std::u16string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a max width
 * std::string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedMaxW(std::string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a max width
 * std::u16string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedMaxW(std::u16string text, double w, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen at a scaled size
 * std::string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedScaled(std::string text, double scaleX, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen at a scaled size
 * std::u16string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedScaled(std::u16string text, double scaleX, double scaleY, u16 color, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a set width
 * std::string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedAbsW(std::string text, double w, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a set width
 * std::u16string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedAbsW(std::u16string text, double w,  double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a max width
 * std::string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedMaxW(std::string text, double w, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a max width
 * std::u16string test is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedMaxW(std::u16string text, double w,  double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen at a scaled size
 * std::string test is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedScaled(std::string text, double scaleX, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen at a scaled size
 * std::u16string test is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * u16 color is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedScaled(std::u16string text, double scaleX, double scaleY, u16 color, int xPos, int yPos, bool top, bool invert = false);

/**
 * Gets the width of a std::string of text with a max
 * std::string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthMaxW(std::string text, int w);

/**
 * Gets the width of a std::u16string of text with a max
 * std::u16string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthMaxW(std::u16string text, int w);

/**
 * Gets the width of a std::string of scaled text
 * std::string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthScaled(std::string text, double scale);

/**
 * Gets the width of a std::u16string of scaled text
 * std::u16string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthScaled(std::u16string text, double scale);

/**
 * Gets the width of a std::string of text
 * std::string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidth(std::string text);

/**
 * Gets the width of a std::u16string of text
 * std::u16string text is the text to get the width of
 * returns the width of the text
 */
int getTextWidth(std::u16string text);

#endif //GRAPHICS_HPP