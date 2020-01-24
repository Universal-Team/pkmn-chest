#ifndef SOUND_HPP
#define SOUND_HPP
#include <maxmod9.h>

namespace Sound {
	extern mm_sound_effect click, back;
	void load(const char *path);
	void play(mm_sound_effect &sound);
}


#endif
