#ifndef FLAG_UTIL_HPP
#define FLAG_UTIL_HPP

#include "coretypes.h"

namespace FlagUtil {
	// Gets a bitflag from a byte array
	bool getFlag(u8 *data, int offset, int bitIndex);

	// Sets a bitflag in a byte array
	void setFlag(u8 *data, int offset, int bitIndex, bool v);
}

#endif
