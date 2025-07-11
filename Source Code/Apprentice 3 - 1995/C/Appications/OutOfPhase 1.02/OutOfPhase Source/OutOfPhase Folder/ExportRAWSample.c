/* ExportRAWSample.c */
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

#include "ExportRAWSample.h"
#include "Screen.h"
#include "RadioButton.h"
#include "SimpleButton.h"
#include "Memory.h"
#include "Alert.h"
#include "EventLoop.h"
#include "Files.h"
#include "BufferedFileOutput.h"
#include "SampleStorageActual.h"
#include "SampleConsts.h"


typedef enum
	{
		eSignSigned EXECUTE(= -4523),
		eSignUnsigned,
		eSignSignBit
	} SignModes;


typedef enum
	{
		eEndianLittle EXECUTE(= -26123),
		eEndianBig
	} EndianType;


#define WINDOWXSIZE (400)

#define PROMPTX (10)
#define PROMPTY (5)

#define SIGNPROMPTX (PROMPTX)
#define SIGNPROMPTY (PROMPTY + 20 + 6)

#define SIGNEDBUTTONX (SIGNPROMPTX + 100)
#define SIGNEDBUTTONY (SIGNPROMPTY - 4)
#define SIGNEDBUTTONWIDTH (80)
#define SIGNEDBUTTONHEIGHT (21)

#define UNSIGNEDBUTTONX (SIGNEDBUTTONX + SIGNEDBUTTONWIDTH + 10)
#define UNSIGNEDBUTTONY (SIGNEDBUTTONY)
#define UNSIGNEDBUTTONWIDTH (SIGNEDBUTTONWIDTH)
#define UNSIGNEDBUTTONHEIGHT (SIGNEDBUTTONHEIGHT)

#define SIGNBITBUTTONX (UNSIGNEDBUTTONX + UNSIGNEDBUTTONWIDTH + 10)
#define SIGNBITBUTTONY (UNSIGNEDBUTTONY)
#define SIGNBITBUTTONWIDTH (SIGNEDBUTTONWIDTH)
#define SIGNBITBUTTONHEIGHT (SIGNEDBUTTONHEIGHT)

#define ENDIANPROMPTX (SIGNPROMPTX)
#define ENDIANPROMPTY (SIGNPROMPTY + SIGNEDBUTTONHEIGHT + 3)

#define LITTLEENDIANBUTTONX (SIGNEDBUTTONX)
#define LITTLEENDIANBUTTONY (ENDIANPROMPTY - 4)
#define LITTLEENDIANBUTTONWIDTH (SIGNEDBUTTONWIDTH)
#define LITTLEENDIANBUTTONHEIGHT (SIGNEDBUTTONHEIGHT)

#define BIGENDIANBUTTONX (LITTLEENDIANBUTTONX + LITTLEENDIANBUTTONWIDTH + 10)
#define BIGENDIANBUTTONY (LITTLEENDIANBUTTONY)
#define BIGENDIANBUTTONWIDTH (SIGNEDBUTTONWIDTH)
#define BIGENDIANBUTTONHEIGHT (SIGNEDBUTTONHEIGHT)

#define OKBUTTONWIDTH (80)
#define OKBUTTONHEIGHT (21)
#define OKBUTTONX (WINDOWXSIZE / 4 - OKBUTTONWIDTH / 2)
#define OKBUTTONY (BIGENDIANBUTTONY + BIGENDIANBUTTONHEIGHT + 13)

#define CANCELBUTTONWIDTH (OKBUTTONWIDTH)
#define CANCELBUTTONHEIGHT (OKBUTTONHEIGHT)
#define CANCELBUTTONX (3 * WINDOWXSIZE / 4 - OKBUTTONWIDTH / 2)
#define CANCELBUTTONY (OKBUTTONY)

#define WINDOWYSIZE (CANCELBUTTONY + CANCELBUTTONHEIGHT + 10)


typedef struct
	{
		WinType*					ScreenID;
		RadioButtonRec*		SignedButton;
		RadioButtonRec*		UnsignedButton;
		RadioButtonRec*		SignBitButton;
		RadioButtonRec*		LittleEndian;
		RadioButtonRec*		BigEndian;
		SimpleButtonRec*	OKButton;
		SimpleButtonRec*	CancelButton;
	} WindowRec;


static void						RedrawWindow(WindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawRadioButton(Window->SignedButton);
		RedrawRadioButton(Window->UnsignedButton);
		RedrawRadioButton(Window->SignBitButton);
		RedrawRadioButton(Window->LittleEndian);
		RedrawRadioButton(Window->BigEndian);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		SetClipRect(Window->ScreenID,0,0,WINDOWXSIZE,WINDOWYSIZE);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Export Raw Sample:",18,
			PROMPTX,PROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Sample Sign:",12,
			SIGNPROMPTX,SIGNPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Endianness:",11,
			ENDIANPROMPTX,ENDIANPROMPTY,ePlain);
	}


/* this routine saves the data in the provided sample storage object as a raw */
/* data file.  it handles any error reporting to the user.  the object is */
/* NOT disposed, so the caller has to do that. */
void								ExportRAWSample(struct SampleStorageActualRec* TheSample)
	{
		WindowRec*					Window;
		MyBoolean						LoopFlag;
		MyBoolean						DoItFlag EXECUTE(= -1324);

		CheckPtrExistence(TheSample);

		Window = (WindowRec*)AllocPtrCanFail(sizeof(WindowRec),"ExportRAWSample:  WindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to export the sample.",NIL);
				return;
			}

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
		SetWindowName(Window->ScreenID,"Export Raw Sample");

		Window->SignedButton = NewRadioButton(Window->ScreenID,"Signed",
			SIGNEDBUTTONX,SIGNEDBUTTONY,SIGNEDBUTTONWIDTH,SIGNEDBUTTONHEIGHT);
		if (Window->SignedButton == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		SetRadioButtonState(Window->SignedButton,True); /* signed by default */

		Window->UnsignedButton = NewRadioButton(Window->ScreenID,"Unsigned",
			UNSIGNEDBUTTONX,UNSIGNEDBUTTONY,UNSIGNEDBUTTONWIDTH,UNSIGNEDBUTTONHEIGHT);
		if (Window->UnsignedButton == NIL)
			{
			 FailurePoint4:
				DisposeRadioButton(Window->SignedButton);
				goto FailurePoint3;
			}

		Window->SignBitButton = NewRadioButton(Window->ScreenID,"Sign Bit",
			SIGNBITBUTTONX,SIGNBITBUTTONY,SIGNBITBUTTONWIDTH,SIGNBITBUTTONHEIGHT);
		if (Window->SignBitButton == NIL)
			{
			 FailurePoint5:
				DisposeRadioButton(Window->UnsignedButton);
				goto FailurePoint4;
			}

		Window->LittleEndian = NewRadioButton(Window->ScreenID,"Little Endian",
			LITTLEENDIANBUTTONX,LITTLEENDIANBUTTONY,LITTLEENDIANBUTTONWIDTH,
			LITTLEENDIANBUTTONHEIGHT);
		if (Window->LittleEndian == NIL)
			{
			 FailurePoint6:
				DisposeRadioButton(Window->SignBitButton);
				goto FailurePoint5;
			}
		SetRadioButtonState(Window->LittleEndian,True); /* little endian by default */

		Window->BigEndian = NewRadioButton(Window->ScreenID,"Big Endian",
			BIGENDIANBUTTONX,BIGENDIANBUTTONY,BIGENDIANBUTTONWIDTH,BIGENDIANBUTTONHEIGHT);
		if (Window->BigEndian == NIL)
			{
			 FailurePoint7:
				DisposeRadioButton(Window->LittleEndian);
				goto FailurePoint6;
			}

		Window->OKButton = NewSimpleButton(Window->ScreenID,"OK",OKBUTTONX,OKBUTTONY,
			OKBUTTONWIDTH,OKBUTTONHEIGHT);
		if (Window->OKButton == NIL)
			{
			 FailurePoint8:
				DisposeRadioButton(Window->BigEndian);
				goto FailurePoint7;
			}
		SetDefaultButtonState(Window->OKButton,True);

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",CANCELBUTTONX,
			CANCELBUTTONY,CANCELBUTTONWIDTH,CANCELBUTTONHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint9:
				DisposeSimpleButton(Window->OKButton);
				goto FailurePoint8;
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
						case eNoEvent:
							break;
						case eMenuStarting:
							break;
						case eMenuCommand:
							EXECUTE(PRERR(AllowResume,
								"DoImportDialog: Undefined menu option chosen"));
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
							break;
					}
			}
		ERROR((DoItFlag != True) && (DoItFlag != False),PRERR(ForceAbort,
			"DoImportDialog:  DoItFlag is neither true nor false"));

		if (DoItFlag)
			{
				FileSpec*					WhereToPutIt;
				SignModes					Signing;
				EndianType				Endianness;

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
						EXECUTE(PRERR(AllowResume,"ExportRAWSample:  bad endian buttons"));
					}

				if (GetRadioButtonState(Window->SignedButton))
					{
						Signing = eSignSigned;
					}
				else if (GetRadioButtonState(Window->UnsignedButton))
					{
						Signing = eSignUnsigned;
					}
				else if (GetRadioButtonState(Window->SignBitButton))
					{
						Signing = eSignSignBit;
					}
				else
					{
						EXECUTE(PRERR(AllowResume,"ExportRAWSample:  bad signing buttons"));
					}

				WhereToPutIt = PutFile("Untitled.RAW");
				if (WhereToPutIt != NIL)
					{
						if (CreateFile(WhereToPutIt,ApplicationCreator,CODE4BYTES('?','?','?','?')))
							{
								FileType*					FileDesc;

								if (OpenFile(WhereToPutIt,&FileDesc,eReadAndWrite))
									{
										BufferedOutputRec*	File;

										File = NewBufferedOutput(FileDesc);
										if (File != NIL)
											{
												long								Scan;
												long								Limit;

												Limit = GetSampleStorageActualNumFrames(TheSample);
												switch (GetSampleStorageActualNumChannels(TheSample))
													{
														default:
															EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad num channels"));
															break;
														case eSampleStereo:
															switch (GetSampleStorageActualNumBits(TheSample))
																{
																	default:
																		EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad number of bits"));
																		break;
																	case eSample8bit:
																		for (Scan = 0; Scan < Limit; Scan += 1)
																			{
																				largefixedsigned		SampleLeft;
																				largefixedsigned		SampleRight;
																				unsigned char				DataLeft;
																				unsigned char				DataRight;

																				SampleLeft = GetSampleStorageActualValue(
																					TheSample,Scan,eLeftChannel);
																				SampleRight = GetSampleStorageActualValue(
																					TheSample,Scan,eRightChannel);
																				switch (Signing)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad signing selector"));
																							break;
																						case eSignSigned:
																							DataLeft = roundtonearest(largefixed2double(SampleLeft)
																								* MAX8BIT);
																							DataRight = roundtonearest(largefixed2double(SampleRight)
																								* MAX8BIT);
																							break;
																						case eSignUnsigned:
																							DataLeft = roundtonearest(largefixed2double(SampleLeft)
																								* MAX8BIT) + 128;
																							DataRight = roundtonearest(largefixed2double(SampleRight)
																								* MAX8BIT) + 128;
																							break;
																						case eSignSignBit:
																							if (SampleLeft >= 0)
																								{
																									DataLeft = roundtonearest(largefixed2double(SampleLeft)
																										* MAX8BIT);
																								}
																							 else
																								{
																									DataLeft = roundtonearest(- largefixed2double(SampleLeft)
																										* MAX8BIT) | 0x80;
																								}
																							if (SampleRight >= 0)
																								{
																									DataRight = roundtonearest(largefixed2double(SampleRight)
																										* MAX8BIT);
																								}
																							 else
																								{
																									DataRight = roundtonearest(- largefixed2double(SampleRight)
																										* MAX8BIT) | 0x80;
																								}
																							break;
																					}
																				if (!WriteBufferedUnsignedChar(File,DataLeft))
																					{
																					 DiskErrorPoint:
																						AlertHalt("Unable to write the data to the file.",NIL);
																						goto FailureEscapePoint;
																					}
																				if (!WriteBufferedUnsignedChar(File,DataLeft))
																					{
																						goto DiskErrorPoint;
																					}
																			}
																		break;
																	case eSample16bit:
																		for (Scan = 0; Scan < Limit; Scan += 1)
																			{
																				largefixedsigned		SampleLeft;
																				largefixedsigned		SampleRight;
																				unsigned short			DataLeft;
																				unsigned short			DataRight;

																				SampleLeft = GetSampleStorageActualValue(
																					TheSample,Scan,eLeftChannel);
																				SampleRight = GetSampleStorageActualValue(
																					TheSample,Scan,eRightChannel);
																				switch (Signing)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad signing selector"));
																							break;
																						case eSignSigned:
																							DataLeft = roundtonearest(largefixed2double(SampleLeft)
																								* MAX16BIT);
																							DataRight = roundtonearest(largefixed2double(SampleRight)
																								* MAX16BIT);
																							break;
																						case eSignUnsigned:
																							DataLeft = roundtonearest(largefixed2double(SampleLeft)
																								* MAX16BIT) + 32768;
																							DataRight = roundtonearest(largefixed2double(SampleRight)
																								* MAX16BIT) + 32768;
																							break;
																						case eSignSignBit:
																							if (SampleLeft >= 0)
																								{
																									DataLeft = roundtonearest(largefixed2double(SampleLeft)
																										* MAX16BIT);
																								}
																							 else
																								{
																									DataLeft = roundtonearest(- largefixed2double(SampleLeft)
																										* MAX16BIT) | 0x8000;
																								}
																							if (SampleRight >= 0)
																								{
																									DataRight = roundtonearest(largefixed2double(SampleRight)
																										* MAX16BIT);
																								}
																							 else
																								{
																									DataRight = roundtonearest(- largefixed2double(SampleRight)
																										* MAX16BIT) | 0x8000;
																								}
																							break;
																					}
																				switch (Endianness)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad endianness selector"));
																							break;
																						case eEndianLittle:
																							if (!WriteBufferedUnsignedShortLittleEndian(File,DataLeft))
																								{
																									goto DiskErrorPoint;
																								}
																							if (!WriteBufferedUnsignedShortLittleEndian(File,DataLeft))
																								{
																									goto DiskErrorPoint;
																								}
																							break;
																						case eEndianBig:
																							if (!WriteBufferedUnsignedShortBigEndian(File,DataLeft))
																								{
																									goto DiskErrorPoint;
																								}
																							if (!WriteBufferedUnsignedShortBigEndian(File,DataLeft))
																								{
																									goto DiskErrorPoint;
																								}
																							break;
																					}
																			}
																		break;
																}
															break;
														case eSampleMono:
															switch (GetSampleStorageActualNumBits(TheSample))
																{
																	default:
																		EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad number of bits"));
																		break;
																	case eSample8bit:
																		for (Scan = 0; Scan < Limit; Scan += 1)
																			{
																				largefixedsigned		Sample;
																				unsigned char				Data;

																				Sample = GetSampleStorageActualValue(
																					TheSample,Scan,eMonoChannel);
																				switch (Signing)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad signing selector"));
																							break;
																						case eSignSigned:
																							Data = roundtonearest(largefixed2double(Sample) * MAX8BIT);
																							break;
																						case eSignUnsigned:
																							Data = roundtonearest(largefixed2double(Sample) * MAX8BIT) + 128;
																							break;
																						case eSignSignBit:
																							if (Sample >= 0)
																								{
																									Data = roundtonearest(largefixed2double(Sample) * MAX8BIT);
																								}
																							 else
																								{
																									Data = roundtonearest(- largefixed2double(Sample)
																										* MAX8BIT) | 0x80;
																								}
																							break;
																					}
																				if (!WriteBufferedUnsignedChar(File,Data))
																					{
																						goto DiskErrorPoint;
																					}
																			}
																		break;
																	case eSample16bit:
																		for (Scan = 0; Scan < Limit; Scan += 1)
																			{
																				largefixedsigned		Sample;
																				unsigned short			Data;

																				Sample = GetSampleStorageActualValue(
																					TheSample,Scan,eMonoChannel);
																				switch (Signing)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad signing selector"));
																							break;
																						case eSignSigned:
																							Data = roundtonearest(largefixed2double(Sample) * MAX16BIT);
																							break;
																						case eSignUnsigned:
																							Data = roundtonearest(largefixed2double(Sample) * MAX16BIT) + 32768;
																							break;
																						case eSignSignBit:
																							if (Sample >= 0)
																								{
																									Data = roundtonearest(largefixed2double(Sample) * MAX16BIT);
																								}
																							 else
																								{
																									Data = roundtonearest(- largefixed2double(Sample)
																										* MAX16BIT) | 0x8000;
																								}
																							break;
																					}
																				switch (Endianness)
																					{
																						default:
																							EXECUTE(PRERR(ForceAbort,"ExportRAWSample:  bad endianness selector"));
																							break;
																						case eEndianLittle:
																							if (!WriteBufferedUnsignedShortLittleEndian(File,Data))
																								{
																									goto DiskErrorPoint;
																								}
																							break;
																						case eEndianBig:
																							if (!WriteBufferedUnsignedShortBigEndian(File,Data))
																								{
																									goto DiskErrorPoint;
																								}
																							break;
																					}
																			}
																		break;
																}
															break;
													}
											 FailureEscapePoint:
												EndBufferedOutput(File);
											}
										 else
											{
												AlertHalt("There is not enough memory available to "
													"export the sample.",NIL);
											}
										CloseFile(FileDesc);
									}
								 else
									{
										AlertHalt("The file could not be opened for writing.",NIL);
									}
							}
						 else
							{
								AlertHalt("The file could not be created.",NIL);
							}
						DisposeFileSpec(WhereToPutIt);
					}
			}

		DisposeRadioButton(Window->SignedButton);
		DisposeRadioButton(Window->UnsignedButton);
		DisposeRadioButton(Window->SignBitButton);
		DisposeRadioButton(Window->LittleEndian);
		DisposeRadioButton(Window->BigEndian);
		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}
