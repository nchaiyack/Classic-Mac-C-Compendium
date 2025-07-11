/* ModulationSpecifier.c */
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

#include "ModulationSpecifier.h"
#include "Memory.h"
#include "OscillatorSpecifier.h"
#include "OscillatorListSpecifier.h"
#include "DataMunging.h"
#include "Envelope.h"
#include "LFOListSpecifier.h"


typedef struct
	{
		/* how are the streams being modulated */
		ModulationTypes			ModulationType;

		/* which connection in the oscillator is being modulated */
		ModDestTypes				ModulationDestination;

		/* multiply our output by this before modulating (for magnitude control) */
		EnvelopeRec*				ScalingFactorEnvelope;
		LFOListSpecRec*			ScalingFactorLFOList;

		/* add this to our output before modulating (for incompleteness) */
		EnvelopeRec*				OriginAdjustEnvelope;
		LFOListSpecRec*			OriginAdjustLFOList;

		/* what oscillator are we modulating */
		char*								InputOscillatorName;

		/* reference to the oscillator being input from.  this is not made up */
		/* to date until the whole tree has been built. */
		OscillatorRec*			InputOscillatorReference;
	} ModEntryRec;


struct ModulationSpecRec
	{
		long								NumEntries;
		ModEntryRec*				ArrayOfSources;
	};


/* create new list of modulation source specifiers */
ModulationSpecRec*		NewModulationSpecifier(void)
	{
		ModulationSpecRec*	ModSpec;

		ModSpec = (ModulationSpecRec*)AllocPtrCanFail(sizeof(ModulationSpecRec),
			"ModulationSpecRec");
		if (ModSpec == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		ModSpec->ArrayOfSources = (ModEntryRec*)AllocPtrCanFail(0,"ModEntryRec");
		if (ModSpec->ArrayOfSources == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)ModSpec);
				goto FailurePoint1;
			}
		ModSpec->NumEntries = 0;
		return ModSpec;
	}


/* dispose a list of modulation source specifiers */
void									DisposeModulationSpecifier(ModulationSpecRec* ModSpec)
	{
		long								Scan;

		CheckPtrExistence(ModSpec);
		for (Scan = 0; Scan < ModSpec->NumEntries; Scan += 1)
			{
				ReleasePtr(ModSpec->ArrayOfSources[Scan].InputOscillatorName);
				DisposeEnvelope(ModSpec->ArrayOfSources[Scan].ScalingFactorEnvelope);
				DisposeEnvelope(ModSpec->ArrayOfSources[Scan].OriginAdjustEnvelope);
				DisposeLFOListSpecifier(ModSpec->ArrayOfSources[Scan].ScalingFactorLFOList);
				DisposeLFOListSpecifier(ModSpec->ArrayOfSources[Scan].OriginAdjustLFOList);
			}
		ReleasePtr((char*)ModSpec->ArrayOfSources);
		ReleasePtr((char*)ModSpec);
	}


/* append a new entry.  the object becomes owner of the name */
MyBoolean							AppendModulationSpecEntry(ModulationSpecRec* ModSpec,
												ModulationTypes ModulationType,
												ModDestTypes ModulationDestination,
												struct EnvelopeRec* ScalingEnvelope,
												struct LFOListSpecRec* ScalingLFOList,
												struct EnvelopeRec* OriginAdjustEnvelope,
												struct LFOListSpecRec* OriginAdjustLFOList,
												char* InputOscillatorName)
	{
		ModEntryRec*				NewArray;

		CheckPtrExistence(ModSpec);
		CheckPtrExistence(InputOscillatorName);
		CheckPtrExistence(ScalingEnvelope);
		CheckPtrExistence(ScalingLFOList);
		CheckPtrExistence(OriginAdjustEnvelope);
		CheckPtrExistence(OriginAdjustLFOList);
		NewArray = (ModEntryRec*)ResizePtr((char*)ModSpec->ArrayOfSources,
			sizeof(ModEntryRec) * (1 + ModSpec->NumEntries));
		if (NewArray == NIL)
			{
				return False;
			}
		ModSpec->ArrayOfSources = NewArray;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].ModulationType = ModulationType;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].ModulationDestination
			= ModulationDestination;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].ScalingFactorEnvelope = ScalingEnvelope;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].ScalingFactorLFOList = ScalingLFOList;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].OriginAdjustEnvelope = OriginAdjustEnvelope;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].OriginAdjustLFOList = OriginAdjustLFOList;
		ModSpec->ArrayOfSources[ModSpec->NumEntries].InputOscillatorName
			= InputOscillatorName;
		ModSpec->NumEntries += 1;
		return True;
	}


/* get the total number of entries in the modulation list */
long									GetModulationSpecNumEntries(ModulationSpecRec* ModSpec)
	{
		CheckPtrExistence(ModSpec);
		return ModSpec->NumEntries;
	}


/* resolve named references into actual oscillator references.  returns False */
/* if a name couldn't be resolved. */
MyBoolean							ResolveModulationReferences(ModulationSpecRec* ModSpec,
												struct OscillatorListRec* OscillatorList)
	{
		long								ModScan;
		long								OscLimit;

		CheckPtrExistence(ModSpec);
		CheckPtrExistence(OscillatorList);

		OscLimit = GetOscillatorListLength(OscillatorList);
		for (ModScan = 0; ModScan < ModSpec->NumEntries; ModScan += 1)
			{
				long								OscScan;
				OscillatorRec*			PossibleOscillator;

				for (OscScan = 0; OscScan < OscLimit; OscScan += 1)
					{
						char*								OscillatorName;

						PossibleOscillator = GetOscillatorFromList(OscillatorList,OscScan);
						OscillatorName = OscillatorGetName(PossibleOscillator);
						PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[ModScan].
							InputOscillatorName),sizeof(ModSpec->ArrayOfSources[ModScan].
							InputOscillatorName));
						if (PtrSize(OscillatorName) == PtrSize(ModSpec->ArrayOfSources[ModScan].
							InputOscillatorName))
							{
								if (MemEqu(OscillatorName,ModSpec->ArrayOfSources[ModScan].
									InputOscillatorName,PtrSize(OscillatorName)))
									{
										/* found it */
										ModSpec->ArrayOfSources[ModScan].InputOscillatorReference
											= PossibleOscillator;
										goto FoundItPoint;
									}
							}
					}
				return False; /* oops, an unresolved one */
			 FoundItPoint:
				;
			}

		return True;
	}


/* get the modulation type of one of the sources */
ModulationTypes				GetModulationOpType(ModulationSpecRec* ModSpec, long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationOpType:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		return ModSpec->ArrayOfSources[Index].ModulationType;
	}


/* get the destination of the modulation */
ModDestTypes					GetModulationDestType(ModulationSpecRec* ModSpec, long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationDestType:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		return ModSpec->ArrayOfSources[Index].ModulationDestination;
	}


/* get the oscillator name for one of the sources (this is the original string) */
char*									GetModulationOscillatorName(ModulationSpecRec* ModSpec, long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationOscillatorName:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		return ModSpec->ArrayOfSources[Index].InputOscillatorName;
	}


/* get the actual oscillator reference for one of the sources */
struct OscillatorRec*	GetModulationOscillatorRef(ModulationSpecRec* ModSpec, long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationOscillatorRef:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		CheckPtrExistence(ModSpec->ArrayOfSources[Index].InputOscillatorReference);
		return ModSpec->ArrayOfSources[Index].InputOscillatorReference;
	}


/* get the scaling factor envelope */
struct EnvelopeRec*		GetModulationScalingFactorEnvelope(ModulationSpecRec* ModSpec,
												long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationScalingFactorEnvelope:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		CheckPtrExistence(ModSpec->ArrayOfSources[Index].ScalingFactorEnvelope);
		return ModSpec->ArrayOfSources[Index].ScalingFactorEnvelope;
	}


/* get the scaling factor LFO list */
struct LFOListSpecRec*	GetModulationScalingFactorLFOList(ModulationSpecRec* ModSpec,
												long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationScalingFactorLFOList:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		CheckPtrExistence(ModSpec->ArrayOfSources[Index].ScalingFactorLFOList);
		return ModSpec->ArrayOfSources[Index].ScalingFactorLFOList;
	}


/* get the origin adjust envelope */
struct EnvelopeRec*		GetModulationOriginAdjustEnvelope(ModulationSpecRec* ModSpec,
												long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationOriginAdjustEnvelope:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		CheckPtrExistence(ModSpec->ArrayOfSources[Index].OriginAdjustEnvelope);
		return ModSpec->ArrayOfSources[Index].OriginAdjustEnvelope;
	}


/* get the origin adjust LFO list */
struct LFOListSpecRec*	GetModulationOriginAdjustLFOList(ModulationSpecRec* ModSpec,
												long Index)
	{
		CheckPtrExistence(ModSpec);
		ERROR((Index < 0) || (Index >= ModSpec->NumEntries),PRERR(ForceAbort,
			"GetModulationOriginAdjustLFOList:  index out of range"));
		PRNGCHK(ModSpec->ArrayOfSources,&(ModSpec->ArrayOfSources[Index]),
			sizeof(ModSpec->ArrayOfSources[Index]));
		CheckPtrExistence(ModSpec->ArrayOfSources[Index].OriginAdjustLFOList);
		return ModSpec->ArrayOfSources[Index].OriginAdjustLFOList;
	}
