/* Main.c */
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

#define Including_Main_c
#include "MiscInfo.h"
#include "Definitions.h"
#include "Debug.h"
#include "Audit.h"

#include "Main.h"
#include "Memory.h"
#include "StartupOpen.h"
#include "Files.h"
#include "EventLoop.h"
#include "Alert.h"
#include "WindowStuff.h"
#include "GrowIcon.h"
#include "AboutBox.h"
#include "WindowDispatcher.h"


MenuType*					mmAppleMenu;
MenuType*					mmFileMenu;
MenuType*					mmEditMenu;
MenuType*					mmSearchMenu;
MenuType*					mmWindowsMenu;
MenuType*					mmFontMenu;
MenuType*					mmSizeMenu;

MenuItemType*			mAboutThisProgram;
MenuItemType*			mNewFile;
MenuItemType*			mOpenFile;
MenuItemType*			mCloseFile;
MenuItemType*			mSaveFile;
MenuItemType*			mSaveAs;
MenuItemType*			mAutoIndent;
MenuItemType*			mSetTabSize;
MenuItemType*			mConvertTabsToSpaces;
MenuItemType*			mMacintoshLineFeeds;
MenuItemType*			mUnixLineFeeds;
MenuItemType*			mMsDosLineFeeds;
MenuItemType*			mQuit;
MenuItemType*			mUndo;
MenuItemType*			mCut;
MenuItemType*			mCopy;
MenuItemType*			mPaste;
MenuItemType*			mClear;
MenuItemType*			mSelectAll;
MenuItemType*			mEnterSelection;
MenuItemType*			mFind;
MenuItemType*			mFindAgain;
MenuItemType*			mReplace;
MenuItemType*			mReplaceAndFindAgain;
MenuItemType*			mPrefixSelection;
MenuItemType*			mBalanceParens;
MenuItemType*			mShiftLeft;
MenuItemType*			mShiftRight;
MenuItemType*			mGotoLine;
MenuItemType*			mShowSelection;
MenuToFont*				mFontItemList;
MenuItemType*			m9Points;
MenuItemType*			m10Points;
MenuItemType*			m12Points;
MenuItemType*			m14Points;
MenuItemType*			m18Points;
MenuItemType*			m24Points;
MenuItemType*			m30Points;
MenuItemType*			m36Points;
MenuItemType*			mOtherPoints;


MyBoolean					ItsInTheFontList(MenuItemType* MenuID)
	{
		int								Scan;

		for (Scan = 0; Scan < PtrSize((char*)mFontItemList)
			/ sizeof(MenuToFont); Scan += 1)
			{
				if (mFontItemList[Scan].MenuItemID == MenuID)
					{
						return True;
					}
			}
		return False;
	}


FontType					GetFontFromMenuItem(MenuItemType* MenuID)
	{
		int					Scan;

		for (Scan = 0; Scan < PtrSize((char*)mFontItemList)
			/ sizeof(MenuToFont); Scan += 1)
			{
				if (mFontItemList[Scan].MenuItemID == MenuID)
					{
						return mFontItemList[Scan].FontID;
					}
			}
		return 0;
	}


int								main(int argc, char* argv[])
	{
		MyBoolean				TryToOpenUntitledDocument;
		MyBoolean				ContinueFlag;


		APRINT(("+main"));

		/* level 0 initialization */
		if (!InitializeScreen())
			{
				goto InitScreenFailedPoint;
			}

		/* level 1 initialization */
		if (!InitializeAlertSubsystem())
			{
				goto InitAlertFailedPoint;
			}
		if (!InitializeGrowIcon())
			{
				goto InitGrowFailedPoint;
			}
		if (!InitializeWindowDispatcher())
			{
				goto InitWindowDispatcherFailed;
			}

		/* application initialization */
		if (!InitWindowStuff())
			{
				goto InitWindowStuffFailed;
			}

		mmAppleMenu = MakeAppleMenu();
		if (mmAppleMenu == NIL)
			{
				goto MakeAppleMenuFailed;
			}
		mAboutThisProgram = MakeNewMenuItem(mmAppleMenu,"About TextEditor...",0);
		if (mAboutThisProgram == NIL)
			{
				goto MakeFileMenuFailed;
			}
		ShowMenu(mmAppleMenu);

		mmFileMenu = MakeNewMenu("File");
		if (mmFileMenu == NIL)
			{
				goto MakeFileMenuFailed;
			}
		mNewFile = MakeNewMenuItem(mmFileMenu,"New",'N');
		if (mNewFile == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mOpenFile = MakeNewMenuItem(mmFileMenu,"Open...",'O');
		if (mOpenFile == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mCloseFile = MakeNewMenuItem(mmFileMenu,"Close",'W');
		if (mCloseFile == NIL)
			{
				goto MakeEditMenuFailed;
			}
		AppendSeparator(mmFileMenu);
		mSaveFile = MakeNewMenuItem(mmFileMenu,"Save",'S');
		if (mSaveFile == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mSaveAs = MakeNewMenuItem(mmFileMenu,"Save As...",0);
		if (mSaveAs == NIL)
			{
				goto MakeEditMenuFailed;
			}
		AppendSeparator(mmFileMenu);
		mAutoIndent = MakeNewMenuItem(mmFileMenu,"Auto Indent",0);
		if (mAutoIndent == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mSetTabSize = MakeNewMenuItem(mmFileMenu,"Set Tab Size (8)...",'T');
		if (mSetTabSize == NIL)
			{
				goto MakeEditMenuFailed;
			}
		AppendSeparator(mmFileMenu);
		mConvertTabsToSpaces = MakeNewMenuItem(mmFileMenu,"Convert Tabs To Spaces",0);
		if (mConvertTabsToSpaces == NIL)
			{
				goto MakeEditMenuFailed;
			}
		AppendSeparator(mmFileMenu);
		mMacintoshLineFeeds = MakeNewMenuItem(mmFileMenu,"Macintosh Line Feeds",0);
		if (mMacintoshLineFeeds == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mUnixLineFeeds = MakeNewMenuItem(mmFileMenu,"UNIX Line Feeds",0);
		if (mUnixLineFeeds == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mMsDosLineFeeds = MakeNewMenuItem(mmFileMenu,"MS-DOS Line Feeds",0);
		if (mMsDosLineFeeds == NIL)
			{
				goto MakeEditMenuFailed;
			}
		AppendSeparator(mmFileMenu);
		mQuit = MakeNewMenuItem(mmFileMenu,"Quit",'Q');
		if (mQuit == NIL)
			{
				goto MakeEditMenuFailed;
			}
		ShowMenu(mmFileMenu);

		mmEditMenu = MakeNewMenu("Edit");
		if (mmEditMenu == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mUndo = MakeNewMenuItem(mmEditMenu,"Undo",'Z');
		if (mUndo == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mCut = MakeNewMenuItem(mmEditMenu,"Cut",'X');
		if (mCut == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mCopy = MakeNewMenuItem(mmEditMenu,"Copy",'C');
		if (mCopy == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mPaste = MakeNewMenuItem(mmEditMenu,"Paste",'V');
		if (mPaste == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mClear = MakeNewMenuItem(mmEditMenu,"Clear",0);
		if (mClear == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mSelectAll = MakeNewMenuItem(mmEditMenu,"Select All",'A');
		if (mSelectAll == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mPrefixSelection = MakeNewMenuItem(mmEditMenu,"Prefix Selection...",0);
		if (mPrefixSelection == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mBalanceParens = MakeNewMenuItem(mmEditMenu,"Balance Parentheses",'B');
		if (mBalanceParens == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mShiftLeft = MakeNewMenuItem(mmEditMenu,"Shift Left",'L');
		if (mShiftLeft == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mShiftRight = MakeNewMenuItem(mmEditMenu,"Shift Right",'R');
		if (mShiftRight == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		ShowMenu(mmEditMenu);

		mmSearchMenu = MakeNewMenu("Search");
		if (mmSearchMenu == NIL)
			{
				goto MakeSearchMenuFailed;
			}
		mFind = MakeNewMenuItem(mmSearchMenu,"Find...",'F');
		if (mFind == NIL)
			{
				goto MakeFontMenuFailed;
			}
		mEnterSelection = MakeNewMenuItem(mmSearchMenu,"Enter Selection",'E');
		if (mEnterSelection == NIL)
			{
				goto MakeFontMenuFailed;
			}
		mFindAgain = MakeNewMenuItem(mmSearchMenu,"Find Again",'G');
		if (mFindAgain == NIL)
			{
				goto MakeFontMenuFailed;
			}
		mReplace = MakeNewMenuItem(mmSearchMenu,"Replace",'=');
		if (mReplace == NIL)
			{
				goto MakeFontMenuFailed;
			}
		mReplaceAndFindAgain = MakeNewMenuItem(mmSearchMenu,"Replace and Find Again",'H');
		if (mReplaceAndFindAgain == NIL)
			{
				goto MakeFontMenuFailed;
			}
		AppendSeparator(mmSearchMenu);
		mGotoLine = MakeNewMenuItem(mmSearchMenu,"Goto Line...",'J');
		if (mGotoLine == NIL)
			{
				goto MakeFontMenuFailed;
			}
		AppendSeparator(mmSearchMenu);
		mShowSelection = MakeNewMenuItem(mmSearchMenu,"Show Selection",0);
		if (mShowSelection == NIL)
			{
				goto MakeFontMenuFailed;
			}
		ShowMenu(mmSearchMenu);

		mmFontMenu = MakeNewMenu("Font");
		if (mmFontMenu == NIL)
			{
				goto MakeFontMenuFailed;
			}
		{
			long						Scan;
			long						Limit;

			Limit = GetNumAvailableFonts();
			mFontItemList = (MenuToFont*)AllocPtrCanFail(
				Limit * sizeof(MenuToFont),"FontMenuList");
			if (mFontItemList == NIL)
				{
				 FontFailure1:
					goto MakeSizeMenuFailed;
				}
			for (Scan = 0; Scan < Limit; Scan += 1)
				{
					char*							FontName;

					mFontItemList[Scan].FontID = GetIndexedFont(Scan);
					FontName = GetNameOfFont(mFontItemList[Scan].FontID);
					if (FontName == NIL)
						{
							long							Index;

						 FontFailure2:
							for (Index = 0; Index < Scan; Index += 1)
								{
									KillMenuItem(mFontItemList[Index].MenuItemID);
								}
							ReleasePtr((char*)mFontItemList);
							goto FontFailure1;
						}
					mFontItemList[Scan].MenuItemID = MakeNewMenuItem(mmFontMenu,FontName,0);
					ReleasePtr(FontName);
					if (mFontItemList[Scan].MenuItemID == NIL)
						{
							goto FontFailure2;
						}
				}
		}
		ShowMenu(mmFontMenu);

		mmSizeMenu = MakeNewMenu("Size");
		if (mmSizeMenu == NIL)
			{
				goto MakeSizeMenuFailed;
			}
		m9Points = MakeNewMenuItem(mmSizeMenu,"9 Points",0);
		if (m9Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m10Points = MakeNewMenuItem(mmSizeMenu,"10 Points",0);
		if (m10Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m12Points = MakeNewMenuItem(mmSizeMenu,"12 Points",0);
		if (m12Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m14Points = MakeNewMenuItem(mmSizeMenu,"14 Points",0);
		if (m14Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m18Points = MakeNewMenuItem(mmSizeMenu,"18 Points",0);
		if (m18Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m24Points = MakeNewMenuItem(mmSizeMenu,"24 Points",0);
		if (m24Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m30Points = MakeNewMenuItem(mmSizeMenu,"30 Points",0);
		if (m30Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		m36Points = MakeNewMenuItem(mmSizeMenu,"36 Points",0);
		if (m36Points == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		AppendSeparator(mmSizeMenu);
		mOtherPoints = MakeNewMenuItem(mmSizeMenu,"Other Size (9)...",0);
		if (mOtherPoints == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		ShowMenu(mmSizeMenu);

		mmWindowsMenu = MakeNewMenu("Windows");
		if (mmWindowsMenu == NIL)
			{
				goto MakeWindowsMenuFailed;
			}
		ShowMenu(mmWindowsMenu);

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
				MenuItemType*		MenuItem;
				char						KeyPress;
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
				/* handle an event */
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
							DispatchMenuStarting(Window);
							break;
						case eMenuCommand:
							CheckPtrExistence(MenuItem);
							if (MenuItem == mAboutThisProgram)
								{
									ShowAboutBox();
								}
							else if (MenuItem == mNewFile)
								{
									OpenDocument(NIL);
								}
							else if (MenuItem == mOpenFile)
								{
									FileSpec*					Where;
									unsigned long			FileTypeList[1] = {CODE4BYTES('T','E','X','T')};

									Where = GetFileStandard(1,FileTypeList);
									if (Where != NIL)
										{
											OpenDocument(Where);
										}
								}
							else if (MenuItem == mQuit)
								{
									SetQuitPending();
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
						if (DoCloseAllQuitPending())
							{
								ContinueFlag = False;
							}
						 else
							{
								AbortQuitInProgress();
							}
					}
			}
		HideAnyAboutBoxes();
		ClearStartupDocuments();
		ReleasePtr((char*)mFontItemList);

		/* application cleanup */
		KillMenuAndDeleteItems(mmWindowsMenu);
	 MakeWindowsMenuFailed:
		KillMenuAndDeleteItems(mmSizeMenu);
	 MakeSizeMenuFailed:
		KillMenuAndDeleteItems(mmFontMenu);
	 MakeFontMenuFailed:
		KillMenuAndDeleteItems(mmSearchMenu);
	 MakeSearchMenuFailed:
		KillMenuAndDeleteItems(mmEditMenu);
	 MakeEditMenuFailed:
		KillMenuAndDeleteItems(mmFileMenu);
	 MakeFileMenuFailed:
		KillMenuAndDeleteItems(mmAppleMenu);
	 MakeAppleMenuFailed:
		KillWindowStuff();

		/* level 1 cleanup */
	 InitWindowStuffFailed:
		ShutdownWindowDispatcher();
	 InitWindowDispatcherFailed:
		ShutdownGrowIcon();
	 InitGrowFailedPoint:
		ShutdownAlertSubsystem();

	 InitAlertFailedPoint:
		/* level 0 cleanup */
		ShutdownScreen();

	 InitScreenFailedPoint:
		APRINT(("-main"));
		return 0;
	}
