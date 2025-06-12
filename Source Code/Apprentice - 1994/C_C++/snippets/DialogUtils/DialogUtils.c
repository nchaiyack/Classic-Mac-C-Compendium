/*****************************************************************************************************

	DialogUtils.c - A plethora of useful routines for dealing with dialogs.  This include some
					routines for easier access to dialog stuff, as well as a couple of
					generic dialogs that can be used in apps.
					
					- Remember to include a file called DialogUtils.rsrc in compiling this.
					
			©1994 Paul Rademacher
				  Dept. Of Computer Science
				  West Virginia University
				  paulr@cs.wvu.edu
			
			1/5/94

*****************************************************************************************************/

#include	"DialogUtils.h"

/********************************************** highlightDefault() **********************************/
/**** Found in public domain - credit anyways to Paul DuBois and Brian Bechtel  				    */
/**** I made a few changes to the code I found - namely, making the item a variable					*/
/**** instead of always the first button, and making it "port-friendly"								*/

void		highlightDefault( DialogPtr dPtr, short itemNum )
{
    short 		unusedItemType;
    Handle 		unusedItemHandle;
    Rect 		box;
    PenState 	p;
	GrafPtr		oldPort;

    /*  This next little piece of code puts the default heavy rounded
        box around the <itemNumber> button, so the user knows that pressing
        return is the same as hitting <itemNumber>
    */
    
    GetPort( &oldPort ); // So we can return to it later
    
    SetPort(dPtr);      /* without this, can't highlite <itemNumber> */
    GetDItem( dPtr, itemNum, &unusedItemType, &unusedItemHandle, &box);
    GetPenState( &p );
    PenSize( 3, 3 );
    InsetRect( &box, -4, -4 );
    FrameRoundRect( &box, 16, 16 );
    PenSize( p.pnSize.h, p.pnSize.v );
    
    SetPort( oldPort );
}


/************************************************************ fakeMouseClick() **********************/
/* Highlights and dehighlights a button to simulate a mouse click within it.						*/

void		fakeMouseClick( DialogPtr dPtr, short itemNum )
{
	ControlHandle	controlHandle;
	short			itemType;
	Rect			itemRect;
	long			time;

	GetDItem( dPtr, itemNum, &itemType, ( Handle * ) &controlHandle, &itemRect );
	
	if ( controlHandle == nil )
		return;

	HiliteControl( controlHandle, kButtonPress );
	
	Delay( 8, &time );
	
	HiliteControl( controlHandle, kEnableControl ); // We must assume it was enabled before
}


/************************************************************ getDItemHandle() ************************/
/* Gets the ControlHandle for the item you want in the dialog box dlog.  							*/
/* Handy for setting checkboxes and radio buttons 													*/

Handle			getDItemHandle(DialogPtr dlog, short item)
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	itemHandle;
    
    GetDItem( dlog, item, &itemType, &itemHandle, &itemRect);
    
    return( itemHandle );
}


/************************************************************* setDItemText() ***********************/

void			setDItemText( DialogPtr dlog, short item, Str255 text )
{
	Handle	handle;
	
	handle = getDItemHandle( dlog, item );
	
	if ( handle != nil )
		SetIText( handle, text );
}


/************************************************************* getDItemText() ***********************/

void			getDItemText( DialogPtr dlog, short item, StringPtr text )
{
	Handle	handle;
	
	handle = getDItemHandle( dlog, item );
	
	if ( handle != nil )
		GetIText( handle, text );
}


/************************************************************* getDItemType() ***********************/

short			getDItemType( DialogPtr dlog, short item )
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	itemHandle;
    
    GetDItem( dlog, item, &itemType, &itemHandle, &itemRect);
    
    return( itemType );
}


/**************************************************************** getDItemValue() *******************/

short		getDItemValue( DialogPtr dlog, short item )
{
	ControlHandle	handle;
	
	handle = ( ControlHandle ) getDItemHandle( dlog, item );	
	
	if ( handle == nil )
		return( -1 );
	
	return( GetCtlValue( handle ) );
}


/**************************************************************** setDItemValue() *******************/

void		setDItemValue( DialogPtr dlog, short item, short value )
{
	ControlHandle	handle;
	
	handle = ( ControlHandle ) getDItemHandle( dlog, item );	
	
	if ( handle == nil )
		return;
	
	SetCtlValue( handle, value );
}


/**************************************************************** enableDItem() *********************/

void		enableDItem( DialogPtr dlog, short item )
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	controlHandle;
    
    GetDItem( dlog, item, &itemType, &controlHandle, &itemRect);
    
	if ( controlHandle != nil )	
		HiliteControl( ( ControlHandle ) controlHandle, kEnableControl );
}


/**************************************************************** getDItemRect() ********************/

Rect		getDItemRect( DialogPtr dlog, short item )
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	controlHandle;
    
    GetDItem( dlog, item, &itemType, &controlHandle, &itemRect);
    	
	return( itemRect );
}


/**************************************************************** disableDItem() *********************/

void		disableDItem( DialogPtr dlog, short item )
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	controlHandle;
    
    GetDItem( dlog, item, &itemType, &controlHandle, &itemRect);
    
	if ( controlHandle != nil )	
		HiliteControl( ( ControlHandle ) controlHandle, kDisableControl );
}


/********************************************************* drawTextInUserItem() *********************/

void			drawTextInUserItem( DialogPtr dlog, short item, StringPtr text )
{
    short 	itemType;
    Rect 	itemRect;
    Handle 	controlHandle;
    FontInfo	fontInfo;
    
    GetDItem( dlog, item, &itemType, &controlHandle, &itemRect);
    
    GetFontInfo( &fontInfo );
    
    EraseRect( &itemRect );
    
    MoveTo( itemRect.left + 4, itemRect.top + fontInfo.ascent );
    DrawString( text );
}


/********************************************************** drawDottedBoxText() *********************/

void			drawDottedBoxText( Rect *rect, StringPtr text )
{
    FontInfo	fontInfo;
    Rect		eraseRect;
    short		width;
   
    PenPat( gray );
    FrameRect( rect );
    PenNormal();
    
    GetFontInfo( &fontInfo );
    
    width = StringWidth( text );
    
    TextMode( srcCopy );
    
    MoveTo( rect->left + 8, rect->top + fontInfo.descent );
    DrawString( text );
}


/***************************************************** drawDItemDottedBoxText() *********************/

void			drawDItemDottedBoxText( DialogPtr dlog, short item, StringPtr text )
{
    FontInfo	fontInfo;
    Rect		eraseRect;
    short		width;
    short 		itemType;
    Rect 		itemRect;
    Handle 		controlHandle;
    
    GetDItem( dlog, item, &itemType, &controlHandle, &itemRect);

	drawDottedBoxText( &itemRect, text );    
}


/*********************************************************** setAlternatingDefaultItems() ***********/
/* This routine, if passed ok as item, will make the OK button the default ( draw a heavy border 	*/
/* around it ).  If passed cancel, it will make that the default and gray out the ok button.		*/
/* Does nothing if item is not 'ok' or 'cancel' ( 1 or 2 ).											*/

void		setAlternatingDefaultItems( DialogPtr dlog, short item )
{
	GrafPtr			oldPort;
	short			other;

    GetPort( &oldPort ); /* So we can return to it later */
    
    SetPort( dlog );      /* without this, can't highlite <itemNumber> */
    
   	if ( item == ok )
   		other = cancel;
   	else
   		other = ok;
    
	if ( item == ok OR item == cancel )
	{
		PenMode( srcBic );
		
		highlightDefault( dlog, other ); /* This erases previous border */
		
		PenMode( srcOr );
		
		highlightDefault( dlog, item );
		
		/* We disable ( gray out ) the ok button if the default is cancel */
		
		if ( item == ok )
			enableDItem( dlog, ok );
		else
			disableDItem( dlog, ok );
	}

    SetPort( oldPort );
}


/*********************************************************************** doSaveChanges() ************/
/* Warning: this routine uses ParamText(), so you can't use ParamText() elsewhere to modify your 	*/
/* input text.																						*/

short		doSaveChanges( StringPtr actionText, Str255 docName )
{
	DialogPtr	dlog;
	short		item;
	GrafPtr		oldPort;
	OSErr		err;

	dlog = GetNewDialog( rSaveChangesDLOG, 0L, kMoveToFront );
	
	if ( dlog != nil )
	{
		GetPort( &oldPort );
		SetPort( dlog );

		ParamText( docName, actionText, "\p", "\p" );
		
		ShowWindow( dlog );

		highlightDefault( dlog, dSaveChangesOK );
		
		do 
		{
			ModalDialog( nil, &item );
		} while( item != dSaveChangesOK AND item != dSaveChangesCancel AND item != dSaveChangesNo );
				
		DisposDialog( dlog );
		SetPort( oldPort );
		
		return( item );
	}
		
	return( dSaveChangesOK ); /* Save the changes as a default */
}


/**************************************************************************** promptForString() *****/
/* This brings up a dialog with a prompt, an edit text field, and an 'ok' button.  It won't return	*/
/* until the edit text field contains a string of the length indicated by the args.					*/
/* Returns which button was pressed.																*/

short		promptForString( StringPtr prompt, StringPtr defaultText, short minLength, 
				short maxLength, StringPtr dest )
{
	DialogPtr	dlog;
	short		item;
	GrafPtr		oldPort;
	OSErr		err;
	Str255		minString;
	Str255		maxString;

	dlog = GetNewDialog( rPromptForStringDLOG, 0L, kMoveToFront );
	
	if ( dlog != nil )
	{
		GetPort( &oldPort );
		SetPort( dlog );
		
		setDItemText( dlog, dPromptForStringPrompt, prompt );
		setDItemText( dlog, dPromptForStringText, defaultText );
		SelIText( dlog, dPromptForStringText, 0, kEntireText );
		
		ShowWindow( dlog );

		highlightDefault( dlog, dPromptForStringOK );

		NumToString( minLength, minString );
		NumToString( maxLength, maxString );

		( ( DialogPeek ) dlog )->aDefItem = dPromptForStringOK;
		
		do 
		{
			ModalDialog( genericFilterProc, &item );
			
			getDItemText( dlog, dPromptForStringText, dest );
			
			if ( item == dPromptForStringOK )
			{
				if ( dest[ 0 ] < minLength OR dest[ 0 ] > maxLength )
				{
					ParamText( minString, maxString, "\p", "\p" );
				
					CautionAlert( rTextLengthALRT, nil );
				
					highlightDefault( dlog, dPromptForStringOK ); /* We re-highlight it */
					
					item = 0; /* This makes it so we don't exit the 'do' loop */
				}
			}
		} while( item != dPromptForStringOK AND item != dPromptForStringCancel );
				
		DisposDialog( dlog );
		SetPort( oldPort );
		
		return( item );
	}
	
	return( dPromptForStringCancel );
}


/********************************************************************* doDualChoice() ***************/
/* Given text and a default button number, brings up a small dialog with the text, an 'ok' and a 	*/
/* 'cancel' button, and returns which button was clicked.  Returns ok=1 or cancel=2					*/
/* A return value of zero indicates a dialog error.													*/
/* This routine does not use ParamText(), so feel free to use ParamText to modify your prompt.		*/
/* If okText is nil, then "OK" is the text in the OK button.  Use this to customize the OK text.	*/

short		doDualChoice( const StringPtr prompt, const short defaultButton, 
				const StringPtr okText )
{
	DialogPtr		dlog;
	short			item;
	GrafPtr			oldPort;
	OSErr			err;
	Str255			actionString;
	ControlHandle	okButtonHandle;

	dlog = GetNewDialog( rDualChoiceDLOG, 0L, kMoveToFront );
	
	if ( dlog != nil )
	{
		GetPort( &oldPort );
		SetPort( dlog );

		setDItemText( dlog, dDualChoiceText, prompt );
		
		okButtonHandle = ( ControlHandle ) getDItemHandle( dlog, dDualChoiceOK );
		
		if ( okText == nil )
		{
			GetIndString( actionString, rDialogUtilsText, kDialogUtilsTextOK );

			SetCTitle( okButtonHandle, actionString );
		}
		else
		{
			SetCTitle( okButtonHandle, okText );
		}
		
		ShowWindow( dlog );

		highlightDefault( dlog, defaultButton );

		( ( DialogPeek ) dlog )->aDefItem = defaultButton;
		
		do 
		{
			ModalDialog( genericFilterProc, &item );
		} while( item != dDualChoiceOK AND item != dDualChoiceCancel );
				
		DisposDialog( dlog );
		SetPort( oldPort );
		
		return( item );
	}
	
	return( 0 );
}


/************************************************************************ genericFilterProc() *******/

pascal Boolean	genericFilterProc( DialogPtr dlog,EventRecord *event,short *itemHit )
{
	char 		theChar;
	short		mouseClick;
	Point		localPoint;

	mouseClick = -1;
	
	localPoint = event->where;
	GlobalToLocal( &localPoint );
	
	switch( event->what )
	{
		case osEvt:
			break;
		case mouseDown:
			break;
		case keyDown:
		case autoKey:
			theChar = event->message & charCodeMask;
			
			if ( ( ( event->modifiers & cmdKey) != 0 AND ( theChar == '.' ) ) OR theChar == kEscape )
			{
				/* Simulates a Mouse Click in the Cancel Button */
				mouseClick = *itemHit = cancel;
			}
			else
			if ( theChar == kReturn OR theChar == kEnter )
			{
				/* Simulates a Mouse Click in the default Button */
				mouseClick = *itemHit = ( ( DialogPeek ) dlog )->aDefItem;
			}
			else
			{
				switch ( theChar )
				{
					default:
						return( false );
						break;
				}
			}
			break;
		case updateEvt:
			if ( ( DialogPtr ) event->message == dlog )
				highlightDefault( dlog, ( ( DialogPeek ) dlog )->aDefItem );
			break;
		default:
			return( false );
	}
	
	if ( mouseClick != -1 )
	{
		fakeMouseClick( dlog, mouseClick );
		
		return( true );
	}
	else
		return( false );
}


