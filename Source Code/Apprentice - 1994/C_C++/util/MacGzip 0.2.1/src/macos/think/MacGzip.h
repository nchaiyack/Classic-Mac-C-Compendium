
#pragma once
/*****************************************************/
/* STR resources */

#define FILE_MANAGER	128
#define GETFINFOERROR	1
#define CREATEERROR	2
#define FSDELETEERROR	3
#define RENAMEERROR	4
#define PBGETFINFO		5

#define FILE_IO			129
#define W_ERROR		1
#define R_ERROR		2

#define GENERIC		130
#define NO_CURSOR		1
#define EMPTY_FILE		2
#define NOT_TEXT		3
#define RES_FORK		4
#define NO_SAVE_PREF	5
#define EMPTY_SUFFIX	6
#define OLD_SYSTEM	7
#define BAD_DISK		8


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

void SetUpMenus(void);

void MainEvent(void);

void DoCommand(long mResult);
void DoFile(short item);
void DoEdit(short item);


void DoMouseDown(short windowPart, WindowPtr whichWindow, EventRecord *myEvent);
void DoContent(WindowPtr whichWindow, EventRecord *myEvent);
