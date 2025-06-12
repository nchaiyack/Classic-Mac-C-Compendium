//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		pcscrn.c
*	AUTHOR:		Norman Gaskill
*	CREATED:	Dec. 16, 1990
*/

//--------NOTE: file EGAVGA.BGI must be in current directory--------------

# include <stdio.h>
# define __COLORS         //for THINK_C
# include <conio.h>
# include <graphics.h>    //BORLAND Turbo C++ library of graphics functions
# undef __COLORS          //for THINK_C
# define  LIGHTGRAY  7    //for use by floodfill

int current_window = -1;         //keeps track of current window number
boolean window_defined = FALSE;  //valid window variable
int win_data [MAX_WINDOWS] [6];  //window data array

# include	"pcscrn.h"
# include   "error.h"
extern Error    *gerror;

/******************************************************************
* You must call constructor at the beginning of main().  
******************************************************************/
PC_Screen::PC_Screen(void)
{
  int x,y,width,height;
  int graphdriver = DETECT, graphmode, error_code;

  initgraph(&graphdriver, &graphmode, "..\\bgi"); //Initialize graphics,
  error_code = graphresult();                     //system must be EGA or VGA
  if (error_code != grOk)                         //If no graphics
    return(FALSE);                                //hardware found is found
  if ((graphdriver != EGA) && (graphdriver != VGA))
  {
    closegraph();
    return FALSE;
  }
  width = getmaxx();
  height = getmaxy();
  x = width/2;
  y = height/2;
  device_frame->set(x,y,width,-height);
  normalized_frame->height = normalized_frame->width / 
                             get_device_aspect_ratio();
}

/******************************************************************
* Add new window to screen
******************************************************************/
int PC_Screen::new_window(Frame *frame)
{
  int left,
      right,
      top,
      bottom;
  Coord2  *old_pt,
          *new_pt;
  
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

  if (num_windows < MAX_WINDOWS)
  {
    setviewport(left,top,right,bottom,1); //1=truncate output
    win_data[num_windows][0] = left;
    win_data[num_windows][1] = top;
    win_data[num_windows][2] = right;
    win_data[num_windows][3] = bottom;
    current_window = num_windows;
    num_windows++;
    return num_windows-1;
  }
  else
  {
    gerror->report("Ran out of windows");
    return num_windows;
  }
}

/******************************************************************
* Bring window to front.
******************************************************************/
void  PC_Screen::make_closest(int window_num)
{
  if (window_num > -1 && window_num < num_windows)
  {
    current_window = window_num;
    setviewport(win_data[current_window][0],      //resets viewport
                win_data[current_window][1],
                win_data[current_window][2],
                win_data[current_window][3],1);   //1=truncate output
    setfillstyle(SOLID_FILL, win_data[current_window][4]);  //set and
    floodfill(2,2,LIGHTGRAY);                     //flood fill current window
    setcolor(win_data[current_window][5]);        //reset draw color
  }
  else
    gerror->report("Illegal window number");
}

/******************************************************************
* Get coordinate frame of window in device coordinates.
******************************************************************/
void  PC_Screen::get_window_device_frame(int window_num,Frame *frame)
{
  int x,
      y,  
      width,
      height;
  struct viewporttype viewinfo;

  if (window_num > -1 && window_num < num_windows)
  {
    getviewsettings(&viewinfo);
    width = viewinfo.right - viewinfo.left;
    height = viewinfo.bottom - viewinfo.top;
    x = width/2;
    y = height/2;

    frame->set(x,y,width,-height);
  }
  else
    gerror->report("Illegal window number");
}

/******************************************************************
* Sets the current drawing window.
******************************************************************/
void  PC_Screen::set_current_window(int window_num)
{
  if (window_num > -1 && window_num < num_windows)
  {
    window_defined = TRUE;
    current_window = window_num;
    setviewport(win_data[current_window][0],      //resets viewport
                win_data[current_window][1],
                win_data[current_window][2],
                win_data[current_window][3],1);   //1=truncate output
  }
  else
    gerror->report("Illegal window number");
}

/******************************************************************
* sets the current drawing color.  Call set_current_window() first!
*----------------------------------------------------------------
* Note from R. Gonzalez:  N. Gaskill had difficulty here because
* the Turbo C++ file "graphics.h" conflicted with
* my use of the identifier "color" and/or with my list of color
* constants, as defined in my "color.h".  To get around this
* he had to use "# define __COLORS" and was unable to take advan-
* tage of the names given in "graphics.h".  I think he could have
* gotten around this by momentarily #undef-ing my constants and
* #define-ing constants with new names for the ones given in
* graphics.h.  As it is, however, he is forced to use numerical
* constants below.
******************************************************************/
void  PC_Screen::set_pen_color(int color)
{
  if (window_defined)
  {
    switch (color)                                //correct colors for PC
    {
      case 0:  color = 0;
        break;
      case 1:  color = 15;
        break;
      case 2:  color = 4;
        break;
      case 3:  color = 14;
        break;
      case 4:  color = 2;
        break;
      case 5:  color = 1;
        break;
      case 6:  color = 3;
        break;
      case 7:  color = 5;
        break;
      default: break;
    }
    setcolor(color);                              //set draw color
    setfillstyle(SOLID_FILL, color);              //set flood fill color
    win_data[current_window][5] = color;          //save draw color 
  }
  else
    gerror->report("Can't set color with no windows");
}

/******************************************************************
* Make the window the current color.   Call set_current_window() first!
******************************************************************/
void  PC_Screen::fill_window(void)
{
  if (window_defined)
	{
		clearviewport();                              //clear for next
		floodfill(2,2,LIGHTGRAY);                     //fill window
    win_data[current_window][4] = getcolor();     //save fill color
  }
  else
    gerror->report("Can't fill window with no windows");
}

/******************************************************************
* Move present pen position to new position using device
* coordinates.  Call set_current_window() first!
******************************************************************/
void  PC_Screen::move_to(Coord2* c)
{
  if (window_defined)
    moveto((int) c->x,(int) c->y);
  else
    gerror->report("Can't move_to() with no windows");
}

/******************************************************************
* Draw from present pen position to new position using device
* coordinates.  Call set_current_window() first!
******************************************************************/
void  PC_Screen::draw_to(Coord2* c)
{
  if (window_defined)
    lineto((int) c->x,(int) c->y);
  else
    gerror->report("Can't draw_to() with no windows");
}

/******************************************************************
* look for keyhit.
******************************************************************/
void  PC_Screen::wait(void)
{
  printf("Press any key to exit");
  getch();
}

/******************************************************************
* Destroy screen.
******************************************************************/
PC_Screen::~PC_Screen(void)
{
  int window_num;
  
  closegraph(); 
}

  
