#include "colors.hpp"
#include <nds.h>

#include "config.hpp"
#include "graphics.hpp"
#include "json.hpp"
#include "tonccpy.h"

const u16 defaultPalette[] = {
	0, 0xFBDE, 0xBDEF, // WHITE_TEXT
	0, 0x8C63, 0xCA52, // GRAY_TEXT
	0, (u16)(0x801F & 0xFBDE), (u16)(0x801F & 0xBDEF), // RED_TEXT
	0, (u16)(0xFC00 & 0xFBDE), (u16)(0xFC00 & 0xBDEF), // BLUE_TEXT
	0xE739, 0x98C6, 0x94A5, 0x8842
};

const std::string keys[] = {"CLEAR", "WHITE", "DARKISH_GRAY", "CLEAR", "BLACK", "GRAY", "CLEAR", "RED", "DARK_RED", "CLEAR", "BLUE", "DARK_BLUE", "LIGHT_GRAY", "DARK_GRAY", "DARKER_GRAY", "DARKERER_GRAY"};

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
