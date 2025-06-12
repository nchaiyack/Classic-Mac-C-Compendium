/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/* A simple program to test my Popup CDEF. A dialog containing various
	types of popup controls is displayed. You can make selections from the
	menus, and notice how the current item is marked with a check mark.
	Click the "Quit" button to quit the program.
	
	A type-in popup menu is also demonstrated, and a functions that show
	how type-in popup menus can be supported are provided. To make it
	easier to find the relavent code, sections of code that are used
	for the type-in popup menu are bracketed with "TYPEIN_BEGIN" and
	"TYPEIN_END".
	
	94/03/14 aih - created by hacking Thread Library test application */

#include <GestaltEqu.h>
#include <Traps.h>
#include "PopupLib.h"

/*----------------------------------------------------------------------------*/
/* global definitions and declarations */
/*----------------------------------------------------------------------------*/

/* It is much easier to debug an application than it is to debug
	a code resource. So that the CDEF can be debugged from within
	an application, we can attach the CDEF compiled as part of this
	application to a popup menu control, replacing the 'CDEF'
	resource for the control. If you define CDEF_ATTACH as 1 then
	the CDEF will be "attached", so that it can be debugged within
	this application. */
#ifndef CDEF_ATTACH
	#define CDEF_ATTACH (0)
#endif /* CDEF_ATTACH */

/* dialog items */
enum {
	rDialog = 128,
	iQuit = 1,
	iFontPopup,
	iSizeTitle,
	iSizeText,
	iSizePopup,
	iStylePopup,
	iAlignPopup,
	iLongTitlePopup,
	iIconsPopup,
	iRightAlignedPopup,
	iWindowFontPopup,
	iDisabledItemPopup,
	iDisabledPopup,
	iNoTitlePopup,
	iLast
};

/*----------------------------------------------------------------------------*/
/* assertions */
/*----------------------------------------------------------------------------*/

#ifndef NDEBUG
	#define myassert(x) ((void) ((x) || assertfailed()))
#else
	#define myassert(x) ((void) 0)
#endif

#define require(x)	myassert(x)
#define check(x)		myassert(x)
#define ensure(x)		myassert(x)

static int assertfailed(void)
{
	DebugStr((StringPtr) "\p An assertion failed.");
	return(0);
}

/*----------------------------------------------------------------------------*/
/* standard Macintosh initializations */
/*----------------------------------------------------------------------------*/

/* initialize application heap */
static void HeapInit(long stack, short masters)
{
	SetApplLimit(GetApplLimit() - stack);
	MaxApplZone();
	while (masters-- > 0)
		MoreMasters();
}

/* initialize managers */
static void ManagersInit(void)
{
	EventRecord event;
	short i;
	
	/* standard initializations */
	InitGraf((Ptr) &thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	FlushEvents(everyEvent, 0);
	InitCursor();
	
	/* so first window will be frontmost */
	for (i = 0; i < 4; i++)
		EventAvail(everyEvent, &event);
}

/*----------------------------------------------------------------------------*/
/* event utilities */
/*----------------------------------------------------------------------------*/

/* Functions for determining whether a trap is available. Based on
	functions given in IM VI. */

/* return number of toolbox traps */
static short TrapNumToolbox(void)
{
	short result = 0;
	
	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		result = 0x0200;
	else
		result = 0x0400;
	return(result);
}

/* return the type of the trap */
static TrapType TrapTypeGet(short trap)
{
	return((trap & 0x0800) > 0 ? ToolTrap : OSTrap);
}

/* true if the trap is available  */
static Boolean TrapAvailable(short trap)
{
	TrapType type;
	
	type = TrapTypeGet(trap);
	if (type == ToolTrap) {
		trap &= 0x07FF;
		if (trap >= TrapNumToolbox())
			trap = _Unimplemented;
	}
	return(NGetTrapAddress(trap, type) != NGetTrapAddress(_Unimplemented, ToolTrap));
}

/* true if the WaitNextEvent trap is available */
static Boolean MacHasWNE(void)
{
	static Boolean initialized;
	static Boolean wne;
	
	if (! initialized) {
		/* do only once for efficiency */
		wne = TrapAvailable(_WaitNextEvent);
		initialized = true;
	}
	return(wne);
}

/* Call GetNextEvent or WaitNextEvent, depending on which one is available.
	The parameters to this function are identical to those to WaitNextEvent.
	If GetNextEvent is called the extra parameters are ignored. */
static Boolean EventGet(short mask, EventRecord *event,
	unsigned long sleep, RgnHandle cursor)
{
	Boolean result = false;

	if (MacHasWNE())
		result = WaitNextEvent(mask, event, sleep, cursor);
	else {
		SystemTask();
		result = GetNextEvent(mask, event);
	}
	if (! result) {
		/* make sure it's a null event, even if the system thinks otherwise, e.g.,
			some desk accessory events (see comment in TransSkell event loop) */
		event->what = nullEvent;
	}
	return(result);
}

/*----------------------------------------------------------------------------*/
/* dialog utilities */
/*----------------------------------------------------------------------------*/

/* get the text of the dialog item */
static void GetDText(DialogPtr dlg, short item, Str255 str)
{
	short type;
	Handle hitem;
	Rect box;

	GetDItem(dlg, item, &type, &hitem, &box);
	GetIText(hitem, str);
}

/* set the text of the dialog item */
static void SetDText(DialogPtr dlg, short item, const Str255 str)
{
	short type;
	Handle hitem;
	Rect box;

	GetDItem(dlg, item, &type, &hitem, &box);
	SetIText(hitem, str);
}

/* return the numeric value of the dialog item */
static long GetDNum(DialogPtr dlg, short item)
{
	long num;
	Str255 str;
	
	GetDText(dlg, item, str);
	StringToNum(str, &num);
	return(num);
}

/* set the text of the dialog item to the number */
static void SetDNum(DialogPtr dlg, short item, long num)
{
	Str255 str;

	NumToString(num, str);
	SetDText(dlg, item, str);
}

/* return the control handle for the item */
static ControlHandle GetDControl(DialogPtr dlg, short item)
{
	short type;
	Handle hitem;
	Rect box;

	GetDItem(dlg, item, &type, &hitem, &box);
	return((ControlHandle) hitem);
}

/* return a handle to the popup control's menu */
static MenuHandle GetCtlMenu(ControlHandle ctl)
{
	return((**(PopupPrivateHandle) (**ctl).contrlData).mHandle);
}

/*----------------------------------------------------------------------------*/
/* menu utilities */
/*----------------------------------------------------------------------------*/

/*	Given a font family id and true in 'outlined', OutlineFontSizes will outline
	all items in a size menu that actually exist in that font. If 'outlined' is
	false, all items will be set to plain text, which is useful if you don't
	have any font information. */
static void OutlineFontSizes(MenuHandle menu, short family, Boolean outlined)
{
	short		nitems;	/* number of items in menu */
	short		item;		/* current item number */
	long		size;		/* size of current menu item */
	Str255	name;		/* name of item */
	Boolean	found;	/* flag that we've already found the menu item */

	found = false;
	nitems = CountMItems(menu);
	for (item = 1; item <= nitems; item++) {
		GetItem(menu, item, name);
		StringToNum(name, &size);
		if (outlined && RealFont(family, size))
			SetItemStyle(menu, item, outline);
		else
			SetItemStyle(menu, item, 0);
	}
}

/* TYPEIN_BEGIN */
/* return item number with given title, or 0 if not found */
static short FindMenuItem(MenuHandle menu, const Str255 ptitle)
{
	short item;		/* index to menu items */
	short nitems;	/* number of items in menu */
	Str255 name;	/* name of current item */
	
	nitems = CountMItems(menu);
	for (item = 1; item <= nitems; item++) {
		GetItem(menu, item, name);
		if (EqualString(ptitle, name, false, true))
			break;
	}
	return(item <= nitems ? item : 0);
}
/* TYPEIN_END */

/* TYPEIN_BEGIN */
/* AdjustTypeInPopupMenu adjusts a type-in popup menu. The 'dlg' parameter
	is a pointer to the dialog containing the type-in popup meun. The
	'popupItem' parameter is the item number of a type-in popup control. The
	'textItem' parameter is the item number of the type-in editable text field.
	The 'insertedCustomValue' parameter should initially be false; subsequently,
	it must contain the value returned by the previous call to
	AdjustTypeInPopupMenu.
	
	You should call AdjustTypeInPopupMenu whenever there's a click in the
	popup menu control, but before TrackControl (or DialogSelect) is called
	to handle the click. If the user entered a value into the text field that
	is not in the popup menu, then the user's entry is inserted as the
	first item in the popup menu and a dashed line is inserted to separate
	the user's entry from the predefined values in the menu. When the user
	chooses a menu item or enters a value into the text field that is one of
	the predefined values in the menu, then the items inserted into the menu
	are deleted. The 'insertedCustomValue' flag is used to determine if a
	user's entry was inserted into the menu. */
static Boolean AdjustTypeInPopupMenu(DialogPtr dlg,
	short popupItem, short textItem,
	Boolean insertedCustomValue)
{
	ControlHandle ctl;	/* handle to size popup control */
	MenuHandle menu;		/* handle to popup menu's handle */
	short item;				/* index to item in menu */
	short nitems;			/* number of items in size menu */
	Str255 text;			/* string in text item */
	
	/* adjust user's choice */
	GetDText(dlg, textItem, text);
	ctl = GetDControl(dlg, popupItem);
	menu = GetCtlMenu(ctl);
	item = FindMenuItem(menu, text);
	if (item == 0) {

		/* user entered a size not found in the menu, so add the user's
			choice as the first item in the menu (IM-VI, p2-37) */
		if (! insertedCustomValue) {
			InsMenuItem(menu, (StringPtr) "\p(-", 0);
			InsMenuItem(menu, text, 0);
			for (nitems = CountMItems(menu); nitems > 0; nitems--)
				SetItemMark(menu, nitems, noMark);
			SetItemMark(menu, 1, checkMark);
			SetCtlMax(ctl, GetCtlMax(ctl) + 2);
			insertedCustomValue = true;
		}
		SetItem(menu, 1, text);
		SetCtlValue(ctl, 1);
	}
	else if (item > 2 && insertedCustomValue) {

		/* remove user's choice, since selected item is in menu */
		insertedCustomValue = false;
		SetCtlValue(ctl, 1);
		DelMenuItem(menu, 1);
		DelMenuItem(menu, 1);
		SetCtlMax(ctl, GetCtlMax(ctl) - 2);
		SetCtlValue(ctl, item - 2);
	}
	else
		SetCtlValue(ctl, item);

	return(insertedCustomValue);
}
/* TYPEIN_END */

/*----------------------------------------------------------------------------*/
/* The event loop. */
/*----------------------------------------------------------------------------*/

/* abort on error */
static void fatal(Boolean exit, const StringPtr msg)
{
	if (exit) {
		DebugStr(msg);
		ExitToShell();
	}
}

/* create the dialog and run the program */
static void Run(void)
{
	Boolean sizeHasCustomValue;/* true if size menu has a custom value */
	EventRecord event;			/* event record for getting next event */
	WindowPtr window;				/* for handling window events */
	DialogPtr dlg;					/* popup demo dialog */
	Boolean quit;					/* true if time to quit application */
	Str255 str;						/* utility string */
	short font;						/* for getting font */
	short i;							/* index to popup menu controls */
	const short popups[] = {	/* all of the popup menu controls in the dialog */
		iFontPopup,
		iSizePopup,
		iStylePopup,
		iAlignPopup,
		iLongTitlePopup,
		iIconsPopup,
		iRightAlignedPopup,
		iWindowFontPopup,
		iDisabledItemPopup,
		iDisabledPopup,
		iNoTitlePopup,
		iLast,
	};

	/* create the demo dialog */
	dlg = GetNewDialog(rDialog, NULL, (WindowPtr) -1);
	fatal(dlg == NULL, "\p nil dialog pointer");
	SelIText(dlg, iSizeText, 0, 32767);
	
	#if CDEF_ATTACH
		/* It is much easier to debug an application than it is to debug
			a code resource. So that the CDEF can be debugged from within
			an application, we attach the CDEF compiled as part of this
			application to a popup menu control, replacing the 'CDEF'
			resource for the control. */
		for (i = 0; popups[i] != iLast; i++)
			PopupCDEFAttach(GetDControl(dlg, popups[i]));
	#endif /* CDEF_ATTACH */
	
	/* disable a popup menu control so we can see what a disabled popup
		menu control looks like */
	HiliteControl(GetDControl(dlg, iDisabledPopup), 255);
	
	/* display the dialog and handle events */
	ShowWindow(dlg);
	sizeHasCustomValue = false;
	quit = false;
	while (! quit) {
	
		/* Set the dialog's font and font size so we can see the effect of the
			useWFont variation code. We need to do this once every time through
			the event loop since (in system 6.0) DialogSelect resets the font
			to the system font. */
		SetPort(dlg);
		TextFont(geneva);
		TextSize(9);
	
		/* handle the next event; this is a pretty simple (but inelegant)
			event loop */
		SetCursor(&arrow);
		(void) EventGet(everyEvent, &event, GetCaretTime(), NULL);
		switch (event.what) {
		case updateEvt:
			/* handle an update event */
			window = (WindowPtr) event.message;
			BeginUpdate(window);
			if (window == dlg) {
				DrawDialog(dlg);
				event.what = nullEvent;
			}
			EndUpdate(window);
			break;
		case mouseDown:
			/* handle a click in a window's drag bar */
			switch (FindWindow(event.where, &window)) {
			case inDrag:
				DragWindow(window, event.where, &screenBits.bounds);
				break;
			}
			break;
		}
		
		/* handle dialog events */
		if (IsDialogEvent(&event)) {
			DialogPtr dlgHit;
			short itemHit;
			Point where;
			
			/* TYPEIN_BEGIN */
			/* check if user is clicking in size type-in popup menu */
			if (event.what == mouseDown) {
				where = event.where;
				GlobalToLocal(&where);
				if (FindDItem(dlg, where) + 1 == iSizePopup) {

					/* adjust the size popup menu before it's pulled down */
					sizeHasCustomValue = AdjustTypeInPopupMenu(dlg,
													iSizePopup, iSizeText,
													sizeHasCustomValue);

					/* outline font sizes available in the selected font */
					GetItem(GetCtlMenu(GetDControl(dlg, iFontPopup)),
						GetCtlValue(GetDControl(dlg, iFontPopup)), str);
					GetFNum(str, &font);
					OutlineFontSizes(GetCtlMenu(GetDControl(dlg, iSizePopup)),
						font, true);
				}
			}
			/* TYPEIN_END */
			
			/* handle the dialog event */
			if (DialogSelect(&event, &dlgHit, &itemHit)) {
			
				/* handle a click in one of the dialog's buttons */
				check(dlg == dlgHit);
				switch (itemHit) {
				case iQuit:
					quit = true;
					break;
				case iSizePopup:
					/* TYPEIN_BEGIN */
					/* Set the value displayed in the text field to the value
						chosen from the size popup menu and select the text in
						the size edit field. */
					GetItem(GetCtlMenu(GetDControl(dlg, iSizePopup)),
						GetCtlValue(GetDControl(dlg, iSizePopup)), str);
					SetDText(dlg, iSizeText, str);
					SelIText(dlg, iSizeText, 0, 32767);
					break;
					/* TYPEIN_END */
				}
			}
		}
	}
	
	/* dispose of everything */
	#if CDEF_ATTACH
		for (i = 0; popups[i] != iLast; i++)
			PopupCDEFDetach(GetDControl(dlg, popups[i]));
	#endif /* CDEF_ATTACH */
	DisposeDialog(dlg);
}

void main(void)
{
	HeapInit(0, 4);
	ManagersInit();
	Run();
}
