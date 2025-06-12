/*_____________________________________________________________________

  	oop.c - Reusable Object-Oriented Window and Command Management Module.
	
	John Norstad
	Academic Computing and Network Services
	Northwestern University
	j-norstad@nwu.edu
	
	Copyright © Northwestern University, 1991.
	
	This module defines a superclass for Mac windows. It defines methods 
	for handling the standard window events (periodic, click, key, update, 
	activate, deactivate, grow, zoom, close, and command).
	
	The class supports normal document windows and both modeless and 
	movable modal dialog windows.
	
	In most cases the methods are defined to do nothing for document 
	windows, and to do the usual processing for dialog windows (the equivalent
	of the "DialogSelect" toolbox routine). For document windows you will want 
	to override most if not all of these methods. For dialog windows the 
	superclass methods will often suffice.
	
	Commands are handled by first dispatching a DoCommand message to
	the top window object. If the top window object cannot handle the
	command, a list of registered command handlers is called. A function
	is exported which permits callers to register command handlers.
	
	The module also exports an event handling function which takes
	care of dispatching events by sending messages to the appropriate
	window objects. The typical main event loop consists of repeated
	calls to this function.
	
	The module's initialization function initializes itself and also takes
	care of the usual memory and toolbox initialization tasks.
	
	Message dispatching is accomplished via dispatch tables, which are
	structs of type "oop_Dispatch" containing pointers to method functions.  
	Each subclass has its own method dispatch table.
	
	For example, if "dispatch" is a pointer to a dispatch table, and "w" 
	is a pointer to a window record, then
	
		(*dispatch->DoUpdate)(w)
		
	sends a "DoUpdate" message to the window "w".
	
	Each subclass must provide a dispatch table for that subclass. 
	Method overriding is accomplished by storing pointers to subclass 
	methods in the dispatch table. The superclass methods can be used by 
	storing pointers to the superclass methods in the dispatch table.
	
	For example, in module "mywind.c", you might declare a dispatch
	table as follows:
	
		static oop_Dispatch	dispatch = {
										oop_DoPeriodic,
										mywind_DoClick,
										mywind_DoKey,
										mywind_DoUpdate,
										oop_DoActivate,
										oop_DoDeactivate,
										mywind_DoGrow,
										mywind_DoZoom,
										mywind_DoClose,
										mywind_DoCommand
									};

	In this example the "mywind" subclass has overridden all but the
	DoPeriodic, DoActivate, and DoDeactivate methods.
	
	Inherited methods can be called from within a subclass method by
	simply calling the appropriate function.  For example, in the 
	"mywind" class, the mywind_DoClose method can (and usually does)
	call the oop_DoClose method.
	
	Instance variables for a window object are stored in a separate struct of
	type "WindObj". A handle to this struct is stored in the window record 
	"refCon" field.  In addition to the instance variables, this struct also
	contains a pointer to the method dispatch table for the subclass to
	which the window belongs, plus a handle to subclass instance 
	variables for the window object.
	
	Note the difference between the instance variable structs and the
	dispatch table structs: Each individual window has its own instance
	variables, while there is only a single dispatch table per window 
	subclass. Also, the dispatch table struct definition "oop_Dispatch" 
	is exported from the oop.c module, while the instance variable struct
	definition "WindObj" is private.
	
	Here's a picture:
	
	WindowRecord      WindObj            Type defined by subclass
   ------------    -----------         ----------
   | 	        |    |Instance |         |Subclass|
	|   refCon |--->|Vars     |    |--->|Instance|
	|          |    |         |    |    |Vars    |
	------------    |    info |-----    ----------
	                |dispatch |-----
						 -----------    |    oop_Dispatch
                                  |    ------------
                                  |    |          |---> method
                                  |--->|          |---> method
													|          |---> method
													------------

____________________________________________________________________*/

/*_____________________________________________________________________

	Header Files.
_____________________________________________________________________*/

#pragma load "precompile"
#include "utl.h"
#include "oop.h"

/*_____________________________________________________________________

	Defines.
_____________________________________________________________________*/

#define cutCmd			3		/* item number of edit menu cut command */
#define copyCmd		4		/* item number of edit menu copy command */
#define pasteCmd		5		/* item number of edit menu paste command */
#define clearCmd		6		/* item number of edit menu clear command */

#define dragSlop		4		/* window drag slop in pixels */

/*_____________________________________________________________________

	Types.
_____________________________________________________________________*/
	
typedef struct WindOjb {

	/* Class instance variables. */

	short				kind;				/* kind of window */
	Boolean			dialog;			/* true if dialog */
	Boolean			modal;			/* true if movable modal dialog */
	Boolean			moved;			/* true if moved or size changed */
	RgnHandle		ibeamRgn;		/* ibeam region */
	short				defaultBtn;		/* item number of default button */
	short				cancelBtn;		/* item number of cancel button */
	short				itemHit;			/* item number of item hit */
	
	/* Other information. */
	
	Handle			info;				/* handle to subclass instance vars */
	oop_Dispatch	*dispatch;		/* pointer to method dispatch table */	
	
} WindObj;

/*______________________________________________________________________

	Variables.
_____________________________________________________________________*/

EventRecord			Event;					/* the current event */

static FCommand	(**CmdHandlers)[];	/* handle to array of command handlers */
static short		numCmdHandlers = 0;	/* number of command handlers */

static short		AppleMenuID;			/* apple menu id */
static short		LastAppleMenuItem;	/* item number of last apple menu item */
static short		FileMenuID;				/* file menu id */
static short		CloseMenuItem;			/* item number of close command */
static short		EditMenuID;				/* edit menu id */
static short		SelectAllItem;			/* item number of select all command,
															or 0 if none. */
static FPeriodic	DoPeriodic;				/* ptr to global DoPeriodic function */
static FDisk		DoDisk;					/* ptr to global DoDisk function */
static Rect			DragRect;				/* window drag rectangle */
static CursHandle	IBeamHandle;			/* handle to ibeam cursor */
static Boolean		InForeground = true;	/* true if in foreground */

/*_____________________________________________________________________

	GetWindObj - Get Window Object.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = handle to window object.
_____________________________________________________________________*/

static WindObj **GetWindObj (WindowPtr w)

{
	return (WindObj**)((WindowPeek)w)->refCon;
}

/*_____________________________________________________________________

	OutlineDefaultButton - Outline Default Button.

	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of outline useritem.
				
	This routine is for use as a useritem handler to outline default
	buttons in dialogs.
_____________________________________________________________________*/

pascal void OutlineDefaultButton (WindowPtr theWindow, short itemNo)

{
	short				itemType;		/* item type */
	Handle			item;				/* handle to item */
	Rect				box;				/* item rectangle */

	GetDItem(theWindow, itemNo, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&box, 16, 16);
	PenSize(1, 1);
}	

/*_____________________________________________________________________

	oop_GetWindKind - Get Window Kind.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = window kind.
_____________________________________________________________________*/

short oop_GetWindKind (WindowPtr w)

{
	if (!w) {
		return noWind;
	} else if (((WindowPeek)w)->windowKind < 0) {
		return daWind;
	} else {
		return (**(WindObj**)((WindowPeek)w)->refCon).kind;
	}
}

/*_____________________________________________________________________

	oop_GetWindInfo - Get Window Info.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = handle to window subclass instance variables.
_____________________________________________________________________*/

Handle oop_GetWindInfo (WindowPtr w)

{
	return (**(WindObj**)((WindowPeek)w)->refCon).info;
}

/*_____________________________________________________________________

	oop_GetWindItemHit - Get Window ItemHit Instance Variable.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = itemHit instance variable.
_____________________________________________________________________*/

short oop_GetWindItemHit (WindowPtr w)

{
	return (**(WindObj**)((WindowPeek)w)->refCon).itemHit;
}

/*_____________________________________________________________________

	oop_ClearWindItemHit - Clear Window ItemHit Instance Variable.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void oop_ClearWindItemHit (WindowPtr w)

{
	(**(WindObj**)((WindowPeek)w)->refCon).itemHit = 0;
}

/*_____________________________________________________________________

	oop_GetWindIbeamRgn - Get Window IbeamRgn Instance Variable.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = ibeamRgn instance varialbe.
_____________________________________________________________________*/

RgnHandle oop_GetWindIbeamRgn (WindowPtr w)

{
	return (**(WindObj**)((WindowPeek)w)->refCon).ibeamRgn;
}

/*_____________________________________________________________________

	oop_Moved - Find Out if Window Has Moved.
	
	Entry:	w = pointer to window record.
	
	Exit:		function result = true if window has moved or changed size.
_____________________________________________________________________*/

Boolean oop_Moved (WindowPtr w)

{
	return (**(WindObj**)((WindowPeek)w)->refCon).moved;
}

/*_____________________________________________________________________

	oop_InForeground - Determine Whether We Are in the Foreground.
_____________________________________________________________________*/

Boolean oop_InForeground (void)

{
	return InForeground;
}

/*_____________________________________________________________________

	Init - Complete Initialization.
_____________________________________________________________________*/

#pragma segment init

static void Init (void)

{
	Rect						screenRect;	/* screen rectangle */
	ProcessSerialNumber	myPsn;		/* my psn */
	ProcessSerialNumber	frontPsn;	/* front process psn */
	EventRecord				dummy;		/* dummy event record */
	
	/* Initialize the toolbox. */
	
	InitGraf(&qd.thePort);				/* initialize quickdraw */
	InitFonts();							/* initialize font manager */
	FlushEvents(everyEvent, 0);		/* drain event queue */
	InitWindows();							/* initialize window manager */
	InitMenus();							/* initialize menu manager */
	TEInit();								/* initialize textedit */
	InitDialogs(nil);						/* initialize dialog manager */
	InitCursor();							/* initialize cursor */
	
	/* Initialize the command handler array. */
	
	CmdHandlers = (FCommand(**)[])NewHandle(0);
	
	/* Initialize the drag rect. */
	
	screenRect = qd.screenBits.bounds;
	SetRect(&DragRect, dragSlop, titleBarHeight + dragSlop, 
		screenRect.right - dragSlop, screenRect.bottom - dragSlop);
		
	/* Get the ibeam cursor. */
		
	IBeamHandle = GetCursor(iBeamCursor);
	
	/* Figure out whether we're in the foreground or background. */
	
	if (utl_GestaltFlag(gestaltOSAttr, gestaltLaunchCanReturn) &&
		utl_GestaltFlag(gestaltOSAttr, gestaltLaunchFullFileSpec) &&
		utl_GestaltFlag(gestaltOSAttr, gestaltLaunchControl)) {
		GetCurrentProcess(&myPsn);
		GetFrontProcess(&frontPsn);
		SameProcess(&myPsn, &frontPsn, &InForeground);
	}
	
	/* Call EventAvail a few times to force any windows to initally
		appear in the foreground.  See TN 180. */
	
	EventAvail(everyEvent, &dummy);
	EventAvail(everyEvent, &dummy);
}

#pragma segment Main

/*_____________________________________________________________________

	oop_Init - Initialize.
	
	Entry:	appleMenuID = apple menu id.
				lastAppleMenuItem = item number of last user item in apple menu.
				fileMenuID = file menu id.
				closeMenuItem = item number of close command in file menu,
					or 0 if none.
				editMenuID = edit menu id.
				selectAllItem = item number of select all command in edit menu,
					or 0 if none.
				doPeriodic = pointer to global DoPeriodic function.
				doDisk = pointer to global disk insertion event handler, or nil
					if none.
				extraMasterPtrBlocks = number of extra master pointer blocks
					to allocate.
				extraStackSpace = number of extra bytes of stack space to
					allocate.
_____________________________________________________________________*/

void oop_Init (short appleMenuID, short lastAppleMenuItem, 
	short fileMenuID, short closeMenuItem,
	short editMenuID, short selectAllItem, 
	FPeriodic doPeriodic, FDisk doDisk,
	long extraMasterPtrBlocks, long extraStackSpace)

{
#ifndef THINK_C
	extern void _DataInit();
#endif

	short			i;					/* loop index */

#ifndef THINK_C
	UnloadSeg(_DataInit);
#endif
	
	/* Copy parameters to global variables. */
	
	AppleMenuID = appleMenuID;
	LastAppleMenuItem = lastAppleMenuItem;
	FileMenuID = fileMenuID;
	CloseMenuItem = closeMenuItem;
	EditMenuID = editMenuID;
	SelectAllItem = selectAllItem;
	DoPeriodic = doPeriodic;
	DoDisk = doDisk;
	
	/* Initialize memory. */
	
	SetApplLimit(GetApplLimit() - extraStackSpace);
	MaxApplZone();
	for (i = 0; i < extraMasterPtrBlocks; i++) MoreMasters();
	
	/* Complete initialization. */
	
	Init();
}

/*_____________________________________________________________________

	oop_RegisterCommandHandler - Register Command Handler.
	
	Entry:	handler = command handler.
_____________________________________________________________________*/

void oop_RegisterCommandHandler (FCommand handler)

{
	numCmdHandlers++;
	SetHandleSize((Handle)CmdHandlers, numCmdHandlers*sizeof(FCommand));
	(**CmdHandlers)[numCmdHandlers-1] = handler;
}

/*_____________________________________________________________________

	oop_NewWindow - Initialize New Document Window.
	
	Entry:	w = pointer to window record.
				kind = kind of window.
				info = handle to window info.
				dispatch = pointer to method dispatch table.
_____________________________________________________________________*/

void oop_NewWindow (WindowPtr w, short kind, Handle info, oop_Dispatch *dispatch)

{
	WindObj		**wobj;			/* handle to new window object */
	
	wobj = (WindObj**)NewHandle(sizeof(WindObj));
	((WindowPeek)w)->refCon = (long)wobj;
	(**wobj).kind = kind;
	(**wobj).dialog = false;
	(**wobj).modal = false;
	(**wobj).moved = false;
	(**wobj).ibeamRgn = NewRgn();
	(**wobj).info = info;
	(**wobj).dispatch = dispatch;
	if (InForeground) (*DoPeriodic)(FrontWindow());
}

/*_____________________________________________________________________

	oop_NewDialog - Initialize New Dialog Window.
	
	Entry:	d = pointer to dialog record.
				kind = kind of window.
				info = handle to window info.
				dispatch = pointer to method dispatch table.
				modal = true if movable modal dialog.
				defaultBtn = item number of default button, or 0 if none.
				cancelBtn = item number of cancel button, or 0 if none.
				
	If the dialog contains a default button, an extra userItem is appended
	to the end of the dialog item list to outline the default button 
	(see IM I-427).
_____________________________________________________________________*/

void oop_NewDialog (WindowPtr d, short kind, Handle info, oop_Dispatch *dispatch,
	Boolean modal, short defaultBtn, short cancelBtn)

{
	WindObj		**wobj;			/* handle to window object */
	Handle		items;			/* handle to item list */
	long			size;				/* size of item list */
	short			itemType;		/* default button item type */
	Handle		item;				/* handle to default button control */
	Rect			box;				/* default button rectangle */

	wobj = (WindObj**)NewHandle(sizeof(WindObj));
	((WindowPeek)d)->refCon = (long)wobj;
	(**wobj).kind = kind;
	(**wobj).dialog = true;
	(**wobj).modal = modal;
	(**wobj).moved = false;
	(**wobj).ibeamRgn = utl_BuildDialogIBeamRgn(d);
	(**wobj).defaultBtn = defaultBtn;
	(**wobj).cancelBtn = cancelBtn;
	(**wobj).itemHit = 0;
	(**wobj).info = info;
	(**wobj).dispatch = dispatch;
	if (defaultBtn) {
		items = ((DialogPeek)d)->items;
		size = GetHandleSize(items);
		SetHandleSize(items, size+14);
		(**(short**)items)++;
		*(ProcPtr*)(*items+size) = (ProcPtr)OutlineDefaultButton;
		GetDItem(d, defaultBtn, &itemType, &item, &box);
		BlockMove(&box, *items+size+4, 8);
		*(*items+size+12) = userItem | itemDisable;
		*(*items+size+13) = 0;
	}
	if (InForeground) (*DoPeriodic)(FrontWindow());
}

/*_____________________________________________________________________

	oop_DoPeriodic - Periodic Method.
	
	Entry:	w = pointer to window record.
	
	This method changes the cursor to an ibeam if the mouse is over the
	ibeam region, otherwise it changes the cursor to an arrow.
	
	For dialogs, it also calls TEIdle to blink the insertion point.
_____________________________________________________________________*/

void oop_DoPeriodic (WindowPtr w)

{
	WindObj		**wobj;		/* handle to window object */
	Point			where;		/* current mouse location */

	wobj = GetWindObj(w);
	GetMouse(&where);
	SetCursor(PtInRgn(where, (**wobj).ibeamRgn) ? *IBeamHandle : &qd.arrow);
	if ((**wobj).dialog) TEIdle(((DialogPeek)w)->textH);
}

/*_____________________________________________________________________

	oop_DoClick - Mouse Down Event Handler Method.
	
	Entry:	w = pointer to window record.
				where = mouse click location in local coordinates.
				modifiers = modifiers from event record.
				
	For document windows this method does nothing. 
	
	For dialog windows the method calls DialogSelect to handle the event.
	If an enabled item is selected, the itemHit instance variable is
	set to the item number of the selected item.
_____________________________________________________________________*/

void oop_DoClick (WindowPtr w, Point where, short modifiers)

{
#pragma unused (where)
#pragma unused (modifiers)

	WindObj				**wobj;		/* handle to window object */
	DialogPtr			dlog;			/* pointer to dialog record */
	short					itemHit;		/* item number of item hit */

	wobj = GetWindObj(w);
	if (!(**wobj).dialog) return;
	if (DialogSelect(&Event, &dlog, &itemHit)) (**wobj).itemHit = itemHit;
}

/*_____________________________________________________________________

	oop_DoKey - Key Down and AutoKey Event Handler Method.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
				
	For document windows this method does nothing. 
	
	For dialog windows the method first checks to see if a special key
	was typed:
	
		Return or Enter: Treated same as click on default button.
		Escape or Cmd-period: Treated same as click on cancel button.
		
	If a special key was typed, the button is flashed and the itemHit
	instance variable is set to the item number of the button.
	
	If the key is not special, DialogSelect is called to handle the event.
_____________________________________________________________________*/

void oop_DoKey (WindowPtr w, char key, short modifiers)

{
	WindObj				**wobj;		/* handle to window object */
	DialogPtr			dlog;			/* pointer to dialog record */
	short					itemHit;		/* item number of item hit */

	wobj = GetWindObj(w);
	if (!(**wobj).dialog) return;
	if ((**wobj).itemHit = utl_CheckSpecialDialogKey(w, key, modifiers, 
		(**wobj).defaultBtn, (**wobj).cancelBtn)) return;
	DialogSelect(&Event, &dlog, &itemHit);
}

/*_____________________________________________________________________

	oop_DoUpdate - Update Event Handler Method.
	
	Entry:	w = pointer to window record.
	
	For document windows the window is erased. This method is seldom used for
	document windows, since they almost always contain something that needs to
	be drawn!
	
	For dialog windows the dialog is drawn and the default button, if any,
	is outlined.
_____________________________________________________________________*/

void oop_DoUpdate (WindowPtr w)

{
	WindObj		**wobj;				/* handle to window object */

	wobj = GetWindObj(w);
	EraseRect(&w->portRect);
	if ((**wobj).dialog) DrawDialog(w);
}

/*_____________________________________________________________________

	oop_DoActivate - Activate Event Handler Method.
	
	Entry:	w = pointer to window record.
	
	For document windows this method does nothing.
	
	For dialog windows DialogSelect is called to handle the event.
_____________________________________________________________________*/

void oop_DoActivate (WindowPtr w)

{
	WindObj				**wobj;		/* handle to window object */
	DialogPtr			dlog;			/* pointer to dialog record */
	short					itemHit;		/* item number of item hit */

	wobj = GetWindObj(w);
	if (!(**wobj).dialog) return;
	DialogSelect(&Event, &dlog, &itemHit);
}

/*_____________________________________________________________________

	oop_DoDeactivate - Deactivate Event Handler Method.
	
	Entry:	w = pointer to window record.
	
	For document windows this method does nothing.
	
	For dialog windows DialogSelect is called to handle the event.
_____________________________________________________________________*/

void oop_DoDeactivate (WindowPtr w)

{
	WindObj				**wobj;		/* handle to window object */
	DialogPtr			dlog;			/* pointer to dialog record */
	short					itemHit;		/* item number of item hit */

	wobj = GetWindObj(w);
	if (!(**wobj).dialog) return;
	DialogSelect(&Event, &dlog, &itemHit);
}

/*_____________________________________________________________________

	oop_DoGrow - Window Grow Method.
	
	Entry:	w = pointer to window record.
				where = mouse location.
				
	This method does nothing.
_____________________________________________________________________*/

void oop_DoGrow (WindowPtr w, Point where)

{
#pragma unused (w)
#pragma unused (where)
}

/*_____________________________________________________________________

	oop_DoZoom - Window Zoom Method.
	
	Entry:	w = pointer to window record.
				where = mouse location.
				partCode = inZoomIn or inZoomOut part code.
				
	This method does nothing.
_____________________________________________________________________*/

void oop_DoZoom (WindowPtr w, Point where, short partCode)

{
#pragma unused (w)
#pragma unused (where)
#pragma unused (partCode)
}

/*_____________________________________________________________________

	oop_DoClose - Window Close Method.
	
	Entry:	w = pointer to window record.
	
	This method disposes the WindObj record and the window itself.
_____________________________________________________________________*/

void oop_DoClose (WindowPtr w)

{
	WindObj			**wobj;		/* handle to window object */
	
	wobj = GetWindObj(w);
	DisposeRgn((**wobj).ibeamRgn);
	if ((**wobj).dialog) {
		DisposDialog(w);
	} else {
		DisposeWindow(w);
	}
	DisposHandle((Handle)wobj);
	if (InForeground) (*DoPeriodic)(FrontWindow());
}

/*_____________________________________________________________________

	oop_DoCommand - Command Handler Method.
	
	Entry:	w = pointer to window record.
				theMenu = menu index.
				theItem = item index.
				
	Exit:		function result = true if command handled.
				
	For document windows this method does nothing.
	
	For dialog windows the Edit menu commands cut, copy, paste, clear,
	and select all (if present) are handled.
_____________________________________________________________________*/

#pragma segment command

Boolean oop_DoCommand (WindowPtr w, short theMenu, short theItem)

{
	WindObj			**wobj;		/* handle to window object */
	TEHandle			textH;		/* handle to active TextEdit record */

	wobj = GetWindObj(w);
	if (!(**wobj).dialog) return false;
	textH = ((DialogPeek)w)->textH;
	if (theMenu != EditMenuID) return false;
	if (theItem == SelectAllItem) {
		TESetSelect(0, 0x7fff, textH);
		return true;
	} else {
		switch (theItem) {
			case cutCmd:
			case copyCmd:
				if (theItem == cutCmd) 
					TECut(textH); 
				else 
					TECopy(textH);
				ZeroScrap();
				TEToScrap();
				return true;
			case pasteCmd:
				TEFromScrap();
				TEPaste(textH);
				return true;
			case clearCmd:
				TEDelete(textH);
				return true;
		}
	}
	return false;
}

#pragma segment Main

/*_____________________________________________________________________

	DispatchCommand - Dispatch a Command.
	
	Entry:	top = pointer to top window record.
				kind = top window kind.
				dispatch = pointer to method dispatch table for top
					window object.
				mResult = 16/menu number, 16/item number.
				
	This function handles all commands for DAs.
	
	The Close command is handled by dispatching a DoClose messaage to the
	top window object.
	
	All other commands are processed as follows:
	
	If there is a top window and it is not a DA window, then a DoCommand
	message is dispatched to the top window.  The DoCommand method returns
	true if the command was processed, in which case we're done.  Otherwise,
	the registered DoCommand methods are given an opportunity to process the 
	command, in the order in which they were registered. For these later
	calls, nil is passed for the window pointer parameter.
	
	Note that it is possible for a DoCommand method to be called twice -
	once for the top window, and then again later because it is in the
	command handler list.
_____________________________________________________________________*/

static void DispatchCommand (WindowPtr top, short kind,
	oop_Dispatch *dispatch, unsigned long mResult)

{
	short				theMenu;			/* menu number */
	short				theItem;			/* item number */
	Str255			daName;			/* da name */
	GrafPtr			savePort;		/* saved grafPort */
	Boolean			callHandlers;	/* true to pass command to command handlers */
	short				i;					/* loop index */
	
	theMenu = (mResult >> 16) & 0xFFFF;
	theItem = mResult & 0xFFFF;
	callHandlers = true;
	if (theMenu == AppleMenuID && theItem > LastAppleMenuItem) {
		GetItem(GetMHandle(AppleMenuID), theItem, daName);
		GetPort(&savePort);
		(void) OpenDeskAcc(daName);
		SetPort(savePort);
		callHandlers = false;
	} else if (theMenu == FileMenuID && theItem == CloseMenuItem && top) {
		if (kind == daWind) {
			CloseDeskAcc(((WindowPeek)top)->windowKind);
		} else {
			(*dispatch->doClose)(top);
		}
		callHandlers = false;
	} else if (theMenu == EditMenuID) {
		callHandlers = !SystemEdit(theItem-1);
	}
	if (callHandlers) {
		if (top && kind != daWind) {
			callHandlers = !(*dispatch->doCommand)(top, theMenu, theItem);
		}
		if (callHandlers) {
			for (i = 0; i < numCmdHandlers; i++) {
				if ((*(**CmdHandlers)[i])(nil, theMenu, theItem)) break;
			}
		}
	}
	HiliteMenu(0);
}

/*_____________________________________________________________________

	oop_DoEvent - Process One Event.
	
	Entry		event = pointer to event record, or nil if oop_DoEvent should
					fetch the next event.
				eventMask = event mask for WaitNextEvent call.
				sleep = sleep time for WaitNextEvent call.
				mouseRgn = mouse region for WaitNextEvent call.
	
	A DoPeriodic message is dispatched to the top window object.
	
	The next event is fetched and the appropriate message is dispatched
	to the appropriate window object.
_____________________________________________________________________*/

void oop_DoEvent (EventRecord *event, short eventMask, long sleep, 
	RgnHandle mouseRgn)

{
	WindowPtr			top;				/* pointer to top window record */
	short					kind;				/* top window kind */
	WindObj				**wobj;			/* handle to window object */
	oop_Dispatch		*dispatch;		/* pointer to method dispatch table */
	short					partCode;		/* FindWindow part code */
	WindowPtr			w;					/* window clicked in */
	Point					where;			/* location of mouse click in local coords */
	char					key;				/* the key pressed */
	Rect					oldPortRect;	/* old window portrect */
	Rect					newPortRect;	/* new window portrect */
	GrafPtr				savedPort;		/* saved grafport */
	Boolean				wneResult;		/* WaitNextEvent function result */

	GetPort(&savedPort);
	top = FrontWindow();
	kind = oop_GetWindKind(top);
	if (InForeground) (*DoPeriodic)(top);
	if (top && kind != daWind) {
		SetPort(top);
		wobj = GetWindObj(top);
		dispatch = (**wobj).dispatch;
		if (InForeground) (*dispatch->doPeriodic)(top);
	} else {
		dispatch = nil;
	}
	if (event) {
		Event = *event;
		wneResult = true;
	} else {
		wneResult = utl_WaitNextEvent(eventMask, &Event, sleep, mouseRgn);
	}
	if (top && (**wobj).dialog) IsDialogEvent(&Event);
	if (wneResult) {
		switch (Event.what) {
			case mouseDown:
				switch (partCode = FindWindow(Event.where, &w)) {
					case inSysWindow:
						SystemClick(&Event, w);
						break;
					case inMenuBar:
						DispatchCommand(top, kind, dispatch, MenuSelect(Event.where));
						break;
					case inGoAway:
						if (TrackGoAway(w, Event.where)) {
							(*dispatch->doClose)(w);
							InitCursor();
						}
					case inDrag:
						oldPortRect = newPortRect = w->portRect;
						SetPort(w);
						LocalToGlobal((Point*)&oldPortRect);
						DragWindow(w, Event.where, &DragRect);
						LocalToGlobal((Point*)&newPortRect);
						if (abs(oldPortRect.top - newPortRect.top) >= dragSlop ||
							abs(oldPortRect.left - newPortRect.left) >= dragSlop) 
								(**wobj).moved = true;
						break;
					case inGrow:
						SetPort(w);
						oldPortRect = w->portRect;
						(*dispatch->doGrow)(w, Event.where);
						if (abs(oldPortRect.right - (w->portRect).right) >= dragSlop ||
							abs(oldPortRect.bottom - (w->portRect).bottom) >= dragSlop) 
								(**wobj).moved = true;
						break;
					case inZoomIn:
					case inZoomOut:
						SetPort(w);
						oldPortRect = w->portRect;
						(*dispatch->doZoom)(w, Event.where, partCode);
						if (abs(oldPortRect.right - (w->portRect).right) >= dragSlop ||
							abs(oldPortRect.bottom - (w->portRect).bottom) >= dragSlop) 
								(**wobj).moved = true;
						break;
					case inContent:
						if (w != top) {
							if (!(**wobj).modal) {
								SelectWindow(w);
							} else {
								SysBeep(0);
							}
						} else {
							SetPort(w);
							where = Event.where;
							GlobalToLocal(&where);
							(*dispatch->doClick)(w, where, Event.modifiers);
						}
						break;
				}
				break;
			case keyDown:
			case autoKey:
				key = Event.message & charCodeMask;
				if ((Event.modifiers & cmdKey) && key != '.') {
					DispatchCommand(top, kind, dispatch, MenuKey(key));
				} else {
					if (dispatch) (*dispatch->doKey)(top, key, Event.modifiers);
				}
				break;
			case activateEvt:
				w = (WindowPtr)Event.message;
				SetPort(w);
				wobj = GetWindObj(w);
				dispatch = (**wobj).dispatch;
				if (Event.modifiers & activeFlag) {
					(*dispatch->doActivate)(w);
				} else {
					(*dispatch->doDeactivate)(w);
				}
				break;
			case updateEvt:
				w = (WindowPtr)Event.message;
				wobj = GetWindObj(w);
				dispatch = (**wobj).dispatch;
				SetPort(w);
				BeginUpdate(w);
				(*dispatch->doUpdate)(w);
				EndUpdate(w);
				break;
			case diskEvt:
				if (DoDisk) (*DoDisk)(Event.message);
				break;
			case osEvt:
				if (((Event.message >> 24) & 0xff) != suspendResumeMessage) break;
				InForeground = Event.message & 1;
				if (InForeground) InitCursor();
				if (dispatch) {
					SetPort(top);
					if (InForeground) {
						(*dispatch->doActivate)(top);
					} else {
						(*dispatch->doDeactivate)(top);
					}
				}
				break;
			case kHighLevelEvent:
				AEProcessAppleEvent(&Event);
				break;
		}
	}
	SetPort(savedPort);
}

/*_____________________________________________________________________

	oop_UpdateAll - Update All Windows.
_____________________________________________________________________*/

void oop_UpdateAll (void)

{
	WindowPtr		w;				/* traverses window list */
	WindowPeek		wp;			/* windowpeek pointer */
	WindObj			**wobj;		/* handle to window object */
	oop_Dispatch	*dispatch;	/* pointer to method dispatch table */
	GrafPtr			savedPort;	/* saved grafport */
	
	GetPort(&savedPort);
	w = FrontWindow();
	while (w) {
		if (oop_GetWindKind(w) != daWind) {
			wp = (WindowPeek)w;
			if (!EmptyRgn(wp->updateRgn)) {
				SetPort(w);
				wobj = GetWindObj(w);
				dispatch = (**wobj).dispatch;
				BeginUpdate(w);
				(*dispatch->doUpdate)(w);
				EndUpdate(w);
			}
		}
		w = (WindowPtr)(wp->nextWindow);
	}
	SetPort(savedPort);
}

/*_____________________________________________________________________

	oop_ModalUpdate - Update a Window in a Modal Dialog.
	
	Entry:	theDialog = pointer to modal dialog.
				theEvent = pointer to event record.
				
	Exit:		function result = false.
	
	This function should be used as a filter proc for all modal dialogs
	and alerts. It takes care of the "pending update peril" problem (see
	TN 304) which happens when a modal dialog is active and an update
	event occurs for some other window in the application.
_____________________________________________________________________*/

pascal Boolean oop_ModalUpdate (DialogPtr theDialog,
	EventRecord *theEvent, short *itemHit)
	
{
#pragma unused (itemHit)

	WindowPtr		w;				/* pointer to window to be updated */
	WindObj			**wobj;		/* handle to window object */
	oop_Dispatch	*dispatch;	/* pointer to method dispatch table */
	GrafPtr			savedPort;	/* saved grafport */
	
	w = (WindowPtr)theEvent->message;
	if (theEvent->what == updateEvt && w != theDialog && oop_GetWindKind(w) != daWind) {
		GetPort(&savedPort);
		SetPort(w);
		wobj = GetWindObj(w);
		dispatch = (**wobj).dispatch;
		BeginUpdate(w);
		(*dispatch->doUpdate)(w);
		EndUpdate(w);
		SetPort(savedPort);
	}
	return false;
}
