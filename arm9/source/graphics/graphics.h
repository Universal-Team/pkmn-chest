#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <nds.h>
#include <string>
#include <vector>

// Converts 3 0-255 (0x0-0xff) values to a BGR15 value
#define BGR15(b, g, r)   ((b)|((g)<<5)|((r)<<10))

struct ImageData {
    uint width;
    uint height;
};


// Initializes the screens for drawing
void initGraphics(void);

/*
 * Loads a .bmp image into a vector of raw pixel data
 * std::string path is the path of the .bmp file
 * std::vector<u16>& imageBuffer is the vector to load the raw pixel data into
 * Returns an ImageData with the Width and Height of the image
 */
ImageData loadBmp(std::string path, std::vector<u16>& imageBuffer);

/*
 * Loads a .png image into a vector of raw pixel data
 * std::string path is the path of the .png file
 * std::vector<u16>& imageBuffer is the vector to load the raw pixel data into
 * Returns an ImageData with the Width and Height of the image
 */
ImageData loadPng(std::string path, std::vector<u16>& imageBuffer);

/*
 * Draws an image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * std::vector<u16> imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImage(int x, int y, int w, int h, std::vector<u16> imageBuffer, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * double scale is the Scale to draw the image at
 * std::vector<u16> imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageScaled(int x, int y, int w, int h, double scale, std::vector<u16> imageBuffer, bool top);

/*
 * Draws a scaled image to the screen from a vector of raw pixel data
 * int x is the X position
 * int y is the Y position
 * int w is the Width
 * int h is the Height
 * u16 color is the color to tint the image
 * std::vector<u16> imageBuffer is the raw pixel data
 * bool top is whether to draw on the top or bottom screen
 */
void drawImageTinted(int x, int y, int w, int h, u16 color, std::vector<u16> imageBuffer, bool top);

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

#endif //GRAPHICS_H
