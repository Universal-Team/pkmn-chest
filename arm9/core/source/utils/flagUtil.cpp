#include "flagUtil.hpp"

bool FlagUtil::getFlag(u8 *data, int offset, int bitIndex)
{
	bitIndex &= 7; // ensure bit access is 0-7
	return (data[offset] >> bitIndex & 1) != 0;
}

void FlagUtil::setFlag(u8 *data, int offset, int bitIndex, bool v)
{
	bitIndex &= 7; // ensure bit access is 0-7
	data[offset] &= (u8)~(1 << bitIndex);
	data[offset] |= (u8)((v ? 1 : 0) << bitIndex);
}
