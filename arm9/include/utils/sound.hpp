#ifndef SOUND_HPP
#define SOUND_HPP
#include <maxmod9.h>

namespace Sound {
	extern mm_sound_effect click, back;
	void init(void);
	void play(mm_sound_effect &sound);
	void playBgm(int song);
}


#endif
