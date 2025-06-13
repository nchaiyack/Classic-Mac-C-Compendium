/* Cell Proj 1.0 */

#include "Cell_Proto.h"
#include "Cell_Definitions.h"
#include "Cell_Variables.h"

void ToolBoxInit(void)
{
	unsigned long mySeed;

	InitGraf( &qd.thePort );
	GetDateTime(&mySeed);
	LMSetRndSeed(mySeed);	// doesn't seem to much be working
	InitFonts();
	FlushEvents( everyEvent, REMOVE_ALL_EVENTS );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( nil );
	InitCursor();
}

static void initGWorlds(void) {
	if (NewGWorld(&gLifeGWorld,0,&(gCellWindow->portRect),nil,nil,0)) {
		DebugStr("\pCouldn't Allocate GWorld");
		ExitToShell();
	}
}

static Point centerRect(Rect *r)
{
	Rect	screenRect = qd.screenBits.bounds;
	short	screenHeight = screenRect.bottom - screenRect.top;
	short	screenWidth = screenRect.right - screenRect.left;
	short	rHeight = r->bottom - r->top;
	short	rWidth = r->right - r->left;
	Point	offsetValue;

	/* calculate where the center SHOULD be */
	offsetValue.h = (screenWidth - rWidth) / 2;
	offsetValue.v = (screenHeight - rHeight) / 2;

	/* offset it by the initial rect position, so it's a good offset */
	offsetValue.h -= r->left;
	offsetValue.v -= r->top;
	
	return(offsetValue);
}

void WindowInit(void)
{
	WStateDataHandle theStates;
	Rect	stdSize = { 0, 0, CELL_HEIGHT + 10, CELL_WIDTH + 10 };
	Rect	bigSize = { 0, 0, 2 * (CELL_HEIGHT + 10), 2 * (CELL_WIDTH + 10) };
	Point	centerPoint;

	gCellWindow = GetNewWindow( BASE_RES_ID, nil,(WindowPtr) MOVE_TO_FRONT );

	SizeWindow(gCellWindow,CELL_WIDTH + 10, CELL_HEIGHT + 10, true);

	theStates = (WStateDataHandle)(((WindowPeek)gCellWindow)->dataHandle);

	centerPoint = centerRect(&stdSize);
	OffsetRect(&stdSize,centerPoint.h,centerPoint.v);

	centerPoint = centerRect(&bigSize);
	OffsetRect(&bigSize,centerPoint.h,centerPoint.v);

	(**theStates).userState = stdSize;
	(**theStates).stdState = bigSize;

	initGWorlds();

	SetPort( gCellWindow );
}

void MenuBarInit(void)
{
	Handle		myMenuBar;
	
	myMenuBar = GetNewMBar( BASE_RES_ID );
	SetMenuBar( myMenuBar );
	gAppleMenu = GetMHandle( APPLE_MENU_ID );
	
	AddResMenu( gAppleMenu, 'DRVR' );
	DrawMenuBar();
}

void DisplayCellWindow(void)
{
	ShowWindow( gCellWindow );
	DrawControls( gCellWindow );
}

void PlaceRandomCells(void)
{
	long	randomLiveCell;
	long	count;

	memset(gCellStatus,0,NUMBER_OF_CELLS);

	startTime = TickCount();
	generations = 0;

	for ( count = 0; count < NUMBER_OF_CELLS/6 ; count++ )
	{
		randomLiveCell = Randomize(NUMBER_OF_CELLS-1);
		gCellStatus[randomLiveCell] = 1;
	}
	
}	

long Randomize(long range )
{
	float	rawResult;
	
	rawResult = Random();
	if (rawResult < 0)
		rawResult *= -1;
	return(((rawResult * range) / 32768) + 1);
}
