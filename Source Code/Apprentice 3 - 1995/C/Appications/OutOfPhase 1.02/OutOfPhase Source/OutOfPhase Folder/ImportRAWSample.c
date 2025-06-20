/* ImportRAWSample.c */
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

#include "ImportRAWSample.h"
#include "MainWindowStuff.h"
#include "Files.h"
#include "Alert.h"
#include "Memory.h"
#include "Screen.h"
#include "TextEdit.h"
#include "RadioButton.h"
#include "SimpleButton.h"
#include "Numbers.h"
#include "Main.h"
#include "Menus.h"
#include "EventLoop.h"
#include "SampleConsts.h"
#include "BufferedFileInput.h"
#include "SampleObject.h"


#define DEFAULTSAMPLINGRATE (22050)


typedef enum
	{
		eSignSigned EXECUTE(= -8542),
		eSignUnsigned,
		eSignSignBit
	} SignModes;


typedef enum
	{
		eEndianLittle EXECUTE(= -22512),
		eEndianBig
	} EndianType;


/* prototype for the function that does the work. */
static void						DoImportDialog(FileType* FileDesc, MainWindowRec* MainWindow,
												FileSpec* WhereIsTheFile);


/* this routine gets a file and imports it according to some generic parameters */
/* that the user can specify.  any errors are handled by the routine. */
void									ImportRAWSample(struct MainWindowRec* MainWindow)
	{
		FileSpec*						WhereIsTheFile;

		CheckPtrExistence(MainWindow);
		WhereIsTheFile = GetFileAny();
		if (WhereIsTheFile != NIL)
			{
				FileType*						TheFileDescriptor;

				if (OpenFile(WhereIsTheFile,&TheFileDescriptor,eReadOnly))
					{
						DoImportDialog(TheFileDescriptor,MainWindow,WhereIsTheFile);
						CloseFile(TheFileDescriptor);
					}
				 else
					{
						AlertHalt("Unable to open the file.",NIL);
					}
				DisposeFileSpec(WhereIsTheFile);
			}
	}


#define WINDOWXSIZE (450)

#define PROMPTX (10)
#define PROMPTY (5)

#define BYTESTOSKIPPROMPTX (PROMPTX)
#define BYTESTOSKIPPROMPTY (PROMPTY + 20 + 6)

#define BYTESTOSKIPEDITX (BYTESTOSKIPPROMPTX + 70)
#define BYTESTOSKIPEDITY (BYTESTOSKIPPROMPTY - 3)
#define BYTESTOSKIPEDITWIDTH (100)
#define BYTESTOSKIPEDITHEIGHT (21)

#define SKIPBETWEENPROMPTX (BYTESTOSKIPEDITX + BYTESTOSKIPEDITWIDTH + 10)
#define SKIPBETWEENPROMPTY (BYTESTOSKIPPROMPTY)

#define SKIPBETWEENEDITX (SKIPBETWEENPROMPTX + 120)
#define SKIPBETWEENEDITY (BYTESTOSKIPEDITY)
#define SKIPBETWEENEDITWIDTH (BYTESTOSKIPEDITWIDTH)
#define SKIPBETWEENEDITHEIGHT (BYTESTOSKIPEDITHEIGHT)

#define BITSPROMPTX (PROMPTX)
#define BITSPROMPTY (BYTESTOSKIPPROMPTY + BYTESTOSKIPEDITHEIGHT + 6)

#define BITS8BUTTONX (BITSPROMPTX + 100)
#define BITS8BUTTONY (BITSPROMPTY - 4)
#define BITS8BUTTONWIDTH (80)
#define BITS8BUTTONHEIGHT (21)

#define BITS16BUTTONX (BITS8BUTTONX + BITS8BUTTONWIDTH + 10)
#define BITS16BUTTONY (BITS8BUTTONY)
#define BITS16BUTTONWIDTH (BITS8BUTTONWIDTH)
#define BITS16BUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define SIGNPROMPTX (BITSPROMPTX)
#define SIGNPROMPTY (BITSPROMPTY + BITS8BUTTONHEIGHT + 3)

#define SIGNEDBUTTONX (BITS8BUTTONX)
#define SIGNEDBUTTONY (SIGNPROMPTY - 4)
#define SIGNEDBUTTONWIDTH (BITS8BUTTONWIDTH)
#define SIGNEDBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define UNSIGNEDBUTTONX (SIGNEDBUTTONX + SIGNEDBUTTONWIDTH + 10)
#define UNSIGNEDBUTTONY (SIGNEDBUTTONY)
#define UNSIGNEDBUTTONWIDTH (BITS8BUTTONWIDTH)
#define UNSIGNEDBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define SIGNBITBUTTONX (UNSIGNEDBUTTONX + UNSIGNEDBUTTONWIDTH + 10)
#define SIGNBITBUTTONY (UNSIGNEDBUTTONY)
#define SIGNBITBUTTONWIDTH (BITS8BUTTONWIDTH)
#define SIGNBITBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define ENDIANPROMPTX (SIGNPROMPTX)
#define ENDIANPROMPTY (SIGNPROMPTY + BITS8BUTTONHEIGHT + 3)

#define LITTLEENDIANBUTTONX (BITS8BUTTONX)
#define LITTLEENDIANBUTTONY (ENDIANPROMPTY - 4)
#define LITTLEENDIANBUTTONWIDTH (BITS8BUTTONWIDTH)
#define LITTLEENDIANBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define BIGENDIANBUTTONX (LITTLEENDIANBUTTONX + LITTLEENDIANBUTTONWIDTH + 10)
#define BIGENDIANBUTTONY (LITTLEENDIANBUTTONY)
#define BIGENDIANBUTTONWIDTH (BITS8BUTTONWIDTH)
#define BIGENDIANBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define NUMCHANNELSPROMPTX (ENDIANPROMPTX)
#define NUMCHANNELSPROMPTY (ENDIANPROMPTY + BITS8BUTTONHEIGHT + 3)

#define NUMCHANNELSMONOBUTTONX (BITS8BUTTONX)
#define NUMCHANNELSMONOBUTTONY (NUMCHANNELSPROMPTY - 4)
#define NUMCHANNELSMONOBUTTONWIDTH (BITS8BUTTONWIDTH)
#define NUMCHANNELSMONOBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define NUMCHANNELSSTEREOBUTTONX (NUMCHANNELSMONOBUTTONX + NUMCHANNELSMONOBUTTONWIDTH + 10)
#define NUMCHANNELSSTEREOBUTTONY (NUMCHANNELSMONOBUTTONY)
#define NUMCHANNELSSTEREOBUTTONWIDTH (BITS8BUTTONWIDTH)
#define NUMCHANNELSSTEREOBUTTONHEIGHT (BITS8BUTTONHEIGHT)

#define FILELENGTHPROMPTX (NUMCHANNELSPROMPTX)
#define FILELENGTHPROMPTY (NUMCHANNELSPROMPTY + BITS8BUTTONHEIGHT + 3)

#define FILELENGTHPLACEX (FILELENGTHPROMPTX + 80)
#define FILELENGTHPLACEY (FILELENGTHPROMPTY)

#define OKBUTTONWIDTH (80)
#define OKBUTTONHEIGHT (21)
#define OKBUTTONX (WINDOWXSIZE / 4 - OKBUTTONWIDTH / 2)
#define OKBUTTONY (FILELENGTHPLACEY + 24)

#define CANCELBUTTONWIDTH (OKBUTTONWIDTH)
#define CANCELBUTTONHEIGHT (OKBUTTONHEIGHT)
#define CANCELBUTTONX (3 * WINDOWXSIZE / 4 - OKBUTTONWIDTH / 2)
#define CANCELBUTTONY (OKBUTTONY)

#define WINDOWYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 10)


typedef struct
	{
		WinType*					ScreenID;
		TextEditRec*			BytesToSkipEdit;
		RadioButtonRec*		Bits8Button;
		RadioButtonRec*		Bits16Button;
		RadioButtonRec*		SignedButton;
		RadioButtonRec*		UnsignedButton;
		RadioButtonRec*		SignBitButton;
		RadioButtonRec*		LittleEndian;
		RadioButtonRec*		BigEndian;
		RadioButtonRec*		MonoButton;
		RadioButtonRec*		StereoButton;
		TextEditRec*			SkipBetweenEdit;
		FileType*					FileDescriptor;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static void						RedrawWindow(WindowRec* Window)
	{
		char*								FileLengthString;

		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->BytesToSkipEdit);
		TextEditFullRedraw(Window->SkipBetweenEdit);
		RedrawRadioButton(Window->Bits8Button);
		RedrawRadioButton(Window->Bits16Button);
		RedrawRadioButton(Window->SignedButton);
		RedrawRadioButton(Window->UnsignedButton);
		RedrawRadioButton(Window->SignBitButton);
		RedrawRadioButton(Window->LittleEndian);
		RedrawRadioButton(Window->BigEndian);
		RedrawRadioButton(Window->MonoButton);
		RedrawRadioButton(Window->StereoButton);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		SetClipRect(Window->ScreenID,0,0,WINDOWXSIZE,WINDOWYSIZE);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Import Raw Sample:",18,
			PROMPTX,PROMPTY,ePlain);
		FileLengthString = IntegerToString(GetFileLength(Window->FileDescriptor));
		if (FileLengthString != NIL)
			{
				DrawTextLine(Window->ScreenID,GetScreenFont(),9,FileLengthString,
					PtrSize(FileLengthString),FILELENGTHPLACEX,FILELENGTHPLACEY,ePlain);
				ReleasePtr(FileLengthString);
			}
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Initial Skip:",13,
			BYTESTOSKIPPROMPTX,BYTESTOSKIPPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Skip Between Frames:",20,
			SKIPBETWEENPROMPTX,SKIPBETWEENPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Number Of Bits:",15,
			BITSPROMPTX,BITSPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Sample Sign:",12,
			SIGNPROMPTX,SIGNPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Endianness:",11,
			ENDIANPROMPTX,ENDIANPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"File Length:",12,
			FILELENGTHPROMPTX,FILELENGTHPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Number Of Channels:",19,
			NUMCHANNELSPROMPTX,NUMCHANNELSPROMPTY,ePlain);
	}


static void						DoImportDialog(FileType* FileDesc, MainWindowRec* MainWindow,
												FileSpec* WhereIsTheFile)
	{
		WindowRec*					Window;
		char*								StringTemp;
		MyBoolean						LoopFlag;
		MyBoolean						DoItFlag EXECUTE(= -31342);
		TextEditRec*				ActiveTextEdit;

		CheckPtrExistence(FileDesc);
		CheckPtrExistence(MainWindow);

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),"DoImportDialog:  WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to import the sample.",NIL);
				return;
			}
		Window->FileDescriptor = FileDesc;

		Window->ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(WINDOWXSIZE),
			DialogTopEdge(WINDOWYSIZE),WINDOWXSIZE,WINDOWYSIZE,
			(void (*)(void*))&RedrawWindow,Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		SetWindowName(Window->ScreenID,"Import Raw Sample");

		Window->BytesToSkipEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,BYTESTOSKIPEDITX,BYTESTOSKIPEDITY,BYTESTOSKIPEDITWIDTH,
			BYTESTOSKIPEDITHEIGHT);
		if (Window->BytesToSkipEdit == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		StringTemp = IntegerToString(0);
		if (StringTemp != NIL)
			{
				TextEditNewRawData(Window->BytesToSkipEdit,StringTemp,SYSTEMLINEFEED);
				ReleasePtr(StringTemp);
			}

		Window->SkipBetweenEdit = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,SKIPBETWEENEDITX,SKIPBETWEENEDITY,SKIPBETWEENEDITWIDTH,
			SKIPBETWEENEDITHEIGHT);
		if (Window->SkipBetweenEdit == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->BytesToSkipEdit);
				goto FailurePoint3;
			}
		StringTemp = IntegerToString(0);
		if (StringTemp != NIL)
			{
				TextEditNewRawData(Window->SkipBetweenEdit,StringTemp,SYSTEMLINEFEED);
				ReleasePtr(StringTemp);
			}

		Window->Bits8Button = NewRadioButton(Window->ScreenID,"8-Bit",
			BITS8BUTTONX,BITS8BUTTONY,BITS8BUTTONWIDTH,BITS8BUTTONHEIGHT);
		if (Window->Bits8Button == NIL)
			{
			 FailurePoint5:
				DisposeTextEdit(Window->SkipBetweenEdit);
				goto FailurePoint4;
			}
		SetRadioButtonState(Window->Bits8Button,True); /* 8-bit by default */

		Window->Bits16Button = NewRadioButton(Window->ScreenID,"16-Bit",
			BITS16BUTTONX,BITS16BUTTONY,BITS16BUTTONWIDTH,BITS16BUTTONHEIGHT);
		if (Window->Bits16Button == NIL)
			{
			 FailurePoint6:
				DisposeRadioButton(Window->Bits8Button);
				goto FailurePoint5;
			}

		Window->SignedButton = NewRadioButton(Window->ScreenID,"Signed",
			SIGNEDBUTTONX,SIGNEDBUTTONY,SIGNEDBUTTONWIDTH,SIGNEDBUTTONHEIGHT);
		if (Window->SignedButton == NIL)
			{
			 FailurePoint7:
				DisposeRadioButton(Window->Bits16Button);
				goto FailurePoint6;
			}
		SetRadioButtonState(Window->SignedButton,True); /* signed by default */

		Window->UnsignedButton = NewRadioButton(Window->ScreenID,"Unsigned",
			UNSIGNEDBUTTONX,UNSIGNEDBUTTONY,UNSIGNEDBUTTONWIDTH,UNSIGNEDBUTTONHEIGHT);
		if (Window->UnsignedButton == NIL)
			{
			 FailurePoint8:
				DisposeRadioButton(Window->SignedButton);
				goto FailurePoint7;
			}

		Window->SignBitButton = NewRadioButton(Window->ScreenID,"Sign Bit",
			SIGNBITBUTTONX,SIGNBITBUTTONY,SIGNBITBUTTONWIDTH,SIGNBITBUTTONHEIGHT);
		if (Window->SignBitButton == NIL)
			{
			 FailurePoint9:
				DisposeRadioButton(Window->UnsignedButton);
				goto FailurePoint8;
			}

		Window->LittleEndian = NewRadioButton(Window->ScreenID,"Little Endian",
			LITTLEENDIANBUTTONX,LITTLEENDIANBUTTONY,LITTLEENDIANBUTTONWIDTH,
			LITTLEENDIANBUTTONHEIGHT);
		if (Window->LittleEndian == NIL)
			{
			 FailurePoint10:
				DisposeRadioButton(Window->SignBitButton);
				goto FailurePoint9;
			}
		SetRadioButtonState(Window->LittleEndian,True); /* little endian by default */

		Window->BigEndian = NewRadioButton(Window->ScreenID,"Big Endian",
			BIGENDIANBUTTONX,BIGENDIANBUTTONY,BIGENDIANBUTTONWIDTH,BIGENDIANBUTTONHEIGHT);
		if (Window->BigEndian == NIL)
			{
			 FailurePoint11:
				DisposeRadioButton(Window->LittleEndian);
				goto FailurePoint10;
			}

		Window->OKButton = NewSimpleButton(Window->ScreenID,"OK",OKBUTTONX,OKBUTTONY,
			OKBUTTONWIDTH,OKBUTTONHEIGHT);
		if (Window->OKButton == NIL)
			{
			 FailurePoint12:
				DisposeRadioButton(Window->BigEndian);
				goto FailurePoint11;
			}
		SetDefaultButtonState(Window->OKButton,True);

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",CANCELBUTTONX,
			CANCELBUTTONY,CANCELBUTTONWIDTH,CANCELBUTTONHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint13:
				DisposeSimpleButton(Window->OKButton);
				goto FailurePoint12;
			}

		Window->MonoButton = NewRadioButton(Window->ScreenID,"Mono",NUMCHANNELSMONOBUTTONX,
			NUMCHANNELSMONOBUTTONY,NUMCHANNELSMONOBUTTONWIDTH,NUMCHANNELSMONOBUTTONHEIGHT);
		if (Window->MonoButton == NIL)
			{
			 FailurePoint14:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint13;
			}
		SetRadioButtonState(Window->MonoButton,True); /* mono by default */

		Window->StereoButton = NewRadioButton(Window->ScreenID,"Stereo",
			NUMCHANNELSSTEREOBUTTONX,NUMCHANNELSSTEREOBUTTONY,NUMCHANNELSSTEREOBUTTONWIDTH,
			NUMCHANNELSSTEREOBUTTONHEIGHT);
		if (Window->StereoButton == NIL)
			{
			 FailurePoint15:
				DisposeRadioButton(Window->MonoButton);
				goto FailurePoint14;
			}

		ActiveTextEdit = Window->BytesToSkipEdit;
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
							if (TextEditIBeamTest(Window->BytesToSkipEdit,X,Y)
								|| TextEditIBeamTest(Window->SkipBetweenEdit,X,Y))
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
										"DoImportDialog: Undefined menu option chosen"));
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
									if (ActiveTextEdit == Window->BytesToSkipEdit)
										{
											ActiveTextEdit = Window->SkipBetweenEdit;
										}
									 else
										{
											ActiveTextEdit = Window->BytesToSkipEdit;
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
							else if (TextEditHitTest(Window->SkipBetweenEdit,X,Y))
								{
									if (ActiveTextEdit != Window->SkipBetweenEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->SkipBetweenEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->BytesToSkipEdit,X,Y))
								{
									if (ActiveTextEdit != Window->BytesToSkipEdit)
										{
											DisableTextEditSelection(ActiveTextEdit);
											ActiveTextEdit = Window->BytesToSkipEdit;
											EnableTextEditSelection(ActiveTextEdit);
										}
									TextEditDoMouseDown(ActiveTextEdit,X,Y,Modifiers);
								}
							else if (RadioButtonHitTest(Window->Bits8Button,X,Y))
								{
									if (RadioButtonMouseDown(Window->Bits8Button,X,Y))
										{
											SetRadioButtonState(Window->Bits16Button,False);
										}
								}
							else if (RadioButtonHitTest(Window->Bits16Button,X,Y))
								{
									if (RadioButtonMouseDown(Window->Bits16Button,X,Y))
										{
											SetRadioButtonState(Window->Bits8Button,False);
										}
								}
							else if (RadioButtonHitTest(Window->SignedButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->SignedButton,X,Y))
										{
											SetRadioButtonState(Window->UnsignedButton,False);
											SetRadioButtonState(Window->SignBitButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->UnsignedButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->UnsignedButton,X,Y))
										{
											SetRadioButtonState(Window->SignedButton,False);
											SetRadioButtonState(Window->SignBitButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->SignBitButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->SignBitButton,X,Y))
										{
											SetRadioButtonState(Window->SignedButton,False);
											SetRadioButtonState(Window->UnsignedButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->LittleEndian,X,Y))
								{
									if (RadioButtonMouseDown(Window->LittleEndian,X,Y))
										{
											SetRadioButtonState(Window->BigEndian,False);
										}
								}
							else if (RadioButtonHitTest(Window->BigEndian,X,Y))
								{
									if (RadioButtonMouseDown(Window->BigEndian,X,Y))
										{
											SetRadioButtonState(Window->LittleEndian,False);
										}
								}
							else if (RadioButtonHitTest(Window->MonoButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->MonoButton,X,Y))
										{
											SetRadioButtonState(Window->StereoButton,False);
										}
								}
							else if (RadioButtonHitTest(Window->StereoButton,X,Y))
								{
									if (RadioButtonMouseDown(Window->StereoButton,X,Y))
										{
											SetRadioButtonState(Window->MonoButton,False);
										}
								}
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"DoImportDialog:  DoItFlag is neither true nor false"));

		if (DoItFlag)
			{
				NumBitsType				NumBits;
				NumChannelsType		NumChannels;
				SignModes					SigningMode;
				EndianType				Endianness;
				long							FramePadding;
				long							InitialSkip;
				BufferedInputRec*	File;
				SampleObjectRec*	ReturnedSampleObject;

				StringTemp = TextEditGetRawData(Window->BytesToSkipEdit,SYSTEMLINEFEED);
				if (StringTemp == NIL)
					{
						AlertHalt("There is not enough memory available to import the sample.",NIL);
						goto DoItFailedPoint;
					}
				InitialSkip = StringToInteger(StringTemp,PtrSize(StringTemp));
				ReleasePtr(StringTemp);

				StringTemp = TextEditGetRawData(Window->SkipBetweenEdit,SYSTEMLINEFEED);
				if (StringTemp == NIL)
					{
						AlertHalt("There is not enough memory available to import the sample.",NIL);
						goto DoItFailedPoint;
					}
				FramePadding = StringToInteger(StringTemp,PtrSize(StringTemp));
				ReleasePtr(StringTemp);

				if (GetRadioButtonState(Window->Bits8Button))
					{
						NumBits = eSample8bit;
					}
				else if (GetRadioButtonState(Window->Bits16Button))
					{
						NumBits = eSample16bit;
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"DoImportDialog:  bits radio button problem"));
					}

				if (GetRadioButtonState(Window->SignedButton))
					{
						SigningMode = eSignSigned;
					}
				else if (GetRadioButtonState(Window->UnsignedButton))
					{
						SigningMode = eSignUnsigned;
					}
				else if (GetRadioButtonState(Window->SignBitButton))
					{
						SigningMode = eSignSignBit;
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"DoImportDialog:  sign radio button problem"));
					}

				if (GetRadioButtonState(Window->LittleEndian))
					{
						Endianness = eEndianLittle;
					}
				else if (GetRadioButtonState(Window->BigEndian))
					{
						Endianness = eEndianBig;
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"DoImportDialog:  endianness radio button problem"));
					}

				if (GetRadioButtonState(Window->MonoButton))
					{
						NumChannels = eSampleMono;
					}
				else if (GetRadioButtonState(Window->StereoButton))
					{
						NumChannels = eSampleStereo;
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"DoImportDialog:  channels radio button problem"));
					}

				if (GetFileLength(FileDesc) <= InitialSkip)
					{
						AlertHalt("Initial skip value skipped entire file.",NIL);
						goto DoItFailedPoint;
					}
				if (!SetFilePosition(FileDesc,InitialSkip))
					{
						AlertHalt("Unable to read from the file.",NIL);
						goto DoItFailedPoint;
					}
				File = NewBufferedInput(FileDesc);
				if (File == NIL)
					{
						AlertHalt("There is not enough memory available to import the sample.",NIL);
						goto DoItFailedPoint;
					}
				switch (NumChannels)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad num channels"));
							break;
						case eSampleMono:
							switch (NumBits)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad num bits"));
										break;
									case eSample8bit:
										{
											signed char*		Sample;
											long						NumFrames;
											long						Scan;

											NumFrames = (GetFileLength(FileDesc) - InitialSkip)
												/ (1 + FramePadding);
											Sample = (signed char*)AllocPtrCanFail(NumFrames,
												"DoImportDialog[RAW]:  Sample");
											if (Sample == NIL)
												{
													EndBufferedInput(File);
													AlertHalt("There is not enough memory available to import "
														"the sample.",NIL);
													goto DoItFailedPoint;
												}
											for (Scan = 0; Scan < NumFrames; Scan += 1)
												{
													unsigned char		TheValue;
													long						Index;

													if (!ReadBufferedUnsignedChar(File,&TheValue))
														{
															ReleasePtr((char*)Sample);
															EndBufferedInput(File);
															AlertHalt("Unable to read from the file.",NIL);
															goto DoItFailedPoint;
														}
													PRNGCHK(Sample,&(Sample[Scan]),sizeof(Sample[Scan]));
													switch (SigningMode)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad sign mode"));
																break;
															case eSignSigned:
																if ((TheValue & 0x80) != 0)
																	{
																		Sample[Scan] = (TheValue & 0x7f) | ~0x7f;
																	}
																 else
																	{
																		Sample[Scan] = TheValue & 0x7f;
																	}
																break;
															case eSignUnsigned:
																Sample[Scan] = (TheValue & 0xff) - 128;
																break;
															case eSignSignBit:
																if ((TheValue & 0x80) != 0)
																	{
																		Sample[Scan] = -(TheValue & 0x7f);
																	}
																 else
																	{
																		Sample[Scan] = TheValue & 0x7f;
																	}
																break;
														}
													for (Index = 0; Index < FramePadding; Index += 1)
														{
															unsigned char					Stupid;

															if (!ReadBufferedUnsignedChar(File,&Stupid))
																{
																	ReleasePtr((char*)Sample);
																	EndBufferedInput(File);
																	AlertHalt("Unable to read from the file.",NIL);
																	goto DoItFailedPoint;
																}
														}
												}
											ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
												(char*)Sample,NumBits,NumChannels,0,0,0,0,0,0,0,
												DEFAULTSAMPLINGRATE,261.625565300598635);
											ReleasePtr((char*)Sample);
										}
										break;
									case eSample16bit:
										{
											signed short*		Sample;
											long						NumFrames;
											long						Scan;

											NumFrames = (GetFileLength(FileDesc) - InitialSkip)
												/ (2 + FramePadding);
											Sample = (signed short*)AllocPtrCanFail(NumFrames * sizeof(short),
												"DoImportDialog[RAW]:  Sample");
											if (Sample == NIL)
												{
													EndBufferedInput(File);
													AlertHalt("There is not enough memory available to import "
														"the sample.",NIL);
													goto DoItFailedPoint;
												}
											for (Scan = 0; Scan < NumFrames; Scan += 1)
												{
													unsigned short	TheValue;
													long						Index;

													switch (Endianness)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad endianness"));
																break;
															case eEndianLittle:
																if (!ReadBufferedUnsignedShortLittleEndian(File,&TheValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																break;
															case eEndianBig:
																if (!ReadBufferedUnsignedShortBigEndian(File,&TheValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																break;
														}
													PRNGCHK(Sample,&(Sample[Scan]),sizeof(Sample[Scan]));
													switch (SigningMode)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad sign mode"));
																break;
															case eSignSigned:
																if ((TheValue & 0x8000) != 0)
																	{
																		Sample[Scan] = (TheValue & 0x7fff) | ~0x7fff;
																	}
																 else
																	{
																		Sample[Scan] = TheValue & 0x7fff;
																	}
																break;
															case eSignUnsigned:
																Sample[Scan] = (TheValue & 0xffff) - 32768;
																break;
															case eSignSignBit:
																if ((TheValue & 0x8000) != 0)
																	{
																		Sample[Scan] = -(TheValue & 0x7fff);
																	}
																 else
																	{
																		Sample[Scan] = TheValue & 0x7fff;
																	}
																break;
														}
													for (Index = 0; Index < FramePadding; Index += 1)
														{
															unsigned char					Stupid;

															if (!ReadBufferedUnsignedChar(File,&Stupid))
																{
																	ReleasePtr((char*)Sample);
																	EndBufferedInput(File);
																	AlertHalt("Unable to read from the file.",NIL);
																	goto DoItFailedPoint;
																}
														}
												}
											ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
												(char*)Sample,NumBits,NumChannels,0,0,0,0,0,0,0,
												DEFAULTSAMPLINGRATE,261.625565300598635);
											ReleasePtr((char*)Sample);
										}
										break;
								}
							break;
						case eSampleStereo:
							switch (NumBits)
								{
									default:
										EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad num bits"));
										break;
									case eSample8bit:
										{
											signed char*		Sample;
											long						NumFrames;
											long						Scan;

											NumFrames = (GetFileLength(FileDesc) - InitialSkip)
												/ (2 + FramePadding);
											Sample = (signed char*)AllocPtrCanFail(NumFrames * 2,
												"DoImportDialog[RAW]:  Sample");
											if (Sample == NIL)
												{
													EndBufferedInput(File);
													AlertHalt("There is not enough memory available to import "
														"the sample.",NIL);
													goto DoItFailedPoint;
												}
											for (Scan = 0; Scan < NumFrames; Scan += 2)
												{
													unsigned char		LeftValue;
													unsigned char		RightValue;
													long						Index;

													if (!ReadBufferedUnsignedChar(File,&LeftValue))
														{
															ReleasePtr((char*)Sample);
															EndBufferedInput(File);
															AlertHalt("Unable to read from the file.",NIL);
															goto DoItFailedPoint;
														}
													if (!ReadBufferedUnsignedChar(File,&RightValue))
														{
															ReleasePtr((char*)Sample);
															EndBufferedInput(File);
															AlertHalt("Unable to read from the file.",NIL);
															goto DoItFailedPoint;
														}
													PRNGCHK(Sample,&(Sample[Scan]),sizeof(Sample[Scan]));
													PRNGCHK(Sample,&(Sample[Scan + 1]),sizeof(Sample[Scan + 1]));
													switch (SigningMode)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad sign mode"));
																break;
															case eSignSigned:
																if ((LeftValue & 0x80) != 0)
																	{
																		Sample[Scan] = (LeftValue & 0x7f) | ~0x7f;
																	}
																 else
																	{
																		Sample[Scan] = LeftValue & 0x7f;
																	}
																if ((RightValue & 0x80) != 0)
																	{
																		Sample[Scan + 1] = (RightValue & 0x7f) | ~0x7f;
																	}
																 else
																	{
																		Sample[Scan + 1] = RightValue & 0x7f;
																	}
																break;
															case eSignUnsigned:
																Sample[Scan] = (LeftValue & 0xff) - 128;
																Sample[Scan + 1] = (RightValue & 0xff) - 128;
																break;
															case eSignSignBit:
																if ((LeftValue & 0x80) != 0)
																	{
																		Sample[Scan] = -(LeftValue & 0x7f);
																	}
																 else
																	{
																		Sample[Scan] = LeftValue & 0x7f;
																	}
																if ((RightValue & 0x80) != 0)
																	{
																		Sample[Scan + 1] = -(RightValue & 0x7f);
																	}
																 else
																	{
																		Sample[Scan + 1] = RightValue & 0x7f;
																	}
																break;
														}
													for (Index = 0; Index < FramePadding; Index += 1)
														{
															unsigned char					Stupid;

															if (!ReadBufferedUnsignedChar(File,&Stupid))
																{
																	ReleasePtr((char*)Sample);
																	EndBufferedInput(File);
																	AlertHalt("Unable to read from the file.",NIL);
																	goto DoItFailedPoint;
																}
														}
												}
											ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
												(char*)Sample,NumBits,NumChannels,0,0,0,0,0,0,0,
												DEFAULTSAMPLINGRATE,261.625565300598635);
											ReleasePtr((char*)Sample);
										}
										break;
									case eSample16bit:
										{
											signed short*		Sample;
											long						NumFrames;
											long						Scan;

											NumFrames = (GetFileLength(FileDesc) - InitialSkip)
												/ (4 + FramePadding);
											Sample = (signed short*)AllocPtrCanFail(NumFrames * 2 * sizeof(short),
												"DoImportDialog[RAW]:  Sample");
											if (Sample == NIL)
												{
													EndBufferedInput(File);
													AlertHalt("There is not enough memory available to import "
														"the sample.",NIL);
													goto DoItFailedPoint;
												}
											for (Scan = 0; Scan < NumFrames; Scan += 2)
												{
													unsigned short	LeftValue;
													unsigned short	RightValue;
													long						Index;

													switch (Endianness)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad endianness"));
																break;
															case eEndianLittle:
																if (!ReadBufferedUnsignedShortLittleEndian(File,&LeftValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																if (!ReadBufferedUnsignedShortLittleEndian(File,&RightValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																break;
															case eEndianBig:
																if (!ReadBufferedUnsignedShortBigEndian(File,&LeftValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																if (!ReadBufferedUnsignedShortBigEndian(File,&RightValue))
																	{
																		ReleasePtr((char*)Sample);
																		EndBufferedInput(File);
																		AlertHalt("Unable to read from the file.",NIL);
																		goto DoItFailedPoint;
																	}
																break;
														}
													PRNGCHK(Sample,&(Sample[Scan]),sizeof(Sample[Scan]));
													PRNGCHK(Sample,&(Sample[Scan + 1]),sizeof(Sample[Scan + 1]));
													switch (SigningMode)
														{
															default:
																EXECUTE(PRERR(ForceAbort,"DoImportDialog:  bad sign mode"));
																break;
															case eSignSigned:
																if ((LeftValue & 0x8000) != 0)
																	{
																		Sample[Scan] = (LeftValue & 0x7fff) | ~0x7fff;
																	}
																 else
																	{
																		Sample[Scan] = LeftValue & 0x7fff;
																	}
																if ((RightValue & 0x8000) != 0)
																	{
																		Sample[Scan + 1] = (RightValue & 0x7fff) | ~0x7fff;
																	}
																 else
																	{
																		Sample[Scan + 1] = RightValue & 0x7fff;
																	}
																break;
															case eSignUnsigned:
																Sample[Scan] = (LeftValue & 0xffff) - 32768;
																Sample[Scan + 1] = (RightValue & 0xffff) - 32768;
																break;
															case eSignSignBit:
																if ((LeftValue & 0x8000) != 0)
																	{
																		Sample[Scan] = -(LeftValue & 0x7fff);
																	}
																 else
																	{
																		Sample[Scan] = LeftValue & 0x7fff;
																	}
																if ((RightValue & 0x8000) != 0)
																	{
																		Sample[Scan + 1] = -(RightValue & 0x7fff);
																	}
																 else
																	{
																		Sample[Scan + 1] = RightValue & 0x7fff;
																	}
																break;
														}
													for (Index = 0; Index < FramePadding; Index += 1)
														{
															unsigned char					Stupid;

															if (!ReadBufferedUnsignedChar(File,&Stupid))
																{
																	ReleasePtr((char*)Sample);
																	EndBufferedInput(File);
																	AlertHalt("Unable to read from the file.",NIL);
																	goto DoItFailedPoint;
																}
														}
												}
											ReturnedSampleObject = MainWindowCopyRawSampleAndOpen(MainWindow,
												(char*)Sample,NumBits,NumChannels,0,0,0,0,0,0,0,
												DEFAULTSAMPLINGRATE,261.625565300598635);
											ReleasePtr((char*)Sample);
										}
										break;
								}
							break;
					}
				if (ReturnedSampleObject != NIL)
					{
						char*					Filename;

						Filename = ExtractFileName(WhereIsTheFile);
						if (Filename != NIL)
							{
								/* we don't need to release the name */
								SampleObjectNewName(ReturnedSampleObject,Filename);
							}
					}
				EndBufferedInput(File);

				/* jump here when reading in fails */
			 DoItFailedPoint:
				;
			}

		DisposeRadioButton(Window->Bits8Button);
		DisposeRadioButton(Window->Bits16Button);
		DisposeRadioButton(Window->SignedButton);
		DisposeRadioButton(Window->UnsignedButton);
		DisposeRadioButton(Window->SignBitButton);
		DisposeRadioButton(Window->LittleEndian);
		DisposeRadioButton(Window->BigEndian);
		DisposeRadioButton(Window->MonoButton);
		DisposeRadioButton(Window->StereoButton);
		DisposeTextEdit(Window->BytesToSkipEdit);
		DisposeTextEdit(Window->SkipBetweenEdit);
		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}
