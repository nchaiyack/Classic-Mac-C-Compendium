/* See the file Distribution for distribution terms.
	(c) Copyright 1994 Ari Halberstadt */

/*	Functions implementing a popup menu. To create a popup call
	PopupBegin and then PopupCurrentSet to set the initial item.
	Use PopupCurrent to find out which item was selected. There
	are quite a few other functions for controling how the popup
	is displayed: whether the popup is drawn, whether the popup
	is visible, enabling and disabling the popup, whether to
	use the window font to display the popup, setting the text
	style for the title, and more. The defaults are set by
	PopupBegin to be those recommended by Apple in IM-VI. This
	library is the basis for a popup menu 'CDEF'.
	
	You can use the popup CDEF to create a popup menu. The control's
	contrlData field will contain a handle to the popup. If you need to
	directly modify the popup, you can call the popup library routines
	on the handle. However, first call PopupVersion and make sure it
	is equal to kPopupVersion. If it isn't, then don't use the popup
	library to modify the control. In practice, it is better to use
	Control Manager routines to modify the popup.
	
	If you implement keyboard equivalents of menu commands, then you can
	use PopupHilite to hilite the title of the menu while executing the
	menu command.
	
	93/03/15 aih
	- The current selection will display items with small icons properly.
	Formerly, this would display a large icon for items that had small or
	reduced icons, and wouldn't display the large icon once a different icon
	had been selected. Reduced icons still aren't displayed in the current
	selection, and I'm not sure why not, since I copy the command key character
	for reduced icons (0x1D) to the menu item used to draw the current
	selection.
	- When the useWFont variation code is used, the menu will be drawn in
	the font and size of the popup menu's port. Formerly, using the useWFont
	variation code produced incorrect results, but I fixed this by setting the
	system font and size low-memory globals instead of trying to change the
	font and size of the Window Manager's port.
	- The currently selected item is drawn in gray if it is disabled. Formerly,
	the item was only drawn in gray if it was disabled and if the menu item
	was drawn using the MDEF.
	- Updated "To Do" list.
	- Clicking in the popup's title will also pull down the menu. This
	is consistent with the operation of the system 7 popup CDEF.
	- Added support for the popupFixedWidth variation code.
	
	93/12/28 aih
	- Continued cleaning up code, added some more comments.
	- Uses menu definition function to draw the current selection. This
	eliminated a couple of rectangles from the popup structure and
	means that the current selection will always be drawn correctly,
	even if it includes icons and command keys. A special one item menu
	is used to hold a copy of the currently selected menu item. This special
	menu is then used to calculate the height of the item and to draw the
	item.
	
	93/12/26 aih
	- Major overhaul. Rewrote rectangle calculation code, since it wasn't
	working right. It's still not as simple as I'd like, but it's better
	than before. Also added attributes to make more compatible with Apple's
	popup CDEF (such as using the window font to display the menu).
	
	93/12/24 aih
	- Converted to use handles.
	- Removed dependence on all other libraries, so that this library
	is completely self contained; the few external functions that were
	necessary (e.g., strcpy, pstrfit) were copied or coded directly into
	this file. This will make this library more suitable for use as a
	CDEF since the linker won't pull in a lot of extra code from other
	libraries.
	
	93/12/23 aih
	- updated for current version of libraries
	- added port parameter to PopupBegin
	
	91/11/11 aih
	- Removed useless "PopupObj..." stuff
	
	91/03/01-05 aih
	- Update events are handled
	- Added comment describing this file
	- The popup's title is allocated as a handle in the heap
	- The popup is allocated as a handle in the heap
	- Attribute values are only updated if nescessary
	- The function PopupDraw first draws the popup to an offscreen bitmap
	and then copies it to the popup's port. This eliminates flicker when
	the popup is used as a CDEF, since the Control Manager sends a draw
	message to a control whenever the control is clicked, even if no
	drawing is actually needed.
	
	91/01/05 aih
	- Inserted this standard header in all files

	90/12/15 Ari Halberstadt (aih)
	- Created this file */

#include <Script.h>
#include "PopupLib.h"

/*---------------------------------------------------------------------------*/
/* constants */
/*---------------------------------------------------------------------------*/

/* To support drawing of icons and other special menu items in the
	current selection box, we create a special private menu containing
	as its sole item a copy of the currently selected menu item. We
	then call the menu definition function to draw the menu. If this
	feature is disabled, only the text of the current menu item is
	drawn. */
#define DONT_USE_MDEF				(0)
	
#ifndef NDEBUG

	/* Define DRAW_RECTANGLES as 1 to enable framing of the various rectangles
		of the menu. This is useful when debugging since it clearly displays the
		rectangles, which otherwise can be tricky to calculate. */
	#define DRAW_RECTANGLES			(0)

	/* To reduce flicker the popup is normally first drawn to an off-screen
		bitmap, then copied to the screen. To make debugging easier disable
		offscreen drawing. Then you can step through each draw routine to
		ensure that it is drawing correctly. */
	#define DONT_DRAW_OFFSCREEN	(0)
	
	/* Another useful debugging trick is to define the following as something
		greater than 1. This helps spot and fix off-by-one errors caused by
		not taking into account the size of the frame and drop shadow. 
		A value of at least 1/4" (18 pixels) provides good visual feedback
		and allows using a ruler to get rough measurments of areas. */
	#define kFrameSize				(1)	/* width of frame around popup */
	#define kShadowSize				(1)	/* width of shadow around popup */
	
#else /* NDEBUG */

	#define DRAW_RECTANGLES			(0)	/* if 1, draws frames around areas */
	#define DONT_DRAW_OFFSCREEN	(0)	/* if 1, doesn't use offscreen bitmap */
	#define kFrameSize				(1)	/* width of frame around popup */
	#define kShadowSize				(1)	/* size of shadow around popup */
	
#endif /* NDEBUG */

#define kEllipses						'É'	/* character appended to long strings */
#define kPopupMenuID					(1)	/* id of popup's private menu */
#define kShadowOffset				(3)	/* amount to offset shadow from frame */
#define kArrowWidth					(11)	/* width of the down arrow */
#define kArrowHeight					(6)	/* height of the down arrow */
#define kArrowMargin					(5)	/* margin around arrow */
#define kTitleMargin					(5)	/* margin around title */
#define kTitleMarginBottom			(1)	/* margin below title */

/* special values for the command key field of a menu item */
enum {
	kCmdHier = hMenuCmd,
	kCmdScript,
	kCmdIconReduced,
	kCmdIconSmall,
	kCmdReserved
};

/*---------------------------------------------------------------------------*/
/* low-memory globals */
/*---------------------------------------------------------------------------*/

#define TopMenuItem	(0x0A0A)
#define AtMenuBottom	(0x0A0C)
#define SysFontFam	(0x0BA6)
#define SysFontSiz	(0x0BA8)

static void LMSetTopMenuItem(short top)
{
	*(short *) TopMenuItem = top;
}

static void LMSetAtMenuBottom(short bottom)
{
	*(short *) AtMenuBottom = bottom;
}

static short LMGetSysFontFam(void)
{
	return(*(short *) SysFontFam);
}

static void LMSetSysFontFam(short font)
{
	*(short *) SysFontFam = font;
}

static short LMGetSysFontSiz(void)
{
	return(*(short *) SysFontSiz);
}

static void LMSetSysFontSiz(short font)
{
	*(short *) SysFontSiz = font;
}

/*---------------------------------------------------------------------------*/
/* utilities, copied from various libraries to reduce code overhead in CDEF */
/*---------------------------------------------------------------------------*/

#define HandleValidSize(h, n)	(true)
#define MenuValid(m)				(true)
#define require(x)				((void) 0)
#define ensure(x)					((void) 0)
#define check(x)					((void) 0)

/* return minimum of two numbers */
static long min(long a, long b)
{
	return(a < b ? a : b);
}

/* return maximum of two numbers */
static long max(long a, long b)
{
	return(a > b ? a : b);
}

/* fit string to width by truncating it and adding the extra character,
	return width of string */
static short pstrfit(Str255 str, short maxwidth, char extra)
{
	short extrawidth;
	short width;

	require(maxwidth >= 0);
	width = StringWidth(str);
	if (width > maxwidth) {
		if (maxwidth == 0) {
			/* optimization */
			width = 0;
			*str = 0;
		}
		else {
			/* truncate one character (or multi-byte character)
				at a time */
			while (*str > 0) {
				str[*str] = extra;
				width = StringWidth(str);
				if (width <= maxwidth)
					break;
				while (CharByte((Ptr) str, *str) > 0) {
					check(*str > 1);
					--*str;
				}
				check(*str > 0);
				--*str;
			}
		}
	}
	ensure(width <= maxwidth);
	ensure(width == StringWidth(str));
	return(width);
}

/* get the text attributes of the current port */
static void GetTextState(TextState *text)
{
	GrafPtr port;
	
	GetPort(&port);
	text->font = port->txFont;
	text->face = port->txFace;
	text->mode = port->txMode;
	text->size = port->txSize;
}

/* set the text attributes of the current port */
static void SetTextState(const TextState *text)
{
	TextFont(text->font);
	TextFace(text->face);
	TextMode(text->mode);
	TextSize(text->size);
}

/* set the text attributes of the current port to their defaults */
static void TextNormal(void)
{
	TextFont(0);
	TextFace(0);
	TextSize(0);
	TextMode(srcOr);
}

/* Flip the rectangle 'flip' horizontally (mirror image) relative to
	the rectangle 'within'. */	
static void RectFlip(Rect *flip, const Rect *within)
{
	short offset;
	
	require(RectValid(flip));
	require(RectValid(within));
	require(within->left <= flip->left && flip->right <= within->right);
	offset = (within->right - flip->right) - (flip->left - within->left);
	flip->left += offset;
	flip->right += offset;
}

/* paint over the rectangle with the gray pattern */
static void RectDisable(const Rect *r)
{
	PenState pen;
	Pattern pat;

	require(RectValid(r));
	GetIndPattern(pat, sysPatListID, 4);
	GetPenState(&pen);
	PenPat(pat);
	PenMode(patBic);
	PaintRect(r);
	SetPenState(&pen);
}

/* true if the menu item is enabled */
static Boolean MenuItemEnabled(MenuHandle menu, short item)
{
	require(MenuValid(menu));
	require(0 <= item && item <= CountMItems(menu));
	return(item > 31 || ((**menu).enableFlags & (1 << item)) != 0);
}

/*---------------------------------------------------------------------------*/
/* routines for executing a menu definition procedure */
/*---------------------------------------------------------------------------*/

typedef pascal void (*MenuProcPtr)(short msg, MenuHandle menu, Rect *bounds,
	Point hit, short *which);

static void mdef(short msg, MenuHandle menu, Rect *bounds,
	Point hit, short *which)
{
	Handle mdef = (**menu).menuProc;
	SignedByte state = HGetState(mdef);
	HLock(mdef);
	((MenuProcPtr) *mdef)(msg, menu, bounds, hit, which);
	HSetState(mdef, state);
}

static void mdef_draw(MenuHandle menu, const Rect *bounds)
{
	Point hit = { 0, 0 };
	short which = 0;
	Rect tmp = *bounds;
	
	mdef(mDrawMsg, menu, &tmp, hit, &which);
}

/*---------------------------------------------------------------------------*/
/* popup menu routines */
/*---------------------------------------------------------------------------*/

/* true if the popup is valid */
Boolean PopupValid(PopupHandle popup)
{
	if (! HandleValidSize(popup, sizeof(PopupType))) return(false);
	if (! MenuValid((**popup).menu)) return(false);
	if ((**popup).version != kPopupVersion) return(false);
	return(true);
}

/*---------------------------------------------------------------------------*/
/* port setup and restore */
/*---------------------------------------------------------------------------*/
	
/* The menu definition function gets the font size in which to draw the
	popup menu from the font size of the window manager port. So, to
	set the system font, in addition to setting a low-memory global,
	we also have to set the font size for the window manager port. */
static void SetSysFontSize(short size)
{
	GrafPtr svport;
	GrafPtr wmgrPort;
	
	GetPort(&svport);
	GetWMgrPort(&wmgrPort);
	SetPort(wmgrPort);
	TextSize(size);
	SetPort(svport);
}

/* Remember the current drawing environment and set the drawing environment 
	to that needed by the popup menu. Must be balanced with a call
	to PopupPortRestore. */
static void PopupPortSetup(PopupHandle popup)
{
	GrafPtr port;
	PenState pen;
	TextState text;
	
	require(! (**popup).state.envset);	
	
	/* remember current port */
	GetPort(&port);
	(**popup).state.oldenv.port = port;

	/* save settings for popup's port */
	SetPort((**popup).port);
	GetPenState(&pen);
	GetTextState(&text);
	(**popup).state.oldenv.pen = pen;
	(**popup).state.oldenv.text = text;
	
	/* reset text settings to default system settings */
	TextNormal();
	
	/* if using the window's font, use font and font size of popup's port */
	if ((**popup).attr.wfont) {
		TextFont(text.font);
		TextSize(text.size);
	}
	
	/* If using the window's font, then set the system font and size
		to the font and size of the popup's port. */
	if ((**popup).attr.wfont) {
		(**popup).state.oldenv.sysfont = LMGetSysFontFam();
		(**popup).state.oldenv.syssize = LMGetSysFontSiz();
		LMSetSysFontFam((**popup).state.oldenv.text.font);
		LMSetSysFontSiz((**popup).state.oldenv.text.size);
		SetSysFontSize((**popup).state.oldenv.text.size);
	}
	
	/* save clip region and clip to the popup's maximum bounding rectangle */
	if ((**popup).state.oldenv.clip) {
		Rect maxbounds = (**popup).r.maxbounds;
		GetClip((**popup).state.oldenv.clip);
		ClipRect(&maxbounds);
	}
		
	(**popup).state.envset = true;
	ensure((**popup).state.envset);
}

/* Restore the drawing environment to its original state. Must be
	balanced with a call to PopupPortSetup. */
static void PopupPortRestore(PopupHandle popup)
{
	GrafPtr port;
	PenState pen;
	TextState text;
	GrafPtr wmgrPort;
	TextState wmgrText;

	require((**popup).state.envset);
	port = (**popup).state.oldenv.port;
	pen = (**popup).state.oldenv.pen;
	text = (**popup).state.oldenv.text;
	if ((**popup).attr.wfont) {
		LMSetSysFontFam((**popup).state.oldenv.sysfont);
		LMSetSysFontSiz((**popup).state.oldenv.syssize);	
		SetSysFontSize((**popup).state.oldenv.syssize);
	}
	if ((**popup).state.oldenv.clip) {
		SetClip((**popup).state.oldenv.clip);
		SetEmptyRgn((**popup).state.oldenv.clip);
	}
	SetTextState(&text);
	SetPenState(&pen);
	SetPort(port);	
	(**popup).state.envset = false;	
	ensure(! (**popup).state.envset);
}

/*---------------------------------------------------------------------------*/
/* rectangle calculation routines */
/*---------------------------------------------------------------------------*/

/* copy the current item from the main menu to a private menu; we can
	then use the private menu to determine the size of the menu item
	and to draw the menu item */
static void PopupAdjustMenu(PopupHandle popup)
{
	const StringPtr notempty = (StringPtr) "\p ";
	MenuHandle menu;
	Str255 item;
	Style style;
	short icon;
	short cmd;
	
	if (! (**popup).state.selection) {
		/* create the private menu */
		check(*notempty);
		menu = NewMenu(kPopupMenuID, notempty);
		if (menu) {
			(**popup).state.selection = menu;
			AppendMenu(menu, notempty);
		}
	}
	if ((**popup).state.selection) {
		/* Copy the current selection's attributes, except for the item's
			mark, which doesn't need to be displayed. The command key is
			only copied if it specifies a special icon or other feature
			of the menu item. */
		GetItem((**popup).menu, (**popup).state.current, item);
		GetItemCmd((**popup).menu, (**popup).state.current, &cmd);
		GetItemIcon((**popup).menu, (**popup).state.current, &icon);
		GetItemStyle((**popup).menu, (**popup).state.current, &style);
		if (*item) { /* zero length item string is not allowed in menus */
			SetItem((**popup).state.selection, 1, item);
			SetItemIcon((**popup).state.selection, 1, icon);
			SetItemStyle((**popup).state.selection, 1, style);
			if (cmd == 0 ||
				 cmd == kCmdScript ||
				 cmd == kCmdIconReduced ||
				 cmd == kCmdIconSmall)
			{
				/* The command key values 0x1B through 0x1F are reserved for use
					by Apple. The command key value 0x1B indicates a heirarchical
					menu item that has a triangle to its right, which we are not
					interested in displaying in the current selection. The other
					command key characters indicate things like small or reduced
					icons, which we do want to display in the current selection.
					A command key value of 0 indicates the lack of any command
					key. */
				SetItemCmd((**popup).state.selection, 1, cmd);
			}
			if (MenuItemEnabled((**popup).menu, (**popup).state.current))
				EnableItem((**popup).state.selection, 1);
			else
				DisableItem((**popup).state.selection, 1);
		}
	}
	ensure(! (**popup).state.selection ||
				CountMItems((**popup).state.selection) == 1);
}

/* set popup's rectangles and erase old popup if the frame has changed */
static PopupRectanglesSet(PopupHandle popup, const PopupRectanglesType *r)
{
	Rect bounds;
	RgnHandle eraseRgn;
	
	bounds = (**popup).r.bounds;
	if (! EqualRect(&bounds, &r->bounds)) {
		eraseRgn = NewRgn();
		if (eraseRgn && (**popup).utilRgn) {
			/* to reduce flicker only erase the
				area that doesn't overlap */
			check(EmptyRgn((**popup).utilRgn));
			RectRgn(eraseRgn, &bounds);
			RectRgn((**popup).utilRgn, &r->bounds);
			DiffRgn(eraseRgn, (**popup).utilRgn, eraseRgn);
			EraseRgn(eraseRgn);
			DisposeRgn(eraseRgn);
			SetEmptyRgn((**popup).utilRgn);
		}
		else
			EraseRect(&bounds);
	}
	(**popup).r = *r;
}

/* calculate the popup's rectangles */
void PopupCalculate(PopupHandle popup)
{
	struct {								/* margins around some items */
		struct { short left, right; } selection;
		struct { short left, right; } title;
		struct { short left, right; } arrow;
	} margin = {
		{ kFrameSize, 0 },			/* selection */
		{ kTitleMargin, kTitleMargin },	/* title */
		{ 0, kArrowMargin },			/* arrow */
	};
	struct {								/* minimum widths of each item */
		short content;
		short hilite;
		short title;
		short arrow;
		short selection;
	} minwidth;
	short lineHeight;					/* height of a line */
	short menuHeight;					/* height of the selected menu item */
	short menuWidth;					/* width of the menu */
	short maxwidth;					/* for calculating maximum widths of items */
	short width;						/* for calculating widths of items */
	FontInfo font;						/* info about the current font size */
	Str255 title;						/* the popup's title */
	Rect content;						/* rectangle enclosing content (excludes frame) */
	PopupRectanglesType r;			/* the calculated rectangles */
	
	/* don't calculate if drawing is turned off */
	if (! (**popup).attr.draw) return;
	
	/* initialize settings */
	PopupPortSetup(popup);
	PopupTitle(popup, title);

	/* The following calls to CalcMenuSize and GetFontInfo have a bizzare
		requirement. When using a window font other than the system font
		(i.e., using the popupUseWFont variation code) the first call to
		CalcMenuSize would calculate the menu's size using the standard
		system font, instead of the font that the CDEF specified. The
		second call to CalcMenuSize calculates the menu's size using
		the correct font size. Similarly, if GetFontInfo were called before
		CalcMenuSize were called, GetFontInfo would return information about
		the wrong font. This seems either like a bug in the OS (both systems
		6.0.5 and 7.0), or an attempt by Apple to work around some other
		bug in the OS. It took me a long time to figure out a work-around
		for this problem. */
		
	/* get width of menu, including the width of the down arrow */
	CalcMenuSize((**popup).menu);
	CalcMenuSize((**popup).menu);
	menuWidth = (**(**popup).menu).menuWidth + kArrowWidth + kArrowMargin;

	/* get information about the current font */
	GetFontInfo(&font);
	lineHeight = font.ascent + font.descent + font.leading;
	
	/* calculate height of current selection */
	menuHeight = 0;
	PopupAdjustMenu(popup);
	if ((**popup).state.selection) {
		CalcMenuSize((**popup).state.selection);
		menuHeight = (**(**popup).state.selection).menuHeight;
	}
	/* the height must be at least as large as a minimum height */
	if (menuHeight < lineHeight)
		menuHeight = lineHeight;
	if (menuHeight < kArrowHeight + 4)
		menuHeight = kArrowHeight + 4;
		
	/* adjust margins */
	
	if (! *title || (**popup).attr.typein) {
		/* Type-in popups don't display the title or the current selection,
			so their margins aren't needed. If the title is empty then the
			title's margins can also be empty. */
		if ((**popup).attr.typein) {
			margin.arrow.left = 2;
			margin.arrow.right = 2;
			menuHeight = lineHeight;
			font.widMax = 0;
		}
		margin.title.left = 0;
		margin.title.right = 0;
	}
	
	/* for right justified popups we need a little extra space for the drop
		shadow between the current selection area and the title */
	if ((**popup).attr.just == teFlushRight)
		margin.selection.left += kShadowSize;
		
	/* calculate minimum widths */
	
	minwidth.arrow = kArrowWidth;
	minwidth.title = (*title ? font.widMax : 0);
	minwidth.hilite = minwidth.title + margin.title.left + margin.title.right;
	minwidth.selection = font.widMax + minwidth.arrow + margin.arrow.left + margin.arrow.right;
	minwidth.content = minwidth.hilite + minwidth.selection +
							margin.selection.left + margin.selection.right;
	
	/* calculate rectangles */
	
	/* copy maxbounds rectangle */
	r.maxbounds = (**popup).r.maxbounds;
	
	/* calculate content rectangle; all the other rectangles will be
		calculated relative to this rectangle */
	content.top = r.maxbounds.top + kFrameSize;
	content.left = r.maxbounds.left + kFrameSize;
	content.bottom = content.top + menuHeight;
	content.right = content.left +
		max(r.maxbounds.right - r.maxbounds.left - 2 * kFrameSize - kShadowSize,
			 minwidth.content);

	/* vertically center content in maxbounds */
	{	short offset = ((r.maxbounds.bottom - r.maxbounds.top) -
		 		 			 (content.bottom - content.top)) / 2;
		OffsetRect(&content, 0, max(0, offset - kShadowSize));
	}
	
	/* calculate hilite rectangle */
	r.hilite = content;
	r.hilite.left = content.left;
	r.hilite.right = content.right - minwidth.selection;

	/* calculate title rectangle */
	check(content.bottom - content.top >= lineHeight);
	r.title.top = content.top + (content.bottom - content.top - lineHeight) / 2;
	r.title.left = content.left + margin.title.left;
	r.title.bottom = r.title.top + lineHeight - kTitleMarginBottom;
	/* calculate width of title rectangle */
	if ((**popup).attr.typein || ! *title)
		width = 0; /* don't show title */
	else {
		/* popup has a title */
		maxwidth =
			r.hilite.right - r.hilite.left -
			margin.title.left - margin.title.right;
		check(maxwidth >= minwidth.title);
		if ((**popup).attr.title.width) {
			/* use fixed title width as specified by application */
			width = min(maxwidth, (**popup).attr.title.width);
			width = max(width, minwidth.title);
		}
		else {
			/* use actual width of title string */
			Style style = (**popup).port->txFace;
			TextFace((**popup).attr.title.style);
			width = pstrfit(title, maxwidth, kEllipses);
			TextFace(style);
		}
		check(width <= maxwidth);
	}
	check(width >= minwidth.title);
	r.title.right = r.title.left + width;

	/* adjust right edge of hilite rectangle now that width of title is known */
	r.hilite.right = r.title.right + margin.title.right;
	
	/* calculate selection rectangle, initially using the maximum possible width */
	r.selection = content;
	r.selection.left = r.hilite.right + margin.selection.left;
	r.selection.right = content.right - margin.selection.right;

	/* calculate width of current selection */
	if ((**popup).attr.typein)
		width = minwidth.selection;
	else if ((**popup).attr.fixedwidth)
		width = max(r.selection.right - r.selection.left, minwidth.selection);
	else {
		maxwidth = r.selection.right - r.selection.left;
		check(maxwidth >= minwidth.selection);
		width = min(maxwidth, menuWidth);
		width = max(width, minwidth.selection);
	}

	/* adjust right edge now that we know how wide everything is */
	r.selection.right = r.selection.left + width;
	content.right = r.selection.right + margin.selection.right;
	
	/* calculate arrow rectangle--center arrow vertically at right edge of
		selection rectangle */
	check(content.bottom - content.top >= kArrowHeight);
	r.arrow.top = content.top + (content.bottom - content.top - kArrowHeight) / 2;
	r.arrow.left = r.selection.right - kArrowWidth - margin.arrow.right;
	r.arrow.bottom = r.arrow.top + kArrowHeight;
	r.arrow.right = r.arrow.left + kArrowWidth;

	/* calculate frame rectangle--surrounds content area, and includes the
		frame and shadow */
	r.bounds.top = content.top - kFrameSize;
	r.bounds.left = content.left - kFrameSize;
	r.bounds.right = content.right + kFrameSize;
	r.bounds.bottom = content.bottom + kFrameSize + kShadowSize;
	if ((**popup).attr.just != teFlushRight)
		r.bounds.right += kShadowSize; /* in teFlushRight shadow is part of content */
	
	/* mirror image rectangles if using a right justified menu */
	if ((**popup).attr.just == teFlushRight) {
		Rect maxbounds;
		
		/* copy and adjust maxbounds so flipping will work */
		maxbounds.top = r.maxbounds.top;
		maxbounds.left = r.maxbounds.left;
		maxbounds.bottom = r.maxbounds.top +
			max(r.maxbounds.bottom - r.maxbounds.top,
				 r.bounds.bottom - r.bounds.top);
		maxbounds.right = r.maxbounds.left +
			max(r.maxbounds.right - r.maxbounds.left,
				 r.bounds.right - r.bounds.left);

		/* flip the rectangles */
		RectFlip(&r.bounds, &maxbounds);
		RectFlip(&r.hilite, &maxbounds);
		RectFlip(&r.selection, &maxbounds);
		RectFlip(&r.title, &maxbounds);
		RectFlip(&r.arrow, &maxbounds);
	}
	
	/* make sure everything's ok */
	check(RectWidth(&r.hilite) >= minwidth.hilite);
	check(RectWidth(&r.title) >= minwidth.title);
	check(RectWidth(&r.selection) >= minwidth.selection);
	check(RectWidth(&r.arrow) >= minwidth.arrow);
	check(RectWithin(&r.hilite, &r.bounds));
	check(RectWithin(&r.title, &r.hilite));
	check(RectWithin(&r.selection, &r.bounds));
	check(RectWithin(&r.arrow, &r.selection));
	check(! RectWithin(&r.hilite, &r.selection));
	check(! RectWithin(&r.selection, &r.hilite));

	/* set the popup's rectangles and restore the environment */
	PopupRectanglesSet(popup, &r);
	PopupPortRestore(popup);
}

/*---------------------------------------------------------------------------*/
/* more drawing routines */
/*---------------------------------------------------------------------------*/

/* draw the frame around the popup menu */
static void PopupDrawFrame(PopupHandle popup)
{
	Rect frame;
	PenState pen;
	
	require((**popup).state.envset);
	
	/* save state */
	GetPenState(&pen);
	
	/* draw frame */
	frame = (**popup).r.bounds;	
	if ((**popup).attr.just == teFlushRight)
		frame.right = (**popup).r.selection.right + kFrameSize + kShadowSize;
	else
		frame.left = (**popup).r.selection.left - kFrameSize;
	frame.right -= kShadowSize;
	frame.bottom -= kShadowSize;
	PenSize(kFrameSize, kFrameSize);
	FrameRect(&frame);

	/* draw drop shadow */
	PenSize(kShadowSize, kShadowSize);
	MoveTo(frame.right, frame.top + kShadowOffset);
	LineTo(frame.right, frame.bottom);
	LineTo(frame.left + kShadowOffset, frame.bottom);
	
	/* restore state */
	SetPenState(&pen);
}

/* draw the down arrow */
static void PopupDrawArrow(PopupHandle popup)
{
	Rect arrow;		/* arrow's rectangle */
	short	height;	/* height of arrow */
	short	width;	/* width of current line */
	short	i;			/* index to lines of arrow */
	
	require((**popup).state.envset);
	arrow = (**popup).r.arrow;
	height = kArrowHeight;
	width = kArrowWidth - 1;
	for (i = 0; i < height; i++) {
		MoveTo(arrow.left + i, arrow.top + i);
		LineTo(arrow.left + i + width, arrow.top + i);
		width -= 2;
	}
}

/* draw string within bounds; if too long truncate and append ellipses */
static void PopupDrawString(PopupHandle popup, Str255 str, const Rect *bounds)
{
	FontInfo	font;
	short width;
	
	require((**popup).state.envset);
	require(RectValid(bounds));
	GetFontInfo(&font);
	width = pstrfit(str, bounds->right - bounds->left, kEllipses);
	if ((**popup).attr.just == teFlushRight)
		MoveTo(bounds->right - width, bounds->bottom - font.descent);
	else
		MoveTo(bounds->left, bounds->bottom - font.descent);
	DrawString(str);
}

/* Draw the current selection string within the specified rectangle
	by calling the menu definition function. When the menu definition
	function is used to draw the current selection, any icons or other
	attributes associated with the menu item are also drawn. */
static void PopupDrawSelectionMDEF(PopupHandle popup, const Rect *bounds)
{	
	require((**popup).state.envset);
	require(RectValid(bounds));

	/* set globals that determine position of menu */
	LMSetTopMenuItem(bounds->top);
	LMSetAtMenuBottom(bounds->bottom);
	
	/* draw the current selection using the menu definition function */
	mdef_draw((**popup).state.selection, bounds);
}

/* Draw the current selection string within the specified rectangle
	without calling the menu definition function. This will only draw
	the text of the current selection, not any associated icons
	or other attributes, though the text attributes will be set
	to match the text attributes of the menu item. */
static void PopupDrawSelectionString(PopupHandle popup, const Rect *bounds)
{
	Str255 item;		/* current selection string */
	FontInfo font;		/* information about the current font */
	Style itemStyle;	/* menu item's style */
	Style portStyle;	/* port's style */
	Rect selection;	/* rectangle to draw string in */
	
	require((**popup).state.envset);
	
	/* allign the string with the title string and with the string
		in the menu item */
	GetFontInfo(&font);
	selection.left = bounds->left;
	selection.right = bounds->right;
	selection.top = (**popup).r.title.top;
	selection.bottom = (**popup).r.title.bottom;
	if ((**popup).attr.just == teFlushRight)
		selection.right -= font.widMax;
	else
		selection.left += font.widMax;
	
	/* use the text style of the menu item and draw the item */
	GetItem((**popup).menu, (**popup).state.current, item);
	GetItemStyle((**popup).menu, (**popup).state.current, &itemStyle);
	portStyle = (**popup).port->txFace;
	TextFace(itemStyle);
	PopupDrawString(popup, item, &selection);
	TextFace(portStyle);
	
	/* gray over selection if item is disabled */
	if (! MenuItemEnabled((**popup).menu, (**popup).state.current))
		RectDisable(&selection);
}

/* draw the current selection by calling the menu definition function */
static void PopupDrawSelection(PopupHandle popup)
{
	Rect selection;				/* current selection rectangle */
	Boolean usemdef = false;	/* if true, selection is drawn using MDEF */

	require((**popup).state.envset);
	require(! (**popup).attr.typein);
	
	/* exclude arrow from selection rectangle */
	selection = (**popup).r.selection;
	if ((**popup).attr.just == teFlushRight)
		selection.left += kArrowWidth + kArrowMargin;
	else
		selection.right -= kArrowWidth + kArrowMargin;
	check(RectValid(&selection));
	
	if ((**popup).state.selection) {
		/* The menu definition function doesn't respect the right edge of the
			bounding rectangle. If the menu is wider than the bounding rectangle,
			it is simply drawn over whatever happens to be beyond the rectangle's
			right edge. Since this wouldn't look very nice, we only use the
			menu definition function if the menu would fit in the current
			selection rectangle. Merely setting the clip region to the selection
			rectangle wouldn't be sufficient, since long strings should be
			truncated with an ellipses character. */
		if ((**(**popup).state.selection).menuWidth <=
			  selection.right - selection.left)
		{
			#if ! DONT_USE_MDEF
				usemdef = true;
			#endif /* DONT_USE_MDEF */
		}
	}
	
	/* draw current selection */
	if (usemdef)
		PopupDrawSelectionMDEF(popup, &selection);
	else
		PopupDrawSelectionString(popup, &selection);
}

/* draw the title */
static void PopupDrawTitle(PopupHandle popup)
{
	Style style;
	Str255 title;
	Rect rtitle;
	
	require((**popup).state.envset);
	require(! (**popup).attr.typein);
	rtitle = (**popup).r.title;
	style = (**popup).port->txFace;
	TextFace((**popup).attr.title.style);
	PopupTitle(popup, title);
	PopupDrawString(popup, title, &rtitle);
	TextFace(style);
}

/* gray out menu if it's disabled */
static void PopupDrawEnable(PopupHandle popup)
{
	require(PopupValid(popup));
	require((**popup).state.envset);
	if (! (**popup).attr.enabled) {
		Rect bounds = (**popup).r.bounds;
		RectDisable(&bounds);
	}
}

/* erase the popup menu */
static void PopupErase(PopupHandle popup)
{
	Rect bounds;
	
	require((**popup).state.envset);
	bounds = (**popup).r.bounds;
	EraseRect(&bounds);
}

#ifdef DRAW_RECTANGLES
/* draw frames around the parts of the popup */
static void PopupDrawRectangles(PopupHandle popup)
{
	Rect shadow;
	PenState pen;
	Rect frame, r;

	GetIndPattern(black, sysPatListID, 1);
	GetIndPattern(dkGray, sysPatListID, 2);
	GetIndPattern(gray, sysPatListID, 4);
	GetIndPattern(white, sysPatListID, 20);

	GetPenState(&pen);
	
	/* draw grayed outline of frame and shadow */
	if (kFrameSize > 2 || kShadowSize > 2) {
		/* calculate frame */
		frame = (**popup).r.bounds;	
		if ((**popup).attr.just == teFlushRight)
			frame.right = (**popup).r.selection.right + kFrameSize + kShadowSize;
		else
			frame.left = (**popup).r.selection.left - kFrameSize;
		frame.right -= kShadowSize;
		frame.bottom -= kShadowSize;
		
		/* draw outline of frame */
		PenPat(gray);
		FrameRect(&frame);
		InsetRect(&frame, kFrameSize, kFrameSize);
		FrameRect(&frame);
		InsetRect(&frame, -kFrameSize, -kFrameSize);
		
		/* draw outline of shadow */
		shadow.top = frame.bottom;
		shadow.left = frame.left + kShadowOffset;
		shadow.bottom = shadow.top + kShadowSize;
		shadow.right = frame.right + kShadowSize;
		FrameRect(&shadow);
		shadow.top = frame.top + kShadowOffset;
		shadow.left = frame.right;
		shadow.bottom = frame.bottom + kShadowSize;
		shadow.right = shadow.left + kShadowSize;
		FrameRect(&shadow);
		PenNormal();
	}

	/* draw the other rectangles */
	r = (**popup).r.maxbounds; FrameRect(&r);
	r = (**popup).r.title; FrameRect(&r);
	r = (**popup).r.hilite; FrameRect(&r);
	r = (**popup).r.selection; FrameRect(&r);
	r = (**popup).r.arrow; FrameRect(&r);
	SetPenState(&pen);
}
#endif /* DRAW_RECTANGLES */

/* draw the menu */
static void PopupDrawDirect(PopupHandle popup)
{
	require(PopupValid(popup));
	PopupErase(popup);
	if ((**popup).attr.visible) {
	
		/* draw all the parts of the popup */
		if (! (**popup).attr.typein) {
			/* The title should be drawn before the selection is drawn since
				the mdef may change the port's text font, which would cause
				the title to be drawn in the system font even if the popup
				should use the window's font. */
			PopupDrawTitle(popup);
			PopupDrawSelection(popup);
		}
		PopupDrawFrame(popup);
		PopupDrawArrow(popup);
		PopupDrawEnable(popup);
		
		#if DRAW_RECTANGLES
			PopupDrawRectangles(popup);
		#endif /* DRAW_RECTANGLES */
	}
}

/* draw the popup to an offscreen bitmap, then copy the bitmap to the screen */
void PopupDraw(PopupHandle popup)
{
	BitMap	svbits;			/* port's saved bitmap */
	BitMap	offbits;			/* off screen bitmap */
	Boolean	reallocated;	/* if true, bitmap needs to be redrawn */
	long		baseAddrSize;	/* size of base address for bitmap */
	
	require(PopupValid(popup));
	if ( ! (**popup).attr.draw) return;
	
	/* setup drawing environment */
	PopupPortSetup(popup);
		
	/* setup offscreen bitmap */
	if ((**popup).baseAddr) {
		
		/* calculate bitmap dimensions */
		offbits.bounds = (**popup).r.bounds;
		offbits.rowBytes = ((((offbits.bounds.right - offbits.bounds.left) + 15) / 16) * 2);
		baseAddrSize = (long) offbits.rowBytes * (offbits.bounds.bottom - offbits.bounds.top);

		/* reallocate bitmap for drawing offscreen */
		reallocated = false;
		if (GetHandleSize((**popup).baseAddr) != baseAddrSize) {
			ReallocateHandle((**popup).baseAddr, baseAddrSize);
			reallocated = true;
		}
	}
	
	/* draw from the offscreen bitmap */
	if ((**popup).baseAddr && *(**popup).baseAddr && ! MemError()) {
		
		/* lock and dereference base address */
		MoveHHi((**popup).baseAddr);
		HLock((**popup).baseAddr);
		offbits.baseAddr = *(**popup).baseAddr;
	
		if (reallocated) {
			/* substitute offscreen bitmap for port's bitmap and draw the popup */
			GrafPtr port;
			GetPort(&port);
			SetPort((**popup).port);
			svbits = (**popup).port->portBits;
			SetPortBits(&offbits);
			PopupDrawDirect(popup);
			SetPortBits(&svbits);
			SetPort(port);
		}

		/* calculate mask region */
		if ((**popup).utilRgn) {
			check(EmptyRgn((**popup).utilRgn));
			CopyRgn((**popup).port->clipRgn, (**popup).utilRgn);
			SectRgn((**popup).port->visRgn, (**popup).utilRgn, (**popup).utilRgn);
		}
		
		/* copy the popup from the offscreen bitmap to the popup's port */
		CopyBits(&offbits, &(**popup).port->portBits,
			&offbits.bounds, &offbits.bounds, srcCopy, (**popup).utilRgn);
		
		if ((**popup).utilRgn)
			SetEmptyRgn((**popup).utilRgn);
		HUnlock((**popup).baseAddr);
	}
	else {
		/* couldn't allocate offscreen bitmap, but the popup can still be drawn */
		PopupDrawDirect(popup);
	}

	/* restore drawing environment */
	PopupPortRestore(popup);
}

/* hilite the popup's title; useful for keyboard equivalents of commands */
void PopupHilite(PopupHandle popup)
{
	GrafPtr port;
	Rect hilite;
	
	GetPort(&port);
	SetPort((**popup).port);
	hilite = (**popup).r.hilite;
	InvertRect(&hilite);
	SetPort(port);
}

/*---------------------------------------------------------------------------*/
/* event handling */
/*---------------------------------------------------------------------------*/

/* True if point (in local coordinates) is within the popup menu.
	Call this before calling PopupMouseDown. */
Boolean PopupWithin(PopupHandle popup, Point pt)
{
	Boolean result = false;
	
	require(PopupValid(popup));
	if ((**popup).attr.visible && (**popup).attr.enabled) {
		Rect selection = (**popup).r.selection;
		Rect hilite = (**popup).r.hilite;
		result = (PtInRect(pt, &selection) || PtInRect(pt, &hilite));
	}
	return(result);
}

/* call this when there's a mouse down in a popup menu */
void PopupSelect(PopupHandle popup)
{
	long chosen;			/* item selected from menu */
	Point	location;		/* top left of menu */
	Boolean inserted;		/* true if inserted menu into menu list */
	Rect selection;		/* current selection rectangle */
	short oldMenuWidth;	/* saved menu width */
	
	require(PopupValid(popup));
	if (StillDown()) {
	
		/* setup port */
		PopupPortSetup(popup);
		
		/* insert menu into heirarchical menu list if necessary */
		inserted = false;
		if (! GetMHandle((**(**popup).menu).menuID)) {
			InsertMenu((**popup).menu, -1);
			inserted = true;
		}
		
		/* hilite title */
		PopupHilite(popup);
		
		/* calculate position for popup menu */
		location.h = (**popup).r.selection.left;
		location.v = (**popup).r.selection.top;
		LocalToGlobal(&location);
				
		/* adjust width of menu */
		oldMenuWidth = (**(**popup).menu).menuWidth;
		if (! (**popup).attr.typein) {
			/* make the menu wide enough to include the down arrow */
			(**(**popup).menu).menuWidth += kArrowWidth + kArrowMargin;
			if ((**popup).attr.fixedwidth) {
				/* make menu fill all of current selection rectangle */
				short width = (**popup).r.selection.right - (**popup).r.selection.left;
				if (width > (**(**popup).menu).menuWidth)
					(**(**popup).menu).menuWidth = width;
			}
		}
		
		/* let user select an item from the menu */
		chosen = PopUpMenuSelect((**popup).menu, location.v, location.h,
										 (**popup).state.current);
		
		/* restore environment */
		(**(**popup).menu).menuWidth = oldMenuWidth;
		if (inserted)
			DeleteMenu((**(**popup).menu).menuID);
		PopupHilite(popup);
		PopupPortRestore(popup);
		
		/* display the selected item */
		if (LoWord(chosen))
			PopupCurrentSet(popup, LoWord(chosen));
	}
	ensure(PopupValid(popup));
}
	
/*---------------------------------------------------------------------------*/
/* getting and setting attributes */
/*---------------------------------------------------------------------------*/

/* recalculate and redraw the popup */
static void PopupChanged(PopupHandle popup)
{
	/* purge bitmap to force rebuilding it */
	if ((**popup).baseAddr)
		EmptyHandle((**popup).baseAddr);

	/* recalculate and redraw everything */
	PopupCalculate(popup);
	PopupDraw(popup);
}

/* return the version of the library that created the popup menu */
short PopupVersion(PopupHandle popup)
{
	return((**popup).version);
}

/* return the currently selected menu item */
short PopupCurrent(PopupHandle popup)
{
	require(PopupValid(popup));
	return((**popup).state.current);
}

/* set the currently selected menu item */
void PopupCurrentSet(PopupHandle popup, short current)
{
	require(PopupValid(popup));
	if (current != (**popup).state.current) {
		SetItemMark((**popup).menu, (**popup).state.current, noMark);
		SetItemMark((**popup).menu, current, (**popup).attr.mark);
		(**popup).state.current = current;
		PopupChanged(popup);
	}
}

/* turn drawing on or off */
void PopupDrawSet(PopupHandle popup, Boolean draw)
{
	require(PopupValid(popup));
	(**popup).attr.draw = draw;
}

/* make popup visible or invisible */
void PopupVisibleSet(PopupHandle popup, Boolean visible)
{	
	require(PopupValid(popup));	
	if (visible != (**popup).attr.visible) {
		(**popup).attr.visible = visible;
		PopupChanged(popup);
	}	
}

/* set the character used to mark the current menu item */
void PopupMarkSet(PopupHandle popup, char mark)
{
	require(PopupValid(popup));
	if (mark != (**popup).attr.mark) {
		(**popup).attr.mark = mark;
		SetItemMark((**popup).menu, (**popup).state.current, (**popup).attr.mark);
	}	
}

/* enable or disable the menu */
void PopupEnableSet(PopupHandle popup, Boolean enabled)
{
	require(PopupValid(popup));
	if (enabled != (**popup).attr.enabled) {
		(**popup).attr.enabled = enabled;
		PopupChanged(popup);
	}
}

/* turn type-in style menu on or off (IM-VI, p2-37) */
void PopupTypeInSet(PopupHandle popup, Boolean typein)
{
	require(PopupValid(popup));
	if (typein != (**popup).attr.typein) {
		(**popup).attr.typein = typein;
		PopupChanged(popup);
	}
}

/* return rectangle enclosing all of popup */
void PopupBounds(PopupHandle popup, Rect *bounds)
{
	require(PopupValid(popup));
	*bounds = (**popup).r.bounds;
	ensure(RectValid(bounds));
}

/* set popup's maximum bounding rectangle */
void PopupBoundsSet(PopupHandle popup, const Rect *maxbounds)
{
	Rect oldmax;
	
	require(PopupValid(popup));
	require(RectValid(maxbounds));
	oldmax = (**popup).r.maxbounds;
	if (! EqualRect(&oldmax, maxbounds)) {
		if ((**popup).attr.draw) {
			PopupPortSetup(popup);
			PopupErase(popup);
			PopupPortRestore(popup);
		}
		(**popup).r.maxbounds = *maxbounds;
		PopupChanged(popup);
	}
}

/* return popup's title */
void PopupTitle(PopupHandle popup, Str255 title)
{
	*title = 0;
	if ((**popup).attr.title.str) {
		BlockMove(*(**popup).attr.title.str, title,
			**(**popup).attr.title.str + 1);
	}
}

/* set popup's title  */
void PopupTitleSet(PopupHandle popup, const Str255 title)
{
	Str255 oldtitle;
	
	require(PopupValid(popup));
	if ((**popup).attr.title.str) {
		PopupTitle(popup, oldtitle);
		if (! EqualString(title, oldtitle, true, true)) {
			PtrToXHand(title, (**popup).attr.title.str, *title + 1);
			PopupChanged(popup);
		}
	}
}

/* set width of popup's title; the title is resized dynamically
	if the width is zero */
void PopupTitleWidthSet(PopupHandle popup, short width)
{
	require(width >= 0);
	if (width != (**popup).attr.title.width) {
		(**popup).attr.title.width = width;
		PopupChanged(popup);
	}
}

/* set the text style in which the popup's title will be drawn */
void PopupTitleStyleSet(PopupHandle popup, Style style)
{
	if (style != (**popup).attr.title.style) {
		(**popup).attr.title.style = style;
		PopupChanged(popup);
	}
}

/* set whether the popup will use the window's font for drawing the
	title, current selection, and menu */	
void PopupUseWFontSet(PopupHandle popup, Boolean wfont)
{
	if (wfont != (**popup).attr.wfont) {
		(**popup).attr.wfont = wfont;
		PopupChanged(popup);
	}
}

/* set whether the popup will use a fixed width or will be resized
	dynamically */
void PopupFixedWidthSet(PopupHandle popup, Boolean fixedwidth)
{
	if (fixedwidth != (**popup).attr.fixedwidth) {
		(**popup).attr.fixedwidth = fixedwidth;
		PopupChanged(popup);
	}
}

/* set the justification style for drawing the popup */
void PopupJustSet(PopupHandle popup, short just)
{
	if (just != (**popup).attr.just) {
		(**popup).attr.just = just;
		PopupChanged(popup);
	}
}

/*---------------------------------------------------------------------------*/
/* allocation and disposal */
/*---------------------------------------------------------------------------*/

/*	Create a popup menu within the rectangle in the specified port.
	Drawing is initially off. Since the popup's title is initially
	empty, you should call PopupTitleSet if you want the popup to
	have a title. When you're finished configuring the popup call
	PopupDrawSet to enable drawing and then call PopupCalculate.
	The popup's rectangles are only calculated when drawing
	is enabled. The popup menu allocates several utility handles,
	but will function, albeit not as well, even if it can't allocate
	any of the utility handles. The popup menu is drawn to an offscreen
	bitmap and then copied to the screen; the storage for the bitmap
	is kept in a relocatable and purgeable block. */
PopupHandle PopupBegin(GrafPtr port, MenuHandle menu, const Rect *maxbounds)
{
	PopupHandle popup;
	void *tmp;
	
	require(MenuValid(menu));
	require(RectValid(maxbounds));
	
	/* allocate popup */
	popup = (PopupHandle) NewHandleClear(sizeof(PopupType));
	if (popup) {
	
		/* initialize internal state */
		(**popup).version = kPopupVersion;
		(**popup).port = port;
		(**popup).menu = menu;
		(**popup).private.mHandle = menu;
		(**popup).private.mID = (**menu).menuID;
		(**popup).r.maxbounds = *maxbounds;
		
		/* initialize flags affecting display and operation of menu */
		(**popup).attr.visible = true;
		(**popup).attr.enabled = true;
		(**popup).attr.mark = checkMark;
		(**popup).attr.just = GetSysJust();
		
		/* allocate title */
		tmp = NewHandleClear(1);
		(**popup).attr.title.str = tmp;

		/* allocate utility region */
		tmp = NewRgn();
		(**popup).utilRgn = tmp;

		/* allocate region for saving and restoring the clip region */
		tmp = NewRgn();
		(**popup).state.oldenv.clip = tmp;
		
		/* allocate bitmap's base address; the handle is purgeable since
			we can always rebuild the data it contains and since we can
			always draw the popup directly to the screen, though it may
			flicker a bit */
		#if ! DONT_DRAW_OFFSCREEN
			tmp = NewHandle(0);
			(**popup).baseAddr = tmp;
			if ((**popup).baseAddr)
				HPurge((**popup).baseAddr);
		#endif /* DONT_DRAW_OFFSCREEN */
	}
	ensure(! popup || PopupValid(popup));
	return(popup);
}

/* end use of the popup menu */
void PopupEnd(PopupHandle popup)
{
	require(! popup || PopupValid(popup));
	if (popup) {
		if ((**popup).state.oldenv.clip) DisposeRgn((**popup).state.oldenv.clip);
		if ((**popup).state.selection) DisposeMenu((**popup).state.selection);
		if ((**popup).attr.title.str) DisposeHandle((**popup).attr.title.str);
		if ((**popup).baseAddr) DisposeHandle((**popup).baseAddr);
		if ((**popup).utilRgn) DisposeRgn((**popup).utilRgn);
		DisposeHandle((Handle) popup);
		popup = NULL;
	}
	ensure(! PopupValid(popup));
}
