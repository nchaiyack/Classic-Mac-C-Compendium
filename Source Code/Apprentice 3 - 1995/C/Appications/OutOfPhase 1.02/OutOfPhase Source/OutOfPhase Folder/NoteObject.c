/* NoteObject.c */
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

#define ShowMe_NoteObjectRec
#include "NoteObject.h"
#include "Memory.h"
#include "Fractions.h"
#include "DrawCommand.h"
#include "DataMunging.h"
#include "Numbers.h"
#include "Frequency.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


/* create a new note with space for the specified number of parameters */
NoteObjectRec*				NewNote(void)
	{
		NoteObjectRec*			Note;

		Note = (NoteObjectRec*)AllocPtrCanFail(sizeof(NoteObjectRec),"NoteObjectRec:Note");
		if (Note == NIL)
			{
				return NIL;
			}
		Note->Flags = /* duration unspecified*/ eRelease1FromDefault | eRelease2FromDefault
			| ePitchDisplacementStartFromDefault | ePitchDisplacementDepthModeDefault
			| eDetuningModeDefault | eDurationAdjustDefault;
		Note->a.Note.Pitch = 0;
		Note->a.Note.PortamentoDuration = 0;
		Note->a.Note.EarlyLateAdjust = 0;
		Note->a.Note.DurationAdjust = 0;
		Note->a.Note.Tie = NIL;
		Note->a.Note.ReleasePoint1 = 0;
		Note->a.Note.ReleasePoint2 = 0;
		Note->a.Note.OverallLoudnessAdjustment = Double2SmallBCD(1);
		Note->a.Note.StereoPositionAdjustment = 0;
		Note->a.Note.SurroundPositionAdjustment = 0;
		Note->a.Note.Accent1 = 0;
		Note->a.Note.Accent2 = 0;
		Note->a.Note.Accent3 = 0;
		Note->a.Note.Accent4 = 0;
		Note->a.Note.MultisamplePitchAsIf = -1;
		Note->a.Note.PitchDisplacementDepthAdjustment = Double2SmallBCD(1);
		Note->a.Note.PitchDisplacementRateAdjustment = Double2SmallBCD(1);
		Note->a.Note.PitchDisplacementStartPoint = 0;
		Note->a.Note.HurryUpFactor = Double2SmallBCD(1);
		Note->a.Note.Detuning = 0;
		return Note;
	}


/* create a new command */
NoteObjectRec*				NewCommand(void)
	{
		NoteObjectRec*			Note;

		Note = (NoteObjectRec*)AllocPtrCanFail(sizeof(NoteObjectRec),"NoteObjectRec:Command");
		if (Note == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Note->a.Command.StringArgument = AllocPtrCanFail(0,"NoteObjectRec:Command:StringArg");
		if (Note->a.Command.StringArgument == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Note);
				goto FailurePoint1;
			}
		Note->a.Command.Argument1 = 0;
		Note->a.Command.Argument2 = 0;
		Note->a.Command.Argument3 = 0;
		Note->Flags = eCommandFlag;
		return Note;
	}


/* dispose of the note */
void									DisposeNote(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		if (IsItACommand(Note))
			{
				ReleasePtr(Note->a.Command.StringArgument);
			}
		ReleasePtr((char*)Note);
	}


/* find out what type of thing it is */
MyBoolean							IsItACommand(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		return ((Note->Flags & eCommandFlag) != 0);
	}


/* convert the duration of the note into a fraction.  the numbers are not reduced */
/* with the aim of eliminating as much factoring as possible to reduce the number */
/* of lengthy multiplies and divides. */
void									GetNoteDurationFrac(NoteObjectRec* Note, struct FractionRec* Frac)
	{
		unsigned long				Temp;

		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteDurationFrac called on command"));
		/* the denominator factors represent the following: */
		/* 64 for 1/64th note, 3*5*7 for divided notes, and 2 for 3/2 dotted notes */
		Frac->Denominator = (64*3*5*7*2);
		switch (Note->Flags & eDurationMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetNoteDurationFrac:  bad duration value"));
					break;
				case e64thNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 64;
					break;
				case e32ndNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 32;
					break;
				case e16thNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 16;
					break;
				case e8thNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 8;
					break;
				case e4thNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 4;
					break;
				case e2ndNote:
					Frac->Integer = 0;
					Frac->Fraction = 1 * (64*3*5*7*2) / 2;
					break;
				case eWholeNote:
					Frac->Integer = 1;
					Frac->Fraction = 0;
					break;
				case eDoubleNote:
					Frac->Integer = 2;
					Frac->Fraction = 0;
					break;
				case eQuadNote:
					Frac->Integer = 4;
					Frac->Fraction = 0;
					break;
			}
		if ((Note->Flags & eDotModifier) != 0)
			{
				/* since the denominator is (64*3*5*7*2), with a 2 in it, we only have */
				/* to multiply note by 3/2 */
				Temp = (3 * (Frac->Fraction + Frac->Integer * (64*3*5*7*2))) / 2;
				Frac->Integer = Temp / (64*3*5*7*2);
				Frac->Fraction = Temp % (64*3*5*7*2);
			}
		switch (Note->Flags & eDivisionMask)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetNoteDurationFrac:  bad division"));
					break;
				case eDiv1Modifier:
					break;
				case eDiv3Modifier:
					/* since the denominator is (64*3*5*7*2), with a 3 in it, we only have */
					/* to multiply note by 1/3 */
					Temp = (Frac->Fraction + Frac->Integer * (64*3*5*7*2)) / 3;
					Frac->Integer = Temp / (64*3*5*7*2);
					Frac->Fraction = Temp % (64*3*5*7*2);
					break;
				case eDiv5Modifier:
					/* since the denominator is (64*3*5*7*2), with a 5 in it, we only have */
					/* to multiply note by 1/5 */
					Temp = (Frac->Fraction + Frac->Integer * (64*3*5*7*2)) / 5;
					Frac->Integer = Temp / (64*3*5*7*2);
					Frac->Fraction = Temp % (64*3*5*7*2);
					break;
				case eDiv7Modifier:
					/* since the denominator is (64*3*5*7*2), with a 7 in it, we only have */
					/* to multiply note by 1/7 */
					Temp = (Frac->Fraction + Frac->Integer * (64*3*5*7*2)) / 7;
					Frac->Integer = Temp / (64*3*5*7*2);
					Frac->Fraction = Temp % (64*3*5*7*2);
					break;
			}
	}


/* draw the command on the screen, or measure how many pixels wide the image will be */
/* if it will draw, it assumes the clipping rectangle to be set up properly */
OrdType								DrawCommandOnScreen(WinType* Window, OrdType X, OrdType Y,
												FontType Font, FontSizeType FontSize, OrdType FontHeight,
												NoteObjectRec* Note, MyBoolean ActuallyDraw, MyBoolean GreyedOut)
	{
		char*								Name;
		CommandAddrMode			Params;
		OrdType							ReturnValue;

		CheckPtrExistence(Note);
		ERROR(!IsItACommand(Note),PRERR(ForceAbort,"DrawCommandOnScreen:  not a command"));
		/* find out what the command's name is */
		Name = GetCommandName((NoteCommands)(Note->Flags & ~eCommandFlag));
		/* figure out what needs to be drawn for the command */
		Params = GetCommandAddressingMode((NoteCommands)(Note->Flags & ~eCommandFlag));
		/* perform the actual drawing of the command */
		switch (Params)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"DrawCommandOnScreen:  bad command addressing mode"));
					break;
				case eNoParameters:
					ReturnValue = DrawCommandNoParams(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name);
					break;
				case e1SmallExtParameter: /* <1xs> */
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,SmallExtBCDToString(Note->a.Command.Argument1));
					break;
				case e2SmallExtParameters: /* <1xs> <2xs> */
					ReturnValue = DrawCommand2Params(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,SmallExtBCDToString(Note->a.Command.Argument1),
						SmallExtBCDToString(Note->a.Command.Argument2));
					break;
				case e1LargeParameter: /* <1l> */
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,LargeBCDToString(Note->a.Command.Argument1));
					break;
				case eFirstLargeSecondSmallExtParameters: /* <1l> <2xs> */
					ReturnValue = DrawCommand2Params(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,LargeBCDToString(Note->a.Command.Argument1),
						SmallExtBCDToString(Note->a.Command.Argument2));
					break;
				case e1ParamReleaseOrigin: /* origin <1i> */
					ERROR((Note->a.Command.Argument1 != 0) && (Note->a.Command.Argument1 != -1),
						PRERR(ForceAbort,"DrawCommandOnScreen:  bad value in argument 1"));
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,StringToBlockCopy((Note->a.Command.Argument1 >= 0)
						? "From End" : "From Start"));
					break;
				case e1PitchDisplacementMode: /* hertz/steps <1i> */
					ERROR((Note->a.Command.Argument1 != 0) && (Note->a.Command.Argument1 != -1),
						PRERR(ForceAbort,"DrawCommandOnScreen:  bad value in argument 1"));
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,StringToBlockCopy((Note->a.Command.Argument1 >= 0)
						? "Half-Steps" : "Hertz"));
					break;
				case e2IntegerParameters: /* <1i> <2i> */
					ReturnValue = DrawCommand2Params(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,IntegerToString(Note->a.Command.Argument1),
						IntegerToString(Note->a.Command.Argument2));
					break;
				case e1DurationAdjustMode: /* multiplicative/additive <1i> */
					ERROR((Note->a.Command.Argument1 != 0) && (Note->a.Command.Argument1 != -1),
						PRERR(ForceAbort,"DrawCommandOnScreen:  bad value in argument 1"));
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,StringToBlockCopy((Note->a.Command.Argument1 >= 0)
						? "Additive" : "Multiplicative"));
					break;
				case e1IntegerParameter: /* <1i> */
					ReturnValue = DrawCommand1Param(Window,X,Y,Font,FontSize,ActuallyDraw,
						GreyedOut,Name,IntegerToString(Note->a.Command.Argument1));
					break;
				case e1StringParameterWithLineFeeds: /* <string> */
					ReturnValue = DrawCommand1ParamWithLineFeeds(Window,X,Y,Font,FontSize,
						ActuallyDraw,GreyedOut,Name,CopyPtr(Note->a.Command.StringArgument));
					break;
			}
		/* report width of thing to caller */
		return ReturnValue;
	}


/* get a static null terminated string literal containing the name of a command */
char*									GetCommandName(NoteCommands Command)
	{
		char*								Name;

		switch (Command)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetCommandName:  unknown command opcode"));
					break;
				case eCmdRestoreTempo:
					Name = "Restore Tempo";
					break;
				case eCmdSetTempo:
					Name = "Set Tempo";
					break;
				case eCmdIncTempo:
					Name = "Adjust Tempo";
					break;
				case eCmdSweepTempoAbs:
					Name = "Sweep Tempo Absolute";
					break;
				case eCmdSweepTempoRel:
					Name = "Sweep Tempo Relative";
					break;
				case eCmdRestoreStereoPosition:
					Name = "Restore Stereo Position";
					break;
				case eCmdSetStereoPosition:
					Name = "Set Stereo Position";
					break;
				case eCmdIncStereoPosition:
					Name = "Adjust Stereo Position";
					break;
				case eCmdSweepStereoAbs:
					Name = "Sweep Stereo Position Absolute";
					break;
				case eCmdSweepStereoRel:
					Name = "Sweep Stereo Position Relative";
					break;
				case eCmdRestoreVolume:
					Name = "Restore Volume";
					break;
				case eCmdRestoreSurroundPosition:
					Name = "Restore Surround Position";
					break;
				case eCmdSetSurroundPosition:
					Name = "Set Surround Position";
					break;
				case eCmdIncSurroundPosition:
					Name = "Adjust Surround Position";
					break;
				case eCmdSweepSurroundAbs:
					Name = "Sweep Surround Position Absolute";
					break;
				case eCmdSweepSurroundRel:
					Name = "Sweep Surround Position Relative";
					break;
				case eCmdSetVolume:
					Name = "Set Volume";
					break;
				case eCmdIncVolume:
					Name = "Adjust Volume";
					break;
				case eCmdSweepVolumeAbs:
					Name = "Sweep Volume Absolute";
					break;
				case eCmdSweepVolumeRel:
					Name = "Sweep Volume Relative";
					break;
				case eCmdRestoreReleasePoint1:
					Name = "Restore Release Point 1";
					break;
				case eCmdSetReleasePoint1:
					Name = "Set Release Point 1";
					break;
				case eCmdIncReleasePoint1:
					Name = "Adjust Release Point 1";
					break;
				case eCmdReleasePointOrigin1:
					Name = "Set Release Point 1 Origin";
					break;
				case eCmdSweepReleaseAbs1:
					Name = "Sweep Release Point 1 Absolute";
					break;
				case eCmdSweepReleaseRel1:
					Name = "Sweep Release Point 1 Relative";
					break;
				case eCmdRestoreReleasePoint2:
					Name = "Restore Release Point 2";
					break;
				case eCmdSetReleasePoint2:
					Name = "Set Release Point 2";
					break;
				case eCmdIncReleasePoint2:
					Name = "Adjust Release Point 2";
					break;
				case eCmdReleasePointOrigin2:
					Name = "Set Release Point 2 Origin";
					break;
				case eCmdSweepReleaseAbs2:
					Name = "Sweep Release Point 2 Absolute";
					break;
				case eCmdSweepReleaseRel2:
					Name = "Sweep Release Point 2 Relative";
					break;
				case eCmdRestoreAccent1:
					Name = "Restore Accent 1";
					break;
				case eCmdSetAccent1:
					Name = "Set Accent 1";
					break;
				case eCmdIncAccent1:
					Name = "Adjust Accent 1";
					break;
				case eCmdSweepAccentAbs1:
					Name = "Sweep Accent 1 Absolute";
					break;
				case eCmdSweepAccentRel1:
					Name = "Sweep Accent 1 Relative";
					break;
				case eCmdRestoreAccent2:
					Name = "Restore Accent 2";
					break;
				case eCmdSetAccent2:
					Name = "Set Accent 2";
					break;
				case eCmdIncAccent2:
					Name = "Adjust Accent 2";
					break;
				case eCmdSweepAccentAbs2:
					Name = "Sweep Accent 2 Absolute";
					break;
				case eCmdSweepAccentRel2:
					Name = "Sweep Accent 2 Relative";
					break;
				case eCmdRestoreAccent3:
					Name = "Restore Accent 3";
					break;
				case eCmdSetAccent3:
					Name = "Set Accent 3";
					break;
				case eCmdIncAccent3:
					Name = "Adjust Accent 3";
					break;
				case eCmdSweepAccentAbs3:
					Name = "Sweep Accent 3 Absolute";
					break;
				case eCmdSweepAccentRel3:
					Name = "Sweep Accent 3 Relative";
					break;
				case eCmdRestoreAccent4:
					Name = "Restore Accent 4";
					break;
				case eCmdSetAccent4:
					Name = "Set Accent 4";
					break;
				case eCmdIncAccent4:
					Name = "Adjust Accent 4";
					break;
				case eCmdSweepAccentAbs4:
					Name = "Sweep Accent 4 Absolute";
					break;
				case eCmdSweepAccentRel4:
					Name = "Sweep Accent 4 Relative";
					break;
				case eCmdRestorePitchDispDepth:
					Name = "Restore Pitch Displacement Depth";
					break;
				case eCmdSetPitchDispDepth:
					Name = "Set Pitch Displacement Depth";
					break;
				case eCmdIncPitchDispDepth:
					Name = "Adjust Pitch Displacement Depth";
					break;
				case eCmdPitchDispDepthMode:
					Name = "Set Pitch Displacement Mode";
					break;
				case eCmdSweepPitchDispDepthAbs:
					Name = "Sweep Pitch Displacement Depth Absolute";
					break;
				case eCmdSweepPitchDispDepthRel:
					Name = "Sweep Pitch Displacement Depth Relative";
					break;
				case eCmdRestorePitchDispRate:
					Name = "Restore Pitch Displacement Rate";
					break;
				case eCmdSetPitchDispRate:
					Name = "Set Pitch Displacement Rate";
					break;
				case eCmdIncPitchDispRate:
					Name = "Adjust Pitch Displacement Rate";
					break;
				case eCmdSweepPitchDispRateAbs:
					Name = "Sweep Pitch Displacement Rate Absolute";
					break;
				case eCmdSweepPitchDispRateRel:
					Name = "Sweep Pitch Displacement Rate Relative";
					break;
				case eCmdRestorePitchDispStart:
					Name = "Restore Pitch Displacement Start Point";
					break;
				case eCmdSetPitchDispStart:
					Name = "Set Pitch Displacement Start Point";
					break;
				case eCmdIncPitchDispStart:
					Name = "Adjust Pitch Displacement Start Point";
					break;
				case eCmdPitchDispStartOrigin:
					Name = "Set Pitch Displacement Start Point Origin";
					break;
				case eCmdSweepPitchDispStartAbs:
					Name = "Sweep Pitch Displacement Start Point Absolute";
					break;
				case eCmdSweepPitchDispStartRel:
					Name = "Sweep Pitch Displacement Start Point Relative";
					break;
				case eCmdRestoreHurryUp:
					Name = "Restore Hurry-Up";
					break;
				case eCmdSetHurryUp:
					Name = "Set Hurry-Up";
					break;
				case eCmdIncHurryUp:
					Name = "Adjust Hurry-Up";
					break;
				case eCmdSweepHurryUpAbs:
					Name = "Sweep Hurry-Up Absolute";
					break;
				case eCmdSweepHurryUpRel:
					Name = "Sweep Hurry-Up Relative";
					break;
				case eCmdRestoreDetune:
					Name = "Restore Detuning";
					break;
				case eCmdSetDetune:
					Name = "Set Detuning";
					break;
				case eCmdIncDetune:
					Name = "Adjust Detuning";
					break;
				case eCmdDetuneMode:
					Name = "Set Detuning Mode";
					break;
				case eCmdSweepDetuneAbs:
					Name = "Sweep Detuning Absolute";
					break;
				case eCmdSweepDetuneRel:
					Name = "Sweep Detuning Relative";
					break;
				case eCmdRestoreEarlyLateAdjust:
					Name = "Restore Hit Time";
					break;
				case eCmdSetEarlyLateAdjust:
					Name = "Set Hit Time";
					break;
				case eCmdIncEarlyLateAdjust:
					Name = "Adjust Hit Time";
					break;
				case eCmdSweepEarlyLateAbs:
					Name = "Sweep Hit Time Absolute";
					break;
				case eCmdSweepEarlyLateRel:
					Name = "Sweep Hit Time Relative";
					break;
				case eCmdRestoreDurationAdjust:
					Name = "Restore Duration";
					break;
				case eCmdSetDurationAdjust:
					Name = "Set Duration";
					break;
				case eCmdIncDurationAdjust:
					Name = "Adjust Duration";
					break;
				case eCmdSweepDurationAbs:
					Name = "Sweep Duration Absolute";
					break;
				case eCmdSweepDurationRel:
					Name = "Sweep Duration Relative";
					break;
				case eCmdDurationAdjustMode:
					Name = "Set Duration Mode";
					break;
				case eCmdSetMeter:
					Name = "Set Meter";
					break;
				case eCmdSetMeasureNumber:
					Name = "Set Measure Number";
					break;
				case eCmdSetTranspose:
					Name = "Set Transpose";
					break;
				case eCmdAdjustTranspose:
					Name = "Adjust Transpose";
					break;
				case eCmdMarker:
					Name = "Marker";
					break;
			}
		return Name;
	}


/* get the addressing mode for a command */
CommandAddrMode				GetCommandAddressingMode(NoteCommands Command)
	{
		CommandAddrMode			Params;

		switch (Command)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetCommandAddressingMode:  unknown command opcode"));
					break;
				case eCmdRestoreTempo: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetTempo: /* <1xs> */
					Params = e1SmallExtParameter;
					break;
				case eCmdIncTempo: /* <1xs> */
					Params = e1SmallExtParameter;
					break;
				case eCmdSweepTempoAbs: /* <1xs> <2xs> */
					Params = e2SmallExtParameters;
					break;
				case eCmdSweepTempoRel: /* <1xs> <2xs> */
					Params = e2SmallExtParameters;
					break;
				case eCmdRestoreStereoPosition: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetStereoPosition: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncStereoPosition: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepStereoAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepStereoRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreSurroundPosition: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetSurroundPosition: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncSurroundPosition: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepSurroundAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepSurroundRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreVolume: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetVolume: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncVolume: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepVolumeAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepVolumeRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreReleasePoint1: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetReleasePoint1: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncReleasePoint1: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdReleasePointOrigin1: /* origin <1i> */
					Params = e1ParamReleaseOrigin;
					break;
				case eCmdSweepReleaseAbs1: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepReleaseRel1: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreReleasePoint2: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetReleasePoint2: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncReleasePoint2: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdReleasePointOrigin2: /* origin <1i> */
					Params = e1ParamReleaseOrigin;
					break;
				case eCmdSweepReleaseAbs2: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepReleaseRel2: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreAccent1: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetAccent1: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncAccent1: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepAccentAbs1: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepAccentRel1: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreAccent2: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetAccent2: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncAccent2: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepAccentAbs2: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepAccentRel2: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreAccent3: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetAccent3: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncAccent3: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepAccentAbs3: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepAccentRel3: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreAccent4: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetAccent4: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncAccent4: /* <1l> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepAccentAbs4: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepAccentRel4: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestorePitchDispDepth: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetPitchDispDepth: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncPitchDispDepth: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdPitchDispDepthMode: /* hertz/steps <1i> */
					Params = e1PitchDisplacementMode;
					break;
				case eCmdSweepPitchDispDepthAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepPitchDispDepthRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestorePitchDispRate: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetPitchDispRate: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncPitchDispRate: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepPitchDispRateAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepPitchDispRateRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestorePitchDispStart: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetPitchDispStart: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncPitchDispStart: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdPitchDispStartOrigin: /* origin <1i> */
					Params = e1ParamReleaseOrigin;
					break;
				case eCmdSweepPitchDispStartAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepPitchDispStartRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreHurryUp: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetHurryUp: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncHurryUp: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepHurryUpAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepHurryUpRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreDetune: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetDetune: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncDetune: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdDetuneMode: /* hertz/steps <1i> */
					Params = e1PitchDisplacementMode;
					break;
				case eCmdSweepDetuneAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepDetuneRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreEarlyLateAdjust: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetEarlyLateAdjust: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncEarlyLateAdjust: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepEarlyLateAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepEarlyLateRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdRestoreDurationAdjust: /* no parameters */
					Params = eNoParameters;
					break;
				case eCmdSetDurationAdjust: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdIncDurationAdjust: /* <1l> */
					Params = e1LargeParameter;
					break;
				case eCmdSweepDurationAbs: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdSweepDurationRel: /* <1l> <2xs> */
					Params = eFirstLargeSecondSmallExtParameters;
					break;
				case eCmdDurationAdjustMode: /* multiplicative/additive <1i> */
					Params = e1DurationAdjustMode;
					break;
				case eCmdSetMeter: /* <1i> <2i> */
					Params = e2IntegerParameters;
					break;
				case eCmdSetMeasureNumber: /* <1i> */
					Params = e1IntegerParameter;
					break;
				case eCmdSetTranspose: /* <1i> */
					Params = e1IntegerParameter;
					break;
				case eCmdAdjustTranspose: /* <1i> */
					Params = e1IntegerParameter;
					break;
				case eCmdMarker: /* <string> */
					Params = e1StringParameterWithLineFeeds;
					break;
			}
		return Params;
	}


/* get the actual string argument contained in a command. NIL == not defined yet */
char*									GetCommandStringArg(NoteObjectRec* Command)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"GetCommandStringArg:  called on note"));
		return Command->a.Command.StringArgument;
	}


/* get the first numeric argument contained in a command */
long									GetCommandNumericArg1(NoteObjectRec* Command)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"GetCommandNumericArg1:  called on note"));
		return Command->a.Command.Argument1;
	}


/* get the second numeric argument contained in a command */
long									GetCommandNumericArg2(NoteObjectRec* Command)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"GetCommandNumericArg2:  called on note"));
		return Command->a.Command.Argument2;
	}


/* get the third numeric argument contained in a command */
long									GetCommandNumericArg3(NoteObjectRec* Command)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"GetCommandNumericArg3:  called on note"));
		return Command->a.Command.Argument3;
	}


/* put a new string into the command.  the command becomes owner of the string. */
void									PutCommandStringArg(NoteObjectRec* Command, char* NewArg)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"PutCommandStringArg:  called on note"));
		CheckPtrExistence(NewArg);
		ReleasePtr(Command->a.Command.StringArgument);
		Command->a.Command.StringArgument = NewArg;
	}


/* put a new first numeric argument into the command */
void									PutCommandNumericArg1(NoteObjectRec* Command, long NewValue)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"PutCommandNumericArg1:  called on note"));
		Command->a.Command.Argument1 = NewValue;
	}


/* put a new second numeric argument into the command */
void									PutCommandNumericArg2(NoteObjectRec* Command, long NewValue)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"PutCommandNumericArg2:  called on note"));
		Command->a.Command.Argument2 = NewValue;
	}


/* put a new third numeric argument into the command */
void									PutCommandNumericArg3(NoteObjectRec* Command, long NewValue)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,
			"PutCommandNumericArg3:  called on note"));
		Command->a.Command.Argument3 = NewValue;
	}


/* get the pitch of a note */
short									GetNotePitch(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNotePitch:  called on command"));
		return Note->a.Note.Pitch;
	}


/* get the portamento transition time for the note */
double								GetNotePortamentoDuration(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePortamentoDuration:  called on command"));
		return SmallBCD2Double(Note->a.Note.PortamentoDuration);
	}


/* get the early/late adjustment factor for a note */
double								GetNoteEarlyLateAdjust(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteEarlyLateAdjust:  called on command"));
		return SmallBCD2Double(Note->a.Note.EarlyLateAdjust);
	}


/* get the duration adjust value for a note */
double								GetNoteDurationAdjust(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteDurationAdjust:  called on command"));
		return SmallBCD2Double(Note->a.Note.DurationAdjust);
	}


/* get a pointer to the note that this note ties to or NIL if there is no tie */
NoteObjectRec*				GetNoteTieTarget(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteTieTarget:  called on command"));
		if (Note->a.Note.Tie != NIL)
			{
				CheckPtrExistence(Note->a.Note.Tie);
			}
		return Note->a.Note.Tie;
	}


/* get the first release point position from a note */
double								GetNoteReleasePoint1(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteReleasePoint1:  called on command"));
		return SmallBCD2Double(Note->a.Note.ReleasePoint1);
	}


/* get the second release point position from a note */
double								GetNoteReleasePoint2(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteReleasePoint2:  called on command"));
		return SmallBCD2Double(Note->a.Note.ReleasePoint2);
	}


/* get the overall loudness factor for the note */
double								GetNoteOverallLoudnessAdjustment(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteOverallLoudnessAdjustment:  called on command"));
		return SmallBCD2Double(Note->a.Note.OverallLoudnessAdjustment);
	}


/* get the stereo positioning for the note */
double								GetNoteStereoPositioning(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteStereoPositioning:  called on command"));
		return SmallBCD2Double(Note->a.Note.StereoPositionAdjustment);
	}


/* get the surround positioning for the note */
double								GetNoteSurroundPositioning(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteSurroundPositioning:  called on command"));
		return SmallBCD2Double(Note->a.Note.SurroundPositionAdjustment);
	}


/* get the first accent factor for the note */
double								GetNoteAccent1(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteAccent1:  called on command"));
		return SmallBCD2Double(Note->a.Note.Accent1);
	}


/* get the second accent factor for the note */
double								GetNoteAccent2(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteAccent2:  called on command"));
		return SmallBCD2Double(Note->a.Note.Accent2);
	}


/* get the third accent factor for the note */
double								GetNoteAccent3(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteAccent3:  called on command"));
		return SmallBCD2Double(Note->a.Note.Accent3);
	}


/* get the fourth accent factor for the note */
double								GetNoteAccent4(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteAccent4:  called on command"));
		return SmallBCD2Double(Note->a.Note.Accent4);
	}


/* get the pitch that the table selector should treat the note as using */
short									GetNoteMultisampleFalsePitch(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteMultisampleFalsePitch:  called on command"));
		return Note->a.Note.MultisamplePitchAsIf;
	}


/* get the pitch displacement depth adjust factor */
double								GetNotePitchDisplacementDepthAdjust(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePitchDisplacementDepthAdjust:  called on command"));
		return SmallBCD2Double(Note->a.Note.PitchDisplacementDepthAdjustment);
	}


/* get the pitch displacement rate adjust factor for the note */
double								GetNotePitchDisplacementRateAdjust(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePitchDisplacementRateAdjust:  called on command"));
		return SmallBCD2Double(Note->a.Note.PitchDisplacementRateAdjustment);
	}


/* get the pitch displacement envelope start point */
double								GetNotePitchDisplacementStartPoint(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePitchDisplacementStartPoint:  called on command"));
		return SmallBCD2Double(Note->a.Note.PitchDisplacementStartPoint);
	}


/* get the hurryup factor for the note */
double								GetNoteHurryUpFactor(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteHurryUpFactor:  called on command"));
		return SmallBCD2Double(Note->a.Note.HurryUpFactor);
	}


/* get the detuning adjustment for the note */
double								GetNoteDetuning(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteDetuning:  called on command"));
		return SmallBCD2Double(Note->a.Note.Detuning);
	}


/* change the pitch of a note */
void									PutNotePitch(NoteObjectRec* Note, short NewPitch)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNotePitch:  called on command"));
		ERROR((NewPitch < 0) || (NewPitch >= NUMNOTES),PRERR(ForceAbort,
			"PutNotePitch:  pitch value is out of range"));
		Note->a.Note.Pitch = NewPitch;
	}


/* get the portamento transition time for the note */
void									PutNotePortamentoDuration(NoteObjectRec* Note,
												double NewPortamentoDuration)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePortamentoDuration:  called on command"));
		Note->a.Note.PortamentoDuration = Double2SmallBCD(NewPortamentoDuration);
	}


/* change the early/late adjustment factor for a note */
void									PutNoteEarlyLateAdjust(NoteObjectRec* Note, double NewEarlyLate)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteEarlyLateAdjust:  called on command"));
		Note->a.Note.EarlyLateAdjust = Double2SmallBCD(NewEarlyLate);
	}


/* change the duration adjust value for a note */
void									PutNoteDurationAdjust(NoteObjectRec* Note,
												double NewDurationAdjust)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteDurationAdjust:  called on command"));
		Note->a.Note.DurationAdjust = Double2SmallBCD(NewDurationAdjust);
	}


/* change the pointer to the note that this note ties to. */
void									PutNoteTieTarget(NoteObjectRec* Note, NoteObjectRec* NewTieTarget)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteTieTarget:  called on command"));
		if (NewTieTarget != NIL)
			{
				CheckPtrExistence(NewTieTarget);
			}
		Note->a.Note.Tie = NewTieTarget;
	}


/* change the first release point position from a note */
void									PutNoteReleasePoint1(NoteObjectRec* Note, double NewReleasePoint1)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteReleasePoint1:  called on command"));
		Note->a.Note.ReleasePoint1 = Double2SmallBCD(NewReleasePoint1);
	}


/* change the second release point position from a note */
void									PutNoteReleasePoint2(NoteObjectRec* Note, double NewReleasePoint2)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteReleasePoint2:  called on command"));
		Note->a.Note.ReleasePoint2 = Double2SmallBCD(NewReleasePoint2);
	}


/* change the overall loudness factor for the note */
void									PutNoteOverallLoudnessAdjustment(NoteObjectRec* Note,
												double NewLoudnessAdjust)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteOverallLoudnessAdjustment:  called on command"));
		Note->a.Note.OverallLoudnessAdjustment = Double2SmallBCD(NewLoudnessAdjust);
	}


/* change the stereo positioning for the note */
void									PutNoteStereoPositioning(NoteObjectRec* Note,
												double NewStereoPosition)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteStereoPositioning:  called on command"));
		Note->a.Note.StereoPositionAdjustment = Double2SmallBCD(NewStereoPosition);
	}


/* change the surround positioning for the note */
void									PutNoteSurroundPositioning(NoteObjectRec* Note,
												double NewSurroundPosition)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteSurroundPositioning:  called on command"));
		Note->a.Note.SurroundPositionAdjustment = Double2SmallBCD(NewSurroundPosition);
	}


/* change the first accent factor for the note */
void									PutNoteAccent1(NoteObjectRec* Note, double NewAccent1)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteAccent1:  called on command"));
		Note->a.Note.Accent1 = Double2SmallBCD(NewAccent1);
	}


/* change the second accent factor for the note */
void									PutNoteAccent2(NoteObjectRec* Note, double NewAccent2)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteAccent2:  called on command"));
		Note->a.Note.Accent2 = Double2SmallBCD(NewAccent2);
	}


/* change the third accent factor for the note */
void									PutNoteAccent3(NoteObjectRec* Note, double NewAccent3)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteAccent3:  called on command"));
		Note->a.Note.Accent3 = Double2SmallBCD(NewAccent3);
	}


/* change the fourth accent factor for the note */
void									PutNoteAccent4(NoteObjectRec* Note, double NewAccent4)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteAccent4:  called on command"));
		Note->a.Note.Accent4 = Double2SmallBCD(NewAccent4);
	}


/* change the pitch that the table selector should treat the note as using */
void									PutNoteMultisampleFalsePitch(NoteObjectRec* Note,
												short NewFalsePitch)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteMultisampleFalsePitch:  called on command"));
		Note->a.Note.MultisamplePitchAsIf = NewFalsePitch;
	}


/* change the pitch displacement depth adjust factor for the note */
void									PutNotePitchDisplacementDepthAdjust(NoteObjectRec* Note,
												double NewPitchDisplacementDepthAdjust)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePitchDisplacementDepthAdjust:  called on command"));
		Note->a.Note.PitchDisplacementDepthAdjustment
			= Double2SmallBCD(NewPitchDisplacementDepthAdjust);
	}


/* change the pitch displacement rate adjust factor for the note */
void									PutNotePitchDisplacementRateAdjust(NoteObjectRec* Note,
												double NewPitchDisplacementRateAdjust)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePitchDisplacementRateAdjust:  called on command"));
		Note->a.Note.PitchDisplacementRateAdjustment
			= Double2SmallBCD(NewPitchDisplacementRateAdjust);
	}


/* change the pitch displacement envelope start point */
void									PutNotePitchDisplacementStartPoint(NoteObjectRec* Note,
												double NewPitchDisplacementStartPoint)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePitchDisplacementStartPoint:  called on command"));
		Note->a.Note.PitchDisplacementStartPoint
			= Double2SmallBCD(NewPitchDisplacementStartPoint);
	}


/* change the hurryup factor for the note */
void									PutNoteHurryUpFactor(NoteObjectRec* Note, double NewHurryUpFactor)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteHurryUpFactor:  called on command"));
		Note->a.Note.HurryUpFactor = Double2SmallBCD(NewHurryUpFactor);
	}


/* change the detuning adjustment for the note */
void									PutNoteDetuning(NoteObjectRec* Note, double NewDetuning)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteDetuning:  called on command"));
		Note->a.Note.Detuning = Double2SmallBCD(NewDetuning);
	}


/* get the opcode for a command */
unsigned long					GetCommandOpcode(NoteObjectRec* Command)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,"GetCommandOpcode:  called on note"));
		return Command->Flags & ~eCommandFlag;
	}


/* put a new opcode in the command */
void									PutCommandOpcode(NoteObjectRec* Command, unsigned long NewOpcode)
	{
		CheckPtrExistence(Command);
		ERROR(!IsItACommand(Command),PRERR(ForceAbort,"PutCommandOpcode:  called on note"));
		ERROR((NewOpcode & eCommandFlag) != 0,PRERR(AllowResume,
			"PutCommandOpcode:  command flag is set on new opcode word"));
		Command->Flags = (Command->Flags & eCommandFlag) | NewOpcode;
	}


/* get the duration of a note */
unsigned long					GetNoteDuration(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteDuration:  called on command"));
		return (Note->Flags & eDurationMask);
	}


/* get the duration division of a note */
unsigned long					GetNoteDurationDivision(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteDurationDivision:  called on command"));
		return (Note->Flags & eDivisionMask);
	}


/* get the dot status of a note */
MyBoolean							GetNoteDotStatus(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteDotStatus:  called on command"));
		return ((Note->Flags & eDotModifier) != 0);
	}


/* get the flat or sharp status of a note */
unsigned long					GetNoteFlatOrSharpStatus(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteFlatOrSharpStatus:  called on command"));
		return (Note->Flags & (eFlatModifier | eSharpModifier));
	}


/* get the rest status of a note */
MyBoolean							GetNoteIsItARest(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"GetNoteIsItARest:  called on command"));
		return ((Note->Flags & eRestModifier) != 0);
	}


/* get the first release point origin of a note */
unsigned long					GetNoteRelease1Origin(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteRelease1Origin:  called on command"));
		return (Note->Flags & eRelease1OriginMask);
	}


/* get the second release point origin of a note */
unsigned long					GetNoteRelease2Origin(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteRelease2Origin:  called on command"));
		return (Note->Flags & eRelease2OriginMask);
	}


/* get the third release from start instead of end flag of a note */
MyBoolean							GetNoteRelease3FromStartInsteadOfEnd(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteRelease3FromStartInsteadOfEnd:  called on command"));
		return ((Note->Flags & eRelease3FromStartNotEnd) != 0);
	}


/* get the pitch displacement origin of a note */
unsigned long					GetNotePitchDisplacementStartOrigin(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePitchDisplacementStartOrigin:  called on command"));
		return (Note->Flags & ePitchDisplacementStartOriginMask);
	}


/* get the pitch displacement depth pitch conversion mode of a note */
unsigned long					GetNotePitchDisplacementDepthConversionMode(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePitchDisplacementDepthConversionMode:  called on command"));
		return (Note->Flags & ePitchDisplacementDepthModeMask);
	}


/* get the detuning pitch conversion mode of a note */
unsigned long					GetNoteDetuneConversionMode(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteDetuneConversionMode:  called on command"));
		return (Note->Flags & eDetuningModeMask);
	}


/* get the retrigger envelope on tie status of a note */
MyBoolean							GetNoteRetriggerEnvelopesOnTieStatus(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteRetriggerEnvelopesOnTieStatus:  called on command"));
		return ((Note->Flags & eRetriggerEnvelopesOnTieFlag) != 0);
	}


/* get the duration adjustment mode of a note */
unsigned long					GetNoteDurationAdjustMode(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteDurationAdjustMode:  called on command"));
		return (Note->Flags & eDurationAdjustMask);
	}


/* get a flag indicating that portamento should use Hertz instead of halfsteps */
MyBoolean							GetNotePortamentoHertzNotHalfstepsFlag(NoteObjectRec* Note)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNotePortamentoHertzNotHalfstepsFlag:  called on command"));
		return ((Note->Flags & ePortamentoHertzNotHalfsteps) != 0);
	}


/* change the duration of a note */
void									PutNoteDuration(NoteObjectRec* Note, unsigned long NewDuration)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteDuration:  called on command"));
		ERROR((NewDuration != e64thNote) && (NewDuration != e32ndNote)
			&& (NewDuration != e16thNote) && (NewDuration != e8thNote)
			&& (NewDuration != e4thNote) && (NewDuration != e2ndNote)
			&& (NewDuration != eWholeNote) && (NewDuration != eDoubleNote)
			&& (NewDuration != eQuadNote),PRERR(ForceAbort,
			"PutNoteDuration:  bad duration value"));
		Note->Flags = (Note->Flags & ~eDurationMask) | NewDuration;
	}


/* change the duration division of a note */
void									PutNoteDurationDivision(NoteObjectRec* Note,
												unsigned long NewDivision)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteDurationDivision:  called on command"));
		ERROR((NewDivision != eDiv1Modifier) && (NewDivision != eDiv3Modifier)
			&& (NewDivision != eDiv5Modifier) && (NewDivision != eDiv7Modifier),
			PRERR(ForceAbort,"PutNoteDurationDivision:  bad division value"));
		Note->Flags = (Note->Flags & ~eDivisionMask) | NewDivision;
	}


/* change the dot status of a note */
void									PutNoteDotStatus(NoteObjectRec* Note, MyBoolean HasADot)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteDotStatus:  called on command"));
		if (HasADot)
			{
				Note->Flags |= eDotModifier;
			}
		 else
			{
				Note->Flags &= ~eDotModifier;
			}
	}


/* change the flat or sharp status of a note */
void									PutNoteFlatOrSharpStatus(NoteObjectRec* Note,
												unsigned long NewFlatOrSharpStatus)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteFlatOrSharpStatus:  called on command"));
		ERROR((NewFlatOrSharpStatus != 0) && (NewFlatOrSharpStatus != eFlatModifier)
			&& (NewFlatOrSharpStatus != eSharpModifier),PRERR(ForceAbort,
			"PutNoteFlatOrSharpStatus:  bad sharp or flat status"));
		Note->Flags = (Note->Flags & ~(eSharpModifier | eFlatModifier))
			| NewFlatOrSharpStatus;
	}


/* change the rest status of a note */
void									PutNoteIsItARest(NoteObjectRec* Note, MyBoolean IsARest)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,"PutNoteIsItARest:  called on command"));
		if (IsARest)
			{
				Note->Flags |= eRestModifier;
			}
		 else
			{
				Note->Flags &= ~eRestModifier;
			}
	}


/* change the first release point origin of a note */
void									PutNoteRelease1Origin(NoteObjectRec* Note,
												unsigned long NewReleasePoint1Origin)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteRelease1Origin:  called on command"));
		ERROR((NewReleasePoint1Origin != eRelease1FromDefault)
			&& (NewReleasePoint1Origin != eRelease1FromStart)
			&& (NewReleasePoint1Origin != eRelease1FromEnd),PRERR(ForceAbort,
			"PutNoteRelease1Origin:  bad origin value"));
		Note->Flags = (Note->Flags & ~eRelease1OriginMask) | NewReleasePoint1Origin;
	}


/* change the second release point origin of a note */
void									PutNoteRelease2Origin(NoteObjectRec* Note,
												unsigned long NewReleasePoitn2Origin)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteRelease2Origin:  called on command"));
		ERROR((NewReleasePoitn2Origin != eRelease2FromDefault)
			&& (NewReleasePoitn2Origin != eRelease2FromStart)
			&& (NewReleasePoitn2Origin != eRelease2FromEnd),PRERR(ForceAbort,
			"PutNoteRelease2Origin:  bad origin value"));
		Note->Flags = (Note->Flags & ~eRelease2OriginMask) | NewReleasePoitn2Origin;
	}


/* change the third release from start instead of end flag of a note */
void									PutNoteRelease3FromStartInsteadOfEnd(NoteObjectRec* Note,
												MyBoolean ShouldWeReleasePoint3FromStartInsteadOfEnd)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteRelease3FromStartInsteadOfEnd:  called on command"));
		if (ShouldWeReleasePoint3FromStartInsteadOfEnd)
			{
				Note->Flags |= eRelease3FromStartNotEnd;
			}
		 else
			{
				Note->Flags &= ~eRelease3FromStartNotEnd;
			}
	}


/* change the pitch displacement origin of a note */
void									PutNotePitchDisplacementStartOrigin(NoteObjectRec* Note,
												unsigned long NewPitchDisplacementStartOrigin)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePitchDisplacementStartOrigin:  called on command"));
		ERROR((NewPitchDisplacementStartOrigin != ePitchDisplacementStartFromDefault)
			&& (NewPitchDisplacementStartOrigin != ePitchDisplacementStartFromStart)
			&& (NewPitchDisplacementStartOrigin != ePitchDisplacementStartFromEnd),
			PRERR(ForceAbort,"PutNotePitchDisplacementStartOrigin:  bad origin value"));
		Note->Flags = (Note->Flags & ~ePitchDisplacementStartOriginMask)
			| NewPitchDisplacementStartOrigin;
	}


/* change the pitch displacement depth pitch conversion mode of a note */
void									PutNotePitchDisplacementDepthConversionMode(NoteObjectRec* Note,
												unsigned long NewPitchDisplacementDepthConversionMode)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePitchDisplacementDepthConversionMode:  called on command"));
		ERROR((NewPitchDisplacementDepthConversionMode != ePitchDisplacementDepthModeDefault)
			&& (NewPitchDisplacementDepthConversionMode != ePitchDisplacementDepthModeHalfSteps)
			&& (NewPitchDisplacementDepthConversionMode != ePitchDisplacementDepthModeHertz),
			PRERR(ForceAbort,"PutNotePitchDisplacementDepthConversionMode:  bad mode value"));
		Note->Flags = (Note->Flags & ~ePitchDisplacementDepthModeMask)
			| NewPitchDisplacementDepthConversionMode;
	}


/* change the detuning pitch conversion mode of a note */
void									PutNoteDetuneConversionMode(NoteObjectRec* Note,
												unsigned long NewDetuneConversionMode)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteDetuneConversionMode:  called on command"));
		ERROR((NewDetuneConversionMode != eDetuningModeDefault)
			&& (NewDetuneConversionMode != eDetuningModeHalfSteps)
			&& (NewDetuneConversionMode != eDetuningModeHertz),PRERR(ForceAbort,
			"PutNoteDetuneConversionMode:  bad mode value"));
		Note->Flags = (Note->Flags & ~eDetuningModeMask) | NewDetuneConversionMode;
	}


/* change the retrigger envelope on tie status of a note */
void									PutNoteRetriggerEnvelopesOnTieStatus(NoteObjectRec* Note,
												MyBoolean ShouldWeRetriggerEnvelopesOnTie)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNoteRetriggerEnvelopesOnTieStatus:  called on command"));
		if (ShouldWeRetriggerEnvelopesOnTie)
			{
				Note->Flags |= eRetriggerEnvelopesOnTieFlag;
			}
		 else
			{
				Note->Flags &= ~eRetriggerEnvelopesOnTieFlag;
			}
	}


/* change the duration adjustment mode of a note */
void									PutNoteDurationAdjustMode(NoteObjectRec* Note,
												unsigned long NewDurationAdjustMode)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"GetNoteDurationAdjustMode:  called on command"));
		ERROR((NewDurationAdjustMode != eDurationAdjustDefault)
			&& (NewDurationAdjustMode != eDurationAdjustAdditive)
			&& (NewDurationAdjustMode != eDurationAdjustMultiplicative),PRERR(ForceAbort,
			"PutNoteDurationAdjustMode:  bad value"));
		Note->Flags = (Note->Flags & ~eDurationAdjustMask) | NewDurationAdjustMode;
	}


/* change the flag indicating that portamento should use Hertz instead of halfsteps */
void									PutNotePortamentoHertzNotHalfstepsFlag(NoteObjectRec* Note,
												MyBoolean ShouldWeUseHertzInsteadOfHalfsteps)
	{
		CheckPtrExistence(Note);
		ERROR(IsItACommand(Note),PRERR(ForceAbort,
			"PutNotePortamentoHertzNotHalfstepsFlag:  called on command"));
		if (ShouldWeUseHertzInsteadOfHalfsteps)
			{
				Note->Flags |= ePortamentoHertzNotHalfsteps;
			}
		 else
			{
				Note->Flags &= ~ePortamentoHertzNotHalfsteps;
			}
	}


/* Note Object Subblock Format: */
/* Each note/command has the following field: */
/*   4-byte unsigned little endian opcode field */
/*       for a command, the high bit will be 1 and the remaining bits will */
/*       be the opcode.  for a note, the high bit will be 0. */

/* Remainder of Note Variant Subblock: */
/*   definitions for the remaining bits in the opcode field */
/*       bits 0-3:  duration integer. */
/*         0001 = 64th note */
/*         0010 = 32nd note */
/*         0011 = 16th note */
/*         0100 = 8th note */
/*         0101 = quarter note */
/*         0110 = half note */
/*         0111 = whole note */
/*         1000 = double note */
/*         1001 = quad note */
/*         all other values are not permitted */
/*       bits 4-5:  division mask */
/*         00 = divide duration by 1 (no change in duration) */
/*         01 = divide duration by 3 (triplets) */
/*         10 = divide duration by 5 */
/*         11 = divide duration by 7 */
/*       bit 6:  dotted note flag (1 = dot) */
/*       bit 7:  flat modifier (1 = flat);  this is used for notation only */
/*         at most 1 of the flat or sharp modifiers can be set */
/*       bit 8:  sharp modifier (1 = sharp);  this is used for notation only */
/*         at most 1 of the flat or sharp modifiers can be set */
/*       bit 9:  rest modifier (1 = rest instead of note) */
/*       bits 10-11:  release point 1 origin */
/*         01 = use default origin */
/*         10 = measure release point from start of note */
/*         11 = measure release point from end of note */
/*         other values are not permitted */
/*       bits 12-13:  release point 2 origin */
/*         01 = use default origin */
/*         10 = measure release point from start of note */
/*         11 = measure release point from end of note */
/*         other values are not permitted */
/*       bit 14:  release point 3 origin (1 = from start instead of end) */
/*         0 = release 3 occurs at end of note */
/*         1 = release 3 occurs at start of note */
/*       bits 15-16:  pitch displacement start point origin */
/*         01 = use default origin */
/*         10 = pitch displacement start point from start of note */
/*         11 = pitch displacement start point from end of note */
/*         other values are not permitted */
/*       bits 17-18:  pitch displacement depth modulation mode */
/*         01 = use default pitch modulation scale */
/*         10 = treat modulation index as half-step number */
/*         11 = treat modulation index as hertz */
/*         other values are not permitted */
/*       bits 19-20:  detuning mode */
/*         01 = use default treat detuning scale */
/*         10 = treat detuning value as half-step number */
/*         11 = treat detuning value as hertz */
/*         other values are not permitted */
/*       bits 21-22:  note duration adjustment scale */
/*         01 = use default duration adjustment scale */
/*         10 = add duration adjustment to the duration */
/*         11 = multiply the duration adjustment by the duration */
/*         other values are not permitted */
/*       bit 23:  retrigger envelopes on tie flag */
/*       bit 24:  portamento linear in hertz instead of half steps (1 = hertz) */
/*       bits 25-31:  set to zero! */
/*   2-byte signed little endian pitch index */
/*       should be a value in the range 0..383.  Middle C (261.6 Hertz) = 192 */
/*   2-byte little endian small integer coded decimal portamento duration. */
/*       this determines how long a portamento will last, in fractions of a quarter */
/*       note.  it only has effect if the note is the target of a tie.  a value of */
/*       0 means instantaneous, i.e. no portamento. */
/*       A small integer coded decimal is the decimal * 1000 with a range */
/*       of -29.999 to 29.999 */
/*   2-byte little endian small integer coded decimal early/late adjustment */
/*       this determines the displacement in time of the occurrence of the note */
/*       in frations of a quarter note. */
/*   2-byte little endian small integer coded decimal duration adjustment */
/*       this value changes the duration of the note by being added to the */
/*       duration or being multiplied by the duration. */
/*   2-byte little endian small integer coded decimal release point 1 location */
/*       this determines when the release of the first sustain/loop will occur */
/*       in fractions of the current note's duration.  it is relative to the origin */
/*       as determined by the opcode field. */
/*   2-byte little endian small integer coded decimal release point 2 location */
/*       this determines when the release of the second sustain/loop will occur. */
/*   2-byte little endian small integer coded decimal overall loudness adjustment */
/*       this factor scales the total volume output of the oscillators for this */
/*       particular note.  It is multiplied, so a value of 1 makes no change in */
/*       loudness. */
/*   2-byte little endian small integer coded decimal stereo position adjustment. */
/*       this value adjusts where the sound will be located in stereo.  -1 is */
/*       the far left, 1 is the far right, and 0 is center. */
/*   2-byte little endian small integer coded decimal surround position adjustment. */
/*       this value adjusts where the sound will be located in surround sound.  */
/*       1 is front and -1 is rear. */
/*   2-byte little endian small integer coded decimal accent 1 value */
/*   2-byte little endian small integer coded decimal accent 2 value */
/*   2-byte little endian small integer coded decimal accent 3 value */
/*   2-byte little endian small integer coded decimal accent 4 value */
/*   2-byte little endian fake pitch value */
/*       this value has a range of -1..383.  If it is not -1, then it will be used */
/*       to determine which sample a multisampled oscillator will use.  If it is -1 */
/*       then the actual pitch will be used to select a sample. */
/*   2-byte little endian small integer coded decimal pitch disp depth adjustment */
/*       this adjusts the maximum amplitude of the pitch displacement depth */
/*       oscillator (vibrato).  The value has units of either half steps or hertz */
/*       depending on the setting in the opcode word. */
/*   2-byte little endian small integer coded decimal pitch displ rate adjustment */
/*       this adjusts the maximum amplitude of the pitch displacement rate */
/*       oscillator. the units are periods per second. */
/*   2-byte little endian small integer coded decimal pitch displ start point adjust */
/*       this value adjusts when the pitch displacement envelopes start.  the */
/*       location is from start or end of note, depending on the opcode settings, and */
/*       is in fractions of the current note's duration. */
/*   2-byte little endian small integer coded decimal hurry-up factor */
/*       this factor scales the total speed at which all envelopes change.  this is */
/*       multiplicative, so a value of 1 makes no change, and smaller values make */
/*       transitions go faster. */
/*   2-byte little endian small integer coded decimal detuning value */
/*       this value is added to the pitch of the note to detune.  its units are */
/*       either hertz or half steps depending on the opcode word. */

/* Remainder of Command Variant Subblock: */
/*   lower 31 bits of the command opcode: */
/*        16 = restore tempo */
/*        17 = set tempo */
/*        18 = adjust tempo */
/*        19 = sweep tempo absolute */
/*        20 = sweep tempo relative */
/*       32 = restore stereo position */
/*       33 = set stereo position */
/*       34 = adjust stereo position */
/*       35 = sweep stereo position absolute */
/*       36 = sweep stereo position relative */
/*        48 = restore volume */
/*        49 = set volume */
/*        50 = adjust volume */
/*        51 = sweep volume absolute */
/*        52 = sweep volume relative */
/*       64 = restore release point 1 */
/*       65 = set release point 1 */
/*       66 = adjust release point 1 */
/*       67 = set release point 1 origin */
/*       68 = sweep release point 1 absolute */
/*       69 = sweep release point 1 relative */
/*        80 = restore release point 2 */
/*        81 = set release point 2 */
/*        82 = adjust release point 2 */
/*        83 = set release point 2 origin */
/*        84 = sweep release point 2 absolute */
/*        85 = sweep release point 2 relative */
/*       96 = restore accent 1 */
/*       97 = set accent 1 */
/*       98 = adjust accent 1 */
/*       99 = sweep accent 1 absolute */
/*       100 = sweep accent 1 relative */
/*        112 = restore accent 2 */
/*        113 = set accent 2 */
/*        114 = adjust accent 2 */
/*        115 = sweep accent 2 absolute */
/*        116 = sweep accent 2 relative */
/*       128 = restore accent 3 */
/*       129 = set accent 3 */
/*       130 = adjust accent 3 */
/*       131 = sweep accent 3 absolute */
/*       132 = sweep accent 3 relative */
/*        144 = restore accent 4 */
/*        145 = set accent 4 */
/*        146 = adjust accent 4 */
/*        147 = sweep accent 4 absolute */
/*        148 = sweep accent 4 relative */
/*       160 = restore pitch displacement depth */
/*       161 = set pitch displacement depth */
/*       162 = adjust pitch displacement depth */
/*       163 = set pitch displacement depth modulation mode */
/*       164 = sweep pitch displacement depth absolute */
/*       165 = sweep pitch displacement depth relative */
/*        176 = restore pitch displacement rate */
/*        177 = set pitch displacement rate */
/*        178 = adjust pitch displacement rate */
/*        179 = sweep pitch displacement rate absolute */
/*        180 = sweep pitch displacement rate relative */
/*       192 = restore pitch displacement start point */
/*       193 = set pitch displacement start point */
/*       194 = adjust pitch displacement start point */
/*       195 = set pitch displacement start point origin */
/*       196 = sweep pitch displacement start point absolute */
/*       197 = sweep pitch displacement start point relative */
/*        208 = restore hurry-up factor */
/*        209 = set hurry-up factor */
/*        210 = adjust hurry-up factor */
/*        211 = sweep hurry-up factor absolute */
/*        212 = sweep hurry-up factor relative */
/*       224 = restore detuning */
/*       225 = set detuning */
/*       226 = adjust detuning */
/*       227 = set detuning mode */
/*       228 = sweep detuning absolute */
/*       229 = sweep detuning relative */
/*        240 = restore early/late adjust */
/*        241 = set early/late adjust */
/*        242 = adjust early/late adjust */
/*        243 = sweep early/late adjust absolute */
/*        244 = sweep early/late adjust relative */
/*       256 = restore duration adjust */
/*       257 = set duration adjust */
/*       258 = adjust duration adjust */
/*       259 = sweep duration adjust absolute */
/*       260 = sweep duration adjust relative */
/*       261 = set duration adjust mode */
/*        272 = set meter */
/*        273 = set measure number */
/*       288 = comment */
/*        304 = restore surround position */
/*        305 = set surround position */
/*        306 = adjust surround position */
/*        307 = sweep surround position absolute */
/*        308 = sweep surround position relative */
/*       320 = set transpose */
/*       321 = adjust transpose */
/*       no other values besides these are allowed! */
/* Format of data for each command (not including the 4-byte opcode word): */
/* 16 = restore tempo */
/*   no arguments */
/* 17 = set tempo */
/*   4-byte little endian extended integer coded decimal number of beats per minute */
/*       an extended integer coded decimal is 1000 * the decimal value, with */
/*       a range of -1999999.999 to 1999999.999 */
/* 18 = adjust tempo */
/*   4-byte little endian extended integer coded decimal beats per minute adjust */
/* 19 = sweep tempo absolute */
/*   4-byte little endian extended integer coded decimal target beats per minute */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 20 = sweep tempo relative */
/*   4-byte little endian extended integer coded decimal target beats per minute adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 32 = restore stereo position */
/*   no arguments */
/* 33 = set stereo position */
/*   4-byte little endian large integer coded decimal stereo position value */
/*       -1 is left, 1 is right, 0 is center */
/* 34 = adjust stereo position */
/*   4-byte little endian large integer coded decimal stereo position adjustment */
/* 35 = sweep stereo position absolute */
/*   4-byte little endian large integer coded decimal target stereo position */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 36 = sweep stereo position relative */
/*   4-byte little endian large integer coded decimal target stereo position adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 48 = restore volume */
/*   no arguments */
/* 49 = set volume */
/*   4-byte little endian large integer coded decimal volume level specifier */
/* 50 = adjust volume */
/*   4-byte little endian large integer coded decimal volume level adjustment */
/* 51 = sweep volume absolute */
/*   4-byte little endian large integer coded decimal target volume level */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 52 = sweep volume relative */
/*   4-byte little endian large integer coded decimal target volume level adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 64 = restore release point 1 */
/*   no arguments */
/* 65 = set release point 1 */
/*   4-byte little endian large integer coded decimal release point */
/* 66 = adjust release point 1 */
/*   4-byte little endian large integer coded decimal release point adjust */
/* 67 = set release point 1 origin */
/*   1-byte origin specifier */
/*       0 = from start */
/*       1 = from end */
/* 68 = sweep release point 1 absolute */
/*   4-byte little endian large integer coded decimal target release point */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 69 = sweep release point 1 relative */
/*   4-byte little endian large integer coded decimal target release point adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 80 = restore release point 2 */
/*   no arguments */
/* 81 = set release point 2 */
/*   4-byte little endian large integer coded decimal release point */
/* 82 = adjust release point 2 */
/*   4-byte little endian large integer coded decimal release point adjust */
/* 83 = set release point 2 origin */
/*   1-byte origin specifier */
/*       0 = from start */
/*       1 = from end */
/* 84 = sweep release point 2 absolute */
/*   4-byte little endian large integer coded decimal target release point */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 85 = sweep release point 2 relative */
/*   4-byte little endian large integer coded decimal target release point adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 96 = restore accent 1 */
/*   no arguments */
/* 97 = set accent 1 */
/*   4-byte little endian large integer coded decimal accent value */
/* 98 = adjust accent 1 */
/*   4-byte little endian large integer coded decimal accent adjust */
/* 99 = sweep accent 1 absolute */
/*   4-byte little endian large integer coded decimal target accent */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 100 = sweep accent 1 relative */
/*   4-byte little endian large integer coded decimal target accent adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 112 = restore accent 2 */
/*   no arguments */
/* 113 = set accent 2 */
/*   4-byte little endian large integer coded decimal accent value */
/* 114 = adjust accent 2 */
/*   4-byte little endian large integer coded decimal accent adjust */
/* 115 = sweep accent 2 absolute */
/*   4-byte little endian large integer coded decimal target accent */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 116 = sweep accent 2 relative */
/*   4-byte little endian large integer coded decimal target accent adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 128 = restore accent 3 */
/*   no arguments */
/* 129 = set accent 3 */
/*   4-byte little endian large integer coded decimal accent value */
/* 130 = adjust accent 3 */
/*   4-byte little endian large integer coded decimal accent adjust */
/* 131 = sweep accent 3 absolute */
/*   4-byte little endian large integer coded decimal target accent */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 132 = sweep accent 3 relative */
/*   4-byte little endian large integer coded decimal target accent adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 144 = restore accent 4 */
/*   no arguments */
/* 145 = set accent 4 */
/*   4-byte little endian large integer coded decimal accent value */
/* 146 = adjust accent 4 */
/*   4-byte little endian large integer coded decimal accent adjust */
/* 147 = sweep accent 4 absolute */
/*   4-byte little endian large integer coded decimal target accent */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 148 = sweep accent 4 relative */
/*   4-byte little endian large integer coded decimal target accent adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 160 = restore pitch displacement depth */
/*   no arguments */
/* 161 = set pitch displacement depth */
/*   4-byte little endian large integer coded decimal pitch disp depth */
/* 162 = adjust pitch displacement depth */
/*   4-byte little endian large integer coded decimal pitch disp depth adjust */
/* 163 = set pitch displacement depth modulation mode */
/*   1-byte mode flag */
/*       0 = hertz */
/*       1 = half steps */
/* 164 = sweep pitch displacement depth absolute */
/*   4-byte little endian large integer coded decimal target pitch disp depth */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 165 = sweep pitch displacement depth relative */
/*   4-byte little endian large integer coded decimal target pitch disp depth adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 176 = restore pitch displacement rate */
/*   no arguments */
/* 177 = set pitch displacement rate */
/*   4-byte little endian large integer coded decimal pitch disp rate */
/* 178 = adjust pitch displacement rate */
/*   4-byte little endian large integer coded decimal pitch disp rate adjust */
/* 179 = sweep pitch displacement rate absolute */
/*   4-byte little endian large integer coded decimal target pitch disp rate */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 180 = sweep pitch displacement rate relative */
/*   4-byte little endian large integer coded decimal target pitch disp rate adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 192 = restore pitch displacement start point */
/*   no arguments */
/* 193 = set pitch displacement start point */
/*   4-byte little endian large integer coded decimal pitch disp start point */
/* 194 = adjust pitch displacement start point */
/*   4-byte little endian large integer coded decimal pitch disp start point adjust */
/* 195 = set pitch displacement start point origin */
/*   1-byte start point origin */
/*       0 = from start */
/*       1 = from end */
/* 196 = sweep pitch displacement start point absolute */
/*   4-byte little endian large integer coded decimal target pitch disp start point */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 197 = sweep pitch displacement start point relative */
/*   4-byte little endian large integer coded decimal target pitch disp start adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 208 = restore hurry-up factor */
/*   no arguments */
/* 209 = set hurry-up factor */
/*   4-byte little endian large integer coded decimal hurry-up factor */
/* 210 = adjust hurry-up factor */
/*   4-byte little endian large integer coded decimal hurry-up factor adjust */
/* 211 = sweep hurry-up factor absolute */
/*   4-byte little endian large integer coded decimal target hurry-up factor */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 212 = sweep hurry-up factor relative */
/*   4-byte little endian large integer coded decimal target hurry-up factor adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 224 = restore detuning */
/*   no arguments */
/* 225 = set detuning */
/*   4-byte little endian large integer coded decimal detuning */
/* 226 = adjust detuning */
/*   4-byte little endian large integer coded decimal detuning adjust */
/* 227 = set detuning mode */
/*   1-byte detuning mode specifier */
/*       0 = hertz */
/*       1 = half steps */
/* 228 = sweep detuning absolute */
/*   4-byte little endian large integer coded decimal target detuning */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 229 = sweep detuning relative */
/*   4-byte little endian large integer coded decimal target detuning adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 240 = restore early/late adjust */
/*   no arguments */
/* 241 = set early/late adjust */
/*   4-byte little endian large integer coded decimal early/late adjust */
/* 242 = adjust early/late adjust */
/*   4-byte little endian large integer coded decimal early/late adjust adjustment */
/* 243 = sweep early/late adjust absolute */
/*   4-byte little endian large integer coded decimal target early/late adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 244 = sweep early/late adjust relative */
/*   4-byte little endian large integer coded decimal target early/late adjust adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 256 = restore duration adjust */
/*   no arguments */
/* 257 = set duration adjust */
/*   4-byte little endian large integer coded decimal duration adjust */
/* 258 = adjust duration adjust */
/*   4-byte little endian large integer coded decimal duration adjust adjustment */
/* 259 = sweep duration adjust absolute */
/*   4-byte little endian large integer coded decimal target duration adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 260 = sweep duration adjust relative */
/*   4-byte little endian large integer coded decimal target duration adjust adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 261 = set duration adjust mode */
/*   1 byte mode flag */
/*       0 = multiplicative */
/*       1 = additive */
/* 272 = set meter */
/*   4-byte little endian numerator */
/*   4-byte little endian denominator */
/* 273 = set measure number */
/*   4-byte little endian new measure number */
/* 288 = comment */
/*   4-byte little endian length of comment text string */
/*   n-byte comment text string (line feed = 0x0a) */
/* 304 = restore surround position */
/*   no arguments */
/* 305 = set surround position */
/*   4-byte little endian large integer coded decimal surround position value */
/*       1 is front, 0 is middle, -1 is rear */
/* 306 = adjust surround position */
/*   4-byte little endian large integer coded decimal surround position adjustment */
/* 307 = sweep surround position absolute */
/*   4-byte little endian large integer coded decimal target surround position */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 308 = sweep surround position relative */
/*   4-byte little endian large integer coded decimal target surround position adjust */
/*   4-byte little endian extended integer coded decimal number of beats to reach it */
/* 320 = set transpose */
/*   4-byte signed little endian half-step change (<0 decreases pitch) */
/* 321 = adjust transpose */
/*   4-byte signed little endian transpose adjustment */


/* read a note object in from a file.  this does not handle ties since they */
/* are done separately. */
FileLoadingErrors			NoteObjectNewFromFile(NoteObjectRec** ObjectOut,
												struct BufferedInputRec* Input)
	{
		NoteObjectRec*			Note;
		FileLoadingErrors		Error;

		CheckPtrExistence(Input);

		Note = (NoteObjectRec*)AllocPtrCanFail(sizeof(NoteObjectRec),"NoteObjectRec");
		if (Note == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint1:
				return Error;
			}

		/*   4-byte unsigned little endian opcode field */
		/*       for a command, the high bit will be 1 and the remaining bits will */
		/*       be the opcode.  for a note, the high bit will be 0. */
		if (!ReadBufferedUnsignedLongLittleEndian(Input,&(Note->Flags)))
			{
				Error = eFileLoadDiskError;
			 FailurePoint2:
				ReleasePtr((char*)Note);
				goto FailurePoint1;
			}

		if ((Note->Flags & eCommandFlag) == 0)
			{
				signed short				SignedShort;

				/* it's a note */

				/* check opcode flags for validity */
				if (
						(((Note->Flags & eDurationMask) != e64thNote)
						&& ((Note->Flags & eDurationMask) != e32ndNote)
						&& ((Note->Flags & eDurationMask) != e16thNote)
						&& ((Note->Flags & eDurationMask) != e8thNote)
						&& ((Note->Flags & eDurationMask) != e4thNote)
						&& ((Note->Flags & eDurationMask) != e2ndNote)
						&& ((Note->Flags & eDurationMask) != eWholeNote)
						&& ((Note->Flags & eDurationMask) != eDoubleNote)
						&& ((Note->Flags & eDurationMask) != eQuadNote))
					||
						(((Note->Flags & eFlatModifier) != 0)
						&& ((Note->Flags & eSharpModifier) != 0))
					||
						(((Note->Flags & eRelease1OriginMask) != eRelease1FromDefault)
						&& ((Note->Flags & eRelease1OriginMask) != eRelease1FromStart)
						&& ((Note->Flags & eRelease1OriginMask) != eRelease1FromEnd))
					||
						(((Note->Flags & eRelease2OriginMask) != eRelease2FromDefault)
						&& ((Note->Flags & eRelease2OriginMask) != eRelease2FromStart)
						&& ((Note->Flags & eRelease2OriginMask) != eRelease2FromEnd))
					||
						(((Note->Flags & ePitchDisplacementStartOriginMask)
							!= ePitchDisplacementStartFromDefault)
						&& ((Note->Flags & ePitchDisplacementStartOriginMask)
							!= ePitchDisplacementStartFromStart)
						&& ((Note->Flags & ePitchDisplacementStartOriginMask)
							!= ePitchDisplacementStartFromEnd))
					||
						(((Note->Flags & ePitchDisplacementDepthModeMask)
							!= ePitchDisplacementDepthModeDefault)
						&& ((Note->Flags & ePitchDisplacementDepthModeMask)
							!= ePitchDisplacementDepthModeHalfSteps)
						&& ((Note->Flags & ePitchDisplacementDepthModeMask)
							!= ePitchDisplacementDepthModeHertz))
					||
						(((Note->Flags & eDetuningModeMask) != eDetuningModeDefault)
						&& ((Note->Flags & eDetuningModeMask) != eDetuningModeHalfSteps)
						&& ((Note->Flags & eDetuningModeMask) != eDetuningModeHertz))
					||
						(((Note->Flags & eDurationAdjustMask) != eDurationAdjustDefault)
						&& ((Note->Flags & eDurationAdjustMask) != eDurationAdjustAdditive)
						&& ((Note->Flags & eDurationAdjustMask) != eDurationAdjustMultiplicative)))
					{
					 FailurePointNoteBadFormat:
						Error = eFileLoadBadFormat;
						goto FailurePoint2;
					}

				/*   2-byte signed little endian pitch index */
				/*       should be a value in the range 0..383.  Middle C (261.6 Hertz) = 192 */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
					 FailurePointNoteDiskError:
						Error = eFileLoadDiskError;
						goto FailurePoint2;
					}
				if ((SignedShort < 0) || (SignedShort >= NUMNOTES))
					{
						goto FailurePointNoteBadFormat;
					}
				Note->a.Note.Pitch = SignedShort;

				/*   2-byte little endian small integer coded decimal portamento duration. */
				/*       this determines how long a portamento will last, in fractions of a quarter */
				/*       note.  it only has effect if the note is the target of a tie.  a value of */
				/*       0 means instantaneous, i.e. no portamento. */
				/*       A small integer coded decimal is the decimal * 1000 with a range */
				/*       of -29.999 to 29.999 */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.PortamentoDuration = SignedShort;

				/*   2-byte little endian small integer coded decimal early/late adjustment */
				/*       this determines the displacement in time of the occurrence of the note */
				/*       in frations of a quarter note. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.EarlyLateAdjust = SignedShort;

				/*   2-byte little endian small integer coded decimal duration adjustment */
				/*       this value changes the duration of the note by being added to the */
				/*       duration or being multiplied by the duration. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.DurationAdjust = SignedShort;

				Note->a.Note.Tie = NIL;

				/*   2-byte little endian small integer coded decimal release point 1 location */
				/*       this determines when the release of the first sustain/loop will occur */
				/*       in fractions of the current note's duration.  it is relative to the origin */
				/*       as determined by the opcode field. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.ReleasePoint1 = SignedShort;

				/*   2-byte little endian small integer coded decimal release point 2 location */
				/*       this determines when the release of the second sustain/loop will occur. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.ReleasePoint2 = SignedShort;

				/*   2-byte little endian small integer coded decimal overall loudness adjustment */
				/*       this factor scales the total volume output of the oscillators for this */
				/*       particular note.  It is multiplied, so a value of 1 makes no change in */
				/*       loudness. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.OverallLoudnessAdjustment = SignedShort;

				/*   2-byte little endian small integer coded decimal stereo position adjustment. */
				/*       this value adjusts where the sound will be located in stereo.  -1 is */
				/*       the far left, 1 is the far right, and 0 is center. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.StereoPositionAdjustment = SignedShort;

				/*   2-byte little endian small integer coded decimal surround position adjustment. */
				/*       this value adjusts where the sound will be located in surround. */
				/*       1 is front and -1 is rear. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.SurroundPositionAdjustment = SignedShort;

				/*   2-byte little endian small integer coded decimal accent 1 value */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.Accent1 = SignedShort;

				/*   2-byte little endian small integer coded decimal accent 2 value */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.Accent2 = SignedShort;

				/*   2-byte little endian small integer coded decimal accent 3 value */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.Accent3 = SignedShort;

				/*   2-byte little endian small integer coded decimal accent 4 value */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.Accent4 = SignedShort;

				/*   2-byte little endian fake pitch value */
				/*       this value has a range of -1..383.  If it is not -1, then it will be used */
				/*       to determine which sample a multisampled oscillator will use.  If it is -1 */
				/*       then the actual pitch will be used to select a sample. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				if ((SignedShort < -1) || (SignedShort >= NUMNOTES))
					{
						goto FailurePointNoteBadFormat;
					}
				Note->a.Note.MultisamplePitchAsIf = SignedShort;

				/*   2-byte little endian small integer coded decimal pitch disp depth adjustment */
				/*       this adjusts the maximum amplitude of the pitch displacement depth */
				/*       oscillator (vibrato).  The value has units of either half steps or hertz */
				/*       depending on the setting in the opcode word. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.PitchDisplacementDepthAdjustment = SignedShort;

				/*   2-byte little endian small integer coded decimal pitch displ rate adjustment */
				/*       this adjusts the maximum amplitude of the pitch displacement rate */
				/*       oscillator. the units are periods per second. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.PitchDisplacementRateAdjustment = SignedShort;

				/*   2-byte little endian small integer coded decimal pitch displ start point adjust */
				/*       this value adjusts when the pitch displacement envelopes start.  the */
				/*       location is from start or end of note, depending on the opcode settings, and */
				/*       is in fractions of the current note's duration. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.PitchDisplacementStartPoint = SignedShort;

				/*   2-byte little endian small integer coded decimal hurry-up factor */
				/*       this factor scales the total speed at which all envelopes change.  this is */
				/*       multiplicative, so a value of 1 makes no change, and smaller values make */
				/*       transitions go faster. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.HurryUpFactor = SignedShort;

				/*   2-byte little endian small integer coded decimal detuning value */
				/*       this value is added to the pitch of the note to detune.  its units are */
				/*       either hertz or half steps depending on the opcode word. */
				if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
					{
						goto FailurePointNoteDiskError;
					}
				Note->a.Note.Detuning = SignedShort;
			}
		 else
			{
				signed long					SignedLong;
				unsigned char				UnsignedChar;

				/* it's a command; clear out the operand fields to begin with */
				Note->a.Command.StringArgument = AllocPtrCanFail(0,
					"NoteObjectRec:  command string argument");
				if (Note->a.Command.StringArgument == NIL)
					{
						Error = eFileLoadOutOfMemory;
						goto FailurePoint2;
					}
				Note->a.Command.Argument1 = 0;
				Note->a.Command.Argument2 = 0;
				Note->a.Command.Argument3 = 0;

				/* read in operands */
				switch (Note->Flags & ~eCommandFlag)
					{
						default:
						 FailurePoint3:
							ReleasePtr(Note->a.Command.StringArgument);
							Error = eFileLoadBadFormat;
							goto FailurePoint2;

						/*   no arguments */
						case 16: /* 16 = restore tempo */
						case 32: /* 32 = restore stereo position */
						case 48: /* 48 = restore volume */
						case 64: /* 64 = restore release point 1 */
						case 80: /* 80 = restore release point 2 */
						case 96: /* 96 = restore accent 1 */
						case 112: /* 112 = restore accent 2 */
						case 128: /* 128 = restore accent 3 */
						case 144: /* 144 = restore accent 4 */
						case 160: /* 160 = restore pitch displacement depth */
						case 176: /* 176 = restore pitch displacement rate */
						case 192: /* 192 = restore pitch displacement start point */
						case 208: /* 208 = restore hurry-up factor */
						case 224: /* 224 = restore detuning */
						case 240: /* 240 = restore early/late adjust */
						case 256: /* 256 = restore duration adjust */
						case 304: /* 304 = restore surround position */
							break;

						/*   4-byte little endian large/extended integer coded decimal */
						case 17: /* 17 = set tempo */
						case 18: /* 18 = adjust tempo */
						case 33: /* 33 = set stereo position */
						case 34: /* 34 = adjust stereo position */
						case 49: /* 49 = set volume */
						case 50: /* 50 = adjust volume */
						case 65: /* 65 = set release point 1 */
						case 66: /* 66 = adjust release point 1 */
						case 81: /* 81 = set release point 2 */
						case 82: /* 82 = adjust release point 2 */
						case 97: /* 97 = set accent 1 */
						case 98: /* 98 = adjust accent 1 */
						case 113: /* 113 = set accent 2 */
						case 114: /* 114 = adjust accent 2 */
						case 129: /* 129 = set accent 3 */
						case 130: /* 130 = adjust accent 3 */
						case 145: /* 145 = set accent 4 */
						case 146: /* 146 = adjust accent 4 */
						case 161: /* 161 = set pitch displacement depth */
						case 162: /* 162 = adjust pitch displacement depth */
						case 177: /* 177 = set pitch displacement rate */
						case 178: /* 178 = adjust pitch displacement rate */
						case 193: /* 193 = set pitch displacement start point */
						case 194: /* 194 = adjust pitch displacement start point */
						case 209: /* 209 = set hurry-up factor */
						case 210: /* 210 = adjust hurry-up factor */
						case 225: /* 225 = set detuning */
						case 226: /* 226 = adjust detuning */
						case 241: /* 241 = set early/late adjust */
						case 242: /* 242 = adjust early/late adjust */
						case 257: /* 257 = set duration adjust */
						case 258: /* 258 = adjust duration adjust */
						case 273: /* 273 = set measure number */
						case 305: /* 305 = set surround position */
						case 306: /* 306 = adjust surround position */
						case 320: /* 320 = set transpose */
						case 321: /* 321 = adjust transpose */
							if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							Note->a.Command.Argument1 = SignedLong;
							break;

						/*   4-byte little endian large/extended integer coded decimal */
						/*   4-byte little endian large/extended integer coded decimal */
						case 19: /* 19 = sweep tempo absolute */
						case 20: /* 20 = sweep tempo relative */
						case 35: /* 35 = sweep stereo position absolute */
						case 36: /* 36 = sweep stereo position relative */
						case 51: /* 51 = sweep volume absolute */
						case 52: /* 52 = sweep volume relative */
						case 68: /* 68 = sweep release point 1 absolute */
						case 69: /* 69 = sweep release point 1 relative */
						case 84: /* 84 = sweep release point 2 absolute */
						case 85: /* 85 = sweep release point 2 relative */
						case 99: /* 99 = sweep accent 1 absolute */
						case 100: /* 100 = sweep accent 1 relative */
						case 115: /* 115 = sweep accent 2 absolute */
						case 116: /* 116 = sweep accent 2 relative */
						case 131: /* 131 = sweep accent 3 absolute */
						case 132: /* 132 = sweep accent 3 relative */
						case 147: /* 147 = sweep accent 4 absolute */
						case 148: /* 148 = sweep accent 4 relative */
						case 164: /* 164 = sweep pitch displacement depth absolute */
						case 165: /* 165 = sweep pitch displacement depth relative */
						case 179: /* 179 = sweep pitch displacement rate absolute */
						case 180: /* 180 = sweep pitch displacement rate relative */
						case 196: /* 196 = sweep pitch displacement start point absolute */
						case 197: /* 197 = sweep pitch displacement start point relative */
						case 211: /* 211 = sweep hurry-up factor absolute */
						case 212: /* 212 = sweep hurry-up factor relative */
						case 228: /* 228 = sweep detuning absolute */
						case 229: /* 229 = sweep detuning relative */
						case 243: /* 243 = sweep early/late adjust absolute */
						case 244: /* 244 = sweep early/late adjust relative */
						case 259: /* 259 = sweep duration adjust absolute */
						case 260: /* 260 = sweep duration adjust relative */
						case 272: /* 272 = set meter */
						case 307: /* 307 = sweep surround position absolute */
						case 308: /* 308 = sweep surround position relative */
							if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							Note->a.Command.Argument1 = SignedLong;
							if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							Note->a.Command.Argument2 = SignedLong;
							break;

						/*   1-byte origin specifier */
						/*       0 = -1 */
						/*       1 = 0 */
						case 67: /* 67 = set release point 1 origin */
						case 83: /* 83 = set release point 2 origin */
						case 163: /* 163 = set pitch displacement depth modulation mode */
						case 195: /* 195 = set pitch displacement start point origin */
						case 227: /* 227 = set detuning mode */
						case 261: /* 261 = set duration adjust mode */
							if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							if (UnsignedChar == 0)
								{
									Note->a.Command.Argument1 = -1;
								}
							else if (UnsignedChar == 1)
								{
									Note->a.Command.Argument1 = 0;
								}
							else
								{
									Error = eFileLoadBadFormat;
									goto FailurePoint3;
								}
							break;

						/*   4-byte little endian length of comment text string */
						/*   n-byte comment text string (line feed = 0x0a) */
						case 288: /* 288 = comment */
							if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							if (SignedLong < 0)
								{
									Error = eFileLoadBadFormat;
									goto FailurePoint3;
								}
							{
								char*							TempString;

								TempString = AllocPtrCanFail(SignedLong,"NoteObjectRec:  command string argument");
								if (TempString == NIL)
									{
										Error = eFileLoadOutOfMemory;
										goto FailurePoint3;
									}
								ReleasePtr(Note->a.Command.StringArgument);
								Note->a.Command.StringArgument = TempString;
							}
							if (!ReadBufferedInput(Input,SignedLong,Note->a.Command.StringArgument))
								{
									Error = eFileLoadDiskError;
									goto FailurePoint3;
								}
							break;
					}

				/* convert file format opcode to internal enumeration value */
				switch (Note->Flags & ~eCommandFlag)
					{
						default:
							EXECUTE(PRERR(ForceAbort,
								"NoteObjectNewFromFile:  command opcode filter failure"));
							break;
						case 16: /* 16 = restore tempo */
							Note->Flags = eCmdRestoreTempo;
							break;
						case 17: /* 17 = set tempo */
							Note->Flags = eCmdSetTempo;
							break;
						case 18: /* 18 = adjust tempo */
							Note->Flags = eCmdIncTempo;
							break;
						case 19: /* 19 = sweep tempo absolute */
							Note->Flags = eCmdSweepTempoAbs;
							break;
						case 20: /* 20 = sweep tempo relative */
							Note->Flags = eCmdSweepTempoRel;
							break;
						case 32: /* 32 = restore stereo position */
							Note->Flags = eCmdRestoreStereoPosition;
							break;
						case 33: /* 33 = set stereo position */
							Note->Flags = eCmdSetStereoPosition;
							break;
						case 34: /* 34 = adjust stereo position */
							Note->Flags = eCmdIncStereoPosition;
							break;
						case 35: /* 35 = sweep stereo position absolute */
							Note->Flags = eCmdSweepStereoAbs;
							break;
						case 36: /* 36 = sweep stereo position relative */
							Note->Flags = eCmdSweepStereoRel;
							break;
						case 48: /* 48 = restore volume */
							Note->Flags = eCmdRestoreVolume;
							break;
						case 49: /* 49 = set volume */
							Note->Flags = eCmdSetVolume;
							break;
						case 50: /* 50 = adjust volume */
							Note->Flags = eCmdIncVolume;
							break;
						case 51: /* 51 = sweep volume absolute */
							Note->Flags = eCmdSweepVolumeAbs;
							break;
						case 52: /* 52 = sweep volume relative */
							Note->Flags = eCmdSweepVolumeRel;
							break;
						case 64: /* 64 = restore release point 1 */
							Note->Flags = eCmdRestoreReleasePoint1;
							break;
						case 65: /* 65 = set release point 1 */
							Note->Flags = eCmdSetReleasePoint1;
							break;
						case 66: /* 66 = adjust release point 1 */
							Note->Flags = eCmdIncReleasePoint1;
							break;
						case 67: /* 67 = set release point 1 origin */
							Note->Flags = eCmdReleasePointOrigin1;
							break;
						case 68: /* 68 = sweep release point 1 absolute */
							Note->Flags = eCmdSweepReleaseAbs1;
							break;
						case 69: /* 69 = sweep release point 1 relative */
							Note->Flags = eCmdSweepReleaseRel1;
							break;
						case 80: /* 80 = restore release point 2 */
							Note->Flags = eCmdRestoreReleasePoint2;
							break;
						case 81: /* 81 = set release point 2 */
							Note->Flags = eCmdSetReleasePoint2;
							break;
						case 82: /* 82 = adjust release point 2 */
							Note->Flags = eCmdIncReleasePoint2;
							break;
						case 83: /* 83 = set release point 2 origin */
							Note->Flags = eCmdReleasePointOrigin2;
							break;
						case 84: /* 84 = sweep release point 2 absolute */
							Note->Flags = eCmdSweepReleaseAbs2;
							break;
						case 85: /* 85 = sweep release point 2 relative */
							Note->Flags = eCmdSweepReleaseRel2;
							break;
						case 96: /* 96 = restore accent 1 */
							Note->Flags = eCmdRestoreAccent1;
							break;
						case 97: /* 97 = set accent 1 */
							Note->Flags = eCmdSetAccent1;
							break;
						case 98: /* 98 = adjust accent 1 */
							Note->Flags = eCmdIncAccent1;
							break;
						case 99: /* 99 = sweep accent 1 absolute */
							Note->Flags = eCmdSweepAccentAbs1;
							break;
						case 100: /* 100 = sweep accent 1 relative */
							Note->Flags = eCmdSweepAccentRel1;
							break;
						case 112: /* 112 = restore accent 2 */
							Note->Flags = eCmdRestoreAccent2;
							break;
						case 113: /* 113 = set accent 2 */
							Note->Flags = eCmdSetAccent2;
							break;
						case 114: /* 114 = adjust accent 2 */
							Note->Flags = eCmdIncAccent2;
							break;
						case 115: /* 115 = sweep accent 2 absolute */
							Note->Flags = eCmdSweepAccentAbs2;
							break;
						case 116: /* 116 = sweep accent 2 relative */
							Note->Flags = eCmdSweepAccentRel2;
							break;
						case 128: /* 128 = restore accent 3 */
							Note->Flags = eCmdRestoreAccent3;
							break;
						case 129: /* 129 = set accent 3 */
							Note->Flags = eCmdSetAccent3;
							break;
						case 130: /* 130 = adjust accent 3 */
							Note->Flags = eCmdIncAccent3;
							break;
						case 131: /* 131 = sweep accent 3 absolute */
							Note->Flags = eCmdSweepAccentAbs3;
							break;
						case 132: /* 132 = sweep accent 3 relative */
							Note->Flags = eCmdSweepAccentRel3;
							break;
						case 144: /* 144 = restore accent 4 */
							Note->Flags = eCmdRestoreAccent4;
							break;
						case 145: /* 145 = set accent 4 */
							Note->Flags = eCmdSetAccent4;
							break;
						case 146: /* 146 = adjust accent 4 */
							Note->Flags = eCmdIncAccent4;
							break;
						case 147: /* 147 = sweep accent 4 absolute */
							Note->Flags = eCmdSweepAccentAbs4;
							break;
						case 148: /* 148 = sweep accent 4 relative */
							Note->Flags = eCmdSweepAccentRel4;
							break;
						case 160: /* 160 = restore pitch displacement depth */
							Note->Flags = eCmdRestorePitchDispDepth;
							break;
						case 161: /* 161 = set pitch displacement depth */
							Note->Flags = eCmdSetPitchDispDepth;
							break;
						case 162: /* 162 = adjust pitch displacement depth */
							Note->Flags = eCmdIncPitchDispDepth;
							break;
						case 163: /* 163 = set pitch displacement depth modulation mode */
							Note->Flags = eCmdPitchDispDepthMode;
							break;
						case 164: /* 164 = sweep pitch displacement depth absolute */
							Note->Flags = eCmdSweepPitchDispDepthAbs;
							break;
						case 165: /* 165 = sweep pitch displacement depth relative */
							Note->Flags = eCmdSweepPitchDispDepthRel;
							break;
						case 176: /* 176 = restore pitch displacement rate */
							Note->Flags = eCmdRestorePitchDispRate;
							break;
						case 177: /* 177 = set pitch displacement rate */
							Note->Flags = eCmdSetPitchDispRate;
							break;
						case 178: /* 178 = adjust pitch displacement rate */
							Note->Flags = eCmdIncPitchDispRate;
							break;
						case 179: /* 179 = sweep pitch displacement rate absolute */
							Note->Flags = eCmdSweepPitchDispRateAbs;
							break;
						case 180: /* 180 = sweep pitch displacement rate relative */
							Note->Flags = eCmdSweepPitchDispRateRel;
							break;
						case 192: /* 192 = restore pitch displacement start point */
							Note->Flags = eCmdRestorePitchDispStart;
							break;
						case 193: /* 193 = set pitch displacement start point */
							Note->Flags = eCmdSetPitchDispStart;
							break;
						case 194: /* 194 = adjust pitch displacement start point */
							Note->Flags = eCmdIncPitchDispStart;
							break;
						case 195: /* 195 = set pitch displacement start point origin */
							Note->Flags = eCmdPitchDispStartOrigin;
							break;
						case 196: /* 196 = sweep pitch displacement start point absolute */
							Note->Flags = eCmdSweepPitchDispStartAbs;
							break;
						case 197: /* 197 = sweep pitch displacement start point relative */
							Note->Flags = eCmdSweepPitchDispStartRel;
							break;
						case 208: /* 208 = restore hurry-up factor */
							Note->Flags = eCmdRestoreHurryUp;
							break;
						case 209: /* 209 = set hurry-up factor */
							Note->Flags = eCmdSetHurryUp;
							break;
						case 210: /* 210 = adjust hurry-up factor */
							Note->Flags = eCmdIncHurryUp;
							break;
						case 211: /* 211 = sweep hurry-up factor absolute */
							Note->Flags = eCmdSweepHurryUpAbs;
							break;
						case 212: /* 212 = sweep hurry-up factor relative */
							Note->Flags = eCmdSweepHurryUpRel;
							break;
						case 224: /* 224 = restore detuning */
							Note->Flags = eCmdRestoreDetune;
							break;
						case 225: /* 225 = set detuning */
							Note->Flags = eCmdSetDetune;
							break;
						case 226: /* 226 = adjust detuning */
							Note->Flags = eCmdIncDetune;
							break;
						case 227: /* 227 = set detuning mode */
							Note->Flags = eCmdDetuneMode;
							break;
						case 228: /* 228 = sweep detuning absolute */
							Note->Flags = eCmdSweepDetuneAbs;
							break;
						case 229: /* 229 = sweep detuning relative */
							Note->Flags = eCmdSweepDetuneRel;
							break;
						case 240: /* 240 = restore early/late adjust */
							Note->Flags = eCmdRestoreEarlyLateAdjust;
							break;
						case 241: /* 241 = set early/late adjust */
							Note->Flags = eCmdSetEarlyLateAdjust;
							break;
						case 242: /* 242 = adjust early/late adjust */
							Note->Flags = eCmdIncEarlyLateAdjust;
							break;
						case 243: /* 243 = sweep early/late adjust absolute */
							Note->Flags = eCmdSweepEarlyLateAbs;
							break;
						case 244: /* 244 = sweep early/late adjust relative */
							Note->Flags = eCmdSweepEarlyLateRel;
							break;
						case 256: /* 256 = restore duration adjust */
							Note->Flags = eCmdRestoreDurationAdjust;
							break;
						case 257: /* 257 = set duration adjust */
							Note->Flags = eCmdSetDurationAdjust;
							break;
						case 258: /* 258 = adjust duration adjust */
							Note->Flags = eCmdIncDurationAdjust;
							break;
						case 259: /* 259 = sweep duration adjust absolute */
							Note->Flags = eCmdSweepDurationAbs;
							break;
						case 260: /* 260 = sweep duration adjust relative */
							Note->Flags = eCmdSweepDurationRel;
							break;
						case 261: /* 261 = set duration adjust mode */
							Note->Flags = eCmdDurationAdjustMode;
							break;
						case 272: /* 272 = set meter */
							Note->Flags = eCmdSetMeter;
							break;
						case 273: /* 273 = set measure number */
							Note->Flags = eCmdSetMeasureNumber;
							break;
						case 288: /* 288 = comment */
							Note->Flags = eCmdMarker;
							break;
						case 304: /* 304 = restore surround position */
							Note->Flags = eCmdRestoreSurroundPosition;
							break;
						case 305: /* 305 = set surround position */
							Note->Flags = eCmdSetSurroundPosition;
							break;
						case 306: /* 306 = adjust surround position */
							Note->Flags = eCmdIncSurroundPosition;
							break;
						case 307: /* 307 = sweep surround position absolute */
							Note->Flags = eCmdSweepSurroundAbs;
							break;
						case 308: /* 308 = sweep surround position relative */
							Note->Flags = eCmdSweepSurroundRel;
							break;
						case 320: /* 320 = set transpose */
							Note->Flags = eCmdSetTranspose;
							break;
						case 321: /* 321 = adjust transpose */
							Note->Flags = eCmdAdjustTranspose;
							break;
					}
				Note->Flags |= eCommandFlag;
			}

		*ObjectOut = Note;
		return eFileLoadNoError;
	}


/* write a note object to the file.  this does not handle ties since they are */
/* done separately. */
FileLoadingErrors			NoteObjectWriteDataOut(NoteObjectRec* Note,
												struct BufferedOutputRec* Output)
	{
		CheckPtrExistence(Note);
		CheckPtrExistence(Output);

		if ((Note->Flags & eCommandFlag) == 0)
			{
				/* note */

				/*   4-byte unsigned little endian opcode field */
				/*       for a command, the high bit will be 1 and the remaining bits will */
				/*       be the opcode.  for a note, the high bit will be 0. */
				if (!WriteBufferedUnsignedLongLittleEndian(Output,Note->Flags))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte signed little endian pitch index */
				/*       should be a value in the range 0..383.  Middle C (261.6 Hertz) = 192 */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Pitch))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal portamento duration. */
				/*       this determines how long a portamento will last, in fractions of a quarter */
				/*       note.  it only has effect if the note is the target of a tie.  a value of */
				/*       0 means instantaneous, i.e. no portamento. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.PortamentoDuration))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal early/late adjustment */
				/*       this determines the displacement in time of the occurrence of the note */
				/*       in frations of a quarter note. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.EarlyLateAdjust))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal duration adjustment */
				/*       this value changes the duration of the note by being added to the */
				/*       duration or being multiplied by the duration. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.DurationAdjust))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal release point 1 location */
				/*       this determines when the release of the first sustain/loop will occur */
				/*       in fractions of the current note's duration.  it is relative to the origin */
				/*       as determined by the opcode field. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.ReleasePoint1))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal release point 2 location */
				/*       this determines when the release of the second sustain/loop will occur. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.ReleasePoint2))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal overall loudness adjustment */
				/*       this factor scales the total volume output of the oscillators for this */
				/*       particular note.  It is multiplied, so a value of 1 makes no change in */
				/*       loudness. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.OverallLoudnessAdjustment))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal stereo position adjustment. */
				/*       this value adjusts where the sound will be located in stereo.  -1 is */
				/*       the far left, 1 is the far right, and 0 is center. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.StereoPositionAdjustment))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal surround position adjustment. */
				/*       this value adjusts where the sound will be located in surroud. */
				/*       1 is front and -1 is rear */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.SurroundPositionAdjustment))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal accent 1 value */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Accent1))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal accent 2 value */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Accent2))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal accent 3 value */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Accent3))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal accent 4 value */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Accent4))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian fake pitch value */
				/*       this value has a range of -1..383.  If it is not -1, then it will be used */
				/*       to determine which sample a multisampled oscillator will use.  If it is -1 */
				/*       then the actual pitch will be used to select a sample. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.MultisamplePitchAsIf))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal pitch disp depth adjustment */
				/*       this adjusts the maximum amplitude of the pitch displacement depth */
				/*       oscillator (vibrato).  The value has units of either half steps or hertz */
				/*       depending on the setting in the opcode word. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.PitchDisplacementDepthAdjustment))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal pitch displ rate adjustment */
				/*       this adjusts the maximum amplitude of the pitch displacement rate */
				/*       oscillator. the units are periods per second. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.PitchDisplacementRateAdjustment))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal pitch displ start point adjust */
				/*       this value adjusts when the pitch displacement envelopes start.  the */
				/*       location is from start or end of note, depending on the opcode settings, and */
				/*       is in fractions of the current note's duration. */
				if (!WriteBufferedSignedShortLittleEndian(Output,
					Note->a.Note.PitchDisplacementStartPoint))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal hurry-up factor */
				/*       this factor scales the total speed at which all envelopes change.  this is */
				/*       multiplicative, so a value of 1 makes no change, and smaller values make */
				/*       transitions go faster. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.HurryUpFactor))
					{
						return eFileLoadDiskError;
					}

				/*   2-byte little endian small integer coded decimal detuning value */
				/*       this value is added to the pitch of the note to detune.  its units are */
				/*       either hertz or half steps depending on the opcode word. */
				if (!WriteBufferedSignedShortLittleEndian(Output,Note->a.Note.Detuning))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				unsigned long				Command;

				/* write out a command */

				/* convert internal enum index into the file format index */
				switch (Note->Flags & ~eCommandFlag)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NoteObjectWriteDataOut:  bad command"));
							break;
						case eCmdRestoreTempo: /* 16 = restore tempo */
							Command = 16;
							break;
						case eCmdSetTempo: /* 17 = set tempo */
							Command = 17;
							break;
						case eCmdIncTempo: /* 18 = adjust tempo */
							Command = 18;
							break;
						case eCmdSweepTempoAbs: /* 19 = sweep tempo absolute */
							Command = 19;
							break;
						case eCmdSweepTempoRel: /* 20 = sweep tempo relative */
							Command = 20;
							break;
						case eCmdRestoreStereoPosition: /* 32 = restore stereo position */
							Command = 32;
							break;
						case eCmdSetStereoPosition: /* 33 = set stereo position */
							Command = 33;
							break;
						case eCmdIncStereoPosition: /* 34 = adjust stereo position */
							Command = 34;
							break;
						case eCmdSweepStereoAbs: /* 35 = sweep stereo position absolute */
							Command = 35;
							break;
						case eCmdSweepStereoRel: /* 36 = sweep stereo position relative */
							Command = 36;
							break;
						case eCmdRestoreVolume: /* 48 = restore volume */
							Command = 48;
							break;
						case eCmdSetVolume: /* 49 = set volume */
							Command = 49;
							break;
						case eCmdIncVolume: /* 50 = adjust volume */
							Command = 50;
							break;
						case eCmdSweepVolumeAbs: /* 51 = sweep volume absolute */
							Command = 51;
							break;
						case eCmdSweepVolumeRel: /* 52 = sweep volume relative */
							Command = 52;
							break;
						case eCmdRestoreReleasePoint1: /* 64 = restore release point 1 */
							Command = 64;
							break;
						case eCmdSetReleasePoint1: /* 65 = set release point 1 */
							Command = 65;
							break;
						case eCmdIncReleasePoint1: /* 66 = adjust release point 1 */
							Command = 66;
							break;
						case eCmdReleasePointOrigin1: /* 67 = set release point 1 origin */
							Command = 67;
							break;
						case eCmdSweepReleaseAbs1: /* 68 = sweep release point 1 absolute */
							Command = 68;
							break;
						case eCmdSweepReleaseRel1: /* 69 = sweep release point 1 relative */
							Command = 69;
							break;
						case eCmdRestoreReleasePoint2: /* 80 = restore release point 2 */
							Command = 80;
							break;
						case eCmdSetReleasePoint2: /* 81 = set release point 2 */
							Command = 81;
							break;
						case eCmdIncReleasePoint2: /* 82 = adjust release point 2 */
							Command = 82;
							break;
						case eCmdReleasePointOrigin2: /* 83 = set release point 2 origin */
							Command = 83;
							break;
						case eCmdSweepReleaseAbs2: /* 84 = sweep release point 2 absolute */
							Command = 84;
							break;
						case eCmdSweepReleaseRel2: /* 85 = sweep release point 2 relative */
							Command = 85;
							break;
						case eCmdRestoreAccent1: /* 96 = restore accent 1 */
							Command = 96;
							break;
						case eCmdSetAccent1: /* 97 = set accent 1 */
							Command = 97;
							break;
						case eCmdIncAccent1: /* 98 = adjust accent 1 */
							Command = 98;
							break;
						case eCmdSweepAccentAbs1: /* 99 = sweep accent 1 absolute */
							Command = 99;
							break;
						case eCmdSweepAccentRel1: /* 100 = sweep accent 1 relative */
							Command = 100;
							break;
						case eCmdRestoreAccent2: /* 112 = restore accent 2 */
							Command = 112;
							break;
						case eCmdSetAccent2: /* 113 = set accent 2 */
							Command = 113;
							break;
						case eCmdIncAccent2: /* 114 = adjust accent 2 */
							Command = 114;
							break;
						case eCmdSweepAccentAbs2: /* 115 = sweep accent 2 absolute */
							Command = 115;
							break;
						case eCmdSweepAccentRel2: /* 116 = sweep accent 2 relative */
							Command = 116;
							break;
						case eCmdRestoreAccent3: /* 128 = restore accent 3 */
							Command = 128;
							break;
						case eCmdSetAccent3: /* 129 = set accent 3 */
							Command = 129;
							break;
						case eCmdIncAccent3: /* 130 = adjust accent 3 */
							Command = 130;
							break;
						case eCmdSweepAccentAbs3: /* 131 = sweep accent 3 absolute */
							Command = 131;
							break;
						case eCmdSweepAccentRel3: /* 132 = sweep accent 3 relative */
							Command = 132;
							break;
						case eCmdRestoreAccent4: /* 144 = restore accent 4 */
							Command = 144;
							break;
						case eCmdSetAccent4: /* 145 = set accent 4 */
							Command = 145;
							break;
						case eCmdIncAccent4: /* 146 = adjust accent 4 */
							Command = 146;
							break;
						case eCmdSweepAccentAbs4: /* 147 = sweep accent 4 absolute */
							Command = 147;
							break;
						case eCmdSweepAccentRel4: /* 148 = sweep accent 4 relative */
							Command = 148;
							break;
						case eCmdRestorePitchDispDepth: /* 160 = restore pitch displacement depth */
							Command = 160;
							break;
						case eCmdSetPitchDispDepth: /* 161 = set pitch displacement depth */
							Command = 161;
							break;
						case eCmdIncPitchDispDepth: /* 162 = adjust pitch displacement depth */
							Command = 162;
							break;
						case eCmdPitchDispDepthMode: /* 163 = set pitch displacement depth modulation mode */
							Command = 163;
							break;
						case eCmdSweepPitchDispDepthAbs: /* 164 = sweep pitch displacement depth absolute */
							Command = 164;
							break;
						case eCmdSweepPitchDispDepthRel: /* 165 = sweep pitch displacement depth relative */
							Command = 165;
							break;
						case eCmdRestorePitchDispRate: /* 176 = restore pitch displacement rate */
							Command = 176;
							break;
						case eCmdSetPitchDispRate: /* 177 = set pitch displacement rate */
							Command = 177;
							break;
						case eCmdIncPitchDispRate: /* 178 = adjust pitch displacement rate */
							Command = 178;
							break;
						case eCmdSweepPitchDispRateAbs: /* 179 = sweep pitch displacement rate absolute */
							Command = 179;
							break;
						case eCmdSweepPitchDispRateRel: /* 180 = sweep pitch displacement rate relative */
							Command = 180;
							break;
						case eCmdRestorePitchDispStart: /* 192 = restore pitch displacement start point */
							Command = 192;
							break;
						case eCmdSetPitchDispStart: /* 193 = set pitch displacement start point */
							Command = 193;
							break;
						case eCmdIncPitchDispStart: /* 194 = adjust pitch displacement start point */
							Command = 194;
							break;
						case eCmdPitchDispStartOrigin: /* 195 = set pitch displacement start point origin */
							Command = 195;
							break;
						case eCmdSweepPitchDispStartAbs: /* 196 = sweep pitch displacement start point absolute */
							Command = 196;
							break;
						case eCmdSweepPitchDispStartRel: /* 197 = sweep pitch displacement start point relative */
							Command = 197;
							break;
						case eCmdRestoreHurryUp: /* 208 = restore hurry-up factor */
							Command = 208;
							break;
						case eCmdSetHurryUp: /* 209 = set hurry-up factor */
							Command = 209;
							break;
						case eCmdIncHurryUp: /* 210 = adjust hurry-up factor */
							Command = 210;
							break;
						case eCmdSweepHurryUpAbs: /* 211 = sweep hurry-up factor absolute */
							Command = 211;
							break;
						case eCmdSweepHurryUpRel: /* 212 = sweep hurry-up factor relative */
							Command = 212;
							break;
						case eCmdRestoreDetune: /* 224 = restore detuning */
							Command = 224;
							break;
						case eCmdSetDetune: /* 225 = set detuning */
							Command = 225;
							break;
						case eCmdIncDetune: /* 226 = adjust detuning */
							Command = 226;
							break;
						case eCmdDetuneMode: /* 227 = set detuning mode */
							Command = 227;
							break;
						case eCmdSweepDetuneAbs: /* 228 = sweep detuning absolute */
							Command = 228;
							break;
						case eCmdSweepDetuneRel: /* 229 = sweep detuning relative */
							Command = 229;
							break;
						case eCmdRestoreEarlyLateAdjust: /* 240 = restore early/late adjust */
							Command = 240;
							break;
						case eCmdSetEarlyLateAdjust: /* 241 = set early/late adjust */
							Command = 241;
							break;
						case eCmdIncEarlyLateAdjust: /* 242 = adjust early/late adjust */
							Command = 242;
							break;
						case eCmdSweepEarlyLateAbs: /* 243 = sweep early/late adjust absolute */
							Command = 243;
							break;
						case eCmdSweepEarlyLateRel: /* 244 = sweep early/late adjust relative */
							Command = 244;
							break;
						case eCmdRestoreDurationAdjust: /* 256 = restore duration adjust */
							Command = 256;
							break;
						case eCmdSetDurationAdjust: /* 257 = set duration adjust */
							Command = 257;
							break;
						case eCmdIncDurationAdjust: /* 258 = adjust duration adjust */
							Command = 258;
							break;
						case eCmdSweepDurationAbs: /* 259 = sweep duration adjust absolute */
							Command = 259;
							break;
						case eCmdSweepDurationRel: /* 260 = sweep duration adjust relative */
							Command = 260;
							break;
						case eCmdDurationAdjustMode: /* 261 = set duration adjust mode */
							Command = 261;
							break;
						case eCmdSetMeter: /* 272 = set meter */
							Command = 272;
							break;
						case eCmdSetMeasureNumber: /* 273 = set measure number */
							Command = 273;
							break;
						case eCmdMarker: /* 288 = comment */
							Command = 288;
							break;
						case eCmdRestoreSurroundPosition: /* 304 = restore surround position */
							Command = 304;
							break;
						case eCmdSetSurroundPosition: /* 305 = set surround position */
							Command = 305;
							break;
						case eCmdIncSurroundPosition: /* 306 = adjust surround position */
							Command = 306;
							break;
						case eCmdSweepSurroundAbs: /* 307 = sweep surround position absolute */
							Command = 307;
							break;
						case eCmdSweepSurroundRel: /* 308 = sweep surround position relative */
							Command = 308;
							break;
						case eCmdSetTranspose: /* 320 = set transpose */
							Command = 320;
							break;
						case eCmdAdjustTranspose: /* 321 = adjust transpose */
							Command = 321;
							break;
					}
				if (!WriteBufferedUnsignedLongLittleEndian(Output,Command | eCommandFlag))
					{
						return eFileLoadDiskError;
					}

				/* write out the arguments */
				switch (Note->Flags & ~eCommandFlag)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"NoteObjectWriteDataOut:  bad command"));
							break;

						/*   no arguments */
						case eCmdRestoreTempo: /* 16 = restore tempo */
						case eCmdRestoreStereoPosition: /* 32 = restore stereo position */
						case eCmdRestoreVolume: /* 48 = restore volume */
						case eCmdRestoreReleasePoint1: /* 64 = restore release point 1 */
						case eCmdRestoreReleasePoint2: /* 80 = restore release point 2 */
						case eCmdRestoreAccent1: /* 96 = restore accent 1 */
						case eCmdRestoreAccent2: /* 112 = restore accent 2 */
						case eCmdRestoreAccent3: /* 128 = restore accent 3 */
						case eCmdRestoreAccent4: /* 144 = restore accent 4 */
						case eCmdRestorePitchDispDepth: /* 160 = restore pitch displacement depth */
						case eCmdRestorePitchDispRate: /* 176 = restore pitch displacement rate */
						case eCmdRestorePitchDispStart: /* 192 = restore pitch displacement start point */
						case eCmdRestoreHurryUp: /* 208 = restore hurry-up factor */
						case eCmdRestoreDetune: /* 224 = restore detuning */
						case eCmdRestoreEarlyLateAdjust: /* 240 = restore early/late adjust */
						case eCmdRestoreDurationAdjust: /* 256 = restore duration adjust */
						case eCmdRestoreSurroundPosition: /* 304 = restore surround position */
							break;

						/*   4-byte little endian large/extended integer coded decimal */
						case eCmdSetTempo: /* 17 = set tempo */
						case eCmdIncTempo: /* 18 = adjust tempo */
						case eCmdSetStereoPosition: /* 33 = set stereo position */
						case eCmdIncStereoPosition: /* 34 = adjust stereo position */
						case eCmdSetVolume: /* 49 = set volume */
						case eCmdIncVolume: /* 50 = adjust volume */
						case eCmdSetReleasePoint1: /* 65 = set release point 1 */
						case eCmdIncReleasePoint1: /* 66 = adjust release point 1 */
						case eCmdSetReleasePoint2: /* 81 = set release point 2 */
						case eCmdIncReleasePoint2: /* 82 = adjust release point 2 */
						case eCmdSetAccent1: /* 97 = set accent 1 */
						case eCmdIncAccent1: /* 98 = adjust accent 1 */
						case eCmdSetAccent2: /* 113 = set accent 2 */
						case eCmdIncAccent2: /* 114 = adjust accent 2 */
						case eCmdSetAccent3: /* 129 = set accent 3 */
						case eCmdIncAccent3: /* 130 = adjust accent 3 */
						case eCmdSetAccent4: /* 145 = set accent 4 */
						case eCmdIncAccent4: /* 146 = adjust accent 4 */
						case eCmdSetPitchDispDepth: /* 161 = set pitch displacement depth */
						case eCmdIncPitchDispDepth: /* 162 = adjust pitch displacement depth */
						case eCmdSetPitchDispRate: /* 177 = set pitch displacement rate */
						case eCmdIncPitchDispRate: /* 178 = adjust pitch displacement rate */
						case eCmdSetPitchDispStart: /* 193 = set pitch displacement start point */
						case eCmdIncPitchDispStart: /* 194 = adjust pitch displacement start point */
						case eCmdSetHurryUp: /* 209 = set hurry-up factor */
						case eCmdIncHurryUp: /* 210 = adjust hurry-up factor */
						case eCmdSetDetune: /* 225 = set detuning */
						case eCmdIncDetune: /* 226 = adjust detuning */
						case eCmdSetEarlyLateAdjust: /* 241 = set early/late adjust */
						case eCmdIncEarlyLateAdjust: /* 242 = adjust early/late adjust */
						case eCmdSetDurationAdjust: /* 257 = set duration adjust */
						case eCmdIncDurationAdjust: /* 258 = adjust duration adjust */
						case eCmdSetMeasureNumber: /* 273 = set measure number */
						case eCmdSetSurroundPosition: /* 305 = set surround position */
						case eCmdIncSurroundPosition: /* 306 = adjust surround position */
						case eCmdSetTranspose: /* 320 = set transpose */
						case eCmdAdjustTranspose: /* 321 = adjust transpose */
							if (!WriteBufferedSignedLongLittleEndian(Output,Note->a.Command.Argument1))
								{
									return eFileLoadDiskError;
								}
							break;

						/*   4-byte little endian large/extended integer coded decimal */
						/*   4-byte little endian large/extended integer coded decimal */
						case eCmdSweepTempoAbs: /* 19 = sweep tempo absolute */
						case eCmdSweepTempoRel: /* 20 = sweep tempo relative */
						case eCmdSweepStereoAbs: /* 35 = sweep stereo position absolute */
						case eCmdSweepStereoRel: /* 36 = sweep stereo position relative */
						case eCmdSweepVolumeAbs: /* 51 = sweep volume absolute */
						case eCmdSweepVolumeRel: /* 52 = sweep volume relative */
						case eCmdSweepReleaseAbs1: /* 68 = sweep release point 1 absolute */
						case eCmdSweepReleaseRel1: /* 69 = sweep release point 1 relative */
						case eCmdSweepReleaseAbs2: /* 84 = sweep release point 2 absolute */
						case eCmdSweepReleaseRel2: /* 85 = sweep release point 2 relative */
						case eCmdSweepAccentAbs1: /* 99 = sweep accent 1 absolute */
						case eCmdSweepAccentRel1: /* 100 = sweep accent 1 relative */
						case eCmdSweepAccentAbs2: /* 115 = sweep accent 2 absolute */
						case eCmdSweepAccentRel2: /* 116 = sweep accent 2 relative */
						case eCmdSweepAccentAbs3: /* 131 = sweep accent 3 absolute */
						case eCmdSweepAccentRel3: /* 132 = sweep accent 3 relative */
						case eCmdSweepAccentAbs4: /* 147 = sweep accent 4 absolute */
						case eCmdSweepAccentRel4: /* 148 = sweep accent 4 relative */
						case eCmdSweepPitchDispDepthAbs: /* 164 = sweep pitch displacement depth absolute */
						case eCmdSweepPitchDispDepthRel: /* 165 = sweep pitch displacement depth relative */
						case eCmdSweepPitchDispRateAbs: /* 179 = sweep pitch displacement rate absolute */
						case eCmdSweepPitchDispRateRel: /* 180 = sweep pitch displacement rate relative */
						case eCmdSweepPitchDispStartAbs: /* 196 = sweep pitch displacement start point absolute */
						case eCmdSweepPitchDispStartRel: /* 197 = sweep pitch displacement start point relative */
						case eCmdSweepHurryUpAbs: /* 211 = sweep hurry-up factor absolute */
						case eCmdSweepHurryUpRel: /* 212 = sweep hurry-up factor relative */
						case eCmdSweepDetuneAbs: /* 228 = sweep detuning absolute */
						case eCmdSweepDetuneRel: /* 229 = sweep detuning relative */
						case eCmdSweepEarlyLateAbs: /* 243 = sweep early/late adjust absolute */
						case eCmdSweepEarlyLateRel: /* 244 = sweep early/late adjust relative */
						case eCmdSweepDurationAbs: /* 259 = sweep duration adjust absolute */
						case eCmdSweepDurationRel: /* 260 = sweep duration adjust relative */
						case eCmdSetMeter: /* 272 = set meter */
						case eCmdSweepSurroundAbs: /* 307 = sweep surround position absolute */
						case eCmdSweepSurroundRel: /* 308 = sweep surround position relative */
							if (!WriteBufferedSignedLongLittleEndian(Output,Note->a.Command.Argument1))
								{
									return eFileLoadDiskError;
								}
							if (!WriteBufferedSignedLongLittleEndian(Output,Note->a.Command.Argument2))
								{
									return eFileLoadDiskError;
								}
							break;

						/*   1-byte origin specifier */
						/*       -1 = 0 */
						/*       0 = 1 */
						case eCmdReleasePointOrigin1: /* 67 = set release point 1 origin */
						case eCmdReleasePointOrigin2: /* 83 = set release point 2 origin */
						case eCmdPitchDispDepthMode: /* 163 = set pitch displacement depth modulation mode */
						case eCmdPitchDispStartOrigin: /* 195 = set pitch displacement start point origin */
						case eCmdDetuneMode: /* 227 = set detuning mode */
						case eCmdDurationAdjustMode: /* 261 = set duration adjust mode */
							if (!WriteBufferedUnsignedChar(Output,
								(Note->a.Command.Argument1 < 0) ? 0 : 1))
								{
									return eFileLoadDiskError;
								}
							break;

						/*   4-byte little endian length of comment text string */
						/*   n-byte comment text string (line feed = 0x0a) */
						case eCmdMarker: /* 288 = comment */
							if (!WriteBufferedSignedLongLittleEndian(Output,
								PtrSize(Note->a.Command.StringArgument)))
								{
									return eFileLoadDiskError;
								}
							if (!WriteBufferedOutput(Output,PtrSize(Note->a.Command.StringArgument),
								Note->a.Command.StringArgument))
								{
									return eFileLoadDiskError;
								}
							break;
					}
			}

		return eFileLoadNoError;
	}


/* make a complete copy of the note/command and any blocks it has allocated */
NoteObjectRec*				DeepCopyNoteObject(NoteObjectRec* Note)
	{
		NoteObjectRec*			Copy;

		CheckPtrExistence(Note);

		/* duplicate */
		Copy = (NoteObjectRec*)AllocPtrCanFail(sizeof(NoteObjectRec),"NoteObjectRec");
		if (Copy == NIL)
			{
				return NIL;
			}

		/* copy data over */
		Copy->Flags = Note->Flags;
		if ((Note->Flags & eCommandFlag) != 0)
			{
				/* command */
				Copy->a.Command.Argument1 = Note->a.Command.Argument1;
				Copy->a.Command.Argument2 = Note->a.Command.Argument2;
				Copy->a.Command.Argument3 = Note->a.Command.Argument3;
				Copy->a.Command.StringArgument = CopyPtr(Note->a.Command.StringArgument);
				if (Copy->a.Command.StringArgument == NIL)
					{
						ReleasePtr((char*)Copy);
						return NIL;
					}
				SetTag(Copy->a.Command.StringArgument,"NoteObjectRec: StringArgument");
			}
		 else
			{
				/* note */
				Copy->a.Note.Pitch = Note->a.Note.Pitch;
				Copy->a.Note.PortamentoDuration = Note->a.Note.PortamentoDuration;
				Copy->a.Note.EarlyLateAdjust = Note->a.Note.EarlyLateAdjust;
				Copy->a.Note.DurationAdjust = Note->a.Note.DurationAdjust;
				Copy->a.Note.Tie = Note->a.Note.Tie;
				Copy->a.Note.ReleasePoint1 = Note->a.Note.ReleasePoint1;
				Copy->a.Note.ReleasePoint2 = Note->a.Note.ReleasePoint2;
				Copy->a.Note.OverallLoudnessAdjustment = Note->a.Note.OverallLoudnessAdjustment;
				Copy->a.Note.StereoPositionAdjustment = Note->a.Note.StereoPositionAdjustment;
				Copy->a.Note.SurroundPositionAdjustment = Note->a.Note.SurroundPositionAdjustment;
				Copy->a.Note.Accent1 = Note->a.Note.Accent1;
				Copy->a.Note.Accent2 = Note->a.Note.Accent2;
				Copy->a.Note.Accent3 = Note->a.Note.Accent3;
				Copy->a.Note.Accent4 = Note->a.Note.Accent4;
				Copy->a.Note.MultisamplePitchAsIf = Note->a.Note.MultisamplePitchAsIf;
				Copy->a.Note.PitchDisplacementDepthAdjustment = Note->a.Note.PitchDisplacementDepthAdjustment;
				Copy->a.Note.PitchDisplacementRateAdjustment = Note->a.Note.PitchDisplacementRateAdjustment;
				Copy->a.Note.PitchDisplacementStartPoint = Note->a.Note.PitchDisplacementStartPoint;
				Copy->a.Note.HurryUpFactor = Note->a.Note.HurryUpFactor;
				Copy->a.Note.Detuning = Note->a.Note.Detuning;
			}

		return Copy;
	}
