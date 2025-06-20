/* Envelope.c */
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
#include "Envelope.h"
#include "Memory.h"
#include "Frequency.h"


/* create a new envelope record with nothing in it */
EnvelopeRec*						NewEnvelope(void)
	{
		EnvelopeRec*					Envelope;

		Envelope = (EnvelopeRec*)AllocPtrCanFail(sizeof(EnvelopeRec),"EnvelopeRec");
		if (Envelope == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Envelope->PhaseArray = (EnvStepRec*)AllocPtrCanFail(0,"PhaseArray");
		if (Envelope->PhaseArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Envelope);
				goto FailurePoint1;
			}
		Envelope->SustainPhase1 = -1;
		Envelope->SustainPhase2 = -1;
		Envelope->SustainPhase3 = -1;
		Envelope->SustainPhase1Type = eEnvelopeReleasePointNoSkip;
		Envelope->SustainPhase2Type = eEnvelopeReleasePointNoSkip;
		Envelope->SustainPhase3Type = eEnvelopeReleasePointNoSkip;
		Envelope->Origin = 0;
		Envelope->NumPhases = 0;
		Envelope->OverallScalingFactor = 1;
		return Envelope;
	}


/* dispose of an envelope record */
void										DisposeEnvelope(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		ReleasePtr((char*)Envelope->PhaseArray);
		ReleasePtr((char*)Envelope);
	}


/* find out how many frames there are in the envelope */
long										GetEnvelopeNumFrames(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->NumPhases;
	}


/* set a release point.  -1 means this release point is ignored */
void										EnvelopeSetReleasePoint1(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType)
	{
		CheckPtrExistence(Envelope);
		ERROR((Release < -1) || (Release > Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint1:  release point is out of range"));
		ERROR((ReleaseType != eEnvelopeSustainPointSkip)
			&& (ReleaseType != eEnvelopeReleasePointSkip)
			&& (ReleaseType != eEnvelopeSustainPointNoSkip)
			&& (ReleaseType != eEnvelopeReleasePointNoSkip),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint1:  bad release type"));
		Envelope->SustainPhase1 = Release;
		Envelope->SustainPhase1Type = ReleaseType;
	}


/* set a release point.  -1 means this release point is ignored */
void										EnvelopeSetReleasePoint2(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType)
	{
		CheckPtrExistence(Envelope);
		ERROR((Release < -1) || (Release > Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint2:  release point is out of range"));
		ERROR((ReleaseType != eEnvelopeSustainPointSkip)
			&& (ReleaseType != eEnvelopeReleasePointSkip)
			&& (ReleaseType != eEnvelopeSustainPointNoSkip)
			&& (ReleaseType != eEnvelopeReleasePointNoSkip),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint2:  bad release type"));
		Envelope->SustainPhase2 = Release;
		Envelope->SustainPhase2Type = ReleaseType;
	}


/* set a release point.  -1 means this release point is ignored */
void										EnvelopeSetReleasePoint3(EnvelopeRec* Envelope, long Release,
													SustainTypes ReleaseType)
	{
		CheckPtrExistence(Envelope);
		ERROR((Release < -1) || (Release > Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint3:  release point is out of range"));
		ERROR((ReleaseType != eEnvelopeSustainPointSkip)
			&& (ReleaseType != eEnvelopeReleasePointSkip)
			&& (ReleaseType != eEnvelopeSustainPointNoSkip)
			&& (ReleaseType != eEnvelopeReleasePointNoSkip),PRERR(ForceAbort,
			"EnvelopeSetReleasePoint3:  bad release type"));
		Envelope->SustainPhase3 = Release;
		Envelope->SustainPhase3Type = ReleaseType;
	}


/* get the value of a release point */
long										GetEnvelopeReleasePoint1(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase1;
	}


/* get the value of a release point */
long										GetEnvelopeReleasePoint2(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase2;
	}


/* get the value of a release point */
long										GetEnvelopeReleasePoint3(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase3;
	}


/* get the release point type */
SustainTypes						GetEnvelopeReleaseType1(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase1Type;
	}


/* get the release point type */
SustainTypes						GetEnvelopeReleaseType2(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase2Type;
	}


/* get the release point type */
SustainTypes						GetEnvelopeReleaseType3(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->SustainPhase3Type;
	}


/* set the origin of the envelope */
void										EnvelopeSetOrigin(EnvelopeRec* Envelope, long Origin)
	{
		CheckPtrExistence(Envelope);
		Envelope->Origin = Origin;
	}


/* get the origin from the envelope */
long										GetEnvelopeOrigin(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->Origin;
	}


void										EnvelopeSetAccent1Amp(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent1Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent1Amp = Val;
	}


void										EnvelopeSetAccent2Amp(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent2Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent2Amp = Val;
	}


void										EnvelopeSetAccent3Amp(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent3Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent3Amp = Val;
	}


void										EnvelopeSetAccent4Amp(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent4Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent4Amp = Val;
	}


void										EnvelopeSetFreqAmpRolloff(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetFreqAmpRolloff:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].FrequencyAmpRolloff = Val;
	}


void										EnvelopeSetFreqAmpNormalization(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetFreqAmpNormalization:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].FrequencyAmpNormalization = Val;
	}


void										EnvelopeSetAccent1Rate(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent1Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent1Rate = Val;
	}


void										EnvelopeSetAccent2Rate(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent2Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent2Rate = Val;
	}


void										EnvelopeSetAccent3Rate(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent3Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent3Rate = Val;
	}


void										EnvelopeSetAccent4Rate(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetAccent4Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].Accent4Rate = Val;
	}


void										EnvelopeSetFreqRateRolloff(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetFreqRateRolloff:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].FrequencyRateRolloff = Val;
	}


void										EnvelopeSetFreqRateNormalization(EnvelopeRec* Envelope,
													double Val, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetFreqRateNormalization:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		Envelope->PhaseArray[Phase].FrequencyRateNormalization = Val;
	}


EnvNumberType						GetEnvelopeAccent1Amp(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent1Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent1Amp;
	}


EnvNumberType						GetEnvelopeAccent2Amp(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent2Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent2Amp;
	}


EnvNumberType						GetEnvelopeAccent3Amp(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent3Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent3Amp;
	}


EnvNumberType						GetEnvelopeAccent4Amp(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent4Amp:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent4Amp;
	}


EnvNumberType						GetEnvelopeFreqAmpRolloff(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeFreqAmpRolloff:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].FrequencyAmpRolloff;
	}


EnvNumberType						GetEnvelopeFreqAmpNormalization(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeFreqAmpNormalization:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].FrequencyAmpNormalization;
	}


EnvNumberType						GetEnvelopeAccent1Rate(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent1Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent1Rate;
	}


EnvNumberType						GetEnvelopeAccent2Rate(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent2Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent2Rate;
	}


EnvNumberType						GetEnvelopeAccent3Rate(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent3Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent3Rate;
	}


EnvNumberType						GetEnvelopeAccent4Rate(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeAccent4Rate:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].Accent4Rate;
	}


EnvNumberType						GetEnvelopeFreqRateRolloff(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeFreqRateRolloff:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].FrequencyRateRolloff;
	}


EnvNumberType						GetEnvelopeFreqRateNormalization(EnvelopeRec* Envelope, long Phase)
	{
		CheckPtrExistence(Envelope);
		ERROR((Phase < 0) || (Phase >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopeFreqRateNormalization:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Phase]),
			sizeof(Envelope->PhaseArray[Phase]));
		return Envelope->PhaseArray[Phase].FrequencyRateNormalization;
	}


/* set the overall amplitude scaling factor */
void										EnvelopeSetOverallAmplitude(EnvelopeRec* Envelope,
													double OverallAmplitude)
	{
		CheckPtrExistence(Envelope);
		Envelope->OverallScalingFactor = OverallAmplitude;
	}


/* get the overall amplitude */
EnvNumberType						GetEnvelopeOverallAmplitude(EnvelopeRec* Envelope)
	{
		CheckPtrExistence(Envelope);
		return Envelope->OverallScalingFactor;
	}


/* insert a new phase at the specified position.  Values are undefined */
MyBoolean								EnvelopeInsertPhase(EnvelopeRec* Envelope, long Index)
	{
		EnvStepRec*						ArrayTemp;
		long									Scan;

		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index > Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeInsertPhase:  index is out of range"));

		ArrayTemp = (EnvStepRec*)ResizePtr((char*)Envelope->PhaseArray,
			sizeof(EnvStepRec) * (Envelope->NumPhases + 1));
		if (ArrayTemp == NIL)
			{
			 FailurePoint1:
				return False;
			}
		Envelope->PhaseArray = ArrayTemp;

		/* now transfer the data in the arrays to make a hole */
		for (Scan = Envelope->NumPhases - 1; Scan >= Index; Scan -= 1)
			{
				PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Scan]),
					sizeof(Envelope->PhaseArray[Scan]));
				PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Scan + 1]),
					sizeof(Envelope->PhaseArray[Scan + 1]));
				Envelope->PhaseArray[Scan + 1] = Envelope->PhaseArray[Scan];
			}
		Envelope->PhaseArray[Index].Duration = 0;
		Envelope->PhaseArray[Index].EndPoint = 0;
		Envelope->PhaseArray[Index].TransitionType = eEnvelopeLinearInAmplitude;
		Envelope->PhaseArray[Index].TargetType = eEnvelopeTargetAbsolute;
		Envelope->PhaseArray[Index].Accent1Amp = 0;
		Envelope->PhaseArray[Index].Accent2Amp = 0;
		Envelope->PhaseArray[Index].Accent3Amp = 0;
		Envelope->PhaseArray[Index].Accent4Amp = 0;
		Envelope->PhaseArray[Index].FrequencyAmpRolloff = 0;
		Envelope->PhaseArray[Index].FrequencyAmpNormalization = MIDDLEC;
		Envelope->PhaseArray[Index].Accent1Rate = 0;
		Envelope->PhaseArray[Index].Accent2Rate = 0;
		Envelope->PhaseArray[Index].Accent3Rate = 0;
		Envelope->PhaseArray[Index].Accent4Rate = 0;
		Envelope->PhaseArray[Index].FrequencyRateRolloff = 0;
		Envelope->PhaseArray[Index].FrequencyRateNormalization = MIDDLEC;
		Envelope->NumPhases += 1;
		return True;
	}


/* delete a phase from the envelope */
void										EnvelopeDeletePhase(EnvelopeRec* Envelope, long Index)
	{
		long									Scan;

		CheckPtrExistence(Envelope);
		/* since we don't use the length of the arrays for anything, we can leave */
		/* the extra cell at the end and it won't hurt anything */
		ERROR((Index < 0) || (Index > Envelope->NumPhases - 1),PRERR(ForceAbort,
			"EnvelopeDeletePhase:  index is out of range"));
		for (Scan = Index; Scan < Envelope->NumPhases - 1; Scan += 1)
			{
				PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Scan]),
					sizeof(Envelope->PhaseArray[Scan]));
				PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Scan + 1]),
					sizeof(Envelope->PhaseArray[Scan + 1]));
				Envelope->PhaseArray[Scan] = Envelope->PhaseArray[Scan + 1];
			}
		Envelope->NumPhases -= 1;
		if (Envelope->SustainPhase1 > Envelope->NumPhases)
			{
				Envelope->SustainPhase1 = Envelope->NumPhases;
			}
		if (Envelope->SustainPhase2 > Envelope->NumPhases)
			{
				Envelope->SustainPhase2 = Envelope->NumPhases;
			}
		if (Envelope->SustainPhase3 > Envelope->NumPhases)
			{
				Envelope->SustainPhase3 = Envelope->NumPhases;
			}
		if (Envelope->Origin > Envelope->NumPhases)
			{
				Envelope->Origin = Envelope->NumPhases;
			}
	}


/* set a new value for the specified phase's duration */
void										EnvelopeSetPhaseDuration(EnvelopeRec* Envelope, long Index,
													double Duration)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetPhaseDuration:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		Envelope->PhaseArray[Index].Duration = Duration;
	}


/* get the duration from the specified envelope position */
EnvNumberType						GetEnvelopePhaseDuration(EnvelopeRec* Envelope, long Index)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopePhaseDuration:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		return Envelope->PhaseArray[Index].Duration;
	}


/* set a new value for the specified phase's ultimate value */
void										EnvelopeSetPhaseFinalValue(EnvelopeRec* Envelope, long Index,
													double FinalValue)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetPhaseFinalValue:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		Envelope->PhaseArray[Index].EndPoint = FinalValue;
	}


/* get the phase's ultimate value */
EnvNumberType						GetEnvelopePhaseFinalValue(EnvelopeRec* Envelope, long Index)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopePhaseFinalValue:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		return Envelope->PhaseArray[Index].EndPoint;
	}


/* set the value for a phase's transition type */
void										EnvelopeSetPhaseTransitionType(EnvelopeRec* Envelope, long Index,
													EnvTransTypes TransitionType)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetPhaseTransitionType:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		Envelope->PhaseArray[Index].TransitionType = TransitionType;
	}


/* obtain the value in a phase's transition type */
EnvTransTypes						GetEnvelopePhaseTransitionType(EnvelopeRec* Envelope, long Index)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopePhaseTransitionType:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		return Envelope->PhaseArray[Index].TransitionType;
	}


/* set the value for a phase's target type */
void										EnvelopeSetPhaseTargetType(EnvelopeRec* Envelope, long Index,
													EnvTargetTypes TargetType)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"EnvelopeSetPhaseTargetType:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		Envelope->PhaseArray[Index].TargetType = TargetType;
	}


/* get the target type for a phase */
EnvTargetTypes					GetEnvelopePhaseTargetType(EnvelopeRec* Envelope, long Index)
	{
		CheckPtrExistence(Envelope);
		ERROR((Index < 0) || (Index >= Envelope->NumPhases),PRERR(ForceAbort,
			"GetEnvelopePhaseTargetType:  index is out of range"));
		PRNGCHK(Envelope->PhaseArray,&(Envelope->PhaseArray[Index]),
			sizeof(Envelope->PhaseArray[Index]));
		return Envelope->PhaseArray[Index].TargetType;
	}
