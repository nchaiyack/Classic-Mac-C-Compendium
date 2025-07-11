// On the Edge

#include "main.h"

#define FindSuit(card) ( (card) & 3 )
#define FindValue(card) ( ( (card) >> 2 ) + 1 )

WindowPtr myWindow;
Handle suits;
Byte deck[52];
Byte grid[4][4];
Boolean multiple[4][4];
Byte deckPos;
Boolean finished = false, gameInProgress = false, selectingMultiples = false;
Boolean colorQDAvail;
RGBColor litGray = {0xE000, 0xE000, 0xE000},
		 midGray = {0xD000, 0xD000, 0xD000},
		 drkGray = {0xA000, 0xA000, 0xA000};

void main( void )
{
	Initialize( );
	
	RetrieveResources( );
	
	EmptyGrid( );
	
	do EventLoop( ); 
	while( !finished );
}

void StartGame( void )
{
	DialogPtr myDialog;
	short response;
	
	if( gameInProgress )
	{
		myDialog = GetNewDialog( 129, nil, (WindowPtr) -1L );
		
		ShowWindow( myDialog );
		SetPort( myDialog );
		
		OutlineButton( myDialog, iYes );
		
		ModalDialog( nil, &response );
		
		DisposDialog( myDialog );
		
		if( response == iNo )
			return;
	}
	
	gameInProgress = true;
	
	selectingMultiples = false;
	
	EmptyGrid( );

	SetPort( myWindow );
	BackColor( whiteColor );
	EraseRect( &( myWindow->portRect ) );
	RedrawWindow( );
}

void EmptyGrid( void )
{
	short count, count2;
		
	for( count = 0; count <= 3; count++ )
	{
		for( count2 = 0; count2 <= 3; count2++ )
		{
			grid[count][count2] = Empty;
			multiple[count][count2] = false;
		}
	}
	
	Shuffle( );
}

void PromptMessage( short messageID )
{
	Str255 message;
	
	GetIndString( message, 128, messageID );
	
	SetPort( myWindow );
	MoveTo( 20, 290 );
	TextFont( systemFont );
	TextSize( 12 );
	ForeColor( blackColor );
	
	DrawString( message );
}

void ClearMessage( void )
{
	Rect myRect = {257, 0, 320, 256};
	
	BackColor( whiteColor );
	EraseRect( &myRect );
}

void EventLoop( void )
{
	EventRecord e;
	WindowPtr window;
	short part;
	
	do {
		SetCursor( &qd.arrow );
		WaitNextEvent( everyEvent, &e, -1L, nil );
		
		switch( e.what )
		{
			case keyDown:
			if( e.modifiers & cmdKey )
				HandleMenus( MenuKey( e.message & charCodeMask ) );
			break;
			
			case mouseDown:
			part = FindWindow( e.where, &window );
			switch( part )
			{		
				case inDrag:
				DragWindow( window, e.where, &qd.screenBits.bounds );
				break;
				
				case inMenuBar:
				HandleMenus( MenuSelect( e.where ) );
				break;
				
				case inSysWindow:
				SystemClick( &e, window );
				break;
				
				case inContent:
				SetPort( window );
				GlobalToLocal( &e.where );
				HandleClick( &e.where );
				break;
			}
			break;
			
			case updateEvt:	
			BeginUpdate( (WindowPtr) e.message );
			SetPort( (WindowPtr) e.message );
			RedrawWindow( );
			EndUpdate( (WindowPtr) e.message );
			break;
		}
	}
	while( !finished );
}

void HandleMenus( long selection )
{
	register short menu, item;
	
	menu = selection>>16;
	item = (short) selection;

	switch( menu )
	{
		case mApple:
		
		if( item == iAbout )
		{
			Alert( 128, nil );
		}
		else
		{
			MenuHandle appleMenu;
			Str255 deskAccessory;
			
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, deskAccessory );
			OpenDeskAcc( deskAccessory );
		}
		break;
		
		case mFile:
		switch( item )
		{
			case iPlay:
			StartGame( );
			break;
			
			case iHelp:
			Alert( 130, nil );
			Alert( 131, nil );
			break;
			
			case iQuit:
			finished = true;
			break;
		}
		break;
		
		case mEdit:
		break;
	}
	HiliteMenu( 0 );
		
}

void HandleClick( Point *wherePtr )
{
	if( gameInProgress )
	{
		if( selectingMultiples )
			SelectMultiples( wherePtr );
		else
			PlaceCards( wherePtr );
	}
}

void SelectMultiples( Point *wherePtr )
{
	Rect myRect;
	Point where;
	short total = 0, count, count2;
	
	where = *wherePtr;
	
	where.h >>= 6;
	where.v >>= 6;
	
	if( where.v > 3 )
	{
		FinishSelecting( );
		return;
	}
	
	for( count = 0; count <=3; count++ )
	{
		for( count2 = 0; count2 <=3; count2++ )
		{
			if( multiple[count][count2] )
				total += FindValue( grid[count][count2] );
		}
	}
	
	if( multiple[where.h][where.v] )
	{
		multiple[where.h][where.v] = false;
		DrawSquare( where.h, where.v );
		return;
	}
	else
	{
		total += FindValue( grid[where.h][where.v] );
		
		if( total > 10 )
			return;
		
		multiple[where.h][where.v] = true;
		DrawSquare( where.h, where.v );
		
		if( total == 10 )
		{
			for( count = 0; count <=3; count++ )
			{
				for( count2 = 0; count2 <=3; count2++ )
				{
					if( multiple[count][count2] )
					{
						grid[count][count2] = Empty;
						multiple[count][count2] = false;
						
						DrawSquare( count, count2 );
					}
				}
			}
		}
	}
}

void PlaceCards( Point *wherePtr )
{
	Point where;
	Byte currentCard, cardValue;
	short count, count2;
	
	where = *wherePtr;
	
	where.h >>= 6;
	where.v >>= 6;
	
	if( where.v > 3 )
		return;
	
	if( grid[ where.h ][ where.v ] != Empty )
		return;
	
	currentCard = deck[ deckPos ];
	cardValue = FindValue( currentCard );
	
	if( cardValue == King &&
		!( (where.h == 0 || where.h == 3 ) &&
		   (where.v == 0 || where.v == 3 ) )
	  )
		return;
	
	if( cardValue == Queen &&
		!( (where.h == 1 && where.v == 0) ||
		   (where.h == 2 && where.v == 0) ||
		   (where.h == 1 && where.v == 3) ||
		   (where.h == 2 && where.v == 3) )
	  )
		return;
	
	if( cardValue == Jack &&
		!( (where.h == 0 && where.v == 1) ||
		   (where.h == 0 && where.v == 2) ||
		   (where.h == 3 && where.v == 1) ||
		   (where.h == 3 && where.v == 2) )
	  )
		return;
	
	grid[where.h][where.v] = currentCard;
	
	DrawCard( currentCard, where.h, where.v );
	
	if( FindValue(grid[0][0]) == King && FindValue(grid[0][3]) == King &&
		FindValue(grid[3][0]) == King && FindValue(grid[3][3]) == King &&
		FindValue(grid[1][0]) == Queen && FindValue(grid[2][0]) == Queen &&
	    FindValue(grid[1][3]) == Queen && FindValue(grid[2][3]) == Queen &&
	    FindValue(grid[0][1]) == Jack && FindValue(grid[0][2]) == Jack &&
	    FindValue(grid[3][1]) == Jack && FindValue(grid[3][2]) == Jack )
	{
		ClearMessage( );
		
		PromptMessage( msgYouWin );
		
		gameInProgress = false;
		
		return;
	}
	
	selectingMultiples = true;
	for( count = 0; count<=3; count++ )
	{
		for( count2 = 0; count2<=3; count2++ )
		{
			if( grid[count][count2] == Empty )
				selectingMultiples = false;
		}
	}
	
	if( selectingMultiples )
	{
		StartSelecting( );
		return;
	}
	
	DrawNewCard( );
}

void StartSelecting( void )
{
	ClearMessage( );
	
	PromptMessage( msgSelectMultOf10 );
}

void FinishSelecting( void )
{
	Rect myRect;
	short count, count2;
	
	ClearMessage( );
	
	for( count = 0; count <= 3; count++ )
	{
		for( count2 = 0; count2 <= 3; count2++ )
		{
			if( multiple[count][count2] == true )
			{
				multiple[count][count2] = false;
				DrawSquare( count, count2 );
			}
			
			if( grid[count][count2] == Empty )
			{
				selectingMultiples = false;
			}
		}
	}
	
	if( selectingMultiples )
	{
		gameInProgress = selectingMultiples = false;
		
		ClearMessage( );
		PromptMessage( msgGameOver );
		
		return;
	}
	
	DrawNewCard( );
}

void DrawNewCard( void )
{
	short currentCard, cardValue;
	
	currentCard = deck[ ++deckPos ];
	cardValue = FindValue( currentCard );
	DrawCard( currentCard, 3, 4 );
	
	if( ( cardValue == King &&
	      !( (grid[0][0] == Empty) ||
	         (grid[0][3] == Empty) ||
	         (grid[3][0] == Empty) ||
	         (grid[3][3] == Empty) )
	    )
	 || ( cardValue == Queen &&
          !( (grid[1][0] == Empty) ||
	         (grid[2][0] == Empty) ||
	         (grid[1][3] == Empty) ||
	         (grid[2][3] == Empty) )
	    )
	 || ( cardValue == Jack &&
	 	  !( (grid[0][1] == Empty) ||
	         (grid[0][2] == Empty) ||
	         (grid[3][1] == Empty) ||
	         (grid[3][2] == Empty) )
	    )
	  )
	{
		gameInProgress = false;
		
		PromptMessage( msgGameOver );
	}
}

void RedrawWindow( void )
{
	short count, count2;
	Rect myRect;
	
	for( count = 0; count<=3; count++ )
	{
		for( count2 = 0; count2<=3; count2++ )
		{
			DrawSquare( count2, count );
		}
	}
	
	if( selectingMultiples )
		PromptMessage( msgSelectMultOf10 );
	else
		DrawCard( deckPos < 52? deck[deckPos]: Empty, 3, 4 );
	
	if( !gameInProgress )
	{
		PromptMessage( msgGameOver );
	}
}

void DrawSquare( short x, short y )
{
	Rect myRect;
	
	myRect.top = y*64;
	myRect.bottom = myRect.top + 65;
	myRect.left = x*64;
	myRect.right = myRect.left + 65;
	
	ForeColor( blueColor );
	FrameRect( &myRect );

	InsetRect( &myRect, 1, 1 );
	if( colorQDAvail )
	{
		RGBBackColor( &litGray );
		EraseRect( &myRect );
		ForeColor( whiteColor );
		FrameRect( &myRect );
		
		InsetRect( &myRect, 1, 1 );
		FrameRect( &myRect );
		InsetRect( &myRect, -1, -1 );
		
		RGBForeColor( &drkGray );
		MoveTo( myRect.right-1, myRect.top );
		LineTo( myRect.right-1, myRect.bottom-1 );
		LineTo( myRect.left, myRect.bottom-1 );
		LineTo( myRect.left+1, myRect.bottom-2 );
		LineTo( myRect.right-2, myRect.bottom-2 );
		LineTo( myRect.right-2, myRect.top+1 );
	}
	else
	{
		BackColor( whiteColor );
		EraseRect( &myRect );
	}
	InsetRect( &myRect, -1, -1 );
	
	if( grid[x][y] != Empty )
	{
		DrawCard( grid[x][y], x, y );
		if( multiple[x][y] )
		{
			myRect.top = (y*64) + 10;
			myRect.left = (x*64) + 30;
			myRect.bottom = myRect.top + 16;
			myRect.right = myRect.left + 16;
			
			ForeColor( blueColor );
			
			if( colorQDAvail )
				RGBBackColor( &midGray );
			else
				BackColor( whiteColor );
				
			DrawSICN( &myRect, suits, 4 );
		}
	}
	else
	{
		LabelGrid( x, y );
	}
}

void DrawCard( Byte card, short horiz, short vert )
{
	Rect myRect;
	Byte mySuit, myValue;
		
	myRect.top = vert*64;
	myRect.bottom = myRect.top + 64;
	myRect.left = horiz*64;
	myRect.right = myRect.left + 64;
	
	mySuit = FindSuit( card );
	myValue = FindValue( card );
	
	if( colorQDAvail )
		DrawColorCard( &myRect, mySuit, myValue );
	else
		DrawBWCard( &myRect, mySuit, myValue );
}

void DrawBWCard( Rect *myRectP, short mySuit, short myValue )
{
	Rect myRect;
	char disp[] = "A 2 3 4 5 6 7 8 9 10J Q K ";

	myRect = *myRectP;
	
	InsetRect( &myRect, 10, 5 );
	
	if( myValue <= 13 )
	{
		FrameRoundRect( &myRect, 8, 8 );
		
		TextFont( geneva );
		TextSize( 12 );
		MoveTo( myRect.left + 4, myRect.top + 15 );
		DrawText( disp, (myValue-1)*2, 2 );
		
		myRect.bottom -= 5;
		myRect.right -= 5;
		myRect.top = myRect.bottom - 16;
		myRect.left = myRect.right - 16;
		
		ForeColor( mySuit <= 1? redColor: blackColor );
		DrawSICN( &myRect, suits, mySuit );
	}
}

void DrawColorCard( Rect *myRectP, short mySuit, short myValue )
{
	Rect myRect;
	char disp[] = "A 2 3 4 5 6 7 8 9 10J Q K ";
	RgnHandle myRegion, tempRegion;
	
	myRect = *myRectP;

	InsetRect( &myRect, 10, 5 );
	
	if( myValue <= 13 )
	{
		ForeColor( blackColor );
		FrameRoundRect( &myRect, 8, 8 );
		InsetRect( &myRect, 1, 1 );
		RGBForeColor( &midGray );
		PaintRoundRect( &myRect, 8, 8 );
		
		
		ForeColor( whiteColor );
		FrameRoundRect( &myRect, 8, 8 );
		InsetRect( &myRect, 1, 1 );
		FrameRoundRect( &myRect, 8, 8 );
		
		tempRegion = NewRgn( );
		myRegion = NewRgn( );
		GetClip( tempRegion );
		
		OpenRgn( );
		MoveTo( myRect.right+1, myRect.top );
		LineTo( myRect.left, myRect.bottom+1 );
		LineTo( myRect.right+1, myRect.bottom+1 );
		LineTo( myRect.right+1, myRect.top );
		CloseRgn( myRegion );
		SetClip( myRegion );
		RGBForeColor( &drkGray );
		FrameRoundRect( &myRect, 8, 8 );
		InsetRect( &myRect, -1, -1 );
		FrameRoundRect( &myRect, 8, 8 );
		
		SetClip( tempRegion );
		DisposeRgn( myRegion );
		DisposeRgn( tempRegion );
		
		InsetRect( &myRect, -1, -1 );
		
		TextFont( geneva );
		TextSize( 12 );
		
		ForeColor( whiteColor );
		MoveTo( myRect.left + 5, myRect.top + 16 );
		DrawText( disp, (myValue-1)*2, 2 );
		
		ForeColor( blackColor );
		MoveTo( myRect.left + 4, myRect.top + 15 );
		DrawText( disp, (myValue-1)*2, 2 );
		
		myRect.bottom -= 5;
		myRect.right -= 5;
		myRect.top = myRect.bottom - 16;
		myRect.left = myRect.right - 16;
		
		ForeColor( mySuit <= 1? redColor: blackColor );
		RGBBackColor( &midGray );
		DrawSICN( &myRect, suits, mySuit );
	}
}

void LabelGrid( short horiz, short vert )
{
	Str255 message;
	
	GetIndString( message, 129, (vert*4)+horiz+1 );
	
	TextFont( geneva );
	TextSize( 9 );
		
	if( colorQDAvail )
	{
		MoveTo( horiz*64 + 21, vert*64 + 37);
		ForeColor( whiteColor );
		DrawString( message );
	}
	
	MoveTo( horiz*64 + 20, vert*64 + 36);
	ForeColor( redColor );
	DrawString( message );
}

void OutlineButton( DialogPtr theDialog, short which )
{
	short itemType;
	Handle itemHandle;
	Rect itemRect;
	
	GetDItem( theDialog, which, &itemType, &itemHandle, &itemRect );
	InsetRect( &itemRect, -4, -4 );
	PenSize( 3, 3 );
	FrameRoundRect( &itemRect, 16, 16 );
}

void DrawSICN( Rect *destRect, Handle SICNhandle, short which )
{
	BitMap myBitMap;
	Rect srcRect = {0, 0, 16, 16};
	
	HLock( SICNhandle );
	
	myBitMap.rowBytes = 2;
	myBitMap.bounds = srcRect;
	myBitMap.baseAddr = *SICNhandle + (which * 32);
	
	CopyBits( &myBitMap, &myWindow->portBits, &srcRect, destRect, srcCopy, nil );
	
	HUnlock( SICNhandle );
}

void Initialize( void )
{
	SysEnvRec thisSysInfo;
	
	MaxApplZone( );	
	MoreMasters( );

	InitGraf( &qd.thePort );
	InitFonts( );
	FlushEvents( everyEvent, 0L );
	InitWindows( );
	InitMenus( );
	TEInit( );
	InitDialogs( 0L );
	InitCursor( );

	// GetDateTime( (unsigned long *)( &randSeed ) ); // seed the random # generator
	GetDateTime( (unsigned long *)LMGetRndSeed() );
	
	SysEnvirons( curSysEnvVers, &thisSysInfo );
	colorQDAvail = thisSysInfo.hasColorQD;
}

void RetrieveResources( void )
{
	short count;
	Handle menuBar;
	MenuHandle menu;
	
	// menus
	
	menuBar = GetNewMBar( 128 );
	if( menuBar == nil )
	{
		SysBeep( 1 );
		ExitToShell( );
	}
	
	SetMenuBar( menuBar );
	
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	DrawMenuBar( );
	
	// window
	
	if( colorQDAvail )
		myWindow = GetNewCWindow( 128, nil, (WindowPtr) -1L );
	else
		myWindow = GetNewWindow( 128, nil, (WindowPtr) -1L );
	
	if( myWindow == nil )
	{
		SysBeep( 1 );
		ExitToShell( );
	}
	ShowWindow( myWindow );
	SetPort( myWindow );
	
	// suit SICNs
	
	suits = GetResource( 'SICN', 128 );
}

void Shuffle( void )
{
	short count, count2, swap, temp;
	
	deckPos = 0;
	
	for( count = 0; count <= 51; count++ )
	{
		deck[count] = count;
	}
	
	for( count2 = 1; count2 <= 20; count2++ )
	{
		for( count = 0; count <= 51; count++ )
		{
			swap = RandomBefore( 52 );
			temp = deck[ count ];
			deck[ count ] = deck[ swap ];
			deck[ swap ] = temp;
		}
	}
}

short RandomBefore( short what )
{
	short random;
	
	random = Random( );
	
	if( random < 0 )
		random = -random;
	
	random %= what;
	
	return random;
}