/*______________________________________________________________________

	hlp.c - Help Module.
	
	Copyright � 1988-1991 Northwestern University.
	
	This reusable module implements help windows.  The help window contains
	a scrolling field containing the text and an optional table of contents.
	
	The caller supplies several resources - a WIND resource for the window,
	a sequence of STR# resources containing the text, and a TCON resource 
	for the table of contents.
	
	The sequence of STR# resources must be terminated by an empty STR#
	resource (an STR# resource of size 2 containing 0 strings).
	
	There is a limit of 8k lines of text in the help window
_____________________________________________________________________*/

#pragma load "precompile"
#include "rep.h"
#include "rpp.h"
#include "utl.h"
#include "hlp.h"
#include "doc.h"

#pragma segment hlp

/*______________________________________________________________________

	Global Constants and Variables.
_____________________________________________________________________*/

static	hlp_PBlock 		P;						/* param block */
static	WindowPtr		HelpWindow;			/* pointer to window record */
static	Handle			Report;				/* handle to report */
static	Handle			TabConRez;			/* handle to TCON resource */

static	short				TopLine;		/* line number of top line in table
													of contents rectangle */
static	short				BotLine;		/* line number of bottom line in table
													of contents rectangle */
static	short				LastLine;	/* line number of last line in table
													of contents */
static	short				LastCell;	/* cell number of last cell in table
													of contents rectangle */
													
static	Rect				Cell1Rect;	/* top cell rectangle */
static	Rect				Cell2Rect;	/* second cell rectangle */
static	Rect				CellARect;	/* second to bottom cell rectangle */
static	Rect				CellBRect;	/* bottom cell rectangle */

static	Rect				TCScrollRect;	/* table of contents scrolling
														rectangle */
													
static	RgnHandle		UpdateRgn;	/* update region for ScrollRect calls */													
												
static 	PolyHandle		TopTri;		/* top table of contents scrolling 
													triangle */
static	PolyHandle		BotTri;		/* bottom table of contents scrolling
													triangle */

/*______________________________________________________________________

	DelayScroll - Slow Down Scrolling.
	
	Entry:	n = delay factor (smaller value = longer delay).
_____________________________________________________________________*/

static void DelayScroll (short n)

{
	long			waitTill;

	if (n >= 32) return;
	waitTill = TickCount() + 8 - (n>>2);
	while (TickCount() < waitTill) {
		if (!StillDown()) break;
	}
}

/*______________________________________________________________________

	GetLine - Get Table of Contents Line.
	
	Entry:	n = line number.
				
	Exit:		function result = pointer to line.
_____________________________________________________________________*/

static unsigned char *GetLine (short n)

{
	unsigned char		*q;
	
	q = (unsigned char *)*TabConRez + 4;
	while (true) {
		if (*q & docScreen) {
			if (n) {
				n--;
			} else {
				return q+1;
			}
		}
		q += *(q+1) + 4;
		if ((long)q & 1) q++;
	}
}

/*______________________________________________________________________

	DoTcon - Check for and Process Mouse Down in Table of Contents.
	
	Entry:	where = location of mouse down event, in local coords.
_____________________________________________________________________*/

static void DoTcon (Point where)

{
	Point				curLoc;				/* current mouse loc */
	short				oldCell;				/* old cell pos, or -1 if none. */
	short				newCell;				/* new cell pos, or -1 if none */
	unsigned char	*p;					/* pointer into TCON resource */
	Rect				frameRect;			/* rect enclosing table of contents */
	Rect				invertRect;			/* rect to be inverted */
	long				startTicks;			/* tick count at mouse down */
								
	/* Return if mouse loc is not in the table of contents rectangle. */
													
	curLoc = where;
	frameRect = P.tabConRect;
	InsetRect(&frameRect, 1, 1);
	if (!PtInRect(curLoc, &frameRect)) return;
	
	/* Track the mouse and hilite the table of contents lines. */
	
	oldCell = -1;
	invertRect = frameRect;
	startTicks = TickCount();
	while (true) {
		if (PtInRect(curLoc, &frameRect)) {
			newCell = (curLoc.v - frameRect.top - 1) / P.tabConLSep;
		} else if (curLoc.h >= frameRect.left && curLoc.h <= frameRect.right) {
			if (TopLine && curLoc.v <= frameRect.top) {
				newCell = 0;
			} else if (BotLine < LastLine && curLoc.v >= frameRect.bottom) {
				newCell = LastCell;
			} else {
				newCell = -1;
			}
		} else { 
			newCell = -1;
		}
		if (newCell != oldCell) {
			if (oldCell >= 0) {
				invertRect.top = frameRect.top + P.tabConLSep*oldCell;
				invertRect.bottom = invertRect.top + P.tabConLSep;
				InvertRect(&invertRect);
			}
			if (newCell >= 0) {
				if (!newCell && TopLine) {
					/* scroll down 1 line */
					ScrollRect(&TCScrollRect, 0, P.tabConLSep, UpdateRgn);
					SetOrigin(0, 0);
					p = GetLine(TopLine);
					TextBox(p+1, *p, &Cell2Rect, teJustLeft);
					TopLine--;
					if (!TopLine) {
						EraseRect(&Cell1Rect);
						p = GetLine(0);
						TextBox(p+1, *p, &Cell1Rect, teJustLeft);
					}
					if (BotLine == LastLine) {
						EraseRect(&CellBRect);
						PaintPoly(BotTri);
					}
					BotLine--;
					newCell = -1;
					if (TopLine) {
						GetMouse(&curLoc);
						DelayScroll(Cell2Rect.top - curLoc.v);
					}
				} else if (newCell == LastCell && BotLine < LastLine) {
					/* scroll up 1 line */
					ScrollRect(&TCScrollRect, 0, -P.tabConLSep, UpdateRgn);
					SetOrigin(0, 0);
					p = GetLine(BotLine);
					TextBox(p+1, *p, &CellARect, teJustLeft);
					BotLine++;
					if (BotLine == LastLine) {
						EraseRect(&CellBRect);
						p = GetLine(LastLine);
						TextBox(p+1, *p, &CellBRect, teJustLeft);
					}
					if (!TopLine) {
						EraseRect(&Cell1Rect);
						PaintPoly(TopTri);
					}
					TopLine++;
					newCell = -1;
					if (BotLine < LastLine) {
						GetMouse(&curLoc);
						DelayScroll(curLoc.v - CellARect.bottom);
					}
				} else {
					invertRect.top = frameRect.top + P.tabConLSep*newCell;
					invertRect.bottom = invertRect.top + P.tabConLSep;
					InvertRect(&invertRect);
				}
			}
			oldCell = newCell;
		}
		if (!StillDown()) break;
		GetMouse(&curLoc);
	}
	
	/* Jump to the selected section. */
	
	if ((newCell >= 0) && 
		(newCell || !TopLine) && 
		(newCell < LastCell || BotLine == LastLine)) {
		while (TickCount() < startTicks+8);
		invertRect.top = frameRect.top + P.tabConLSep*newCell;
		invertRect.bottom = invertRect.top + P.tabConLSep;
		InvertRect(&invertRect);
		p = GetLine(newCell + TopLine) - 3;
		rep_Jump(Report, *((short*)p), true);
	}
}

/*______________________________________________________________________

	DrawTcon - Draw the Table of Contents.
_____________________________________________________________________*/

static void DrawTcon (void)

{
	Rect				theBox;				/* rectangle enclosing cur line */
	unsigned char	*q;					/* pointer into TCON resource */
	short				oldFont;				/* saved font number */
	short				oldSize;				/* saved font size */
	Rect				frameBox;			/* box for framing */
	short				i;						/* cell index */
	
	theBox = Cell1Rect;
	oldFont = qd.thePort->txFont;
	oldSize = qd.thePort->txSize;
	TextFont(P.tabConFNum);
	TextSize(P.tabConFSize);
	for (i = 0; i <= LastCell; i++) {
		if (!i && TopLine) {
			PaintPoly(TopTri);
		} else if (i == LastCell && BotLine < LastLine) {
			PaintPoly(BotTri);
		} else {
			q = GetLine(TopLine+i);
			TextBox(q+1, *q, &theBox, teJustLeft);
		}
		OffsetRect(&theBox, 0, P.tabConLSep);
	}
	TextFont(oldFont);
	TextSize(oldSize);
	frameBox = P.tabConRect;
	FrameRect(&frameBox);
	MoveTo(frameBox.right, frameBox.top+2);
	LineTo(frameBox.right, frameBox.bottom);
	LineTo(frameBox.left+2, frameBox.bottom);
}

/*______________________________________________________________________

	hlp_Open - Open Help Window.
	
	Entry:	theWindow = pointer to opened window, positioned and sized,
					invisible.
					
				p = pointer to parameter block, with fields set as follows:
	
				firstStrID = resource id of first STR# resource.
				listDefID = resource id of type 1 report LDEF.
				textRect = rectangle enclosing the text and its scroll bar.
				fontNum = font number for text.
				fontSize = font size for text.
				tabConID = resource id of TCON resource.
				tabConRect = rectangle for table of contents.
				tabConFNum = table of contents font number.
				tabConFSize = table of contents font size.
				tabConLSep = table of contents line separation, in pixels.
				tag = tag of line to jump to initially, or 0 if none.  Takes
					precedence over scrollLine.
				tagRezID = resource id of TAG resource.
				scrollLine = line number of line to jump to initially.
				cellRezID = resource id of CELL resource.
				cellOption = CELL resource disposal option:
					0 = release the CELL resource.
					1 = keep the CELL resource, and leave it purgable.
					2 = keep the CELL resource, and leave it unpurgable.
				extraUpdate = pointer to a funtion to be called during
					update event processing, or nil if none.  This function 
					can draw any extra decorative stuff in the help window 
					(e.g., a "Table of Contents" title above the table of 
					contents).
					
	Exit:		window initialized and shown.
	
	For best results, the textRect rectangle height should 2 more than an 
	even multiple of the window font's ascent plus descent plus leading
	(12 for Geneva 9).  This will make an even number of rows appear in 
	the rectangle, which makes scrolling look nice.
		
	Similarly, the tabConRect rectangle height should be 2 more than an 
	even multiple of tabConLSep.  If this rectangle is not tall enough to 
	contain all of the table of contents lines, then the menu will scroll.
	
	The tabConRect rectangle should not be taller than the total number
	of table of contents entries.  That is, you should gaurantee that
	
		(tabConRect.bottom - tabConRect.top - 2) / tabConLSep <=
			total number of table of contents entries.
_____________________________________________________________________*/

void hlp_Open (WindowPtr theWindow, hlp_PBlock *p)
	
{
	short				h, v;					/* h and v coords for triangles */
	short				tsize;				/* size (height) of triangles */
	unsigned char	*q;					/* ptr into TCON resource */
	short				n;						/* loop index */
	
	P = *p;
	HelpWindow = theWindow;
	SetPort(HelpWindow);

	/* Initialize table of contents. */
	
	/* Create the top and bottom table of contents scrolling triangles. */

	h = p->tabConRect.left + 5;
	v = p->tabConRect.top + p->tabConLSep + 1 - 
		((p->tabConLSep-p->tabConFSize)>>1);
	tsize = p->tabConFSize-2;
	TopTri = OpenPoly();
	MoveTo(h,v);
	Line(tsize<<1, 0);
	Line(-tsize, -tsize);
	LineTo(h, v);
	ClosePoly();
	v = p->tabConRect.bottom - p->tabConLSep - 1 + 
		((p->tabConLSep-p->tabConFSize)>>1);
	BotTri = OpenPoly();
	MoveTo(h,v);
	Line(tsize<<1, 0);
	Line(-tsize, tsize);
	LineTo(h, v);
	ClosePoly();

	/* Load and lock the TCON resource. */

	TabConRez = GetResource('TCON', p->tabConID);
	MoveHHi(TabConRez);
	HLock(TabConRez);
	
	/* Initialize global variables. */
	
	TopLine = 0;
	q = (unsigned char*)*TabConRez + 4;
	LastLine = -1;
	for (n = 0; n < **(short**)TabConRez; n++) {
		if (*q & docScreen) LastLine++;
		q += *(q+1) + 4;
		if ((long)q & 1) q++;
	}
	LastCell = (p->tabConRect.bottom - p->tabConRect.top - 2) /
		p->tabConLSep - 1;
	BotLine = LastCell;
	UpdateRgn = NewRgn();
	Cell1Rect = p->tabConRect;
	InsetRect(&Cell1Rect, 4, 1);
	CellBRect = TCScrollRect = Cell1Rect;
	Cell1Rect.bottom = Cell1Rect.top + p->tabConLSep;
	Cell2Rect = Cell1Rect;
	OffsetRect(&Cell2Rect, 0, p->tabConLSep);
	CellBRect.top = CellBRect.bottom - p->tabConLSep;
	CellARect = CellBRect;
	OffsetRect(&CellARect, 0, -p->tabConLSep);
	TCScrollRect.top += p->tabConLSep;
	TCScrollRect.bottom -= p->tabConLSep;
	
	/* Set the window's font id and size. */
	
	TextFont(p->fontNum);
	TextSize(p->fontSize);
	
	/* Initialize the report. */
	
	rep_Init(&p->textRect, HelpWindow, 1, p->firstStrID, p->listDefID, 
		p->cellRezID, &Report);
	if (p->tag) {
		rep_Jump(Report, rep_Tag(p->tagRezID, p->tag), false);
	} else if (p->scrollLine) {
		rep_Jump(Report, p->scrollLine, false);
	}
	
	/* Size the window. */
	
	hlp_Zoom();
	
	/* Show the window. */
	
	utl_LockControls(HelpWindow);
	ShowWindow(HelpWindow);
}

/*______________________________________________________________________

	hlp_Close - Close Help Window.
	
	Exit:			auxiliary data structures deallocated.
	
	It is the callers responsibility to actually dispose of the window
	proper.
_____________________________________________________________________*/

void hlp_Close (void)

{
	SetPort(HelpWindow);
	HUnlock(TabConRez);
	KillPoly(TopTri);
	KillPoly(BotTri);
	DisposeRgn(UpdateRgn);
	rep_Dispose(Report, P.cellOption);
}

/*______________________________________________________________________

	hlp_Click - Process Mouse Down Event.
	
	Entry:		where = mouse down location, local coords.
_____________________________________________________________________*/

void hlp_Click (Point where)

{
	Rect				repRect;			/* report rectangle */

	rep_GetRect(Report, &repRect);
	if (PtInRect(where, &repRect)) {
		rep_Scroll(Report, where);
	} else {
		DoTcon(where);
	}
}

/*______________________________________________________________________

	hlp_Activate - Process Activate Event.
_____________________________________________________________________*/

void hlp_Activate (void)

{
	rep_Activate(Report, true);
	utl_DrawGrowIcon(HelpWindow);
}

/*______________________________________________________________________

	hlp_Deactivate - Process Deactivate Event.
_____________________________________________________________________*/

void hlp_Deactivate (void)

{
	rep_Activate(Report, false);
	utl_DrawGrowIcon(HelpWindow);
}

/*______________________________________________________________________

	hlp_Update - Process Update Event.
	
	It is the caller's responsibility to set the port to the help window,
	call BeginUpdate, and erase the portrect before calling this routine,
	and to call EndUpdate after calling it.
_____________________________________________________________________*/

void hlp_Update (void)

{
	DrawTcon();
	rep_Update(Report);
	utl_DrawGrowIcon(HelpWindow);
	if (P.extraUpdate) (*P.extraUpdate)();
}

/*______________________________________________________________________

	hlp_Key - Process Key Event.
	
	Entry:		key = ascii code of key pressed.
					modifiers = key modifiers.
_____________________________________________________________________*/

void hlp_Key (short key, short modifiers)

{
	
	if (key == upArrow || key == downArrow) {
		(void) rep_Key(Report, key, modifiers);
	}
}

/*______________________________________________________________________

	hlp_Jump - Jump to a Tag.
	
	Entry:		tag = tag to jump to.
_____________________________________________________________________*/

void hlp_Jump (short tag)

{
	rep_Jump(Report, rep_Tag(P.tagRezID, tag), true);
}

/*______________________________________________________________________

	hlp_Print - Print Help Text.
	
	Entry:		p = pointer to print parameter block.
					printOne = true to bypass print job dialog.
					
	Exit:			function result = error code.
_____________________________________________________________________*/
	
	
OSErr hlp_Print (rpp_PrtBlock *p, Boolean printOne)

{
	return rpp_Print(Report, printOne, p);
}

/*______________________________________________________________________

	ChangeSize - Process Window Size Change.
	
	Entry:	height = new window height.
_____________________________________________________________________*/

static void ChangeSize (short height)

{
	short			newTabConHeight;			/* new tcon rect height */
	short			newLastCell;				/* new cell num of last cell */
	short			deltaCell;					/* change in number of cells */
	short			deltaHeight;				/* change in height of tcon rect */
	Rect			inval;						/* rect to be invalidated */
	
	/* Change report size. */
	
	rep_Height(Report, height - 6);
	
	/* Change table of contents size. */
	
	newTabConHeight = height - 20 - P.tabConRect.top;
	newTabConHeight -= (newTabConHeight - 2) % P.tabConLSep;
	newLastCell = (newTabConHeight - 2) / P.tabConLSep - 1;
	if (newLastCell > LastLine) newLastCell = LastLine;
	deltaCell = newLastCell - LastCell;
	deltaHeight = deltaCell * P.tabConLSep;
	P.tabConRect.bottom += deltaHeight;
	LastCell = newLastCell;
	BotLine += deltaCell;
	if (BotLine > LastLine) {
		TopLine -= BotLine-LastLine;
		BotLine = LastLine;
	}
	OffsetRect(&CellARect, 0, deltaHeight);
	OffsetRect(&CellBRect, 0, deltaHeight);
	TCScrollRect.bottom += deltaHeight;
	OffsetPoly(BotTri, 0, deltaHeight);
	inval = P.tabConRect;
	inval.right += 1;
	inval.bottom = HelpWindow->portRect.bottom;
	InvalRect(&inval);
}

/*______________________________________________________________________

	hlp_Grow - Process Window Grow Operation.
	
	Entry:	height = new window height.
				width = new window width.
_____________________________________________________________________*/

void hlp_Grow (short height, short width)

{
	utl_InvalGrow(HelpWindow);
	SizeWindow(HelpWindow, width, height, true);
	utl_InvalGrow(HelpWindow);
	ChangeSize(height);
}
	
/*______________________________________________________________________

	hlp_Zoom - Process Window Zoom Operation.
_____________________________________________________________________*/

void hlp_Zoom (void)

{
	ChangeSize(HelpWindow->portRect.bottom - HelpWindow->portRect.top);
}

/*______________________________________________________________________

	hlp_Save - Save Help Document.
	
	Entry:		prompt = prompt string for standard file dialog.
					defName = default file name for standard file dialog.
					creator = creator type for saved text file.
						
	Exit:			function result = os error code.
_____________________________________________________________________*/
	
	
OSErr hlp_Save (Str255 prompt, Str255 defName, OSType creator)

{
	Boolean			good;			/* true if file saved */
	
	return rep_Save (Report, prompt, defName, creator, &good);
}

/*______________________________________________________________________

	hlp_GetScrollPos - Get Help Window Scrolling Position.
	
	Exit:			function result = window scrolling position (line number
						at top of window).
_____________________________________________________________________*/

short hlp_GetScrollPos (void)

{
	return rep_GetPos(Report);
}

/*______________________________________________________________________

	hlp_GetTconRect - Get Help Window Table of Contents Rectangle.
	
	Exit:		tconRect = tcon rectangle.
_____________________________________________________________________*/

void hlp_GetTconRect (Rect *tconRect)

{
	*tconRect = P.tabConRect;
}
