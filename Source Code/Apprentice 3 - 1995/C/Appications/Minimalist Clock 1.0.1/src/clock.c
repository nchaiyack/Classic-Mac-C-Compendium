/* ----------------------------------------------------------------------
clock.c
---------------------------------------------------------------------- */

#include	"the_defines.h"
#include	"the_globals.h"
#include	"the_prototypes.h"


/* ----------------------------------------------------------------------
CreateClock
---------------------------------------------------------------------- */
void CreateClock()
{
	WindowRecord	*clockWinRec;
	Rect			myWinRect;
	Str255 			myTitle;

	myWinRect = qd.screenBits.bounds;
	myWinRect.left = myWinRect.right - (CLOCK_WIDTH + 20);
	myWinRect.right = myWinRect.left + CLOCK_WIDTH;
	myWinRect.top = myWinRect.top + 40;
	myWinRect.bottom = myWinRect.top - 1;
	
	clockWinRec = NIL;
	gClock = NewWindow(clockWinRec,&myWinRect,"\p ",FALSE,4,(WindowPtr)-1L,FALSE,0L);
	
	UpdateClock();
	ShowWindow(gClock);
}

/* ----------------------------------------------------------------------
UpdateClock
---------------------------------------------------------------------- */
void UpdateClock()
{
	unsigned long	rawTime;
	DateTimeRec		theDate;
	Str255			theNewTime,theOldTime;
	Handle			iOh = GetIntlResource(0);
	GrafPtr			savedPort;

	GetDateTime(&rawTime);
	if (gDrawDate)
	{
		IUDatePString(rawTime,0,theNewTime,(Handle)iOh);
		gTicksOld = TickCount();
	}
	else
		IUTimePString(rawTime,FALSE,theNewTime,(Handle)iOh);

	GetPort(&savedPort);
	SetPort(gClock);
	GetWTitle(gClock,theOldTime);
	if (!EqualString(theOldTime,theNewTime,FALSE,FALSE) || gDrawDate)
		SetWTitle(gClock,theNewTime);
	SetPort(savedPort);
	gDrawDate = gDrawDate && FALSE;
}
