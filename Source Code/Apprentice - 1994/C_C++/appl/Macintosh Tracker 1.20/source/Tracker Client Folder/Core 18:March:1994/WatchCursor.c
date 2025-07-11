/* WatchCursor.c */

#define COMPILING_WATCHCURSOR_C
#include "WatchCursor.h"
#include "EventLoop.h"


#define FirstCursID (128)

static MyBoolean	CursorsLoaded = False;

static short			CursorPhase = 0;

static Cursor*		LastCursor;

Cursor						WatchCursorTable[NumWatchCursors];

ushort						WatchCursorIndex;

ulong							LastWatchCursorInc;


void		StartTimeConsumingOperation(void)
	{
		short			Scan;
		Cursor**	Temp;

		if (!CursorsLoaded)
			{
				CursorsLoaded = True;
				for (Scan = 0; Scan < NumWatchCursors; Scan += 1)
					{
						ERROR(ResLoad == 0,PRERR(ForceAbort,
							"Automatic resource loading is disabled."));
						Temp = (Cursor**)GetResource('CURS',Scan + FirstCursID);
						if (Temp == NIL)
							{
								PRERR(ForceAbort,"Unable to load cursor resource.");
							}
						ERROR(ResErr != noErr,PRERR(ForceAbort,"Resource Error occurred."));
						WatchCursorTable[Scan] = **Temp;
					}
			}
		if (CursorPhase == 0)
			{
				WatchCursorIndex = 0;
				LastWatchCursorInc = TickCount();
			}
		CursorPhase -= 1;
	}


void		EndTimeConsumingOperation(void)
	{
		if (CursorPhase == 0)
			{
				return;
			}
//	if (CursorPhase == -1)
//		{
//		}
		CursorPhase += 1;
	}
