/* Main.c */
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

#define Including_Main_c

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "Main.h"
#include "Screen.h"
#include "Alert.h"
#include "Scroll.h"
#include "GrowIcon.h"
#include "StartupOpen.h"
#include "DataMunging.h"
#include "Memory.h"
#include "MainWindowStuff.h"
#include "WindowDispatcher.h"
#include "AboutBox.h"
#include "EditImages.h"
#include "NoteImages.h"
#include "NoteButtonImages.h"
#include "LittleButtonImages.h"
#include "Factoring.h"
#include "Files.h"
#include "GlobalWindowMenuList.h"
#include "PlayAIFFFile.h"


#define STARTUPDELAY (3)


char*							GlobalSearchString;
char*							GlobalReplaceString;

MenuType*					mmAppleMenu;
MenuType*					mmFileMenu;
MenuType*					mmEditMenu;
MenuType*					mmSearchMenu;
MenuType*					mmObjectsMenu;
MenuType*					mmFunctionMenu;
MenuType*					mmSampleMenu;
MenuType*					mmWindowMenu;

MenuItemType*			mAboutThisProgram;
MenuItemType*			mNewFile;
MenuItemType*			mOpenFile;
MenuItemType*			mCloseFile;
MenuItemType*			mSaveFile;
MenuItemType*			mSaveAs;
MenuItemType*			mSetTabSize;
MenuItemType*			mPlay;
MenuItemType*			mPlayThisTrackFromHere;
MenuItemType*			mPlayAllTracksFromHere;
MenuItemType*			mPlayAIFFFile;
MenuItemType*			mQuit;

MenuItemType*			mUndo;
MenuItemType*			mCut;
MenuItemType*			mCopy;
MenuItemType*			mPaste;
MenuItemType*			mClear;
MenuItemType*			mSelectAll;
MenuItemType*			mShiftLeft;
MenuItemType*			mShiftRight;
MenuItemType*			mBalanceParens;
MenuItemType*			mEnterSelection;
MenuItemType*			mFind;
MenuItemType*			mFindAgain;
MenuItemType*			mReplace;
MenuItemType*			mReplaceAndFindAgain;
MenuItemType*			mShowSelection;

MenuItemType*			mNewSample;
MenuItemType*			mNewFunction;
MenuItemType*			mNewAlgoSample;
MenuItemType*			mNewWaveTable;
MenuItemType*			mNewAlgoWaveTable;
MenuItemType*			mNewTrack;
MenuItemType*			mNewInstrument;
MenuItemType*			mBuildFunction;
MenuItemType*			mUnbuildFunction;
MenuItemType*			mUnbuildAllFunctions;
MenuItemType*			mBuildEntireProject;
MenuItemType*			mDisassembleFunction;
MenuItemType*			mOpenObject;
MenuItemType*			mEditTrackAttributes;
MenuItemType*			mDeleteObject;
MenuItemType*			mCalculator;
MenuItemType*			mEvaluateCalc;
MenuItemType*			mAlgoSampToSample;
MenuItemType*			mSampleEditLoop1;
MenuItemType*			mSampleEditLoop2;
MenuItemType*			mSampleEditLoop3;
MenuItemType*			mImportWAVFormat;
MenuItemType*			mImportAIFFFormat;
MenuItemType*			mImportRAWFormat;
MenuItemType*			mExportWAVFormat;
MenuItemType*			mExportAIFFFormat;
MenuItemType*			mExportRAWFormat;
MenuItemType*			mCopyObject;
MenuItemType*			mPasteObject;
MenuItemType*			mTransposeSelection;
MenuItemType*			mGotoMeasureBar;


int								main(int argc, char* argv[])
	{
		MyBoolean				TryToOpenUntitledDocument;
		MyBoolean				ContinueFlag;
#if 0
		double					StartupTime;
		MyBoolean				InitialAboutBoxStillVisible;
#endif

		APRINT(("+main"));

		/* level 0 initialization */
		if (!InitializeScreen())
			{
				goto InitScreenFailurePoint;
			}
		SetErrorBeepEnable(False); /* it's a music program, so disable nasty noises */

		/* level 1 initialization */
		if (!InitializeAlertSubsystem())
			{
				goto InitAlertFailurePoint;
			}
		if (!InitializeGrowIcon())
			{
				goto InitGrowIconFailurePoint;
			}

		/* application initialization */
		if (!InitializeFactoring())
			{
				goto InitFactoringFailurePoint;
			}
		if (!InitializeEditImages())
			{
				goto InitEditImagesFailurePoint;
			}
		if (!InitializeNoteImages())
			{
				goto InitNoteImagesFailurePoint;
			}
		if (!InitializeNoteButtonImages())
			{
				goto InitNoteButtonImagesFailurePoint;
			}
		if (!InitializeLittleButtonImages())
			{
				goto InitializeLittleButtonImagesFailurePoint;
			}
		if (!InitializeWindowDispatcher())
			{
				goto InitWindowDispatcherFailurePoint;
			}
		if (!InitializeDocuments())
			{
				goto InitDocumentsFailurePoint;
			}
		GlobalSearchString = AllocPtrCanFail(0,"GlobalSearchString");
		if (GlobalSearchString == NIL)
			{
				goto AllocGlobalSearchStringFailurePoint;
			}
		GlobalReplaceString = AllocPtrCanFail(0,"GlobalReplaceString");
		if (GlobalReplaceString == NIL)
			{
				goto AllocGlobalReplaceStringFailurePoint;
			}
		if (!InitializeGlobalWindowMenuList())
			{
				goto InitializeGlobalMenuWindowListFailurePoint;
			}

#if 0
#if !DEBUG
		ShowAboutBox();
#endif
		InitialAboutBoxStillVisible = True;
		StartupTime = ReadTimer();
#endif

		mmAppleMenu = MakeAppleMenu();
		if (mmAppleMenu == NIL)
			{
				goto MakeAppleMenuFailurePoint;
			}
		mAboutThisProgram = MakeNewMenuItem(mmAppleMenu,"About Out Of Phase...",0);
		if (mAboutThisProgram == NIL)
			{
				goto MakeFileMenuFailurePoint;
			}
		ShowMenu(mmAppleMenu);

		mmFileMenu = MakeNewMenu("File");
		if (mmFileMenu == NIL)
			{
				goto MakeFileMenuFailurePoint;
			}
		mNewFile = MakeNewMenuItem(mmFileMenu,"New",'N');
		if (mNewFile == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mOpenFile = MakeNewMenuItem(mmFileMenu,"Open...",'O');
		if (mOpenFile == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mCloseFile = MakeNewMenuItem(mmFileMenu,"Close",'W');
		if (mCloseFile == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		AppendSeparator(mmFileMenu);
		mSaveFile = MakeNewMenuItem(mmFileMenu,"Save",'S');
		if (mSaveFile == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mSaveAs = MakeNewMenuItem(mmFileMenu,"Save As...",0);
		if (mSaveAs == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		AppendSeparator(mmFileMenu);
		mSetTabSize = MakeNewMenuItem(mmFileMenu,"Set Tab Size...",0);
		if (mSetTabSize == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		AppendSeparator(mmFileMenu);
		mPlay = MakeNewMenuItem(mmFileMenu,"Play...",'P');
		if (mPlay == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mPlayThisTrackFromHere = MakeNewMenuItem(mmFileMenu,"Play This Track",'T');
		if (mPlayThisTrackFromHere == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mPlayAllTracksFromHere = MakeNewMenuItem(mmFileMenu,"Play All Tracks",'Y');
		if (mPlayAllTracksFromHere == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		AppendSeparator(mmFileMenu);
		mPlayAIFFFile = MakeNewMenuItem(mmFileMenu,"Play AIFF File",0);
		if (mPlayAIFFFile == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		AppendSeparator(mmFileMenu);
		mQuit = MakeNewMenuItem(mmFileMenu,"Quit",'Q');
		if (mQuit == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		ShowMenu(mmFileMenu);

		mmEditMenu = MakeNewMenu("Edit");
		if (mmEditMenu == NIL)
			{
				goto MakeEditMenuFailurePoint;
			}
		mUndo = MakeNewMenuItem(mmEditMenu,"Undo",'Z');
		if (mUndo == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		AppendSeparator(mmEditMenu);
		mCut = MakeNewMenuItem(mmEditMenu,"Cut",'X');
		if (mCut == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mCopy = MakeNewMenuItem(mmEditMenu,"Copy",'C');
		if (mCopy == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mPaste = MakeNewMenuItem(mmEditMenu,"Paste",'V');
		if (mPaste == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mClear = MakeNewMenuItem(mmEditMenu,"Clear",0);
		if (mClear == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mSelectAll = MakeNewMenuItem(mmEditMenu,"Select All",'A');
		if (mSelectAll == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		AppendSeparator(mmEditMenu);
		mShiftLeft = MakeNewMenuItem(mmEditMenu,"Shift Left",'L');
		if (mShiftLeft == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mShiftRight = MakeNewMenuItem(mmEditMenu,"Shift Right",'R');
		if (mShiftRight == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mBalanceParens = MakeNewMenuItem(mmEditMenu,"Balance",'B');
		if (mBalanceParens == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		AppendSeparator(mmEditMenu);
		mTransposeSelection = MakeNewMenuItem(mmEditMenu,"Transpose Selection",0);
		if (mTransposeSelection == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mGotoMeasureBar = MakeNewMenuItem(mmEditMenu,"Go To Measure...",0);
		if (mGotoMeasureBar == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		AppendSeparator(mmEditMenu);
		mShowSelection = MakeNewMenuItem(mmEditMenu,"Show Selection",0);
		if (mShowSelection == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		ShowMenu(mmEditMenu);

		mmSearchMenu = MakeNewMenu("Search");
		if (mmSearchMenu == NIL)
			{
				goto MakeSearchMenuFailurePoint;
			}
		mEnterSelection = MakeNewMenuItem(mmSearchMenu,"Enter Selection",'E');
		if (mEnterSelection == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		mFind = MakeNewMenuItem(mmSearchMenu,"Find...",'F');
		if (mFind == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		mFindAgain = MakeNewMenuItem(mmSearchMenu,"Find Again",'G');
		if (mFindAgain == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		mReplace = MakeNewMenuItem(mmSearchMenu,"Replace",'=');
		if (mReplace == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		mReplaceAndFindAgain = MakeNewMenuItem(mmSearchMenu,"Replace and Find Again",'H');
		if (mReplaceAndFindAgain == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		ShowMenu(mmSearchMenu);

		mmObjectsMenu = MakeNewMenu("Objects");
		if (mmObjectsMenu == NIL)
			{
				goto MakeObjectsMenuFailurePoint;
			}
		mDeleteObject = MakeNewMenuItem(mmObjectsMenu,"Delete",0);
		if (mDeleteObject == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mOpenObject = MakeNewMenuItem(mmObjectsMenu,"Edit",0);
		if (mOpenObject == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mEditTrackAttributes = MakeNewMenuItem(mmObjectsMenu,"Edit Track Attributes",0);
		if (mEditTrackAttributes == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mCopyObject = MakeNewMenuItem(mmObjectsMenu,"Copy Object",0);
		if (mCopyObject == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mPasteObject = MakeNewMenuItem(mmObjectsMenu,"Paste Object",0);
		if (mPasteObject == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mNewTrack = MakeNewMenuItem(mmObjectsMenu,"New Track",0);
		if (mNewTrack == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewInstrument = MakeNewMenuItem(mmObjectsMenu,"New Instrument",0);
		if (mNewInstrument == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewWaveTable = MakeNewMenuItem(mmObjectsMenu,"New Wave Table",0);
		if (mNewWaveTable == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewAlgoWaveTable = MakeNewMenuItem(mmObjectsMenu,"New Algorithmic Wave Table",0);
		if (mNewAlgoWaveTable == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewSample = MakeNewMenuItem(mmObjectsMenu,"New Sample",0);
		if (mNewSample == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewAlgoSample = MakeNewMenuItem(mmObjectsMenu,"New Algorithmic Sample",0);
		if (mNewAlgoSample == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mNewFunction = MakeNewMenuItem(mmObjectsMenu,"New Function Module",0);
		if (mNewFunction == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mCalculator = MakeNewMenuItem(mmObjectsMenu,"Calculator",0);
		if (mCalculator == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		AppendSeparator(mmObjectsMenu);
		mEvaluateCalc = MakeNewMenuItem(mmObjectsMenu,"Evaluate",'D');
		if (mEvaluateCalc == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		ShowMenu(mmObjectsMenu);

		mmFunctionMenu = MakeNewMenu("Build");
		if (mmFunctionMenu == NIL)
			{
				goto MakeFunctionMenuFailurePoint;
			}
		mBuildFunction = MakeNewMenuItem(mmFunctionMenu,"Compile",'K');
		if (mBuildFunction == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		AppendSeparator(mmFunctionMenu);
		mUnbuildFunction = MakeNewMenuItem(mmFunctionMenu,"Remove Objects For This",0);
		if (mUnbuildFunction == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		mUnbuildAllFunctions = MakeNewMenuItem(mmFunctionMenu,"Remove Objects For All",0);
		if (mUnbuildAllFunctions == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		mBuildEntireProject = MakeNewMenuItem(mmFunctionMenu,"Build All Objects",'U');
		if (mBuildEntireProject == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		AppendSeparator(mmFunctionMenu);
		mDisassembleFunction = MakeNewMenuItem(mmFunctionMenu,"Disassemble",0);
		if (mDisassembleFunction == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		AppendSeparator(mmFunctionMenu);
		mAlgoSampToSample = MakeNewMenuItem(mmFunctionMenu,"Open Algorithmic Sample As New Sample",0);
		if (mAlgoSampToSample == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		ShowMenu(mmFunctionMenu);

		mmSampleMenu = MakeNewMenu("Samples");
		if (mmSampleMenu == NIL)
			{
				goto MakeSampleMenuFailedPoint;
			}
		mSampleEditLoop1 = MakeNewMenuItem(mmSampleMenu,"Edit Loop 1",'1');
		if (mSampleEditLoop1 == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mSampleEditLoop2 = MakeNewMenuItem(mmSampleMenu,"Edit Loop 2",'2');
		if (mSampleEditLoop2 == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mSampleEditLoop3 = MakeNewMenuItem(mmSampleMenu,"Edit Loop 3",'3');
		if (mSampleEditLoop3 == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		AppendSeparator(mmSampleMenu);
		mImportWAVFormat = MakeNewMenuItem(mmSampleMenu,"Import WAV Sample",0);
		if (mImportWAVFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mImportAIFFFormat = MakeNewMenuItem(mmSampleMenu,"Import AIFF Sample",0);
		if (mImportAIFFFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mImportRAWFormat = MakeNewMenuItem(mmSampleMenu,"Import Raw Sample",0);
		if (mImportRAWFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		AppendSeparator(mmSampleMenu);
		mExportWAVFormat = MakeNewMenuItem(mmSampleMenu,"Export WAV Sample",0);
		if (mExportWAVFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mExportAIFFFormat = MakeNewMenuItem(mmSampleMenu,"Export AIFF Sample",0);
		if (mExportAIFFFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		mExportRAWFormat = MakeNewMenuItem(mmSampleMenu,"Export Raw Sample",0);
		if (mExportRAWFormat == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		ShowMenu(mmSampleMenu);

		mmWindowMenu = MakeNewMenu("Windows");
		if (mmWindowMenu == NIL)
			{
				goto MakeWindowMenuFailedPoint;
			}
		ShowMenu(mmWindowMenu);

		PrepareStartupDocuments(argc,argv);
		TryToOpenUntitledDocument = True;
		ContinueFlag = True;
		while (ContinueFlag)
			{
				OrdType					XLoc;
				OrdType					YLoc;
				ModifierFlags		Modifiers;
				FileSpec*				StartupItem;
				WinType*				Window;
				char						KeyPress;
				MenuItemType*		MenuItem;
				EventType				TheEvent;

				/* see if there are any startup items to open */
				if (GetStartupObject(&StartupItem))
					{
						/* if it returns True, then we can go ahead */
						if (StartupItem == NIL)
							{
								/* if we haven't opened an untitled document, and the thing */
								/* returned True, except it returned a NIL item, then we */
								/* can open an untitled document */
								if (TryToOpenUntitledDocument)
									{
										OpenDocument(NIL);
										TryToOpenUntitledDocument = False;
									}
							}
						 else
							{
								/* if it isn't NIL, then open the actual thing */
								/* this swallows the startup item record so we don't have */
								/* do dispose of it */
								OpenDocument(StartupItem);
								TryToOpenUntitledDocument = False;
							}
					}
#if 0
				if (InitialAboutBoxStillVisible)
					{
						if (TimerDifference(ReadTimer(),StartupTime) > STARTUPDELAY)
							{
								InitialAboutBoxStillVisible = False;
								HideAnyAboutBoxes();
							}
					}
#endif
				TheEvent = GetAnEvent(&XLoc,&YLoc,&Modifiers,&Window,&MenuItem,&KeyPress);
				switch (TheEvent)
					{
						case eMouseUp:
							break;
						case eMenuStarting:
							EnableMenuItem(mAboutThisProgram);
							EnableMenuItem(mNewFile);
							EnableMenuItem(mOpenFile);
							EnableMenuItem(mQuit);
							EnableMenuItem(mPlayAIFFFile);
							DispatchMenuStarting(Window);
							break;
						case eMenuCommand:
							if (MenuItem == mAboutThisProgram)
								{
									ShowAboutBox();
								}
							else if (MenuItem == mOpenFile)
								{
									FileSpec*					Where;
									unsigned long			FileTypeList[1] = {ApplicationFileType};

									Where = GetFileStandard(1,FileTypeList);
									if (Where != NIL)
										{
											OpenDocument(Where);
										}
								}
							else if (MenuItem == mNewFile)
								{
									OpenDocument(NIL);
								}
							else if (MenuItem == mQuit)
								{
									SetQuitPending();
								}
							else if (MenuItem == mPlayAIFFFile)
								{
									PlayAIFFFile();
								}
							else
								{
									DispatchProcessMenuCommand(Window,MenuItem);
								}
							break;
						case eKeyPressed:
							DispatchDoKeyDown(Window,KeyPress,Modifiers);
							break;
						case eMouseDown:
							DispatchDoMouseDown(Window,XLoc,YLoc,Modifiers);
							break;
						case eWindowClosing:
							DispatchCloseWindow(Window);
							break;
						case eWindowResized:
							DispatchWindowJustResized(Window);
							break;
						case eActiveWindowChanged:
							DispatchActiveWindowJustChanged(Window);
							break;
						case eNoEvent:
						case eCheckCursor:
							DispatchDoIdle(Window,TheEvent == eCheckCursor,XLoc,YLoc,Modifiers);
							break;
						default:
							EXECUTE(PRERR(AllowResume,"Unimplemented event received"));
							break;
					}
				if (CheckQuitPending())
					{
						/* ask "are you sure" here; */
						DoCloseAllQuitPending();
						if (CheckQuitPending())
							{
								/* if quit is still pending, then we exit */
								ContinueFlag = False;
							}
					}
			}
		ClearStartupDocuments();


		/* application cleanup */
	 LastMenuConstructionFailurePoint:
		KillMenuAndDeleteItems(mmWindowMenu);
	 MakeWindowMenuFailedPoint:
		KillMenuAndDeleteItems(mmSampleMenu);
	 MakeSampleMenuFailedPoint:
		KillMenuAndDeleteItems(mmFunctionMenu);
	 MakeFunctionMenuFailurePoint:
		KillMenuAndDeleteItems(mmObjectsMenu);
	 MakeObjectsMenuFailurePoint:
		KillMenuAndDeleteItems(mmSearchMenu);
	 MakeSearchMenuFailurePoint:
		KillMenuAndDeleteItems(mmEditMenu);
	 MakeEditMenuFailurePoint:
		KillMenuAndDeleteItems(mmFileMenu);
	 MakeFileMenuFailurePoint:
		KillMenuAndDeleteItems(mmAppleMenu);
	 MakeAppleMenuFailurePoint:
		HideAnyAboutBoxes();
		ShutdownGlobalWindowMenuList();
	 InitializeGlobalMenuWindowListFailurePoint:
		ReleasePtr(GlobalReplaceString);
	 AllocGlobalReplaceStringFailurePoint:
		ReleasePtr(GlobalSearchString);
	 AllocGlobalSearchStringFailurePoint:
		ShutdownDocuments();
	 InitDocumentsFailurePoint:
		ShutdownWindowDispatcher();
	 InitializeLittleButtonImagesFailurePoint:
		ShutdownLittleButtonImages();
	 InitWindowDispatcherFailurePoint:
		CleanupNoteButtonImages();
	 InitNoteButtonImagesFailurePoint:
		CleanupNoteImages();
	 InitNoteImagesFailurePoint:
		CleanupEditImages();
	 InitEditImagesFailurePoint:
		ShutdownFactoring();

	 InitFactoringFailurePoint:
		/* level 1 cleanup */
		ShutdownGrowIcon();
	 InitGrowIconFailurePoint:
		ShutdownAlertSubsystem();

	 InitAlertFailurePoint:
		/* level 0 cleanup */
		ShutdownScreen();

	 InitScreenFailurePoint:
		APRINT(("-main"));
		return 0;
	}
