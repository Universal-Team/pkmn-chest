#ifndef _MEMECRYPTO_H_
#define _MEMECRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "aes.h"
#include "rsa.h"
#include "sha1.h"

#define MEME_LEN 0x60

void memecrypto_aes_encrypt(unsigned char *buf, unsigned char *output, unsigned char *key);
void memecrypto_aes_decrypt(unsigned char *buf, unsigned char *output, unsigned char *key);

int memecrypto_sign(unsigned char *input, unsigned char *output, int len);
int memecrypto_verify(unsigned char *input, unsigned char *output, int len);

void reverseCrypt(unsigned char *input, unsigned char *output);

#ifdef __cplusplus
}
#endif

#endif //_MEMECRYPTO_H_
