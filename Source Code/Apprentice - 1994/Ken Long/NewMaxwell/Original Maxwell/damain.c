/*
 * Maxwell: particles in box, with gate in middle.  Inspired by the
 * demo by the same name for the Teletype DMD 5620 terminal.
 */

#include <acc.h>
#include <desk.h>
#include <qd.h>
#include <qdvars.h>
#include <event.h>
#include <res.h>
#include <misc.h>
#include <mem.h>
#include <menu.h>
#include <te.h>
#include <font.h>
#include <file.h>
#include <win.h>
#include <control.h>
#include <device.h>
#include "ball.h"

#define	NULL	0

ACC( 0x2400, 2, 0xfffb, 0, 7, "Maxwell" )

#define MAXBALLS	20				/* must be even */

ball Balls[ MAXBALLS ];
int nBalls;
int GateState;
ControlHandle help;

int wide = 300;						/* initial size of box */
int tall = 255;

accopen( dctl, pb )
	dctlentry *dctl;
	paramblockrec *pb;
{
	WindowPeek mywindow;
	Rect bound; int i;
			
	if ( dctl->dCtlWindow == NULL )
	{
		GrafPtr saveport;
		
		GetPort( &saveport );
		SetRect( &bound, 100, 40, 130 + wide, 80 + tall );
		mywindow = NewWindow( 0L, &bound, "Maxwell", -1,
			DocumentProc, -1L, -1, 0L );
		mywindow->windowKind = dctl->dCtlRefNum;
		dctl->dCtlWindow = mywindow;
		initmove();
		SetPort( mywindow );
		helpControl( mywindow );
		InitRandom( TickCount() );
		SetupUniverse();
		SetPort( saveport );
	}
	return 0;
}

accclose( dctl, pb )
	dctlentry *dctl;
	paramblockrec *pb;
{
	DisposeWindow( dctl->dCtlWindow );
	dctl->dCtlWindow = NULL;
	return 0;
}

accctl( dctl, pb )
	dctlentry *dctl;
	paramblockrec *pb;
{
	WindowPtr mywindow;
	int code;
	EventRecord *erp;
	int infront, inmine;
	
	code = pb->paramunion.CntrlParam.CSCode;
	mywindow = dctl->dCtlWindow;
	
	SetPort( mywindow );

	infront = mywindow == FrontWindow();
	
	switch ( code ) {
case accRun:
		step( mywindow, infront );
		break;
case accEvent:
		erp = (EventRecord *)(pb->paramunion.CntrlParam.csParam.eventaddr);
		switch ( erp->what ) {
	case activateEvt:
	case updateEvt:
			setgate(0);
			redraw( mywindow );
			HiliteControl( help, infront ? 0 : 255 );
			break;
	case mouseDown:
			/*
			 * Only mouse down events we care about are in the grow
			 * icon and in the help button.  I tried FindWindow(),
			 * but it refused to distinguish between the grow icon
			 * and the rest of the content region, so instead, I
			 * will just do it directly with co-ordinates.
			 */
			if ( infront ) {
				GlobalToLocal( &erp->where );
				/*
				 * check grow icon...
				 */
				if ( 15+wide < erp->where.a.h
				  && erp->where.a.h < 30+wide
				  && 25+tall < erp->where.a.v &&
				  erp->where.a.v < 40+tall
				) {
					LocalToGlobal( &erp->where );
					resize( mywindow, &erp->where );
				} else
				/*
				 * see if there is any reason to check the help button...
				 */
				if ( erp->where.a.v <= 25 )
					/*
					 * yup, there is...
					 */
					checkhelp( erp, mywindow );
			}
			break;
		}
		break;
	}
}

accprime() {}
accstatus() {}

/*
 * step() moves all the balls
 */
static int nslow;

step(wp, infront)
	GrafPtr wp;
{
	register int i,j;

	nslow = 0;
	sortballs();
	for ( i = 0; i < nBalls; i++ ) {
		for ( j = i+1; j < nBalls; j++ )
			if ( bbump( Balls+i, Balls+j ) )
				break;
		wbump( Balls+i );
		if ( infront )
			setgate( Button() );
	}
	/*
	 * If we just draw the balls in order on the screen it will look
	 * bad, since we have them sorted by x.
	 */
	for ( i = 0; i < 10; i++ )
	for ( j = i; j < nBalls; j += 10 )
		MoveBall( Balls + j, wp );

	if ( nslow < nBalls/3 )
		walls( -1 );
	else
	if ( nslow >= (nBalls/3)<<1 )
		walls( 1 );
	else
		walls( 0 );
}

MoveBall( pA, wp )
	register ball *pA;
	GrafPtr wp;
{
	Rect R;
	register long v2;
	ball before;
	
	before = *pA;		/* save old ball */
	mball( pA );		/* compute new ball */
	
	v2 = pA->vx * pA->vx + pA->vy * pA->vy;
	
	if ( v2 <= SLOW ) nslow++;
	
	pA->pict = (v2 <= SLOW ? 0 : 1);
	
	Draw( wp, &before, pA );
	
	return;
}


/*
 * redraw() is called to deal with update events in our window
 */
redraw( wp )
	GrafPtr wp;
{
	Rect bound; int i;

	BeginUpdate( wp );
	SetRect( &bound, 0, 0, wide+30, tall+40 );
	EraseRect( &bound );				/* clear whole window */
	SetRect( &bound, 15, 25, 15+wide, 25+tall );
	FrameRect( &bound );				/* draw outer border */
	SetRect( &bound, 13+wide/2, 25, 17+wide/2, 25+tall );
	InvertRect( &bound );				/* draw barrier */
	SetRect( &bound, 13+wide/2, 24+tall/3, 17+wide/2, 26+(tall+tall)/3 );
	InvertRect( &bound );				/* remove gate + 1 pixel */
	Toggle();
	GateState = 0;						/* gate is closed */
	DrawControls( wp );
	
	/*
	 * Now draw the balls.  This only works for an even number of balls,
	 * since Draw() wants two balls.
	 */
	for ( i = 0; i < nBalls; i+=2 )
		Draw( wp, Balls + i, Balls + i + 1 );

	EndUpdate( wp );

	/*
	 * now do the grow icon
	 */
	SetRect( &bound, wide+15, tall+25, wide+30, tall+40 );
	ClipRect( &bound );
	DrawGrowIcon( wp );
	SetRect( &bound, 0, 0, 31415, 27182 );
	ClipRect( &bound );
}

/*
 * Draw() draws two balls in the window.  It is used both to draw the
 * initial balls, with the two balls being different balls, and to
 * draw a ball that has moved.  In the latter case, the two balls are
 * the same ball, once at the old position and once at the new postion.
 * This works since drawing is done in xor mode.
 */
Draw( wp, pA, pB )
	GrafPtr wp;
	register ball *pA, *pB;
{
	register long ax, ay, bx, by;
	
	ax = pA->x >> 2;
	ay = pA->y >> 2;
	
	bx = pB->x >> 2;
	by = pB->y >> 2;
	
	movebits( wp, bx-GRAD+15, by-GRAD+25, ax-GRAD+15,
		ay-GRAD+25, (long)pB->pict, (long)pA->pict );
}

/*
 * set gate to a given state.  bs != 0 means make sure the gate is
 * open, and bs == 0 means make sure it is closed.
 */
setgate( bs ) {
	if ( !!bs != GateState ) {
		GateState = !!bs;
		Toggle();
	}
}

/*
 * Change the state of the gate on the screen
 */
Toggle() {
	Rect bound;

	SetRect( &bound, 13+wide/2, 25+tall/3, 17+wide/2, 25+(tall+tall)/3 );
	InvertRect( &bound );
}

InitRandom( seed )
	long seed;
{
	asm {
		move.l	(A5), A0
		move.l	seed(A6), 0xff82(A0)
	}
}

/*
 * Generate a random integer in [low,high].  If "contract" is not zero,
 * it skews the distribution to favor numbers nearer the center of
 * the interval
 */
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

/*
 * Set up balls.
 */
SetupUniverse() {
	int i; long nb;
	
	PenNormal();
	PenMode( patXor );
	
	nb = (long)tall * (long)wide; nb >>= 13; nb <<= 1;
	nBalls = nb + 4;
	if ( nBalls > MAXBALLS )
		nBalls = MAXBALLS;

	for ( i = 0; i < nBalls; i++ ) {
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

/*
 * resize the window
 */
resize( wp, mp )
	WindowPtr wp;
	Point *mp;
{
	Rect sizerect; long result;
	
	SetRect( &sizerect, 150, 80, 2500, 402 );
	result = GrowWindow( wp, mp, &sizerect );
	if ( !result )
		return;
	tall = result >> 16; tall &= 0xffff;
	wide = result & 0xffff;
	
	wide -= 29; wide -= wide % 2;
	tall -= 38; tall -= tall % 3;
	
	DisposeControl( help );
	
	SizeWindow( wp, wide + 30, tall + 40, 0 );
	
	helpControl( wp );
	
	SetRect( &sizerect, 0, 0, wide + 30, tall + 40 );
	InvalRect( &sizerect );
	
	SetupUniverse();
	redraw( wp );
}

/*
 * Sort balls by x co-ordinate.  The first time this is called, it has
 * to do a lot of work, but on subsequent calls, the balls will for the
 * most part be already in order.  We shall use a bubble sort, since
 * it is very fast for an already sorted list.
 */
sortballs() {
	register int i, j, flag;
	
	flag = 1;
	for ( i = 0; i < (nBalls - 1) && flag; i++ )
		for ( flag = 0, j = nBalls-1; j > i; --j )
			if ( Balls[j-1].x > Balls[j].x ) {
				ball temp;
				flag = 1;
				temp = Balls[j-1]; Balls[j-1] = Balls[j];
				Balls[j] = temp;
			}
}

/*
 * repond to a click in the help button
 */
showhelp() {
	Rect bound;
	WindowPtr wp;
	char *h1, *h2, *h3, *h4;
	char helptext[1024];
	
	SetRect( &bound, 91, 68, 421, 303 );
	wp = NewWindow( 0L, &bound, "A", -1, DBoxProc, -1L, 0, 0L );
	SetPort( wp );

	TextMode( srcXor );
	TextSize( 9 );
	
	SetRect( &bound, 10, 10, 320, 225 );

	h1 = "\
Maxwell V2.1 from Mithral Engineering\r\r\
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
 of the balls are fast, the right wall will cool off.\r\r\
This program and its source code are in the public domain";

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

/*
 * put up the help button
 */
helpControl( wp )
	WindowPtr wp;
{
	Rect bound;
	
	SetRect( &bound, (wide>>1)-7, 4 , (wide>>1)+37, 22 );
	
	help = NewControl( wp, &bound, "Info...", -1,0,0,0, PushButProc, 0L );
}

/*
 * find out if the user wants help
 */
checkhelp( erp, win )
	EventRecord * erp;
	WindowPtr win;
{
	int part;
	ControlHandle ch;
	
	part = FindControl( &erp->where, win, &ch );
	
	if ( part != inButton || ch != help )
		return;
		
	 part = TrackControl( ch, &erp->where, 0L );
	
	 if ( part == inButton )
	 	showhelp();
}
