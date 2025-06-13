/* CommandChooser.c */
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

#include "CommandChooser.h"
#include "EventLoop.h"
#include "Screen.h"
#include "Memory.h"
#include "SimpleButton.h"
#include "StringList.h"
#include "Array.h"


#define WINXSIZE (300)
#define WINYSIZE (250)

#define SCROLLX (0)
#define SCROLLY (0)
#define SCROLLWIDTH (WINXSIZE)
#define SCROLLHEIGHT (WINYSIZE - 10 - 21 - 10)

#define OKBUTTONWIDTH (80)
#define OKBUTTONHEIGHT (21)
#define OKBUTTONY (SCROLLY + SCROLLHEIGHT + 10)
#define OKBUTTONX ((WINXSIZE / 4) - (OKBUTTONWIDTH / 2))

#define CANCELBUTTONWIDTH (OKBUTTONWIDTH)
#define CANCELBUTTONHEIGHT (OKBUTTONHEIGHT)
#define CANCELBUTTONY (OKBUTTONY)
#define CANCELBUTTONX (((3 * WINXSIZE) / 4) - (OKBUTTONWIDTH / 2))


typedef struct
	{
		NoteCommands			CommandValue;
	} InfoRecord;


typedef struct
	{
		WinType*					ScreenID;
		StringListRec*		CommandStringList;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static InfoRecord			CommandList[] =
	{
		/* tempo adjustments */
		{eCmdRestoreTempo},
		{eCmdSetTempo},
		{eCmdIncTempo},
		{eCmdSweepTempoAbs},
		{eCmdSweepTempoRel},

		/* stereo positioning adjustments */
		{eCmdRestoreStereoPosition},
		{eCmdSetStereoPosition},
		{eCmdIncStereoPosition},
		{eCmdSweepStereoAbs},
		{eCmdSweepStereoRel},

		/* surround positioning adjustments */
		{eCmdRestoreSurroundPosition},
		{eCmdSetSurroundPosition},
		{eCmdIncSurroundPosition},
		{eCmdSweepSurroundAbs},
		{eCmdSweepSurroundRel},

		/* overall volume adjustments */
		{eCmdRestoreVolume},
		{eCmdSetVolume},
		{eCmdIncVolume},
		{eCmdSweepVolumeAbs},
		{eCmdSweepVolumeRel},

		/* default release point adjustment values */
		{eCmdRestoreReleasePoint1},
		{eCmdSetReleasePoint1},
		{eCmdIncReleasePoint1},
		{eCmdReleasePointOrigin1},
		{eCmdSweepReleaseAbs1},
		{eCmdSweepReleaseRel1},

		{eCmdRestoreReleasePoint2},
		{eCmdSetReleasePoint2},
		{eCmdIncReleasePoint2},
		{eCmdReleasePointOrigin2},
		{eCmdSweepReleaseAbs2},
		{eCmdSweepReleaseRel2},

		/* set the default accent values */
		{eCmdRestoreAccent1},
		{eCmdSetAccent1},
		{eCmdIncAccent1},
		{eCmdSweepAccentAbs1},
		{eCmdSweepAccentRel1},

		{eCmdRestoreAccent2},
		{eCmdSetAccent2},
		{eCmdIncAccent2},
		{eCmdSweepAccentAbs2},
		{eCmdSweepAccentRel2},

		{eCmdRestoreAccent3},
		{eCmdSetAccent3},
		{eCmdIncAccent3},
		{eCmdSweepAccentAbs3},
		{eCmdSweepAccentRel3},

		{eCmdRestoreAccent4},
		{eCmdSetAccent4},
		{eCmdIncAccent4},
		{eCmdSweepAccentAbs4},
		{eCmdSweepAccentRel4},

		/* set pitch displacement depth adjustment */
		{eCmdRestorePitchDispDepth},
		{eCmdSetPitchDispDepth},
		{eCmdIncPitchDispDepth},
		{eCmdPitchDispDepthMode},
		{eCmdSweepPitchDispDepthAbs},
		{eCmdSweepPitchDispDepthRel},

		/* set pitch displacement rate adjustment */
		{eCmdRestorePitchDispRate},
		{eCmdSetPitchDispRate},
		{eCmdIncPitchDispRate},
		{eCmdSweepPitchDispRateAbs},
		{eCmdSweepPitchDispRateRel},

		/* set pitch displacement start point, same way as release point */
		{eCmdRestorePitchDispStart},
		{eCmdSetPitchDispStart},
		{eCmdIncPitchDispStart},
		{eCmdPitchDispStartOrigin},
		{eCmdSweepPitchDispStartAbs},
		{eCmdSweepPitchDispStartRel},

		/* hurry up adjustment */
		{eCmdRestoreHurryUp},
		{eCmdSetHurryUp},
		{eCmdIncHurryUp},
		{eCmdSweepHurryUpAbs},
		{eCmdSweepHurryUpRel},

		/* default detune */
		{eCmdRestoreDetune},
		{eCmdSetDetune},
		{eCmdIncDetune},
		{eCmdDetuneMode},
		{eCmdSweepDetuneAbs},
		{eCmdSweepDetuneRel},

		/* default early/late adjust */
		{eCmdRestoreEarlyLateAdjust},
		{eCmdSetEarlyLateAdjust},
		{eCmdIncEarlyLateAdjust},
		{eCmdSweepEarlyLateAbs},
		{eCmdSweepEarlyLateRel},

		/* default duration adjust */
		{eCmdRestoreDurationAdjust},
		{eCmdSetDurationAdjust},
		{eCmdIncDurationAdjust},
		{eCmdSweepDurationAbs},
		{eCmdSweepDurationRel},
		{eCmdDurationAdjustMode},

		/* set the meter.  this is used by the editor for placing measure bars. */
		/* measuring restarts immediately after this command */
		{eCmdSetMeter},
		/* set the measure number. */
		{eCmdSetMeasureNumber},

		/* transpose controls */
		{eCmdSetTranspose},
		{eCmdAdjustTranspose},

		/* text marker in the score */
		{eCmdMarker}
	};
#define CommandListLength (sizeof(CommandList) / sizeof(CommandList[0]))


/* this thing redraws our window */
static void				RedrawOurWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawStringList(Window->CommandStringList);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
	}


/* display a scrolling list and let the user choose a command.  if the user cancels */
/* then is returns False, otherwise True. */
MyBoolean					ChooseCommandFromList(NoteCommands* Command)
	{
		WindowRec*			Window;
		long						Scan;
		MyBoolean				LoopFlag;
		MyBoolean				DoItFlag EXECUTE(= -424);

		ERROR(Command == NIL,PRERR(ForceAbort,"ChooseCommandFromList:  Command == NIL"));
		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),
			"ChooseCommandFromList WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return False;
			}
		Window->ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(WINXSIZE),
			DialogTopEdge(WINYSIZE),WINXSIZE,WINYSIZE,(void (*)(void*))&RedrawOurWindow,Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Command Chooser");
		Window->OKButton = NewSimpleButton(Window->ScreenID,"OK",OKBUTTONX,OKBUTTONY,
			OKBUTTONWIDTH,OKBUTTONHEIGHT);
		if (Window->OKButton == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		SetDefaultButtonState(Window->OKButton,True);
		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",CANCELBUTTONX,
			CANCELBUTTONY,CANCELBUTTONWIDTH,CANCELBUTTONHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint4:
				DisposeSimpleButton(Window->OKButton);
				goto FailurePoint3;
			}
		Window->CommandStringList = NewStringList(Window->ScreenID,SCROLLX,SCROLLY,
			SCROLLWIDTH,SCROLLHEIGHT,GetScreenFont(),9,StringListDontAllowMultipleSelection,
			"Choose a Command");
		if (Window->CommandStringList == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}
		EnableStringList(Window->CommandStringList);
		for (Scan = 0; Scan < CommandListLength; Scan += 1)
			{
				if (!InsertStringListElement(Window->CommandStringList,
					GetCommandName(CommandList[Scan].CommandValue),NIL,&(CommandList[Scan]),False))
					{
					 FailurePoint6:
						DisposeStringList(Window->CommandStringList);
						goto FailurePoint5;
					}
			}

		LoopFlag = True;
		while (LoopFlag)
			{
				OrdType							X;
				OrdType							Y;
				ModifierFlags				Modifiers;
				struct MenuItemType*	MenuItem;
				char								KeyPress;

				switch (GetAnEvent(&X,&Y,&Modifiers,NIL,&MenuItem,&KeyPress))
					{
						default:
							break;
						case eCheckCursor:
							SetArrowCursor();
							break;
						case eKeyPressed:
							if (KeyPress == 13)
								{
									FlashButton(Window->OKButton);
									DoItFlag = True;
									LoopFlag = False;
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									DoItFlag = False;
									LoopFlag = False;
								}
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window->OKButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->OKButton,X,Y,NIL,NIL))
										{
											DoItFlag = True;
											LoopFlag = False;
										}
								}
							else if (SimpleButtonHitTest(Window->CancelButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->CancelButton,X,Y,NIL,NIL))
										{
											DoItFlag = False;
											LoopFlag = False;
										}
								}
							else if (StringListHitTest(Window->CommandStringList,X,Y))
								{
									if (StringListMouseDown(Window->CommandStringList,X,Y,Modifiers))
										{
											DoItFlag = True;
											LoopFlag = False;
										}
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"ChooseCommandFromList:  DoItFlag is neither true nor false"));

		if (DoItFlag)
			{
				ArrayRec*				SelectionList;

				/* find out what thing was clicked */
				SelectionList = GetListOfSelectedItems(Window->CommandStringList);
				if (SelectionList != NIL)
					{
						/* see if there really is a selection */
						if (ArrayGetLength(SelectionList) > 0)
							{
								InfoRecord*			Info;

								/* recover the info record */
								Info = (InfoRecord*)ArrayGetElement(SelectionList,0);
								*Command = Info->CommandValue;
							}
						 else
							{
								DoItFlag = False; /* there wasn't a real selection, so cancel */
							}
						DisposeArray(SelectionList);
					}
				 else
					{
						DoItFlag = False; /* out of memory, so cancel operation */
					}
			}

		DisposeStringList(Window->CommandStringList);
		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
		return DoItFlag;
	}
