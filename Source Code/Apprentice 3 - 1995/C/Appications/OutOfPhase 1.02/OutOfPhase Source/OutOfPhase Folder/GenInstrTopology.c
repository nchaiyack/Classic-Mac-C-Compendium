/* GenInstrTopology.c */
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

#include "GenInstrTopology.h"
#include "Array.h"
#include "OscillatorSpecifier.h"
#include "ModulationSpecifier.h"
#include "Memory.h"


/* here's the routine that recursively traverses the graph.  SourceOscillator */
/* is no longer on the Independency list and is on the Scratch list */
static MyBoolean			TraverseOscillator(OscillatorRec* SourceOscillator,
												ArrayRec* ScratchList, ArrayRec* IndependencyList)
	{
		ModulationSpecRec*	ListOfModulators;
		long								NumModulators;
		long								Scan;

		CheckPtrExistence(SourceOscillator);
		CheckPtrExistence(ScratchList);
		CheckPtrExistence(IndependencyList);
		ERROR(ArrayFindElement(ScratchList,SourceOscillator) != -1,PRERR(ForceAbort,
			"TraverseOscillator:  source oscillator is still on the scratch list"));
		ERROR(ArrayFindElement(IndependencyList,SourceOscillator) == -1,PRERR(ForceAbort,
			"TraverseOscillator:  source oscillator is not on the independency list"));

		ListOfModulators = OscillatorGetModulatorInputList(SourceOscillator);
		NumModulators = GetModulationSpecNumEntries(ListOfModulators);
		for (Scan = 0; Scan < NumModulators; Scan += 1)
			{
				OscillatorRec*			PossibleOscillator;
				long								Position;

				PossibleOscillator = GetModulationOscillatorRef(ListOfModulators,Scan);
				CheckPtrExistence(PossibleOscillator);
				Position = ArrayFindElement(ScratchList,PossibleOscillator);
				if (Position >= 0)
					{
						/* it's there -- remove it & put it on the scratch list */
						ArrayDeleteElement(ScratchList,Position);
						if (!ArrayAppendElement(IndependencyList,PossibleOscillator))
							{
								return False;
							}
					}
				 else
					{
						/* it wasn't there.  it must be on the list already.  just make sure. */
						ERROR(ArrayFindElement(IndependencyList,PossibleOscillator) == -1,
							PRERR(ForceAbort,"TraverseOscillator:  oscillator not on either list"));
					}
			}

		return True;
	}


/* this routine takes an array of oscillators and builds a series of independent */
/* oscillator groups based on which oscillators depend on others for modulation */
/* input.  the result is an array containing arrays of OscillatorRec's. */
/* for the moment, the ordering is undefined.  if this turns out to matter, or I */
/* implement optimized non-cyclic modulation, then this will need to be changed */
/* by putting them in the proper order and generating dependency annotations. */
struct ArrayRec*			BuildOscillatorLists(struct OscillatorRec** OscillatorArray,
												long NumOscillators)
	{
		ArrayRec*						MasterList;
		ArrayRec*						ScratchList;
		long								Scan;

		CheckPtrExistence(OscillatorArray);
		/* this is just a typical graph traversal algorithm. */
		/*   - take a node (oscillator) out of the list */
		/*   - add it to the independency list */
		/*   - for each oscillator it depends on which isn't in our list yet */
		/*      - add it to our list */
		/*      - remove it from the global list */
		/*      - recursively add the things it depends on. */
		/* this makes one list of independencies.  add it to our master list and */
		/* continue until there are no oscillators left */

		MasterList = NewArray();
		if (MasterList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		ScratchList = NewArray(); /* this list will contain all of the oscillators */
		if (ScratchList == NIL)
			{
			 FailurePoint2:
				while (ArrayGetLength(MasterList) != 0)
					{
						/* master list is an array of arrays. */
						DisposeArray((ArrayRec*)ArrayGetElement(MasterList,0));
						ArrayDeleteElement(MasterList,0);
					}
				DisposeArray(MasterList);
				goto FailurePoint1;
			}

		for (Scan = 0; Scan < NumOscillators; Scan += 1)
			{
				PRNGCHK(OscillatorArray,&(OscillatorArray[Scan]),sizeof(OscillatorArray[Scan]));
				if (!ArrayAppendElement(ScratchList,OscillatorArray[Scan]))
					{
					 FailurePoint3:
						DisposeArray(ScratchList);
						goto FailurePoint2;
					}
			}

		/* now, do it until we have no more things */
		while (ArrayGetLength(ScratchList) > 0)
			{
				ArrayRec*						IndependencyList;
				OscillatorRec*			Root;

				IndependencyList = NewArray();
				if (IndependencyList == NIL)
					{
					 FailurePoint4:
						goto FailurePoint3;
					}

				Root = (OscillatorRec*)ArrayGetElement(ScratchList,0);
				ArrayDeleteElement(ScratchList,0);
				if (!ArrayAppendElement(IndependencyList,Root))
					{
					 FailurePoint4a:
						DisposeArray(IndependencyList);
						goto FailurePoint4;
					}

				if (!TraverseOscillator(Root,ScratchList,IndependencyList))
					{
					 FailurePoint4b:
						goto FailurePoint4a;
					}

				if (!ArrayAppendElement(MasterList,IndependencyList))
					{
					 FailurePoint4c:
						goto FailurePoint4b;
					}
			}

		DisposeArray(ScratchList);

		return MasterList;
	}
