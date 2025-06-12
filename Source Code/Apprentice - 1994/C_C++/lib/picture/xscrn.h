/*----------------------------------------------------------------------*\
|   NAME:
|       xscrn.h
|
|   AUTHOR:
|       Donald C. Snow
|
|   PURPOSE:
|       Definition of the XWindows Screen specific class, to encapsulate
|       macine-specific graphics code.
|
|   HISTORY:
|       Written during the Fall 91 Semester at Rutgers University
|       for an independent study course.
|
|	MODIFIED:	9/23/93 Ralph Gonzalez
|
\*----------------------------------------------------------------------*/


#ifndef xscrn_h
#define xscrn_h


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "screen.h"

#define	BORDER_WIDTH	0

// X has alot more colors, but we're following the code in color.h 
#define maxPixels 	8

/*----------------------------------------------------------------------*\
|       X_Screen class to allow graphics I/O on any XWindows display
\*----------------------------------------------------------------------*/

#define SCREEN X_Screen
class X_Screen:public Generic_Screen
{
private:
  Display 	*theDisplay;
  int 		theDepth;
  int		theScreen;
  unsigned long	theBlackPixel;
  unsigned long	theWhitePixel;

  unsigned long		thePixels[maxPixels];
  char			*theColorNames[maxPixels];	// moved initialization to constructor - Ralph Gonzalez 9/23/93
  Colormap	theColormap;

  Window	theWindows[MAX_WINDOWS];
  GC		theGC;
  
  Window	*current_window;	// moved initialization to constructor - Ralph Gonzalez 9/23/93
  unsigned long	*current_color;	// moved initialization to constructor - Ralph Gonzalez 9/23/93
  int		penX,penY;			// moved initialization to constructor - Ralph Gonzalez 9/23/93
  
public:
  X_Screen(void);
  virtual int		new_window(Frame*);
  virtual void		make_closest(int);
  virtual void		get_window_device_frame(int,Frame*);
  virtual void		set_current_window(int);
  virtual void		set_pen_color(color);
  virtual void		fill_window(void);
  virtual void		move_to(Coord2*);
  virtual void		draw_to(Coord2*);
  virtual void		draw_line(Coord2*,Coord2*);
  virtual boolean	mouse_button_is_down(void);
  virtual void		wait(void);
  virtual			~X_Screen(void);
};
  
#endif
