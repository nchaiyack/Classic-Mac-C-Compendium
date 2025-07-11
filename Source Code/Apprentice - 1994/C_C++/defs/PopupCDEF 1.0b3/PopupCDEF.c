/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/*	This file contains a popup CDEF that uses the file PopupLib.c to handle
	the details of popup menus. This file contains glue to link the Control
	Manager and the routines in PopupLib.c.

	94/03/15 aih
	- Added check against kPopupVersion before a CDEF is attached during
	debugging with PopupCDEFAttach.
	- Moved description to documentation file.
	- Uses bullet instead of check mark as mark character when using
	window font.
	- Added support for the popupFixedWidth variation code.
	
	94/01/19 aih
	- Added call to GetMHandle to allow use of menus created by the application
	that are not in the resource file. Also fixed use of an already disposed
	handle in the CDEF's dispose routine. Thanks to Eric Bowman (bobo@reed.edu)
	for both of these.
	 
	93/12/31 aih
	- Returns 1 as part code (same as 7.0 CDEF) instead of inCheckBox
	
	93/12/26 aih
	- Major overhaul. Now supports most of the features of the System 7.0 CDEF.
	- Added functions for installing a glue handle to the popup CDEF,
	allowing the CDEF to be debugged from within an application. The only
	message that can't be debugged this way is the initCntl message,
	since initCntl is sent by NewControl before we can install the
	glue handle.
	
	93/12/24 aih
	- Adapted to use PopupHandle type
	- Removed dependence on other libraries
	- Simplified by removing complicated variation setting code
	based on parsing the title string
	
	91/03/15 aih
	- Fixed bug which caused a crash if the menu resource specified in the
	control's refCon field couldn't be loaded
	
	91/03/04-05 Ari Halberstadt (aih)
	- Created this file */
	
#include <limits.h>
#include <LoMem.h>
#include <SetUpA4.h>
#include "PopupLib.h"

/* mask for the low 31 bits of the calcCRgns message to CDEFs */
#define calcCRgnsMask		(0x7fffffffL)

/* hilite value for a disabled control */
#define kControlDisabled	(255)

/* System software version needed to use this CDEF. Tested with system 6.0.5
	and 7.0, but may work on systems as early as 4.0.1. */
#define kSystemVersion		(0x0605)

/* The checkMark character is only present in the Chicago system font,
	whereas the bullet character ('�') is present in most fonts. When the
	popupUseWFont variation code is used, we change the mark character
	from the default checkMark to kBulletMark. */
#define kBulletMark			('�')

/* draw the control */
static void Draw(ControlHandle ctl, PopupHandle popup)
{
	Str255 title;
	Rect bounds;
	
	GetCTitle(ctl, title);
	bounds = (**ctl).contrlRect;
	PopupDrawSet(popup, false);
	PopupTitleSet(popup, title);
	PopupBoundsSet(popup, &bounds);
	PopupVisibleSet(popup, (**ctl).contrlVis);
	PopupCurrentSet(popup, (**ctl).contrlValue);
	PopupEnableSet(popup, (**ctl).contrlHilite != kControlDisabled);
	PopupDrawSet(popup, true);
	PopupCalculate(popup);
	PopupDraw(popup);
}

/* return the part of the control that the point is in */
static long Test(ControlHandle ctl, PopupHandle popup, Point where)
{
	return(PopupWithin(popup, where) ? kPopupPartCode : 0);
}

/* calculate the region containing the control */
static void Calculate(ControlHandle ctl, PopupHandle popup, RgnHandle rgn)
{
	Rect bounds;
	
	PopupBounds(popup, &bounds);
	RectRgn(rgn, &bounds);
}

/* load the menu and set 'gotmenu' to true if we called GetMenu */
static MenuHandle LoadMenu(short id, Boolean *gotmenu)
{
	MenuHandle	menu;		/* the menu */
	ProcPtr		errproc;	/* for saving and restoring ResErrProc */
	short			load;		/* for saving and restoring ResLoad */
	
	/* we can tell if the menu hasn't been loaded by a call to GetMenu
		since the menu handle will be a nil resource handle */
	*gotmenu = false;
	load = ResLoad;
	SetResLoad(false);
	menu = (MenuHandle) GetResource('MENU', id);
	SetResLoad(load);
	if (menu && ! *menu) {
		/* The menu is in the resource file, but it hasn't been loaded
			yet, so we have to call GetMenu. */
		errproc = ResErrProc;
		ResErrProc = NULL;
		menu = GetMenu(id);
		ResErrProc = errproc;
		*gotmenu = true;
	}
	else if (! menu) {
		/* To allow use of menus created by the application that are not in the
			resource file we look for the menu in the menu list. */
		menu = GetMHandle(id);
	}
	return(menu);
}

/* initialize the control */
static void Initialize(ControlHandle ctl, short var)
{
	MenuHandle	menu;		/* the menu */
	PopupHandle popup;	/* handle to the popup menu */
	Rect			bounds;	/* control's bounding rectangle */
	short			nitems;	/* number of items in menu */
	Boolean		gotmenu;	/* true if we created the menu by calling GetMenu */
	
	menu = LoadMenu((**ctl).contrlMin, &gotmenu);
	if (menu) {
		if (gotmenu && (var & popupUseAddResMenu) != 0)
			AddResMenu(menu, (**ctl).contrlRfCon);
		bounds = (**ctl).contrlRect;
		popup = PopupBegin((**ctl).contrlOwner, menu, &bounds);
		if (popup) {
			(**popup).state.gotmenu = gotmenu;
			PopupDrawSet(popup, false);
			PopupTitleWidthSet(popup, (**ctl).contrlMax);
			if (((**ctl).contrlValue & popupTitleNoStyle) == 0)
				PopupTitleStyleSet(popup, (**ctl).contrlValue >> CHAR_BIT);
			if (((**ctl).contrlValue & popupTitleRightJust) == popupTitleRightJust)
				PopupJustSet(popup, teFlushRight);
			if ((var & popupUseWFont) != 0)
				PopupMarkSet(popup, kBulletMark);
			PopupTypeInSet(popup, (var & popupTypeIn) != 0);
			PopupUseWFontSet(popup, (var & popupUseWFont) != 0);
			PopupFixedWidthSet(popup, (var & popupFixedWidth) != 0);
			PopupDrawSet(popup, true);
			PopupCalculate(popup);
			nitems = CountMItems(menu);
			(**ctl).contrlMax = nitems;
			(**ctl).contrlMin = (nitems > 0 ? 1 : 0);
			(**ctl).contrlValue = (nitems > 0 ? 1 : 0);
			(**ctl).contrlData = (Handle) popup;
			(**ctl).contrlAction = (ProcPtr) -1L;
		}
	}
}

/* dispose of the control */
static void Dispose(ControlHandle ctl, PopupHandle popup)
{
	MenuHandle menu;
	Boolean gotmenu;
	
	menu = (**popup).menu;
	gotmenu = (**popup).state.gotmenu;
	PopupEnd(popup);
	if (gotmenu)
		ReleaseResource((Handle) menu);
	(**ctl).contrlData = NULL;
}

/* track a mouse click in the control */
static void Track(ControlHandle ctl, PopupHandle popup)
{
	short value;
	
	PopupSelect(popup);
	value = PopupCurrent(popup);
	(**ctl).contrlValue = value;
}

/* entry point for CDEF */
pascal long PopupCDEF(short var, ControlHandle ctl, short msg, long param)
{
	PopupHandle	popup = NULL;
	SysEnvRec world;
	long result = 0;
	
	/* setup global variables */
	RememberA0();
	SetUpA4();
	
	/* check system software */
	(void) SysEnvirons(curSysEnvVers, &world);
	if (world.systemVersion >= kSystemVersion) {
		
		/* execute message */
		popup = (PopupHandle) (**ctl).contrlData;
		if (msg == initCntl) {
			Initialize(ctl, var);
			popup = (PopupHandle) (**ctl).contrlData;
		}
		else if (popup) {
			switch (msg) {
			case drawCntl:
				param = LoWord(param); /* see TN196 */
				Draw(ctl, popup);
				break;
			case testCntl:
				result = Test(ctl, popup, *(Point *) &param);
				break;
			case calcCRgns:
				param &= calcCRgnsMask;
				/* no break */
			case calcCntlRgn:
			case calcThumbRgn:
				Calculate(ctl, popup, (RgnHandle) param);
				break;
			case dispCntl:
				Dispose(ctl, popup);
				popup = NULL;
				break;
			case autoTrack:
				param = LoWord(param); /* see TN196 */
				Track(ctl, popup);
				break;
			}
		}
	}
	RestoreA4();
	return(result);
}

#ifdef CDEF
pascal long main(short var, ControlHandle ctl, short msg, long param)
{
	return(PopupCDEF(var, ctl, msg, param));
}
#endif /* CDEF */

#if ! CDEF && ! NDEBUG

/* Functions for attaching a glue handle so the CDEF can be debugged
	from within an application. */
	
#define ASM_JMP (0x4EF9)	/* jump instruction */

/* the structure installed in the contrlDefProc field */
typedef struct {
	short jmp;					/* jump instruction */
	void *addr;					/* address of CDEF function */
	Handle contrlDefProc;	/* saved value of contrlDefProc field */
} PopupGlueType, *PopupGluePtr, **PopupGlueHandle;

/* Set the control's defproc field to a small glue handle that will
	jump to PopupCDEF. This makes debugging a lot easier, since you
	can then step through the code with a debugger. */
void PopupCDEFAttach(ControlHandle ctl)
{
	PopupGlueHandle glue;
	
	if (PopupVersion((PopupHandle) (**ctl).contrlData) == kPopupVersion) {
		glue = (PopupGlueHandle) NewHandleClear(sizeof(PopupGlueType));
		if (glue) {
			(**glue).jmp = ASM_JMP;
			(**glue).addr = PopupCDEF;
			(**glue).contrlDefProc = (**ctl).contrlDefProc;
			(**ctl).contrlDefProc = (Handle) glue;
		}
	}
}

/* Dispose of the glue handle created with PopupCDEFAttach and set the
	control's contrlDefProc field to point to its original value. */
void PopupCDEFDetach(ControlHandle ctl)
{
	PopupGlueHandle glue;
	
	glue = (PopupGlueHandle) (**ctl).contrlDefProc;
	(**ctl).contrlDefProc = (**glue).contrlDefProc;
	DisposeHandle((Handle) glue);
}

#endif /* ! CDEF && ! NDEBUG */
