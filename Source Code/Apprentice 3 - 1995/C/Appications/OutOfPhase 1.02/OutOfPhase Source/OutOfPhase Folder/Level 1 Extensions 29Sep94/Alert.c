/* Alert.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "Alert.h"
#include "Screen.h"
#include "DataMunging.h"
#include "Memory.h"
#include "EventLoop.h"
#include "SimpleButton.h"
#include "WrapTextBox.h"
#include "Menus.h"


#define ALERTEMERGENCY (20480)

#define ALERTWIDTH (400)
#define ALERTHEIGHT (160)

#define DLOGWIDTH (400)
#define DLOGHEIGHT (180)

#define BUTTONWIDTH (93)

static unsigned char				RawStopIcon[] =
	{
		0x00,0xFF,0xFF,0x00,0x01,0xFF,0xFF,0x80,0x03,0x80,0x01,0xC0,0x07,0x7F,0xFE,0xE0,
		0x0E,0xFF,0xFF,0x70,0x1D,0xFF,0xFF,0xB8,0x3B,0xFF,0xFF,0xDC,0x77,0xFF,0xFF,0xEE,
		0xEF,0xFF,0xFF,0xF7,0xDF,0xFD,0x3F,0xFB,0xDF,0xF9,0x2F,0xFB,0xDF,0xE9,0x27,0xFB,
		0xDF,0xC9,0x27,0xFB,0xDF,0xC9,0x27,0xFB,0xDF,0xC8,0x24,0xFB,0xDF,0xC0,0x04,0xFB,
		0xDF,0xC0,0x00,0xFB,0xDF,0xC0,0x01,0xFB,0xDF,0xC0,0x01,0xFB,0xDF,0xC0,0x01,0xFB,
		0xDF,0xE0,0x03,0xFB,0xDF,0xE0,0x03,0xFB,0xDF,0xF0,0x07,0xFB,0xDF,0xF8,0x0F,0xFB,
		0xEF,0xFF,0xFF,0xF7,0x77,0xFF,0xFF,0xEE,0x3B,0xFF,0xFF,0xDC,0x1D,0xFF,0xFF,0xB8,
		0x0E,0xFF,0xFF,0x70,0x07,0x00,0x00,0xE0,0x03,0xFF,0xFF,0xC0,0x01,0xFF,0xFF,0x80
	};

static unsigned char				RawExclamationIcon[] =
	{
		0x00,0x01,0x00,0x00,0x00,0x03,0x80,0x00,0x00,0x02,0x80,0x00,0x00,0x06,0xC0,0x00,
		0x00,0x04,0x40,0x00,0x00,0x0C,0x60,0x00,0x00,0x08,0x20,0x00,0x00,0x1B,0xB0,0x00,
		0x00,0x13,0x90,0x00,0x00,0x37,0xD8,0x00,0x00,0x27,0xC8,0x00,0x00,0x67,0xCC,0x00,
		0x00,0x47,0xC4,0x00,0x00,0xC7,0xC6,0x00,0x00,0x87,0xC2,0x00,0x01,0x83,0x83,0x00,
		0x01,0x03,0x81,0x00,0x03,0x03,0x81,0x80,0x02,0x03,0x80,0x80,0x06,0x03,0x80,0xC0,
		0x04,0x01,0x00,0x40,0x0C,0x01,0x00,0x60,0x08,0x01,0x00,0x20,0x18,0x01,0x00,0x30,
		0x10,0x00,0x00,0x10,0x30,0x01,0x80,0x18,0x20,0x03,0xC0,0x08,0x60,0x03,0xC0,0x0C,
		0x40,0x01,0x80,0x04,0xC0,0x00,0x00,0x06,0xFF,0xFF,0xFF,0xFE,0x7F,0xFF,0xFF,0xFC
	};

static unsigned char				RawSpeechIcon[] =
	{
		0x00,0x00,0x00,0x00,0x0F,0xFF,0xFC,0x00,0x08,0x00,0x06,0x00,0x08,0x00,0x05,0x00,
		0x08,0x00,0x04,0x80,0x08,0x00,0x04,0x40,0x08,0x00,0x04,0x20,0x08,0x00,0x07,0xF0,
		0x0B,0xE0,0x00,0x10,0x08,0x84,0xF2,0x58,0x08,0x80,0x00,0x18,0x08,0x80,0x00,0x18,
		0x08,0x86,0x79,0x98,0x08,0x80,0x00,0x18,0x0B,0xE0,0x00,0x18,0x08,0x07,0x79,0x98,
		0x08,0x00,0x00,0x18,0x08,0x00,0x00,0x18,0x0A,0xF2,0xE3,0x98,0x08,0x00,0x00,0x18,
		0x08,0x00,0x00,0x18,0x0B,0xBB,0xB2,0x58,0x08,0x00,0x00,0x18,0x08,0x00,0x00,0x18,
		0x0A,0xFD,0x79,0x98,0x08,0x00,0x00,0x18,0x08,0x00,0x00,0x18,0x0B,0x5B,0x90,0x18,
		0x08,0x00,0x00,0x18,0x08,0x00,0x00,0x18,0x0F,0xFF,0xFF,0xF8,0x03,0xFF,0xFF,0xF8
	};


static Bitmap*				SpeechIcon;
static Bitmap*				ExclamationIcon;
static Bitmap*				StopIcon;

EXECUTE(static MyBoolean Initialized = False;)

static char*					EmergencyMemory;


/* initialize the alert system, allocate the bitmaps and temporary memory stash */
MyBoolean					InitializeAlertSubsystem(void)
	{
		ERROR(Initialized,PRERR(ForceAbort,
			"InitializeAlertSubsystem called more than once"));
		EXECUTE(Initialized = True);
		SpeechIcon = MakeBitmap(RawSpeechIcon,32,32,4);
		if (SpeechIcon == NIL)
			{
			 FailurePoint1:
				return False;
			}
		ExclamationIcon = MakeBitmap(RawExclamationIcon,32,32,4);
		if (ExclamationIcon == NIL)
			{
			 FailurePoint2:
				DisposeBitmap(SpeechIcon);
				goto FailurePoint1;
			}
		StopIcon = MakeBitmap(RawStopIcon,32,32,4);
		if (StopIcon == NIL)
			{
			 FailurePoint3:
				DisposeBitmap(ExclamationIcon);
				goto FailurePoint2;
			}
		EmergencyMemory = AllocPtrCanFail(ALERTEMERGENCY,"AlertEmergencyPtr");
		if (EmergencyMemory == NIL)
			{
			 FailurePoint4:
				DisposeBitmap(StopIcon);
				goto FailurePoint3;
			}
		return True;
	}


/* clean up internal data structures */
void							ShutdownAlertSubsystem(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"Alert subsystem not initialized"));
		DisposeBitmap(SpeechIcon);
		DisposeBitmap(ExclamationIcon);
		DisposeBitmap(StopIcon);
		if (EmergencyMemory != NIL)
			{
				ReleasePtr(EmergencyMemory);
			}
	}


typedef struct
	{
		WinType*						ScreenID;
		char*								TotalMessage;
		SimpleButtonRec*		OKButton;
		Bitmap*							IconThing;
	} AlertRec;


static void				AlertUpdater(AlertRec* Window)
	{
		/* DON'T call CheckPtrExistence since Window is allocated on the stack */
		SetClipRect(Window->ScreenID,0,0,ALERTWIDTH,ALERTHEIGHT);
		DrawBitmap(Window->ScreenID,23,13,Window->IconThing);
		DrawWrappedTextBox(Window->ScreenID,Window->TotalMessage,GetUglyFont(),12,
			23 + 32 + 23,13,ALERTWIDTH - (23 + 32 + 23 + 13) - 10,
			ALERTHEIGHT - (13 + 13 + 21 + 6));
		RedrawSimpleButton(Window->OKButton);
	}


static void				LocalAlert(Bitmap* TheIcon, char* Message, char* ExtraInfo)
	{
		AlertRec				Window;
		EXECUTE(static MyBoolean	Alerting = False;)

		APRINT(("+LocalAlert: '%t' '%t'",Message,ExtraInfo));
		ERROR(!Initialized,PRERR(ForceAbort,"Alert subsystem not initialized"));
		ERROR(Message == NIL,PRERR(ForceAbort,"LocalAlert:  Message is NIL"));
		ERROR(Alerting,PRERR(ForceAbort,"LocalAlert called recursively"));
		EXECUTE(Alerting = True;)
		if (EmergencyMemory != NIL)
			{
				ReleasePtr(EmergencyMemory);
				EmergencyMemory = NIL;
			}
		ErrorBeep();
		SetArrowCursor();
		if (ExtraInfo != NIL)
			{
				char*					Key;
				char*					Replacement;
				char*					Original;
				char*					Temp;

				Key = BlockFromRaw("_",1);
				Replacement = BlockFromRaw(ExtraInfo,StrLen(ExtraInfo));
				Original = BlockFromRaw(Message,StrLen(Message));
				if ((Key == NIL) || (Replacement == NIL) || (Original == NIL))
					{
						/* we don't care about releasing the memory since this is a fatal */
						/* error anyway. */
					 Fault:
						PRERR(ForceAbort,"Out of memory in alert handler -- can't continue");
					}
				Temp = ReplaceBlockCopy(Original,Key,Replacement);
				if (Temp == NIL)
					{
						goto Fault;
					}
				ReleasePtr(Key);
				ReleasePtr(Replacement);
				ReleasePtr(Original);
				Window.TotalMessage = BlockToStringCopy(Temp);
				ReleasePtr(Temp);
				if (Window.TotalMessage == NIL)
					{
						goto Fault;
					}
			}
		 else
			{
				Window.TotalMessage = StringFromRaw(Message);
			}
		Window.IconThing = TheIcon;
		Window.ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,AlertLeftEdge(ALERTWIDTH),
			AlertTopEdge(ALERTHEIGHT),ALERTWIDTH,ALERTHEIGHT,
			(void (*)(void*))&AlertUpdater,&Window);
		if (Window.ScreenID == NIL)
			{
				goto Fault;
			}
		Window.OKButton = NewSimpleButton(Window.ScreenID,"OK",ALERTWIDTH
			- (13 + BUTTONWIDTH),ALERTHEIGHT - (13 + 21 + 3),BUTTONWIDTH,21);
		if (Window.OKButton == NIL)
			{
				goto Fault;
			}
		SetDefaultButtonState(Window.OKButton,True);
		while (True)
			{
				OrdType							X,Y;
				ModifierFlags				Modifiers;
				WinType*						TheWindow;
				MenuItemType*				MenuItem;
				char								KeyPressed;

				switch (GetAnEvent(&X,&Y,&Modifiers,&TheWindow,&MenuItem,&KeyPressed))
					{
						default:
							break;
						case eCheckCursor:
							SetArrowCursor();
							break;
						case eNoEvent:
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window.OKButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window.OKButton,X,Y,NIL,NIL))
										{
										 ExitPoint:
											DisposeSimpleButton(Window.OKButton);
											ReleasePtr(Window.TotalMessage);
											KillWindow(Window.ScreenID);
											EXECUTE(Alerting = False;)
											EmergencyMemory = AllocPtrCanFail(ALERTEMERGENCY,
												"AlertEmergencyPtr");
											APRINT(("-LocalAlert"));
											return;
										}
								}
							break;
						case eKeyPressed:
							if (KeyPressed == 13)
								{
									FlashButton(Window.OKButton);
									goto ExitPoint;
								}
							break;
					}
			}
		APRINT(("-LocalAlert"));
	}


/* display a halt (Stopsign) alert.  ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertHalt(char* Message, char* ExtraInfo)
	{
		LocalAlert(StopIcon,Message,ExtraInfo);
	}


/* display a warning (!) alert. ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertWarning(char* Message, char* ExtraInfo)
	{
		LocalAlert(ExclamationIcon,Message,ExtraInfo);
	}


/* display an informational (I) alert. ExtraInfo is inserted into Message where */
/* the _ character is */
void							AlertInfo(char* Message, char* ExtraInfo)
	{
		LocalAlert(SpeechIcon,Message,ExtraInfo);
	}


typedef struct
	{
		WinType*						ScreenID;
		char*								TotalMessage;
		SimpleButtonRec*		YesButton;
		SimpleButtonRec*		NoButton;
		SimpleButtonRec*		CancelButton;
	} YesNoCancelRec;


static void				YesNoCancelUpdater(YesNoCancelRec* Window)
	{
		/* do NOT call CheckPtrExistence since Window is allocated on the stack */
		DrawBitmap(Window->ScreenID,23,13,ExclamationIcon);
		DrawWrappedTextBox(Window->ScreenID,Window->TotalMessage,GetUglyFont(),12,
			23 + 32 + 23,13,DLOGWIDTH - (23 + 32 + 23 + 13) - 16,
			DLOGHEIGHT - (13 + 13 + 21 + 6));
		RedrawSimpleButton(Window->YesButton);
		RedrawSimpleButton(Window->NoButton);
		if (Window->CancelButton != NIL)
			{
				RedrawSimpleButton(Window->CancelButton);
			}
	}


/* present a dialog with 3 buttons (yes, no, and cancel).  if Cancel is NIL, */
/* then there will only be 2 buttons.  ExtraInfo is inserted into Message where */
/* the _ character is */
YesNoCancelType		AskYesNoCancel(char* Message, char* ExtraInfo,
										char* Yes, char* No, char* Cancel)
	{
		YesNoCancelRec			Window;
		YesNoCancelType			ReturnValue;

		ERROR(!Initialized,PRERR(ForceAbort,"Alert subsystem not initialized"));
		if (ExtraInfo != NIL)
			{
				MyBoolean			Error;
				char*					Key;
				char*					Replacement;
				char*					Original;
				char*					Temp;

				Error = False;
				Key = BlockFromRaw("_",1);
				if (Key != NIL)
					{
						Replacement = BlockFromRaw(ExtraInfo,StrLen(ExtraInfo));
						if (Replacement != NIL)
							{
								Original = BlockFromRaw(Message,StrLen(Message));
								if (Original != NIL)
									{
										Temp = ReplaceBlockCopy(Original,Key,Replacement);
										if (Temp != NIL)
											{
												Window.TotalMessage = BlockToStringCopy(Temp);
												ReleasePtr(Temp);
												if (Window.TotalMessage == NIL)
													{
														Error = True;
													}
											}
										 else
											{
												Error = True;
											}
										ReleasePtr(Original);
									}
								 else
									{
										Error = True;
									}
								ReleasePtr(Replacement);
							}
						 else
							{
								Error = True;
							}
						ReleasePtr(Key);
					}
				 else
					{
						Error = True;
					}
				if (Error)
					{
						return eCancel;
					}
			}
		 else
			{
				Window.TotalMessage = StringFromRaw(Message);
				if (Window.TotalMessage == NIL)
					{
						return eCancel;
					}
			}
		SetArrowCursor();
		Window.ScreenID = MakeNewWindow(eModelessDialogWindow,eWindowNotClosable,
			eWindowNotZoomable,eWindowNotResizable,DialogLeftEdge(DLOGWIDTH),
			DialogTopEdge(DLOGHEIGHT),DLOGWIDTH,DLOGHEIGHT,
			(void (*)(void*))&YesNoCancelUpdater,&Window);
		if (Window.ScreenID == 0)
			{
			 MemOut1:
				ReleasePtr(Window.TotalMessage);
				return eCancel;
			}
		Window.YesButton = NewSimpleButton(Window.ScreenID,Yes,DLOGWIDTH
			- (13 + BUTTONWIDTH),DLOGHEIGHT - (13 + 21 + 3),BUTTONWIDTH,21);
		if (Window.YesButton == NIL)
			{
			 MemOut2:
				KillWindow(Window.ScreenID);
				goto MemOut1;
			}
		SetDefaultButtonState(Window.YesButton,True);
		Window.NoButton = NewSimpleButton(Window.ScreenID,No,13,
			DLOGHEIGHT - (13 + 21 + 3),BUTTONWIDTH,21);
		if (Window.NoButton == NIL)
			{
			 MemOut3:
				DisposeSimpleButton(Window.YesButton);
				goto MemOut2;
			}
		if (Cancel != NIL)
			{
				Window.CancelButton = NewSimpleButton(Window.ScreenID,Cancel,DLOGWIDTH
					- (13 + BUTTONWIDTH),DLOGHEIGHT - (13 + 21 + 3 + 21 + 10),BUTTONWIDTH,21);
				if (Window.CancelButton == NIL)
					{
					 MemOut4:
						DisposeSimpleButton(Window.NoButton);
						goto MemOut3;
					}
			}
		 else
			{
				Window.CancelButton = NIL;
			}
		while (True)
			{
				OrdType							X,Y;
				ModifierFlags				Modifiers;
				WinType*						TheWindow;
				MenuItemType*				MenuItem;
				char								Key;

				switch (GetAnEvent(&X,&Y,&Modifiers,&TheWindow,&MenuItem,&Key))
					{
						default:
							break;
						case eCheckCursor:
							SetArrowCursor();
							break;
						case eNoEvent:
							break;
						case eMouseDown:
							if (SimpleButtonHitTest(Window.YesButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window.YesButton,X,Y,NIL,NIL))
										{
											ReturnValue = eYes;
											goto ExitPoint;
										}
								}
							else if (SimpleButtonHitTest(Window.NoButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window.NoButton,X,Y,NIL,NIL))
										{
											ReturnValue = eNo;
											goto ExitPoint;
										}
								}
							else if ((Window.CancelButton != NIL)
								&& SimpleButtonHitTest(Window.CancelButton,X,Y))
								{
									if (SimpleButtonMouseDown(Window.CancelButton,X,Y,NIL,NIL))
										{
											ReturnValue = eCancel;
											goto ExitPoint;
										}
								}
							break;
						case eKeyPressed:
							if (Key == 13)
								{
									FlashButton(Window.YesButton);
									ReturnValue = eYes;
									goto ExitPoint;
								}
							else if ((Key == eCancelKey) && (Window.CancelButton != NIL))
								{
									FlashButton(Window.CancelButton);
									ReturnValue = eCancel;
									goto ExitPoint;
								}
							break;
					}
			}

	 ExitPoint:
		DisposeSimpleButton(Window.YesButton);
		DisposeSimpleButton(Window.NoButton);
		if (Window.CancelButton != NIL)
			{
				DisposeSimpleButton(Window.CancelButton);
			}
		ReleasePtr(Window.TotalMessage);
		KillWindow(Window.ScreenID);
		return ReturnValue;
	}
