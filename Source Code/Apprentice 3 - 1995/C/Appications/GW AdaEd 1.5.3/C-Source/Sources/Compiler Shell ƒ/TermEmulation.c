/* TermEmulation.c */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "TermEmulation.h"
#include "ExecMain.h"
#include "ExecShellGlobals.h"

static	void	doESC(Parms *p);
static	void	doDraw(Parms *p);
static	void	doNUM1(Parms *p);
static	void	doNUM2(Parms *p);
static	void	doClear(Parms *p);
static	void	doMove(Parms *p);
static	void	doVideo(Parms *p);
static	void	doSingleChar(Parms *p);

static	void	cursorUpdate(void);
static	void	cursorOff(void);
static	void	cursorOn(void);
static	void	cursorBlink(void);

static	Boolean	DoKeys(EventRecord *theEvent);
static	void	KeyTypedIn(char ch);

static	void	scrValidate(void);
static	void	scrSetCursor(void);
static	void	scrSetFace(void);
static	void	scrDrawText(char *str, short offset,
						short len, short style);
static	void	scrDrawChar(char ch, short style);
static	void	scrEraseFromTo(short fromCol, short fromRow, 
								short toCol, short toRow);

static	void	scrRectFromTo(Rect *r, short fromCol, short fromRow, 
								short toCol, short toRow);
static	void	scrCharToRect(short c, short r, Rect *rect);

static	void	scrNextRow(void);
static	void	scrNextCol(void);
static	void	scrPrevRow(void);
static	void	scrPrevCol(void);

static	void	scrScrollScreen(short i);

/* Escape Sequences Recognized and their meanings:
 *     (n is optional, if omitted n = 0)
 *
 *  Move Cursor
 *		ESC [ n A		move cursor up n lines
 *		ESC [ n B		move cursor down n lines
 *		ESC [ n C		move cursor forward n columns
 *		ESC [ n D		move cursor backward n columns
 *
 *		ESC [ H			move cursor to home position
 *		ESC [ y ; x H	move cursor to x,y position
 *		ESC [ y ; x f	same as above, but for the vt52
 *		ESC [ s			save cursor position (only one position saved)
 *		ESC [ u			restore cursor to saved position
 *
 *  Erasing Screen Contents
 *		ESC [ n J		erase in display
 *						n = 0 from current position to end of screen
 *						n = 1 from start of screen to current position
 *						n = 2 all of the screen
 *		ESC [ n K		erase in line
 *						n = 0 from current position to end of line
 *						n = 1 from start of line to current position
 *						n = 2 all of the line
 *
 *  Font Settings
 *		ESC [ 4 m		underline mode
 *		ESC [ 5 m		blinking mode (implemented as bold)
 *		ESC [ 7 m		reverse video mode
 *		ESC [ m			back to default mode (normal)
 *
 *  Single Character Sequences Recognized
 *
 *  BELL				call SysBeep(1)
 *  LF					move cursor back to beginning of next line
 *  FF					clear screen and position cursor at top of screen
 *  NEW_LINE, TAB		will be implemented later!
 */


/* Prototypes for local macros */
#define colToX(c) (c * width + border)
#define rowToY(r) (r * height + border + fontInfo.ascent)


/* Table below provides the transitions for a state transition
 * machine that will interpret the escape sequences from above.
 * If there is no explicit transition for the next input char,
 * doDraw is called on the current character and the default
 * state is restored (stNONE).
 */

#define chESC  { ESC, 0 }
#define	chSingleChar	{ LF, FF, BELL, 0 }
#define	chCSI	{ '[', 0 }
#define	chNumeric { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }
#define	chSemicolon	{ ';', 0 }
#define	chMove	{ 'A', 'B', 'C', 'D', 'H', 'f', 's', 'u', 0 }
#define	chClear	{ 'J', 'K', 0 }
#define	chVideo	{ 'm', 0 }


/* NOTE: Borrowed from ipredef.h */
/* Define special characters in text file */

#define PAGE_MARK  0x0C 	       /* page terminator */
#define LINE_FEED  0x0a 	       /* line terminator */


#define tableSize 11
static State table[tableSize] = {
/*0*/	{ stNONE,	stESC,	doESC,		chESC },
/*1*/	{ stNONE,	stNONE,	doSingleChar,	chSingleChar },

/*2*/	{ stESC,	stNUM1,	doNothing,	chCSI },
	
/*3*/	{ stNUM1,	stNUM1,	doNUM1,		chNumeric },
/*4*/	{ stNUM1,	stNUM2,	doNothing,	chSemicolon },
/*5*/	{ stNUM1,	stNONE,	doMove,		chMove },
/*6*/	{ stNUM1,	stNONE,	doClear,	chClear },
/*7*/	{ stNUM1,	stNONE,	doVideo,	chVideo },

/*8*/	{ stNUM2,	stNUM2,	doNUM2,		chNumeric },
/*9*/	{ stNUM2,	stNONE,	doMove,		chMove },
/*10*/	{ stNUM2,	stNONE,	doClear,	chClear }

};


#define cols 80
#define rows 25
#define MaxInputBuffer 256

static	int currIdx = 0;						// index into table
static	Parms parmState = { ' ', 0, 0 };		// parameters in ESC seq.
static	short savedCol = 0, savedRow = 0;		// saved x, y
static	short col, row;							// current cursor pos.
static	short width, height;					// character widht, height
static	char screen[rows][cols];				// screen contents
static	char faceStyle[rows][cols];				// screen font face options

static	WindowPtr theWindow = NULL;				// window
static	Rect bounds;
static	FontInfo fontInfo;
static	short	initialFace;					// normal text face
static	short	currentFace;					// blinking, bold, or underline

static	FILE	*fileEcho = NULL;


/* Input Buffer */
static	char inputBuffer[MaxInputBuffer];
static	short indexBuffer = -1;

/* Available for all to use as a scratch area */
char	scrBuffer[300];

static	void	scrDrawText(char *str, short offset,
						short len, short style);
/* Notes:
 *
 * 1) SetPort is commented out in all private routines.  The only
 *    way for execution to reach a private a routine is through one
 *    of the public routines.  All public routines must call SetPort
 *    before they call an internal (private) routine.
 *
 * 2) The mac ctype.h doesn't have a isascii() macro.  For the getc
 *    routine we use isprint() to accept characters.  Is this ok?
 *    For now, I guess it is. UPDATE: Not true!  I removed the isprint()
 *    call, it now returns any character that is returned.  Further
 *	  UPDATE: See scrGetc to see how this works.
 *
 * 3) When using bold, use also condense.  That will keep the bold
 *    characters the same width as the normal characters (at least
 *    for monaco, which is the font with which we tested this).
 */


WindowPtr	scrCreateWindow(short font, short size/*, short r, short c*/)
					// public routine
{
#define border 4
Rect wbox = { 100, 100, 200, 300 };

	/*  create the window  */

theWindow = GetNewWindow(128, nil, (WindowPtr) -1);
//	MoveWindow(theWindow, 20, 45, 0);
SetPort(theWindow);

	/*  set up the font characteristics  */

TextFont(font);
TextSize(size);
TextFace(bold+condense);// use bold+condense to calculate width of screen
						// with largest possible font
GetFontInfo(&fontInfo);
height = fontInfo.ascent + fontInfo.descent + fontInfo.leading;
width = fontInfo.widMax;
		
currentFace = initialFace = ftNormal;

	/*  set initial window size  */
// rows = r;			These arguments are ignored for now!
// cols = c;

scrRectFromTo(&bounds, 0, 0, cols, rows);
InsetRect(&bounds, -border, -border);
OffsetRect(&bounds, -bounds.left, -bounds.top);
SizeWindow(theWindow, bounds.right, bounds.bottom, 0);

	/*  done  */

ShowWindow(theWindow);
scrClearScreen();
scrGotoXY(0, 0);
return theWindow;			// return the window to the outside
}

void	scrClearScreen(void)					// public routine
{
register short r, c;

/* put spaces in screen */
for (r = 0; r < rows; r++)
	for (c = 0; c < cols; c++) {
		screen[r][c] = ' ';
		faceStyle[r][c] = initialFace;
		}

// this routine doesn't change the cursor location
SetPort(theWindow);
EraseRect(&theWindow->portRect);
}

void	scrDrawScreen(void)					// public routine
{
register short r, c;
short saveCol, saveRow;

	SetPort(theWindow);
	saveCol = col;						// save current cursor position
	saveRow = row;
	for (r = 0; r < rows; r++) {
		register short from, style;

		// Find style runs in the faceStyle array so that
		// we can use one draw call to draw a chunk of text
		// (this speeds up the process)

		for (from = 0; from < cols; from += c) {
			c = from;
			for (style = faceStyle[r][c];		// start of a run?
				 (style == faceStyle[r][c] && c < cols);
					c++)
				{}		// empty loop body
			scrGotoXY(from, r);
			scrDrawText(&screen[r][from], 0, c-from, style);
		}
	}
	scrGotoXY(saveCol, saveRow);		// restore cursor position
}

void	scrGotoXY(short c, short r)					// public routine
{
	SetPort(theWindow);

	if (c >= 0 && c < cols)
		col = c;
	if (r >= 0 && r < rows)
		row = r;

	scrSetCursor();
}

/****
 *	cursor package - variables used by the cursor routines
 *
 ****/
static Rect cursorRect;
static Boolean cursorState;
static long cursorTime;

/****
 *	cursorUpdate()
 *
 *	Update cursor block
 *
 ****/

static void cursorUpdate(void)
{
scrCharToRect(col, row, &cursorRect);
}

/****
 *	cursorOff()
 *
 *	Turn off the cursor in the terminal emulation window.
 *
 ****/

static void cursorOff(void)
{
if (cursorState) {
	EraseRect(&cursorRect);
	cursorState = false;
	cursorTime = TickCount() + GetCaretTime();
	}
}

/****
 *	cursorOn()
 *
 *	Turn on the cursor in the terminal emulation window.
 *	Store time when it was last turned on.
 *
 ****/

static void cursorOn(void)
{
cursorUpdate();
InvertRect(&cursorRect);
cursorState = true;
cursorTime = TickCount() + GetCaretTime();
}

/****
 *	cursorBlink()
 *
 *	Blink the cursor image.
 *
 ****/

static void cursorBlink(void)
{

	if (ApplicationIsPaused())
		cursorOff();

	else if (TickCount() > cursorTime) {
		InvertRect(&cursorRect);
		cursorState = !cursorState;
		cursorTime = TickCount() + GetCaretTime();
	}
}

/****
 *	scrCopyToFile
 *
 *	Provide a file pointer to send a copy of all output
 *
 ****/

void	scrCopyToFile(FILE *fp)
{
if (fileEcho) {
	fclose(fileEcho);
	fileEcho = NULL;		// just to be safe
	}

fileEcho = fp;
}

/****
 * scrGetc()
 *
 *		Handle keyboard events and returns the character typed.
 *		While it is waiting for input, it blinks a cursor (using
 *		a block cursor). The cursor is blinked at the same rate
 *		the caret is supposed to blink.
 *
 *		10/21/93 - To handle backspace, this routine now will
 *		continue reading input until a return key is pressed.
 *		Once the return is pressed, the routine will return the
 *		first character in the buffer (just read).  Next time
 *		the routine is called, it will continue returning chars
 *		from the buffer, until the buffer is empty.  At this point
 *		it will reset and start over from the beginning.
 *
 *****/

char scrGetc(void)					// public routine
{
EventRecord	theEvent;
Boolean		moreEvents;
char		ch;


SetPort(theWindow);

// If we don't have any more characters in the buffer
// then get some characters until a end-of-line is
// entered.

if (indexBuffer < 0) {

	cursorOn();
	moreEvents = true;
	while (moreEvents) {

		cursorBlink();		// all idle events
//		HiliteMenu(0);
//		SystemTask ();		/* Handle desk accessories */

		if (WaitNextEvent(everyEvent, &theEvent, LMGetCaretTime(), nil)) {
			switch (theEvent.what) {
				case keyDown:
				case autoKey:
					moreEvents = DoKeys(&theEvent);
					ch = inputBuffer[indexBuffer];
					break;

		  		default:
		  			Do1Event(&theEvent);
		  			break;
			    }
			}
		}
	cursorOff();
	}
else {
	// We still have characters in the buffer, so get
	// one from there and...

	ch = inputBuffer[++indexBuffer];
	if (ch == LINE_FEED)		// we have emptied the buffer
		indexBuffer = -1;
	}
return ch;
}

/****
 *	DoKeys
 *
 ****/

static Boolean DoKeys(EventRecord *theEvent)
{
Boolean moreEvents;

	moreEvents = true;

	if ((ApplicationIsPaused()) || (theEvent->modifiers & cmdKey)) {
		HandleKeys(theEvent);

		if (ApplicationIsPaused())
			cursorOff();
	}

    else {
		char ch;

		ObscureCursor();	// nice cursor disappearing as soon as we start typing [Fabrizio Oddone]
		ch = theEvent->message & charCodeMask;

		switch (ch) {
			case '\b':		// backspace == '\b' == 8
				if (indexBuffer >= 0) {
					Rect block;

					cursorOff();
					scrPutc(ESC);
					scrPutc('[');
					scrPutc('D');

					scrCharToRect(col, row, &block);
					EraseRect(&block);	// last character
					indexBuffer--;
				}
				else
					SysBeep(1);
				break;

			// control-U
			case 4:			// control-D
			case 26:		// control-Z
				KeyTypedIn(EOF);
				moreEvents = false;
				indexBuffer = 0;
				break;

			case '\r':		// return
				KeyTypedIn(LINE_FEED);
				moreEvents = false;
				indexBuffer = 0;
				break;

			default:
				KeyTypedIn(ch);
				break;
		}
	}

	return moreEvents;
}

/****
 *	KeyTypedIn
 *
 *	Store the key typed in, and display the character on the screen.
 *
 ****/

static void KeyTypedIn(char ch)
{

	// if buffer is one short of being full, then only allow
	// a LINE_FEED into it.  Any other character typed will
	// beep.

	if ((indexBuffer + 1 >= MaxInputBuffer) && (ch != LINE_FEED))
		SysBeep(1);		// maybe we should put a message someplace

	else {
		inputBuffer[++indexBuffer] = ch;

		cursorOff();
		scrSetCursor();
		scrPutc(ch);
	}
}

/****
 * scrPutc()
 *
 *		Display a character at the current cursor position.
 *		This routine uses a state transition machine to parse
 *		the escape sequences.  The transitions are defined above
 *		in the table variable.  Here we check current state and
 *		transition values and off-we go...
 *****/

static Boolean transition(char ch, char isItHere[])
{
register short i;

	for (i = 0; (isItHere[i] != '\0') && (ch != isItHere[i]); i++)
		;
	return (ch == isItHere[i]);
}
	
void scrPutc(char ch)					// public routine
{
	short curState, i, nxtState;
	Boolean	useDefault = true;

	// put the 'ch' to a file, if we have a file pointer
	if (fileEcho)
		fprintf(fileEcho, "%c", ch);

	SetPort(theWindow);
	parmState.ch = ch;			// store current ch in parameters

	// Loop throught the table for all transitions going out
	// of the current state...
	curState = table[currIdx].state;
	for (i = currIdx; curState == table[i].state; i++) {

		// if a transition can be made, then ...
		if (transition(ch, table[i].chcmp)) {
			short j;
			Boolean found;
	
			// Take this transition (look in the table for
			// the index to the gotoState).  If this fails, it's
			// because the table was not built correctly.

			found = false;
			nxtState = table[i].gotoState;
			for (j = 0; j < tableSize; j++) {
				if (table[j].state == nxtState) {
					currIdx = j;
					found = true;		// found it, so...
					break;				// get out of here
				}
			}

			if (found) {
				useDefault = false;
				if (table[i].action != doNothing)
					(*table[i].action)(&parmState);
			}
			else {
				char *a = "INTERNAL ERROR: Invalid Transition Table"
							" for TerminalEmulation";
				char *b = "Press <Return> to Quit";

				// DONT CALL scrPutc HERE!
				// this is an internal error, thus we should fail
				SysBeep(30);				// catch their attention
				scrClearScreen();
				scrGotoXY(0, rows/2);
				scrDrawText(a, 0, strlen(a), normal);
				scrNextRow();
				scrDrawText(b, 0, strlen(b), normal);
				scrGetc();
				gShellQuitting = true;
			}
		}
	}

	if (useDefault) {			// default action for all states
		doDraw(&parmState);		// is to draw the character and
		currIdx = 0;			// to return to the first state
	}
}

/****
 * scrPutStr()
 *
 *		Display a string starting at the current location.  Use
 *		scrPutc to display each character, thus this will recognize
 *		any ESC sequence and other special characters.
 *
 *****/

void	scrPutStr(char *str)
{
	for (;*str; str++)
		scrPutc(*str);
}


/*
 * Private Routines:  the routines that follow are used only
 * from inside of this file.  They are not available to the
 * users of this module.
 */

static	void	scrValidate(void)
{
if (theWindow == NULL) {
	// Fail miserably
	SysBeep(30);
	gShellQuitting = true;
	}
}

static	void	scrSetCursor(void)
{
	//SetPort(theWindow);
	MoveTo(colToX(col), rowToY(row));
	cursorUpdate();
}

static	void	scrDrawText(char *str, short offset,
						short len, short style)
{
Rect rect;

	//SetPort(theWindow);
	scrRectFromTo(&rect, col, row, col+len-1, row);
	EraseRect(&rect);

	if (style == ftNormal)		TextFace(normal);
	if (style & ftUnderline)	TextFace(underline);
	if (style & ftBlinking)		TextFace(bold+condense);
	DrawText(str, offset, len);
	if (style & ftReverse)
		InvertRect(&rect);
}

static	void	scrDrawChar(char ch, short style)
{
Rect rect;

	//SetPort(theWindow);
	scrCharToRect(col, row, &rect);
	EraseRect(&rect);
	scrSetCursor();

	if (style == ftNormal)		TextFace(normal);
	if (style & ftUnderline)	TextFace(underline);
	if (style & ftBlinking)		TextFace(bold+condense);
	DrawChar(ch);
	if (style & ftReverse)		// if reverse
		InvertRect(&rect);
}

static	void	scrEraseFromTo(short fromCol, short fromRow, 
								short toCol, short toRow)
{
register short r, c;
Rect rect;

	// Error checking - make sure these values are correct
	if (fromCol < 0)
		fromCol = 0;
	if (toCol > cols)
		toCol = cols;
	if (fromRow < 0)
		fromRow = 0;
	if (toRow > rows)
		toRow = rows;

	// Store spaces in area to be erased
	for (r = fromRow; r < toRow; r++)
		for (c = fromCol; c < toCol; c++) {
			screen[r][c] = ' ';
			faceStyle[r][c] = initialFace;
		}

	scrRectFromTo(&rect, fromCol, fromRow, toCol, toRow);
	//SetPort(theWindow);
	EraseRect(&rect);
	scrSetCursor();
}

static	void	scrCharToRect(short c, short r, Rect *rect)
{
rect->left = colToX(c);
rect->top = rowToY(r) - fontInfo.ascent;
rect->right = rect->left + width;
rect->bottom = rect->top + height;
}

static	void	scrRectFromTo(Rect *r, short fromCol, short fromRow, 
								short toCol, short toRow)
{
Rect topLeft, botRight;

scrCharToRect(fromCol, fromRow, &topLeft);
scrCharToRect(toCol, toRow, &botRight);
UnionRect(&topLeft, &botRight, r);
}

#ifdef OLD
/*** Transition Testing Routines 
 ***  they return true if the transition is to be made.  In
 ***  retrospect, this could have been accomplished with a string
 ***  a loop in scrPutc checking for a character to be in the string.
 ***/
static	Boolean chANY(char ch) { return true; }
static	Boolean chESC(char ch) { return (ch == ESC); }
static	Boolean chCSI(char ch) { return (ch == '['); }
static	Boolean chNumeric(char ch) { return (ch >= '0' && ch <= '9'); }
static	Boolean chSemicolon(char ch) { return ch == ';'; }
static	Boolean chMove(char ch) {
	return ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' ||
		   ch == 'H' || ch == 'f' || ch == 's' || ch == 'u';
}
static	Boolean chClear(char ch) {
	return ch == 'J' || ch == 'K';
}
static	Boolean	chVideo(char ch) { return ch == 'm'; }

static	Boolean chSingleChar(char ch) {
	return ch == LF || ch == FF || ch == BELL;
}
#endif

/*** Action Routines ***/
static	void	doDraw(Parms *p) 
{
	screen[row][col] = p->ch;
	faceStyle[row][col] = currentFace;
	scrDrawChar(p->ch, currentFace);
	scrNextCol();
}

static	void	doESC(Parms *p) 
{
	p->ch = ' ';
	p->col = 0; // current x position
	p->row = 0; // current y position
}

static	void	doNUM1(Parms *p)
{
	p->row = p->row * 10 + (p->ch - '0');
}

static	void	doNUM2(Parms *p) 
{
	p->col = p->col * 10 + (p->ch - '0');
}

static	void	doMove(Parms *p) 
{
short val;

	val = p->row;		// if there is only value in ESC sequence,
						// it is stored in 'row'

	// The switch statement changes the values for row, col.
	// The scrGoto at the bottom moves the cursor.
	switch (p->ch) {
		case 'A':		// move up one line
			do {
				scrPrevRow();
				val--;
			} while (val > 0);
			break;
		case 'B':		// move down one line
			do {
				scrNextRow();
				val--;
			} while (val > 0);
			break;
		case 'C':		// move to the right (forward) one col
			do {
				scrNextCol();
				val--;
			} while (val > 0);
			break;
		case 'D':		// move to the left (backward) one col
			do {
				scrPrevCol();
				val--;
			} while (val > 0);
			break;
		case 'H':		// direct cursor positioning (VT100)
		case 'f':		// direct cursor positioning (VT52)
			scrGotoXY(p->col, p->row);
			break;
		case 's':		// save cursor position
			savedCol = col;
			savedRow = row;
			break;
		case 'u':		// restore cursor position
			scrGotoXY(savedCol, savedRow);
			break;
	}

}

static	void	doVideo(Parms *p)
{
short val;

	val = p->row;		// if there is only value in ESC sequence,
						// it is stored in 'row'

	// The switch statement changes the values for row, col.
	// The scrGoto at the bottom moves the cursor.
	switch (val) {
		case 0:				// back to initial settings
			currentFace = initialFace;
			break;
		case 4:				// underline;
			currentFace += ftUnderline;
			break;
		case 5:				// blinking
			currentFace += ftBlinking;
			break;
		case 7:				// reverse video
			currentFace += ftReverse;
			break;
		default:
			// ignore any other of these kind
			break;
	}
}

static	void	doSingleChar(Parms *p)
{
	switch (p->ch) {
		case FF:
			scrClearScreen();
			break;
		case LF:
			col = 0;
			scrNextRow();
			break;
		case BELL:
			SysBeep(1);
			break;
	}
}


static	void	doClear(Parms *p) 
{
short fromRow, fromCol, toRow, toCol;
short val;

	val = p->row;		// if there is only value in ESC sequence,
						// it is stored in 'row'
	switch (p->ch) {
		case 'J':			// erasing display
			switch (val) {
				case 0:				// from current position to end of screen
					fromCol = col;
					fromRow = row;
					toCol = cols;
					toRow = rows;
					break;
				case 1:				// from start of screen to current position
					fromCol = 0;
					fromRow = 0;
					toCol = col;
					toRow = row;
					break;
				case 2:				// all of the screen
					fromCol = 0;
					fromRow = 0;
					toCol = cols;
					toRow = rows;
					break;
			}
			break;

		case 'K':			// erasing lines
			switch (val) {
				case 0:				// from current position to end of line
					fromCol = col;
					fromRow = row;
					toCol = cols;
					toRow = row;
					break;
				case 1:				// from start of line to current position
					fromCol = 0;
					fromRow = row;
					toCol = col;
					toRow = row;
					break;
				case 2:				// all of the line
					fromCol = 0;
					fromRow = row;
					toCol = cols;
					toRow = row;
					break;
			}
			break;
	}

	scrEraseFromTo(fromCol, fromRow, toCol, toRow);
}


static	void	scrNextRow(void)
{
	row++;
	if (row >= rows) {
		row--;
		scrScrollScreen(1);
	}
	scrSetCursor();
}

static	void	scrNextCol(void)
{
	col++;
	if (col >= cols) {
		col = 0;
		scrNextRow();
	}
	scrSetCursor();
}

static	void	scrPrevRow(void)
{
	if (row > 0)
		--row;
	scrSetCursor();
}

static	void	scrPrevCol(void)
{
	if (col > 0)
		--col;
	scrSetCursor();
}

static	void	scrScrollScreen(short i)
{
	if (i >= 1 && i <= rows) {
		Rect r;
		RgnHandle rgn;
		register int j, k;

		BlockMoveData(&screen[i][0], &screen[0][0], (rows - i) * cols);
		BlockMoveData(&faceStyle[i][0], &faceStyle[0][0], (rows - i) * cols);
		for (j = rows - i; j < rows; j++)
			for (k = 0; k < cols; k++) {
				screen[j][k] = ' ';
				faceStyle[j][k] = initialFace;
			}
		scrRectFromTo(&r, 0, 0, cols, rows);
		rgn = NewRgn();
		ScrollRect(&r, 0, -height * i, rgn);
		EraseRgn(rgn);
		InvalRgn(rgn);
		DisposeRgn(rgn);
	}
}
