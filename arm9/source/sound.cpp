#include "sound.hpp"
#include <nds.h>
#include <maxmod9.h>

#include "config.hpp"
#include "soundbank.h"
#include "soundbank_bin.h"

mm_sound_effect Sound::click, Sound::back;

void Sound::init(void) {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect(SFX_CLICK);
	mmLoadEffect(SFX_BACK);
	mmLoad(MOD_BGMCENTER1);
	mmLoad(MOD_BGMCENTER4);
	mmLoad(MOD_BGMCENTER5);
	mmLoad(MOD_BGMTWINLEAFTOWN);
	mmLoad(MOD_BGMELM);
	mmLoad(MOD_BGMOAK);
	mmLoad(MOD_BGMGAMECORNER);

	mmSetModuleVolume(0x200);

	Sound::click = {
		{SFX_CLICK},			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		0xC0,	// volume
		0x80,	// panning
	};
	Sound::back = {
		{SFX_BACK},				// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		0xC0,	// volume
		0x80,	// panning
	};

	if(Config::music == 1)		mmStart(MOD_BGMCENTER1, MM_PLAY_LOOP);
	else if(Config::music == 2)	mmStart(MOD_BGMCENTER4, MM_PLAY_LOOP);
	else if(Config::music == 3)	mmStart(MOD_BGMCENTER5, MM_PLAY_LOOP);
	else if(Config::music == 4)	mmStart(MOD_BGMTWINLEAFTOWN, MM_PLAY_LOOP);
	else if(Config::music == 5)	mmStart(MOD_BGMELM, MM_PLAY_LOOP);
	else if(Config::music == 6)	mmStart(MOD_BGMOAK, MM_PLAY_LOOP);
	else if(Config::music == 7)	mmStart(MOD_BGMGAMECORNER, MM_PLAY_LOOP);
}

void Sound::play(mm_sound_effect &sound) {
	if(Config::playSfx)	mmEffectEx(&sound);
}

void Sound::playBgm(int song) {
	if(song == 0)		mmStop();
	else if(song == 1)	mmStart(MOD_BGMCENTER1, MM_PLAY_LOOP);
	else if(song == 2)	mmStart(MOD_BGMCENTER4, MM_PLAY_LOOP);
	else if(song == 3)	mmStart(MOD_BGMCENTER5, MM_PLAY_LOOP);
	else if(song == 4)	mmStart(MOD_BGMTWINLEAFTOWN, MM_PLAY_LOOP);
	else if(song == 5)	mmStart(MOD_BGMELM, MM_PLAY_LOOP);
	else if(song == 6)	mmStart(MOD_BGMOAK, MM_PLAY_LOOP);
	else if(song == 7)	mmStart(MOD_BGMGAMECORNER, MM_PLAY_LOOP);
}
