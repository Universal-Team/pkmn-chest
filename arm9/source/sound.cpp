#include "sound.h"
#include <nds.h>

#include "config.h"
#include "soundbank.h"
#include "soundbank_bin.h"

mm_sound_effect Sound::click, Sound::back;
short bgmCenter1 = MOD_BGMCENTER1, bgmCenter4 = MOD_BGMCENTER4, bgmTwinleafTown = MOD_BGMTWINLEAFTOWN, bgmElm = MOD_BGMELM, bgmGameCorner = MOD_BGMGAMECORNER, bgmOak = MOD_BGMOAK;

void Sound::init(void) {
	mmInitDefaultMem((mm_addr)soundbank_bin);
	
	mmLoadEffect(SFX_CLICK);
	mmLoadEffect(SFX_BACK);
	mmLoad(MOD_BGMCENTER1);
	mmLoad(MOD_BGMCENTER4);
	mmLoad(MOD_BGMTWINLEAFTOWN);
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

	if(Config::music == 1)		mmStart(bgmCenter1, MM_PLAY_LOOP);
	else if(Config::music == 2)	mmStart(bgmCenter4, MM_PLAY_LOOP);
	else if(Config::music == 3)	mmStart(bgmTwinleafTown, MM_PLAY_LOOP);
	else if(Config::music == 4)	mmStart(bgmElm, MM_PLAY_LOOP);
	else if(Config::music == 5)	mmStart(bgmOak, MM_PLAY_LOOP);
	else if(Config::music == 6)	mmStart(bgmGameCorner, MM_PLAY_LOOP);
}

void Sound::play(mm_sound_effect &sound) {
	if(Config::playSfx)	mmEffectEx(&sound);
}

void Sound::playBgm(int song) {
	if(song == 0)		mmStop();
	else if(song == 1)	mmStart(bgmCenter1, MM_PLAY_LOOP);
	else if(song == 2)	mmStart(bgmCenter4, MM_PLAY_LOOP);
	else if(song == 3)	mmStart(bgmTwinleafTown, MM_PLAY_LOOP);
	else if(song == 4)	mmStart(bgmElm, MM_PLAY_LOOP);
	else if(song == 5)	mmStart(bgmOak, MM_PLAY_LOOP);
	else if(song == 6)	mmStart(bgmGameCorner, MM_PLAY_LOOP);
}
