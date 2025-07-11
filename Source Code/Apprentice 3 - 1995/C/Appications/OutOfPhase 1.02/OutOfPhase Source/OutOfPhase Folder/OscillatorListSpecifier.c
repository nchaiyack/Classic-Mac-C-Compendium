/* OscillatorListSpecifier.c */
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

#include "OscillatorListSpecifier.h"
#include "Memory.h"
#include "OscillatorSpecifier.h"


struct OscillatorListRec
	{
		long										NumOscillators;
		struct OscillatorRec**	OscillatorArray;
	};


/* create a new array of oscillators */
OscillatorListRec*			NewOscillatorListSpecifier(void)
	{
		OscillatorListRec*		OscList;

		OscList = (OscillatorListRec*)AllocPtrCanFail(sizeof(OscillatorListRec),
			"OscillatorListRec");
		if (OscList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		OscList->OscillatorArray = (struct OscillatorRec**)AllocPtrCanFail(0,
			"struct OscillatorRec");
		if (OscList->OscillatorArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)OscList);
				goto FailurePoint1;
			}
		OscList->NumOscillators = 0;
		return OscList;
	}


/* dispose of oscillator list */
void										DisposeOscillatorListSpecifier(OscillatorListRec* OscList)
	{
		long									Scan;

		CheckPtrExistence(OscList);
		for (Scan = 0; Scan < OscList->NumOscillators; Scan += 1)
			{
				PRNGCHK(OscList->OscillatorArray,&(OscList->OscillatorArray[Scan]),
					sizeof(OscList->OscillatorArray[Scan]));
				DisposeOscillatorSpecifier(OscList->OscillatorArray[Scan]);
			}
		ReleasePtr((char*)OscList->OscillatorArray);
		ReleasePtr((char*)OscList);
	}


/* append a new oscillator */
MyBoolean								AppendOscillatorToList(OscillatorListRec* OscList,
													struct OscillatorRec* NewOscillator)
	{
		struct OscillatorRec**	NewList;

		CheckPtrExistence(OscList);
		CheckPtrExistence(NewOscillator);
		NewList = (struct OscillatorRec**)ResizePtr((char*)OscList->OscillatorArray,
			sizeof(struct OscillatorRec*) * (1 + OscList->NumOscillators));
		if (NewList == NIL)
			{
				return False;
			}
		OscList->OscillatorArray = NewList;
		OscList->OscillatorArray[OscList->NumOscillators] = NewOscillator;
		OscList->NumOscillators += 1;
		return True;
	}


/* get one of the oscillators from the list */
struct OscillatorRec*		GetOscillatorFromList(OscillatorListRec* OscList, long Index)
	{
		CheckPtrExistence(OscList);
		ERROR((Index < 0) || (Index >= OscList->NumOscillators),PRERR(ForceAbort,
			"GetOscillatorFromList:  index is out of range"));
		PRNGCHK(OscList->OscillatorArray,&(OscList->OscillatorArray[Index]),
			sizeof(OscList->OscillatorArray[Index]));
		return OscList->OscillatorArray[Index];
	}


/* find out how many oscillators there are in the list */
long										GetOscillatorListLength(OscillatorListRec* OscList)
	{
		CheckPtrExistence(OscList);
		return OscList->NumOscillators;
	}


/* resolve all modulation references.  returns False if it couldn't be done */
MyBoolean								ResolveOscillatorListModulators(OscillatorListRec* OscList)
	{
		long									Scan;

		CheckPtrExistence(OscList);
		for (Scan = 0; Scan < OscList->NumOscillators; Scan += 1)
			{
				PRNGCHK(OscList->OscillatorArray,&(OscList->OscillatorArray[Scan]),
					sizeof(OscList->OscillatorArray[Scan]));
				if (!ResolveOscillatorModulators(OscList->OscillatorArray[Scan],OscList))
					{
						return False;
					}
			}
		return True;
	}
