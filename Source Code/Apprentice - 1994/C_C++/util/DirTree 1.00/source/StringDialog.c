/* StringDialog.c */
/*****************************************************************************/
/*                                                                           */
/*    File System Tree Viewer                                                */
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

#include "StringDialog.h"
#include "Screen.h"
#include "EventLoop.h"
#include "SimpleButton.h"
#include "TextEdit.h"
#include "Memory.h"
#include "DataMunging.h"
#include "Alert.h"
#include "Menus.h"


#define HEIGHT (76)
#define WIDTH (350)

#define TEXTLEFT (10)
#define TEXTTOP (13)
#define TEXTWIDTH (120)
#define TEXTHEIGHT (20)

#define EDITLEFT (140)
#define EDITTOP (10)
#define EDITWIDTH (200)
#define EDITHEIGHT (20)

#define OKWIDTH (93)
#define OKHEIGHT (21)
#define OKLEFT (80)
#define OKTOP (44)

#define CANCELWIDTH (93)
#define CANCELHEIGHT (21)
#define CANCELLEFT (80 + 93 + 20)
#define CANCELTOP (44)


typedef struct
	{
		WinType*							ScreenID;
		SimpleButtonRec*			OKButton;
		TextEditRec*					TextField;
		char*									StaticPrompt;
		SimpleButtonRec*			CancelButton;
	} StringDialogRec;


static void		LocalUpdateRoutine(StringDialogRec* Window)
	{
		CheckPtrExistence(Window);
		SetClipRect(Window->ScreenID,0,0,WIDTH,HEIGHT);
		DrawTextLine(Window->ScreenID,GetScreenFont(),9,Window->StaticPrompt,
			StrLen(Window->StaticPrompt),TEXTLEFT,TEXTTOP,ePlain);
		RedrawSimpleButton(Window->OKButton);
		RedrawSimpleButton(Window->CancelButton);
		TextEditFullRedraw(Window->TextField);
	}


/* present a dialog box displaying the string and allowing the user to make */
/* changes. if the user clicks OK, then True is returned, else False is returned. */
MyBoolean		DoStringDialog(char* Prompt, char** TheString,
							MenuItemType* MCut, MenuItemType* MPaste, MenuItemType* MCopy,
							MenuItemType* MUndo, MenuItemType* MSelectAll, MenuItemType* MClear)
	{
		StringDialogRec*		Window;
		MyBoolean						DoItFlag;

		/* make sure the string they gave us really exists */
		CheckPtrExistence(*TheString);

		Window = (StringDialogRec*)AllocPtrCanFail(sizeof(StringDialogRec),"StringDialogRec");
		if (Window == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory to display the dialog box.",NIL);
				return False;
			}

		Window->ScreenID = MakeNewWindow(eDialogWindow,eWindowNotClosable,eWindowNotZoomable,
			eWindowNotResizable,DialogLeftEdge(WIDTH),DialogTopEdge(HEIGHT),WIDTH,HEIGHT,
			(void (*)(void*))&LocalUpdateRoutine,Window);
		if (Window->ScreenID == 0)
			{
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}

		Window->OKButton = NewSimpleButton(Window->ScreenID,"OK",OKLEFT,OKTOP,
			OKWIDTH,OKHEIGHT);
		if (Window->OKButton == NIL)
			{
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		SetDefaultButtonState(Window->OKButton,True);

		Window->CancelButton = NewSimpleButton(Window->ScreenID,"Cancel",CANCELLEFT,
			CANCELTOP,CANCELWIDTH,CANCELHEIGHT);
		if (Window->CancelButton == NIL)
			{
			 FailurePoint4:
				DisposeSimpleButton(Window->OKButton);
				goto FailurePoint3;
			}

		Window->TextField = NewTextEdit(Window->ScreenID,eTENoScrollBars,GetScreenFont(),9,
			EDITLEFT,EDITTOP,EDITWIDTH,EDITHEIGHT);
		if (Window->TextField == NIL)
			{
			 FailurePoint5:
				DisposeSimpleButton(Window->CancelButton);
				goto FailurePoint4;
			}
		TextEditNewRawData(Window->TextField,*TheString,""/*shouldn't be a linefeed*/);
		TextEditDoMenuSelectAll(Window->TextField);
		EnableTextEditSelection(Window->TextField);

		/* allocation is finished; copy over parameters */
		Window->StaticPrompt = Prompt;

		/* do our local event loop */
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
							if (TextEditIBeamTest(Window->TextField,X,Y))
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
							TextEditUpdateCursor(Window->TextField);
							break;
						case eMenuStarting:
							EnableMenuItem(MPaste);
							if (TextEditIsThereValidSelection(Window->TextField))
								{
									EnableMenuItem(MCut);
									EnableMenuItem(MCopy);
									EnableMenuItem(MClear);
								}
							EnableMenuItem(MSelectAll);
							if (TextEditCanWeUndo(Window->TextField))
								{
									EnableMenuItem(MUndo);
								}
							break;
						case eMenuCommand:
							if (MenuItem == MPaste)
								{
									TextEditDoMenuPaste(Window->TextField);
								}
							else if (MenuItem == MCut)
								{
									TextEditDoMenuCut(Window->TextField);
								}
							else if (MenuItem == MCopy)
								{
									TextEditDoMenuCopy(Window->TextField);
								}
							else if (MenuItem == MClear)
								{
									TextEditDoMenuClear(Window->TextField);
								}
							else if (MenuItem == MUndo)
								{
									TextEditDoMenuUndo(Window->TextField);
									TextEditShowSelection(Window->TextField);
								}
							else if (MenuItem == MSelectAll)
								{
									TextEditDoMenuSelectAll(Window->TextField);
								}
							else
								{
									EXECUTE(PRERR(AllowResume,
										"DoStringDialog: Undefined menu option chosen"));
								}
							break;
						case eKeyPressed:
							if (KeyPress == 13)
								{
									char*									RawTemp;

									FlashButton(Window->OKButton);
								 OKButtonClickedPoint:
									RawTemp = TextEditGetRawData(Window->TextField,
										""/*shouldn't be a linefeed*/);
									if (RawTemp != NIL)
										{
											ReleasePtr(*TheString);
											*TheString = RawTemp;
											DoItFlag = True;
										}
									 else
										{
											DoItFlag = False;
										}
									goto AllDonePoint;
								}
							else if (KeyPress == eCancelKey)
								{
									FlashButton(Window->CancelButton);
									DoItFlag = False;
									goto AllDonePoint;
								}
							else
								{
									TextEditDoKeyPressed(Window->TextField,KeyPress,Modifiers);
								}
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window->OKButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->OKButton,X,Y,NIL,NIL))
										{
											goto OKButtonClickedPoint;
										}
								}
							else if (TextEditHitTest(Window->TextField,X,Y))
								{
									TextEditDoMouseDown(Window->TextField,X,Y,Modifiers);
								}
							else if (SimpleButtonHitTest(Window->CancelButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window->CancelButton,X,Y,NIL,NIL))
										{
											DoItFlag = False;
											goto AllDonePoint;
										}
								}
							break;
					}
			}
	 AllDonePoint:
		DisposeTextEdit(Window->TextField);
		DisposeSimpleButton(Window->OKButton);
		DisposeSimpleButton(Window->CancelButton);
		KillWindow(Window->ScreenID);
		ReleasePtr((char*)Window);
		return DoItFlag;
	}
