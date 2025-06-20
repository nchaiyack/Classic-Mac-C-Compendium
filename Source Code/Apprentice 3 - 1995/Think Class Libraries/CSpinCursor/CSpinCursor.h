/******************************************************************************
	CSpinCursor.h
	
	Implements a spinning colorcursor. A list of 'crsr' cursors is specified 
	with an'acur' resource. Repeatedly invoking the Spin() method sequences
	through the list of cursors.
	
	SUPERCLASS = CObject.c
	
	Copyright � 1993 Martin R. Wachter. All rights reserved.

******************************************************************************/

#define kacurID				1024
#define kthresholdTime		1
#define	kintervalTime		2

#define	_H_CSpinCursor

#include "CObject.h"					/* Interface for its superclass		*/

typedef	struct	acurRec	{				/* Modified 'acur' resource format	*/
	short		numCursors;				/*   Number of cursors in list		*/
	short		counter;				/*   Counter (not used by us)		*/
	CCrsrHandle	cursors[4];				/*   List of Cursor handles			*/
} acurRec, **acurHand;


struct CSpinCursor : public CObject {	/* Class Declaration				*/

	acurHand	acurH;					/* Handle to 'acur' resource		*/
	short		threshold;				/* Tick delay before first spin		*/
	short		interval;				/* Tick delay between spins			*/
	short		cursIndex;				/* Index into CURSlist				*/
	long		nextSpinTime;			/* Tick count for the next spin		*/

	void		ISpinCursor(short acurID, short aThreshold, short anInterval);
	void		Dispose(void);
	void		Reset(void);
	void		SetTimes(short newThreshold, short newInterval);
	void		Spin(void);
};