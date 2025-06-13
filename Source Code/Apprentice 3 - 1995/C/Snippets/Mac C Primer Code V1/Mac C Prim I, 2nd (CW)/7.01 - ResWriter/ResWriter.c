/********************************************************/
/*														*/
/*  ResWriter Code from Chapter Seven of				*/
/*														*/
/*	*** The Macintosh Programming Primer, 2nd Ed. ***	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/********************************************************/

#define kBaseResID			128
#define kMoveToFront		(WindowPtr)-1L

#define iText				4

#define kDisableButton		255
#define kEnableButton		0

#define kWriteTextOut		true
#define kDontWriteTextOut	false

#define kMinTextPosition	0
#define kMaxTextPosition	32767


/***************/
/*  Functions  */
/***************/

void	ToolBoxInit( void );
Boolean	DoTextDialog( StringHandle oldTextHandle );


/*pascal OSErr SetDialogDefaultItem(DialogPtr theDialog, short newItem) 
    = { 0x303C, 0x0304, 0xAA68 };        
pascal OSErr SetDialogCancelItem(DialogPtr theDialog, short newItem)
    = { 0x303C, 0x0305, 0xAA68 };
pascal OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks)
    = { 0x303C, 0x0306, 0xAA68 };*/


/**************************** main **********************/

void	main( void )
{
	StringHandle	textHandle;
	
	ToolBoxInit();
	
	textHandle = GetString( kBaseResID );
	
	if ( textHandle == nil )
	{
		SysBeep( 20 );
		ExitToShell();
	}
	
	if ( DoTextDialog( textHandle ) == kWriteTextOut )
	{
		ChangedResource( (Handle)textHandle );
		WriteResource( (Handle)textHandle );
	}
}


/****************** ToolBoxInit *********************/

void	ToolBoxInit( void )
{
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}


/****************** DoTextDialog *********************/

Boolean	DoTextDialog( StringHandle textHandle )
{
	DialogPtr		dialog;
	Boolean			done;
	short			itemHit, itemType;
	Handle			OKItemHandle, textItemHandle;
	Rect			itemRect;
	Str255			itemText;
	
	dialog = GetNewDialog( kBaseResID, nil, kMoveToFront );
	
	GetDItem( dialog, ok, &itemType, &OKItemHandle, &itemRect );
	GetDItem( dialog, iText, &itemType, &textItemHandle, &itemRect );
	
	HLock( (Handle)textHandle );
	SetIText( textItemHandle, *textHandle );
	HUnlock( (Handle)textHandle );
	
	SelIText( dialog, iText, kMinTextPosition, kMaxTextPosition );
	
	ShowWindow( dialog );
	SetPort( dialog );
	
	SetDialogDefaultItem( dialog, ok );
	SetDialogCancelItem( dialog, cancel );
	SetDialogTracksCursor( dialog, true );
	
	done = false;
	while ( ! done )
	{
		GetIText( textItemHandle, itemText );
		
		if ( itemText[ 0 ] == 0 )
			HiliteControl( (ControlHandle)OKItemHandle, kDisableButton );
		else
			HiliteControl( (ControlHandle)OKItemHandle, kEnableButton );
			
		ModalDialog( nil, &itemHit );
		
		done = ( (itemHit == ok) || (itemHit == cancel) );
	}
	
	if ( itemHit == ok )
	{
		GetIText( textItemHandle, itemText );
		SetHandleSize( (Handle)textHandle, (Size)(itemText[ 0 ] + 1) );
		
		HLock( (Handle)textHandle );
		GetIText( textItemHandle, *textHandle );
		HUnlock( (Handle)textHandle );
	
		DisposDialog( dialog );
		
		return( kWriteTextOut );
	}
	else
	{
		DisposDialog( dialog );
		
		return( kDontWriteTextOut );
	}
}