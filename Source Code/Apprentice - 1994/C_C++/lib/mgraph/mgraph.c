/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		mgraph.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	March 19, 1990
*	MODIFIED:	March 15, 1994
*				Adapted macgraph.c to agree with xgraph version
*
*   PROJECT CONTENTS:
*   yourfiles.c, mgraph.c, ANSI, MacTraps libraries, MacHeaders option.
*	
*	mgraph.c contains several Macintosh routines to supplement
*	a typical stdio-type application with a graphics window, so that
*	you don't have to deal directly with the Macintosh Toolbox (see
*	below.) Your program must #include "mgraph.h" and must call
*	init_graphics() first.
*	
*	The device coordinate system (usually given in pixels) is norm-
*	alized to -1 to 1 horizontally by default. The vertical limits
*	depend on the screen's aspect ratio. This is used for placing and
*	sizing the graphics window.
*
*	The window's own view coordinate system (which is used for
*	drawing) is also normalized, by default to -1 to 1 horizontally
*	and vertically. 
*
*	(The Macintosh Toolbox is a set of routines in ROM to produce the
*	graphical user interface.  These Toolbox functions are declared in
*	MacHeaders.  They are distinguished from my own functions by
*	beginning with a capital letter.)
*/

# include	"mgraph.h"
# include	<stdio.h>

/*---------------------------------------------------------------------*/
/*	The following #define's may be adjusted to your taste: */
/*---------------------------------------------------------------------*/
# define	TITLE				"\pGraphics"
/* # define	MOVABLE */					/* use this if your program handles window moves */
# define	BACKGROUND			BLACK	/* this is unused, see background_color(), below */
# define	GRAPH_WIDTH			1.	/* Window sizing & placement in 	 */
# define	GRAPH_HEIGHT		1.	/* normalized screen coords (below). */
# define	GRAPH_CENTER_X		0.	
# define	GRAPH_CENTER_Y		0.

/*---------------------------------------------------------------------*/
/*	You can change these too if you want: */
/*---------------------------------------------------------------------*/
# define	NORM_WIDTH			2.	/* define normalized screen coords */
									/* height depends on aspect ratio */
# define	NORM_CENTER_X		0.
# define	NORM_CENTER_Y		0.
# define	VIEW_WIDTH			2.	/* define normalized window coords for drawing */
# define	VIEW_HEIGHT			2.
# define	VIEW_CENTER_X		0.
# define	VIEW_CENTER_Y		0.
/*---------------------------------------------------------------------*/

/*	These represent the dimensions of the graphics window given in
*	actual window coordinates. Also the window coords of its center:
*/
static double		gGraphWidth,	
					gGraphHeight,
					gGraphX,
					gGraphY;
					
/*	These represent the dimensions of normalized window coordinate
*	system used for drawing, and coords of its center:
*/
static double		gViewWidth,		
					gViewHeight,
					gViewX,
					gViewY;

static int			transform_x(double),
					transform_y(double);

/* globals for Macintosh implementation */
static WindowPtr	gGraphWindow,
					gConsoleWindow;
static CursHandle	gCursor;
static color		gPresentcolor;
# define			NIL_POINTER			0L
# define			MOVE_TO_FRONT		-1L
# define			REMOVE_ALL_EVENTS	0
# define			VISIBLE				1
# define			NO_GO_AWAY			0
# define			NIL_REF_CON			NIL_POINTER

/*	These represent the dimensions of the physical display device in
*	device coords, and the device coords of its center:
*/
static double		gDeviceWidth,	
					gDeviceHeight,
					gDeviceCenterX,
					gDeviceCenterY;
					
/*	These represent the dimensions of the normalized device coordinate
*	system, used for window sizing. The desired coords for the center
*	of the device are also given:
*/
static double		gNormalDeviceWidth,	
					gNormalDeviceHeight,
					gNormalDeviceCenterX,
					gNormalDeviceCenterY;

/************************************************************************
*	You must call init_graphics() at the beginning of main().
*	Likely don't need all these initializations; they are taken from Mark
*	& Reed's "Macintosh Programming Primer", Addison-Wesley, 1989
************************************************************************/
void	init_graphics(void)
{
	Rect		rectangle;
	double		ratio,
				temp;
	int			left,
				top,
				width,
				height;
	
/*	Need a stdio function call to produce the generic console environment
*	BEFORE the remaining inits:
*/
	printf("\n");
	gConsoleWindow = FrontWindow();
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,REMOVE_ALL_EVENTS);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL_POINTER);
	InitCursor();
	
	/* calculate device and normalized screen coord systems */
	/* Note Macs use inverted y axis. This makes gDeviceHeight negative, */
	/* which compensates the remaining equations automatically. */

	gDeviceWidth = screenBits.bounds.right - screenBits.bounds.left;
	gDeviceHeight = screenBits.bounds.top - screenBits.bounds.bottom;	
	gDeviceCenterX = gDeviceWidth/2;
	gDeviceCenterY = -gDeviceHeight/2;
	
	ratio = get_screen_aspect_ratio();
	
	gNormalDeviceWidth = NORM_WIDTH;
	gNormalDeviceHeight = gNormalDeviceWidth/ratio;
	gNormalDeviceCenterX = NORM_CENTER_X;
	gNormalDeviceCenterY = NORM_CENTER_Y;

	/* convert window location from normalized to device coords */
	
	temp = GRAPH_CENTER_X-GRAPH_WIDTH/2.;
	ratio = gDeviceWidth/gNormalDeviceWidth;
	left = gDeviceCenterX + (temp-gNormalDeviceCenterX) * ratio;
	width = (ratio*GRAPH_WIDTH);
	
	temp = GRAPH_CENTER_Y+GRAPH_HEIGHT/2.;
	ratio = gDeviceHeight/gNormalDeviceHeight;
	top = (int) (gDeviceCenterY + (temp-gNormalDeviceCenterY) * ratio);
	height = (int) (-ratio*GRAPH_HEIGHT);

	SetRect(&rectangle,left,top,left+width,top+height);
	
	/* create graphics window */

# ifdef MOVABLE
 	   gGraphWindow = NewWindow(NIL_POINTER,&rectangle,TITLE,
  	      VISIBLE,noGrowDocProc,(WindowPtr) MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);
# else
	    gGraphWindow = NewWindow(NIL_POINTER,&rectangle,TITLE,
 	       VISIBLE,plainDBox,(WindowPtr) MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);
# endif
	SetPort(gGraphWindow);

	gGraphX = width/2;
	gGraphY = height/2;
	gGraphWidth = width;
	gGraphHeight = -height;
	
	set_graphics_coords(VIEW_WIDTH,VIEW_HEIGHT,VIEW_CENTER_X,VIEW_CENTER_Y);
    pen_color(WHITE);
    background_color(BLACK);
    erase_graphics();
}

/************************************************************************
*	Return device aspect ratio: width/height
************************************************************************/
double	get_screen_aspect_ratio(void)
{
	double	ratio;
	
	ratio = gDeviceWidth/gDeviceHeight;
	if (ratio < 0.)
		ratio = -ratio;
		
	return ratio;
}

/************************************************************************
*	Define view coordinate system for draw_line(), draw_circle(), etc.
************************************************************************/
void	set_graphics_coords(double width,double height,double x,double y)
{
	gViewWidth = width;
	gViewHeight = height;
	gViewX = x;
	gViewY = y;
}

/************************************************************************
*	Transform view coordinate x to window coordinates
************************************************************************/
static int	transform_x(double x)
{
	return (int) (gGraphX+(x-gViewX)*gGraphWidth/gViewWidth);
}

/************************************************************************
*	Transform view coordinate y to window coordinates
************************************************************************/
static int	transform_y(double y)
{
	return (int) (gGraphY+(y-gViewY)*gGraphHeight/gViewHeight);
}

/************************************************************************
*	pen_color() sets the current drawing color. 
************************************************************************/
void	pen_color(color x)
{	
	gPresentcolor = x;
	
	switch (x)
	{
		case 0:	ForeColor(blackColor);
				break;
		case 1:	ForeColor(whiteColor);
				break;
		case 2:	ForeColor(redColor);
				break;
		case 3:	ForeColor(yellowColor);
				break;
		case 4:	ForeColor(greenColor);
				break;
		case 5:	ForeColor(blueColor);
				break;
		case 6:	ForeColor(cyanColor);
				break;
		case 7:	ForeColor(magentaColor);
				break;
		default:	break;
	}
	return;
}

/************************************************************************
*	background_color() sets the background drawing color.
************************************************************************/
void	background_color(color x)
{
	switch (x)
	{
		case 0:	BackColor(blackColor);
				break;
		case 1:	BackColor(whiteColor);
				break;
		case 2:	BackColor(redColor);
				break;
		case 3:	BackColor(yellowColor);
				break;
		case 4:	BackColor(greenColor);
				break;
		case 5:	BackColor(blueColor);
				break;
		case 6:	BackColor(cyanColor);
				break;
		case 7:	BackColor(magentaColor);
				break;
		default:	break;
	}
	return;
}

/************************************************************************
*	erase_graphics() makes the graphics window the background color. 
************************************************************************/
void	erase_graphics(void)
{
	EraseRect(&(gGraphWindow->portRect));

	return;
}

/************************************************************************
*	bring graphics window to front. 
************************************************************************/
void	graphics_to_front(void)
{
	SelectWindow(gGraphWindow);
}

/************************************************************************
*	draw_line() is used to draw lines using view coordinates.
************************************************************************/
void		draw_line(double x1,double y1,double x2,double y2)
{
	int		window_x1,
			window_y1,
			window_x2,
			window_y2;

	window_x1 = transform_x(x1);
	window_y1 = transform_y(y1);
	window_x2 = transform_x(x2);
	window_y2 = transform_y(y2);
	
	MoveTo(window_x1,window_y1);
	LineTo(window_x2,window_y2);
}

/************************************************************************
*	Move present pen position to new position using view
*	coordinates.  Nothing is drawn.
************************************************************************/
void	move_to(double x,double y)
{
	int		window_x,
			window_y;
			
	window_x = transform_x(x);
	window_y = transform_y(y);
	
	MoveTo(window_x,window_y);
	return;
}

/************************************************************************
*	Draw from present pen position to new position using view
*	coordinates.
************************************************************************/
void	draw_to(double x,double y)
{
	int		window_x,
			window_y;
			
	window_x = transform_x(x);
	window_y = transform_y(y);
	
	LineTo(window_x,window_y);
	return;
}

/************************************************************************
*	draw_circle() draws a circle using view coordinates.
************************************************************************/
void draw_circle(double center_x,double center_y,double r)
{
	Rect	myRect,
			*myRectPtr;
	int		window_x,
			window_y,
			window_r;
			
	window_x = transform_x(center_x);
	window_y = transform_y(center_y);
	window_r = (int) (r*(double) gGraphWidth/gViewWidth);
	
	myRectPtr = &myRect;
	
	myRectPtr->left = window_x-window_r;
	myRectPtr->right = window_x+window_r;
	myRectPtr->top = window_y-window_r;
	myRectPtr->bottom = window_y+window_r;
	
	FrameOval(&myRect);
	return;	 	
}

/************************************************************************
*	fill_circle() draws a circle using view coordinates.  The circle
*	is filled with the present pen color
************************************************************************/
void fill_circle(double center_x,double center_y,double r)
{
	Rect	myRect,
			*myRectPtr;
	int		window_x,
			window_y,
			window_r;
			
	window_x = transform_x(center_x);
	window_y = transform_y(center_y);
	window_r = (int) (r*(double) gGraphWidth/gViewWidth);
	
	myRectPtr = &myRect;
	
	myRectPtr->left = window_x-window_r;
	myRectPtr->right = window_x+window_r;
	myRectPtr->top = window_y-window_r;
	myRectPtr->bottom = window_y+window_r;
	
	PaintOval(&myRect);
	return;	 	
}

/************************************************************************
*	mouse_button_is_down() checks whether the mouse button is down,
*	returns TRUE or FALSE.
************************************************************************/
boolean		mouse_button_is_down(void)
{
	return Button();
}

/************************************************************************
*	get_mouse_location returns the mouse coordinates in terms of the 
*	view coordinate system. It uses the inverse transformation of that
*	in transform_x() and transform_y().
************************************************************************/
void	get_mouse_location(double *x_ptr,double *y_ptr)
{
	Point		mouseLoc;
	int			x,
				y;
	
	GetMouse(&mouseLoc);
	x = mouseLoc.h;
	y = mouseLoc.v;
	*x_ptr = gViewX+(x-gGraphX)*gViewWidth/gGraphWidth;
	*y_ptr = gViewY+(y-gGraphY)*gViewHeight/gGraphHeight;

	return;
}
	
/************************************************************************
*	wait until button is pressed
************************************************************************/
void	wait(void)
{
	while (!mouse_button_is_down())
		;
}
