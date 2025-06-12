/*----------------------------------------------------------------------*\
|   
|   FILE: 	XWindow_Screen.cc
|   AUTHOR: 	Donald C. Snow
|   CREATED:	Fall, 1991
|	MODIFIED:	Sept. 23, 1993 (constructor and new_window())
|          
\*----------------------------------------------------------------------*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "xscrn.h"
#include "error.h"
#include "frame.h"

extern Error *gerror;

/*----------------------------------------------------------------------*\
|   
|       XWindow Screen methods...
|
\*----------------------------------------------------------------------*/




/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::X_Screen
|
|   ALGORITHM:
|       1) Open Display
|       2) Initalize colors
|       3) Create the Graphic Context
|       4) Calculate device_frame and normalized_frame
|
\*----------------------------------------------------------------------*/


X_Screen::X_Screen(void)
{
  double	x,y;
  int		width,height;
  XColor	theRGBColor, theHardwareColor;
  XGCValues	theGCValues;
  int	       	theStatus;
  int		i;

  theDisplay = XOpenDisplay(NULL);
  if (theDisplay == NULL)
    {
      gerror->report("SCREEN ERROR: Cannot connect to the X Server.");
	  initialized = FALSE;
      return;
    }
    
//	The following initializations correct a bug apparent in ANSI C++. 9/23/93 Ralph Gonzalez
	theColorNames[0] = "Black";
	theColorNames[1] = "White";
	theColorNames[2] = "Red";
	theColorNames[3] = "Yellow";
	theColorNames[4] = "Green";
	theColorNames[5] = "Blue";
	theColorNames[6] = "Cyan";
	theColorNames[7] = "Magenta";
	current_window = NULL;
	current_color = NULL;
	penX = 0;
	penY = 0;
//

  theScreen = DefaultScreen(theDisplay);
  theDepth = DefaultDepth(theDisplay,theScreen);
  theBlackPixel = BlackPixel(theDisplay, theScreen);
  theWhitePixel = WhitePixel(theDisplay, theScreen);
  theColormap = DefaultColormap(theDisplay, theScreen);
  current_color = &thePixels[BLACK];

  // initalize the color array  

  if (theDepth > 1)
    {
      for (i=0; i < maxPixels; i++)
	{
	  theStatus = XLookupColor(theDisplay, theColormap, theColorNames[i],
				   &theRGBColor, &theHardwareColor);
	  if (theStatus != 0)
	    {
	      theStatus = XAllocColor(theDisplay, theColormap,
				      &theHardwareColor);
	      if (theStatus != 0)
		{
		  thePixels[i] = theHardwareColor.pixel;
		}
	      else
		{
		  thePixels[i] = theBlackPixel;
		}
	    }
	}
    }
  else // initalize color aray for Black and White display 
    {
      for(i=0; i<maxPixels; i++)
	{
	  if (strcmp("White", theColorNames[i]) == 0)
	    {
	      thePixels[i] = theWhitePixel;
	    }
	  else
	    {
	      thePixels[i] = theBlackPixel;
	    }
	}
    }

  // Create the GC 

  theGC = XCreateGC(theDisplay, RootWindow(theDisplay,theScreen),
		    (unsigned long) 0, &theGCValues);
  
  if (theGC == 0)
    {
      gerror->report("SCREEN ERROR: Couldn't create a new GC.");
	  initialized = FALSE;
      return;
    }

  // calculate device_frame and normalized_frame 

  width = DisplayWidth(theDisplay,theScreen);
  height = DisplayHeight(theDisplay,theScreen);
  x = width/2;
  y = height/2;
  device_frame->set(x,y,width,-height);
  normalized_frame->height =
    normalized_frame->width / get_device_aspect_ratio();
}



/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::new_window
|
|   ALGORITHM:
|       1) Convert frame to device_frame
|       2) Create simple X window
|       3) Set some values for the window (explained below)
|       4) Set up to recieve input on the window
|
\*----------------------------------------------------------------------*/


int	X_Screen::new_window(Frame *frame)
{
  int		left,right,top,bottom;
  unsigned int	width,height;
  Coord2	*old_pt,*new_pt;
  
  XSizeHints	theSizeHints;
  XWMHints	theWMHints;

  XSetWindowAttributes	theWindowAttributes;
  unsigned long		theWindowAttributesMask;

  if (num_windows < MAX_WINDOWS)
    {
      old_pt = new Coord2;
      new_pt = new Coord2;

      // Convert frame to device_frame 
      
      old_pt->set(frame->x-frame->width/2.,frame->y-frame->height/2.);
      new_pt->convert(old_pt,normalized_frame,device_frame);
      left =  (int) new_pt->x;
      bottom =  (int) new_pt->y;

      old_pt->set(frame->x+frame->width/2.,frame->y+frame->height/2.);
      new_pt->convert(old_pt,normalized_frame,device_frame);
      right =  (int) new_pt->x;
      top = (int) new_pt->y;

      width = right - left;
      height =  bottom - top;
      
      delete old_pt;
      delete new_pt;

      num_windows++;
      set_current_window(num_windows-1);

      // create simple X window 

      *current_window =
	XCreateSimpleWindow(theDisplay,RootWindow(theDisplay,theScreen),
			    left,top,width,height,BORDER_WIDTH,
			    thePixels[WHITE],thePixels[BLACK]);
      
      // Set some values... 
      
       //	override_redirect: tells the window manager running on the 
       //	diplay to leave this window alone (ie no title bar, no 
       //	resizing, ect. 

      theWindowAttributes.override_redirect = TRUE;
      theWindowAttributesMask = CWOverrideRedirect;
      XChangeWindowAttributes(theDisplay,*current_window,
			      theWindowAttributesMask,&theWindowAttributes);

      // NormalState: ie not iconified, shrunk, ect 

      theWMHints.initial_state = NormalState;
      theWMHints.flags = StateHint;
      XSetWMHints(theDisplay,*current_window,&theWMHints);

       //	USPostion,USSize: Specifies that the user supplied the 
       //	position and size of the window (possibly as command line 
       //	arguments). This is actually a lie, as we don't want the user 
       //	selecting the position and size of the window, and the WM 
       //	would let them do just that without these specifications.

      theSizeHints.flags = USPosition | USSize;
      XSetNormalHints(theDisplay,*current_window,&theSizeHints);

      XMapWindow(theDisplay,*current_window);
      XFlush(theDisplay);
      
      // Set up for input from this window  
      
      XSelectInput(theDisplay, *current_window, ButtonPressMask);
      return(num_windows-1);
    }
  else
    {
      gerror->report("Ran out of windows.");
      return num_windows-1;	// corrected minor bug 9/23/93 - Ralph Gonzalez
    }
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::make_closest
|
|   ALGORITHM:
|       Obvious - XRaiseWindow does just what we want
|
\*----------------------------------------------------------------------*/


void 	X_Screen::make_closest(int window_num)
{
  if (window_num>-1 && window_num<num_windows)
    XRaiseWindow(theDisplay,theWindows[window_num]);
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::get_window_device_frame
|
|   ALGORITHM:
|       1) Get the geometry of the window
|       2) Calculate the device frame from geometry values
|
\*----------------------------------------------------------------------*/


void X_Screen::get_window_device_frame(int window_num, Frame *frame)
{
  Window	rootDummy;
  int		top,left,x,y;
  unsigned int	width,height;
  double	heightcast;
  unsigned int  borderDummy,depthDummy;

  if (window_num > -1 && window_num < num_windows)
    {
       //	We don't need the dummy values, but we need something there for
       //	the function call
      
      XGetGeometry(theDisplay,theWindows[window_num],&rootDummy,&left,&top,
		   &width,&height,&borderDummy,&depthDummy);
      x = width/2;
      y = height/2;
      
       //	Nifty bug was here: we have to cast from an unsigned integer 
       //	before taking the negative. Otherwise, we get an invalid result
      
      heightcast = height;
      frame->set(x,y,width,-heightcast);
    }
  else
    {
      gerror->report("SCREEN ERROR: Illegal window number to make closest.");
    }
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::set_current_window
|
|   ALGORITHM:
|       Obvious - just set the current_window variable
|
\*----------------------------------------------------------------------*/

void X_Screen::set_current_window(int window_num)
{
  if (window_num > -1 && window_num < num_windows)
      current_window = &theWindows[window_num];
  else
    gerror->report("SCREEN ERROR:Illegal window number to set.");
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::set_pen_color
|
|   ALGORITHM:
|       1) Update current_color variable (for fill_window)
|	2) Change foreground color of the GC
|
\*----------------------------------------------------------------------*/


void X_Screen::set_pen_color(color x)
{
  if (current_window != NULL)
    {
      current_color = &thePixels[x];
      XSetForeground(theDisplay, theGC, *current_color);
    }
  else
    gerror->report("SCREEN ERROR: Can't set color with no windows.");
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::fill_window
|
|   ALGORITHM:
|       1) Set the background color 
|       2) Clear the window using this color
|
\*----------------------------------------------------------------------*/


void X_Screen::fill_window(void)
{
  if (current_window != NULL)
    {
      XSetWindowBackground(theDisplay,*current_window,*current_color);
      XClearWindow(theDisplay, *current_window);
      XFlush(theDisplay);
    }
  else
    gerror->report("SCREEN ERROR:Can't fill window with no windows.");
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::move_to
|
|   ALGORITHM:
|       Obvious - just update the pen position variables, penX and penY
|
\*----------------------------------------------------------------------*/


void X_Screen::move_to(Coord2* c)
{
  if (current_window != NULL)
    {
      penX =  (int) c->x;
      penY =  (int) c->y;
    }
  else
    gerror->report("SCREEN ERROR:Can't move_to() with no windows.");
}


/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::draw_line
|
|   ALGORITHM:
|       1) Draw line between specifed coords
|       2) Update penX and penY variables
|
\*----------------------------------------------------------------------*/


void X_Screen::draw_line(Coord2* c1,Coord2* c2)
{
  if (current_window != NULL)
    {
      XDrawLine(theDisplay,*current_window,theGC,(int) c1->x,(int) c1->y,
		(int) c2->x,(int) c2->y);
      XFlush(theDisplay);
      penX = (int) c2->x;
      penY = (int) c2->y;
    }
  else
    gerror->report("SCREEN ERROR: Can't draw_to() with no windows.");
}
  

/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::draw_to
|
|   ALGORITHM:
|       1) Draw line from old penX,penY to coords specified
|       2) Update penX and penY with coords specified
|
\*----------------------------------------------------------------------*/


void X_Screen::draw_to(Coord2* c)
{
  int x,y;

  x = (int) c->x;
  y = (int) c->y;
  
  if (current_window != NULL)
    {
      XDrawLine(theDisplay,*current_window,theGC,penX,penY,x,y);
      XFlush(theDisplay);
      penX =  x;
      penY =  y;
    }
  else
    gerror->report("SCREEN ERROR: Can't draw_to() with no windows.");
}



/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::mouse_button_is_down
|
|   ALGORITHM:
|       1) Check if there is an event pending 
|       2) We only selected for mouse button events, so if there
|          is an event pending, clear it out and return True, else
|          return False.
|  Note: Since events in X are aschyronous, this function should
|  more accurately be called "has mouse button been pressed?" 
|
\*----------------------------------------------------------------------*/


boolean X_Screen::mouse_button_is_down(void)
{
  XEvent	theEventDummy;
  if (XPending(theDisplay))
    {
      XNextEvent(theDisplay,&theEventDummy);
      return(TRUE);
    }
  else
    return(FALSE);
}



/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::wait
|
|   ALGORITHM:
|       Obvious - XNextEvent does just what we want, waits until
|       the next (mouse button) event.
|
\*----------------------------------------------------------------------*/


void X_Screen::wait(void)
{
  XEvent	theEventDummy;
  
  XNextEvent(theDisplay,&theEventDummy);
}



/*----------------------------------------------------------------------*\
|   NAME:
|       X_Screen::~X_Screen
|
|   ALGORITHM:
|       1) Destroy all windows on the display
|       2) Close the Display
|       3) Call inherited destroy method
|
\*----------------------------------------------------------------------*/


X_Screen::~X_Screen(void)
{
  int window_num;

  for (window_num=0; window_num<num_windows; window_num++)
    XDestroyWindow(theDisplay, theWindows[window_num]);
  XCloseDisplay(theDisplay);
}

    



  



      
