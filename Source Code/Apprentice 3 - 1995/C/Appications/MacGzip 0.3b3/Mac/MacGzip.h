#pragma once

/*****************************************************/
/* MacGzip Signature */

#define GZIP_ID		'Gzip'

/*****************************************************/
/* SND resources */

#define SOUND_ID		128

/*****************************************************/
/* PICT resources */

#define ABOUT			132
#define GPL			130

/*****************************************************/
/* key codes */

#define IsOptKey(a)		a[1] & 0x00000004
#define IsAKey(a)		a[0] & 0x01000000
#define IsBKey(a)		a[0] & 0x00080000

/*****************************************************/
/* resource IDs of menus */
#define appleID			1
#define fileID			2
#define editID			3
#define gzipID			4

#define appleM			0
#define fileM			1
#define editM			2
#define gzipM			3

#define fmOpen			1
#define fmPrefs		2
#define fmQuit			4

#define emUndo 		1
#define emCut			3
#define emCopy		4
#define emPaste		5
#define emClear		6

#define gmAscii 		1
#define gmBin			2
#define gmComp		4
#define gmUncomp		5

#define dlogAbout		128







