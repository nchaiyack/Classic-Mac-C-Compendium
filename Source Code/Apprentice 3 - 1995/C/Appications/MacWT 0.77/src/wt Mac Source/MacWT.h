/*
** File:		MacWT.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/


extern DialogPtr		gWindow;
extern EventRecord		gTheEvent;
extern Boolean			quitting, gPaused, gShowFPS, gGameOn, gTrueColor, gKeyboardControl, gUseQuickdraw;
extern Str255			gWorldFile;
extern Str15			gWTVersion;
extern long				gStartTicks, gFrameCount;
extern short			gScreenDepth;

extern void RefreshWTWindow(void);

extern short GetAndProcessEvent(void);	// returns TRUE if we had an event...
extern void BeginGame(void);			// game begins...
extern void TogglePause(void);			// pause/continue game
extern void ShowIfPaused(void);
extern void MacAttractMode(void);
void GetScreenSize(short *width, short *height);

#define	MacKeyDown(kmp, code)	((kmp[code>>3] >> (code & 7)) & 1)