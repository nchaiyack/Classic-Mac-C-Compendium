//� Maxwell.c

//� Maxwell: particles in box, with gate in middle.  Inspired by the
//� demo by the same name for the Teletype DMD 5620 terminal.

typedef struct particle 
{
	long x,y;				//� location.
	long vx, vy;			//� velocity.
	int	pict;				//� which picture this ball is.
} ball;

#define	GRAD		6			//� radius for drawing (quickdraw units).
#define	CRAD		28			//� radius for collision (box units).
#define	SDIM		(2 * GRAD)	//� diameter on screen.
#define MAXBALLS	20			//� must be even.
#define	SLOW		225			//� max v squared for slow ball.

#define TWO(X) ((X)+(X))
#define THREE(X) (TWO(X)+(X))

static int nslow;
static void enable (MenuHandle menu, short item, short ok);

ball Balls[ MAXBALLS ];
int nBalls;
int GateState;				//� != 0 if gate is open.
ControlHandle help;

int wide = 300;						//� initial size of box.
int tall = 255;

WindowPtr	maxwellWindow;
Rect		dragRect;
Rect		windowBounds = { 40, 100, 335, 430 };

MenuHandle	appleMenu, fileMenu, editMenu, widthMenu;

enum	{
	appleID = 1,
	fileID,
	editID,
	widthID
	};

enum	{
	openItem = 1,
	closeItem,
	quitItem = 4
	};

SetUpWindow()
{
	Rect bound; 
	int i;
	GrafPtr saveport;

	dragRect = screenBits.bounds;

	SetRect( &bound, 40, 100, 430, 335);
	maxwellWindow = NewWindow( 0L, &windowBounds, "\pMaxwell", -1, documentProc, (WindowPtr)-1L, -1, 0L );

	InitMoving();
	SetPort( maxwellWindow );
	AddHelpControl( maxwellWindow );
	GetDateTime (&randSeed);
	SetupUniverse();
}

CleanUp ()
{
	DisposeWindow(maxwellWindow);
}


void SetUpMenus(void)
{
	InsertMenu(appleMenu = NewMenu(appleID, "\p\024"), 0);
	InsertMenu(fileMenu = NewMenu(fileID, "\pFile"), 0);
	InsertMenu(editMenu = NewMenu(editID, "\pEdit"), 0);
	DrawMenuBar();
	AddResMenu(appleMenu, 'DRVR');
	AppendMenu(fileMenu, "\pOpen/O;Close/W;(-;Quit/Q");
	AppendMenu(editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
}

void AdjustMenus(void)
{
	register WindowPeek wp = (WindowPeek) FrontWindow();
	short kind = wp ? wp->windowKind : 0;
	Boolean DA = kind < 0;
	
	enable(editMenu, 1, DA);
	enable(editMenu, 3, DA);
	enable(editMenu, 4, DA);
	enable(editMenu, 5, DA);
	enable(editMenu, 6, DA);
	
	enable(fileMenu, openItem, !((WindowPeek) maxwellWindow)->visible);
	enable(fileMenu, closeItem, DA || ((WindowPeek) maxwellWindow)->visible);
}

static void enable(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
}

void HandleMenu (long mSelect)
{
	int			menuID = HiWord(mSelect);
	int			menuItem = LoWord(mSelect);
	Str255		name;
	GrafPtr		savePort;
	WindowPeek	frontWindow;
	
	switch (menuID)
	{
		case	appleID:
			GetPort(&savePort);
			GetItem(appleMenu, menuItem, name);
			OpenDeskAcc(name);
			SetPort(savePort);
		break;
	
		case	fileID:
			switch (menuItem)
			{
				case	openItem:
					ShowWindow(maxwellWindow);
					SelectWindow(maxwellWindow);
				break;
  								
				case	closeItem:
					if ((frontWindow = (WindowPeek) FrontWindow()) == 0L)
				break;
				
				if (frontWindow->windowKind < 0)
					CloseDeskAcc(frontWindow->windowKind);
				else 
					if ((frontWindow = (WindowPeek) maxwellWindow) != NULL)
							HideWindow(maxwellWindow);
  				break;
  							
				case	quitItem:
					ExitToShell();
				break;
			}
		break;
  				
		case	editID:
			if (!SystemEdit(menuItem-1))
				SysBeep(5);
		break;
	}
}

//� StepBall() moves all the balls
StepBall(GrafPtr wp, int infront)
{
	register int i,j;

	nslow = 0;
	SortBalls();
	for ( i = 0; i < nBalls; i++ ) 
	{
		for ( j = i+1; j < nBalls; j++ )
			if ( BallCollide( Balls+i, Balls+j ) )
				break;
		WallCollide( Balls+i );
		if ( infront )
			SetTheGate( Button() );
	}
	//� If we just draw the balls in order on the screen it will look
	//� bad, since we have them sorted by x.
	for ( i = 0; i < 10; i++ )
	for ( j = i; j < nBalls; j += 10 )
		MoveBall( Balls + j, wp );

	if ( nslow < nBalls/3 )
		WallForces( -1 );
	else
	if ( nslow >= (nBalls/3)<<1 )
		WallForces( 1 );
	else
		WallForces( 0 );
}

MoveBall(ball *pA, GrafPtr wp)
{
	Rect R;
	register long v2;
	ball before;
	
	before = *pA;		//� save old ball.
	ComputeBall( pA );		//� compute new ball.
	
	v2 = pA->vx * pA->vx + pA->vy * pA->vy;
	
	if ( v2 <= SLOW ) nslow++;
	
	pA->pict = (v2 <= SLOW ? 0 : 1);
	
	Draw( wp, &before, pA );
	
	return;
}

//� Redraw() is called to deal with update events in our window.

Redraw(GrafPtr wp)
{
	Rect bound; int i;

	BeginUpdate( wp );
	SetRect( &bound, 0, 0, wide+30, tall+40 );
	EraseRect( &bound );				//� clear whole window.
	SetRect( &bound, 15, 25, 15+wide, 25+tall );
	FrameRect( &bound );				//� draw outer border.
	SetRect( &bound, 13+wide/2, 25, 17+wide/2, 25+tall );
	InvertRect( &bound );				//� draw barrier.
	SetRect( &bound, 13+wide/2, 24+tall/3, 17+wide/2, 26+(tall+tall)/3 );
	InvertRect( &bound );				//� remove gate + 1 pixel.
	Toggle();
	GateState = 0;						//� gate is closed.
	DrawControls( wp );
	
	//� Now draw the balls.  This only works for an even number 
	//� of balls,since Draw() wants two balls.
	for ( i = 0; i < nBalls; i+=2 )
		Draw( wp, Balls + i, Balls + i + 1 );

	EndUpdate( wp );

	//� Now do the grow icon.
	SetRect( &bound, wide+15, tall+25, wide+30, tall+40 );
	ClipRect( &bound );
	DrawGrowIcon( wp );
	SetRect( &bound, 0, 0, 31415, 27182 );
	ClipRect( &bound );
}

//� Draw() draws two balls in the window.  It is used both to draw the
//� initial balls, with the two balls being different balls, and to
//� draw a ball that has moved.  In the latter case, the two balls are
//� the same ball, once at the old position and once at the new postion.
//� This works since drawing is done in xor mode.
Draw(GrafPtr wp, ball *pA, ball *pB)
{
	register long ax, ay, bx, by;
	
	ax = pA->x >> 2;
	ay = pA->y >> 2;
	
	bx = pB->x >> 2;
	by = pB->y >> 2;
	
	MoveBits( wp, bx-GRAD + 15, by-GRAD + 25, ax-GRAD + 15,
		ay-GRAD + 25, (long)pB->pict, (long)pA->pict );
}

//� set gate to a given state.  bs != 0 means make sure the gate is
//� open, and bs == 0 means make sure it is closed.
SetTheGate( bs ) 
{
	if ( !!bs != GateState ) 
	{
		GateState = !!bs;
		Toggle();
	}
}

//� Change the state of the gate on the screen
Toggle() 
{
	Rect bound;

	SetRect( &bound, 13 + wide / 2, 
					 25 + tall / 3, 
					 17 + wide / 2, 
					 25 + (tall + tall) / 3 );
	InvertRect( &bound );
}

//� Generate a random integer in [low,high].  If "contract" is not zero,
//� it skews the distribution to favor numbers nearer the center of
//� the interval

rani(low,high,contract)
	int low, high;
{
	register long r;
	register int range;
	
	r = (Random()>>1) + 16384;
	if ( !contract )
		return r * (high-low) / 32768L + low;
	
	range = (high - low) >> 1;
	
	r = r * range;
	r /= 32768;
	r *= Random();
	range = r / 32768;
	
	return ( (low + high) >> 1 ) + range;
}

//� Set up balls.

SetupUniverse() 
{
	int i; long nb;
	
	PenNormal();
	PenMode( patXor );
	
	nb = (long)tall * (long)wide; nb >>= 13; nb <<= 1;
	nBalls = nb + 4;
	if ( nBalls > MAXBALLS )
		nBalls = MAXBALLS;

	for ( i = 0; i < nBalls; i++ ) 
	{
		register long a,b;
		
		Balls[i].x  = rani(CRAD, (wide<<2)-CRAD, 0);
		Balls[i].y  = rani(CRAD, (tall<<2)-CRAD, 0);
		Balls[i].vx = a = rani(-40, 40, 1);
		Balls[i].vy = b = rani(-40, 40, 1);
		if ( a*a + b*b <= SLOW )
			Balls[i].pict = 0;
		else
			Balls[i].pict = 1;
	}
}


//� ReSize the window

ReSize(WindowPtr wp, Point *mp)
{
	Rect sizeRect; long result;
	
	SetRect( &sizeRect, 150, 80, 2500, 402 );
	result = GrowWindow( wp, *mp, &sizeRect );
	if ( !result )
		return;
	tall = result >> 16; tall &= 0xffff;
	wide = result & 0xffff;
	
	wide -= 29; wide -= wide % 2;
	tall -= 38; tall -= tall % 3;
	
	DisposeControl( help );
	
	SizeWindow( wp, wide + 30, tall + 40, 0 );
	
	AddHelpControl( wp );
	
	SetRect( &sizeRect, 0, 0, wide + 30, tall + 40 );
	InvalRect( &sizeRect );
	
	SetupUniverse();
	Redraw( wp );
}


//� Sort balls by x co-ordinate.  The first time this is called, it has
//� to do a lot of work, but on subsequent calls, the balls will for the
//� most part be already in order.  We shall use a bubble sort, since
//� it is very fast for an already sorted list.

SortBalls() 
{
	register int i, j, flag;
	
	flag = 1;
	for ( i = 0; i < (nBalls - 1) && flag; i++ )
		for ( flag = 0, j = nBalls-1; j > i; --j )
			if ( Balls[j-1].x > Balls[j].x ) 
			{
				ball temp;
				flag = 1;
				temp = Balls[j-1]; Balls[j-1] = Balls[j];
				Balls[j] = temp;
			}
}


//� repond to a click in the help button

ShowHelp() 
{
	Rect bound;
	WindowPtr wp;
	char *h1, *h2, *h3, *h4;
	char helptext[1024];
	
	SetRect( &bound, 91, 68, 421, 303 );
	wp = NewWindow( 0L, &bound, "\pA", -1, dBoxProc, (WindowPtr)-1L, 0, 0L );
	SetPort( wp );

	TextMode( srcXor );
	TextSize( 9 );
	
	SetRect( &bound, 5, 5, 325, 230 );

	h1 = "\
Maxwell V2.1 from Mithral Engineering.\r\
This program and its source code are in the public domain.\r\r\
Whenever the Maxwell window is in front, holding down the\
 mouse button opens the gate so that balls may go from one\
 side to the other.\r\r";
 	h2 = "\
Try to get all the fast balls ( the black ones ) in the right\
 half of the window, and all the slow ones in the left.\r\r\
Due to roundoff errors, at each collision there is a slight";
	h3 = "\
 net decrease in the total energy of the balls.  To balance\
 this, the right wall will become 'hot' if less than one third\
 of the balls are fast balls.  When a slow ball hits the hot right";
 	h4 = "\
 wall, it will become a very fast ball.  When more than one third\
 of the balls are fast, the right wall will cool off.\r\
--------------------------------------------------------------\r\
Made to run under THINK C� 5.0.4 by Kenneth A. Long on 3 Feb 94.\r\
The shell is basically \"Bullseye,\" and all merged into one .c file.";

	strcpy( helptext, h1 );
	strcat( helptext, h2 );
	strcat( helptext, h3 );
	strcat( helptext, h4 );

	TextBox( helptext, (long)strlen( helptext ), &bound, teJustLeft );
	
	while ( !Button() )
		;
	FlushEvents( mDownMask, 0 );
	DisposeWindow( wp );
}


//� put up the help button

AddHelpControl(WindowPtr wp)
{
	Rect bound;
	
	SetRect( &bound, (wide>>1)-7, 4 , (wide>>1) + 37, 22 );
	
	help = NewControl( wp, &bound, "\pHelp!", -1,0,0,0, pushButProc, 0L );
}


//� find out if the user wants help

CheckHelp(EventRecord *erp, WindowPtr win)
{
	int part;
	ControlHandle ch;
	
	part = FindControl( erp->where, win, &ch );
	
	if ( part != inButton || ch != help )
		return;
		
	 part = TrackControl( ch, erp->where, 0L );
	
	 if ( part == inButton )
	 	ShowHelp();
}



//� move or draw ball on screen.

//� There are three off-screen bitmaps.  The first two hold the images of
//� a slow ball and a fast ball.  The third is used for combining the
//� before and after pictures so that a screen update of a moving ball
//� whose new postion overlaps its old position can be done in one
//� screen operation.  This is intended to reduce flicker, by cutting
//� number of copybits calls to the screen from two to one per slow
//� moving ball.  However, it increases the total number of copybits
//� calls, which makes things run slower.

//� The use of this third bitmap is enabled by defining SMOOTH.  It will
//� run ~25% slower with SMOOTH defined.


char slowBits[ 2 * (SDIM) * ((SDIM + 15)/16) ];		//� slow bits.
char fastBits[ 2 * (SDIM) * ((SDIM + 15)/16) ];		//� fast bits.

#ifdef SMOOTH
//� combined bits.
char combBits[ 2 * (2 * SDIM) * ((2 * SDIM + 15) / 16) ];	
#endif

BitMap slowMap, fastMap			//� bitmaps for slow and fast bits.
#ifdef SMOOTH
	,combMap					//� and combined bits.
#endif
;

//� MoveBits() draws two balls.  The first is in the square with corner
//� at Sx, Sy, and side SDIM.  The second is at Dx and Dy, and has the
//� same size.  Which drawing to use for each is determined by op and 
//� np.

MoveBits(GrafPtr win, long Sx, long Sy, long Dx, long Dy, 
					long op, long np)
{
	register int sx, sy, dx, dy;
	int tx,ty;
	int rt, rl;
	Rect S,D;

	//� The pointers to the bits in the bitmaps may have changed if we
	//� have been moved on the heap since we were initialized, so 
	//� we shall fix them.
	slowMap.baseAddr = slowBits;
	fastMap.baseAddr = fastBits;
#ifdef SMOOTH
	combMap.baseAddr = combBits;
#endif
	
	//� convert to integers.
	sx = Sx; 
	sy = Sy; 
	dx = Dx; 
	dy = Dy; 

	tx = dx - sx;			//� relative x positions.
	ty = dy - sy;			//� relative y positions.
	
	
#define pict(v) ((v) == 0 ? &slowMap : &fastMap )

	SetPort( win );

//� If the balls do not overlap, or if SMOOTH is not defined, then we
//� simply want to erase the first ball and draw the second.
#ifdef SMOOTH
	//� If the balls can't possibly overlap, then just draw them 
	//� directly on the screen
	if ( tx < - GRAD || tx > GRAD || ty < - GRAD || ty > GRAD ) 
	{
#endif
		SetRect( &S, 0, 0, SDIM, SDIM );
		SetRect( &D, sx, sy, sx + SDIM, sy + SDIM );
		CopyBits( pict(op), &win->portBits, &S, &D, srcXor, 0L );
		SetRect( &D, dx, dy, dx + SDIM, dy + SDIM );
		CopyBits( pict(np), &win->portBits, &S, &D, srcXor, 0L );
		return;
#ifdef SMOOTH
	}
	
	//� The balls are close enough that we may combine their 
	//� updates into one CopyBits to the screen.
	
	//� The rest of this is a pain to explain without a diagram,
	//� so figure it out for yourself!
	if ( ty > 0 )
		if ( tx > 0 )	
		{ 
			rt = 0;    
			rl = 0; 
		}
		else			
			{ 
				rt = 0;    
				rl = SDIM; 
		}
	else
		if ( tx > 0 )	
		{ 
			rt = SDIM; 
			rl = 0; 
		}
		else			
			{ 
				rt = SDIM; 
				rl = SDIM; 
		}
		
	SetRect( &D, 0, 0, 2 * SDIM, 2 * SDIM );
	CopyBits( &combMap, &combMap, &D, &D, srcXor, 0L );
	SetRect( &S, 0, 0, SDIM, SDIM );
	SetRect( &D, rl, rt, rl+SDIM, rt+SDIM );
	CopyBits( pict(op), &combMap, &S, &D, srcCopy, 0L );
	SetRect( &D, rl + tx, rt + ty, rl + tx + SDIM, rt + ty + SDIM );
	CopyBits( pict(np), &combMap, &S, &D, srcXor, 0L );
	SetRect( &S, 0, 0, SDIM * 2, SDIM * 2 );
	SetRect( &D, sx - rl, sy - rt, sx - rl + 2 * SDIM, sy - rt + 2 * SDIM );
	CopyBits( &combMap, &win->portBits, &S, &D, srcXor, 0L );
#endif
}

//� Initialize the bitmaps

InitMoving() 
{		
	Rect S,D;
	GrafPort port;

	//� Get a grafport to do our thing in.
	OpenPort( &port );
	PenNormal();
	
	//� Bitmap for slow bits...
	slowMap.baseAddr = slowBits;
	slowMap.rowBytes = 2*((SDIM+15)/16);
	slowMap.bounds.top = 0;
	slowMap.bounds.bottom = 2*SDIM;
	slowMap.bounds.left = 0;
	slowMap.bounds.right = 2*SDIM;
	
	//� Bitmap for fast bits...
	fastMap.baseAddr = fastBits;
	fastMap.rowBytes = 2*((SDIM+15)/16);
	fastMap.bounds.top = 0;
	fastMap.bounds.bottom = 2*SDIM;
	fastMap.bounds.left = 0;
	fastMap.bounds.right = 2*SDIM;
	
	SetPortBits( &slowMap );		//� prepare to draw slow ball.
	
	SetRect( &S, 0, 0, SDIM, SDIM );
	FrameOval( &S );

	//� ...risking the taste-police - :)
	MoveTo( 3, 7 ); 
	LineTo( 4, 8 );	
	LineTo( 7, 8 ); 
	LineTo( 8, 7 );
	MoveTo( 4, 4 ); 
	LineTo( 4, 4 );
	MoveTo( 7, 4 ); 
	LineTo( 7, 4 );

	SetPortBits( &fastMap );		//� prepare to draw fast ball.
	InvertOval( &S );
	
#ifdef SMOOTH
	//� Bitmap for combined drawings...
	combMap.baseAddr = combBits;
	combMap.rowBytes = 2*((SDIM+SDIM+15)/16);
	combMap.bounds.top = 0;
	combMap.bounds.bottom = 2*SDIM;
	combMap.bounds.left = 0;
	combMap.bounds.right = 2*SDIM;
#endif	
}


//� Do collisions and movement.

//� BallCollide() deals with a possible collision between two 
//� specific balls.
//� The first ball will not be to the right of the second ball.  
//� Returns one if the second ball is far enough to the right so 
//� that no balls farther right could collide with the first ball, 
//� else returns zero.

BallCollide(ball *pA, ball *pB)
{
	register long k;
	long tAvx, tAvy;
	register long tBx, tBy;
	long tBvx, tBvy;
	long WIDE, TALL;
	
	WIDE = wide<<2;			//� scale from window to physical co-ords.
	TALL = tall<<2;
	
	//� Deal with the barrier and the gate.
	if ( TWO(pA->x) <= WIDE && TWO(pB->x) >= WIDE )
		//� gate closed means no collision.
		if ( ! GateState )
			return 0;
		else
			//� If either ball is below gate, no collision.
			if ( THREE(pA->y) < TALL || THREE(pB->y) < TALL )
				return 0;
		else
			//� If either ball is above gate, no collision.
			if (THREE(pA->y) > 
				TWO(TALL) || 
				THREE(pB->y) > 
				TWO(TALL) )
			return 0;

	//� Shift to A's co-ordinate system.
	tBx = pB->x - pA->x;
	tBy = pB->y - pA->y;
	
	tBvx = pB->vx - pA->vx;	
	tAvx = 0;
	
	tBvy = pB->vy - pA->vy;	
	tAvy = 0;
	
	//� See if the balls are close enough to have collided.
	if ( tBx > TWO(CRAD) )
		return 1;
	if ( tBx * tBx + tBy * tBy > (CRAD * CRAD<<2) )
		return 0;
	
	k = tBx * tBvx + tBy * tBvy;
	
	//� Make sure they are going towards each other.
	if ( k > -1 )
		return 0;
		
	k = ( tBy * tBvx - tBx * tBvy ) / k;
	
	tAvx = ( tBvx - k * tBvy ) / ( 1 + k*k );
	tAvy = ( k * tBvx + tBvy ) / ( 1 + k*k );
	
	tBvx =  k * tAvy;
	tBvy = -k * tAvx;
	
	pB->vx = pA->vx + tBvx;
	pB->vy = pA->vy + tBvy;
	pA->vx += tAvx;
	pA->vy += tAvy;
	
	return 0;
}

//� Because the calculations above use longs instead of floats, we have a
//� lot of round off error.  This seems to manifest itself by causing the
//� balls to slow down over time.  We use the walls to correct this.

//� If "we" is greater than zero, we are attempting to add energy to the
//� system.  We do this by looking for slow balls bouncing off the right
//� wall.  When we find such a ball, we give it a swift kick towards the
//� left.

//� If "we" is less than zero, then we are trying to remove energt from
//� the system for some reason.  In this case, the outer walls become
//� slightly sticky, with the ball slowing down by abs(we) perpendicular
//� to the wall.

//� This stuff is done in WallCollide().

static int we = 0;			//� wall energy factor.

WallForces( i ) 
{
	we = i;
}


//� WallCollide() checks for collisions between a ball walls or the gate.
WallCollide(ball *pA)
{
	register long WIDE, TALL;
	
	WIDE = wide<<2; TALL = tall<<2;
	
	if ( (pA->x <= CRAD && pA->vx < 0)
	  || (pA->x >= WIDE-CRAD && pA->vx > 0) ) 
	{
		pA->vx = -pA->vx;
		if ( we > 0 ) 
		{
			if ( pA->vx < 0 && pA->pict == 0 && pA->x >= WIDE-CRAD)
				pA->vx -= 30;				//� swift kick.
		} 
		else 
			{
				if ( pA->vx > 0 )
					pA->vx += we;
				else
					pA->vx -= we;
		}
	}
	if ((pA->y <= CRAD && pA->vy < 0) ||
		(pA->y >= TALL-CRAD && pA->vy > 0) ) 
	{
		pA->vy = -pA->vy;
		if ( we < 0 ) 
		{
			if ( pA->vy > 0 )
				pA->vy += we;
			else
				pA->vy -= we;
		}

	}
	
	//� if the ball is on the same level as the gate, and the gate 
	//� is open, there is nothing for the ball to hit on the barrier.

	if ( TALL < THREE(pA->y) && THREE(pA->y) < TWO(TALL) && GateState )
		return;
		
	WIDE >>= 1;			//� location of the barrier.

	//� see if the ball hits the barrier.
	if ( pA->x <= WIDE && pA->vx > 0 ) 
	{
		if ( pA->x + CRAD >= WIDE || pA->x + pA->vx > WIDE )
			pA->vx = -pA->vx;
	} 
	else
		if ( pA->x >= WIDE &&  pA->vx < 0 )
			if ( pA->x - CRAD <= WIDE || pA->x + pA->vx < WIDE )
				pA->vx = -pA->vx;
}

ComputeBall(ball *pA)
{
	register long vx, vy;
	pA->x += vx = pA->vx;
	pA->y += vy = pA->vy;

	
	//� check for stalled balls, and offer them a chance to get going again.
	if ( vx == 0 && vy == 0 ) 
	{
		if ( Random() > 0 )
			pA->vx = Random() > 0 ? 1 : -1;
		if ( Random() > 0 )
			pA->vy = Random() > 0 ? 1 : -1;
	}
}

void InitMacintosh(void)
{
	MaxApplZone();
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

void HandleMouseDown (EventRecord	*theEvent)
{
	WindowPtr	theWindow;
	int			windowCode = FindWindow (theEvent->where, &theWindow);
	int infront;
	
	switch (windowCode)
	{
		case inSysWindow: 
			SystemClick (theEvent, theWindow);
		break;
			
		case inMenuBar:
			AdjustMenus();
			HandleMenu(MenuSelect(theEvent->where));
		break;
			
		case inDrag:
			if (theWindow == maxwellWindow)
			DragWindow(maxwellWindow, theEvent->where, &dragRect);
		break;
			
		case inContent:
			if (theWindow == maxwellWindow)
			{				
					GlobalToLocal( &theEvent->where );
					
					//� check grow icon...
					if ( 15 + wide < theEvent->where.h
						&& theEvent->where.h < 30 + wide
						&& 25 + tall < theEvent->where.v &&
						theEvent->where.v < 40 + tall) 
					{
						LocalToGlobal( &theEvent->where );
						ReSize( maxwellWindow, &theEvent->where );
					} 
					else
						//� see if there is any reason to check the 
						//� help button...
						if ( theEvent->where.v <= 25 )
							//� yup, there is...
							CheckHelp( theEvent, maxwellWindow );
			}
		break;
			
		case inGoAway:
			if (theWindow == maxwellWindow && 
				TrackGoAway(maxwellWindow, theEvent->where))
				HideWindow(maxwellWindow);
		break;
	}
}

void HandleEvent(void)
{
	int			ok, infront;
	EventRecord	theEvent;

	HiliteMenu(0);
	SystemTask ();		/* Handle desk accessories */

	StepBall( maxwellWindow, infront );

	ok = GetNextEvent (everyEvent, &theEvent);
	if (ok)
	switch (theEvent.what)
	{
		case mouseDown:
			HandleMouseDown(&theEvent);
		break;
			
		case keyDown: 
		case autoKey:
			if ((theEvent.modifiers & cmdKey) != 0)
			{
				AdjustMenus();
				HandleMenu(MenuKey((char) (theEvent.message & charCodeMask)));
			}
		break;
			
		case activateEvt:
		case updateEvt:
			SetTheGate(0);
			Redraw( maxwellWindow );
			HiliteControl( help, infront ? 0 : 255 );
		break;
	}
}

void main( void)
{
	InitMacintosh();
	SetUpMenus();
	SetUpWindow();
	for (;;)
		HandleEvent();
}
