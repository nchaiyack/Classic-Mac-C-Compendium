/*
	FILE:			ArrowCDEF Tester.c
	DESCRIPTION:	A simple program to test the ArrowCDEF.
	AUTHOR:		David Hay
	CREATED:		March 17, 1994
	
	Copyright © 1994 by David Hay
*/

#include <ctype.h>
#include "ArrowCDEF.h"

#define 	kBaseResID		128
#define 	kMoveToFront		(WindowPtr)-1L

#define	kEnterKey			0x03
#define	kReturnKey		0x0d
#define 	kEscapeKey		0x1b

#define	kVisualDelay		8
#define	kScrollDelay		15

#define	kArrowCntl		2
#define	kOKOutline		3
#define	kTextItem			4
#define	kActiveItem		6


void	ToolBoxInit( void )
{
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	MaxApplZone();
	InitCursor();
}

pascal Boolean DialogFilter( DialogPtr theDialog, EventRecord* theEvent, short* itemHit )
{
	short		itemType;
	Handle		itemHandle;
	Rect			itemRect;
	char			key;
	long			finalTicks;
	Point			mousePoint;
	
	switch( theEvent->what )
	{
		case keyDown:
		case autoKey:
			key = ( char ) ( ( theEvent->message ) & charCodeMask );
			
			if ( key == kReturnKey || key == kEnterKey )
			{
					/*		Respond as if the OK button was pressed		*/
				GetDItem( theDialog, ok, &itemType, &itemHandle, &itemRect );
				HiliteControl( (ControlHandle)itemHandle, inButton );
				Delay( kVisualDelay, &finalTicks );
				HiliteControl( (ControlHandle)itemHandle, 0 );
				*itemHit = ok;
				return( true );
			}
			else
			{
					/*		Only allow numbers to be entered			*/
				if ( !isdigit( key ) && !iscntrl( key ) && key != '-')
				{
					SysBeep( 10 );
					*itemHit = kTextItem;
					return( true );
				}
			}
			break;
	}
	return( false );
}

pascal void ArrowActionProc( ControlHandle theControl, short part )
{
	long			theValue;
	Str255		theString;
	DialogPtr		theDialog;
	Rect			iRect;
	Handle		iHandle;
	short		iType;
	static long		delay = kScrollDelay;
	static long		endTicks = 0;
	static short	oldDir = kPlainArrow;

	if ( endTicks < TickCount() && part != kPlainArrow )
	{
		theValue = GetCtlValue( theControl );
		theDialog = (DialogPtr)(*theControl )->contrlOwner;
		GetDItem(theDialog, kTextItem, &iType, &iHandle, &iRect);
		if ( part == inUpButton )			/*	pressed up arrow, increment theValue	*/
		{
			if ( theValue < GetCtlMax( theControl ) )
				theValue++;
			if ( oldDir != kUpArrow )		/*	remember which part was pressed last	*/
			{
				oldDir = kUpArrow;
				delay = kScrollDelay;
			}
		}
		else if ( part == inDownButton )		/*	pressed down arrow, decrement theValue	*/
		{
			if ( theValue > GetCtlMin( theControl ) )
				theValue--;
			if ( oldDir != kDownArrow )	/*	remember which part was pressed last	*/
			{
				oldDir = kDownArrow;
				delay = kScrollDelay;
			}
		}
			/*	Set the new value, display it, and hilite it.		*/
		SetCtlValue( theControl, theValue );
		NumToString( theValue, theString );
		SetIText(iHandle, theString);
		SelIText(theDialog, kTextItem, 0, 32767);
		
		endTicks = TickCount() + delay;
		if ( delay > 0 )
		{
			delay--;
		}
	}
}
			
Point	 PositionDialog( DialogPtr theDialog, Boolean move )
{
	short	width;		/* width of theDialog */
	short	height;		/* height of theDialog */
	Point		newLoc;		/* new location of theDialog */

	width = theDialog->portRect.right - theDialog->portRect.left;
	height = theDialog->portRect.bottom - theDialog->portRect.top;
	newLoc.h = ( screenBits.bounds.right - width ) / 2;
	newLoc.v = ( screenBits.bounds.bottom - GetMBarHeight() - height ) / 3
				+ GetMBarHeight();

	if ( move )	/*	if we're supposed to move the dialog, do so	*/
	{
		MoveWindow( theDialog, newLoc.h, newLoc.v, true );
	}
}

pascal void ButtonOutline( DialogPtr theDialog, short itemID )
{
	short 		itemType;			/* The type of the item in GetDItem */
	Handle		theItem;			/* handle to the item */
	Rect			itemBox;			/* the box containing the item */
	PenState		savedPenState;		/* old pen state resored on exit */
	GrafPtr		origPort;			/* the current port saved. restored on exit */
	
	GetPort( &origPort );	/* save the old port */
	SetPort( theDialog );

	GetDItem( theDialog, itemID, &itemType, &theItem, &itemBox );

		/*		Draw the button outline	*/
	GetPenState( &savedPenState );
	PenNormal();
	PenSize( 3, 3 );
	FrameRoundRect( &itemBox, 16, 16 );

		/*		Restore the old settings	*/
	SetPenState( &savedPenState );
	SetPort( origPort );
}

main()
{
	GrafPtr			savePort;		/*	The old graphics port			*/
	DialogPtr			theDialog;		/*	The dialog to display			*/
	short			itemHit;		/*	Last item hit in the dialog		*/
	short			itemType;		/*	The type of item			*/
	Handle			itemHandle;	/*	handle to the item			*/
	Rect				itemRect;		/*	an item's Rect				*/
	Str255			theString;		/*	misc. pascal string			*/
	long				theValue;		/*	misc value.				*/
	Boolean			done;		/*	is the program done yet?		*/
	
	ToolBoxInit();

	GetPort( &savePort );
	
	theDialog = GetNewDialog( kBaseResID, nil, kMoveToFront );
	if ( theDialog == NULL )
	{
		SysBeep( 10 );
		ExitToShell();
	}

		/*		Center the dialog, select it, and make it  the current graphics port.	*/
	PositionDialog( theDialog, true );
	SelectWindow( theDialog );
	SetPort( theDialog );

		/*		Give the arrow control an action procedure	*/
	GetDItem( theDialog, kArrowCntl, &itemType, &itemHandle, &itemRect );
	SetCtlAction( (ControlHandle) itemHandle, ArrowActionProc );
	
		/*		Install a routine to draw the default button outline to a user item.	*/
	GetDItem( theDialog, kOKOutline, &itemType, &itemHandle, &itemRect );
	SetDItem( theDialog, kOKOutline, itemType, &ButtonOutline, &itemRect );

		/*		Make the arrow activator initially active.		*/
	GetDItem( theDialog, kActiveItem, &itemType, &itemHandle, &itemRect );
	SetCtlValue( (ControlHandle)itemHandle, 1 );
	
		/*		Done with initializations.
				Show the dialog and call ModalDialog until OK is pressed.	*/
	ShowWindow( theDialog );
	done = false;
	while( !done )
	{
		ModalDialog( DialogFilter, &itemHit );
		switch ( itemHit )
		{
			case ok:				/*	OK button pressed...exit the program	.		*/
				done = true;
				break;
			case kTextItem:		/*	Text entered, get the new value			*/
				GetDItem( theDialog, kTextItem, &itemType, &itemHandle, &itemRect );
				GetIText( itemHandle, theString );
				StringToNum( theString, &theValue );
				GetDItem( theDialog, kArrowCntl, &itemType, &itemHandle, &itemRect );
				SetCtlValue( (ControlHandle)itemHandle, theValue );
				break;
			case kActiveItem:		/*	Change the active status of the arrow control	*/
				GetDItem( theDialog, kActiveItem, &itemType, &itemHandle, &itemRect );
				theValue = 1 - GetCtlValue( (ControlHandle)itemHandle );
				SetCtlValue( (ControlHandle)itemHandle, theValue );
				GetDItem( theDialog, kArrowCntl, &itemType, &itemHandle, &itemRect );
				HiliteControl( (ControlHandle)itemHandle, (theValue) ? 0 : 255 );
				break;
			default:
				break;
		}
	}
	
	DisposDialog( theDialog );
	SetPort( savePort );
	ExitToShell();
}