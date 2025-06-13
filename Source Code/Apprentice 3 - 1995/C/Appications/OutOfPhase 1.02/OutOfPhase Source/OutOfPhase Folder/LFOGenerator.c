/* LFOGenerator.c */
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

#include "LFOGenerator.h"
#include "Memory.h"
#include "EnvelopeState.h"
#include "LFOSpecifier.h"
#include "LFOListSpecifier.h"
#include "FloatingPoint.h"
#include "RandomNumbers.h"
#include "Frequency.h"
#include "Multisampler.h"
#include "64BitMath.h"
#include "SampleConsts.h"


#define TWOPI ((float)6.28318530717958648)


static long								RandomSeed = PARKANDMILLERMINIMUM;


typedef struct LFOOneStateRec
	{
		/* pointer to the LFO generating function */
		float										(*GenFunction)(struct LFOOneStateRec* State, float Phase,
															float OriginalValue, float Amplitude);

		/* phase index counter */
		FastFixedType						CurrentPhase;

		/* the envelope controlling the amplitude. */
		EvalEnvelopeRec*				LFOAmplitudeEnvelope;
		/* the envelope controlling the frequency.  the value from here is periods per second */
		EvalEnvelopeRec*				LFOFrequencyEnvelope;

		/* if this is True, then modulation is linear, otherwise modulation is */
		/* exponential */
		LFOArithSelect					ModulationMode;

		/* what kind of operator are we using */
		LFOOscTypes							Operator;

		/* source information for the wave table */
		MultiSampleRec*					WaveTableSourceSelector;

		/* this is true if the wave table was defined */
		MyBoolean								WaveTableWasDefined;
		/* number of frames per table */
		long										FramesPerTable;
		/* number of tables */
		long										NumberOfTables;
		/* raw wave table data array */
		void**									WaveTableMatrix;
		/* number of bits in the data for the table */
		NumBitsType							TableNumBits;
		/* envelope controlling wave table index */
		EvalEnvelopeRec*				WaveTableIndexEnvelope;

		/* modulation method */
		LFOModulationTypes			ModulationMethod;

		/* link to the next one */
		struct LFOOneStateRec*	Next;
	} LFOOneStateRec;


struct LFOGenRec
	{
		/* list of single LFO entries, which we sum up. */
		LFOOneStateRec*					LFOList;

		/* this is the correction factor from LFOFrequencyEnvelope which converts */
		/* it from periods per second into an interval we can add to CurrentPhase */
		FastFixedType						CorrectionFactor;

		/* number of envelope clock pulses per second */
		float										EnvelopeTicksPerSecond;

		/* link for caching records */
		LFOGenRec*							GarbageLink;
	};


/* prototypes */
static float						AddConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						AddWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						MultWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);
static float						InvMultWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude);


static LFOGenRec*					LFOGenFreeList = NIL;
static LFOOneStateRec*		LFOOneStateFreeList = NIL;


/* flush cached LFO generator records */
void								FlushLFOGeneratorRecords(void)
	{
		while (LFOGenFreeList != NIL)
			{
				LFOGenRec*				Temp;

				Temp = LFOGenFreeList;
				LFOGenFreeList = LFOGenFreeList->GarbageLink;
				ReleasePtr((char*)Temp);
			}

		while (LFOOneStateFreeList != NIL)
			{
				LFOOneStateRec*		Temp;

				Temp = LFOOneStateFreeList;
				LFOOneStateFreeList = LFOOneStateFreeList->Next;
				ReleasePtr((char*)Temp);
			}
	}


#if DEBUG
static void					ValidateLFOGen(LFOGenRec* LFOGen)
	{
		LFOGenRec*				Scan;

		Scan = LFOGenFreeList;
		while (Scan != NIL)
			{
				if (Scan == LFOGen)
					{
						PRERR(ForceAbort,"ValidateLFOGen:  it's on the free list");
					}
				Scan = Scan->GarbageLink;
			}
	}
#else
#define ValidateLFOGen(x) ((void)0)
#endif


#if DEBUG
static void					ValidateLFOOneState(LFOOneStateRec* OneState)
	{
		LFOOneStateRec*		Scan;

		Scan = LFOOneStateFreeList;
		while (Scan != NIL)
			{
				if (Scan == OneState)
					{
						PRERR(ForceAbort,"ValidateLFOOneState:  it's on the free list");
					}
				Scan = Scan->Next;
			}
	}
#else
#define ValidateLFOOneState(x) ((void)0)
#endif


/* create a new LFO generator based on a list of specifications */
/* it returns the largest pre-origin time for all of the envelopes it contains */
/* AmplitudeScaling and FrequencyScaling are provided primarily for frequency */
/* LFO control; other LFOs are controlled through the accent parameters, and */
/* should supply 1 (no scaling) for these parameters. */
LFOGenRec*					NewLFOGenerator(struct LFOListSpecRec* LFOListSpec,
											long* MaxPreOriginTime, float Accent1, float Accent2,
											float Accent3, float Accent4, float FrequencyHertz,
											float HurryUp, float TicksPerSecond, float AmplitudeScaling,
											float FrequencyScaling, LFOArithSelect ModulationMode,
											float FreqForMultisampling)
	{
		LFOGenRec*				LFOGen;
		long							ListLimit;
		long							ListScan;
		LFOOneStateRec*		ListTail;
		long							MaxPreOrigin;

		CheckPtrExistence(LFOListSpec);
		if (LFOGenFreeList != NIL)
			{
				LFOGen = LFOGenFreeList;
				LFOGenFreeList = LFOGenFreeList->GarbageLink;
			}
		 else
			{
				LFOGen = (LFOGenRec*)AllocPtrCanFail(sizeof(LFOGenRec),"LFOGenRec");
				if (LFOGen == NIL)
					{
						return NIL;
					}
			}

		/* calculate correction factor.  when this factor is multiplied by some */
		/* thing in periods/second, it converts it to periods/update */
		LFOGen->CorrectionFactor = Double2FastFixed(1 / TicksPerSecond);

		/* remember the envelope rate */
		LFOGen->EnvelopeTicksPerSecond = TicksPerSecond;

		/* build the list of thingers */
		LFOGen->LFOList = NIL;
		ListTail = NIL;
		ListLimit = LFOListSpecGetNumElements(LFOListSpec);
		MaxPreOrigin = 0;
		for (ListScan = 0; ListScan < ListLimit; ListScan += 1)
			{
				LFOSpecRec*				OneLFOSpec;
				LFOOneStateRec*		ListNode;
				long							PreOriginTime;

				/* allocate an entry */
				if (LFOOneStateFreeList != NIL)
					{
						ListNode = LFOOneStateFreeList;
						LFOOneStateFreeList = LFOOneStateFreeList->Next;
					}
				 else
					{
						ListNode = (LFOOneStateRec*)AllocPtrCanFail(sizeof(LFOOneStateRec),
							"LFOOneStateRec");
						if (ListNode == NIL)
							{
							 FailurePoint1:
								while (LFOGen->LFOList != NIL)
									{
										ListNode = LFOGen->LFOList;
										LFOGen->LFOList = LFOGen->LFOList->Next;
										DisposeEnvelopeStateRecord(ListNode->LFOAmplitudeEnvelope);
										DisposeEnvelopeStateRecord(ListNode->LFOFrequencyEnvelope);
										ListNode->Next = LFOOneStateFreeList;
										LFOOneStateFreeList = ListNode;
									}
								LFOGen->GarbageLink = LFOGenFreeList;
								LFOGenFreeList = LFOGen;
								return NIL;
							}
					}
				/* initialize phase index */
				ListNode->CurrentPhase = 0;
				/* get the data to put in it */
				OneLFOSpec = LFOListSpecGetLFOSpec(LFOListSpec,ListScan);
				/* Add frequency envelope generator */
				ListNode->LFOFrequencyEnvelope = NewEnvelopeStateRecord(
					GetLFOSpecFrequencyEnvelope(OneLFOSpec),Accent1,Accent2,Accent3,Accent4,
					FrequencyHertz,FrequencyScaling,HurryUp,TicksPerSecond,&PreOriginTime);
				if (ListNode->LFOFrequencyEnvelope == NIL)
					{
					 FailurePoint2:
						ListNode->Next = LFOOneStateFreeList;
						LFOOneStateFreeList = ListNode;
						goto FailurePoint1;
					}
				if (PreOriginTime > MaxPreOrigin)
					{
						MaxPreOrigin = PreOriginTime;
					}
				/* determine what mode to use and calculate amplitude */
				switch (ModulationMode)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation mode"));
							break;
						case eLFOArithAdditive:
							ListNode->ModulationMode = eLFOArithAdditive;
							break;
						case eLFOArithGeometric:
							ListNode->ModulationMode = eLFOArithGeometric;
							break;
						case eLFOArithDefault:
							switch (LFOSpecGetAddingMode(OneLFOSpec))
								{
									default:
										EXECUTE(PRERR(ForceAbort,
											"NewLFOGenerator:  bad value from LFOSpecGetAddingMode"));
										break;
									case eLFOArithmetic:
										ListNode->ModulationMode = eLFOArithAdditive;
										break;
									case eLFOGeometric:
										ListNode->ModulationMode = eLFOArithGeometric;
										break;
								}
							break;
					}
				/* add the amplitude envelope generator */
				ListNode->LFOAmplitudeEnvelope = NewEnvelopeStateRecord(
					GetLFOSpecAmplitudeEnvelope(OneLFOSpec),Accent1,Accent2,Accent3,Accent4,
					FrequencyHertz,AmplitudeScaling,HurryUp,TicksPerSecond,&PreOriginTime);
				if (ListNode->LFOAmplitudeEnvelope == NIL)
					{
					 FailurePoint3:
						DisposeEnvelopeStateRecord(ListNode->LFOFrequencyEnvelope);
						goto FailurePoint2;
					}
				if (PreOriginTime > MaxPreOrigin)
					{
						MaxPreOrigin = PreOriginTime;
					}
				/* determine what function to use */
				ListNode->Operator = LFOSpecGetOscillatorType(OneLFOSpec);
				ListNode->ModulationMethod = LFOSpecGetModulationMode(OneLFOSpec);
				switch (ListNode->Operator)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad oscillator type"));
							break;
						case eLFOConstant1:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddConst;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultConst;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultConst;
										break;
								}
							break;
						case eLFOSignedSine:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddSignSine;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultSignSine;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultSignSine;
										break;
								}
							break;
						case eLFOPositiveSine:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddPosSine;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultPosSine;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultPosSine;
										break;
								}
							break;
						case eLFOSignedTriangle:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddSignTriangle;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultSignTriangle;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultSignTriangle;
										break;
								}
							break;
						case eLFOPositiveTriangle:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddPosTriangle;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultPosTriangle;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultPosTriangle;
										break;
								}
							break;
						case eLFOSignedSquare:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddSignSquare;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultSignSquare;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultSignSquare;
										break;
								}
							break;
						case eLFOPositiveSquare:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddPosSquare;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultPosSquare;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultPosSquare;
										break;
								}
							break;
						case eLFOSignedRamp:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddSignRamp;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultSignRamp;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultSignRamp;
										break;
								}
							break;
						case eLFOPositiveRamp:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddPosRamp;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultPosRamp;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultPosRamp;
										break;
								}
							break;
						case eLFOSignedLinearFuzz:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddSignFuzz;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultSignFuzz;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultSignFuzz;
										break;
								}
							break;
						case eLFOPositiveLinearFuzz:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddPosFuzz;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultPosFuzz;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultPosFuzz;
										break;
								}
							break;
						case eLFOWaveTable:
							switch (ListNode->ModulationMethod)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"NewLFOGenerator:  bad modulation type"));
										break;
									case eLFOAdditive:
										ListNode->GenFunction = &AddWaveTable;
										break;
									case eLFOMultiplicative:
										ListNode->GenFunction = &MultWaveTable;
										break;
									case eLFOInverseMultiplicative:
										ListNode->GenFunction = &InvMultWaveTable;
										break;
								}
							ListNode->WaveTableSourceSelector = NewMultisampleWaveTable(
								GetLFOSpecSampleSelector(OneLFOSpec));
							if (ListNode->WaveTableSourceSelector == NIL)
								{
								 FailurePoint4:
									if (ListNode->Operator == eLFOWaveTable)
										{
											DisposeEnvelopeStateRecord(ListNode->LFOAmplitudeEnvelope);
										}
									goto FailurePoint3;
								}
							ListNode->WaveTableWasDefined = GetMultisampleReferenceWaveTable(
								ListNode->WaveTableSourceSelector,FreqForMultisampling,
								&(ListNode->WaveTableMatrix),&(ListNode->FramesPerTable),
								&(ListNode->NumberOfTables),&(ListNode->TableNumBits));
							ListNode->WaveTableIndexEnvelope = NewEnvelopeStateRecord(
								GetLFOSpecWaveTableIndexEnvelope(OneLFOSpec),Accent1,Accent2,Accent3,
								Accent4,FrequencyHertz,AmplitudeScaling,HurryUp,TicksPerSecond,
								&PreOriginTime);
							if (ListNode->WaveTableIndexEnvelope == NIL)
								{
								 FailurePoint5:
									if (ListNode->Operator == eLFOWaveTable)
										{
											DisposeMultisample(ListNode->WaveTableSourceSelector);
										}
									goto FailurePoint4;
								}
							if (PreOriginTime > MaxPreOrigin)
								{
									MaxPreOrigin = PreOriginTime;
								}
							break;
					}
				/* link it in */
				ListNode->Next = NIL;
				if (ListTail != NIL)
					{
						ListTail->Next = ListNode;
					}
				 else
					{
						LFOGen->LFOList = ListNode;
					}
				ListTail = ListNode;
			}

		*MaxPreOriginTime = MaxPreOrigin;
		return LFOGen;
	}


/* fix up the origin time so that envelopes start at the proper times */
void								LFOGeneratorFixEnvelopeOrigins(LFOGenRec* LFOGen,
											long ActualPreOriginTime)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		while (Scan != NIL)
			{
				ValidateLFOOneState(Scan);
				EnvelopeStateFixUpInitialDelay(Scan->LFOAmplitudeEnvelope,ActualPreOriginTime);
				EnvelopeStateFixUpInitialDelay(Scan->LFOFrequencyEnvelope,ActualPreOriginTime);
				if (Scan->Operator == eLFOWaveTable)
					{
						EnvelopeStateFixUpInitialDelay(Scan->WaveTableIndexEnvelope,
							ActualPreOriginTime);
					}
				Scan = Scan->Next;
			}
	}


/* this function computes one LFO cycle and returns the value from the LFO generator. */
/* it should be called on the envelope clock. */
FastFixedType				LFOGenUpdateCycle(LFOGenRec* LFOGen, FastFixedType OriginalValue)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		if (Scan != NIL)
			{
				float							Iterator;

				Iterator = FastFixed2Float(OriginalValue);
				while (Scan != NIL)
					{
						/* perform the calculations */
						ERROR((Scan->ModulationMode != eLFOArithAdditive)
							&& (Scan->ModulationMode != eLFOArithGeometric),PRERR(AllowResume,
							"LFOGenUpdateCycle:  bad value for ModulationMode"));
						if (Scan->ModulationMode == eLFOArithAdditive)
							{
								Iterator = (*Scan->GenFunction)(Scan,FastFixed2Float(Scan->CurrentPhase),
									Iterator,FastFixed2Float(EnvelopeUpdate(Scan->LFOAmplitudeEnvelope)));
							}
						 else
							{
								MyBoolean					Sign;

								Sign = (Iterator < 0);
								if (Sign)
									{
										Iterator = - Iterator;
									}
								if (Iterator != 0)
									{
										/* the LOG2 is to normalize the values, so that 1/12 will */
										/* be 1 halfstep */
										Iterator = (float)FEXP((*Scan->GenFunction)(Scan,
											FastFixed2Float(Scan->CurrentPhase),(float)FLN(Iterator),
											FastFixed2Float(EnvelopeUpdate(
											Scan->LFOAmplitudeEnvelope)) * (float)LOG2));
									}
								if (Sign)
									{
										Iterator = - Iterator;
									}
							}
						Scan->CurrentPhase = (Scan->CurrentPhase
							+ FastFixedTimesFastFixedToFastFixed(LFOGen->CorrectionFactor,
							EnvelopeUpdate(Scan->LFOFrequencyEnvelope)))
							& ((1L << FASTFIXEDPRECISION) - 1);
						/* go to next one */
						Scan = Scan->Next;
					}
				return Double2FastFixed(Iterator);
			}
		 else
			{
				return OriginalValue;
			}
	}


/* pass the key-up impulse on to the envelopes contained inside */
void								LFOGeneratorKeyUpSustain1(LFOGenRec* LFOGen)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		while (Scan != NIL)
			{
				EnvelopeKeyUpSustain1(Scan->LFOAmplitudeEnvelope);
				EnvelopeKeyUpSustain1(Scan->LFOFrequencyEnvelope);
				if (Scan->Operator == eLFOWaveTable)
					{
						EnvelopeKeyUpSustain1(Scan->WaveTableIndexEnvelope);
					}
				Scan = Scan->Next;
			}
	}


/* pass the key-up impulse on to the envelopes contained inside */
void								LFOGeneratorKeyUpSustain2(LFOGenRec* LFOGen)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		while (Scan != NIL)
			{
				EnvelopeKeyUpSustain2(Scan->LFOAmplitudeEnvelope);
				EnvelopeKeyUpSustain2(Scan->LFOFrequencyEnvelope);
				if (Scan->Operator == eLFOWaveTable)
					{
						EnvelopeKeyUpSustain2(Scan->WaveTableIndexEnvelope);
					}
				Scan = Scan->Next;
			}
	}


/* pass the key-up impulse on to the envelopes contained inside */
void								LFOGeneratorKeyUpSustain3(LFOGenRec* LFOGen)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		while (Scan != NIL)
			{
				EnvelopeKeyUpSustain3(Scan->LFOAmplitudeEnvelope);
				EnvelopeKeyUpSustain3(Scan->LFOFrequencyEnvelope);
				if (Scan->Operator == eLFOWaveTable)
					{
						EnvelopeKeyUpSustain3(Scan->WaveTableIndexEnvelope);
					}
				Scan = Scan->Next;
			}
	}


/* retrigger envelopes from the origin point */
void								LFOGeneratorRetriggerFromOrigin(LFOGenRec* LFOGen, float Accent1,
											float Accent2, float Accent3, float Accent4, float FrequencyHertz,
											float HurryUp, float TicksPerSecond, float AmplitudeScaling,
											float FrequencyScaling, MyBoolean ActuallyRetrigger)
	{
		LFOOneStateRec*		Scan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		Scan = LFOGen->LFOList;
		while (Scan != NIL)
			{
				EnvelopeRetriggerFromOrigin(Scan->LFOAmplitudeEnvelope,Accent1,Accent2,
					Accent3,Accent4,FrequencyHertz,AmplitudeScaling,HurryUp,
					LFOGen->EnvelopeTicksPerSecond,ActuallyRetrigger);
				EnvelopeRetriggerFromOrigin(Scan->LFOFrequencyEnvelope,Accent1,Accent2,
					Accent3,Accent4,FrequencyHertz,FrequencyScaling,HurryUp,
					LFOGen->EnvelopeTicksPerSecond,ActuallyRetrigger);
				if (Scan->Operator == eLFOWaveTable)
					{
						EnvelopeRetriggerFromOrigin(Scan->WaveTableIndexEnvelope,Accent1,Accent2,
							Accent3,Accent4,FrequencyHertz,FrequencyScaling,HurryUp,
							LFOGen->EnvelopeTicksPerSecond,ActuallyRetrigger);
					}
				Scan = Scan->Next;
			}
	}


/* dispose the LFO generator */
void								DisposeLFOGenerator(LFOGenRec* LFOGen)
	{
		LFOOneStateRec*		StateScan;

		CheckPtrExistence(LFOGen);
		ValidateLFOGen(LFOGen);

		StateScan = LFOGen->LFOList;

		/* dispose of the master record */
		LFOGen->GarbageLink = LFOGenFreeList;
		LFOGenFreeList = LFOGen;

		/* dispose of each element */
		while (StateScan != NIL)
			{
				LFOOneStateRec*		Temp;

				ValidateLFOOneState(StateScan);
				if (StateScan->Operator == eLFOWaveTable)
					{
						DisposeMultisample(StateScan->WaveTableSourceSelector);
						DisposeEnvelopeStateRecord(StateScan->WaveTableIndexEnvelope);
					}
				DisposeEnvelopeStateRecord(StateScan->LFOAmplitudeEnvelope);
				DisposeEnvelopeStateRecord(StateScan->LFOFrequencyEnvelope);
				Temp = StateScan;
				StateScan = StateScan->Next;
				Temp->Next = LFOOneStateFreeList;
				LFOOneStateFreeList = Temp;
			}
	}


static float						AddConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue + Amplitude;
	}


static float						AddSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue + Amplitude * FSIN(Phase * TWOPI);
	}


static float						AddPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue + Amplitude * 0.5 * (1 + FSIN(Phase * TWOPI));
	}


static float						AddSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue + Phase * 4 * Amplitude;
	}


static float						AddPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue + (Phase + 1) * 2 * Amplitude;
	}


static float						AddSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue + Amplitude;
			}
		 else
			{
				return OriginalValue - Amplitude;
			}
	}


static float						AddPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue + Amplitude;
			}
		 else
			{
				return OriginalValue;
			}
	}


static float						AddSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue + Amplitude * (2 * Phase - 1);
	}


static float						AddPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue + Amplitude * Phase;
	}


static float						AddSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = ((((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * 2) - 1) * Amplitude
			+ OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static float						AddPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = (((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * Amplitude)
			+ OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static float						MultConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * Amplitude;
	}


static float						MultSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * Amplitude * FSIN(Phase * TWOPI);
	}


static float						MultPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * Amplitude * 0.5 * (1 + FSIN(Phase * TWOPI));
	}


static float						MultSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue * Phase * 4 * Amplitude;
	}


static float						MultPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue * (Phase + 1) * 2 * Amplitude;
	}


static float						MultSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue * Amplitude;
			}
		 else
			{
				return OriginalValue * - Amplitude;
			}
	}


static float						MultPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue * Amplitude;
			}
		 else
			{
				return 0;
			}
	}


static float						MultSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * Amplitude * (2 * Phase - 1);
	}


static float						MultPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * Amplitude * Phase;
	}


static float						MultSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = ((((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * 2) - 1) * Amplitude
			* OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static float						MultPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = (((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * Amplitude)
			* OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static float						InvMultConst(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * (1 - Amplitude);
	}


static float						InvMultSignSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * (1 - Amplitude * FSIN(Phase * TWOPI));
	}


static float						InvMultPosSine(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * (1 - Amplitude * 0.5 * (1 + FSIN(Phase * TWOPI)));
	}


static float						InvMultSignTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue * (1 - Phase * 4 * Amplitude);
	}


static float						InvMultPosTriangle(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.25)
			{
			}
		else if (Phase < 0.75)
			{
				Phase = 0.5 - Phase;
			}
		else
			{
				Phase = Phase - 1;
			}
		return OriginalValue * (1 - ((Phase + 1) * 2 * Amplitude));
	}


static float						InvMultSignSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue * (1 - Amplitude);
			}
		 else
			{
				return OriginalValue * (1 + Amplitude);
			}
	}


static float						InvMultPosSquare(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (Phase < 0.5)
			{
				return OriginalValue * (1 - Amplitude);
			}
		 else
			{
				return OriginalValue;
			}
	}


static float						InvMultSignRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * (1 - Amplitude * (2 * Phase - 1));
	}


static float						InvMultPosRamp(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		return OriginalValue * (1 - Amplitude * Phase);
	}


static float						InvMultSignFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = (1 - ((((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * 2) - 1) * Amplitude)
			* OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static float						InvMultPosFuzz(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		long									OldSeed;
		float									ReturnValue;

		OldSeed = SetParkAndMillerRandomSeed(RandomSeed);
		ReturnValue = (1 - (((float)ParkAndMillerRandom() - PARKANDMILLERMINIMUM)
			/ (PARKANDMILLERMAXIMUM - PARKANDMILLERMINIMUM - 1) * Amplitude))
			* OriginalValue;
		RandomSeed = SetParkAndMillerRandomSeed(OldSeed);
		return ReturnValue;
	}


static FastFixedType		CalcWaveTableValue(LFOOneStateRec* State, float Phase)
	{
		FastFixedType					Index;
		LongLongRec						FrameIndex;
		signed long						Final; /* both 16-bit int & largefixedsigned */

		ERROR(!State->WaveTableWasDefined,PRERR(ForceAbort,
			"CalcWaveTableValue:  no wave table"));

		Index = EnvelopeUpdate(State->WaveTableIndexEnvelope);
		if (Index < 0)
			{
				Index = 0;
			}
		else if (Index > Int2FastFixed(State->NumberOfTables - 1))
			{
				Index = Int2FastFixed(State->NumberOfTables - 1);
			}
		Double2LongLong(Phase,FrameIndex);
		if (State->TableNumBits == eSample8bit)
			{
				/* 8-bit */
				if (FastFixed2Int(Index) == State->NumberOfTables - 1)
					{
						/* this is done in case the wave table index is at the maximum, */
						/* in which case there is no table+1 to interpolate with. */
						signed char*				WaveData;

						FastFixedType				LeftWeight;
						long								ArraySubscript;
						signed long					LeftValue;
						signed long					RightValue;

						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index)]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index)]));
						WaveData = (signed char*)(State->WaveTableMatrix[FastFixed2Int(Index)]);

						LeftWeight = LongLongLowHalf(FrameIndex) >> (32 - FASTFIXEDPRECISION);
						ArraySubscript = LongLongHighHalf(FrameIndex) & (State->FramesPerTable - 1);
						/* L+F(R-L) */
						LeftValue = ((signed long)WaveData[ArraySubscript]) << 8; /* convert to 16-bit */
						RightValue = ((signed long)WaveData[ArraySubscript + 1]) << 8; /* to 16-bit */
						Final = LeftValue + ((LeftWeight * (RightValue - LeftValue)) >> 15);
					}
				 else
					{
						signed char*				WaveData0;
						signed char*				WaveData1;
						FastFixedType				Wave0Weight;

						FastFixedType				LeftWeight;
						long								ArraySubscript;
						signed long					Left0Value;
						signed long					Right0Value;
						signed long					Left1Value;
						signed long					Right1Value;
						FastFixedType				Wave0Temp;

						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index)]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index)]));
						WaveData0 = (signed char*)(State->WaveTableMatrix[
							FastFixed2Int(Index)]);
						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index) + 1]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index) + 1]));
						WaveData1 = (signed char*)(State->WaveTableMatrix[
							FastFixed2Int(Index) + 1]);
						Wave0Weight = Index & FASTFIXEDFRACTMASK;

						LeftWeight = LongLongLowHalf(FrameIndex) >> (32 - FASTFIXEDPRECISION);
						ArraySubscript = LongLongHighHalf(FrameIndex) & (State->FramesPerTable - 1);
						/* L+F(R-L) -- applied twice */
						Left0Value = ((signed long)WaveData0[ArraySubscript]) << 8; /* convert to 16-bit */
						Right0Value = ((signed long)WaveData0[ArraySubscript + 1]) << 8; /* to 16-bit */
						Left1Value = ((signed long)WaveData1[ArraySubscript]) << 8; /* convert to 16-bit */
						Right1Value = ((signed long)WaveData1[ArraySubscript + 1]) << 8; /* to 16-bit */
						Wave0Temp = Left0Value + ((LeftWeight * (Right0Value - Left0Value)) >> 15);
						Final = Wave0Temp + ((Wave0Weight * (Left1Value + ((LeftWeight
							* (Right1Value - Left1Value)) >> 15) - Wave0Temp)) >> 15);
					}
			}
		 else
			{
				/* 16-bit */
				if (FastFixed2Int(Index) == State->NumberOfTables - 1)
					{
						/* this is done in case the wave table index is at the maximum, */
						/* in which case there is no table+1 to interpolate with. */
						signed short*				WaveData;

						FastFixedType				LeftWeight;
						long								ArraySubscript;
						signed long					LeftValue;
						signed long					RightValue;

						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index)]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index)]));
						WaveData = (signed short*)(State->WaveTableMatrix[FastFixed2Int(Index)]);

						LeftWeight = LongLongLowHalf(FrameIndex) >> (32 - FASTFIXEDPRECISION);
						ArraySubscript = LongLongHighHalf(FrameIndex) & (State->FramesPerTable - 1);
						/* L+F(R-L) */
						LeftValue = WaveData[ArraySubscript];
						RightValue = WaveData[ArraySubscript + 1];
						Final = LeftValue + ((LeftWeight * (RightValue - LeftValue)) >> 15);
					}
				 else
					{
						signed short*				WaveData0;
						signed short*				WaveData1;
						FastFixedType				Wave0Weight;

						FastFixedType				LeftWeight;
						long								ArraySubscript;
						signed long					Left0Value;
						signed long					Right0Value;
						signed long					Left1Value;
						signed long					Right1Value;
						FastFixedType				Wave0Temp;

						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index)]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index)]));
						WaveData0 = (signed short*)(State->WaveTableMatrix[
							FastFixed2Int(Index)]);
						PRNGCHK(State->WaveTableMatrix,&(State->WaveTableMatrix[FastFixed2Int(
							Index) + 1]),sizeof(State->WaveTableMatrix[FastFixed2Int(Index) + 1]));
						WaveData1 = (signed short*)(State->WaveTableMatrix[
							FastFixed2Int(Index) + 1]);
						Wave0Weight = Index & FASTFIXEDFRACTMASK;

						LeftWeight = LongLongLowHalf(FrameIndex) >> (32 - FASTFIXEDPRECISION);
						ArraySubscript = LongLongHighHalf(FrameIndex) & (State->FramesPerTable - 1);
						/* L+F(R-L) -- applied twice */
						Left0Value = WaveData0[ArraySubscript];
						Right0Value = WaveData0[ArraySubscript + 1];
						Left1Value = WaveData1[ArraySubscript];
						Right1Value = WaveData1[ArraySubscript + 1];
						Wave0Temp = Left0Value + ((LeftWeight * (Right0Value - Left0Value)) >> 15);
						Final = Wave0Temp + ((Wave0Weight * (Left1Value + ((LeftWeight
							* (Right1Value - Left1Value)) >> 15)) - Wave0Temp) >> 15);
					}
			}
		return Final;
	}


static float						AddWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (State->WaveTableWasDefined)
			{
				return OriginalValue + Amplitude
					* ((float)CalcWaveTableValue(State,Phase) / MAX16BIT);
			}
		 else
			{
				return OriginalValue;
			}
	}


static float						MultWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (State->WaveTableWasDefined)
			{
				return OriginalValue * Amplitude
					* ((float)CalcWaveTableValue(State,Phase) / MAX16BIT);
			}
		 else
			{
				return 0;
			}
	}


static float						InvMultWaveTable(LFOOneStateRec* State, float Phase,
													float OriginalValue, float Amplitude)
	{
		if (State->WaveTableWasDefined)
			{
				return OriginalValue * (1 - Amplitude
					* ((float)CalcWaveTableValue(State,Phase) / MAX16BIT));
			}
		 else
			{
				return OriginalValue;
			}
	}
