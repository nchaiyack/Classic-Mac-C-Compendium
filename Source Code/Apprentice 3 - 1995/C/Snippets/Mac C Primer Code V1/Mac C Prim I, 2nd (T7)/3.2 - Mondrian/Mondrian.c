/********************************************************/
/*														*/
/*  Mondrian Code from Chapter Three of					*/
/*														*/
/*	*** The Macintosh Programming Primer, 2nd Ed. ***	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/********************************************************/

#define	kBaseResID			128
#define	kMoveToFront		(WindowPtr)-1
#define kRandomUpperLimit	32768


/*************/
/*  Globals  */
/*************/

long	gFillColor = blackColor;


/***************/
/*  Functions  */
/***************/

void	ToolBoxInit( void );
void	WindowInit( void );
void	MainLoop( void );
void	DrawRandomRect( void );
void	RandomRect( Rect *rectPtr );
short	Randomize( short range );


/****************** main ***************************/

void	main( void )
{
	ToolBoxInit();
	WindowInit();
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
	WindowPtr	window;

	window = GetNewWindow( kBaseResID , nil,
									kMoveToFront );
	
	if ( window == nil
	 )
	{
		SysBeep( 10 );	/*  Couldn't load the WIND resource!!!  */
		ExitToShell();
	}
	
	ShowWindow( window );
	SetPort( window );
}


/****************** MainLoop ***********************/

void	MainLoop( void )
{
	GetDateTime( (unsigned long *)(&qd.randSeed) );
	
	while ( ! Button() )
	{
		DrawRandomRect();
		
		if ( gFillColor == blackColor )
			gFillColor = whiteColor;
		else
			gFillColor = blackColor;
	}
}


/****************** DrawRandomRect *****************/

void	DrawRandomRect( void )
{
	Rect	randomRect;
	
	RandomRect( &randomRect );
	ForeColor( gFillColor );
	PaintOval( &randomRect );
}


/****************** RandomRect *********************/

void	RandomRect( Rect *rectPtr )
{
	WindowPtr	window;

	window = FrontWindow();
	
	rectPtr->left = Randomize( window->portRect.right
		- window->portRect.left );
	rectPtr->right = Randomize( window->portRect.right
		- window->portRect.left );
	rectPtr->top = Randomize( window->portRect.bottom
		- window->portRect.top );
	rectPtr->bottom = Randomize( window->portRect.bottom
		- window->portRect.top );
}


/****************** Randomize **********************/

short	Randomize( short range )
{
	long	randomNumber;
	
	randomNumber = Random();
	
	if ( randomNumber < 0 )
		randomNumber *= -1;
	
	return( (randomNumber * range) / kRandomUpperLimit );
}