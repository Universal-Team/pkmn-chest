#ifndef IMGCPY_H
#define IMGCPY_H


#ifdef __cplusplus
extern "C" {
#endif

#include <nds.h>

typedef unsigned int uint;

// Modified tonccpy for copying images with transparency
void imgcpy(void *dst, const void *src, const u16 *pal, uint size, uint palOfs);

#ifdef __cplusplus
}
#endif
#endif
