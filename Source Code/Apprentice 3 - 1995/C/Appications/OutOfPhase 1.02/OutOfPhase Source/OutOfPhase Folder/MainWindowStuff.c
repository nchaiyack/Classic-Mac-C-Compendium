/* MainWindowStuff.c */
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

#include "MainWindowStuff.h"
#include "Memory.h"
#include "Files.h"
#include "WindowDispatcher.h"
#include "Array.h"
#include "CodeCenter.h"
#include "TextEdit.h"
#include "SampleList.h"
#include "FunctionList.h"
#include "AlgoSampList.h"
#include "WaveTableList.h"
#include "AlgoWaveTableList.h"
#include "InstrList.h"
#include "TrackList.h"
#include "Alert.h"
#include "StartupOpen.h"
#include "CalculatorWindow.h"
#include "GrowIcon.h"
#include "Main.h"
#include "DisassemblyWindow.h"
#include "PcodeSystem.h"
#include "DataMunging.h"
#include "NumberDialog.h"
#include "Numbers.h"
#include "GlobalWindowMenuList.h"
#include "BinaryCodedDecimal.h"
#include "PlayPrefsDialog.h"
#include "ImportWAVSample.h"
#include "ImportRAWSample.h"
#include "ImportAIFFSample.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


#define MINTABCOUNT (1)
#define MAXTABCOUNT (255)
#define DEFAULTTABSIZE (2)


#define MAINWINSIZEX (450)
#define MAINWINSIZEY (250)

/* top-left corner */
#define TRACKX(W,H) (-1)
#define TRACKY(W,H) (-1)
#define TRACKWIDTH(W,H) ((W) / 3)
#define TRACKHEIGHT(W,H) ((H) / 3)

/* top-middle */
#define WAVETABLEX(W,H) (TRACKX(W,H) + TRACKWIDTH(W,H) + 1)
#define WAVETABLEY(W,H) (TRACKY(W,H))
#define WAVETABLEWIDTH(W,H) (((W) - WAVETABLEX(W,H)) / 2)
#define WAVETABLEHEIGHT(W,H) (TRACKHEIGHT(W,H))

/* top-right corner */
#define SAMPLEX(W,H) (WAVETABLEX(W,H) + WAVETABLEWIDTH(W,H) + 1)
#define SAMPLEY(W,H) (WAVETABLEY(W,H))
#define SAMPLEWIDTH(W,H) ((W) - SAMPLEX(W,H) + 1)
#define SAMPLEHEIGHT(W,H) (WAVETABLEHEIGHT(W,H))

/* middle-left */
#define INSTRUMENTX(W,H) (TRACKX(W,H))
#define INSTRUMENTY(W,H) (TRACKY(W,H) + TRACKHEIGHT(W,H))
#define INSTRUMENTWIDTH(W,H) (TRACKWIDTH(W,H))
#define INSTRUMENTHEIGHT(W,H) (((H) - INSTRUMENTY(W,H)) / 2)

/* middle-middle */
#define ALGOWAVETABLEX(W,H) (WAVETABLEX(W,H))
#define ALGOWAVETABLEY(W,H) (INSTRUMENTY(W,H))
#define ALGOWAVETABLEWIDTH(W,H) (WAVETABLEWIDTH(W,H))
#define ALGOWAVETABLEHEIGHT(W,H) (INSTRUMENTHEIGHT(W,H))

/* middle-right */
#define ALGOSAMPLEX(W,H) (SAMPLEX(W,H))
#define ALGOSAMPLEY(W,H) (ALGOWAVETABLEY(W,H))
#define ALGOSAMPLEWIDTH(W,H) (SAMPLEWIDTH(W,H))
#define ALGOSAMPLEHEIGHT(W,H) (ALGOWAVETABLEHEIGHT(W,H))

/* bottom-left corner */
#define FUNCTIONX(W,H) (INSTRUMENTX(W,H))
#define FUNCTIONY(W,H) (INSTRUMENTY(W,H) + INSTRUMENTHEIGHT(W,H))
#define FUNCTIONWIDTH(W,H) (INSTRUMENTWIDTH(W,H))
#define FUNCTIONHEIGHT(W,H) ((H) - FUNCTIONY(W,H) + 1)

/* bottom-middle and bottom-right corner */
#define COMMENTX(W,H) (FUNCTIONX(W,H) + FUNCTIONWIDTH(W,H) + 1)
#define COMMENTY(W,H) (FUNCTIONY(W,H) + 1)
#define COMMENTWIDTH(W,H) ((W) - COMMENTX(W,H) + 1)
#define COMMENTHEIGHT(W,H) (FUNCTIONHEIGHT(W,H) - 1)


/* this structure contains all the information for a document */
struct MainWindowRec
	{
		WinType*						ScreenID;
		MyBoolean						EverBeenSaved;
		FileSpec*						TheFileLocation; /* only valid if EverBeenSaved is true */
		FileType*						TheFile; /* only valid if EverBeenSaved is true */
		GenericWindowRec*		MyGenericWindow; /* how the window event dispatcher knows us */
		MenuItemType*				MyMenuItem;

		FileSpec*						DeleteUndoFileLocation; /* NIL = none */
		FileType*						DeleteUndoFile; /* NIL = none */

		ArrayRec*						ListOfCalcWindows;
		ArrayRec*						ListOfDisassemblies;
		CodeCenterRec*			CodeCenter; /* keeps all the object code */

		/* generic control parameters */
		long								TabSize;
		TextEditRec*				CommentInfo;

		/* score data */
		SampleListRec*			SampleList;
		FunctionListRec*		FunctionList;
		AlgoSampListRec*		AlgoSampList;
		WaveTableListRec*		WaveTableList;
		AlgoWaveTableListRec*	AlgoWaveTableList;
		InstrListRec*				InstrumentList;
		TrackListRec*				TrackList;

		MyBoolean						StuffModified;

		/* playback preferences */
		MyBoolean						StereoPlayback;
		MyBoolean						SurroundEncoding;
		long								SamplingRate;
		long								EnvelopeUpdateRate;
		LargeBCDType				DefaultBeatsPerMinute;
		LargeBCDType				OverallVolumeScalingFactor;
		OutputNumBitsType		OutputNumBits;
		MyBoolean						InterpolateOverTime;
		MyBoolean						InterpolateAcrossWaves;
		LargeBCDType				ScanningGap;
		LargeBCDType				BufferDuration;
		MyBoolean						ClipWarning;
		char*								SongPostProcessing;
		MyBoolean						SongPostProcessingEnable;
	};


static ArrayRec*				ListOfDocuments = NIL;


/* initialize internal data structures for documents */
MyBoolean						InitializeDocuments(void)
	{
		APRINT(("+InitializeDocuments"));
		ListOfDocuments = NewArray();
		if (ListOfDocuments == NIL)
			{
				APRINT(("-InitializeDocuments failed"));
				return False;
			}
		APRINT(("-InitializeDocuments"));
		return True;
	}


/* clean up any internal data structures used for documents */
void								ShutdownDocuments(void)
	{
		APRINT(("+ShutdownDocuments"));
		ERROR(ArrayGetLength(ListOfDocuments) != 0,PRERR(AllowResume,
			"ShutdownDocuments:  Not all documents disposed of"));
		DisposeArray(ListOfDocuments);
		APRINT(("-ShutdownDocuments"));
	}


/* open a new document window and load the document in.  the function takes */
/* ownership of the file specification. */
void								OpenDocument(FileSpec* TheFile)
	{
		MainWindowRec*		Window;

		/* allocation */
		Window = (MainWindowRec*)AllocPtrCanFail(sizeof(MainWindowRec),"MainWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to open the document.",NIL);
				if (TheFile != NIL)
					{
						DisposeFileSpec(TheFile);
					}
				return;
			}
		if (!ArrayAppendElement(ListOfDocuments,Window))
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}

		/* constructing members */

		Window->TabSize = DEFAULTTABSIZE;

		Window->CodeCenter = NewCodeCenter();
		if (Window->CodeCenter == NIL)
			{
			 FailurePoint3:
				ArrayDeleteElement(ListOfDocuments,ArrayFindElement(ListOfDocuments,Window));
				goto FailurePoint2;
			}

		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,
			eWindowZoomable,eWindowResizable,4 + WindowOtherEdgeWidths(eDocumentWindow),
			4 + WindowTitleBarHeight(eDocumentWindow),MAINWINSIZEX,MAINWINSIZEY,
			(void (*)(void*))&MainWindowUpdator,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint4:
				DisposeCodeCenter(Window->CodeCenter);
				goto FailurePoint3;
			}

		Window->CommentInfo = NewTextEdit(Window->ScreenID,
			eTEHScrollBar | eTEVScrollBar,GetScreenFont(),9,
			COMMENTX(MAINWINSIZEX,MAINWINSIZEY),COMMENTY(MAINWINSIZEX,MAINWINSIZEY),
			COMMENTWIDTH(MAINWINSIZEX,MAINWINSIZEY),COMMENTHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->CommentInfo == NIL)
			{
			 FailurePoint5:
				KillWindow(Window->ScreenID);
				goto FailurePoint4;
			}

		Window->SampleList = NewSampleList(Window,Window->CodeCenter,
			Window->ScreenID,SAMPLEX(MAINWINSIZEX,MAINWINSIZEY),
			SAMPLEY(MAINWINSIZEX,MAINWINSIZEY),SAMPLEWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			SAMPLEHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->SampleList == NIL)
			{
			 FailurePoint6:
				DisposeTextEdit(Window->CommentInfo);
				goto FailurePoint5;
			}

		Window->FunctionList = NewFunctionList(Window,Window->CodeCenter,
			Window->ScreenID,FUNCTIONX(MAINWINSIZEX,MAINWINSIZEY),
			FUNCTIONY(MAINWINSIZEX,MAINWINSIZEY),FUNCTIONWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			FUNCTIONHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->FunctionList == NIL)
			{
			 FailurePoint7:
				DisposeSampleList(Window->SampleList);
				goto FailurePoint6;
			}

		Window->AlgoSampList = NewAlgoSampList(Window,Window->CodeCenter,
			Window->ScreenID,ALGOSAMPLEX(MAINWINSIZEX,MAINWINSIZEY),
			ALGOSAMPLEY(MAINWINSIZEX,MAINWINSIZEY),
			ALGOSAMPLEWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			ALGOSAMPLEHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->AlgoSampList == NIL)
			{
			 FailurePoint8:
				DisposeFunctionList(Window->FunctionList);
				goto FailurePoint7;
			}

		Window->WaveTableList = NewWaveTableList(Window,Window->CodeCenter,
			Window->ScreenID,WAVETABLEX(MAINWINSIZEX,MAINWINSIZEY),
			WAVETABLEY(MAINWINSIZEX,MAINWINSIZEY),WAVETABLEWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			WAVETABLEHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->WaveTableList == NIL)
			{
			 FailurePoint9:
				DisposeAlgoSampList(Window->AlgoSampList);
				goto FailurePoint8;
			}

		Window->AlgoWaveTableList = NewAlgoWaveTableList(Window,Window->CodeCenter,
			Window->ScreenID,ALGOWAVETABLEX(MAINWINSIZEX,MAINWINSIZEY),
			ALGOWAVETABLEY(MAINWINSIZEX,MAINWINSIZEY),
			ALGOWAVETABLEWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			ALGOWAVETABLEHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->AlgoWaveTableList == NIL)
			{
			 FailurePoint10:
				DisposeWaveTableList(Window->WaveTableList);
				goto FailurePoint9;
			}

		Window->InstrumentList = NewInstrList(Window,Window->CodeCenter,
			Window->ScreenID,INSTRUMENTX(MAINWINSIZEX,MAINWINSIZEY),
			INSTRUMENTY(MAINWINSIZEX,MAINWINSIZEY),INSTRUMENTWIDTH(MAINWINSIZEX,MAINWINSIZEY),
			INSTRUMENTHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->InstrumentList == NIL)
			{
			 FailurePoint11:
				DisposeAlgoWaveTableList(Window->AlgoWaveTableList);
				goto FailurePoint10;
			}

		Window->TrackList = NewTrackList(Window,Window->CodeCenter,
			Window->ScreenID,TRACKX(MAINWINSIZEX,MAINWINSIZEY),TRACKY(MAINWINSIZEX,MAINWINSIZEY),
			TRACKWIDTH(MAINWINSIZEX,MAINWINSIZEY),TRACKHEIGHT(MAINWINSIZEX,MAINWINSIZEY));
		if (Window->TrackList == NIL)
			{
			 FailurePoint12:
				DisposeInstrList(Window->InstrumentList);
				goto FailurePoint11;
			}

		Window->ListOfCalcWindows = NewArray();
		if (Window->ListOfCalcWindows == NIL)
			{
			 FailurePoint13:
				DisposeTrackList(Window->TrackList);
				goto FailurePoint12;
			}

		Window->ListOfDisassemblies = NewArray();
		if (Window->ListOfDisassemblies == NIL)
			{
			 FailurePoint14:
				DisposeArray(Window->ListOfCalcWindows);
				goto FailurePoint13;
			}

		Window->MyGenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&MainWindowDoIdle,
			(void (*)(void*))&MainWindowBecomeActive,
			(void (*)(void*))&MainWindowBecomeInactive,
			(void (*)(void*))&MainWindowJustResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&MainWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&MainWindowDoKeyDown,
			(void (*)(void*))&MainWindowClose,
			(void (*)(void*))&MainWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&MainWindowDoMenuCommand);
		if (Window->MyGenericWindow == NIL)
			{
			 FailurePoint15:
				DisposeArray(Window->ListOfDisassemblies);
				goto FailurePoint14;
			}

		Window->MyMenuItem = MakeNewMenuItem(mmWindowMenu,"x",0);
		if (Window->MyMenuItem == NIL)
			{
			 FailurePoint16:
				CheckOutDyingWindow(Window->MyGenericWindow);
				goto FailurePoint15;
			}
		if (!RegisterWindowMenuItem(Window->MyMenuItem,(void (*)(void*))&ActivateThisWindow,
			Window->ScreenID))
			{
			 FailurePoint17:
				KillMenuItem(Window->MyMenuItem);
				goto FailurePoint16;
			}

		Window->SongPostProcessing = StringToBlockCopy("#song postprocessing\x0a");
		if (Window->SongPostProcessing == NIL)
			{
			 FailurePoint18:
				DeregisterWindowMenuItem(Window->MyMenuItem);
				goto FailurePoint17;
			}
		SetTag(Window->SongPostProcessing,"SongPostProcessing");

		Window->StereoPlayback = True;
		Window->SurroundEncoding = False;
		Window->SamplingRate = 44100;
		Window->EnvelopeUpdateRate = 441;
		Window->DefaultBeatsPerMinute = Double2LargeBCD(120);
		Window->OverallVolumeScalingFactor = Double2LargeBCD(1);
		Window->OutputNumBits = eOutput16Bits;
		Window->InterpolateOverTime = True;
		Window->InterpolateAcrossWaves = True;
		Window->StuffModified = False;
		Window->ScanningGap = Double2LargeBCD(2);
		Window->BufferDuration = Double2LargeBCD(8);
		Window->ClipWarning = True;
		Window->SongPostProcessingEnable = False;

		Window->DeleteUndoFileLocation = NIL;
		Window->DeleteUndoFile = NIL;

		/* reading the data or making an empty instrument */
		if (TheFile == NIL)
			{
				/* create a new empty one */
				Window->EverBeenSaved = False;
			}
		 else
			{
				FileType*					FileDesc;

				/* read a file in and update the file refnum and location variables */
				if (OpenFile(TheFile,&FileDesc,eReadAndWrite))
					{
						BufferedInputRec*	InputFileThang;

						InputFileThang = NewBufferedInput(FileDesc);
						if (InputFileThang == NIL)
							{
								AlertHalt("There is not enough memory available to read the file.",NIL);
								ReleasePtr(Window->SongPostProcessing);
								goto FailurePoint18;
							}
						 else
							{
								FileLoadingErrors	Error;

								SetWatchCursor();
								Error = MainWindowReadData(Window,InputFileThang);
								if (Error == eFileLoadNoError)
									{
										Error = SampleListReadData(Window->SampleList,InputFileThang);
										if (Error == eFileLoadNoError)
											{
												Error = FunctionListReadData(Window->FunctionList,
													InputFileThang);
												if (Error == eFileLoadNoError)
													{
														Error = AlgoSampListReadData(Window->AlgoSampList,
															InputFileThang);
														if (Error == eFileLoadNoError)
															{
																Error = WaveTableListReadData(Window->WaveTableList,
																	InputFileThang);
																if (Error == eFileLoadNoError)
																	{
																		Error = AlgoWaveTableListReadData(
																			Window->AlgoWaveTableList,InputFileThang);
																		if (Error == eFileLoadNoError)
																			{
																				Error = InstrListReadData(Window->InstrumentList,
																					InputFileThang);
																				if (Error == eFileLoadNoError)
																					{
																						Error = TrackListReadData(Window->TrackList,
																							InputFileThang);
																					}
																			}
																	}
															}
													}
											}
									}
								switch (Error)
									{
										default:
											EXECUTE(PRERR(ForceAbort,"OpenDocument:  bad error code"));
											break;
										case eFileLoadNoError:
											break;
										case eFileLoadBadFormat:
											AlertHalt("The file format is unrecognized.",NIL);
											break;
										case eFileLoadDiskError:
											AlertHalt("A disk error occurred and the file could not be "
												"completely loaded.",NIL);
											break;
										case eFileLoadOutOfMemory:
											AlertHalt("There is not enough memory available to completely "
												"load the file.  Try closing windows or quitting other "
												"applications to make more memory available.",NIL);
											break;
									}
							}
						EndBufferedInput(InputFileThang);
						MainWindowDeselectAllOtherStringLists(Window,NIL/*deselect all*/);
					}
				 else
					{
						AlertHalt("Unable to open file for reading.",NIL);
						ReleasePtr(Window->SongPostProcessing);
						goto FailurePoint18;
					}
				Window->EverBeenSaved = True;
				Window->TheFileLocation = TheFile;
				Window->TheFile = FileDesc;
			}

		MainWindowDispatchNameChange(Window);
	}


/* save the document into the current file.  if it hasn't been saved, then call SaveAs */
/* it returns False if it fails. */
MyBoolean						SaveDocument(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if (!Window->EverBeenSaved)
			{
				return SaveDocumentAs(Window);
			}
		 else
			{
				FileType*					TempFile;
				FileSpec*					TempFileLocation;
				BufferedOutputRec*	OutputFileThang;
				FileLoadingErrors	Error;

				/* create new file and write the data out here */
				/* create a temporary file in the same directory */
				TempFileLocation = NewTempFileInTheSameDirectory(Window->TheFileLocation);
				if (TempFileLocation == NIL)
					{
					 FailurePoint1:
						AlertHalt("Unable to write out the data.",NIL);
						return False;
					}
				/* open the file */
				if (!OpenFile(TempFileLocation,&TempFile,eReadAndWrite))
					{
					 FailurePoint2:
						DisposeFileSpec(TempFileLocation);
						goto FailurePoint1;
					}
				/* write all the data out */
				OutputFileThang = NewBufferedOutput(TempFile);
				if (OutputFileThang == NIL)
					{
					 FailurePoint3:
						CloseFile(TempFile);
						DeleteFile(TempFileLocation);
						goto FailurePoint2;
					}
				SetWatchCursor();
				Error = MainWindowWriteData(Window,OutputFileThang);
				if (Error == eFileLoadNoError)
					{
						Error = SampleListWriteData(Window->SampleList,OutputFileThang);
						if (Error == eFileLoadNoError)
							{
								Error = FunctionListWriteData(Window->FunctionList,OutputFileThang);
								if (Error == eFileLoadNoError)
									{
										Error = AlgoSampListWriteData(Window->AlgoSampList,OutputFileThang);
										if (Error == eFileLoadNoError)
											{
												Error = WaveTableListWriteData(Window->WaveTableList,
													OutputFileThang);
												if (Error == eFileLoadNoError)
													{
														Error = AlgoWaveTableListWriteData(Window->AlgoWaveTableList,
															OutputFileThang);
														if (Error == eFileLoadNoError)
															{
																Error = InstrListWriteData(Window->InstrumentList,
																	OutputFileThang);
																if (Error == eFileLoadNoError)
																	{
																		Error = TrackListWriteData(Window->TrackList,
																			OutputFileThang);
																		if (Error == eFileLoadNoError)
																			{
																				if (EndBufferedOutput(OutputFileThang))
																					{
																						Error = eFileLoadNoError;
																					}
																				 else
																					{
																						Error = eFileLoadDiskError;
																					}
																			}
																	}
															}
													}
											}
									}
							}
					}
				switch (Error)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"OpenDocument:  bad error code"));
							break;
						case eFileLoadNoError:
							break;
						case eFileLoadDiskError:
							AlertHalt("A disk error occurred and the file could not be saved.",NIL);
							goto FailurePoint3;
						case eFileLoadOutOfMemory:
							AlertHalt("There is not enough memory available to save the file.  "
								"Try closing windows or quitting other applications to make more "
								"memory available.",NIL);
							goto FailurePoint3;
					}
				FlushLocalBuffers(TempFile);
				/* swap the files on the disk */
				if (!SwapFileDataForks(TempFileLocation,Window->TheFileLocation,TempFile,
					&(Window->TheFile)))
					{
						/* if this fails, then the parameters are unaltered */
						goto FailurePoint3;
					}
				/* at this point, Window->TheFile has been fixed up, TempFile is closed, */
				/* and we need to dispose TempFileLocation */
				DisposeFileSpec(TempFileLocation);
				/* mark the data to indicate that it has been saved. */
				FunctionListMarkAllObjectsSaved(Window->FunctionList);
				SampleListMarkAllObjectsSaved(Window->SampleList);
				AlgoSampListMarkAllObjectsSaved(Window->AlgoSampList);
				WaveTableListMarkAllObjectsSaved(Window->WaveTableList);
				AlgoWaveTableListMarkAllObjectsSaved(Window->AlgoWaveTableList);
				InstrListMarkAllObjectsSaved(Window->InstrumentList);
				TrackListMarkAllObjectsSaved(Window->TrackList);
				TextEditHasBeenSaved(Window->CommentInfo);
				Window->StuffModified = False;
				/* return true since it saved correctly */
				return True;
			}
		EXECUTE(PRERR(ForceAbort,"SaveDocument:  control reached end of function"));
	}


/* close all open documents, subject to the user's ok */
void								DoCloseAllQuitPending(void)
	{
		while (ArrayGetLength(ListOfDocuments) > 0)
			{
				MainWindowRec*		Window;

				Window = (MainWindowRec*)ArrayGetElement(ListOfDocuments,0);
				ActivateThisWindow(Window->ScreenID);
				if (!CloseDocument(Window))
					{
						/* user cancelled */
						AbortQuitInProgress();
						return;
					}
				/* else keep going */
			}
	}


/* close a document.  If the user cancelled, then return False, otherwise True */
MyBoolean						CloseDocument(MainWindowRec* Window)
	{
		long				Scan;

		CheckPtrExistence(Window);
		if (HasDocumentBeenModified(Window))
			{
				/* hafta ask permission */
				YesNoCancelType		WhatToDo;
				char*							Filename;
				char*							XFilename;

				XFilename = GetCopyOfDocumentName(Window);
				if (XFilename == NIL)
					{
					 FailurePoint1:
						AlertHalt("There is not enough memory to close the document.",NIL);
						return False;
					}
				Filename = BlockToStringCopy(XFilename);
				ReleasePtr(XFilename);
				if (Filename == NIL)
					{
						goto FailurePoint1;
					}
				WhatToDo = AskYesNoCancel("Save changes to document '_'?",
					Filename,"Save","Don't Save","Cancel");
				ReleasePtr(Filename);
				if (WhatToDo == eYes)
					{
						if (!SaveDocument(Window))
							{
								/* save was cancelled or failed */
								return False;
							}
					}
				else if (WhatToDo == eCancel)
					{
						return False; /* user cancelled */
					}
				/* fall through to deletion point */
			}

		if (Window->DeleteUndoFileLocation != NIL)
			{
				CloseFile(Window->DeleteUndoFile);
				DeleteFile(Window->DeleteUndoFileLocation);
				DisposeFileSpec(Window->DeleteUndoFileLocation);
			}

		/* perform the data deletion here */
		DisposeFunctionList(Window->FunctionList);
		DisposeSampleList(Window->SampleList);
		DisposeAlgoSampList(Window->AlgoSampList);
		DisposeWaveTableList(Window->WaveTableList);
		DisposeAlgoWaveTableList(Window->AlgoWaveTableList);
		DisposeInstrList(Window->InstrumentList);
		DisposeTrackList(Window->TrackList);

		DisposeTextEdit(Window->CommentInfo);

		for (Scan = 0; Scan < ArrayGetLength(Window->ListOfCalcWindows); Scan += 1)
			{
				DisposeCalculatorWindow((CalcWindowRec*)ArrayGetElement(
					Window->ListOfCalcWindows,Scan));
			}
		DisposeArray(Window->ListOfCalcWindows);

		for (Scan = 0; Scan < ArrayGetLength(Window->ListOfDisassemblies); Scan += 1)
			{
				DisposeDisassemblyWindow((DisaWindowRec*)ArrayGetElement(
					Window->ListOfDisassemblies,Scan));
			}
		DisposeArray(Window->ListOfDisassemblies);

		/* delete object code */
		DisposeCodeCenter(Window->CodeCenter);

		ReleasePtr(Window->SongPostProcessing);

		DeregisterWindowMenuItem(Window->MyMenuItem);
		KillMenuItem(Window->MyMenuItem);
		CheckOutDyingWindow(Window->MyGenericWindow);
		KillWindow(Window->ScreenID);
		if (Window->EverBeenSaved)
			{
				CloseFile(Window->TheFile);
				DisposeFileSpec(Window->TheFileLocation);
			}
		ERROR(ArrayFindElement(ListOfDocuments,Window) < 0,PRERR(ForceAbort,
			"CloseDocument:  couldn't find document in the list"));
		ArrayDeleteElement(ListOfDocuments,ArrayFindElement(ListOfDocuments,Window));
		ReleasePtr((char*)Window);
		return True;
	}


/* save the document into a new file (don't disturb the current one).  returns False */
/* if it fails.  this function calls SaveDocument() to do the work. */
MyBoolean						SaveDocumentAs(MainWindowRec* Window)
	{
		FileSpec*					NewWhere;
		char*							FilenameNull;
		char*							StringTempFileNameThing;

		CheckPtrExistence(Window);
		StringTempFileNameThing = GetCopyOfDocumentName(Window);
		if (StringTempFileNameThing == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory to save the document.",NIL);
				return False;
			}
		FilenameNull = BlockToStringCopy(StringTempFileNameThing);
		ReleasePtr(StringTempFileNameThing);
		if (FilenameNull == NIL)
			{
				goto FailurePoint1;
			}
		/* where do they want to save it */
		NewWhere = PutFile(FilenameNull);
		ReleasePtr(FilenameNull);
		if (NewWhere != NIL)
			{
				FileType*					NewFile;
				char*							XFilename;
				MyBoolean					SaveWasSuccessful;

				XFilename = ExtractFileName(NewWhere);
				if (XFilename == NIL)
					{
						goto FailurePoint1;
					}
				FilenameNull = BlockToStringCopy(XFilename);
				ReleasePtr(XFilename);
				if (FilenameNull == NIL)
					{
						goto FailurePoint1;
					}
				if (!CreateFile(NewWhere,ApplicationCreator,ApplicationFileType))
					{
						AlertHalt("Couldn't create the file '_'.",FilenameNull);
						ReleasePtr(FilenameNull);
						return False;
					}
				if (!OpenFile(NewWhere,&NewFile,eReadAndWrite))
					{
						AlertHalt("Couldn't open the file '_'.",FilenameNull);
						ReleasePtr(FilenameNull);
						return False;
					}
				if (Window->EverBeenSaved)
					{
						CloseFile(Window->TheFile);
						DisposeFileSpec(Window->TheFileLocation);
						Window->EverBeenSaved = False;
					}
				Window->EverBeenSaved = True;
				Window->TheFileLocation = NewWhere;
				Window->TheFile = NewFile;
				ReleasePtr(FilenameNull);
				SaveWasSuccessful = SaveDocument(Window);
				if (!SaveWasSuccessful)
					{
						/* if save wasn't successful, then get rid of the file stuff */
						CloseFile(Window->TheFile);
						DeleteFile(Window->TheFileLocation);
						DisposeFileSpec(Window->TheFileLocation);
						Window->EverBeenSaved = False;
					}
				MainWindowDispatchNameChange(Window);
				return SaveWasSuccessful;
			}
		 else
			{
				return False;
			}
		EXECUTE(PRERR(ForceAbort,"SaveDocumentAs:  control reached end of function"));
	}


/* return True if the document has been modified & should be saved. */
MyBoolean						HasDocumentBeenModified(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->StuffModified
			|| TextEditDoesItNeedToBeSaved(Window->CommentInfo)
			|| DoesFunctionListNeedToBeSaved(Window->FunctionList)
			|| DoesSampleListNeedToBeSaved(Window->SampleList)
			|| DoesAlgoSampListNeedToBeSaved(Window->AlgoSampList)
			|| DoesWaveTableListNeedToBeSaved(Window->WaveTableList)
			|| DoesAlgoWaveTableListNeedToBeSaved(Window->AlgoWaveTableList)
			|| DoesInstrListNeedToBeSaved(Window->InstrumentList)
			|| DoesTrackListNeedToBeSaved(Window->TrackList);
	}


void								MainWindowDoIdle(MainWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->CommentInfo);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->CommentInfo,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								MainWindowBecomeActive(MainWindowRec* Window)
	{
		OrdType			XSize;
		OrdType			YSize;

		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->CommentInfo);
		FunctionListBecomeActive(Window->FunctionList);
		SampleListBecomeActive(Window->SampleList);
		AlgoSampListBecomeActive(Window->AlgoSampList);
		WaveTableListBecomeActive(Window->WaveTableList);
		AlgoWaveTableListBecomeActive(Window->AlgoWaveTableList);
		InstrListBecomeActive(Window->InstrumentList);
		TrackListBecomeActive(Window->TrackList);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(True/*enablegrowicon*/));
	}


void								MainWindowBecomeInactive(MainWindowRec* Window)
	{
		OrdType			XSize;
		OrdType			YSize;

		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->CommentInfo);
		FunctionListBecomeInactive(Window->FunctionList);
		SampleListBecomeInactive(Window->SampleList);
		AlgoSampListBecomeInactive(Window->AlgoSampList);
		WaveTableListBecomeInactive(Window->WaveTableList);
		AlgoWaveTableListBecomeInactive(Window->AlgoWaveTableList);
		InstrListBecomeInactive(Window->InstrumentList);
		TrackListBecomeInactive(Window->TrackList);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,GetGrowIcon(False/*disablegrowicon*/));
	}


void								MainWindowJustResized(MainWindowRec* Window)
	{
		OrdType			XSize;
		OrdType			YSize;

		CheckPtrExistence(Window);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,0,0,XSize,YSize);
		DrawBoxErase(Window->ScreenID,0,0,XSize,YSize);
		SetTextEditPosition(Window->CommentInfo,
			COMMENTX(XSize,YSize),COMMENTY(XSize,YSize),
			COMMENTWIDTH(XSize,YSize),COMMENTHEIGHT(XSize,YSize));
		SetFunctionListLocation(Window->FunctionList,
			FUNCTIONX(XSize,YSize),FUNCTIONY(XSize,YSize),
			FUNCTIONWIDTH(XSize,YSize),FUNCTIONHEIGHT(XSize,YSize));
		SetSampleListLocation(Window->SampleList,
			SAMPLEX(XSize,YSize),SAMPLEY(XSize,YSize),
			SAMPLEWIDTH(XSize,YSize),SAMPLEHEIGHT(XSize,YSize));
		SetAlgoSampListLocation(Window->AlgoSampList,
			ALGOSAMPLEX(XSize,YSize),ALGOSAMPLEY(XSize,YSize),
			ALGOSAMPLEWIDTH(XSize,YSize),ALGOSAMPLEHEIGHT(XSize,YSize));
		SetWaveTableListLocation(Window->WaveTableList,
			WAVETABLEX(XSize,YSize),WAVETABLEY(XSize,YSize),WAVETABLEWIDTH(XSize,YSize),
			WAVETABLEHEIGHT(XSize,YSize));
		SetAlgoWaveTableListLocation(Window->AlgoWaveTableList,
			ALGOWAVETABLEX(XSize,YSize),ALGOWAVETABLEY(XSize,YSize),
			ALGOWAVETABLEWIDTH(XSize,YSize),ALGOWAVETABLEHEIGHT(XSize,YSize));
		SetInstrListLocation(Window->InstrumentList,INSTRUMENTX(XSize,YSize),
			INSTRUMENTY(XSize,YSize),INSTRUMENTWIDTH(XSize,YSize),INSTRUMENTHEIGHT(XSize,YSize));
		SetTrackListLocation(Window->TrackList,TRACKX(XSize,YSize),TRACKY(XSize,YSize),
			TRACKWIDTH(XSize,YSize),TRACKHEIGHT(XSize,YSize));
	}


void								MainWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if ((XLoc >= GetWindowWidth(Window->ScreenID) - 15)
			&& (XLoc < GetWindowWidth(Window->ScreenID))
			&& (YLoc >= GetWindowHeight(Window->ScreenID) - 15)
			&& (YLoc < GetWindowHeight(Window->ScreenID)))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				MainWindowJustResized(Window);
			}
		else if (TextEditHitTest(Window->CommentInfo,XLoc,YLoc))
			{
				TextEditDoMouseDown(Window->CommentInfo,XLoc,YLoc,Modifiers);
			}
		else if (FunctionListHitTest(Window->FunctionList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->FunctionList);
				FunctionListDoMouseDown(Window->FunctionList,XLoc,YLoc,Modifiers);
			}
		else if (SampleListHitTest(Window->SampleList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->SampleList);
				SampleListDoMouseDown(Window->SampleList,XLoc,YLoc,Modifiers);
			}
		else if (AlgoSampListHitTest(Window->AlgoSampList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->AlgoSampList);
				AlgoSampListDoMouseDown(Window->AlgoSampList,XLoc,YLoc,Modifiers);
			}
		else if (WaveTableListHitTest(Window->WaveTableList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->WaveTableList);
				WaveTableListDoMouseDown(Window->WaveTableList,XLoc,YLoc,Modifiers);
			}
		else if (AlgoWaveTableListHitTest(Window->AlgoWaveTableList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->AlgoWaveTableList);
				AlgoWaveTableListDoMouseDown(Window->AlgoWaveTableList,XLoc,YLoc,Modifiers);
			}
		else if (InstrListHitTest(Window->InstrumentList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->InstrumentList);
				InstrListDoMouseDown(Window->InstrumentList,XLoc,YLoc,Modifiers);
			}
		else if (TrackListHitTest(Window->TrackList,XLoc,YLoc))
			{
				MainWindowDeselectAllOtherStringLists(Window,Window->TrackList);
				TrackListDoMouseDown(Window->TrackList,XLoc,YLoc,Modifiers);
			}
	}


void								MainWindowDoKeyDown(unsigned char KeyCode, ModifierFlags Modifiers,
											MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditDoKeyPressed(Window->CommentInfo,KeyCode,Modifiers);
	}


void								MainWindowClose(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		CloseDocument(Window);
	}


void								MainWindowUpdator(MainWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		FunctionListRedraw(Window->FunctionList);
		SampleListRedraw(Window->SampleList);
		AlgoSampListRedraw(Window->AlgoSampList);
		WaveTableListRedraw(Window->WaveTableList);
		AlgoWaveTableListRedraw(Window->AlgoWaveTableList);
		InstrListRedraw(Window->InstrumentList);
		TrackListRedraw(Window->TrackList);
		TextEditFullRedraw(Window->CommentInfo);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize-15,YSize-15,
			GetGrowIcon(Window->MyGenericWindow == GetCurrentWindowID()));
	}


/* enable global menu items.  these are menu items that can be chosen regardless */
/* of which editor is open (such as New Object, Save...) */
void								MainWindowMenuSetup(MainWindowRec* Window)
	{
		MainWindowEnableGlobalMenus(Window);
		EnableMenuItem(mPaste);
		if (TextEditIsThereValidSelection(Window->CommentInfo))
			{
				EnableMenuItem(mCut);
				EnableMenuItem(mCopy);
				EnableMenuItem(mClear);
			}
		EnableMenuItem(mSelectAll);
		if (TextEditCanWeUndo(Window->CommentInfo))
			{
				EnableMenuItem(mUndo);
				ChangeItemName(mUndo,"Undo Text Edit");
			}
		if (Window->DeleteUndoFileLocation != NIL)
			{
				EnableMenuItem(mUndo);
				ChangeItemName(mUndo,"Undo Object Deletion");
			}
		ChangeItemName(mCloseFile,"Close Document");
		EnableMenuItem(mCloseFile);

		EnableMenuItem(mPasteObject);
		EnableMenuItem(mCopyObject);
		EnableMenuItem(mOpenObject);
		EnableMenuItem(mDeleteObject);
		if (FunctionListIsThereSelection(Window->FunctionList))
			{
				ChangeItemName(mOpenObject,"Edit Function Module");
				ChangeItemName(mDeleteObject,"Delete Function Module");
				ChangeItemName(mCopyObject,"Copy Function Module");
			}
		else if (SampleListIsThereSelection(Window->SampleList))
			{
				ChangeItemName(mOpenObject,"Edit Sample");
				ChangeItemName(mDeleteObject,"Delete Sample");
				ChangeItemName(mCopyObject,"Copy Sample");
			}
		else if (AlgoSampListIsThereSelection(Window->AlgoSampList))
			{
				ChangeItemName(mOpenObject,"Edit Algorithmic Sample");
				ChangeItemName(mDeleteObject,"Delete Algorithmic Sample");
				ChangeItemName(mCopyObject,"Copy Algorithmic Sample");
			}
		else if (WaveTableListIsThereSelection(Window->WaveTableList))
			{
				ChangeItemName(mOpenObject,"Edit Wave Table");
				ChangeItemName(mDeleteObject,"Delete Wave Table");
				ChangeItemName(mCopyObject,"Copy Wave Table");
			}
		else if (AlgoWaveTableListIsThereSelection(Window->AlgoWaveTableList))
			{
				ChangeItemName(mOpenObject,"Edit Algorithmic Wave Table");
				ChangeItemName(mDeleteObject,"Delete Algorithmic Wave Table");
				ChangeItemName(mCopyObject,"Copy Algorithmic Wave Table");
			}
		else if (InstrListIsThereSelection(Window->InstrumentList))
			{
				ChangeItemName(mOpenObject,"Edit Instrument");
				ChangeItemName(mDeleteObject,"Delete Instrument");
				ChangeItemName(mCopyObject,"Copy Instrument");
			}
		else if (TrackListIsThereSelection(Window->TrackList))
			{
				ChangeItemName(mOpenObject,"Edit Track");
				ChangeItemName(mDeleteObject,"Delete Track");
				ChangeItemName(mCopyObject,"Copy Track");
			}
		else
			{
				DisableMenuItem(mOpenObject);
				DisableMenuItem(mDeleteObject);
				DisableMenuItem(mCopyObject);
			}
		SetItemCheckmark(Window->MyMenuItem);
	}


void								MainWindowDoMenuCommand(MainWindowRec* Window,
											MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (MainWindowDoGlobalMenuItem(Window,MenuItem))
			{
			}
		else if (MenuItem == mPaste)
			{
				TextEditDoMenuPaste(Window->CommentInfo);
			}
		else if (MenuItem == mCut)
			{
				TextEditDoMenuCut(Window->CommentInfo);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->CommentInfo);
			}
		else if (MenuItem == mClear)
			{
				TextEditDoMenuClear(Window->CommentInfo);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->CommentInfo);
			}
		else if (MenuItem == mUndo)
			{
				/* object deletion undo takes precedence */
				if (Window->DeleteUndoFileLocation != NIL)
					{
						MyBoolean					DidIt = True;

						SetWatchCursor();
						if (!FunctionListPasteFromFile(Window->FunctionList,
							Window->DeleteUndoFile))
							{
								if (!SampleListPasteFromFile(Window->SampleList,
									Window->DeleteUndoFile))
									{
										if (!AlgoSampListPasteFromFile(Window->AlgoSampList,
											Window->DeleteUndoFile))
											{
												if (!WaveTableListPasteFromFile(Window->WaveTableList,
													Window->DeleteUndoFile))
													{
														if (!AlgoWaveTableListPasteFromFile(Window->AlgoWaveTableList,
															Window->DeleteUndoFile))
															{
																if (!InstrListPasteFromFile(Window->InstrumentList,
																	Window->DeleteUndoFile))
																	{
																		if (!TrackListPasteFromFile(Window->TrackList,
																			Window->DeleteUndoFile))
																			{
																				/* couldn't do it, maybe out of memory */
																				DidIt = False;
																			}
																	}
															}
													}
											}
									}
							}
						if (DidIt)
							{
								CloseFile(Window->DeleteUndoFile);
								DeleteFile(Window->DeleteUndoFileLocation);
								DisposeFileSpec(Window->DeleteUndoFileLocation);
								Window->DeleteUndoFileLocation = NIL;
								Window->DeleteUndoFile = NIL;
							}
					}
				 else
					{
						TextEditDoMenuUndo(Window->CommentInfo);
					}
			}
		else if (MenuItem == mCloseFile)
			{
				CloseDocument(Window);
			}
		else if (MenuItem == mOpenObject)
			{
				if (FunctionListIsThereSelection(Window->FunctionList))
					{
						FunctionListOpenSelection(Window->FunctionList);
					}
				else if (SampleListIsThereSelection(Window->SampleList))
					{
						SampleListOpenSelection(Window->SampleList);
					}
				else if (AlgoSampListIsThereSelection(Window->AlgoSampList))
					{
						AlgoSampListOpenSelection(Window->AlgoSampList);
					}
				else if (WaveTableListIsThereSelection(Window->WaveTableList))
					{
						WaveTableListOpenSelection(Window->WaveTableList);
					}
				else if (AlgoWaveTableListIsThereSelection(Window->AlgoWaveTableList))
					{
						AlgoWaveTableListOpenSelection(Window->AlgoWaveTableList);
					}
				else if (InstrListIsThereSelection(Window->InstrumentList))
					{
						InstrListOpenSelection(Window->InstrumentList);
					}
				else if (TrackListIsThereSelection(Window->TrackList))
					{
						TrackListOpenSelection(Window->TrackList);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,
							"MainWindowDoMenuCommand(mOpenObject):  no known selection"));
					}
			}
		else if (MenuItem == mDeleteObject)
			{
				if (FunctionListIsThereSelection(Window->FunctionList))
					{
						FunctionListDeleteSelection(Window->FunctionList);
					}
				else if (SampleListIsThereSelection(Window->SampleList))
					{
						SampleListDeleteSelection(Window->SampleList);
					}
				else if (AlgoSampListIsThereSelection(Window->AlgoSampList))
					{
						AlgoSampListDeleteSelection(Window->AlgoSampList);
					}
				else if (WaveTableListIsThereSelection(Window->WaveTableList))
					{
						WaveTableListDeleteSelection(Window->WaveTableList);
					}
				else if (AlgoWaveTableListIsThereSelection(Window->AlgoWaveTableList))
					{
						AlgoWaveTableListDeleteSelection(Window->AlgoWaveTableList);
					}
				else if (InstrListIsThereSelection(Window->InstrumentList))
					{
						InstrListDeleteSelection(Window->InstrumentList);
					}
				else if (TrackListIsThereSelection(Window->TrackList))
					{
						TrackListDeleteSelection(Window->TrackList);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,
							"MainWindowDoMenuCommand(mDeleteObject):  no known selection"));
					}
			}
		else if (MenuItem == mPasteObject)
			{
				if (!FunctionListPasteObject(Window->FunctionList))
					{
						if (!SampleListPasteObject(Window->SampleList))
							{
								if (!AlgoSampListPasteObject(Window->AlgoSampList))
									{
										if (!WaveTableListPasteObject(Window->WaveTableList))
											{
												if (!AlgoWaveTableListPasteObject(Window->AlgoWaveTableList))
													{
														if (!InstrListPasteObject(Window->InstrumentList))
															{
																if (!TrackListPasteObject(Window->TrackList))
																	{
																		/* oh well, it must not be an object */
																	}
															}
													}
											}
									}
							}
					}
			}
		else if (MenuItem == mCopyObject)
			{
				if (FunctionListIsThereSelection(Window->FunctionList))
					{
						FunctionListCopyObject(Window->FunctionList);
					}
				else if (SampleListIsThereSelection(Window->SampleList))
					{
						SampleListCopyObject(Window->SampleList);
					}
				else if (AlgoSampListIsThereSelection(Window->AlgoSampList))
					{
						AlgoSampListCopyObject(Window->AlgoSampList);
					}
				else if (WaveTableListIsThereSelection(Window->WaveTableList))
					{
						WaveTableListCopyObject(Window->WaveTableList);
					}
				else if (AlgoWaveTableListIsThereSelection(Window->AlgoWaveTableList))
					{
						AlgoWaveTableListCopyObject(Window->AlgoWaveTableList);
					}
				else if (InstrListIsThereSelection(Window->InstrumentList))
					{
						InstrListCopyObject(Window->InstrumentList);
					}
				else if (TrackListIsThereSelection(Window->TrackList))
					{
						TrackListCopyObject(Window->TrackList);
					}
				else
					{
						EXECUTE(PRERR(AllowResume,
							"MainWindowDoMenuCommand(mCopyObject):  no known selection"));
					}
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"MainWindowDoMenuCommand:  unknown menu command"));
			}
	}


/* deselect any selection in a scrolling list other than the specified list. */
/* specified list may be NIL for unconditional deselect */
void								MainWindowDeselectAllOtherStringLists(MainWindowRec* Window,
											void* TheDontDeselectStringList)
	{
		CheckPtrExistence(Window);
		if (TheDontDeselectStringList != Window->FunctionList)
			{
				FunctionListDeselect(Window->FunctionList);
			}
		if (TheDontDeselectStringList != Window->SampleList)
			{
				SampleListDeselect(Window->SampleList);
			}
		if (TheDontDeselectStringList != Window->AlgoSampList)
			{
				AlgoSampListDeselect(Window->AlgoSampList);
			}
		if (TheDontDeselectStringList != Window->WaveTableList)
			{
				WaveTableListDeselect(Window->WaveTableList);
			}
		if (TheDontDeselectStringList != Window->AlgoWaveTableList)
			{
				AlgoWaveTableListDeselect(Window->AlgoWaveTableList);
			}
		if (TheDontDeselectStringList != Window->InstrumentList)
			{
				InstrListDeselect(Window->InstrumentList);
			}
		if (TheDontDeselectStringList != Window->TrackList)
			{
				TrackListDeselect(Window->TrackList);
			}
	}


/* create a new calculator window.  the main window keeps track of all calculator */
/* objects that it has created */
void								MainWindowNewCalculator(MainWindowRec* Window)
	{
		CalcWindowRec*		Calc;

		CheckPtrExistence(Window);
		Calc = NewCalculatorWindow(Window,Window->CodeCenter);
		if (Calc == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to "
					"open a calculator window.",NIL);
				return;
			}
		if (!ArrayAppendElement(Window->ListOfCalcWindows,Calc))
			{
			 FailurePoint2:
				DisposeCalculatorWindow(Calc);
				goto FailurePoint1;
			}
	}


/* when a calculator window closes, it calls this to make sure the main window */
/* object knows that it no longer exists */
void								MainWindowCalculatorClosingNotify(MainWindowRec* Window,
											CalcWindowRec* Calc)
	{
		CheckPtrExistence(Window);
		ERROR(ArrayFindElement(Window->ListOfCalcWindows,Calc) < 0,PRERR(ForceAbort,
			"MainWindowCalculatorClosingNotify:  unknown calculator window"));
		ArrayDeleteElement(Window->ListOfCalcWindows,
			ArrayFindElement(Window->ListOfCalcWindows,Calc));
	}


/* notify the main window that a new disassembly window has been created. */
MyBoolean						MainWindowNewDisassemblyNotify(MainWindowRec* Window,
											struct DisaWindowRec* DisassemblyWindow)
	{
		CheckPtrExistence(Window);
		if (!ArrayAppendElement(Window->ListOfDisassemblies,DisassemblyWindow))
			{
				return False;
			}
		return True;
	}


/* notify the main window that a disassembly window has been destroyed. */
void								MainWindowDisassemblyClosingNotify(MainWindowRec* Window,
											struct DisaWindowRec* DisassemblyWindow)
	{
		CheckPtrExistence(Window);
		ERROR(ArrayFindElement(Window->ListOfDisassemblies,DisassemblyWindow) < 0,
			PRERR(ForceAbort,"MainWindowDisassemblyClosingNotify:  unknown disassembly"));
		ArrayDeleteElement(Window->ListOfDisassemblies,
			ArrayFindElement(Window->ListOfDisassemblies,DisassemblyWindow));
	}


/* get the number of spaces per tab that editors should use */
long								MainWindowGetTabSize(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->TabSize;
	}


/* build any function objects that need to be built.  returns True if all of them */
/* were built without a problem. */
MyBoolean						MainWindowMakeUpToDateFunctions(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return FunctionListMakeUpToDate(Window->FunctionList);
	}


/* build any algorithmic samples that need to be built.  returns True if all of them */
/* were built without a problem. */
MyBoolean						MainWindowMakeUpToDateAlgoSamps(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return AlgoSampListMakeUpToDate(Window->AlgoSampList);
	}


/* build any algorithmic wave tables that need to be built.  returns True if all of */
/* them were built without a problem. */
MyBoolean						MainWindowMakeUpToDateAlgoWaveTables(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return AlgoWaveTableListMakeUpToDate(Window->AlgoWaveTableList);
	}


/* build any instrument specifications that need to be built.  returns True if all of */
/* them were built without a problem. */
MyBoolean						MainWindowMakeUpToDateInstrList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return InstrListMakeUpToDate(Window->InstrumentList);
	}


/* build everything.  returns True if everything built correctly */
MyBoolean						MainWindowMakeEverythingUpToDate(MainWindowRec* Window)
	{
		MyBoolean					ReturnValue;

		CheckPtrExistence(Window);
		ReturnValue = False;
		SetWatchCursor();
		if (MainWindowMakeUpToDateFunctions(Window))
			{
				if (MainWindowMakeUpToDateAlgoSamps(Window))
					{
						if (MainWindowMakeUpToDateAlgoWaveTables(Window))
							{
								if (MainWindowMakeUpToDateInstrList(Window))
									{
										ReturnValue = True;
									}
							}
					}
			}
		return ReturnValue;
	}


void								MainWindowEnableGlobalMenus(MainWindowRec* Window)
	{
		char*							StrNumber;

		CheckPtrExistence(Window);
		EnableMenuItem(mSaveAs);
		if (HasDocumentBeenModified(Window) || !Window->EverBeenSaved)
			{
				EnableMenuItem(mSaveFile);
			}
		EnableMenuItem(mNewFunction);
		EnableMenuItem(mNewSample);
		EnableMenuItem(mNewAlgoSample);
		EnableMenuItem(mNewWaveTable);
		EnableMenuItem(mNewAlgoWaveTable);
		EnableMenuItem(mNewInstrument);
		EnableMenuItem(mNewTrack);
		EnableMenuItem(mUnbuildAllFunctions);
		EnableMenuItem(mBuildEntireProject);
		EnableMenuItem(mCalculator);
		EnableMenuItem(mSetTabSize);
		EnableMenuItem(mPlay);

		StrNumber = IntegerToString(Window->TabSize);
		if (StrNumber != NIL)
			{
				char*							StrKey;

				StrKey = StringToBlockCopy("_");
				if (StrKey != NIL)
					{
						char*							StrValue;

						StrValue = StringToBlockCopy("Set Tab Size... (_)");
						if (StrValue != NIL)
							{
								char*							StrResult;

								StrResult = ReplaceBlockCopy(StrValue,StrKey,StrNumber);
								if (StrResult != NIL)
									{
										char*							Temp;

										Temp = BlockToStringCopy(StrResult);
										if (Temp != NIL)
											{
												ReleasePtr(StrResult);
												StrResult = Temp;
												ChangeItemName(mSetTabSize,StrResult);
											}
										ReleasePtr(StrResult);
									}
								ReleasePtr(StrValue);
							}
						ReleasePtr(StrKey);
					}
				ReleasePtr(StrNumber);
			}

		EnableMenuItem(mImportWAVFormat);
		EnableMenuItem(mImportRAWFormat);
		EnableMenuItem(mImportAIFFFormat);

		WindowMenuEnableItems();
	}


/* this checks to see if the menu item is a global menu item.  if it is, the */
/* associated action is performed and it returns True.  if not, then it returns */
/* False and the specific editor window must handle the menu item. */
MyBoolean						MainWindowDoGlobalMenuItem(MainWindowRec* Window,
											MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (MenuItem == mSaveAs)
			{
				SaveDocumentAs(Window);
			}
		else if (MenuItem == mSaveFile)
			{
				SaveDocument(Window);
			}
		else if (MenuItem == mNewFunction)
			{
				FunctionListNewModule(Window->FunctionList);
			}
		else if (MenuItem == mNewSample)
			{
				SampleListNewSample(Window->SampleList);
			}
		else if (MenuItem == mNewAlgoSample)
			{
				AlgoSampListNewAlgoSamp(Window->AlgoSampList);
			}
		else if (MenuItem == mNewWaveTable)
			{
				WaveTableListNewWaveTable(Window->WaveTableList);
			}
		else if (MenuItem == mNewAlgoWaveTable)
			{
				AlgoWaveTableListNewAlgoWaveTable(Window->AlgoWaveTableList);
			}
		else if (MenuItem == mNewInstrument)
			{
				InstrListNewInstr(Window->InstrumentList);
			}
		else if (MenuItem == mNewTrack)
			{
				TrackListNewTrack(Window->TrackList);
			}
		else if (MenuItem == mUnbuildAllFunctions)
			{
				FunctionListUnbuildAll(Window->FunctionList);
				AlgoSampListUnbuildAll(Window->AlgoSampList);
				AlgoWaveTableListUnbuildAll(Window->AlgoWaveTableList);
				InstrListUnbuildAll(Window->InstrumentList);
			}
		else if (MenuItem == mBuildEntireProject)
			{
				MainWindowMakeEverythingUpToDate(Window);
			}
		else if (MenuItem == mCalculator)
			{
				MainWindowNewCalculator(Window);
			}
		else if (MenuItem == mSetTabSize)
			{
				Window->TabSize = DoNumberDialog("Enter new tab size:",Window->TabSize,mCut,
					mPaste,mCopy,mUndo,mSelectAll,mClear);
				if (Window->TabSize < MINTABCOUNT)
					{
						Window->TabSize = MINTABCOUNT;
					}
				if (Window->TabSize > MAXTABCOUNT)
					{
						Window->TabSize = MAXTABCOUNT;
					}
				Window->StuffModified = True;
				/* we should probably have a dispatch that changes the tab size in */
				/* all objects */
			}
		else if (MenuItem == mPlay)
			{
				DoPlayPrefsDialog(Window,Window->TrackList);
			}
		else if (MenuItem == mImportWAVFormat)
			{
				ImportWAVSample(Window);
			}
		else if (MenuItem == mImportRAWFormat)
			{
				ImportRAWSample(Window);
			}
		else if (MenuItem == mImportAIFFFormat)
			{
				ImportAIFFSample(Window);
			}
		else if (DispatchWindowMenuItem(MenuItem))
			{
				/* no action, but fall through and return True */
			}
		else
			{
				return False;
			}
		return True;
	}


/* get a copy of the left-channel array from a stereo sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleLeftCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = SampleListGetSampleLeftFixed(((MainWindowRec*)Window)->SampleList,
			NameCopy,DataOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* get a copy of the right-channel array from a stereo sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleRightCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = SampleListGetSampleRightFixed(((MainWindowRec*)Window)->SampleList,
			NameCopy,DataOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* get a copy of the sample array from a mono sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleMonoCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = SampleListGetSampleMonoFixed(((MainWindowRec*)Window)->SampleList,
			NameCopy,DataOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* get the number of frames per wave period for the specified wave table.  an error */
/* code is returned indicating success or failure.  the caller is responsible for */
/* disposing of the name string. */
SampleErrors				MainWindowGetWaveTableFrameCount(void* Window,
											char* NullTerminatedName, long* FrameCountOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = WaveTableListGetWaveTableFrameCount(
			((MainWindowRec*)Window)->WaveTableList,NameCopy,FrameCountOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* get the number of tables in the specified wave table.  an error */
/* code is returned indicating success or failure.  the caller is responsible for */
/* disposing of the name string. */
SampleErrors				MainWindowGetWaveTableTableCount(void* Window,
											char* NullTerminatedName, long* TableCountOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = WaveTableListGetWaveTableTableCount(
			((MainWindowRec*)Window)->WaveTableList,NameCopy,TableCountOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* get a copy of the sample array from a wave table.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetWaveTableArray(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut)
	{
		char*							NameCopy;
		SampleErrors			ReturnValue;

		CheckPtrExistence(Window);
		NameCopy = StringToBlockCopy(NullTerminatedName);
		if (NameCopy == NIL)
			{
				return eEvalSampleNotEnoughMemoryToCopy;
			}
		ReturnValue = WaveTableListGetWaveTableArray(
			((MainWindowRec*)Window)->WaveTableList,NameCopy,DataOut);
		ReleasePtr(NameCopy);
		return ReturnValue;
	}


/* open a new sample editor initialized with the parameters and install the data */
/* in the array RawData into it.  this array is NOT largefixedsigned, but rather is */
/* signed char or signed short, depending on the setting of NumBits.  the caller */
/* is responsible for disposing of RawData. */
struct SampleObjectRec*	MainWindowCopyRawSampleAndOpen(MainWindowRec* Window,
											char* RawData, NumBitsType NumBits, NumChannelsType NumChannels,
											long Origin, long LoopStart1, long LoopStart2, long LoopStart3,
											long LoopEnd1, long LoopEnd2, long LoopEnd3, long SamplingRate,
											double NaturalFrequency)
	{
		CheckPtrExistence(Window);
		if (RawData != NIL)
			{
				CheckPtrExistence(RawData);
				return SampleListCopyRawSampleAndOpen(Window->SampleList,RawData,NumBits,
					NumChannels,Origin,LoopStart1,LoopStart2,LoopStart3,LoopEnd1,LoopEnd2,
					LoopEnd3,SamplingRate,NaturalFrequency);
			}
		 else
			{
				return NIL;
			}
	}


/* open a new wave table editor initialized with the parameters and install the data */
/* in the array RawData into it.  this array is not largefixedsigned, but rather is */
/* signed char or signed short, depending on the setting of NumBits.  the caller */
/* is responsible for disposing of RawData. */
struct WaveTableObjectRec*	MainWindowCopyRawWaveTableAndOpen(MainWindowRec* Window,
											char* RawData, NumBitsType NumBits, long NumTables,
											long FramesPerTable)
	{
		CheckPtrExistence(Window);
		if (RawData != NIL)
			{
				CheckPtrExistence(RawData);
				return WaveTableListCopyRawWaveTableAndOpen(Window->WaveTableList,RawData,
					NumBits,NumTables,FramesPerTable);
			}
		 else
			{
				return NIL;
			}
	}


/* get a copy of the name of the current document file.  the name is a heap-allocated */
/* non-null-terminated block. */
char*								GetCopyOfDocumentName(MainWindowRec* Window)
	{
		char*							Filename;

		if (Window->EverBeenSaved)
			{
				Filename = ExtractFileName(Window->TheFileLocation);
			}
		 else
			{
				Filename = StringToBlockCopy("Untitled");
			}
		if (Filename != NIL)
			{
				SetTag(Filename,"GetCopyOfDocumentName");
			}
		return Filename;
	}


/* dispatch a name change event.  this tells all editors that the document title */
/* has changed, and the window titles should be updated accordingly. */
void								MainWindowDispatchNameChange(MainWindowRec* Window)
	{
		char*							Filename;

		CheckPtrExistence(Window);
		Filename = GetCopyOfDocumentName(Window);
		if (Filename != NIL)
			{
				char*							FilenameNullTerminated;

				SampleListGlobalNameChange(Window->SampleList,Filename);
				FunctionListGlobalNameChange(Window->FunctionList,Filename);
				AlgoSampListGlobalNameChange(Window->AlgoSampList,Filename);
				WaveTableListGlobalNameChange(Window->WaveTableList,Filename);
				AlgoWaveTableListGlobalNameChange(Window->AlgoWaveTableList,Filename);
				InstrListGlobalNameChange(Window->InstrumentList,Filename);
				TrackListGlobalNameChange(Window->TrackList,Filename);
				FilenameNullTerminated = BlockToStringCopy(Filename);
				if (FilenameNullTerminated != NIL)
					{
						SetWindowName(Window->ScreenID,FilenameNullTerminated);
						ChangeItemName(Window->MyMenuItem,FilenameNullTerminated);
						ReleasePtr(FilenameNullTerminated);
					}
				ReleasePtr(Filename);
			}
	}


MyBoolean						MainWindowGetStereo(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->StereoPlayback;
	}


MyBoolean						MainWindowGetSurround(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->SurroundEncoding;
	}


long								MainWindowGetSamplingRate(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->SamplingRate;
	}


long								MainWindowGetEnvelopeRate(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->EnvelopeUpdateRate;
	}


double							MainWindowGetBeatsPerMinute(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return LargeBCD2Double(Window->DefaultBeatsPerMinute);
	}


double							MainWindowGetVolumeScaling(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return LargeBCD2Double(Window->OverallVolumeScalingFactor);
	}


OutputNumBitsType		MainWindowGetOutputNumBits(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->OutputNumBits;
	}


MyBoolean						MainWindowGetInterpolationOverTime(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->InterpolateOverTime;
	}


MyBoolean						MainWindowGetInterpolationAcrossWaves(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->InterpolateAcrossWaves;
	}


double							MainWindowGetScanningGap(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return LargeBCD2Double(Window->ScanningGap);
	}


double							MainWindowGetBufferDuration(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return LargeBCD2Double(Window->BufferDuration);
	}


MyBoolean						MainWindowGetClipWarning(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->ClipWarning;
	}


char*								MainWindowGetPostProcessing(MainWindowRec* Window)
	{
		char*							StringTemp;

		CheckPtrExistence(Window);
		StringTemp = CopyPtr(Window->SongPostProcessing);
		if (StringTemp != NIL)
			{
				SetTag(StringTemp,"SongPostProcessing");
			}
		return StringTemp;
	}


MyBoolean						MainWindowGetPostProcessingEnable(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->SongPostProcessingEnable;
	}


void								PutMainWindowStereo(MainWindowRec* Window, MyBoolean NewStereoFlag)
	{
		CheckPtrExistence(Window);
		if (Window->StereoPlayback != NewStereoFlag)
			{
				Window->StereoPlayback = NewStereoFlag;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowSurround(MainWindowRec* Window, MyBoolean NewSurround)
	{
		CheckPtrExistence(Window);
		if (Window->SurroundEncoding != NewSurround)
			{
				Window->SurroundEncoding = NewSurround;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowSamplingRate(MainWindowRec* Window,
											long NewSamplingRate)
	{
		CheckPtrExistence(Window);
		if (NewSamplingRate < MINSAMPLINGRATE)
			{
				NewSamplingRate = MINSAMPLINGRATE;
			}
		if (NewSamplingRate > MAXSAMPLINGRATE)
			{
				NewSamplingRate = MAXSAMPLINGRATE;
			}
		if (Window->SamplingRate != NewSamplingRate)
			{
				Window->SamplingRate = NewSamplingRate;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowEnvelopeRate(MainWindowRec* Window,
											long NewEnvelopeRate)
	{
		CheckPtrExistence(Window);
		if (NewEnvelopeRate < 1)
			{
				NewEnvelopeRate = 1;
			}
		if (NewEnvelopeRate > MAXSAMPLINGRATE)
			{
				NewEnvelopeRate = MAXSAMPLINGRATE;
			}
		if (Window->EnvelopeUpdateRate != NewEnvelopeRate)
			{
				Window->EnvelopeUpdateRate = NewEnvelopeRate;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowBeatsPerMinute(MainWindowRec* Window,
											double NewBeatsPerMinute)
	{
		LargeBCDType			BPM;

		CheckPtrExistence(Window);
		BPM = Double2LargeBCD(NewBeatsPerMinute);
		if (Window->DefaultBeatsPerMinute != BPM)
			{
				Window->DefaultBeatsPerMinute = BPM;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowVolumeScaling(MainWindowRec* Window,
											double NewVolumeScaling)
	{
		LargeBCDType			Vol;

		CheckPtrExistence(Window);
		Vol = Double2LargeBCD(NewVolumeScaling);
		if (Window->OverallVolumeScalingFactor != Vol)
			{
				Window->OverallVolumeScalingFactor = Vol;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowOutputNumBits(MainWindowRec* Window,
											OutputNumBitsType NewOutputNumBits)
	{
		CheckPtrExistence(Window);
		ERROR((NewOutputNumBits != eOutput8Bits) && (NewOutputNumBits != eOutput16Bits)
			&& (NewOutputNumBits != eOutput24Bits) && (NewOutputNumBits != eOutput32Bits),
			PRERR(ForceAbort,"PutMainWindowOutputNumBits:  bad value"));
		if (Window->OutputNumBits != NewOutputNumBits)
			{
				Window->OutputNumBits = NewOutputNumBits;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowInterpolationOverTime(MainWindowRec* Window,
											MyBoolean NewInterpOverTime)
	{
		CheckPtrExistence(Window);
		if (Window->InterpolateOverTime != NewInterpOverTime)
			{
				Window->InterpolateOverTime = NewInterpOverTime;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowInterpolationAcrossWaves(MainWindowRec* Window,
											MyBoolean NewInterpAcrossWaves)
	{
		CheckPtrExistence(Window);
		if (Window->InterpolateAcrossWaves != NewInterpAcrossWaves)
			{
				Window->InterpolateAcrossWaves = NewInterpAcrossWaves;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowScanningGap(MainWindowRec* Window, double NewScanningGap)
	{
		LargeBCDType			Gap;

		CheckPtrExistence(Window);
		Gap = Double2LargeBCD(NewScanningGap);
		if (Gap != Window->ScanningGap)
			{
				Window->ScanningGap = Gap;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowBufferDuration(MainWindowRec* Window,
											double NewBufferDuration)
	{
		LargeBCDType			Buffering;

		CheckPtrExistence(Window);
		Buffering = Double2LargeBCD(NewBufferDuration);
		if (Buffering != Window->BufferDuration)
			{
				Window->BufferDuration = Buffering;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowClipWarning(MainWindowRec* Window,
											MyBoolean NewClipWarning)
	{
		CheckPtrExistence(Window);
		if (NewClipWarning != Window->ClipWarning)
			{
				Window->ClipWarning = NewClipWarning;
				Window->StuffModified = True;
			}
	}


void								PutMainWindowPostProcessing(MainWindowRec* Window,
											char* NewPostProcessing)
	{
		CheckPtrExistence(Window);
		CheckPtrExistence(NewPostProcessing);
		ReleasePtr(Window->SongPostProcessing);
		Window->SongPostProcessing = NewPostProcessing;
		Window->StuffModified = True;
	}


void								PutMainWindowPostProcessingEnable(MainWindowRec* Window,
											MyBoolean NewPostProcessingEnable)
	{
		CheckPtrExistence(Window);
		if (Window->SongPostProcessingEnable != NewPostProcessingEnable)
			{
				Window->SongPostProcessingEnable = NewPostProcessingEnable;
				Window->StuffModified = True;
			}
	}


/* General Information Subblock Structure: */
/*   4-byte file format version code */
/*       "Syn1" - first file format */
/*   1-byte unsigned tab size code */
/*       should be in the range of 1..255 */
/*   4-byte little endian comment text length (positive 2s complement, in bytes) */
/*   n-byte character data for comment text (line feed = 0x0a) */
/*   1-byte stereo playback flag */
/*       0 = mono */
/*       1 = stereo */
/*   1-byte surround encoding flag */
/*       0 = no surround encoding */
/*       1 = generic surround encoding */
/*   4-byte little endian output sampling rate */
/*       should be in the range of 100..65535 */
/*   4-byte little endian envelope update rate */
/*       should be in the range of 1..65535 */
/*   4-byte little endian large integer coded decimal beats per minute */
/*       large integer coded decimal is decimal * 1000000 with a */
/*       range of -1999.999999 to 1999.999999 */
/*   4-byte little endian large integer coded decimal total volume scaling factor */
/*   1-byte number of bits to output */
/*       should be 8, 16, 24, or 32 */
/*   1-byte flag for interpolation over time */
/*       0 = don't interpolate over time */
/*       1 = do interpolate over time (when resampling waveforms) */
/*   1-byte flag for interpolation across waves */
/*       0 = don't interpolate across waves */
/*       1 = do interpolate across waves (when wave table synthesis index is */
/*           not an integer) */
/*   4-byte little endian large integer coded decimal scanning gap */
/*   4-byte little endian large integer coded decimal buffer duration (in seconds) */
/*   1-byte flag for clipping warning */
/*       0 = don't warn about clipped samples */
/*       1 = do warn about clipped samples */
/*   1-byte flag for song post processing enabling */
/*       0 = don't do song postprocessing */
/*       1 = do song postprocessing */
/*   4-byte little endian length of song post processing function */
/*   n-bytes of post processing function text (line fed = 0x0a) */


/* read the general information subblock from the specified file. */
FileLoadingErrors		MainWindowReadData(MainWindowRec* Window,
											struct BufferedInputRec* Input)
	{
		char							FileFormatVersion[4];
		signed long				BlockLength;
		char*							StringTemp;
		unsigned char			BuffChar;
		signed long				SignedLong;

		CheckPtrExistence(Window);
		CheckPtrExistence(Input);

		/*   4-byte file format version code */
		/*       "Syn1" - first file format */
		if (!ReadBufferedInput(Input,4,FileFormatVersion))
			{
				return eFileLoadDiskError;
			}
		if (!MemEqu(FileFormatVersion,"Syn1",4))
			{
				return eFileLoadBadFormat;
			}

		/*   1-byte unsigned tab size code */
		/*       should be in the range of 1..255 */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar < MINTABCOUNT)
			{
				BuffChar = MINTABCOUNT;
			}
		if (BuffChar > MAXTABCOUNT)
			{
				BuffChar = MAXTABCOUNT;
			}
		Window->TabSize = BuffChar;

		/*   4-byte little endian comment text length (in bytes) */
		if (!ReadBufferedSignedLongLittleEndian(Input,&BlockLength))
			{
				return eFileLoadDiskError;
			}
		if (BlockLength < 0)
			{
				return eFileLoadBadFormat;
			}
		StringTemp = AllocPtrCanFail(BlockLength,"MainWindowReadData:  comment string temp");
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		/*   n-byte character data for comment text (line feed = 0x0a) */
		if (!ReadBufferedInput(Input,BlockLength,StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		if (!TextEditNewRawData(Window->CommentInfo,StringTemp,"\x0a"))
			{
				ReleasePtr(StringTemp);
				return eFileLoadOutOfMemory;
			}
		TextEditHasBeenSaved(Window->CommentInfo);
		ReleasePtr(StringTemp);

		/*   1-byte stereo playback flag */
		/*       0 = mono */
		/*       1 = stereo */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->StereoPlayback = False;
			}
		else if (BuffChar == 1)
			{
				Window->StereoPlayback = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   1-byte surround encoding flag */
		/*       0 = no surround encoding */
		/*       1 = dolby surround encoding */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->SurroundEncoding = False;
			}
		else if (BuffChar == 1)
			{
				Window->SurroundEncoding = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   4-byte little endian output sampling rate */
		/*       should be in the range of 100..65535 */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		if (SignedLong < MINSAMPLINGRATE)
			{
				SignedLong = MINSAMPLINGRATE;
			}
		if (SignedLong > MAXSAMPLINGRATE)
			{
				SignedLong = MAXSAMPLINGRATE;
			}
		Window->SamplingRate = SignedLong;

		/*   4-byte little endian envelope update rate */
		/*       should be in the range of 1..65535 */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		if (SignedLong < 1)
			{
				SignedLong = 1;
			}
		if (SignedLong > MAXSAMPLINGRATE)
			{
				SignedLong = MAXSAMPLINGRATE;
			}
		Window->EnvelopeUpdateRate = SignedLong;

		/*   4-byte little endian large integer coded decimal beats per minute */
		/*       large integer coded decimal is decimal * 1000000 with a */
		/*       range of -1999.999999 to 1999.999999 */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		Window->DefaultBeatsPerMinute = SignedLong;

		/*   4-byte little endian large integer coded total volume scaling factor */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		Window->OverallVolumeScalingFactor = SignedLong;

		/*   1-byte number of bits to output */
		/*       should be 8, 16, 24, or 32 */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 8)
			{
				Window->OutputNumBits = eOutput8Bits;
			}
		else if (BuffChar == 16)
			{
				Window->OutputNumBits = eOutput16Bits;
			}
		else if (BuffChar == 24)
			{
				Window->OutputNumBits = eOutput24Bits;
			}
		else if (BuffChar == 32)
			{
				Window->OutputNumBits = eOutput32Bits;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   1-byte flag for interpolation over time */
		/*       0 = don't interpolate over time */
		/*       1 = do interpolate over time (when resampling waveforms) */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->InterpolateOverTime = False;
			}
		else if (BuffChar == 1)
			{
				Window->InterpolateOverTime = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   1-byte flag for interpolation across waves */
		/*       0 = don't interpolate across waves */
		/*       1 = do interpolate across waves (when wave table synthesis index is */
		/*           not an integer) */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->InterpolateAcrossWaves = False;
			}
		else if (BuffChar == 1)
			{
				Window->InterpolateAcrossWaves = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   4-byte little endian large integer coded decimal scanning gap */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		Window->ScanningGap = SignedLong;

		/*   4-byte little endian large integer coded decimal buffer duration (in seconds) */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		Window->BufferDuration = SignedLong;

		/*   1-byte flag for clipping warning */
		/*       0 = don't warn about clipped samples */
		/*       1 = do warn about clipped samples */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->ClipWarning = False;
			}
		else if (BuffChar == 1)
			{
				Window->ClipWarning = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   1-byte flag for song post processing enabling */
		/*       0 = don't do song postprocessing */
		/*       1 = do song postprocessing */
		if (!ReadBufferedUnsignedChar(Input,&BuffChar))
			{
				return eFileLoadDiskError;
			}
		if (BuffChar == 0)
			{
				Window->SongPostProcessingEnable = False;
			}
		else if (BuffChar == 1)
			{
				Window->SongPostProcessingEnable = True;
			}
		else
			{
				return eFileLoadBadFormat;
			}

		/*   4-byte little endian length of song post processing function */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				return eFileLoadDiskError;
			}
		if (SignedLong < 0)
			{
				return eFileLoadBadFormat;
			}

		/*   n-bytes of post processing function text (line fed = 0x0a) */
		StringTemp = AllocPtrCanFail(SignedLong,"SongPostProcessing");
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!ReadBufferedInput(Input,SignedLong,StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(Window->SongPostProcessing);
		Window->SongPostProcessing = StringTemp;

		return eFileLoadNoError;
	}


/* write the general information subblock to the specified file. */
FileLoadingErrors		MainWindowWriteData(MainWindowRec* Window,
											struct BufferedOutputRec* Output)
	{
		char*							StringTemp;

		CheckPtrExistence(Window);
		CheckPtrExistence(Output);

		/*   4-byte file format version code */
		/*       "Syn1" - first file format */
		if (!WriteBufferedOutput(Output,4,"Syn1"))
			{
				return eFileLoadDiskError;
			}

		/*   1-byte unsigned tab size code */
		/*       should be in the range of 1..255 */
		if (!WriteBufferedUnsignedChar(Output,Window->TabSize))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian comment text length (in bytes) */
		StringTemp = TextEditGetRawData(Window->CommentInfo,"\x0a");
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		/*   n-byte character data for comment text (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   1-byte stereo playback flag */
		/*       0 = mono */
		/*       1 = stereo */
		if (Window->StereoPlayback)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   1-byte surround encoding flag */
		/*       0 = no surround encoding */
		/*       1 = dolby surround encoding */
		if (Window->SurroundEncoding)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   4-byte little endian output sampling rate */
		/*       should be in the range of 100..65535 */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->SamplingRate))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian envelope update rate */
		/*       should be in the range of 1..65535 */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->EnvelopeUpdateRate))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian large integer coded decimal beats per minute */
		/*       large integer coded decimal is decimal * 1000000 with a */
		/*       range of -1999.999999 to 1999.999999 */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->DefaultBeatsPerMinute))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian large integer coded total volume scaling factor */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->OverallVolumeScalingFactor))
			{
				return eFileLoadDiskError;
			}

		/*   1-byte number of bits to output */
		/*       should be 8, 16, 24, or 32 */
		switch (Window->OutputNumBits)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"MainWindowWriteData:  bad value in Window->OutputNumBits"));
					break;
				case eOutput8Bits:
					if (!WriteBufferedUnsignedChar(Output,8))
						{
							return eFileLoadDiskError;
						}
					break;
				case eOutput16Bits:
					if (!WriteBufferedUnsignedChar(Output,16))
						{
							return eFileLoadDiskError;
						}
					break;
				case eOutput24Bits:
					if (!WriteBufferedUnsignedChar(Output,24))
						{
							return eFileLoadDiskError;
						}
					break;
				case eOutput32Bits:
					if (!WriteBufferedUnsignedChar(Output,32))
						{
							return eFileLoadDiskError;
						}
					break;
			}

		/*   1-byte flag for interpolation over time */
		/*       0 = don't interpolate over time */
		/*       1 = do interpolate over time (when resampling waveforms) */
		if (Window->InterpolateOverTime)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   1-byte flag for interpolation across waves */
		/*       0 = don't interpolate across waves */
		/*       1 = do interpolate across waves (when wave table synthesis index is */
		/*           not an integer) */
		if (Window->InterpolateAcrossWaves)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   4-byte little endian large integer coded decimal scanning gap */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->ScanningGap))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian large integer coded decimal buffer duration (in seconds) */
		if (!WriteBufferedSignedLongLittleEndian(Output,Window->BufferDuration))
			{
				return eFileLoadDiskError;
			}

		/*   1-byte flag for clipping warning */
		/*       0 = don't warn about clipped samples */
		/*       1 = do warn about clipped samples */
		if (Window->ClipWarning)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   1-byte flag for song post processing enabling */
		/*       0 = don't do song postprocessing */
		/*       1 = do song postprocessing */
		if (Window->SongPostProcessingEnable)
			{
				if (!WriteBufferedUnsignedChar(Output,1))
					{
						return eFileLoadDiskError;
					}
			}
		 else
			{
				if (!WriteBufferedUnsignedChar(Output,0))
					{
						return eFileLoadDiskError;
					}
			}

		/*   4-byte little endian length of song post processing function */
		StringTemp = MainWindowGetPostProcessing(Window);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-bytes of post processing function text (line fed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		return eFileLoadNoError;
	}


/* get the sample list object for the specified document.  the actual thing */
/* is returned */
struct SampleListRec*	MainWindowGetSampleList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->SampleList;
	}


/* get the algorithmic sample list object for the specified document.  the actual */
/* thing is returned. */
struct AlgoSampListRec*	MainWindowGetAlgoSampList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->AlgoSampList;
	}


/* get the wave table list object for the specified document.  the actual thing */
/* is returned */
struct WaveTableListRec*	MainWindowGetWaveTableList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->WaveTableList;
	}


/* get the algorithmic wave table list object for the specified document.  the */
/* actual thing is returned */
struct AlgoWaveTableListRec*	MainWindowGetAlgoWaveTableList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->AlgoWaveTableList;
	}


/* get the instrument list object for the specified document.  the actual thing */
/* is returned */
struct InstrListRec*	MainWindowGetInstrList(MainWindowRec* Window)
	{
		CheckPtrExistence(Window);
		return Window->InstrumentList;
	}


/* this updates the object deletion undo information */
void								MainWindowNewDeleteUndoInfo(MainWindowRec* Window,
											struct FileSpec* Location, struct FileType* File)
	{
		CheckPtrExistence(Window);
		CheckPtrExistence(Location);
		CheckPtrExistence(File);
		if (Window->DeleteUndoFileLocation != NIL)
			{
				CloseFile(Window->DeleteUndoFile);
				DeleteFile(Window->DeleteUndoFileLocation);
				DisposeFileSpec(Window->DeleteUndoFileLocation);
			}
		Window->DeleteUndoFileLocation = Location;
		Window->DeleteUndoFile = File;
	}
