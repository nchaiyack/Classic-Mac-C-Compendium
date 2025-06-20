/* LFOListSpecifier.c */
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

#include "LFOListSpecifier.h"
#include "Memory.h"
#include "LFOSpecifier.h"


struct LFOListSpecRec
	{
		long									NumLFOSpecs;
		struct LFOSpecRec**		Array;
	};


/* allocate a new LFO spec list */
LFOListSpecRec*				NewLFOListSpecifier(void)
	{
		LFOListSpecRec*			LFOListSpec;

		LFOListSpec = (LFOListSpecRec*)AllocPtrCanFail(sizeof(LFOListSpecRec),
			"LFOListSpecRec");
		if (LFOListSpec == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		LFOListSpec->Array = (struct LFOSpecRec**)AllocPtrCanFail(0,"LFOListSpecRec Array");
		if (LFOListSpec->Array == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)LFOListSpec);
				goto FailurePoint1;
			}
		LFOListSpec->NumLFOSpecs = 0;
		return LFOListSpec;
	}


/* dispose of an LFO spec list and all the specs inside of it */
void									DisposeLFOListSpecifier(LFOListSpecRec* LFOListSpec)
	{
		long								Scan;

		CheckPtrExistence(LFOListSpec);
		for (Scan = 0; Scan < LFOListSpec->NumLFOSpecs; Scan += 1)
			{
				PRNGCHK(LFOListSpec->Array,&(LFOListSpec->Array[Scan]),
					sizeof(LFOListSpec->Array[Scan]));
				DisposeLFOSpecifier(LFOListSpec->Array[Scan]);
			}
		ReleasePtr((char*)LFOListSpec->Array);
		ReleasePtr((char*)LFOListSpec);
	}


/* get a LFOSpecRec out of the list */
struct LFOSpecRec*		LFOListSpecGetLFOSpec(LFOListSpecRec* LFOListSpec, long Index)
	{
		CheckPtrExistence(LFOListSpec);
		ERROR((Index < 0) || (Index >= LFOListSpec->NumLFOSpecs),PRERR(ForceAbort,
			"LFOListSpecGetLFOSpec:  index out of range"));
		PRNGCHK(LFOListSpec->Array,&(LFOListSpec->Array[Index]),
			sizeof(LFOListSpec->Array[Index]));
		return LFOListSpec->Array[Index];
	}


/* create a new LFO spec entry in the list */
MyBoolean							LFOListSpecAppendNewEntry(LFOListSpecRec* LFOListSpec,
												struct LFOSpecRec* NewEntry)
	{
		struct LFOSpecRec**	NewArray;

		CheckPtrExistence(LFOListSpec);
		NewArray = (struct LFOSpecRec**)ResizePtr((char*)LFOListSpec->Array,
			sizeof(struct LFOSpecRec*) * (LFOListSpec->NumLFOSpecs + 1));
		if (NewArray == NIL)
			{
				return False;
			}
		LFOListSpec->Array = NewArray;
		LFOListSpec->Array[LFOListSpec->NumLFOSpecs] = NewEntry;
		LFOListSpec->NumLFOSpecs += 1;
		return True;
	}


/* get the number of elements in the list */
long									LFOListSpecGetNumElements(LFOListSpecRec* LFOListSpec)
	{
		CheckPtrExistence(LFOListSpec);
		return LFOListSpec->NumLFOSpecs;
	}
