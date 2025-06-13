/*
 * SkelDlogFilter.c - Implement standard dialog filter that processes orphan
 * 		dialog events.  See TransSkel Programmer's Notes 2, 8, and 12.
 *
 * 09 Jun 92
 * - Check whether default button is actually *active* when return/enter
 * is typed.  Sheesh.
 * 11 Jun 92
 * - Added flag argument to SkelDlogFilter() allowing caller to determine
 * whether or not standard filter should do return/enter processing.  Since
 * this is now under control of caller, the test is whether the flag is true,
 * rather than whether there's a dialog-specific filter or not.
 * 04 Nov 93
 * - Made the filter function check whether there's an event hook installed
 * and pass the event to it if so, before dealing with the event.
 * 03 Jan 94
 * - Added functions SkelDlogDefaultItem() and SkelDlogCancelItem() for
 * specifying which dialog items are the default and cancel items.  This
 * allows (i) a different item than aDefItem to be used for the OK button
 * and (ii) keyboard equivalents for the Cancel item to be handled
 * automatically by the standard filter.
 * 04 Jan 94
 * - Added function SkelDlogFilterYD() for installing filters that can be used
 * for calls like the System 7 Standard File dialogs CustomGetFile() and
 * CustomPutFile().
 * - Changed type of SkelDlogFilter() from SkelDlogFilterProcPtr to
 * ModalFilterProcPtr.  (They're equivalent, and compiler now treats them
 * that way, so there's no need for SkelDlogFilterProcPtr anymore.)
 * 07 Jan 94
 * - StandardFilter() now only checks keyDown, not autoKey; if the keypress
 * is one it would respond to, the dialog will end up being dismissed and there
 * is no opportunity for autoKey to occur.
 * 12 Jan 94
 * - Stopped using CallPascalB(); the compiler's smart enough now to figure out
 * when Pascal calling conventions need to be used with (*func)(args) syntax.
 * 13 Jan 94
 * - Formerly when a key was typed that maps to a button, the standard filter
 * checked the button to see if it was active.  If so, it flashed the button and
 * returned the item.  Otherwise it returned false.  This leads to the behavior
 * that the key ends up in the current edittext item if there is one when the
 * button is inactive and not otherwise.  Now the filter flashes the button and
 * returns true if the button is active, but turns the event into a null event
 * if the button's inactive so the key doesn't get processed further.
 * 14 Apr 94
 * - Added stuff for testing whether the pointer is in an edit field and
 * changing the pointer to an I-beam if so.  This is enabled/disabled by
 * calling SkelDlogTracksCursor().
 * - Also remembered to actually initialize the cancelItem field of the
 * FilterInfo struct when a new one is pushed on the stack...
 * 23 Apr 94
 * - Changed stack depth from 10 to 5.  No more than 2 alerts/dialogs should
 * appear at once, anyway, so even 5 is excessive.
 * - Pass mouse click events to regular event router if the click is in the
 * drag region of an underlying window and the command-key is down.  This
 * allows those windows to be dragged around in their current plane under
 * modal dialogs.  (As per Apple's Human Interface Guidelines.)
 * 26 Apr 94
 * - Send null events to SkelRouteEvent(), without claiming to have handled
 * them.  This helps keep window-specific idle procedures running if they're
 * supposed to run even when the window isn't frontmost.
 * 29 Apr 94
 * - Bug fix: TestDlogButton() wasn't checking properly to see whether or not
 * a button was disabled.  Fixed.
 * - Removed TestDlogButton() and key mapping code from StandardFilter() and
 * repackaged it at SkelDlogMapKeyToButton() because the key mapping operation
 * is one that's useful from modeless and movable modal dialog filters, too.
 * - On update or activate, instead of testing whether the window to which the
 * event applies is registered with TransSkel, just check whether it's the modal
 * being handled.  If not, pass the event to the router.  Even if the event's
 * for another dialog, the router will now handle it correctly, whether the
 * window is registered or not.
 * 03 May 94
 * - Introduced a bug in ModalFilterYD() in release 3.13 by putting an extra
 * semicolon at the end of the first "if" line.  Resulted in machine lockups.
 * Fixed.
 * 24 Oct 94
 * - Modified functions that take filter arguments or return filter values to
 * use UPP's when necessary so that they now take or return UPP's.  This
 * provides PowerPC support.  The return value for PowerPC code is actually
 * a routine descriptor.  These are allocated statically.
 * 02 Nov 94
 * - If the cursor was in an exit text item and the user selected an OK or Cancel
 * by hitting a key, the dialog would be dismissed but the cursor would remain
 * as an i-beam.  Compensate by calling InitCursor() in SkelRmveDlogFilter().
 */

# include	"TransSkel.h"


/* Compatibility machinery for non-universal header compilation */

# ifndef CallModalFilterProc
# define CallModalFilterProc(filter,dlog,evt,item) \
				((*(ModalFilterProcPtr) (filter)) (dlog,evt,item))
# endif

# ifndef CallModalFilterYDProc
# define CallModalFilterYDProc(filter,dlog,evt,item,data) \
					((*(ModalFilterYDProcPtr) (filter)) (dlog,evt,item,data))
# endif


/*
 * Filter function stack. Used so that the filter function mechanism
 * can be reentrant (multiple simultaneous alerts and dialogs).  Maximum
 * reentrancy level is controlled by maxStack.
 *
 * The filter field is a generic pointer because functions of different
 * types are assigned to it.
 *
 * The defaultItem and cancelItem fields are positive to indicate an
 * item to which return/enter and cmd-period/escape should be mapped.
 * They are zero to indicate no key mapping.  defaultItem can be
 * negative to indicate that return/enter should be mapped to the
 * aDefItem item in the dialog/alert record.
 *
 * trackCursor is true if the cursor should be mapped to an i-beam whenever
 * it's in an edittext item.
 */

# define	maxStack	5


typedef	struct FilterInfo	FilterInfo;

struct FilterInfo
{
	UniversalProcPtr	filter;			/* filter passed in by caller */
	short				defaultItem;
	short				cancelItem;
	Boolean				trackCursor;
};


static short	top = -1;
static FilterInfo filterStack[maxStack];


/*
 * Standard filter function.  The regular alert- or dialog-specific filter is
 * piggybacked onto this.  The standard filter intercepts events that would
 * be ignored otherwise (currently this includes OS events, command clicks
 * in drag region of underlying windows, and update and activate/deactivate
 * events for windows other than the given dialog) and shoves them through
 * the normal TransSkel event dispatch mechanism.
 *
 * The standard filter also performs key-to-button mapping, returning the
 * default button when return/enter is typed, and/or the cancel button
 * when cmd-period/escape is typed.
 *
 * Returns false even for an event that's handled, if it doesn't result in
 * an item hit, but map the event to a null event so the Dialog Manager doesn't
 * try to do anything further with it.
 */

static pascal Boolean 
StandardFilter (DialogPtr d, EventRecord *e, short *item)
{
FilterInfo	*info;
short	what = e->what;
char	c;
SkelEventHookProcPtr	p;
GrafPtr	port;

	if ((p = SkelGetEventHook ()) != (SkelEventHookProcPtr) nil)
	{
		if ((*p) (e))	/* call hook, and return if hook handles event */
			return (true);
	}

	info = &filterStack[top];

	/*
	 * If the event is a null event and the filter should make the cursor an
	 * I-beam when the pointer's in an edit text field, check that.
	 */

	if (what == nullEvent)
	{
		if (info->trackCursor)
			SkelSetDlogCursor (d);
		SkelRouteEvent (e);
		return (false);
	}

	if (what == osEvt)				/* MultiFinder event */
	{
		SkelRouteEvent (e);
		e->what = nullEvent;
		return (false);
	}

	/*
	 * Command clicks in the drag region of underlying windows are processed,
	 * allowing those windows to be dragged around in their current plane under
	 * the dialog.  This doesn't test whether the port is the same as the dialog
	 * because modals don't have a drag region.
	 */
	if (what == mouseDown && (e->modifiers & cmdKey)
			&& FindWindow (e->where, &port) == inDrag)
	{
		SkelRouteEvent (e);
		e->what = nullEvent;
		return (false);
	}

	if (what == updateEvt || what == activateEvt)
	{
		if (d != (DialogPtr) e->message)
		{
			SkelRouteEvent (e);
			e->what = nullEvent;
			return (false);
		}
	}

	/*
	 * If event is a key-press, handle key-to-button mapping if necessay.
	 */

	if (what == keyDown)
	{
		if (SkelDlogMapKeyToButton (d, e, item, info->defaultItem, info->cancelItem))
			return (true);
	}
	return (false);
}


/*
 * Run ModalDialog()-style filter.
 *
 * If dialog-specific filter doesn't handle the event, pass it to
 * StandardFilter().
 */

static pascal Boolean 
ModalFilter (DialogPtr d, EventRecord *e, short *item)
{
FilterInfo	*info;

	info = &filterStack[top];
	if (info->filter != nil)
	{
		/*
		 * It's necessary to cast the first argument because for non-PPC compiles the
		 * universal headers make CallModalFilterProc() a #define similar to the one
		 * shown near the top of this file, EXCEPT that they don't properly cast the
		 * first argument!
		 */
		if (CallModalFilterProc ((ModalFilterProcPtr) info->filter, d, e, item))
			return (true);
	}
	if (StandardFilter (d, e, item))
		return (true);
	return (false);
}


/*
 * Run ModalDialog()-style filter that takes a data argument.
 *
 * If dialog-specific filter doesn't handle the event, pass it to
 * StandardFilter().
 */
 
static pascal Boolean 
ModalFilterYD (DialogPtr d, EventRecord *e, short *item, void *data)
{
FilterInfo	*info;

	info = &filterStack[top];
	if (info->filter != nil)
	{
		/*
		 * See comment in ModalFilter() above about casting.
		 */
		if (CallModalFilterYDProc ((ModalFilterYDProcPtr) info->filter, d, e, item, data))
			return (true);
	}
	if (StandardFilter (d, e, item))
		return (true);
	return (false);
}


/*
 * Install a ModalDialog()-style filter, or a filter that takes a data
 * argument.
 *
 * In each case, remember the filter specified by the caller, so it can be
 * invoked later.  Return a pointer to a function that invokes the filter
 * specified by the caller, and the standard filter if necessary.  The caller
 * passes the returned filter to an alert or dialog call that takes a filter
 * function argument.
 *
 * doReturn is true if the standard filter should try to handle return/enter
 * key as a synonym for clicking the default button if that button is active.
 */

static void
InstallNewFilter (UniversalProcPtr filter, Boolean doReturn)
{
FilterInfo	*info;

	info = &filterStack[++top];
	info->filter = filter; 
	info->defaultItem = (doReturn ? -1 : 0);
	info->cancelItem = 0;
	info->trackCursor = false;
}


/*
 * For 68K code we can use function pointers directly, but for PowerPC code we
 * need routine descriptors.
 */

# if skelPPC		/* PowerPC code */
static RoutineDescriptor modalDesc =
				BUILD_ROUTINE_DESCRIPTOR (uppModalFilterProcInfo, ModalFilter);
static RoutineDescriptor modalYDDesc =
				BUILD_ROUTINE_DESCRIPTOR (uppModalFilterYDProcInfo, ModalFilterYD);
# endif


pascal ModalFilterUPP
SkelDlogFilter (ModalFilterUPP filter, Boolean doReturn)
{
	InstallNewFilter ((UniversalProcPtr) filter, doReturn);
# if skelPPC		/* PowerPC code */
	return (&modalDesc);
# else				/* 68K code */
	return (ModalFilter);
# endif
}


pascal ModalFilterYDUPP
SkelDlogFilterYD (ModalFilterYDUPP filter, Boolean doReturn)
{
	InstallNewFilter ((UniversalProcPtr) filter, doReturn);
# if skelPPC		/* PowerPC code */
	return (&modalYDDesc);
# else				/* 68K code */
	return (ModalFilterYD);
# endif
}


/*
 * Remove the last filter function (which reinstalls the previous one
 * as a side effect).  Reinitialize the cursor in case it was being
 * tracked as an i-beam in an edittext item and the user dismissed the
 * dialog by typing a key rather than by moving the mouse out of the
 * text item and clicking a button.
 */

pascal void
SkelRmveDlogFilter (void)
{
	if (top >= 0)
	{
		InitCursor ();
		--top;
	}
}


/*
 * Set default or cancel item for the current filter
 */

pascal void
SkelDlogDefaultItem (short item)
{
	if (top >= 0)
		filterStack[top].defaultItem = item;
}


pascal void
SkelDlogCancelItem (short item)
{
	if (top >= 0)
		filterStack[top].cancelItem = item;
}


/*
 * Specify whether or not to track the cursor in edittext items
 */

pascal void
SkelDlogTracksCursor (Boolean track)
{
	if (top >= 0)
		filterStack[top].trackCursor = track;
}
