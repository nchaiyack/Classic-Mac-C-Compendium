/********************************************************/
/*														*/
/*  Flying Line Code from Chapter Three of				*/
/*														*/
/*	*** The Macintosh Programming Primer, 2nd Ed. ***	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/********************************************************/


#define	kNumLines			50	/*  Try 100 or 150  */
#define	kMoveToFront		(WindowPtr)-1L
#define kRandomUpperLimit	32768

#define	kEmptyString		"\p"
#define	kEmptyTitle			kEmptyString
#define	kVisible			true
#define	kNoGoAway			false
#define	kNilRefCon			(long)nil


/*************/
/*  Globals  */
/*************/

Rect		gLines[ kNumLines ];
short		gDeltaTop=3, gDeltaBottom=3;  /* These four are the  */
short		gDeltaLeft=2, gDeltaRight=6;  /* key to flying line!  */
short		gOldMBarHeight;


/***************/
/*  Functions  */
/***************/

void	ToolBoxInit( void );
void	WindowInit( void );
void	LinesInit( void );
void	MainLoop( void );
void	RandomRect( Rect *myRectPtr );
short	Randomize( short range );
void	RecalcLine( short i );
void	DrawLine( short i );


/****************** main ***************************/

void	main( void )
{
	ToolBoxInit();
	WindowInit();
	LinesInit();
	MainLoop();
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


/****************** WindowInit ***********************/

void	WindowInit( void )
{
	Rect		mBarRect;
	RgnHandle	mBarRgn;
	WindowPtr	window;

	gOldMBarHeight = LMGetMBarHeight();
	LMSetMBarHeight( 0 );

	window = NewWindow( nil, &(qd.screenBits.bounds),
		kEmptyTitle, kVisible, plainDBox, kMoveToFront,
		kNoGoAway, kNilRefCon );

	SetRect( &mBarRect, qd.screenBits.bounds.left,
				qd.screenBits.bounds.top,
				qd.screenBits.bounds.right,
				qd.screenBits.bounds.top+gOldMBarHeight );

	mBarRgn = NewRgn();
	RectRgn( mBarRgn, &mBarRect );
	UnionRgn( window->visRgn, mBarRgn, window->visRgn );
	DisposeRgn( mBarRgn );
	SetPort( window );
	FillRect( &(window->portRect), &qd.black );
								/* Change black to ltGray, */
	PenMode( patXor );	/*   <--- and comment out this line  */
}


/****************** LinesInit **********************/

void	LinesInit( void )
{
	short i;
	
	HideCursor();
	GetDateTime( (unsigned long *)(&qd.randSeed) );
	RandomRect( &(gLines[ 0 ]) );
	DrawLine( 0 );
	
	for ( i=1; i<kNumLines; i++ )
	{
		gLines[ i ] = gLines[ i-1 ];
		RecalcLine( i );
		DrawLine( i );
	}
}


/****************** MainLoop ***********************/

void	MainLoop( void )
{
	short i;
	
	while ( ! Button() )
	{
		DrawLine( kNumLines - 1 );
		for ( i=kNumLines-1; i>0; i-- )
			gLines[ i ] = gLines[ i-1 ];
		RecalcLine( 0 );
		DrawLine( 0 );
	}
	LMSetMBarHeight( gOldMBarHeight );
}


/****************** RandomRect *********************/

void	RandomRect( Rect *myRectPtr )
{
	WindowPtr	myWindow;

	myWindow = FrontWindow();
	
	myRectPtr->left = Randomize( myWindow->portRect.right
		- myWindow->portRect.left );
	myRectPtr->right = Randomize( myWindow->portRect.right
		- myWindow->portRect.left );
	myRectPtr->top = Randomize( myWindow->portRect.bottom
		- myWindow->portRect.top );
	myRectPtr->bottom = Randomize( myWindow->portRect.bottom
		- myWindow->portRect.top );
}


/****************** Randomize **********************/

short	Randomize( short range )
{
	long	rawResult;
	
	rawResult = Random();
	if ( rawResult < 0 ) rawResult *= -1;
	return( (rawResult * range) / 32768 );
}


/****************** RecalcLine *********************/

void	RecalcLine( short i )
{
	WindowPtr	myWindow;

	myWindow = FrontWindow();
	
	gLines[ i ].top += gDeltaTop;
	if ( ( gLines[ i ].top < myWindow->portRect.top ) ||
		 ( gLines[ i ].top > myWindow->portRect.bottom ) )
	{
		gDeltaTop *= -1;	
		gLines[ i ].top += 2*gDeltaTop;
	}
	
	gLines[ i ].bottom += gDeltaBottom;
	if ( ( gLines[ i ].bottom < myWindow->portRect.top ) ||
		 ( gLines[ i ].bottom > myWindow->portRect.bottom ) )
	{
		gDeltaBottom *= -1;	
		gLines[ i ].bottom += 2*gDeltaBottom;
	}
	
	gLines[ i ].left += gDeltaLeft;
	if ( ( gLines[ i ].left < myWindow->portRect.left ) ||
		 ( gLines[ i ].left > myWindow->portRect.right ) )
	{
		gDeltaLeft *= -1;	
		gLines[ i ].left += 2*gDeltaLeft;
	}
	
	gLines[ i ].right += gDeltaRight;
	if ( ( gLines[ i ].right < myWindow->portRect.left ) ||
		 ( gLines[ i ].right > myWindow->portRect.right ) )
	{
		gDeltaRight *= -1;	
		gLines[ i ].right += 2*gDeltaRight;
	}
}


/****************** DrawLine ***********************/

void	DrawLine( short i )
{
	MoveTo( gLines[ i ].left, gLines[ i ].top );
	LineTo( gLines[ i ].right, gLines[ i ].bottom );
}