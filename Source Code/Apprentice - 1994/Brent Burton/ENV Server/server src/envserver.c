/*
** envserver.c
**
** main source file for the environment server.
**
*/


#include "es_headers"

#include "aevent.h"
#include "envserver.h"
#include "utils.h"
#include "logging.h"

static void Check4Sys7(void);
static void DoEvent(EventRecord *);
void Cleanup(void);

Boolean gDone = FALSE;      /* are we done yet? */

void main()
{
	int				haveEvent;					/* do we have an event */
	EventRecord		theEvent;					/* the current event   */
	long			sleepTime = 20000;			/* time to sleep/WNE   */
	int				eventMask = everyEvent;		/* current event mask  */

#if LOGGING
	LogInit("Server.log");
#endif
	Check4Sys7();	/* this exits if Sys 7 isn't running */
	InitAEs();		/* initialize my Apple Event handlers */
	
	while ( !gDone) {
		haveEvent = WaitNextEvent( eventMask, &theEvent, sleepTime, NULL);
		if (haveEvent)
			DoEvent(&theEvent);
	} /* while */
	Cleanup();
} /* main */


static void Check4Sys7(void)
{
#define _Gestalt 0xa1ad   /* Gestalt trap number (from IM) */
	if (TrapAvailable(_Gestalt)) {
		long result, mask=0x0700; /* mask = system 7.x */

		if ( (Gestalt( gestaltSystemVersion, &result) == noErr) &&
			( mask == (result & mask)) )
			return;
	}
	/* Let user know we need sys 7. */
	/* if Gestalt isn't on the system, don't even check the others */
	/* Since this is a background-only application, beep 3 times. ;-) */

	SysBeep(1);  SysBeep(1);  SysBeep(1);
	Cleanup();

} /* Check4Sys7 */


/********************
** Cleanup
**
** This routine is for removing any allocated structures, closing drivers,
** etc.  Anything that *can* be cleaned up or shut down, do it here.
** Then, ExitToShell().
*********************/
void Cleanup(void)
{
#if LOGGING
	LogClose();
#endif
	ExitToShell();
} /* Cleanup */


static void DoEvent(EventRecord *theEvent)
{
	switch (theEvent->what)
	{
		case kHighLevelEvent:			/* ignore all other events */
			DoAppleEvent(theEvent);
			break;

		case nullEvent: 				/* perhaps handle null events */
			break;
	}
} /* DoEvent */





