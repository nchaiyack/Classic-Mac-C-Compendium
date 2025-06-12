/* EditWindowRec.c */
/*****************************************************************************/
/*                                                                           */
/*    Offline USENET News Viewer                                             */
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

#include "EditWindowRec.h"
#include "TextEdit.h"
#include "Memory.h"
#include "GrowIcon.h"
#include "Main.h"
#include "WindowDispatcher.h"
#include "Menus.h"
#include "Displayer.h"
#include "DataMunging.h"
#include "Numbers.h"
#include "FindDialog.h"
#include "NumberDialog.h"
#include "Files.h"
#include "Alert.h"


#define POSITIONS (5)
#define HOFFSET (10)
#define VOFFSET (20)


struct EditWindowRec
	{
		WinType*						ScreenID;
		TextEditRec*				EditBox;
		long								PlacementIndex;
		GenericWindowRec*		GenericWindow;
		DisplayWindowRec*		Owner;
	};


static long							PlacementArray[POSITIONS];

static char*						FindString = NIL;
static char*						ReplaceString = NIL;


/* initialize internal static structures for edit window */
MyBoolean							InitializeEditWindow(void)
	{
		FindString = AllocPtrCanFail(0,"FindString");
		if (FindString == NIL)
			{
			 FailurePoint1:
				return False;
			}
		ReplaceString = AllocPtrCanFail(0,"ReplaceString");
		if (ReplaceString == NIL)
			{
			 FailurePoint2:
				ReleasePtr(FindString);
				goto FailurePoint1;
			}
		return True;
	}


/* dispose of internal static structures for edit window */
void									ShutdownEditWindow(void)
	{
		ReleasePtr(FindString);
		ReleasePtr(ReplaceString);
	}


/* find a place to put the window */
static long						PlaceWindow(void)
	{
		long								SmallestIndex;
		long								Entries;
		long								Scan;

		SmallestIndex = 0;
		Entries = 0x7fffffff;
		for (Scan = 0; Scan < POSITIONS; Scan += 1)
			{
				if (Entries > PlacementArray[Scan])
					{
						Entries = PlacementArray[Scan];
						SmallestIndex = Scan;
					}
			}
		return SmallestIndex;
	}


/* create a new editing window */
EditWindowRec*				NewEditWindow(char* DataBlock, char* LineFeed, char* WindowName,
												struct DisplayWindowRec* Owner)
	{
		EditWindowRec*			Window;
		OrdType							LeftEdge;
		OrdType							TopEdge;
		char*								TempStr;

		Window = (EditWindowRec*)AllocPtrCanFail(sizeof(EditWindowRec),"EditWindowRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Window->Owner = Owner;
		Window->PlacementIndex = PlaceWindow();
		LeftEdge = 2 + HOFFSET * (POSITIONS - Window->PlacementIndex - 1);
		TopEdge = 2 + VOFFSET * Window->PlacementIndex;
		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,eWindowZoomable,
			eWindowResizable,LeftEdge + WindowOtherEdgeWidths(eDocumentWindow),TopEdge
			+ WindowTitleBarHeight(eDocumentWindow),GetScreenWidth() - 2
			* WindowOtherEdgeWidths(eDocumentWindow) - LeftEdge - 1,GetScreenHeight()
			- 2 - TopEdge - WindowOtherEdgeWidths(eDocumentWindow)
			- WindowTitleBarHeight(eDocumentWindow),(void (*)(void*))&EditWindowDoUpdate,
			Window);
		if (Window->ScreenID == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		TempStr = BlockToStringCopy(WindowName);
		if (TempStr != NIL)
			{
				SetWindowName(Window->ScreenID,TempStr);
				ReleasePtr(TempStr);
			}
		Window->EditBox = NewTextEdit(Window->ScreenID,eTEVScrollBar | eTEHScrollBar,
			GetMonospacedFont(),9,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
		if (Window->EditBox == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		if (!TextEditNewRawData(Window->EditBox,DataBlock,LineFeed))
			{
			 FailurePoint4:
				DisposeTextEdit(Window->EditBox);
				goto FailurePoint3;
			}
		Window->GenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&EditWindowDoIdle,
			(void (*)(void*))&EditWindowBecomeActive,
			(void (*)(void*))&EditWindowBecomeInactive,
			(void (*)(void*))&EditWindowResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&EditWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&EditWindowDoKeyDown,
			(void (*)(void*))&EditWindowClose,
			(void (*)(void*))&EditWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&EditWindowDoMenuCommand);
		if (Window->GenericWindow == NIL)
			{
			 FailurePoint5:
				goto FailurePoint4;
			}
		if (!DisplayWindowNewArticleWindow(Owner,Window))
			{
			 FailurePoint6:
				CheckOutDyingWindow(Window->GenericWindow);
				goto FailurePoint5;
			}
		PlacementArray[Window->PlacementIndex] += 1;
		return Window;
	}


/* dispose of the editing window */
void									DisposeEditWindow(EditWindowRec* Window)
	{
		CheckPtrExistence(Window);
		PlacementArray[Window->PlacementIndex] -= 1;
		DisplayWindowArticleDead(Window->Owner,Window);
		CheckOutDyingWindow(Window->GenericWindow);
		DisposeTextEdit(Window->EditBox);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
	}


void									EditWindowDoIdle(EditWindowRec* Window,
												MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
												ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		TextEditUpdateCursor(Window->EditBox);
		if (CheckCursorFlag)
			{
				if (TextEditIBeamTest(Window->EditBox,XLoc,YLoc))
					{
						SetIBeamCursor();
					}
				 else
					{
						SetArrowCursor();
					}
			}
	}


void									EditWindowBecomeActive(EditWindowRec* Window)
	{
		OrdType							XSize;
		OrdType							YSize;

		CheckPtrExistence(Window);
		EnableTextEditSelection(Window->EditBox);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize - 15,YSize - 15,GetGrowIcon(True));
	}


void									EditWindowBecomeInactive(EditWindowRec* Window)
	{
		OrdType							XSize;
		OrdType							YSize;

		CheckPtrExistence(Window);
		DisableTextEditSelection(Window->EditBox);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize - 15,YSize - 15,GetGrowIcon(False));
	}


void									EditWindowResized(EditWindowRec* Window)
	{
		CheckPtrExistence(Window);
		SetTextEditPosition(Window->EditBox,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
	}


void									EditWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
												ModifierFlags Modifiers, EditWindowRec* Window)
	{
		CheckPtrExistence(Window);
		if (TextEditHitTest(Window->EditBox,XLoc,YLoc))
			{
				TextEditDoMouseDown(Window->EditBox,XLoc,YLoc,Modifiers);
			}
	}


void									EditWindowDoKeyDown(unsigned char KeyCode,
												ModifierFlags Modifiers, EditWindowRec* Window)
	{
		CheckPtrExistence(Window);
		switch (KeyCode)
			{
				default:
					break;
				case eLeftArrow:
				case eRightArrow:
				case eUpArrow:
				case eDownArrow:
					TextEditDoKeyPressed(Window->EditBox,KeyCode,Modifiers);
					break;
				case 3:
					TextEditShowSelection(Window->EditBox);
					break;
			}
	}


void									EditWindowClose(EditWindowRec* Window)
	{
		DisposeEditWindow(Window);
	}


void									EditWindowMenuSetup(EditWindowRec* Window)
	{
		char*								StrNumber;

		CheckPtrExistence(Window);
		EnableMenuItem(mCloseFile);
		ChangeItemName(mCloseFile,"Close Article");
		EnableMenuItem(mCloseFile);
		if (TextEditIsThereValidSelection(Window->EditBox))
			{
				EnableMenuItem(mCopy);
				EnableMenuItem(mEnterSelection);
			}
		EnableMenuItem(mSelectAll);
		EnableMenuItem(mFind);
		if (PtrSize(FindString) != 0)
			{
				EnableMenuItem(mFindAgain);
			}
		EnableMenuItem(mShowSelection);
		EnableMenuItem(mBalanceParens);

		EnableMenuItem(mSaveArticle);
		EnableMenuItem(mAppendArticle);
		if (TextEditIsThereValidSelection(Window->EditBox))
			{
				EnableMenuItem(mSaveSelection);
				EnableMenuItem(mAppendSelection);
			}

		EnableMenuItem(mGotoLine);
		StrNumber = IntegerToString(GetTextEditSelectStartLine(Window->EditBox) + 1);
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
	}


static void						FindAgain(EditWindowRec* Window)
	{
		CheckPtrExistence(Window);
		TextEditFindAgain(Window->EditBox,FindString);
		TextEditShowSelection(Window->EditBox);
	}


void									EditWindowDoMenuCommand(EditWindowRec* Window,
												struct MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (MenuItem == mCloseFile)
			{
				DisposeEditWindow(Window);
			}
		else if (MenuItem == mCopy)
			{
				TextEditDoMenuCopy(Window->EditBox);
			}
		else if (MenuItem == mSelectAll)
			{
				TextEditDoMenuSelectAll(Window->EditBox);
			}
		else if (MenuItem == mEnterSelection)
			{
				char*						NewString;

				NewString = TextEditGetSelection(Window->EditBox);
				if (NewString != NIL)
					{
						ReleasePtr(FindString);
						FindString = NewString;
					}
			}
		else if (MenuItem == mFind)
			{
				switch (DoFindDialog(&FindString,&ReplaceString,mCut,mPaste,mCopy,mUndo,
					mSelectAll,mClear))
					{
						case eFindCancel:
							break;
						case eFindFromStart:
							SetTextEditInsertionPoint(Window->EditBox,0,0); /* reset selection */
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
		else if (mBalanceParens == MenuItem)
			{
				TextEditBalanceParens(Window->EditBox);
			}
		else if (mGotoLine == MenuItem)
			{
				long						Line;

				Line = DoNumberDialog("Goto Line:",GetTextEditSelectStartLine(
					Window->EditBox) + 1,mCut,mPaste,mCopy,mUndo,mSelectAll,mClear);
				Line -= 1;
				if (Line < 0)
					{
						Line = 0;
					}
				if (Line > GetTextEditNumLines(Window->EditBox) - 1)
					{
						Line = GetTextEditNumLines(Window->EditBox) - 1;
					}
				SetTextEditInsertionPoint(Window->EditBox,Line,0);
				TextEditShowSelection(Window->EditBox);
			}
		else if (MenuItem == mShowSelection)
			{
				TextEditShowSelection(Window->EditBox);
			}
		else if (MenuItem == mSaveArticle)
			{
				FileSpec*						WhereToSave;

				WhereToSave = PutFile("");
				if (WhereToSave != NIL)
					{
						FileType*						FileDescriptor;

						if (!CreateFile(WhereToSave,CODE4BYTES('?','?','?','?'),
							CODE4BYTES('T','E','X','T')))
							{
								AlertHalt("Unable to create file.",NIL);
							 SaveArticleFailurePoint1:
								DisposeFileSpec(WhereToSave);
								return;
							}
						if (!OpenFile(WhereToSave,&FileDescriptor,eReadAndWrite))
							{
								AlertHalt("Unable to open file for writing.",NIL);
							 SaveArticleFailurePoint2:
								goto SaveArticleFailurePoint1;
							}
						if (!TestEditWriteDataToFile(Window->EditBox,FileDescriptor,SYSTEMLINEFEED))
							{
								AlertHalt("Unable to write data to the file.",NIL);
							 SaveArticleFailurePoint3:
								CloseFile(FileDescriptor);
								goto SaveArticleFailurePoint2;
							}
						CloseFile(FileDescriptor);
						DisposeFileSpec(WhereToSave);
					}
			}
		else if (MenuItem == mAppendArticle)
			{
				FileSpec*						WhereToSave;
				unsigned long				FileTypeArray[1] = {CODE4BYTES('T','E','X','T')};

				WhereToSave = GetFileStandard(1,FileTypeArray);
				if (WhereToSave != NIL)
					{
						FileType*						FileDescriptor;
						long								OldFileLength;

						if (!OpenFile(WhereToSave,&FileDescriptor,eReadAndWrite))
							{
								AlertHalt("Unable to open file for writing.",NIL);
							 AppendArticleFailurePoint1:
								DisposeFileSpec(WhereToSave);
								return;
							}
						OldFileLength = GetFileLength(FileDescriptor);
						if (!SetFilePosition(FileDescriptor,OldFileLength))
							{
								AlertHalt("Couldn't seek to end of file.",NIL);
							 AppendArticleFailurePoint2:
								CloseFile(FileDescriptor);
								goto AppendArticleFailurePoint1;
							}
						if (!TestEditWriteDataToFile(Window->EditBox,FileDescriptor,SYSTEMLINEFEED))
							{
								AlertHalt("Unable to append data to the file.",NIL);
							 AppendArticleFailurePoint3:
								/* undo any damage we might have done */
								(void)SetFileLength(FileDescriptor,OldFileLength);
								goto AppendArticleFailurePoint2;
							}
						CloseFile(FileDescriptor);
						DisposeFileSpec(WhereToSave);
					}
			}
		else if (MenuItem == mSaveSelection)
			{
				char*								DataBlock;
				FileSpec*						WhereToSave;

				DataBlock = TextEditGetSelection(Window->EditBox);
				if (DataBlock == NIL)
					{
						AlertHalt("There is not enough memory available to save selection.",NIL);
					 SaveSelectionFailurePoint1:
						return;
					}
				WhereToSave = PutFile("");
				if (WhereToSave != NIL)
					{
						FileType*						FileDescriptor;

						if (!CreateFile(WhereToSave,CODE4BYTES('?','?','?','?'),
							CODE4BYTES('T','E','X','T')))
							{
								AlertHalt("Unable to create file.",NIL);
							 SaveSelectionFailurePoint2:
								ReleasePtr(DataBlock);
								DisposeFileSpec(WhereToSave);
								goto SaveSelectionFailurePoint1;
							}
						if (!OpenFile(WhereToSave,&FileDescriptor,eReadAndWrite))
							{
								AlertHalt("Unable to open file for writing.",NIL);
							 SaveSelectionFailurePoint3:
								goto SaveSelectionFailurePoint2;
							}
						if (0 != WriteToFile(FileDescriptor,DataBlock,PtrSize(DataBlock)))
							{
								AlertHalt("Unable to write data to the file.",NIL);
							 SaveSelectionFailurePoint4:
								CloseFile(FileDescriptor);
								goto SaveSelectionFailurePoint3;
							}
						CloseFile(FileDescriptor);
						DisposeFileSpec(WhereToSave);
					}
				ReleasePtr(DataBlock);
			}
		else if (MenuItem == mAppendSelection)
			{
				char*								DataBlock;
				FileSpec*						WhereToSave;
				unsigned long				FileTypeArray[1] = {CODE4BYTES('T','E','X','T')};

				DataBlock = TextEditGetSelection(Window->EditBox);
				if (DataBlock == NIL)
					{
						AlertHalt("There is not enough memory available to append selection.",NIL);
					 AppendSelectionFailurePoint1:
						return;
					}
				WhereToSave = GetFileStandard(1,FileTypeArray);
				if (WhereToSave != NIL)
					{
						FileType*						FileDescriptor;
						long								OldFileLength;

						if (!OpenFile(WhereToSave,&FileDescriptor,eReadAndWrite))
							{
								AlertHalt("Unable to open file for writing.",NIL);
							 AppendSelectionFailurePoint2:
								DisposeFileSpec(WhereToSave);
								ReleasePtr(DataBlock);
								goto AppendSelectionFailurePoint1;
							}
						OldFileLength = GetFileLength(FileDescriptor);
						if (!SetFilePosition(FileDescriptor,OldFileLength))
							{
								AlertHalt("Couldn't seek to end of file.",NIL);
							 AppendSelectionFailurePoint3:
								CloseFile(FileDescriptor);
								goto AppendSelectionFailurePoint2;
							}
						if (0 != WriteToFile(FileDescriptor,DataBlock,PtrSize(DataBlock)))
							{
								AlertHalt("Unable to append data to the file.",NIL);
							 AppendSelectionFailurePoint4:
								/* undo any damage we might have done */
								(void)SetFileLength(FileDescriptor,OldFileLength);
								goto AppendSelectionFailurePoint3;
							}
						CloseFile(FileDescriptor);
						DisposeFileSpec(WhereToSave);
					}
				ReleasePtr(DataBlock);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"EditWindowDoMenuCommand:  unknown menu command"));
			}
	}


void									EditWindowDoUpdate(EditWindowRec* Window)
	{
		OrdType							XSize;
		OrdType							YSize;

		CheckPtrExistence(Window);
		TextEditFullRedraw(Window->EditBox);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		SetClipRect(Window->ScreenID,XSize - 15,YSize - 15,XSize,YSize);
		DrawBitmap(Window->ScreenID,XSize - 15,YSize - 15,
			GetGrowIcon(Window->GenericWindow == GetCurrentWindowID()));
	}
