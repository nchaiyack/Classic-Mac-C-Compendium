/* FindDialog.c */
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

#include "FindDialog.h"
#include "Screen.h"
#include "EventLoop.h"
#include "SimpleButton.h"
#include "TextEdit.h"
#include "Memory.h"
#include "Alert.h"
#include "Menus.h"


#define WIDTH (490)
#define HEIGHT (114)

#define SEARCHPROMPTX (13)
#define SEARCHPROMPTY (11)

#define SEARCHEDITX (114)
#define SEARCHEDITY (8)
#define SEARCHEDITWIDTH (364)
#define SEARCHEDITHEIGHT (22)

#define REPLACEPROMPTX (13)
#define REPLACEPROMPTY (37)

#define REPLACEEDITX (114)
#define REPLACEEDITY (34)
#define REPLACEEDITWIDTH (364)
#define REPLACEEDITHEIGHT (22)

#define FROMSTARTX (6)
#define FROMSTARTY (74)
#define FROMSTARTWIDTH (111)
#define FROMSTARTHEIGHT (21)

#define FINDAGAINX (128)
#define FINDAGAINY (74)
#define FINDAGAINWIDTH (111)
#define FINDAGAINHEIGHT (21)

#define DONTFINDX (249)
#define DONTFINDY (74)
#define DONTFINDWIDTH (111)
#define DONTFINDHEIGHT (21)

#define CANCELX (369)
#define CANCELY (74)
#define CANCELWIDTH (111)
#define CANCELHEIGHT (21)


typedef struct
	{
		WinType*							ScreenID;
		TextEditRec*					StringToFind;
		TextEditRec*					StringToReplace;
		TextEditRec*					ActiveTextEdit;
		SimpleButtonRec*			FindFromStartButton;
		SimpleButtonRec*			FindAgainButton;
		SimpleButtonRec*			DontFindButton;
		SimpleButtonRec*			CancelButton;
	} FindDialogRec;


static void		LocalUpdate(FindDialogRec* Window)
	{
		CheckPtrExistence(Window);
		SetClipRect(Window->ScreenID,0,0,WIDTH,HEIGHT);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Search for:",11,
			SEARCHPROMPTX,SEARCHPROMPTY,ePlain);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,"Replace with:",13,
			REPLACEPROMPTX,REPLACEPROMPTY,ePlain);
		RedrawSimpleButton(Window->FindFromStartButton);
		RedrawSimpleButton(Window->FindAgainButton);
		RedrawSimpleButton(Window->DontFindButton);
		RedrawSimpleButton(Window->CancelButton);
		TextEditFullRedraw(Window->StringToFind);
		TextEditFullRedraw(Window->StringToReplace);
	}


FindOpType		DoFindDialog(char** SearchKey, char** Replace,
								MenuItemType* MCut, MenuItemType* MPaste, MenuItemType* MCopy,
								MenuItemType* MUndo, MenuItemType* MSelectAll, MenuItemType* MClear)
	{
		FindOpType					ReturnCode;
		FindDialogRec*			Window;

		/* make sure the things exist */
		CheckPtrExistence(*SearchKey);
		CheckPtrExistence(*Replace);

		Window = (FindDialogRec*)AllocPtrCanFail(sizeof(FindDialogRec),"FindDialogRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory to display the dialog box.",NIL);
				return eFindCancel;
			}

		Window->ScreenID = MakeNewWindow(eDialogWindow,eWindowNotClosable,eWindowNotZoomable,
			eWindowNotResizable,DialogLeftEdge(WIDTH),DialogTopEdge(HEIGHT),WIDTH,HEIGHT,
			(void (*)(void*))&LocalUpdate,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}

		Window->StringToFind = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),
			9,SEARCHEDITX,SEARCHEDITY,SEARCHEDITWIDTH,SEARCHEDITHEIGHT);
		if (Window->StringToFind == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		TextEditNewRawData(Window->StringToFind,*SearchKey,SYSTEMLINEFEED);
		TextEditDoMenuSelectAll(Window->StringToFind);
		Window->ActiveTextEdit = Window->StringToFind;
		EnableTextEditSelection(Window->ActiveTextEdit);

		Window->StringToReplace = NewTextEdit(Window->ScreenID,eTENoScrollBars,
			GetScreenFont(),9,REPLACEEDITX,REPLACEEDITY,REPLACEEDITWIDTH,REPLACEEDITHEIGHT);
		if (Window->StringToReplace == NIL)
			{
			 FailurePoint4:
				DisposeTextEdit(Window->StringToFind);
				goto FailurePoint3;
			}
		TextEditNewRawData(Window->StringToReplace,*Replace,SYSTEMLINEFEED);
		TextEditDoMenuSelectAll(Window->StringToReplace);

		Window->FindFromStartButton = NewSimpleButton(Window->ScreenID,"Find From Start",
			FROMSTARTX,FROMSTARTY,FROMSTARTWIDTH,FROMSTARTHEIGHT);
		if (Window->FindFromStartButton == NIL)
			{
			 FailurePoint5:
				DisposeTextEdit(Window->StringToReplace);
				goto FailurePoint4;
			}

		Window->FindAgainButton = NewSimpleButton(Window->ScreenID,"Find Again",
			FINDAGAINX,FINDAGAINY,FINDAGAINWIDTH,FINDAGAINHEIGHT);
		if (Window->FindAgainButton == NIL)
			{
			 FailurePoint6:
				DisposeSimpleButton(Window->FindFromStartButton);
				goto FailurePoint5;
			}
		SetDefaultButtonState(Window->FindAgainButton,True);

		Window->DontFindButton = NewSimpleButton(Window->ScreenID,"Don't Find",
			DONTFINDX,DONTFINDY,DONTFINDWIDTH,DONTFINDHEIGHT);
		if (Window->DontFindButton == NIL)
			{
			 FailurePoint7:
				DisposeSimpleButton(Window->FindAgainButton);
				goto FailurePoint6;
			}

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",
			CANCELX,CANCELY,CANCELWIDTH,CANCELHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint8:
				DisposeSimpleButton(Window->DontFindButton);
				goto FailurePoint7;
			}

		/* now do our loco event loop */
		while (1)
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
							if (TextEditIBeamTest(Window->StringToFind,X,Y)
								|| (TextEditIBeamTest(Window->StringToReplace,X,Y)))
								{
									SetIBeamCursor();
								}
							 else
								{
									SetArrowCursor();
								}
							goto NoEventPoint;
							break;
						case eNoEvent:
						 NoEventPoint:
							TextEditUpdateCursor(Window->ActiveTextEdit);
							break;
						case eMenuStarting:
							EnableMenuItem(MPaste);
							if (TextEditIsThereValidSelection(Window->ActiveTextEdit))
								{
									EnableMenuItem(MCut);
									EnableMenuItem(MCopy);
									EnableMenuItem(MClear);
								}
							EnableMenuItem(MSelectAll);
							if (TextEditCanWeUndo(Window->ActiveTextEdit))
								{
									EnableMenuItem(MUndo);
								}
							break;
						case eMenuCommand:
							if (MenuItem == MPaste)
								{
									TextEditDoMenuPaste(Window->ActiveTextEdit);
								}
							else if (MenuItem == MCut)
								{
									TextEditDoMenuCut(Window->ActiveTextEdit);
								}
							else if (MenuItem == MCopy)
								{
									TextEditDoMenuCopy(Window->ActiveTextEdit);
								}
							else if (MenuItem == MClear)
								{
									TextEditDoMenuClear(Window->ActiveTextEdit);
								}
							else if (MenuItem == MUndo)
								{
									TextEditDoMenuUndo(Window->ActiveTextEdit);
									TextEditShowSelection(Window->ActiveTextEdit);
								}
							else if (MenuItem == MSelectAll)
								{
									TextEditDoMenuSelectAll(Window->ActiveTextEdit);
								}
							else
								{
									EXECUTE(PRERR(AllowResume,
										"DoFindDialog: Undefined menu option chosen"));
								}
							break;
						case eKeyPressed:
							if (KeyPress == 13)
								{
									if ((Modifiers & eCommandKey) != 0)
										{
											/* command-return means actually interpret the return key */
											/* in this implementation, multi-line search keys aren't */
											/* supported, but in case we get around to changing it, */
											/* we have this bit of code here */
											/* TextEditDoKeyPressed(Window->ActiveTextEdit,KeyPress, */
											/* Modifiers & ~eCommandKey); */
										}
									 else
										{
											/* otherwise, leave the box & do the thing. */
											FlashButton(Window->FindAgainButton);
											ReturnCode = eFindAgain;
											goto AllDonePoint;
										}
								}
							else if (KeyPress == 9)
								{
									if ((Modifiers & eCommandKey) != 0)
										{
											/* command-tab means actually interpret the return key */
											TextEditDoKeyPressed(Window->ActiveTextEdit,KeyPress,
												Modifiers & ~eCommandKey);
										}
									 else
										{
											/* otherwise, switch to the other text entry box */
											DisableTextEditSelection(Window->ActiveTextEdit);
											if (Window->ActiveTextEdit == Window->StringToFind)
												{
													Window->ActiveTextEdit = Window->StringToReplace;
												}
											 else
												{
													Window->ActiveTextEdit = Window->StringToFind;
												}
											TextEditDoMenuSelectAll(Window->ActiveTextEdit);
											EnableTextEditSelection(Window->ActiveTextEdit);
										}
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									ReturnCode = eFindCancel;
									goto AllDonePoint;
								}
							else
								{
									TextEditDoKeyPressed(Window->ActiveTextEdit,KeyPress,Modifiers);
								}
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window->FindFromStartButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->FindFromStartButton,X,Y,NIL,NIL))
										{
											ReturnCode = eFindFromStart;
											goto AllDonePoint;
										}
								}
							else if (SimpleButtonHitTest(Window->FindAgainButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->FindAgainButton,X,Y,NIL,NIL))
										{
											ReturnCode = eFindAgain;
											goto AllDonePoint;
										}
								}
							else if (SimpleButtonHitTest(Window->DontFindButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->DontFindButton,X,Y,NIL,NIL))
										{
											ReturnCode = eDontFind;
											goto AllDonePoint;
										}
								}
							else if (SimpleButtonHitTest(Window->CancelButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->CancelButton,X,Y,NIL,NIL))
										{
											ReturnCode = eFindCancel;
											goto AllDonePoint;
										}
								}
							else if (TextEditHitTest(Window->StringToFind,X,Y))
								{
									if (Window->ActiveTextEdit != Window->StringToFind)
										{
											DisableTextEditSelection(Window->ActiveTextEdit);
											Window->ActiveTextEdit = Window->StringToFind;
											EnableTextEditSelection(Window->ActiveTextEdit);
										}
									TextEditDoMouseDown(Window->ActiveTextEdit,X,Y,Modifiers);
								}
							else if (TextEditHitTest(Window->StringToReplace,X,Y))
								{
									if (Window->ActiveTextEdit == Window->StringToReplace)
										{
											DisableTextEditSelection(Window->ActiveTextEdit);
											Window->ActiveTextEdit = Window->StringToReplace;
											EnableTextEditSelection(Window->ActiveTextEdit);
										}
									TextEditDoMouseDown(Window->ActiveTextEdit,X,Y,Modifiers);
								}
							break;
					}
			}
	 AllDonePoint:
		if (ReturnCode != eFindCancel)
			{
				char*				NewFindStuff;
				char*				NewReplaceStuff;

				NewFindStuff = TextEditGetRawData(Window->StringToFind,SYSTEMLINEFEED);
				if (NewFindStuff == NIL)
					{
					 EndFailure1:
						ReturnCode = eFindCancel;
						goto ReleaseAndExit;
					}
				NewReplaceStuff = TextEditGetRawData(Window->StringToReplace,SYSTEMLINEFEED);
				if (NewReplaceStuff == NIL)
					{
					 EndFailure2:
						ReleasePtr(NewFindStuff);
						goto EndFailure1;
					}
				ReleasePtr(*SearchKey);
				*SearchKey = NewFindStuff;
				ReleasePtr(*Replace);
				*Replace = NewReplaceStuff;
			}
	 ReleaseAndExit:
		DisposeTextEdit(Window->StringToFind);
		DisposeTextEdit(Window->StringToReplace);
		DisposeSimpleButton(Window->FindFromStartButton);
		DisposeSimpleButton(Window->FindAgainButton);
		DisposeSimpleButton(Window->DontFindButton);
		DisposeSimpleButton(Window->CancelButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
		return ReturnCode;
	}
