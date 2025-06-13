/*
	Code release for Safari II 1.1.3
	Changes since 1.0 release:
		It's now FREE! (please send comments & a postcard)
		The sicn's were eliminated from the apple menu so it should now work on older systems
		The dialog box in which I begged for a job is gone (I've found work, thanks!)
		The option to 'score when hit' in rectangle surrounding animal now works.
		   
	There still seems to be problems with the sound playing on older 68000 Macs.  Any advice on
	this problem, or more descript diagnoses would be appreciated.

	SAFARI II © 1991 by John Gallaugher
		A program written in Think C.  Portions copyright Symantec and Apple Computer.

	In 1985 I wrote a program called Safari to try to teach myself the Macintosh toolbox.  
	The original Safari was simple and cute, but it was also a program which doesn't work on
	today's machines.  Safari II is an update to the orginal Safari.  It should work on all
	Macs with at least 1 meg of memory running system 6.0.x or greater.
	
	Although I have done quite a bit of Macintosh database development, I have never programmed
	professioanlly in the toolbox or in C.  As such, please excuse the clumsiness of some of
	this code.  I am distributing the source code and game for free in hopes that it will help
	others learn some of the basics of Macintosh game development.  My experience has found that
	there's not much free source code out there to help the would-be game developmer.  I hope my
	modest submission helps some folks get started.
	
	Although this product is offerd for free, I DO ask those pros and semi-pros out there to
	offer suggestions on how I can improve this code.  Since I'm pretty green in toolbox coding
	I would greatly appreciate advice.  Please don't hold back,  criticize me to the hilt!  
	I need and welcome the help.
	
	Also, please don't distribute any modified versions of Safari.  I welcome you to learn from
	the code and encourage you to produce original work, but I would like all updates of the game
	to pass through me.
	
	Lastly, please send comments!  I love to hear from folks who have received my game.  
	My E-Mail addresses are:
	CIS - 70137,763
	AOL - GALLAUGHER
	
	If you'd care to send any U.S. Mail (I love postcards), forward
	them to:
		Safari
		c/o John Gallaugher
		Three Allegheny Center #118
		Pittsburgh, PA 15212
		
	A special thanks to the helpful folks at the Boston Computer Society's MacTech group, and to
	the outstanding faculty of the Boston College Computer Science Program for their patient help
	in answering my silly questions.
	
	This program seems to work fine on my 4 meg LC running under System 7.0 if you run into any
	problems, PLEASE let me know.
	
	RESOURCE NOTE:  All sounds were created using the microphone on a Macintosh LC.
		All pictures were created using MacPaint.  The 'RGN ' resources which are used to detect
		the boundries of the animal bodies were created using a utilty program called 'Regions'
		created by the late Duane Blehm (it should be available via local user groups and on-line
		services).  All other resources were created directly in ResEdit.
*/

#include <Sound.h> /* adds routines need to play Asynch Sound */

/*
	sorry about any inconsistencies in declaring resource numbers.  I started
	and stopped the project several times and added things on the fly without
	thinking of a logical numbering system.  Feel free to suggest a convention.
*/

#define M_T_F					((void *) -1L) /* used in setting up windows */
#define	BASE_RES_ID				400
#define	REMOVE_ALL_EVENTS		0

#define	BORDER					6  /* number of pixels between window and screen width */
#define	TITLE_BAR_HEIGHT		19 /* I guessed, is there a TB call to get this? */
#define	TICK_DELAY				1  /* this cycles the animation through 1 tick, or 1/60th of a second */

/* Menu IDs */
#define	APPLE_MENU_ID			1
#define	FILE_MENU_ID			401
#define	EDIT_MENU_ID			402
#define	GAME_MENU_ID			403

/* Menu Items */
#define	ABOUT_ITEM				1
#define	NEW_GAME_ITEM			1
#define	PAUSE_RESUME_ITEM		2
#define	STOP_ITEM				3
#define	QUIT_ITEM				5
#define	UNDO_ITEM				1
#define	CUT_ITEM				3
#define	COPY_ITEM				4
#define	PASTE_ITEM				5
#define	CLEAR_ITEM				6
#define	GAME_OPTIONS_ITEM		1
#define	ANIMAL_POINT_VALUES_ITEM	2
#define	HALL_OF_FAME_ITEM		3

/* Alerts */
#define	ERROR_ALERT_ID			401
#define	ERROR_EDIT_TEXT			402

/* Dialogs */
#define	GAME_OPTIONS_DIALOG		200
#define	SAVE_HIGH_SCORES_DIALOG	300
#define	ANIMAL_POINT_VALUES_DIALOG	129
#define	INTRO_DIALOG			475
#define	ABOUT_DIALOG			476
#define	PLAYER_NAME_DIALOG		500
#define	HALL_OF_FAME_DIALOG		550

/* Dialog Items */
#define OK_BUTTON				1
#define	SAVE_AND_CLEAR_BUTTON	2
#define	CANCEL_BUTTON			3
#define	TIMED_RADIO				4
#define	ESCAPED_RADIO			5
#define	NINE_INCH_RADIO			6
#define	CURRENT_SCREEN_RADIO	7
#define	INCREASE_YES_RADIO		8
#define	INCREASE_NO_RADIO		9
#define	IN_BOX_SURROUNDING_ANIMAL_RADIO	10
#define	IN_ANIMAL_OUTLINE_RADIO	11
#define	MINUTES					12
#define	SECONDS					13
#define	MAX_ESCAPED				14
#define SPEED_POP_UP			15
#define	SLOW_RADIO				25
#define	MEDIUM_RADIO			26
#define	FAST_RADIO				27
#define	PLAYER_NAME_ITEXT		3
#define	CLEAR_SCORES_BUTTON		2
#define	OK_APV_DBOX				9
#define	SHOW_INTRO_SCREEN		2
#define	HELP_BUTTON				3
#define	HEAD_ICON				5
#define	OUCH_ICON				7

/* icons */
#define	MY_HEAD					476
#define	OUCH_HEAD				477

/* Error Strings */
#define	NO_MBAR					BASE_RES_ID
#define	NO_MENU					BASE_RES_ID + 1
#define	NO_WIND					BASE_RES_ID + 3
#define NO_STR					BASE_RES_ID + 4
#define CANT_LOAD_SND			BASE_RES_ID + 5
#define	MINUTES_ERROR			BASE_RES_ID + 6
#define	SECONDS_ERROR			BASE_RES_ID + 7
#define	ESCAPED_ERROR			BASE_RES_ID + 8
#define	NO_RGN					BASE_RES_ID + 9

/* 'STR ' resource IDs */
#define	TIMED_OR_ESCAPED		200
#define ANIMAL_SPEED			201
#define	WINDOW_SIZE				202
#define	INCREASE_SPEED			203
#define	SCORE_WHEN_HIT			204
#define	MINUTES_STRING			212
#define	SECONDS_STRING			213
#define	MAX_ESCAPED_STRING		214
#define	HIGH_SCORE_STRING		128
#define	HIGH_NAMES_STRING		131

/* cursor */
#define	CROSSHAIR				BASE_RES_ID

/* button types */
#define	PLAY_AGAIN_BUTTON		128
#define	QUIT_BUTTON				129

/* sounds */
#define	TARZAN_SOUND			409
#define	INTRO_SOUND				410
#define	OUCH_SOUND				411

/* PICTs */
#define ANIMAL_PICTURE			BASE_RES_ID  /* picture containing animal bitmaps */

/* animals */
#define	RHINO					0
#define	ELEPHANT				1
#define	ZEBRA					2
#define	GORILLA					3
#define	ANTELOPE				4
#define	GIRAFFE					5
#define	LION					6
#define	CHEETAH					7
#define WARTHOG					8

#define	DRAG_THRESHOLD			30

#define	WINDOW_HOME_LEFT		5
#define	WINDOW_HOME_TOP			45
#define	NEW_WINDOW_OFFSET		20

#define	HOPELESSLY_FATAL_ERROR	"\pGoodness knows what happened!"

/* this structure is used for each row.  There is one animal per row. */
struct
{
	Rect	rect;		/* the rect into which the animal bitmap is drawn */
	Boolean	running;	/* is an animal running in the row? */
	int		animal;		/* what type of animal is it? */
	int		speed;		/* how fast does it go? */
	int		step;		/* alternates each pass so that different animal positions are
							drawn, creating the illusion of a stride */
}
row[20];				/* there can be no more than 20 rows of animals */

struct
{
	Rect	rect[2];	/* source rectangles from which animal bitmaps are taken */
	int		value;		/* point value of the animal */
	int		speed;
	int		region[2];	/* array containing two regions for each animal; */
						/* used to check if someone has clicked inside animal outline */
}
animal[9];				/* there are nine different animals */

/* unusual variable names in comments */

/*
	increaseSpeed is true when Game Options dialog has the speed of the game
		increase as the score increases.
	easyShoot allows player to score by clicking in a rect surrounding the animal
		instead of clicking directly within outline of the animal (good for kids).
	loopValue keeps track of a group of statements which mimic a for loop, but which
		increment only once every pass through the animation procedure.  This is done
		instead of a for loop so the game can respond to an event after each animal
		is drawn.  
	speedFactor and speedIncrease regulate initial speed of game and increase
		in speed as game play continues
*/

Boolean			gDone, gamePaused, gameOver, increaseSpeed, easyShoot;
Boolean			standardWindow, showHighScores, escapedGame;
MenuHandle		gAppleMenu, gFileMenu, gEditMenu;
Rect			gDragRect, whereAnimalsRun; /* latter is where animation occurs */
int				windowHeight, windowWidth, lastTicks;

int				gameScore, animalsEscaped, animalRows;
int				loopValue, speedFactor, speedIncrease;
long			maxEscaped, timeRemaining, lastTime, topScores[3];

WindowPtr		gameWindow;
GrafPtr			offscreen;
ControlHandle	playAgainControl, quitControl;
Str255			preferenceString[8], name;

pascal Boolean GameOptionsDProc(DialogPtr theDialog, EventRecord *theEvent, int *itemHit);

main(void)
{
	ToolBoxInit();
	
	SetUpFirstGlobals();
	
	MenuBarInit();
	WindowInit();
	SetUpDragRect();
	
	InitOffscreen();
	SetUpGame();
	IntroDialog();

	EventLoop();
}

ToolBoxInit(void)
{
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	InitCursor();
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	FlushEvents(everyEvent, REMOVE_ALL_EVENTS);
}

SetUpFirstGlobals(void)
{	
	DialogPtr	theDialog;

	showHighScores = TRUE;
/*
	DialogInit() requires that the Dialog be created since it calls the DITL,
	so we create the dialog, call the procedure, and dispose of the dialog
	without ever displaying the dialog
*/
	if ((theDialog = GetNewDialog(GAME_OPTIONS_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	lastTicks = TickCount();
	DialogInit(theDialog);
	DisposDialog(theDialog);
}

MenuBarInit(void)
{
	Handle	myMenuBar;
	
	if ((myMenuBar = GetNewMBar(BASE_RES_ID)) == NULL) 
	{
		ErrorHandler(NO_MBAR);
	}
	SetMenuBar(myMenuBar);
	
	if ((gAppleMenu = GetMHandle(APPLE_MENU_ID)) == NULL)
	{
		ErrorHandler(NO_MENU);
	}
	AddResMenu(gAppleMenu, 'DRVR');
	
	if ((gEditMenu = GetMHandle(EDIT_MENU_ID)) == NULL)
	{
		ErrorHandler(NO_MENU);
	}
	
	if ((gFileMenu = GetMHandle(FILE_MENU_ID)) == NULL)
	{
		ErrorHandler(NO_MENU);
	}
	
	DrawMenuBar();
}

WindowInit(void)
{
	if ((gameWindow = GetNewWindow(BASE_RES_ID, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	playAgainControl = GetNewControl(PLAY_AGAIN_BUTTON, gameWindow);
	quitControl = GetNewControl(QUIT_BUTTON, gameWindow);
}

CenterWindow(WindowPtr theWindow)
{
	int		newLeft, newTop;
	Rect	windowRect = theWindow->portRect;
	Rect	screenRect = screenBits.bounds;
	
	newLeft = ((screenRect.right - screenRect.left) - (windowRect.right - windowRect.left)) / 2;
	newTop = ((screenRect.bottom - (screenRect.top + MBarHeight + TITLE_BAR_HEIGHT)) 
		- (windowRect.bottom - windowRect.top)) / 2;
	MoveWindow(theWindow, newLeft, newTop + MBarHeight + TITLE_BAR_HEIGHT, TRUE);
	ShowWindow(theWindow);
}

AdjustWindowSize(void)
{
	Rect	windowRect = gameWindow->portRect;
	int		h, v;
	
	SetPort(gameWindow);
	
	if (!standardWindow) /* if it's not the 9" screen window, then */
	{
		SizeWindow(gameWindow,
			screenBits.bounds.right - BORDER, 
			screenBits.bounds.bottom - (MBarHeight + BORDER + TITLE_BAR_HEIGHT),
			TRUE);
	}
	else
	{
		SizeWindow(gameWindow, 506, 298, TRUE);
	}
	
	windowRect = gameWindow->portRect;
	
	windowWidth = windowRect.right - windowRect.left;
	windowHeight = windowRect.bottom - windowRect.top;
	
	CenterWindow(gameWindow);
	TextFont(systemFont);
	
	v = windowHeight / 2 + 20;
	h = windowWidth / 2 - 100;
	MoveControl(playAgainControl, h, v);
	h = windowWidth / 2 + 20;
	MoveControl(quitControl, h, v);
}

Boolean CreateOffscreenBitMap(GrafPtr *newOffscreen, Rect *inBounds)
{
	GrafPtr savePort;
	GrafPtr newPort;

	GetPort(&savePort);    /* need this to restore thePort after OpenPort */

	newPort = (GrafPtr) NewPtr(sizeof(GrafPort));    /* allocate the grafPort */
	if (MemError() != noErr)
	{
		return false;                 /* failed to allocate the off-screen port */
	}
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
	if (MemError() != noErr)
	{   /* check to see if we had enough room for the bits */
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

void DestroyOffscreenBitMap(GrafPtr oldOffscreen)
{
	ClosePort(oldOffscreen);						/* dump the visRgn and clipRgn */
	DisposPtr(oldOffscreen->portBits.baseAddr);		/* dump the bits */
	DisposPtr((Ptr)oldOffscreen);					/* dump the port */
}

InitOffscreen(void)
{
	Rect	offScreenRect;
	
	SetRect(&offScreenRect, 0, 0, 227, 151); 		/* size of ANIMAL_PICTURE rect */

	if (!CreateOffscreenBitMap(&offscreen, &offScreenRect)) 
	{
		SysBeep(1);
		ExitToShell();
    }
    
	SetPort(offscreen);
	DrawPicture(GetPicture(ANIMAL_PICTURE), &offScreenRect);
	SetPort(gameWindow);
}

SetUpDragRect(void)
{
	gDragRect = screenBits.bounds;
	gDragRect.left += DRAG_THRESHOLD;
	gDragRect.right -= DRAG_THRESHOLD;
	gDragRect.bottom -= DRAG_THRESHOLD;
}

DoSound(int whichOne, Boolean asynch)
{
	Handle					theSnd;
	static SndChannelPtr	chanPtr;
	OSErr					err;

	theSnd = GetResource('snd ', whichOne);
	MoveHHi((Handle) theSnd);
	HLock((Handle) theSnd);
	if (theSnd != NULL) 
	{
		if (chanPtr != NULL)
		{
			(void) SndDisposeChannel(chanPtr, TRUE);
			chanPtr = NULL;
		}
		
		chanPtr = NULL;
		
		err = SndNewChannel(&chanPtr, 0, 0, NULL);
		
		if ((asynch) && (err == noErr))
		{
			(void) SndPlay(chanPtr, theSnd, TRUE);
		}
		else
		{
			(void) SndPlay(NULL, theSnd, FALSE);
		}
	}
	HUnlock((Handle) theSnd);
}

SetUpGame(void)
{
	register int	i;
	long			stringValue;
	Rect			windowRect;
	
	SetPreferences(); 								/* get current game settings */
	if (showHighScores)
	{
		GetHighScores();
	}
	AdjustWindowSize();
	HideControl(playAgainControl);
	HideControl(quitControl);

	SetRect(&windowRect, 0, 0, windowWidth, windowHeight);
	EraseRect(&windowRect);
	speedIncrease = 0;
	
	/* Set Animal Values */
	
	SetRect(&animal[RHINO].rect[0], 2, 2, 35, 35);
	SetRect(&animal[RHINO].rect[1], 40, 2, 73, 35);
	animal[RHINO].value = 10;
	animal[RHINO].speed = 2;
	animal[RHINO].region[0] = 400;
	animal[RHINO].region[1] = 401;
				
	SetRect(&animal[ELEPHANT].rect[0], 2, 40, 35, 73);
	SetRect(&animal[ELEPHANT].rect[1], 40, 40, 73, 73);
	animal[ELEPHANT].value = 10;
	animal[ELEPHANT].speed = 2;
	animal[ELEPHANT].region[0] = 402;
	animal[ELEPHANT].region[1] = 403;
				
	SetRect(&animal[ZEBRA].rect[0], 2, 78, 35, 111);
	SetRect(&animal[ZEBRA].rect[1], 40, 78, 73, 111);
	animal[ZEBRA].value = 60;
	animal[ZEBRA].speed = 4;
	animal[ZEBRA].region[0] = 404;
	animal[ZEBRA].region[1] = 405;
		
	SetRect(&animal[GORILLA].rect[0], 2, 116, 35, 149);
	SetRect(&animal[GORILLA].rect[1], 40, 116, 73, 149);
	animal[GORILLA].value = 50;
	animal[GORILLA].speed = 3;
	animal[GORILLA].region[0] = 406;
	animal[GORILLA].region[1] = 407;
			
	SetRect(&animal[ANTELOPE].rect[0], 78, 2, 111, 35);
	SetRect(&animal[ANTELOPE].rect[1], 116, 2, 149, 35);
	animal[ANTELOPE].value = 75;
	animal[ANTELOPE].speed = 5;
	animal[ANTELOPE].region[0] = 408;
	animal[ANTELOPE].region[1] = 409;
		
	SetRect(&animal[GIRAFFE].rect[0], 78, 40, 111, 73);
	SetRect(&animal[GIRAFFE].rect[1], 116, 40, 149, 73);
	animal[GIRAFFE].value = 60;
	animal[GIRAFFE].speed = 4;
	animal[GIRAFFE].region[0] = 410;
	animal[GIRAFFE].region[1] = 411;
				
	SetRect(&animal[LION].rect[0], 78, 78, 111, 111);
	SetRect(&animal[LION].rect[1], 116, 78, 149, 111);
	animal[LION].value = 75;
	animal[LION].speed = 5;
	animal[LION].region[0] = 412;
	animal[LION].region[1] = 413;
			
	SetRect(&animal[CHEETAH].rect[0], 78, 116, 111, 149);
	SetRect(&animal[CHEETAH].rect[1], 116, 116, 149, 149);
	animal[CHEETAH].value = 80;
	animal[CHEETAH].speed = 6;
	animal[CHEETAH].region[0] = 414;
	animal[CHEETAH].region[1] = 415;
				
	SetRect(&animal[WARTHOG].rect[0], 154, 2, 187, 35);
	SetRect(&animal[WARTHOG].rect[1], 192, 2, 225, 35);
	animal[WARTHOG].value = 60;
	animal[WARTHOG].speed = 4;
	animal[WARTHOG].region[0] = 416;
	animal[WARTHOG].region[1] = 417;
			
	gameScore = 0;
	
	if (escapedGame)
	{
		animalsEscaped = 0;
	}
	else
	{
		StringToNum(preferenceString[MINUTES], &stringValue);
		timeRemaining = stringValue * 60;
		StringToNum(preferenceString[SECONDS], &stringValue);
		timeRemaining += stringValue;
		GetDateTime(&lastTime);
	}
	
	DrawScoreBar();
	
	SetRect(&whereAnimalsRun, 0, 20, windowWidth, windowHeight);
		
	animalRows = (windowHeight - 20) / 33;
	
	if (animalRows > 20)
	{
		animalRows = 20;
	}
			
	for (i = 0; i < animalRows; i++)
	{
		SetRect(&row[i].rect, 0, 33 * i + 20, 33, 33 * i + 53);
		row[i].running = FALSE;
	}
	loopValue = 0;
	gamePaused = FALSE;
	gameOver = FALSE;
	
}

SetPreferences(void)
{
	register int	i;
	long			currentSetting;
	
	for (i = 0; i <= 4; i++)
	{
		StringToNum(preferenceString[i], &currentSetting);
		switch (currentSetting)
		{
			case TIMED_RADIO:
			case ESCAPED_RADIO:
				escapedGame = (currentSetting == ESCAPED_RADIO);
				break;
			case NINE_INCH_RADIO:
			case CURRENT_SCREEN_RADIO:
				standardWindow = (currentSetting == NINE_INCH_RADIO);
				break;
			case INCREASE_YES_RADIO:
			case INCREASE_NO_RADIO:
				increaseSpeed = (currentSetting == INCREASE_YES_RADIO);
				break;
			case SLOW_RADIO:
				speedFactor = 1;
				break;
			case MEDIUM_RADIO:
				speedFactor = 3;
				break;
			case FAST_RADIO:
				speedFactor = 5;
				break;
			case IN_ANIMAL_OUTLINE_RADIO:
			case IN_BOX_SURROUNDING_ANIMAL_RADIO:
				easyShoot = (currentSetting == IN_BOX_SURROUNDING_ANIMAL_RADIO);
				break;
			default:
				break;
		}
	}
	StringToNum(preferenceString[MAX_ESCAPED], &maxEscaped);
}

GetHighScores(void)
{
	register int	i;
	StringHandle	theString;
				
	for (i = 0; i <= 2; i++)
	{
		if ((theString = GetString(HIGH_SCORE_STRING + i)) == NULL)
			ErrorHandler(NO_STR);
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		StringToNum(*theString, &topScores[i]);
		HUnlock((Handle) theString);
	}
}

DrawScoreBar(void)
{
	Str255	scoreString;
	int		length;
	Rect	theRect;
	
	SetRect(&theRect, 0, 0, windowWidth, 20);
	EraseRect(&theRect);
	
	MoveTo(2, 17);
	DrawString("\pScore: ");
	NumToString(gameScore, scoreString);
	DrawString(scoreString);

	if (showHighScores)
	{
		PostHighScore();
	}
	
	if (escapedGame)
	{
		length = StringWidth("\pAnimals Escaped: 500") + 2;
		MoveTo(windowWidth - length, 17);
		NumToString(animalsEscaped, scoreString);
		DrawString("\pAnimals Escaped: ");
		DrawString(scoreString);
	}
	else
	{
		length = StringWidth("\pTime Remaining: 00:00") + 2;
		MoveTo(windowWidth - length, 17);
		DrawString("\pTime Remaining: ");
		PostTimeRemaining();
	}
}

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
		length = StringWidth("\p00:00") + 2;
		SetRect(&timeRect, windowWidth - length, 0, windowWidth, 17);
		EraseRect(&timeRect);
		MoveTo(windowWidth - length, 17);
		timeRemaining -= diffTime;
		if (timeRemaining > 0)
		{
			mins = timeRemaining / 60;
			NumToString(mins, timeString);
			DrawString(timeString);
			DrawString("\p:");
			secs = timeRemaining - mins * 60;
			if (secs < 10)
			{
				DrawString("\p0");
			}
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

PostHighScore(void)
{
	StringHandle	theString;
	int				length;
	
	if ((theString = GetString(HIGH_SCORE_STRING)) == NULL)
	{
		ErrorHandler(NO_STR);
	}
	MoveHHi((Handle) theString);
	HLock((Handle) theString);
	
	length = StringWidth("\pHigh Score: ") + 2 + StringWidth(*theString);
	MoveTo((windowWidth - length) / 2, 17);
	DrawString("\pHigh Score: ");
	DrawString(*theString);
	
	HUnlock((Handle) theString);
}

EventLoop(void)
{
	Boolean		hasGNE = WNEAvailable();
	RgnHandle	cursorRgn = NewRgn();
	Boolean		gotEvent;
	EventRecord	event;
	
	gDone = FALSE;
	
	while (!gDone)
	{
		if (hasGNE)
		{
			gotEvent = WaitNextEvent(everyEvent, &event, (long)NULL, cursorRgn);
		}
		else
		{
			SystemTask();
			gotEvent = GetNextEvent(everyEvent, &event);
		}
		
		AdjustCursor(event.where);
		
		if (gotEvent)
		{
			DoEvent(event);
		}
		else
		{
			DoIdle();
		}
	}
}

AdjustCursor(Point where)
{	
	if (FrontWindow() != gameWindow)
	{
		SetCursor(&arrow);
	}
	else
	{
		GlobalToLocal(&where);
		if (PtInRect(where, &whereAnimalsRun))
		{
			SetCursor(*GetCursor(CROSSHAIR));
		}
		else
		{
			SetCursor(&arrow);
		}
	}
}

DoEvent(EventRecord event)
{
	GrafPtr		oldPort;

	switch (event.what)
	{
		case mouseDown:
			DoMouseDown(event);
			break;
		case keyDown:
		case autoKey:
			if ((event.modifiers & cmdKey) != 0)
			{
				AdjustMenus();
				DoMenuChoice(MenuKey(event.message & charCodeMask));
			}
			break;
		case updateEvt:
			if (!IsDAWindow((WindowPtr) event.message))
			{
				GetPort(&oldPort);
				SetPort((GrafPtr) event.message);
				BeginUpdate((GrafPtr) event.message);
				GetDateTime(&lastTime);
				if (FrontWindow() == gameWindow)
				{
					DrawScoreBar();
					if ((gameOver) || (gamePaused))
					{
						DrawContent();
					}
					if (gameOver)
					{
						WriteGameOver();
					}
					DrawControls(gameWindow);
				}
				EndUpdate((GrafPtr) event.message);
				SetPort(oldPort);
			}
			break;
		default:
			break;
	}
}

DoIdle(void)
{
	if ((!gamePaused) && (!gameOver))
	{
		if ((TickCount() - lastTicks) >= TICK_DELAY)
		{
			Animate();
			lastTicks = TickCount();
		}
	}
}

DoMouseDown(EventRecord event)
{
	WindowPtr	whichWindow;
	
	switch (FindWindow(event.where, &whichWindow))
	{
		case inMenuBar:
			AdjustMenus();
			DoMenuChoice(MenuSelect(event.where));
			break;
		case inSysWindow:
			SystemClick(&event, whichWindow);
			break;
		case inDrag:
			DragWindow(whichWindow, event.where, &gDragRect);
			break;
		case inContent:
			SelectWindow(whichWindow);
			if ((!gamePaused) && (!gameOver))
			{
				HandleInContent(event.where);
			}
			if (gameOver)
			{
				DoGameOverButtons(event.where);
			}
			break;
		default:
			break;
	}
}

AdjustMenus(void)
{
	if (IsDAWindow(FrontWindow()))
	{
		EnableItem(gEditMenu, UNDO_ITEM);
		EnableItem(gEditMenu, CUT_ITEM);
		EnableItem(gEditMenu, PASTE_ITEM);
		EnableItem(gEditMenu, COPY_ITEM);
		EnableItem(gEditMenu, CLEAR_ITEM);
	}
	else
	{
		DisableItem(gEditMenu, UNDO_ITEM);
		DisableItem(gEditMenu, CUT_ITEM);
		DisableItem(gEditMenu, PASTE_ITEM);
		DisableItem(gEditMenu, COPY_ITEM);
		DisableItem(gEditMenu, CLEAR_ITEM);
	}
	
	if (gameOver)
	{
		DisableItem(gFileMenu, PAUSE_RESUME_ITEM);
		DisableItem(gFileMenu, STOP_ITEM);
	}
	else
	{
		EnableItem(gFileMenu, PAUSE_RESUME_ITEM);
		EnableItem(gFileMenu, STOP_ITEM);		
	}
	
	if (gamePaused)
	{
		SetItem(gFileMenu, PAUSE_RESUME_ITEM, "\pResume");
	}
	else
	{
		SetItem(gFileMenu, PAUSE_RESUME_ITEM, "\pPause");
	}		
}

IsDAWindow(WindowPtr whichWindow)
{
	if (whichWindow == NULL)
	{
		return (FALSE);
	}
	else
	{
		return (((WindowPeek) whichWindow)->windowKind < 0);
	}
}

DoMenuChoice(long selection)
{
	int	menu;
	int	item;
	
	if (selection != 0)
	{
		menu = HiWord(selection);
		item = LoWord(selection);
		switch (menu)
		{
			case APPLE_MENU_ID:
				DoAppleChoice(item);
				break;
			case FILE_MENU_ID:
				DoFileChoice(item);
			case EDIT_MENU_ID:
				DoEditChoice(item);
				break;
			case GAME_MENU_ID:
				DoGameChoice(item);
				break;
			default:
				break;
		}
		HiliteMenu(0);
	}
}

DoAppleChoice(int item)
{
	Str255	accName;
	GrafPtr	oldPort;
	
	switch (item)
	{
		case ABOUT_ITEM:
			AboutDialog();
			break;
		default:
			GetItem(gAppleMenu, item, accName);
			GetPort(&oldPort);
			(void) OpenDeskAcc(accName);
			SetPort(oldPort);
			break;
	}
}

DoFileChoice(int item)
{
	switch (item)
	{
		case NEW_GAME_ITEM:
			SetUpGame();
			break;
		case PAUSE_RESUME_ITEM:
			if (!gamePaused)
			{
				gamePaused = TRUE;
			}
			else
			{
				gamePaused = FALSE;		
				GetDateTime(&lastTime);
			}		
			break;
		case STOP_ITEM:
			EndGame();
			break;
		case QUIT_ITEM:
			gDone = TRUE;
			break;
		default:
			break;
	}
}

DoEditChoice(int item)
{
	SystemEdit(item - 1);
}

DoGameChoice(int item)
{
	switch (item)
	{
		case GAME_OPTIONS_ITEM:
			GameOptionsDialog();
			break;
		case ANIMAL_POINT_VALUES_ITEM:
			AnimalPointValuesDialog();
			break;
		case HALL_OF_FAME_ITEM:
			HallOfFameDialog();
			break;
		default:
			break;
	}
}

HandleInContent(Point where)
{
	register int	i;
	int				h, v;
	RgnHandle		animalRgn;
	
	GlobalToLocal(&where);
	
	for (i = 0; i < animalRows; i++)
	{
		if (PtInRect(where, &row[i].rect))
		{
			if (!easyShoot)
			{
				animalRgn = (RgnHandle) GetResource('RGN ', animal[row[i].animal].region[row[i].step]);
				
				h = row[i].rect.right - (**animalRgn).rgnBBox.right;
				v = row[i].rect.bottom - (**animalRgn).rgnBBox.bottom;
			
				OffsetRgn(animalRgn, h, v);
				
				if (PtInRgn(where, animalRgn)) 
				{
					PaintRgn(animalRgn);
					DoSound(BASE_RES_ID + row[i].animal, TRUE);
					AnimalShot(i);
				}
			}	
			else
			{
				DoSound(BASE_RES_ID + row[i].animal, TRUE);
				AnimalShot(i);
			}	
		}
	}
}

DoGameOverButtons(Point where)
{
	ControlHandle	whichControl;
	
	GlobalToLocal(&where);
	
	(void) FindControl(where, gameWindow, &whichControl);
	
	if (whichControl != NULL)
	{
		if (TrackControl(whichControl, where, NULL) != 0)
		{
			if (whichControl == playAgainControl)
			{
				SetUpGame();
			}
			if (whichControl == quitControl)
			{
				ExitToShell();
			}
		}
	}
}

AnimalShot(int i)
{
	Str255	scoreString;
	int		length;
	Rect	scoreRect;
	
	gameScore += animal[row[i].animal].value;
	length = StringWidth("\pScore: ") + 2;
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
   	{
		speedIncrease = gameScore / 1000;
	}
}

ErrorHandler(int stringNum)
{
	StringHandle	errorStringH;
	
	if ((errorStringH = GetString(stringNum)) == NULL)
	{
		ParamText(HOPELESSLY_FATAL_ERROR, NULL, NULL, NULL);
	}
	else
	{
		MoveHHi((Handle) errorStringH);
		HLock((Handle) errorStringH);
		ParamText(*errorStringH, NULL, NULL, NULL);
		HUnlock((Handle) errorStringH);
	}
	StopAlert(ERROR_ALERT_ID, NULL);
	ExitToShell();
}

Animate(void)
{
	int		r;
	Rect	copyRect;
	GrafPtr	secondOffScreen;
	
	/* Open loop goes from 0 to animalRows - 1 & allows for WaitNextEvent to occur
		between each animal drawing */
	
	loopValue %= animalRows;
	
	if (!row[loopValue].running)
	{
		row[loopValue].running = TRUE;
		r = Randomize(9);
		row[loopValue].animal = r;
		row[loopValue].speed = animal[r].speed;
		row[loopValue].step = 0;
	}
	
	row[loopValue].step = (row[loopValue].step == 1) ? 0 : 1;
	
	OffsetRect(&row[loopValue].rect,
		row[loopValue].speed * speedFactor + speedIncrease, 0);

	SetRect(&copyRect,
		row[loopValue].rect.left - (row[loopValue].speed * speedFactor + speedIncrease),
		row[loopValue].rect.top,
		row[loopValue].rect.right,
		row[loopValue].rect.bottom);

	if (!CreateOffscreenBitMap(&secondOffScreen, &copyRect)) 
	{
		SysBeep(1);
		ExitToShell();
 	}
  	SetPort(secondOffScreen);
   	EraseRect(&copyRect);
   		
   	CopyBits(&offscreen->portBits, &secondOffScreen->portBits, 
   		&animal[row[loopValue].animal].rect[row[loopValue].step], &row[loopValue].rect, srcCopy, NULL);
   	SetPort(gameWindow);
   	CopyBits(&secondOffScreen->portBits, &gameWindow->portBits,
   		&copyRect, &copyRect, srcCopy, NULL);
   	DestroyOffscreenBitMap(secondOffScreen);

   	if (row[loopValue].rect.left >= windowWidth)
   	{
		row[loopValue].rect.left = 0;
		row[loopValue].rect.right = 33;
   		row[loopValue].running = FALSE;
   		if (escapedGame)
   		{
 			AnimalEscaped();
 		}
   	}

	loopValue++;
	if (!escapedGame)
	{
		PostTimeRemaining();
	}
}

DrawContent(void)
{
	GrafPtr			oldPort;
	register int	i;
	
	GetPort(&oldPort);
	SetPort(gameWindow);

	for (i = 0; i < animalRows; i++)
	{
   		CopyBits(&offscreen->portBits, &gameWindow->portBits,
 			&animal[row[i].animal].rect[row[i].step], &row[i].rect,srcCopy, NULL);
 	}
	SetPort(oldPort);
}

Randomize(int range)
{
 	long	rawResult = Random();
 	
 	if (rawResult < 0)
 	{
 		rawResult *= -1;
 	}
 	return (rawResult % range);
}
 
AnimalEscaped(void)
{
	int		length = StringWidth("\p000") + 2;
	Rect	escapedRect;
 	Str255	scoreString;
		
	animalsEscaped++;
	SetRect(&escapedRect, windowWidth - length, 0, windowWidth, 20);
	MoveTo(windowWidth - length, 17);
	NumToString(animalsEscaped, scoreString);
	EraseRect(&escapedRect);
	DrawString(scoreString);
	if ((escapedGame) && (maxEscaped <= animalsEscaped))
	{
		EndGame();
	}
}


WriteGameOver(void)
{
	int		length = StringWidth("\pGame Over");
	Rect	myRect;
	
	SetRect(&myRect, windowWidth / 2 - length / 2 - 2, windowHeight / 2 - 10, 
			windowWidth / 2 + length / 2 + 2, windowHeight / 2 + 10);
	EraseRect(&myRect);
	MoveTo(windowWidth / 2 - length / 2, windowHeight / 2);
	DrawString("\pGame Over");
}

EndOfGameControls(void)
{
	ShowControl(playAgainControl);
	ShowControl(quitControl);
}

EndGame(void)
{
	gameOver = TRUE;
	WriteGameOver();
	EndOfGameControls();
	if ((gameScore >= topScores[2]) && (showHighScores))
	{
		UpdateHighScores();
	}
}

UpdateHighScores(void)
{
	register int	i;
	StringHandle	theString, topNames[3];
	Str255			numString;
	
	GetPlayerName();
	
	for (i = 0; i <= 2; i++)
	{
		if ((theString = GetString(HIGH_NAMES_STRING + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		topNames[i] = theString;
		HUnlock((Handle) theString);	
	}
		
	if (gameScore >= topScores[0])
	{
		topScores[2] = topScores[1];
		SetString(topNames[2], *topNames[1]);
		topScores[1] = topScores[0];
		SetString(topNames[1], *topNames[0]);
		topScores[0] = gameScore;
		SetString(topNames[0], name);
	}
	else if (gameScore >= topScores[1])
	{
		topScores[2] = topScores[1];
		SetString(topNames[2], *topNames[1]);
		topScores[1] = gameScore;
		SetString(topNames[1], name);
	}
	else
	{
		topScores[2] = gameScore;
		SetString(topNames[2], name);
	}
	
	for (i = 0; i <= 2; i++)
	{
		if ((theString = GetString(HIGH_NAMES_STRING + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);
		SetString(theString, *topNames[i]);
		ChangedResource((Handle) theString);
		WriteResource((Handle) theString);
		HUnlock((Handle) theString);
		
		if ((theString = GetString(HIGH_SCORE_STRING + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		NumToString(topScores[i], numString);
		SetString(theString, numString);
		ChangedResource((Handle) theString);
		WriteResource((Handle) theString);
	}
	
	HallOfFameDialog();
}

GetPlayerName(void)
{
	GrafPtr 		savePort;
	int				stringNum = 502 - (gameScore >= topScores[1]) - (gameScore >= topScores[0]);
	StringHandle	whichString;
	DialogPtr		theDialog;
	int				itemHit;
	int				itemType;
	Handle			itemHandle;
	Rect			itemRect;
	Boolean			dialogDone = FALSE;
	
	GetPort(&savePort);

	if ((whichString = GetString(stringNum)) == NULL)
	{
		ParamText(HOPELESSLY_FATAL_ERROR, NULL, NULL, NULL);
	}
	MoveHHi((Handle) whichString);
	HLock((Handle) whichString);
	ParamText(*whichString, NULL, NULL, NULL);
	HUnlock((Handle) whichString);
	if ((theDialog = GetNewDialog(PLAYER_NAME_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	SelIText(theDialog, PLAYER_NAME_ITEXT, 0, 255);
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	SetPort(theDialog);
	
	SetCursor(&arrow);
	
	DoSound(TARZAN_SOUND, TRUE);
	
	while (!dialogDone)
	{
		ModalDialog(NULL, &itemHit);
		switch (itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
			default:
				break;
		}
	}
	
	GetDItem(theDialog, PLAYER_NAME_ITEXT, &itemType, &itemHandle, &itemRect);
	MoveHHi((Handle) itemHandle);
	HLock(itemHandle);
	GetIText(itemHandle, name);
	HUnlock(itemHandle);
	
	DisposDialog(theDialog);
	SetPort(savePort);
}

AnimalPointValuesDialog(void)
{	
	GrafPtr		savePort;
	DialogPtr	theDialog;
	Boolean		dialogDone = FALSE;
	int			itemHit;

	GetPort(&savePort);
	
	if ((theDialog = GetNewDialog(ANIMAL_POINT_VALUES_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	SetPort(theDialog);
	
	while (!dialogDone)
	{
		ModalDialog(NULL, &itemHit);
		switch (itemHit - 1)
		{
			case RHINO:
			case ELEPHANT:
			case ZEBRA:
			case GORILLA:
			case ANTELOPE:
			case GIRAFFE:
			case LION:
			case CHEETAH:
			case WARTHOG:	
				DoSound(BASE_RES_ID + itemHit - 1, TRUE);
				break;
			case OK_APV_DBOX:
				dialogDone = TRUE;
				break;
			default:
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

AboutDialog(void)
{	
	GrafPtr 	savePort;
	DialogPtr	theDialog;
	int			itemType;
	Handle		itemHandle;
	Rect		itemRect;
	Boolean		dialogDone = FALSE;
	int			itemHit;
	
	#define ICON_ITEM	5

	GetPort(&savePort);
	
	if ((theDialog = GetNewDialog(ABOUT_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	SetPort(theDialog);
	
	GetDItem(theDialog, ICON_ITEM, &itemType, &itemHandle, &itemRect);
	PlotIcon(&itemRect, GetIcon(MY_HEAD));
	
	while (!dialogDone)
	{
		ModalDialog(NULL, &itemHit);
		switch (itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
			case SHOW_INTRO_SCREEN:
				IntroDialog();
				break;
			case HEAD_ICON:
				PlotIcon(&itemRect, GetIcon(OUCH_HEAD));
				DoSound(OUCH_SOUND, FALSE);
				PlotIcon(&itemRect, GetIcon(MY_HEAD));
				break;
			default:
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

HallOfFameDialog(void)
{	
	GrafPtr 		savePort;
	DialogPtr		theDialog;
	register int	i;
	StringHandle	theString;
	int				itemType;
	Handle			itemHandle;
	Rect			itemRect;
	Boolean 		dialogDone = FALSE;
	int				itemHit;
	
	GetPort(&savePort);
	
	if ((theDialog = GetNewDialog(HALL_OF_FAME_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	for (i = 0; i <= 5; i++)
	{
		if ((theString = GetString(128 + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		GetDItem(theDialog, i + 3, &itemType, &itemHandle, &itemRect);
		SetIText(itemHandle, *theString);
		HUnlock((Handle) theString);	
	}
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	SetPort(theDialog);
	
	while (!dialogDone)
	{
		ModalDialog(NULL, &itemHit);
		switch (itemHit)
		{
			case OK_BUTTON:
				dialogDone = TRUE;
				break;
			case CLEAR_SCORES_BUTTON:
				dialogDone = TRUE;
				ClearHighScores();
				break;
			default:
				break;
		}
	}
	DisposDialog(theDialog);
	SetPort(savePort);
}

ClearHighScores(void)
{
	register int	i;
	StringHandle	theString;
	
	for (i = 0; i <= 2; i++)
	{
		if ((theString = GetString(HIGH_SCORE_STRING + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		HNoPurge((Handle) theString);
		SetString(theString, "\p0"); 
		ChangedResource((Handle) theString);
		WriteResource((Handle) theString);
		HPurge((Handle) theString);
		
		if ((theString = GetString(HIGH_NAMES_STRING + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		HNoPurge((Handle) theString);
		SetString(theString, "\pPlayer Name"); 
		ChangedResource((Handle) theString);
		WriteResource((Handle) theString);
		HPurge((Handle) theString);
		topScores[i] = 0;
	}
}

Boolean CheckITextValues(DialogPtr theDialog)
{
	register int	i;
	int				itemType;
	Handle			itemHandle;
	Rect			itemRect;
	long			stringValue;
	Boolean			dialogDone = TRUE;
	
	for (i = MINUTES; i <= MAX_ESCAPED; i++)
	{
		GetDItem(theDialog, i, &itemType, &itemHandle, &itemRect);
		MoveHHi((Handle) itemHandle);
		HLock(itemHandle);
		GetIText(itemHandle, preferenceString[i]);
		HUnlock(itemHandle);
	}

	StringToNum(preferenceString[MINUTES], &stringValue);
	if ((stringValue < 0) || (stringValue > 99))
	{
		BadValue(MINUTES_ERROR);
		SelIText(theDialog, MINUTES, 0, 255);
		dialogDone = FALSE;
	}
	
	StringToNum(preferenceString[SECONDS], &stringValue);
	if ((stringValue < 0) || (stringValue > 59))
	{
		BadValue(SECONDS_ERROR);
		SelIText(theDialog, SECONDS, 0, 255);
		dialogDone = FALSE;
	}
	
	StringToNum(preferenceString[MAX_ESCAPED], &stringValue);
	if ((stringValue < 1) || (stringValue > 99))
	{
		BadValue(ESCAPED_ERROR);
		SelIText(theDialog, MAX_ESCAPED, 0, 255);
		dialogDone = FALSE;
	}
	
	return (dialogDone);
}

CheckHighScoreDisplay(void)
{
	register int	i;
	StringHandle	theString;
	long			savedValues[8], preferenceValue[8];
	
	showHighScores = TRUE;
	
	for (i = 0; i <= 7; i++)
	{
		if ((theString = GetString(200 + i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		StringToNum(*theString , &savedValues[i]);	
		HUnlock((Handle) theString);
		StringToNum(preferenceString[i], &preferenceValue[i]);
	}
	
	for (i = 0; i <= 4; i++)
	{
		if (savedValues[i] != preferenceValue[i])
		{
			showHighScores = FALSE;
		}
	}

	if (showHighScores)
	{
		switch (savedValues[TIMED_OR_ESCAPED - 200])
		{
			case TIMED_RADIO:
				if (savedValues[MINUTES_STRING - 200] != preferenceValue[MINUTES_STRING - 200])
				{
					showHighScores = FALSE;
				}
				if (savedValues[SECONDS_STRING - 200] != preferenceValue[SECONDS_STRING - 200])
				{
					showHighScores = FALSE;
				}
				break;
			case ESCAPED_RADIO:
				if (savedValues[MAX_ESCAPED_STRING - 200] != preferenceValue[MAX_ESCAPED_STRING - 200])
				{
					showHighScores = FALSE;
				}
				break;
			default:
				break;
		}	
	}
	
	if (showHighScores)
	{
		GetHighScores();
	}
}

GameOptionsDialog(void)
{
	GrafPtr		savePort;
	DialogPtr	theDialog;
	Boolean		dialogDone = FALSE;
	int			itemHit;
	int			itemType;
	Handle		itemHandle;
	Rect		itemRect;

	GetPort(&savePort);

	if ((theDialog = GetNewDialog(GAME_OPTIONS_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	DialogInit(theDialog);
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	while (!dialogDone)
	{
		ModalDialog((ProcPtr) &GameOptionsDProc, &itemHit);
		switch (itemHit)
		{
			case OK_BUTTON:
				dialogDone = CheckITextValues(theDialog);
				if (dialogDone)
				{
					showHighScores = FALSE;
					CheckHighScoreDisplay();
					SetUpGame();
				}
				break;
			case SAVE_AND_CLEAR_BUTTON:
				dialogDone = CheckITextValues(theDialog);
				if (dialogDone)
				{
					UpdatePreferences(theDialog);	
					ClearHighScores();
					showHighScores = TRUE;
					SetUpGame();
				}
				break;
			case CANCEL_BUTTON:
				dialogDone = TRUE;
				break;
			case TIMED_RADIO:
			case ESCAPED_RADIO:
				GetDItem(theDialog, TIMED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == TIMED_RADIO));
				GetDItem(theDialog, ESCAPED_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == ESCAPED_RADIO));
				NumToString(itemHit, preferenceString[TIMED_OR_ESCAPED - 200]);
				break;
			case SPEED_POP_UP:
				GetDItem(theDialog, SPEED_POP_UP, &itemType, &itemHandle, &itemRect);
				itemHit = GetCtlValue((ControlHandle) itemHandle);
				NumToString(itemHit + 24, preferenceString[ANIMAL_SPEED - 200]);
				break;
			case NINE_INCH_RADIO:
			case CURRENT_SCREEN_RADIO:
				GetDItem(theDialog, NINE_INCH_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == NINE_INCH_RADIO));
				GetDItem(theDialog, CURRENT_SCREEN_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == CURRENT_SCREEN_RADIO));
				NumToString(itemHit, preferenceString[WINDOW_SIZE - 200]);
				break;
			case INCREASE_YES_RADIO:
			case INCREASE_NO_RADIO:
				GetDItem(theDialog, INCREASE_YES_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == INCREASE_YES_RADIO));
				GetDItem(theDialog, INCREASE_NO_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == INCREASE_NO_RADIO));
				NumToString(itemHit, preferenceString[INCREASE_SPEED - 200]);
				break;
			case IN_ANIMAL_OUTLINE_RADIO:
			case IN_BOX_SURROUNDING_ANIMAL_RADIO:
				GetDItem(theDialog, IN_ANIMAL_OUTLINE_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == IN_ANIMAL_OUTLINE_RADIO));
				GetDItem(theDialog, IN_BOX_SURROUNDING_ANIMAL_RADIO, &itemType, &itemHandle, &itemRect);
				SetCtlValue((ControlHandle) itemHandle, (itemHit == IN_BOX_SURROUNDING_ANIMAL_RADIO));
				NumToString(itemHit, preferenceString[SCORE_WHEN_HIT - 200]);
				break;
			default:
				break;
		}
	}
	
	DisposDialog(theDialog);
	SetPort(savePort);
}

pascal Boolean GameOptionsDProc(DialogPtr theDialog, EventRecord *theEvent, int *itemHit)
{
	int			itemType;
	Handle		itemHandle;
	Rect		itemRect;

	switch (theEvent->what)
	{
		case updateEvt:
			if (IsDialogEvent(theEvent))
			{
				SetPort(theDialog);
				GetDItem(theDialog, 25, &itemType, &itemHandle, &itemRect);
				FrameRect(&itemRect);
				GetDItem(theDialog, 26, &itemType, &itemHandle, &itemRect);
				FrameRect(&itemRect);
				GetDItem(theDialog, 27, &itemType, &itemHandle, &itemRect);
				FrameRect(&itemRect);
			}
			return (FALSE);
			break;
		default:
			return (FALSE);
			break;
	}
}

UpdatePreferences(DialogPtr theDialog)
{
	register int	i;
	StringHandle	theString;
	
	for (i = 200; i <= 214; i += (i == 204) ? 8 : 1)
	{
		if ((theString = GetString(i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		HNoPurge((Handle) theString);
		SetString(theString, preferenceString[i - 200]); 
		ChangedResource((Handle) theString);
		WriteResource((Handle) theString);
		HPurge((Handle) theString);
	}
}

DialogInit(DialogPtr theDialog)
{
	register int	i;
	StringHandle	theString;
	long			itemNumber;
	int				itemType;
	Handle			itemHandle;
	Rect			itemRect;

	for (i = TIMED_OR_ESCAPED; i <= SCORE_WHEN_HIT; i++)
	{
		if ((theString = GetString(i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		StringToNum(*theString, &itemNumber);
		if ((itemNumber == SLOW_RADIO) || (itemNumber == MEDIUM_RADIO) || 
			(itemNumber == FAST_RADIO))
		{
			GetDItem(theDialog, SPEED_POP_UP, &itemType, &itemHandle, &itemRect);
			SetCtlValue((ControlHandle) itemHandle, itemNumber - 24);
		}
		else
		{
			GetDItem(theDialog, itemNumber, &itemType, &itemHandle, &itemRect);
			SetCtlValue((ControlHandle) itemHandle, TRUE);
		}
		NumToString(itemNumber, preferenceString[i - 200]);
		HUnlock((Handle) theString);
	}

	for (i = MINUTES_STRING; i <= MAX_ESCAPED_STRING; i++)
	{
		if ((theString = GetString(i)) == NULL)
		{
			ErrorHandler(NO_STR);
		}
		MoveHHi((Handle) theString);
		HLock((Handle) theString);	
		GetDItem(theDialog, i - 200, &itemType, &itemHandle, &itemRect);
		SetIText(itemHandle, *theString);
/*
		item number is just a place holder in the next two lines (it's significant
		as the item number in all other code) I couldn't get the string handle
		to equate to the string, so this is my work around. I make the string
		handle a number, then make the number a string.  Ugly but functional.
*/
		StringToNum(*theString, &itemNumber);
		NumToString(itemNumber, preferenceString[i - 200]);
		HUnlock((Handle) theString);
	}
}

BadValue(int stringNum)
{
	StringHandle	errorStringH;
	
	if ((errorStringH = GetString(stringNum)) == NULL)
	{
		ParamText(HOPELESSLY_FATAL_ERROR, NULL, NULL, NULL);
	}
	else
	{
		MoveHHi((Handle) errorStringH);
		HLock((Handle) errorStringH);
		ParamText(*errorStringH, NULL, NULL, NULL);
		HUnlock((Handle) errorStringH);
	}
	StopAlert(ERROR_EDIT_TEXT, NULL);
}

IntroDialog(void)
{
	GrafPtr		savePort;
	DialogPtr	theDialog;
	Rect		dialogRect, clippingRect;
	GrafPtr		dLogOffscreen;
	int			itemHit, i, beginTicks, currentTicks, h;

	GetPort(&savePort);

	if ((theDialog = GetNewDialog(INTRO_DIALOG, NULL, M_T_F)) == NULL)
	{
		ErrorHandler(NO_WIND);
	}
	CenterWindow(theDialog);
	ShowWindow(theDialog);
	
	SetPort(theDialog);

	dialogRect = theDialog->portRect;
	if (!CreateOffscreenBitMap(&dLogOffscreen, &dialogRect)) 
	{
		SysBeep(1);
		ExitToShell();
    }
    
	SetPort(dLogOffscreen);
	
	DrawPicture(GetPicture(475), &dialogRect);
  
	SetPort(theDialog);
	
	DoSound(INTRO_SOUND, TRUE);
	
	h = (dialogRect.bottom - dialogRect.top) / 2;
	
	SetRect(&clippingRect, dialogRect.left, h, dialogRect.right, h);
	for (i = 1; i <= h + 1; i += 4)
	{
		ClipRect(&clippingRect);
		clippingRect.top -= 4;
		clippingRect.bottom += 4;
		CopyBits(&dLogOffscreen->portBits, &theDialog->portBits,
			&dialogRect, &dialogRect, srcCopy, NULL);			
		beginTicks = TickCount();
		do
		{
			currentTicks = TickCount();
		}
		while ((currentTicks - beginTicks) <= 2);
	}
	
	while (!Button());
	
	DisposDialog(theDialog);
	SetPort(savePort);
}
