/* WindowStuff.c */
/*****************************************************************************/
/*                                                                           */
/*    Stupid Fred's Text Editor                                              */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This software is Public Domain; it may be used for any purpose         */
/*    whatsoever without restriction.                                        */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "WindowStuff.h"
#include "WindowDispatcher.h"
#include "TextEdit.h"
#include "Files.h"
#include "Array.h"
#include "Memory.h"
#include "Alert.h"
#include "Menus.h"
#include "DataMunging.h"
#include "Main.h"
#include "GrowIcon.h"
#include "Numbers.h"
#include "NumberDialog.h"
#include "FindDialog.h"
#include "StringDialog.h"


#define MINTABCOUNT (1)
#define MAXTABCOUNT (255)

#define WindowOverlayCycle (5)
#define WindowDownOffset (20)
#define WindowRightOffset (6)
#define WindowScreenBorder (1)

#define DEFAULTFONTSIZE (9)

typedef enum {eMacLF EXECUTE(= -23767), eDosLF, eUnixLF} LineFeedTypes;


struct MyWinRec
	{
		WinType*						ScreenID;
		TextEditRec*				TheEdit;
		MenuItemType*				WindowMenuEntry;
		GenericWindowRec*		GenericWindow;
		MyBoolean						NeedsToBeSaved;

		FontType						TheFont;
		FontSizeType				FontSize;

		MyBoolean						EverSaved;
		FileSpec*						FileLocation; /* valid only if EverSaved == True */
		FileType*						TheFile; /* valid only if EverSaved == True */
		LineFeedTypes				LineFeed;
	};


static ArrayRec*					MasterWindowList;

static char*							SearchKey;
static char*							ReplaceThang;
static char*							PrefixPtr;


MyBoolean						InitWindowStuff(void)
	{
		MasterWindowList = NewArray();
		if (MasterWindowList == NIL)
			{
			 FailurePoint1:
				return False;
			}
		SearchKey = AllocPtrCanFail(0,"SearchKey");
		if (SearchKey == NIL)
			{
			 FailurePoint2:
				DisposeArray(MasterWindowList);
				goto FailurePoint1;
			}
		ReplaceThang = AllocPtrCanFail(0,"ReplaceText");
		if (ReplaceThang == NIL)
			{
			 FailurePoint3:
				ReleasePtr(SearchKey);
				goto FailurePoint2;
			}
		PrefixPtr = AllocPtrCanFail(0,"PrefixPtr");
		if (PrefixPtr == NIL)
			{
			 FailurePoint4:
				ReleasePtr(ReplaceThang);
				goto FailurePoint3;
			}
		return True;
	}


void								KillWindowStuff(void)
	{
		DisposeArray(MasterWindowList);
		ReleasePtr(SearchKey);
		ReleasePtr(ReplaceThang);
		ReleasePtr(PrefixPtr);
	}


static LineFeedTypes	IdentifyLineFeed(char* String)
	{
		if (String[0] == 0x0a)
			{
				return eUnixLF;
			}
		if (String[0] == 0x0d)
			{
				if (String[1] == 0x0a)
					{
						return eDosLF;
					}
				 else
					{
						return eMacLF;
					}
			}
		EXECUTE(PRERR(AllowResume,"IdentifyLineFeed:  couldn't identify line feed"));
		return eDosLF; /* default to MS-DOS */
	}


static void					CalculateProperLineFeed(MyWinRec* Window, char* RawBlock)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(Window);
		CheckPtrExistence(RawBlock);
		Limit = PtrSize(RawBlock);
		Scan = 0;
		while (Scan < Limit)
			{
				if ((RawBlock[Scan] == 0x0a) || (RawBlock[Scan] == 0x0d))
					{
						char						Buffer[3];

						Buffer[0] = RawBlock[Scan];
						if (Scan + 1 < Limit)
							{
								Buffer[1] = RawBlock[Scan + 1];
								Buffer[2] = 0;
							}
						 else
							{
								Buffer[2] = 0;
							}
						Window->LineFeed = IdentifyLineFeed(Buffer);
						return;
					}
				Scan += 1;
			}
		Window->LineFeed = IdentifyLineFeed(SYSTEMLINEFEED);
	}


char*								GetProperStaticLineFeed(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		switch (Window->LineFeed)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"GetProperStaticLineFeed:  unknown line feed type"));
					break;
				case eMacLF:
					return "\x0d";
				case eUnixLF:
					return "\x0a";
				case eDosLF:
					return "\x0d\x0a";
			}
	}


MyWinRec*						OpenDocument(struct FileSpec* Where)
	{
		MyWinRec*					Window;
		long							Limit;
		long							Scan;
		OrdType						X;
		OrdType						Y;
		MyBoolean					MemoryFailure EXECUTE(= -999);
		char*							FilenameNull;
		char							MenuShortcut;

		/* allocate the window record */
		Window = (MyWinRec*)AllocPtrCanFail(sizeof(MyWinRec),"MyWinRec");
		if (Window == NIL)
			{
				MemoryFailure = True;
			 FailurePoint1:
				ERROR((MemoryFailure != True) && (MemoryFailure != False),PRERR(ForceAbort,
					"OpenDocument:  forgot to set MemoryFailure"));
				if (Where != NIL)
					{
						DisposeFileSpec(Where);
					}
				if (MemoryFailure)
					{
						AlertHalt("There is not enough memory available to open a new window.",NIL);
					}
				return NIL;
			}

		/* search for a hole in the window list so that we can add windows */
		/* in the holes to get a nice stacking effect on the screen. */
		Limit = ArrayGetLength(MasterWindowList);
		Scan = 0;
		while (Scan < Limit)
			{
				if (ArrayGetElement(MasterWindowList,Scan) == NIL)
					{
						goto FoundAHolePoint;
					}
				Scan += 1;
			}
		ERROR(Scan != Limit,PRERR(ForceAbort,"OpenDocument:  loop invariant error"));
		if (!ArrayAppendElement(MasterWindowList,NIL))
			{
				MemoryFailure = True;
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		/* fallthrough:  Scan is still a proper index since we appended NIL. */
		/* in event of errors, we don't have to delete from array since we appended */
		/* NIL instead of the window pointer */
	 FoundAHolePoint:
		ArraySetElement(MasterWindowList,Window,Scan);

		/* create the new window */
		X = WindowScreenBorder + (WindowOverlayCycle - (Scan % WindowOverlayCycle))
			* WindowRightOffset;
		Y = WindowTitleBarHeight(eDocumentWindow) + WindowScreenBorder
			+ (Scan % WindowOverlayCycle) * WindowDownOffset;
		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,eWindowZoomable,
			eWindowResizable,X,Y,GetScreenWidth() - X - WindowScreenBorder
			- WindowOtherEdgeWidths(eDocumentWindow),GetScreenHeight() - Y
			- WindowScreenBorder - WindowOtherEdgeWidths(eDocumentWindow),
			(void (*)(void*))&WindowUpdate,Window);
		if (Window->ScreenID == NIL)
			{
				MemoryFailure = True;
			 FailurePoint3:
				ArraySetElement(MasterWindowList,NIL,Scan);
				goto FailurePoint2;
			}

		Window->TheFont = GetMonospacedFont();
		Window->FontSize = DEFAULTFONTSIZE;
		Window->TheEdit = NewTextEdit(Window->ScreenID,eTEVScrollBar | eTEHScrollBar,
			Window->TheFont,Window->FontSize,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
		if (Window->TheEdit == NIL)
			{
				MemoryFailure = True;
			 FailurePoint4:
				KillWindow(Window->ScreenID);
				goto FailurePoint3;
			}

		/* register window with window handler */
		Window->GenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&WindowDoIdle,
			(void (*)(void*))&WindowBecomeActive,
			(void (*)(void*))&WindowBecomeInactive,
			(void (*)(void*))&WindowResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&WindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&WindowDoKeyDown,
			(void (*)(void*))&WindowClose,
			(void (*)(void*))&WindowMenuSetup,
			(void (*)(void*,MenuItemType*))&WindowDoMenuCommand);
		if (Window->GenericWindow == NIL)
			{
				MemoryFailure = True;
			 FailurePoint5:
				DisposeTextEdit(Window->TheEdit);
				goto FailurePoint4;
			}

		/* figure out what the filename is */
		if (Where != NIL)
			{
				char*							Filename;

				Filename = ExtractFileName(Where);
				if (Filename == NIL)
					{
						MemoryFailure = True;
					 FailurePoint6:
						CheckOutDyingWindow(Window->GenericWindow);
						goto FailurePoint5;
					}
				FilenameNull = BlockToStringCopy(Filename);
				ReleasePtr(Filename);
			}
		 else
			{
				FilenameNull = StringFromRaw("Untitled");
			}
		if (FilenameNull == NIL)
			{
				MemoryFailure = True;
				goto FailurePoint6;
			}

		/* set the name of the window */
		SetWindowName(Window->ScreenID,FilenameNull);

		/* create a menu item */
		/* we assign the keyboard shortcuts '1'..'9' to the first 9 files, and */
		/* '0' for the 10th. */
		if ((Scan >= 0) && (Scan <= 8))
			{
				MenuShortcut = Scan + '1';
			}
		else if (Scan == 9)
			{
				MenuShortcut = '0';
			}
		else
			{
				MenuShortcut = 0;
			}
		Window->WindowMenuEntry = MakeNewMenuItem(mmWindowsMenu,FilenameNull,MenuShortcut);
		if (Window->WindowMenuEntry == NIL)
			{
				MemoryFailure = True;
			 FailurePoint7:
				ReleasePtr(FilenameNull);
				goto FailurePoint6;
			}

		/* set the flag that knows if the file has ever been saved */
		Window->EverSaved = (Where != NIL);
		Window->FileLocation = Where;

		Window->LineFeed = IdentifyLineFeed(SYSTEMLINEFEED);

		/* if the file is a real file, then read the data in */
		if (Where != NIL)
			{
				long							FileLength;
				char*							RawBlock;
				MyBoolean					SucceedFlag;

				/* open the file */
				if (!OpenFile(Where,&(Window->TheFile),eReadAndWrite))
					{
						MemoryFailure = False; /* we have our own error message */
					 FailurePoint8:
						KillMenuItem(Window->WindowMenuEntry);
						if (!MemoryFailure)
							{
								AlertHalt("A disk error occurred and the file '_' "
									"couldn't be opened.",FilenameNull);
							}
						goto FailurePoint7;
					}
				/* find out how long the file is and try to allocate a block for it */
				FileLength = GetFileLength(Window->TheFile);
				RawBlock = AllocPtrCanFail(FileLength,"RawFileData");
				if (RawBlock == NIL)
					{
						MemoryFailure = True;
					 FailurePoint9:
						CloseFile(Window->TheFile);
						goto FailurePoint8;
					}
				/* try to read all the data in.  failing to read all the data is not */
				/* considered a fatal error, but warrents a notification. */
				SetFilePosition(Window->TheFile,0);
				if (0 != ReadFromFile(Window->TheFile,&(RawBlock[0]),FileLength))
					{
						AlertWarning("A disk error occurred andt he file '_' couldn't be "
							"completely loaded.",FilenameNull);
					}
				/* figure out what linefeed was used */
				CalculateProperLineFeed(Window,RawBlock);
				/* now, try to put the data block into the text edit object */
				SucceedFlag = TextEditNewRawData(Window->TheEdit,RawBlock,
					GetProperStaticLineFeed(Window));
				/* dump the raw block */
				ReleasePtr(RawBlock);
				if (!SucceedFlag)
					{
						MemoryFailure = True;
					 FailurePoint10:
						goto FailurePoint9;
					}
				/* indicate that it has been saved (since no changes have been made) */
				TextEditHasBeenSaved(Window->TheEdit);
			}
		SetTextEditAutoIndent(Window->TheEdit,True); /* default to auto-indent */
		Window->NeedsToBeSaved = False;

		ReleasePtr(FilenameNull);
		return Window;
	}


MyBoolean						CloseDocument(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		if (Window->NeedsToBeSaved || TextEditDoesItNeedToBeSaved(Window->TheEdit))
			{
				char*							FilenameNull;
				YesNoCancelType		Options;

				if (Window->EverSaved)
					{
						char*							Filename;

						Filename = ExtractFileName(Window->FileLocation);
						if (Filename == NIL)
							{
								/* I sure hope this sort of thing never happens */
							 FailurePoint1:
								AlertHalt("There is not enough memory to close the document.",NIL);
								return False;
							}
						FilenameNull = BlockToStringCopy(Filename);
						ReleasePtr(Filename);
					}
				 else
					{
						FilenameNull = StringFromRaw("Untitled");
					}
				if (FilenameNull == NIL)
					{
						goto FailurePoint1;
					}
				Options = AskYesNoCancel("The document '_' has not been saved.  Do you "
					"want to save your changes?",FilenameNull,"Save","Don't Save","Cancel");
				ReleasePtr(FilenameNull);
				switch (Options)
					{
						default:
							EXECUTE(PRERR(ForceAbort,"CloseDocument:  bad value from AskYesNoCancel"));
							break;
						case eYes:
							if (!Save(Window))
								{
									return False; /* save was cancelled or failed */
								}
							break;
						case eCancel:
							return False; /* user cancelled */
						case eNo:
							/* fall through to deletion point */
							break;
					}
			}
		/* remove window from master array */
		ArraySetElement(MasterWindowList,NIL,ArrayFindElement(MasterWindowList,Window));
		CheckOutDyingWindow(Window->GenericWindow);
		if (Window->EverSaved)
			{
				CloseFile(Window->TheFile);
				DisposeFileSpec(Window->FileLocation);
			}
		DisposeTextEdit(Window->TheEdit);
		KillMenuItem(Window->WindowMenuEntry);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
		return True;
	}


MyBoolean						DoCloseAllQuitPending(void)
	{
		long							Scan;
		long							Limit;

		Limit = ArrayGetLength(MasterWindowList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				MyWinRec*					Window;

				Window = ArrayGetElement(MasterWindowList,Scan);
				if (Window != NIL)
					{
						/* bring window to the top */
						ActivateThisWindow(Window->ScreenID);
						WindowUpdate(Window);
						/* try to close it */
						if (!CloseDocument(Window))
							{
								/* if it refused to close, then we have to stop. */
								return False;
							}
					}
			}
		return True;
	}


MyBoolean						Save(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		if (!Window->EverSaved)
			{
				return SaveAs(Window);
			}
		 else
			{
				FileType*					TempFile;
				FileSpec*					TempFileLocation;

				/* create a temporary file in the same directory */
				TempFileLocation = NewTempFileInTheSameDirectory(Window->FileLocation);
				if (TempFileLocation == NIL)
					{
						char*							Filename;
						char*							FilenameNull;

					 FailurePoint1:
						Filename = ExtractFileName(Window->FileLocation);
						if (Filename == NIL)
							{
								AlertHalt("Unable to save file.  Old contents lost.",NIL);
								return False;
							}
						FilenameNull = BlockToStringCopy(Filename);
						ReleasePtr(Filename);
						if (FilenameNull == NIL)
							{
								goto FailurePoint1;
							}
						AlertHalt("The file '_' could not be written to.",FilenameNull);
						ReleasePtr(FilenameNull);
						return False;
					}
				/* open the file */
				if (!OpenFile(TempFileLocation,&TempFile,eReadAndWrite))
					{
					 FailurePoint2:
						DisposeFileSpec(TempFileLocation);
						goto FailurePoint1;
					}
				/* write data to the file */
				if (!TestEditWriteDataToFile(Window->TheEdit,TempFile,
					GetProperStaticLineFeed(Window)))
					{
					 FailurePoint3:
						CloseFile(TempFile);
						DeleteFile(TempFileLocation);
						goto FailurePoint2;
					}
				FlushLocalBuffers(TempFile);
				/* swap the files on the disk */
				if (!SwapFileDataForks(TempFileLocation,Window->FileLocation,TempFile,
					&(Window->TheFile)))
					{
						/* if this fails, then the parameters are unaltered */
						goto FailurePoint3;
					}
				/* at this point, Window->TheFile has been fixed up, TempFile is closed, */
				/* and we need to dispose TempFileLocation */
				DisposeFileSpec(TempFileLocation);
				/* mark it saved */
				TextEditHasBeenSaved(Window->TheEdit);
				Window->NeedsToBeSaved = False;
			}
		return True;
	}


MyBoolean						SaveAs(MyWinRec* Window)
	{
		FileSpec*					NewWhere;
		char*							FilenameNull;

		if (Window->EverSaved)
			{
				char*							Filename;

				Filename = ExtractFileName(Window->FileLocation);
				if (Filename == NIL)
					{
					 FailurePoint1:
						AlertHalt("There is not enough memory to save the document.",NIL);
						return False;
					}
				FilenameNull = BlockToStringCopy(Filename);
				ReleasePtr(Filename);
			}
		 else
			{
				FilenameNull = StringFromRaw("Untitled");
			}
		if (FilenameNull == NIL)
			{
				goto FailurePoint1;
			}
		NewWhere = PutFile(FilenameNull);
		ReleasePtr(FilenameNull);
		if (NewWhere != NIL)
			{
				FileType*					NewFile;
				char*							XFilename;

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
				if (!CreateFile(NewWhere,ApplicationCreator,CODE4BYTES('T','E','X','T')))
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
				if (Window->EverSaved)
					{
						CloseFile(Window->TheFile);
						DisposeFileSpec(Window->FileLocation);
					}
				Window->EverSaved = True;
				Window->FileLocation = NewWhere;
				Window->TheFile = NewFile;
				SetWindowName(Window->ScreenID,FilenameNull);
				ChangeItemName(Window->WindowMenuEntry,FilenameNull);
				ReleasePtr(FilenameNull);
				return Save(Window);
			}
		 else
			{
				return False;
			}
		EXECUTE(PRERR(ForceAbort,"SaveAs:  Not Reached"));
	}


void								WindowDoIdle(MyWinRec* Window, MyBoolean CheckCursorFlag,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->TheEdit);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->TheEdit,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void								WindowBecomeActive(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->TheEdit);
		SetClipRect(Window->ScreenID,0,0,GetWindowWidth(Window->ScreenID),
			GetWindowHeight(Window->ScreenID));
		DrawBitmap(Window->ScreenID,GetWindowWidth(Window->ScreenID) - 15,
			GetWindowHeight(Window->ScreenID) - 15,GetGrowIcon(True/*enablegrowicon*/));
	}


void								WindowBecomeInactive(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->TheEdit);
		SetClipRect(Window->ScreenID,0,0,GetWindowWidth(Window->ScreenID),
			GetWindowHeight(Window->ScreenID));
		DrawBitmap(Window->ScreenID,GetWindowWidth(Window->ScreenID) - 15,
			GetWindowHeight(Window->ScreenID) - 15,GetGrowIcon(False/*disablegrowicon*/));
	}


void								WindowResized(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		SetTextEditPosition(Window->TheEdit,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
	}


void								WindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, MyWinRec* Window)
	{
		OrdType						WinHeight;
		OrdType						WinWidth;

		CheckPtrExistence(Window);
		WinWidth = GetWindowWidth(Window->ScreenID);
		WinHeight = GetWindowHeight(Window->ScreenID);
		if ((XLoc >= WinWidth - 15) && (XLoc < WinWidth)
			&& (YLoc >= WinHeight - 15) && (YLoc < WinHeight))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				WindowResized(Window);
			}
		 else
			{
				TextEditDoMouseDown(Window->TheEdit,XLoc,YLoc,Modifiers);
			}
	}


void								WindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		if (KeyCode == 3)
			{
				TextEditShowSelection(Window->TheEdit);
			}
		else if (((KeyCode >= 32) && ((Modifiers & eCommandKey) == 0)) || (KeyCode == 13)
			|| (KeyCode == eLeftArrow) || (KeyCode == eRightArrow) || (KeyCode == eUpArrow)
			|| (KeyCode == eDownArrow) || (KeyCode == 8) || (KeyCode == 9))
			{
				TextEditDoKeyPressed(Window->TheEdit,KeyCode,Modifiers);
			}
	}


void								WindowClose(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		CloseDocument(Window);
	}


void								WindowUpdate(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->TheEdit);
		SetClipRect(Window->ScreenID,0,0,GetWindowWidth(Window->ScreenID),
			GetWindowHeight(Window->ScreenID));
		DrawBitmap(Window->ScreenID,GetWindowWidth(Window->ScreenID) - 15,
			GetWindowHeight(Window->ScreenID) - 15,
			GetGrowIcon(Window->GenericWindow == GetCurrentWindowID()));
	}


void								WindowMenuSetup(MyWinRec* Window)
	{
		char*							StrNumber;
		long							Scan;
		long							Limit;

		CheckPtrExistence(Window);
		ChangeItemName(mCloseFile,"Close File");
		EnableMenuItem(mCloseFile);
		EnableMenuItem(mSaveAs);
		if (Window->NeedsToBeSaved || TextEditDoesItNeedToBeSaved(Window->TheEdit))
			{
				EnableMenuItem(mSaveFile);
			}
		EnableMenuItem(mSetTabSize);
		EnableMenuItem(mMacintoshLineFeeds);
		EnableMenuItem(mUnixLineFeeds);
		EnableMenuItem(mMsDosLineFeeds);
		switch (Window->LineFeed)
			{
				default:
					EXECUTE(PRERR(AllowResume,"WindowMenuSetup:  unknown line feed type"));
					break;
				case eMacLF:
					SetItemCheckmark(mMacintoshLineFeeds);
					break;
				case eUnixLF:
					SetItemCheckmark(mUnixLineFeeds);
					break;
				case eDosLF:
					SetItemCheckmark(mMsDosLineFeeds);
					break;
			}

		StrNumber = IntegerToString(GetTextEditSpacesPerTab(Window->TheEdit));
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

		EnableMenuItem(mConvertTabsToSpaces);
		EnableMenuItem(mPaste);
		EnableMenuItem(mAutoIndent);
		if (TextEditIsAutoIndentEnabled(Window->TheEdit))
			{
				SetItemCheckmark(mAutoIndent);
			}
		if (TextEditIsThereValidSelection(Window->TheEdit))
			{
				EnableMenuItem(mCut);
				EnableMenuItem(mCopy);
				EnableMenuItem(mClear);
				EnableMenuItem(mEnterSelection);
				EnableMenuItem(mReplace);
				if (PtrSize(SearchKey) != 0)
					{
						EnableMenuItem(mReplaceAndFindAgain);
					}
			}
		EnableMenuItem(mShiftLeft);
		EnableMenuItem(mShiftRight);
		EnableMenuItem(mBalanceParens);
		EnableMenuItem(mSelectAll);
		EnableMenuItem(mFind);
		if (PtrSize(SearchKey) != 0)
			{
				EnableMenuItem(mFindAgain);
			}
		EnableMenuItem(mShowSelection);
		Limit = PtrSize((char*)mFontItemList) / sizeof(MenuToFont);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				EnableMenuItem(mFontItemList[Scan].MenuItemID);
				if (mFontItemList[Scan].FontID == Window->TheFont)
					{
						SetItemCheckmark(mFontItemList[Scan].MenuItemID);
					}
			}
		EnableMenuItem(m9Points);
		EnableMenuItem(m10Points);
		EnableMenuItem(m12Points);
		EnableMenuItem(m14Points);
		EnableMenuItem(m18Points);
		EnableMenuItem(m24Points);
		EnableMenuItem(m30Points);
		EnableMenuItem(m36Points);
		EnableMenuItem(mOtherPoints);

		StrNumber = IntegerToString(Window->FontSize);
		if (StrNumber != NIL)
			{
				char*							StrKey;

				StrKey = StringToBlockCopy("_");
				if (StrKey != NIL)
					{
						char*							StrValue;

						StrValue = StringToBlockCopy("Other Point Size... (_)");
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
												ChangeItemName(mOtherPoints,StrResult);
											}
										ReleasePtr(StrResult);
									}
								ReleasePtr(StrValue);
							}
						ReleasePtr(StrKey);
					}
				ReleasePtr(StrNumber);
			}

		switch (Window->FontSize)
			{
				case 9:  SetItemCheckmark(m9Points); break;
				case 10:  SetItemCheckmark(m10Points); break;
				case 12:  SetItemCheckmark(m12Points); break;
				case 14:  SetItemCheckmark(m14Points); break;
				case 18:  SetItemCheckmark(m18Points); break;
				case 24:  SetItemCheckmark(m24Points); break;
				case 30:  SetItemCheckmark(m30Points); break;
				case 36:  SetItemCheckmark(m36Points); break;
				default:  SetItemCheckmark(mOtherPoints); break;
			}
		Limit = ArrayGetLength(MasterWindowList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				MyWinRec*					OtherWindow;

				OtherWindow = ArrayGetElement(MasterWindowList,Scan);
				if (OtherWindow != NIL)
					{
						EnableMenuItem(OtherWindow->WindowMenuEntry);
						if (OtherWindow == Window)
							{
								SetItemCheckmark(OtherWindow->WindowMenuEntry);
							}
					}
			}
		EnableMenuItem(mGotoLine);

		StrNumber = IntegerToString(GetTextEditSelectStartLine(Window->TheEdit) + 1);
		if (StrNumber != NIL)
			{
				char*							StrKey;

				StrKey = StringToBlockCopy("_");
				if (StrKey != NIL)
					{
						char*							StrValue;

						StrValue = StringToBlockCopy("Goto Line... (_)");
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
												ChangeItemName(mGotoLine,StrResult);
											}
										ReleasePtr(StrResult);
									}
								ReleasePtr(StrValue);
							}
						ReleasePtr(StrKey);
					}
				ReleasePtr(StrNumber);
			}

		EnableMenuItem(mPrefixSelection);
		if (TextEditCanWeUndo(Window->TheEdit))
			{
				EnableMenuItem(mUndo);
			}
	}


void								WindowDoMenuCommand(MyWinRec* Window, MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		CheckPtrExistence(MenuItem);
		if (MenuItem == mCloseFile)
			{
				WindowClose(Window);
			}
		else if (MenuItem == mSaveFile)
			{
				Save(Window);
			}
		else if (MenuItem == mSaveAs)
			{
				SaveAs(Window);
			}
		else if (MenuItem == mSetTabSize)
			{
				long						NewTabCount;

				NewTabCount = DoNumberDialog("Enter new tab size:",
					GetTextEditSpacesPerTab(Window->TheEdit),mCut,mPaste,mCopy,mUndo,
					mSelectAll,mClear);
				if (NewTabCount < MINTABCOUNT)
					{
						NewTabCount = MINTABCOUNT;
					}
				if (NewTabCount > MAXTABCOUNT)
					{
						NewTabCount = MAXTABCOUNT;
					}
				SetTextEditTabSize(Window->TheEdit,NewTabCount);
				TextEditFullRedraw(Window->TheEdit);
			}
		else if (MenuItem == mConvertTabsToSpaces)
			{
				TextEditConvertTabsToSpaces(Window->TheEdit);
			}
		else if (MenuItem == mMacintoshLineFeeds)
			{
				Window->LineFeed = eMacLF;
				Window->NeedsToBeSaved = True;
			}
		else if (MenuItem == mUnixLineFeeds)
			{
				Window->LineFeed = eUnixLF;
				Window->NeedsToBeSaved = True;
			}
		else if (MenuItem == mMsDosLineFeeds)
			{
				Window->LineFeed = eDosLF;
				Window->NeedsToBeSaved = True;
			}
		else if (MenuItem == mUndo)
			{
				TextEditDoMenuUndo(Window->TheEdit);
				TextEditShowSelection(Window->TheEdit);
			}
		else if (MenuItem == mCut)
			{
				TextEditDoMenuCut(Window->TheEdit);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->TheEdit);
			}
		else if (MenuItem == mPaste)
			{
				TextEditDoMenuPaste(Window->TheEdit);
			}
		else if (MenuItem == mClear)
			{
				TextEditDoMenuClear(Window->TheEdit);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->TheEdit);
			}
		else if (MenuItem == mEnterSelection)
			{
				char*						NewString;

				NewString = TextEditGetSelection(Window->TheEdit);
				if (NewString != NIL)
					{
						ReleasePtr(SearchKey);
						SearchKey = NewString;
					}
			}
		else if (MenuItem == mFind)
			{
				switch (DoFindDialog(&SearchKey,&ReplaceThang,mCut,mPaste,mCopy,mUndo,
					mSelectAll,mClear))
					{
						case eFindCancel:
							break;
						case eFindFromStart:
							SetTextEditInsertionPoint(Window->TheEdit,0,0); /* reset selection */
							FindAgain(Window);
							break;
						case eFindAgain:
							FindAgain(Window);
							break;
						case eDontFind:
							break;
						default:
							EXECUTE(PRERR(AllowResume,"FindSomething: Unknown find opcode"));
							break;
					}
			}
		else if (MenuItem == mFindAgain)
			{
				FindAgain(Window);
			}
		else if (MenuItem == mReplace)
			{
				Replace(Window);
			}
		else if (MenuItem == mReplaceAndFindAgain)
			{
				if (Replace(Window))
					{
						FindAgain(Window);
					}
			}
		else if (MenuItem == mShiftLeft)
			{
				TextEditShiftSelectionLeftOneTab(Window->TheEdit);
			}
		else if (MenuItem == mShiftRight)
			{
				TextEditShiftSelectionRightOneTab(Window->TheEdit);
			}
		else if (MenuItem == mShowSelection)
			{
				TextEditShowSelection(Window->TheEdit);
			}
		else if (ItsInTheFontList(MenuItem))
			{
				Window->TheFont = GetFontFromMenuItem(MenuItem);
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m9Points)
			{
				Window->FontSize = 9;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m10Points)
			{
				Window->FontSize = 10;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m12Points)
			{
				Window->FontSize = 12;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m14Points)
			{
				Window->FontSize = 14;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m18Points)
			{
				Window->FontSize = 18;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m24Points)
			{
				Window->FontSize = 24;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m30Points)
			{
				Window->FontSize = 30;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == m36Points)
			{
				Window->FontSize = 36;
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (MenuItem == mOtherPoints)
			{
				Window->FontSize = DoNumberDialog("Enter new point size:",Window->FontSize,
					mCut,mPaste,mCopy,mUndo,mSelectAll,mClear);
				if (Window->FontSize < 4)
					{
						Window->FontSize = 4;
					}
				if (Window->FontSize > 127)
					{
						Window->FontSize = 127;
					}
				SetTextEditFontStuff(Window->TheEdit,Window->TheFont,Window->FontSize);
			}
		else if (mAutoIndent == MenuItem)
			{
				SetTextEditAutoIndent(Window->TheEdit,
					!TextEditIsAutoIndentEnabled(Window->TheEdit));
			}
		else if (mBalanceParens == MenuItem)
			{
				TextEditBalanceParens(Window->TheEdit);
			}
		else if (mGotoLine == MenuItem)
			{
				long						Line;

				Line = DoNumberDialog("Goto Line:",GetTextEditSelectStartLine(
					Window->TheEdit) + 1,mCut,mPaste,mCopy,mUndo,mSelectAll,mClear);
				Line -= 1;
				if (Line < 0)
					{
						Line = 0;
					}
				if (Line > GetTextEditNumLines(Window->TheEdit) - 1)
					{
						Line = GetTextEditNumLines(Window->TheEdit) - 1;
					}
				SetTextEditInsertionPoint(Window->TheEdit,Line,0);
				TextEditShowSelection(Window->TheEdit);
			}
		else if (mPrefixSelection == MenuItem)
			{
				if (DoStringDialog("Enter line prefix:",&PrefixPtr,
					mCut,mPaste,mCopy,mUndo,mSelectAll,mClear))
					{
						long						Scan;
						long						Limit;
						long						PrefixLen;

						Limit = GetTextEditSelectEndLine(Window->TheEdit);
						if (GetTextEditSelectEndCharPlusOne(Window->TheEdit) == 0)
							{
								Limit -= 1;
							}
						PrefixLen = PtrSize(PrefixPtr);
						for (Scan = GetTextEditSelectStartLine(Window->TheEdit);
							Scan <= Limit; Scan += 1)
							{
								char*						Line;
								char*						LineCopy;
								long						LineLen;

								Line = GetTextEditLine(Window->TheEdit,Scan);
								if (Line != NIL)
									{
										LineLen = PtrSize(PrefixPtr);
										LineCopy = InsertBlockIntoBlockCopy(Line,PrefixPtr,0,LineLen);
										if (LineCopy != NIL)
											{
												SetTextEditLine(Window->TheEdit,Scan,LineCopy);
												ReleasePtr(LineCopy);
											}
										ReleasePtr(Line);
									}
							}
					}
			}
		else if (NIL != WhichWindowMenuItem(MenuItem))
			{
				ActivateThisWindow((WhichWindowMenuItem(MenuItem))->ScreenID);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"Unimplemented menu command chosen"));
			}
	}


void								FindAgain(MyWinRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditFindAgain(Window->TheEdit,SearchKey);
		TextEditShowSelection(Window->TheEdit);
	}


MyBoolean						Replace(MyWinRec* Window)
	{
		if (TextEditIsThereValidSelection(Window->TheEdit))
			{
				TextEditInsertRawData(Window->TheEdit,ReplaceThang,SYSTEMLINEFEED);
				return True;
			}
		 else
			{
				return False;
			}
	}


MyWinRec*						WhichWindowMenuItem(MenuItemType* TheItem)
	{
		long							Scan;
		long							Limit;

		Limit = ArrayGetLength(MasterWindowList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				MyWinRec*					WinTemp;

				WinTemp = ArrayGetElement(MasterWindowList,Scan);
				if ((WinTemp != NIL) && (WinTemp->WindowMenuEntry == TheItem))
					{
						return WinTemp;
					}
			}
		return NIL;
	}
