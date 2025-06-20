/* NoteAttributeDialog.c */
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

#include "NoteAttributeDialog.h"
#include "Memory.h"
#include "NoteObject.h"
#include "Screen.h"
#include "Main.h"
#include "Menus.h"
#include "EventLoop.h"
#include "SimpleButton.h"
#include "TextEdit.h"
#include "RadioButton.h"
#include "Alert.h"
#include "Numbers.h"
#include "WrapTextBox.h"
#include "SymbolicDuration.h"
#include "SymbolicPitch.h"
#include "DataMunging.h"
#include "SymbolicIsItInThere.h"
#include "CheckBox.h"
#include "TrackObject.h"


#define WINXSIZE (500)

#define INDENTVALUE (80)

#define BUTTON1WIDTH (50)
#define BUTTON2WIDTH (70)
#define BUTTON3WIDTH (60)

#define DESCRIPTIONX (10)
#define DESCRIPTIONY (6)
#define DESCRIPTIONWIDTH (WINXSIZE - DESCRIPTIONX - DESCRIPTIONX)
#define DESCRIPTIONHEIGHT (15)

#define EARLYLATEADJUSTPROMPTX (DESCRIPTIONX)
#define EARLYLATEADJUSTPROMPTY (DESCRIPTIONY + DESCRIPTIONHEIGHT + 3 + 5)

#define EARLYLATEADJUSTEDITX (EARLYLATEADJUSTPROMPTX + INDENTVALUE)
#define EARLYLATEADJUSTEDITY (EARLYLATEADJUSTPROMPTY - 3)
#define EARLYLATEADJUSTEDITWIDTH (70)
#define EARLYLATEADJUSTEDITHEIGHT (19)

#define RELEASE1PROMPTX (EARLYLATEADJUSTEDITX + EARLYLATEADJUSTEDITWIDTH + 10)
#define RELEASE1PROMPTY (EARLYLATEADJUSTPROMPTY)

#define RELEASE1EDITX (RELEASE1PROMPTX + INDENTVALUE)
#define RELEASE1EDITY (RELEASE1PROMPTY - 3)
#define RELEASE1EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define RELEASE1EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define RELEASE1STARTBUTTONX (RELEASE1EDITX + RELEASE1EDITWIDTH + 5)
#define RELEASE1STARTBUTTONY (RELEASE1EDITY)
#define RELEASE1STARTBUTTONWIDTH (BUTTON1WIDTH)
#define RELEASE1STARTBUTTONHEIGHT (RELEASE1EDITHEIGHT)

#define RELEASE1ENDBUTTONX (RELEASE1STARTBUTTONX + RELEASE1STARTBUTTONWIDTH)
#define RELEASE1ENDBUTTONY (RELEASE1STARTBUTTONY)
#define RELEASE1ENDBUTTONWIDTH (BUTTON2WIDTH)
#define RELEASE1ENDBUTTONHEIGHT (RELEASE1STARTBUTTONHEIGHT)

#define RELEASE1DEFAULTBUTTONX (RELEASE1ENDBUTTONX + RELEASE1ENDBUTTONWIDTH)
#define RELEASE1DEFAULTBUTTONY (RELEASE1ENDBUTTONY)
#define RELEASE1DEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define RELEASE1DEFAULTBUTTONHEIGHT (RELEASE1ENDBUTTONHEIGHT)

#define LOUDNESSPROMPTX (EARLYLATEADJUSTPROMPTX)
#define LOUDNESSPROMPTY (EARLYLATEADJUSTPROMPTY + EARLYLATEADJUSTEDITHEIGHT - 1)

#define LOUDNESSEDITX (LOUDNESSPROMPTX + INDENTVALUE)
#define LOUDNESSEDITY (LOUDNESSPROMPTY - 3)
#define LOUDNESSEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define LOUDNESSEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define RELEASE2PROMPTX (LOUDNESSEDITX + LOUDNESSEDITWIDTH + 10)
#define RELEASE2PROMPTY (LOUDNESSPROMPTY)

#define RELEASE2EDITX (RELEASE2PROMPTX + INDENTVALUE)
#define RELEASE2EDITY (RELEASE2PROMPTY - 3)
#define RELEASE2EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define RELEASE2EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define RELEASE2STARTBUTTONX (RELEASE2EDITX + RELEASE2EDITWIDTH + 5)
#define RELEASE2STARTBUTTONY (RELEASE2EDITY)
#define RELEASE2STARTBUTTONWIDTH (BUTTON1WIDTH)
#define RELEASE2STARTBUTTONHEIGHT (RELEASE2EDITHEIGHT)

#define RELEASE2ENDBUTTONX (RELEASE2STARTBUTTONX + RELEASE2STARTBUTTONWIDTH)
#define RELEASE2ENDBUTTONY (RELEASE2STARTBUTTONY)
#define RELEASE2ENDBUTTONWIDTH (BUTTON2WIDTH)
#define RELEASE2ENDBUTTONHEIGHT (RELEASE2STARTBUTTONHEIGHT)

#define RELEASE2DEFAULTBUTTONX (RELEASE2ENDBUTTONX + RELEASE2ENDBUTTONWIDTH)
#define RELEASE2DEFAULTBUTTONY (RELEASE2ENDBUTTONY)
#define RELEASE2DEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define RELEASE2DEFAULTBUTTONHEIGHT (RELEASE2ENDBUTTONHEIGHT)

#define ACCENT1PROMPTX (EARLYLATEADJUSTPROMPTX)
#define ACCENT1PROMPTY (LOUDNESSPROMPTY + LOUDNESSEDITHEIGHT - 1)

#define ACCENT1EDITX (ACCENT1PROMPTX + INDENTVALUE)
#define ACCENT1EDITY (ACCENT1PROMPTY - 3)
#define ACCENT1EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define ACCENT1EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define ACCENT2PROMPTX (ACCENT1EDITX + ACCENT1EDITWIDTH + 10)
#define ACCENT2PROMPTY (ACCENT1PROMPTY)

#define ACCENT2EDITX (ACCENT2PROMPTX + INDENTVALUE)
#define ACCENT2EDITY (ACCENT2PROMPTY - 3)
#define ACCENT2EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define ACCENT2EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define ACCENT3PROMPTX (EARLYLATEADJUSTPROMPTX)
#define ACCENT3PROMPTY (ACCENT1PROMPTY + ACCENT1EDITHEIGHT - 1)

#define ACCENT3EDITX (ACCENT3PROMPTX + INDENTVALUE)
#define ACCENT3EDITY (ACCENT3PROMPTY - 3)
#define ACCENT3EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define ACCENT3EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define ACCENT4PROMPTX (ACCENT3EDITX + ACCENT3EDITWIDTH + 10)
#define ACCENT4PROMPTY (ACCENT3PROMPTY)

#define ACCENT4EDITX (ACCENT4PROMPTX + INDENTVALUE)
#define ACCENT4EDITY (ACCENT4PROMPTY - 3)
#define ACCENT4EDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define ACCENT4EDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PITCHDISPRATEPROMPTX (EARLYLATEADJUSTPROMPTX)
#define PITCHDISPRATEPROMPTY (ACCENT3PROMPTY + ACCENT3EDITHEIGHT - 1)

#define PITCHDISPRATEEDITX (PITCHDISPRATEPROMPTX + INDENTVALUE)
#define PITCHDISPRATEEDITY (PITCHDISPRATEPROMPTY - 3)
#define PITCHDISPRATEEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define PTICHDISPRATEEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PITCHDISPDEPTHPROMPTX (PITCHDISPRATEEDITX + PITCHDISPRATEEDITWIDTH + 10)
#define PITCHDISPDEPTHPROMPTY (PITCHDISPRATEPROMPTY)

#define PITCHDISPDEPTHEDITX (PITCHDISPDEPTHPROMPTX + INDENTVALUE)
#define PITCHDISPDEPTHEDITY (PITCHDISPDEPTHPROMPTY - 3)
#define PITCHDISPDEPTHEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define PITCHDISPDEPTHEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PITCHDISPDEPTHHERTZBUTTONX (PITCHDISPDEPTHEDITX + PITCHDISPDEPTHEDITWIDTH + 5)
#define PITCHDISPDEPTHHERTZBUTTONY (PITCHDISPDEPTHEDITY)
#define PITCHDISPDEPTHHERTZBUTTONWIDTH (BUTTON1WIDTH)
#define PITCHDISPDEPTHHERTZBUTTONHEIGHT (PITCHDISPDEPTHEDITHEIGHT)

#define PITCHDISPDEPTHHALFSTEPBUTTONX (PITCHDISPDEPTHHERTZBUTTONX\
					+ PITCHDISPDEPTHHERTZBUTTONWIDTH)
#define PITCHDISPDEPTHHALFSTEPBUTTONY (PITCHDISPDEPTHHERTZBUTTONY)
#define PITCHDISPDEPTHHALFSTEPBUTTONWIDTH (BUTTON2WIDTH)
#define PITCHDISPDEPTHHALFSTEPBUTTONHEIGHT (PITCHDISPDEPTHHERTZBUTTONHEIGHT)

#define PITCHDISPDEPTHDEFAULTBUTTONX (PITCHDISPDEPTHHALFSTEPBUTTONX\
					+ PITCHDISPDEPTHHALFSTEPBUTTONWIDTH)
#define PITCHDISPDEPTHDEFAULTBUTTONY (PITCHDISPDEPTHHALFSTEPBUTTONY)
#define PITCHDISPDEPTHDEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define PITCHDISPDEPTHDEFAULTBUTTONHEIGHT (PITCHDISPDEPTHHALFSTEPBUTTONHEIGHT)

#define HURRYUPPROMPTX (EARLYLATEADJUSTPROMPTX)
#define HURRYUPPROMPTY (PITCHDISPRATEPROMPTY + PTICHDISPRATEEDITHEIGHT - 1)

#define HURRYUPEDITX (HURRYUPPROMPTX + INDENTVALUE)
#define HURRYUPEDITY (HURRYUPPROMPTY - 3)
#define HURRYUPEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define HURRYUPEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PITCHDISPSTARTPROMPTX (HURRYUPEDITX + HURRYUPEDITWIDTH + 10)
#define PITCHDISPSTARTPROMPTY (HURRYUPPROMPTY)

#define PITCHDISPSTARTEDITX (PITCHDISPSTARTPROMPTX + INDENTVALUE)
#define PITCHDISPSTARTEDITY (PITCHDISPSTARTPROMPTY - 3)
#define PITCHDISPSTARTEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define PITCHDISPSTARTEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PITCHDISPSTARTSTARTBUTTONX (PITCHDISPSTARTEDITX + PITCHDISPSTARTEDITWIDTH + 5)
#define PITCHDISPSTARTSTARTBUTTONY (PITCHDISPSTARTEDITY)
#define PITCHDISPSTARTSTARTBUTTONWIDTH (BUTTON1WIDTH)
#define PITCHDISPSTARTSTARTBUTTONHEIGHT (PITCHDISPSTARTEDITHEIGHT)

#define PITCHDISPSTARTENDBUTTONX (PITCHDISPSTARTSTARTBUTTONX\
					+ PITCHDISPSTARTSTARTBUTTONWIDTH)
#define PITCHDISPSTARTENDBUTTONY (PITCHDISPSTARTSTARTBUTTONY)
#define PITCHDISPSTARTENDBUTTONWIDTH (BUTTON2WIDTH)
#define PITCHDISPSTARTENDBUTTONHEIGHT (PITCHDISPSTARTSTARTBUTTONHEIGHT)

#define PITCHDISPSTARTDEFAULTBUTTONX (PITCHDISPSTARTENDBUTTONX\
					+ PITCHDISPSTARTENDBUTTONWIDTH)
#define PITCHDISPSTARTDEFAULTBUTTONY (PITCHDISPSTARTENDBUTTONY)
#define PITCHDISPSTARTDEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define PITCHDISPSTARTDEFAULTBUTTONHEIGHT (PITCHDISPSTARTENDBUTTONHEIGHT)

#define STEREOPOSPROMPTX (EARLYLATEADJUSTPROMPTX)
#define STEREOPOSPROMPTY (HURRYUPPROMPTY + HURRYUPEDITHEIGHT - 1)

#define STEREOPOSEDITX (STEREOPOSPROMPTX + INDENTVALUE)
#define STEREOPOSEDITY (STEREOPOSPROMPTY - 3)
#define STEREOPOSEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define STEREOPOSEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define DETUNEPROMPTX (STEREOPOSEDITX + STEREOPOSEDITWIDTH + 10)
#define DETUNEPROMPTY (STEREOPOSPROMPTY)

#define DETUNEEDITX (DETUNEPROMPTX + INDENTVALUE)
#define DETUNEEDITY (DETUNEPROMPTY - 3)
#define DETUNEEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define DETUNEEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define DETUNEHERTZBUTTONX (DETUNEEDITX + DETUNEEDITWIDTH + 5)
#define DETUNEHERTZBUTTONY (DETUNEEDITY)
#define DETUNEHERTZBUTTONWIDTH (BUTTON1WIDTH)
#define DETUNEHERTZBUTTONHEIGHT (DETUNEEDITHEIGHT)

#define DETUNEHALFSTEPBUTTONX (DETUNEHERTZBUTTONX + DETUNEHERTZBUTTONWIDTH)
#define DETUNEHALFSTEPBUTTONY (DETUNEHERTZBUTTONY)
#define DETUNEHALFSTEPBUTTONWIDTH (BUTTON2WIDTH)
#define DETUNEHALFSTEPBUTTONHEIGHT (DETUNEHERTZBUTTONHEIGHT)

#define DETUNEDEFAULTBUTTONX (DETUNEHALFSTEPBUTTONX + DETUNEHALFSTEPBUTTONWIDTH)
#define DETUNEDEFAULTBUTTONY (DETUNEHALFSTEPBUTTONY)
#define DETUNEDEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define DETUNEDEFAULTBUTTONHEIGHT (DETUNEHALFSTEPBUTTONHEIGHT)

#define SURROUNDPOSPROMPTX (EARLYLATEADJUSTPROMPTX)
#define SURROUNDPOSPROMPTY (STEREOPOSPROMPTY + STEREOPOSEDITHEIGHT - 1)

#define SURROUNDPOSEDITX (SURROUNDPOSPROMPTX + INDENTVALUE)
#define SURROUNDPOSEDITY (SURROUNDPOSPROMPTY - 3)
#define SURROUNDPOSEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define SURROUNDPOSEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define DURATIONPROMPTX (EARLYLATEADJUSTPROMPTX)
#define DURATIONPROMPTY (SURROUNDPOSPROMPTY + STEREOPOSEDITHEIGHT - 1)

#define DURATIONEDITX (DURATIONPROMPTX + INDENTVALUE)
#define DURATIONEDITY (DURATIONPROMPTY - 3)
#define DURATIONEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define DURATIONEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define DURATIONADJUSTPROMPTX (DURATIONEDITX + DURATIONEDITWIDTH + 10)
#define DURATIONADJUSTPROMPTY (DURATIONPROMPTY)

#define DURATIONADJUSTEDITX (DURATIONADJUSTPROMPTX + INDENTVALUE)
#define DURATIONADJUSTEDITY (DURATIONADJUSTPROMPTY - 3)
#define DURATIONADJUSTEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define DURATIONADJUSTEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define DURATIONADJUSTADDBUTTONX (DURATIONADJUSTEDITX + DURATIONADJUSTEDITWIDTH + 5)
#define DURATIONADJUSTADDBUTTONY (DURATIONADJUSTEDITY)
#define DURATIONADJUSTADDBUTTONWIDTH (BUTTON1WIDTH)
#define DURATIONADJUSTADDBUTTONHEIGHT (DURATIONADJUSTEDITHEIGHT)

#define DURATIONADJUSTMULTBUTTONX (DURATIONADJUSTADDBUTTONX\
					+ DURATIONADJUSTADDBUTTONWIDTH)
#define DURATIONADJUSTMULTBUTTONY (DURATIONADJUSTADDBUTTONY)
#define DURATIONADJUSTMULTBUTTONWIDTH (BUTTON2WIDTH)
#define DURATIONADJUSTMULTBOTTONHEIGHT (DURATIONADJUSTADDBUTTONHEIGHT)

#define DURATIONADJUSTDEFAULTBUTTONX (DURATIONADJUSTMULTBUTTONX\
					+ DURATIONADJUSTMULTBUTTONWIDTH)
#define DURATIONADJUSTDEFAULTBUTTONY (DURATIONADJUSTMULTBUTTONY)
#define DURATIONADJUSTDEFAULTBUTTONWIDTH (BUTTON3WIDTH)
#define DURATIONADJUSTDEFAULTBUTTONHEIGHT (DURATIONADJUSTMULTBOTTONHEIGHT)

#define REALPITCHPROMPTX (EARLYLATEADJUSTPROMPTX)
#define REALPITCHPROMPTY (DURATIONPROMPTY + DURATIONEDITHEIGHT - 1)

#define REALPITCHEDITX (REALPITCHPROMPTX + INDENTVALUE)
#define REALPITCHEDITY (REALPITCHPROMPTY - 3)
#define REALPITCHEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define REALPITCHEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define FAKEPITCHPROMPTX (REALPITCHEDITX + REALPITCHEDITWIDTH + 10)
#define FAKEPITCHPROMPTY (REALPITCHPROMPTY)

#define FAKEPITCHEDITX (FAKEPITCHPROMPTX + INDENTVALUE)
#define FAKEPITCHEDITY (FAKEPITCHPROMPTY - 3)
#define FAKEPITCHEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define FAKEPITCHEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PORTAMENTOPROMPTX (FAKEPITCHPROMPTX)
#define PORTAMENTOPROMPTY (FAKEPITCHPROMPTY + REALPITCHEDITHEIGHT - 1)

#define PORTAMENTOEDITX (PORTAMENTOPROMPTX + INDENTVALUE)
#define PORTAMENTOEDITY (PORTAMENTOPROMPTY - 3)
#define PORTAMENTOEDITWIDTH (EARLYLATEADJUSTEDITWIDTH)
#define PORTAMENTOEDITHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define PORTAHERTZBUTTONX (PORTAMENTOEDITX + PORTAMENTOEDITWIDTH + 5)
#define PORTAHERTZBUTTONY (PORTAMENTOEDITY)
#define PORTAHERTZBUTTONWIDTH (BUTTON1WIDTH)
#define PORTAHERTZBUTTONHEIGHT (PORTAMENTOEDITHEIGHT)

#define PORTAHALFSTEPBUTTONX (PORTAHERTZBUTTONX + PORTAHERTZBUTTONWIDTH)
#define PORTAHALFSTEPBUTTONY (PORTAHERTZBUTTONY)
#define PORTAHALFSTEPBUTTONWIDTH (BUTTON2WIDTH)
#define PORTAHALFSTEPBUTTONHEIGHT (PORTAHERTZBUTTONHEIGHT)

#define RESTCHECKBOXX (EARLYLATEADJUSTPROMPTX)
#define RESTCHECKBOXY (PORTAMENTOPROMPTY - 3)
#define RESTCHECKBOXWIDTH (INDENTVALUE + EARLYLATEADJUSTEDITWIDTH)
#define RESTCHECKBOXHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define RETRIGCHECKBOXX (EARLYLATEADJUSTPROMPTX)
#define RETRIGCHECKBOXY (RESTCHECKBOXY + EARLYLATEADJUSTEDITHEIGHT - 1)
#define RETRIGCHECKBOXWIDTH (RESTCHECKBOXWIDTH)
#define RETRIGCHECKBOXHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define RELEASE3CHECKBOXX (RETRIGCHECKBOXX + INDENTVALUE + EARLYLATEADJUSTEDITWIDTH + 10)
#define RELEASE3CHECKBOXY (RETRIGCHECKBOXY)
#define RELEASE3CHECKBOXWIDTH (RESTCHECKBOXWIDTH)
#define RELEASE3CHECKBOXHEIGHT (EARLYLATEADJUSTEDITHEIGHT)

#define OKBUTTONWIDTH (80)
#define OKBUTTONHEIGHT (21)
#define OKBUTTONY (RETRIGCHECKBOXY + RETRIGCHECKBOXHEIGHT + 15)
#define OKBUTTONX ((WINXSIZE / 4) - (OKBUTTONWIDTH / 2))

#define CANCELBUTTONWIDTH (OKBUTTONWIDTH)
#define CANCELBUTTONHEIGHT (OKBUTTONHEIGHT)
#define CANCELBUTTONY (OKBUTTONY)
#define CANCELBUTTONX (((3 * WINXSIZE) / 4) - (OKBUTTONWIDTH / 2))

#define WINYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 10)


typedef struct
	{
		WinType*						ScreenID;
		SimpleButtonRec*		OKButton;
		SimpleButtonRec*		CancelButton;
		TextEditRec*				EarlyLateEdit;
		TextEditRec*				Release1Edit;
		RadioButtonRec*			Release1StartButton;
		RadioButtonRec*			Release1EndButton;
		RadioButtonRec*			Release1DefaultButton;
		TextEditRec*				LoudnessEdit;
		TextEditRec*				Release2Edit;
		RadioButtonRec*			Release2StartButton;
		RadioButtonRec*			Release2EndButton;
		RadioButtonRec*			Release2DefaultButton;
		TextEditRec*				Accent1Edit;
		TextEditRec*				Accent2Edit;
		TextEditRec*				Accent3Edit;
		TextEditRec*				Accent4Edit;
		TextEditRec*				PitchDispRateEdit;
		TextEditRec*				PitchDispDepthEdit;
		RadioButtonRec*			PitchDispDepthHertzButton;
		RadioButtonRec*			PitchDispDepthHalfStepButton;
		RadioButtonRec*			PitchDispDepthDefaultButton;
		TextEditRec*				HurryUpEdit;
		TextEditRec*				PitchDispStartEdit;
		RadioButtonRec*			PitchDispStartStartButton;
		RadioButtonRec*			PitchDispStartEndButton;
		RadioButtonRec*			PitchDispStartDefaultButton;
		TextEditRec*				StereoPosEdit;
		TextEditRec*				DetuneEdit;
		RadioButtonRec*			DetuneHertzButton;
		RadioButtonRec*			DetuneHalfstepButton;
		RadioButtonRec*			DetuneDefaultButton;
		TextEditRec*				SurroundPosEdit;
		TextEditRec*				DurationEdit;
		TextEditRec*				DurationAdjustEdit;
		RadioButtonRec*			DurationAdjustAddButton;
		RadioButtonRec*			DurationAdjustMultButton;
		RadioButtonRec*			DurationAdjustDefaultButton;
		TextEditRec*				RealPitchEdit;
		TextEditRec*				FakePitchEdit;
		TextEditRec*				PortamentoEdit;
		CheckBoxRec*				RestBox;
		RadioButtonRec*			PortaHertzButton;
		RadioButtonRec*			PortaHalfstepButton;
		CheckBoxRec*				RetrigBox;
		CheckBoxRec*				Release3Box;
	} WindowRec;


static void					UpdateTheWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		TextEditFullRedraw(Window->EarlyLateEdit);
		TextEditFullRedraw(Window->Release1Edit);
		RedrawRadioButton(Window->Release1StartButton);
		RedrawRadioButton(Window->Release1EndButton);
		RedrawRadioButton(Window->Release1DefaultButton);
		TextEditFullRedraw(Window->LoudnessEdit);
		TextEditFullRedraw(Window->Release2Edit);
		RedrawRadioButton(Window->Release2StartButton);
		RedrawRadioButton(Window->Release2EndButton);
		RedrawRadioButton(Window->Release2DefaultButton);
		TextEditFullRedraw(Window->Accent1Edit);
		TextEditFullRedraw(Window->Accent2Edit);
		TextEditFullRedraw(Window->Accent3Edit);
		TextEditFullRedraw(Window->Accent4Edit);
		TextEditFullRedraw(Window->PitchDispRateEdit);
		TextEditFullRedraw(Window->PitchDispDepthEdit);
		RedrawRadioButton(Window->PitchDispDepthHertzButton);
		RedrawRadioButton(Window->PitchDispDepthHalfStepButton);
		RedrawRadioButton(Window->PitchDispDepthDefaultButton);
		TextEditFullRedraw(Window->HurryUpEdit);
		TextEditFullRedraw(Window->PitchDispStartEdit);
		RedrawRadioButton(Window->PitchDispStartStartButton);
		RedrawRadioButton(Window->PitchDispStartEndButton);
		RedrawRadioButton(Window->PitchDispStartDefaultButton);
		TextEditFullRedraw(Window->StereoPosEdit);
		TextEditFullRedraw(Window->DetuneEdit);
		RedrawRadioButton(Window->DetuneHertzButton);
		RedrawRadioButton(Window->DetuneHalfstepButton);
		RedrawRadioButton(Window->DetuneDefaultButton);
		TextEditFullRedraw(Window->SurroundPosEdit);
		TextEditFullRedraw(Window->DurationEdit);
		TextEditFullRedraw(Window->DurationAdjustEdit);
		RedrawRadioButton(Window->DurationAdjustAddButton);
		RedrawRadioButton(Window->DurationAdjustMultButton);
		RedrawRadioButton(Window->DurationAdjustDefaultButton);
		TextEditFullRedraw(Window->RealPitchEdit);
		TextEditFullRedraw(Window->FakePitchEdit);
		TextEditFullRedraw(Window->PortamentoEdit);
		RedrawCheckBox(Window->RestBox);
		RedrawRadioButton(Window->PortaHertzButton);
		RedrawRadioButton(Window->PortaHalfstepButton);
		RedrawCheckBox(Window->RetrigBox);
		RedrawCheckBox(Window->Release3Box);
		SetClipRect(Window->ScreenID,0,0,WINXSIZE,WINYSIZE);
		DrawWrappedTextBox(Window->ScreenID,"Adjust the note's attributes:",
			GetScreenFont(),9,DESCRIPTIONX,DESCRIPTIONY,DESCRIPTIONWIDTH,DESCRIPTIONHEIGHT);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Hit Time:",9,
			EARLYLATEADJUSTPROMPTX,EARLYLATEADJUSTPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Release 1:",10,
			RELEASE1PROMPTX,RELEASE1PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Loudness:",9,
			LOUDNESSPROMPTX,LOUDNESSPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Release 2:",10,
			RELEASE2PROMPTX,RELEASE2PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Accent 1:",9,
			ACCENT1PROMPTX,ACCENT1PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Accent 2:",9,
			ACCENT2PROMPTX,ACCENT2PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Accent 3:",9,
			ACCENT3PROMPTX,ACCENT3PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Accent 4:",9,
			ACCENT4PROMPTX,ACCENT4PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Pitch Disp Rate:",16,
			PITCHDISPRATEPROMPTX,PITCHDISPRATEPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Pitch Disp Depth:",17,
			PITCHDISPDEPTHPROMPTX,PITCHDISPDEPTHPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Hurry-Up:",9,
			HURRYUPPROMPTX,HURRYUPPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Pitch Disp Start:",17,
			PITCHDISPSTARTPROMPTX,PITCHDISPSTARTPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Stereo Pos:",11,
			STEREOPOSPROMPTX,STEREOPOSPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Detune:",7,
			DETUNEPROMPTX,DETUNEPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Surround Pos:",13,
			SURROUNDPOSPROMPTX,SURROUNDPOSPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Duration:",9,
			DURATIONPROMPTX,DURATIONPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Duration Adjust:",16,
			DURATIONADJUSTPROMPTX,DURATIONADJUSTPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Real Pitch:",11,
			REALPITCHPROMPTX,REALPITCHPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Effective Pitch:",16,
			FAKEPITCHPROMPTX,FAKEPITCHPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Portamento:",11,
			PORTAMENTOPROMPTX,PORTAMENTOPROMPTY,ePlain);
	}


/* show dialog box allowing user to edit note's parameters */
void						EditNoteParametersDialog(NoteObjectRec* Note, TrackObjectRec* Track)
	{
		WindowRec*				Window;
		char*							StringTemp;
		MyBoolean					LoopFlag;
		TextEditRec*			ActiveTextEdit;
		MyBoolean					DoItFlag EXECUTE(= -31342);

		CheckPtrExistence(Note);

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),"TrackAttributeDialogWindow");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to edit the note's attributes.",
					NIL);
				return;
			}

		Window->ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(WINXSIZE),
			DialogTopEdge(WINYSIZE),WINXSIZE,WINYSIZE,(void (*)(void*))&UpdateTheWindow,Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Edit Note");

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

		Window->EarlyLateEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,EARLYLATEADJUSTEDITX,EARLYLATEADJUSTEDITY,EARLYLATEADJUSTEDITWIDTH,
			EARLYLATEADJUSTEDITHEIGHT);
		if (Window->EarlyLateEdit == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}
		StringTemp = LongDoubleToString(GetNoteEarlyLateAdjust(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->EarlyLateEdit);
				goto FailurePoint5;
			}
		TextEditNewRawData(Window->EarlyLateEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->EarlyLateEdit);

		Window->Release1Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,RELEASE1EDITX,RELEASE1EDITY,RELEASE1EDITWIDTH,RELEASE1EDITHEIGHT);
		if (Window->Release1Edit == NIL)
			{
			 FailurePoint7:
				goto FailurePoint6;
			}
		StringTemp = LongDoubleToString(GetNoteReleasePoint1(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint8:
				DisposeTextEdit(Window->Release1Edit);
				goto FailurePoint7;
			}
		TextEditNewRawData(Window->Release1Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Release1Edit);

		Window->Release1StartButton = NewRadioButton(Window->ScreenID,"Start",
			RELEASE1STARTBUTTONX,RELEASE1STARTBUTTONY,RELEASE1STARTBUTTONWIDTH,
			RELEASE1STARTBUTTONHEIGHT);
		if (Window->Release1StartButton == NIL)
			{
			 FailurePoint9:
				goto FailurePoint8;
			}

		Window->Release1EndButton = NewRadioButton(Window->ScreenID,"End",
			RELEASE1ENDBUTTONX,RELEASE1ENDBUTTONY,RELEASE1ENDBUTTONWIDTH,
			RELEASE1ENDBUTTONHEIGHT);
		if (Window->Release1EndButton == NIL)
			{
			 FailurePoint10:
				DisposeRadioButton(Window->Release1StartButton);
				goto FailurePoint9;
			}

		Window->Release1DefaultButton = NewRadioButton(Window->ScreenID,"Default",
			RELEASE1DEFAULTBUTTONX,RELEASE1DEFAULTBUTTONY,RELEASE1DEFAULTBUTTONWIDTH,
			RELEASE1DEFAULTBUTTONHEIGHT);
		if (Window->Release1DefaultButton == NIL)
			{
			 FailurePoint11:
				DisposeRadioButton(Window->Release1EndButton);
				goto FailurePoint10;
			}

		switch (GetNoteRelease1Origin(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad release point 1 start/end flag"));
					break;
				case eRelease1FromStart:
					SetRadioButtonState(Window->Release1StartButton,True);
					break;
				case eRelease1FromEnd:
					SetRadioButtonState(Window->Release1EndButton,True);
					break;
				case eRelease1FromDefault:
					SetRadioButtonState(Window->Release1DefaultButton,True);
					break;
			}

		Window->LoudnessEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,LOUDNESSEDITX,LOUDNESSEDITY,LOUDNESSEDITWIDTH,LOUDNESSEDITHEIGHT);
		if (Window->LoudnessEdit == NIL)
			{
			 FailurePoint12:
				DisposeRadioButton(Window->Release1DefaultButton);
				goto FailurePoint11;
			}
		StringTemp = LongDoubleToString(GetNoteOverallLoudnessAdjustment(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint13:
				DisposeTextEdit(Window->LoudnessEdit);
				goto FailurePoint12;
			}
		TextEditNewRawData(Window->LoudnessEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->LoudnessEdit);

		Window->Release2Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,RELEASE2EDITX,RELEASE2EDITY,RELEASE2EDITWIDTH,RELEASE2EDITHEIGHT);
		if (Window->Release2Edit == NIL)
			{
			 FailurePoint14:
				goto FailurePoint13;
			}
		StringTemp = LongDoubleToString(GetNoteReleasePoint2(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint15:
				DisposeTextEdit(Window->Release2Edit);
				goto FailurePoint14;
			}
		TextEditNewRawData(Window->Release2Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Release2Edit);

		Window->Release2StartButton = NewRadioButton(Window->ScreenID,"Start",
			RELEASE2STARTBUTTONX,RELEASE2STARTBUTTONY,RELEASE2STARTBUTTONWIDTH,
			RELEASE2STARTBUTTONHEIGHT);
		if (Window->Release2StartButton == NIL)
			{
			 FailurePoint16:
				goto FailurePoint15;
			}

		Window->Release2EndButton = NewRadioButton(Window->ScreenID,"End",
			RELEASE2ENDBUTTONX,RELEASE2ENDBUTTONY,RELEASE2ENDBUTTONWIDTH,
			RELEASE2ENDBUTTONHEIGHT);
		if (Window->Release2EndButton == NIL)
			{
			 FailurePoint17:
				DisposeRadioButton(Window->Release2StartButton);
				goto FailurePoint16;
			}

		Window->Release2DefaultButton = NewRadioButton(Window->ScreenID,"Default",
			RELEASE2DEFAULTBUTTONX,RELEASE2DEFAULTBUTTONY,RELEASE2DEFAULTBUTTONWIDTH,
			RELEASE2DEFAULTBUTTONHEIGHT);
		if (Window->Release2DefaultButton == NIL)
			{
			 FailurePoint18:
				DisposeRadioButton(Window->Release2EndButton);
				goto FailurePoint17;
			}

		switch (GetNoteRelease2Origin(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad release point 2 start/end flag"));
					break;
				case eRelease2FromStart:
					SetRadioButtonState(Window->Release2StartButton,True);
					break;
				case eRelease2FromEnd:
					SetRadioButtonState(Window->Release2EndButton,True);
					break;
				case eRelease2FromDefault:
					SetRadioButtonState(Window->Release2DefaultButton,True);
					break;
			}

		Window->Accent1Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,ACCENT1EDITX,ACCENT1EDITY,ACCENT1EDITWIDTH,ACCENT1EDITHEIGHT);
		if (Window->Accent1Edit == NIL)
			{
			 FailurePoint19:
				DisposeRadioButton(Window->Release2DefaultButton);
				goto FailurePoint18;
			}
		StringTemp = LongDoubleToString(GetNoteAccent1(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint20:
				DisposeTextEdit(Window->Accent1Edit);
				goto FailurePoint19;
			}
		TextEditNewRawData(Window->Accent1Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Accent1Edit);

		Window->Accent2Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,ACCENT2EDITX,ACCENT2EDITY,ACCENT2EDITWIDTH,ACCENT2EDITHEIGHT);
		if (Window->Accent2Edit == NIL)
			{
			 FailurePoint21:
				goto FailurePoint20;
			}
		StringTemp = LongDoubleToString(GetNoteAccent2(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint22:
				DisposeTextEdit(Window->Accent2Edit);
				goto FailurePoint21;
			}
		TextEditNewRawData(Window->Accent2Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Accent2Edit);

		Window->Accent3Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,ACCENT3EDITX,ACCENT3EDITY,ACCENT3EDITWIDTH,ACCENT3EDITHEIGHT);
		if (Window->Accent3Edit == NIL)
			{
			 FailurePoint23:
				goto FailurePoint22;
			}
		StringTemp = LongDoubleToString(GetNoteAccent3(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint24:
				DisposeTextEdit(Window->Accent3Edit);
				goto FailurePoint23;
			}
		TextEditNewRawData(Window->Accent3Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Accent3Edit);

		Window->Accent4Edit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,ACCENT4EDITX,ACCENT4EDITY,ACCENT4EDITWIDTH,ACCENT4EDITHEIGHT);
		if (Window->Accent4Edit == NIL)
			{
			 FailurePoint25:
				goto FailurePoint24;
			}
		StringTemp = LongDoubleToString(GetNoteAccent4(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint26:
				DisposeTextEdit(Window->Accent4Edit);
				goto FailurePoint25;
			}
		TextEditNewRawData(Window->Accent4Edit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->Accent4Edit);

		Window->PitchDispRateEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,PITCHDISPRATEEDITX,PITCHDISPRATEEDITY,PITCHDISPRATEEDITWIDTH,
			PTICHDISPRATEEDITHEIGHT);
		if (Window->PitchDispRateEdit == NIL)
			{
			 FailurePoint27:
				goto FailurePoint26;
			}
		StringTemp = LongDoubleToString(GetNotePitchDisplacementRateAdjust(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint28:
				DisposeTextEdit(Window->PitchDispRateEdit);
				goto FailurePoint27;
			}
		TextEditNewRawData(Window->PitchDispRateEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->PitchDispRateEdit);

		Window->PitchDispDepthEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,PITCHDISPDEPTHEDITX,PITCHDISPDEPTHEDITY,PITCHDISPDEPTHEDITWIDTH,
			PITCHDISPDEPTHEDITHEIGHT);
		if (Window->PitchDispDepthEdit == NIL)
			{
			 FailurePoint29:
				goto FailurePoint28;
			}
		StringTemp = LongDoubleToString(GetNotePitchDisplacementDepthAdjust(Note),
			7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint30:
				DisposeTextEdit(Window->PitchDispDepthEdit);
				goto FailurePoint29;
			}
		TextEditNewRawData(Window->PitchDispDepthEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->PitchDispDepthEdit);

		Window->PitchDispDepthHertzButton = NewRadioButton(Window->ScreenID,"Hertz",
			PITCHDISPDEPTHHERTZBUTTONX,PITCHDISPDEPTHHERTZBUTTONY,
			PITCHDISPDEPTHHERTZBUTTONWIDTH,PITCHDISPDEPTHHERTZBUTTONHEIGHT);
		if (Window->PitchDispDepthHertzButton == NIL)
			{
			 FailurePoint31:
				goto FailurePoint30;
			}

		Window->PitchDispDepthHalfStepButton = NewRadioButton(Window->ScreenID,"Halfsteps",
			PITCHDISPDEPTHHALFSTEPBUTTONX,PITCHDISPDEPTHHALFSTEPBUTTONY,
			PITCHDISPDEPTHHALFSTEPBUTTONWIDTH,PITCHDISPDEPTHHALFSTEPBUTTONHEIGHT);
		if (Window->PitchDispDepthHalfStepButton == NIL)
			{
			 FailurePoint32:
				DisposeRadioButton(Window->PitchDispDepthHertzButton);
				goto FailurePoint31;
			}

		Window->PitchDispDepthDefaultButton = NewRadioButton(Window->ScreenID,"Default",
			PITCHDISPDEPTHDEFAULTBUTTONX,PITCHDISPDEPTHDEFAULTBUTTONY,
			PITCHDISPDEPTHDEFAULTBUTTONWIDTH,PITCHDISPDEPTHDEFAULTBUTTONHEIGHT);
		if (Window->PitchDispDepthDefaultButton == NIL)
			{
			 FailurePoint33:
				DisposeRadioButton(Window->PitchDispDepthHalfStepButton);
				goto FailurePoint32;
			}

		switch (GetNotePitchDisplacementDepthConversionMode(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad pitch disp depth control flag"));
					break;
				case ePitchDisplacementDepthModeHertz:
					SetRadioButtonState(Window->PitchDispDepthHertzButton,True);
					break;
				case ePitchDisplacementDepthModeHalfSteps:
					SetRadioButtonState(Window->PitchDispDepthHalfStepButton,True);
					break;
				case ePitchDisplacementDepthModeDefault:
					SetRadioButtonState(Window->PitchDispDepthDefaultButton,True);
					break;
			}

		Window->HurryUpEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,HURRYUPEDITX,HURRYUPEDITY,HURRYUPEDITWIDTH,HURRYUPEDITHEIGHT);
		if (Window->HurryUpEdit == NIL)
			{
			 FailurePoint34:
				DisposeRadioButton(Window->PitchDispDepthDefaultButton);
				goto FailurePoint33;
			}
		StringTemp = LongDoubleToString(GetNoteHurryUpFactor(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint35:
				DisposeTextEdit(Window->HurryUpEdit);
				goto FailurePoint34;
			}
		TextEditNewRawData(Window->HurryUpEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->HurryUpEdit);

		Window->PitchDispStartEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,PITCHDISPSTARTEDITX,PITCHDISPSTARTEDITY,
			PITCHDISPSTARTEDITWIDTH,PITCHDISPSTARTEDITHEIGHT);
		if (Window->PitchDispStartEdit == NIL)
			{
			 FailurePoint36:
				goto FailurePoint35;
			}
		StringTemp = LongDoubleToString(GetNotePitchDisplacementStartPoint(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint37:
				DisposeTextEdit(Window->PitchDispStartEdit);
				goto FailurePoint36;
			}
		TextEditNewRawData(Window->PitchDispStartEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->PitchDispStartEdit);

		Window->PitchDispStartStartButton = NewRadioButton(Window->ScreenID,"Start",
			PITCHDISPSTARTSTARTBUTTONX,PITCHDISPSTARTSTARTBUTTONY,
			PITCHDISPSTARTSTARTBUTTONWIDTH,PITCHDISPSTARTSTARTBUTTONHEIGHT);
		if (Window->PitchDispStartStartButton == NIL)
			{
			 FailurePoint38:
				goto FailurePoint37;
			}

		Window->PitchDispStartEndButton = NewRadioButton(Window->ScreenID,"End",
			PITCHDISPSTARTENDBUTTONX,PITCHDISPSTARTENDBUTTONY,
			PITCHDISPSTARTENDBUTTONWIDTH,PITCHDISPSTARTENDBUTTONHEIGHT);
		if (Window->PitchDispStartEndButton == NIL)
			{
			 FailurePoint39:
				DisposeRadioButton(Window->PitchDispStartStartButton);
				goto FailurePoint38;
			}

		Window->PitchDispStartDefaultButton = NewRadioButton(Window->ScreenID,"Default",
			PITCHDISPSTARTDEFAULTBUTTONX,PITCHDISPSTARTDEFAULTBUTTONY,
			PITCHDISPSTARTDEFAULTBUTTONWIDTH,PITCHDISPSTARTDEFAULTBUTTONHEIGHT);
		if (Window->PitchDispStartDefaultButton == NIL)
			{
			 FailurePoint40:
				DisposeRadioButton(Window->PitchDispStartEndButton);
				goto FailurePoint39;
			}

		switch (GetNotePitchDisplacementStartOrigin(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad pitch disp start origin flag"));
					break;
				case ePitchDisplacementStartFromStart:
					SetRadioButtonState(Window->PitchDispStartStartButton,True);
					break;
				case ePitchDisplacementStartFromEnd:
					SetRadioButtonState(Window->PitchDispStartEndButton,True);
					break;
				case ePitchDisplacementStartFromDefault:
					SetRadioButtonState(Window->PitchDispStartDefaultButton,True);
					break;
			}

		Window->StereoPosEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,STEREOPOSEDITX,STEREOPOSEDITY,STEREOPOSEDITWIDTH,
			STEREOPOSEDITHEIGHT);
		if (Window->StereoPosEdit == NIL)
			{
			 FailurePoint41:
				DisposeRadioButton(Window->PitchDispStartDefaultButton);
				goto FailurePoint40;
			}
		StringTemp = LongDoubleToString(GetNoteStereoPositioning(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint42:
				DisposeTextEdit(Window->StereoPosEdit);
				goto FailurePoint41;
			}
		TextEditNewRawData(Window->StereoPosEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->StereoPosEdit);

		Window->DetuneEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,DETUNEEDITX,DETUNEEDITY,DETUNEEDITWIDTH,DETUNEEDITHEIGHT);
		if (Window->DetuneEdit == NIL)
			{
			 FailurePoint43:
				goto FailurePoint42;
			}
		StringTemp = LongDoubleToString(GetNoteDetuning(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint44:
				DisposeTextEdit(Window->DetuneEdit);
				goto FailurePoint43;
			}
		TextEditNewRawData(Window->DetuneEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->DetuneEdit);

		Window->DetuneHertzButton = NewRadioButton(Window->ScreenID,"Hertz",
			DETUNEHERTZBUTTONX,DETUNEHERTZBUTTONY,DETUNEHERTZBUTTONWIDTH,
			DETUNEHERTZBUTTONHEIGHT);
		if (Window->DetuneHertzButton == NIL)
			{
			 FailurePoint45:
				goto FailurePoint44;
			}

		Window->DetuneHalfstepButton = NewRadioButton(Window->ScreenID,"Halfsteps",
			DETUNEHALFSTEPBUTTONX,DETUNEHALFSTEPBUTTONY,DETUNEHALFSTEPBUTTONWIDTH,
			DETUNEHALFSTEPBUTTONHEIGHT);
		if (Window->DetuneHalfstepButton == NIL)
			{
			 FailurePoint46:
				DisposeRadioButton(Window->DetuneHertzButton);
				goto FailurePoint45;
			}

		Window->DetuneDefaultButton = NewRadioButton(Window->ScreenID,"Default",
			DETUNEDEFAULTBUTTONX,DETUNEDEFAULTBUTTONY,DETUNEDEFAULTBUTTONWIDTH,
			DETUNEDEFAULTBUTTONHEIGHT);
		if (Window->DetuneDefaultButton == NIL)
			{
			 FailurePoint47:
				DisposeRadioButton(Window->DetuneHalfstepButton);
				goto FailurePoint46;
			}

		switch (GetNoteDetuneConversionMode(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad detune conversion mode flag"));
					break;
				case eDetuningModeHertz:
					SetRadioButtonState(Window->DetuneHertzButton,True);
					break;
				case eDetuningModeHalfSteps:
					SetRadioButtonState(Window->DetuneHalfstepButton,True);
					break;
				case eDetuningModeDefault:
					SetRadioButtonState(Window->DetuneDefaultButton,True);
					break;
			}

		Window->DurationEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,DURATIONEDITX,DURATIONEDITY,DURATIONEDITWIDTH,
			DURATIONEDITHEIGHT);
		if (Window->DurationEdit == NIL)
			{
			 FailurePoint48:
				DisposeRadioButton(Window->DetuneDefaultButton);
				goto FailurePoint47;
			}
		StringTemp = NumericDurationToString(GetNoteDuration(Note),GetNoteDotStatus(Note),
			GetNoteDurationDivision(Note));
		if (StringTemp == NIL)
			{
			 FailurePoint49:
				DisposeTextEdit(Window->DurationEdit);
				goto FailurePoint48;
			}
		TextEditNewRawData(Window->DurationEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->DurationEdit);

		Window->DurationAdjustEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,DURATIONADJUSTEDITX,DURATIONADJUSTEDITY,
			DURATIONADJUSTEDITWIDTH,DURATIONADJUSTEDITHEIGHT);
		if (Window->DurationAdjustEdit == NIL)
			{
			 FailurePoint50:
				goto FailurePoint49;
			}
		StringTemp = LongDoubleToString(GetNoteDurationAdjust(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint51:
				DisposeTextEdit(Window->DurationAdjustEdit);
				goto FailurePoint50;
			}
		TextEditNewRawData(Window->DurationAdjustEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->DurationAdjustEdit);

		Window->DurationAdjustAddButton = NewRadioButton(Window->ScreenID,"Add",
			DURATIONADJUSTADDBUTTONX,DURATIONADJUSTADDBUTTONY,DURATIONADJUSTADDBUTTONWIDTH,
			DURATIONADJUSTADDBUTTONHEIGHT);
		if (Window->DurationAdjustAddButton == NIL)
			{
			 FailurePoint52:
				goto FailurePoint51;
			}

		Window->DurationAdjustMultButton = NewRadioButton(Window->ScreenID,"Mult",
			DURATIONADJUSTMULTBUTTONX,DURATIONADJUSTMULTBUTTONY,DURATIONADJUSTMULTBUTTONWIDTH,
			DURATIONADJUSTMULTBOTTONHEIGHT);
		if (Window->DurationAdjustMultButton == NIL)
			{
			 FailurePoint53:
				DisposeRadioButton(Window->DurationAdjustAddButton);
				goto FailurePoint52;
			}

		Window->DurationAdjustDefaultButton = NewRadioButton(Window->ScreenID,"Default",
			DURATIONADJUSTDEFAULTBUTTONX,DURATIONADJUSTDEFAULTBUTTONY,
			DURATIONADJUSTDEFAULTBUTTONWIDTH,DURATIONADJUSTDEFAULTBUTTONHEIGHT);
		if (Window->DurationAdjustDefaultButton == NIL)
			{
			 FailurePoint54:
				DisposeRadioButton(Window->DurationAdjustMultButton);
				goto FailurePoint53;
			}

		switch (GetNoteDurationAdjustMode(Note))
			{
				default:
					EXECUTE(PRERR(AllowResume,
						"EditNoteParametersDialog:  bad duration adjust mode flag"));
					break;
				case eDurationAdjustAdditive:
					SetRadioButtonState(Window->DurationAdjustAddButton,True);
					break;
				case eDurationAdjustMultiplicative:
					SetRadioButtonState(Window->DurationAdjustMultButton,True);
					break;
				case eDurationAdjustDefault:
					SetRadioButtonState(Window->DurationAdjustDefaultButton,True);
					break;
			}

		Window->RealPitchEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,REALPITCHEDITX,REALPITCHEDITY,REALPITCHEDITWIDTH,
			REALPITCHEDITHEIGHT);
		if (Window->RealPitchEdit == NIL)
			{
			 FailurePoint55:
				DisposeRadioButton(Window->DurationAdjustDefaultButton);
				goto FailurePoint54;
			}
		StringTemp = NumericPitchToString(GetNotePitch(Note),GetNoteFlatOrSharpStatus(Note));
		if (StringTemp == NIL)
			{
			 FailurePoint56:
				DisposeTextEdit(Window->RealPitchEdit);
				goto FailurePoint55;
			}
		TextEditNewRawData(Window->RealPitchEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->RealPitchEdit);

		Window->FakePitchEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,FAKEPITCHEDITX,FAKEPITCHEDITY,FAKEPITCHEDITWIDTH,
			FAKEPITCHEDITHEIGHT);
		if (Window->FakePitchEdit == NIL)
			{
			 FailurePoint57:
				goto FailurePoint56;
			}
		if (GetNoteMultisampleFalsePitch(Note) == -1)
			{
				/* default */
				StringTemp = StringToBlockCopy("default");
			}
		 else
			{
				StringTemp = NumericPitchToString(GetNoteMultisampleFalsePitch(Note),0);
			}
		if (StringTemp == NIL)
			{
			 FailurePoint58:
				DisposeTextEdit(Window->FakePitchEdit);
				goto FailurePoint57;
			}
		TextEditNewRawData(Window->FakePitchEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->FakePitchEdit);

		Window->PortamentoEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,PORTAMENTOEDITX,PORTAMENTOEDITY,PORTAMENTOEDITWIDTH,
			PORTAMENTOEDITHEIGHT);
		if (Window->PortamentoEdit == NIL)
			{
			 FailurePoint59:
				goto FailurePoint58;
			}
		StringTemp = LongDoubleToString(GetNotePortamentoDuration(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint60:
				DisposeTextEdit(Window->PortamentoEdit);
				goto FailurePoint59;
			}
		TextEditNewRawData(Window->PortamentoEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->PortamentoEdit);

		Window->RestBox = NewCheckBox(Window->ScreenID,"Note Is Actually a Rest",
			RESTCHECKBOXX,RESTCHECKBOXY,RESTCHECKBOXWIDTH,RESTCHECKBOXHEIGHT);
		if (Window->RestBox == NIL)
			{
			 FailurePoint61:
				goto FailurePoint60;
			}
		if (GetNoteIsItARest(Note))
			{
				SetCheckBoxState(Window->RestBox,True);
			}

		Window->PortaHertzButton = NewRadioButton(Window->ScreenID,"Hertz",
			PORTAHERTZBUTTONX,PORTAHERTZBUTTONY,PORTAHERTZBUTTONWIDTH,PORTAHERTZBUTTONHEIGHT);
		if (Window->PortaHertzButton == NIL)
			{
			 FailurePoint62:
				DisposeCheckBox(Window->RestBox);
				goto FailurePoint61;
			}

		Window->PortaHalfstepButton = NewRadioButton(Window->ScreenID,"Halfsteps",
			PORTAHALFSTEPBUTTONX,PORTAHALFSTEPBUTTONY,PORTAHALFSTEPBUTTONWIDTH,
			PORTAHALFSTEPBUTTONHEIGHT);
		if (Window->PortaHalfstepButton == NIL)
			{
			 FailurePoint63:
				DisposeRadioButton(Window->PortaHertzButton);
				goto FailurePoint62;
			}

		if (GetNotePortamentoHertzNotHalfstepsFlag(Note))
			{
				SetRadioButtonState(Window->PortaHertzButton,True);
			}
		 else
			{
				SetRadioButtonState(Window->PortaHalfstepButton,True);
			}

		Window->RetrigBox = NewCheckBox(Window->ScreenID,"Retrigger Envelopes on Tie",
			RETRIGCHECKBOXX,RETRIGCHECKBOXY,RETRIGCHECKBOXWIDTH,RETRIGCHECKBOXHEIGHT);
		if (Window->RetrigBox == NIL)
			{
			 FailurePoint64:
				DisposeRadioButton(Window->PortaHalfstepButton);
				goto FailurePoint63;
			}
		if (GetNoteRetriggerEnvelopesOnTieStatus(Note))
			{
				SetCheckBoxState(Window->RetrigBox,True);
			}

		Window->Release3Box = NewCheckBox(Window->ScreenID,
			"Release 3 From Start Instead Of End",RELEASE3CHECKBOXX,RELEASE3CHECKBOXY,
			RELEASE3CHECKBOXWIDTH,RELEASE3CHECKBOXHEIGHT);
		if (Window->Release3Box == NIL)
			{
			 FailurePoint65:
				DisposeCheckBox(Window->RetrigBox);
				goto FailurePoint64;
			}
		if (GetNoteRelease3FromStartInsteadOfEnd(Note))
			{
				SetCheckBoxState(Window->Release3Box,True);
			}

		Window->SurroundPosEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,SURROUNDPOSEDITX,SURROUNDPOSEDITY,SURROUNDPOSEDITWIDTH,
			SURROUNDPOSEDITHEIGHT);
		if (Window->SurroundPosEdit == NIL)
			{
			 FailurePoint66:
				DisposeCheckBox(Window->Release3Box);
				goto FailurePoint65;
			}
		StringTemp = LongDoubleToString(GetNoteSurroundPositioning(Note),7,1e-4,1e6);
		if (StringTemp == NIL)
			{
			 FailurePoint67:
				DisposeTextEdit(Window->SurroundPosEdit);
				goto FailurePoint66;
			}
		TextEditNewRawData(Window->SurroundPosEdit,StringTemp,SYSTEMLINEFEED);
		ReleasePtr(StringTemp);
		TextEditHasBeenSaved(Window->SurroundPosEdit);


		ActiveTextEdit = Window->EarlyLateEdit;
		EnableTextEditSelection(ActiveTextEdit);
		TextEditDoMenuSelectAll(ActiveTextEdit);
		LoopFlag = True;
		while (LoopFlag)
			{
				OrdType							X;
				OrdType							Y;
				ModifierFlags				Modifiers;
				MenuItemType*				MenuItem;
				char								KeyPress;

				switch (GetAnEvent(&X,&Y,&Modifiers,NIL,&MenuItem,&KeyPress))
					{
						default:
							break;
						case eCheckCursor:
							if (TextEditIBeamTest(Window->EarlyLateEdit,X,Y)
								|| TextEditIBeamTest(Window->Release1Edit,X,Y)
								|| TextEditIBeamTest(Window->LoudnessEdit,X,Y)
								|| TextEditIBeamTest(Window->Release2Edit,X,Y)
								|| TextEditIBeamTest(Window->Accent1Edit,X,Y)
								|| TextEditIBeamTest(Window->Accent2Edit,X,Y)
								|| TextEditIBeamTest(Window->Accent3Edit,X,Y)
								|| TextEditIBeamTest(Window->Accent4Edit,X,Y)
								|| TextEditIBeamTest(Window->PitchDispRateEdit,X,Y)
								|| TextEditIBeamTest(Window->PitchDispDepthEdit,X,Y)
								|| TextEditIBeamTest(Window->HurryUpEdit,X,Y)
								|| TextEditIBeamTest(Window->PitchDispStartEdit,X,Y)
								|| TextEditIBeamTest(Window->StereoPosEdit,X,Y)
								|| TextEditIBeamTest(Window->SurroundPosEdit,X,Y)
								|| TextEditIBeamTest(Window->DetuneEdit,X,Y)
								|| TextEditIBeamTest(Window->DurationEdit,X,Y)
								|| TextEditIBeamTest(Window->DurationAdjustEdit,X,Y)
								|| TextEditIBeamTest(Window->RealPitchEdit,X,Y)
								|| TextEditIBeamTest(Window->FakePitchEdit,X,Y)
								|| TextEditIBeamTest(Window->PortamentoEdit,X,Y))
								{
									SetIBeamCursor();
								}
							 else
								{
									SetArrowCursor();
								}
							goto UpdateCursorPoint;
							break;
						case eNoEvent:
						 UpdateCursorPoint:
							TextEditUpdateCursor(ActiveTextEdit);
							break;
						case eMenuStarting:
							EnableMenuItem(mPaste);
							if (TextEditIsThereValidSelection(ActiveTextEdit))
								{
									EnableMenuItem(mCut);
									EnableMenuItem(mCopy);
									EnableMenuItem(mClear);
								}
							EnableMenuItem(mSelectAll);
							if (TextEditCanWeUndo(ActiveTextEdit))
								{
									EnableMenuItem(mUndo);
								}
							break;
						case eMenuCommand:
							if (MenuItem == mPaste)
								{
									TextEditDoMenuPaste(ActiveTextEdit);
								}
							else if (MenuItem == mCut)
								{
									TextEditDoMenuCut(ActiveTextEdit);
								}
							else if (MenuItem == mCopy)
								{
									TextEditDoMenuCopy(ActiveTextEdit);
								}
							else if (MenuItem == mClear)
								{
									TextEditDoMenuClear(ActiveTextEdit);
								}
							else if (MenuItem == mUndo)
								{
									TextEditDoMenuUndo(ActiveTextEdit);
									TextEditShowSelection(ActiveTextEdit);
								}
							else if (MenuItem == mSelectAll)
								{
									TextEditDoMenuSelectAll(ActiveTextEdit);
								}
							else
								{
									EXECUTE(PRERR(AllowResume,
										"EditNoteParametersDialog: Undefined menu option chosen"));
								}
							break;
						case eKeyPressed:
							if (KeyPress == 13)
								{
									FlashButton(Window->OKButton);
									DoItFlag = True;
									LoopFlag = False;
								}
							else if (KeyPress == 9)
								{
									DisableTextEditSelection(ActiveTextEdit);
									if ((eShiftKey & Modifiers) == 0)
										{
											/* tab forwards */
											if (ActiveTextEdit == Window->EarlyLateEdit)
												{
													ActiveTextEdit = Window->Release1Edit;
												}
											else if (ActiveTextEdit == Window->Release1Edit)
												{
													ActiveTextEdit = Window->LoudnessEdit;
												}
											else if (ActiveTextEdit == Window->LoudnessEdit)
												{
													ActiveTextEdit = Window->Release2Edit;
												}
											else if (ActiveTextEdit == Window->Release2Edit)
												{
													ActiveTextEdit = Window->Accent1Edit;
												}
											else if (ActiveTextEdit == Window->Accent1Edit)
												{
													ActiveTextEdit = Window->Accent2Edit;
												}
											else if (ActiveTextEdit == Window->Accent2Edit)
												{
													ActiveTextEdit = Window->Accent3Edit;
												}
											else if (ActiveTextEdit == Window->Accent3Edit)
												{
													ActiveTextEdit = Window->Accent4Edit;
												}
											else if (ActiveTextEdit == Window->Accent4Edit)
												{
													ActiveTextEdit = Window->PitchDispRateEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispRateEdit)
												{
													ActiveTextEdit = Window->PitchDispDepthEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispDepthEdit)
												{
													ActiveTextEdit = Window->HurryUpEdit;
												}
											else if (ActiveTextEdit == Window->HurryUpEdit)
												{
													ActiveTextEdit = Window->PitchDispStartEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispStartEdit)
												{
													ActiveTextEdit = Window->StereoPosEdit;
												}
											else if (ActiveTextEdit == Window->StereoPosEdit)
												{
													ActiveTextEdit = Window->DetuneEdit;
												}
											else if (ActiveTextEdit == Window->DetuneEdit)
												{
													ActiveTextEdit = Window->SurroundPosEdit;
												}
											else if (ActiveTextEdit == Window->SurroundPosEdit)
												{
													ActiveTextEdit = Window->DurationEdit;
												}
											else if (ActiveTextEdit == Window->DurationEdit)
												{
													ActiveTextEdit = Window->DurationAdjustEdit;
												}
											else if (ActiveTextEdit == Window->DurationAdjustEdit)
												{
													ActiveTextEdit = Window->RealPitchEdit;
												}
											else if (ActiveTextEdit == Window->RealPitchEdit)
												{
													ActiveTextEdit = Window->FakePitchEdit;
												}
											else if (ActiveTextEdit == Window->FakePitchEdit)
												{
													ActiveTextEdit = Window->PortamentoEdit;
												}
											else
												{
													ActiveTextEdit = Window->EarlyLateEdit;
												}
										}
									 else
										{
											/* tab backwards */
											if (ActiveTextEdit == Window->PortamentoEdit)
												{
													ActiveTextEdit = Window->FakePitchEdit;
												}
											else if (ActiveTextEdit == Window->FakePitchEdit)
												{
													ActiveTextEdit = Window->RealPitchEdit;
												}
											else if (ActiveTextEdit == Window->RealPitchEdit)
												{
													ActiveTextEdit = Window->DurationAdjustEdit;
												}
											else if (ActiveTextEdit == Window->DurationAdjustEdit)
												{
													ActiveTextEdit = Window->DurationEdit;
												}
											else if (ActiveTextEdit == Window->DurationEdit)
												{
													ActiveTextEdit = Window->SurroundPosEdit;
												}
											else if (ActiveTextEdit == Window->SurroundPosEdit)
												{
													ActiveTextEdit = Window->DetuneEdit;
												}
											else if (ActiveTextEdit == Window->DetuneEdit)
												{
													ActiveTextEdit = Window->StereoPosEdit;
												}
											else if (ActiveTextEdit == Window->StereoPosEdit)
												{
													ActiveTextEdit = Window->PitchDispStartEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispStartEdit)
												{
													ActiveTextEdit = Window->HurryUpEdit;
												}
											else if (ActiveTextEdit == Window->HurryUpEdit)
												{
													ActiveTextEdit = Window->PitchDispDepthEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispDepthEdit)
												{
													ActiveTextEdit = Window->PitchDispRateEdit;
												}
											else if (ActiveTextEdit == Window->PitchDispRateEdit)
												{
													ActiveTextEdit = Window->Accent4Edit;
												}
											else if (ActiveTextEdit == Window->Accent4Edit)
												{
													ActiveTextEdit = Window->Accent3Edit;
												}
											else if (ActiveTextEdit == Window->Accent3Edit)
												{
													ActiveTextEdit = Window->Accent2Edit;
												}
											else if (ActiveTextEdit == Window->Accent2Edit)
												{
													ActiveTextEdit = Window->Accent1Edit;
												}
											else if (ActiveTextEdit == Window->Accent1Edit)
												{
													ActiveTextEdit = Window->Release2Edit;
												}
											else if (ActiveTextEdit == Window->Release2Edit)
												{
													ActiveTextEdit = Window->LoudnessEdit;
												}
											else if (ActiveTextEdit == Window->LoudnessEdit)
												{
													ActiveTextEdit = Window->Release1Edit;
												}
											else if (ActiveTextEdit == Window->Release1Edit)
												{
													ActiveTextEdit = Window->EarlyLateEdit;
												}
											else
												{
													ActiveTextEdit = Window->PortamentoEdit;
												}
										}
									TextEditDoMenuSelectAll(ActiveTextEdit);
									EnableTextEditSelection(ActiveTextEdit);
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									DoItFlag = False;
									LoopFlag = False;
								}
							else
								{
									TextEditDoKeyPressed(ActiveTextEdit,KeyPress,Modifiers);
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
							else if (TextEditHitTest(Window->EarlyLateEdit,X,Y))
								{
									if (ActiveTextEdit != Window->EarlyLateEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->EarlyLateEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->Release1Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Release1Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Release1Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->Release1StartButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release1StartButton,X,Y))
										{
											SetRadioButtonState(Window->Release1EndButton,False);
											SetRadioButtonState(Window->Release1DefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->Release1EndButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release1EndButton,X,Y))
										{
											SetRadioButtonState(Window->Release1StartButton,False);
											SetRadioButtonState(Window->Release1DefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->Release1DefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release1DefaultButton,X,Y))
										{
											SetRadioButtonState(Window->Release1StartButton,False);
											SetRadioButtonState(Window->Release1EndButton,False);
										}
								}
							else if (TextEditHitTest(Window->LoudnessEdit,X,Y))
								{
									if (ActiveTextEdit != Window->LoudnessEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->LoudnessEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->Release2Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Release2Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Release2Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->Release2StartButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release2StartButton,X,Y))
										{
											SetRadioButtonState(Window->Release2EndButton,False);
											SetRadioButtonState(Window->Release2DefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->Release2EndButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release2EndButton,X,Y))
										{
											SetRadioButtonState(Window->Release2StartButton,False);
											SetRadioButtonState(Window->Release2DefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->Release2DefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->Release2DefaultButton,X,Y))
										{
											SetRadioButtonState(Window->Release2StartButton,False);
											SetRadioButtonState(Window->Release2EndButton,False);
										}
								}
							else if (TextEditHitTest(Window->Accent1Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Accent1Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Accent1Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->Accent2Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Accent2Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Accent2Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->Accent3Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Accent3Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Accent3Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->Accent4Edit,X,Y))
								{
									if (ActiveTextEdit != Window->Accent4Edit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->Accent4Edit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->PitchDispRateEdit,X,Y))
								{
									if (ActiveTextEdit != Window->PitchDispRateEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->PitchDispRateEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->PitchDispDepthEdit,X,Y))
								{
									if (ActiveTextEdit != Window->PitchDispDepthEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->PitchDispDepthEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->PitchDispDepthHertzButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispDepthHertzButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispDepthHalfStepButton,False);
											SetRadioButtonState(Window->PitchDispDepthDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->PitchDispDepthHalfStepButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispDepthHalfStepButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispDepthHertzButton,False);
											SetRadioButtonState(Window->PitchDispDepthDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->PitchDispDepthDefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispDepthDefaultButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispDepthHalfStepButton,False);
											SetRadioButtonState(Window->PitchDispDepthHertzButton,False);
										}
								}
							else if (TextEditHitTest(Window->HurryUpEdit,X,Y))
								{
									if (ActiveTextEdit != Window->HurryUpEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->HurryUpEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->PitchDispStartEdit,X,Y))
								{
									if (ActiveTextEdit != Window->PitchDispStartEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->PitchDispStartEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->PitchDispStartStartButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispStartStartButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispStartEndButton,False);
											SetRadioButtonState(Window->PitchDispStartDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->PitchDispStartEndButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispStartEndButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispStartStartButton,False);
											SetRadioButtonState(Window->PitchDispStartDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->PitchDispStartDefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PitchDispStartDefaultButton,X,Y))
										{
											SetRadioButtonState(Window->PitchDispStartStartButton,False);
											SetRadioButtonState(Window->PitchDispStartEndButton,False);
										}
								}
							else if (TextEditHitTest(Window->StereoPosEdit,X,Y))
								{
									if (ActiveTextEdit != Window->StereoPosEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->StereoPosEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->SurroundPosEdit,X,Y))
								{
									if (ActiveTextEdit != Window->SurroundPosEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->SurroundPosEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->DetuneEdit,X,Y))
								{
									if (ActiveTextEdit != Window->DetuneEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->DetuneEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->DetuneHertzButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DetuneHertzButton,X,Y))
										{
											SetRadioButtonState(Window->DetuneHalfstepButton,False);
											SetRadioButtonState(Window->DetuneDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->DetuneHalfstepButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DetuneHalfstepButton,X,Y))
										{
											SetRadioButtonState(Window->DetuneHertzButton,False);
											SetRadioButtonState(Window->DetuneDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->DetuneDefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DetuneDefaultButton,X,Y))
										{
											SetRadioButtonState(Window->DetuneHertzButton,False);
											SetRadioButtonState(Window->DetuneHalfstepButton,False);
										}
								}
							else if (TextEditHitTest(Window->DurationEdit,X,Y))
								{
									if (ActiveTextEdit != Window->DurationEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->DurationEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->DurationAdjustEdit,X,Y))
								{
									if (ActiveTextEdit != Window->DurationAdjustEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->DurationAdjustEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->DurationAdjustAddButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DurationAdjustAddButton,X,Y))
										{
											SetRadioButtonState(Window->DurationAdjustMultButton,False);
											SetRadioButtonState(Window->DurationAdjustDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->DurationAdjustMultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DurationAdjustMultButton,X,Y))
										{
											SetRadioButtonState(Window->DurationAdjustAddButton,False);
											SetRadioButtonState(Window->DurationAdjustDefaultButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->DurationAdjustDefaultButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->DurationAdjustDefaultButton,X,Y))
										{
											SetRadioButtonState(Window->DurationAdjustAddButton,False);
											SetRadioButtonState(Window->DurationAdjustMultButton,False);
										}
								}
							else if (TextEditHitTest(Window->RealPitchEdit,X,Y))
								{
									if (ActiveTextEdit != Window->RealPitchEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->RealPitchEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->FakePitchEdit,X,Y))
								{
									if (ActiveTextEdit != Window->FakePitchEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->FakePitchEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->PortamentoEdit,X,Y))
								{
									if (ActiveTextEdit != Window->PortamentoEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->PortamentoEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (CheckBoxHitTest(Window->RestBox,X,Y))
								{
									CheckBoxMouseDown(Window->RestBox,X,Y);
								}
							else if (RadioButtonHitTest(Window->PortaHertzButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PortaHertzButton,X,Y))
										{
											SetRadioButtonState(Window->PortaHalfstepButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->PortaHalfstepButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->PortaHalfstepButton,X,Y))
										{
											SetRadioButtonState(Window->PortaHertzButton,False);
										}
								}
							else if (CheckBoxHitTest(Window->RetrigBox,X,Y))
								{
									CheckBoxMouseDown(Window->RetrigBox,X,Y);
								}
							else if (CheckBoxHitTest(Window->Release3Box,X,Y))
								{
									CheckBoxMouseDown(Window->Release3Box,X,Y);
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"EditNoteParametersDialog:  DoItFlag is neither true nor false"));

		if (DoItFlag)
			{
				MyBoolean					ErrorNoMemory;

				/* this is a bit of overkill.  we should only mark the object as altered */
				/* if something was actually changed.  and there's no reason to recompute */
				/* the entire schedule. */
				TrackObjectAltered(Track,0);

				ErrorNoMemory = False;

				if (TextEditDoesItNeedToBeSaved(Window->EarlyLateEdit))
					{
						StringTemp = TextEditGetRawData(Window->EarlyLateEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteEarlyLateAdjust(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->Release1Edit))
					{
						StringTemp = TextEditGetRawData(Window->Release1Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteReleasePoint1(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->Release1StartButton))
					{
						PutNoteRelease1Origin(Note,eRelease1FromStart);
					}
				else if (GetRadioButtonState(Window->Release1EndButton))
					{
						PutNoteRelease1Origin(Note,eRelease1FromEnd);
					}
				else if (GetRadioButtonState(Window->Release1DefaultButton))
					{
						PutNoteRelease1Origin(Note,eRelease1FromDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  release point 1 "
							"radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->LoudnessEdit))
					{
						StringTemp = TextEditGetRawData(Window->LoudnessEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteOverallLoudnessAdjustment(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->Release2Edit))
					{
						StringTemp = TextEditGetRawData(Window->Release2Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteReleasePoint2(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->Release2StartButton))
					{
						PutNoteRelease2Origin(Note,eRelease2FromStart);
					}
				else if (GetRadioButtonState(Window->Release2EndButton))
					{
						PutNoteRelease2Origin(Note,eRelease2FromEnd);
					}
				else if (GetRadioButtonState(Window->Release2DefaultButton))
					{
						PutNoteRelease2Origin(Note,eRelease2FromDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  release point 2 "
							"radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->Accent1Edit))
					{
						StringTemp = TextEditGetRawData(Window->Accent1Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteAccent1(Note,StringToLongDouble(StringTemp,PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->Accent2Edit))
					{
						StringTemp = TextEditGetRawData(Window->Accent2Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteAccent2(Note,StringToLongDouble(StringTemp,PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->Accent3Edit))
					{
						StringTemp = TextEditGetRawData(Window->Accent3Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteAccent3(Note,StringToLongDouble(StringTemp,PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->Accent4Edit))
					{
						StringTemp = TextEditGetRawData(Window->Accent4Edit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteAccent4(Note,StringToLongDouble(StringTemp,PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->PitchDispRateEdit))
					{
						StringTemp = TextEditGetRawData(Window->PitchDispRateEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNotePitchDisplacementRateAdjust(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->PitchDispDepthEdit))
					{
						StringTemp = TextEditGetRawData(Window->PitchDispDepthEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNotePitchDisplacementDepthAdjust(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->PitchDispDepthHertzButton))
					{
						PutNotePitchDisplacementDepthConversionMode(Note,
							ePitchDisplacementDepthModeHertz);
					}
				else if (GetRadioButtonState(Window->PitchDispDepthHalfStepButton))
					{
						PutNotePitchDisplacementDepthConversionMode(Note,
							ePitchDisplacementDepthModeHalfSteps);
					}
				else if (GetRadioButtonState(Window->PitchDispDepthDefaultButton))
					{
						PutNotePitchDisplacementDepthConversionMode(Note,
							ePitchDisplacementDepthModeDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  pitch disp depth "
							"radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->HurryUpEdit))
					{
						StringTemp = TextEditGetRawData(Window->HurryUpEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteHurryUpFactor(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->PitchDispStartEdit))
					{
						StringTemp = TextEditGetRawData(Window->PitchDispStartEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNotePitchDisplacementStartPoint(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->PitchDispStartStartButton))
					{
						PutNotePitchDisplacementStartOrigin(Note,ePitchDisplacementStartFromStart);
					}
				else if (GetRadioButtonState(Window->PitchDispStartEndButton))
					{
						PutNotePitchDisplacementStartOrigin(Note,ePitchDisplacementStartFromEnd);
					}
				else if (GetRadioButtonState(Window->PitchDispStartDefaultButton))
					{
						PutNotePitchDisplacementStartOrigin(Note,ePitchDisplacementStartFromDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  pitch disp origin "
							"radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->StereoPosEdit))
					{
						StringTemp = TextEditGetRawData(Window->StereoPosEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteStereoPositioning(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->SurroundPosEdit))
					{
						StringTemp = TextEditGetRawData(Window->SurroundPosEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteSurroundPositioning(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->DetuneEdit))
					{
						StringTemp = TextEditGetRawData(Window->DetuneEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteDetuning(Note,StringToLongDouble(StringTemp,PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->DetuneHertzButton))
					{
						PutNoteDetuneConversionMode(Note,eDetuningModeHertz);
					}
				else if (GetRadioButtonState(Window->DetuneHalfstepButton))
					{
						PutNoteDetuneConversionMode(Note,eDetuningModeHalfSteps);
					}
				else if (GetRadioButtonState(Window->DetuneDefaultButton))
					{
						PutNoteDetuneConversionMode(Note,eDetuningModeDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  detune conversion "
							"mode radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->DurationEdit))
					{
						StringTemp = TextEditGetRawData(Window->DurationEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								unsigned long				Duration;
								MyBoolean						DotFlag;
								unsigned long				Division;

								Duration = GetNoteDuration(Note);
								DotFlag = GetNoteDotStatus(Note);
								Division = GetNoteDurationDivision(Note);
								StringToNumericDuration(StringTemp,&Duration,&DotFlag,&Division);
								PutNoteDuration(Note,Duration);
								PutNoteDotStatus(Note,DotFlag);
								PutNoteDurationDivision(Note,Division);
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->DurationAdjustEdit))
					{
						StringTemp = TextEditGetRawData(Window->DurationAdjustEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNoteDurationAdjust(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				if (GetRadioButtonState(Window->DurationAdjustAddButton))
					{
						PutNoteDurationAdjustMode(Note,eDurationAdjustAdditive);
					}
				else if (GetRadioButtonState(Window->DurationAdjustMultButton))
					{
						PutNoteDurationAdjustMode(Note,eDurationAdjustMultiplicative);
					}
				else if (GetRadioButtonState(Window->DurationAdjustDefaultButton))
					{
						PutNoteDurationAdjustMode(Note,eDurationAdjustDefault);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  duration adjust mode "
							"radio buttons are in an inconsistent state"));
					}

				if (TextEditDoesItNeedToBeSaved(Window->RealPitchEdit))
					{
						StringTemp = TextEditGetRawData(Window->RealPitchEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								short						PitchValue;
								unsigned long		SharpFlatThing;

								PitchValue = GetNotePitch(Note);
								SharpFlatThing = GetNoteFlatOrSharpStatus(Note);
								StringToNumericPitch(StringTemp,&PitchValue,&SharpFlatThing);
								PutNotePitch(Note,PitchValue);
								PutNoteFlatOrSharpStatus(Note,SharpFlatThing);
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->FakePitchEdit))
					{
						StringTemp = TextEditGetRawData(Window->FakePitchEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								if (IsItInThere(StringTemp,"def"/*ault*/))
									{
										PutNoteMultisampleFalsePitch(Note,-1);
									}
								 else
									{
										short						PitchValue;
										unsigned long		SharpFlatThing;

										PitchValue = GetNotePitch(Note);
										SharpFlatThing = 0;
										StringToNumericPitch(StringTemp,&PitchValue,&SharpFlatThing);
										PutNoteMultisampleFalsePitch(Note,PitchValue);
									}
								ReleasePtr(StringTemp);
							}
					}

				if (TextEditDoesItNeedToBeSaved(Window->PortamentoEdit))
					{
						StringTemp = TextEditGetRawData(Window->PortamentoEdit,SYSTEMLINEFEED);
						if (StringTemp == NIL)
							{
								ErrorNoMemory = True;
							}
						 else
							{
								PutNotePortamentoDuration(Note,StringToLongDouble(StringTemp,
									PtrSize(StringTemp)));
								ReleasePtr(StringTemp);
							}
					}

				PutNoteIsItARest(Note,GetCheckBoxState(Window->RestBox));

				if (GetRadioButtonState(Window->PortaHertzButton))
					{
						PutNotePortamentoHertzNotHalfstepsFlag(Note,True);
					}
				else if (GetRadioButtonState(Window->PortaHalfstepButton))
					{
						PutNotePortamentoHertzNotHalfstepsFlag(Note,False);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"EditNoteParametersDialog:  portamento mode "
							"radio buttons are in an inconsistent state"));
					}

				PutNoteRetriggerEnvelopesOnTieStatus(Note,GetCheckBoxState(Window->RetrigBox));

				PutNoteRelease3FromStartInsteadOfEnd(Note,GetCheckBoxState(Window->Release3Box));

				if (ErrorNoMemory)
					{
						AlertHalt("There was not enough memory to save all of the attributes.",NIL);
					}
			}

		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		DisposeTextEdit(Window->EarlyLateEdit);
		DisposeTextEdit(Window->Release1Edit);
		DisposeRadioButton(Window->Release1StartButton);
		DisposeRadioButton(Window->Release1EndButton);
		DisposeRadioButton(Window->Release1DefaultButton);
		DisposeTextEdit(Window->LoudnessEdit);
		DisposeTextEdit(Window->Release2Edit);
		DisposeRadioButton(Window->Release2StartButton);
		DisposeRadioButton(Window->Release2EndButton);
		DisposeRadioButton(Window->Release2DefaultButton);
		DisposeTextEdit(Window->Accent1Edit);
		DisposeTextEdit(Window->Accent2Edit);
		DisposeTextEdit(Window->Accent3Edit);
		DisposeTextEdit(Window->Accent4Edit);
		DisposeTextEdit(Window->PitchDispRateEdit);
		DisposeTextEdit(Window->PitchDispDepthEdit);
		DisposeRadioButton(Window->PitchDispDepthHertzButton);
		DisposeRadioButton(Window->PitchDispDepthHalfStepButton);
		DisposeRadioButton(Window->PitchDispDepthDefaultButton);
		DisposeTextEdit(Window->HurryUpEdit);
		DisposeTextEdit(Window->PitchDispStartEdit);
		DisposeRadioButton(Window->PitchDispStartStartButton);
		DisposeRadioButton(Window->PitchDispStartEndButton);
		DisposeRadioButton(Window->PitchDispStartDefaultButton);
		DisposeTextEdit(Window->StereoPosEdit);
		DisposeTextEdit(Window->DetuneEdit);
		DisposeRadioButton(Window->DetuneHertzButton);
		DisposeRadioButton(Window->DetuneHalfstepButton);
		DisposeRadioButton(Window->DetuneDefaultButton);
		DisposeTextEdit(Window->SurroundPosEdit);
		DisposeTextEdit(Window->DurationEdit);
		DisposeTextEdit(Window->DurationAdjustEdit);
		DisposeRadioButton(Window->DurationAdjustAddButton);
		DisposeRadioButton(Window->DurationAdjustMultButton);
		DisposeRadioButton(Window->DurationAdjustDefaultButton);
		DisposeTextEdit(Window->RealPitchEdit);
		DisposeTextEdit(Window->FakePitchEdit);
		DisposeTextEdit(Window->PortamentoEdit);
		DisposeCheckBox(Window->RestBox);
		DisposeRadioButton(Window->PortaHertzButton);
		DisposeRadioButton(Window->PortaHalfstepButton);
		DisposeCheckBox(Window->RetrigBox);
		DisposeCheckBox(Window->Release3Box);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
		/* no return value */
	}
