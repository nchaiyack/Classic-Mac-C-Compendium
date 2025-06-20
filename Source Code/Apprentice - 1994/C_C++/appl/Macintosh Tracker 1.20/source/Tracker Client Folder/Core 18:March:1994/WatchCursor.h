/* WatchCursor.h */

#pragma once

#define CursorStepDelay (60)
#define NumWatchCursors (8)

#ifndef COMPILING_WATCHCURSOR_C
	extern Cursor		WatchCursorTable[NumWatchCursors];
	extern ushort		WatchCursorIndex;
	extern ulong		LastWatchCursorInc;
#endif

void		StartTimeConsumingOperation(void);
void		EndTimeConsumingOperation(void);
#define CheckCursor(crap) if (TickCount()-LastWatchCursorInc > CursorStepDelay)\
	{\
		SetCursor(&(WatchCursorTable[WatchCursorIndex]));\
		WatchCursorIndex = WatchCursorIndex + 1;\
		if (WatchCursorIndex >= NumWatchCursors)\
			{\
				WatchCursorIndex = 0;\
			}\
		LastWatchCursorInc = TickCount();\
	}
