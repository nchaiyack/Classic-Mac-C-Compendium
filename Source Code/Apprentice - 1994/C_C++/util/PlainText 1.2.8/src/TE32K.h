/*-------------------------------------------------------------------
This version of TE32K.c is based on code written by Roy Wood and enhanced 
by Patrick C. Beard, Dave Platt, and Teddy Slottow. I have modified the 
code extensively. The following is the legal notice supplied by Roy Wood.


					"The TE32K TextEdit Replacement
					==============================

							by Roy Wood
Legal Stuff:
============

TE32K is provided free of charge, since no-one will ever pay me anything 
for it anyway.  However, if you do use TE32K in a piece of software which 
you market, for every copy you sell, you are required to donate $1.00 to 
the World Wildlife Fund, 90 Eglinton Avenue East, Suite 504, Toronto, 
Ontario, Canada, M4P 2Z7."

To that I add the following copyright notice which applies to this version
and is intended to protect the what I feel is Roy's original intent.

				Copyright (c) 1993-1994 by Melburn R. Park

This code is free. It may be freely distributed and incorporated into other
programs, according to the conditions stated by Roy Wood, above. This code
may not be subsequently copyrighted, sold, or assigned in any way that 
restricts its use by any others, including its authors, in the ways that
have just been specified.

Melburn R. Park
Dept. Anatomy & Neurobiology
University of Tennessee
Memphis, Tennessee, USA
mpark@nb.utmem.edu

-------------------------------------------------------------------*/

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __SYSEQU__
#include <SysEqu.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __SCRAP__
#include <Scrap.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

extern pascal void doMessage(short);

#define FALSE 0
#define TRUE 1

#define __UNDOACTION__


/*--------------------------- Types ----------------------------*/
typedef	struct
{
	long	top,left,bottom,right;
} LongRect;


typedef	struct
{
	long	h,v;
} LongPoint;

typedef pascal Boolean (*TE32KProcPtr)(void);

typedef	struct
{
	LongRect			destRect;
	LongRect			viewRect;
	short				lineHeight;
	short				fontAscent;
	LongPoint			selPoint;
	long				selStart;
	long				selEnd;
	short				active;
	TE32KProcPtr		clikLoop;
	long				clickTime;
	long				clickLoc;
	short				clikStuff;
	long				caretTime;
	short				caretState;
	long				teLength;
	Handle				hText;
	short				txFont;
	char				txFace;
	short				txMode;
	short				txSize;
	short				tabWidth;
	short				tabChars;
	short				maxLineWidth;
	Boolean				crOnly;
	Boolean				wrapToLength;
	Boolean				showInvisibles;
	Boolean				autoIndent;
	RgnHandle			selRgn;
	GrafPtr				inPort;
	long				nLines;
	long				undoStart;
	long				undoEnd;
	long				undoDelta;
	Handle				undoBuf;
	short				resetUndo;
	short				theCharWidths[256];
#ifdef __PPCC__
	long				lineStarts[1];
#else
	long				lineStarts[];
#endif
	
} 	TE32KRec,*TE32KPtr,**TE32KHandle;
	
extern	void			SetLongRect(LongRect *,long,long,long,long);
extern	void			LongRectToRect(LongRect *,Rect *);
extern	void			RectToLongRect(Rect *,LongRect *);
extern	void    		OffsetLongRect(LongRect *, long, long);
extern	long 			indexToLine(long,TE32KHandle);
extern	long 			indexToParagraph(long,TE32KHandle);

extern	void			TE32KInit(void);
extern	TE32KHandle		TE32KNew(LongRect *,LongRect *);
extern	void			TE32KDispose(TE32KHandle);
extern	void 			TE32KCalText(TE32KHandle);
extern	void			TE32KSetText(Ptr,long,TE32KHandle);
extern	void			TE32KUseTextHandle(Handle,TE32KHandle);
extern	Handle			TE32KGetText(TE32KHandle);
extern	void			TE32KUpdate(LongRect *,TE32KHandle);
extern	void			TE32KScroll(long,long,TE32KHandle);
extern	void			TE32KActivate(TE32KHandle);
extern	void			TE32KDeactivate(TE32KHandle);
extern	void			TE32KIdle(TE32KHandle);
extern	Boolean			TE32KKey(unsigned char,TE32KHandle,short);
extern	void			TE32KClick(Point,unsigned char,TE32KHandle);
extern	void			TE32KSetSelect(long,long,TE32KHandle);
extern	OSErr			TE32KToScrap(void);
extern	OSErr			TE32KFromScrap(void);
extern	void			TE32KCopy(TE32KHandle);
extern	void			TE32KCut(TE32KHandle);
extern	void			TE32KDelete(TE32KHandle);
extern	void			TE32KInsert(Ptr,long,TE32KHandle);
extern	void			TE32KPaste(TE32KHandle);
extern	Handle			TE32KScrapHandle(void);
extern	long			TE32KGetScrapLen(void);
extern	void			TE32KSetScrapLen(long);
extern	void			TE32KGetPoint(long,LongPoint *,TE32KHandle);
extern	long			TE32KGetOffset(LongPoint *,TE32KHandle);
extern	void			TE32KSelView(TE32KHandle);
extern	void			positionView(TE32KHandle,long);
extern	void			TE32KSetFontStuff(short,short,short,short,TE32KHandle);
extern	void			TE32KAutoView(char, TE32KHandle);
extern	void			TE32KUndo(TE32KHandle);
extern void				xorCaret(TE32KHandle);

extern Handle			TE32KScrpHandle;
extern long				TE32KScrpLength;
extern TE32KHandle		ClickedTE32KH;
