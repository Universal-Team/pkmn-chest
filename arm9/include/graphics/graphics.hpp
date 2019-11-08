#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP
#include <nds.h>
#include <string>
#include <vector>

#include "utils.hpp"

// Converts 3 0-255 (0x0-0xff) values to a BGR15 value
#define BGR15(b, g, r)   ((b)|((g)<<5)|((r)<<10))

struct Image {
	u16 width;
	u16 height;
	std::vector<u8> bitmap;
	std::vector<u16> palette;
	u16 palOfs;
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
 * Loads a .gfx image into an Image struct
 * std::string path is the path to the .gfx file
 */
Image loadImage(std::string path);

/*
 * Draws an image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * const Image &image is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 * int paletteOffset is how much to offset the palette by (default 0)
 */
void drawImage(int x, int y, int w, int h, const Image &image, bool top, int paletteOffset = 0);

/*
 * Faster image draw that doesn't skip transparency
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * const Image &image is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageDMA(int x, int y, int w, int h, const Image &image, bool top);

/*
 * Faster image draw that doesn't skip transparency
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * const Image &image is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageSegmentDMA(int x, int y, int w, int h, const Image &image, int imageWidth, bool top);

/*
 * Draws an image to the screen from a portion of a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * const Image &image is the raw pixel data
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageSegment(int x, int y, int w, int h, const Image &image, int imageWidth, int xOffset, int yOffset, bool top);

/*
 * Draws a scaled image to the screen from a portion of a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * double scale is the Scale to draw the X at
 * double scale is the Scale to draw the Y at
 * const Image &image is the raw pixel data
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageSegmentScaled(int x, int y, int w, int h, double scaleX, double scaleY, const Image &image, int imageWidth, int xOffset, int yOffset, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * double scaleX is the Scale to draw the X at
 * double scaleY is the Scale to draw the Y at
 * const Image &image is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageScaled(int x, int y, int w, int h, double scaleX, double scaleY, const Image &image, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * int palette is the color to tint the image
 * const Image &image is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageTinted(int x, int y, int w, int h, int palette, const Image &image, bool top);

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
 * u8 color is the BGR15 color to fill with
 */
void fillSpriteColor(int id, bool top, u8 color);

/*
 * Fills a sprite with raw pixel data from a vector
 * int id is the id of the sprite
 * int x is the x position to draw at
 * int y is the y positoin to draw at
 * int w is the width of the image
 * int h is the height of the image
 * const Image &image is the raw pixel data
 */
void fillSpriteImage(int id, bool top, int x, int y, int w, int h, const Image &image, int spriteW = 32);

/*
 * Fills a sprite with scaled raw pixel data from a vector
 * int id is the id of the sprite
 * bool top is whether it's on the top or bottom screen
 * int x is the x position to draw at
 * int y is the y positoin to draw at
 * int w is the width of the image
 * int h is the height of the image
 * size is the width * height of the image
 * const Image &image is the raw pixel data
 */
void fillSpriteImageScaled(int id, bool top, int x, int y, int w, int h, double scale, const Image &image);

/*
 * Fills a sprite with raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * const Image &image is the raw pixel data
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteSegment(int id, bool top, const Image &image, int w, int h, int imageWidth, int xOffset, int yOffset);

/*
 * Fills a sprite with scaled raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * double scale is the Scale to draw the image at
 * const Image &image is the raw pixel data
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteSegmentScaled(int id, bool top, double scale, const Image &image, int w, int h, int imageWidth, int xOffset, int yOffset);

/*
 * Fills a sprite with tinted raw pixel data from a vector of a spritesheet
 * int id is the id of the sprite
 * const Image &image is the raw pixel data
 * int palette is the color to tint the pixels
 * int w is the width of the portion to put in the sprite
 * int h is the height of the portion to put in the sprite
 * int imageWidth is the width of the spritesheet
 * int xOffset is the X position in the sheet to start at
 * int yOffset is the Y position in the sheet to start at
 */
void fillSpriteSegmentTinted(int id, bool top, const Image &image, int palette, int w, int h, int imageWidth, int xOffset, int yOffset);

/**
 * Fills a sprite with text
 * int id is the sprite to print to
 * std::string text is the text that will be printed
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool invert is whether to swap the colors
 */
void fillSpriteText(int id, bool top, const std::string &text, int palette, int xPos, int yPos, bool invert = false);

/**
 * Fills a sprite with text
 * int id is the sprite to print to
 * std::u16string text is the text that will be printed
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool invert is whether to swap the colors
 */
void fillSpriteText(int id, bool top, const std::u16string &text, int palette, int xPos, int yPos, bool invert = false);

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
 * Changes a sprite's alpha
 * int id is the id of the sprite
 * int alpha is the alpha to draw the sprite at, 0 is clear, 15 is full opacity
 */
void setSpriteAlpha(int id, bool top, int alpha);

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
 * std::string text is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printText(const std::string &text, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen
 * std::u16string text is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printText(const std::u16string &text, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text centered on a screen
 * std::u16string text is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCentered(const std::string &text, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints text centered on a screen
 * std::u16string text is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCentered(const std::u16string &text, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text centered on a screen
 * int palette is the color to tint the text
 * std::string text is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTinted(const std::string &text, int palette, int xOffset, int yPos, bool top, bool invert = false);
/**
 * Prints colored text centered on a screen
 * int palette is the color to tint the text
 * std::string text is the text that will be printed
 * int xOffset is the X offset from the center to center on
 * int y is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTinted(std::u16string text, int palette, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen
 * int palette is the color to tint the text
 * std::string text is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTinted(const std::string &text, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen
 * int palette is the color to tint the text
 * std::u16string text is the text that will be printed
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTinted(const std::u16string &text, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a max width
 * std::string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextMaxW(const std::string &text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen with a max width
 * std::u16string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextMaxW(const std::u16string &text, double w, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen at a scaled size
 * std::string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextScaled(const std::string &text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints text to the a screen at a scaled size
 * std::u16string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextScaled(const std::u16string &text, double scaleX, double scaleY, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a set width
 * std::string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredMaxW(const std::string &text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen with a max width
 * std::u16string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredMaxW(const std::u16string &text, double w, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen at a scaled size
 * std::string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredScaled(const std::string &text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered text to the a screen at a scaled size
 * std::u16string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredScaled(const std::u16string &text, double scaleX, double scaleY, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a max width
 * std::string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedMaxW(const std::string &text, double w, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen with a max width
 * std::u16string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedMaxW(std::u16string text, double w, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen at a scaled size
 * std::string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedScaled(const std::string &text, double scaleX, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints centered colored text to the a screen at a scaled size
 * std::u16string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xOffset is the X offset from the center to center on
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextCenteredTintedScaled(std::u16string text, double scaleX, double scaleY, int palette, int xOffset, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a max width
 * std::string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedMaxW(const std::string &text, double w, double scaleY, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen with a max width
 * std::u16string text is the text that will be printed
 * double w is the max width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedMaxW(const std::u16string &text, double w,  double scaleY, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen at a scaled size
 * std::string text is the text that will be printed
 * double w is the width the text should be drawn at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedScaled(const std::string &text, double scaleX, double scaleY, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Prints colored text to the a screen at a scaled size
 * std::u16string text is the text that will be printed
 * double scaleX is the width scale the text should be draw at
 * double scaleY is the height scale the text should be draw at
 * int palette is the color to tint the text
 * int xPos is the X position to print at
 * int yPos is the Y position to print at
 * bool top is whether to draw on the top or bottom screen
 * bool invert is whether to swap the colors
 */
void printTextTintedScaled(const std::u16string &text, double scaleX, double scaleY, int palette, int xPos, int yPos, bool top, bool invert = false);

/**
 * Gets the width of a std::string of text with a max
 * const std::string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthMaxW(const std::string &text, int w);

/**
 * Gets the width of a std::u16string of text with a max
 * const std::u16string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthMaxW(const std::u16string &text, int w);

/**
 * Gets the width of a std::string of scaled text
 * const std::string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthScaled(const std::string &text, double scale);

/**
 * Gets the width of a std::u16string of scaled text
 * const std::u16string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidthScaled(const std::u16string &text, double scale);

/**
 * Gets the width of a std::string of text
 * const std::string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidth(const std::string &text);

/**
 * Gets the width of a std::u16string of text
 * const std::u16string &text is the text to get the width of
 * returns the width of the text
 */
int getTextWidth(const std::u16string &text);

#endif //GRAPHICS_HPP