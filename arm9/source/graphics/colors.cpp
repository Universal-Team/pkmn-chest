#include "colors.hpp"
#include <nds.h>

#include "config.hpp"
#include "graphics.hpp"
#include "json.hpp"
#include "tonccpy.h"

const u16 defaultPalette[] = {
	0x0000, 0xFBDE, 0xE739, 0xCA52, 0xBDEF, 0x98C6, 0x94A5, 0x8842, 0x8000, // Grays
	0x801E, 0x800F, 0xF800, 0xBC00, 0, 0, 0, // Colors
	0x0000, 0xFBDE, 0xBDEF, 0x0000, // WHITE_TEXT
	0x0000, 0x8C63, 0xCA52, 0x0000, // GRAY_TEXT
	0x0000, 0x801E, 0x800F, 0x0000, // RED_TEXT
	0x0000, 0xF800, 0xBC00, 0x0000, // BLUE_TEXT
};

const std::string keys[] = {
	"CLEAR", "WHITE", "LIGHT_GRAY", "GRAY", "DARKISH_GRAY", "DARK_GRAY", "DARKER_GRAY", "DARKERER_GRAY", "BLACK",
	"RED", "DARK_RED", "BLUE", "DARK_BLUE",
	"WHITE_TEXT_1", "WHITE_TEXT_2", "WHITE_TEXT_3", "WHITE_TEXT_4"
	"GRAY_TEXT_1",  "GRAY_TEXT_2",  "GRAY_TEXT_3",  "GRAY_TEXT_4"
	"RED_TEXT_1",   "RED_TEXT_2",   "RED_TEXT_3",   "RED_TEXT_4"
	"BLUE_TEXT_1",  "BLUE_TEXT_2",  "BLUE_TEXT_3",  "BLUE_TEXT_4"
};

void Colors::load(void) {
	nlohmann::json themeJson;

	FILE* file = fopen((Config::getString("themeDir")+"/theme.json").c_str(), "r");
	if(file) {
		themeJson = nlohmann::json::parse(file, nullptr, false);
		fclose(file);
	}

	u16 palette[sizeof(defaultPalette)];
	tonccpy(palette, defaultPalette, sizeof(palette));

	for(int i=0;i<16;i++) {
		if(themeJson.contains(keys[i]) && themeJson[keys[i]].is_string()) {
			palette[i] = stoi(themeJson[keys[i]].get_ref<std::string&>(), nullptr, 0);
		}
	}

	tonccpy(BG_PALETTE, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, palette, sizeof(palette));
}
