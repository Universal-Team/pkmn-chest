#include "sound.h"
#include <nds.h>

#include "config.h"
#include "soundbank.h"
#include "soundbank_bin.h"

mm_sound_effect Sound::click, Sound::back;
short bgmCenter = MOD_BGMCENTER, bgmElm = MOD_BGMELM, bgmGameCorner = MOD_BGMGAMECORNER, bgmOak = MOD_BGMOAK;

void Sound::init(void) {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect(SFX_CLICK);
	mmLoadEffect(SFX_BACK);
	mmLoad(MOD_BGMCENTER);
	mmLoad(MOD_BGMELM);
	mmLoad(MOD_BGMGAMECORNER);
	mmLoad(MOD_BGMOAK);

	Sound::click = {
		{SFX_CLICK},			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
	Sound::back = {
		{SFX_BACK},				// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};

	if(Config::music == 1)		mmStart(bgmCenter, MM_PLAY_LOOP);
	else if(Config::music == 2)	mmStart(bgmElm, MM_PLAY_LOOP);
	else if(Config::music == 3)	mmStart(bgmOak, MM_PLAY_LOOP);
	else if(Config::music == 4)	mmStart(bgmGameCorner, MM_PLAY_LOOP);
}

void Sound::play(mm_sound_effect &sound) {
	if(Config::playSfx)	mmEffectEx(&sound);
}

void Sound::playBgm(int song) {
	if(song == 0)		mmStop();
	else if(song == 1)	mmStart(bgmCenter, MM_PLAY_LOOP);
	else if(song == 2)	mmStart(bgmElm, MM_PLAY_LOOP);
	else if(song == 3)	mmStart(bgmOak, MM_PLAY_LOOP);
	else if(song == 4)	mmStart(bgmGameCorner, MM_PLAY_LOOP);
}
