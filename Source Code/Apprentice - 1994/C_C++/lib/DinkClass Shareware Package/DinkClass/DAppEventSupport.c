/*
	File:		DAppEventSupport.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <6>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <5>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <4>	 9/20/92	MTG		bringing the C++ version up to date with the THINKC viersion
		 <3>	  8/9/92	MTG		merging changes from the ThinkC version
		 <2>	  8/8/92	MTG		fixing warnings

	To Do:
*/


//
// This file holds the impelemtation of the Event dispatch code for
// the DApplication class.  It has the event object list maintence
// code and the "first crack" methods for the various Events returned by WaitNextEvent
// which do something important then sends the event on to the
// event handler model.  The First Crack methods are NOT to do the work that
// the DEventHandler method overrides are intended to do, just the
// critical tasks which must be done befor the eventhandler can
// do its job.
//

#include "DApplication.h"
#include "DEventHandler.h"
#include "DDocument.h"
#include <AppleEvents.h>
#include <DinkUtils.h>

#define	kOSEvent	app4Evt
#define	kSuspendResumeMessage	0x01
#define	kMouseMovedMessage	0xFA


void DApplication::EventLoop(void)
{
	long	gotEvent;
	EventRecord	theEvent;
	
	while(fDone == FALSE)
	{
		gotEvent = WaitNextEvent( everyEvent, &theEvent, fSleepVal, NULL);

		gPassItOn = TRUE;
		switch(theEvent.what) 
		{
			case nullEvent:
				NullEvent(&theEvent);
				break;
	
			case mouseDown:
				MouseDown(&theEvent);
				break;
	
			case activateEvt:// put the correct window in the front!!! and make it the mTarget!!
				ActivateEvt(&theEvent);
				break;
	
			case autoKey:
			case keyDown:					
				KeyDown(&theEvent);
				break;
	
			case diskEvt:
				DiskEvt(&theEvent);
				break;		
				
			case kHighLevelEvent:
				HighLevelEvent(&theEvent);
				break;
	
			case kOSEvent:
				OSEvent(&theEvent);
				break;
	
			case updateEvt:
				UpdateEvt(&theEvent);
				break;
			
			default:
				//EnterMBStr("unknown event!!!");
				break;
	
		}// end big event switch!!

	}// end while mDone is still false
}// end member function EventLoop


//
// The following section of code is the "First Crack" methods requiered
// by the Macintosh os, and the DinkClass event handling architecture.
//

void DApplication::MouseDown(EventRecord *theEvent)
{
	WindowPtr	theWindow;
	short		partCode;
	long		result;
	
	partCode = FindWindow( theEvent->where, &theWindow);
	if( partCode == inMenuBar )
	{
		ClearMenus();
		fTarget->SetUpMenues();
		
		result = MenuSelect(theEvent->where);
		if(HiWrd(result) != 0)
		{
			fTarget->HandleMenuChoice(HiWrd(result), LoWrd(result) );
			HiliteMenu(0);
		}
	}
	else
	{
		if ( theWindow != FrontWindow() )
			SelectWindow( theWindow);	// The resulting activate event will
										// correctly set the ftarget of me in
										// the Activate method bellow.
		 else
			fTarget->HandleMouseDown(theEvent, partCode, theWindow); 
	}

}// end MouseDown member function 


void DApplication::NullEvent(EventRecord *theEvent)
{
	fTarget->HandleNullEvent(theEvent);

	if( fDeadHandlers->NumItems())
		FlushDeadHandlers();
}


void DApplication::ActivateEvt(EventRecord *theEvent)
{
	short windowKind;
	
	windowKind = ((WindowPeek) theEvent->message)->windowKind;
	if (windowKind >= userKind)
	{
		fTarget = (DEventHandler *) GetWRefCon((WindowPtr)theEvent->message);
		fTarget->HandleActivateEvt(theEvent);
	}
	else
		fTarget = this;
	// I'm making the same assumption that userKind windows are DWidnows which
	// is a safe assumption as DWindows are created by the user.
	// windows under sys 7 will be DWindows.
	
	// whatch for active windows not being the front one, if this happens
	// then a check for frontness needs to be put here and if window isn't 
	// the front on then bring it up.
}



void DApplication::AutoKey(EventRecord *theEvent)
{
	fTarget->HandleAutoKey(theEvent);
	// BIG STUBB!!!!
}



void DApplication::KeyDown(EventRecord *theEvent)					
{
	char key;
	long result;
	
	key = (char) theEvent->message & charCodeMask;
	if( theEvent->modifiers & cmdKey )	//command key!!
	{
		ClearMenus();
		fTarget->SetUpMenues();
		
		result = MenuKey(key);
		if(HiWrd(result) != 0)
		{
			fTarget->HandleMenuChoice(HiWrd(result), LoWrd(result) );
			HiliteMenu(0);
		}
	}
	else // not menu command key
		fTarget->HandleKeyDown(theEvent);

}



void DApplication::DiskEvt(EventRecord *theEvent)
{
	Point mountPoint;
	short	err;
	
	if (HiWrd(theEvent->message) != noErr) 
	{
		SetPt(&mountPoint, kDILeft, kDITop);
		err = DIBadMount(mountPoint, theEvent->message);
	}
	fTarget->HandleDiskEvt(theEvent);
}



void DApplication::HighLevelEvent(EventRecord *theEvent)
{
	AEProcessAppleEvent(theEvent);
	
	//fTarget->HandleHighLevelEvent(theEvent);
}


void DApplication::OSEvent(EventRecord *theEvent)
{
	switch ((theEvent->message >> 24) & 0xFF) 
	{
		case kMouseMovedMessage:
			DisposeRgn( fCursorRgn);
			fCursorRgn = NewRgn();
			SetRectRgn(fCursorRgn, -32766, -32766, 32766, 32766); 
			break;

		case kSuspendResumeMessage:
			fInBackground = !(theEvent->message & resumeFlag);
			if (fInBackground)	//going into the background
			{
				fSleepVal = BACKGROUNDSLEEP; // we don't wan't any NULL events when we're in the background
				GiveClipToSystem();
			}
			else	// coming out of the background
			{
				fSleepVal = SLEEPVAL;// put back to normal!!
				if(theEvent->message & convertClipboardFlag)
					GetClipFromSystem();
				// I don't do anything else for I know that the front window will
				// be getting an activate event next loop!!!
			}
			break;
	}
	fTarget->HandleOSEvent(theEvent);
}


void DApplication::UpdateEvt(EventRecord *theEvent)
{
	DWindow *upDateWindow;
	short windowKind;
	
	windowKind = ((WindowPeek) theEvent->message)->windowKind;

	// I'm making the same assumption that userKind windows are DWidnows which
	// is a safe assumption as DWindows are created by the user
	// windows under sys 7 will be DWindows.  This is a bit of a risk hear, if
	// the window is not in my appliction plane (which shouldn't happen) this 
	// will brake.

	if (windowKind >= userKind)
	{
		upDateWindow = (DWindow *) GetWRefCon((WindowPtr)theEvent->message);
		gPassItOn = FALSE;
		upDateWindow->HandleUpdateEvt(theEvent);
	}
	gPassItOn = TRUE;
	
}



//
// The following section of code is in support of the multiple
// event handling object modle used in the DinkClass library
//


// Itterate over the list of deadHandlers event hadlers and delete
// them all.  It is assumed that they have all cleaned up after
// themselves and its ok to puff them.
Boolean DApplication::FlushDeadHandlers(void)
{
	DEventHandler *nextHandler;
	
	fIterator->Init(fDeadHandlers);
	
	// First remove the dead handlers from the event hadler list (see bellow 
	// for WHY they haven't been removed already) then delete them.  The error
	// checking (like is nextHandler in the list of fEventHandlers? or fDeadHandlers?)
	// is done in RemoveItem, but we have to be carefull with that delete guy.  
	// He'll nuke you if given any excuse.
	
	while( nextHandler = (DEventHandler *)fIterator->GetCurrentThenIncrement() )
	{
		fEventHandlers->RemoveItem(nextHandler);
		
		if( fDeadHandlers->RemoveItem(nextHandler) )
			delete nextHandler;
	}
	return TRUE;
}

// Take killMe out of the list of itsEventHandlers and
// up-date the list pointers in that list and finaly
// place that guy in the list of deadHandlers for flushing

Boolean DApplication::RemoveHandler( DEventHandler *killMe)
{
	DEventHandler *nextHandler;
	
	fIterator->Init(fEventHandlers);
	
	// First loop over the event hadlers and reset all refrences to killMe
	// to killMe->fNextHandler.  Then if killMe is the fTarget reset that
	// to point to either killMe->fNextHandler or gApplication 
	// (if killMe->fNextHandler == NULL)
	
	while( nextHandler = (DEventHandler *)fIterator->GetCurrentThenIncrement() )
	{
		if (killMe == (nextHandler)->fNextHandler)
			 (nextHandler)->fNextHandler = killMe->fNextHandler;
		if (killMe == fTarget)
		{
			if ( fTarget->fNextHandler)
			 	fTarget = killMe->fNextHandler;
			else
				fTarget = gApplication;// ok for now but this may make dangleing pointers later!!!!!
		}
	}

	// I don't remove the killMe object form the fEventHandlers list at this point
	// because if I did then the CleanUp() method which tells all the eventhandlers
	// to kill themselvs would not work dew to side effects of objects killing themselves
	// and telling thier dependents to do the same (eg DWindow->KillMeNext() does this)
	//
	// If I did remove the event handler from the fEventHandlers list then that
	// list would change while an iterator was opperating on it,  resulting in the
	// iteration getting lost in space.
	
	if( fEventHandlers->ItemInList(killMe) )
	{
		fDeadHandlers->AddItem( killMe );
		return TRUE; // killMe is taken out of the picture and ready to die
	}
	else
		return FALSE;
}	


Boolean DApplication::InstalHandler( DEventHandler *addMe)
{
	
	if (fEventHandlers->ItemInList( addMe ) == FALSE)
		fEventHandlers->AddItem( addMe );
	return TRUE;

}	


//
// ClearMenus is a utility method which gets called by the first
// crack mouse down DApplication method if the mouse event was in the 
// menu bar.  I think this class of member function should be 
// partitiond away from the rest of the class definition.
//

void DApplication::ClearMenus()
{
	MenuHandle	menu;
	int menuItem;
	
	menu = GetMHandle(rFileMenu);
	for (menuItem=1; menuItem<=CountMItems(menu); menuItem++)
	{
		EnableMenuItem( menu, menuItem, FALSE);
	}

	menu = GetMHandle(rEditMenu);
	for (menuItem=1; menuItem<=CountMItems(menu); menuItem++)
	{
		EnableMenuItem( menu, menuItem, FALSE);
	}
}

