/* ModulationOscControl.c */
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

#include "ModulationOscControl.h"
#include "ModulationSpecifier.h"
#include "WaveTableOscControl.h"
#include "SampleOscControl.h"
#include "FastModulation.h"
#include "OscillatorSpecifier.h"
#include "Array.h"
#include "Memory.h"
#include "LFOListSpecifier.h"
#include "LFOGenerator.h"
#include "EnvelopeState.h"
#include "Envelope.h"
#include "ErrorDaemon.h"


typedef struct
	{
		OscillatorTypes						OscillatorType;
		union
			{
				WaveTableTemplateRec*			WaveTableTemplate;
				SampleTemplateRec*				SampleTemplate;
			} u;
	} TemplateElemRec;


struct ModOscTemplateRec
	{
		long											NumberOfOscillators;

		/* this is an array of records, NumberOfOscillators elements long */
		TemplateElemRec*					TemplateArray;

		/* this is an array of integers, one for each template */
		long*											PhaseGenNumElements;
		/* these are arrays of arrays.  there is one array for each template, and */
		/* that array has PhaseGenNumElements[i] elements in it */
		ModulationTypes**					PhaseGenModulateHow;
		long**										PhaseGenIndirectionTable;
		EnvelopeRec***						PhaseGenScalingFactorEnvelope;
		LFOListSpecRec***					PhaseGenScalingFactorLFOList;
		EnvelopeRec***						PhaseGenOriginAdjustEnvelope;
		LFOListSpecRec***					PhaseGenOriginAdjustLFOList;

		/* this is an array of integers, one for each template */
		long*											OutputNumElements;
		/* these are arrays of arrays.  there is one array for each template, and */
		/* that array has OutputNumElements[i] elements in it */
		ModulationTypes**					OutputModulateHow;
		long**										OutputIndirectionTable;
		EnvelopeRec***						OutputScalingFactorEnvelope;
		LFOListSpecRec***					OutputScalingFactorLFOList;
		EnvelopeRec***						OutputOriginAdjustEnvelope;
		LFOListSpecRec***					OutputOriginAdjustLFOList;

		/* this flag indicates whether we are doing stereo playback or not */
		MyBoolean									StereoPlayback;

		/* envelope update rate */
		float											TicksPerSecond;

		/* list link */
		ModOscTemplateRec*				Next;
	};


typedef struct
	{
		OscillatorTypes						OscillatorType;
		union
			{
				WaveTableStateRec*				WaveTableState;
				SampleStateRec*						SampleState;
			} u;
	} StateElemRec;


struct ModOscStateRec
	{
		/* array of state objects, one for each oscillator (Template.NumberOfOscillators) */
		StateElemRec*							StateArray;

		/* this is an array of fast fixeds, one for each oscillator.  it remembers the */
		/* previous output values of all oscillators for the purposes of feedback. */
		float*										OldValues;

		/* this is like OldValues, except that the oscillator values from the current */
		/* stage of execution are put in here.  this is to avoid changing the OldValues */
		/* array while a synthesis step is in progress, so that modulation with feedback */
		/* is order independent.  at the end of the cycle, this and OldValues are */
		/* swapped.  (we don't want to allocate an array each cycle, so we keep this one */
		/* around to save time.) */
		float*										BuildOldValues;

		/* a copy of the template information.  do NOT dispose any of the pointers in it */
		ModOscTemplateRec					Template;

		/* workspace for each envelope iteration of phase gen modulation parameters */
		float**										PhaseGenModulationScaling;
		float**										PhaseGenModulationOrigin;
		EvalEnvelopeRec***				PhaseGenScalingFactorEnvelope;
		LFOGenRec***							PhaseGenScalingFactorLFO;
		EvalEnvelopeRec***				PhaseGenOriginAdjustEnvelope;
		LFOGenRec***							PhaseGenOriginAdjustLFO;

		/* workspace for each envelope iteration of output modulation parameters */
		float**										OutputModulationScaling;
		float**										OutputModulationOrigin;
		EvalEnvelopeRec***				OutputScalingFactorEnvelope;
		LFOGenRec***							OutputScalingFactorLFO;
		EvalEnvelopeRec***				OutputOriginAdjustEnvelope;
		LFOGenRec***							OutputOriginAdjustLFO;

		/* list thing */
		ModOscStateRec*						Next;
	};




static ModOscTemplateRec*				TemplateFreeList = NIL;
static ModOscStateRec*					StateFreeList = NIL;


/* get rid of all cached memory for state or template records */
void									FlushModOscControl(void)
	{
		while (TemplateFreeList != NIL)
			{
				ModOscTemplateRec*	Temp;

				Temp = TemplateFreeList;
				TemplateFreeList = TemplateFreeList->Next;
				ReleasePtr((char*)Temp);
			}

		while (StateFreeList != NIL)
			{
				ModOscStateRec*			Temp;

				Temp = StateFreeList;
				StateFreeList = StateFreeList->Next;
				ReleasePtr((char*)Temp);
			}
	}


#if DEBUG
static void						ValidateTemplate(ModOscTemplateRec* Template)
	{
		ModOscTemplateRec*	Scan;

		CheckPtrExistence(Template);
		Scan = TemplateFreeList;
		while (Scan != NIL)
			{
				if (Scan == Template)
					{
						PRERR(ForceAbort,"ValidateTemplate:  template is on free list");
					}
				Scan = Scan->Next;
			}
	}
#else
#define ValidateTemplate(x) ((void)0)
#endif


#if DEBUG
static void						ValidateState(ModOscStateRec* State)
	{
		ModOscStateRec*			Scan;

		CheckPtrExistence(State);
		Scan = StateFreeList;
		while (Scan != NIL)
			{
				if (Scan == State)
					{
						PRERR(ForceAbort,"ValidateState:  state is on free list");
					}
				Scan = Scan->Next;
			}
	}
#else
#define ValidateState(x) ((void)0)
#endif


/* perform one envelope update cycle */
void									UpdateModOscEnvelopes(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				ERROR((State->StateArray[Scan].OscillatorType != eOscillatorSampled)
					&& (State->StateArray[Scan].OscillatorType != eOscillatorWaveTable),
					PRERR(ForceAbort,"UpdateModOscEnvelopes:  bad oscillator type"));
				if (State->StateArray[Scan].OscillatorType == eOscillatorSampled)
					{
						UpdateSampleEnvelopes(State->StateArray[Scan].u.SampleState);
					}
				 else
					{
						UpdateWaveTableEnvelopes(State->StateArray[Scan].u.WaveTableState);
					}
				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						PRNGCHK(State->PhaseGenModulationScaling,
							&(State->PhaseGenModulationScaling[Scan]),
							sizeof(State->PhaseGenModulationScaling[Scan]));
						PRNGCHK(State->PhaseGenModulationScaling[Scan],
							&(State->PhaseGenModulationScaling[Scan][Index]),
							sizeof(State->PhaseGenModulationScaling[Scan][Index]));
						PRNGCHK(State->PhaseGenScalingFactorLFO,
							&(State->PhaseGenScalingFactorLFO[Scan]),
							sizeof(State->PhaseGenScalingFactorLFO[Scan]));
						PRNGCHK(State->PhaseGenScalingFactorLFO[Scan],
							&(State->PhaseGenScalingFactorLFO[Scan][Index]),
							sizeof(State->PhaseGenScalingFactorLFO[Scan][Index]));
						PRNGCHK(State->PhaseGenScalingFactorEnvelope,
							&(State->PhaseGenScalingFactorEnvelope[Scan]),
							sizeof(State->PhaseGenScalingFactorEnvelope[Scan]));
						PRNGCHK(State->PhaseGenScalingFactorEnvelope[Scan],
							&(State->PhaseGenScalingFactorEnvelope[Scan][Index]),
							sizeof(State->PhaseGenScalingFactorEnvelope[Scan][Index]));
						State->PhaseGenModulationScaling[Scan][Index]
							= FastFixed2Float(LFOGenUpdateCycle(
								State->PhaseGenScalingFactorLFO[Scan][Index],
								EnvelopeUpdate(State->PhaseGenScalingFactorEnvelope[Scan][Index])));
						PRNGCHK(State->PhaseGenModulationOrigin,
							&(State->PhaseGenModulationOrigin[Scan]),
							sizeof(State->PhaseGenModulationOrigin[Scan]));
						PRNGCHK(State->PhaseGenModulationOrigin[Scan],
							&(State->PhaseGenModulationOrigin[Scan][Index]),
							sizeof(State->PhaseGenModulationOrigin[Scan][Index]));
						PRNGCHK(State->PhaseGenOriginAdjustLFO,
							&(State->PhaseGenOriginAdjustLFO[Scan]),
							sizeof(State->PhaseGenOriginAdjustLFO[Scan]));
						PRNGCHK(State->PhaseGenOriginAdjustLFO[Scan],
							&(State->PhaseGenOriginAdjustLFO[Scan][Index]),
							sizeof(State->PhaseGenOriginAdjustLFO[Scan][Index]));
						PRNGCHK(State->PhaseGenOriginAdjustEnvelope,
							&(State->PhaseGenOriginAdjustEnvelope[Scan]),
							sizeof(State->PhaseGenOriginAdjustEnvelope[Scan]));
						PRNGCHK(State->PhaseGenOriginAdjustEnvelope[Scan],
							&(State->PhaseGenOriginAdjustEnvelope[Scan][Index]),
							sizeof(State->PhaseGenOriginAdjustEnvelope[Scan][Index]));
						State->PhaseGenModulationOrigin[Scan][Index]
							= FastFixed2Float(LFOGenUpdateCycle(
								State->PhaseGenOriginAdjustLFO[Scan][Index],
								EnvelopeUpdate(State->PhaseGenOriginAdjustEnvelope[Scan][Index])));
					}
				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						PRNGCHK(State->OutputModulationScaling,
							&(State->OutputModulationScaling[Scan]),
							sizeof(State->OutputModulationScaling[Scan]));
						PRNGCHK(State->OutputModulationScaling[Scan],
							&(State->OutputModulationScaling[Scan][Index]),
							sizeof(State->OutputModulationScaling[Scan][Index]));
						PRNGCHK(State->OutputScalingFactorLFO,
							&(State->OutputScalingFactorLFO[Scan]),
							sizeof(State->OutputScalingFactorLFO[Scan]));
						PRNGCHK(State->OutputScalingFactorLFO[Scan],
							&(State->OutputScalingFactorLFO[Scan][Index]),
							sizeof(State->OutputScalingFactorLFO[Scan][Index]));
						PRNGCHK(State->OutputScalingFactorEnvelope,
							&(State->OutputScalingFactorEnvelope[Scan]),
							sizeof(State->OutputScalingFactorEnvelope[Scan]));
						PRNGCHK(State->OutputScalingFactorEnvelope[Scan],
							&(State->OutputScalingFactorEnvelope[Scan][Index]),
							sizeof(State->OutputScalingFactorEnvelope[Scan][Index]));
						State->OutputModulationScaling[Scan][Index]
							= FastFixed2Float(LFOGenUpdateCycle(
								State->OutputScalingFactorLFO[Scan][Index],
								EnvelopeUpdate(State->OutputScalingFactorEnvelope[Scan][Index])));
						PRNGCHK(State->OutputModulationOrigin,
							&(State->OutputModulationOrigin[Scan]),
							sizeof(State->OutputModulationOrigin[Scan]));
						PRNGCHK(State->OutputModulationOrigin[Scan],
							&(State->OutputModulationOrigin[Scan][Index]),
							sizeof(State->OutputModulationOrigin[Scan][Index]));
						PRNGCHK(State->OutputOriginAdjustLFO,
							&(State->OutputOriginAdjustLFO[Scan]),
							sizeof(State->OutputOriginAdjustLFO[Scan]));
						PRNGCHK(State->OutputOriginAdjustLFO[Scan],
							&(State->OutputOriginAdjustLFO[Scan][Index]),
							sizeof(State->OutputOriginAdjustLFO[Scan][Index]));
						PRNGCHK(State->OutputOriginAdjustEnvelope,
							&(State->OutputOriginAdjustEnvelope[Scan]),
							sizeof(State->OutputOriginAdjustEnvelope[Scan]));
						PRNGCHK(State->OutputOriginAdjustEnvelope[Scan],
							&(State->OutputOriginAdjustEnvelope[Scan][Index]),
							sizeof(State->OutputOriginAdjustEnvelope[Scan][Index]));
						State->OutputModulationOrigin[Scan][Index]
							= FastFixed2Float(LFOGenUpdateCycle(
								State->OutputOriginAdjustLFO[Scan][Index],
								EnvelopeUpdate(State->OutputOriginAdjustEnvelope[Scan][Index])));
					}
			}
	}


/* dispose of the modulation oscillator state record */
void									DisposeModOscState(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				ERROR((State->StateArray[Scan].OscillatorType != eOscillatorSampled)
					&& (State->StateArray[Scan].OscillatorType != eOscillatorWaveTable),
					PRERR(ForceAbort,"DisposeModOscState:  bad oscillator type"));
				if (State->StateArray[Scan].OscillatorType == eOscillatorSampled)
					{
						DisposeSampleState(State->StateArray[Scan].u.SampleState);
					}
				 else
					{
						DisposeWaveTableState(State->StateArray[Scan].u.WaveTableState);
					}
				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						DisposeEnvelopeStateRecord(State->PhaseGenScalingFactorEnvelope[Scan][Index]);
						DisposeEnvelopeStateRecord(State->PhaseGenOriginAdjustEnvelope[Scan][Index]);
						DisposeLFOGenerator(State->PhaseGenScalingFactorLFO[Scan][Index]);
						DisposeLFOGenerator(State->PhaseGenOriginAdjustLFO[Scan][Index]);
					}
				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						DisposeEnvelopeStateRecord(State->OutputScalingFactorEnvelope[Scan][Index]);
						DisposeEnvelopeStateRecord(State->OutputOriginAdjustEnvelope[Scan][Index]);
						DisposeLFOGenerator(State->OutputScalingFactorLFO[Scan][Index]);
						DisposeLFOGenerator(State->OutputOriginAdjustLFO[Scan][Index]);
					}
				ReleasePtr((char*)State->PhaseGenModulationScaling[Scan]);
				ReleasePtr((char*)State->PhaseGenModulationOrigin[Scan]);
				ReleasePtr((char*)State->OutputModulationScaling[Scan]);
				ReleasePtr((char*)State->OutputModulationOrigin[Scan]);
				ReleasePtr((char*)State->PhaseGenScalingFactorEnvelope[Scan]);
				ReleasePtr((char*)State->PhaseGenScalingFactorLFO[Scan]);
				ReleasePtr((char*)State->PhaseGenOriginAdjustEnvelope[Scan]);
				ReleasePtr((char*)State->PhaseGenOriginAdjustLFO[Scan]);
				ReleasePtr((char*)State->OutputScalingFactorEnvelope[Scan]);
				ReleasePtr((char*)State->OutputScalingFactorLFO[Scan]);
				ReleasePtr((char*)State->OutputOriginAdjustEnvelope[Scan]);
				ReleasePtr((char*)State->OutputOriginAdjustLFO[Scan]);
			}
		ReleasePtr((char*)State->StateArray);
		ReleasePtr((char*)State->PhaseGenModulationScaling);
		ReleasePtr((char*)State->PhaseGenModulationOrigin);
		ReleasePtr((char*)State->PhaseGenScalingFactorEnvelope);
		ReleasePtr((char*)State->PhaseGenScalingFactorLFO);
		ReleasePtr((char*)State->PhaseGenOriginAdjustEnvelope);
		ReleasePtr((char*)State->PhaseGenOriginAdjustLFO);
		ReleasePtr((char*)State->OutputModulationScaling);
		ReleasePtr((char*)State->OutputModulationOrigin);
		ReleasePtr((char*)State->OutputScalingFactorEnvelope);
		ReleasePtr((char*)State->OutputScalingFactorLFO);
		ReleasePtr((char*)State->OutputOriginAdjustEnvelope);
		ReleasePtr((char*)State->OutputOriginAdjustLFO);

		ReleasePtr((char*)State->OldValues);
		ReleasePtr((char*)State->BuildOldValues);

		State->Next = StateFreeList;
		StateFreeList = State;
	}


/* dispose of the modulation oscillator information template */
void									DisposeModOscTemplate(ModOscTemplateRec* Template)
	{
		long								Scan;

		CheckPtrExistence(Template);
		ValidateTemplate(Template);

		for (Scan = 0; Scan < Template->NumberOfOscillators; Scan += 1)
			{
				PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
					sizeof(Template->TemplateArray[Scan]));
				ERROR((Template->TemplateArray[Scan].OscillatorType != eOscillatorSampled)
					&& (Template->TemplateArray[Scan].OscillatorType != eOscillatorWaveTable),
					PRERR(ForceAbort,"DisposeModOscTemplate:  bad oscillator type"));
				if (Template->TemplateArray[Scan].OscillatorType == eOscillatorSampled)
					{
						DisposeSampleTemplate(Template->TemplateArray[Scan].u.SampleTemplate);
					}
				 else
					{
						DisposeWaveTableTemplate(Template->TemplateArray[Scan].u.WaveTableTemplate);
					}
			}
		ReleasePtr((char*)Template->TemplateArray);

		for (Scan = 0; Scan < Template->NumberOfOscillators; Scan += 1)
			{
				PRNGCHK(Template->PhaseGenModulateHow,&(Template->PhaseGenModulateHow[Scan]),
					sizeof(Template->PhaseGenModulateHow[Scan]));
				ReleasePtr((char*)(Template->PhaseGenModulateHow[Scan]));
				PRNGCHK(Template->PhaseGenScalingFactorEnvelope,
					&(Template->PhaseGenScalingFactorEnvelope[Scan]),
					sizeof(Template->PhaseGenScalingFactorEnvelope[Scan]));
				ReleasePtr((char*)(Template->PhaseGenScalingFactorEnvelope[Scan]));
				PRNGCHK(Template->PhaseGenScalingFactorLFOList,
					&(Template->PhaseGenScalingFactorLFOList[Scan]),
					sizeof(Template->PhaseGenScalingFactorLFOList[Scan]));
				ReleasePtr((char*)(Template->PhaseGenScalingFactorLFOList[Scan]));
				PRNGCHK(Template->PhaseGenOriginAdjustEnvelope,
					&(Template->PhaseGenOriginAdjustEnvelope[Scan]),
					sizeof(Template->PhaseGenOriginAdjustEnvelope[Scan]));
				ReleasePtr((char*)(Template->PhaseGenOriginAdjustEnvelope[Scan]));
				PRNGCHK(Template->PhaseGenOriginAdjustLFOList,
					&(Template->PhaseGenOriginAdjustLFOList[Scan]),
					sizeof(Template->PhaseGenOriginAdjustLFOList[Scan]));
				ReleasePtr((char*)(Template->PhaseGenOriginAdjustLFOList[Scan]));
				PRNGCHK(Template->OutputModulateHow,&(Template->OutputModulateHow[Scan]),
					sizeof(Template->OutputModulateHow[Scan]));
				ReleasePtr((char*)(Template->OutputModulateHow[Scan]));
				PRNGCHK(Template->OutputScalingFactorEnvelope,&(Template->OutputScalingFactorEnvelope[
					Scan]),sizeof(Template->OutputScalingFactorEnvelope[Scan]));
				ReleasePtr((char*)(Template->OutputScalingFactorEnvelope[Scan]));
				PRNGCHK(Template->OutputScalingFactorLFOList,&(Template->OutputScalingFactorLFOList[
					Scan]),sizeof(Template->OutputScalingFactorLFOList[Scan]));
				ReleasePtr((char*)(Template->OutputScalingFactorLFOList[Scan]));
				PRNGCHK(Template->OutputOriginAdjustEnvelope,&(Template->OutputOriginAdjustEnvelope[
					Scan]),sizeof(Template->OutputOriginAdjustEnvelope[Scan]));
				ReleasePtr((char*)(Template->OutputOriginAdjustEnvelope[Scan]));
				PRNGCHK(Template->OutputOriginAdjustLFOList,&(Template->OutputOriginAdjustLFOList[
					Scan]),sizeof(Template->OutputOriginAdjustLFOList[Scan]));
				ReleasePtr((char*)(Template->OutputOriginAdjustLFOList[Scan]));
				PRNGCHK(Template->PhaseGenIndirectionTable,&(Template->PhaseGenIndirectionTable[
					Scan]),sizeof(Template->PhaseGenIndirectionTable[Scan]));
				ReleasePtr((char*)(Template->PhaseGenIndirectionTable[Scan]));
				PRNGCHK(Template->OutputIndirectionTable,&(Template->OutputIndirectionTable[
					Scan]),sizeof(Template->OutputIndirectionTable[Scan]));
				ReleasePtr((char*)(Template->OutputIndirectionTable[Scan]));
			}
		ReleasePtr((char*)(Template->PhaseGenModulateHow));
		ReleasePtr((char*)(Template->PhaseGenScalingFactorEnvelope));
		ReleasePtr((char*)(Template->PhaseGenScalingFactorLFOList));
		ReleasePtr((char*)(Template->PhaseGenOriginAdjustEnvelope));
		ReleasePtr((char*)(Template->PhaseGenOriginAdjustLFOList));
		ReleasePtr((char*)(Template->OutputModulateHow));
		ReleasePtr((char*)(Template->OutputScalingFactorEnvelope));
		ReleasePtr((char*)(Template->OutputScalingFactorLFOList));
		ReleasePtr((char*)(Template->OutputOriginAdjustEnvelope));
		ReleasePtr((char*)(Template->OutputOriginAdjustLFOList));
		ReleasePtr((char*)Template->PhaseGenNumElements);
		ReleasePtr((char*)Template->OutputNumElements);
		ReleasePtr((char*)Template->PhaseGenIndirectionTable);
		ReleasePtr((char*)Template->OutputIndirectionTable);

		Template->Next = TemplateFreeList;
		TemplateFreeList = Template;
	}


/* create a new modulation oscillator template */
ModOscTemplateRec*		NewModOscTemplate(struct ArrayRec* OscillatorList,
												float EnvelopeTicksPerSecond, long SamplingRate,
												MyBoolean Stereo, MyBoolean TimeInterp, MyBoolean WaveInterp,
												ErrorDaemonRec* ErrorDaemon)
	{
		ModOscTemplateRec*	Template;
		long								Scan;
		long								Limit;

		CheckPtrExistence(ErrorDaemon);
		CheckPtrExistence(OscillatorList);
		ERROR(ArrayGetLength(OscillatorList) == 0,PRERR(ForceAbort,
			"NewModOscTemplate:  oscillator list is empty"));

		if (TemplateFreeList != NIL)
			{
				Template = TemplateFreeList;
				TemplateFreeList = TemplateFreeList->Next;
			}
		 else
			{
				Template = (ModOscTemplateRec*)AllocPtrCanFail(sizeof(ModOscTemplateRec),
					"ModOscTemplateRec");
				if (Template == NIL)
					{
						return NIL;
					}
			}
		EXECUTE(Template->Next = (ModOscTemplateRec*)0x81818181;)

		Template->NumberOfOscillators = ArrayGetLength(OscillatorList);

		Template->StereoPlayback = Stereo;
		Template->TicksPerSecond = EnvelopeTicksPerSecond;

		Template->TemplateArray = (TemplateElemRec*)AllocPtrCanFail(sizeof(TemplateElemRec)
			* Template->NumberOfOscillators,"TemplateElemRec[]");
		if (Template->TemplateArray == NIL)
			{
			 FailurePoint1:
				Template->Next = TemplateFreeList;
				TemplateFreeList = Template;
				return NIL;
			}
		Template->PhaseGenNumElements = (long*)AllocPtrCanFail(sizeof(long)
			* Template->NumberOfOscillators,"PhaseGenNumElements[]");
		if (Template->PhaseGenNumElements == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Template->TemplateArray);
				goto FailurePoint1;
			}
		Template->PhaseGenModulateHow = (ModulationTypes**)AllocPtrCanFail(
			sizeof(ModulationTypes*) * Template->NumberOfOscillators,
			"PhaseGenModulateHow[][]");
		if (Template->PhaseGenModulateHow == NIL)
			{
			 FailurePoint3:
				ReleasePtr((char*)Template->PhaseGenNumElements);
				goto FailurePoint2;
			}
		Template->PhaseGenScalingFactorEnvelope = (EnvelopeRec***)AllocPtrCanFail(
			sizeof(EnvelopeRec**) * Template->NumberOfOscillators,
			"PhaseGenScalingFactorEnvelope[][]");
		if (Template->PhaseGenScalingFactorEnvelope == NIL)
			{
			 FailurePoint4:
				ReleasePtr((char*)Template->PhaseGenModulateHow);
				goto FailurePoint3;
			}
		Template->PhaseGenScalingFactorLFOList = (LFOListSpecRec***)AllocPtrCanFail(
			sizeof(LFOListSpecRec**) * Template->NumberOfOscillators,
			"PhaseGenScalingFactorLFOList[][]");
		if (Template->PhaseGenScalingFactorLFOList == NIL)
			{
			 FailurePoint5:
				ReleasePtr((char*)Template->PhaseGenScalingFactorEnvelope);
				goto FailurePoint4;
			}
		Template->PhaseGenOriginAdjustEnvelope = (EnvelopeRec***)AllocPtrCanFail(
			sizeof(EnvelopeRec**) * Template->NumberOfOscillators,
			"PhaseGenOriginAdjustEnvelope[][]");
		if (Template->PhaseGenOriginAdjustEnvelope == NIL)
			{
			 FailurePoint6:
				ReleasePtr((char*)Template->PhaseGenScalingFactorLFOList);
				goto FailurePoint5;
			}
		Template->PhaseGenOriginAdjustLFOList = (LFOListSpecRec***)AllocPtrCanFail(
			sizeof(LFOListSpecRec**) * Template->NumberOfOscillators,
			"PhaseGenOriginAdjustLFOList[][]");
		if (Template->PhaseGenOriginAdjustLFOList == NIL)
			{
			 FailurePoint7:
				ReleasePtr((char*)Template->PhaseGenOriginAdjustEnvelope);
				goto FailurePoint6;
			}
		Template->OutputNumElements = (long*)AllocPtrCanFail(sizeof(long)
			* Template->NumberOfOscillators,"OutputNumElements[]");
		if (Template->OutputNumElements == NIL)
			{
			 FailurePoint8:
				ReleasePtr((char*)Template->PhaseGenOriginAdjustLFOList);
				goto FailurePoint7;
			}
		Template->OutputModulateHow = (ModulationTypes**)AllocPtrCanFail(
			sizeof(ModulationTypes*) * Template->NumberOfOscillators,"OutputModulateHow[][]");
		if (Template->OutputModulateHow == NIL)
			{
			 FailurePoint9:
				ReleasePtr((char*)Template->OutputNumElements);
				goto FailurePoint8;
			}
		Template->OutputScalingFactorEnvelope = (EnvelopeRec***)AllocPtrCanFail(
			sizeof(EnvelopeRec**) * Template->NumberOfOscillators,
			"OutputScalingFactorEnvelope[][]");
		if (Template->OutputScalingFactorEnvelope == NIL)
			{
			 FailurePoint10:
				ReleasePtr((char*)Template->OutputModulateHow);
				goto FailurePoint9;
			}
		Template->OutputScalingFactorLFOList = (LFOListSpecRec***)AllocPtrCanFail(
			sizeof(LFOListSpecRec**) * Template->NumberOfOscillators,
			"OutputScalingFactorLFOList[][]");
		if (Template->OutputScalingFactorLFOList == NIL)
			{
			 FailurePoint11:
				ReleasePtr((char*)Template->OutputScalingFactorEnvelope);
				goto FailurePoint10;
			}
		Template->OutputOriginAdjustEnvelope = (EnvelopeRec***)AllocPtrCanFail(
			sizeof(EnvelopeRec**) * Template->NumberOfOscillators,
			"OutputOriginAdjustEnvelope[][]");
		if (Template->OutputOriginAdjustEnvelope == NIL)
			{
			 FailurePoint12:
				ReleasePtr((char*)Template->OutputScalingFactorLFOList);
				goto FailurePoint11;
			}
		Template->OutputOriginAdjustLFOList = (LFOListSpecRec***)AllocPtrCanFail(
			sizeof(LFOListSpecRec**) * Template->NumberOfOscillators,
			"OutputOriginAdjustLFOList[][]");
		if (Template->OutputOriginAdjustLFOList == NIL)
			{
			 FailurePoint13:
				ReleasePtr((char*)Template->OutputOriginAdjustEnvelope);
				goto FailurePoint12;
			}
		Template->PhaseGenIndirectionTable = (long**)AllocPtrCanFail(sizeof(long*)
			* Template->NumberOfOscillators,"PhaseGenIndirectionTable[][]");
		if (Template->PhaseGenIndirectionTable == NIL)
			{
			 FailurePoint14:
				ReleasePtr((char*)Template->OutputOriginAdjustLFOList);
				goto FailurePoint13;
			}
		Template->OutputIndirectionTable = (long**)AllocPtrCanFail(sizeof(long*)
			* Template->NumberOfOscillators,"OutputIndirectionTable[][]");
		if (Template->OutputIndirectionTable == NIL)
			{
			 FailurePoint15:
				ReleasePtr((char*)Template->PhaseGenIndirectionTable);
				goto FailurePoint14;
			}

		Limit = ArrayGetLength(OscillatorList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				OscillatorRec*			Oscillator;
				ModulationSpecRec*	ModulationSourceList;
				long								Count;
				long								Index;
				long								TotalNumModulators;

				Oscillator = (OscillatorRec*)ArrayGetElement(OscillatorList,Scan);
				CheckPtrExistence(Oscillator);

				switch (OscillatorGetWhatKindItIs(Oscillator))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NewModOscTemplate:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
								sizeof(Template->TemplateArray[Scan]));
							Template->TemplateArray[Scan].OscillatorType = eOscillatorSampled;
							Template->TemplateArray[Scan].u.SampleTemplate = NewSampleTemplate(
								Oscillator,EnvelopeTicksPerSecond,SamplingRate,Stereo,TimeInterp,
								WaveInterp,ErrorDaemon);
							if (Template->TemplateArray[Scan].u.SampleTemplate == NIL)
								{
								 FailurePoint16a:
									for (Index = 0; Index < Scan; Index += 1)
										{
											PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Index]),
												sizeof(Template->TemplateArray[Index]));
											switch (Template->TemplateArray[Index].OscillatorType)
												{
													default:
														EXECUTE(PRERR(ForceAbort,"NewModOscTemplate:  bad oscillator type"));
														break;
													case eOscillatorSampled:
														DisposeSampleTemplate(Template->TemplateArray[Index]
															.u.SampleTemplate);
														break;
													case eOscillatorWaveTable:
														DisposeWaveTableTemplate(Template->TemplateArray[Index]
															.u.WaveTableTemplate);
														break;
												}
											PRNGCHK(Template->PhaseGenModulateHow,
												&(Template->PhaseGenModulateHow[Index]),
												sizeof(Template->PhaseGenModulateHow[Index]));
											ReleasePtr((char*)(Template->PhaseGenModulateHow[Index]));
											PRNGCHK(Template->PhaseGenScalingFactorEnvelope,
												&(Template->PhaseGenScalingFactorEnvelope[Index]),
												sizeof(Template->PhaseGenScalingFactorEnvelope[Index]));
											ReleasePtr((char*)(Template->PhaseGenScalingFactorEnvelope[Index]));
											PRNGCHK(Template->PhaseGenScalingFactorLFOList,
												&(Template->PhaseGenScalingFactorLFOList[Index]),
												sizeof(Template->PhaseGenScalingFactorLFOList[Index]));
											ReleasePtr((char*)(Template->PhaseGenScalingFactorLFOList[Index]));
											PRNGCHK(Template->PhaseGenOriginAdjustEnvelope,
												&(Template->PhaseGenOriginAdjustEnvelope[Index]),
												sizeof(Template->PhaseGenOriginAdjustEnvelope[Index]));
											ReleasePtr((char*)(Template->PhaseGenOriginAdjustEnvelope[Index]));
											PRNGCHK(Template->PhaseGenOriginAdjustLFOList,
												&(Template->PhaseGenOriginAdjustLFOList[Index]),
												sizeof(Template->PhaseGenOriginAdjustLFOList[Index]));
											ReleasePtr((char*)(Template->PhaseGenOriginAdjustLFOList[Index]));
											PRNGCHK(Template->OutputModulateHow,
												&(Template->OutputModulateHow[Index]),
												sizeof(Template->OutputModulateHow[Index]));
											ReleasePtr((char*)(Template->OutputModulateHow[Index]));
											PRNGCHK(Template->OutputScalingFactorEnvelope,
												&(Template->OutputScalingFactorEnvelope[Index]),
												sizeof(Template->OutputScalingFactorEnvelope[Index]));
											ReleasePtr((char*)(Template->OutputScalingFactorEnvelope[Index]));
											PRNGCHK(Template->OutputScalingFactorLFOList,
												&(Template->OutputScalingFactorLFOList[Index]),
												sizeof(Template->OutputScalingFactorLFOList[Index]));
											ReleasePtr((char*)(Template->OutputScalingFactorLFOList[Index]));
											PRNGCHK(Template->OutputOriginAdjustEnvelope,
												&(Template->OutputOriginAdjustEnvelope[Index]),
												sizeof(Template->OutputOriginAdjustEnvelope[Index]));
											ReleasePtr((char*)(Template->OutputOriginAdjustEnvelope[Index]));
											PRNGCHK(Template->OutputOriginAdjustLFOList,
												&(Template->OutputOriginAdjustLFOList[Index]),
												sizeof(Template->OutputOriginAdjustLFOList[Index]));
											ReleasePtr((char*)(Template->OutputOriginAdjustLFOList[Index]));
										}
									goto FailurePoint15;
								}
							break;
						case eOscillatorWaveTable:
							PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
								sizeof(Template->TemplateArray[Scan]));
							Template->TemplateArray[Scan].OscillatorType = eOscillatorWaveTable;
							Template->TemplateArray[Scan].u.WaveTableTemplate = NewWaveTableTemplate(
								Oscillator,EnvelopeTicksPerSecond,SamplingRate,Stereo,TimeInterp,
								WaveInterp,ErrorDaemon);
							if (Template->TemplateArray[Scan].u.WaveTableTemplate == NIL)
								{
									goto FailurePoint16a;
								}
							break;
					}

				ModulationSourceList = OscillatorGetModulatorInputList(Oscillator);
				TotalNumModulators = GetModulationSpecNumEntries(ModulationSourceList);

				/* count the number of phase generator modulators */
				Count = 0;
				for (Index = 0; Index < TotalNumModulators; Index += 1)
					{
						switch (GetModulationDestType(ModulationSourceList,Index))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"NewModOscTemplate:  bad modulation target type"));
									break;
								case eModulatePhaseGen:
									Count += 1;
									break;
								case eModulateOutput:
									break;
							}
					}
				PRNGCHK(Template->PhaseGenNumElements,&(Template->PhaseGenNumElements[Scan]),
					sizeof(Template->PhaseGenNumElements[Scan]));
				Template->PhaseGenNumElements[Scan] = Count;

				PRNGCHK(Template->PhaseGenModulateHow,&(Template->PhaseGenModulateHow[Scan]),
					sizeof(Template->PhaseGenModulateHow[Scan]));
				Template->PhaseGenModulateHow[Scan] = (ModulationTypes*)AllocPtrCanFail(
					sizeof(ModulationTypes) * Count,"PhaseGenModulateHow[]");
				if (Template->PhaseGenModulateHow[Scan] == NIL)
					{
					 FailurePoint16b:
						PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
							sizeof(Template->TemplateArray[Scan]));
						switch (Template->TemplateArray[Scan].OscillatorType)
							{
								default:
									EXECUTE(PRERR(ForceAbort,"NewModOscTemplate:  bad oscillator type"));
									break;
								case eOscillatorSampled:
									DisposeSampleTemplate(Template->TemplateArray[Scan].u.SampleTemplate);
									break;
								case eOscillatorWaveTable:
									DisposeWaveTableTemplate(Template->TemplateArray[Scan].u.WaveTableTemplate);
									break;
							}
						goto FailurePoint16a;
					}
				PRNGCHK(Template->PhaseGenScalingFactorEnvelope,
					&(Template->PhaseGenScalingFactorEnvelope[Scan]),
					sizeof(Template->PhaseGenScalingFactorEnvelope[Scan]));
				Template->PhaseGenScalingFactorEnvelope[Scan] = (EnvelopeRec**)AllocPtrCanFail(
					sizeof(EnvelopeRec*) * Count,"PhaseGenScalingFactorEnvelope[]");
				if (Template->PhaseGenScalingFactorEnvelope[Scan] == NIL)
					{
					 FailurePoint16c:
						ReleasePtr((char*)(Template->PhaseGenModulateHow[Scan]));
						goto FailurePoint16b;
					}
				PRNGCHK(Template->PhaseGenScalingFactorLFOList,
					&(Template->PhaseGenScalingFactorLFOList[Scan]),
					sizeof(Template->PhaseGenScalingFactorLFOList[Scan]));
				Template->PhaseGenScalingFactorLFOList[Scan] = (LFOListSpecRec**)AllocPtrCanFail(
					sizeof(LFOListSpecRec*) * Count,"PhaseGenScalingFactorLFOList[]");
				if (Template->PhaseGenScalingFactorLFOList[Scan] == NIL)
					{
					 FailurePoint16d:
						ReleasePtr((char*)(Template->PhaseGenScalingFactorEnvelope[Scan]));
						goto FailurePoint16c;
					}
				PRNGCHK(Template->PhaseGenOriginAdjustEnvelope,
					&(Template->PhaseGenOriginAdjustEnvelope[Scan]),
						sizeof(Template->PhaseGenOriginAdjustEnvelope[Scan]));
				Template->PhaseGenOriginAdjustEnvelope[Scan] = (EnvelopeRec**)AllocPtrCanFail(
					sizeof(EnvelopeRec*) * Count,"PhaseGenOriginAdjustEnvelope[]");
				if (Template->PhaseGenOriginAdjustEnvelope[Scan] == NIL)
					{
					 FailurePoint16e:
						ReleasePtr((char*)(Template->PhaseGenScalingFactorLFOList[Scan]));
						goto FailurePoint16d;
					}
				PRNGCHK(Template->PhaseGenOriginAdjustLFOList,
					&(Template->PhaseGenOriginAdjustLFOList[Scan]),
						sizeof(Template->PhaseGenOriginAdjustLFOList[Scan]));
				Template->PhaseGenOriginAdjustLFOList[Scan] = (LFOListSpecRec**)AllocPtrCanFail(
					sizeof(LFOListSpecRec*) * Count,"PhaseGenOriginAdjustLFOList[]");
				if (Template->PhaseGenOriginAdjustLFOList[Scan] == NIL)
					{
					 FailurePoint16f:
						ReleasePtr((char*)(Template->PhaseGenOriginAdjustEnvelope[Scan]));
						goto FailurePoint16e;
					}
				PRNGCHK(Template->PhaseGenIndirectionTable,
					&(Template->PhaseGenIndirectionTable[Scan]),
					sizeof(Template->PhaseGenIndirectionTable[Scan]));
				Template->PhaseGenIndirectionTable[Scan] = (long*)AllocPtrCanFail(
					sizeof(long) * Count,"PhaseGenIndirectionTable[]");
				if (Template->PhaseGenIndirectionTable[Scan] == NIL)
					{
					 FailurePoint16g:
						ReleasePtr((char*)Template->PhaseGenOriginAdjustLFOList[Scan]);
						goto FailurePoint16f;
					}

				/* build phase generator modulation structures */
				Count = 0;
				for (Index = 0; Index < TotalNumModulators; Index += 1)
					{
						long								Position;

						switch (GetModulationDestType(ModulationSourceList,Index))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"NewModOscTemplate:  bad modulation target type"));
									break;
								case eModulatePhaseGen:
									PRNGCHK(Template->PhaseGenModulateHow,
										&(Template->PhaseGenModulateHow[Scan]),
										sizeof(Template->PhaseGenModulateHow[Scan]));
									PRNGCHK(Template->PhaseGenModulateHow[Scan],
										&(Template->PhaseGenModulateHow[Scan][Count]),
										sizeof(Template->PhaseGenModulateHow[Scan][Count]));
									Template->PhaseGenModulateHow[Scan][Count] = GetModulationOpType(
										ModulationSourceList,Index);
									PRNGCHK(Template->PhaseGenScalingFactorEnvelope,
										&(Template->PhaseGenScalingFactorEnvelope[Scan]),
										sizeof(Template->PhaseGenScalingFactorEnvelope[Scan]));
									PRNGCHK(Template->PhaseGenScalingFactorEnvelope[Scan],
										&(Template->PhaseGenScalingFactorEnvelope[Scan][Count]),
										sizeof(Template->PhaseGenScalingFactorEnvelope[Scan][Count]));
									Template->PhaseGenScalingFactorEnvelope[Scan][Count]
										= GetModulationScalingFactorEnvelope(ModulationSourceList,Index);
									PRNGCHK(Template->PhaseGenScalingFactorLFOList,
										&(Template->PhaseGenScalingFactorLFOList[Scan]),
										sizeof(Template->PhaseGenScalingFactorLFOList[Scan]));
									PRNGCHK(Template->PhaseGenScalingFactorLFOList[Scan],
										&(Template->PhaseGenScalingFactorLFOList[Scan][Count]),
										sizeof(Template->PhaseGenScalingFactorLFOList[Scan][Count]));
									Template->PhaseGenScalingFactorLFOList[Scan][Count]
										= GetModulationScalingFactorLFOList(ModulationSourceList,Index);
									PRNGCHK(Template->PhaseGenOriginAdjustEnvelope,
										&(Template->PhaseGenOriginAdjustEnvelope[Scan]),
										sizeof(Template->PhaseGenOriginAdjustEnvelope[Scan]));
									PRNGCHK(Template->PhaseGenOriginAdjustEnvelope[Scan],
										&(Template->PhaseGenOriginAdjustEnvelope[Scan][Count]),
										sizeof(Template->PhaseGenOriginAdjustEnvelope[Scan][Count]));
									Template->PhaseGenOriginAdjustEnvelope[Scan][Count]
										= GetModulationOriginAdjustEnvelope(ModulationSourceList,Index);
									PRNGCHK(Template->PhaseGenOriginAdjustLFOList,
										&(Template->PhaseGenOriginAdjustLFOList[Scan]),
										sizeof(Template->PhaseGenOriginAdjustLFOList[Scan]));
									PRNGCHK(Template->PhaseGenOriginAdjustLFOList[Scan],
										&(Template->PhaseGenOriginAdjustLFOList[Scan][Count]),
										sizeof(Template->PhaseGenOriginAdjustLFOList[Scan][Count]));
									Template->PhaseGenOriginAdjustLFOList[Scan][Count]
										= GetModulationOriginAdjustLFOList(ModulationSourceList,Index);
									Position = ArrayFindElement(OscillatorList,
										GetModulationOscillatorRef(ModulationSourceList,Index));
									ERROR(Position == -1,PRERR(ForceAbort,
										"NewModOscTemplate:  reference to oscillator not in list"));
									PRNGCHK(Template->PhaseGenIndirectionTable,
										&(Template->PhaseGenIndirectionTable[Scan]),
										sizeof(Template->PhaseGenIndirectionTable[Scan]));
									PRNGCHK(Template->PhaseGenIndirectionTable[Scan],
										&(Template->PhaseGenIndirectionTable[Scan][Count]),
										sizeof(Template->PhaseGenIndirectionTable[Scan][Count]));
									Template->PhaseGenIndirectionTable[Scan][Count] = Position;
									Count += 1;
									break;
								case eModulateOutput:
									break;
							}
					}

				/* count the number of output modulators */
				Count = 0;
				for (Index = 0; Index < TotalNumModulators; Index += 1)
					{
						switch (GetModulationDestType(ModulationSourceList,Index))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"NewModOscTemplate:  bad modulation target type"));
									break;
								case eModulatePhaseGen:
									break;
								case eModulateOutput:
									Count += 1;
									break;
							}
					}
				PRNGCHK(Template->OutputNumElements,&(Template->OutputNumElements[Scan]),
					sizeof(Template->OutputNumElements[Scan]));
				Template->OutputNumElements[Scan] = Count;

				PRNGCHK(Template->OutputModulateHow,&(Template->OutputModulateHow[Scan]),
					sizeof(Template->OutputModulateHow[Scan]));
				Template->OutputModulateHow[Scan] = (ModulationTypes*)AllocPtrCanFail(
					sizeof(ModulationTypes) * Count,"OutputModulateHow[]");
				if (Template->OutputModulateHow[Scan] == NIL)
					{
					 FailurePoint16h:
						ReleasePtr((char*)(Template->PhaseGenIndirectionTable[Scan]));
						goto FailurePoint16g;
					}
				PRNGCHK(Template->OutputScalingFactorEnvelope,
					&(Template->OutputScalingFactorEnvelope[Scan]),
					sizeof(Template->OutputScalingFactorEnvelope[Scan]));
				Template->OutputScalingFactorEnvelope[Scan] = (EnvelopeRec**)AllocPtrCanFail(
					sizeof(EnvelopeRec*) * Count,"OutputScalingFactorEnvelope[]");
				if (Template->OutputScalingFactorEnvelope[Scan] == NIL)
					{
					 FailurePoint16i:
						ReleasePtr((char*)(Template->OutputModulateHow[Scan]));
						goto FailurePoint16h;
					}
				PRNGCHK(Template->OutputScalingFactorLFOList,
					&(Template->OutputScalingFactorLFOList[Scan]),
					sizeof(Template->OutputScalingFactorLFOList[Scan]));
				Template->OutputScalingFactorLFOList[Scan] = (LFOListSpecRec**)AllocPtrCanFail(
					sizeof(LFOListSpecRec*) * Count,"OutputScalingFactorLFOList[]");
				if (Template->OutputScalingFactorLFOList[Scan] == NIL)
					{
					 FailurePoint16j:
						ReleasePtr((char*)(Template->OutputScalingFactorEnvelope[Scan]));
						goto FailurePoint16i;
					}
				PRNGCHK(Template->OutputOriginAdjustEnvelope,
					&(Template->OutputOriginAdjustEnvelope[Scan]),
					sizeof(Template->OutputOriginAdjustEnvelope[Scan]));
				Template->OutputOriginAdjustEnvelope[Scan] = (EnvelopeRec**)AllocPtrCanFail(
					sizeof(EnvelopeRec*) * Count,"OutputOriginAdjustEnvelope[]");
				if (Template->OutputOriginAdjustEnvelope[Scan] == NIL)
					{
					 FailurePoint16k:
						ReleasePtr((char*)(Template->OutputScalingFactorLFOList[Scan]));
						goto FailurePoint16j;
					}
				PRNGCHK(Template->OutputOriginAdjustLFOList,
					&(Template->OutputOriginAdjustLFOList[Scan]),
					sizeof(Template->OutputOriginAdjustLFOList[Scan]));
				Template->OutputOriginAdjustLFOList[Scan] = (LFOListSpecRec**)AllocPtrCanFail(
					sizeof(LFOListSpecRec*) * Count,"OutputOriginAdjustLFOList[]");
				if (Template->OutputOriginAdjustLFOList[Scan] == NIL)
					{
					 FailurePoint16l:
						ReleasePtr((char*)(Template->OutputOriginAdjustEnvelope[Scan]));
						goto FailurePoint16k;
					}

				PRNGCHK(Template->OutputIndirectionTable,
					&(Template->OutputIndirectionTable[Scan]),
					sizeof(Template->OutputIndirectionTable[Scan]));
				Template->OutputIndirectionTable[Scan] = (long*)AllocPtrCanFail(
					sizeof(long) * Count,"OutputIndirectionTable[]");
				if (Template->OutputIndirectionTable[Scan] == NIL)
					{
					 FailurePoint16m:
						ReleasePtr((char*)(Template->OutputOriginAdjustLFOList[Scan]));
						goto FailurePoint16l;
					}

				/* build output modulation structures */
				Count = 0;
				for (Index = 0; Index < TotalNumModulators; Index += 1)
					{
						long								Position;

						switch (GetModulationDestType(ModulationSourceList,Index))
							{
								default:
									EXECUTE(PRERR(ForceAbort,
										"NewModOscTemplate:  bad modulation target type"));
									break;
								case eModulatePhaseGen:
									break;
								case eModulateOutput:
									PRNGCHK(Template->OutputModulateHow,
										&(Template->OutputModulateHow[Scan]),
										sizeof(Template->OutputModulateHow[Scan]));
									PRNGCHK(Template->OutputModulateHow[Scan],
										&(Template->OutputModulateHow[Scan][Count]),
										sizeof(Template->OutputModulateHow[Scan][Count]));
									Template->OutputModulateHow[Scan][Count] = GetModulationOpType(
										ModulationSourceList,Index);
									PRNGCHK(Template->OutputScalingFactorEnvelope,
										&(Template->OutputScalingFactorEnvelope[Scan]),
										sizeof(Template->OutputScalingFactorEnvelope[Scan]));
									PRNGCHK(Template->OutputScalingFactorEnvelope[Scan],
										&(Template->OutputScalingFactorEnvelope[Scan][Count]),
										sizeof(Template->OutputScalingFactorEnvelope[Scan][Count]));
									Template->OutputScalingFactorEnvelope[Scan][Count]
										= GetModulationScalingFactorEnvelope(ModulationSourceList,Index);
									PRNGCHK(Template->OutputScalingFactorLFOList,
										&(Template->OutputScalingFactorLFOList[Scan]),
										sizeof(Template->OutputScalingFactorLFOList[Scan]));
									PRNGCHK(Template->OutputScalingFactorLFOList[Scan],
										&(Template->OutputScalingFactorLFOList[Scan][Count]),
										sizeof(Template->OutputScalingFactorLFOList[Scan][Count]));
									Template->OutputScalingFactorLFOList[Scan][Count]
										= GetModulationScalingFactorLFOList(ModulationSourceList,Index);
									PRNGCHK(Template->OutputOriginAdjustEnvelope,
										&(Template->OutputOriginAdjustEnvelope[Scan]),
										sizeof(Template->OutputOriginAdjustEnvelope[Scan]));
									PRNGCHK(Template->OutputOriginAdjustEnvelope[Scan],
										&(Template->OutputOriginAdjustEnvelope[Scan][Count]),
										sizeof(Template->OutputOriginAdjustEnvelope[Scan][Count]));
									Template->OutputOriginAdjustEnvelope[Scan][Count]
										= GetModulationOriginAdjustEnvelope(ModulationSourceList,Index);
									PRNGCHK(Template->OutputOriginAdjustLFOList,
										&(Template->OutputOriginAdjustLFOList[Scan]),
										sizeof(Template->OutputOriginAdjustLFOList[Scan]));
									PRNGCHK(Template->OutputOriginAdjustLFOList[Scan],
										&(Template->OutputOriginAdjustLFOList[Scan][Count]),
										sizeof(Template->OutputOriginAdjustLFOList[Scan][Count]));
									Template->OutputOriginAdjustLFOList[Scan][Count]
										= GetModulationOriginAdjustLFOList(ModulationSourceList,Index);
									Position = ArrayFindElement(OscillatorList,
										GetModulationOscillatorRef(ModulationSourceList,Index));
									ERROR(Position == -1,PRERR(ForceAbort,
										"NewModOscTemplate:  reference to oscillator not in list"));
									PRNGCHK(Template->OutputIndirectionTable,
										&(Template->OutputIndirectionTable[Scan]),
										sizeof(Template->OutputIndirectionTable[Scan]));
									PRNGCHK(Template->OutputIndirectionTable[Scan],
										&(Template->OutputIndirectionTable[Scan][Count]),
										sizeof(Template->OutputIndirectionTable[Scan][Count]));
									Template->OutputIndirectionTable[Scan][Count] = Position;
									Count += 1;
									break;
							}
					}

				PRNGCHK(Template->OutputNumElements,&(Template->OutputNumElements[Scan]),
					sizeof(Template->OutputNumElements[Scan]));
				PRNGCHK(Template->PhaseGenNumElements,&(Template->PhaseGenNumElements[Scan]),
					sizeof(Template->PhaseGenNumElements[Scan]));
				ERROR(Template->OutputNumElements[Scan] + Template->PhaseGenNumElements[Scan]
					!= TotalNumModulators,PRERR(ForceAbort,
					"NewModOscTemplate:  modulator count inconsistency"));
			}

		return Template;
	}


/* create a new modulation oscillator state object. */
ModOscStateRec*				NewModOscState(ModOscTemplateRec* Template,
												float FreqForMultisampling, float Accent1, float Accent2,
												float Accent3, float Accent4, float Loudness, float HurryUp,
												long* PreOriginTimeOut, float StereoPosition,
												float InitialFrequency)
	{
		long								Scan;
		ModOscStateRec*			State;
		long								MaxOrigin;

		CheckPtrExistence(Template);
		ValidateTemplate(Template);

		if (StateFreeList != NIL)
			{
				State = StateFreeList;
				StateFreeList = StateFreeList->Next;
			}
		 else
			{
				State = (ModOscStateRec*)AllocPtrCanFail(sizeof(ModOscStateRec),"ModOscStateRec");
				if (State == NIL)
					{
						return NIL;
					}
			}
		EXECUTE(State->Next = (ModOscStateRec*)0x81818181;)

		State->StateArray = (StateElemRec*)AllocPtrCanFail(sizeof(StateElemRec)
			* Template->NumberOfOscillators,"StateArray");
		if (State->StateArray == NIL)
			{
			 FailurePoint1:
				State->Next = StateFreeList;
				StateFreeList = State;
				return NIL;
			}
		State->OldValues = (float*)AllocPtrCanFail(sizeof(float)
			* Template->NumberOfOscillators,"OldValues");
		if (State->OldValues == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)State->StateArray);
				goto FailurePoint1;
			}
		State->BuildOldValues = (float*)AllocPtrCanFail(sizeof(float)
			* Template->NumberOfOscillators,"BuildOldValues");
		if (State->BuildOldValues == NIL)
			{
			 FailurePoint3:
				ReleasePtr((char*)State->OldValues);
				goto FailurePoint2;
			}
		State->PhaseGenModulationScaling = (float**)AllocPtrCanFail(sizeof(float*)
			* Template->NumberOfOscillators,"PhaseGenModulationScaling[]");
		if (State->PhaseGenModulationScaling == NIL)
			{
			 FailurePoint4:
				ReleasePtr((char*)State->BuildOldValues);
				goto FailurePoint3;
			}
		State->PhaseGenModulationOrigin = (float**)AllocPtrCanFail(sizeof(float*)
			* Template->NumberOfOscillators,"PhaseGenModulationOrigin[]");
		if (State->PhaseGenModulationOrigin == NIL)
			{
			 FailurePoint5:
				ReleasePtr((char*)State->PhaseGenModulationScaling);
				goto FailurePoint4;
			}
		State->PhaseGenScalingFactorEnvelope = (EvalEnvelopeRec***)AllocPtrCanFail(
			sizeof(EvalEnvelopeRec**) * Template->NumberOfOscillators,
			"PhaseGenScalingFactorEnvelope[]");
		if (State->PhaseGenScalingFactorEnvelope == NIL)
			{
			 FailurePoint6:
				ReleasePtr((char*)State->PhaseGenModulationOrigin);
				goto FailurePoint5;
			}
		State->PhaseGenScalingFactorLFO = (LFOGenRec***)AllocPtrCanFail(
			sizeof(LFOGenRec**) * Template->NumberOfOscillators,"PhaseGenScalingFactorLFO[]");
		if (State->PhaseGenScalingFactorLFO == NIL)
			{
			 FailurePoint7:
				ReleasePtr((char*)State->PhaseGenScalingFactorEnvelope);
				goto FailurePoint6;
			}
		State->PhaseGenOriginAdjustEnvelope = (EvalEnvelopeRec***)AllocPtrCanFail(
			sizeof(EvalEnvelopeRec**) * Template->NumberOfOscillators,
			"PhaseGenOriginAdjustEnvelope[]");
		if (State->PhaseGenOriginAdjustEnvelope == NIL)
			{
			 FailurePoint8:
				ReleasePtr((char*)State->PhaseGenScalingFactorLFO);
				goto FailurePoint7;
			}
		State->PhaseGenOriginAdjustLFO = (LFOGenRec***)AllocPtrCanFail(
			sizeof(LFOGenRec**) * Template->NumberOfOscillators,"PhaseGenOriginAdjustLFO[]");
		if (State->PhaseGenOriginAdjustLFO == NIL)
			{
			 FailurePoint9:
				ReleasePtr((char*)State->PhaseGenOriginAdjustEnvelope);
				goto FailurePoint8;
			}
		State->OutputModulationScaling = (float**)AllocPtrCanFail(sizeof(float*)
			* Template->NumberOfOscillators,"OutputModulationScaling[]");
		if (State->OutputModulationScaling == NIL)
			{
			 FailurePoint10:
				ReleasePtr((char*)State->PhaseGenOriginAdjustLFO);
				goto FailurePoint9;
			}
		State->OutputModulationOrigin = (float**)AllocPtrCanFail(sizeof(float*)
			* Template->NumberOfOscillators,"OutputModulationOrigin[]");
		if (State->OutputModulationOrigin == NIL)
			{
			 FailurePoint11:
				ReleasePtr((char*)State->OutputModulationScaling);
				goto FailurePoint10;
			}
		State->OutputScalingFactorEnvelope = (EvalEnvelopeRec***)AllocPtrCanFail(
			sizeof(EvalEnvelopeRec**) * Template->NumberOfOscillators,
			"OutputScalingFactorEnvelope[]");
		if (State->OutputScalingFactorEnvelope == NIL)
			{
			 FailurePoint12:
				ReleasePtr((char*)State->OutputModulationOrigin);
				goto FailurePoint11;
			}
		State->OutputScalingFactorLFO = (LFOGenRec***)AllocPtrCanFail(sizeof(LFOGenRec**)
			* Template->NumberOfOscillators,"OutputScalingFactorLFO[]");
		if (State->OutputScalingFactorLFO == NIL)
			{
			 FailurePoint13:
				ReleasePtr((char*)State->OutputScalingFactorEnvelope);
				goto FailurePoint12;
			}
		State->OutputOriginAdjustEnvelope = (EvalEnvelopeRec***)AllocPtrCanFail(
			sizeof(EvalEnvelopeRec**) * Template->NumberOfOscillators,
			"OutputOriginAdjustEnvelope[]");
		if (State->OutputOriginAdjustEnvelope == NIL)
			{
			 FailurePoint14:
				ReleasePtr((char*)State->OutputScalingFactorLFO);
				goto FailurePoint13;
			}
		State->OutputOriginAdjustLFO = (LFOGenRec***)AllocPtrCanFail(sizeof(LFOGenRec**)
			* Template->NumberOfOscillators,"OutputOriginAdjustLFO[]");
		if (State->OutputOriginAdjustLFO == NIL)
			{
			 FailurePoint15:
				ReleasePtr((char*)State->OutputOriginAdjustEnvelope);
				goto FailurePoint14;
			}

		MaxOrigin = 0;
		for (Scan = 0; Scan < Template->NumberOfOscillators; Scan += 1)
			{
				long								TempOrigin;
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
					sizeof(Template->TemplateArray[Scan]));
				State->StateArray[Scan].OscillatorType
					= Template->TemplateArray[Scan].OscillatorType;
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NewModOscState:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							State->StateArray[Scan].u.SampleState = NewSampleState(
								Template->TemplateArray[Scan].u.SampleTemplate,FreqForMultisampling,
								Accent1,Accent2,Accent3,Accent4,Loudness,HurryUp,&TempOrigin,
								StereoPosition,InitialFrequency);
							if (State->StateArray[Scan].u.SampleState == NIL)
								{
								 FailurePoint16a:
									for (Index = 0; Index < Scan; Index += 1)
										{
											long								Dumper;

											switch (State->StateArray[Index].OscillatorType)
												{
													default:
														EXECUTE(PRERR(ForceAbort,"NewModOscState:  bad oscillator type"));
														break;
													case eOscillatorSampled:
														DisposeSampleState(State->StateArray[Index].u.SampleState);
														break;
													case eOscillatorWaveTable:
														DisposeWaveTableState(State->StateArray[Index].u.WaveTableState);
														break;
												}
											for (Dumper = 0; Dumper < Template->PhaseGenNumElements[Index]; Dumper += 1)
												{
													DisposeEnvelopeStateRecord(State->PhaseGenScalingFactorEnvelope[Index][Dumper]);
													DisposeLFOGenerator(State->PhaseGenScalingFactorLFO[Index][Dumper]);
													DisposeEnvelopeStateRecord(State->PhaseGenOriginAdjustEnvelope[Index][Dumper]);
													DisposeLFOGenerator(State->PhaseGenOriginAdjustLFO[Index][Dumper]);
												}
											for (Dumper = 0; Dumper < Template->OutputNumElements[Index]; Dumper += 1)
												{
													DisposeEnvelopeStateRecord(State->OutputScalingFactorEnvelope[Index][Dumper]);
													DisposeLFOGenerator(State->OutputScalingFactorLFO[Index][Dumper]);
													DisposeEnvelopeStateRecord(State->OutputOriginAdjustEnvelope[Index][Dumper]);
													DisposeLFOGenerator(State->OutputOriginAdjustLFO[Index][Dumper]);
												}
											ReleasePtr((char*)State->PhaseGenModulationScaling[Index]);
											ReleasePtr((char*)State->PhaseGenModulationOrigin[Index]);
											ReleasePtr((char*)State->PhaseGenScalingFactorEnvelope[Index]);
											ReleasePtr((char*)State->PhaseGenScalingFactorLFO[Index]);
											ReleasePtr((char*)State->PhaseGenOriginAdjustEnvelope[Index]);
											ReleasePtr((char*)State->PhaseGenOriginAdjustLFO[Index]);
											ReleasePtr((char*)State->OutputModulationScaling[Index]);
											ReleasePtr((char*)State->OutputModulationOrigin[Index]);
											ReleasePtr((char*)State->OutputScalingFactorEnvelope[Index]);
											ReleasePtr((char*)State->OutputScalingFactorLFO[Index]);
											ReleasePtr((char*)State->OutputOriginAdjustEnvelope[Index]);
											ReleasePtr((char*)State->OutputOriginAdjustLFO);
										}
									ReleasePtr((char*)State->OutputOriginAdjustLFO);
									goto FailurePoint15;
								}
							break;
						case eOscillatorWaveTable:
							State->StateArray[Scan].u.WaveTableState = NewWaveTableState(
								Template->TemplateArray[Scan].u.WaveTableTemplate,FreqForMultisampling,
								Accent1,Accent2,Accent3,Accent4,Loudness,HurryUp,&TempOrigin,
								StereoPosition,InitialFrequency);
							if (State->StateArray[Scan].u.WaveTableState == NIL)
								{
									goto FailurePoint16a;
								}
							break;
					}

				if (TempOrigin > MaxOrigin)
					{
						MaxOrigin = TempOrigin;
					}

				PRNGCHK(State->OldValues,&(State->OldValues[Scan]),sizeof(State->OldValues[Scan]));
				State->OldValues[Scan] = 0;

				State->PhaseGenScalingFactorEnvelope[Scan] = (EvalEnvelopeRec**)
					AllocPtrCanFail(sizeof(EvalEnvelopeRec*) * Template->PhaseGenNumElements[Scan],
					"PhaseGenScalingFactorEnvelope[][]");
				if (State->PhaseGenScalingFactorEnvelope[Scan] == NIL)
					{
					 FailurePoint16b:
						switch (State->StateArray[Scan].OscillatorType)
							{
								default:
									EXECUTE(PRERR(ForceAbort,"NewModOscState:  bad oscillator type"));
									break;
								case eOscillatorSampled:
									DisposeSampleState(State->StateArray[Scan].u.SampleState);
									break;
								case eOscillatorWaveTable:
									DisposeWaveTableState(State->StateArray[Scan].u.WaveTableState);
									break;
							}
						goto FailurePoint16a;
					}
				for (Index = 0; Index < Template->PhaseGenNumElements[Scan]; Index += 1)
					{
						State->PhaseGenScalingFactorEnvelope[Scan][Index] = NewEnvelopeStateRecord(
							Template->PhaseGenScalingFactorEnvelope[Scan][Index],Accent1,Accent2,
							Accent3,Accent4,InitialFrequency,1,HurryUp,Template->TicksPerSecond,
							&TempOrigin);
						if (State->PhaseGenScalingFactorEnvelope[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16c:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeEnvelopeStateRecord(
											State->PhaseGenScalingFactorEnvelope[Scan][Dumper]);
									}
								ReleasePtr((char*)State->PhaseGenScalingFactorEnvelope[Scan]);
								goto FailurePoint16b;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->PhaseGenScalingFactorLFO[Scan] = (LFOGenRec**)AllocPtrCanFail(
					sizeof(LFOGenRec*) * Template->PhaseGenNumElements[Scan],
					"PhaseGenScalingFactorLFO[][]");
				if (State->PhaseGenScalingFactorLFO[Scan] == NIL)
					{
					 FailurePoint16d:
						Index = Template->PhaseGenNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16c;
					}
				for (Index = 0; Index < Template->PhaseGenNumElements[Scan]; Index += 1)
					{
						State->PhaseGenScalingFactorLFO[Scan][Index] = NewLFOGenerator(
							Template->PhaseGenScalingFactorLFOList[Scan][Index],&TempOrigin,
							Accent1,Accent2,Accent3,Accent4,InitialFrequency,HurryUp,
							Template->TicksPerSecond,1,1,eLFOArithDefault,FreqForMultisampling);
						if (State->PhaseGenScalingFactorLFO[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16e:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeLFOGenerator(State->PhaseGenScalingFactorLFO[Scan][Dumper]);
									}
								ReleasePtr((char*)State->PhaseGenScalingFactorLFO[Scan]);
								goto FailurePoint16d;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->PhaseGenOriginAdjustEnvelope[Scan] = (EvalEnvelopeRec**)
					AllocPtrCanFail(sizeof(EvalEnvelopeRec*) * Template->PhaseGenNumElements[Scan],
					"PhaseGenOriginAdjustEnvelope[][]");
				if (State->PhaseGenOriginAdjustEnvelope[Scan] == NIL)
					{
					 FailurePoint16f:
						Index = Template->PhaseGenNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16e;
					}
				for (Index = 0; Index < Template->PhaseGenNumElements[Scan]; Index += 1)
					{
						State->PhaseGenOriginAdjustEnvelope[Scan][Index] = NewEnvelopeStateRecord(
							Template->PhaseGenOriginAdjustEnvelope[Scan][Index],Accent1,Accent2,
							Accent3,Accent4,InitialFrequency,1,HurryUp,Template->TicksPerSecond,
							&TempOrigin);
						if (State->PhaseGenOriginAdjustEnvelope[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16g:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeEnvelopeStateRecord(
											State->PhaseGenOriginAdjustEnvelope[Scan][Dumper]);
									}
								ReleasePtr((char*)State->PhaseGenOriginAdjustEnvelope[Scan]);
								goto FailurePoint16f;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->PhaseGenOriginAdjustLFO[Scan] = (LFOGenRec**)AllocPtrCanFail(
					sizeof(LFOGenRec*) * Template->PhaseGenNumElements[Scan],
					"PhaseGenOriginAdjustLFO[][]");
				if (State->PhaseGenOriginAdjustLFO[Scan] == NIL)
					{
					 FailurePoint16h:
						Index = Template->PhaseGenNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16g;
					}
				for (Index = 0; Index < Template->PhaseGenNumElements[Scan]; Index += 1)
					{
						State->PhaseGenOriginAdjustLFO[Scan][Index] = NewLFOGenerator(
							Template->PhaseGenOriginAdjustLFOList[Scan][Index],&TempOrigin,
							Accent1,Accent2,Accent3,Accent4,InitialFrequency,HurryUp,
							Template->TicksPerSecond,1,1,eLFOArithDefault,FreqForMultisampling);
						if (State->PhaseGenOriginAdjustLFO[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16i:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeLFOGenerator(State->PhaseGenOriginAdjustLFO[Scan][Dumper]);
									}
								ReleasePtr((char*)State->PhaseGenOriginAdjustLFO[Scan]);
								goto FailurePoint16h;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->OutputScalingFactorEnvelope[Scan] = (EvalEnvelopeRec**)
					AllocPtrCanFail(sizeof(EvalEnvelopeRec*) * Template->OutputNumElements[Scan],
					"OutputScalingFactorEnvelope[][]");
				if (State->OutputScalingFactorEnvelope[Scan] == NIL)
					{
					 FailurePoint16j:
						Index = Template->PhaseGenNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16i;
					}
				for (Index = 0; Index < Template->OutputNumElements[Scan]; Index += 1)
					{
						State->OutputScalingFactorEnvelope[Scan][Index] = NewEnvelopeStateRecord(
							Template->OutputScalingFactorEnvelope[Scan][Index],Accent1,Accent2,
							Accent3,Accent4,InitialFrequency,1,HurryUp,Template->TicksPerSecond,
							&TempOrigin);
						if (State->OutputScalingFactorEnvelope[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16k:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeEnvelopeStateRecord(
											State->OutputScalingFactorEnvelope[Scan][Dumper]);
									}
								ReleasePtr((char*)State->OutputScalingFactorEnvelope[Scan]);
								goto FailurePoint16j;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->OutputScalingFactorLFO[Scan] = (LFOGenRec**)AllocPtrCanFail(
					sizeof(LFOGenRec*) * Template->OutputNumElements[Scan],
					"OutputScalingFactorLFO[][]");
				if (State->OutputScalingFactorLFO[Scan] == NIL)
					{
					 FailurePoint16l:
						Index = Template->OutputNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16k;
					}
				for (Index = 0; Index < Template->OutputNumElements[Scan]; Index += 1)
					{
						State->OutputScalingFactorLFO[Scan][Index] = NewLFOGenerator(
							Template->OutputScalingFactorLFOList[Scan][Index],&TempOrigin,
							Accent1,Accent2,Accent3,Accent4,InitialFrequency,HurryUp,
							Template->TicksPerSecond,1,1,eLFOArithDefault,FreqForMultisampling);
						if (State->OutputScalingFactorLFO[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16m:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeLFOGenerator(State->OutputScalingFactorLFO[Scan][Dumper]);
									}
								ReleasePtr((char*)State->OutputScalingFactorLFO[Scan]);
								goto FailurePoint16l;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->OutputOriginAdjustEnvelope[Scan] = (EvalEnvelopeRec**)
					AllocPtrCanFail(sizeof(EvalEnvelopeRec*) * Template->OutputNumElements[Scan],
					"OutputOriginAdjustEnvelope[][]");
				if (State->OutputOriginAdjustEnvelope[Scan] == NIL)
					{
					 FailurePoint16n:
						Index = Template->OutputNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16m;
					}
				for (Index = 0; Index < Template->OutputNumElements[Scan]; Index += 1)
					{
						State->OutputOriginAdjustEnvelope[Scan][Index] = NewEnvelopeStateRecord(
							Template->OutputOriginAdjustEnvelope[Scan][Index],Accent1,Accent2,
							Accent3,Accent4,InitialFrequency,1,HurryUp,Template->TicksPerSecond,
							&TempOrigin);
						if (State->OutputOriginAdjustEnvelope[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16o:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeEnvelopeStateRecord(
											State->OutputOriginAdjustEnvelope[Scan][Dumper]);
									}
								ReleasePtr((char*)State->OutputOriginAdjustEnvelope[Scan]);
								goto FailurePoint16n;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->OutputOriginAdjustLFO[Scan] = (LFOGenRec**)AllocPtrCanFail(
					sizeof(LFOGenRec*) * Template->OutputNumElements[Scan],
					"OutputOriginAdjustLFO[][]");
				if (State->OutputOriginAdjustLFO[Scan] == NIL)
					{
					 FailurePoint16p:
						Index = Template->OutputNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16o;
					}
				for (Index = 0; Index < Template->OutputNumElements[Scan]; Index += 1)
					{
						State->OutputOriginAdjustLFO[Scan][Index] = NewLFOGenerator(
							Template->OutputOriginAdjustLFOList[Scan][Index],&TempOrigin,
							Accent1,Accent2,Accent3,Accent4,InitialFrequency,HurryUp,
							Template->TicksPerSecond,1,1,eLFOArithDefault,FreqForMultisampling);
						if (State->OutputOriginAdjustLFO[Scan][Index] == NIL)
							{
								long								Dumper;

							 FailurePoint16q:
								for (Dumper = 0; Dumper < Index; Dumper += 1)
									{
										DisposeLFOGenerator(State->OutputOriginAdjustLFO[Scan][Dumper]);
									}
								ReleasePtr((char*)State->OutputOriginAdjustLFO[Scan]);
								goto FailurePoint16p;
							}
						if (TempOrigin > MaxOrigin)
							{
								MaxOrigin = TempOrigin;
							}
					}

				State->PhaseGenModulationScaling[Scan] = (float*)AllocPtrCanFail(
					sizeof(float) * Template->PhaseGenNumElements[Scan],
					"PhaseGenModulationScaling[][]");
				if (State->PhaseGenModulationScaling[Scan] == NIL)
					{
					 FailurePoint16r:
						Index = Template->OutputNumElements[Scan]; /* fudge up index for previous */
						goto FailurePoint16q;
					}

				State->PhaseGenModulationOrigin[Scan] = (float*)AllocPtrCanFail(
					sizeof(float) * Template->PhaseGenNumElements[Scan],
					"PhaseGenModulationOrigin[][]");
				if (State->PhaseGenModulationOrigin[Scan] == NIL)
					{
					 FailurePoint16s:
						ReleasePtr((char*)State->PhaseGenModulationScaling[Scan]);
						goto FailurePoint16r;
					}

				State->OutputModulationScaling[Scan] = (float*)AllocPtrCanFail(
					sizeof(float) * Template->OutputNumElements[Scan],"OutputModulationScaling[][]");
				if (State->OutputModulationScaling[Scan] == NIL)
					{
					 FailurePoint16t:
						ReleasePtr((char*)State->PhaseGenModulationOrigin[Scan]);
						goto FailurePoint16s;
					}

				State->OutputModulationOrigin[Scan] = (float*)AllocPtrCanFail(
					sizeof(float) * Template->OutputNumElements[Scan],"OutputModulationOrigin[][]");
				if (State->OutputModulationOrigin[Scan] == NIL)
					{
					 FailurePoint16u:
						ReleasePtr((char*)State->OutputModulationScaling[Scan]);
						goto FailurePoint16t;
					}
			}

		State->Template = *Template;
		*PreOriginTimeOut = MaxOrigin;

		return State;
	}


/* fix up pre-origin time for the modulation oscillator state object */
void									FixUpModOscStatePreOrigin(ModOscStateRec* State,
												long ActualPreOrigin)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"FixUpModOscStatePreOrigin:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							FixUpSampleStatePreOrigin(State->StateArray[Scan].u.SampleState,
								ActualPreOrigin);
							break;
						case eOscillatorWaveTable:
							FixUpWaveTableStatePreOrigin(State->StateArray[Scan].u.WaveTableState,
								ActualPreOrigin);
							break;
					}

				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						EnvelopeStateFixUpInitialDelay(State->PhaseGenScalingFactorEnvelope[
							Scan][Index],ActualPreOrigin);
						LFOGeneratorFixEnvelopeOrigins(State->PhaseGenScalingFactorLFO[Scan][Index],
							ActualPreOrigin);
						EnvelopeStateFixUpInitialDelay(State->PhaseGenOriginAdjustEnvelope[
							Scan][Index],ActualPreOrigin);
						LFOGeneratorFixEnvelopeOrigins(State->PhaseGenOriginAdjustLFO[Scan][Index],
							ActualPreOrigin);
					}

				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						EnvelopeStateFixUpInitialDelay(State->OutputScalingFactorEnvelope[
							Scan][Index],ActualPreOrigin);
						LFOGeneratorFixEnvelopeOrigins(State->OutputScalingFactorLFO[Scan][Index],
							ActualPreOrigin);
						EnvelopeStateFixUpInitialDelay(State->OutputOriginAdjustEnvelope[
							Scan][Index],ActualPreOrigin);
						LFOGeneratorFixEnvelopeOrigins(State->OutputOriginAdjustLFO[Scan][Index],
							ActualPreOrigin);
					}
			}
	}


/* set a new frequency for a modulation oscillator state object.  used for */
/* portamento and modulation of frequency (vibrato) */
void									ModOscStateNewFrequency(ModOscStateRec* State,
												float NewFrequencyHertz)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ModOscStateNewFrequency:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							SampleStateNewFrequency(State->StateArray[Scan].u.SampleState,
								NewFrequencyHertz);
							break;
						case eOscillatorWaveTable:
							WaveTableStateNewFrequency(State->StateArray[Scan].u.WaveTableState,
								NewFrequencyHertz);
							break;
					}
			}
	}


/* send a key-up signal to one of the oscillators */
void									ModOscKeyUpSustain1(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ModOscKeyUpSustain1:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							SampleKeyUpSustain1(State->StateArray[Scan].u.SampleState);
							break;
						case eOscillatorWaveTable:
							WaveTableKeyUpSustain1(State->StateArray[Scan].u.WaveTableState);
							break;
					}

				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain1(State->PhaseGenScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain1(State->PhaseGenScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain1(State->PhaseGenOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain1(State->PhaseGenOriginAdjustLFO[Scan][Index]);
					}

				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain1(State->OutputScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain1(State->OutputScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain1(State->OutputOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain1(State->OutputOriginAdjustLFO[Scan][Index]);
					}
			}
	}


/* send a key-up signal to one of the oscillators */
void									ModOscKeyUpSustain2(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ModOscKeyUpSustain2:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							SampleKeyUpSustain2(State->StateArray[Scan].u.SampleState);
							break;
						case eOscillatorWaveTable:
							WaveTableKeyUpSustain2(State->StateArray[Scan].u.WaveTableState);
							break;
					}

				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain2(State->PhaseGenScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain2(State->PhaseGenScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain2(State->PhaseGenOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain2(State->PhaseGenOriginAdjustLFO[Scan][Index]);
					}

				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain2(State->OutputScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain2(State->OutputScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain2(State->OutputOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain2(State->OutputOriginAdjustLFO[Scan][Index]);
					}
			}
	}


/* send a key-up signal to one of the oscillators */
void									ModOscKeyUpSustain3(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ModOscKeyUpSustain3:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							SampleKeyUpSustain3(State->StateArray[Scan].u.SampleState);
							break;
						case eOscillatorWaveTable:
							WaveTableKeyUpSustain3(State->StateArray[Scan].u.WaveTableState);
							break;
					}

				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain3(State->PhaseGenScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain3(State->PhaseGenScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain3(State->PhaseGenOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain3(State->PhaseGenOriginAdjustLFO[Scan][Index]);
					}

				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						EnvelopeKeyUpSustain3(State->OutputScalingFactorEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain3(State->OutputScalingFactorLFO[Scan][Index]);
						EnvelopeKeyUpSustain3(State->OutputOriginAdjustEnvelope[Scan][Index]);
						LFOGeneratorKeyUpSustain3(State->OutputOriginAdjustLFO[Scan][Index]);
					}
			}
	}


/* restart a modulation oscillator oscillator.  this is used for tie continuations */
void									RestartModOscState(ModOscStateRec* State,
												float NewFreqMultisampling, float NewAccent1, float NewAccent2,
												float NewAccent3, float NewAccent4, float NewLoudness,
												float NewHurryUp, MyBoolean RetriggerEnvelopes,
												float NewStereoPosition, float NewInitialFrequency)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				long								Index;

				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				switch (State->StateArray[Scan].OscillatorType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"RestartModOscState:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							RestartSampleState(State->StateArray[Scan].u.SampleState,
								NewFreqMultisampling,NewAccent1,NewAccent2,NewAccent3,NewAccent4,
								NewLoudness,NewHurryUp,RetriggerEnvelopes,NewStereoPosition,
								NewInitialFrequency);
							break;
						case eOscillatorWaveTable:
							RestartWaveTableState(State->StateArray[Scan].u.WaveTableState,
								NewFreqMultisampling,NewAccent1,NewAccent2,NewAccent3,NewAccent4,
								NewLoudness,NewHurryUp,RetriggerEnvelopes,NewStereoPosition,
								NewInitialFrequency);
							break;
					}

				for (Index = 0; Index < State->Template.PhaseGenNumElements[Scan]; Index += 1)
					{
						EnvelopeRetriggerFromOrigin(State->PhaseGenScalingFactorEnvelope[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							1,NewHurryUp,State->Template.TicksPerSecond,RetriggerEnvelopes);
						LFOGeneratorRetriggerFromOrigin(State->PhaseGenScalingFactorLFO[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							NewHurryUp,State->Template.TicksPerSecond,1,1,RetriggerEnvelopes);
						EnvelopeRetriggerFromOrigin(State->PhaseGenOriginAdjustEnvelope[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							1,NewHurryUp,State->Template.TicksPerSecond,RetriggerEnvelopes);
						LFOGeneratorRetriggerFromOrigin(State->PhaseGenOriginAdjustLFO[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							NewHurryUp,State->Template.TicksPerSecond,1,1,RetriggerEnvelopes);
					}

				for (Index = 0; Index < State->Template.OutputNumElements[Scan]; Index += 1)
					{
						EnvelopeRetriggerFromOrigin(State->OutputScalingFactorEnvelope[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							1,NewHurryUp,State->Template.TicksPerSecond,RetriggerEnvelopes);
						LFOGeneratorRetriggerFromOrigin(State->OutputScalingFactorLFO[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							NewHurryUp,State->Template.TicksPerSecond,1,1,RetriggerEnvelopes);
						EnvelopeRetriggerFromOrigin(State->OutputOriginAdjustEnvelope[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							1,NewHurryUp,State->Template.TicksPerSecond,RetriggerEnvelopes);
						LFOGeneratorRetriggerFromOrigin(State->OutputOriginAdjustLFO[Scan][Index],
							NewAccent1,NewAccent2,NewAccent3,NewAccent4,NewInitialFrequency,
							NewHurryUp,State->Template.TicksPerSecond,1,1,RetriggerEnvelopes);
					}
			}
	}


/* generate a sequence of samples (called for each envelope clock) */
void									ModOscGenSamples(ModOscStateRec* State,
												long SampleCount, largefixedsigned* RawBuffer)
	{
		CheckPtrExistence(State);
		ValidateState(State);

		if (State->Template.StereoPlayback)
			{
				PRNGCHK(RawBuffer,RawBuffer,SampleCount * 2 * sizeof(largefixedsigned));
			}
		 else
			{
				PRNGCHK(RawBuffer,RawBuffer,SampleCount * sizeof(largefixedsigned));
			}
		while (SampleCount > 0)
			{
				long								Scan;
				float*							Temp;

				for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
					{
						PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
							sizeof(State->StateArray[Scan]));
						PRNGCHK(State->BuildOldValues,&(State->BuildOldValues[Scan]),
							sizeof(State->BuildOldValues[Scan]));
						switch (State->StateArray[Scan].OscillatorType)
							{
								default:
									EXECUTE(PRERR(ForceAbort,"ModOscGenSamples:  bad oscillator type"));
									break;
								case eOscillatorSampled:
									State->BuildOldValues[Scan] = SampleGenOneSample(
										State->StateArray[Scan].u.SampleState,
										State->Template.PhaseGenModulateHow[Scan],
										State->PhaseGenModulationScaling[Scan],
										State->PhaseGenModulationOrigin[Scan],
										State->OldValues,
										State->Template.PhaseGenIndirectionTable[Scan],
										State->Template.PhaseGenNumElements[Scan],
										State->Template.OutputModulateHow[Scan],
										State->OutputModulationScaling[Scan],
										State->OutputModulationOrigin[Scan],
										State->OldValues,
										State->Template.OutputIndirectionTable[Scan],
										State->Template.OutputNumElements[Scan],
										RawBuffer);
									break;
								case eOscillatorWaveTable:
									State->BuildOldValues[Scan] = WaveTableGenOneSample(
										State->StateArray[Scan].u.WaveTableState,
										State->Template.PhaseGenModulateHow[Scan],
										State->PhaseGenModulationScaling[Scan],
										State->PhaseGenModulationOrigin[Scan],
										State->OldValues,
										State->Template.PhaseGenIndirectionTable[Scan],
										State->Template.PhaseGenNumElements[Scan],
										State->Template.OutputModulateHow[Scan],
										State->OutputModulationScaling[Scan],
										State->OutputModulationOrigin[Scan],
										State->OldValues,
										State->Template.OutputIndirectionTable[Scan],
										State->Template.OutputNumElements[Scan],
										RawBuffer);
									break;
							}
					}
				if (State->Template.StereoPlayback)
					{
						RawBuffer += 2;
					}
				 else
					{
						RawBuffer += 1;
					}
				SampleCount -= 1;
				Temp = State->BuildOldValues;
				State->BuildOldValues = State->OldValues;
				State->OldValues = Temp;
			}
	}


/* find out if the modulation oscillator has finished */
MyBoolean							ModOscIsItFinished(ModOscStateRec* State)
	{
		long								Scan;

		CheckPtrExistence(State);
		ValidateState(State);

		for (Scan = 0; Scan < State->Template.NumberOfOscillators; Scan += 1)
			{
				PRNGCHK(State->StateArray,&(State->StateArray[Scan]),
					sizeof(State->StateArray[Scan]));
				if (State->StateArray[Scan].OscillatorType == eOscillatorSampled)
					{
						if (!SampleIsItFinished(State->StateArray[Scan].u.SampleState))
							{
								return False; /* someone is still running */
							}
					}
				 else
					{
						if (!WaveTableIsItFinished(State->StateArray[Scan].u.WaveTableState))
							{
								return False; /* someone is still running */
							}
					}
			}

		return True; /* nobody is still running */
	}
