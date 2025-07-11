/* EnvelopeState.c */
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

#define ShowMeEnvelopeRec
#include "EnvelopeState.h"
#include "FastFixedPoint.h"
#include "Envelope.h"
#include "Memory.h"
#include "LinearTransition.h"
#include "Frequency.h"
#include "FloatingPoint.h"


/* local prototypes */
static void						EnvStepToNextInterval(EvalEnvelopeRec* State);
static FastFixedType	EnvUpdateLinearAbsolute(EvalEnvelopeRec* State);
static FastFixedType	EnvUpdateLinearDecibels(EvalEnvelopeRec* State);
static FastFixedType	EnvUpdateSustain(EvalEnvelopeRec* State);


typedef struct OneEnvPhaseRec
	{
		/* how many envelope cycles does this phase last */
		long											Duration;
		/* what amplitude are we trying to attain */
		FastFixedType							FinalAmplitude;
		/* what does the curve look like */
		EnvTransTypes							TransitionType;
		/* way of deriving the target */
		EnvTargetTypes						TargetType;

		/* pointer for making linked lists */
		struct OneEnvPhaseRec*		PhaseLink;
	} OneEnvPhaseRec;


struct EvalEnvelopeRec
	{
		/* number of envelope phases. 0 phases means the envelope produces constant value */
		long											NumPhases;
		/* list of definitions for each transition phase.  when this goes NIL, then */
		/* we are done. */
		OneEnvPhaseRec*						CurrentPhaseRecord;
		/* this remembers the first phase */
		OneEnvPhaseRec*						PhaseListHead;

		/* phase at which first sustain occurs. (released by key-up) */
		/* if the value of this is N, then sustain will occur after phase N has */
		/* completed.  if it is 0, then sustain will occur after phase 0 has completed. */
		/* if it is -1, then sustain will not occur.  sustain may not be NumPhases since */
		/* that would be the end of envelope and would be the same as final value hold. */
		long											SustainPhase1;
		long											OriginalSustainPhase1;
		SustainTypes							SustainPhase1Type;
		/* phase at which second sustain occurs. */
		long											SustainPhase2;
		long											OriginalSustainPhase2;
		SustainTypes							SustainPhase2Type;
		/* phase at which note-end sustain occurs */
		long											SustainPhase3;
		long											OriginalSustainPhase3;
		SustainTypes							SustainPhase3Type;

		/* what output phase are we generating right now.  this is the phase index of */
		/* the record currently in CurrentPhaseRecord. */
		long											Phase;

		/* what is the origin phase */
		long											Origin;

		/* this is the envelope transition generator */
		LinearTransRec*						LinearTransition;
		/* this is the countdown for this linear transition.  this is NOT used for */
		/* linear-decibel transitions.  instead, we hack it ourselves. */
		long											LinearTransitionCounter;
		/* stuff needed for linear-decibel transitions */
		long											LinearTransitionTotalDuration;
		float											InitialDecibels;
		float											FinalDecibels;
		/* hold value for when we are sustaining */
		FastFixedType							LastOutputtedValue;

		/* number of cycles of the envelope that occur before the origin */
		long											PreOriginTime;

		/* this function performs one update cycle */
		FastFixedType							(*EnvelopeUpdate)(EvalEnvelopeRec* State);

		/* flag indicating that envelope has finished evaluating the last phase */
		MyBoolean									EnvelopeHasFinished;

		/* we remember the template that was used to construct us */
		EnvelopeRec*							Template;

		/* pointer for maintaining the free list */
		EvalEnvelopeRec*					GarbageLink;
	};


static EvalEnvelopeRec*			EnvelopeStateFreeList = NIL;
static OneEnvPhaseRec*			EnvelopeEntryFreeList = NIL;


/* flush cached envelope state records */
void								FlushEvalEnvelopeStateRecords(void)
	{
		while (EnvelopeStateFreeList != NIL)
			{
				EvalEnvelopeRec*		Temp;

				Temp = EnvelopeStateFreeList;
				EnvelopeStateFreeList = EnvelopeStateFreeList->GarbageLink;
				ReleasePtr((char*)Temp);
			}

		while (EnvelopeEntryFreeList != NIL)
			{
				OneEnvPhaseRec*			Temp;

				Temp = EnvelopeEntryFreeList;
				EnvelopeEntryFreeList = EnvelopeEntryFreeList->PhaseLink;
				ReleasePtr((char*)Temp);
			}
	}


#if DEBUG
static void					DebugCheckState(EvalEnvelopeRec* State)
	{
		EvalEnvelopeRec*	Scan;

		Scan = EnvelopeStateFreeList;
		while (Scan != NIL)
			{
				if (Scan == State)
					{
						PRERR(ForceAbort,"DebugCheckState:  released envelope record passed in");
					}
				Scan = Scan->GarbageLink;
			}
	}
#else
#define DebugCheckState(x) ((void)0)
#endif


#if DEBUG
static void					DebugCheckPhase(OneEnvPhaseRec* Phase)
	{
		OneEnvPhaseRec*		Scan;

		Scan = EnvelopeEntryFreeList;
		while (Scan != NIL)
			{
				if (Scan == Phase)
					{
						PRERR(ForceAbort,"DebugCheckPhase:  released envelope phase passed in");
					}
				Scan = Scan->PhaseLink;
			}
	}
#else
#define DebugCheckPhase(x) ((void)0)
#endif


/* dispose of an envelope state record */
void								DisposeEnvelopeStateRecord(EvalEnvelopeRec* State)
	{
		OneEnvPhaseRec*		PhaseScan;

		CheckPtrExistence(State);
		DebugCheckState(State);

		DisposeLinearTransition(State->LinearTransition);
		PhaseScan = State->PhaseListHead;
		while (PhaseScan != NIL)
			{
				OneEnvPhaseRec*			Temp;

				Temp = PhaseScan;
				DebugCheckPhase(Temp);
				PhaseScan = PhaseScan->PhaseLink;
				Temp->PhaseLink = EnvelopeEntryFreeList;
				EnvelopeEntryFreeList = Temp;
			}
		State->GarbageLink = EnvelopeStateFreeList;
		EnvelopeStateFreeList = State;
	}


/* create a new envelope state record.  Accent factors have no effect with a value */
/* of 1, attenuate at smaller values, and amplify at larger values. */
EvalEnvelopeRec*		NewEnvelopeStateRecord(struct EnvelopeRec* Template,
											float Accent1, float Accent2, float Accent3, float Accent4,
											float FrequencyHertz, float Loudness, float HurryUp,
											float TicksPerSecond, long* PreOriginTime)
	{
		EvalEnvelopeRec*	State;
		long							Scan;
		OneEnvPhaseRec*		PhaseTail;

		CheckPtrExistence(Template);
		/* we can only handle it if the envelope definition's internal values are floats */
		/* this generates a type error if they aren't */
		EXECUTE((void)((EnvNumberType*)NIL != (float*)NIL);)

		/* allocate state record */
		if (EnvelopeStateFreeList != NIL)
			{
				State = EnvelopeStateFreeList;
				EnvelopeStateFreeList = EnvelopeStateFreeList->GarbageLink;
			}
		 else
			{
				State = (EvalEnvelopeRec*)AllocPtrCanFail(sizeof(EvalEnvelopeRec),
					"EvalEnvelopeRec");
				if (State == NIL)
					{
						return NIL;
					}
			}

		/* fill in the fields */
		State->NumPhases = Template->NumPhases;
		State->SustainPhase1 = Template->SustainPhase1;
		State->OriginalSustainPhase1 = Template->SustainPhase1;
		State->SustainPhase1Type = Template->SustainPhase1Type;
		State->SustainPhase2 = Template->SustainPhase2;
		State->OriginalSustainPhase2 = Template->SustainPhase2;
		State->SustainPhase2Type = Template->SustainPhase2Type;
		State->SustainPhase3 = Template->SustainPhase3;
		State->OriginalSustainPhase3 = Template->SustainPhase3;
		State->SustainPhase3Type = Template->SustainPhase3Type;
		State->Phase = -1;
		State->Origin = Template->Origin;

		/* build initial delay transition */
		State->LinearTransition = NewLinearTransition(0,0,1);
		if (State->LinearTransition == NIL)
			{
			 FailurePoint1:
				State->GarbageLink = EnvelopeStateFreeList;
				EnvelopeStateFreeList = State;
				return NIL;
			}
		State->LinearTransitionCounter = 0;
		State->LinearTransitionTotalDuration = 0;
		State->EnvelopeUpdate = &EnvUpdateLinearAbsolute;
		State->LastOutputtedValue = 0;
		State->EnvelopeHasFinished = False;

		State->Template = Template;

		/* build list of nodes */
		PhaseTail = NIL;
		State->CurrentPhaseRecord = NIL;
		State->PhaseListHead = NIL;
		State->PreOriginTime = 0;
		for (Scan = 0; Scan < State->NumPhases; Scan += 1)
			{
				OneEnvPhaseRec*			Phase;

				/* allocate phase record */
				if (EnvelopeEntryFreeList != NIL)
					{
						Phase = EnvelopeEntryFreeList;
						EnvelopeEntryFreeList = EnvelopeEntryFreeList->PhaseLink;
					}
				 else
					{
						Phase = (OneEnvPhaseRec*)AllocPtrCanFail(sizeof(OneEnvPhaseRec),
							"OneEnvPhaseRec");
						if (Phase == NIL)
							{
							 FailurePoint2:
								while (State->PhaseListHead != NIL)
									{
										Phase = State->PhaseListHead;
										State->PhaseListHead = State->PhaseListHead->PhaseLink;
										Phase->PhaseLink = EnvelopeEntryFreeList;
										EnvelopeEntryFreeList = Phase;
									}
								goto FailurePoint1;
							}
					}

				/* fill in phase record parameters */
				PRNGCHK(Template->PhaseArray,&(Template->PhaseArray[Scan]),
					sizeof(Template->PhaseArray[Scan]));
				Phase->TransitionType = Template->PhaseArray[Scan].TransitionType;
				Phase->TargetType = Template->PhaseArray[Scan].TargetType;
				/* calculate the total duration.  the effect of accents is this: */
				/*  - the accent is the base-2 log of a multiplier for the rate.  a value of 0 */
				/*    does not change the rate.  -1 halves the rate, and 1 doubles the rate. */
				/*  - the accent scaling factor is the base-2 log for scaling the accent. */
				/*    a value of 0 eliminates the effect of the accent, a value of 1 does not */
				/*    scale the accent. */
				/*  - pitch has two factors:  normalization point and rolloff.  rolloff */
				/*    determines how much the signal will decrease with each octave.  0 */
				/*    removes effect, 1 halfs signal with each octave.  normalization point */
				/*    determines what pitch will be the invariant point. */
				Phase->Duration = TicksPerSecond * HurryUp * Template->PhaseArray[Scan].Duration
					* FPOWER(2, - (Accent1 * Template->PhaseArray[Scan].Accent1Rate
						+ Accent2 * Template->PhaseArray[Scan].Accent2Rate
						+ Accent3 * Template->PhaseArray[Scan].Accent3Rate
						+ Accent4 * Template->PhaseArray[Scan].Accent4Rate
						+ (FLN(FrequencyHertz / Template->PhaseArray[Scan].FrequencyRateNormalization)
						/ (float)LOG2) * Template->PhaseArray[Scan].FrequencyRateRolloff));
				/* the final amplitude scaling values are computed similarly to the rate */
				/* scaling values. */
				Phase->FinalAmplitude = Double2FastFixed(Template->PhaseArray[Scan].EndPoint
					* Template->OverallScalingFactor * Loudness
					* FPOWER(2, - (Accent1 * Template->PhaseArray[Scan].Accent1Amp
						+ Accent2 * Template->PhaseArray[Scan].Accent2Amp
						+ Accent3 * Template->PhaseArray[Scan].Accent3Amp
						+ Accent4 * Template->PhaseArray[Scan].Accent4Amp
						+ (FLN(FrequencyHertz / Template->PhaseArray[Scan].FrequencyAmpNormalization)
						/ (float)LOG2) * Template->PhaseArray[Scan].FrequencyAmpRolloff)));

				/* append to the list */
				Phase->PhaseLink = NIL;
				if (PhaseTail != NIL)
					{
						PhaseTail->PhaseLink = Phase;
					}
				 else
					{
						State->CurrentPhaseRecord = Phase;
						State->PhaseListHead = Phase;
					}
				PhaseTail = Phase;

				/* adjust initial countdown */
				if (Scan < Template->Origin)
					{
						/* this occurs before the origin, so add it in */
						State->PreOriginTime += Phase->Duration;
					}
			}
		*PreOriginTime = State->PreOriginTime;

		return State;
	}


/* when all envelopes have been computed, then the total (i.e. largest) pre-origin */
/* time will be known and we can tell all envelopes how long they must wait */
/* before starting */
void								EnvelopeStateFixUpInitialDelay(EvalEnvelopeRec* State,
											long MaximumPreOriginTime)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);

		State->LinearTransitionCounter = MaximumPreOriginTime - State->PreOriginTime;
		State->LinearTransitionTotalDuration = MaximumPreOriginTime - State->PreOriginTime;
	}


/* perform a single cycle of the envelope and return the amplitude for it's */
/* point.  should be called at key-down to obtain initial amplitude. */
FastFixedType				EnvelopeUpdate(EvalEnvelopeRec* State)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);
		return (*State->EnvelopeUpdate)(State);
	}


/* find out if envelope has reached the end */
MyBoolean						IsEnvelopeAtEnd(EvalEnvelopeRec* State)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);
		return State->EnvelopeHasFinished;
	}


/* create key-up impulse.  call this before calling EnvelopeUpdate during a */
/* given cycle.  this call preserves the current level of the envelope but */
/* skips to the phase after the particular sustain. */
void								EnvelopeKeyUpSustain1(EvalEnvelopeRec* State)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);

		if (State->Phase <= State->SustainPhase1)
			{
				/* find out if we should skip ahead to the sustain point */
				if ((State->SustainPhase1Type == eEnvelopeSustainPointSkip)
					|| (State->SustainPhase1Type == eEnvelopeReleasePointSkip))
					{
						while ((State->CurrentPhaseRecord != NIL)
							&& (State->Phase < State->SustainPhase1))
							{
								State->Phase += 1;
								State->CurrentPhaseRecord = State->CurrentPhaseRecord->PhaseLink;
							}
						State->SustainPhase1 = -1;
						EnvStepToNextInterval(State);
						return;
					}
			}
		if (State->Phase < State->SustainPhase1)
			{
				/* if we haven't even reached the sustain phase, then cancel the sustain */
				/* phase so that it can't happen */
				State->SustainPhase1 = -1;
			}
		else if (State->Phase == State->SustainPhase1)
			{
				/* or, if we are sustaining, then break the sustain */
				State->SustainPhase1 = -1;
				if (State->EnvelopeUpdate == &EnvUpdateSustain)
					{
						/* we are sustaining, so break it */
						EnvStepToNextInterval(State);
					}
				/* else we haven't reached it, but we broke it above */
			}
		/* otherwise, we must be past it so just ignore */
	}


/* create key-up impulse.  call this before calling EnvelopeUpdate during a */
/* given cycle.  this call preserves the current level of the envelope but */
/* skips to the phase after the particular sustain. */
void								EnvelopeKeyUpSustain2(EvalEnvelopeRec* State)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);

		if (State->Phase <= State->SustainPhase2)
			{
				/* find out if we should skip ahead to the sustain point */
				if ((State->SustainPhase2Type == eEnvelopeSustainPointSkip)
					|| (State->SustainPhase2Type == eEnvelopeReleasePointSkip))
					{
						while ((State->CurrentPhaseRecord != NIL)
							&& (State->Phase < State->SustainPhase2))
							{
								State->Phase += 1;
								State->CurrentPhaseRecord = State->CurrentPhaseRecord->PhaseLink;
							}
						State->SustainPhase2 = -1;
						EnvStepToNextInterval(State);
						return;
					}
			}
		if (State->Phase < State->SustainPhase2)
			{
				/* if we haven't even reached the sustain phase, then cancel the sustain */
				/* phase so that it can't happen */
				State->SustainPhase2 = -1;
			}
		else if (State->Phase == State->SustainPhase2)
			{
				/* or, if we are sustaining, then break the sustain */
				State->SustainPhase2 = -1;
				if (State->EnvelopeUpdate == &EnvUpdateSustain)
					{
						/* we are sustaining, so break it */
						EnvStepToNextInterval(State);
					}
				/* else we haven't reached it, but we broke it above */
			}
		/* otherwise, we must be past it so just ignore */
	}


/* create key-up impulse.  call this before calling EnvelopeUpdate during a */
/* given cycle.  this call preserves the current level of the envelope but */
/* skips to the phase after the particular sustain. */
void								EnvelopeKeyUpSustain3(EvalEnvelopeRec* State)
	{
		CheckPtrExistence(State);
		DebugCheckState(State);

		if (State->Phase <= State->SustainPhase3)
			{
				/* find out if we should skip ahead to the sustain point */
				if ((State->SustainPhase3Type == eEnvelopeSustainPointSkip)
					|| (State->SustainPhase3Type == eEnvelopeReleasePointSkip))
					{
						while ((State->CurrentPhaseRecord != NIL)
							&& (State->Phase < State->SustainPhase3))
							{
								State->Phase += 1;
								State->CurrentPhaseRecord = State->CurrentPhaseRecord->PhaseLink;
							}
						State->SustainPhase3 = -1;
						EnvStepToNextInterval(State);
						return;
					}
			}
		if (State->Phase < State->SustainPhase3)
			{
				/* if we haven't even reached the sustain phase, then cancel the sustain */
				/* phase so that it can't happen */
				State->SustainPhase3 = -1;
			}
		else if (State->Phase == State->SustainPhase3)
			{
				/* or, if we are sustaining, then break the sustain */
				State->SustainPhase3 = -1;
				if (State->EnvelopeUpdate == &EnvUpdateSustain)
					{
						/* we are sustaining, so break it */
						EnvStepToNextInterval(State);
					}
				/* else we haven't reached it, but we broke it above */
			}
		/* otherwise, we must be past it so just ignore */
	}


/* update routine for linear-absolute intervals */
static FastFixedType	EnvUpdateLinearAbsolute(EvalEnvelopeRec* State)
	{
		/* decrement the counter */
		State->LinearTransitionCounter -= 1;

		/* see if we should advance to the next state */
		if (State->LinearTransitionCounter < 0)
			{
				/* yup */
				EnvStepToNextInterval(State);
				/* a new function is now in charge, so defer to it */
				return (*State->EnvelopeUpdate)(State);
			}
		 else
			{
				/* nope, we need to compute the next value */
				State->LastOutputtedValue = LinearTransitionUpdate(State->LinearTransition);
				return State->LastOutputtedValue;
			}
	}


/* update routine for linear-decibel intervals */
static FastFixedType	EnvUpdateLinearDecibels(EvalEnvelopeRec* State)
	{
		/* decrement the counter */
		State->LinearTransitionCounter -= 1;

		/* see if we should advance to the next state */
		if (State->LinearTransitionCounter < 0)
			{
				/* yup */
				EnvStepToNextInterval(State);
				/* a new function is now in charge, so defer to it */
				return (*State->EnvelopeUpdate)(State);
			}
		 else
			{
				float								Index0To1;
				long double					NowDecibels; /* intermediate -- long double */
				MyBoolean						Negative;

				/* we need to compute the next value */
				/*State->LastOutputtedValue = LinearTransitionUpdate(State->LinearTransition);*/
				if (State->LinearTransitionTotalDuration > 1)
					{
						Index0To1 = 1 - ((float)State->LinearTransitionCounter
							/ (State->LinearTransitionTotalDuration - 1));
					}
				 else
					{
						Index0To1 = 1;
					}
				NowDecibels = (State->FinalDecibels * Index0To1)
					+ (State->InitialDecibels * (1 - Index0To1));
				Negative = False;
				if (NowDecibels < 0)
					{
						NowDecibels = - NowDecibels;
						Negative = True;
					}
				/* some funny stuff might go on here since we're treating the FastFixed */
				/* numbers as integers, but it shouldn't matter since the curve should look */
				/* the same if the ratio of start to finish is the same no matter what */
				/* the actual magnitudes are. */
				/* in fact, the exponential base doesn't matter: */
				/*  exp((ln 2 + ln 3) / 2) = 2.4494897427831781 */
				/*  10^((20*log 2 + 20*log 3) / 2 / 20) = 2.4494897427831781 */
				State->LastOutputtedValue = FEXP(NowDecibels);
				if (Negative)
					{
						State->LastOutputtedValue = - State->LastOutputtedValue;
					}
				return State->LastOutputtedValue;
			}
	}


/* sustain on a particular value */
static FastFixedType	EnvUpdateSustain(EvalEnvelopeRec* State)
	{
		return State->LastOutputtedValue;
	}


/* routine to step to the next non-zero width interval */
static void						EnvStepToNextInterval(EvalEnvelopeRec* State)
	{
		OneEnvPhaseRec*			CurrentPhase;

		/* first, check to see if we should sustain */
		if ((State->Phase >= 0)
			&& (((State->Phase == State->SustainPhase1)
				&& ((State->SustainPhase1Type == eEnvelopeSustainPointSkip)
				|| (State->SustainPhase1Type == eEnvelopeSustainPointNoSkip)))
			||
			((State->Phase == State->SustainPhase2)
				&& ((State->SustainPhase2Type == eEnvelopeSustainPointSkip)
				|| (State->SustainPhase2Type == eEnvelopeSustainPointNoSkip)))
			||
			((State->Phase == State->SustainPhase3)
				&& ((State->SustainPhase3Type == eEnvelopeSustainPointSkip)
				|| (State->SustainPhase3Type == eEnvelopeSustainPointNoSkip)))))
			{
				/* yup, sustain */
				State->EnvelopeUpdate = &EnvUpdateSustain;
				return;
			}

		/* if no sustain, then we can advance to the next phase */
		State->Phase += 1;
		CurrentPhase = State->CurrentPhaseRecord;
		if (CurrentPhase == NIL)
			{
				/* oh, look, no more phases, so we must be done.  just sustain */
				/* the last value indefinitely */
				State->EnvelopeUpdate = &EnvUpdateSustain;
				State->EnvelopeHasFinished = True;
				return;
			}
		 else
			{
				State->CurrentPhaseRecord = State->CurrentPhaseRecord->PhaseLink;
			}

		/* well, we actually have to do some work. */
		DebugCheckPhase(CurrentPhase);
		if (CurrentPhase->Duration > 0)
			{
				/* if duration is greater than 0, then we go normally */
				State->LinearTransitionTotalDuration = CurrentPhase->Duration;
				State->LinearTransitionCounter = State->LinearTransitionTotalDuration;
				/* figure out what routine to use */
				switch (CurrentPhase->TransitionType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"EnvStepToNextInterval:  bad envelope curve value"));
							break;
						case eEnvelopeLinearInAmplitude:
							State->EnvelopeUpdate = &EnvUpdateLinearAbsolute;
							switch (CurrentPhase->TargetType)
								{
									default:
										EXECUTE(PRERR(ForceAbort,
											"EnvStepToNextInterval:  bad envelope target type"));
										break;
									case eEnvelopeTargetAbsolute:
										RefillLinearTransition(State->LinearTransition,
											State->LastOutputtedValue,CurrentPhase->FinalAmplitude,
											State->LinearTransitionTotalDuration);
										break;
									case eEnvelopeTargetScaling:
										RefillLinearTransition(State->LinearTransition,
											State->LastOutputtedValue,
											State->LastOutputtedValue
												* FastFixed2Float(CurrentPhase->FinalAmplitude),
											State->LinearTransitionTotalDuration);
										break;
								}
							break;
						case eEnvelopeLinearInDecibels:
							{
								long							Temp;
								MyBoolean					Negative;

								/* figure out end points */
								State->EnvelopeUpdate = &EnvUpdateLinearDecibels;
								Temp = State->LastOutputtedValue;
								Negative = False;
								if (Temp < 0)
									{
										Temp = - Temp;
										Negative = True;
									}
								if (Temp == 0)
									{
										Temp = 1;
									}
								State->InitialDecibels = FLN(Temp);
								if (Negative)
									{
										State->InitialDecibels = - State->InitialDecibels;
									}
								switch (CurrentPhase->TargetType)
									{
										default:
											EXECUTE(PRERR(ForceAbort,
												"EnvStepToNextInterval:  bad envelope target type"));
											break;
										case eEnvelopeTargetAbsolute:
											Temp = CurrentPhase->FinalAmplitude;
											break;
										case eEnvelopeTargetScaling:
											Temp = FastFixed2Float(CurrentPhase->FinalAmplitude)
												* State->LastOutputtedValue;
											break;
									}
								Negative = False;
								if (Temp < 0)
									{
										Temp = - Temp;
										Negative = True;
									}
								if (Temp == 0)
									{
										Temp = 1;
									}
								State->FinalDecibels = FLN(Temp);
								if (Negative)
									{
										State->FinalDecibels = - State->FinalDecibels;
									}
							}
							break;
					}
			}
		 else
			{
				/* they want the transition immediately */
				switch (CurrentPhase->TargetType)
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"EnvStepToNextInterval:  bad envelope target type"));
							break;
						case eEnvelopeTargetAbsolute:
							State->LastOutputtedValue = CurrentPhase->FinalAmplitude;
							break;
						case eEnvelopeTargetScaling:
							State->LastOutputtedValue = FastFixed2Float(CurrentPhase->FinalAmplitude)
								* State->LastOutputtedValue;
							break;
					}
				/* do it again.  this will handle ties nicely too */
				EnvStepToNextInterval(State);
			}
	}


/* retrigger envelopes from the origin point */
void								EnvelopeRetriggerFromOrigin(EvalEnvelopeRec* State,
											float Accent1, float Accent2, float Accent3, float Accent4,
											float FrequencyHertz, float Loudness, float HurryUp,
											float TicksPerSecond, MyBoolean ActuallyRetrigger)
	{
		OneEnvPhaseRec*		Phase;
		long							Scan;

		CheckPtrExistence(State);
		DebugCheckState(State);

		/* if we actually retrigger, then reset the state */
		if (ActuallyRetrigger)
			{
				State->Phase = -1;
				State->CurrentPhaseRecord = State->PhaseListHead;
				while (State->Phase < State->Origin - 1)
					{
						State->CurrentPhaseRecord = State->CurrentPhaseRecord->PhaseLink;
						CheckPtrExistence(State->CurrentPhaseRecord); /* must not become NIL */
						State->Phase += 1;
					}
				State->SustainPhase1 = State->OriginalSustainPhase1;
				State->SustainPhase2 = State->OriginalSustainPhase2;
				State->SustainPhase3 = State->OriginalSustainPhase3;
				State->LinearTransitionCounter = 0; /* force transition on next update */
				State->EnvelopeUpdate = &EnvUpdateLinearAbsolute;
			}

		/* no matter what, refill the parameters */
		Phase = State->PhaseListHead;
		for (Scan = 0; Scan < State->NumPhases; Scan += 1)
			{
				CheckPtrExistence(Phase);
				DebugCheckPhase(Phase);

				/* fill in phase record parameters */
				PRNGCHK(State->Template->PhaseArray,&(State->Template->PhaseArray[Scan]),
					sizeof(State->Template->PhaseArray[Scan]));
				/* calculate the total duration.  the effect of accents is this: */
				/*  - the accent is the base-2 log of a multiplier for the rate.  a value of 0 */
				/*    does not change the rate.  -1 halves the rate, and 1 doubles the rate. */
				/*  - the accent scaling factor is the base-2 log for scaling the accent. */
				/*    a value of 0 eliminates the effect of the accent, a value of 1 does not */
				/*    scale the accent. */
				/*  - pitch has two factors:  normalization point and rolloff.  rolloff */
				/*    determines how much the signal will decrease with each octave.  0 */
				/*    removes effect, 1 halfs signal with each octave.  normalization point */
				/*    determines what pitch will be the invariant point. */
				Phase->Duration = TicksPerSecond * HurryUp
					* State->Template->PhaseArray[Scan].Duration
					* FPOWER(2, - (Accent1 * State->Template->PhaseArray[Scan].Accent1Rate
						+ Accent2 * State->Template->PhaseArray[Scan].Accent2Rate
						+ Accent3 * State->Template->PhaseArray[Scan].Accent3Rate
						+ Accent4 * State->Template->PhaseArray[Scan].Accent4Rate
						+ (FLN(FrequencyHertz / State->Template->PhaseArray[Scan].FrequencyRateNormalization)
						/ (float)LOG2) * State->Template->PhaseArray[Scan].FrequencyRateRolloff));
				/* the final amplitude scaling values are computed similarly to the rate */
				/* scaling values. */
				Phase->FinalAmplitude = Double2FastFixed(
					State->Template->PhaseArray[Scan].EndPoint
					* State->Template->OverallScalingFactor * Loudness
					* FPOWER(2, - (Accent1 * State->Template->PhaseArray[Scan].Accent1Amp
						+ Accent2 * State->Template->PhaseArray[Scan].Accent2Amp
						+ Accent3 * State->Template->PhaseArray[Scan].Accent3Amp
						+ Accent4 * State->Template->PhaseArray[Scan].Accent4Amp
						+ (FLN(FrequencyHertz / State->Template->PhaseArray[Scan].FrequencyAmpNormalization)
						/ (float)LOG2) * State->Template->PhaseArray[Scan].FrequencyAmpRolloff)));

				/* go to the next one */
				Phase = Phase->PhaseLink;
			}
	}
