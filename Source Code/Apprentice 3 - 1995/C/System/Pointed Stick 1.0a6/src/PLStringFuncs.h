/*
	PLStringFuncs.h -- C string conversion functions for pascal
		
	Copyright Apple Computer,Inc.  1989, 1990
	All rights reserved

*/

#ifndef __PLSTRINGFUNCS__
#define __PLSTRINGFUNCS__

#ifndef	__TYPES__
#include <Types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

pascal short		PLstrcmp(StringPtr str1, StringPtr str2);
pascal short		PLstrcompare(StringPtr str1, StringPtr str2);
pascal short 		PLstrncmp(StringPtr str1, StringPtr str2, short num);
pascal StringPtr 	PLstrcpy(StringPtr str1, StringPtr str2);
pascal StringPtr 	PLstrncpy(StringPtr str1, StringPtr str2, short num);
pascal StringPtr	PLstrcat(StringPtr str1, StringPtr str2);
pascal StringPtr 	PLstrncat(StringPtr str1, StringPtr str2, short num);
pascal Ptr 			PLstrchr(StringPtr str1, short ch1);
pascal Ptr 			PLstrrchr(StringPtr str1, short ch1);
pascal Ptr 			PLstrpbrk(StringPtr str1, StringPtr str2);
pascal short 		PLstrspn(StringPtr str1, StringPtr str2);
pascal Ptr 			PLstrstr(StringPtr str1, StringPtr str2);
pascal short 		PLstrlen(StringPtr str);
pascal short		PLpos(StringPtr str1, StringPtr str2);



#if 0
// Rad new version of PLstrcpy by Jim Reekes
#pragma parameter PStringCopy(__A1,__A0)
pascal void PStringCopy(Str255 dest, Str255 source)
    = {	0x7000,					// moveq	#$00,D0
    	0x1010,					// move.b	(A0),D0
    	0x12D8,					// move.b	(A0)+,(A1)+
    	0x51C8, 0xFFFC};		// dbf		D0,*-2
#else
#define PStringCopy PLstrcpy
#endif

#pragma parameter SmallBlockMove(__A0, __A1, __D0)
pascal void SmallBlockMove(Ptr source, Ptr dest, short length)
    = {	0x12D8,					// move.b	(A0)+,(A1)+
    	0x51C8, 0xFFFC};		// dbf		D0,*-2

#ifdef __cplusplus
}
#endif

#endif
