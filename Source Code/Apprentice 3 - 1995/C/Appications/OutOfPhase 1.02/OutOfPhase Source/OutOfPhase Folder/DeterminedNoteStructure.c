/* DeterminedNoteStructure.c */
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
#define ShowMeIncrParamUpdateRec
#include "DeterminedNoteStructure.h"
#include "Memory.h"
#include "BinaryCodedDecimal.h"
#include "NoteObject.h"
#include "IncrementalParameterUpdator.h"
#include "FloatingPoint.h"
#include "Frequency.h"
#include "PlayTrackInfoThang.h"


static FrozenNoteRec*				FrozenNoteFreeList = NIL;


/* flush cached frozen note blocks */
void								FlushFrozenNoteStructures(void)
	{
		while (FrozenNoteFreeList != NIL)
			{
				FrozenNoteRec*		Temp;

				Temp = FrozenNoteFreeList;
				FrozenNoteFreeList = FrozenNoteFreeList->Next;
				ReleasePtr((char*)Temp);
			}
	}


/* make sure the frozen note structure is valid */
#if DEBUG
void								ValidateFrozenNote(FrozenNoteRec* FrozenNote)
	{
		FrozenNoteRec*		Scan;

		CheckPtrExistence(FrozenNote);
		Scan = FrozenNoteFreeList;
		while (Scan != NIL)
			{
				if (Scan == FrozenNote)
					{
						PRERR(ForceAbort,"ValidateFrozenNote:  note record is on free list");
					}
				Scan = Scan->Next;
			}
	}
#endif


/* dispose of a note object */
void								DisposeFrozenNote(FrozenNoteRec* FrozenNote)
	{
		ValidateFrozenNote(FrozenNote);
		FrozenNote->Next = FrozenNoteFreeList;
		FrozenNoteFreeList = FrozenNote;
	}


/* build a new note object with all parameters determined.  *StartAdjustOut */
/* indicates how many ticks before (negative) or after (positive) now that */
/* the key-down should occur.  this is added to the scanning gap size and envelope */
/* origins to figure out how to schedule the note */
FrozenNoteRec*			FixNoteParameters(struct IncrParamUpdateRec* GlobalParamSource,
											struct NoteObjectRec* Note, long* StartAdjustOut,
											float OverallVolumeScaling, float EnvelopeTicksPerDurationTick)
	{
		FrozenNoteRec*		FrozenNote;
		float							FloatTemp;
		float							FloatOtherTemp;
		long							IntTemp;

		CheckPtrExistence(GlobalParamSource);
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"FixNoteParameters:  note is a command"));
		ERROR(GetNoteIsItARest(Note),PRERR(ForceAbort,"FixNoteParameters:  can't handle rests"));

		if (FrozenNoteFreeList != NIL)
			{
				FrozenNote = FrozenNoteFreeList;
				FrozenNoteFreeList = FrozenNoteFreeList->Next;
			}
		 else
			{
				FrozenNote = (FrozenNoteRec*)AllocPtrCanFail(sizeof(FrozenNoteRec),"FrozenNoteRec");
				if (FrozenNote == NIL)
					{
						return NIL;
					}
			}
		EXECUTE(FrozenNote->Next = (FrozenNoteRec*)0x81818181;)

		/* reference to the note that defines this note. */
		FrozenNote->OriginalNote = Note;

		/* frequency determined by pitch index + detuning, in Hertz */
		IntTemp = Note->a.Note.Pitch + GlobalParamSource->TransposeHalfsteps;
		if (IntTemp < 0)
			{
				IntTemp = 0;
			}
		else if (IntTemp > NUMNOTES - 1)
			{
				IntTemp = NUMNOTES - 1;
			}
		FloatTemp = (float)FEXP(((float)(IntTemp - CENTERNOTE) / 12) * (float)LOG2)
			* (float)MIDDLEC;
		switch (Note->Flags & eDetuningModeMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad detuning mode"));
					break;
				case eDetuningModeDefault:
					FloatOtherTemp = SmallBCD2Single(Note->a.Note.Detuning)
						* LargeBCD2Single(GlobalParamSource->CurrentDetune);
					if (GlobalParamSource->DetuneHertz)
						{
							goto DetuneHertzPoint;
						}
					 else
						{
							goto DetuneHalfStepsPoint;
						}
					break;
				case eDetuningModeHalfSteps:
					FloatOtherTemp = SmallBCD2Single(Note->a.Note.Detuning)
						+ LargeBCD2Single(GlobalParamSource->CurrentDetune);
				 DetuneHalfStepsPoint:
					FrozenNote->NominalFrequency = FloatTemp
						* (float)FEXP((FloatOtherTemp / 12) * (float)LOG2);
					break;
				case eDetuningModeHertz:
					FloatOtherTemp = SmallBCD2Single(Note->a.Note.Detuning)
						+ LargeBCD2Single(GlobalParamSource->CurrentDetune);
				 DetuneHertzPoint:
					FrozenNote->NominalFrequency = FloatTemp + FloatOtherTemp;
					break;
			}

		/* frequency used for doing multisampling, in Hertz */
		if (Note->a.Note.MultisamplePitchAsIf != -1)
			{
				FrozenNote->MultisampleFrequency = (float)FEXP(((float)(Note->a.Note
					.MultisamplePitchAsIf - CENTERNOTE) / 12) * (float)LOG2) * (float)MIDDLEC;
			}
		 else
			{
				FrozenNote->MultisampleFrequency = FrozenNote->NominalFrequency;
			}

		/* acceleration of envelopes */
		FrozenNote->HurryUpFactor = SmallBCD2Single(Note->a.Note.HurryUpFactor)
			* LargeBCD2Single(GlobalParamSource->CurrentHurryUp);

		/* duration, in envelope ticks */
		switch (Note->Flags & eDurationMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad duration flags"));
					break;
				case e64thNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 64;
					break;
				case e32ndNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 32;
					break;
				case e16thNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 16;
					break;
				case e8thNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 8;
					break;
				case e4thNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 4;
					break;
				case e2ndNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION / 2;
					break;
				case eWholeNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION;
					break;
				case eDoubleNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION * 2;
					break;
				case eQuadNote:
					IntTemp = DURATIONUPDATECLOCKRESOLUTION * 4;
					break;
			}
		switch (Note->Flags & eDivisionMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad division flags"));
					break;
				case eDiv1Modifier:
					break;
				case eDiv3Modifier:
					IntTemp = IntTemp / 3;
					break;
				case eDiv5Modifier:
					IntTemp = IntTemp / 5;
					break;
				case eDiv7Modifier:
					IntTemp = IntTemp / 7;
					break;
			}
		if ((Note->Flags & eDotModifier) != 0)
			{
				IntTemp = (IntTemp * 3) / 2;
			}
		FloatTemp = IntTemp;
		switch (Note->Flags & eDurationAdjustMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad duration adjust flags"));
					break;
				case eDurationAdjustDefault:
					if (GlobalParamSource->DurationAdjustAdditive)
						{
							goto DurationAdjustAddPoint;
						}
					 else
						{
							goto DurationAdjustMultPoint;
						}
					break;
				case eDurationAdjustAdditive:
				 DurationAdjustAddPoint:
					FloatTemp = FloatTemp + SmallBCD2Single(Note->a.Note.DurationAdjust)
						* (DURATIONUPDATECLOCKRESOLUTION / 4);
					break;
				case eDurationAdjustMultiplicative:
				 DurationAdjustMultPoint:
					FloatTemp = FloatTemp * SmallBCD2Single(Note->a.Note.DurationAdjust);
					break;
			}
		if (GlobalParamSource->DurationAdjustAdditive)
			{
				FloatTemp = FloatTemp + LargeBCD2Single(GlobalParamSource->CurrentDurationAdjust)
					* (DURATIONUPDATECLOCKRESOLUTION / 4);
			}
		 else
			{
				FloatTemp = FloatTemp * LargeBCD2Single(GlobalParamSource->CurrentDurationAdjust);
			}
		/* this line is what converts from duration update ticks to envelope ticks */
		FrozenNote->Duration = FloatTemp * EnvelopeTicksPerDurationTick;

		/* portamento duration, in envelope ticks */
		FrozenNote->PortamentoDuration = SmallBCD2Single(Note->a.Note.PortamentoDuration)
			* (DURATIONUPDATECLOCKRESOLUTION / 4) * EnvelopeTicksPerDurationTick;

		/* first release point, in envelope ticks after start of note */
		switch (Note->Flags & eRelease1OriginMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad release point 1 origin flags"));
					break;
				case eRelease1FromStart:
					FrozenNote->ReleasePoint1 = SmallBCD2Single(Note->a.Note.ReleasePoint1)
						* FrozenNote->Duration;
					FrozenNote->Release1FromStart = True;
					break;
				case eRelease1FromEnd:
					FrozenNote->ReleasePoint1 = (1 - SmallBCD2Single(Note->a.Note.ReleasePoint1))
						* FrozenNote->Duration;
					FrozenNote->Release1FromStart = False;
					break;
				case eRelease1FromDefault:
					if (GlobalParamSource->ReleasePoint1FromStart)
						{
							FrozenNote->ReleasePoint1 = (SmallBCD2Single(Note->a.Note.ReleasePoint1)
								+ LargeBCD2Single(GlobalParamSource->CurrentReleasePoint1))
								* FrozenNote->Duration;
							FrozenNote->Release1FromStart = True;
						}
					 else
						{
							FrozenNote->ReleasePoint1 = (1 - (SmallBCD2Single(
								Note->a.Note.ReleasePoint1) + LargeBCD2Single(
								GlobalParamSource->CurrentReleasePoint1))) * FrozenNote->Duration;
							FrozenNote->Release1FromStart = False;
						}
					break;
			}

		/* second release point, in envelope ticks after start of note */
		switch (Note->Flags & eRelease2OriginMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad release point 2 origin flags"));
					break;
				case eRelease2FromStart:
					FrozenNote->ReleasePoint2 = SmallBCD2Single(Note->a.Note.ReleasePoint2)
						* FrozenNote->Duration;
					FrozenNote->Release2FromStart = True;
					break;
				case eRelease2FromEnd:
					FrozenNote->ReleasePoint2 = (1 - SmallBCD2Single(Note->a.Note.ReleasePoint2))
						* FrozenNote->Duration;
					FrozenNote->Release2FromStart = False;
					break;
				case eRelease2FromDefault:
					if (GlobalParamSource->ReleasePoint2FromStart)
						{
							FrozenNote->ReleasePoint2 = (SmallBCD2Single(Note->a.Note.ReleasePoint2)
								+ LargeBCD2Single(GlobalParamSource->CurrentReleasePoint2))
								* FrozenNote->Duration;
							FrozenNote->Release2FromStart = True;
						}
					 else
						{
							FrozenNote->ReleasePoint2 = (1 - (SmallBCD2Single(
								Note->a.Note.ReleasePoint2) + LargeBCD2Single(
								GlobalParamSource->CurrentReleasePoint2))) * FrozenNote->Duration;
							FrozenNote->Release2FromStart = False;
						}
					break;
			}

		/* third release point, in envelope ticks after start of note */
		if ((Note->Flags & eRelease3FromStartNotEnd) != 0)
			{
				FrozenNote->ReleasePoint3 = 0;
				FrozenNote->Release3FromStart = True;
			}
		 else
			{
				FrozenNote->ReleasePoint3 = FrozenNote->Duration;
				FrozenNote->Release3FromStart = False;
			}

		/* overall loudness adjustment for envelopes, including global volume scaling */
		FrozenNote->LoudnessAdjust = SmallBCD2Single(Note->a.Note.OverallLoudnessAdjustment)
			* LargeBCD2Single(GlobalParamSource->CurrentVolume) * OverallVolumeScaling;

		/* stereo positioning for note */
		FloatTemp = SmallBCD2Single(Note->a.Note.StereoPositionAdjustment)
			+ LargeBCD2Single(GlobalParamSource->CurrentStereoPosition);
		if (FloatTemp < -1)
			{
				FloatTemp = -1;
			}
		else if (FloatTemp > 1)
			{
				FloatTemp = 1;
			}
		FrozenNote->StereoPosition = FloatTemp;

		/* accent values for controlling envelopes */
		FrozenNote->Accent1 = SmallBCD2Single(Note->a.Note.Accent1)
			+ LargeBCD2Single(GlobalParamSource->CurrentAccent1);
		FrozenNote->Accent2 = SmallBCD2Single(Note->a.Note.Accent2)
			+ LargeBCD2Single(GlobalParamSource->CurrentAccent2);
		FrozenNote->Accent3 = SmallBCD2Single(Note->a.Note.Accent3)
			+ LargeBCD2Single(GlobalParamSource->CurrentAccent3);
		FrozenNote->Accent4 = SmallBCD2Single(Note->a.Note.Accent4)
			+ LargeBCD2Single(GlobalParamSource->CurrentAccent4);

		/* pitch displacement maximum depth, in tonal Hertz */
		FloatTemp = SmallBCD2Single(Note->a.Note.PitchDisplacementDepthAdjustment);
		switch (Note->Flags & ePitchDisplacementDepthModeMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad pitch disp depth flags"));
					break;
				case ePitchDisplacementDepthModeHalfSteps:
					FrozenNote->PitchDisplacementDepthInHertz = False;
					break;
				case ePitchDisplacementDepthModeHertz:
					FrozenNote->PitchDisplacementDepthInHertz = True;
					break;
				case ePitchDisplacementDepthModeDefault:
					FloatTemp = FloatTemp * LargeBCD2Single(
						GlobalParamSource->CurrentPitchDisplacementDepthLimit);
					FrozenNote->PitchDisplacementDepthInHertz
						= GlobalParamSource->PitchDisplacementDepthHertz;
					break;
			}
		FrozenNote->PitchDisplacementDepthLimit = FloatTemp;

		/* pitch displacement maximum rate, in LFO Hertz */
		FrozenNote->PitchDisplacementRateLimit = SmallBCD2Double(
			Note->a.Note.PitchDisplacementRateAdjustment)
			* LargeBCD2Double(GlobalParamSource->CurrentPitchDisplacementRateLimit);

		/* pitch displacement start point, in envelope clocks after start of note */
		switch (Note->Flags & ePitchDisplacementStartOriginMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"FixNoteParameters:  bad pitch disp start flags"));
					break;
				case ePitchDisplacementStartFromStart:
					FrozenNote->PitchDisplacementStartPoint = FrozenNote->Duration
						* SmallBCD2Single(Note->a.Note.PitchDisplacementStartPoint);
					break;
				case ePitchDisplacementStartFromEnd:
					FrozenNote->PitchDisplacementStartPoint = FrozenNote->Duration
						* (1 - SmallBCD2Single(Note->a.Note.PitchDisplacementStartPoint));
					break;
				case ePitchDisplacementStartFromDefault:
					if (GlobalParamSource->PitchDisplacementStartPointFromStart)
						{
							FrozenNote->PitchDisplacementStartPoint = FrozenNote->Duration
								* (SmallBCD2Single(Note->a.Note.PitchDisplacementStartPoint)
								+ LargeBCD2Single(GlobalParamSource->CurrentPitchDisplacementStartPoint));
						}
					 else
						{
							FrozenNote->PitchDisplacementStartPoint = FrozenNote->Duration
								* (1 - (SmallBCD2Single(Note->a.Note.PitchDisplacementStartPoint)
								+ LargeBCD2Single(GlobalParamSource->CurrentPitchDisplacementStartPoint)));
						}
					break;
			}

		*StartAdjustOut = (SmallBCD2Single(Note->a.Note.EarlyLateAdjust)
			+ LargeBCD2Single(GlobalParamSource->CurrentEarlyLateAdjust))
			* FrozenNote->Duration;

		return FrozenNote;
	}
