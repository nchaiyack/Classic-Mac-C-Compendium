//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		macscrn.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	Oct. 3, 1990
*/

//	Functions beginning with a capital letter and variables	
//	containing capitals are declared in Mac headers, and
//	defined in the Macintosh Toolbox.

# include   "Dialogs.h" // TC4: #include "DialogMgr.h"
# include   "Events.h"  // TC4: #include "EventMgr.h"
# include   "TextEdit.h"
# include   "Memory.h"  // TC4: #include "MemoryMgr.h"
# include   "Menus.h"   // TC4: #include "MenuMgr.h"
# include   "Quickdraw.h"
# include   "Windows.h" // TC4: #include "WindowMgr.h"
# include   "Fonts.h"   // TC5 only
# include   "OSEvents.h"    // TC5 only
# include   "LoMem.h"   // TC5 only

# define	NIL_POINTER			0L
# define	MOVE_TO_FRONT		-1L
# define	REMOVE_ALL_EVENTS	0
# define	TITLE				"\p"
# define	VISIBLE				1
# define	NO_GO_AWAY			0
# define	NIL_REF_CON			NIL_POINTER
//	remove following line to retain menu bar.  This is recommended
//	to make it easier to recover from errors when debugging!
# define	NO_MBAR

# include	"macscrn.h"
# include   "error.h"
extern Error    *gerror;

/******************************************************************
*	You must call constructor at the beginning of main().  Likely don't
*	need all these initializations; most are taken from Mark &
*	Reed's "Macintosh Programming Primer", Addison-Wesley, 1989
******************************************************************/
Mac_Screen::Mac_Screen(void)
{
	double		x,
				y,
				width,
				height;
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,REMOVE_ALL_EVENTS);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL_POINTER);
	InitCursor();
	HideCursor();

# ifdef	NO_MBAR
	old_mbar_height = MBarHeight;
	MBarHeight = 0;
# endif
	
	width = screenBits.bounds.right - screenBits.bounds.left;
	height = screenBits.bounds.top - screenBits.bounds.bottom;
	x = screenBits.bounds.left + width/2.;
	y = screenBits.bounds.bottom + height/2.;
	device_frame->set(x,y,width,height);
	
	normalized_frame->height = 	normalized_frame->width /
		get_device_aspect_ratio();
		
	current_window = NULL;
}

/******************************************************************
*	Add new window to screen
******************************************************************/
int		Mac_Screen::new_window(Frame *frame)
{
	Rect	rectangle;
	int		left,
			right,
			top,
			bottom;
	Coord2	*old_pt,
			*new_pt;
	Rect		mbar_rect;		// to allow window to overlap menu 
	RgnHandle	mbar_rgn;
	WindowPtr	temp_window;
	
	old_pt = new Coord2;
	new_pt = new Coord2;
	
	old_pt->set(frame->x-frame->width/2.,frame->y-frame->height/2.);
	new_pt->convert(old_pt,normalized_frame,device_frame);
	left = new_pt->x;
	bottom = new_pt->y;
	
	old_pt->set(frame->x+frame->width/2.,frame->y+frame->height/2.);
	new_pt->convert(old_pt,normalized_frame,device_frame);
	right = new_pt->x;
	top = new_pt->y;
	
	delete old_pt;
	delete new_pt;
	
	SetRect(&rectangle,left,top,right,bottom);
	
// TC4:     temp_window = NewWindow(NIL_POINTER,&rectangle,TITLE,
//      VISIBLE,plainDBox,MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);

    temp_window = NewWindow(NIL_POINTER,&rectangle,TITLE,
        VISIBLE,plainDBox,(WindowPtr) MOVE_TO_FRONT,NO_GO_AWAY,NIL_REF_CON);

# ifdef	NO_MBAR
	if (top < screenBits.bounds.top+old_mbar_height)
	{
		if (bottom < screenBits.bounds.top+old_mbar_height)
			SetRect(&mbar_rect,0,0,right-left,bottom-top);
		else
			SetRect(&mbar_rect,0,0,right-left,
				screenBits.bounds.top+old_mbar_height-top);
		mbar_rgn = NewRgn();
		RectRgn(mbar_rgn,&mbar_rect);
		UnionRgn(temp_window->visRgn,mbar_rgn,temp_window->visRgn);
		DisposeRgn(mbar_rgn);
	}
# endif

	if (num_windows < MAX_WINDOWS)
	{
		window[num_windows++] = temp_window;
		set_current_window(num_windows-1);
		return num_windows-1;
	}
	else
	{
		gerror->report("Ran out of windows");
		DisposeWindow(temp_window);
		return num_windows-1;
	}
}

/******************************************************************
*	Bring window to front.
******************************************************************/
void	Mac_Screen::make_closest(int window_num)
{
	if (window_num > -1 && window_num < num_windows)
		SelectWindow(window[window_num]);
	else
		gerror->report("Illegal window number");
}

/******************************************************************
*	Get coordinate frame of window in device coordinates.
******************************************************************/
void	Mac_Screen::get_window_device_frame(int window_num,
											Frame *frame)
{
	double	x,
			y,
			width,
			height;
			
	if (window_num > -1 && window_num < num_windows)
	{
		width =		window[window_num]->portRect.right -
					window[window_num]->portRect.left;
		height =	window[window_num]->portRect.top -
					window[window_num]->portRect.bottom;
		x = 		window[window_num]->portRect.left + width/2.;
		y = 		window[window_num]->portRect.bottom + height/2.;
		frame->set(x,y,width,height);
	}
	else
		gerror->report("Illegal window number");
}

/******************************************************************
*	Sets the current drawing window.
******************************************************************/
void	Mac_Screen::set_current_window(int window_num)
{
	if (window_num > -1 && window_num < num_windows)
	{
		current_window = window[window_num];
		SetPort(current_window);
	}
	else
		gerror->report("Illegal window number");
}

/******************************************************************
*	sets the current drawing color.  The colors are defined
*	in "Quickdraw.h".  Call set_current_window() first!
******************************************************************/
void	Mac_Screen::set_pen_color(color x)
{
	if (current_window != NULL)
		switch ((int) x)
		{
			case 0:		ForeColor(blackColor);
						break;
			case 1:		ForeColor(whiteColor);
						break;
			case 2:		ForeColor(redColor);
						break;
			case 3:		ForeColor(yellowColor);
						break;
			case 4:		ForeColor(greenColor);
						break;
			case 5:		ForeColor(blueColor);
						break;
			case 6:		ForeColor(cyanColor);
						break;
			case 7:		ForeColor(magentaColor);
						break;
			default:	break;
		}
	else
		gerror->report("Can't set color with no windows");
}

/******************************************************************
*	calls the appropriate Mac Toolbox function to make the window
*	the current color.   Call set_current_window() first!
******************************************************************/
void	Mac_Screen::fill_window(void)
{
	if (current_window != NULL)
		FillRect(&(current_window->portRect),black);
	else
		gerror->report("Can't fill window with no windows");
}

/******************************************************************
*	Move present pen position to new position using device
*	coordinates.  Call set_current_window() first!
******************************************************************/
void	Mac_Screen::move_to(Coord2* c)
{
	if (current_window != NULL)
		MoveTo((int) c->x,(int) c->y);
	else
		gerror->report("Can't move_to() with no windows");
}

/******************************************************************
*	Draw from present pen position to new position using device
*	coordinates.  Call set_current_window() first!
******************************************************************/
void	Mac_Screen::draw_to(Coord2* c)
{
	if (current_window != NULL)
		LineTo((int) c->x,(int) c->y);
	else
		gerror->report("Can't draw_to() with no windows");
}

/******************************************************************
*	mouse_button_is_down() checks whether the mouse button is down,
*	returns TRUE if so, FALSE if not.
******************************************************************/
boolean	Mac_Screen::mouse_button_is_down(void)
{
	return Button();
}

/******************************************************************
*	Destroy screen.
******************************************************************/
Mac_Screen::~Mac_Screen(void)
{
	int		window_num;
	
	for (window_num=0 ; window_num<num_windows ; window_num++)
		DisposeWindow(window[window_num]);

# ifdef	NO_MBAR
	MBarHeight = old_mbar_height;
# endif
}

