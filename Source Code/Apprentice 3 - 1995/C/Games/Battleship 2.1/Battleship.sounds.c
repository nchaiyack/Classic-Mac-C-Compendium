#include	"Battleship.sounds.h"

/********/
/* main */
/********/

void	main ( void )
{
	ToolBoxInit ();
	MenuBarInit ();
	
	HandleAppleChoice ( iAbout );
	useColor = IsColour();

	WindowInit ();
	PlaceEnemyShips ();
	
	NewGame ();
	EventLoop ();
}
			

/**************/
/* ToolBoxInit*/
/**************/

void	ToolBoxInit ( void )
{
	InitGraf ( &thePort );
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs ( nil );
	InitCursor ();
}

/**************/
/* WindowInit */
/**************/

void	WindowInit ( void )
{
	if ( useColor )
		youWindow = GetNewCWindow ( kYouWindowID, nil, kMoveToFront );
	else
		youWindow = GetNewWindow ( kYouWindowID, nil, kMoveToFront );
	
	if ( youWindow == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	
	SetWRefCon ( youWindow, (long) kYouWindowID );

	SetPort ( youWindow );
	ShowWindow ( youWindow );

	if ( useColor )
		enemyWindow = GetNewCWindow ( kEnemyWindowID, nil, kMoveToFront );
	else
		enemyWindow = GetNewWindow ( kEnemyWindowID, nil, kMoveToFront );
	
	if ( enemyWindow == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	
	SetWRefCon ( enemyWindow, (long) kEnemyWindowID );
	SetPort ( enemyWindow );
	ShowWindow ( enemyWindow );
}


/***************/
/* MenuBarInit */
/***************/

void	MenuBarInit ( void )
{
	Handle			menuBar;
	MenuHandle		menu;
	
	menuBar = GetNewMBar ( kBaseResID );
	SetMenuBar ( menuBar );
	
	menu = GetMHandle ( mApple );
	AddResMenu ( menu, 'DRVR' );
	
	DrawMenuBar ();
	
	HandleSound ();
}

/***************/
/* HandleSound */
/***************/

void	HandleSound ( void )
{
	MenuHandle	menuHandle;
	
	menuHandle = GetMHandle ( mOptions );
	
	
	if ( gHasSound )
	{
		CheckItem ( menuHandle, iSound, kRemoveCheckMark );
		gHasSound = !gHasSound;
	}
	else
	{
		CheckItem ( menuHandle, iSound, kAddCheckMark );
		gHasSound = !gHasSound;
	}
}

/*************/
/* EventLoop */
/*************/

void	EventLoop ( void )
{
	EventRecord	event;
	DialogPtr	dialog;
	Boolean		dialogDone = FALSE;
	int			result;
	Handle		mySndHandle;
	
	gDone = false;
	while ( gDone == false )
	{
		if ( WaitNextEvent ( everyEvent, &event, MAXLONG, nil ) )
			DoEvent ( &event );

		if ( enemyCount >= 17 ) /* you win! */
		{
			SelectWindow ( enemyWindow );
			SetPort ( enemyWindow );
			DrawDots ( enemy );
			
			if ( gHasSound )
			{
				mySndHandle = GetResource ( 'snd ', kYouWinSnd );
				if ( mySndHandle != NULL )
					SndPlay ( nil, mySndHandle, TRUE );
			}
			
			dialog = GetNewDialog ( kBaseResID + 1, nil, kMoveToFront );
			SetDialogDefaultItem ( dialog, ok );
			SetDialogCancelItem ( dialog, cancel );
			ParamText( kCongratulationsStr, kYouWinStr, "\p", "\p" );
			SetPort ( dialog );
			ShowWindow ( dialog );
			while ( !dialogDone )
			{
				ModalDialog ( nil, &result );
				
				switch ( result )
				{
					case ok:
						dialogDone = TRUE;
						DisposDialog  ( dialog );
						NewGame ();
						break;
					case cancel:
						DisposDialog  ( dialog );
						dialogDone = TRUE;
						gDone = TRUE;
						break;
				}
			}
			dialogDone = FALSE;
		}
		else if ( ( gYouHasGone ) && ( !gDone ) )
		{
			PickEnemyShot ();
			gYouHasGone = !gYouHasGone;
		}
		if ( youCount >= 17 ) /* computer wins! */
		{
			SelectWindow ( enemyWindow );
			SetPort ( enemyWindow );
			DrawDots ( enemy );
			
			if ( gHasSound )
			{
				mySndHandle = GetResource ( 'snd ', kYouLoseSnd );
				if ( mySndHandle != NULL )
					SndPlay ( nil, mySndHandle, TRUE );
			}
			
			dialog = GetNewDialog ( kBaseResID + 1, nil, kMoveToFront );
			SetDialogDefaultItem ( dialog, ok );
			SetDialogCancelItem ( dialog, cancel );
			ParamText( kSorryStr, kYouLoseStr, "\p", "\p" );
			SetPort ( dialog );
			ShowWindow ( dialog );
			while ( !dialogDone )
			{
				ModalDialog ( nil, &result );
				
				switch ( result )
				{
					case ok:
						dialogDone = TRUE;
						DisposDialog  ( dialog );
						NewGame ();
						break;
					case cancel:
						DisposDialog  ( dialog );
						dialogDone = TRUE;
						gDone = TRUE;
						break;
				}
			}
			dialogDone = FALSE;
		}
	}
}


/***********/
/* DoEvent */
/***********/

void	DoEvent ( EventRecord *eventPtr )
{
	Boolean	becomingActive;
	char	theChar;
	
	switch ( eventPtr->what )
	{
		case mouseDown:
			HandleMouseDown ( eventPtr );
			break;
		case keyDown:
		case autoKey:
			theChar = eventPtr->message & charCodeMask;
			if ( (eventPtr->modifiers & cmdKey) != 0 )
				HandleMenuChoice ( MenuKey ( theChar ) );
			break;
		case updateEvt:
			DoUpdate ( eventPtr );
			break;
		case activateEvt:
			becomingActive = ( ( eventPtr->modifiers &activeFlag ) == activeFlag );
			break;
	}
}

/*******************/
/* HandleMouseDown */
/*******************/

void	HandleMouseDown ( EventRecord *eventPtr )
{
	WindowPtr	window;
	short int	thePart;
	long		menuChoice;
	
	thePart = FindWindow ( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inMenuBar:
			menuChoice = MenuSelect ( eventPtr->where );
			HandleMenuChoice ( menuChoice );
			break;
		case inSysWindow:
			SystemClick ( eventPtr, window );
			break;
		case inContent:
			SelectWindow ( window );
			if ( GetWRefCon ( window ) == kEnemyWindowID )
			{
				SetPort ( window );
				CheckPoint ( eventPtr->where );
			}
			break;
		case inDrag:
			DragWindow ( window, eventPtr->where, &screenBits.bounds );
			break;
		case inGoAway:
			if ( TrackGoAway ( window, eventPtr->where ) )
				gDone = true;
			break;
	}
}

/************/
/* DoUpdate */
/************/

void	DoUpdate ( EventRecord *eventPtr )
{
	WindowPtr	window;
	
	window = ( WindowPtr ) eventPtr->message;
	
	BeginUpdate ( window );
	
	if ( GetWRefCon ( window ) == kEnemyWindowID )
	{
		SetPort ( window );
		DrawDots ( enemy );
	}

	if ( GetWRefCon ( window ) == kYouWindowID )
	{
		SetPort ( window );
		DrawDots ( you );
	}
	
	EndUpdate ( window );
}

/********************/
/* HandleMenuChoice */
/********************/

void	HandleMenuChoice ( long menuChoice )
{
	short	menu;
	short	item;
	
	if ( menuChoice != 0 )
	{
		menu = HiWord ( menuChoice );
		item = LoWord ( menuChoice );
		
		switch ( menu )
		{
			case mApple:
				HandleAppleChoice ( item );
				break;
			case mFile:
				HandleFileChoice ( item );
				break;
			case mOptions:
				HandleOptionsChoice ( item );
		}
		
		HiliteMenu ( 0 );
	}
}

/********************/
/* HandleAppleChoice*/
/********************/

void	HandleAppleChoice ( short item )
{
	MenuHandle	appleMenu;
	Handle		mySndHandle;
	Str255		accName;
	short		accNumber;
	DialogPtr	dialog;
	Boolean		dialogDone = FALSE, showHelp = FALSE;
	int			result;
	
	if ( gHasSound )
	{
		mySndHandle = GetResource ( 'snd ', kBombDropSnd );
		if ( mySndHandle != NULL )
			SndPlay ( nil, mySndHandle, TRUE );
		
		mySndHandle = GetResource ( 'snd ', kExplosionSnd );
		if ( mySndHandle != NULL )
			SndPlay ( nil, mySndHandle, TRUE );
	}
	
	switch ( item )
	{
		case iAbout:
			dialog = GetNewDialog ( kBaseResID + 3, nil, kMoveToFront );
			ParamText( kVersionNumberStr, "\p", "\p", "\p" );
			SetDialogDefaultItem ( dialog, ok );
			SetDialogCancelItem ( dialog, cancel );
			SetPort ( dialog );
			ShowWindow ( dialog );
			
			while ( !dialogDone )
			{
				ModalDialog ( nil, &result );
				
				switch ( result )
				{
					case ok:
						dialogDone = TRUE;
						break;
					case cancel:
						showHelp = TRUE;
						dialogDone = TRUE;
						break;
				}
			}
			DisposDialog  ( dialog );
			dialogDone = FALSE;
		
			if ( showHelp )
			{
				dialog = GetNewDialog ( kBaseResID + 4, nil, kMoveToFront );
				ParamText( kVersionNumberStr, "\p", "\p", "\p" );
				SetDialogDefaultItem ( dialog, ok );
				SetPort ( dialog );
				ShowWindow ( dialog );
				while ( !dialogDone )
				{
					ModalDialog ( nil, &result );
					
					switch ( result )
					{
						case ok:
							dialogDone = TRUE;
							break;
					}
				}
				DisposDialog  ( dialog );
			}
			break;
		default:
			appleMenu = GetMHandle ( mApple );
			GetItem ( appleMenu, item, accName );
			accNumber = OpenDeskAcc ( accName );
			break;
	}
}

/********************/
/* HandleFileChoice */
/********************/

void	 HandleFileChoice ( short item )
{
	switch ( item )
	{
		case iQuit:
			gDone = TRUE;
			break;
		case iNew:
			NewGame ();
			break;
	}
}

/***********************/
/* HandleOptionsChoice */
/***********************/

void	HandleOptionsChoice ( short item )
{
	switch ( item )
	{
		case iSound:
			HandleSound ();
			break;
	}
}

/***********/
/* NewGame */
/***********/

void	NewGame ( void )
{
	int			i;
	DialogPtr	dialog;
	
	youCount = 0;
	enemyCount = 0;
	shotCount = 0;
	gYouHasGone = FALSE;
	
	for ( i = 0; i < kNumShips; i++ )
		hitShips[i] = 0;
	
	GridInit ( enemy );
	GridInit ( you );
	SetPort ( enemyWindow );
	DrawDots ( enemy );
	SetPort ( youWindow );
	DrawDots ( you );
	PlaceEnemyShips ();
	
	dialog = GetNewDialog ( kBaseResID, nil, kMoveToFront );
	SetDialogDefaultItem ( dialog, ok );
	SetDialogCancelItem ( dialog, cancel );
	ParamText( kRotateStr, kPlaceStr, "\p", "\p" );
	SetPort ( dialog );
	ShowWindow ( dialog );
	DrawDialog ( dialog );
	
	SelectWindow ( youWindow );
	SetPort ( youWindow );
	
	PlaceAShip ( kCarrierMin, kCarrierMax, kHorizCarrierID, kCarrier );
	PlaceAShip ( kBattleshipMin, kBattleshipMax, kHorizBattleshipID, kBattleship );
	PlaceAShip ( kCruiserMin, kCruiserMax, kHorizCruiserID, kCruiser );
	PlaceAShip ( kSubMin, kSubMax, kHorizSubID, kSub );
	PlaceAShip ( kDestroyerMin, kDestroyerMax, kHorizDestroyerID, kDestroyer );
	
	DisposeDialog ( dialog );
	
	SelectWindow ( enemyWindow );
	SetPort ( enemyWindow );
}

/**************/
/* PlaceAShip */
/**************/

void	PlaceAShip ( int theMin, int theMax, int theHorizID, int theShip )
{
	EventRecord			event;
	Point				thePoint, tempPoint, oldPoint;
	Rect				theRect, tempRect, oldRect;
	char				theChar;
	Boolean				horiz = TRUE, oldHoriz = TRUE;
	
	do /* gets a random temp point for the placement of the first ship */
	{
		tempPoint.v = Randomize ();
		tempPoint.h = Randomize ();
		
		Inbounds ( &tempPoint, horiz, theMin, theMax );

	} while ( !ItFits ( horiz, tempPoint, theMin, theMax ) );
	
	PtToShipRect ( tempPoint, &oldRect, horiz, theMin, theMax );
	PtToRect ( tempPoint, &tempRect );
	oldPoint = tempPoint;
	
	DrawShipPart ( theHorizID, oldRect );
	
	while ( !Button () )
	{
		GetMouse ( &thePoint );

		if ( WaitNextEvent ( everyEvent, &event, MAXLONG, nil ) )
		{
			if ( ( event.what == keyDown ) || ( event.what == autoKey ) )
				horiz = !horiz;
/*			theChar = event.message & charCodeMask;
			if ( theChar == 'm' )
				horiz = !horiz;
*/		}
		
		tempPoint.v = thePoint.v / kPixelSize;
		tempPoint.h = thePoint.h / kPixelSize;
		
		/* This is really long.  If someone can shorten it, I would appreciate it. */
		if ( 		( !PtInRect ( thePoint, &tempRect ) ) &&
					( Inbounds ( &tempPoint, horiz, theMin, theMax ) ) && 
					( ItFits ( horiz, tempPoint, theMin, theMax ) ) && 
					( ( oldPoint.h != tempPoint.h ) || ( oldPoint.v != tempPoint.v ) ) || 
					( ( oldHoriz != horiz ) &&
					( Inbounds ( &tempPoint, horiz, theMin, theMax ) ) &&
					( ItFits ( horiz, tempPoint, theMin, theMax ) ) ) )
		{
			EraseShipPart ( oldRect );
			
			if ( oldHoriz )
				DrawHorizDots ( oldPoint, tempPoint, &theRect, theMin, theMax );
			else
				DrawVertDots ( oldPoint, tempPoint, &theRect, theMin, theMax );
			
			PtToShipRect ( tempPoint, &tempRect, horiz, theMin, theMax );
			if ( horiz )
				DrawShipPart ( theHorizID, tempRect );
			else
				DrawShipPart ( theHorizID + 1, tempRect );
	
			PtToShipRect ( tempPoint, &oldRect, horiz, theMin, theMax );
			PtToRect ( tempPoint, &tempRect );
			oldPoint = tempPoint;
			oldHoriz = horiz;
		}
		else if ( horiz != oldHoriz )
			horiz = oldHoriz;
	}
	
	SetShipToGrid ( you, oldPoint, horiz, theMin, theMax, theShip );
	
	while ( Button () );
}

/**********/
/* ItFits */
/**********/

Boolean	ItFits ( Boolean horiz, Point thePoint, int theMin, int theMax )
{
	Boolean fits = FALSE;
	
	if  ( horiz )
		fits = FitsHoriz ( you, thePoint.h - theMin, thePoint.v, 1 + theMin + theMax );
	else
		fits = FitsVert ( you, thePoint.h, thePoint.v - theMin, 1 + theMin + theMax );
		
	return ( fits );
}

/*****************/
/* DrawHorizDots */
/*****************/

void	DrawHorizDots ( Point aPoint, Point tempPoint, Rect *theRect, int min, int max )
{
	int			i = aPoint.h;
	int			test = i + max;
	PicHandle	picture;
	
	picture = GetPicture ( kNullDotID );
	if ( picture == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	
	for ( aPoint.h = i - min; aPoint.h <= test; aPoint.h++ ) 
	{
		/* PtToDotRect ( aPoint, theRect ); */
		(*theRect).left = ( aPoint.h + 1 ) * 19 - 10;
		(*theRect).top = ( aPoint.v + 1 ) * 19 - 10;
		(*theRect).right = ( aPoint.h + 1 ) * 19;
		(*theRect).bottom = ( aPoint.v + 1 ) * 19;
		
		DrawPicture ( picture, theRect );
	}
	
}

/****************/
/* DrawVertDots */
/****************/

void	DrawVertDots ( Point aPoint, Point tempPoint, Rect *theRect, int min, int max )
{
	int 		i = aPoint.v;
	int			test = i + max;
	PicHandle	picture;
	
	picture = GetPicture ( kNullDotID );
	if ( picture == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	
	for ( aPoint.v = i - min; aPoint.v <= test; aPoint.v++ )
	{
		/* PtToDotRect ( aPoint, theRect ); */
		(*theRect).left = ( aPoint.h + 1 ) * 19 - 10;
		(*theRect).top = ( aPoint.v + 1 ) * 19 - 10;
		(*theRect).right = ( aPoint.h + 1 ) * 19;
		(*theRect).bottom = ( aPoint.v + 1 ) * 19;
		
		DrawPicture ( picture, theRect );
	}
	
}

/***************/
/* PtToDotRect */
/***************/

void	PtToDotRect ( Point aPoint, Rect *aRect )
{
	(*aRect).left = ( aPoint.h + 1 ) * 19 - 10;
	(*aRect).top = ( aPoint.v + 1 ) * 19 - 10;
	(*aRect).right = ( aPoint.h + 1 ) * 19;
	(*aRect).bottom = ( aPoint.v + 1 ) * 19;
}

/************/
/* PtToRect */
/************/

void	PtToRect ( Point aPoint, Rect *aRect )
{
	(*aRect).left = ( aPoint.h + 1 ) * 19 - 16;
	(*aRect).top = ( aPoint.v + 1 ) * 19 - 16;
	(*aRect).right = ( aPoint.h + 1 ) * 19 + 6;
	(*aRect).bottom = ( aPoint.v + 1 ) * 19 + 6;
}

/****************/
/* PtToShipRect */
/****************/

void	PtToShipRect ( Point aPoint, Rect *aRect, Boolean horiz, int min, int max )
{
	if ( horiz )
	{
		(*aRect).left = ( aPoint.h + 1 ) * 19 - 10 - ( kPixelSize * min );
		(*aRect).top = ( aPoint.v + 1 ) * 19 - 10;
		(*aRect).right = ( aPoint.h + 1 ) * 19 + ( kPixelSize * max );
		(*aRect).bottom = ( aPoint.v + 1 ) * 19;
	}
	else
	{
		(*aRect).left = ( aPoint.h + 1 ) * 19 - 10;
		(*aRect).top = ( aPoint.v + 1 ) * 19 - 10 - ( kPixelSize * min );
		(*aRect).right = ( aPoint.h + 1 ) * 19;
		(*aRect).bottom = ( aPoint.v + 1 ) * 19 + ( kPixelSize * max );
	}
}

/*****************/
/* SetShipToGrid */
/*****************/
/* purpose: This function takes the current location of the ship upon a mouse down
/* 			and puts it into the grid.
/* variables: The grid to set it to; the point of the current mouse location
/*			after it has been translated to the grid coordinates; the vertical
/*			or horizontal orientation of the ship; the minimum grid point that the
/*			ship is at; the maximum grid point that the ship is at; the character
/*			type of the ship.
 */

void	SetShipToGrid ( Grid theGrid[][kGridSize], Point tempPoint, 
							Boolean horiz, int min, int max, int c )
{
	int i;
	
	if ( horiz )
	{
		if ( ( c == kCarrier ) || ( c == kBattleship ) ||
				( c == kDestroyer ) || ( c == kCruiser ) )
			theGrid[tempPoint.h - min][tempPoint.v].shipResID = 133;
		else
			theGrid[tempPoint.h - min][tempPoint.v].shipResID = 134;
		
		theGrid[tempPoint.h - min][tempPoint.v].shipType = c;
		
		for ( i = tempPoint.h - ( min - 1 ); i <= tempPoint.h + ( max - 1 ); i++ )
		{
			theGrid[i][tempPoint.v].shipResID = 135;
			theGrid[i][tempPoint.v].shipType = c;
		}
		
		if ( ( c == kSub ) || ( c == kCruiser ) || ( c == kDestroyer ) )
			theGrid[tempPoint.h + max][tempPoint.v].shipResID = 136;
		else
			theGrid[tempPoint.h + max][tempPoint.v].shipResID = 137;

		theGrid[tempPoint.h + max][tempPoint.v].shipType = c;
	}
	else
	{
		if ( ( c == kCarrier ) || ( c == kBattleship ) ||
				( c == kDestroyer ) || ( c == kCruiser ) )
			theGrid[tempPoint.h][tempPoint.v - min].shipResID = 138;
		else
			theGrid[tempPoint.h][tempPoint.v - min].shipResID = 139;
		
		theGrid[tempPoint.h][tempPoint.v - min].shipType = c;
		
		for ( i = tempPoint.v - ( min - 1 ); i <= tempPoint.v + ( max - 1 ); i++ )
		{
			theGrid[tempPoint.h][i].shipResID = 140;
			(theGrid[tempPoint.h][i].shipType) = c;
		}
		
		if ( ( c == kSub ) || ( c == kCruiser ) || ( c == kDestroyer ) )
			theGrid[tempPoint.h][tempPoint.v + max].shipResID = 141;
		else
			theGrid[tempPoint.h][tempPoint.v + max].shipResID = 142;
		
		(theGrid[tempPoint.h][tempPoint.v + max].shipType) = c;
	}
}

/************/
/* DrawDots */
/************/

void	DrawDots ( Grid theGrid[][kGridSize] )
{
	Rect		pictureRect;
	WindowPtr	window;
	PicHandle	hitDot, missDot, nullDot;
	int			i, j, windowSize;
	Boolean		isYou = FALSE;
	
	if ( theGrid == you )
		isYou = TRUE;
	
	window = FrontWindow ();
	windowSize = window->portRect.right - window->portRect.left;

	EraseRect ( &window->portRect );
	
	if ( useColor )
	{
		hitDot = GetPicture ( kHitDotID );
		if ( hitDot == nil )
		{
			SysBeep ( 10 );
			ExitToShell ();
		}
	}
	else
	{
		hitDot = GetPicture ( kBWHitDotID );
		if ( hitDot == nil )
		{
			SysBeep ( 10 );
			ExitToShell ();
		}
	}

	missDot = GetPicture ( kMissDotID );
	if ( missDot == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}

	nullDot = GetPicture ( kNullDotID );
	if ( nullDot == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	
	i = 0;
	for ( i; i <= 9; i++ )
	{
		j = 0;
		for ( j; j <= 9; j++ )
		{
			pictureRect.left = ( i + 1 ) * 19 - 10;
			pictureRect.top = ( j + 1 ) * 19 - 10;
			pictureRect.right = ( i + 1 ) * 19;
			pictureRect.bottom = ( j + 1 ) * 19;
			
			if ( isYou && ( theGrid[i][j].shipResID != 0 ) )
				DrawShipPart ( theGrid[i][j].shipResID, pictureRect );
			else if ( ( ( enemyCount >= 17 ) || ( youCount >= 17 ) ) &&
							( theGrid[i][j].shipResID != 0 ) )
				DrawShipPart ( theGrid[i][j].shipResID, pictureRect );
			
			if ( theGrid[i][j].hitOrMiss == noTry )
				DrawPicture ( nullDot, &pictureRect );
			else if ( theGrid[i][j].hitOrMiss == miss )
				DrawPicture ( missDot, &pictureRect );
			else
				DrawPicture ( hitDot, &pictureRect );
		}
	}
	CheckIfSink ( kCarrier, kNoSound );
	CheckIfSink ( kBattleship, kNoSound );
	CheckIfSink ( kCruiser, kNoSound );
	CheckIfSink ( kSub, kNoSound );
	CheckIfSink ( kDestroyer, kNoSound );
}

/****************/
/* DrawShipPart */
/****************/

void	DrawShipPart ( int resID, Rect pictureRect )
{
	PicHandle	picture;

	pictureRect.left -= 3;
	pictureRect.top -= 3;
	pictureRect.right += 3;
	pictureRect.bottom += 3;
			
	picture = GetPicture ( resID );
	if ( picture == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}

	DrawPicture ( picture, &pictureRect );
}

/*****************/
/* EraseShipPart */
/*****************/

void	EraseShipPart ( Rect pictureRect )
{
	pictureRect.left -= 3;
	pictureRect.top -= 3;
	pictureRect.right += 3;
	pictureRect.bottom += 3;
			
	EraseRect ( &pictureRect );
}

/************/
/* GridInit */
/************/

void	GridInit ( Grid aGrid[][kGridSize] )
{
	int		i, j;
	
	for ( i = 0; i < kGridSize; i++ )
	{
		for ( j = 0; j < kGridSize; j++ )
		{
			aGrid[i][j].shipType = kNoShip;
			aGrid[i][j].shipResID = 0;
			aGrid[i][j].hitOrMiss = noTry;
		}
	}
}

/*******************/
/* PlaceEnemyShips */
/*******************/

void	PlaceEnemyShips ( void )
{
	int		s;
	
	GetDateTime( &randSeed );
	
	GridInit ( enemy );

	PlaceAnEnemyShip ( kCarrierSize, kCarrierMin, kCarrierMax, kCarrier );
	PlaceAnEnemyShip ( kBattleshipSize, kBattleshipMin, kBattleshipMax, kBattleship );
	PlaceAnEnemyShip ( kCruiserSize, kCruiserMin, kCruiserMax, kCruiser );
	PlaceAnEnemyShip ( kSubSize, kSubMin, kSubMax, kSub );
	PlaceAnEnemyShip ( kDestroyerSize, kDestroyerMin, kDestroyerMax, kDestroyer );
}

/********************/
/* PlaceAnEnemyShip */
/********************/

void	PlaceAnEnemyShip ( int shipSize, int min, int max, int c )
{
	Boolean				ItDoesntFit = TRUE, horiz;
	int 				dir;
	Point				aPoint;
	
	while ( ItDoesntFit )
	{
		aPoint.h = Randomize ();
		aPoint.v = Randomize ();
		
		dir = Randomize ();
		
		if ( ( ( float ) dir / 2 ) == ( dir / 2 ) )   	/* It's even so horizontal */
		{
			if ( ( aPoint.h <= 10 - shipSize ) && ( aPoint.h >= 0 ) && 
						( FitsHoriz ( enemy, aPoint.h, aPoint.v, shipSize ) ) )
			{
				horiz = TRUE;
				aPoint.h += min;
				SetShipToGrid ( enemy, aPoint, horiz, min, max, c );
				ItDoesntFit = FALSE;
			}
		}
		else											/* It's odd so vertical */
		{
			if ( ( aPoint.v <= 10 - shipSize ) && ( aPoint.v >= 0 ) && 
						( FitsVert ( enemy, aPoint.h, aPoint.v, shipSize ) ) )
			{
				horiz = FALSE;
				aPoint.v += min;
				SetShipToGrid ( enemy, aPoint, horiz, min, max, c );
				ItDoesntFit = FALSE;
			}
		}
	}
}

/************/
/* FitsVert */
/************/

Boolean	FitsVert ( Grid theGrid[][kGridSize], int x, int y, int shipSize )
{
	Boolean fits = TRUE;
	int		i;
	
	for ( i = y; i < y + shipSize; i++ )
	{
		if ( theGrid[x][i].shipResID != 0 )
			fits = FALSE;
	}
	return ( fits );
}

/*************/
/* FitsHoriz */
/*************/

Boolean	FitsHoriz ( Grid theGrid[][kGridSize], int x, int y, int shipSize )
{
	Boolean fits = TRUE;
	int		i;
	
	for ( i = x; i < x + shipSize; i++ )
	{
		if ( theGrid[i][y].shipResID != 0 )
			fits = FALSE;
	}
	return ( fits );
}

/*************/
/* Randomize */
/*************/

int		Randomize ( void )
{
	long	randomNumber;
	
	randomNumber = Random ();
	
	if ( randomNumber < 0 )
		randomNumber *= -1;
	
	if ( randomNumber == 0 )
		return ( randomNumber );
	else
		return ( ( ( randomNumber * kRandomRange ) / kRandomUpperLimit ) );
}

/**************/
/* CheckPoint */
/**************/

void	CheckPoint ( Point mouseLocal )
{
	Rect		dotRect;
	Point		mouseCopy;
	Handle		mySndHandle;
	
	GlobalToLocal ( &mouseLocal );
	mouseCopy = mouseLocal;
	
	if ( mouseLocal.v == 200 )
		mouseLocal.v = 9;
	else
		mouseLocal.v /= kPixelSize;
	if ( mouseLocal.h == 200 )
		mouseLocal.h = 9;
	else
		mouseLocal.h /= kPixelSize;
	
	PtToDotRect ( mouseLocal, &dotRect );
			
	if ( ( PtInRect ( mouseCopy, &dotRect ) ) &&
				 ( enemy[mouseLocal.h][mouseLocal.v].hitOrMiss == noTry ) )
	{
		if ( gHasSound )
		{
			mySndHandle = GetResource ( 'snd ', kBombDropSnd );
			if ( mySndHandle != NULL )
				SndPlay ( nil, mySndHandle, TRUE );
		}
		
		if ( enemy[mouseLocal.h][mouseLocal.v].shipResID == 0 )
		{
			enemy[mouseLocal.h][mouseLocal.v].hitOrMiss = miss;
			DrawOneDot ( kMissDotID, &dotRect );
		
			if ( gHasSound )
			{
				mySndHandle = GetResource ( 'snd ', kSplashSnd );
				if ( mySndHandle != NULL )
					SndPlay ( nil, mySndHandle, TRUE );
			}
		}
		else
		{
			enemy[mouseLocal.h][mouseLocal.v].hitOrMiss = hit;
			(hitShips[(enemy[mouseLocal.h][mouseLocal.v].shipType)])++;
			DrawOneDot ( kHitDotID, &dotRect );
			CheckIfSink ( (enemy[mouseLocal.h][mouseLocal.v].shipType), kSound );
			
			if ( gHasSound )
			{
				mySndHandle = GetResource ( 'snd ', kExplosionSnd );
				if ( mySndHandle != NULL  )
					SndPlay ( nil, mySndHandle, TRUE );
			}
			
			enemyCount++;
		}
		
		gYouHasGone = TRUE;
	}
}


/***************/
/* CheckIfSink */
/***************/

int		CheckIfSink ( int theShip, Boolean play )
{
	Boolean	drawParts = FALSE;
	int		i = 0;
	int		j = 0;
	Rect	theRect;
	Handle	mySndHandle;
	
	switch ( theShip )
	{
		case kCarrier:
			if ( hitShips[theShip] == kCarrierSize )
				drawParts = TRUE;
			break;
		case kBattleship:
			if ( hitShips[theShip] == kBattleshipSize )
			{
				drawParts = TRUE;
				if ( gHasSound && play )
				{
					mySndHandle = GetResource ( 'snd ', kYouSankSnd );
					if ( mySndHandle != NULL )
						SndPlay ( nil, mySndHandle, TRUE );
				}
			}
			break;
		case kCruiser:
			if ( hitShips[theShip] == kCruiserSize )
				drawParts = TRUE;
			break;
		case kSub:
			if ( hitShips[theShip] == kSubSize )
				drawParts = TRUE;
			break;
		case kDestroyer:
			if ( hitShips[theShip] == kDestroyerSize )
				drawParts = TRUE;
			break;
	}
	if ( drawParts )
	{
		for ( i = 0; i < kGridSize; i++ )
		{
			for ( j = 0; j < kGridSize; j++ )
			{
				if ( (enemy[i][j].shipType) == theShip )
				{
					theRect.left = ( i + 1 ) * 19 - 10;
					theRect.top = ( j + 1 ) * 19 - 10;
					theRect.right = ( i + 1 ) * 19;
					theRect.bottom = ( j + 1 ) * 19;
					
					DrawShipPart ( enemy[i][j].shipResID,theRect );
				}
			}
		}
	}
}


/**************/
/* DrawOneDot */
/**************/

void	DrawOneDot ( int dotID, Rect *dotRect )
{
	PicHandle	picture;
	
	picture = GetPicture ( dotID );
	if ( picture == nil )
	{
		SysBeep ( 10 );
		ExitToShell ();
	}
	DrawPicture ( picture, dotRect );
}

/************/
/* Inbounds */
/************/

Boolean	Inbounds ( Point *tempPoint, Boolean horiz, int min, int max )
{
	Boolean	vTest = TRUE;
	Boolean hTest = TRUE;
	
	if ( horiz )
	{
		if ( (*tempPoint).h < min )
		{
			(*tempPoint).h = min;
			hTest = FALSE;
		}
		if ( (*tempPoint).h > ( 9 - max ) )
		{
			(*tempPoint).h = ( 9 - max );
			hTest = FALSE;
		}
		if ( (*tempPoint).v < 0 )
		{
			(*tempPoint).v = 0;
			vTest = FALSE;
		}
		if ( (*tempPoint).v > 9 )
		{
			(*tempPoint).v = 9;
			vTest = FALSE;
		}
	}
	
	else
	{
		if ( (*tempPoint).v < min )
		{
			(*tempPoint).v = min;
			hTest = FALSE;
		}
		if ( (*tempPoint).v > ( 9 - max ) )
		{
			(*tempPoint).v = ( 9 - max );
			hTest = FALSE;
		}
		if ( (*tempPoint).h < 0 )
		{
			(*tempPoint).h = 0;
			vTest = FALSE;
		}
		if ( (*tempPoint).h > 9 )
		{
			(*tempPoint).h = 9;
			vTest = FALSE;
		}
	}
		
	if ( ( hTest == FALSE ) && ( vTest == FALSE ) )
		return ( FALSE );
	else
		return ( TRUE );
}

/*****************/
/* PickEnemyShot */
/*****************/

void	PickEnemyShot ( void )
{
	Rect				theRect;
	
	static cond			lastTry = miss;
	static Point		lastPoint =  {0,0}, firstHitPoint  = {0,0};
	static Boolean		keepTrying = FALSE, isHorizontal = FALSE, hitsYet = FALSE;
	static Direction	lastDir = right;
	static Point		thisPoint = {0,0};
	
	if ( !youCount )
	{
		lastTry = miss;
		keepTrying = FALSE;
		isHorizontal = FALSE;
		hitsYet = FALSE;
		lastDir = right;
	}

	if ( keepTrying == TRUE )
	{
		if ( ( lastDir == right ) && ( thisPoint.h < 9 ) )
		{
			thisPoint.h++;
			
			if ( you[thisPoint.h][thisPoint.v].hitOrMiss == noTry )
			{
				lastTry = PlaceTheShot ( thisPoint );
				
				if ( lastTry == miss )
				{
					thisPoint = firstHitPoint;
					lastDir = left;
				}
				
				if ( lastTry ==  hit )
				{
					lastPoint = thisPoint;
					hitsYet = TRUE;
					isHorizontal = TRUE;
				}
				return;
			}
			else
			{
				thisPoint = firstHitPoint;
				lastDir = left;
			}
		}
		else if ( ( lastDir == right ) && ( ( isHorizontal ) || ( thisPoint.h == 9 ) ) )
		{
			thisPoint = firstHitPoint;
			lastDir = left;
		}
		
		if ( ( lastDir == left ) && ( thisPoint.h > 0 ) )
		{
			thisPoint.h--;
			
			if ( you[thisPoint.h][thisPoint.v].hitOrMiss == noTry )
			{
				lastTry = PlaceTheShot ( thisPoint );
				
				switch ( lastTry )
				{
					case hit:
						lastPoint = thisPoint;
						isHorizontal = TRUE;
						hitsYet = TRUE;
						break;

					case miss:
						if ( hitsYet )
						{
							keepTrying = FALSE;
							lastDir = right;
							hitsYet = FALSE;
						}
						else
						{
							thisPoint = firstHitPoint;
							lastDir = up;
						}
						break;
				}
					
				return;
			}
			else
			{
				if  ( hitsYet )
				{
					lastDir = right;
					keepTrying = FALSE;
					hitsYet = FALSE;
				}
				else
				{
					thisPoint = firstHitPoint;
					lastDir = up;
				}
			}
		}
		else if ( ( isHorizontal ) || ( lastDir == left ) )
		{
			if  ( hitsYet )
			{
				lastDir = right;
				keepTrying = FALSE;
				hitsYet = FALSE;
			}
			else
			{
				lastDir = up;
			}
		}

		if ( ( lastDir == up ) && ( thisPoint.v > 0 ) )
		{
			thisPoint.v--;
			
			if ( you[thisPoint.h][thisPoint.v].hitOrMiss == noTry )
			{
				lastTry = PlaceTheShot ( thisPoint );
				
				if ( lastTry == miss )
				{
					thisPoint = firstHitPoint;
					lastDir = down;
				}
				
				if ( lastTry ==  hit )
				{
					lastPoint = thisPoint;
					hitsYet = TRUE;
				}
				return;
			}
			else
			{
				thisPoint = firstHitPoint;
				lastDir = down;
			}
		}
		else if ( ( !isHorizontal ) && (lastDir == up ) )
		{
			lastDir = down;
			thisPoint = firstHitPoint;
		}
		
		if ( ( lastDir == down ) && ( thisPoint.v < 9 ) )
		{
			thisPoint.v++;
			
			if ( you[thisPoint.h][thisPoint.v].hitOrMiss == noTry )
			{
				lastTry = PlaceTheShot ( thisPoint );
				
				if ( lastTry == miss )
				{
					if ( hitsYet )
					{
						keepTrying = FALSE;
						lastDir = right;
						hitsYet = FALSE;
					}
					else
					{
						lastPoint = thisPoint;
						lastDir = up;
					}
				}
				
				if ( lastTry == hit )
				{
					hitsYet = TRUE;
				}
					
				return;
			}
			else
			{
				lastDir = right;
				hitsYet = FALSE;
			}
		}
		else if ( !isHorizontal )
		{
			lastDir = right;
			hitsYet = FALSE;
			keepTrying  = FALSE;
		}
	}

	do
	{
		do
		{
			thisPoint.h = Randomize ();
		} while ( ( thisPoint.h < 0 ) || ( thisPoint.h > 9 ) );
		do
		{
			thisPoint.v = Randomize ();
		} while ( ( thisPoint.v < 0 ) || ( thisPoint.v > 9 ) );
	}  while ( IsBadRandom ( thisPoint, shotCount < kMinSpacedShots ) );
	
	lastTry = PlaceTheShot ( thisPoint );
	
	if ( lastTry == hit )
	{
		firstHitPoint = thisPoint;
		keepTrying = TRUE;
	}
	else
		keepTrying = FALSE;
	
	lastDir = right;
	lastPoint = thisPoint;
	isHorizontal = FALSE;
	
	return;
}

/***************/
/* IsBadRandom */
/***************/

Boolean	IsBadRandom ( Point thePoint, Boolean canCheck )
{
	Point	tempPoint = thePoint;
	
	if ( ( you[thePoint.h][thePoint.v].hitOrMiss == hit ) ||
				( you[thePoint.h][thePoint.v].hitOrMiss == miss ) )
		return ( TRUE );
	
	if ( canCheck ) /* After a certain number of tries which is #define, 
						this will be false.  It prevents having random
						guesses next to each other early in the game. */
	{
		if ( thePoint.h > 0 )
		{
			if ( ( you[thePoint.h - 1][thePoint.v].hitOrMiss == hit ) ||
						( you[thePoint.h - 1][thePoint.v].hitOrMiss == miss ) )
				return ( TRUE );
		}
		if ( thePoint.h < 9 )
		{
			if ( ( you[thePoint.h + 1][thePoint.v].hitOrMiss == hit ) ||
						( you[thePoint.h + 1][thePoint.v].hitOrMiss == miss ) )
				return ( TRUE );
		}
		if ( thePoint.v > 0 )
		{
			if ( ( you[thePoint.h][thePoint.v-1].hitOrMiss == hit ) ||
						( you[thePoint.h][thePoint.v-1].hitOrMiss == miss ) )
				return ( TRUE );
		}
		if ( thePoint.v < 9 )
		{
			if ( ( you[thePoint.h][thePoint.v+1].hitOrMiss == hit ) ||
						( you[thePoint.h][thePoint.v+1].hitOrMiss == miss ) )
				return ( TRUE );
		}
	}
	
	/* If the point is surrounded by white on all sides, why guess there? */
	if ( ( you[thePoint.h - 1][thePoint.v].hitOrMiss == miss ) && 
			( you[thePoint.h + 1][thePoint.v].hitOrMiss == miss ) &&
			( you[thePoint.h][thePoint.v - 1].hitOrMiss == miss ) &&
			( you[thePoint.h][thePoint.v + 1].hitOrMiss == miss ) )
		return ( TRUE );
	
	return ( FALSE );
}

/****************/
/* PlaceTheShot */
/****************/

cond	PlaceTheShot ( Point thePoint )
{
	Rect  theRect;
	Handle		mySndHandle;
	
	SetPort ( youWindow );
	
	if ( gHasSound )
	{
		mySndHandle = GetResource ( 'snd ', kBombDropSnd );
		if ( mySndHandle != NULL )
			SndPlay ( nil, mySndHandle, TRUE );
	}
	
	if ( you[thePoint.h][thePoint.v].shipResID != 0 )
	{
		you[thePoint.h][thePoint.v].hitOrMiss = hit;
		
		if ( gHasSound )
		{
			mySndHandle = GetResource ( 'snd ', kExplosionSnd );
			if ( mySndHandle != NULL )
				SndPlay ( nil, mySndHandle, TRUE );
		}
		
		youCount++;
	}
	else
	{
		you[thePoint.h][thePoint.v].hitOrMiss = miss;
		
		if ( gHasSound )
		{
			mySndHandle = GetResource ( 'snd ', kSplashSnd );
			if ( mySndHandle != NULL )
				SndPlay ( nil, mySndHandle, TRUE );
		}
	}
	
	PtToDotRect ( thePoint, &theRect );
	DrawOneDot ( you[thePoint.h][thePoint.v].hitOrMiss, &theRect );
	
	shotCount++;
	
	return ( you[thePoint.h][thePoint.v].hitOrMiss );
}

/************/
/* IsColour */
/************/

Boolean	IsColour( void )
{
	SysEnvRec		mySE;
	
	/* Check to see if colour quickdraw is installed */
	SysEnvirons( 2, &mySE );
	return( mySE.hasColorQD );
}


