/* LinearTransition.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "LinearTransition.h"
#include "Memory.h"


struct LinearTransRec
	{
		long						LastValue;
		long						Remainder;
		long						Limit;
		long						ValueIncrement;
		long						RemainderIncrement;
		MyBoolean				Negative;
		LinearTransRec*	Link;
	};


static LinearTransRec*			DeadRecordList = NIL;


/* open a new linear transition state record */
LinearTransRec*			NewLinearTransition(long Start, long Destination, long TicksToReach)
	{
		LinearTransRec*		TransRec;

		if (DeadRecordList == NIL)
			{
				TransRec = (LinearTransRec*)AllocPtrCanFail(sizeof(LinearTransRec),
					"LinearTransRec");
				if (TransRec == NIL)
					{
						return NIL;
					}
			}
		 else
			{
				TransRec = DeadRecordList;
				DeadRecordList = DeadRecordList->Link;
			}
		RefillLinearTransition(TransRec,Start,Destination,TicksToReach);
		return TransRec;
	}


#if DEBUG
static void					CheckTransRec(LinearTransRec* TransRec)
	{
		LinearTransRec*		Scan;

		Scan = DeadRecordList;
		while (Scan != NIL)
			{
				if (Scan == TransRec)
					{
						PRERR(ForceAbort,"CheckTransRec:  record is on the garbage list");
					}
				Scan = Scan->Link;
			}
	}
#else
#define CheckTransRec(x) ((void)0)
#endif


/* refill a linear transition with new state information */
void								RefillLinearTransition(LinearTransRec* TransRec, long Start,
											long Destination, long TicksToReach)
	{
		CheckPtrExistence(TransRec);
		CheckTransRec(TransRec);
		TransRec->LastValue = Start;
		TransRec->Remainder = 0;
		TransRec->Limit = TicksToReach;
		TransRec->ValueIncrement = (Destination - Start) / TicksToReach;
		TransRec->RemainderIncrement = (Destination - Start) % TicksToReach;
		if (TransRec->RemainderIncrement < 0)
			{
				TransRec->RemainderIncrement = - TransRec->RemainderIncrement;
				TransRec->Negative = True;
			}
		 else
			{
				TransRec->Negative = False;
			}
	}


/* get rid of a linear transition state record */
void								DisposeLinearTransition(LinearTransRec* TransRec)
	{
		CheckPtrExistence(TransRec);
		CheckTransRec(TransRec);
		TransRec->Link = DeadRecordList;
		DeadRecordList = TransRec;
	}


/* get rid of all cached transition state records */
void								FlushLinearTransitionRecords(void)
	{
		LinearTransRec*		Scan;

		Scan = DeadRecordList;
		while (Scan != NIL)
			{
				LinearTransRec*		Temp;

				Temp = Scan;
				Scan = Scan->Link;
				ReleasePtr((char*)Temp);
			}
		DeadRecordList = NIL;
	}


/* execute one cycle and return the value */
long								LinearTransitionUpdate(LinearTransRec* TransRec)
	{
		long							Adjustment;

		CheckPtrExistence(TransRec);
		CheckTransRec(TransRec);
		TransRec->LastValue += TransRec->ValueIncrement;
		TransRec->Remainder += TransRec->RemainderIncrement;
		if (TransRec->Remainder >= TransRec->Limit)
			{
				Adjustment = TransRec->Remainder / TransRec->Limit;
				TransRec->Remainder -= Adjustment * TransRec->Limit;
				if (TransRec->Negative)
					{
						Adjustment = - Adjustment;
					}
				TransRec->LastValue += Adjustment;
			}
		return TransRec->LastValue;
	}


/* execute multiple cycles and return the value */
long								LinearTransitionUpdateMultiple(LinearTransRec* TransRec,
											long NumCycles)
	{
		double						NewRemainder;
		long							WholePortion;

		CheckPtrExistence(TransRec);
		CheckTransRec(TransRec);

		TransRec->LastValue += TransRec->ValueIncrement * NumCycles;
		NewRemainder = TransRec->Remainder + ((double)TransRec->RemainderIncrement * NumCycles);
		WholePortion = NewRemainder / TransRec->Limit;
		TransRec->Remainder = NewRemainder - ((double)TransRec->Limit * WholePortion);
		if (TransRec->Negative)
			{
				WholePortion = - WholePortion;
			}
		TransRec->LastValue += WholePortion;
		return TransRec->LastValue;
	}
