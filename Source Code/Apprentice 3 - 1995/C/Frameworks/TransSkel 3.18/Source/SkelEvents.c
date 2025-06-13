/*
 * SkelDoEvents (mask) - process all pending events of types indicated in mask
 * SkelDoUpdates () - process all pending update events
 *
 * These routines may be called any time subsequent to the call of SkelInit().
 */

# include	"TransSkel.h"


static short	sdeMask;


/*
 * Make sure any events of proper type are processed before
 * proceeding.  I.e., wait until there are no more events of the
 * type we're waiting for, then terminate SkelDoEvents().
 */

static pascal void
CheckEvents (void)
{
EventRecord	event;

	if (!EventAvail (sdeMask, &event))
		SkelStopEventLoop ();
}


/*
 * Process all events of type(s) given in mask.  It is possible to call this
 * recursively.
 * Operation:
 * - Save current SkelDoEvents() mask, current TransSkel event mask, and
 * current background procedure.
 * - Install the new mask into TransSkel and save a copy in a local variable.
 * Install a new background procedure that checks whether any events of the
 * desired type(s) are available or not.
 * - Call SkelMain() to initiate an event loop.  The background task calls
 * SkelWhoa() to terminate SkelMain() when there are no more events of
 * interest available.
 * - Restore the previous background procedure and TransSkel mask, and
 * previous SkelDoEvents() mask.  The latter is necessary in case this is
 * a recursive call.
 */

pascal void
SkelDoEvents (short mask)			/* can be called recursively */
{
short	oldSdeMask;
short	oldTSMask;
SkelIdleProcPtr	oldIdle;

	oldIdle = SkelGetIdle ();			/* get current idle proc */
	oldTSMask = SkelGetEventMask ();	/* and event mask */
	oldSdeMask = sdeMask;				/* and SkelDoEvents() processing types */

	SkelSetIdle (CheckEvents);			/* install new idle & mask */
	SkelSetEventMask (mask);
	sdeMask = mask;						/* <- so CheckEvents can find mask */

	SkelEventLoop ();					/* handle given event types only */

	SkelSetIdle (oldIdle);				/* restore stuff that was changed */
	SkelSetEventMask (oldTSMask);
	sdeMask = oldSdeMask;
}


pascal void
SkelDoUpdates (void)
{
	SkelDoEvents (updateMask);
}
