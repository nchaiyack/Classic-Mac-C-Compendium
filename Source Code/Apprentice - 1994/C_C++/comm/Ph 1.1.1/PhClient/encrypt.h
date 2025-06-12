/*______________________________________________________________________

	encrypt.h - Encryption Interface.
_____________________________________________________________________*/


#ifndef __encrypt__
#define __encrypt__

extern void crypt_start (char *pw);
extern short encryptit (char *to, char *from);

#endif