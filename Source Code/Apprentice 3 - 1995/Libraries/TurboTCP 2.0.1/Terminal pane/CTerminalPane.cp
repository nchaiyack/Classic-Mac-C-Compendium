/*
** CTerminalPane.cp
**
**	TerminalPane library
**	Terminal display pane
**
**	Copyright © 1993–94, FrostByte Design / Eric Scouten
**
*/


#include "CTerminalPane.h"

#include <Events.h>
#include "CBureaucrat.h"
#include "CScrollPane.h"
#include "Global.h"
#include "LongCoordinates.h"
#include "LongQD.h"



extern CBureaucrat* gGopher;


#define resetInvalsChar 3

TCL_DEFINE_CLASS_D1(CTerminalPane, CPanorama)


//	—— constructor ——

/*______________________________________________________________________
**
** constructors
**
**	Initialize the pane. Nothing special here. Parameters and usage are all the same
**	as for CPanorama.
**
*/

CTerminalPane::CTerminalPane() : CPanorama()
	{ CTerminalPaneX(); }

CTerminalPane::CTerminalPane(CView* anEnclosure, CBureaucrat* aSupervisor,
						short aWidth, short aHeight, short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing)
	: CPanorama(anEnclosure, aSupervisor, aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing)
	{ CTerminalPaneX(); }

void CTerminalPane::ITerminalPane(CView* anEnclosure, CBureaucrat* aSupervisor,
						short aWidth, short aHeight, short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing)
{
	CPanorama::IPanorama(anEnclosure, aSupervisor, aWidth, aHeight, aHEncl, aVEncl, aHSizing, aVSizing);
	if (member(itsEnclosure, CScrollPane))
		((CScrollPane*) itsEnclosure)->SetSteps(pixelsX, pixelsY);
}


/*______________________________________________________________________
**
** CTerminalPaneX (private method)
**
**	Finish initialization for terminal pane.
**
*/

void CTerminalPane::CTerminalPaneX()

{
	LongRect theBounds;

	SetLongRect(&theBounds, 0, 0, sizeX, sizeY);
	SetBounds(&theBounds);
	SetCanBeGopher(TRUE);
	SetWantsClicks(TRUE);
	
	if (member(itsEnclosure, CScrollPane))
		((CScrollPane*) itsEnclosure)->SetSteps(pixelsX, pixelsY);
	
	charsToInvalLine = resetInvalsChar;
	blinkCursor = FALSE;
	cursorVis = TRUE;
	lastCursorCol = lastCursorLine = 0;
	lastCursorTick = 0L;
	disableKeyScroll = FALSE;
	DoClearScreen();
}


//	—— drawing  ——

/*______________________________________________________________________
**
** Draw
**
**	Draw characters from the theScreen array onto the real screen.
**
**		area (Rect*):	area to be redrawn (in frame coordinates)
**
*/

void CTerminalPane::Draw(Rect* area)

{
	short	left, top, right, bottom;		// char coordinatates of draw region
	short	dLine;					// where to draw now
	LongRect	theLongArea;				// frame coordinates of region
	LongPt	theLongPt;
	Point		thePoint;


	// figure draw region

	charsToInvalLine = resetInvalsChar;
	QDToFrameR(area, &theLongArea);
	left = (theLongArea.left - offsetX) / pixelsX;
	right = (theLongArea.right -offsetX + 1) / pixelsX;
	top = (theLongArea.top - offsetY) / pixelsY;
	bottom = (theLongArea.bottom -offsetY + 1) / pixelsY;


	// do range checking

	left = TCLMax(left, 0);
	top = TCLMax(top, 0);
	right = TCLMin(right, maxX-1);
	bottom = TCLMin(bottom, maxY-1);


	// hard-wire for Monaco 9: Mom, don’t look at this code!

	TextFont(4);
	TextFace(0);
	TextSize(9);


	// draw the stuff

	if (left <= right) {
		dLine = top;
		while (dLine <= bottom) {
			SetLongPt(&theLongPt, left * pixelsX, dLine * pixelsY);
			FrameToQD(&theLongPt, &thePoint);
			MoveTo(thePoint.h + offsetX, thePoint.v + offsetY + pixelsY-2);
			DrawText(&theScreen[dLine][left], 0, right-left+1);
			dLine++;
		}
	}


	// check to see if we overwrote the cursor

	if ((left <= theColumn) && (right >= theColumn) &&
	 (top <= theLine) && (bottom >= theLine) && cursorVis)
	 	InvertCursor(theColumn, theLine);
	
}


//	—— blinking cursor support ——

/*______________________________________________________________________
**
** Dawdle
**
**	Blink the cursor if necessary. Cursor blinks at the user-defined rate for insertion point
**	blinking.
**
**		maxSleep (long*):	maximum sleep value, updated if necessary
**
*/

void CTerminalPane::Dawdle(long* maxSleep)

{
	CPanorama::Dawdle(maxSleep);
	if (blinkCursor) {
		*maxSleep = TCLMin(*maxSleep, GetCaretTime());
		if (LMGetTicks() >= lastCursorTick + GetCaretTime()) {
			cursorVis = !cursorVis;
			lastCursorTick = LMGetTicks();
			InvalCharRect(theColumn, theLine, theColumn, theLine);
		}
	}
}


/*______________________________________________________________________
**
** BecomeGopher
**
**	Become the gopher (or leave gopher status). All our routine does is force a refresh
**	of the cursor.
**
**		fBecoming (Boolean):		TRUE if becoming gopher
**
**		return (Boolean):		TRUE if successful in changing status
**
*/

Boolean CTerminalPane::BecomeGopher(Boolean fBecoming)

{
	if (!fBecoming)
		cursorVis = TRUE;
	InvalCharRect(theColumn, theLine, theColumn, theLine);
	return CPanorama::BecomeGopher(fBecoming);
}


/*______________________________________________________________________
**
** SetBlinking
**
**	Turns on or off cursor blinking.
**
**		blinkMode (Boolean):		TRUE to enable cursor blinking
**
*/

void CTerminalPane::SetBlinking(Boolean blinkMode)

{
	blinkCursor = blinkMode;
	if ((blinkMode)	&& (gGopher == this)) {
		lastCursorCol = theColumn;
		lastCursorLine = theLine;
		CursorMoved();
	}
	else {
		cursorVis = TRUE;
		InvalCharRect(theColumn, theLine, theColumn, theLine);
	}
}


//	—— scrolling ——

/*______________________________________________________________________
**
** ScrollToSelection
**
**	Ensure that the current cursor location is visible.
**
*/

void CTerminalPane::ScrollToSelection()

{
	short	hSpan, vSpan;
	LongRect	topLeftRect, botRightRect;			// top left / bottom right cells of selection
											// held over from CTable::ScrollToSelection
											// because we might implement character selection later
	LongPt	selPos;


	// get current window parameters

	if (EmptyLongRect(&aperture))					// nothing selected or nothing visible
		return;
	GetFrameSpan(&hSpan, &vSpan);
	selPos = position;							// init to current scroll position


	// figure out where cursor (or selection) is

	CalcCharRect(theColumn, theLine, theColumn, theLine, &topLeftRect);
	topLeftRect.left -= offsetX;
	topLeftRect.top -= offsetY;
	topLeftRect.right += offsetX;
	topLeftRect.bottom += offsetY;
	botRightRect = topLeftRect;
			// adapt later for charcter selections


	// calc vertical scroll

	if (topLeftRect.bottom >= position.v + vSpan)		// Is the top of selection below the bottom of frame?
		selPos.v = topLeftRect.bottom - vSpan;
	else if (botRightRect.top < position.v)			// Is the bottom of selection above the top of the frame?
		selPos.v = botRightRect.top;


	// calc horizontal scroll

	if (topLeftRect.right >= position.h + hSpan)		// Is the left edge of selection past the right edge of frame?
		selPos.h = topLeftRect.right - hSpan;
	else if (botRightRect.left < position.h)			// Is the right edge of selection before the left edge of frame?
		selPos.h = botRightRect.left;

	if ((selPos.h != position.h) || (selPos.v != position.v))
		ScrollTo(&selPos, TRUE);

}


/*______________________________________________________________________
**
** DisableKeyScroll
**
**	Turn on or off keyboard scrolling commands.
**
**		disable (Boolean):	TRUE to turn off keyboard scrolling
**
*/

void CTerminalPane::DisableKeyScroll(Boolean disable)

{
	disableKeyScroll = disable;
}


/*______________________________________________________________________
**
** DoKeyDown
**
**	Override to enable or disable key scrolling (which is implemented in CPanorama).
**
*/

void CTerminalPane::DoKeyDown(char theChar, Byte keyCode, EventRecord* macEvent)

{
	if (disableKeyScroll)
		CBureaucrat::DoKeyDown(theChar, keyCode, macEvent);
	else
		CPanorama::DoKeyDown(theChar, keyCode, macEvent);
}


//	—— terminal emulation methods ——

/*______________________________________________________________________
**
** DoClearScreen
**
**	Clear the screen and move the cursor to (0,0).
**
*/

void CTerminalPane::DoClearScreen()

{
	register short x, y;
	
	theLine = theColumn = 0;
	CursorMoved();

	for (y = 0; y < maxY; ++y)
		for (x = 0; x < maxX; ++x)
			theScreen[y][x] = ' ';
	Refresh();
}


/*______________________________________________________________________
**
** DoWriteChar
**
**	Write a character to the terminal. This method handles *basic* terminal emulation.
**	To provide more sophisticated emulation, override this method.
**
**		theChar (char):		the character to write
**
*/

void CTerminalPane::DoWriteChar(char theChar)

{

	// parse a few control characters

	switch (theChar) {

		case charNUL:
			break;

		case charBEL:
			SysBeep(0);
			break;

		case charBS:
			if (theColumn > 0)
				theColumn--;
			CursorMoved();
			break;

		case charHT:
			theColumn = ((short) ((theColumn + 7) / 8)) * 8;
			if (theColumn >= maxX)
				theColumn = maxX-1;
			CursorMoved();
			break;
			
		case charLF:
			if (theLine < maxY-1)
				theLine++;
			else
				ScrollTerm();
			CursorMoved();
			ScrollToSelection();
			break;
			
		case charFF:
			DoClearScreen();
			ScrollToSelection();
			break;
			
		case charCR:
			theColumn = 0;
			CursorMoved();
			break;
		
		default:
			theScreen[theLine][theColumn] = theChar;
			InvalCharRect(theColumn, theLine, theColumn, theLine);
			if (theColumn < maxX-1)
				theColumn++;
			CursorMoved();
	}
}


/*______________________________________________________________________
**
** DoWriteStr
**
**	Write a string to the terminal. This method is optimized to handle text strings. It skips
**	any control characters and sends them directly to DoWriteChar.
**
**		theStr (char*):		the string to write (C string)
**
*/

void CTerminalPane::DoWriteStr(char* theStr)

{
	short leftCol;

	// optimize for text characters
	
	while (*theStr) {
		if (*theStr >= ' ') {
			leftCol = theColumn;
			while (*theStr >= ' ') {
				theScreen[theLine][theColumn] = *(theStr++);
				if (theColumn < maxX-1)
					theColumn++;
			}
			InvalCharRect(leftCol, theLine, theColumn, theLine);
			CursorMoved();
		}
		if (*theStr)
			DoWriteChar(*(theStr++));
	}
}


/*______________________________________________________________________
**
** DoWriteBfr
**
**	Write the contents of a text buffer to the terminal. This method
**	merely dishes out the characters to the DoWriteChar method.
**
**		theStr (char*):		the buffer to write
**		theSize (short):	size of the data buffer
**
*/

void CTerminalPane::DoWriteBfr(char* theStr, short theSize)

{
	while (theSize--)
		DoWriteChar(*(theStr++));
}


/*______________________________________________________________________
**
** DoWriteCharNum
**
**	Write a character number to the terminal. Provided as a debugging routine
**	by other classes. The number is bracketed by the two characters indicated, i.e.
**	if you call ::DoWriteCharNum('!', '[', ']'), you get [33] written to the terminal.
**
**		theChar (char):		the character number to write
**		leftBracket (char):	prefix to character number
**		rightBracket (char):	suffix to character number
**
*/

void CTerminalPane::DoWriteCharNum(char theChar, char leftBracket, char rightBracket)

{
	Str255 cNumber;

	if (leftBracket)
		DoWriteChar(leftBracket);

	NumToString((short) theChar, cNumber);
	cNumber[cNumber[0]+1] = '\0';
	DoWriteStr((char*) (&cNumber)+1);

	if (rightBracket)	
		DoWriteChar(rightBracket);
}


/*______________________________________________________________________
**
** DoEraseChar
**
**	Erase one character from screen (backspace).
**
*/

void CTerminalPane::DoEraseChar()

{
	register short x;
	
	if (theColumn > 0) {
		theColumn--;
		for (x = theColumn; x < maxX-1; x++)
			theScreen[theLine][x] = theScreen[theLine][x+1];
		theScreen[theLine][maxX-1] = ' ';
		InvalCharRect(theColumn, theLine, maxX-1, theLine);
		CursorMoved();
	}
}


/*______________________________________________________________________
**
** DoEraseLine
**
**	Move the cursor back to beginning of line & erase line.
**
*/

void CTerminalPane::DoEraseLine()

{
	theColumn = 0;
	ClearToEOL(0, theLine);
	CursorMoved();
}


//	—— private screen handling methods ——

/*______________________________________________________________________
**
** ClearToEOL (protected method)
**
**	Clear from the indicated point to the end of a line. Upper left
**	corner is (0,0).
**
**		col (short):	first column in line to clear
**		line (short):	line to be cleared
**
*/

void CTerminalPane::ClearToEOL(short col, short line)

{
	InvalCharRect(col, line, maxX-1, line);
	while (col < maxX)
		theScreen[line][col++] = ' ';
}


/*______________________________________________________________________
**
** ClearToEOS (protected method)
**
**	Clear from the indicated point to the end of the screen. Upper left
**	corner is (0,0).
**
**		col (short):	first column in first line to clear
**		line (short):	first line to be cleared
**
*/

void CTerminalPane::ClearToEOS(short col, short line)

{
	while (line < maxY) {
		ClearToEOL(col, line++);
		col = 0;
	}
}


/*______________________________________________________________________
**
** ScrollTerm
**
**	Push everything on screen up one line.
**
*/

void CTerminalPane::ScrollTerm()

{
	BlockMove(&theScreen[1][0], &theScreen[0][0], (maxY-1) * maxX);
	ClearToEOL(0, maxY-1);
	Refresh();
}


/*______________________________________________________________________
**
** InvalCharRect (protected method)
**
**	Invalidate a section of the pane based on the character coordinates
**	provided. Upper left corner is (0,0).
**
**		left (short):		left edge of character rectangle
**		top (short):		top edge
**		right (short):		right edge, inclusive
**		bottom (short):		bottom edge, inclusive
**
*/

void CTerminalPane::InvalCharRect(short left, short top, short right, short bottom)

{
	LongRect	theLongRect;

	CalcCharRect(left, top, right, bottom, &theLongRect);
	Prepare();
	RefreshLongRect(&theLongRect);
}


/*______________________________________________________________________
**
** CursorMoved
**
**	Update the blinking cursor block.
**
*/

void CTerminalPane::CursorMoved()

{
	cursorVis = TRUE;
	lastCursorTick = LMGetTicks();
	if (lastCursorLine == theLine) {
		switch (charsToInvalLine) {
			case 0:							// optimization to reduce calls to InvalRect()
				break;
			case 1:
				InvalCharRect(0, theLine, maxX-1, theLine);
				charsToInvalLine = 0;
				break;
			default:
				InvalCharRect(lastCursorCol, lastCursorLine, lastCursorCol, lastCursorLine);
				InvalCharRect(theColumn, theLine, theColumn, theLine);
				charsToInvalLine--;
		}
	}
	else {
		InvalCharRect(lastCursorCol, lastCursorLine, lastCursorCol, lastCursorLine);
		InvalCharRect(theColumn, theLine, theColumn, theLine);
		charsToInvalLine = resetInvalsChar;
	}
	lastCursorCol = theColumn;
	lastCursorLine = theLine;
}


/*______________________________________________________________________
**
** CalcCharRect (protected method)
**
**	Calculate the screen coordinates for a specified rectangle of
**	character coordinates. Upper left corner is (0,0).
**
**		left (short):			left edge of char rectangle
**		top (short):			top edge
**		right (short):			right edge, inclusize
**		bottom (short):			bottom edge, inclusive
**		theRect (LongRect*):	coordinates returned to this rectangle
**
*/

void CTerminalPane::CalcCharRect(short left, short top, short right, short bottom,
							LongRect* theRect)

{
	SetLongRect(theRect,
				(long) left * pixelsX + offsetX,
				(long) top * pixelsY + offsetY,
				(long) (right+1) * pixelsX + offsetX,
				(long) (bottom+1) * pixelsY + offsetY);
}


/*______________________________________________________________________
**
** InvertCursor
**
**	Invert a single character.
**
**		col (short):	the column number
**		line (short):	the line number
**
*/

void CTerminalPane::InvertCursor(short col, short line)

{
	LongRect	theLongRect;
	Rect		theRect;
	
	CalcCharRect(col, line, col, line, &theLongRect);
	LongToQDRect(&theLongRect, &theRect);
	InvertRect(&theRect);
	if (gGopher != this) {
		InsetRect(&theRect, 1, 1);
		InvertRect(&theRect);
	}
}
