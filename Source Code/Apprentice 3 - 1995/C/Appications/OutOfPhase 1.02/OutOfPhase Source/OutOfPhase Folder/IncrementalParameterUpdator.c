/* IncrementalParameterUpdator.c */
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

#define ShowMeIncrParamUpdateRec
#define ShowMe_NoteObjectRec;
#include "IncrementalParameterUpdator.h"
#include "PlayTrackInfoThang.h"
#include "Memory.h"
#include "TrackObject.h"
#include "LinearTransition.h"
#include "NoteObject.h"
#include "FrameObject.h"
#include "TempoController.h"


/* build a new incremental parameter updator */
IncrParamUpdateRec*		NewInitializedParamUpdator(struct TrackObjectRec* Template,
												struct TempoControlRec* TempoControl)
	{
		IncrParamUpdateRec*	Updator;

		CheckPtrExistence(Template);

		Updator = (IncrParamUpdateRec*)AllocPtrCanFail(sizeof(IncrParamUpdateRec),
			"IncrParamUpdateRec");
		if (Updator == NIL)
			{
			 FailurePoint1:
				return NIL;
			}

		Updator->DefaultStereoPosition = Double2LargeBCD(
			TrackObjectGetStereoPositioning(Template));
		Updator->CurrentStereoPosition = Updator->DefaultStereoPosition;
		Updator->StereoPositionChange = NewLinearTransition(0,0,1);
		if (Updator->StereoPositionChange == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Updator);
				goto FailurePoint1;
			}
		Updator->StereoPositionChangeCountdown = 0;

		Updator->DefaultVolume = Double2LargeBCD(TrackObjectGetOverallLoudness(Template));
		Updator->CurrentVolume = Updator->DefaultVolume;
		Updator->VolumeChange = NewLinearTransition(0,0,1);
		if (Updator->VolumeChange == NIL)
			{
			 FailurePoint3:
				DisposeLinearTransition(Updator->StereoPositionChange);
				goto FailurePoint2;
			}
		Updator->VolumeChangeCountdown = 0;

		Updator->DefaultReleasePoint1 = Double2LargeBCD(
			TrackObjectGetReleasePoint1(Template));
		Updator->CurrentReleasePoint1 = Updator->DefaultReleasePoint1;
		Updator->ReleasePoint1Change = NewLinearTransition(0,0,1);
		if (Updator->ReleasePoint1Change == NIL)
			{
			 FailurePoint4:
				DisposeLinearTransition(Updator->VolumeChange);
				goto FailurePoint3;
			}
		Updator->ReleasePoint1ChangeCountdown = 0;
		switch (TrackObjectGetReleasePoint1StartEndFlag(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetReleasePoint1StartEndFlag"));
					break;
				case eRelease1FromStart:
					Updator->ReleasePoint1FromStart = True;
					break;
				case eRelease1FromEnd:
					Updator->ReleasePoint1FromStart = False;
					break;
			}

		Updator->DefaultReleasePoint2 = Double2LargeBCD(
			TrackObjectGetReleasePoint2(Template));
		Updator->CurrentReleasePoint2 = Updator->DefaultReleasePoint2;
		Updator->ReleasePoint2Change = NewLinearTransition(0,0,1);
		if (Updator->ReleasePoint2Change == NIL)
			{
			 FailurePoint5:
				DisposeLinearTransition(Updator->ReleasePoint1Change);
				goto FailurePoint4;
			}
		Updator->ReleasePoint2ChangeCountdown = 0;
		switch (TrackObjectGetReleasePoint2StartEndFlag(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetReleasePoint2StartEndFlag"));
					break;
				case eRelease2FromStart:
					Updator->ReleasePoint2FromStart = True;
					break;
				case eRelease2FromEnd:
					Updator->ReleasePoint2FromStart = False;
					break;
			}

		Updator->DefaultAccent1 = Double2LargeBCD(TrackObjectGetAccent1(Template));
		Updator->CurrentAccent1 = Updator->DefaultAccent1;
		Updator->Accent1Change = NewLinearTransition(0,0,1);
		if (Updator->Accent1Change == NIL)
			{
			 FailurePoint6:
				DisposeLinearTransition(Updator->ReleasePoint2Change);
				goto FailurePoint5;
			}
		Updator->Accent1ChangeCountdown = 0;

		Updator->DefaultAccent2 = Double2LargeBCD(TrackObjectGetAccent2(Template));
		Updator->CurrentAccent2 = Updator->DefaultAccent2;
		Updator->Accent2Change = NewLinearTransition(0,0,1);
		if (Updator->Accent2Change == NIL)
			{
			 FailurePoint7:
				DisposeLinearTransition(Updator->Accent1Change);
				goto FailurePoint6;
			}
		Updator->Accent2ChangeCountdown = 0;

		Updator->DefaultAccent3 = Double2LargeBCD(TrackObjectGetAccent3(Template));
		Updator->CurrentAccent3 = Updator->DefaultAccent3;
		Updator->Accent3Change = NewLinearTransition(0,0,1);
		if (Updator->Accent3Change == NIL)
			{
			 FailurePoint8:
				DisposeLinearTransition(Updator->Accent2Change);
				goto FailurePoint7;
			}
		Updator->Accent3ChangeCountdown = 0;

		Updator->DefaultAccent4 = Double2LargeBCD(TrackObjectGetAccent4(Template));
		Updator->CurrentAccent4 = Updator->DefaultAccent4;
		Updator->Accent4Change = NewLinearTransition(0,0,1);
		if (Updator->Accent4Change == NIL)
			{
			 FailurePoint9:
				DisposeLinearTransition(Updator->Accent3Change);
				goto FailurePoint8;
			}
		Updator->Accent4ChangeCountdown = 0;

		Updator->DefaultPitchDisplacementDepthLimit = Double2LargeBCD(
			TrackObjectGetPitchDisplacementDepthAdjust(Template));
		Updator->CurrentPitchDisplacementDepthLimit = Updator->DefaultPitchDisplacementDepthLimit;
		Updator->PitchDisplacementDepthLimitChange = NewLinearTransition(0,0,1);
		if (Updator->PitchDisplacementDepthLimitChange == NIL)
			{
			 FailurePoint10:
				DisposeLinearTransition(Updator->Accent4Change);
				goto FailurePoint9;
			}
		Updator->PitchDisplacementDepthLimitChangeCountdown = 0;
		switch (TrackObjectGetPitchDisplacementDepthControlFlag(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetPitchDisplacementDepthControlFlag"));
					break;
				case ePitchDisplacementDepthModeHalfSteps:
					Updator->PitchDisplacementDepthHertz = False;
					break;
				case ePitchDisplacementDepthModeHertz:
					Updator->PitchDisplacementDepthHertz = True;
					break;
			}

		Updator->DefaultPitchDisplacementRateLimit = Double2LargeBCD(
			TrackObjectGetPitchDisplacementRateAdjust(Template));
		Updator->CurrentPitchDisplacementRateLimit = Updator->DefaultPitchDisplacementRateLimit;
		Updator->PitchDisplacementRateLimitChange = NewLinearTransition(0,0,1);
		if (Updator->PitchDisplacementRateLimitChange == NIL)
			{
			 FailurePoint11:
				DisposeLinearTransition(Updator->PitchDisplacementDepthLimitChange);
				goto FailurePoint10;
			}
		Updator->PitchDisplacementRateLimitChangeCountdown = 0;

		Updator->DefaultPitchDisplacementStartPoint = Double2LargeBCD(
			TrackObjectGetPitchDisplacementStartPoint(Template));
		Updator->CurrentPitchDisplacementStartPoint = Updator->DefaultPitchDisplacementStartPoint;
		Updator->PitchDisplacementStartPointChange = NewLinearTransition(0,0,1);
		if (Updator->PitchDisplacementStartPointChange == NIL)
			{
			 FailurePoint12:
				DisposeLinearTransition(Updator->PitchDisplacementRateLimitChange);
				goto FailurePoint11;
			}
		Updator->PitchDisplacementStartPointChangeCountdown = 0;
		switch (TrackObjectGetPitchDisplacementFromStartOrEnd(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetPitchDisplacementFromStartOrEnd"));
					break;
				case ePitchDisplacementStartFromStart:
					Updator->PitchDisplacementStartPointFromStart = True;
					break;
				case ePitchDisplacementStartFromEnd:
					Updator->PitchDisplacementStartPointFromStart = False;
					break;
			}

		Updator->DefaultHurryUp = Double2LargeBCD(TrackObjectGetHurryUp(Template));
		Updator->CurrentHurryUp = Updator->DefaultHurryUp;
		Updator->HurryUpChange = NewLinearTransition(0,0,1);
		if (Updator->HurryUpChange == NIL)
			{
			 FailurePoint13:
				DisposeLinearTransition(Updator->PitchDisplacementStartPointChange);
				goto FailurePoint12;
			}
		Updator->HurryUpChangeCountdown = 0;

		Updator->DefaultDetune = Double2LargeBCD(TrackObjectGetDetune(Template));
		Updator->CurrentDetune = Updator->DefaultDetune;
		Updator->DetuneChange = NewLinearTransition(0,0,1);
		if (Updator->DetuneChange == NIL)
			{
			 FailurePoint14:
				DisposeLinearTransition(Updator->HurryUpChange);
				goto FailurePoint13;
			}
		Updator->DetuneChangeCountdown = 0;
		switch (TrackObjectGetDetuneControlFlag(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetDetuneControlFlag"));
					break;
				case eDetuningModeHalfSteps:
					Updator->DetuneHertz = False;
					break;
				case eDetuningModeHertz:
					Updator->DetuneHertz = True;
					break;
			}

		Updator->DefaultEarlyLateAdjust = Double2LargeBCD(
			TrackObjectGetEarlyLateAdjust(Template));
		Updator->CurrentEarlyLateAdjust = Updator->DefaultEarlyLateAdjust;
		Updator->EarlyLateAdjustChange = NewLinearTransition(0,0,1);
		if (Updator->EarlyLateAdjustChange == NIL)
			{
			 FailurePoint15:
				DisposeLinearTransition(Updator->DetuneChange);
				goto FailurePoint14;
			}
		Updator->EarlyLateAdjustChangeCountdown = 0;

		Updator->DefaultDurationAdjust = Double2LargeBCD(
			TrackObjectGetDurationAdjust(Template));
		Updator->CurrentDurationAdjust = Updator->DefaultDurationAdjust;
		Updator->DurationAdjustChange = NewLinearTransition(0,0,1);
		if (Updator->DurationAdjustChange == NIL)
			{
			 FailurePoint16:
				DisposeLinearTransition(Updator->EarlyLateAdjustChange);
				goto FailurePoint15;
			}
		Updator->DurationAdjustChangeCountdown = 0;
		switch (TrackObjectGetDurationModeFlag(Template))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"NewInitializedParamUpdator:  bad value "
						"from TrackObjectGetDurationModeFlag"));
					break;
				case eDurationAdjustAdditive:
					Updator->DurationAdjustAdditive = True;
					break;
				case eDurationAdjustMultiplicative:
					Updator->DurationAdjustAdditive = False;
					break;
			}

		Updator->DefaultSurroundPosition = Double2LargeBCD(
			TrackObjectGetSurroundPositioning(Template));
		Updator->CurrentSurroundPosition = Updator->DefaultSurroundPosition;
		Updator->SurroundPositionChange = NewLinearTransition(0,0,1);
		if (Updator->SurroundPositionChange == NIL)
			{
			 FailurePoint17:
				DisposeLinearTransition(Updator->DurationAdjustChange);
				goto FailurePoint16;
			}

		Updator->TempoControl = TempoControl;

		Updator->TransposeHalfsteps = 0;

		return Updator;
	}


/* dispose of the incremental parameter updator */
void									DisposeParamUpdator(IncrParamUpdateRec* Updator)
	{
		CheckPtrExistence(Updator);

		DisposeLinearTransition(Updator->StereoPositionChange);
		DisposeLinearTransition(Updator->VolumeChange);
		DisposeLinearTransition(Updator->ReleasePoint1Change);
		DisposeLinearTransition(Updator->ReleasePoint2Change);
		DisposeLinearTransition(Updator->Accent1Change);
		DisposeLinearTransition(Updator->Accent2Change);
		DisposeLinearTransition(Updator->Accent3Change);
		DisposeLinearTransition(Updator->Accent4Change);
		DisposeLinearTransition(Updator->PitchDisplacementDepthLimitChange);
		DisposeLinearTransition(Updator->PitchDisplacementRateLimitChange);
		DisposeLinearTransition(Updator->PitchDisplacementStartPointChange);
		DisposeLinearTransition(Updator->HurryUpChange);
		DisposeLinearTransition(Updator->DetuneChange);
		DisposeLinearTransition(Updator->EarlyLateAdjustChange);
		DisposeLinearTransition(Updator->DurationAdjustChange);
		DisposeLinearTransition(Updator->SurroundPositionChange);

		ReleasePtr((char*)Updator);
	}


static void						UpdateOne(LargeBCDType* Current, LinearTransRec* Change,
												long* ChangeCountdown, long NumTicks)
	{
		if (NumTicks < *ChangeCountdown)
			{
				*Current = LinearTransitionUpdateMultiple(Change,NumTicks);
				*ChangeCountdown -= NumTicks;
			}
		else if (*ChangeCountdown > 0)
			{
				*Current = LinearTransitionUpdateMultiple(Change,*ChangeCountdown);
				*ChangeCountdown = 0;
			}
	}


/* execute a series of update cycles.  the value passed in is the number of */
/* duration ticks.  there are DURATIONUPDATECLOCKRESOLUTION (64*2*3*5*7) ticks */
/* in a whole note */
void									ExecuteParamUpdate(IncrParamUpdateRec* Updator, long NumTicks)
	{
		CheckPtrExistence(Updator);

		UpdateOne(&Updator->CurrentStereoPosition,Updator->StereoPositionChange,
			&Updator->StereoPositionChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentVolume,Updator->VolumeChange,
			&Updator->VolumeChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentReleasePoint1,Updator->ReleasePoint1Change,
			&Updator->ReleasePoint1ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentReleasePoint2,Updator->ReleasePoint2Change,
			&Updator->ReleasePoint2ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentAccent1,Updator->Accent1Change,
			&Updator->Accent1ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentAccent2,Updator->Accent2Change,
			&Updator->Accent2ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentAccent3,Updator->Accent3Change,
			&Updator->Accent3ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentAccent4,Updator->Accent4Change,
			&Updator->Accent4ChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentPitchDisplacementDepthLimit,
			Updator->PitchDisplacementDepthLimitChange,
			&Updator->PitchDisplacementDepthLimitChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentPitchDisplacementRateLimit,
			Updator->PitchDisplacementRateLimitChange,
			&Updator->PitchDisplacementRateLimitChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentPitchDisplacementStartPoint,
			Updator->PitchDisplacementStartPointChange,
			&Updator->PitchDisplacementStartPointChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentHurryUp,Updator->HurryUpChange,
			&Updator->HurryUpChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentDetune,Updator->DetuneChange,
			&Updator->DetuneChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentEarlyLateAdjust,Updator->EarlyLateAdjustChange,
			&Updator->EarlyLateAdjustChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentDurationAdjust,Updator->DurationAdjustChange,
			&Updator->DurationAdjustChangeCountdown,NumTicks);
		UpdateOne(&Updator->CurrentSurroundPosition,Updator->SurroundPositionChange,
			&Updator->SurroundPositionChangeCountdown,NumTicks);
	}


/* sweep the value to a new value */
static void							SweepToNewValue(LargeBCDType Current,
													LinearTransRec* Change, long* ChangeCountdown,
													LargeBCDType TargetValue, SmallExtBCDType NumBeatsToReach)
	{
		*ChangeCountdown = SmallExtBCD2Double(NumBeatsToReach)
			* (DURATIONUPDATECLOCKRESOLUTION / 4);
		RefillLinearTransition(Change,Current,TargetValue,*ChangeCountdown);
	}


/* sweep to a new value relative to the current value */
static void							SweepToAdjustedValue(LargeBCDType Current,
													LinearTransRec* Change, long* ChangeCountdown,
													LargeBCDType TargetValue, SmallExtBCDType NumBeatsToReach)
	{
		*ChangeCountdown = SmallExtBCD2Double(NumBeatsToReach)
			* (DURATIONUPDATECLOCKRESOLUTION / 4);
		RefillLinearTransition(Change,Current,TargetValue + Current,*ChangeCountdown);
	}


/* sweep to a new value relative to the current value */
static void							SweepToAdjustedValueMultiplicatively(LargeBCDType Current,
													LinearTransRec* Change, long* ChangeCountdown,
													LargeBCDType TargetValue, SmallExtBCDType NumBeatsToReach)
	{
		*ChangeCountdown = SmallExtBCD2Double(NumBeatsToReach)
			* (DURATIONUPDATECLOCKRESOLUTION / 4);
		RefillLinearTransition(Change,Current,Double2LargeBCD(LargeBCD2Double(TargetValue)
			* LargeBCD2Double(Current)),*ChangeCountdown);
	}


/* evaluate a command frame & set any parameters accordingly */
void									ExecuteParamCommandFrame(IncrParamUpdateRec* Updator,
												struct FrameObjectRec* CommandFrame)
	{
		NoteObjectRec*			Note;

		CheckPtrExistence(Updator);
		CheckPtrExistence(CommandFrame);
		Note = GetNoteFromFrame(CommandFrame,0);
		CheckPtrExistence(Note);
		ERROR(!IsItACommand(Note),PRERR(ForceAbort,
			"ExecuteParamCommandFrame:  not a command frame"));

		switch (Note->Flags & ~eCommandFlag)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"ExecuteParamCommandFrame:  unknown command opcode"));
					break;

				case eCmdRestoreTempo: /* restore the tempo to the default for the score */
					TempoControlRestoreDefault(Updator->TempoControl);
					break;
				case eCmdSetTempo: /* set tempo to <1xs> number of beats per second */
					TempoControlSetBeatsPerMinute(Updator->TempoControl,
						SmallExtBCD2LargeBCD(Note->a.Command.Argument1));
					break;
				case eCmdIncTempo: /* add <1xs> to the tempo control */
					TempoControlAdjustBeatsPerMinute(Updator->TempoControl,
						SmallExtBCD2LargeBCD(Note->a.Command.Argument1));
					break;
				case eCmdSweepTempoAbs: /* <1xs> = target tempo, <2xs> = # of beats to reach it */
					TempoControlSweepToNewValue(Updator->TempoControl,
						SmallExtBCD2LargeBCD(Note->a.Command.Argument1),Note->a.Command.Argument2);
					break;
				case eCmdSweepTempoRel: /* <1xs> = target adjust (add to tempo), <2xs> = # beats */
					TempoControlSweepToAdjustedValue(Updator->TempoControl,
						SmallExtBCD2LargeBCD(Note->a.Command.Argument1),Note->a.Command.Argument2);
					break;

				case eCmdRestoreStereoPosition: /* restore stereo position to channel's default */
					Updator->CurrentStereoPosition = Updator->DefaultStereoPosition;
					Updator->StereoPositionChangeCountdown = 0;
					break;
				case eCmdSetStereoPosition: /* set position in channel <1l>: -1 = left, 1 = right */
					Updator->CurrentStereoPosition = Note->a.Command.Argument1;
					Updator->StereoPositionChangeCountdown = 0;
					break;
				case eCmdIncStereoPosition: /* adjust stereo position by adding <1l> */
					Updator->CurrentStereoPosition += Note->a.Command.Argument1;
					Updator->StereoPositionChangeCountdown = 0;
					break;
				case eCmdSweepStereoAbs: /* <1l> = new pos, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentStereoPosition,Updator->StereoPositionChange,
						&Updator->StereoPositionChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepStereoRel: /* <1l> = pos adjust, <2xs> = # beats to get there */
					SweepToAdjustedValue(Updator->CurrentStereoPosition,
						Updator->StereoPositionChange,&Updator->StereoPositionChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreSurroundPosition: /* restore surround position to channel's default */
					Updator->CurrentSurroundPosition = Updator->DefaultSurroundPosition;
					Updator->SurroundPositionChangeCountdown = 0;
					break;
				case eCmdSetSurroundPosition: /* set position in channel <1l>: 1 = front, -1 = rear */
					Updator->CurrentSurroundPosition = Note->a.Command.Argument1;
					Updator->SurroundPositionChangeCountdown = 0;
					break;
				case eCmdIncSurroundPosition: /* adjust surround position by adding <1l> */
					Updator->CurrentSurroundPosition += Note->a.Command.Argument1;
					Updator->SurroundPositionChangeCountdown = 0;
					break;
				case eCmdSweepSurroundAbs: /* <1l> = new pos, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentSurroundPosition,Updator->SurroundPositionChange,
						&Updator->SurroundPositionChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepSurroundRel: /* <1l> = pos adjust, <2xs> = # beats to get there */
					SweepToAdjustedValue(Updator->CurrentSurroundPosition,
						Updator->SurroundPositionChange,&Updator->SurroundPositionChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreVolume: /* restore the volume to the default for the channel */
					Updator->CurrentVolume = Updator->DefaultVolume;
					Updator->VolumeChangeCountdown = 0;
					break;
				case eCmdSetVolume: /* set the volume to the specified level (0..1) in <1l> */
					Updator->CurrentVolume = Note->a.Command.Argument1;
					Updator->VolumeChangeCountdown = 0;
					break;
				case eCmdIncVolume: /* multiply <1l> by the volume control */
					Updator->CurrentVolume = Double2LargeBCD(LargeBCD2Double(
						Note->a.Command.Argument1) * LargeBCD2Double(Updator->CurrentVolume));
					Updator->VolumeChangeCountdown = 0;
					break;
				case eCmdSweepVolumeAbs: /* <1l> = new volume, <2xs> = # of beats to reach it */
					SweepToNewValue(Updator->CurrentVolume,Updator->VolumeChange,
						&Updator->VolumeChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepVolumeRel: /* <1l> = volume adjust, <2xs> = # of beats to reach it */
					SweepToAdjustedValueMultiplicatively(Updator->CurrentVolume,
						Updator->VolumeChange,&Updator->VolumeChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreReleasePoint1: /* restore release point to master default */
					Updator->CurrentReleasePoint1 = Updator->DefaultReleasePoint1;
					Updator->ReleasePoint1ChangeCountdown = 0;
					break;
				case eCmdSetReleasePoint1: /* set the default release point to new value <1l> */
					Updator->CurrentReleasePoint1 = Note->a.Command.Argument1;
					Updator->ReleasePoint1ChangeCountdown = 0;
					break;
				case eCmdIncReleasePoint1: /* add <1l> to default release point for adjustment */
					Updator->CurrentReleasePoint1 += Note->a.Command.Argument1;
					Updator->ReleasePoint1ChangeCountdown = 0;
					break;
				case eCmdReleasePointOrigin1: /* <1i> -1 = from start, 0 = from end of note */
					Updator->ReleasePoint1FromStart = (Note->a.Command.Argument1 < 0);
					break;
				case eCmdSweepReleaseAbs1: /* <1l> = new release, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentReleasePoint1,Updator->ReleasePoint1Change,
						&Updator->ReleasePoint1ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepReleaseRel1: /* <1l> = release adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentReleasePoint1,
						Updator->ReleasePoint1Change,&Updator->ReleasePoint1ChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreReleasePoint2: /* restore release point to master default */
					Updator->CurrentReleasePoint2 = Updator->DefaultReleasePoint2;
					Updator->ReleasePoint2ChangeCountdown = 0;
					break;
				case eCmdSetReleasePoint2: /* set the default release point to new value <1l> */
					Updator->CurrentReleasePoint2 = Note->a.Command.Argument1;
					Updator->ReleasePoint2ChangeCountdown = 0;
					break;
				case eCmdIncReleasePoint2: /* add <1l> to default release point for adjustment */
					Updator->CurrentReleasePoint2 += Note->a.Command.Argument1;
					Updator->ReleasePoint2ChangeCountdown = 0;
					break;
				case eCmdReleasePointOrigin2: /* <1i> -1 = from start, 0 = from end of note */
					Updator->ReleasePoint2FromStart = (Note->a.Command.Argument1 < 0);
					break;
				case eCmdSweepReleaseAbs2: /* <1l> = new release, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentReleasePoint2,Updator->ReleasePoint2Change,
						&Updator->ReleasePoint2ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepReleaseRel2: /* <1l> = release adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentReleasePoint2,
						Updator->ReleasePoint2Change,&Updator->ReleasePoint2ChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreAccent1: /* restore accent value to master default */
					Updator->CurrentAccent1 = Updator->DefaultAccent1;
					Updator->Accent1ChangeCountdown = 0;
					break;
				case eCmdSetAccent1: /* specify the new default accent in <1l> */
					Updator->CurrentAccent1 = Note->a.Command.Argument1;
					Updator->Accent1ChangeCountdown = 0;
					break;
				case eCmdIncAccent1: /* add <1l> to the default accent */
					Updator->CurrentAccent1 += Note->a.Command.Argument1;
					Updator->Accent1ChangeCountdown = 0;
					break;
				case eCmdSweepAccentAbs1: /* <1l> = new accent, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentAccent1,Updator->Accent1Change,
						&Updator->Accent1ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepAccentRel1: /* <1l> = accent adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentAccent1,Updator->Accent1Change,
						&Updator->Accent1ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestoreAccent2: /* restore accent value to master default */
					Updator->CurrentAccent2 = Updator->DefaultAccent2;
					Updator->Accent2ChangeCountdown = 0;
					break;
				case eCmdSetAccent2: /* specify the new default accent in <1l> */
					Updator->CurrentAccent2 = Note->a.Command.Argument1;
					Updator->Accent2ChangeCountdown = 0;
					break;
				case eCmdIncAccent2: /* add <1l> to the default accent */
					Updator->CurrentAccent2 += Note->a.Command.Argument1;
					Updator->Accent2ChangeCountdown = 0;
					break;
				case eCmdSweepAccentAbs2: /* <1l> = new accent, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentAccent2,Updator->Accent2Change,
						&Updator->Accent2ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepAccentRel2: /* <1l> = accent adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentAccent2,Updator->Accent2Change,
						&Updator->Accent2ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestoreAccent3: /* restore accent value to master default */
					Updator->CurrentAccent3 = Updator->DefaultAccent3;
					Updator->Accent3ChangeCountdown = 0;
					break;
				case eCmdSetAccent3: /* specify the new default accent in <1l> */
					Updator->CurrentAccent3 = Note->a.Command.Argument1;
					Updator->Accent3ChangeCountdown = 0;
					break;
				case eCmdIncAccent3: /* add <1l> to the default accent */
					Updator->CurrentAccent3 += Note->a.Command.Argument1;
					Updator->Accent3ChangeCountdown = 0;
					break;
				case eCmdSweepAccentAbs3: /* <1l> = new accent, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentAccent3,Updator->Accent3Change,
						&Updator->Accent3ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepAccentRel3: /* <1l> = accent adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentAccent3,Updator->Accent3Change,
						&Updator->Accent3ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestoreAccent4: /* restore accent value to master default */
					Updator->CurrentAccent4 = Updator->DefaultAccent4;
					Updator->Accent4ChangeCountdown = 0;
					break;
				case eCmdSetAccent4: /* specify the new default accent in <1l> */
					Updator->CurrentAccent4 = Note->a.Command.Argument1;
					Updator->Accent4ChangeCountdown = 0;
					break;
				case eCmdIncAccent4: /* add <1l> to the default accent */
					Updator->CurrentAccent4 += Note->a.Command.Argument1;
					Updator->Accent4ChangeCountdown = 0;
					break;
				case eCmdSweepAccentAbs4: /* <1l> = new accent, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentAccent4,Updator->Accent4Change,
						&Updator->Accent4ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepAccentRel4: /* <1l> = accent adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentAccent4,Updator->Accent4Change,
						&Updator->Accent4ChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestorePitchDispDepth: /* restore max pitch disp depth value to default */
					Updator->CurrentPitchDisplacementDepthLimit = Updator->DefaultPitchDisplacementDepthLimit;
					Updator->PitchDisplacementDepthLimitChangeCountdown = 0;
					break;
				case eCmdSetPitchDispDepth: /* set new max pitch disp depth <1l> */
					Updator->CurrentPitchDisplacementDepthLimit = Note->a.Command.Argument1;
					Updator->PitchDisplacementDepthLimitChangeCountdown = 0;
					break;
				case eCmdIncPitchDispDepth: /* add <1l> to the default pitch disp depth */
					Updator->CurrentPitchDisplacementDepthLimit += Note->a.Command.Argument1;
					Updator->PitchDisplacementDepthLimitChangeCountdown = 0;
					break;
				case eCmdPitchDispDepthMode: /* <1i>:  -1: Hertz, 0: half-steps */
					Updator->PitchDisplacementDepthHertz = (Note->a.Command.Argument1 < 0);
					break;
				case eCmdSweepPitchDispDepthAbs: /* <1l> = new depth, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentPitchDisplacementDepthLimit,
						Updator->PitchDisplacementDepthLimitChange,
						&Updator->PitchDisplacementDepthLimitChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;
				case eCmdSweepPitchDispDepthRel: /* <1l> = depth adjust, <2xs> = # of beats */
					SweepToAdjustedValue(Updator->CurrentPitchDisplacementDepthLimit,
						Updator->PitchDisplacementDepthLimitChange,
						&Updator->PitchDisplacementDepthLimitChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestorePitchDispRate: /* restore max pitch disp rate to the master default */
					Updator->CurrentPitchDisplacementRateLimit = Updator->DefaultPitchDisplacementRateLimit;
					Updator->PitchDisplacementRateLimitChangeCountdown = 0;
					break;
				case eCmdSetPitchDispRate: /* set new max pitch disp rate in seconds to <1l> */
					Updator->CurrentPitchDisplacementRateLimit = Note->a.Command.Argument1;
					Updator->PitchDisplacementRateLimitChangeCountdown = 0;
					break;
				case eCmdIncPitchDispRate: /* add <1l> to the default max pitch disp rate */
					Updator->CurrentPitchDisplacementRateLimit += Note->a.Command.Argument1;
					Updator->PitchDisplacementRateLimitChangeCountdown = 0;
					break;
				case eCmdSweepPitchDispRateAbs: /* <1l> = new rate, <2xs> = # of beats to get there */
					SweepToNewValue(Updator->CurrentPitchDisplacementRateLimit,
						Updator->PitchDisplacementRateLimitChange,
						&Updator->PitchDisplacementRateLimitChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;
				case eCmdSweepPitchDispRateRel: /* <1l> = rate adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentPitchDisplacementRateLimit,
						Updator->PitchDisplacementRateLimitChange,
						&Updator->PitchDisplacementRateLimitChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestorePitchDispStart: /* restore pitch disp start point to default */
					Updator->CurrentPitchDisplacementStartPoint = Updator->DefaultPitchDisplacementStartPoint;
					Updator->PitchDisplacementStartPointChangeCountdown = 0;
					break;
				case eCmdSetPitchDispStart: /* set the start point to <1l> */
					Updator->CurrentPitchDisplacementStartPoint = Note->a.Command.Argument1;
					Updator->PitchDisplacementStartPointChangeCountdown = 0;
					break;
				case eCmdIncPitchDispStart: /* add <1l> to the pitch disp start point */
					Updator->CurrentPitchDisplacementStartPoint += Note->a.Command.Argument1;
					Updator->PitchDisplacementStartPointChangeCountdown = 0;
					break;
				case eCmdPitchDispStartOrigin: /* specify the origin, same as for release point <1i> */
					Updator->PitchDisplacementStartPointFromStart = (Note->a.Command.Argument1 < 0);
					break;
				case eCmdSweepPitchDispStartAbs: /* <1l> = new vib start, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentPitchDisplacementStartPoint,
						Updator->PitchDisplacementStartPointChange,
						&Updator->PitchDisplacementStartPointChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;
				case eCmdSweepPitchDispStartRel: /* <1l> = vib adjust, <2xs> = # of beats */
					SweepToAdjustedValue(Updator->CurrentPitchDisplacementStartPoint,
						Updator->PitchDisplacementStartPointChange,
						&Updator->PitchDisplacementStartPointChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreHurryUp: /* restore default hurryup factor */
					Updator->CurrentHurryUp = Updator->DefaultHurryUp;
					Updator->HurryUpChangeCountdown = 0;
					break;
				case eCmdSetHurryUp: /* set the hurryup factor to <1l> */
					Updator->CurrentHurryUp = Note->a.Command.Argument1;
					Updator->HurryUpChangeCountdown = 0;
					break;
				case eCmdIncHurryUp: /* multiply <1l> by the hurryup factor */
					Updator->CurrentHurryUp *= Note->a.Command.Argument1;
					Updator->HurryUpChangeCountdown = 0;
					break;
				case eCmdSweepHurryUpAbs: /* <1l> = new hurryup factor, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentHurryUp,Updator->HurryUpChange,
						&Updator->HurryUpChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepHurryUpRel: /* <1l> = hurryup adjust, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentHurryUp,
						Updator->HurryUpChange,&Updator->HurryUpChangeCountdown,
						Note->a.Command.Argument1,Note->a.Command.Argument2);
					break;

				case eCmdRestoreDetune: /* restore the default detune factor */
					Updator->CurrentDetune = Updator->DefaultDetune;
					Updator->DetuneChangeCountdown = 0;
					break;
				case eCmdSetDetune: /* set the detune factor to <1l> */
					Updator->CurrentDetune = Note->a.Command.Argument1;
					Updator->DetuneChangeCountdown = 0;
					break;
				case eCmdIncDetune: /* add <1l> to current detune factor */
					Updator->CurrentDetune += Note->a.Command.Argument1;
					Updator->DetuneChangeCountdown = 0;
					break;
				case eCmdDetuneMode: /* <1i>:  -1: Hertz, 0: half-steps */
					Updator->DetuneHertz = (Note->a.Command.Argument1 < 0);
					break;
				case eCmdSweepDetuneAbs: /* <1l> = new detune, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentDetune,Updator->DetuneChange,
						&Updator->DetuneChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepDetuneRel: /* <1l> = detune adjust, <2xs> = # of beats */
					SweepToAdjustedValue(Updator->CurrentDetune,Updator->DetuneChange,
						&Updator->DetuneChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestoreEarlyLateAdjust: /* restore the default early/late adjust value */
					Updator->CurrentEarlyLateAdjust = Updator->DefaultEarlyLateAdjust;
					Updator->EarlyLateAdjustChangeCountdown = 0;
					break;
				case eCmdSetEarlyLateAdjust: /* set the early/late adjust value to <1l> */
					Updator->CurrentEarlyLateAdjust = Note->a.Command.Argument1;
					Updator->EarlyLateAdjustChangeCountdown = 0;
					break;
				case eCmdIncEarlyLateAdjust: /* add <1l> to the current early/late adjust value */
					Updator->CurrentEarlyLateAdjust += Note->a.Command.Argument1;
					Updator->EarlyLateAdjustChangeCountdown = 0;
					break;
				case eCmdSweepEarlyLateAbs: /* <1l> = new early/late adjust, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentEarlyLateAdjust,Updator->EarlyLateAdjustChange,
						&Updator->EarlyLateAdjustChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepEarlyLateRel: /* <1l> = early/late delta, <2xs> = # of beats to get there */
					SweepToAdjustedValue(Updator->CurrentEarlyLateAdjust,Updator->EarlyLateAdjustChange,
						&Updator->EarlyLateAdjustChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;

				case eCmdRestoreDurationAdjust: /* restore the default duration adjust value */
					Updator->CurrentDurationAdjust = Updator->DefaultDurationAdjust;
					Updator->DurationAdjustChangeCountdown = 0;
					break;
				case eCmdSetDurationAdjust: /* set duration adjust value to <1l> */
					Updator->CurrentDurationAdjust = Note->a.Command.Argument1;
					Updator->DurationAdjustChangeCountdown = 0;
					break;
				case eCmdIncDurationAdjust: /* add <1l> to the current duration adjust value */
					Updator->CurrentDurationAdjust += Note->a.Command.Argument1;
					Updator->DurationAdjustChangeCountdown = 0;
					break;
				case eCmdSweepDurationAbs: /* <1l> = new duration adjust, <2xs> = # of beats */
					SweepToNewValue(Updator->CurrentDurationAdjust,Updator->DurationAdjustChange,
						&Updator->DurationAdjustChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdSweepDurationRel: /* <1l> = duration adjust delta, <2xs> = # of beats */
					SweepToAdjustedValue(Updator->CurrentDurationAdjust,Updator->DurationAdjustChange,
						&Updator->DurationAdjustChangeCountdown,Note->a.Command.Argument1,
						Note->a.Command.Argument2);
					break;
				case eCmdDurationAdjustMode: /* <1i>:  -1: Multiplicative, 0: Additive */
					Updator->DurationAdjustAdditive = (Note->a.Command.Argument1 >= 0);
					break;

				case eCmdSetMeter: /* <1i> = numerator, <2i> = denominator */
					break;
				case eCmdSetMeasureNumber: /* <1i> = new number */
					break;

				case eCmdSetTranspose: /* <1i> = new transpose value */
					Updator->TransposeHalfsteps = Note->a.Command.Argument1;
					break;
				case eCmdAdjustTranspose: /* <1i> = transpose adjustor */
					Updator->TransposeHalfsteps += Note->a.Command.Argument1;
					break;

				case eCmdMarker: /* <string> holds the text */
					break;
			}
	}
