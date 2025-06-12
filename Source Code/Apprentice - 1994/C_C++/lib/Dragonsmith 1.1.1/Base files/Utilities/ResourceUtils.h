/*
	ResourceUtils.h
*/

#pragma	once

#ifndef kInvalidRefNum
#define	kInvalidRefNum	-1
#endif

OSErr SaveRes (Handle h);
OSErr CopyResHandle (Handle h, short destRF, Boolean overwrite);
