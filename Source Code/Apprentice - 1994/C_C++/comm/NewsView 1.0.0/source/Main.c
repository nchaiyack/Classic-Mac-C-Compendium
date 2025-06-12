/* Main.c */
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

#define CompilingMainC
#include "Main.h"
#include "Menus.h"
#include "Screen.h"
#include "Alert.h"
#include "GrowIcon.h"
#include "WindowDispatcher.h"
#include "Displayer.h"
#include "StartupOpen.h"
#include "Files.h"


MenuType*					mmAppleMenu;
MenuType*					mmFileMenu;
MenuType*					mmEditMenu;

MenuItemType*			mAboutThisProgram;
MenuItemType*			mOpenFile;
MenuItemType*			mSaveArticle;
MenuItemType*			mAppendArticle;
MenuItemType*			mSaveSelection;
MenuItemType*			mAppendSelection;
MenuItemType*			mCloseFile;
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
MenuItemType*			mBalanceParens;
MenuItemType*			mGotoLine;
MenuItemType*			mShowSelection;


int								main(int argc, char* argv[])
	{
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
		if (!InitializeDisplayer())
			{
				goto InitWindowStuffFailed;
			}

		mmAppleMenu = MakeAppleMenu();
		if (mmAppleMenu == NIL)
			{
				goto MakeAppleMenuFailed;
			}
		mAboutThisProgram = MakeNewMenuItem(mmAppleMenu,"About NewsView...",0);
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
		mSaveArticle = MakeNewMenuItem(mmFileMenu,"Save Article...",'S');
		if (mSaveArticle == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mAppendArticle = MakeNewMenuItem(mmFileMenu,"Append Article To File...",0);
		if (mAppendArticle == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mSaveSelection = MakeNewMenuItem(mmFileMenu,"Save Selection...",0);
		if (mSaveSelection == NIL)
			{
				goto MakeEditMenuFailed;
			}
		mAppendSelection = MakeNewMenuItem(mmFileMenu,"Append Selection To File...",0);
		if (mAppendSelection == NIL)
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
				goto AfterMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mCut = MakeNewMenuItem(mmEditMenu,"Cut",'X');
		if (mCut == NIL)
			{
				goto AfterMenuFailed;
			}
		mCopy = MakeNewMenuItem(mmEditMenu,"Copy",'C');
		if (mCopy == NIL)
			{
				goto AfterMenuFailed;
			}
		mPaste = MakeNewMenuItem(mmEditMenu,"Paste",'V');
		if (mPaste == NIL)
			{
				goto AfterMenuFailed;
			}
		mClear = MakeNewMenuItem(mmEditMenu,"Clear",0);
		if (mClear == NIL)
			{
				goto AfterMenuFailed;
			}
		mSelectAll = MakeNewMenuItem(mmEditMenu,"Select All",'A');
		if (mSelectAll == NIL)
			{
				goto AfterMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mEnterSelection = MakeNewMenuItem(mmEditMenu,"Enter Selection",'E');
		if (mEnterSelection == NIL)
			{
				goto AfterMenuFailed;
			}
		mFind = MakeNewMenuItem(mmEditMenu,"Find...",'F');
		if (mFind == NIL)
			{
				goto AfterMenuFailed;
			}
		mFindAgain = MakeNewMenuItem(mmEditMenu,"Find Again",'G');
		if (mFindAgain == NIL)
			{
				goto AfterMenuFailed;
			}
		AppendSeparator(mmEditMenu);
		mBalanceParens = MakeNewMenuItem(mmEditMenu,"Balance Parentheses",'B');
		if (mBalanceParens == NIL)
			{
				goto AfterMenuFailed;
			}
		mGotoLine = MakeNewMenuItem(mmEditMenu,"Goto Line...",'J');
		if (mGotoLine == NIL)
			{
				goto AfterMenuFailed;
			}
		mShowSelection = MakeNewMenuItem(mmEditMenu,"Show Selection",0);
		if (mShowSelection == NIL)
			{
				goto AfterMenuFailed;
			}
		ShowMenu(mmEditMenu);

		PrepareStartupDocuments(argc,argv);
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
							}
						 else
							{
								/* if it isn't NIL, then open the actual thing */
								/* this swallows the startup item record so we don't have */
								/* do dispose of it */
								OpenDisplayWindow(StartupItem);
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
							EnableMenuItem(mOpenFile);
							EnableMenuItem(mQuit);
							DispatchMenuStarting(Window);
							break;
						case eMenuCommand:
							if (MenuItem == mAboutThisProgram)
								{
									AlertInfo("NewsView written by Thomas R. Lawrence "
										"(tomlaw@world.std.com), 1994, to be an exceedingly simple "
										"offline news reader.",NIL);
								}
							else if (MenuItem == mOpenFile)
								{
									FileSpec*					Where;

									Where = GetFileAny();
									if (Where != NIL)
										{
											OpenDisplayWindow(Where);
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
		ClearStartupDocuments();

		/* application cleanup */
	 AfterMenuFailed:
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
