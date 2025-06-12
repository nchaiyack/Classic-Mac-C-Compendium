/*	Code release for Safari II 1.1.
	Changes since 1.0 release:
		It's now FREE!  (please send comments & a postcard)
		The sicn's were eliminated from the apple menu so
		  it should now work on older systems
		The dialog box in which I begged for a job is gone
		   (I've found work, thanks!)
		The option to 'score when hit' in rectangle
		   surrounding animal now works.
		   
	There still seems to be problems with the sound playing
	on older 68000 Macs.  Any advice on this problem, or
	more descript diagnoses would be appreciated.

	SAFARI II © 1991 by John Gallaugher
		A program written in Think C.  Portions copywright
		Symantec and Apple Computer.

	In 1985 I wrote a program called Safari to try to teach myself
	the Macintosh toolbox.  The original Safari was simple and cute, 
	but it was also a program which doesn't work on today's machines.
	Safari II is an update to the orginal Safari.  It should work
	on all Macs with at least 1 meg of memory running system 6.0.x
	or greater.
	
	Although I have done quite a bit of Macintosh database development,
	I have never programmed professioanlly in the toolbox or in C.
	As such, please excuse the clumsiness of some of this code.  I
	am distributing the source code and game for free in hopes
	that it will help others learn some of the basics of Macintosh
	game development.  My experience has found that there's not
	much free source code out there to help the would-be game
	developmer.  I hope my modest submition helps some folks get
	started.
	
	Although this product is offerd for free, I DO ask those pros
	and semi-pros out there to offer suggestions on how I can
	improve this code.  Since I'm pretty green in toolbox coding
	I would greatly appreciate advice.  Please don't hold back, 
	criticize me to the hilt!  I need and welcome the help.
	
	Also, please don't distribute any modified versions of
	Safari.  I welcome you to learn from the code and encourage
	you to produce original work, but I would like all updates
	of the game to pass through me.
	
	Lastly, please send comments!  I love to hear from folks who
	have received my game.  My E-Mail addresses are:
	CIS - 70137,763
	AOL - GALLAUGHER
	
	If you'd care to send any U.S. Mail (I love postcards), forward
	them to:
		Safari
		c/o John Gallaugher
		Three Allegheny Center #118
		Pittsburgh, PA 15212
		
	A special thanks to the helpful folks at the Boston Computer
	Society's MacTech group, and to the outstanding faculty of
	the Boston College Computer Science Program for their patient
	help in answering my silly questions.
	
	This program seems to work fine on my 4 meg LC running under System 7.0
	if you run into any problems, PLEASE let me know.
	
	RESOURCE NOTE:  All sounds were created using the microphone on a Macintosh LC.
		All pictures were created using MacPaint.  The 'RGN ' resources which
		are used to detect the boundries of the animal bodies were created using
		a utilty program called 'Regions' created by the late Duane Blehm (it should
		be available via local user groups and on-line services).  All other
		resources were created directly in ResEdit.
 */

#include <Sound.h> /* adds routines need to play Asynch Sound */

/* sorry about any inconsistencies in declaring resource numbers.  I started
	and stopped the project several times and added things on the fly without
	thinking of a logical numbering system.  Feel free to suggest a convention */

#define	NIL_POINTER			0L
#define M_T_F		((void *)-1L) /* for "Move To Front".  Used in setting up windows */
#define	BASE_RES_ID			400
#define MOVE_TO_FRONT		-1L
#define	REMOVE_ALL_EVENTS	0

#define	BORDER				6  /* number of pixels between window and screen width */
#define	TITLE_BAR_HEIGHT	19 /* I guessed, is there a TB call to get this? */
#define	TICK_DELAY			1  /* this cycles the animation through 1 tick, or 1/60th of a second */

/* Menu IDs */
#define	APPLE_MENU_ID	1
#define	FILE_MENU_ID	401
#define	EDIT_MENU_ID	402
#define	GAME_MENU_ID	403

/* Menu Items */
#define	ABOUT_ITEM	1
#define	NEW_GAME_ITEM	1
#define	PAUSE_RESUME_ITEM	2
#define	STOP_ITEM	3
#define	QUIT_ITEM	5
#define	UNDO_ITEM	1
#define	CUT_ITEM	3
#define	COPY_ITEM	4
#define	PASTE_ITEM	5
#define	CLEAR_ITEM	6
#define	GAME_OPTIONS_ITEM	1
#define	ANIMAL_POINT_VALUES_ITEM	2
#define	HALL_OF_FAME_ITEM	3

/* Alerts */
#define	ERROR_ALERT_ID	401
#define	ERROR_EDIT_TEXT	402
#define	EXPIRED_ALERT	403

/* Dialogs */
#define	GAME_OPTIONS_DIALOG			200
#define	SAVE_HIGH_SCORES_DIALOG		300
#define	ANIMAL_POINT_VALUES_DIALOG	129
#define	INTRO_DIALOG				475
#define	ABOUT_DIALOG				476
#define	PLAYER_NAME_DIALOG			500
#define	HALL_OF_FAME_DIALOG			550

/* Dialog Items */
#define OK_BUTTON	1
#define	SAVE_AND_CLEAR_BUTTON	2
#define	CANCEL_BUTTON	3
#define	MINUTES				5
#define	SECONDS				6
#define	MAX_ESCAPED			7
#define	ESCAPED_RADIO	10
#define	TIMED_RADIO		11
#define	NINE_INCH_RADIO			22
#define	CURRENT_SCREEN_RADIO	23
#define	INCREASE_YES_RADIO	19
#define	INCREASE_NO_RADIO	20
#define	SLOW_RADIO		14
#define	MEDIUM_RADIO	15
#define	FAST_RADIO		16
#define	IN_ANIMAL_OUTLINE_RADIO	28
#define	IN_BOX_SURROUNDING_ANIMAL_RADIO	27
#define	PLAYER_NAME_ITEXT	3
#define	CLEAR_SCORES_BUTTON	2
#define	OK_APV_DBOX			10
#define	SHOW_INTRO_SCREEN	2
#define	HELP_BUTTON	3
#define	HEAD_ICON	5
#define	OUCH_ICON	7

/* icons */
#define	MY_HEAD		476
#define	OUCH_HEAD	477

/* Error Strings */
#define	NO_MBAR	BASE_RES_ID
#define	NO_MENU	BASE_RES_ID+1
#define	NO_WIND	BASE_RES_ID+3
#define NO_STR	BASE_RES_ID+4
#define CANT_LOAD_SND	BASE_RES_ID+5
#define	MINUTES_ERROR	BASE_RES_ID+6
#define	SECONDS_ERROR	BASE_RES_ID+7
#define	ESCAPED_ERROR	BASE_RES_ID+8
#define	NO_RGN	BASE_RES_ID+9

/* 'STR ' Resource ID's */
#define	TIMED_OR_ESCAPED	200
#define ANIMAL_SPEED		201
#define	WINDOW_SIZE			202
#define	INCREASE_SPEED		203
#define	SCORE_WHEN_HIT		204
#define	MINUTES_STRING		205
#define	SECONDS_STRING		206
#define	MAX_ESCAPED_STRING	207
#define	HIGH_SCORE_STRING	128
#define	HIGH_NAMES_STRING	131

#define	ON	1
#define	OFF	0
#define	TIMED_GAME		0
#define	ESCAPED_GAME	1

/* Cursor */
#define	CROSSHAIR	400

/* Button Types */
#define	PLAY_AGAIN_BUTTON	128
#define	QUIT_BUTTON			129

/* Sounds */
#define NIL_SOUND_CHANNEL	0L
#define	TARZAN_SOUND		410
#define	INTRO_SOUND			411
#define	OUCH_SOUND			412

/* PICTs */
#define ANIMAL_PICTURE	400  /* this is the picture which contains the animals for your bitmaps */

/* ANIMALS */
#define	RHINO		1
#define	ELEPHANT	2
#define	ZEBRA		3
#define	GORILLA		4
#define	ANTELOPE	5
#define	GIRAFFE		6
#define	LION		7
#define	CHEETAH		8
#define WARTHOG		9

#define	DRAG_THRESHOLD	30

#define	WINDOW_HOME_LEFT	5
#define	WINDOW_HOME_TOP	45
#define	NEW_WINDOW_OFFSET	20

#define	MIN_SLEEP	0L
#define	NIL_MOUSE_REGION	0l  /* used in WaitNextEvent */

#define	BRING_TO_FRONT	TRUE  	/* used in MoveWindow */

#define	WNE_TRAP_NUM	0x60
#define	UNIMPL_TRAP_NUM	0x9F

#define	NIL_STRING	"\p"
#define	HOPELESSLY_FATAL_ERROR	"\pGoodness knows what happened!"

#define SYNCHRONOUS		FALSE	/* for sound */
#define	ASYNCHRONOUS	TRUE

#include "Safari II.proto.h"

/* this structure is used for each row.  There is one animal per row. */
struct	rowStruct
{
	Rect	rect;		/* the rect the animal is in (into which the bitmap is drawn */
	Boolean	running;	/* is there an animal running in the row? */
	int		animal;		/* what type of animal is it? */
	int		speed;		/* how fast does it go? */
	int		step;		/* alternates each pass so that different animal positions are
							drawn, creating the illusion of a stride */
};
struct	rowStruct	row[21]; /* Even if you have a really big Mac screen you can't have more than 21 rows of animals */

/* unusual variable names in comments */

/* increaseSpeed is true when the Game Options dialog has the speed of the game increase
	  as the score increases to the score.
	easyShoot allows the player to score by shooting in a rect surrounding the animal
	  and does not require them to shoot directly within the outline of the animal
	  (good for kids) */
Boolean	gDone, gWNEImplemented, gamePaused, gameOver, increaseSpeed, easyShoot, standardWindow, showHighScores;
EventRecord	gTheEvent;
MenuHandle	gAppleMenu, gFileMenu, gEditMenu;
Rect	gDragRect, offScreenRect, whereAnimalsRun; /* whereAnimalsRun is the rect where animation occurs */
int	gNewWindowLeft = WINDOW_HOME_LEFT, gNewWindowTop = WINDOW_HOME_TOP;
int	windowHeight, windowWidth, noOfTicks, lastTicks;

/* animalValue is the point value of the animal.  
   anRegion is a two dimention array with
	  two regions for each animal. One for each stride position.  The rgns are used to check
	  if someone has clicked directly inside the region.
   loopValue keeps track of a group of statements which mimic a for loop, but which
      increment only once every pass through the animation procedure.  This is done
      instead of a for loop so that the game can respond to an event after each animal
      is drawn.  
   gameType tells if the game ends after a period of time, or after a given number of animals escape 
   speedFactor and speedIncrease regulate the initial speed of the game and the increase
       in speed as game play continues*/
int	gameScore, animalsEscaped, animalRows, animalSpeed[11], animalValue[11];
int	anRegion[11][2], loopValue, gameType, maxTime, speedFactor,speedIncrease;
long maxEscaped, timeRemaining, lastTime, topScores[4];
Rect	animalRect[11][2];

PicHandle	animalPicture;
WindowPtr	gameWindow;
GrafPtr   offscreen;
ControlHandle	playAgainControl, quitControl;
Str255		preferenceString[8], name, highScoreName[4];

SndChannelPtr	chanPtr;

/**** main ****/
main (void)
{
	ToolBoxInit ();
	
	SetUpFirstGlobals();
	
	MenuBarInit();
	WindowInit();
	SetUpDragRect();
	
	InitOffscreen();
	LoadInResources();
	SetUpGame();
	IntroDialog();

	MainLoop();
}

/**** ToolBoxInit ****/
ToolBoxInit (void)
{
	InitGraf( &thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( NIL_POINTER );
	InitCursor();
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS);
}

/*** SetUpFirstGlobals ***/
SetUpFirstGlobals(void)
{	
	DialogPtr	theDialog;

	showHighScores = TRUE;
	/* DialogInit() requires that the Dialog be created since it calls the DITL,
		so we create the dialog, call the procedure, and dispose of the dialog
		without ever displaying the dialog */
	if (( theDialog = GetNewDialog( GAME_OPTIONS_DIALOG, NIL_POINTER, M_T_F)) 
			== NIL_POINTER)
		ErrorHandler( NO_WIND);
	lastTicks = TickCount();
	DialogInit(theDialog, GAME_OPTIONS_DIALOG);
	DisposDialog(theDialog);
}

/**** MenuBarInit ****/
MenuBarInit(void)
{
	Handle	myMenuBar;
	
	if ( ( myMenuBar = GetNewMBar( BASE_RES_ID )) == NIL_POINTER ) 
			ErrorHandler (NO_MBAR);
	SetMenuBar( myMenuBar );
	if ( ( gAppleMenu = GetMHandle ( APPLE_MENU_ID )) == NIL_POINTER )
			ErrorHandler (NO_MENU);
	AddResMenu( gAppleMenu, 'DRVR');
	if ( ( gEditMenu = GetMHandle ( EDIT_MENU_ID )) == NIL_POINTER )
		ErrorHandler( NO_MENU );
	if ( ( gFileMenu = GetMHandle ( FILE_MENU_ID )) == NIL_POINTER )
		ErrorHandler( NO_MENU );
		
	DrawMenuBar ();
}

WindowInit(void)
{
	if (( gameWindow = GetNewWindow( BASE_RES_ID, NIL_POINTER, M_T_F)) 
			== NIL_POINTER)
		ErrorHandler( NO_WIND);
		
	playAgainControl = GetNewControl(PLAY_AGAIN_BUTTON, gameWindow);
	quitControl = GetNewControl(QUIT_BUTTON, gameWindow);
}

/** CenterWindow **/
CenterWindow (WindowPtr theWindow)
{
	int	newLeft, newTop;
	Rect	theRect, gScreenRect;
	
	theRect = theWindow->portRect;
	gScreenRect = screenBits.bounds;
	
	newLeft = ((gScreenRect.right - gScreenRect.left) - (theRect.right - theRect.left))/2;
	newTop = ((gScreenRect.bottom - (gScreenRect.top + MBarHeight + TITLE_BAR_HEIGHT)) 
		- (theRect.bottom - theRect.top))/2;
	MoveWindow (theWindow, newLeft, newTop+MBarHeight+TITLE_BAR_HEIGHT, BRING_TO_FRONT);
	ShowWindow(theWindow);
}

/*** AdjustWindowSize ***/
AdjustWindowSize(void)
{
	Rect	windowRect;
	int	length, h, v;
	
	SetPort( gameWindow);
	
	if (!(standardWindow)) /* if it's not the 9" screen window, then */
	{
		SizeWindow(gameWindow,
				screenBits.bounds.right - BORDER, 
				screenBits.bounds.bottom - (MBarHeight + BORDER + TITLE_BAR_HEIGHT),
				TRUE);
	}
	else
		SizeWindow(gameWindow, 506, 298, TRUE);
	
	windowRect = gameWindow->portRect;
	
	windowWidth = windowRect.right - windowRect.left;
	windowHeight = windowRect.bottom - windowRect.top;
	
	CenterWindow(gameWindow);
	TextFont( systemFont);
	
	v = windowHeight/2 + 20;
	h = windowWidth/2 - 100;
	MoveControl(playAgainControl, h, v);
	
	h = windowWidth/2 + 20;
	MoveControl(quitControl, h, v);
}

/**** CreateOffscreenBitMap (borrowed from Apple TN#41) ****/
Boolean CreateOffscreenBitMap(GrafPtr *newOffscreen, Rect *inBounds)
{
  GrafPtr savePort;
  GrafPtr newPort;

  GetPort(&savePort);    /* need this to restore thePort after OpenPort */

  newPort = (GrafPtr) NewPtr(sizeof(GrafPort));    /* allocate the grafPort */
  if (MemError() != noErr)
    return false;                 /* failed to allocate the off-screen port */
  /*
  the call to OpenPort does the following . . . 
    allocates space for visRgn (set to screenBits.bounds) and clipRgn (set wide open)
    sets portBits to screenBits
    sets portRect to screenBits.bounds
    etc. (see IM I-163,164)
    side effect: does a SetPort(&offScreen)
  */
  OpenPort(newPort);
  /* make bitmap the size of the bounds that caller supplied */
  newPort->portRect = *inBounds;
  newPort->portBits.bounds = *inBounds;
  RectRgn(newPort->clipRgn, inBounds);    /* avoid wide-open clipRgn, to be safe  */
  RectRgn(newPort->visRgn, inBounds);     /* in case newBounds is > screen bounds */

  /* rowBytes is size of row, it must be rounded up to an even number of bytes */
  newPort->portBits.rowBytes = ((inBounds->right - inBounds->left + 15) >> 4) << 1;

  /* number of bytes in BitMap is rowBytes * number of rows */
  /* see notes at end of Technical Note about using _NewHandle rather than _NewPtr */
  newPort->portBits.baseAddr =
           NewPtr(newPort->portBits.rowBytes * (long) (inBounds->bottom - inBounds->top));
  if (MemError()!=noErr) {   /* check to see if we had enough room for the bits */
    SetPort(savePort);
    ClosePort(newPort);      /* dump the visRgn and clipRgn */
    DisposPtr((Ptr)newPort); /* dump the GrafPort */
    return false;            /* tell caller we failed */
    }
  /* since the bits are just memory, let's clear them before we start */
  EraseRect(inBounds);     /* OpenPort did a SetPort(newPort) so we are ok */
  *newOffscreen = newPort;
  SetPort(savePort);
  return true;               /* tell caller we succeeded! */
}

/**** DestroyOffscreenBitMap (TN#41) ****/
void DestroyOffscreenBitMap(GrafPtr oldOffscreen)
{
  ClosePort(oldOffscreen);                       /* dump the visRgn and clipRgn */
  DisposPtr(oldOffscreen->portBits.baseAddr);    /* dump the bits */
  DisposPtr((Ptr)oldOffscreen);                  /* dump the port */
}

/**** InitOffscreen ****/
InitOffscreen(void)
{	
	SetRect(&offScreenRect, 0,0,227, 151); /* This is how big the Rect of ANIMAL_PICTURE is */

	if (!CreateOffscreenBitMap(&offscreen, &offScreenRect)) 
	{
		SysBeep(1);
		ExitToShell();
    }
    
	animalPicture = GetPicture ( ANIMAL_PICTURE);
	SetPort (offscreen);
	DrawPicture(animalPicture, &offScreenRect);
	SetPort(gameWindow);
}

/*** LoadInResources ***/
LoadInResources(void)
{
	/* do this at the beginning of the program, so that all resources are in memory.
	   if you don't do this, the resources will be loaded in each time they're accessed and
	   you'll notice the performance degrade when you first click on an animal and you
	   access the rgns or sound for that animal */
	int			i;
	RgnHandle	theRgn;
	Handle		theSnd;
	
	for (i=400; i<=417; ++i)
	{
		if (( theRgn = (RgnHandle) GetResource( 'RGN ', i) ) 
				== NIL_POINTER)
			ErrorHandler( NO_RGN);
	}
	
	for (i=401; i<=412; ++i)
	{
		if (( theSnd = GetResource('snd ', i) ) 
				== NIL_POINTER)
			ErrorHandler( CANT_LOAD_SND);
	}
}

/**** SetUpDragRect ****/
SetUpDragRect(void)
{
	gDragRect = screenBits.bounds;
	gDragRect.left +=DRAG_THRESHOLD;
	gDragRect.right -=DRAG_THRESHOLD;
	gDragRect.bottom -=DRAG_THRESHOLD;
}

/*** SetUpGame ***/
SetUpGame(void)
{
	int	i;
	long	stringValue;
	Str255	myString;
	Rect	offRect, onRect, windowRect;
	DialogPtr	theDialog;
	
	SetPreferences(); /* gets the current game settings */
	if (showHighScores)
		GetHighScores();
	AdjustWindowSize();
	HideControl(playAgainControl);
	HideControl(quitControl);

	SetRect(&windowRect, 0, 0, windowWidth, windowHeight);
	EraseRect(&windowRect);
	speedIncrease = 0;
	
	/* Rhino */
	SetRect(&animalRect[1][1], 2, 2, 35, 35);
	SetRect(&animalRect[1][2], 40, 2, 73, 35);
		
	/* Elephant */
	SetRect(&animalRect[2][1], 2, 40, 35, 73);
	SetRect(&animalRect[2][2], 40, 40, 73, 73);
		
	/* Zebra */
	SetRect(&animalRect[3][1], 2, 78, 35, 111);
	SetRect(&animalRect[3][2], 40, 78, 73, 111);

	/* Gorrilla */
	SetRect(&animalRect[4][1], 2, 116, 35, 149);
	SetRect(&animalRect[4][2], 40, 116, 73, 149);
	
	/* Antelope */
	SetRect(&animalRect[5][1], 78, 2, 111, 35);
	SetRect(&animalRect[5][2], 116, 2, 149, 35);
	
	/* Giraffe */
	SetRect(&animalRect[6][1], 78, 40, 111, 73);
	SetRect(&animalRect[6][2], 116, 40, 149, 73);
		
	/* Lion */
	SetRect(&animalRect[7][1], 78, 78, 111, 111);
	SetRect(&animalRect[7][2], 116, 78, 149, 111);
	
	/* Cheetah */
	SetRect(&animalRect[8][1], 78, 116, 111, 149);
	SetRect(&animalRect[8][2], 116, 116, 149, 149);
	
	/* Warthog */
	SetRect(&animalRect[9][1], 154, 2, 187, 35);
	SetRect(&animalRect[9][2], 192, 2, 225, 35);
	
	/* Set Animal Values */
	
	animalValue[RHINO] = 10;
	animalSpeed[RHINO] = 2;
	
	animalValue[ELEPHANT] = 10;
	animalSpeed[ELEPHANT] = 2;
	
	animalValue[ZEBRA] = 60;
	animalSpeed[ZEBRA] = 4;
	
	animalValue[GORILLA] = 50;
	animalSpeed[GORILLA] = 3;
	
	animalValue[ANTELOPE] = 75;
	animalSpeed[ANTELOPE] = 5;
	
	animalValue[GIRAFFE] = 60;
	animalSpeed[GIRAFFE] = 4;
	
	animalValue[LION] = 75;
	animalSpeed[LION] = 5;
	
	animalValue[CHEETAH] = 80;
	animalSpeed[CHEETAH] = 6;
		
	animalValue[WARTHOG] = 60;
	animalSpeed[WARTHOG] = 4;
	
	anRegion[RHINO][1] = 400;
	anRegion[RHINO][2] = 401;
	
	anRegion[ELEPHANT][1] = 402;
	anRegion[ELEPHANT][2] = 403;
	
	anRegion[ZEBRA][1] = 404;
	anRegion[ZEBRA][2] = 405;
	
	anRegion[GORILLA][1] = 406;
	anRegion[GORILLA][2] = 407;

	anRegion[ANTELOPE][1] = 408;
	anRegion[ANTELOPE][2] = 409;
	
	anRegion[GIRAFFE][1] = 410;
	anRegion[GIRAFFE][2] = 411;
	
	anRegion[LION][1] = 412;
	anRegion[LION][2] = 413;
	
	anRegion[CHEETAH][1] = 414;
	anRegion[CHEETAH][2] = 415;
	
	anRegion[WARTHOG][1] = 416;
	anRegion[WARTHOG][2] = 417;
	
	gameScore = 0;
	
	if (gameType == ESCAPED_GAME)
		animalsEscaped = 0;
	else
	{
		StringToNum(&(preferenceString[MINUTES]),&stringValue);
		timeRemaining = stringValue * 60;
		StringToNum(&(preferenceString[SECONDS]),&stringValue);
		timeRemaining += stringValue;
		GetDateTime(&lastTime);
	}
	
	DrawScoreBar();
	
	SetRect(&whereAnimalsRun, 0, 20, windowWidth, windowHeight);
		
	animalRows = (windowHeight-20)/33;
	
	if (animalRows >20)
		animalRows =20;
			
	for (i=1; i <= animalRows; ++i)
	{
		SetRect(&row[i].rect, 0, (33*(i-1))+20, 33, (33*(i-1))+53);
		row[i].running = FALSE;
	}
	loopValue = 1;
	gamePaused = FALSE;
	gameOver = FALSE;
	
}

/*** SetPreferences ***/
SetPreferences(void)
{
	int	i;
	long	currentSetting;
	
	for( i=0; i<=5; ++i)
	{
		StringToNum(&(preferenceString[i]),&currentSetting);
		switch (currentSetting)
		{
			case TIMED_RADIO:
				gameType = TIMED_GAME;
				break;
			case ESCAPED_RADIO:
				gameType = ESCAPED_GAME;
				break;
			case NINE_INCH_RADIO:
				standardWindow = TRUE;
				break;
			case CURRENT_SCREEN_RADIO:
				standardWindow = FALSE;
				break;
			case INCREASE_YES_RADIO:
				increaseSpeed = TRUE;
				break;
			case INCREASE_NO_RADIO:
				increaseSpeed = FALSE;
				break;
			case SLOW_RADIO:
				speedFactor = 1;
				break;
			case MEDIUM_RADIO:
				speedFactor = 4;
				break;
			case FAST_RADIO:
				speedFactor = 5;
				break;
			case IN_ANIMAL_OUTLINE_RADIO:
				easyShoot = FALSE;
				break;
			case IN_BOX_SURROUNDING_ANIMAL_RADIO:
				easyShoot = TRUE;
				break;
		}
	}
	StringToNum(&(preferenceString[MAX_ESCAPED]),&maxEscaped);
}

	
/*** GetHighScores ***/
GetHighScores(void)
{
	int	i;
	StringHandle	theString;
				
	for (i=0; i<=2; ++i)
	{
		if ( ( theString = GetString( i + HIGH_SCORE_STRING ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		StringToNum(*theString,&topScores[(i+1)]);
		HUnlock((Handle)theString);
	}
}

/*** DrawScoreBar ***/
DrawScoreBar(void)
{
	Str255	scoreString;
	int		length;
	Rect	theRect;
	
	SetRect(&theRect, 0, 0, windowWidth, 20);
	EraseRect(&theRect);
	MoveTo(2,17);
	DrawString("\pScore: ");
	NumToString(gameScore, scoreString);
	DrawString(scoreString);

	if (showHighScores)
		PostHighScore();
	
	if (gameType == ESCAPED_GAME)
	{
		length = StringWidth("\pAnimals Escaped: 500") +2;
		MoveTo(windowWidth - length, 17);
		NumToString(animalsEscaped, scoreString);
		DrawString("\pAnimals Escaped: ");
		DrawString(scoreString);
	} else
	{
		length = StringWidth("\pTime Remaining: 00:00") +2;
		MoveTo(windowWidth - length, 17);
		DrawString("\pTime Remaining: ");
		PostTimeRemaining();
	}
}

/*** PostTimeRemaining ***/
PostTimeRemaining(void)
{
	long	diffTime, currentTime;
	int		mins, secs, length;
	Str255	timeString;
	Rect	timeRect;
	
	GetDateTime(&currentTime);
	diffTime = currentTime - lastTime;
	if (diffTime > 0)
	{
		length = StringWidth("\p00:00") +2;
		SetRect(&timeRect, windowWidth - length, 0, windowWidth, 17);
		EraseRect(&timeRect);
		MoveTo(windowWidth - length, 17);
		timeRemaining -= diffTime;
		if(timeRemaining > 0)
		{
			mins = timeRemaining/60;
			NumToString(mins,timeString);
			DrawString(timeString);
			DrawString("\p:");
			secs = timeRemaining - mins*60;
			if ((secs <= 9) && (secs >= 0))
				DrawString("\p0");
			NumToString(secs, timeString);
			DrawString(timeString);
		}
		else
		{
			DrawString("\p0:00");
			EndGame();
		}
	}
	GetDateTime(&lastTime);
}

/*** PostHighScore ***/
PostHighScore(void)
{
	long			i;
	int				length;
	Str255			myString;
	StringHandle	theString;
	
	length = StringWidth("\pHigh Score: ") +2;
		
	if ( ( theString = GetString( HIGH_SCORE_STRING ) ) == NIL_POINTER )
		ErrorHandler(NO_STR);
	MoveHHi((Handle)theString);
	HLock((Handle)theString);
	
	StringToNum(*theString,&i);
	NumToString( i, myString);
	
	length += StringWidth (myString);
	MoveTo((windowWidth-length)/2, 17);
	DrawString("\pHigh Score: ");
	DrawString(myString);
	
	HUnlock((Handle)theString);
}

/**** MainLoop ****/
MainLoop(void)
{
	
	gDone = FALSE;
	gWNEImplemented = ( NGetTrapAddress( WNE_TRAP_NUM, ToolTrap ) 
			!= NGetTrapAddress( UNIMPL_TRAP_NUM, ToolTrap ) );
	while (gDone == FALSE)
	{
			HandleEvent();
			
			if ( (!(gamePaused)) && (!(gameOver)) )
			{
				noOfTicks = TickCount();
				if ( (noOfTicks - lastTicks) >= TICK_DELAY)
				{
					Animate();
					lastTicks = TickCount();
				}
			}
	}
}

/**** HandleEvent ****/
HandleEvent(void)
{
	char	theChar;
	GrafPtr	oldPort;
	WindowPtr	window;

	
	if (gWNEImplemented )
		WaitNextEvent( everyEvent, &gTheEvent, MIN_SLEEP, NIL_MOUSE_REGION );
	else
	{
		SystemTask ();
		GetNextEvent( everyEvent, &gTheEvent );
	}
	
	AdjustCurosr(gTheEvent.where);
	
	switch (gTheEvent.what)
	{
		case mouseDown:
			HandleMouseDown();
			break;
		case keyDown:
		case autoKey:
			theChar = gTheEvent.message & charCodeMask;
			if (( gTheEvent.modifiers & cmdKey ) != 0)
			{
				AdjustMenus();
				HandleMenuChoice( MenuKey ( theChar ) );
			}
			break;
		case updateEvt:
			if (! IsDAWindow( (WindowPtr)gTheEvent.message ) )
			{
				GetPort( &oldPort);
				SetPort( (GrafPtr)gTheEvent.message);
				
				BeginUpdate( (GrafPtr)gTheEvent.message);
				
				GetDateTime(&lastTime);
				window = FrontWindow();
				if (window == gameWindow )
				{
					DrawScoreBar();
					if (gameOver || gamePaused )
						DrawContent();
					if (gameOver)
						WriteGameOver();
					DrawControls(gameWindow);
				}
				EndUpdate( (GrafPtr)gTheEvent.message);
				SetPort(oldPort);
			}
			break;
	}
}

/*** AdjustCurosr ***/
AdjustCurosr(Point mouse)
{	
	WindowPtr	window;
	
	window = FrontWindow();
	
	if (window != gameWindow )
	{
		SetCursor(&arrow);
		return;
	}
	
	GlobalToLocal(&mouse);
	
	if ( PtInRect(mouse, &whereAnimalsRun) )
		SetCursor( *GetCursor(CROSSHAIR) );
	else
		SetCursor(&arrow);
}

/**** HandleMouseDown ****/
HandleMouseDown(void)
{
	WindowPtr	whichWindow;
	short int	thePart;
	long int	menuChoice, windSize;
	
	thePart = FindWindow( gTheEvent.where, &whichWindow );
	switch( thePart )
	{
		case inMenuBar:
			AdjustMenus();
			menuChoice = MenuSelect( gTheEvent.where);
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow:
			SystemClick( &gTheEvent, whichWindow);
			break;
		case inDrag:
			DragWindow( whichWindow, gTheEvent.where, &gDragRect);
			break;
		case inContent:
			SelectWindow( whichWindow);
			if ( (!(gamePaused)) && (!(gameOver)) )
				HandleInContent( gTheEvent.where);
			if (gameOver)
				HandleGameOverButtons(gTheEvent.where);
			break;
	}
}

/**** AdjustMenus ****/
AdjustMenus(void)
{
	if ( IsDAWindow( FrontWindow() ) )
	{
		EnableItem( gEditMenu, UNDO_ITEM);
		EnableItem( gEditMenu, CUT_ITEM);
		EnableItem( gEditMenu, PASTE_ITEM);
		EnableItem( gEditMenu, COPY_ITEM);
		EnableItem( gEditMenu, CLEAR_ITEM);
	}
	else
	{
		DisableItem( gEditMenu, UNDO_ITEM);
		DisableItem( gEditMenu, CUT_ITEM);
		DisableItem( gEditMenu, PASTE_ITEM);
		DisableItem( gEditMenu, COPY_ITEM);
		DisableItem( gEditMenu, CLEAR_ITEM);
	}
	
	if (gameOver)
	{
		DisableItem( gFileMenu, PAUSE_RESUME_ITEM);
		DisableItem( gFileMenu, STOP_ITEM);
	}
	else
	{
		EnableItem( gFileMenu, PAUSE_RESUME_ITEM);
		EnableItem( gFileMenu, STOP_ITEM);		
	}
	if ((gamePaused))
	{
		SetItem(gFileMenu, PAUSE_RESUME_ITEM, "\pResume");
	} else
	{
		SetItem(gFileMenu, PAUSE_RESUME_ITEM, "\pPause");
	}		
}

/**** IsDAWindow ****/
IsDAWindow(WindowPtr whichWindow)
{
	if ( whichWindow == NIL_POINTER)
		return ( FALSE);
	else
		return( ( (WindowPeek)whichWindow)->windowKind < 0);
}

/**** HandleMenuChoice ****/
HandleMenuChoice(long int menuChoice)
{
	int	theMenu;
	int	theItem;
	
	if( menuChoice != 0 )
	{
		theMenu = HiWord( menuChoice );
		theItem = LoWord( menuChoice );
		switch ( theMenu)
		{
			case APPLE_MENU_ID:
				HandleAppleChoice( theItem );
				break;
			case FILE_MENU_ID:
				HandleFileChoice( theItem );
			case EDIT_MENU_ID:
				HandleEditChoice( theItem);
				break;
			case GAME_MENU_ID:
				HandleGameChoice( theItem);
				break;
		}
		HiliteMenu( 0 );
	}
}

/**** HandleAppleChoice ****/
HandleAppleChoice(int theItem)
{
	Str255	accName;
	int	accNumber;
	GrafPtr	oldPort;
	
	switch( theItem )
	{
		case ABOUT_ITEM:
			AboutDialog();
			break;
		default :
			GetItem( gAppleMenu, theItem, accName);
			GetPort(&oldPort);
			accNumber = OpenDeskAcc( accName);
			SetPort(oldPort);
			break;
	}
}

/**** HandleFileChoice ****/
HandleFileChoice(int theItem)
{
	WindowPtr	whichWindow;
	switch	( theItem)
	{
		case NEW_GAME_ITEM:
			SetUpGame();
			break;
		case PAUSE_RESUME_ITEM:
			/* Handle Pause/Resume Selection */
			if (!(gamePaused))
			{
				gamePaused= TRUE;
			} else
			{
				gamePaused= FALSE;		
				GetDateTime(&lastTime);
			}		
			break;
		case STOP_ITEM:
			EndGame();
			break;
		case QUIT_ITEM:
			gDone = TRUE;
			break;
	}
}

/**** HandleEditChoice ****/
HandleEditChoice (int theItem)
{
	SystemEdit( theItem - 1);
}

/**** HandleGameChoice ****/
HandleGameChoice(int theItem)
{
	WindowPtr	whichWindow;
	switch	( theItem)
	{
		case GAME_OPTIONS_ITEM:
			GameOptionsDialog(GAME_OPTIONS_DIALOG);
			break;
		case ANIMAL_POINT_VALUES_ITEM:
			AnimalPointValuesDialog();
			break;
		case HALL_OF_FAME_ITEM:
			HallOfFameDialog();
			break;
	}
}

/*** HandleInContent ***/
HandleInContent(Point mouse)
{
	int	i, h, v;
	RgnHandle	animalRgn;
	Str255	mySound;
	
	GlobalToLocal(&mouse);
	
	for (i = 1; i <= animalRows; ++i)
	{
		if ( PtInRect ( mouse, &row[i].rect) )
		{
			if (!(easyShoot))
			{
/*				animalRgn = NewRgn(); */
				animalRgn = (RgnHandle) GetResource( 'RGN ', anRegion[row[i].animal][row[i].step]);
				
				h = row[i].rect.right - (**animalRgn).rgnBBox.right;
				v = row[i].rect.bottom - (**animalRgn).rgnBBox.bottom;
			
				OffsetRgn(animalRgn, h, v);
				
				if (PtInRgn( mouse, animalRgn)) 
				{
					PaintRgn(animalRgn);
					DoTheSound( BASE_RES_ID + row[i].animal, TRUE );
					AnimalShot(i);
				}
			}	
			else
			{
				DoTheSound( BASE_RES_ID + row[i].animal, TRUE );
				AnimalShot(i);
			}	
		}
	}
}

/*** HandleGameOverButtons ***/
HandleGameOverButtons(Point mouse)
{
	int				partCode, inButton;
	ControlHandle	whichControl;
	
	GlobalToLocal(&mouse);
	
	partCode = FindControl(mouse, gameWindow, &whichControl);
	
	if (whichControl != NIL_POINTER)
	{
		inButton = TrackControl(whichControl, mouse, NIL_POINTER);
		if (inButton != 0)
		{
			if ( whichControl == playAgainControl )
				SetUpGame();
			if (whichControl == quitControl)
				ExitToShell();
		}
	}
}

/*** AnimalShot ***/
AnimalShot(int i)
{
	Str255	scoreString;
	int		length;
	Rect	scoreRect;
	
	gameScore = gameScore + animalValue[row[i].animal];
	length = StringWidth("\pScore: ")+2;
	SetRect(&scoreRect, length, 0, length + StringWidth("\p000000"), 20);
	EraseRect(&scoreRect);
	MoveTo(length, 17);
	NumToString(gameScore, scoreString);
	DrawString(scoreString);
		
	EraseRect(&row[i].rect);
	row[i].rect.left = 0;
	row[i].rect.right = 33;
   	row[i].running = FALSE;
   	
   	if (increaseSpeed)
		speedIncrease = gameScore/1000;
}

/**** ErrorHandler ****/
ErrorHandler(int stringNum)
{
	StringHandle	errorStringH;
	
	if ( ( errorStringH = GetString( stringNum ) ) == NIL_POINTER )
		ParamText( HOPELESSLY_FATAL_ERROR, NIL_STRING, NIL_STRING, NIL_STRING);
	else
	{
		MoveHHi((Handle)errorStringH);
		HLock( (Handle)errorStringH);
		ParamText( *errorStringH, NIL_STRING, NIL_STRING, NIL_STRING);
		HUnlock( (Handle)errorStringH);
	}
	StopAlert( ERROR_ALERT_ID, NIL_POINTER);
	ExitToShell();
}

/*** Animate ***/
Animate(void)
{	int	i, r;
	Rect	copyRect;
	GrafPtr   secondOffScreen;
	
	/* Open loop - goes from 1 to animalRows & allows for WaitNextEvent to occur
		between each animal drawing */
	if (loopValue > animalRows )
		loopValue = 1;
	{		
		if (! (row[loopValue].running) )
		{
			row[loopValue].running = TRUE;
			do{
				r = Randomize(10);
			}
			while (r<1 || r>9);
			row[loopValue].animal = r;
			row[loopValue].speed = animalSpeed[r];
			row[loopValue].step = 1;
		}
				
		if (row[loopValue].step == 2)
			row[loopValue].step = 1;
		else
			row[loopValue].step = 2;
		
		OffsetRect(&row[loopValue].rect, 
				row[loopValue].speed * speedFactor + speedIncrease, 0);
	
		SetRect(&copyRect, row[loopValue].rect.left
			- (row[loopValue].speed * speedFactor + speedIncrease), row[loopValue].rect.top,
				row[loopValue].rect.right, row[loopValue].rect.bottom);

		if (!CreateOffscreenBitMap(&secondOffScreen, &copyRect)) 
			{
				SysBeep(1);
				ExitToShell();
 		   }
  	 	SetPort (secondOffScreen);
   		EraseRect(&copyRect);
   		
   		CopyBits(&offscreen->portBits, &secondOffScreen->portBits, 
   					&animalRect[row[loopValue].animal][row[loopValue].step], &row[loopValue].rect, srcCopy, 0L);
   		SetPort (gameWindow);
   		CopyBits(&secondOffScreen->portBits, &(*gameWindow).portBits,
   					&copyRect, &copyRect, srcCopy, 0L);
   		DestroyOffscreenBitMap(secondOffScreen);

   		if (row[loopValue].rect.left >= windowWidth)
   		{
			row[loopValue].rect.left = 0;
			row[loopValue].rect.right = 33;
   			row[loopValue].running = FALSE;
   			if(gameType == ESCAPED_GAME)
 				AnimalEscaped();
   		}
	}
	++loopValue;
	if (gameType == TIMED_GAME)
		PostTimeRemaining();
}

/*** DrawContent ***/
DrawContent(void)
{	int	i, r;
	Rect	copyRect;
	GrafPtr   secondOffScreen;
	GrafPtr	oldPort;
	
	GetPort( &oldPort);
	SetPort( gameWindow);

	for (i = 1; i<= animalRows; ++i)
   		CopyBits(&offscreen->portBits, &(*gameWindow).portBits,
 					&animalRect[row[i].animal][row[i].step], &row[i].rect,srcCopy, 0L);
	SetPort(oldPort);
}

/** Randomize **/
 Randomize (int range)
 {
 	long	rawResult;
 	rawResult = Random();
 	if( rawResult < 0 ) rawResult *= -1;
 	return( (rawResult * range)/32768);
 }
 
 /*** AnimalEscaped ***/
 AnimalEscaped(void)
 {
 	Str255	scoreString;
	int		length;
	Rect	escapedRect;
		
	++animalsEscaped;
	length = StringWidth("\p000") +2;
	SetRect(&escapedRect, windowWidth - length, 0, windowWidth, 20);
	MoveTo(windowWidth - length, 17);
	NumToString(animalsEscaped, scoreString);
	EraseRect(&escapedRect);
	DrawString(scoreString);
	if ( (gameType == ESCAPED_GAME) && (maxEscaped <= animalsEscaped) )
		EndGame();
 }
 

/*** DoTheSound ***/
DoTheSound(int whichOne, Boolean asynch)
{
	Handle	theSnd;
	OSErr	err, myErr;

	theSnd = GetResource('snd ', whichOne);
	MoveHHi((Handle)theSnd);
	HLock((Handle)theSnd);
	if ( theSnd != NIL_POINTER) 
	{
		if (chanPtr != NIL_POINTER)
			{
				err = SndDisposeChannel(chanPtr, TRUE);
				chanPtr = NIL_POINTER;
			}
			
		chanPtr = NIL_POINTER;
		
		myErr = SndNewChannel(&chanPtr, 0, 0, NIL_POINTER);
		
		if ( (asynch == TRUE) && (myErr == noErr) )
			err = SndPlay(chanPtr, theSnd, TRUE);
		else
			err = SndPlay(NIL_POINTER, theSnd, FALSE);
	}
	HUnlock((Handle)theSnd);
}

/*** WriteGameOver ***/
WriteGameOver(void)
{
	int	length, h, v;
	Rect	myRect;
	
	length = StringWidth("\pGame Over");
	SetRect(&myRect, windowWidth/2 - length/2 - 2, windowHeight/2 - 10 , 
			windowWidth/2 + length/2 + 2, windowHeight/2 + 10);
	EraseRect(&myRect);
	MoveTo(windowWidth/2 - length/2, windowHeight/2);
	DrawString("\pGame Over");
}

/*** EndOfGameControls ***/
EndOfGameControls(void)
{
	ShowControl(playAgainControl);
	ShowControl(quitControl);
}

/*** EndGame ***/
EndGame(void)
{
	gameOver = TRUE;
	WriteGameOver();
	EndOfGameControls();
	if ((gameScore >= topScores[3]) && (showHighScores))
		UpdateHighScores();
}

/*** UpdateHighScores ***/
UpdateHighScores(void)
{
	int				i;
	Str255			numString;
	StringHandle	theString, topNames[4];
	
	GetPlayerName();
	
	for (i=0; i<=2; ++i)
	{
		if ( ( theString = GetString( (i+131) ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		topNames[i+1] = theString;
		HUnlock((Handle)theString);	
	}
		
	if (gameScore >= topScores[1])
	{
		topScores[3] = topScores[2];
		SetString(topNames[3], *topNames[2]);
		topScores[2] = topScores[1];
		SetString(topNames[2], *topNames[1]);
		topScores[1] = gameScore;
		SetString(topNames[1], name);
	}
	else if (gameScore >= topScores[2])
	{
		topScores[3] = topScores[2];
		SetString(topNames[3], *topNames[2]);
		topScores[2] = gameScore;
		SetString(topNames[2], name);
	}
	else
	{
		topScores[3] = gameScore;
		SetString(topNames[3], name);
	}
	for ( i = 0; i<=2; ++i)
	{
		if ( ( theString = GetString( i + HIGH_NAMES_STRING ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);
		SetString(theString, *topNames[i+1]);
		ChangedResource((Handle)theString);
		WriteResource((Handle)theString);
		HUnlock((Handle)theString);
		
		if ( ( theString = GetString( i + HIGH_SCORE_STRING ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		NumToString(topScores[i+1], numString);
		SetString(theString, numString);
		ChangedResource((Handle)theString);
		WriteResource((Handle)theString);
	}
	HallOfFameDialog();
}

/*** GetPlayerName ***/
GetPlayerName(void)
{
	StringHandle	whichString;
	Handle			itemHandle;
	int				stringNum, itemType;
	DialogPtr		theDialog;
	int				itemHit, dialogDone = FALSE;
	GrafPtr 		savePort;
	Rect			itemRect;

	if (gameScore >= topScores[1])
		stringNum = 500;
	else if (gameScore >=topScores[2])
		stringNum = 501;
	else
		stringNum = 502;
		
	GetPort(&savePort);

	if ( ( whichString = GetString( stringNum ) ) == NIL_POINTER )
		ParamText( HOPELESSLY_FATAL_ERROR, NIL_STRING, NIL_STRING, NIL_STRING);

	MoveHHi((Handle)whichString);
	HLock( (Handle)whichString);
	ParamText( *whichString, NIL_STRING, NIL_STRING, NIL_STRING);
	HUnlock( (Handle)whichString);

	if (( theDialog = GetNewDialog( PLAYER_NAME_DIALOG, NIL_POINTER, M_T_F)) 
		== NIL_POINTER)
	ErrorHandler( NO_WIND);

	SelIText(theDialog, PLAYER_NAME_ITEXT, 0, 255);
	dialogDone = FALSE;
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	
	SetCursor(&arrow);
	
	DoTheSound( TARZAN_SOUND, TRUE );
	
	while (dialogDone == FALSE)
	{
		ModalDialog( NIL_POINTER, &itemHit);
		switch( itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
		}
	}
	
	GetDItem( theDialog, PLAYER_NAME_ITEXT, &itemType, &itemHandle, &itemRect);
	MoveHHi((Handle)itemHandle);
	HLock(itemHandle);
	GetIText(itemHandle, &name);
	HUnlock(itemHandle);
	
	DisposDialog(theDialog);
	SetPort(savePort);
}

/**** AnimalPointValuesDialog ****/
AnimalPointValuesDialog(void)
{	
	DialogPtr	theDialog;
	int			itemHit, dialogDone = FALSE;
	GrafPtr savePort;

	GetPort(&savePort);
	
	if (( theDialog = GetNewDialog( ANIMAL_POINT_VALUES_DIALOG, NIL_POINTER, M_T_F)) 
		== NIL_POINTER)
	ErrorHandler( NO_WIND);

	dialogDone = FALSE;
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	while (dialogDone == FALSE)
	{
		ModalDialog( NIL_POINTER, &itemHit);
		switch( itemHit)
		{
			case	RHINO:
			case	ELEPHANT:
			case	ZEBRA:
			case	GORILLA:
			case	ANTELOPE:
			case	GIRAFFE:
			case	LION:
			case	CHEETAH:
			case	 WARTHOG:	
				DoTheSound( BASE_RES_ID + itemHit, TRUE );
				break;
			case OK_APV_DBOX:
				dialogDone = TRUE;
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

/**** AboutDialog ****/
AboutDialog(void)
{	
	DialogPtr	theDialog;
	int			itemHit, dialogDone = FALSE, itemType;
	GrafPtr savePort;
	OSErr	err;
	Handle	theSnd, theIcon, itemHandle;
	Rect	itemRect;
	
	#define ICON_ITEM	5

	GetPort(&savePort);
	
	if (( theDialog = GetNewDialog( ABOUT_DIALOG, NIL_POINTER, M_T_F)) 
		== NIL_POINTER)
	ErrorHandler( NO_WIND);

	dialogDone = FALSE;
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	
	GetDItem(theDialog, ICON_ITEM, &itemType, &itemHandle, &itemRect);
	theIcon = GetIcon(MY_HEAD);
	PlotIcon (&itemRect, theIcon);
	
	SetPort(theDialog);
	while (dialogDone == FALSE)
	{
		ModalDialog( NIL_POINTER, &itemHit);
		switch( itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
			case SHOW_INTRO_SCREEN:
				IntroDialog();
				break;
			case HELP_BUTTON:
				break;
			case HEAD_ICON:
				theIcon = GetIcon(OUCH_HEAD);
				PlotIcon(&itemRect, theIcon);
				if (( theSnd = GetResource('snd ', OUCH_SOUND) ) 
							== NIL_POINTER)
					ErrorHandler( CANT_LOAD_SND);
				SndPlay(NIL_POINTER, theSnd, FALSE);
				theIcon = GetIcon(MY_HEAD);
				PlotIcon(&itemRect, theIcon);
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

/**** HallOfFameDialog ****/
HallOfFameDialog(void)
{	
	DialogPtr	theDialog;
	int			itemHit, dialogDone = FALSE, i, itemType;
	Rect	itemRect;
	Handle	itemHandle;
	StringHandle	theString;
	GrafPtr savePort;

	GetPort(&savePort);
	
	if (( theDialog = GetNewDialog( HALL_OF_FAME_DIALOG, NIL_POINTER, M_T_F)) 
		== NIL_POINTER)
	ErrorHandler( NO_WIND);
	
	for (i = 0; i<=5; ++i)
	{
		if ( ( theString = GetString( (i +  128) ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		GetDItem( theDialog, (i+3), &itemType, &itemHandle, &itemRect);
		SetIText( itemHandle, *theString);
		HUnlock((Handle)theString);	
	}

	dialogDone = FALSE;
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	while (dialogDone == FALSE)
	{
		ModalDialog( NIL_POINTER, &itemHit);
		switch( itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
			case CLEAR_SCORES_BUTTON:
				dialogDone = TRUE;
				ClearHighScores();
				DrawScoreBar();
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

/*** ClearHighScores ***/
ClearHighScores(void)
{
	int				itemType, i;
	Rect			itemRect;
	Handle			itemHandle;
	StringHandle	theString;
	

	for (i = 0; i<=2; ++i)
	{
		if ( ( theString = GetString( i + HIGH_SCORE_STRING ) ) == NIL_POINTER )
				ErrorHandler(NO_STR);
		HNoPurge((Handle)theString);
		SetString( theString, "\p0"); 
		ChangedResource((Handle)theString);
		WriteResource((Handle)theString);
		HPurge((Handle)theString);
		
		if ( ( theString = GetString( i + HIGH_NAMES_STRING ) ) == NIL_POINTER )
				ErrorHandler(NO_STR);
		HNoPurge((Handle)theString);
		SetString( theString, "\pPlayer Name"); 
		ChangedResource((Handle)theString);
		WriteResource((Handle)theString);
		HPurge((Handle)theString);
		topScores[i+1] = 0;
	}
}


/*** CheckITextValues ***/
Boolean CheckITextValues(DialogPtr	theDialog, int dLogID)
{
	int		itemType, i;
	Rect	itemRect;
	Handle	itemHandle;

	long	stringValue;
	Boolean	dialogDone;
	
	dialogDone = TRUE;
	for (i = MINUTES; i <= MAX_ESCAPED; ++i)
	{
		GetDItem( theDialog, i, &itemType, &itemHandle, &itemRect);
		MoveHHi((Handle)itemHandle);
		HLock(itemHandle);
		GetIText(itemHandle, &preferenceString[i]);
		HUnlock(itemHandle);
	}
	

	StringToNum(&preferenceString[MAX_ESCAPED],&stringValue);

	if (stringValue<1 || stringValue>99)
	{
		BadValue(ESCAPED_ERROR);
		SelIText(theDialog, MAX_ESCAPED, 0, 255);
		dialogDone = FALSE;
	}
	StringToNum(&preferenceString[SECONDS],&stringValue);
	if (stringValue<0 || stringValue>59)
	{
		BadValue(SECONDS_ERROR);
		SelIText(theDialog, SECONDS, 0, 255);
		dialogDone = FALSE;
	}
	StringToNum(&preferenceString[MINUTES],&stringValue);
	if (stringValue<0 || stringValue>99)
	{
		BadValue(MINUTES_ERROR);
		SelIText(theDialog, MINUTES, 0, 255);
		dialogDone = FALSE;
	}
	return (dialogDone);
}

/*** CheckHighScoreDisplay ***/
CheckHighScoreDisplay(void)
{
	int		i;
	long	savedValues[8], preferenceValue[8];
	Str255	savedPrefs[8];
	StringHandle	theString;
	
	showHighScores = TRUE;
	
	for (i = 0; i<=7; ++i)
	{
		if ( ( theString = GetString( i + 200 ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		StringToNum(*theString , &savedValues[i]);	
		NumToString(savedValues[i], savedPrefs[i]);
		HUnlock((Handle)theString);
		StringToNum(&preferenceString[i], &preferenceValue[i]);
	}
	
	for (i=0; i<=4; ++i)
	{
		if (savedValues[i] != preferenceValue[i]) 
			showHighScores = FALSE;
	}
	
/* This bit of code checks to see which of the 3 strings, MINUTES & SECONDS, or
	MAX_ESCAPED to check to see if the preferences match the highScorePrefs.
	This is done because, i.e. so a mismatched MINS and SECS don't screw up
	the display and recording of high score if it both preferences are set to
	be an ESCAPED_GAME */
	if (showHighScores)
	{
		switch (savedValues[TIMED_OR_ESCAPED - 200])
			{
				case TIMED_RADIO:
					if ( savedValues[MINUTES_STRING-200] != 
							preferenceValue[MINUTES_STRING-200])
							showHighScores = FALSE;
					if ( savedValues[SECONDS_STRING-200] != 
							preferenceValue[SECONDS_STRING-200])
							showHighScores = FALSE;
					break;
				case ESCAPED_RADIO:
					if ( savedValues[MAX_ESCAPED_STRING-200] != 
							preferenceValue[MAX_ESCAPED_STRING-200])
							showHighScores = FALSE;
					break;
			}	
	}
	
	if (showHighScores)
		GetHighScores();
}

/**** GameOptionsDialog ****/
GameOptionsDialog(int dLogID)
{
	DialogPtr	theDialog;
	int		itemType, i, itemHit, dialogDone = FALSE;
	Rect	itemRect;
	Handle	itemHandle;
	long	stringValue;			
	GrafPtr savePort;
	StringHandle	theString, secondString;

	GetPort(&savePort);

	if (( theDialog = GetNewDialog( dLogID, NIL_POINTER, M_T_F)) 
			== NIL_POINTER)
		ErrorHandler( NO_WIND);
		
	DialogInit(theDialog, dLogID);
	
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	while (dialogDone == FALSE)
	{
		ModalDialog( NIL_POINTER, &itemHit);
		switch( itemHit)
		{
			case OK_BUTTON:
				dialogDone = CheckITextValues(theDialog, dLogID);
				if (!dialogDone)
				{
					break;
				}
				showHighScores = FALSE;
				CheckHighScoreDisplay();
				SetUpGame();
				break;
				
			case SAVE_AND_CLEAR_BUTTON:
				dialogDone = CheckITextValues(theDialog, dLogID);
				if (!dialogDone)
				{
					break;
				}
				else
				{
					UpdatePreferences(theDialog, dLogID);	
					ClearHighScores();
					showHighScores = TRUE;
					SetUpGame();
					break;
				}
				
			case CANCEL_BUTTON:
				dialogDone = TRUE;
				break;

			case TIMED_RADIO:
				GetDItem( theDialog, ESCAPED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, TIMED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(TIMED_RADIO, preferenceString[TIMED_OR_ESCAPED-200]);
				break;
			case ESCAPED_RADIO:
				GetDItem( theDialog, TIMED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, ESCAPED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(ESCAPED_RADIO, preferenceString[TIMED_OR_ESCAPED-200]);
				break;
				
			case SLOW_RADIO:
				GetDItem( theDialog, MEDIUM_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, FAST_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, SLOW_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(SLOW_RADIO, preferenceString[ANIMAL_SPEED-200]);
				break;
			case MEDIUM_RADIO:
				GetDItem( theDialog, SLOW_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, FAST_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, MEDIUM_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(MEDIUM_RADIO, preferenceString[ANIMAL_SPEED-200]);
				break;
			case FAST_RADIO:
				GetDItem( theDialog, MEDIUM_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, SLOW_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, FAST_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(FAST_RADIO, preferenceString[ANIMAL_SPEED-200]);
				break;
				
			case NINE_INCH_RADIO:
				GetDItem( theDialog, CURRENT_SCREEN_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, NINE_INCH_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(NINE_INCH_RADIO, preferenceString[WINDOW_SIZE-200]);
				break;
			case CURRENT_SCREEN_RADIO:
				GetDItem( theDialog, NINE_INCH_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, CURRENT_SCREEN_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(CURRENT_SCREEN_RADIO, preferenceString[WINDOW_SIZE-200]);
				break;
				
			case INCREASE_YES_RADIO:
				GetDItem( theDialog, INCREASE_NO_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, INCREASE_YES_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(INCREASE_YES_RADIO, preferenceString[INCREASE_SPEED-200]);
				break;
			case INCREASE_NO_RADIO:
				GetDItem( theDialog, INCREASE_YES_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, INCREASE_NO_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(INCREASE_NO_RADIO, preferenceString[INCREASE_SPEED-200]);
				break;
						
			case IN_ANIMAL_OUTLINE_RADIO:
				GetDItem( theDialog, IN_BOX_SURROUNDING_ANIMAL_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, IN_ANIMAL_OUTLINE_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(IN_ANIMAL_OUTLINE_RADIO, preferenceString[SCORE_WHEN_HIT-200]);
				break;
			case IN_BOX_SURROUNDING_ANIMAL_RADIO:
				GetDItem( theDialog, IN_ANIMAL_OUTLINE_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, OFF);
				GetDItem( theDialog, IN_BOX_SURROUNDING_ANIMAL_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue( (ControlHandle)itemHandle, ON);
				NumToString(IN_BOX_SURROUNDING_ANIMAL_RADIO, preferenceString[SCORE_WHEN_HIT-200]);
				break;
		}
	}
	
	DisposDialog(theDialog);
	SetPort(savePort);
}

/*** UpdatePreferences ***/
UpdatePreferences(DialogPtr theDialog, int dLogID)
{
	int		itemType, i;
	Rect	itemRect;
	Handle	itemHandle;
	StringHandle	theString;
	
	for (i = 200; i<=207; ++i)
	{
		if ( ( theString = GetString( i ) ) == NIL_POINTER )
				ErrorHandler(NO_STR);
		HNoPurge((Handle)theString);
		SetString( theString, preferenceString[i-200]); 
		ChangedResource((Handle)theString);
		WriteResource((Handle)theString);
		HPurge((Handle)theString);
	}
}

/*** DialogInit ***/
DialogInit(DialogPtr theDialog, int dLogID)
{
	int		itemType, i, baseResDiff;
	Rect	itemRect;
	Handle	itemHandle;
	Str255	myString;
	long	itemNumber;
	StringHandle	theString;


	for (i = TIMED_OR_ESCAPED; i<=SCORE_WHEN_HIT; ++i)
	{
		if ( ( theString = GetString( i ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		StringToNum(*theString,&itemNumber);
		GetDItem( theDialog, itemNumber, &itemType, &itemHandle, &itemRect);
		SetCtlValue( (ControlHandle)itemHandle, ON);
		NumToString(itemNumber, preferenceString[i-200]);
		HUnlock((Handle)theString);
	}

	for (i = MINUTES_STRING; i<=MAX_ESCAPED_STRING; ++i)
	{
		if ( ( theString = GetString( i ) ) == NIL_POINTER )
			ErrorHandler(NO_STR);
		MoveHHi((Handle)theString);
		HLock((Handle)theString);	
		GetDItem( theDialog, (i-200), &itemType, &itemHandle, &itemRect);
		SetIText( itemHandle, *theString);
		/* item number is just a place holder in the next two lines (it's significant
			as the item number in all other code) I couldn't get the string handle
			to equate to the string, so this is my work around. I make the string
			handle a number, then make the number a string.  Ugly but functional */
		StringToNum(*theString,&itemNumber);
		NumToString(itemNumber, preferenceString[i-200]);
		HUnlock((Handle)theString);
	}
}

/**** BadValue ****/
BadValue(int stringNum)
{
	StringHandle	errorStringH;
	
	if ( ( errorStringH = GetString( stringNum ) ) == NIL_POINTER )
		ParamText( HOPELESSLY_FATAL_ERROR, NIL_STRING, NIL_STRING, NIL_STRING);
	else
	{
		MoveHHi((Handle)errorStringH);
		HLock( (Handle)errorStringH);
		ParamText( *errorStringH, NIL_STRING, NIL_STRING, NIL_STRING);
		HUnlock( (Handle)errorStringH);
	}
	StopAlert( ERROR_EDIT_TEXT, NIL_POINTER);
}

/*** IntroDialog ***/
IntroDialog(void)
{
	DialogPtr	theDialog;
	Rect	dialogRect, clippingRect, dLogOffscreenRect;
	GrafPtr savePort, dLogOffscreen;
	int			itemHit, dialogDone = FALSE, i, beginTicks, currentTicks, h;
	PicHandle	introPic;

	GetPort(&savePort);

	if (( theDialog = GetNewDialog( INTRO_DIALOG, NIL_POINTER, M_T_F)) 
			== NIL_POINTER)
		ErrorHandler( NO_WIND);

	dialogDone = FALSE;
	CenterWindow( theDialog);
	ShowWindow( theDialog);
	
	SetPort(theDialog);

	dialogRect = theDialog->portRect;

	if (!CreateOffscreenBitMap(&dLogOffscreen, &dialogRect)) 
	{
		SysBeep(1);
		ExitToShell();
    }
	introPic = GetPicture ( 475);
	SetPort (dLogOffscreen);
	DrawPicture(introPic, &dialogRect);
  
	SetPort(theDialog);
	
	DoTheSound(INTRO_SOUND, TRUE );
	
	h = (dialogRect.bottom-dialogRect.top)/2;
	
	SetRect(&clippingRect, dialogRect.left, h,
		 dialogRect.right,h);
	for (i = 1; i<= h+1; i+=4)
	{
		ClipRect(&clippingRect);
		clippingRect.top -= 4;
		clippingRect.bottom += 4;
		CopyBits(&dLogOffscreen->portBits, &(*theDialog).portBits,&dialogRect,
			&dialogRect, srcCopy,0L);			
		beginTicks = TickCount();
		do
		{
			currentTicks = TickCount();
		}
		while ((currentTicks - beginTicks) <=2);
	}
	
	while (!(Button()));
	
	DisposDialog(theDialog);
	SetPort(savePort);
}