/*
	File:			VoicePickerDemo.c

	Contains:		A sample application that uses the Voice Picker library. Look at 'DoShowVP'
				function to see how to display the Voice Picker. Also note that the library requires
				the following files to be in the project:
				¥ VoicePickerLib.68 || VoicePickerLib.PPC
				¥ VoicePicker.rsrc
				¥ VoicePicker Balloons.rsrc (if you want balloon help)
				In alternative you can include the Voice Picker source file:
				¥ VoicePicker.c
				¥ VoicePicker.rsrc
				¥ VoicePicker Balloons.rsrc (if you want balloon help)
				
	Written by:	Luigi Belverato
				P.O. Box 19,
				20080 Basiglio MI,
				Italy

	Copyright:	©1995 Luigi Belverato
	
	Change History (most recent first):
				06/28/95	0.0.0d1	Added DisposeRoutineDescriptor for Apple Events handlers

				06/23/95	0.0.0d0

	Notes: 		This code uses Apple's Universal Interfaces for C version 2.01f.
	
				Send bug reports to lbelvera@micronet.it
				(if I don't reply within a few days use snail mail as I might have changed internet provider)
*/

//-----------------------------------------------------------------------
// this text document was formatted with Geneva 9, tab size 4

#include "VoicePickerDemo.h"

//Globals

Boolean				gDone=false;
Boolean				gInBackground=false;
FeaturesRec			gFeatures;

VoiceData				gVoiceData;	/*this global lets us keep note of the voice chosen by the user
								between subsequent calls to Voice Picker and eventually use
								that voice in a real application*/

AEEventHandlerUPP		gHandleOAPPProc=nil;
AEEventHandlerUPP		gHandleQUITProc=nil;

void main(void)
{
	MaxApplZone();

	if (!IsCompatible())
		MissingFeaturesError();

	GatherFeatures();
	ToolboxInit();
	MenuBarInit();
	
	MainEventLoop();
}

 void ToolboxInit(void)
 {
 	short		count=0;
 	EventRecord	event;
 	
	if (gFeatures.appleEvents)
	{
		gHandleOAPPProc=NewAEEventHandlerProc(HandleOAPP);
		gHandleQUITProc=NewAEEventHandlerProc(HandleQUIT);
		InstallAEEntries();
	}
 
	 InitGraf(&qd.thePort);
	 InitFonts();
	 InitWindows();
	 InitMenus();
	 TEInit();
	 InitDialogs(nil);
	 InitCursor();
	 
	 for (count=1;count<=3;count++)
	 	EventAvail(everyEvent,&event);
 }
 
 void MenuBarInit(void)
 {
	 Handle	menuBar;
	 MenuHandle	menu;
	 
	 menuBar=GetNewMBar(kmenuBar);
	 
	 if (menuBar==nil)
	 {
	 	DoErrorIndex(kNoMBAR,"\p");
	 	gDone=true;
	 }
	 else
	 {
		 SetMenuBar(menuBar);
		 DisposeHandle(menuBar);
		 
		 menu=GetMenuHandle(mApple);
		 if (menu!=nil)
			 AppendResMenu(menu,'DRVR');
		 		 
		 DrawMenuBar();
	}
}

void MainEventLoop(void)
{
	while (!gDone)
		EventGetter();
}

void EventGetter(void)
{
	RgnHandle		cursorRgn=nil;
	Boolean		gotEvent=false;
	EventRecord	event;
		
	gotEvent=WaitNextEvent(everyEvent,&event,kSleep,cursorRgn);
	
	if (gotEvent)
		HandleEvent(&event);

}

void HandleEvent(EventRecord *event)
{

	switch(event->what)
	{
		case mouseDown:
			HandleMouseDown(event);
			break;
		case keyDown:
		case autoKey:
			HandleKeyPress(event);
			break;
		case activateEvt:
			HandleActivate(event);
			break;
		case updateEvt:
			HandleUpdate(event);
			break;
		case diskEvt:
			HandleDiskInsert(event);
			break;
		case osEvt:
			HandleOSEvent(event);
			break;
		case kHighLevelEvent:
			HandleHighLevelEvent(event);
			break;
	}
}

void HandleActivate(EventRecord *event)
{
	WindowRef	theWindow;
	Boolean		becomingActive;
	
	theWindow=(WindowRef)event->message;
	becomingActive=(event->modifiers&activeFlag)!=0;
	
	if (becomingActive && gInBackground)
		gInBackground=true;
}

void HandleDiskInsert(EventRecord *event)
{
	Point	aPoint={100,100};
	
	if (HiWord(event->message)!=noErr)
		(void)DIBadMount(aPoint,event->message);
}


void HandleKeyPress(EventRecord *event)
{
	char	theChar;

	theChar=event->message & charCodeMask;
	if (event->modifiers & cmdKey)
	{
		AdjustMenus();
		HandleMenuCommand(MenuKey(theChar));
	}
}

void HandleMouseDown(EventRecord *event)
{
	WindowRef	theWindow;
	short		part;
	
	part=FindWindow(event->where,&theWindow);
	
	switch (part)
	{
		case inMenuBar:
			AdjustMenus();
			HandleMenuCommand(MenuSelect(event->where));
			break;
		case inSysWindow:
			SystemClick(event,(WindowPtr)theWindow);
			break;
	}
}

void HandleOSEvent(EventRecord *event)
{
	#pragma unused (event)

}	

void HandleUpdate(EventRecord *event)
{
	#pragma unused (event)

}

void AdjustMenus(void)
{
	WindowRef	theWindow;
	MenuHandle	menu;
	
	theWindow=FrontWindow();
	
	menu=GetMenuHandle(mFile);
	if (menu!=nil)
	{
		
		if (IsDAWindow(theWindow))
		{
			EnableItem(menu,iClose);
			EnableItem(menu,iQuit);
		}
		else
		{
			DisableItem(menu,iClose);
			EnableItem(menu,iQuit);
		}
	}
	
	menu=GetMenuHandle(mEdit);
	if (menu!=nil)
	{
		if (IsDAWindow(theWindow))
		{
			EnableItem(menu,iUndo);
			EnableItem(menu,iCut);
			EnableItem(menu,iCopy);
			EnableItem(menu,iPaste);
			EnableItem(menu,iClear);
			EnableItem(menu,iSelectAll);
		}
		else
		{
			DisableItem(menu,iUndo);
			DisableItem(menu,iCut);
			DisableItem(menu,iCopy);
			DisableItem(menu,iPaste);
			DisableItem(menu,iClear);
			DisableItem(menu,iSelectAll);
		}
	}

	menu=GetMenuHandle(mTest);
	if (menu!=nil)
	{
		if (IsDAWindow(theWindow))
		{
			DisableItem(menu,iShowVP);
		}
		else
		{
			EnableItem(menu,iShowVP);
		}
	}
}

void CloseAnyWindow(WindowRef window)
{
	if (IsDAWindow(window))
		CloseDeskAcc(GetWindowKind(window));
}

Boolean IsDAWindow(WindowRef window)
{
	if (!window)
		return false;
	else
		return GetWindowKind(window);
}

Boolean IsCompatible(void)
{
	OSErr	error;
	long		theFeature;
	
	Boolean compatible=true;
	
	compatible=TrapAvailable(_Gestalt);
	if (!compatible)
		return false;

	error=Gestalt(gestaltPopupAttr,&theFeature);
	if ((error==noErr) && (compatible))
		compatible=BitTst(&theFeature,31-gestaltPopupPresent);
	if ((error!=noErr) || (!compatible))
		return false;
	
	error=Gestalt(gestaltSpeechAttr,&theFeature);
	if ((error==noErr) && (compatible))
		compatible=BitTst(&theFeature,31-gestaltSpeechMgrPresent);
	if ((error!=noErr) || (!compatible))
		return false;
	
	return true;
}

void GatherFeatures(void)
{
	OSErr	error;
	long	theFeature;
	
	error=Gestalt(gestaltAppleEventsAttr,&theFeature);
	if (error==noErr)
		gFeatures.appleEvents=BitTst(&theFeature,31-gestaltAppleEventsPresent);
	else
		gFeatures.appleEvents=false;
}

Boolean TrapAvailable(short theTrap)
{
	TrapType	theTrapType;
	
	theTrapType=GetTrapType(theTrap);
	if ((theTrapType==ToolTrap) && ((theTrap &= 0x07FF)>=NumToolboxTraps()))
		return false;
	else
		return (NGetTrapAddress(_Unimplemented,ToolTrap)!=NGetTrapAddress(theTrap,theTrapType));
}

TrapType GetTrapType(short theTrap)
{
	if ((theTrap & 0x0800)==0)
		return (OSTrap);
	else
		return (ToolTrap);
}

short NumToolboxTraps(void)
{
	if (NGetTrapAddress(0xA86E,ToolTrap)==NGetTrapAddress(0xAA6E,ToolTrap))
		return (0x200);
	else
		return (0x400);
}

void MissingFeaturesError(void)
{
	(void)StopAlert(kAlrtMissingFeatures,nil);
	ExitToShell();
}

void DoErrorIndex(short stringIndex,Str255 theString)
{
	Str255	theMessage;
	
	GetIndString(theMessage,kErrorStr,stringIndex);
	if (theString[0])
	{
		ParamText(theMessage,theString,"\p","\p");
		(void)StopAlert(kAlrtOK,nil);
	}
}

void	HandleHighLevelEvent(EventRecord *theEvent)
{
	OSErr	error;

	error = AEProcessAppleEvent(theEvent);
}

void InstallAEEntries(void)
{
	OSErr	error;

	error=AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,gHandleOAPPProc, 0, false);
	
	error=AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,gHandleQUITProc, 0, false);
}

pascal OSErr HandleOAPP(AppleEvent *theAppleEvent, AppleEvent *reply,long handlerRefcon)
{
	#pragma unused (reply,handlerRefcon)
	
	OSErr	error;
	
	error = CheckGotRequiredParams(theAppleEvent);
	
	return error;
}

pascal OSErr HandleQUIT(AppleEvent *theAppleEvent, AppleEvent *reply,long handlerRefcon)
{
	#pragma unused (reply,handlerRefcon)
	
	OSErr		error;

	error=CheckGotRequiredParams(theAppleEvent);
	if (error==noErr)
	{
		DoQuit();
	}
	return error;
}

OSErr CheckGotRequiredParams (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size		actualSize;
	OSErr		error;

	error=AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr,typeWildCard, &returnedType,nil, 0, &actualSize);

	if (error==errAEDescNotFound)
		error=noErr;
	else
		if (error==noErr)
			error=errAEParamMissed;
	return error;
}

Boolean IsInBackground(void)
{
	OSErr				error;
	ProcessSerialNumber	frontPSN,thisPSN;
	Boolean				result=true;
	
	error=GetFrontProcess(&frontPSN);
	if (error==noErr)
	{
		error=GetCurrentProcess(&thisPSN);
		if (error==noErr)
		{
			error=SameProcess(&frontPSN,&thisPSN,&result);
		}
	}
	return !result;
}

void HandleMenuCommand(long menuResult)
{
	short	menuID;
	short	menuItem;	
	
	menuID=HiWord(menuResult);
	menuItem=LoWord(menuResult);
	
	switch(menuID)
	{
		case mApple:
			HandleAppleChoice(menuItem);
			break;
		case mFile:
			HandleFileChoice(menuItem);
			break;
		case mEdit:
			HandleEditChoice(menuItem);
			break;
		case mTest:
			HandleTestChoice(menuItem);
			break;
	}
	HiliteMenu(0);
}

void HandleAppleChoice(short menuItem)
{
	MenuHandle	menuH;
	Str255		accName;
	
	switch (menuItem)
	{
		case iAbout:
			DoAbout();
			break;
		default:
			menuH=GetMenuHandle(mApple);
			if (menuH)
			{
				GetMenuItemText(menuH,menuItem,accName);
				OpenDeskAcc(accName);
			}
			break;
	}
}

void HandleFileChoice(short menuItem)
{
	switch(menuItem)
	{
		case iQuit:
			DoQuit();
			break;
	}
}

void HandleEditChoice(short menuItem)
{
	SystemEdit(menuItem-1);
}

void HandleTestChoice(short menuItem)
{
	switch(menuItem)
	{
		case iShowVP:
			DoShowVP();
			break;
	}
}

void DoAbout(void)
{
	
	Alert(kAlrtAbout,nil);
}

void DoQuit(void)
{
	gDone=true;
	
	if (gHandleOAPPProc!=nil)
		DisposeRoutineDescriptor(gHandleOAPPProc);
		
	if (gHandleQUITProc!=nil)
		DisposeRoutineDescriptor(gHandleQUITProc);

}

void DoShowVP(void)
{
	Boolean	confirmation=false;
	
	confirmation=VoicePicker("\pSelect a voice:",&gVoiceData);
	//confirmation=VoicePicker(nil,&gVoiceData);	//Voice Picker will display the default prompt

}
