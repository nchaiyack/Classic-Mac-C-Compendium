/*
** CTerminalPane.h
**
**	TerminalPane library
**	Terminal display pane
**
**	Copyright � 1993-94, FrostByte Design / Eric Scouten
**
*/


#pragma once
#include "CPanorama.h"


/*
** These constants define the terminal size. You may change them here. With a little
** work, these parameters could be changed on the fly. But I haven�t been so ambitious. :-P
*/

#define maxX				80					// width of screen
#define maxY				24					// height of screen

#define pixelsX				6					// default font char width (Monaco 9)
#define pixelsY				11					// default font char height

#define offsetX				4					// horizontal offset from screen edge
#define offsetY				4					// vertical offset from screen edge

#define sizeX				488					// 80 columns * 6 pixels + 8 margin
#define sizeY				272					// 24 rows * 11 pixels + 8 margin


// define the characters we will recognize

#ifndef _ASCII
enum {										// standard ASCII keycodes
	charNUL = 0,
	charBEL = 7,
	charBS,
	charHT,
	charLF,
	charVT,
	charFF,
	charCR,
	charDEL = 127
};
#define _ASCII 1
#endif


/*______________________________________________________________________
**
** CTerminalPane
**
**	This class provides a rudimentary terminal emulator. It�s nothing fancy or snazzy.
**	It doesn�t do VT100; it doesn�t do scrollback; it doesn�t handle any kind of formatting.
**
*/

class CTerminalPane : public CPanorama {

	TCL_DECLARE_CLASS;

protected:
	char			theScreen[maxY][maxX];			// contents of the screen
	short		theColumn;					// terminal cursor column number
	short		theLine;						// terminal cursor line number
	Boolean		blinkCursor;					// cursor blinking is enabled
	Boolean		cursorVis;					// cursor is inverted
	Boolean		disableKeyScroll;				// disable Home/End/PageUp/PageDown
	short		lastCursorCol;					// position of displayed cursor
	short		lastCursorLine;
	long			lastCursorTick;					// time at last cursor flash
	short		charsToInvalLine;				// trigger optimization to invalidate entire line at once

	
	// contructors
	
public:
				CTerminalPane();
				CTerminalPane(CView* anEnclosure, CBureaucrat* aSupervisor,
							short aWidth = 0, short aHeight = 0,
							short aHEncl = 0, short aVEncl = 0,
							SizingOption aHSizing = sizELASTIC,
							SizingOption aVSizing = sizELASTIC);
	void			ITerminalPane(CView* anEnclosure, CBureaucrat* aSupervisor,
							short aWidth, short aHeight, short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing);
private:
	void			CTerminalPaneX();

	// drawing
	
public:
	virtual void	Draw(Rect* area);
	
	
	// blinking cursor support
	
	virtual void	Dawdle(long* maxSleep);
	virtual Boolean	BecomeGopher(Boolean fBecoming);
	virtual void	SetBlinking(Boolean blinkMode);


	// scrolling
	
	virtual void	ScrollToSelection();
	virtual void	DisableKeyScroll(Boolean disable);
	virtual void	DoKeyDown(char theChar, Byte keyCode, EventRecord* macEvent);
	
	
	// terminal emulation methods
	
	virtual void	DoClearScreen();
	virtual void	DoWriteChar(char theChar);
	virtual void	DoWriteStr(char* theStr);
	virtual void	DoWriteBfr(char* theStr, short theSize);
	virtual void	DoWriteCharNum(char theChar, char leftBracket, char rightBracket);
	virtual void	DoEraseChar();
	virtual void	DoEraseLine();
	
	
	// private screen handling methods
	
protected:
	virtual void	ClearToEOL(short col, short line);
	virtual void	ClearToEOS(short col, short line);
	virtual void	ScrollTerm();
	virtual void	InvalCharRect(short left, short top, short right, short bottom);
	virtual void	CursorMoved();
	virtual void	CalcCharRect(short left, short top, short right, short bottom, LongRect* theRect);
	virtual void	InvertCursor(short col, short line);

};
