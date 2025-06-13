/*******************************************************/ 
/*                  Page Flipper 1.0.1                 */
/*       									           */
/* 			   This Code May Be Used Freely.           */
/* 		( If you use it, I wouldn't mind credit )	   */
/*		( and a copy of the program it's used in.)	   */
/*													   */
/*			  The Page Flipper Archive    			   */
/*	 ( source code, resources demo app, & Read ME )	   */
/*		May Be Distributed Freely, So Long As It	   */
/*					Is Not Modified					   */
/*													   */
/*       		Created By Eric Long 				   */
/*			    ( Using THINK C 7.x )				   */
/*    AOL: EricMLong  -  CompuServe 72764,1072		   */
/*******************************************************/ 


/* 	konstants 	*/

#define	 kBaseResID		128
#define	 kDefaultOutline	3
#define	 kSleep			20L

/* Window Stuff */
#define    kMoveToFront		(WindowPtr)-1L

/* Menus */
#define		mApple			kBaseResID
#define			iAbout			1

#define		mFile			kBaseResID+1
#define	 		iSwitchSides	1
#define	 		iQuit			3

#define	 	iPageControl	2
#define	 	iClipRegion		3
#define	 	iPageNum		5
#define	 	kNumPageZooms	6
#define	 	kZoomDelay		2L	/* Anything less doesn't draw right, more takes too long */
#define	 	kNumPages		10


/************************* Globals ******************************/

Boolean			gDone, gRightSide = true;
DialogPtr		gDLOGFromRight, gDLOGFromLeft;
RgnHandle		gPageMinus[2], gPagePlus[2];
long			gPageNum = 1L;


/************************ Prototypes **********************************/

void			main( void );
void			ToolBoxInit ( void );
void			MenuBarInit( void );
void			EventLoop ( void );
void			DoEvent( EventRecord *eventPtr );							
void			DoUpdate( WindowPtr window );
void			HandleNull( EventRecord *event );
void			HandleMouseDown (EventRecord *eventPtr );
void			HandleKeyDown(EventRecord *eventPtr);
void			HandleMenuChoice( long menuChoice );
void			HandleAppleChoice( short item );
void			HandleFileChoice( short item );
void			HandlePageDialog( EventRecord *event );
void			Flip_Page( Rect pageRect, Rect ctrlRect,
							Boolean pagePlus, Boolean fromRight);
void			Init_PageDLOGs( void );
void			SetUpPageControls( void );
void			SwitchSides( void );		

/******************** main ***********************************/

void		main ( void )
{
	ToolBoxInit ();
	MaxApplZone();
	MenuBarInit();
	
	Init_PageDLOGs();
	
	FlushEvents( everyEvent, 0 );		
	EventLoop();
}
		

/************************* ToolBoxInit ******************************/

void		ToolBoxInit ( void )
{
	InitGraf ( &qd.thePort );
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs(0L);
	InitCursor ();
}


/*********************** MenuBarInit ***************************/

void	MenuBarInit( void )
{
	Handle			menuBar;
	MenuHandle		menu;
	
	menuBar = GetNewMBar( kBaseResID );
	SetMenuBar( menuBar );
	
	menu = GetMHandle( mApple );
	AddResMenu( menu, 'DRVR' );
	
	DrawMenuBar();	
}


/************************ EventLoop ************************/

void	EventLoop ( void )
{
	EventRecord		event;
	
	while ( gDone == false )
	{
		if ( WaitNextEvent( everyEvent, &event, kSleep, nil) )
				DoEvent ( &event );
		else
			HandleNull( &event );
	}
}


/************************** DoEvent ***********************/

void	DoEvent( EventRecord *eventPtr )
{	
	switch ( eventPtr->what )
	{
		case mouseDown:
			HandleMouseDown( eventPtr );
			break;
		case keyDown:
		case autoKey:
			HandleKeyDown( eventPtr );
			break;
		case updateEvt:
			DoUpdate((WindowPtr)eventPtr->message);
			break;
	}			 
}


/********************* DoUpdate ******************************/

void		DoUpdate( WindowPtr	window )
{
	BeginUpdate( window );
	
	UpdtDialog((DialogPtr)window, window->visRgn);
		
	EndUpdate( window );
}


/******************* HandleNull **************************/

void	HandleNull( EventRecord *eventPtr )
{
	;
}		


/******************* HandleMouseDown **************************/

void	HandleMouseDown ( EventRecord *eventPtr )
{
	WindowPtr		window;
	short			thePart;
	long			menuChoice;
	
	thePart = FindWindow ( eventPtr->where, &window );
	
	switch ( thePart )
	{
		case inGoAway:
			gDone = true;
			break;
		case inMenuBar:
			menuChoice = MenuSelect( eventPtr->where );
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow :
			SystemClick ( eventPtr, window );
			break;
		case inDrag :
			DragWindow ( window, eventPtr->where,
						&qd.screenBits.bounds );
			break;
		case inContent:
			HandlePageDialog( eventPtr );
			break;
	}
}


/******************** HandleKeyDown ****************************/

void	HandleKeyDown(EventRecord *eventPtr)
{
	char			theChar;
		
	theChar = eventPtr->message & charCodeMask;
	if ( (eventPtr->modifiers & cmdKey) != 0 )
		HandleMenuChoice( MenuKey( theChar ) );
}


/********************** HandleMenuChoice **************************/

void	HandleMenuChoice( long menuChoice )
{
	short		menu;
	short		item;
	
	if ( menuChoice != 0 )
	{
		menu = HiWord( menuChoice );
		item = LoWord( menuChoice );
		
		switch( menu )
		{
			case mApple:
				HandleAppleChoice( item );
				break;
			case mFile:
				HandleFileChoice( item );
				break;
		}
		HiliteMenu( 0 );
	}
}


/********************* HandleAppleChoice ***************************/

void	HandleAppleChoice( short item )
{
	MenuHandle	appleMenu;
	Str255		accName;
	short		accNumber;
	
	switch ( item )
	{
		case iAbout:
			NoteAlert(kBaseResID, nil );
			break;
		default:
			appleMenu = GetMHandle( mApple );
			GetItem( appleMenu, item, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}


/********************* HandleFileChoice ****************************/

void	HandleFileChoice( short item )
{		
	switch (item)
	{
		case iSwitchSides:
			SwitchSides();
			break;
		case iQuit:
			gDone = true;
			break;
	}
}


/********************** HandlePageDialog *******************************/ 

void	HandlePageDialog( EventRecord *event )
{
	Boolean			isPagePlus = false, isPageMinus = false;
	short			itemType;
	Rect			itemRect, pageRect, ctrlRect;
	Handle			itemHandle;
	Handle			textIHandle;
	Str255			numStr;
	DialogPtr		dialog;
	
	if (gRightSide)
		dialog = gDLOGFromRight;
	else
		dialog = gDLOGFromLeft;
		
	SetPort(dialog);
	
	GlobalToLocal(&(event->where));

	isPageMinus = PtInRgn(event->where, gPageMinus[gRightSide]);
	if (! isPageMinus)
		isPagePlus = PtInRgn(event->where, gPagePlus[gRightSide]);
	
	if (isPagePlus || isPageMinus)
	{
		GetDItem(dialog, iPageNum, &itemType, &textIHandle, &itemRect);
		GetDItem(dialog, iClipRegion, &itemType, &itemHandle, &pageRect);
		GetDItem(dialog, iPageControl, &itemType, &itemHandle, &ctrlRect);
		
		Flip_Page( pageRect, ctrlRect, isPagePlus, gRightSide );

		if (isPagePlus){
			gPageNum++;
			if (gPageNum > kNumPages)
				gPageNum = 1;
		}
		else{
			gPageNum--;
			if (! gPageNum)
				gPageNum = 10;
		}
		NumToString(gPageNum, numStr);
		SetIText(textIHandle, numStr);
	}
}


/************************** Flip_Page *******************************/

void	Flip_Page( Rect pageRect, Rect ctrlRect,
						Boolean pagePlus, Boolean fromRight)
{
	Rect			flipRect;
	short			zoomLoop, vOffset, hOffset;
	short			vDirection, hDirection;
	short			height, width;
	RgnHandle		saveRegion, myClipRgn;
	PenState		savePenState;
	long			time;
	
	GetPenState(&savePenState);
	saveRegion = NewRgn();
	GetClip(saveRegion);
		
	height = ctrlRect.bottom - ctrlRect.top;	// +1
	width = ctrlRect.right - ctrlRect.left;		// +2

	myClipRgn = NewRgn();
	OpenRgn();
		MoveTo(pageRect.left, pageRect.top);
		LineTo(pageRect.right, pageRect.top);
		if (fromRight)
		{
			LineTo(pageRect.right, pageRect.bottom - height);
			LineTo(pageRect.right - width, pageRect.bottom - height);
			LineTo(pageRect.right - width, pageRect.bottom);
			LineTo(pageRect.left, pageRect.bottom);
		}
		else
		{
			LineTo(pageRect.right, pageRect.bottom);
			LineTo(pageRect.left + width, pageRect.bottom);
			LineTo(pageRect.left + width, pageRect.bottom - height);
			LineTo(pageRect.left, pageRect.bottom - height);
		}
		LineTo(pageRect.left, pageRect.top);
	CloseRgn(myClipRgn);
	
	SetClip(myClipRgn);
	EraseRgn(myClipRgn);
	
	vOffset = (short)((pageRect.right - width) / (kNumPageZooms + 1));
	hOffset = (short)((pageRect.bottom - height) / (kNumPageZooms + 1));
	
	flipRect = pageRect;
	
	if (fromRight){
		if (pagePlus){
			OffsetRect(&flipRect, ctrlRect.left, ctrlRect.top);
			hDirection = vDirection = -1;
		}
		else	/* page minus */
			hDirection = vDirection = 1;
	}
	else{	/* From Left */
		if (pagePlus){
			OffsetRect(&flipRect, -(flipRect.right - ctrlRect.right), ctrlRect.top);
			hDirection = -1;
			vDirection = 1;
		}
		else{	/* page minus */
			hDirection = 1;
			vDirection = -1;
		}
	}

	for (zoomLoop = 0; zoomLoop < kNumPageZooms; zoomLoop++)
	{
			/* Draw Zoom Rect */
		OffsetRect(&flipRect, (vOffset * vDirection), (hOffset * hDirection));
		FrameRect(&flipRect);
		if (gRightSide){	/* Connect zoom lines for a triangle */
			MoveTo(flipRect.left,pageRect.bottom);
			LineTo(pageRect.right,flipRect.top);
		}
		else{
			MoveTo(flipRect.right, pageRect.bottom);
			LineTo(pageRect.left, flipRect.top);
		}
		
		Delay(kZoomDelay, &time);
		
		PenPat(&qd.white);
		FrameRect(&flipRect);	/* Erase Zoom Rect and triangle line */
		if (gRightSide){
			MoveTo(flipRect.left,pageRect.bottom);
			LineTo(pageRect.right,flipRect.top);
		}
		else{
			MoveTo(flipRect.right, pageRect.bottom);
			LineTo(pageRect.left, flipRect.top);
		}
		PenPat(&qd.black);
	}
	
	SetPenState(&savePenState);
	SetClip(saveRegion);
	InvalRgn(myClipRgn);
	
	DisposeRgn(saveRegion);
	DisposeRgn(myClipRgn);	
}


/********************* Init_PageDLOGs ***********************/ 

void			Init_PageDLOGs( void )
{
	short	iType;
	Rect	iRect;
	Handle	iHandle;
	Str255	numStr;
	
	
	gDLOGFromLeft = GetNewDialog( kBaseResID+1, nil, kMoveToFront );
	
	gDLOGFromRight = GetNewDialog( kBaseResID, nil, kMoveToFront );
	SetPort(gDLOGFromRight);
	GetDItem(gDLOGFromRight, iPageNum, &iType, &iHandle, &iRect);
	NumToString(gPageNum,numStr);
	SetIText(iHandle, numStr);

	ShowWindow(gDLOGFromRight);
	
	SetUpPageControls();
				
}


/************************** SetUpPageControls ***************************/ 

void			SetUpPageControls( void )
{
	short	iType;
	Rect	iRect;
	Handle	iHandle;
	
		/* Controls For Right Side */
	GetDItem(gDLOGFromRight, iPageControl, &iType, &iHandle, &iRect);	
	gPageMinus[1] = NewRgn();
	OpenRgn();
	MoveTo(iRect.left,iRect.top);		/* Start & End Point */
	LineTo(iRect.right,iRect.top);		/* Across Top */
	LineTo(iRect.left,iRect.bottom);	/* Down To Bottom, Left Corner */	
	LineTo(iRect.left,iRect.top);		/* Back To Top, Left */
	CloseRgn(gPageMinus[1]);

	gPagePlus[1] = NewRgn();
	OpenRgn();
	MoveTo(iRect.right,iRect.bottom);	/* Start & End Point */
	LineTo(iRect.right,iRect.top);		/* Up to Top */
	LineTo(iRect.right-1,iRect.top);	/* Over one to for alignment purposes */
	LineTo(iRect.left,iRect.bottom);	/* Down to Bottom, left */
	LineTo(iRect.right,iRect.bottom);	/* Back to Bottom, Right */
	CloseRgn(gPagePlus[1]);

		/* Controls for Left Side */
	GetDItem(gDLOGFromLeft, iPageControl, &iType, &iHandle, &iRect);	
	gPageMinus[0] = NewRgn();
	OpenRgn();
	MoveTo(iRect.left,iRect.top);		/* Start & End Point */
	LineTo(iRect.right,iRect.top);		/* Across Top */
	LineTo(iRect.right,iRect.bottom);	/* Down To Bottom, right Corner */	
	LineTo(iRect.left,iRect.top);		/* Back To Top, Left */
	CloseRgn(gPageMinus[0]);

	gPagePlus[0] = NewRgn();
	OpenRgn();
	MoveTo(iRect.left,iRect.bottom);	/* Start & End Point */
	LineTo(iRect.left,iRect.top);		/* Up to Top */
	LineTo(iRect.left+1,iRect.top);		/* Over one to for alignment purposes */
	LineTo(iRect.right,iRect.bottom);	/* Down to Bottom, right */
	LineTo(iRect.left,iRect.bottom);	/* Back to Bottom, Left */
	CloseRgn(gPagePlus[0]);
}
	

/*************************** SwitchSides *****************************/ 

void			SwitchSides( void )
{
	Str255	numStr;
	short	iType;
	Rect	iRect;
	Handle	iHandle;
	
	gPageNum = 1L;
	
	if (gRightSide)
	{
		SetPort(gDLOGFromLeft);
		GetDItem( gDLOGFromLeft, iPageNum, &iType, &iHandle, &iRect);
		NumToString(gPageNum, numStr);
		SetIText(iHandle, numStr);
		HideWindow(gDLOGFromRight);
		ShowWindow(gDLOGFromLeft);
		gRightSide = false;
	}
	else
	{
		SetPort(gDLOGFromRight);
		GetDItem( gDLOGFromRight, iPageNum, &iType, &iHandle, &iRect);
		NumToString(gPageNum, numStr);
		SetIText(iHandle, numStr);
		HideWindow(gDLOGFromLeft);
		ShowWindow(gDLOGFromRight);
		gRightSide = true;
	}
}
		
	