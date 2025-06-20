/* PlayPrefsDialog.c */
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

#include "PlayPrefsDialog.h"
#include "Memory.h"
#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"
#include "Alert.h"
#include "MainWindowStuff.h"
#include "StringList.h"
#include "TrackList.h"
#include "Array.h"
#include "TrackObject.h"
#include "TextEdit.h"
#include "DataMunging.h"
#include "SimpleButton.h"
#include "Numbers.h"
#include "Main.h"
#include "RadioButton.h"
#include "CheckBox.h"
#include "SampleDeviceOutput.h"
#include "DiskFileOutput.h"
#include "PlayAIFFFile.h"


#define WINXSIZE (465)

#define INDENTVALUE (100)

#define TRACKLISTX (0)
#define TRACKLISTY (0)
#define TRACKLISTWIDTH (200)
#define TRACKLISTHEIGHT (240)

#define SELECTALLBUTTONWIDTH (85)
#define SELECTALLBUTTONHEIGHT (21)
#define SELECTALLBUTTONX (TRACKLISTX + (TRACKLISTWIDTH / 2)\
					- ((TRACKLISTWIDTH / 2) / 2) - (SELECTALLBUTTONWIDTH / 2))
#define SELECTALLBUTTONY (TRACKLISTY + TRACKLISTHEIGHT + 5)

#define SELECTNONEBUTTONWIDTH (SELECTALLBUTTONWIDTH)
#define SELECTNONEBUTTONHEIGHT (SELECTALLBUTTONHEIGHT)
#define SELECTNONEBUTTONX (TRACKLISTX + (TRACKLISTWIDTH / 2)\
					+ ((TRACKLISTWIDTH / 2) / 2) - (SELECTALLBUTTONWIDTH / 2))
#define SELECTNONEBUTTONY (SELECTALLBUTTONY)

#define CANCELBUTTONWIDTH (95)
#define CANCELBUTTONHEIGHT (SELECTALLBUTTONHEIGHT)
#define CANCELBUTTONX (((1 * WINXSIZE) / 8) - (CANCELBUTTONWIDTH / 2))
#define CANCELBUTTONY (SELECTALLBUTTONY + SELECTALLBUTTONHEIGHT + 10)

#define DONTPLAYBUTTONWIDTH (CANCELBUTTONWIDTH)
#define DONTPLAYBUTTONHEIGHT (SELECTALLBUTTONHEIGHT)
#define DONTPLAYBUTTONX (((3 * WINXSIZE) / 8) - (DONTPLAYBUTTONWIDTH / 2))
#define DONTPLAYBUTTONY (CANCELBUTTONY)

#define PLAYAUDIOBUTTONWIDTH (CANCELBUTTONWIDTH)
#define PLAYAUDIOBUTTONHEIGHT (SELECTALLBUTTONHEIGHT)
#define PLAYAUDIOBUTTONX (((5 * WINXSIZE) / 8) - (PLAYAUDIOBUTTONWIDTH / 2))
#define PLAYAUDIOBUTTONY (CANCELBUTTONY)

#define PLAYDISKBUTTONWIDTH (CANCELBUTTONWIDTH)
#define PLAYDISKBUTTONHEIGHT (SELECTALLBUTTONHEIGHT)
#define PLAYDISKBUTTONX (((7 * WINXSIZE) / 8) - (PLAYDISKBUTTONWIDTH / 2))
#define PLAYDISKBUTTONY (CANCELBUTTONY)

#define SAMPLERATEPROMPTX (TRACKLISTX + TRACKLISTWIDTH + 20)
#define SAMPLERATEPROMPTY (13)

#define SAMPLERATEEDITX (SAMPLERATEPROMPTX + INDENTVALUE)
#define SAMPLERATEEDITY (SAMPLERATEPROMPTY - 3)
#define SAMPLERATEEDITWIDTH (130)
#define SAMPLERATEEDITHEIGHT (21)

#define ENVELOPERATEPROMPTX (SAMPLERATEPROMPTX)
#define ENVELOPERATEPROMPTY (SAMPLERATEPROMPTY + SAMPLERATEEDITHEIGHT + 3)

#define ENVELOPERATEEDITX (ENVELOPERATEPROMPTX + INDENTVALUE)
#define ENVELOPERATEEDITY (ENVELOPERATEPROMPTY - 3)
#define ENVELOPERATEEDITWIDTH (SAMPLERATEEDITWIDTH)
#define ENVELOPERATEEDITHEIGHT (SAMPLERATEEDITHEIGHT)

#define BEATSPERMINPROMPTX (SAMPLERATEPROMPTX)
#define BEATSPERMINPROMPTY (ENVELOPERATEPROMPTY + ENVELOPERATEEDITHEIGHT + 3)

#define BEATSPERMINEDITX (BEATSPERMINPROMPTX + INDENTVALUE)
#define BEATSPERMINEDITY (BEATSPERMINPROMPTY - 3)
#define BEATSPERMINEDITWIDTH (SAMPLERATEEDITWIDTH)
#define BEATSPERMINEDITHEIGHT (SAMPLERATEEDITHEIGHT)

#define VOLUMEPROMPTX (SAMPLERATEPROMPTX)
#define VOLUMEPROMPTY (BEATSPERMINPROMPTY + BEATSPERMINEDITHEIGHT + 3)

#define VOLUMEEDITX (VOLUMEPROMPTX + INDENTVALUE)
#define VOLUMEEDITY (VOLUMEPROMPTY - 3)
#define VOLUMEEDITWIDTH (SAMPLERATEEDITWIDTH)
#define VOLUMEEDITHEIGHT (SAMPLERATEEDITHEIGHT)

#define SCANNINGGAPPROMPTX (SAMPLERATEPROMPTX)
#define SCANNINGGAPPROMPTY (VOLUMEPROMPTY + VOLUMEEDITHEIGHT + 3)

#define SCANNINGGAPEDITX (SCANNINGGAPPROMPTX + INDENTVALUE)
#define SCANNINGGAPEDITY (SCANNINGGAPPROMPTY - 3)
#define SCANNINGGAPEDITWIDTH (SAMPLERATEEDITWIDTH)
#define SCANNINGGAPEDITHEIGHT (SAMPLERATEEDITHEIGHT)

#define BUFFERDURATIONPROMPTX (SAMPLERATEPROMPTX)
#define BUFFERDURATIONPROMPTY (SCANNINGGAPPROMPTY + SCANNINGGAPEDITHEIGHT + 3)

#define BUFFERDURATIONEDITX (BUFFERDURATIONPROMPTX + INDENTVALUE)
#define BUFFERDURATIONEDITY (BUFFERDURATIONPROMPTY - 3)
#define BUFFERDURATIONWIDTH (SAMPLERATEEDITWIDTH)
#define BUFFERDURATIONHEIGHT (SAMPLERATEEDITHEIGHT)

#define BITS8BUTTONX (BUFFERDURATIONPROMPTX)
#define BITS8BUTTONY (BUFFERDURATIONPROMPTY + VOLUMEEDITHEIGHT + 5)
#define BITS8BUTTONWIDTH (100)
#define BITS8BUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define BITS16BUTTONX (BITS8BUTTONX)
#define BITS16BUTTONY (BITS8BUTTONY + BITS8BUTTONHEIGHT - 1)
#define BITS16BUTTONWIDTH (BITS8BUTTONWIDTH)
#define BITS16BUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define BITS24BUTTONX (BITS8BUTTONX)
#define BITS24BUTTONY (BITS16BUTTONY + BITS16BUTTONHEIGHT - 1)
#define BITS24BUTTONWIDTH (BITS8BUTTONWIDTH)
#define BITS24BUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define BITS32BUTTONX (BITS8BUTTONX)
#define BITS32BUTTONY (BITS24BUTTONY + BITS24BUTTONHEIGHT - 1)
#define BITS32BUTTONWIDTH (BITS8BUTTONWIDTH)
#define BITS32BUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define STEREOBUTTONX (BITS8BUTTONX + 105)
#define STEREOBUTTONY (BITS8BUTTONY)
#define STEREOBUTTONWIDTH (80)
#define STEREOBUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define MONOBUTTONX (STEREOBUTTONX)
#define MONOBUTTONY (STEREOBUTTONY + STEREOBUTTONHEIGHT - 1)
#define MONOBUTTONWIDTH (STEREOBUTTONWIDTH)
#define MONOBUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define INTERPTIMEBUTTONX (STEREOBUTTONX)
#define INTERPTIMEBUTTONY (MONOBUTTONY + MONOBUTTONHEIGHT + 5)
#define INTERPTIMEBUTTONWIDTH (140)
#define INTERPTIMEBUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define INTERPWAVEBUTTONX (STEREOBUTTONX)
#define INTERPWAVEBUTTONY (INTERPTIMEBUTTONY + INTERPTIMEBUTTONHEIGHT - 1)
#define INTERPWAVEBUTTONWIDTH (INTERPTIMEBUTTONWIDTH)
#define INTERPWAVEBUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define CLIPWARNBUTTONX (STEREOBUTTONX)
#define CLIPWARNBUTTONY (INTERPWAVEBUTTONY + INTERPWAVEBUTTONHEIGHT + 5)
#define CLIPWARNBUTTONWIDTH (INTERPTIMEBUTTONWIDTH)
#define CLIPWARNBUTTONHEIGHT (SAMPLERATEEDITHEIGHT)

#define WINYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 10)


typedef struct
	{
		WinType*						ScreenID;
		StringListRec*			TrackActivationList;
		SimpleButtonRec*		SelectAllButton;
		SimpleButtonRec*		SelectNoneButton;
		SimpleButtonRec*		CancelButton;
		SimpleButtonRec*		DontPlayButton;
		SimpleButtonRec*		PlayAudioButton;
		SimpleButtonRec*		PlayDiskButton;
		TextEditRec*				SampleRateEdit;
		TextEditRec*				EnvelopeRateEdit;
		TextEditRec*				BeatsPerMinEdit;
		TextEditRec*				VolumeEdit;
		TextEditRec*				ScanningGapEdit;
		TextEditRec*				DurationWidthEdit;
		RadioButtonRec*			Bits8Button;
		RadioButtonRec*			Bits16Button;
		RadioButtonRec*			Bits24Button;
		RadioButtonRec*			Bits32Button;
		RadioButtonRec*			StereoButton;
		RadioButtonRec*			MonoButton;
		CheckBoxRec*				InterpTimeButton;
		CheckBoxRec*				InterpWaveButton;
		CheckBoxRec*				ClipWarnButton;
	} WindowRec;


static void					RedrawTheWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawStringList(Window->TrackActivationList);
		RedrawSimpleButton(Window->SelectAllButton);
		RedrawSimpleButton(Window->SelectNoneButton);
		RedrawSimpleButton(Window->CancelButton);
		RedrawSimpleButton(Window->DontPlayButton);
		RedrawSimpleButton(Window->PlayAudioButton);
		RedrawSimpleButton(Window->PlayDiskButton);
		TextEditFullRedraw(Window->SampleRateEdit);
		TextEditFullRedraw(Window->EnvelopeRateEdit);
		TextEditFullRedraw(Window->BeatsPerMinEdit);
		TextEditFullRedraw(Window->VolumeEdit);
		TextEditFullRedraw(Window->ScanningGapEdit);
		TextEditFullRedraw(Window->DurationWidthEdit);
		RedrawRadioButton(Window->Bits8Button);
		RedrawRadioButton(Window->Bits16Button);
		RedrawRadioButton(Window->Bits24Button);
		RedrawRadioButton(Window->Bits32Button);
		RedrawRadioButton(Window->StereoButton);
		RedrawRadioButton(Window->MonoButton);
		RedrawCheckBox(Window->InterpTimeButton);
		RedrawCheckBox(Window->InterpWaveButton);
		RedrawCheckBox(Window->ClipWarnButton);
		SetClipRect(Window->ScreenID,0,0,WINXSIZE,WINYSIZE);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Sampling Rate:",14,
			SAMPLERATEPROMPTX,SAMPLERATEPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Envelope Rate:",14,
			ENVELOPERATEPROMPTX,ENVELOPERATEPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Beats Per Minute:",17,
			BEATSPERMINPROMPTX,BEATSPERMINPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Inverse Volume:",15,
			VOLUMEPROMPTX,VOLUMEPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Scanning Gap:",13,
			SCANNINGGAPPROMPTX,SCANNINGGAPPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Buffer Seconds:",15,
			BUFFERDURATIONPROMPTX,BUFFERDURATIONPROMPTY,ePlain);
	}


typedef enum
	{
		eCancelled EXECUTE(= -21985),
		eDontPlay,
		ePlayToDevice,
		ePlayToDisk
	} CommandType;


/* show the play prefs dialog (which also allows the user to play) */
void								DoPlayPrefsDialog(struct MainWindowRec* MainWindow,
											struct TrackListRec* TrackList)
	{
		WindowRec*				Window;
		ArrayRec*					ArrayTemp;
		long							Scan;
		long							Limit;
		MyBoolean					LoopFlag;
		TextEditRec*			ActiveTextEdit;
		CommandType				Operation EXECUTE(= (CommandType)-31342);
		char*							StringTemp;

		CheckPtrExistence(MainWindow);

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),"WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to display the playback "
					"dialog box.",NIL);
				return;
			}

		Window->ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(WINXSIZE),
			DialogTopEdge(WINYSIZE),WINXSIZE,WINYSIZE,
			(void (*)(void*))&RedrawTheWindow,Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Play Parameters");

		Window->TrackActivationList = NewStringList(Window->ScreenID,TRACKLISTX,
			TRACKLISTY,TRACKLISTWIDTH,TRACKLISTHEIGHT,GetScreenFont(),9,
			StringListDoAllowMultipleSelection,"Tracks to Play:");
		if (Window->TrackActivationList == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		EnableStringList(Window->TrackActivationList);
		ArrayTemp = TrackListGetListOfAllTracks(TrackList);
		if (ArrayTemp == NIL)
			{
			 FailurePoint4:
				DisposeStringList(Window->TrackActivationList);
				goto FailurePoint3;
			}
		Limit = ArrayGetLength(ArrayTemp);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				char*							TrackName;
				TrackObjectRec*		TheTrackObject;
				char*							TrackNameNullTerminated;

				TheTrackObject = (TrackObjectRec*)ArrayGetElement(ArrayTemp,Scan);
				TrackName = TrackObjectGetNameCopy(TheTrackObject);
				if (TrackName == NIL)
					{
					 FailurePoint4a:
						DisposeArray(ArrayTemp);
						goto FailurePoint4;
					}
				TrackNameNullTerminated = BlockToStringCopy(TrackName);
				if (TrackNameNullTerminated == NIL)
					{
					 FailurePoint4b:
						ReleasePtr(TrackName);
						goto FailurePoint4a;
					}
				if (!InsertStringListElement(Window->TrackActivationList,
					TrackNameNullTerminated,NIL,TheTrackObject,False))
					{
					 FailurePoint4c:
						ReleasePtr(TrackNameNullTerminated);
						goto FailurePoint4b;
					}
				if (TrackObjectShouldItBePlayed(TheTrackObject))
					{
						SelectStringListElement(Window->TrackActivationList,TheTrackObject);
					}
				ReleasePtr(TrackNameNullTerminated);
				ReleasePtr(TrackName);
			}
		DisposeArray(ArrayTemp);

		Window->SelectAllButton = NewSimpleButton(Window->ScreenID,"Select All",
			SELECTALLBUTTONX,SELECTALLBUTTONY,SELECTALLBUTTONWIDTH,SELECTALLBUTTONHEIGHT);
		if (Window->SelectAllButton == NIL)
			{
			 FailurePoint5:
				goto FailurePoint4;
			}

		Window->SelectNoneButton = NewSimpleButton(Window->ScreenID,"Deselect All",
			SELECTNONEBUTTONX,SELECTNONEBUTTONY,SELECTNONEBUTTONWIDTH,SELECTNONEBUTTONHEIGHT);
		if (Window->SelectNoneButton == NIL)
			{
			 FailurePoint6:
				DisposeSimpleButton(Window->SelectAllButton);
				goto FailurePoint5;
			}

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",
			CANCELBUTTONX,CANCELBUTTONY,CANCELBUTTONWIDTH,CANCELBUTTONHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint7:
				DisposeSimpleButton(Window->SelectNoneButton);
				goto FailurePoint6;
			}

		Window->DontPlayButton = NewSimpleButton(Window->ScreenID,"Done",
			DONTPLAYBUTTONX,DONTPLAYBUTTONY,DONTPLAYBUTTONWIDTH,DONTPLAYBUTTONHEIGHT);
		if (Window->DontPlayButton == NIL)
			{
			 FailurePoint8:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint7;
			}

		Window->PlayAudioButton = NewSimpleButton(Window->ScreenID,"Play To Audio",
			PLAYAUDIOBUTTONX,PLAYAUDIOBUTTONY,PLAYAUDIOBUTTONWIDTH,PLAYAUDIOBUTTONHEIGHT);
		if (Window->PlayAudioButton == NIL)
			{
			 FailurePoint9:
				DisposeSimpleButton(Window->DontPlayButton);
				goto FailurePoint8;
			}

		Window->PlayDiskButton = NewSimpleButton(Window->ScreenID,"Play To Disk",
			PLAYDISKBUTTONX,PLAYDISKBUTTONY,PLAYDISKBUTTONWIDTH,PLAYDISKBUTTONHEIGHT);
		if (Window->PlayDiskButton == NIL)
			{
			 FailurePoint10:
				DisposeSimpleButton(Window->PlayAudioButton);
				goto FailurePoint9;
			}

		Window->SampleRateEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,SAMPLERATEEDITX,SAMPLERATEEDITY,SAMPLERATEEDITWIDTH,
			SAMPLERATEEDITHEIGHT);
		if (Window->SampleRateEdit == NIL)
			{
			 FailurePoint11:
				DisposeSimpleButton(Window->PlayDiskButton);
				goto FailurePoint10;
			}
		StringTemp = IntegerToString(MainWindowGetSamplingRate(MainWindow));
		if (StringTemp == NIL)
			{
			 FailurePoint12:
				DisposeTextEdit(Window->SampleRateEdit);
				goto FailurePoint11;
			}
		TextEditNewRawData(Window->SampleRateEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);

		Window->EnvelopeRateEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,ENVELOPERATEEDITX,ENVELOPERATEEDITY,ENVELOPERATEEDITWIDTH,
			ENVELOPERATEEDITHEIGHT);
		if (Window->EnvelopeRateEdit == NIL)
			{
			 FailurePoint13:
				goto FailurePoint12;
			}
		StringTemp = IntegerToString(MainWindowGetEnvelopeRate(MainWindow));
		if (StringTemp == NIL)
			{
			 FailurePoint14:
				DisposeTextEdit(Window->EnvelopeRateEdit);
				goto FailurePoint13;
			}
		TextEditNewRawData(Window->EnvelopeRateEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);

		Window->BeatsPerMinEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,BEATSPERMINEDITX,BEATSPERMINEDITY,BEATSPERMINEDITWIDTH,
			BEATSPERMINEDITHEIGHT);
		if (Window->BeatsPerMinEdit == NIL)
			{
			 FailurePoint15:
				goto FailurePoint14;
			}
		StringTemp = LongDoubleToString(MainWindowGetBeatsPerMinute(MainWindow),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint16:
				DisposeTextEdit(Window->BeatsPerMinEdit);
				goto FailurePoint15;
			}
		TextEditNewRawData(Window->BeatsPerMinEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);

		Window->VolumeEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,VOLUMEEDITX,VOLUMEEDITY,VOLUMEEDITWIDTH,VOLUMEEDITHEIGHT);
		if (Window->VolumeEdit == NIL)
			{
			 FailurePoint17:
				goto FailurePoint16;
			}
		StringTemp = LongDoubleToString(MainWindowGetVolumeScaling(MainWindow),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint18:
				DisposeTextEdit(Window->VolumeEdit);
				goto FailurePoint17;
			}
		TextEditNewRawData(Window->VolumeEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);

		Window->Bits8Button = NewRadioButton(Window->ScreenID,"8-Bit Output",
			BITS8BUTTONX,BITS8BUTTONY,BITS8BUTTONWIDTH,BITS8BUTTONHEIGHT);
		if (Window->Bits8Button == NIL)
			{
			 FailurePoint19:
				goto FailurePoint18;
			}

		Window->Bits16Button = NewRadioButton(Window->ScreenID,"16-Bit Output",
			BITS16BUTTONX,BITS16BUTTONY,BITS16BUTTONWIDTH,BITS16BUTTONHEIGHT);
		if (Window->Bits16Button == NIL)
			{
			 FailurePoint20:
				DisposeRadioButton(Window->Bits8Button);
				goto FailurePoint19;
			}

		Window->Bits24Button = NewRadioButton(Window->ScreenID,"24-Bit Output",
			BITS24BUTTONX,BITS24BUTTONY,BITS24BUTTONWIDTH,BITS24BUTTONHEIGHT);
		if (Window->Bits24Button == NIL)
			{
			 FailurePoint21:
				DisposeRadioButton(Window->Bits16Button);
				goto FailurePoint20;
			}

		Window->Bits32Button = NewRadioButton(Window->ScreenID,"32-Bit Output",
			BITS32BUTTONX,BITS32BUTTONY,BIT                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        