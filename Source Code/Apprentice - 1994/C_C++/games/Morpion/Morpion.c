/*
	Morpion (French) == 5 In A Row (English)
	The first one who puts 5 markers in a row (horizontal, vertical or diagonal) wins. The computer plays against itself.
	
	This small fader is intended as a code example. It is based on FaderShell, written by Tom Dowdy.
	This code is hereby placed into the public domain. Use it as a template to write your faders!
	
	In FaderShell, the fader created its own Quickdraw globals and copied the application's globals into them, then copied them
	back into the application's. This seemed useless, so I skipped it : this fader uses directly DarkSide's Quickdraw globals.
	
	You can use global variables, they are stored as an offset from A4 using THINK's SetupA4/RestoreA4 procedures.
	
	This fader also demonstrates the use of callback routines by playing sound. Note that in order to play sound, you must
	request a sound channel from DarkSide by creating a 'Chnl' 0 resource. 
*/

#include <Memory.h>
#include <Windows.h>
#include <Dialogs.h>
#include <Errors.h>

#include "Fader.h"								// include DarkSide's interface

typedef struct  {								// This will allow us to use DarkSide's Quickdraw globals.
	char 		privates[76];						// QD is initialized by PreflightFader. Afterwards we access
	long 		randSeed;							// QuickDraw globals with QD->randSeed, QD->white, etc.
	BitMap 	screenBits;
	Cursor 	arrow;
	Pattern 	dkGray;
	Pattern 	ltGray;
	Pattern 	gray;
	Pattern 	black;
	Pattern 	white;
	GrafPtr 	thePort;
} *QDGlobalsPtr;

QDGlobalsPtr	QD;

// Constants. These are specific to Morpion, you can throw them away if you are writing a new fader.

#define	border		2

#define	empty		0
#define	player1		1
#define	player2		2

// Global variables, referenced using A4. Also Morpion-specific.

short 		xsize, ysize;						// Width and height of grid
short		xc, yc;							// Pixel coordinates of topleft corner
Handle		grid, points, possible;	
short		value[5][5];
short		turn;								// Tells who's up
Boolean		draw, winner;						// End-of-game flags
short		unit;								// Size of squares

// grid is considered as a two dimensional array 0..xsize-1, 0..ysize-1 of chars
// points as an array 0..xsize-1, 0..ysize-1 of shorts
// possible as an array 0..xsize*ysize-1 of points

#define	Grid(x, y)		((char*) (*grid)) [xsize * y + x]
#define	Points(x, y)	((short*) (*points)) [xsize * y + x]
#define	Possible(x)	((Point*) (*possible)) [x]

void StartGame (void)
{
	long		ix;
	char 		*p;

	turn = player1;												// white plays first
	draw = winner = false;										
	for (p = *grid, ix = xsize*ysize; ix; p++, ix--) *p = empty;			// clear the grid
}

// Called when fader is starting up, before window has been created.
// We initialize our global variables here.

OSErr	PreflightFader(MachineInfoPtr machineInfo, long *minTicks, long *maxTicks)
{	
	Rect		bounds;
	long		gridsize;
	short	a, b;
	
	*maxTicks = *minTicks = 1;									// Tell DarkSide how often we want to be called.
	QD = (QDGlobalsPtr) machineInfo->applicationQD;					// Use DarkSide's QuickDraw globals
	
	// The rest is specific to Morpion.
	
	unit = machineInfo->faderSettings->theShorts[1];					// read the settings
	bounds = machineInfo->theScreens[0].bounds;						// compute the size of our grid
	xsize = (bounds.right - bounds.left - 6 * unit) / unit;
	ysize = (bounds.bottom - bounds.top - 6 * unit) / unit;
	xc = bounds.left + (bounds.right - bounds.left - unit * xsize) / 2;
	yc = bounds.top + (bounds.bottom - bounds.top - unit * ysize) / 2;
	
	if (!(grid = BestNewHandle (gridsize = xsize * ysize)))				// allocate memory for the grid
		return memFullErr;
	if (!(possible = BestNewHandle (4*gridsize)))						// allocate memory for the 'possible' array
		return memFullErr;
	if (!(points = BestNewHandle (2*gridsize)))						// allocate memory for the 'points' array
		return memFullErr;
		
	for (a = 0; a < 5; a++)										// initialize the 'value' array
		for (b = 0; b < 5; b++)
			value [a][b] = 0;
	value [0][0] = 10; value [0][1] = 20; value [0][2] = 80; value [0][3] = 300; value [0][4] = 2000;
	value [1][0] = 20; value [2][0] = 70; value [3][0] = 400; value [4][0] = 4000;
	
	StartGame();

	return noErr;
}

// Called when fader is starting up, after window has been created. We usually erase the screens here.
// We then draw the grid.

OSErr	InitializeFader(MachineInfoPtr machineInfo)
{
	short		screenIndex;
	short		ix;

	PenPat(QD->black);											// erase the screens.
	for (screenIndex = 0; screenIndex < machineInfo->numScreens; screenIndex++) 
		PaintRect(&machineInfo->theScreens[screenIndex].bounds);
		
	PenPat(QD->white);											// draw the grid.
	for (ix = 0; ix <= xsize; ix++) {
		MoveTo (xc + ix * unit, yc);
		Line (0, unit * ysize);
	}
	for (ix = 0; ix <= ysize; ix++) {
		MoveTo (xc, yc + ix * unit);
		Line (unit * xsize, 0);
	}
	
	return noErr;
}

// The DoRow and Play routines are Morpion's IA. They are not part of the fader's interface with DarkSide.

void DoRow (short bx, short by, short dx, short dy)
{
	short 	i;
	short 	x, y;
	short 	nfriend, nenemy;
	char	g;
	
	for (nfriend = nenemy = 0, x = bx, y = by, i = 5; i; x += dx, y += dy, i--) {
		if ((g = Grid(x, y)) == turn)								// count how many enemy/friendly markers
			nfriend++;										// are present in those five squares
		else if (g != empty)
			nenemy++;
	}
	
	for (x = bx, y = by, i = 5; i; x += dx, y += dy, i--)					// increment the strategic value of the free squares
		if (Grid(x, y) == empty)									// depending on nenemy and nfriendly
			Points(x, y) += value [nfriend][nenemy];
}

Point Play (void)
{
	short	ix, iy, npos, max, n, p;
	
	for (ix = 0; ix < xsize; ix++)									// Only allow empty spots to be played
		for (iy = 0; iy < ysize; iy++)
			Points(ix, iy) = Grid(ix, iy) == empty ? 0 : -1;
				
	for (ix = 0; ix < xsize-4; ix++)									// Walk the whole grid, giving more points to each
		for (iy = 0; iy < ysize; iy++)								// interesting spot
			DoRow (ix, iy, 1, 0);
	for (ix = 0; ix < xsize; ix++)
		for (iy = 0; iy < ysize-4; iy++)
			DoRow (ix, iy, 0, 1);
	for (ix = 0; ix < xsize-4; ix++)
		for (iy = 0; iy < ysize-4; iy++)
			DoRow (ix, iy, 1, 1);
	for (ix = 0; ix < xsize-4; ix++)
		for (iy = 4; iy < ysize; iy++)
			DoRow (ix, iy, 1, -1);

	for (npos = 0, max = -1, ix = 0; ix < xsize; ix++)					// Look for the most interesting spot
		for (iy = 0; iy < ysize; iy++) {
			p = Points(ix, iy);
			if (p > max) {										// If this square is more interesting than all those
				max = p; npos = 1;								// previously visited, forget about them
				Possible (0).h = ix;
				Possible (0).v = iy;
			}
			else if (p == max) {									// If it is equally interesting than the best previously
				npos++;										// visited squares, add it to the array of possible
				Possible (npos-1).h = ix;							// moves.
				Possible (npos-1).v = iy;
			}
		}
	
	if (max == 0)												// Nothing interesting : this is a draw
		draw = true;
	else if (npos) {												
		n = Random();											// Randomly choose a square from the list
		if (n < 0) n = -n;										// of best possible squares
		n = n % npos;

		if (max >= value [4][0])									// This means we just won
			winner = true;

		return Possible (n);										// Pick up a solution randomly from the set of
	}														// the best solutions
	else
		draw = true;											// The grid is full
}

// Called regularly. This is the heart of the fader. Here we do whatever we please...

OSErr	IdleFader(MachineInfoPtr machineInfo)
{
	Point			location;
	Rect			r;
	long			finalTicks;
	short		a;
	
	location = Play();											// find out where we want to play

	if (!draw) {												// If the player actually played something
		r.left = xc + border + unit * location.h;
		r.top = yc + border + unit * location.v;
		r.right = r.left + unit - 2 * border + 1;
		r.bottom = r.top + unit - 2 * border + 1;
		if (turn == player1)
			EraseOval (&r);									// draw the new marker in the proper color
		else {
			PenPat (QD->white);
			FrameOval (&r);
			PenPat (QD->black);
		}
		(void) PlayResourceSnd (machineInfo, 128, true);				// play sound asynchronously
		for (a = 4; a; a--) {										// make it blink so that the user sees it
			InvertRect (&r);
			Delay (2, &finalTicks);
		}
		
		Delay (60 - machineInfo->faderSettings->theShorts[0], &finalTicks);	// wait a little moment
	}
	
	if (draw || winner) {											// if the game's over
		PenPat (QD->black);
		TextMode (srcXor);
		r = machineInfo->theScreens[0].bounds;
		MoveTo (r.left+2*unit, r.top + 2*unit);						// print a message explaining why
		if (draw)
			DrawString ("\p5 in a row is now impossible. This is a draw!");
		else 
			DrawString (turn == player1 ? "\pWhite just won!" : "\pBlack just won!");
		PenNormal();
		
		Delay (240, &finalTicks);									// wait a few seconds
		StartGame();											// and start a new game
		InitializeFader(machineInfo);
		return noErr;
	}
	

	Grid(location.h, location.v) = turn;								// update the grid in memory
	turn = (turn == player1) ? player2 : player1;						// change turn
		
	return noErr;
}

// Called when the fade is tearing down

OSErr	DisposeFader(MachineInfoPtr machineInfo)
{
	DisposHandle (grid);											// release all the memory we reserved
	DisposHandle (points);
	DisposHandle (possible);
	return noErr;
}

// Called when there is an update event for our fade window.

OSErr	UpdateFader(MachineInfoPtr machineInfo)
{
	InitializeFader(machineInfo);									// erase the screen, draw the grid
	return noErr;
}

// Called when there is an events in the settings dialog. itemHit will be the item the user has selected, or 0 when the dialog
// is being set up. 
// itemHit - itemOffset will allow you to determine which item in your dialog list this corresponds to.
// If you don't wish to do any special processing of this event, simply return fnfErr and the standard effect will take place.

OSErr	HitFader(MachineInfoPtr machineInfo, DialogPtr dPtr, short itemHit, short itemOffset)
{
	return fnfErr;
}