/* Copyright 1994 Ralph Gonzalez */

/*
*	FILE:		xgraph.c
*	AUTHOR:		R. Gonzalez, partially adapted from code by Don Snow
*	CREATED:	April 16, 1993
*
*	xgraph.c contains several X Window routines to supplement
*	a typical stdio-type application with a graphics window.
*	Your program must #include "xgraph.h" and must call
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
*/

# include	"xgraph.h"
# include	<stdio.h>
# include	<stdlib.h>
# include	<string.h>
# include	<X11/Xlib.h>
# include	<X11/Xutil.h>

/*---------------------------------------------------------------------*/
/*	The following #define's may be adjusted to your taste: */
/*---------------------------------------------------------------------*/
# define	TITLE				"Graphics"
# define	MOVABLE					/* delete this for immobile window */
# define	BACKGROUND			BLACK
# define	GRAPH_WIDTH			1.	/* Window sizing & placement in */
# define	GRAPH_HEIGHT		1.	/* normalized screen coords:    */
# define	GRAPH_CENTER_X		0.	/* center is (0,0), width is 2. */
# define	GRAPH_CENTER_Y		0.
/*---------------------------------------------------------------------*/
/*	You can change these too if you want: */
/*---------------------------------------------------------------------*/
# define	NORM_WIDTH			2.	/* define normalized screen coords */
									/* height depends on aspect ratio */
# define	NORM_CENTER_X		0.
# define	NORM_CENTER_Y		0.
# define	VIEW_WIDTH			2.	/* define normalized window coords */
# define	VIEW_HEIGHT			2.
# define	VIEW_CENTER_X		0.
# define	VIEW_CENTER_Y		0.
/*---------------------------------------------------------------------*/

# define	BORDER_WIDTH		0
/* X has alot more colors, but we're only using 8 */
# define	maxPixels 			8

/* globals for graphics coordinate system */
static double		gGraphWidth,
					gGraphHeight,
					gGraphX,
					gGraphY,
					gViewWidth,
					gViewHeight,
					gViewX,
					gViewY;

static int			transform_x(double),
					transform_y(double);

/* globals for XWindow implementation */
static Display			*gDisplay;
static int	 			gDepth;
static int				gScreen;
static unsigned long	gBlackPixel;
static unsigned long	gWhitePixel;
static unsigned long	gPixels[maxPixels];
static char				*gColorNames[maxPixels] = 
	{"Black","White","Red","Yellow","Green","Blue","Cyan","Magenta"};
static Colormap			gColormap;
static GC				gGC;
static Window			gGraphWindow;
static int				gPenX=0,gPenY=0;

/* globals for screen coordinate system (for window sizing) */
static double	gDeviceWidth,
				gDeviceHeight,
				gDeviceCenterX,
				gDeviceCenterY,
				gNormalDeviceWidth,
				gNormalDeviceHeight,
				gNormalDeviceCenterX,
				gNormalDeviceCenterY;

/************************************************************************
*	You must call init_graphics() at the beginning of main().
************************************************************************/
void	init_graphics(void)
{
	Window			rootDummy;
	XColor			theRGBColor, theHardwareColor;
	XGCValues		theGCValues;
	int	       		theStatus;
	XSizeHints		theSizeHints;
	XWMHints		theWMHints;
	XSetWindowAttributes	theWindowAttributes;
	unsigned long	theWindowAttributesMask;
	int				i;
	double			ratio,
					temp;
	int				left,
					top;
	unsigned int	width,
					height,
					borderDummy,
					depthDummy;

	gDisplay = XOpenDisplay(NULL);
	if (gDisplay == NULL)
    {
		printf("SCREEN ERROR: Cannot connect to the X Server.\n");
		return;
    }
	gScreen = DefaultScreen(gDisplay);
	gDepth = DefaultDepth(gDisplay,gScreen);
	gBlackPixel = BlackPixel(gDisplay,gScreen);
	gWhitePixel = WhitePixel(gDisplay,gScreen);
	gColormap = DefaultColormap(gDisplay,gScreen);

	/* initalize the color array */

	if (gDepth > 1)
	{
		for (i=0; i < maxPixels; i++)
		{
			theStatus = XLookupColor(gDisplay,gColormap,gColorNames[i],
				   &theRGBColor,&theHardwareColor);
			if (theStatus != 0)
			{
				theStatus = XAllocColor(gDisplay,gColormap,
				      &theHardwareColor);
				if (theStatus != 0)
				{
					gPixels[i] = theHardwareColor.pixel;
				}
				else
				{
					gPixels[i] = gBlackPixel;
				}
			}
		}
	}
	else /* initalize color aray for Black and White display */
    {
		for(i=0; i<maxPixels; i++)
		{
			if (strcmp("White", gColorNames[i]) == 0)
			{
				gPixels[i] = gWhitePixel;
			}
			else
			{
				gPixels[i] = gBlackPixel;
			}
		}
	}

	/* Create the GC */

	gGC = XCreateGC(gDisplay,RootWindow(gDisplay,gScreen),
		    (unsigned long) 0,&theGCValues);
  
	if (gGC == 0)
	{
		printf("SCREEN ERROR: Couldn't create a new GC.\n");
		return;
	}

	/* calculate device and normalized screen coord systems */

	gDeviceWidth = DisplayWidth(gDisplay,gScreen);
	gDeviceHeight = -DisplayHeight(gDisplay,gScreen);
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
	left = (int) (gDeviceCenterX + (temp-gNormalDeviceCenterX) * ratio);
	width = (int) (ratio*GRAPH_WIDTH);
	
	temp = GRAPH_CENTER_Y+GRAPH_HEIGHT/2.;
	ratio = gDeviceHeight/gNormalDeviceHeight;
	top = (int) (gDeviceCenterY + (temp-gNormalDeviceCenterY) * ratio);
	height = (int) (-ratio*GRAPH_HEIGHT);
	
	/* create graphics window */

	gGraphWindow =
	XCreateSimpleWindow(gDisplay,RootWindow(gDisplay,gScreen),
			    left,top,width,height,BORDER_WIDTH,
			    gPixels[WHITE],gPixels[BLACK]);
	XStoreName(gDisplay,gGraphWindow,TITLE);
	
# ifndef MOVABLE
	/* override_redirect: tells the window manager running on the */
	/* diplay to leave this window alone (ie no title bar, no */
	/* resizing, ect. */

	theWindowAttributes.override_redirect = TRUE;
	theWindowAttributesMask = CWOverrideRedirect;
	XChangeWindowAttributes(gDisplay,gGraphWindow,
			      theWindowAttributesMask,&theWindowAttributes);
# endif

	/* NormalState: ie not iconified, shrunk, etc */

	theWMHints.initial_state = NormalState;
	theWMHints.flags = StateHint;
	XSetWMHints(gDisplay,gGraphWindow,&theWMHints);

	/* USPostion,USSize: Specifies that the user supplied the */
	/* position and size of the window (possibly as command line */
	/* arguments). This is actually a lie, as we don't want the user */
	/* selecting the position and size of the window, and the WM */
	/* would let them do just that without these specifications. */

	theSizeHints.flags = USPosition | USSize;
	XSetNormalHints(gDisplay,gGraphWindow,&theSizeHints);

	XMapWindow(gDisplay,gGraphWindow);
	XFlush(gDisplay);
      
	/* Set up for input from this window */
      
	XSelectInput(gDisplay,gGraphWindow,ButtonPressMask);

	/* find window coordinates, set normalized window coords */
	
	XGetGeometry(gDisplay,gGraphWindow,&rootDummy,&left,&top,
		&width,&height,&borderDummy,&depthDummy);
	gGraphX = width/2;
	gGraphY = height/2;
	gGraphWidth = width;
	gGraphHeight = -((int) height);	/* can't negate unsigned int */
	
	set_graphics_coords(VIEW_WIDTH,VIEW_HEIGHT,VIEW_CENTER_X,VIEW_CENTER_Y);
    pen_color(WHITE);
    background_color(BLACK);
    erase_graphics();
	
# ifdef MOVABLE
	sleep(1);	/* wait for window manager to kick in */
# endif
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
	XSetForeground(gDisplay,gGC,gPixels[x]);
}

/************************************************************************
*	background_color() sets the background drawing color.
************************************************************************/
void	background_color(color x)
{	
	XSetWindowBackground(gDisplay,gGraphWindow,gPixels[x]); 
}

/************************************************************************
*	bring graphics window to front. 
************************************************************************/
void	graphics_to_front(void)
{
	XRaiseWindow(gDisplay,gGraphWindow); 
}

/************************************************************************
*	erase_graphics() makes the graphics window the background color. 
************************************************************************/
void	erase_graphics(void)
{
	XClearWindow(gDisplay,gGraphWindow);
	XFlush(gDisplay);
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
	
	XDrawLine(gDisplay,gGraphWindow,gGC,window_x1,window_y1,
		window_x2,window_y2);
	XFlush(gDisplay);
	
	gPenX = window_x2;
	gPenY = window_y2;
}

/************************************************************************
*	Move present pen position to new position using view
*	coordinates.  Nothing is drawn.
************************************************************************/
void	move_to(double x,double y)
{
	gPenX = transform_x(x);
	gPenY = transform_y(y);
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
	
	XDrawLine(gDisplay,gGraphWindow,gGC,gPenX,gPenY,window_y,window_y);
	XFlush(gDisplay);
	gPenX =  window_x;
	gPenY =  window_y;
}

/************************************************************************
*	draw_circle() draws a circle using view coordinates.
************************************************************************/
void draw_circle(double center_x,double center_y,double r)
{
	int				window_x,
					window_y,
					window_r,
					start_angle = 0,
					path_angle = 360*64;	/* in 1/64 of degrees */
	unsigned int	width,
					height;
			
	window_x = transform_x(center_x);
	window_y = transform_y(center_y);
	window_r = (int) (r*gGraphWidth/gViewWidth);
	window_x = window_x - window_r;
	window_y = window_y - window_r;
	width = window_r*2;
	height = width;	
	
	XDrawArc(gDisplay,gGraphWindow,gGC,window_x,window_y,width,height,
		start_angle,path_angle);
	XFlush(gDisplay);
	gPenX =  window_x;
	gPenY =  window_y;
}

/************************************************************************
*	fill_circle() draws a circle using view coordinates.  The circle
*	is filled with the present pen color
************************************************************************/
void fill_circle(double center_x,double center_y,double r)
{
	int				window_x,
					window_y,
					window_r,
					start_angle = 0,
					path_angle = 360*64;	/* in 1/64 of degrees */
	unsigned int	width,
					height;
			
	window_x = transform_x(center_x);
	window_y = transform_y(center_y);
	window_r = (int) (r*gGraphWidth/gViewWidth);
	window_x = window_x - window_r;
	window_y = window_y - window_r;
	width = window_r*2;
	height = width;	
	
	XFillArc(gDisplay,gGraphWindow,gGC,window_x,window_y,width,height,
		start_angle,path_angle);
	XFlush(gDisplay);
	gPenX =  window_x;
	gPenY =  window_y;
}

/************************************************************************
*	mouse_button_is_down() checks whether the mouse button is down,
*	returns TRUE or FALSE.
************************************************************************/
boolean		mouse_button_is_down(void)
{
	XEvent	theEventDummy;
	
	if (XPending(gDisplay))
	{
		XNextEvent(gDisplay,&theEventDummy);
		return TRUE;
	}
	else
		return FALSE;
}

/************************************************************************
*	wait until button is pressed
************************************************************************/
void	wait(void)
{
	XEvent	theEventDummy;
  
	XNextEvent(gDisplay,&theEventDummy);
}

/************************************************************************
*	get_mouse_location returns the mouse coordinates in terms of the 
*	view coordinate system. It uses the inverse transformation of that
*	in transform_x() and transform_y().
************************************************************************/
void	get_mouse_location(double *x_ptr,double *y_ptr)
{
	static Window	dummy_root,dummy_child; /* static so we don't have
											to allocate them repeatedly */
	boolean			in_screen;
	int				root_x,
					root_y,
					x,
					y;
	unsigned int	button_state;
	
	in_screen = XQueryPointer(gDisplay,gGraphWindow,&dummy_root,
		&dummy_child,&root_x,&root_y,&x,&y,&button_state);
	
	if (in_screen)
	{
		*x_ptr = gViewX+(x-gGraphX)*gViewWidth/gGraphWidth;
		*y_ptr = gViewY+(y-gGraphY)*gViewHeight/gGraphHeight;
	}
	else
	{
		*x_ptr = 0.;
		*y_ptr = 0.;
	}

	return;
}

