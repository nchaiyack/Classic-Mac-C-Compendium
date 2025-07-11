/* InstrumentStructure.c */
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

#include "InstrumentStructure.h"
#include "Memory.h"
#include "LFOListSpecifier.h"
#include "OscillatorListSpecifier.h"


struct InstrumentRec
	{
		/* this is the overall loudness factor for the instrument, which can be used */
		/* to differentiate "loud" on a quite instrument vs. "loud" on a loud instrument. */
		InstrNumberType							OverallLoudness;

		/* list of LFO operators on the frequency */
		struct LFOListSpecRec*			FrequencyLFOList;

		/* list of oscillators */
		struct OscillatorListRec*		OscillatorList;
	};


/* create a new instrument specification record */
InstrumentRec*						NewInstrumentSpecifier(void)
	{
		InstrumentRec*			Instr;

		Instr = (InstrumentRec*)AllocPtrCanFail(sizeof(InstrumentRec),"InstrumentRec");
		if (Instr == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Instr->OverallLoudness = 0;
		Instr->FrequencyLFOList = NewLFOListSpecifier();
		if (Instr->FrequencyLFOList == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Instr);
				goto FailurePoint1;
			}
		Instr->OscillatorList = NewOscillatorListSpecifier();
		if (Instr->OscillatorList == NIL)
			{
			 FailurePoint3:
				DisposeLFOListSpecifier(Instr->FrequencyLFOList);
				goto FailurePoint2;
			}
		return Instr;
	}


/* dispose of the instrument specification record */
void											DisposeInstrumentSpecification(InstrumentRec* Instr)
	{
		CheckPtrExistence(Instr);
		DisposeOscillatorListSpecifier(Instr->OscillatorList);
		DisposeLFOListSpecifier(Instr->FrequencyLFOList);
		ReleasePtr((char*)Instr);
	}


/* get the overall loudness of the instrument */
InstrNumberType						GetInstrumentOverallLoudness(InstrumentRec* Instr)
	{
		CheckPtrExistence(Instr);
		return Instr->OverallLoudness;
	}


/* put a new value for overall loudness */
void											InstrumentSetOverallLoudness(InstrumentRec* Instr,
														double NewLoudness)
	{
		CheckPtrExistence(Instr);
		Instr->OverallLoudness = NewLoudness;
	}


/* get the instrument's frequency LFO list */
struct LFOListSpecRec*		GetInstrumentFrequencyLFOList(InstrumentRec* Instr)
	{
		CheckPtrExistence(Instr);
		return Instr->FrequencyLFOList;
	}


/* get the instrument's oscillator list */
struct OscillatorListRec*	GetInstrumentOscillatorList(InstrumentRec* Instr)
	{
		CheckPtrExistence(Instr);
		return Instr->OscillatorList;
	}


/* resolve oscillator name references in modulation lists. returns False if there */
/* are unresolvable names */
MyBoolean									ResolveInstrOscillatorModulations(InstrumentRec* Instr)
	{
		CheckPtrExistence(Instr);
		return ResolveOscillatorListModulators(Instr->OscillatorList);
	}
