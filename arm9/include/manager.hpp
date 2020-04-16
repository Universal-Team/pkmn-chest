#ifndef MANAGER_HPP
#define MANAGER_HPP
#include <nds.h>

#include "graphics.hpp"
#include "loader.hpp"
#include "Sav.hpp"

#define BALL_COUNT 26

extern bool topScreen, inParty;
extern int arrowID, currentSaveBox, currentBankBox, heldPokemonID, keyboardSpriteID, arrowMode;
extern std::vector<int> menuIconID, partyIconID;
extern std::string savePath;
extern Image arrowBlue, arrowRed, arrowYellow, ball[BALL_COUNT], bankBox, boxBgTop, boxButton, infoBox, itemIcon, keyboardKey, listBg, menuBg, menuButton, menuButtonBlue, party, search, setToSelf, shiny;
extern std::vector<Image> types;

// Returns the current box for the cursor's current screen
int currentBox(void);

/*
 * Returns the Pokémon in the provided slot
 * on the cursor's current screen
 */
std::unique_ptr<PKX> currentPokemon(int x, int y);

/*
 * Gets the image offset for a Pokémon
 * const PKX &pkm is the Pokémon to get the offset for
 * Returns an int of the offset to use
 */
int getPokemonIndex(const PKX &pkm);

/*
 * Gets the image offset for a Pokémon
 * Species species is the Species of the Pokémon
 * u16 alternativeForm is the alternative form of the Pokémon
 * Gender gender is the Gender of the Pokémon
 * bool egg is whether the Pokémon is in an egg
 * Returns an int of the offset to use
 */
int getPokemonIndex(Species species, u16 alternativeForm, Gender gender, bool egg);

/*
 * Loads a Pokémon sprite into an Image struct
 * int index is the Pokémon sprite's index
 */
Image loadPokemonSprite(int index);

/* Resets the Pokémon sprites to their positions for the box
 * bool top is whether to reset the top or bottom sprites
 */
void resetPokemonSpritesPos(bool top);

/*
 * Fills the arrow sprite with the desired color
 * int arrowMode is the desired color, 0 = red, 1 = blue, 2 = yellow
 */
void fillArrow(int arrowMode);

/* 
 * Loads type images
 * Language lang is the language to try load, if it fails English will be used
 */
void loadTypes(Language lang);

// Inits sprites, run before first loadGraphics(), but should only be run once
void initSprites(void);

/* Loads the images into RAM and creates the sprites
 * initSprites() must be run before running this for the first time
 */
void loadGraphics(void);

// Draws both boxes and the backgrounds
void drawBoxScreen(void);

// Returns the path to the current box's wallpaper
std::string boxBgPath(bool top, int box = save->boxWallpaper(currentSaveBox));

// Draws a box
void drawBox(bool top);

// Draws the info for the Pokémon provided
void drawPokemonInfo(const PKX &pkm);

// Fills the held Pokémon sprite
void setHeldPokemon(const PKX &pkm);

// Main Pokémon box mangaement loop
void manageBoxes(void);

#endif
