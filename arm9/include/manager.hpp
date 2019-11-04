#ifndef MANAGER_HPP
#define MANAGER_HPP
#include <nds.h>

#include "graphics.hpp"
#include "loader.hpp"
#include "Sav.hpp"

extern bool topScreen;
extern int arrowID, shinyID, currentSaveBox, currentBankBox, heldPokemonID, keyboardSpriteID, arrowMode;
extern std::vector<int> menuIconID, partyIconID;
extern std::string savePath;
extern Image arrowBlue, arrowRed, arrowYellow, ballSheet, bankBox, boxBgTop, boxButton, fileBrowseBg, infoBox, keyboardKey, menuBg, menuButton, menuButtonBlue, menuIconSheet, optionsBg, search, shiny, listBg, types;

#define boxHeight() (save->generation() == Generation::FOUR ? 148 : 151)
#define boxWidth() (save->generation() == Generation::FOUR ? 162 : 158)
#define typesHeight() ((Config::getLang("lang") == Lang::en || Config::getLang("lang") == Lang::it || Config::getLang("lang") == Lang::jp) ? 12 : 14)
#define typesWidth() (Config::getLang("lang") == Lang::jp ? 30 : 32)

// Returns the current box for the cursor's current screen
int currentBox(void);

/*
 * Returns the Pokémon in the provided slot
 * on the cursor's current screen
 */
std::shared_ptr<PKX> currentPokemon(int slot);

/*
 * Gets the bmp number for a Pokémon
 * std::shared_ptr<PKX> pkm is the Pokémon to get the bmp for
 * Returns an int of the bmp to use
 */
int getPokemonIndex(std::shared_ptr<PKX> pkm);

/*
 * Gets the bmp number for a Pokémon
 * int species is the National dex number of the Pokémon
 * int alternativeForm is the alternative form of the Pokémon
 * int gender is the gender of the Pokémon
 * bool egg is wheher the Pokémon is in an egg
 * Returns an int of the bmp to use
 */
int getPokemonIndex(int species, int alternativeForm = 0, int gender = 0, bool egg = false);

/*
 * Loads a Pokémon sprite into an Image struct
 * int dexNo is the Pokémon dex number
 */
Image loadPokemonSprite(int dexNo);

// Loads the images into RAM and creates the sprites
void loadGraphics(void);

// Draws both boxes and the backgrounds
void drawBoxScreen(void);

// Returns the path to the current box's wallpaper
std::string boxBgPath(bool top, int box = save->boxWallpaper(currentSaveBox));

// Draws a box
void drawBox(bool top);

// Draws the info for the Pokémon provided
void drawPokemonInfo(std::shared_ptr<PKX> pkm);

// Fills the held Pokémon sprite
void setHeldPokemon(std::shared_ptr<PKX> pkm);

// Main Pokémon box mangaement loop
void manageBoxes(void);

#endif
