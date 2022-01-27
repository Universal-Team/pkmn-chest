#include "misc.hpp"

#include "PKX.hpp"
#include "banks.hpp"
#include "colors.hpp"
#include "config.hpp"
#include "flashcard.hpp"
#include "i18n.hpp"
#include "i18n_ext.hpp"
#include "input.hpp"
#include "loader.hpp"
#include "manager.hpp"
#include "sound.hpp"
#include "summary.hpp"

#include <strings.h>

struct Text {
	int x;
	int y;
	char text[32];
} textStatsC1[]{
	{-20, 20},
	{-20, 36},
	{-20, 52},
	{-20, 68},
	{-20, 84},
	{-20, 100},
},
	textStatsC2[]{
		{20, 20},
		{20, 36},
		{20, 52},
		{20, 68},
		{20, 84},
		{20, 100},
	},
	textStatsC3[]{
		{60, 20},
		{60, 36},
		{60, 52},
		{60, 68},
		{60, 84},
		{60, 100},
	},
	textStatsC4[]{
		{100, 20},
		{100, 36},
		{100, 52},
		{100, 68},
		{100, 84},
		{100, 100},
	};

std::vector<std::string> statsLabels = {
	"hp", "attack", "defense", "spAtk", "spDef", "speed", "base", "iv", "ev", "total"};
std::vector<std::string> originLabels = {
	"metLevel", "metYear", "metMonth", "metDay", "metLocation", "originGame", "fatefulEncounter"};

static constexpr pksm::Stat statOrder[] = {
	pksm::Stat::HP, pksm::Stat::ATK, pksm::Stat::DEF, pksm::Stat::SPATK, pksm::Stat::SPDEF, pksm::Stat::SPD};

void drawMiniBoxes(int currentBox) {
	if(currentBox < 0)
		currentBox = (topScreen ? Banks::bank->boxes() - 1 : save->maxBoxes() - 1) + currentBox;
	// Clear text
	drawRectangle(210, 0, 46, 192, CLEAR, false, true);

	// Load palette
	for(unsigned int i = 0; i < types.size(); i++) {
		BG_PALETTE_SUB[0xD0 + i] = types[i].palette[(i == 0 || i == 8) ? 1 : 2];
	}

	for(int i = 0; i < 5; i++) {
		drawRectangle(170, 10 + (i * 33), 38, 32, WHITE, false, true);
		drawOutline(170, 10 + (i * 33), 38, 32, DARK_GRAY, false, true);
		for(int j = 0; j < 30; j++) {
			if((topScreen ? Banks::bank->pkm(currentBox, j)->species() : save->pkm(currentBox, j)->species()) !=
			   pksm::Species::None) {
				// Type 1
				pksm::Type type =
					topScreen ? Banks::bank->pkm(currentBox, j)->type1() : save->pkm(currentBox, j)->type1();
				drawRectangle(
					172 + ((j - ((j / 6) * 6)) * 6), 12 + ((j / 6) * 6) + (i * 33), 2, 4, 0xD0 + u8(type), false, true);

				// Type 2
				type = topScreen ? Banks::bank->pkm(currentBox, j)->type2() : save->pkm(currentBox, j)->type2();
				drawRectangle(
					174 + ((j - ((j / 6) * 6)) * 6), 12 + ((j / 6) * 6) + (i * 33), 2, 4, 0xD0 + u8(type), false, true);
			}
		}
		// Print box number
		printText(std::to_string(currentBox + 1), 210, 20 + (i * 33), false, true);
		if(currentBox < (topScreen ? Banks::bank->boxes() - 1 : save->maxBoxes() - 1))
			currentBox++;
		else
			currentBox = 0;
	}
}

int selectBox(int currentBox) {
	setSpritePosition(arrowID, false, 205, 14);
	updateOam();
	drawMiniBoxes(currentBox);

	int pressed, held, screenPos = currentBox;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(currentBox > 0)
				currentBox--;
			else {
				screenPos =
					(topScreen ? Banks::bank->boxes() - 1 : save->maxBoxes() - 1) - (currentBox - screenPos - 1);
				currentBox = (topScreen ? Banks::bank->boxes() - 1 : save->maxBoxes() - 1);
			}
		} else if(held & KEY_DOWN) {
			if(currentBox < (topScreen ? Banks::bank->boxes() - 1 : save->maxBoxes() - 1))
				currentBox++;
			else {
				screenPos  = 0 - (currentBox - screenPos + 1);
				currentBox = 0;
			}
		} else if(pressed & KEY_A) {
			return currentBox;
		} else if(pressed & KEY_B) {
			return -1;
		}

		// Scroll screen if needed
		if(currentBox < screenPos) {
			screenPos = currentBox;
			drawMiniBoxes(screenPos);
		} else if(currentBox > screenPos + 4) {
			screenPos = currentBox - 4;
			drawMiniBoxes(screenPos);
		}

		// Move cursor
		setSpritePosition(arrowID, false, 205, 14 + (33 * (currentBox - screenPos)));
		updateOam();
	}
}

int selectForm(const pksm::PKX &pkm) {
	// Exit if only one form
	if(save->formCount(pkm.species()) == 1)
		return -1;

	if(pkm.species() == pksm::Species::Unown) {
		int num = tolower(Input::getLine(1)[0]);

		if(num == 33)
			return 26; // !
		else if(num == 63)
			return 27; // ?
		else if(num > 96 && num < 123)
			return num - 97; // a-z
		else
			return -1;
	}

	// Draw background
	drawRectangle(0, 60, 256, 72, DARKERER_GRAY, DARKER_GRAY, false, true);
	drawOutline(0, 60, 256, 72, LIGHT_GRAY, false, true);

	// Draw forms
	for(int i = 0; i < save->formCount(pkm.species()); i++) {
		Image image = loadPokemonSprite(getPokemonIndex(pkm.species(), i, pkm.gender(), pkm.egg()));
		fillSpriteImage(i, false, 32, 0, 0, image);
		setSpritePosition(i, false, (i * 32) + (128 - ((32 * save->formCount(pkm.species())) / 2)), 80);
		setSpritePriority(i, false, 1);
		setSpriteVisibility(i, false, true);
		setSpriteAlpha(i, false, 15);
	}
	updateOam();

	// Move arrow to current form
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(
		arrowID, false, (pkm.alternativeForm() * 32) + (128 - ((32 * save->formCount(pkm.species())) / 2)) + 28, 84);
	updateOam();

	int pressed, held, currentForm = pkm.alternativeForm();
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_LEFT) {
			if(currentForm > 0)
				currentForm--;
			else
				currentForm = save->formCount(pkm.species()) - 1;
		} else if(held & KEY_RIGHT) {
			if(currentForm < save->formCount(pkm.species()) - 1)
				currentForm++;
			else
				currentForm = 0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			resetPokemonSpritesPos(false);
			return currentForm;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			resetPokemonSpritesPos(false);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int i = 0; i < 5; i++) {
				if(touch.px > (i * 32) + (128 - ((32 * save->formCount(pkm.species())) / 2)) &&
				   touch.px < (i * 32) + (128 - ((32 * save->formCount(pkm.species())) / 2)) + 32 && touch.py > 72 &&
				   touch.py < 104) {
					Sound::play(Sound::click);
					resetPokemonSpritesPos(false);
					return i;
				}
			}
		}

		// Move arrow
		setSpritePosition(
			arrowID, false, (currentForm * 32) + (128 - ((32 * save->formCount(pkm.species())) / 2)) + 28, 84);
		updateOam();
	}
}

void drawItemList(int screenPos, const std::vector<std::string> &itemList, bool background) {
	if(background) {
		// Clear the screen
		drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);

		// Draw search icon
		drawImage(256 - search.width, 0, search, false, false);
	}

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Print items
	for(unsigned i = 0; i < std::min(9u, itemList.size() - screenPos); i++) {
		printText(itemList[screenPos + i], 4, 4 + (i * 20), false, true);
	}
}

std::string selectItem(int current, const std::vector<std::string> &strings) {
	std::vector<std::string> itemList = strings;

	// Set arrow position
	setSpritePosition(arrowID, false, 4 + getTextWidth(strings[current]), -2);
	setSpriteVisibility(arrowID, false, true);
	updateOam();

	// Print items
	drawItemList(current, itemList, true);

	int held, pressed, screenPos = current, newMove = current, entriesPerScreen = 9;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(newMove > 0)
				newMove--;
			else
				newMove = itemList.size() - 1;
		} else if(held & KEY_DOWN) {
			if(newMove < (int)itemList.size() - 1)
				newMove++;
			else
				newMove = 0;
		} else if(held & KEY_LEFT) {
			newMove -= entriesPerScreen;
			if(newMove < 0)
				newMove = 0;
		} else if(held & KEY_RIGHT) {
			newMove += entriesPerScreen;
			if(newMove > (int)itemList.size() - 1)
				newMove = itemList.size() - 1;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			for(unsigned int i = 0; i < strings.size(); i++) {
				drawRectangle(0, 0, 256, 192, CLEAR, false, true);
				return itemList[newMove];
			}
		}
		if(pressed & KEY_B) {
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			Sound::play(Sound::back);
			return itemList[current];
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			if(touch.px >= 256 - search.width && touch.py <= search.height) {
				goto search;
			}
			for(int i = 0; i < entriesPerScreen; i++) {
				if(touch.px >= 4 && touch.px <= 4 + getTextWidth(itemList[screenPos + i]) && touch.py >= 4 + (i * 20) &&
				   touch.py <= 4 + ((i + 1) * 20)) {
					drawRectangle(0, 0, 256, 192, CLEAR, false, true);
					return itemList[screenPos + i];
					break;
				}
			}
		} else if(pressed & KEY_Y) {
		search:
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			std::string str = Input::getLine();
			itemList.clear();
			if(str != "")
				itemList.push_back("-----");
			for(int i = 0; i < (int)strings.size(); i++) {
				if(strncasecmp(str.c_str(), strings[i].c_str(), str.length()) == 0) {
					itemList.push_back(strings[i]);
				}
			}
			newMove   = 0;
			screenPos = 0;
			drawItemList(screenPos, itemList, false);
			setSpriteVisibility(arrowID, false, true);
			updateOam();
		}

		// Scroll screen if needed
		if(newMove < screenPos) {
			screenPos = newMove;
			drawItemList(screenPos, itemList, false);
		} else if(newMove > screenPos + entriesPerScreen - 1) {
			screenPos = newMove - entriesPerScreen + 1;
			drawItemList(screenPos, itemList, false);
		}

		// Move cursor
		setSpritePosition(arrowID, false, 4 + getTextWidth(itemList[newMove]), (20 * (newMove - screenPos) - 2));
		updateOam();
	}
}

void selectMoves(pksm::PKX &pkm) {
	// Clear screen
	drawImageDMA(0, 0, listBg, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);
	printText(i18n::localize(Config::getLang("lang"), "moves"), 4, 0, false, true);

	// Print moves
	for(int i = 0; i < 4; i++) {
		printText(i18n::move(Config::getLang("lang"), pkm.move(i)), 4, 16 + (i * 16), false, true);
	}

	// Set arrow position
	setSpritePosition(arrowID, false, 4 + getTextWidth(i18n::move(Config::getLang("lang"), pkm.move(0))), 10);
	setSpriteVisibility(arrowID, false, true);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < 3)
				selection++;
		} else if(held & KEY_LEFT) {
			selection = 0;
		} else if(held & KEY_RIGHT) {
			selection = 3;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			optionSelected = true;
		} else if(pressed & KEY_B) {
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < 4; i++) {
				if(touch.px >= 4 && touch.px <= 4 + getTextWidth(i18n::move(Config::getLang("lang"), pkm.move(i))) &&
				   touch.py >= 16 + (i * 16) && touch.py <= 16 + ((i + 1) * 16)) {
					selection      = i;
					optionSelected = true;
					break;
				}
			}
		}

		if(optionSelected) {
			optionSelected = false;
			pkm.move(
				selection,
				selectItem(pkm.move(selection), save->availableMoves(), i18n::rawMoves(Config::getLang("lang"))));

			// Clear screen
			drawImageDMA(0, 0, listBg, false, false);
			drawRectangle(0, 0, 256, 192, CLEAR, false, true);
			printText(i18n::localize(Config::getLang("lang"), "moves"), 4, 0, false, true);

			// Print moves
			for(int i = 0; i < 4; i++) {
				printText(i18n::move(Config::getLang("lang"), pkm.move(i)), 4, 16 + (i * 16), false, true);
			}
		}

		setSpritePosition(arrowID,
						  false,
						  4 + getTextWidth(i18n::move(Config::getLang("lang"), pkm.move(selection))),
						  (selection * 16) + 10);
		updateOam();
	}
}

pksm::Nature selectNature(pksm::Nature currentNature) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw labels (not a for loop as speed is 3rd)
	{
		int x = -2;
		printTextCenteredTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[1]),
									48,
									1,
									TextColor::blue,
									((x++) * 48),
									4,
									false,
									false);
		printTextCenteredTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[2]),
									48,
									1,
									TextColor::blue,
									((x++) * 48),
									4,
									false,
									false);
		printTextCenteredTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[5]),
									48,
									1,
									TextColor::blue,
									((x++) * 48),
									4,
									false,
									false);
		printTextCenteredTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[3]),
									48,
									1,
									TextColor::blue,
									((x++) * 48),
									4,
									false,
									false);
		printTextCenteredTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[4]),
									48,
									1,
									TextColor::blue,
									((x++) * 48),
									4,
									false,
									false);

		int y = 0;
		printTextTintedScaled(i18n::localize(Config::getLang("lang"), statsLabels[1]),
							  0.8,
							  0.8,
							  TextColor::red,
							  1,
							  ((y++) * 32) + 22,
							  false,
							  false);
		printTextTintedScaled(i18n::localize(Config::getLang("lang"), statsLabels[2]),
							  0.8,
							  0.8,
							  TextColor::red,
							  1,
							  ((y++) * 32) + 22,
							  false,
							  false);
		printTextTintedScaled(i18n::localize(Config::getLang("lang"), statsLabels[5]),
							  0.8,
							  0.8,
							  TextColor::red,
							  1,
							  ((y++) * 32) + 22,
							  false,
							  false);
		printTextTintedScaled(i18n::localize(Config::getLang("lang"), statsLabels[3]),
							  0.8,
							  0.8,
							  TextColor::red,
							  1,
							  ((y++) * 32) + 22,
							  false,
							  false);
		printTextTintedScaled(i18n::localize(Config::getLang("lang"), statsLabels[4]),
							  0.8,
							  0.8,
							  TextColor::red,
							  1,
							  ((y++) * 32) + 22,
							  false,
							  false);
	}

	// Print natures
	for(int y = 0; y < 5; y++) {
		for(int x = 0; x < 5; x++) {
			printTextCenteredMaxW(i18n::nature(Config::getLang("lang"), pksm::Nature((y * 5) + x)),
								  48,
								  1,
								  ((x - 2) * 48),
								  (y * 32) + 32,
								  false,
								  false);
		}
	}

	int arrowX = u8(currentNature) - ((u8(currentNature) / 5) * 5), selection = u8(currentNature) / 5, pressed, held;
	// Move arrow to current nature
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID,
					  false,
					  (arrowX * 48) + (getTextWidthMaxW(i18n::nature(Config::getLang("lang"), currentNature), 48) / 2) +
						  28,
					  (selection * 32) + 24);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
			else
				selection = 4;
		} else if(held & KEY_DOWN) {
			if(selection < 4)
				selection++;
			else
				selection = 0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)
				arrowX--;
			else
				arrowX = 4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)
				arrowX++;
			else
				arrowX = 0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			return pksm::Nature((selection * 5) + arrowX);
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return pksm::Nature::INVALID;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y = 0; y < 5; y++) {
				for(int x = 0; x < 5; x++) {
					if(touch.px > (x * 48) + 8 && touch.px < (x * 48) + 56 && touch.py > (y * 32) + 8 &&
					   touch.py < (y * 32) + 56) {
						Sound::play(Sound::click);
						return pksm::Nature((y * 5) + x);
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(
			arrowID,
			false,
			(arrowX * 48) +
				(getTextWidthMaxW(i18n::nature(Config::getLang("lang"), pksm::Nature((selection * 5) + arrowX)), 48) /
				 2) +
				28,
			(selection * 32) + 24);
		updateOam();
	}
}

pksm::Ball selectPokeball(pksm::Ball currentBall) {
	if(currentBall < pksm::Ball::None || currentBall > pksm::Ball::Beast)
		currentBall = pksm::Ball::Poke;

	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw Pokéballs
	for(int y = 0; y < 5; y++) {
		for(int x = 0; x < 5; x++) {
			fillSpriteColor((y * 5) + x, false, CLEAR);
			fillSpriteImage((y * 5) + x, false, 32, 0, 0, ball[(y * 5) + x + 1]);
			setSpritePosition((y * 5) + x, false, (x * 48) + 24, (y * 32) + 23);
			setSpriteVisibility((y * 5) + x, false, true);
			setSpriteAlpha((y * 5) + x, false, 15);
		}
	}
	updateOam();

	u8 ball    = u8(currentBall) - 1;
	int arrowX = ball - ((ball / 5) * 5), selection = ball / 5, pressed, held;
	// Move arrow to current ball
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, (arrowX * 48) + 40, (selection * 32) + 16);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
			else
				selection = 4;
		} else if(held & KEY_DOWN) {
			if(selection < 4)
				selection++;
			else
				selection = 0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)
				arrowX--;
			else
				arrowX = 4;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 4)
				arrowX++;
			else
				arrowX = 0;
		} else if(pressed & KEY_A) {
			if(!(save->generation() < pksm::Generation::FIVE && (selection * 5) + arrowX == 24)) {
				Sound::play(Sound::click);
				resetPokemonSpritesPos(false);
				return pksm::Ball((selection * 5) + arrowX + 1);
			}
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			resetPokemonSpritesPos(false);
			return pksm::Ball::INVALID;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y = 0; y < 5; y++) {
				for(int x = 0; x < 5; x++) {
					if(touch.px > (x * 48) + 8 && touch.px < (x * 48) + 56 && touch.py > (y * 32) + 8 &&
					   touch.py < (y * 32) + 56) {
						if(!(save->generation() < pksm::Generation::FIVE && (y * 5) + x == 24)) {
							Sound::play(Sound::click);
							resetPokemonSpritesPos(false);
							return pksm::Ball((y * 5) + x + 1);
						}
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(arrowID, false, (arrowX * 48) + 40, (selection * 32) + 16);
		updateOam();
	}
}

int selectWallpaper(int currentWallpaper) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw wallpapers
	for(int y = 0; y < 4; y++) {
		for(int x = 0; x < 6; x++) {
			Image image = loadImage(boxBgPath(false, (y * 6) + x));
			if(!(image.width == 0 || image.height == 0)) {
				fillSpriteImageScaled((y * 6) + x,
									  false,
									  32,
									  0,
									  0,
									  32.0f / std::max(image.width, image.height),
									  32.0f / std::max(image.width, image.height),
									  image);
				setSpritePosition((y * 6) + x, false, (x * 40) + 12, (y * 38) + 24);
				setSpriteVisibility((y * 6) + x, false, true);
				setSpriteAlpha((y * 6) + x, false, 15);
			}
		}
	}
	updateOam();

	int arrowX = currentWallpaper - ((currentWallpaper / 6) * 6), selection = currentWallpaper / 6, pressed, held;
	// Move arrow to current wallpaper
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, (arrowX * 36) + 44, (selection * 36) + 20);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
			else
				selection = 3;
		} else if(held & KEY_DOWN) {
			if(selection < 3)
				selection++;
			else
				selection = 0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)
				arrowX--;
			else
				arrowX = 5;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 5)
				arrowX++;
			else
				arrowX = 0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			resetPokemonSpritesPos(false);
			return (selection * 6) + arrowX;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			resetPokemonSpritesPos(false);
			return -1;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y = 0; y < 4; y++) {
				for(int x = 0; x < 6; x++) {
					if(touch.px > (x * 40) + 12 && touch.px < (x * 40) + 44 && touch.py > (y * 38) + 24 &&
					   touch.py < (y * 38) + 56) {
						Sound::play(Sound::click);
						resetPokemonSpritesPos(false);
						return (y * 6) + x;
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(arrowID, false, (arrowX * 36) + 44, (selection * 36) + 20);
		updateOam();
	}
}

void drawOriginPage(const pksm::PKX &pkm, std::vector<std::string> &varText) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Print text
	varText = {
		std::to_string(pkm.metLevel()),
		std::to_string(pkm.metDate().year()),
		std::to_string(pkm.metDate().month()),
		std::to_string(pkm.metDate().day()),
		i18n::location(Config::getLang("lang"), pkm.generation(), pkm.metLocation()),
		i18n::game(Config::getLang("lang"), pkm.version()),
		pkm.fatefulEncounter() ? i18n::localize(Config::getLang("lang"), "yes")
							   : i18n::localize(Config::getLang("lang"), "no"),
	};
	printText(i18n::localize(Config::getLang("lang"), "origin"), 4, 0, false, true);
	for(unsigned i = 0; i < originLabels.size(); i++) {
		printText(
			i18n::localize(Config::getLang("lang"), originLabels[i]) + ": " + varText[i], 4, (i + 1) * 16, false, true);
	}
}

void selectOrigin(pksm::PKX &pkm) {
	std::vector<std::string> varText;
	drawImageDMA(0, 0, listBg, false, false);
	drawOriginPage(pkm, varText);

	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID,
					  false,
					  4 + getTextWidth(i18n::localize(Config::getLang("lang"), originLabels[0]) + ": " + varText[0]),
					  10);
	updateOam();

	bool optionSelected = false;
	int pressed, held, selection = 0;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < (int)originLabels.size() - 1)
				selection++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(unsigned i = 0; i < originLabels.size(); i++) {
				if(touch.py > 4 + (i * 20) && touch.py < 4 + (i + 1) * 20) {
					selection      = i;
					optionSelected = true;
				}
			}
		}

		if(optionSelected) {
			Sound::play(Sound::click);
			optionSelected = false;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			switch(selection) {
				case 0: { // Level
					int num = Input::getInt(100);
					if(num != -1)
						pkm.metLevel(num);
					break;
				}
				case 1: { // Year
					int num      = Input::getInt(2099);
					Date metDate = pkm.metDate();
					if(num != -1 && num <= (num > 2000 ? 2099 : 99)) {
						if(num < 2000)
							metDate.year(num + 2000);
						else
							metDate.year(num);

						bool leapYear =
							metDate.year() % 400 == 0 || (metDate.year() % 4 == 0 && metDate.year() % 100 != 0);
						if(leapYear && metDate.month() == 2 && metDate.day() > 28) {
							pkm.metDate(28);
						}
					}
					pkm.metDate(metDate);
					break;
				}
				case 2: { // Month
					int num = Input::getInt(12);
					if(num > 0) {
						Date metDate = pkm.metDate();
						bool leapYear =
							metDate.year() % 400 == 0 || (metDate.year() % 4 == 0 && metDate.year() % 100 != 0);

						if(num == 2 && metDate.day() > (leapYear ? 29 : 28)) {
							metDate.day(leapYear ? 29 : 28);
						} else if((num == 4 || num == 6 || num == 9 || num == 11) && metDate.day() > 30) {
							metDate.day(30);
						}

						metDate.month(num);

						pkm.metDate(metDate);
					}
					break;
				}
				case 3: { // Day
					int num;
					Date metDate  = pkm.metDate();
					bool leapYear = metDate.year() % 400 == 0 || (metDate.year() % 4 == 0 && metDate.year() % 100 != 0);

					switch(metDate.month()) {
						case 2:
							num = Input::getInt(leapYear ? 29 : 28);
							break;
						case 4:
						case 6:
						case 9:
						case 11:
							num = Input::getInt(30);
							break;
						default:
							num = Input::getInt(31);
							break;
					}
					if(num > 0)
						metDate.day(num);
					pkm.metDate(metDate);
					break;
				}
				case 4: { // Location
					std::vector<std::string> locations;
					int location = 0, i = 0;
					for(auto it = i18n::rawLocations(Config::getLang("lang"), pkm.generation()).begin();
						it != i18n::rawLocations(Config::getLang("lang"), pkm.generation()).end();
						it++) {
						locations.push_back(it->second);
						if(it->first == pkm.metLocation())
							location = i;
						i++;
					}

					int num = selectItem<int>(location, 0, locations.size(), locations);

					for(auto it = i18n::rawLocations(Config::getLang("lang"), pkm.generation()).begin();
						it != i18n::rawLocations(Config::getLang("lang"), pkm.generation()).end();
						it++) {
						if(it->second == locations[num]) {
							pkm.metLocation(it->first);
							break;
						}
					}

					break;
				}
				case 5: { // Game
					pksm::GameVersion ver =
						selectItem<pksm::GameVersion>(pkm.version(),
													  0,
													  i18n::rawGames(Config::getLang("lang")).size(),
													  i18n::rawGames(Config::getLang("lang")));
					pkm.version(ver);
					break;
				}
				case 6: { // Fateful encounter
					pkm.fatefulEncounter(!pkm.fatefulEncounter());
				}
			}
			drawOriginPage(pkm, varText);
			setSpriteVisibility(arrowID, false, true);
			updateOam();
		}

		// Move arrow
		setSpritePosition(arrowID,
						  false,
						  4 +
							  getTextWidth(i18n::localize(Config::getLang("lang"), originLabels[selection]) + ": " +
										   varText[selection]),
						  (selection * 16) + 10);
		updateOam();
	}
}

void drawStatsPage(const pksm::PKX &pkm, bool background) {
	if(background) {
		// Clear the screen
		drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);

		// Draw lines
		for(unsigned i = 1; i < (sizeof(textStatsC1) / sizeof(textStatsC1[0])); i++) {
			drawRectangle(16, textStatsC1[i].y, 230, 1, LIGHT_GRAY, false, false);
		}
		drawRectangle(128, 4, 1, 112, LIGHT_GRAY, false, false);
		drawRectangle(168, 4, 1, 112, LIGHT_GRAY, false, false);
		drawRectangle(208, 4, 1, 112, LIGHT_GRAY, false, false);
	}

	// Clear text
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Print stat info labels
	{
		u8 i = u8(pkm.nature());
		printText(i18n::localize(Config::getLang("lang"), statsLabels[0]), 20, textStatsC1[0].y, false, true);
		printTextTintedMaxW(
			i18n::localize(Config::getLang("lang"), statsLabels[1]),
			80,
			1,
			(i != 0 && i < 5 ? TextColor::red : i != 0 && !(i % 5) ? TextColor::blue : TextColor::white),
			20,
			textStatsC1[1].y,
			false,
			true);
		printTextTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[2]),
							80,
							1,
							(i != 6 && i > 4 && i < 10 ? TextColor::red
													   : i != 6 && !((i - 1) % 5) ? TextColor::blue : TextColor::white),
							20,
							textStatsC1[2].y,
							false,
							true);
		printTextTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[3]),
							80,
							1,
							(i != 18 && i > 14 && i < 20
								 ? TextColor::red
								 : i != 18 && !((i - 3) % 5) ? TextColor::blue : TextColor::white),
							20,
							textStatsC1[3].y,
							false,
							true);
		printTextTintedMaxW(
			i18n::localize(Config::getLang("lang"), statsLabels[4]),
			80,
			1,
			(i != 24 && i > 19 ? TextColor::red : i != 24 && !((i - 4) % 5) ? TextColor::blue : TextColor::white),
			20,
			textStatsC1[4].y,
			false,
			true);
		printTextTintedMaxW(i18n::localize(Config::getLang("lang"), statsLabels[5]),
							80,
							1,
							(i != 12 && i > 9 && i < 15
								 ? TextColor::red
								 : i != 12 && !((i - 2) % 5) ? TextColor::blue : TextColor::white),
							20,
							textStatsC1[5].y,
							false,
							true);
	}

	// Print column titles
	printTextCenteredMaxW(i18n::localize(Config::getLang("lang"), statsLabels[6]),
						  30,
						  1,
						  textStatsC1[0].x,
						  textStatsC1[0].y - 16,
						  false,
						  true);
	printTextCentered(
		i18n::localize(Config::getLang("lang"), statsLabels[7]), textStatsC2[0].x, textStatsC2[0].y - 16, false, true);
	printTextCentered(
		i18n::localize(Config::getLang("lang"), statsLabels[8]), textStatsC3[0].x, textStatsC3[0].y - 16, false, true);
	printTextCenteredMaxW(i18n::localize(Config::getLang("lang"), statsLabels[9]),
						  30,
						  1,
						  textStatsC4[0].x,
						  textStatsC4[0].y - 16,
						  false,
						  true);

	// Set base stat info
	snprintf(textStatsC1[0].text, sizeof(textStatsC1[0].text), "%i", pkm.baseHP());
	snprintf(textStatsC1[1].text, sizeof(textStatsC1[1].text), "%i", pkm.baseAtk());
	snprintf(textStatsC1[2].text, sizeof(textStatsC1[2].text), "%i", pkm.baseDef());
	snprintf(textStatsC1[3].text, sizeof(textStatsC1[3].text), "%i", pkm.baseSpa());
	snprintf(textStatsC1[4].text, sizeof(textStatsC1[4].text), "%i", pkm.baseSpd());
	snprintf(textStatsC1[5].text, sizeof(textStatsC1[5].text), "%i", pkm.baseSpe());

	// Set & print other stat info and
	for(unsigned i = 0; i < (sizeof(textStatsC1) / sizeof(textStatsC1[0])); i++) {
		snprintf(textStatsC2[i].text, sizeof(textStatsC2[i].text), "%i", pkm.iv(statOrder[i]));
		snprintf(textStatsC3[i].text, sizeof(textStatsC3[i].text), "%i", pkm.ev(statOrder[i]));
		snprintf(textStatsC4[i].text, sizeof(textStatsC4[i].text), "%i", pkm.stat(statOrder[i]));

		printTextCentered(textStatsC1[i].text, textStatsC1[i].x, textStatsC1[i].y, false, true);
		printTextCentered(textStatsC2[i].text, textStatsC2[i].x, textStatsC2[i].y, false, true);
		printTextCentered(textStatsC3[i].text, textStatsC3[i].x, textStatsC3[i].y, false, true);
		printTextCentered(textStatsC4[i].text, textStatsC4[i].x, textStatsC4[i].y, false, true);
	}

	// Draw Hidden Power type
	printText(i18n::localize(Config::getLang("lang"), "hpType") + ":", 20, 118, false, true);
	drawImage(24 + getTextWidth(i18n::localize(Config::getLang("lang"), "hpType") + ":"),
			  120,
			  types[u8(pkm.hpType())],
			  false,
			  true);
}

void selectStats(pksm::PKX &pkm) {
	drawStatsPage(pkm, true);
	setSpritePosition(
		arrowID, false, 128 + (textStatsC2[0].x + (getTextWidth(textStatsC2[0].text) / 2)) + 2, textStatsC2[0].y - 6);
	setSpriteVisibility(arrowID, false, true);
	updateOam();

	bool optionSelected = false;
	int held, pressed, selection = 0, column = 0;
	touchPosition touch;
	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
		} else if(held & KEY_DOWN) {
			if(selection < 6)
				selection++;
		} else if(pressed & KEY_LEFT) {
			if(column > 0)
				column--;
		} else if(held & KEY_RIGHT) {
			if(column < 1)
				column++;
		} else if(pressed & KEY_A) {
			optionSelected = true;
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			return;
		} else if(pressed & KEY_TOUCH) {
			touchRead(&touch);
			for(unsigned i = 0; i < (sizeof(textStatsC2) / sizeof(textStatsC2[0])); i++) {
				if(touch.px >= 128 + (textStatsC2[selection].x - (getTextWidth(textStatsC2[selection].text) / 2)) &&
				   touch.px <= 128 + (textStatsC2[selection].x + (getTextWidth(textStatsC2[selection].text) / 2)) &&
				   touch.py >= textStatsC2[i].y && touch.py <= textStatsC2[i].y + 16) {
					column         = 0;
					selection      = i;
					optionSelected = true;
					break;
				}
			}
			for(unsigned i = 0; i < (sizeof(textStatsC3) / sizeof(textStatsC3[0])); i++) {
				if(touch.px >= 128 + (textStatsC3[selection].x - (getTextWidth(textStatsC3[selection].text) / 2)) &&
				   touch.px <= 128 + (textStatsC3[selection].x + (getTextWidth(textStatsC3[selection].text) / 2)) &&
				   touch.py >= textStatsC3[i].y && touch.py <= textStatsC3[i].y + 16) {
					column         = 1;
					selection      = i;
					optionSelected = true;
					break;
				}
			}
			if(touch.px > 24 + getTextWidth(i18n::localize(Config::getLang("lang"), "hpType") + ":") &&
			   touch.px < 24 + getTextWidth(i18n::localize(Config::getLang("lang"), "hpType") + ":") +
					   types[u8(pkm.hpType())].width &&
			   touch.py > 120 && touch.py < 132) {
				selection      = 6;
				optionSelected = true;
			}
		}

		if(optionSelected) {
			optionSelected = 0;
			setSpriteVisibility(arrowID, false, false);
			updateOam();
			if(selection == 6) { // Hidden Power Type
				pksm::Type type = selectHPType(pkm.hpType());
				if(type != pksm::Type::INVALID)
					pkm.hpType(type);
			} else if(column == 0) { // IV
				int num = Input::getInt(31);
				if(num != -1)
					pkm.iv(statOrder[selection], num);
			} else { // EV
				int total = 0;
				for(int i = 0; i < 6; i++) {
					if(i != selection)
						total += pkm.ev(statOrder[i]);
				}
				int num = Input::getInt(std::min(510 - total, 255));
				if(num != -1)
					pkm.ev(statOrder[selection], num);
			}
			setSpriteVisibility(arrowID, false, true);
			updateOam();
			drawStatsPage(pkm, false);
		}

		if(selection == 6) { // Hidden Power type
			setSpritePosition(arrowID,
							  false,
							  25 + getTextWidth(i18n::localize(Config::getLang("lang"), "hpType") + ":") +
								  types[u8(pkm.hpType())].width + 2,
							  112);
		} else if(column == 0) {
			setSpritePosition(arrowID,
							  false,
							  128 + (textStatsC2[selection].x + (getTextWidth(textStatsC2[selection].text) / 2)) + 2,
							  textStatsC2[selection].y - 6);
		} else {
			setSpritePosition(arrowID,
							  false,
							  128 + (textStatsC3[selection].x + (getTextWidth(textStatsC3[selection].text) / 2)) + 2,
							  textStatsC3[selection].y - 6);
		}
		updateOam();
	}
}

pksm::Type selectHPType(pksm::Type type) {
	// Clear screen
	drawRectangle(0, 0, 256, 192, DARKERER_GRAY, DARKER_GRAY, false, false);
	drawRectangle(0, 0, 256, 192, CLEAR, false, true);

	// Draw types
	for(int y = 0; y < 4; y++) {
		for(int x = 0; x < 4; x++) {
			fillSpriteColor((y * 4) + x, false, CLEAR);
			fillSpriteImage((y * 4) + x, false, 32, 0, 0, types[(y * 4) + x + 1]);
			setSpritePosition((y * 4) + x,
							  false,
							  (x * 52) + 34 + ((32 - types[0].width) / 2),
							  (y * 32) + 42 + ((12 - types[0].height) / 2));
			setSpriteVisibility((y * 4) + x, false, true);
			setSpriteAlpha((y * 4) + x, false, 15);
		}
	}
	updateOam();

	u8 currentType = u8(type) - 1;
	int arrowX = currentType - ((currentType / 4) * 4), selection = currentType / 4, pressed, held;
	// Move arrow to current wallpaper
	setSpriteVisibility(arrowID, false, true);
	setSpritePosition(arrowID, false, (arrowX * 52) + 34 + types[0].width + 2, (selection * 32) + 34);
	updateOam();

	while(1) {
		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held    = keysDownRepeat();
		} while(!held);

		if(held & KEY_UP) {
			if(selection > 0)
				selection--;
			else
				selection = 3;
		} else if(held & KEY_DOWN) {
			if(selection < 3)
				selection++;
			else
				selection = 0;
		} else if(held & KEY_LEFT) {
			if(arrowX > 0)
				arrowX--;
			else
				arrowX = 3;
		} else if(held & KEY_RIGHT) {
			if(arrowX < 3)
				arrowX++;
			else
				arrowX = 0;
		} else if(pressed & KEY_A) {
			Sound::play(Sound::click);
			resetPokemonSpritesPos(false);
			return pksm::Type((selection * 4) + arrowX + 1);
		} else if(pressed & KEY_B) {
			Sound::play(Sound::back);
			resetPokemonSpritesPos(false);
			return pksm::Type::INVALID;
		} else if(pressed & KEY_TOUCH) {
			touchPosition touch;
			touchRead(&touch);
			for(int y = 0; y < 4; y++) {
				for(int x = 0; x < 4; x++) {
					if(touch.px > (x * 52) + 34 && touch.px < (x * 52) + 34 + types[0].width &&
					   touch.py > (y * 32) + 42 && touch.py < (y * 32) + 42 + types[0].height) {
						Sound::play(Sound::click);
						resetPokemonSpritesPos(false);
						return pksm::Type((y * 4) + x + 1);
					}
				}
			}
		}

		// Move arrow
		setSpritePosition(arrowID, false, (arrowX * 52) + 34 + types[0].width + 2, (selection * 32) + 34);
		updateOam();
	}
}
