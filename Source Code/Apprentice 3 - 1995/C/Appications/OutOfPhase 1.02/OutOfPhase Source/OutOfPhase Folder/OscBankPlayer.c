/* OscBankPlayer.c */
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

#define ShowMeFrozenNoteRec
#define ShowMe_NoteObjectRec
#include "OscBankPlayer.h"
#include "LFOGenerator.h"
#include "IncrementalParameterUpdator.h"
#include "LFOSpecifier.h"
#include "LFOListSpecifier.h"
#include "Memory.h"
#include "Array.h"
#include "OscillatorListSpecifier.h"
#include "OscillatorSpecifier.h"
#include "InstrumentStructure.h"
#include "GenInstrTopology.h"
#include "SampleOscControl.h"
#include "WaveTableOscControl.h"
#include "ModulationOscControl.h"
#include "DeterminedNoteStructure.h"
#include "NoteObject.h"
#include "FloatingPoint.h"
#include "Frequency.h"
#include "ErrorDaemon.h"


typedef struct
	{
		/* this is the reference to the template object */
		void*										TemplateReference;

		/* perform one envelope update cycle */
		void										(*UpdateEnvelopes)(void* State);
		/* dispose of the state record */
		void										(*DisposeState)(void* State);
		/* dispose of the information template */
		void										(*DisposeTemplate)(void* Template);
		/* create a new state object. */
		void*										(*NewState)(void* Template,
															float FreqForMultisampling, float Accent1, float Accent2,
															float Accent3, float Accent4, float Loudness, float HurryUp,
															long* PreOriginTimeOut, float StereoPosition,
															float InitialFrequency);
		/* fix up pre-origin time for the state object */
		void										(*FixUpStatePreOrigin)(void* State, long ActualPreOrigin);
		/* set a new frequency for a state object.  used for portamento */
		/* and modulation of frequency (vibrato) */
		void										(*StateNewFrequency)(void* State, float NewFrequencyHertz);
		/* send a key-up signal to one of the oscillators */
		void										(*KeyUpSustain1)(void* State);
		void										(*KeyUpSustain2)(void* State);
		void										(*KeyUpSustain3)(void* State);
		/* restart a oscillator.  this is used for tie continuations */
		void										(*RestartState)(void* State, float NewFreqMultisampling,
															float NewAccent1, float NewAccent2, float NewAccent3,
															float NewAccent4, float NewLoudness, float NewHurryUp,
															MyBoolean RetriggerEnvelopes, float NewStereoPosition,
															float NewInitialFrequency);
		/* generate a sequence of samples (called for each envelope clock) */
		void										(*GenSamples)(void* State, long SampleCount,
															largefixedsigned* RawBuffer);
		/* find out if the oscillator has finished yet */
		MyBoolean								(*IsItFinished)(void* State);
	} OscBankVectorRec;


struct OscBankTemplateRec
	{
		/* general parameters */
		MyBoolean								StereoOutput;
		float										OverallVolumeScalingFactor;
		long										SamplingRate;
		float										EnvelopeUpdateRate;
		MyBoolean								TimeInterpolation;
		MyBoolean								WaveInterpolation;

		/* parameter updator for track (so we can set up individual notes properly) */
		IncrParamUpdateRec*			ParamUpdator;

		/* template for the pitch displacement LFO */
		LFOListSpecRec*					PitchLFOTemplate;

		/* instrument overall loudness */
		float										InstrOverallLoudness;

		/* list of template records describing each oscillator */
		OscBankVectorRec*				TemplateArray;
		/* this is the number of oscillators in the array */
		long										NumOscillatorsInBank;
	};


typedef struct OscStateRec
	{
		/* this is a reference to the state object for this oscillator */
		void*										StateReference;

		/* copy of the routine vectors */
		OscBankVectorRec				Template;

		/* next oscillator in the list */
		struct OscStateRec*			Next;
	} OscStateRec;


struct OscStateBankRec
	{
		/* what are we derived from */
		OscBankTemplateRec*			BankTemplate;

		/* list of oscillators that this oscillator bank is comprised of */
		OscStateRec*						OscillatorList;

		/* this calculates the differential values for periodic pitch displacements */
		LFOGenRec*							PitchLFO;

		/* if this object ties to a note, then this is the note to tie to.  this is */
		/* used for finding existing oscillators for tie continuations. */
		struct NoteObjectRec*		TieToNote;

		/* portamento control parameters */
		long										PortamentoCounter; /* 0 = done */
		long										TotalPortamentoTicks;
		float										InitialFrequency;
		float										FinalFrequency;
		float										CurrentFrequency;
		/* True = portamento linear to Hertz; False = portamento linear to half-steps */
		MyBoolean								PortamentoHertz;
		MyBoolean								PitchLFOHertz;

		/* various counters (in terms of envelope ticks) */
		/* negative = expired */
		long										Release1Countdown;
		long										Release2Countdown;
		long										Release3Countdown;
		long										PitchLFOStartCountdown;

		/* next oscillator bank in the list */
		struct OscStateBankRec*	Next;
	};


static OscStateRec*							StateFreeList = NIL;
static OscStateBankRec*					StateBankFreeList = NIL;


/* flush all cached oscillator state bank records */
void									FlushCachedOscStateBankRecords(void)
	{
		while (StateBankFreeList != NIL)
			{
				OscStateBankRec*		Temp;

				Temp = StateBankFreeList;
				StateBankFreeList = StateBankFreeList->Next;
				ReleasePtr((char*)Temp);
			}

		while (StateFreeList != NIL)
			{
				OscStateRec*				Temp;

				Temp = StateFreeList;
				StateFreeList = StateFreeList->Next;
				ReleasePtr((char*)Temp);
			}
	}


#if DEBUG
static void					ValidateOscState(OscStateRec* State)
	{
		OscStateRec*			Scan;

		CheckPtrExistence(State);
		Scan = StateFreeList;
		while (Scan != NIL)
			{
				if (Scan == State)
					{
						PRERR(ForceAbort,"ValidateOscState:  state object is on free list");
					}
				Scan = Scan->Next;
			}
	}
#else
#define ValidateOscState(x) ((void)0)
#endif


#if DEBUG
static void					ValidateOscStateBank(OscStateBankRec* StateBank)
	{
		OscStateBankRec*	Scan;

		CheckPtrExistence(StateBank);
		Scan = StateBankFreeList;
		while (Scan != NIL)
			{
				if (Scan == StateBank)
					{
						PRERR(ForceAbort,"ValidateOscStateBank:  state bank object is on free list");
					}
				Scan = Scan->Next;
			}
	}
#else
#define ValidateOscStateBank(x) ((void)0)
#endif


/* construct an oscillator bank template record.  various parameters are passed in */
/* which are needed for synthesis.  ParameterUpdator is the parameter information */
/* record for the whole track of which this is a part. */
OscBankTemplateRec*		NewOscBankTemplate(struct InstrumentRec* InstrumentDefinition,
												MyBoolean StereoFlag, LargeBCDType OverallVolumeScalingReciprocal,
												long SamplingRate, float EnvelopeRate, MyBoolean TimeInterp,
												MyBoolean WaveInterp, struct IncrParamUpdateRec* ParameterUpdator,
												ErrorDaemonRec* ErrorDaemon)
	{
		OscBankTemplateRec*	Template;
		ArrayRec*						TwoDOscList;
		OscillatorListRec*	OscillatorListObject;
		OscillatorRec**			OscillatorVector;
		long								Scan;

		CheckPtrExistence(InstrumentDefinition);
		CheckPtrExistence(ParameterUpdator);
		CheckPtrExistence(ErrorDaemon);

		Template = (OscBankTemplateRec*)AllocPtrCanFail(sizeof(OscBankTemplateRec),
			"OscBankTemplateRec");
		if (Template == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		/* the oscillator bank template contains all of the information needed for */
		/* constructing oscillators as notes are to be executed. */
		/* number of oscillators in a bank. */
		OscillatorListObject = GetInstrumentOscillatorList(InstrumentDefinition);
		OscillatorVector = (OscillatorRec**)AllocPtrCanFail(sizeof(OscillatorRec*)
			* GetOscillatorListLength(OscillatorListObject),"OscillatorRec[]");
		if (OscillatorVector == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Template);
				goto FailurePoint1;
			}
		for (Scan = 0; Scan < GetOscillatorListLength(OscillatorListObject); Scan += 1)
			{
				PRNGCHK(OscillatorVector,&(OscillatorVector[Scan]),sizeof(OscillatorVector[Scan]));
				OscillatorVector[Scan] = GetOscillatorFromList(OscillatorListObject,Scan);
			}
		TwoDOscList = BuildOscillatorLists(OscillatorVector,
			GetOscillatorListLength(OscillatorListObject));
		if (TwoDOscList == NIL)
			{
			 FailurePoint3:
				ReleasePtr((char*)OscillatorVector);
				goto FailurePoint2;
			}

		/* get LFO information */
		Template->PitchLFOTemplate = GetInstrumentFrequencyLFOList(InstrumentDefinition);

		/* vector containing templates for all of the oscillators */
		Template->TemplateArray = (OscBankVectorRec*)AllocPtrCanFail(
			sizeof(OscBankVectorRec) * ArrayGetLength(TwoDOscList),
			"OscBankVectorRec");
		if (Template->TemplateArray == NIL)
			{
			 FailurePoint4:
				for (Scan = 0; Scan < ArrayGetLength(TwoDOscList); Scan += 1)
					{
						DisposeArray((ArrayRec*)ArrayGetElement(TwoDOscList,Scan));
					}
				DisposeArray(TwoDOscList);
				goto FailurePoint3;
			}

		Template->NumOscillatorsInBank = 0;
		for (Scan = 0; Scan < ArrayGetLength(TwoDOscList); Scan += 1)
			{
				ArrayRec*					OneOscArray;

				OneOscArray = (ArrayRec*)ArrayGetElement(TwoDOscList,Scan);
				CheckPtrExistence(OneOscArray);
				if ((ArrayGetLength(OneOscArray) == 1) && (GetModulationSpecNumEntries(
					OscillatorGetModulatorInputList((OscillatorRec*)ArrayGetElement(
					OneOscArray,0))) == 0))
					{
						OscillatorRec*		Osc;

						Osc = (OscillatorRec*)ArrayGetElement(OneOscArray,0);
						CheckPtrExistence(Osc);
						switch (OscillatorGetWhatKindItIs(Osc))
							{
								default:
									EXECUTE(PRERR(ForceAbort,"NewOscBankTemplate:  bad oscillator type"));
									break;
								case eOscillatorSampled:
									PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
										sizeof(Template->TemplateArray[Scan]));
									Template->TemplateArray[Scan].TemplateReference
										= NewSampleTemplate(Osc,EnvelopeRate,SamplingRate,StereoFlag,
										TimeInterp,WaveInterp,ErrorDaemon);
									if (Template->TemplateArray[Scan].TemplateReference == NIL)
										{
										 FailurePoint5:
											for (Scan = 0; Scan < Template->NumOscillatorsInBank; Scan += 1)
												{
													(*Template->TemplateArray[Scan]
														.DisposeTemplate)(Template->TemplateArray[
														Scan].TemplateReference);
												}
											goto FailurePoint4;
										}
									Template->TemplateArray[Scan].UpdateEnvelopes
										= (void (*)(void*))&UpdateSampleEnvelopes;
									Template->TemplateArray[Scan].DisposeState
										= (void (*)(void*))&DisposeSampleState;
									Template->TemplateArray[Scan].DisposeTemplate
										= (void (*)(void*))&DisposeSampleTemplate;
									Template->TemplateArray[Scan].NewState
										= (void* (*)(void*,float,float,float,float,float,
										float,float,long*,float,float))&NewSampleState;
									Template->TemplateArray[Scan].FixUpStatePreOrigin
										= (void (*)(void*,long))&FixUpSampleStatePreOrigin;
									Template->TemplateArray[Scan].StateNewFrequency
										= (void (*)(void*,float))&SampleStateNewFrequency;
									Template->TemplateArray[Scan].KeyUpSustain1
										= (void (*)(void*))&SampleKeyUpSustain1;
									Template->TemplateArray[Scan].KeyUpSustain2
										= (void (*)(void*))&SampleKeyUpSustain2;
									Template->TemplateArray[Scan].KeyUpSustain3
										= (void (*)(void*))&SampleKeyUpSustain3;
									Template->TemplateArray[Scan].RestartState
										= (void (*)(void*,float,float,float,float,float,float,float,
										MyBoolean,float,float))&RestartSampleState;
									Template->TemplateArray[Scan].GenSamples
										= (void (*)(void*,long,largefixedsigned*))&SampleGenSamples;
									Template->TemplateArray[Scan].IsItFinished
										= (MyBoolean (*)(void*))&SampleIsItFinished;
									break;
								case eOscillatorWaveTable:
									PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
										sizeof(Template->TemplateArray[Scan]));
									Template->TemplateArray[Scan].TemplateReference
										= NewWaveTableTemplate(Osc,EnvelopeRate,SamplingRate,StereoFlag,
										TimeInterp,WaveInterp,ErrorDaemon);
									if (Template->TemplateArray[Scan].TemplateReference == NIL)
										{
											goto FailurePoint5;
										}
									Template->TemplateArray[Scan].UpdateEnvelopes
										= (void (*)(void*))&UpdateWaveTableEnvelopes;
									Template->TemplateArray[Scan].DisposeState
										= (void (*)(void*))&DisposeWaveTableState;
									Template->TemplateArray[Scan].DisposeTemplate
										= (void (*)(void*))&DisposeWaveTableTemplate;
									Template->TemplateArray[Scan].NewState
										= (void* (*)(void*,float,float,float,float,float,
										float,float,long*,float,float))&NewWaveTableState;
									Template->TemplateArray[Scan].FixUpStatePreOrigin
										= (void (*)(void*,long))&FixUpWaveTableStatePreOrigin;
									Template->TemplateArray[Scan].StateNewFrequency
										= (void (*)(void*,float))&WaveTableStateNewFrequency;
									Template->TemplateArray[Scan].KeyUpSustain1
										= (void (*)(void*))&WaveTableKeyUpSustain1;
									Template->TemplateArray[Scan].KeyUpSustain2
										= (void (*)(void*))&WaveTableKeyUpSustain2;
									Template->TemplateArray[Scan].KeyUpSustain3
										= (void (*)(void*))&WaveTableKeyUpSustain3;
									Template->TemplateArray[Scan].RestartState
										= (void (*)(void*,float,float,float,float,float,float,float,
										MyBoolean,float,float))&RestartWaveTableState;
									Template->TemplateArray[Scan].GenSamples
										= (void (*)(void*,long,largefixedsigned*))&WaveTableGenSamples;
									Template->TemplateArray[Scan].IsItFinished
										= (MyBoolean (*)(void*))&WaveTableIsItFinished;
									break;
							}
					}
				 else
					{
						PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
							sizeof(Template->TemplateArray[Scan]));
						Template->TemplateArray[Scan].TemplateReference
							= NewModOscTemplate(OneOscArray,EnvelopeRate,SamplingRate,StereoFlag,
							TimeInterp,WaveInterp,ErrorDaemon);
						if (Template->TemplateArray[Scan].TemplateReference == NIL)
							{
								goto FailurePoint5;
							}
						Template->TemplateArray[Scan].UpdateEnvelopes
							= (void (*)(void*))&UpdateModOscEnvelopes;
						Template->TemplateArray[Scan].DisposeState
							= (void (*)(void*))&DisposeModOscState;
						Template->TemplateArray[Scan].DisposeTemplate
							= (void (*)(void*))&DisposeModOscTemplate;
						Template->TemplateArray[Scan].NewState
							= (void* (*)(void*,float,float,float,float,float,
							float,float,long*,float,float))&NewModOscState;
						Template->TemplateArray[Scan].FixUpStatePreOrigin
							= (void (*)(void*,long))&FixUpModOscStatePreOrigin;
						Template->TemplateArray[Scan].StateNewFrequency
							= (void (*)(void*,float))&ModOscStateNewFrequency;
						Template->TemplateArray[Scan].KeyUpSustain1
							= (void (*)(void*))&ModOscKeyUpSustain1;
						Template->TemplateArray[Scan].KeyUpSustain2
							= (void (*)(void*))&ModOscKeyUpSustain2;
						Template->TemplateArray[Scan].KeyUpSustain3
							= (void (*)(void*))&ModOscKeyUpSustain3;
						Template->TemplateArray[Scan].RestartState
							= (void (*)(void*,float,float,float,float,float,float,float,
							MyBoolean,float,float))&RestartModOscState;
						Template->TemplateArray[Scan].GenSamples
							= (void (*)(void*,long,largefixedsigned*))&ModOscGenSamples;
						Template->TemplateArray[Scan].IsItFinished
							= (MyBoolean (*)(void*))&ModOscIsItFinished;
					}
				Template->NumOscillatorsInBank += 1;
			}

		/* playback control parameters */
		Template->StereoOutput = StereoFlag;
		Template->OverallVolumeScalingFactor = 1.0 / LargeBCD2Single(OverallVolumeScalingReciprocal);
		Template->SamplingRate = SamplingRate;
		Template->EnvelopeUpdateRate = EnvelopeRate;
		Template->TimeInterpolation = TimeInterp;
		Template->WaveInterpolation = WaveInterp;

		Template->ParamUpdator = ParameterUpdator;

		Template->InstrOverallLoudness = GetInstrumentOverallLoudness(InstrumentDefinition);

		/* clean up the mess */
		for (Scan = 0; Scan < ArrayGetLength(TwoDOscList); Scan += 1)
			{
				DisposeArray((ArrayRec*)ArrayGetElement(TwoDOscList,Scan));
			}
		DisposeArray(TwoDOscList);
		ReleasePtr((char*)OscillatorVector);

		return Template;
	}


/* dispose of the template */
void									DisposeOscBankTemplate(OscBankTemplateRec* Template)
	{
		long								Scan;

		CheckPtrExistence(Template);

		for (Scan = 0; Scan < Template->NumOscillatorsInBank; Scan += 1)
			{
				PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
					sizeof(Template->TemplateArray[Scan]));
				(*Template->TemplateArray[Scan].DisposeTemplate)(Template->
					TemplateArray[Scan].TemplateReference);
			}
		ReleasePtr((char*)Template->TemplateArray);

		ReleasePtr((char*)Template);
	}


/* construct a new oscillator bank state object based on the note.  the note is */
/* assumed to start "now" in terms of the parameters in the ParameterUpdator.  */
/* the ScanningGapWidth is the number of envelope clock ticks in the current scanning */
/* gap.  this is used to determine how far later than "now" in terms of the back */
/* edge of the scanning gap (different from above) the osc bank should start playing. */
/* *WhenToStartPlayingOut returns the number of envelope ticks after the back edge */
/* of the scanning gap that the note should be started. */
/*     <already played>       |    <scanning gap>     |    <not yet analyzed> */
/*   time ---->    time ---->    time ---->    time ---->    time ---->   time ----> */
/*                            ^A                      ^B     */
/* point A is the back edge of the scanning gap.  as this edge moves forward in time, */
/*   oscillator bank state objects are removed from the queue and playback is commenced */
/*   for them. */
/* point B is the front edge of the scanning gap.  as this edge moves forward in time, */
/*   notes are extracted from the track and state bank objects are created for them. */
/*   ParameterUpdator always reflects parameters at this point in time. */
OscStateBankRec*			NewOscBankState(OscBankTemplateRec* Template,
												long* WhenToStartPlayingOut, struct NoteObjectRec* Note,
												float EnvelopeTicksPerDurationTick)
	{
		OscStateBankRec*		State;
		long								Scan;
		FrozenNoteRec*			FrozenNote;
		long								StartPointAdjust;
		long								MaxOscillatorPreOriginTime;
		OscStateRec*				OneState;
		long								ThisPreOriginTime;

		CheckPtrExistence(Template);
		CheckPtrExistence(Note);

		if (StateBankFreeList != NIL)
			{
				State = StateBankFreeList;
				StateBankFreeList = StateBankFreeList->Next;
			}
		 else
			{
				State = (OscStateBankRec*)AllocPtrCanFail(sizeof(OscStateBankRec),"OscStateBankRec");
				if (State == NIL)
					{
					 FailurePoint1:
						return NIL;
					}
			}
		EXECUTE(State->Next = (OscStateBankRec*)0x81818181;)

		State->BankTemplate = Template;

		/* freeze the parameters */
		FrozenNote = FixNoteParameters(Template->ParamUpdator,Note,&StartPointAdjust,
			Template->OverallVolumeScalingFactor,EnvelopeTicksPerDurationTick);
		if (FrozenNote == NIL)
			{
			 FailurePoint2:
				State->Next = StateBankFreeList;
				StateBankFreeList = State;
				goto FailurePoint1;
			}

		/* list of oscillators that this oscillator bank is comprised of */
		State->OscillatorList = NIL;
		MaxOscillatorPreOriginTime = 0;
		for (Scan = 0; Scan < Template->NumOscillatorsInBank; Scan += 1)
			{
				/* allocate the new record */
				if (StateFreeList != NIL)
					{
						OneState = StateFreeList;
						StateFreeList = StateFreeList->Next;
					}
				 else
					{
						OneState = (OscStateRec*)AllocPtrCanFail(sizeof(OscStateRec),"OscStateRec");
						if (OneState == NIL)
							{
							 FailurePoint3:
								while (State->OscillatorList != NIL)
									{
										/* delink object */
										OneState = State->OscillatorList;
										State->OscillatorList = State->OscillatorList->Next;
										/* dispose members */
										(*OneState->Template.DisposeState)(OneState->StateReference);
										/* stick on free list */
										OneState->Next = StateFreeList;
										StateFreeList = OneState;
									}
								goto FailurePoint2;
							}
					}

				/* copy over the function vectors */
				PRNGCHK(Template->TemplateArray,&(Template->TemplateArray[Scan]),
					sizeof(Template->TemplateArray[Scan]));
				OneState->Template = Template->TemplateArray[Scan];

				/* create the oscillator */
				OneState->StateReference = (OneState->Template.NewState)(
					OneState->Template.TemplateReference,FrozenNote->MultisampleFrequency,
					FrozenNote->Accent1,FrozenNote->Accent2,FrozenNote->Accent3,FrozenNote->Accent4,
					FrozenNote->LoudnessAdjust * Template->InstrOverallLoudness,
					FrozenNote->HurryUpFactor,&ThisPreOriginTime,FrozenNote->StereoPosition,
					FrozenNote->NominalFrequency);
				if (OneState->StateReference == NIL)
					{
					 FailurePoint3a:
						OneState->Next = StateFreeList;
						StateFreeList = OneState;
						goto FailurePoint3;
					}

				if (ThisPreOriginTime > MaxOscillatorPreOriginTime)
					{
						MaxOscillatorPreOriginTime = ThisPreOriginTime;
					}

				/* link it in */
				OneState->Next = State->OscillatorList;
				State->OscillatorList = OneState;
			}

		/* this calculates the differential values for periodic pitch displacements */
		State->PitchLFOHertz = FrozenNote->PitchDisplacementDepthInHertz;
		if (!State->PitchLFOHertz)
			{
				/* half steps */
				State->PitchLFO = NewLFOGenerator(Template->PitchLFOTemplate,&ThisPreOriginTime,
					FrozenNote->Accent1,FrozenNote->Accent2,FrozenNote->Accent3,FrozenNote->Accent4,
					FrozenNote->NominalFrequency,FrozenNote->HurryUpFactor,
					Template->EnvelopeUpdateRate,FrozenNote->PitchDisplacementDepthLimit / 12,
					FrozenNote->PitchDisplacementRateLimit,eLFOArithGeometric,
					FrozenNote->MultisampleFrequency);
			}
		 else
			{
				State->PitchLFO = NewLFOGenerator(Template->PitchLFOTemplate,&ThisPreOriginTime,
					FrozenNote->Accent1,FrozenNote->Accent2,FrozenNote->Accent3,FrozenNote->Accent4,
					FrozenNote->NominalFrequency,FrozenNote->HurryUpFactor,
					Template->EnvelopeUpdateRate,FrozenNote->PitchDisplacementDepthLimit,
					FrozenNote->PitchDisplacementRateLimit,eLFOArithAdditive,
					FrozenNote->MultisampleFrequency);
			}
		if (State->PitchLFO == NIL)
			{
			 FailurePoint4:
				goto FailurePoint3;
			}
		if (ThisPreOriginTime > MaxOscillatorPreOriginTime)
			{
				MaxOscillatorPreOriginTime = ThisPreOriginTime;
			}

		/* if this object ties to a note, then this is the note to tie to.  this is */
		/* used for finding existing oscillators for tie continuations. */
		State->TieToNote = Note->a.Note.Tie;

		/* portamento control parameters */
		State->PortamentoCounter = 0;
		State->CurrentFrequency = FrozenNote->NominalFrequency;

		/* fix up pre-origin times */
		OneState = State->OscillatorList;
		while (OneState != NIL)
			{
				(*OneState->Template.FixUpStatePreOrigin)(OneState->StateReference,
					MaxOscillatorPreOriginTime);
				OneState = OneState->Next;
			}
		LFOGeneratorFixEnvelopeOrigins(State->PitchLFO,MaxOscillatorPreOriginTime);

		/* various counters (in terms of envelope ticks) */
		if (State->TieToNote == NIL)
			{
				State->Release1Countdown = FrozenNote->ReleasePoint1
					+ MaxOscillatorPreOriginTime;
				State->Release2Countdown = FrozenNote->ReleasePoint2
					+ MaxOscillatorPreOriginTime;
				State->Release3Countdown = FrozenNote->ReleasePoint3
					+ MaxOscillatorPreOriginTime;
			}
		 else
			{
				/* for ties, only honor releases from start */
				if (FrozenNote->Release1FromStart)
					{
						State->Release1Countdown = FrozenNote->ReleasePoint1
							+ MaxOscillatorPreOriginTime;
					}
				 else
					{
						State->Release1Countdown = -1;
					}
				if (FrozenNote->Release2FromStart)
					{
						State->Release2Countdown = FrozenNote->ReleasePoint2
							+ MaxOscillatorPreOriginTime;
					}
				 else
					{
						State->Release2Countdown = -1;
					}
				if (FrozenNote->Release3FromStart)
					{
						State->Release3Countdown = FrozenNote->ReleasePoint3
							+ MaxOscillatorPreOriginTime;
					}
				 else
					{
						State->Release3Countdown = -1;
					}
			}
		State->PitchLFOStartCountdown = FrozenNote->PitchDisplacementStartPoint
			+ MaxOscillatorPreOriginTime;

		/* clean up */
		DisposeFrozenNote(FrozenNote);

		*WhenToStartPlayingOut = StartPointAdjust - MaxOscillatorPreOriginTime;

		return State;
	}


/* this is used for resetting a note for a tie */
/* the FrozenNote object is NOT disposed */
MyBoolean							ResetOscBankState(OscStateBankRec* State,
												struct FrozenNoteRec* FrozenNote,
												float EnvelopeTicksPerDurationTick)
	{
		OscStateRec*				OneState;
		MyBoolean						RetriggerEnvelopes;

		CheckPtrExistence(State);
		ValidateOscStateBank(State);
		CheckPtrExistence(FrozenNote);

		RetriggerEnvelopes = ((FrozenNote->OriginalNote->Flags
			& eRetriggerEnvelopesOnTieFlag) != 0);

		/* go through the oscillators and retrigger them */
		OneState = State->OscillatorList;
		while (OneState != NIL)
			{
				(*OneState->Template.RestartState)(OneState->StateReference,
					FrozenNote->MultisampleFrequency,FrozenNote->Accent1,FrozenNote->Accent2,
					FrozenNote->Accent3,FrozenNote->Accent4,FrozenNote->LoudnessAdjust
					* State->BankTemplate->InstrOverallLoudness,FrozenNote->HurryUpFactor,
					RetriggerEnvelopes,FrozenNote->StereoPosition,
					FrozenNote->NominalFrequency);
				OneState = OneState->Next;
			}

		State->PitchLFOHertz = FrozenNote->PitchDisplacementDepthInHertz;
		LFOGeneratorRetriggerFromOrigin(State->PitchLFO,FrozenNote->Accent1,
			FrozenNote->Accent2,FrozenNote->Accent3,FrozenNote->Accent4,
			FrozenNote->NominalFrequency,FrozenNote->HurryUpFactor,
			State->BankTemplate->EnvelopeUpdateRate,
			FrozenNote->PitchDisplacementDepthLimit
			/ (State->PitchLFOHertz ? 1 : 12), /* div by 12 only if half-steps */
			FrozenNote->PitchDisplacementRateLimit,RetriggerEnvelopes);

		/* if this object ties to a note, then this is the note to tie to.  this is */
		/* used for finding existing oscillators for tie continuations. */
		State->TieToNote = FrozenNote->OriginalNote->a.Note.Tie;

		/* portamento control parameters */
		if (FrozenNote->PortamentoDuration > 0)
			{
				State->PortamentoCounter = FrozenNote->PortamentoDuration;
				State->TotalPortamentoTicks = FrozenNote->PortamentoDuration;
				State->InitialFrequency = State->CurrentFrequency; /* save current pitch */
				State->FinalFrequency = FrozenNote->NominalFrequency;
				State->PortamentoHertz = ((FrozenNote->OriginalNote->Flags
					& ePortamentoHertzNotHalfsteps) != 0);
			}
		 else
			{
				State->PortamentoCounter = 0;
				State->CurrentFrequency = FrozenNote->NominalFrequency;
			}

		/* various counters (in terms of envelope ticks) */
		if (State->TieToNote == NIL)
			{
				State->Release1Countdown = FrozenNote->ReleasePoint1;
				State->Release2Countdown = FrozenNote->ReleasePoint2;
				State->Release3Countdown = FrozenNote->ReleasePoint3;
			}
		 else
			{
				/* for ties, only honor releases from start */
				if (FrozenNote->Release1FromStart)
					{
						State->Release1Countdown = FrozenNote->ReleasePoint1;
					}
				 else
					{
						State->Release1Countdown = -1;
					}
				if (FrozenNote->Release2FromStart)
					{
						State->Release2Countdown = FrozenNote->ReleasePoint2;
					}
				 else
					{
						State->Release2Countdown = -1;
					}
				if (FrozenNote->Release3FromStart)
					{
						State->Release3Countdown = FrozenNote->ReleasePoint3;
					}
				 else
					{
						State->Release3Countdown = -1;
					}
			}
		if (RetriggerEnvelopes)
			{
				State->PitchLFOStartCountdown = FrozenNote->PitchDisplacementStartPoint;
			}
		 else
			{
				State->PitchLFOStartCountdown = -1;
			}

		return True;
	}


/* get rid of a state bank */
void									DisposeOscStateBank(OscStateBankRec* State)
	{
		OscStateRec*				OneStateScan;

		CheckPtrExistence(State);
		ValidateOscStateBank(State);

		OneStateScan = State->OscillatorList;
		while (OneStateScan != NIL)
			{
				OscStateRec*				Temp;

				(*OneStateScan->Template.DisposeState)(OneStateScan->StateReference);
				Temp = OneStateScan;
				OneStateScan = OneStateScan->Next;
				Temp->Next = StateFreeList;
				StateFreeList = Temp;
			}

		DisposeLFOGenerator(State->PitchLFO);

		State->Next = StateBankFreeList;
		StateBankFreeList = State;
	}


/* get the reference to the note that this bank ties to.  NIL if it doesn't */
struct NoteObjectRec*	GetOscStateTieTarget(OscStateBankRec* State)
	{
		CheckPtrExistence(State);
		ValidateOscStateBank(State);

		return State->TieToNote;
	}


/* perform one envelope clock cycle on a state bank.  this returns True if the */
/* state bank is done and should be retired.  (it will return false if it is a */
/* tie source.) */
MyBoolean							UpdateOscStateBank(OscStateBankRec* State, long NumFrames,
												largefixedsigned* OutputData)
	{
		OscStateRec*				OneStateScan;
		MyBoolean						OscillatorsRunning;
		float								Frequency;

		CheckPtrExistence(State);
		ValidateOscStateBank(State);

		if (State->Release1Countdown >= 0)
			{
				if (State->Release1Countdown == 0)
					{
						OneStateScan = State->OscillatorList;
						while (OneStateScan != NIL)
							{
								(*OneStateScan->Template.KeyUpSustain1)(OneStateScan->StateReference);
								OneStateScan = OneStateScan->Next;
							}
						LFOGeneratorKeyUpSustain1(State->PitchLFO);
					}
				State->Release1Countdown -= 1;
			}

		if (State->Release2Countdown >= 0)
			{
				if (State->Release2Countdown == 0)
					{
						OneStateScan = State->OscillatorList;
						while (OneStateScan != NIL)
							{
								(*OneStateScan->Template.KeyUpSustain2)(OneStateScan->StateReference);
								OneStateScan = OneStateScan->Next;
							}
						LFOGeneratorKeyUpSustain2(State->PitchLFO);
					}
				State->Release2Countdown -= 1;
			}

		if (State->Release3Countdown >= 0)
			{
				if (State->Release3Countdown == 0)
					{
						OneStateScan = State->OscillatorList;
						while (OneStateScan != NIL)
							{
								(*OneStateScan->Template.KeyUpSustain3)(OneStateScan->StateReference);
								OneStateScan = OneStateScan->Next;
							}
						LFOGeneratorKeyUpSustain3(State->PitchLFO);
					}
				State->Release3Countdown -= 1;
			}

		/* perform portamento */
		if (State->PortamentoCounter > 0)
			{
				/* decrement is done before interpolation so that the final frequency */
				/* will actually be reached. */
				State->PortamentoCounter -= 1;
				if (State->PortamentoHertz)
					{
						/* this transition is linear, so it's easy to compute */
						/* L+F(R-L) */
						State->CurrentFrequency = State->InitialFrequency
							+ ((float)(State->TotalPortamentoTicks
							- State->PortamentoCounter) / State->TotalPortamentoTicks)
							* (State->FinalFrequency - State->InitialFrequency);
					}
				 else
					{
						/* this transition is log-linear, so it's a bit messier */
						State->CurrentFrequency = State->InitialFrequency * (float)FEXP(
							((float)(State->TotalPortamentoTicks - State->PortamentoCounter)
							/ State->TotalPortamentoTicks)
							* (((float)FLN(State->FinalFrequency) / (float)LOG2)
							- ((float)FLN(State->InitialFrequency) / (float)LOG2)) * LOG2);
					}
			}

		/* update the pitch LFO modulation & figure out what the current pitch is */
		if (State->PitchLFOStartCountdown > 0)
			{
				State->PitchLFOStartCountdown -= 1;
				Frequency = State->CurrentFrequency;
			}
		 else
			{
				/* do some pitch stuff */
				Frequency = FastFixed2Float(LFOGenUpdateCycle(State->PitchLFO,
					Double2FastFixed(State->CurrentFrequency)));
			}

		/* perform a cycle of resampling */
		OscillatorsRunning = False;
		OneStateScan = State->OscillatorList;
		while (OneStateScan != NIL)
			{
				(*OneStateScan->Template.StateNewFrequency)(OneStateScan->StateReference,Frequency);
				(*OneStateScan->Template.UpdateEnvelopes)(OneStateScan->StateReference);
				(*OneStateScan->Template.GenSamples)(OneStateScan->StateReference,NumFrames,OutputData);
				OscillatorsRunning = OscillatorsRunning || !(*OneStateScan->Template
					.IsItFinished)(OneStateScan->StateReference);
				OneStateScan = OneStateScan->Next;
			}

		return !OscillatorsRunning;
	}
