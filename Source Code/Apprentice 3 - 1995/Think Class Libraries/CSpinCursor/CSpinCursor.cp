/******************************************************************************
	CSpinCursor.cp
	
	Implements a spinning color cursor. A list of 'crsr' cursors is specified 
	with an'acur' resource. Repeatedly invoking the Spin() method sequences
	through the list of cursors.
	
	SUPERCLASS = CObject.c
	
	Copyright © 1993 Martin R. Wachter. All rights reserved.
	Based on some code I found from GHD on AOL a few years ago.
	
******************************************************************************/

#include "CSpinCursor.h"

typedef	struct	acurCURS	{			/* For each cursor in an 'acur'		*/
	short	resID;						/*   CURS resource ID				*/
	short	padding;					/*   Two bytes of padding			*/
} acurCURS;

typedef	struct	acurTemplate	{		/* Resource template for 'acur'		*/
	short		numCursors;				/*   Number of cursors in list		*/
	short		counter;				/*   Counter (not used by us)		*/
	acurCURS	CURSlist[8];				/*   'CURS' ID/padding pairs		*/
} acurTemplate, **acurTemplateH;


/******************************************************************************
ISpinCursor

	Initialize a Spinner object
******************************************************************************/
 
void	CSpinCursor::ISpinCursor(
	short	acurID,						/* 'acur' resource ID				*/
	short	aThreshold,					/* Tick delay before first spin		*/
	short	anInterval)					/* Ticks between spins				*/
{
	register acurTemplateH	tempH;		/* 'acur' handle with entries		*/
										/*   as ID & padding pairs			*/
	register short			i;			/* Loop counter						*/
	register CCrsrHandle		aCursor;/* Handle to 'CURS' resource		*/
	
										/* Cursor IDs are in an 'acur'		*/
										/*   resource						*/
	tempH = (acurTemplateH) GetResource('acur', acurID);
	
	DetachResource((Handle)tempH);				/* We are going to change the		*/
	HNoPurge((Handle)tempH);					/*   'acur' resource in memory.		*/
										/*   Treat it as a regular handle	*/
										/*   and make it nonpurgeable.		*/
	
		/* Cursor IDs list entries in the 'acur' are 4 bytes long.	*/
		/* The first 2 bytes are a 'CURS' resource ID and the		*/
		/* last 2 bytes are padding. This is so that the ID/padding	*/
		/* entry can be replaced with a handle (4 bytes long) to	*/
		/* the corresponding 'CURS' resource at runtime. "tempH"	*/
		/* and "acurH" both refer to the same handle. "tempH"		*/
		/* treats the entries as ID/padding pairs, whereas "acurH"	*/
		/* treats the entries as cursor handles.					*/
		
	acurH = (acurHand) tempH;
	
	for (i = 0; i < (**tempH).numCursors; i++) {
										/* Get handle to Cursor				*/
		aCursor = GetCCursor((**tempH).CURSlist[i].resID);
		HNoPurge((Handle)aCursor);				/* Make it nonpurgeable				*/
		
										/* Replace 'CURS' ID/padding pairs	*/
										/*   with a handle to the Cursor	*/
		(**acurH).cursors[i] = aCursor;
	}
	
	SetTimes(aThreshold, anInterval);	/* Specify delay times				*/
	Reset();							/* Initialize counters				*/
}


/******************************************************************************
Dispose

	Dispose of a Spinner
******************************************************************************/

void	CSpinCursor::Dispose()
{
short	i;

	for (i = 0; i < (**acurH).numCursors; i++) {
		DisposCCursor((**acurH).cursors[i]);
	}

	DisposHandle((Handle)acurH);		/* Throw out our modified copy		*/
										/*   of the 'acur' resource			*/
	inherited::Dispose();
}


/******************************************************************************
Reset

	Reset counters to initial values. Subsequent spins will start with
	the first cursor and will occur after the "threshold" delay.
******************************************************************************/
 
void	CSpinCursor::Reset()
{
	cursIndex = 0;						/* Start with the first cursor		*/
	nextSpinTime = 0;					/* Indicates first spin is next		*/
}


/******************************************************************************
SetTimes

	Specifiy values for the threshold delay before the first spin
	and the interval between spins
******************************************************************************/

void	CSpinCursor::SetTimes(
	short	newThreshold,
	short	newInterval)
{
	threshold = newThreshold;			/* Set instance variables			*/
	interval = newInterval;
}


/******************************************************************************
Spin

	Spin the cursor. Send this message repeatedly during long delays
	to spin the cursor. Cursor spinning is accomplished by changing
	the cursor after a specified time interval.
******************************************************************************/

void	CSpinCursor::Spin()
{
	register long	theTickCount;
	
	theTickCount = TickCount();			/* Time in ticks since startup		*/
	
	if (nextSpinTime == 0) {			/* Cursor has not yet spun			*/
										/* Set time counter so that the		*/
										/*   first spin will occur after	*/
										/*   "threshold" ticks				*/
		nextSpinTime = theTickCount + threshold;
	
	} else if (theTickCount >= nextSpinTime) {
										/* Enough time has passed. Change	*/
										/*   the cursor						*/
		SetCCursor((**acurH).cursors[cursIndex]);
		
		cursIndex++;					/* Increment cursor index			*/
		if (cursIndex >= (**acurH).numCursors) {
			cursIndex = 0;				/* Wrap around to the first one		*/
		}
										/* Next spin will occur after at	*/
										/*   least "interval" ticks			*/
		nextSpinTime = theTickCount + interval;
	}
}