/*
*	FILE:		offscreen2.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	June 28, 1993
*
*	Demonstrates
*	offscreen drawing. Code taken from CopyBits example in Think Ref 2.0
*	and from Offscreen Graphics example. (Also tried Offscreen PixMaps
*	example and Creating Offscreen BitMaps example.)
*
*	This version checks for presence of Color QuickDraw.
*
*	It also tries to sync to vertical retrace -- doesn't work if bitmap too large.
*
*	Try using global coords for NewGWorld rec.
*
*	PROJECT CONTENTS:
*		offscreen.c, MacTraps, ANSI
*
*	COMPILATION:
*		68881 code generation if avail. MacHeaders prefix.
*/

# define CENTER_X	0.
# define CENTER_Y	0.
# define SPOKE_SIZE	.6
# define RADIUS1	.5
# define RADIUS2	.4
# define RADIUS3	.3
# define PI			3.1415926
# define INCR		PI/30.
# define MAP_WIDE 300
# define MAP_HIGH 300

# define OFFSCREEN	// comment this out to see effect

//	For B&W case:
#define MAP_WBYTES  ( ( ( MAP_WIDE / 16 ) +1 ) * 2 )
#define MAP_SIZE ( MAP_WBYTES * MAP_HIGH )
//

//	For Color case:
# include	<QDOffscreen.h>
//

# include	<math.h>
# include	<GestaltEqu.h>

void	init_graphics(void);
double	get_aspect_ratio(void);
void	set_view(double,double,double,double);
void	draw_line(double,double,double,double);	
void	move_to(double,double);
void	draw_to(double,double);
void	draw_circle(double,double,double);	
void	fill_circle(double,double,double);	
int		mouse_button_is_down(void);
void	get_mouse_location(double*,double*);
void	wait(void);		

WindowPtr	myWindow;
Boolean		color_QD,		// used to initialize window and offscreen bit/pixmap
			color_avail;	// use this to decide what maps to black and white

# define	SMALL_COPY_RECT	// use to find smallest rect which needs to be redrawn
Rect		circle_rect;	

# define	SYNC	// For syncing to vertical retrace. (From Alex Chafee in c.s.m.p.)

# ifdef SYNC
# include	<Retrace.h>
# include	<Start.h>

typedef struct
{
	long	goodA5;
	VBLTask	task;
} MYVBLTASK;

void	InstallSync(void);
void	WaitForSync(void);
void	RemoveSync(void);
pascal void Task(void);

long		vCounter;	// like Ticks
long		vblJunk;
MYVBLTASK	mytask;
DefVideoRec	video;
# endif

# define GLOBAL	// not sure this is necessary...

main()
{
	double	x,
			y,
			old_x,
			old_y,
			angle = 0.;
	Rect sRect,dRect;
	Rect changed_rect,		// CopyBits on the entire window is too slow for proper
		 rect2;				// syncing, so we'll make a rect which holds the new
		 					// and old images.
# ifdef GLOBAL
	Rect	r,
			*pr;
# endif

//	For B&W case:
	BitMap saveMap,tempMap;
//
//	For Color case:
	GWorldPtr currPort;    // Saves the current port prior to setting up
							// offscreen world
	GDHandle currDev;       // Saves the current device prior to setting up
							// offscreen world
	GWorldPtr gMyOffG;	// Pointer to OffScreen Graphics World
//

	init_graphics();	/* Don't forget to do this FIRST! */
	
# ifdef SYNC
	InstallSync();
# endif
	
# ifdef OFFSCREEN
	if (!color_QD)
	{
		// allocate new B&W "canvas"
		tempMap.baseAddr = ( QDPtr )NewPtr( MAP_SIZE  );
		tempMap.rowBytes = MAP_WBYTES;	// initialize needed fields
		tempMap.bounds = myWindow->portRect;
		saveMap =  myWindow->portBits;
# ifndef SMALL_COPY_RECT
		sRect = dRect = myWindow->portRect;	// set source and destination rects
# endif
		SetPortBits( &tempMap );
	}
	else
	{
		// Build Offscreen Graphics world
		GetGWorld(&currPort,&currDev);

		// Create Offscreen Graphics world.
# ifdef GLOBAL
		r = myWindow->portRect;
		LocalToGlobal((Point*) &r);
		LocalToGlobal(((Point*) &r) +1);
		NewGWorld(&gMyOffG, 0, &r, nil, nil, 0);
		pr = &gMyOffG->portRect;
		OffsetRect(pr,-pr->left,-pr->top);
# else
		NewGWorld(&gMyOffG, 0, &myWindow->portRect, nil, nil, 0);
# endif

		// Lock down Pixels that we are drawing to so that memory will not move
		LockPixels (gMyOffG->portPixMap);
		// Setup drawing area to be our offscreen graphics world
# ifndef SMALL_COPY_RECT
		sRect = dRect = myWindow->portRect;	// set source and destination rects
# endif
		SetGWorld (gMyOffG, nil);
	}

	BackColor(whiteColor);	// for proper operation of CopyBits()
	
	EraseRect( &myWindow->portRect );	
// with color QD can also say: EraseRect(&(*gMyOffG->portPixMap)->bounds);

# endif

	old_x = RADIUS1 + CENTER_X;
	old_y = CENTER_Y;
	
	while (!mouse_button_is_down())
	{
# ifdef OFFSCREEN
		if (!color_QD)	
			SetPortBits( &tempMap );
		else
			SetGWorld (gMyOffG, nil);
# endif

		x = cos(angle)*SPOKE_SIZE + CENTER_X;
		y = sin(angle)*SPOKE_SIZE + CENTER_Y;

//		erase last drawing:
		ForeColor(whiteColor);
		fill_circle(old_x,old_y,RADIUS1);
		rect2 = circle_rect;
//
		
		ForeColor(cyanColor);	// non-white colors map to black for B&W case.
								// In that case I won't fill this circle
		if (color_avail)
			fill_circle(x,y,RADIUS1);
		else
			draw_circle(x,y,RADIUS1);
		
		UnionRect(&circle_rect,&rect2,&changed_rect);
# ifdef SMALL_COPY_RECT
		sRect = dRect = changed_rect;
# endif

		ForeColor(yellowColor);
		
		if (color_avail)
			fill_circle(x,y,RADIUS2);
		else
			draw_circle(x,y,RADIUS2);

		ForeColor(magentaColor);
		fill_circle(x,y,RADIUS3);
		
		old_x = x;
		old_y = y;
		angle += INCR;

# ifdef OFFSCREEN
		if (!color_QD)	
		{
			SetPortBits( &saveMap );
			ForeColor(blackColor);		// entire bitmap is drawn in this color
# ifdef SYNC
			WaitForSync();
# endif
			CopyBits(&tempMap,&myWindow->portBits, &sRect, &dRect, srcCopy, nil);
		}
		else
		{
			SetGWorld (currPort, currDev);
			ForeColor(blackColor);		// to preserve proper colors
# ifdef SYNC
			WaitForSync();
# endif
			CopyBits((BitMap *) (*(gMyOffG->portPixMap)),
				&((GrafPtr)myWindow)->portBits, &sRect, &dRect, srcCopy, nil);
		}
# endif
	}
	
# ifdef OFFSCREEN
	if (!color_QD)	
		DisposePtr((Ptr)tempMap.baseAddr);	// free up canvas RAM
	else
	{
		// Now unlock Pixels.
		UnlockPixels (gMyOffG->portPixMap);
		DisposeGWorld(gMyOffG);
	}
# endif

# ifdef SYNC
	RemoveSync();
# endif
}

/////////////////////////////////////////////////////////////////////////
//	Following code adapted from macgraph-ANSI.c
/////////////////////////////////////////////////////////////////////////

# define	NIL_POINTER			0L
# define	MOVE_TO_FRONT		-1L
# define	REMOVE_ALL_EVENTS	0
# define	TITLE				"\pGraphics"
# define	VISIBLE				1
# define	NO_GO_AWAY			0
# define	NIL_REF_CON			NIL_POINTER
# define	BACKGROUND			BLACK

static WindowPtr	gDrawWindow,
					gConsoleWindow;
static CursHandle	gCursor;
static int			gDrawwidth,
					gDrawheight,
					gPresentcolor;
static double		gViewwidth = 2.,
					gViewheight = 2.,
					gViewx = 1.,
					gViewy = 1.;

static int			transform_x(double),
					transform_y(double);
					
/************************************************************************
*	You must call init_graphics() at the beginning of main().
*	Likely don't need all these initializations; they are taken from Mark
*	& Reed's "Macintosh Programming Primer", Addison-Wesley, 1989
************************************************************************/
void	init_graphics(void)
{
	double	aspect;
	Rect	rectangle;
	short int	left,
				right,
				top,
				bottom;
	long		temp;
	
	left = 5;
	right = MAP_WIDE+left;
	top = 45;
	bottom = MAP_HIGH+top;
		
	SetRect(&rectangle,left,top,right,bottom);
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,REMOVE_ALL_EVENTS);	
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL_POINTER);
	InitCursor();

//	Check for color availability:
	Gestalt(gestaltQuickdrawVersion,&temp);
	color_QD = temp >= gestalt8BitQD;
//

	if (!color_QD)
	{
		gDrawWindow = NewWindow(NIL_POINTER,&rectangle,
			TITLE,VISIBLE,plainDBox,(WindowPtr) MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);
		color_avail = false;
	}
	else
	{
		gDrawWindow = NewCWindow(NIL_POINTER,&rectangle,
			TITLE,VISIBLE,plainDBox,(CWindowPtr) MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);
		color_avail = (**((CGrafPtr) gDrawWindow)->portPixMap).pixelSize > 1;
	}

	SetPort(gDrawWindow);
	gDrawwidth = gDrawWindow->portRect.right - gDrawWindow->portRect.left;
	gDrawheight = gDrawWindow->portRect.bottom - gDrawWindow->portRect.top;
	aspect = get_aspect_ratio();
	set_view(2.*aspect,2.,1.*aspect,1.);

    myWindow = gDrawWindow;
}

/************************************************************************
*	Return screen aspect ratio: width/height
************************************************************************/
double	get_aspect_ratio(void)
{
	return (double) gDrawwidth/(double) gDrawheight;
}

/************************************************************************
*	Define view coordinate system for draw_line(), draw_circle(), etc.
************************************************************************/
void	set_view(double width,double height,double x,double y)
{
	gViewwidth = width;
	gViewheight = height;
	gViewx = x;
	gViewy = y;
}

/************************************************************************
*	Transform view coordinate x to screen coordinates
************************************************************************/
static int	transform_x(double x)
{
	return (int) ((x+gViewx)*(double) gDrawwidth/gViewwidth);
}

/************************************************************************
*	Transform view coordinate y to screen coordinates
************************************************************************/
static int	transform_y(double y)
{
	return (int) ((gViewheight-gViewy-y)*(double) gDrawheight/gViewheight);
}

/************************************************************************
*	draw_line() is used to draw lines using view coordinates.
************************************************************************/
void		draw_line(double x1,double y1,double x2,double y2)
{
	int		screen_x1,
			screen_y1,
			screen_x2,
			screen_y2;

	screen_x1 = transform_x(x1);
	screen_y1 = transform_y(y1);
	screen_x2 = transform_x(x2);
	screen_y2 = transform_y(y2);
	
	MoveTo(screen_x1,screen_y1);
	LineTo(screen_x2,screen_y2);
}

/************************************************************************
*	Move present pen position to new position using view
*	coordinates.  Nothing is drawn.
************************************************************************/
void	move_to(double x,double y)
{
	int		screen_x,
			screen_y;
			
	screen_x = transform_x(x);
	screen_y = transform_y(y);
	
	MoveTo(screen_x,screen_y);
	return;
}

/************************************************************************
*	Draw from present pen position to new position using view
*	coordinates.
************************************************************************/
void	draw_to(double x,double y)
{
	int		screen_x,
			screen_y;
			
	screen_x = transform_x(x);
	screen_y = transform_y(y);
	
	LineTo(screen_x,screen_y);
	return;
}

/************************************************************************
*	draw_circle() draws a circle using the Mac Toolbox routines.  It
*	accepts view coordinates.
************************************************************************/
void draw_circle(double x,double y,double r)
{
	Rect	myRect,
			*myRectPtr;
	int		screen_x,
			screen_y,
			screen_r;
			
	screen_x = transform_x(x);
	screen_y = transform_y(y);
	screen_r = (int) (r*(double) gDrawwidth/gViewwidth);
	
	myRectPtr = &myRect;
	
	myRectPtr->left = screen_x-screen_r;
	myRectPtr->right = screen_x+screen_r;
	myRectPtr->top = screen_y-screen_r;
	myRectPtr->bottom = screen_y+screen_r;
	
	FrameOval(&myRect);
	
	circle_rect = myRect;
	
	return;	 	
}

/************************************************************************
*	fill_circle() draws a circle using the Mac Toolbox routines.  It
*	accepts view coordinates.  The circle is filled with the present
*	pen color
************************************************************************/
void fill_circle(double x,double y,double r)
{
	Rect	myRect,
			*myRectPtr;
	int		screen_x,
			screen_y,
			screen_r;
			
	screen_x = transform_x(x);
	screen_y = transform_y(y);
	screen_r = (int) (r*(double) gDrawwidth/gViewwidth);
	
	myRectPtr = &myRect;
	
	myRectPtr->left = screen_x-screen_r;
	myRectPtr->right = screen_x+screen_r;
	myRectPtr->top = screen_y-screen_r;
	myRectPtr->bottom = screen_y+screen_r;
	
	PaintOval(&myRect);

	circle_rect = myRect;
	
	return;	 	
}

/************************************************************************
*	mouse_button_is_down() checks whether the mouse button is down,
*	returns 1 if so, 0 if not.
************************************************************************/
int		mouse_button_is_down(void)
{
	return Button();
}

/************************************************************************
*	get_mouse_location returns the mouse coordinates in terms of the 
*	view coordinate system.  It calls the appropriate Mac Toolbox
*	function to get the Mac screen coordinates of the mouse, and uses
*	the inverse transformation of that in transform_x() and transform_y().
************************************************************************/
void	get_mouse_location(double *x_ptr,double *y_ptr)
{
	Point		mouseLoc;
	
	GetMouse(&mouseLoc);
	*x_ptr = (double) mouseLoc.h/(double) gDrawwidth*gViewwidth-gViewx;
	*y_ptr = gViewheight-gViewy-gViewheight*
			(double) mouseLoc.v/(double) gDrawheight;

	return;
}
	
/************************************************************************
*	wait until button is pressed
************************************************************************/
void	wait(void)
{
	while (!Button())
		;
}

# ifdef SYNC
/************************************************************************
*	Task
************************************************************************/
pascal void Task(void)
{
	long		oldA5;
	MYVBLTASK	vbltask;
	
	asm
	{
		move.l	A5,oldA5	; save off the old value of A5
		move.l	-4(A0), A5	; A0 points to our VBL task record and ear...
							; save CurrentA5 near it
	}
	
	vCounter++;
	mytask.task.vblCount = 1;	// we want to run again real soon
	
	asm
	{
		move.l	oldA5,A5
	}
}

/************************************************************************
*	InstallSync
************************************************************************/
void	InstallSync(void)
{
	GetVideoDefault(&video);
	
	mytask.goodA5 = (long) CurrentA5;
	vCounter = 0;
	
	mytask.task.qType = vType;
	mytask.task.vblAddr = (ProcPtr) Task;
	mytask.task.vblCount = 1;
	mytask.task.vblPhase = 0;
	SlotVInstall((QElemPtr) &mytask.task,video.sdSlot);
}

/************************************************************************
*	RemoveSync
************************************************************************/
void	RemoveSync(void)
{
	SlotVRemove((QElemPtr) &mytask.task,video.sdSlot);
}

/************************************************************************
*	WaitForSync
************************************************************************/
void	WaitForSync(void)
{
	long	vJunk = vCounter;
	
	do {} while (vCounter==vJunk);
}
# endif



	
