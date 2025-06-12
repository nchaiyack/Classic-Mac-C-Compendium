/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */
	
#pragma once

#include "PopupCDEF.h"

#ifndef arrowCursor
	/* arrowCursor is defined in DrawLib.h,
		which also defines TextState */
	typedef struct {
		short	font;
		Style	face;
		short	mode;
		short	size;
	} TextState;
#endif /* arrowCursor */

/* Drawing environment data which must be remembered before any drawing
	of the popup menu is begun and restored after all drawing has finished. */
typedef struct {
	GrafPtr		port;			/* graf port on entry to popup cdef */
	PenState		pen;			/* saved pen state for popup's port */
	TextState	text;			/* saved text state for popup's port */
	RgnHandle	clip;			/* saved clip region for popup's port */
	short			sysfont;		/* saved system font */
	short			syssize;		/* saved system font size */
} PopupEnvType;

/* rectangles defining the various areas of a popup menu */
typedef struct {
	Rect maxbounds;			/* maximum bounding rectangle of popup */
	Rect bounds;				/* rectangle around all of popup */
	Rect hilite;				/* hilited when a selection is being made */
	Rect title;					/* contains the popup's title */
	Rect selection;			/* contains the currently selected item */
	Rect arrow;					/* contains the down arrow */
} PopupRectanglesType;

/* structure containing all the data needed by the popup menu */
typedef struct {

	/* these first two fields will never be changed, other fields may change */
	PopupPrivateType private;	/* for compatability with Apple's CDEF */
	short				version;		/* version of the code that created this popup menu */

	/* fields specified on creation of the popup menu (maxbounds is in
		rectangles field) */
	GrafPtr			port;			/* port to draw into */
	MenuHandle		menu;			/* handle to menu */
	
	/* offscreen bitmap handles */
	RgnHandle		utilRgn;		/* utility region */
	Handle			baseAddr;	/* bitmap's base address */
	
	/* internal state information */
	PopupRectanglesType r;		/* rectangles enclosing areas of the popup menu */
	struct {
		PopupEnvType oldenv; 	/* old drawing environment */
		MenuHandle	selection;	/* menu containing the currently selected item */
		short			current;		/* currently selected item number */
		Boolean		envset:1;	/* true after drawing environment has been setup */
		Boolean		gotmenu:1;	/* used by CDEF; true means CDEF created menu */
	} state;
	
	/* User settable attributes of the menu. External functions
		are provided to manipulate these fields. */
	struct {
		void			*data;		/* pointer to application defined data */
		Boolean		draw:1;		/* false disables drawing and calculating */
		Boolean		visible:1;	/* true means popup is visible */
		Boolean		enabled:1;	/* true if menu is enabled */
		Boolean		typein:1;	/* true shows only arrow, like a type-in menu */
		Boolean		wfont:1;		/* true uses window font, not system font */
		Boolean		fixedwidth:1;/* true uses fixed width for menu */
		char			mark;			/* character used to mark current item */
		char			just;			/* text justification */
		struct {
			Style		style;		/* style of popup's title */
			short		width;		/* width of title; 0 if resized dynamically */
			Handle	str;			/* popup's title string */
		} title;
	} attr;
} PopupType, *PopupPtr, **PopupHandle;

Boolean PopupValid(PopupHandle popup);

void PopupCalculate(PopupHandle popup);
void PopupDraw(PopupHandle popup);
void PopupHilite(PopupHandle popup);
void PopupSelect(PopupHandle popup);
Boolean PopupWithin(PopupHandle popup, Point pt);

short PopupVersion(PopupHandle popup);
short PopupCurrent(PopupHandle popup);
void PopupCurrentSet(PopupHandle popup, short current);
void PopupDrawSet(PopupHandle popup, Boolean draw);
void PopupVisibleSet(PopupHandle popup, Boolean visible);
void PopupMarkSet(PopupHandle popup, char mark);
void PopupEnableSet(PopupHandle popup, Boolean enabled);
void PopupTypeInSet(PopupHandle popup, Boolean typein);
void PopupBounds(PopupHandle popup, Rect *bounds);
void PopupBoundsSet(PopupHandle popup, const Rect *bounds);
void PopupTitle(PopupHandle popup, Str255 title);
void PopupTitleSet(PopupHandle popup, const Str255 title);
void PopupTitleWidthSet(PopupHandle popup, short width);
void PopupTitleStyleSet(PopupHandle popup, Style style);
void PopupUseWFontSet(PopupHandle popup, Boolean wfont);
void PopupFixedWidthSet(PopupHandle popup, Boolean fixedwidth);
void PopupJustSet(PopupHandle popup, short just);

PopupHandle PopupBegin(GrafPtr port, MenuHandle menu, const Rect *bounds);
void PopupEnd(PopupHandle popup);

pascal long PopupCDEF(short var, ControlHandle ctl, short msg, long param);
void PopupCDEFAttach(ControlHandle ctl);
void PopupCDEFDetach(ControlHandle ctl);

const /* EventTableType */ void *PopupEventTable(void);
void PopupEventTableRegister(void);
