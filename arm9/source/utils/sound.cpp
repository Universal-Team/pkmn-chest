#include "sound.hpp"
#include <dirent.h>
#include <maxmod9.h>
#include <nds.h>

#include "config.hpp"

#define SFX_BACK	0
#define SFX_CLICK	1
#define MOD_BGM		0

mm_sound_effect Sound::click, Sound::back;

void Sound::load(const char *path) {
	// char realPath[PATH_MAX];
	// if(access(path, F_OK) == 0) {
	// 	strcpy(realPath, path);
	// } else {
	// 	strcpy(realPath, "nitro:/sound/sfx.msl");
	// }
	mmInitDefault("nitro:/sound/center1.msl");
	
	mmLoadEffect(SFX_BACK);
	mmLoadEffect(SFX_CLICK);

	mmSetModuleVolume(0x200);

	Sound::back = {
		{SFX_BACK},				// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		0xC0,	// volume
		0x80,	// panning
	};
	Sound::click = {
		{SFX_CLICK},			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		0xC0,	// volume
		0x80,	// panning
	};

	mmLoad(MOD_BGM);
	mmStart(MOD_BGM, MM_PLAY_LOOP);
}

void Sound::play(mm_sound_effect &sound) {
	/*if(Config::getBool("playSfx"))*/	mmEffectEx(&sound);
}
