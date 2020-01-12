#include "colors.hpp"
#include <nds.h>

#include "config.hpp"
#include "graphics.hpp"
#include "json.hpp"
#include "tonccpy.h"

const u16 defaultPalette[] = {
	0, 0xFBDE, 0xBDEF, 0xE739, // WHITE_TEXT
	0, 0x8C63, 0xCA52, 0x98C6, // GRAY_TEXT
	0, 0x801E, 0x800F, 0x94A5, // RED_TEXT
	0, 0xF800, 0xBC00, 0x8842, // BLUE_TEXT
};

const std::string keys[] = {"CLEAR", "WHITE", "DARKISH_GRAY", "LIGHT_GRAY", "CLEAR2", "BLACK", "GRAY", "DARK_GRAY", "CLEAR3", "RED", "DARK_RED", "DARKER_GRAY", "CLEAR4", "BLUE", "DARK_BLUE", "DARKERER_GRAY"};

void Colors::load(void) {
	nlohmann::json themeJson;

	FILE* file = fopen((Config::getString("themeDir")+"/theme.json").c_str(), "r");
	if(file) {
		themeJson = nlohmann::json::parse(file, nullptr, false);
		fclose(file);
	}

	u16 palette[16];
	tonccpy(palette, defaultPalette, sizeof(palette));

	for(int i=0;i<16;i++) {
		if(themeJson.contains(keys[i]) && themeJson[keys[i]].is_string()) {
			palette[i] = stoi(themeJson[keys[i]].get_ref<std::string&>(), nullptr, 0);
		}
	}

	tonccpy(BG_PALETTE, palette, sizeof(palette));
	tonccpy(BG_PALETTE_SUB, palette, sizeof(palette));
}
