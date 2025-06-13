/* EditCommandParameters.c */
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

#include "EditCommandParameters.h"
#include "NoteObject.h"
#include "Memory.h"
#include "NoteAttributeDialog.h"
#include "BinaryCodedDecimal.h"
#include "TrackObject.h"
#include "DataMunging.h"
#include "CmdDlgOneParam.h"
#include "CmdDlgTwoParams.h"
#include "CmdDlgOneBinaryChoice.h"
#include "CmdDlgOneString.h"


/* dialog box for command with <1xs> parameter */
static MyBoolean	OneXS(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text)
	{
		double					OneDouble;
		MyBoolean				ChangedFlag;

		OneDouble = SmallExtBCD2Double(GetCommandNumericArg1(NoteCommand));
		ChangedFlag = CommandDialogOneParam(Prompt,Box1Text,&OneDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,Double2SmallExtBCD(OneDouble));
			}
		return ChangedFlag;
	}


/* dialog box for command with <1xs> and <2xs> parameters */
static MyBoolean	TwoXS(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text,
										char* Box2Text)
	{
		double					OneDouble;
		double					TwoDouble;
		MyBoolean				ChangedFlag;

		OneDouble = SmallExtBCD2Double(GetCommandNumericArg1(NoteCommand));
		TwoDouble = SmallExtBCD2Double(GetCommandNumericArg2(NoteCommand));
		ChangedFlag = CommandDialogTwoParams(Prompt,Box1Text,&OneDouble,Box2Text,&TwoDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,Double2SmallExtBCD(OneDouble));
				PutCommandNumericArg2(NoteCommand,Double2SmallExtBCD(TwoDouble));
			}
		return ChangedFlag;
	}


/* dialog box for command with <1l> parameter */
static MyBoolean	OneL(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text)
	{
		double					OneDouble;
		MyBoolean				ChangedFlag;

		OneDouble = LargeBCD2Double(GetCommandNumericArg1(NoteCommand));
		ChangedFlag = CommandDialogOneParam(Prompt,Box1Text,&OneDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,Double2LargeBCD(OneDouble));
			}
		return ChangedFlag;
	}


/* dialog box for 2 param command with first param <1l> and second <2xs> */
static MyBoolean	OneLTwoXS(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text,
										char* Box2Text)
	{
		double					OneDouble;
		double					TwoDouble;
		MyBoolean				ChangedFlag;

		OneDouble = LargeBCD2Double(GetCommandNumericArg1(NoteCommand));
		TwoDouble = SmallExtBCD2Double(GetCommandNumericArg2(NoteCommand));
		ChangedFlag = CommandDialogTwoParams(Prompt,Box1Text,&OneDouble,Box2Text,&TwoDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,Double2LargeBCD(OneDouble));
				PutCommandNumericArg2(NoteCommand,Double2SmallExtBCD(TwoDouble));
			}
		return ChangedFlag;
	}


/* dialog box where the value being negative means one thing and the value */
/* being zero or positive means another. */
static MyBoolean	OneBool(NoteObjectRec* NoteCommand, char* Prompt, char* Negative,
										char* ZeroOrPositive)
	{
		MyBoolean				Flag;
		MyBoolean				ChangedFlag;

		if (GetCommandNumericArg1(NoteCommand) < 0)
			{
				/* negative */
				Flag = True;
			}
		 else
			{
				/* zero or positive */
				Flag = False;
			}
		ChangedFlag = CommandDialogOneBinaryChoice(Prompt,Negative,ZeroOrPositive,&Flag);
		if (ChangedFlag)
			{
				if (Flag)
					{
						/* true = negative */
						PutCommandNumericArg1(NoteCommand,-1);
					}
				 else
					{
						/* false = zero or positive */
						PutCommandNumericArg1(NoteCommand,0);
					}
			}
		return ChangedFlag;
	}


/* dialog box for command with <1i> and <2i> parameters */
static MyBoolean	TwoI(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text,
										char* Box2Text)
	{
		double					OneDouble;
		double					TwoDouble;
		MyBoolean				ChangedFlag;

		OneDouble = GetCommandNumericArg1(NoteCommand);
		TwoDouble = GetCommandNumericArg2(NoteCommand);
		ChangedFlag = CommandDialogTwoParams(Prompt,Box1Text,&OneDouble,Box2Text,&TwoDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,(long)OneDouble);
				PutCommandNumericArg2(NoteCommand,(long)TwoDouble);
			}
		return ChangedFlag;
	}


/* dialog box for command with <1s> parameter */
static MyBoolean	OneStr(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text)
	{
		char*						StringParameter;
		MyBoolean				ChangedFlag;

		StringParameter = GetCommandStringArg(NoteCommand);
		if (StringParameter == NIL)
			{
				StringParameter = AllocPtrCanFail(0,"OneStr");
				if (StringParameter == NIL)
					{
						return False;
					}
			}
		 else
			{
				StringParameter = CopyPtr(StringParameter);
				if (StringParameter == NIL)
					{
						return False;
					}
			}
		ChangedFlag = CommandDialogOneString(Prompt,Box1Text,&StringParameter);
		if (ChangedFlag)
			{
				PutCommandStringArg(NoteCommand,StringParameter);
			}
		 else
			{
				ReleasePtr(StringParameter);
			}
		return ChangedFlag;
	}


/* dialog box for command with <1l> parameter */
static MyBoolean	OneI(NoteObjectRec* NoteCommand, char* Prompt, char* Box1Text)
	{
		double					OneDouble;
		MyBoolean				ChangedFlag;

		OneDouble = GetCommandNumericArg1(NoteCommand);
		ChangedFlag = CommandDialogOneParam(Prompt,Box1Text,&OneDouble);
		if (ChangedFlag)
			{
				PutCommandNumericArg1(NoteCommand,OneDouble);
			}
		return ChangedFlag;
	}


/* present a dialog box appropriate to the object type which allows the */
/* user to edit the object's attributes */
void							EditNoteOrCommandAttributes(struct NoteObjectRec* NoteCommand,
										TrackObjectRec* Track)
	{
		CheckPtrExistence(NoteCommand);
		if (!IsItACommand(NoteCommand))
			{
				/* present note attribute editing box */
				EditNoteParametersDialog(NoteCommand,Track);
			}
		 else
			{
				MyBoolean				SomethingChanged EXECUTE(= -15431);

				/* figure out which command edit box we should present */
				switch (GetCommandOpcode(NoteCommand))
					{
						/* restore the tempo to the default for the score */
						case eCmdRestoreTempo:
							SomethingChanged = False; /* there are no parameters to edit */
							break;

						/* set tempo to <1xs> number of beats per second */
						case eCmdSetTempo:
							SomethingChanged = OneXS(NoteCommand,"Set Tempo:  Enter a new "
								"tempo value.","Beats per Minute:");
							break;

						/* add <1xs> to the tempo control */
						case eCmdIncTempo:
							SomethingChanged = OneXS(NoteCommand,"Increment Tempo:  Enter the "
								"number of beats per minute to change the tempo by.","BPM Adjustment:");
							break;

						/* <1xs> = target tempo, <2xs> = # of beats to reach it */
						case eCmdSweepTempoAbs:
							SomethingChanged = TwoXS(NoteCommand,"Sweep Tempo Absolute:  Enter new "
								"tempo destination value and the number of beats to spread the "
								"transition across.","Destination BPM:","Duration:");
							break;

						/* <1xs> = target adjust (add to tempo), <2xs> = # beats */
						case eCmdSweepTempoRel:
							SomethingChanged = TwoXS(NoteCommand,"Sweep Tempo Relative:  Enter a "
								"tempo adjustment value and the number of beats to spread the "
								"transition across.","BPM Adjustment:","Duration:");
							break;

						/* restore stereo position to channel's default */
						case eCmdRestoreStereoPosition:
							SomethingChanged = False; /* no attributes */
							break;

						/* set position in channel <1l>: -1 = left, 1 = right */
						case eCmdSetStereoPosition:
							SomethingChanged = OneL(NoteCommand,"Set Stereo Position:  Enter a "
								"stereo position value (-1 = hard left ... 1 = hard right).",
								"Stereo Position:");
							break;

						/* adjust stereo position by adding <1l> */
						case eCmdIncStereoPosition:
							SomethingChanged = OneL(NoteCommand,"Adjust Stereo Position:  Enter an "
								"adjustment value for the stereo position (negative values move the "
								"channel left; positive values move the channel right).",
								"Stereo Position Adjustment:");
							break;

						/* <1l> = new pos, <2xs> = # of beats to get there */
						case eCmdSweepStereoAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Stereo Absolute:  Enter "
								"a destination value for the stereo position and the number of beats "
								"to spread the transition across.","Destination Stereo Position:",
								"Duration:");
							break;

						/* <1l> = pos adjust, <2xs> = # beats to get there */
						case eCmdSweepStereoRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Stereo Relative:  Enter "
								"a stereo position adjustment value and the number of beats to "
								"spread the transition across.","Stereo Position Adjustment:",
								"Duration:");
							break;

						/* restore surround position to channel's default */
						case eCmdRestoreSurroundPosition:
							SomethingChanged = False; /* no attributes */
							break;

						/* set surround position in channel <1l>: 1 = front, -1 = rear */
						case eCmdSetSurroundPosition:
							SomethingChanged = OneL(NoteCommand,"Set Surround Position:  Enter a "
								"surround position value (1 = front ... -1 = rear).",
								"Surround Position:");
							break;

						/* adjust surround position by adding <1l> */
						case eCmdIncSurroundPosition:
							SomethingChanged = OneL(NoteCommand,"Adjust Surround Position:  Enter an "
								"adjustment value for the surround position (positive values move the "
								"channel forward; negative values move the channel backward).",
								"Surround Position Adjustment:");
							break;

						/* <1l> = new pos, <2xs> = # of beats to get there */
						case eCmdSweepSurroundAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Surround Absolute:  Enter "
								"a destination value for the surround position and the number of beats "
								"to spread the transition across.","Destination Surround Position:",
								"Duration:");
							break;

						/* <1l> = pos adjust, <2xs> = # beats to get there */
						case eCmdSweepSurroundRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Surround Relative:  Enter "
								"a surround position adjustment value and the number of beats to "
								"spread the transition across.","Surround Position Adjustment:",
								"Duration:");
							break;

						/* restore the volume to the default for the channel */
						case eCmdRestoreVolume:
							SomethingChanged = False; /* no attributes to edit */
							break;

						/* set the volume to the specified level (0..1) in <1l> */
						case eCmdSetVolume:
							SomethingChanged = OneL(NoteCommand,"Set Volume:  Enter an overall "
								"volume level value (0 = silent ... 1 = full volume).","Volume:");
							break;

						/* add <1l> to the volume control */
						case eCmdIncVolume:
							SomethingChanged = OneL(NoteCommand,"Adjust Volume:  Enter a volume "
								"adjustment value (values less than 1 make sound quieter, values "
								"greater than 1 make sound louder).","Volume Adjustment:");
							break;

						/* <1l> = new volume, <2xs> = # of beats to reach it */
						case eCmdSweepVolumeAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Volume Absolute:  Enter "
								"a new volume value and the number of beats to spread the transition "
								"across.","Destination Volume:","Duration:");
							break;

						/* <1l> = volume adjust, <2xs> = # of beats to reach it */
						case eCmdSweepVolumeRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Volume Relative:  Enter "
								"a volume adjustment value and the number of beats to spread the "
								"transition across.","Volume Adjustment:","Duration:");
							break;

						/* restore release point to master default */
						case eCmdRestoreReleasePoint1:
							SomethingChanged = False; /* no attributes to edit */
							break;

						/* set the default release point to new value <1l> */
						case eCmdSetReleasePoint1:
							SomethingChanged = OneL(NoteCommand,"Set Release Point 1:  Enter the "
								"first release point location (0 = start of note; 1 = end of note; "
								"values beyond range are allowed).","Release Point 1:");
							break;

						/* add <1l> to default release point for adjustment */
						case eCmdIncReleasePoint1:
							SomethingChanged = OneL(NoteCommand,"Adjust Release Point 1:  Enter an "
								"adjustment value for the first release point (negative values move "
								"the release earlier; positive values move it later).",
								"Release Point 1 Adjust:");
							break;

						/* if <1i> is < 0, then from start, else from end of note */
						case eCmdReleasePointOrigin1:
							SomethingChanged = OneBool(NoteCommand,"Release Point 1 Origin:  Choose "
								"where the first release point should be measured from.",
								"From Start of Note","From End of Note");
							break;

						/* <1l> = new release, <2xs> = # of beats to get there */
						case eCmdSweepReleaseAbs1:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Release Point 1 Absolute:"
								"  Enter a destination time for the first release point and the "
								"number of beats to spread the transition across.","Release Point 1:",
								"Duration:");
							break;

						/* <1l> = release adjust, <2xs> = # of beats to get there */
						case eCmdSweepReleaseRel1:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Release Point 1 Relative:"
								"  Enter an adjustment value for the first release point and the "
								"number of beats to spread the transition across.",
								"Release Point 1 Adjust:","Duration:");
							break;

						/* restore release point to master default */
						case eCmdRestoreReleasePoint2:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set the default release point to new value <1l> */
						case eCmdSetReleasePoint2:
							SomethingChanged = OneL(NoteCommand,"Set Release Point 2:  Enter the "
								"second release point location (0 = start of note; 1 = end of note; "
								"values beyond range are allowed).","Release Point 2:");
							break;

						/* add <1l> to default release point for adjustment */
						case eCmdIncReleasePoint2:
							SomethingChanged = OneL(NoteCommand,"Adjust Release Point 2:  Enter an "
								"adjustment value for the second release point (negative values move "
								"the release earlier; positive values move it later).",
								"Release Point 2 Adjust:");
							break;

						/* if <1i> is < 0, then from start, else from end of note */
						case eCmdReleasePointOrigin2:
							SomethingChanged = OneBool(NoteCommand,"Release Point 2 Origin:  Choose "
								"where the second release point should be measured from.",
								"From Start of Note","From End of Note");
							break;

						/* <1l> = new release, <2xs> = # of beats to get there */
						case eCmdSweepReleaseAbs2:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Release Point 2 Absolute:"
								"  Enter a destination time for the second release point and the "
								"number of beats to spread the transition across.","Release Point 2:",
								"Duration:");
							break;

						/* <1l> = release adjust, <2xs> = # of beats to get there */
						case eCmdSweepReleaseRel2:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Release Point 2 Relative:"
								"  Enter an adjustment value for the second release point and the "
								"number of beats to spread the transition across.",
								"Release Point 2 Adjust:","Duration:");
							break;

						/* restore accent value to master default */
						case eCmdRestoreAccent1:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* specify the new default accent in <1l> */
						case eCmdSetAccent1:
							SomethingChanged = OneL(NoteCommand,"Set Accent 1:  Enter an accent "
								"factor (0 = normal; less than 0 = diminish; greater than 0 = "
								"strengthen).","Accent 1:");
							break;

						/* add <1l> to the default accent */
						case eCmdIncAccent1:
							SomethingChanged = OneL(NoteCommand,"Adjust Accent 1:  Enter an "
								"adjustment value for the first accent (negative values diminish the "
								"accent; positive values strengthen it).","Accent 1 Adjustment:");
							break;

						/* <1l> = new accent, <2xs> = # of beats to get there */
						case eCmdSweepAccentAbs1:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 1 Absolute:  "
								"Enter an accent value and the number of beats to spread the "
								"transition across.","Destination Accent 1:","Duration:");
							break;

						/* <1l> = accent adjust, <2xs> = # of beats to get there */
						case eCmdSweepAccentRel1:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 1 Relative:  "
								"Enter an accent adjust value and the number of beats to spread the "
								"transition across.","Accent 1 Adjust:","Duration:");
							break;

						/* restore accent value to master default */
						case eCmdRestoreAccent2:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* specify the new default accent in <1l> */
						case eCmdSetAccent2:
							SomethingChanged = OneL(NoteCommand,"Set Accent 2:  Enter an accent "
								"factor (0 = normal; less than 0 = diminish; greater than 0 = "
								"strengthen).","Accent 2:");
							break;

						/* add <1l> to the default accent */
						case eCmdIncAccent2:
							SomethingChanged = OneL(NoteCommand,"Adjust Accent 2:  Enter an "
								"adjustment value for the second accent (negative values diminish the "
								"accent; positive values strengthen it).","Accent 2 Adjustment:");
							break;

						/* <1l> = new accent, <2xs> = # of beats to get there */
						case eCmdSweepAccentAbs2:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 2 Absolute:  "
								"Enter an accent value and the number of beats to spread the "
								"transition across.","Destination Accent 2:","Duration:");
							break;

						/* <1l> = accent adjust, <2xs> = # of beats to get there */
						case eCmdSweepAccentRel2:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 2 Relative:  "
								"Enter an accent adjust value and the number of beats to spread the "
								"transition across.","Accent 2 Adjust:","Duration:");
							break;

						/* restore accent value to master default */
						case eCmdRestoreAccent3:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* specify the new default accent in <1l> */
						case eCmdSetAccent3:
							SomethingChanged = OneL(NoteCommand,"Set Accent 3:  Enter an accent "
								"factor (0 = normal; less than 0 = diminish; greater than 0 = "
								"strengthen).","Accent 3:");
							break;

						/* add <1l> to the default accent */
						case eCmdIncAccent3:
							SomethingChanged = OneL(NoteCommand,"Adjust Accent 3:  Enter an "
								"adjustment value for the second accent (negative values diminish the "
								"accent; positive values strengthen it).","Accent 3 Adjustment:");
							break;

						/* <1l> = new accent, <2xs> = # of beats to get there */
						case eCmdSweepAccentAbs3:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 3 Absolute:  "
								"Enter an accent value and the number of beats to spread the "
								"transition across.","Destination Accent 3:","Duration:");
							break;

						/* <1l> = accent adjust, <2xs> = # of beats to get there */
						case eCmdSweepAccentRel3:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 3 Relative:  "
								"Enter an accent adjust value and the number of beats to spread the "
								"transition across.","Accent 3 Adjust:","Duration:");
							break;

						/* restore accent value to master default */
						case eCmdRestoreAccent4:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* specify the new default accent in <1l> */
						case eCmdSetAccent4:
							SomethingChanged = OneL(NoteCommand,"Set Accent 4:  Enter an accent "
								"factor (0 = normal; less than 0 = diminish; greater than 0 = "
								"strengthen).","Accent 4:");
							break;

						/* add <1l> to the default accent */
						case eCmdIncAccent4:
							SomethingChanged = OneL(NoteCommand,"Adjust Accent 4:  Enter an "
								"adjustment value for the fourth accent (negative values diminish the "
								"accent; positive values strengthen it).","Accent 4 Adjustment:");
							break;

						/* <1l> = new accent, <2xs> = # of beats to get there */
						case eCmdSweepAccentAbs4:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 3 Absolute:  "
								"Enter an accent value and the number of beats to spread the "
								"transition across.","Destination Accent 3:","Duration:");
							break;

						/* <1l> = accent adjust, <2xs> = # of beats to get there */
						case eCmdSweepAccentRel4:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Accent 3 Relative:  "
								"Enter an accent adjust value and the number of beats to spread the "
								"transition across.","Accent 3 Adjust:","Duration:");
							break;

						/* restore max pitch disp depth value to default */
						case eCmdRestorePitchDispDepth:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set new max pitch disp depth <1l> */
						case eCmdSetPitchDispDepth:
							SomethingChanged = OneL(NoteCommand,"Set Pitch Displacement Depth:  "
								"Enter a new maximum pitch displacement depth.","Pitch Disp. Depth:");
							break;

						/* add <1l> to the default pitch disp depth */
						case eCmdIncPitchDispDepth:
							SomethingChanged = OneL(NoteCommand,"Adjust Pitch Displacement Depth:  "
								"Enter an adjustment for the maximum pitch displacement depth.",
								"Pitch Disp. Depth Adjust:");
							break;

						/* <1i>:  <0: Hertz, >=0: half-steps */
						case eCmdPitchDispDepthMode:
							SomethingChanged = OneBool(NoteCommand,"Pitch Displacement Depth Mode:  "
								"Choose whether the pitch displacement depth will be interpreted as "
								"Hertz or halfsteps.","Hertz","Halfsteps");
							break;

						/* <1l> = new depth, <2xs> = # of beats */
						case eCmdSweepPitchDispDepthAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement Depth "
								"Absolute:  Enter the target pitch displacement depth and the number "
								"of beats to spread the transition across.","Dest. Pitch Disp. Depth:",
								"Duration:");
							break;

						/* <1l> = depth adjust, <2xs> = # of beats */
						case eCmdSweepPitchDispDepthRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement Depth "
								"Relative:  Enter an adjustment pitch displacement depth value and "
								"the number of beats to spread the transition across.",
								"Pitch Disp. Depth Adjust:","Duration:");
							break;

						/* restore max pitch disp rate to the master default */
						case eCmdRestorePitchDispRate:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set new max pitch disp rate in seconds to <1l> */
						case eCmdSetPitchDispRate:
							SomethingChanged = OneL(NoteCommand,"Set Pitch Displacement Rate:  Enter "
								"the maximum number of oscillations per second.",
								"Pitch Displacement Rate:");
							break;

						/* add <1l> to the default max pitch disp rate */
						case eCmdIncPitchDispRate:
							SomethingChanged = OneL(NoteCommand,"Adjust Pitch Displacement Rate:  "
								"Enter an adjustment pitch displacement rate value.",
								"Pitch Disp. Rate Adjust:");
							break;

						/* <1l> = new rate, <2xs> = # of beats to get there */
						case eCmdSweepPitchDispRateAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement Rate "
								"Absolute:  Enter a destination pitch displacement rate and the "
								"number of beats to spread the transition across.",
								"Dest. Pitch Disp. Rate:","Duration:");
							break;

						/* <1l> = rate adjust, <2xs> = # of beats to get there */
						case eCmdSweepPitchDispRateRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement Rate "
								"Relative:  Enter an adjustment pitch displacement rate value and "
								"the number of beats to spread the transition across.",
								"Pitch Disp. Rate Adjust:","Duration:");
							break;

						/* restore pitch disp start point to default */
						case eCmdRestorePitchDispStart:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set the start point to <1l> */
						case eCmdSetPitchDispStart:
							SomethingChanged = OneL(NoteCommand,"Set Pitch Displacement Start:  "
								"Enter a new start point for the pitch displacement envelope (0 = "
								"note start; 1 = note end; values out of range are allowed).",
								"Pitch Disp. Start:");
							break;

						/* add <1l> to the pitch disp start point */
						case eCmdIncPitchDispStart:
							SomethingChanged = OneL(NoteCommand,"Adjust Pitch Displacement Start:  "
								"Enter an adjustment for the pitch displacement start point.",
								"Pitch Disp. Start Adjust:");
							break;

						/* specify the origin, same as for release point <1i> */
						case eCmdPitchDispStartOrigin:
							SomethingChanged = OneBool(NoteCommand,"Pitch Displacement Origin:  "
								"Choose where the pitch displacement start point should be measured "
								"from.","From Start of Note","From End of Note");
							break;

						/* <1l> = new vib start, <2xs> = # of beats */
						case eCmdSweepPitchDispStartAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement "
								"Start Absolute:  Enter a pitch displacement start point and the "
								"number of beats to spread the transition across.",
								"Dest. Pitch Disp. Start:","Duration:");
							break;

						/* <1l> = vib adjust, <2xs> = # of beats */
						case eCmdSweepPitchDispStartRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Pitch Displacement "
								"Start Relative:  Enter an adjustment pitch displacement start point "
								"value and the number of beats to spread the transition across.",
								"Pitch Disp. Start Adjust:","Duration:");
							break;

						/* restore default hurryup factor */
						case eCmdRestoreHurryUp:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set the hurryup factor to <1l> */
						case eCmdSetHurryUp:
							SomethingChanged = OneL(NoteCommand,"Set Hurry-Up Factor:  Enter a "
								"hurry-up factor (1 = normal; less than 1 = envelopes execute faster; "
								"greater than 1 = envelopes execute more slowly).","Hurry-Up Factor:");
							break;

						/* add <1l> to the hurryup factor */
						case eCmdIncHurryUp:
							SomethingChanged = OneL(NoteCommand,"Adjust Hurry-Up Factor:  Enter an "
								"adjustment hurry-up value (negative values make envelopes execute "
								"faster; positive values make envelopes execute more slowly).",
								"Hurry-Up Adjustment:");
							break;

						/* <1l> = new hurryup factor, <2xs> = # of beats */
						case eCmdSweepHurryUpAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Hurry-Up Factor Absolute:"
								"  Enter a hurry-up factor and the number of beats to spread the "
								"transition across.","Destination Hurry-Up:","Duration:");
							break;

						/* <1l> = hurryup adjust, <2xs> = # of beats to get there */
						case eCmdSweepHurryUpRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Hurry-Up Factor Relative:"
								"  Enter an adjustment hurry-up value and the number of beats to "
								"spread the transition across.","Hurry-Up Adjustment:","Duration:");
							break;

						/* restore the default detune factor */
						case eCmdRestoreDetune:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set the detune factor to <1l> */
						case eCmdSetDetune:
							SomethingChanged = OneL(NoteCommand,"Set Detuning:  Enter a detuning "
								"value (negative values decrease pitch; positive values increase "
								"pitch).","Detuning:");
							break;

						/* add <1l> to current detune factor */
						case eCmdIncDetune:
							SomethingChanged = OneL(NoteCommand,"Adjust Detuning:  Enter an "
								"adjustment detuning value (negative values decrease pitch; positive "
								"values increase pitch).","Detuning Adjustment:");
							break;

						/* <1i>:  <0: Hertz, >=0: half-steps */
						case eCmdDetuneMode:
							SomethingChanged = OneBool(NoteCommand,"Detuning Mode:  Choose whether "
								"the detuning value is in Hertz or halfsteps.","Hertz","Halfsteps");
							break;

						/* <1l> = new detune, <2xs> = # of beats */
						case eCmdSweepDetuneAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Detuning Absolute:  "
								"Enter a destination detuning value and the number of beats to spread "
								"the transition across.","Destination Detuning:","Duration:");
							break;

						/* <1l> = detune adjust, <2xs> = # of beats */
						case eCmdSweepDetuneRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Detuning Relative:  "
								"Enter an adjustment detuning value and the number of beats to spread "
								"the transition across.","Detuning Adjustment:","Duration:");
							break;

						/* restore the default early/late adjust value */
						case eCmdRestoreEarlyLateAdjust:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set the early/late adjust value to <1l> */
						case eCmdSetEarlyLateAdjust:
							SomethingChanged = OneL(NoteCommand,"Set Early/Late Hit Adjust:  Enter "
								"an early/late hit time adjustment (negative values make note hit "
								"earlier; positive values make note hit later).","Early/Late Adjust:");
							break;

						/* add <1l> to the current early/late adjust value */
						case eCmdIncEarlyLateAdjust:
							SomethingChanged = OneL(NoteCommand,"Adjust Early/Late Hit Adjust:  Enter "
								"an adjustment early/late hit time value (negative values make note "
								"hit earlier; positive values make note hit later).",
								"Early/Late Adjust:");
							break;

						/* <1l> = new early/late adjust, <2xs> = # of beats */
						case eCmdSweepEarlyLateAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Early/Late Hit Adjust "
								"Absolute:  Enter a destination early/late hit time adjustment and "
								"the number of beats to spread the transition across.",
								"Destination Early/Late Adjust:","Duration:");
							break;

						/* <1l> = early/late delta, <2xs> = # of beats to get there */
						case eCmdSweepEarlyLateRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Early/Late Hit Adjust "
								"Relative:  Enter an adjustment early/late hit time value and the "
								"number of beats to spread the transition across","Early/Late Adjust:",
								"Duration:");
							break;

						/* restore the default duration adjust value */
						case eCmdRestoreDurationAdjust:
							SomethingChanged = False; /* no parameters to edit */
							break;

						/* set duration adjust value to <1l> */
						case eCmdSetDurationAdjust:
							SomethingChanged = OneL(NoteCommand,"Set Duration Adjust:  Enter a "
								"duration adjust factor.","Duration Adjust:");
							break;

						/* add <1l> to the current duration adjust value */
						case eCmdIncDurationAdjust:
							SomethingChanged = OneL(NoteCommand,"Adjust Duration Adjust:  Enter an "
								"adjustment duration adjust factor (negative values make note shorter; "
								"positive values make note longer).","Duratin Adjust:");
							break;

						/* <1l> = new duration adjust, <2xs> = # of beats */
						case eCmdSweepDurationAbs:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Duration Adjust Absolute:"
								"  Enter a destination duration adjust factor and the number of beats "
								"to spread the transition across.","Dest. Duration Adjust:",
								"Duration:");
							break;

						/* <1l> = duration adjust delta, <2xs> = # of beats */
						case eCmdSweepDurationRel:
							SomethingChanged = OneLTwoXS(NoteCommand,"Sweep Duration Adjust Relative:"
								"  Enter an adjustment duration adjust value and the number of beats "
								"to spread the transition across.","Duration Adjust:","Duration:");
							break;

						/* <1i>:  <0: Multiplicative, >=0: Additive */
						case eCmdDurationAdjustMode:
							SomethingChanged = OneBool(NoteCommand,"Set Duration Adjust Mode:  "
								"Choose whether the duration adjust scales the note's duration "
								"by multiplication or addition.","Multiply note's duration by value",
								"Add note's duration and value");
							break;

						/* <1i> = numerator, <2i> = denominator */
						case eCmdSetMeter:
							SomethingChanged = TwoI(NoteCommand,"Set Meter:  Enter the time "
								"signature for measure bar placement.","Beats per Measure:",
								"Beat Reference Note:");
							break;

						/* <1i> = new number */
						case eCmdSetMeasureNumber:
							SomethingChanged = OneI(NoteCommand,"Set Measure Number:  Enter the "
								"number for the next measure bar.","Next Measure Number:");
							break;

						/* <1i> = new transpose value */
						case eCmdSetTranspose:
							SomethingChanged = OneI(NoteCommand,"Set Transpose:  enter the number "
								"of half-steps to transpose by.","Half-steps:");
							break;

						/* <1i> = adjusting transpose value */
						case eCmdAdjustTranspose:
							SomethingChanged = OneI(NoteCommand,"Adjust Transpose:  enter the "
								"number of half-steps to adjust the current transpose "
								"value by.","Half-steps:");
							break;

						/* <string> holds the text */
						case eCmdMarker:
							SomethingChanged = OneStr(NoteCommand,"Comment:  Enter a new comment.",
								"Comment:");
							break;

						default:
							EXECUTE(PRERR(AllowResume,"EditNoteOrCommandAttributes:  unknown command"));
							break;
					}
				ERROR((SomethingChanged != False) && (SomethingChanged != True),PRERR(
					AllowResume,"EditNoteOrCommandAttributes:  SomethingChanged is neither "
					"true nor false."));

				if (SomethingChanged)
					{
						TrackObjectAltered(Track,0);
					}
			}
	}
