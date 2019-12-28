#include "imgcpy.h"

void imgcpy(void *dst, const void *src, const u16 *pal, uint size, uint palOfs) {
	if(size==0 || dst==NULL || src==NULL)	return;

	uint count;
	u16 *dst16;	// hword destination
	u8 *src8;	// byte source

	uint dstOfs = (u32)dst&1;
	src8= (u8*)src;

	dst16= (u16*)(dst-dstOfs);

	// Head: 1 byte.
	if(dstOfs != 0) {
		if(pal[(*src8)+palOfs] != 0) {
			*dst16 = (*dst16 & 0xFF) | ((*src8)+palOfs)<<8;
		}
		src8++;
		dst16++;
		if(--size==0)	return;
	}

	// Unaligned main: copy by 2x byte.
	count = size/2;
	while(count--) {
		if(pal[src8[0]+palOfs] != 0) {
			*dst16 = (src8[0]+palOfs) | (*dst16 & 0xff<<8);
		}
		if(pal[src8[1]+palOfs] != 0) {
			*dst16 = (*dst16 & 0xFF) | (src8[1]+palOfs)<<8;
		}
		dst16++;
		src8 += 2;
	}

	// Tail: 1 byte.
	if(size&1) {
		if(pal[(*src8)+palOfs]) {
			*dst16 = (*dst16 &~ 0xFF) | ((*src8)+palOfs);
		}
	}
}