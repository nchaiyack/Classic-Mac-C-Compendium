#include "MovableModal.h"
#include "MyMenus.h"

/* MovableModalDialog requires the following function(s) to exist:
	FindWindowToUpdate( WindowPtr window ) - calls an update procedure for ALL possible windows
		-- This function must exist in a file called UpdatesFunctions.h
	DoThisMenu( long menu ) - calls the menu handling procedure in the main function
		-- This function must exist in a file called MyMenusFunctions.h
		
   The reason this requires such a short file in this case (MyMenus.c) is that normally
   there is a lot more that needs to be done with menus that isn't done here.
   
   Movable Modal Dialog requires the that you call these functions yourself:
	TwiddleMenus() - Sets all menus so that they are hilited properly
	MovableModalMenus() - Sets all menus so that they are dimmed properly
	
	I got most of this code from someone, and don't remember who.  If this looks really
	familiar, and you've put your code out in PD, tell me and I'll give you credit here.
*/

void DoThisMenu( long menuCode );
void FindWindowToUpdate( WindowPtr window );


/** The MovableModal dialog routine.  I don't really know why I include the whichDlog item,
    but it's there.  You really should just use FrontWindow() to determine which window
    to make modal, but I included this.  **/	

void	Nothing( void );
void	Nothing( void )
{}

void	MovableModalDialog( ModalFilterUPP	filterProc, short *itemHit, DialogPtr whichDlog )
{
	EventRecord		theEvent;
		
	Boolean		gotEvent, eventHandled;
	OSErr		myErr;
 	ModalFilterUPP theModalProc;
	
	gotEvent = false;
	
	*itemHit = 0;

	HiliteMenu(0);

	while (!gotEvent)
	{
		eventHandled = false;

		gotEvent = WaitNextEvent(everyEvent, &theEvent, 30, nil);
		
		if( IsDialogEvent( &theEvent ) ) 
		{
			if ( filterProc )
			{				
				if ( !(eventHandled = CallModalFilterProc( filterProc, whichDlog, &theEvent, itemHit )) )
				{
					myErr = GetStdFilterProc(&theModalProc);
					if (myErr == noErr)
						eventHandled = StdFilterProc(whichDlog, &theEvent, itemHit);

					if( !eventHandled )
						eventHandled = DialogSelect( &theEvent, &whichDlog, itemHit );
				}
			}
			else
			{
				myErr = GetStdFilterProc(&theModalProc);
				if (myErr == noErr)
					eventHandled = StdFilterProc(whichDlog, &theEvent, itemHit);
				

				if( !eventHandled )
					eventHandled = DialogSelect( &theEvent, &whichDlog, itemHit );
			}
			
		}
		else
		{
			if ( filterProc )
				eventHandled = CallModalFilterProc( filterProc, whichDlog, &theEvent, itemHit );
		}

	if( theEvent.what == activateEvt )
	{
		SetCursor(&qd.arrow);
		if ((theEvent.modifiers & activeFlag ) != 0)
			DoMyActivate( whichDlog );
		else
			DoMyDeactivate( whichDlog );
	}

		if (!eventHandled)
		{
			switch ( theEvent.what )
			{
				case mouseDown:
					{
					WindowPtr	theWindow;
					long		menuCode;
					short		part = FindWindow(theEvent.where, &theWindow);
		
						switch (part)
						{
							case inDrag:
								if (theWindow==whichDlog)
								{
									DragWindow(theWindow, theEvent.where, 
												&qd.screenBits.bounds);	
								}
								else
									SysBeep(20);
								break;
							case inSysWindow:
								SystemClick(&theEvent, theWindow);
								break;
							case inMenuBar:
								menuCode = MenuSelect(theEvent.where);
								DoThisMenu( menuCode );
								break;
							case inContent:
								if (theWindow != whichDlog)
									SysBeep(20);
								break;
							default:
								SysBeep(20);
								break;
						}
					}
					break;
				case keyDown:
				{
					long menu;
					if(( theEvent.modifiers & cmdKey ) != 0 )
					{
						menu = MenuKey( theEvent.message & charCodeMask );
						DoThisMenu( menu );
					}
				}
					break;
				
				case updateEvt:
					FindWindowToUpdate( (WindowPtr) theEvent.message );
					break;
				case nullEvent:
					break;
			}	
		}
	}
	
	HiliteMenu( 0 );
}


/** When a window is sent to the background, it should disable all of it's controls
    This function does that.  **/
void	DoMyDeactivate( WindowPtr theWindow )
{
	ControlHandle	theList;
	GrafPtr			thePort;
	
	GetPort( &thePort );
	SetPort( theWindow );
	
	theList = (*(WindowPeek)theWindow).controlList;
	
	while( theList != nil )
	{
		HiliteControl( theList, 255 );
		theList = (**theList).nextControl;
	}
	
	SetPort( thePort );
}

/** When a window is activated, it should reactivate all of it's controls.
    This function does that. **/
void	DoMyActivate( WindowPtr theWindow )
{
	ControlHandle	theList;
	GrafPtr			thePort;
	
	GetPort( &thePort );
	SetPort( theWindow );
	
	theList = (*(WindowPeek)theWindow).controlList;
	
	while( theList != nil )
	{
		HiliteControl( theList, 0 );
		theList = (**theList).nextControl;
	}
	
	SetPort( thePort );
}