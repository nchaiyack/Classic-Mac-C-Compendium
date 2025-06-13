/*****
 * spion_world_editor.c
 *
 *	This program is used to create worlds for Spion
 *
 *
 *****/


#include <FixMath.h>
#include <QDOffscreen.h>
#include <Script.h>
#include "spion_world_ed_resources.h"
#include "spion_world_ed_defines.h"


/***************************** Types ****************************/

typedef
enum
	{
	water,
	grass,
	sand,
	ice
	};

typedef
enum
	{
	s64x64 = 64,
	s32x32 = 32,
	s16x16 = 16,
	s8x8 = 8
	};

typedef
struct
	{
	short	width;
	short	height;
	} **world_size_resource;



/***************************** Globals ****************************/

WindowPtr		main_window;		/* the main window */
CWindowRecord	main_window_rec;	/* storage for the main window's info */
WindowPtr		overview_window;	/* the main window */
CWindowRecord	overview_window_rec;/* storage for the main window's info */

short			world_width_blocks;	/* width and height of the world, in blocks */
short			world_height_blocks;

char			*main_window_base;	/* base address of the window */
PixMapHandle	main_window_pixmap;	/* the Pixmap for the main window */
Boolean			screen_pixmap_32_flag;/* TRUE if we need 32-bit mode to write to screen */
long			screen_width;		/* width of screen in bytes */

FSSpec			world_fsspec;		/* File info for current world map file */

Boolean			changes_saved;		/* TRUE if changes to this file have been saved */
Boolean			got_file_data;		/* TRUE if the user has selected a save file */

Fixed			world_h, world_v;

short			num_terrains;

long			grid_lines = 1;		/* 1 if there are gridlines, 0 otherwise */

MenuHandle		terrains_popup_handle;	/* The terrains menu */
MenuHandle		apple_menu;			/* The apple menu */
MenuHandle		file_menu;			/* The file menu */
MenuHandle		edit_menu;			/* The edit menu */
MenuHandle		windows_menu;		/* The windows menu */
MenuHandle		zoom_out_menu;		/* The zoom out menu */
MenuHandle		map_menu;			/* The map menu */

ControlHandle	horiz_scroll_bar;	/* The horizontal scroll bar */
ControlHandle	vert_scroll_bar;	/* The vertical scroll bar */

long			block_size;			/* Size of block we're now using (width and height) */

long			view_height_blocks;	/* Height of the main window, in blocks */
long			view_width_blocks;	/* Width of the main window, in blocks */

long			view_height;		/* Height of the map area */
long			view_width;			/* Width of the map area */

char			*terrains64x64[256];	/* Array of pointers to 64 x 64 terrains */
char			*terrains32x32[256];	/* Array of pointers to 32 x 32 terrains */
char			*terrains16x16[256];	/* Array of pointers to 16 x 16 terrains */
char			*terrains8x8[256];		/* Array of pointers to 8 x 8 terrains */

char			**world_handle = NULL;
char			*world;					/* Pointer to world map data */

char			background_terrain_type = 0;	/* Used to keep track of pen drawing */
char			foreground_terrain_type = 1;

CursHandle		watch_cursor;		/* The watch cursor */

Rect			drag_rect;			/* Permissible drag region */

Rect			overview_frame;		/* Frame of viewing region in overview window */

Boolean			windows_intersect_flag = FALSE;	/* TRUE if the windows intersect */
Boolean			in_background_flag = FALSE;		/* TRUE if we are in the background */

RGBColor		water_color = {2056, 35980, 63479};
RGBColor		sand_color = {65535, 54998, 8481};
RGBColor		grass_color = {10537, 52942, 0};
RGBColor		ice_color = {0, 65535, 65535};
RGBColor		other_color = {35980, 35980, 35980};

/***************************** Prototypes ****************************/

void init_program(void);
void init_menus(void);
void init_windows(void);
void init_graphics_data(void);
void create_offscreen_grafport(CGrafPtr offscreen_ptr, Rect *bounds_rect,
								char **pixmap_base);
void dispose_offscreen_grafport (CGrafPtr offscreen_ptr);
void handle_mouse_down(EventRecord	*event);
void do_menu(long command);
void handle_key_down(EventRecord *event);
void event_loop(void);
void main(void);
void draw_background(void);
void init_graphics_data(void);
void create_offscreen_grafport(CGrafPtr offscreen_ptr, Rect *bounds_rect,
								char **pixmap_base);
void dispose_offscreen_grafport (CGrafPtr offscreen_ptr);
pascal void scroll_action(ControlHandle the_control, short part);
void create_new_world(void);
void open_world(void);
void save_world(void);
void save_as_world(void);
void draw_overview(void);
void draw_overview_pixel(short h, short v, char terrain_type);
void draw_overview_frame(Boolean first_time);
void handle_main_window_click(EventRecord *event);
void handle_overview_click(EventRecord *event);
pascal void draw_frame_action(void);
void set_world_block(short h, short v, char terrain_type);
void handle_update(EventRecord *event);
void handle_activate(EventRecord *event);
void draw_background_carefully(void);
void handle_window_drag(EventRecord *event, WindowPtr window);
void handle_multifinder_event(EventRecord *event);
void recompute_main_window(void);
void check_windows_intersect(void);



/*****************************************************************************\
* procedure init_program                                                      *
*                                                                             *
* Purpose: This procedure initializes the mac managers.                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_program(void)

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
	watch_cursor = GetCursor(watchCursor);
	
} /* init_program() */



/*****************************************************************************\
* procedure init_menus                                                        *
*                                                                             *
* Purpose: This procedure initializes the mac pull-down menus.                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_menus(void)
{

	/* Read the menus in to the menu list */
	InsertMenu(apple_menu = GetMenu(APPLE_MENU), 0);
	InsertMenu(file_menu = GetMenu(FILE_MENU), 0);
	InsertMenu(edit_menu = GetMenu(EDIT_MENU), 0);
	InsertMenu(zoom_out_menu = GetMenu(ZOOM_OUT_MENU), 0);
	InsertMenu(map_menu = GetMenu(MAP_MENU), 0);
	InsertMenu(windows_menu = GetMenu(WINDOWS_MENU), 0);
	DrawMenuBar();	/* Draw the new menu bar */
	
	/* add the desk accessories */
	AddResMenu(apple_menu, 'DRVR');

	/* Load the terrains popup menu */
	terrains_popup_handle = GetMenu(TERRAINS_MENU);

}	/* init_menus() */



/*****************************************************************************\
* procedure init_windows                                                      *
*                                                                             *
* Purpose: This procedure initializes the windows.                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_windows(void)
{

	GDHandle		main_screen;
	PixMapHandle	main_screen_pixmap;
	PixMapHandle	main_window_pixmap;
	Rect			main_window_bounds;
	long			main_window_left;	/* left edge of the window (in bytes from left) */
	long			main_window_top;	/* top edge of the main window */
	Rect			dest_rect;

	/* Open the main window */
	main_window = GetNewCWindow (MAIN_WINDOW, &main_window_rec, (WindowPtr)-1L );
	
	/* Open the overview window (behind the main window) */
	overview_window = GetNewCWindow (OVERVIEW_WINDOW, &overview_window_rec, NULL );

	/* Do the drawing in the main window */
	SetPort(main_window);
	
	/* Ensure that the screen is 8-bit depth */
	main_screen = GetMainDevice ();
	main_screen_pixmap = (*main_screen)->gdPMap;
	if ((*main_screen_pixmap)->pixelSize != 8)
		{
		
		/* Screen is not in 8-bit mode -- abort */
		StopAlert(NOT_8_BIT_MODE, (ProcPtr) 0L);
		ExitToShell();
		
		}

	/* Get the width of the screen, in bytes */
	screen_width = (*main_screen_pixmap)->rowBytes & 0x1FFF;

	/* Find the pixmap for the main window */
	main_window_pixmap = ((CGrafPtr) main_window)->portPixMap;
	
	/* Find the distance from the left edge of the screen to the left edge of
		the window, in longwords */
	main_window_left = -(*main_window_pixmap)->bounds.left;
	
	/* Find the distance from the top of the screen to the top of the window */
	main_window_top = -(*main_window_pixmap)->bounds.top;
	
	/* Find the content rectangle */
	BlockMove (&(main_window->portRect), &main_window_bounds, 8);
	
	/* Find the size of the map region in the main window */
	view_width = main_window_bounds.right - main_window_bounds.left - 16;
	view_height = main_window_bounds.bottom - main_window_bounds.top - 16;

	/* Check whether we need to be in 32-bit mode to access the screen memory */
    screen_pixmap_32_flag = PixMap32Bit(main_window_pixmap);
    
	/* Get the pixmap for the main window */
	main_window_pixmap = ((CGrafPtr) main_window)->portPixMap;

	/* Lock down the screen */
    LockPixels(main_window_pixmap);

	/* Find the base address of the upper left corner of the window */
	main_window_base = ((char *) GetPixBaseAddr(main_window_pixmap)) +
						main_window_top * screen_width + main_window_left;

	/* Install the scroll bars */
	horiz_scroll_bar = GetNewControl (HORIZ_SCROLL_BAR, main_window);
	vert_scroll_bar = GetNewControl (VERT_SCROLL_BAR, main_window);

}	/* init_windows() */



/*****************************************************************************\
* procedure init_graphics_data                                                *
*                                                                             *
* Purpose: This procedure initializes graphics data used by the program by    *
*          reading it into memory from the resource fork.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_graphics_data(void)
{

#define	PIXMAP_WIDTH	64
#define	PIXMAP_HEIGHT	64

	short		i, j, k;
	Handle		offscreen_bitmap_handle, ship_handle, ship_mask_handle;
	Handle		small_terrain_handle, terrain_handle;
	Ptr			ship_pointer, pixmap_ship_pointer, ship_mask_pointer, screen_pat_base;
	PicHandle	picture_handle;
	Rect		dest_rect;
	long		picture_size;
	Ptr			pointer;
	Rect		ship_rect = {0, 0, 33, 33};
	RGBColor	white = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor	rgb_black = {0x0000, 0x0000, 0x0000};
	Rect		port_bounds = {0, 0, PIXMAP_HEIGHT, PIXMAP_WIDTH};
	CGrafPort	offscreen_port;
	char		*pixmap_base;
	
	/* Create an offscreen area to draw these graphics data */
	create_offscreen_grafport(&offscreen_port, &port_bounds, &pixmap_base);
	
	/* Read the terrains into memory */
	for (i = 0; i < 256; i++)
		{

		/* Get the PICT for this terrain */		
		picture_handle = GetPicture(FIRST_TERRAIN_ID + i);
		
		/* If this terrain does not exist, mark it as unused */
		if (!picture_handle)
			{
			terrains64x64[i] = terrains32x32[i] = terrains16x16[i] = terrains8x8[i] = NULL;
			continue;
			}

		/* Draw a 64x64 terrain pattern on the screen */
		SetRect(&dest_rect, 0, 0, 64, 64);
		DrawPicture(picture_handle, &dest_rect);
		
		/* Allocate a blocks of memory for the terrain */
		terrain_handle = NewHandle(64 * 64);
		
		/* Lock down the handle and dereference it for speed */
		HLock(terrain_handle);
		terrains64x64[i] = pointer = *terrain_handle;
		
		/* Find the base address of the terrain in the offscreen pixmap */
		screen_pat_base = pixmap_base;
				
		/* Copy the data from the screen bitmap to the pattern in memory */
		for (j = 0; j < 64; j++)
			{
			/* Move this line */
			BlockMove(screen_pat_base, pointer, 64);
			
			/* Go to next line */
			screen_pat_base += PIXMAP_WIDTH;
			pointer += 64;
			}
		
		/* Draw a 32x32 terrain */
		SetRect(&dest_rect, 0, 0, 32, 32);
		DrawPicture(picture_handle, &dest_rect);
		
		/* Allocate a blocks of memory for the small terrain */
		terrain_handle = NewHandle(32 * 32);

		/* Lock down the handle and dereference it for speed */
		HLock(terrain_handle);
		terrains32x32[i] = pointer = *terrain_handle;
		
		/* Find the base address of the terrain in the offscreen pixmap */
		screen_pat_base = pixmap_base;
				
		/* Copy the data from the screen bitmap to the pattern in memory */
		for (j = 0; j < 32; j++)
			{
			/* Move this line */
			BlockMove(screen_pat_base, pointer, 32);
			
			/* Go to next line */
			screen_pat_base += PIXMAP_WIDTH;
			pointer += 32;
			}
		
		/* Draw a 16x16 terrain */
		SetRect(&dest_rect, 0, 0, 16, 16);
		DrawPicture(picture_handle, &dest_rect);
		
		/* Allocate a blocks of memory for the small terrain */
		terrain_handle = NewHandle(16 * 16);

		/* Lock down the handle and dereference it for speed */
		HLock(terrain_handle);
		terrains16x16[i] = pointer = *terrain_handle;
		
		/* Find the base address of the terrain in the offscreen pixmap */
		screen_pat_base = pixmap_base;
				
		/* Copy the data from the screen bitmap to the pattern in memory */
		for (j = 0; j < 16; j++)
			{
			/* Move this line */
			BlockMove(screen_pat_base, pointer, 16);
			
			/* Go to next line */
			screen_pat_base += PIXMAP_WIDTH;
			pointer += 16;
			}
		
		/* Draw ab 8x8 terrain */
		SetRect(&dest_rect, 0, 0, 8, 8);
		DrawPicture(picture_handle, &dest_rect);
		
		/* Allocate a blocks of memory for the small terrain */
		terrain_handle = NewHandle(8 * 8);

		/* Lock down the handle and dereference it for speed */
		HLock(terrain_handle);
		terrains8x8[i] = pointer = *terrain_handle;
		
		/* Find the base address of the terrain in the offscreen pixmap */
		screen_pat_base = pixmap_base;
				
		/* Copy the data from the screen bitmap to the pattern in memory */
		for (j = 0; j < 8; j++)
			{
			/* Move this line */
			BlockMove(screen_pat_base, pointer, 8);
			
			/* Go to next line */
			screen_pat_base += PIXMAP_WIDTH;
			pointer += 8;
			}
		
		}
		
	/* Throw away the offscreen pixmap */
	dispose_offscreen_grafport(&offscreen_port);
	
	/* Set thePort back to the window */
	SetPort(main_window);
	
}	/* init_graphics_data() */



/*****************************************************************************\
* procedure create_offscreen_grafport                                         *
*                                                                             *
* Purpose: This procedure creates an offscreen grafport which will be used to *
*          convert the PICTs to raw data images.                              *
*                                                                             *                                                                             *
* Parameters: bounds_rect: the size of the desired bitmap.                    *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void create_offscreen_grafport(CGrafPtr offscreen_ptr, Rect *bounds_rect,
								char **pixmap_base)
{

	PixMapHandle	pixmap;
	short			error;
	RGBColor		rgb_color;
	static CGrafPtr	image_port;
	
	/* Open the new port */
	OpenCPort (offscreen_ptr);
	
	/* Get a pointer to the pixel map */
	pixmap = offscreen_ptr->portPixMap;
	HLock(pixmap);
	
	/* create a bitmap of the desired size */
	offscreen_ptr->portRect = *bounds_rect;
	(*pixmap)->bounds = *bounds_rect;
	RectRgn(offscreen_ptr->clipRgn, bounds_rect);
	RectRgn(offscreen_ptr->visRgn, bounds_rect);
	(*pixmap)->rowBytes = (bounds_rect->right - bounds_rect->left);
	
	/* allocate space for the bitmap */
	*pixmap_base = (*pixmap)->baseAddr = NewPtr ((*pixmap)->rowBytes *
										(long) (bounds_rect->bottom - bounds_rect->top) );
	
	/* Set the 15th bit of rowBytes, so this is a PixMap */
	(*pixmap)->rowBytes |= 32768;

	/* Clear the bitmap to white */
	EraseRect(bounds_rect);
	InvertRect(bounds_rect);

}	/*	create_offscreen_grafport()	*/



/*****************************************************************************\
* procedure dispose_offscreen_port                                            *
*                                                                             *
* Purpose: This procedure gets rid of the offscreen grafport created by       *
*          create_offscreen_grafport.                                         *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void dispose_offscreen_grafport (CGrafPtr offscreen_ptr)
{

	/* free the memory the bitmap used */
	DisposPtr ((*(offscreen_ptr->portPixMap))->baseAddr);

	/* close the port */
	ClosePort (offscreen_ptr);

}	/* dispose_offscreen_port() */



/*****************************************************************************\
* procedure handle_mouse_down                                                 *
*                                                                             *
* Purpose: This procedure handles a mouse click.                              *
*                                                                             *
* Parameters: event: the mouseDown event.                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_mouse_down(EventRecord	*event)

{

	WindowPtr		window;
	short			part = FindWindow (event->where, &window);
	Rect			size_rect = {50, 150, 10000, 10000};
	long			new_size;
	
	switch (part)
		{
		case inSysWindow: 
			SystemClick (event, window);
			break;
	    
		case inMenuBar:
			do_menu(MenuSelect(event->where));
			break;
	    
		case inDrag:
	  
	  		handle_window_drag(event, window);

			break;
		
		case inGrow:
	  
	  		/* Grow the window */
	  		new_size = GrowWindow(window, event->where, &size_rect);
	  		SizeWindow(window, LoWord(new_size), HiWord(new_size), TRUE);
	  		
	  		/* Recompute the main window parameters */
	  		recompute_main_window();
			
			/* Check whether the windows now intersect */
			check_windows_intersect();
			
			/* Redraw our position rectangle in the overview window */
			draw_overview_frame(FALSE);

			break;
		
		case inContent:
	
			/* Check for click in the main window map */
			if (window == main_window)
				{
				handle_main_window_click(event);
				}
			
			else	/* click in the overview window */
				{
				handle_overview_click(event);
				}
			
		  	break;
		  	
      }
      
}	/* handle_mouse_down() */



/*****************************************************************************\
* procedure handle_window_drag                                                *
*                                                                             *
* Purpose: This handles a window drag.                                        *
*                                                                             *
* Parameters: event: the mousedown event which occurred                       *
*             window: the window which is being dragged                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_window_drag(EventRecord *event, WindowPtr window)
{

	PixMapHandle	main_pixmap;
	Rect			window_drag_rect;
	
  	if (window == main_window)
  		{
  		main_pixmap = ((CWindowPtr) window)->portPixMap;
  		
  		/* Drag the main window.  We don't allow the window to go partially off-screen,
  			since that would cause problems during a fast update. */
  		BlockMove(&screenBits.bounds, &window_drag_rect, sizeof(Rect));
  		window_drag_rect.left = event->where.h +
  								(*main_pixmap)->bounds.left + 4;
  		window_drag_rect.right -= -event->where.h + 10 +
  									(window->portRect.right - window->portRect.left);
  		window_drag_rect.top = 20;
  		window_drag_rect.bottom -= -event->where.v + 44 +
  									(window->portRect.bottom - window->portRect.top);
  		DragWindow (window, event->where, &window_drag_rect);

		/* Find the base address of the upper left corner of the window */
		main_window_base = ((char *) GetPixBaseAddr(main_pixmap)) +
							(-(*main_pixmap)->bounds.top) * screen_width -
							(*main_pixmap)->bounds.left;
		}

	else
		{
		/* Drag the window anywhere on the screen */
  			DragWindow (window, event->where, &screenBits.bounds);
  		}

	/* Check whether the windows intersect */
	check_windows_intersect();
	
}	/* handle_window_drag() */



/*****************************************************************************\
* procedure recompute_main_window                                             *
*                                                                             *
* Purpose: This is called when the main window has been resized or the block  *
*          size has changed.  It recomputes the scroll bars and other things. *
*                                                                             *
* Parameters: event: the mousedown event which occurred                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 19, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void recompute_main_window()
{

	/* Find the size of the map region in the main window */
	view_width = main_window->portRect.right - main_window->portRect.left - 16;
	view_height = main_window->portRect.bottom - main_window->portRect.top - 16;

	/* Move the scroll bars to their correct places */
	MoveControl(horiz_scroll_bar, 0, view_height + 2);
	SizeControl (horiz_scroll_bar, view_width + 2, 16);
	MoveControl(vert_scroll_bar, view_width + 2, 0);
	SizeControl (vert_scroll_bar, 16, view_height + 2);
	
	/* Find the number of blocks which can fit in the map area now */
	view_width_blocks = view_width/(block_size + grid_lines);
	view_height_blocks = view_height/(block_size + grid_lines);

	/* If we're now hanging off the world, nudge us back */
	if (world_h + view_width_blocks >= world_width_blocks)
		world_h = world_width_blocks - view_width_blocks - 1;
	if (world_v + view_height_blocks >= world_height_blocks)
		world_v = world_height_blocks - view_height_blocks - 1;

	/* Find the new maximums for the scroll bars */
	SetCtlMax(horiz_scroll_bar, world_width_blocks - view_width_blocks - 1);
	SetCtlMax(vert_scroll_bar, world_height_blocks - view_height_blocks - 2);

	/* Set the new scroll bar values */
	SetCtlValue(horiz_scroll_bar, world_h);
	SetCtlValue(vert_scroll_bar, world_v);

	/* Set to update the window */
	EraseRect(&main_window->portRect);
	InvalRect(&main_window->portRect);

}	/* recompute_main_window() */



/*****************************************************************************\
* procedure check_windows_intersect                                           *
*                                                                             *
* Purpose: This checks for intersection between the main window and the       *
*          overview window.  If there is one, it sets windows_intersect_flag. *
*                                                                             *
* Parameters: event: the mousedown event which occurred                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 19, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void check_windows_intersect(void)
{

	Rect	overview_window_rect;
	Rect	temp_rect;

  	/* Convert the overview window's rect into main window coordinates */
  	BlockMove(&overview_window->portRect, &overview_window_rect, sizeof(Rect));
	SetPort(overview_window);
	LocalToGlobal((Point *) &overview_window_rect.top);
	LocalToGlobal((Point *) &overview_window_rect.bottom);
	SetPort(main_window);
	GlobalToLocal((Point *) &overview_window_rect.top);
	GlobalToLocal((Point *) &overview_window_rect.bottom);

	/* Check whether the windows intersect */
	windows_intersect_flag = SectRect(&main_window->portRect,
										&overview_window_rect, &temp_rect);

}	/* check_windows_intersect() */



/*****************************************************************************\
* procedure handle_overview_click                                             *
*                                                                             *
* Purpose: This handles a click in the overview window's content area.        *
*                                                                             *
* Parameters: event: the mousedown event which occurred                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_overview_click(EventRecord *event)
{

	Point		local_point;
	long		dist_vh;
	RgnHandle	frame_region;
	short		block_h, block_v;
	short		new_block_h, new_block_v;
	char		start_block_terrain_type;
	char		draw_terrain_type;

	/* If the overview window isn't front, activate it */
	if (overview_window != FrontWindow())
		{
		SelectWindow(overview_window);
		return;
		}

	/* Use the overview window */
	SetPort(overview_window);

	/* Find the local coordinates of the point in the overview window */
	local_point = event->where;
	GlobalToLocal(&local_point);
	
	/* If the option key was held down, draw in world */
	if (event->modifiers & optionKey)
		{
		
		block_h = local_point.h;
		block_v = local_point.v;

		/* Find the type of the current block */
		start_block_terrain_type = *(world + block_v * world_width_blocks + block_h);
		
		/* Draw in the opposite type of this one */
		draw_terrain_type = (start_block_terrain_type == background_terrain_type) ?
								foreground_terrain_type - 1 : background_terrain_type;
			
		/* Change the world block */
		set_world_block(block_h, block_v, draw_terrain_type);

		/* If the mouse is still down, keep changing the world blocks */
		while (StillDown ())
			{

			/* Find the block where the mouse is now */
			SetPort(overview_window);
			GetMouse (&local_point);
			new_block_h = local_point.h;
			new_block_v = local_point.v;

			/* If this is a new block, change it */
			if ((new_block_h != block_h) || (new_block_v != block_v))
				{
				set_world_block(new_block_h, new_block_v, draw_terrain_type);

				block_h = new_block_h;
				block_v = new_block_v;
				}				
			
			}
		}
	
	else	/* Option key was not down -- move the view rectangle */
		{

		/* Find the legal rectangle */
		BlockMove(&overview_window->portRect, &drag_rect, sizeof(Rect));
		SetRect(&drag_rect, view_width_blocks/2, view_height_blocks/2,
							world_width_blocks - view_width_blocks/2,
							world_height_blocks - view_height_blocks/2 - 1);
		
		/* Convert the frame rectangle to a region */
		frame_region = NewRgn();
		SetRectRgn(frame_region, local_point.h - (view_width_blocks/2 + 1),
									local_point.v - (view_height_blocks/2 + 1),
									local_point.h + view_width_blocks/2 + 1,
									local_point.v + view_height_blocks/2 + 1);
		
		/* Let the user drag the box around */
		dist_vh = DragGrayRgn (frame_region, local_point, &drag_rect,
								&overview_window->portRect, 0, draw_frame_action);
	
		/* Dispose of the region */
		DisposeRgn(frame_region);

		/* Update the overview frame */
		draw_overview_frame(FALSE);
		}

	
	/* Back to drawing in the main window */
	SetPort(main_window);

}	/* handle_overview_click() */


pascal void draw_frame_action(void)
{

	Point	local_point;

	/* Find the current posision of the mouse in the overview window */
	GetMouse (&local_point);

	/* if the mouse is in a legal place, redraw the world */
	if (PtInRect (local_point, &drag_rect))
		{
	
		/* Update our location in the world */
		world_h = local_point.h - view_width_blocks/2;
		world_v = local_point.v - view_height_blocks/2;
	
		/* Update the scroll bars */
		SetCtlValue(horiz_scroll_bar, world_h);
		SetCtlValue(vert_scroll_bar, world_v);
	
		/* Update the background */
		draw_background();
		
		/* Set the port back to the overview window */
		SetPort(overview_window);
		
		}

}	/* draw_frame_action() */



/*****************************************************************************\
* procedure handle_main_window_click                                          *
*                                                                             *
* Purpose: This handles a click in the main window's content area.            *
*                                                                             *
* Parameters: event: the mousedown event which occurred                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_main_window_click(EventRecord *event)
{	

	short			block_h, block_v;
	short			new_block_h, new_block_v;
	char			draw_terrain_type;
	char			new_terrain_type;
	char			this_terrain_type;
	char			start_block_terrain_type;
	long			menu_result;
	Point			local_point;
	long			pinned_point;
	short			control_part;
	ControlHandle	the_control;

	/* If the main window isn't front, activate it */
	if (main_window != FrontWindow())
		{
		SelectWindow(main_window);
		return;
		}

	/* Convert the point to local coordinates */
	local_point = event->where;
	GlobalToLocal(&local_point);
		
	/* Find whether a scroll bar was clicked */ 
	control_part = FindControl (local_point, main_window, &the_control);

	if (control_part)
		{
	
		/* Handle scroll bars */
		if (control_part == inThumb)
			{
			if (TrackControl(the_control, local_point, NULL))
				{
				/* Scroll world according to new thumb location */
				world_h = GetCtlValue(horiz_scroll_bar);
				world_v = GetCtlValue(vert_scroll_bar);
				
				/* Update the background */
				draw_background();
				
				/* Update the overview frame */
				draw_overview_frame(FALSE);

				}
			}
		
		else
			
			/* Handle everything except the thumb */
			TrackControl(the_control, local_point, scroll_action);
		
		}

	else	/* Not in a control -- must be in the map itself */
		{
	
		/* Find the block which was clicked */
		block_h = world_h + (local_point.h)/(block_size + grid_lines);
		block_v = world_v + (local_point.v)/(block_size + grid_lines);
		
		/* If the option key is down, just draw the same pattern as last time */
		if (!(event->modifiers & optionKey))
			{

			/* Find the terrain type of this block */
			this_terrain_type = *(world + block_v * world_width_blocks + block_h);

			/* option key is not down -- Pop up the terrains menu */
			InsertMenu(terrains_popup_handle, hierMenu);
			menu_result = PopUpMenuSelect (terrains_popup_handle,
											event->where.v, event->where.h,
												this_terrain_type + 1);
			DeleteMenu(TERRAINS_MENU);
			
			/* Find the new terrain_type */
			if (menu_result >> 16)
				{
				/* Mask out all but the terrain type byte */
				new_terrain_type = (menu_result & 0xFF);
				}
			else
				new_terrain_type = 0;	/* no selection */

			/* If this terrain is illegal, make it water */
			if (!terrains64x64[new_terrain_type-1])
				new_terrain_type = 1;
			
			/* If there actually was a selection, change the world */
			if (new_terrain_type)
				{
				set_world_block(block_h, block_v, new_terrain_type - 1);
				
				/* Remember the previous and new types as background and foreground*/
				background_terrain_type = this_terrain_type;
				foreground_terrain_type = new_terrain_type;
				}
				
			}
		
		else	/* option key was held down */
			{
			
			/* Find the type of the current block */
			start_block_terrain_type = *(world + block_v * world_width_blocks + block_h);
			
			/* Draw in the opposite type of this one */
			draw_terrain_type = (start_block_terrain_type == background_terrain_type) ?
									foreground_terrain_type - 1 : background_terrain_type;
				
			/* Change the world block */
			set_world_block(block_h, block_v, draw_terrain_type);

			/* If the mouse is still down, keep changing the world blocks */
			while (StillDown ())
				{

				/* Find the block where the mouse is now */
				GetMouse (&local_point);
				
				/* If the mouse is out of the window, scroll the window */
				if (local_point.h < 0)
					{
					if (world_h)	/* horizontally to the left */
						{
						world_h--;
						SetCtlValue(horiz_scroll_bar, world_h);
						draw_background();
						draw_overview_frame(FALSE);
						}	
					else continue;
					}
				
				else if (local_point.h > view_width_blocks * (block_size + grid_lines))
					{
					
					/* horizontally to the right */
					if (world_h + view_width_blocks < world_width_blocks)
						{
						world_h++;
						SetCtlValue(horiz_scroll_bar, world_h);
						draw_background();
						draw_overview_frame(FALSE);
						}	
					else continue;
					}
				
				if (local_point.v < 0)
					{
					if (world_v)	/* vertically up */
						{
						world_v--;
						SetCtlValue(vert_scroll_bar, world_v);
						draw_background();
						draw_overview_frame(FALSE);
						}	
					else continue;
					}
				
				else if (local_point.v > view_height_blocks * (block_size + grid_lines))
					{
					
					/* vertically down */
					if (world_v + view_height_blocks < world_height_blocks)
						{
						world_v++;
						SetCtlValue(vert_scroll_bar, world_v);
						draw_background();
						draw_overview_frame(FALSE);
						}
					else continue;
					
					}

				/* Pin local_point inside the window */
				pinned_point = PinRect (&main_window->portRect, local_point);

				new_block_h = world_h + (LoWord(pinned_point))/(block_size + grid_lines);
				new_block_v = world_v + (HiWord(pinned_point))/(block_size + grid_lines);

				/* If this is a new block, change it */
				if ((new_block_h != block_h) || (new_block_v != block_v))
					{
					set_world_block(new_block_h, new_block_v, draw_terrain_type);

					block_h = new_block_h;
					block_v = new_block_v;
					}				
				
				
				}
			}
		
		}
	
}	/* handle_main_window_click() */



/*****************************************************************************\
* procedure set_world_block                                                   *
*                                                                             *
* Purpose: This procedure sets one block of the world to a given terrain, and *
*          then redraws the world and the overview to reflect the change.     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void set_world_block(short h, short v, char terrain_type)
{

	Point	point;
	Rect	inside_rect;

	/* Change the world block */
	*(world + v * world_width_blocks + h) = terrain_type;
	
	/* Prepare to draw the overview */
	SetPort(overview_window);
	
	/* If the pixel is under the frame, hide the frame */
	BlockMove(&overview_frame, &inside_rect, sizeof(Rect));
	InsetRect(&inside_rect, 1, 1);
	point.h = h; point.v = v;
	if (!PtInRect(point, &inside_rect))
		{
	
		PenMode(patXor);
		FrameRect (&overview_frame);

		
		/* Draw a pixel in the overview window */
		draw_overview_pixel(h, v, terrain_type);
	
		/* Show the overview frame */
		FrameRect (&overview_frame);
		PenMode(patCopy);
		}
		
	else
		{
		
		/* Just draw a pixel in the overview window */
		draw_overview_pixel(h, v, terrain_type);
	
		}

	/* We're done with the overview */
	SetPort(main_window);
	
	/* Draw the new world */
	draw_background();
	
	/* There is now a change which we have not saves */
	changes_saved = FALSE;

}	/* set_world_block() */



/*****************************************************************************\
* procedure scroll_action                                                     *
*                                                                             *
* Purpose: This is the scroll bar action procedure.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 17, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void scroll_action(ControlHandle the_control, short part)
{

	if (the_control == horiz_scroll_bar)
		{

		switch (part)
			{
			case inDownButton:
				world_h++;
				if ((world_h + view_width_blocks) > world_width_blocks)
					world_h--;
				break;
			
			case inUpButton:
				world_h--;
				if (world_h < 0)
					world_h++;
				break;

			case inPageDown:
				world_h += 10;
				if ((world_h + view_width_blocks) > world_width_blocks)
					world_h = world_width_blocks - view_width_blocks;
				break;
			
			case inPageUp:
				world_h -= 10;
				if (world_h < 0)
					world_h = 0;
				break;
			
			default:;
			}
		}

	else	/* vertical scroll bar */
		{

		switch (part)
			{
			case inDownButton:
				world_v++;
				if ((world_v + view_height_blocks) > world_height_blocks)
					world_v--;
				break;
			
			case inUpButton:
				world_v--;
				if (world_v < 0)
					world_v++;
				break;
			
			case inPageDown:
				world_v += 10;
				if ((world_v + view_height_blocks) > world_height_blocks)
					world_v = world_height_blocks - view_height_blocks;
				break;
			
			case inPageUp:
				world_v -= 10;
				if (world_v < 0)
					world_v = 0;
				break;
			
			default:;
			}
		}

	/* Update the world */
	draw_background();

	/* Update the overview frame */
	draw_overview_frame(FALSE);

	/* Set the new scroll bar values */
	SetCtlValue (horiz_scroll_bar, world_h);
	SetCtlValue (vert_scroll_bar, world_v);

}	/* scroll_action() */



/*****************************************************************************\
* procedure do_menu                                                           *
*                                                                             *
* Purpose: This procedure handles a pulldown menu selection.                  *
*                                                                             *
* Parameters: command: the menu selection; menu id in low word, menu item in  *
*                      high word                                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void do_menu(long command)
{
	short	menu_id = HiWord(command);
	short	item = LoWord(command);
	Str255	item_name;
	
	switch(menu_id)
	{
		case APPLE_MENU:
			if (item == ABOUT_ITEM) 
				;
			else
			{
				GetItem(GetMHandle(menu_id), item, item_name);
				OpenDeskAcc(item_name);
			}
				break;
				
		case FILE_MENU:
			
			switch (item)
			{
			
				case NEW_ITEM:
				
					/* Create a new world */
					create_new_world();
					
					break;

				case OPEN_ITEM:
				
					/* Read a world from disk */
					open_world();
					
					break;

				case SAVE_ITEM:
				
					/* Write this world to disk */
					save_world();
					
					break;

				case SAVE_AS_ITEM:
				
					/* Write this world to disk, prompt for filename */
					save_as_world();
					
					break;

				case QUIT_ITEM:
									
					/* Quit */
					ExitToShell();

			}
			
			break;

		case EDIT_MENU:

			break;

		case WINDOWS_MENU:
			
			switch (item)
			{
			
				case MAIN_WINDOW_ITEM:
				
					/* Bring the main window to the front */
					SelectWindow(main_window);
					
					break;

				case OVERVIEW_WINDOW_ITEM:
				
					/* Bring the overview window to the front */
					SelectWindow(overview_window);
					
					break;

			}
			
			break;
			
		case MAP_MENU:
			
			switch (item)
			{
			
				case GRIDLINES_ITEM:
				
					/* Toggle the gridlines */
					grid_lines = 1 - grid_lines;
				
					/* Flip the check in the menu */
					CheckItem(map_menu, GRIDLINES_ITEM, grid_lines);
					
					/* Update the window to reflect this new block situation */
					recompute_main_window();
					
					break;
			
			}
			
			break;
			
		case ZOOM_OUT_MENU:
			
			/* Turn off all the check marks */
			CheckItem (zoom_out_menu, ZOOMx1, FALSE);
			CheckItem (zoom_out_menu, ZOOMx2, FALSE);
			CheckItem (zoom_out_menu, ZOOMx4, FALSE);
			CheckItem (zoom_out_menu, ZOOMx8, FALSE);
			
			switch (item)
			{
			
				case ZOOMx1:
				
					/* Set the block size to 64x64 */
					block_size = s64x64;
					
					/* Check the 1x item */
					CheckItem(zoom_out_menu, ZOOMx1, TRUE);
					
					break;

				case ZOOMx2:
				
					/* Set the block size to 32x32 */
					block_size = s32x32;
					
					/* Check the 2x item */
					CheckItem(zoom_out_menu, ZOOMx2, TRUE);
					
					break;

				case ZOOMx4:
				
					/* Set the block size to 16x16 */
					block_size = s16x16;
					
					/* Check the 4x item */
					CheckItem(zoom_out_menu, ZOOMx4, TRUE);
					
					break;

				case ZOOMx8:
				
					/* Set the block size to 8x8 */
					block_size = s8x8;
					
					/* Check the 8x item */
					CheckItem(zoom_out_menu, ZOOMx8, TRUE);
					
					break;

			}
			
			/* recompute the main window parameters */
			recompute_main_window();

			/* Redraw our position rectangle in the overview window */
			draw_overview_frame(FALSE);
			
			break;
			
	}
	
	HiliteMenu(0);

}	/* do_menu() */



/*****************************************************************************\
* procedure create_new_world                                                  *
*                                                                             *
* Purpose: This procedure creates a new world.                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void create_new_world(void)
{

	long	i;

	/* Make it a 128 x 128 world */
	world_width_blocks = world_height_blocks = 128;
	
	/* Resize the overview window to the correct size */
	SizeWindow(overview_window, world_width_blocks, world_height_blocks, FALSE);
	
	/* If there is already a world, get rid of it */
	if (world_handle)
		DisposeHandle(world_handle);

	/* Allocate memory for the world map (start all water) */
	world_handle = (char **) NewHandleClear (world_width_blocks * world_height_blocks);

	/* Lock down the handle, and dereference */
	HLock(world_handle);
	world = *world_handle;

	/* Set the block size to 32 x 32 */
	block_size = s32x32;
	
	/* Start in the center */
	world_h = world_v = 56;
	
	/* Calculate the number of these blocks which will fit on the screen */
	view_width_blocks = (view_width) / (block_size + grid_lines);
	view_height_blocks = (view_height) / (block_size + grid_lines);

	/* Add the Walls at the Edge of the World */
	for (i = WORLD_MARGIN; i < (world_width_blocks-WORLD_MARGIN); i++)
		{
		
		/* Draw the upper wall */
		*(world + (WORLD_MARGIN)*world_width_blocks + i) = 26;
		
		/* Draw the lower wall */
		*(world + (world_height_blocks-WORLD_MARGIN)*world_width_blocks + i) = 26;
		
		/* Draw the left wall */
		*(world + i*world_width_blocks + WORLD_MARGIN) = 25;
		
		/* Draw the right wall */
		*(world + i*world_width_blocks + (world_width_blocks-WORLD_MARGIN)) = 25;

		}

	/* Draw upper left curve of WatEotW */
	*(world + (WORLD_MARGIN)*world_width_blocks + WORLD_MARGIN) = 28;

	/* Draw upper right curve of WatEotW */
	*(world + (WORLD_MARGIN)*world_width_blocks + world_width_blocks-WORLD_MARGIN) = 29;

	/* Draw lower left curve of WatEotW */
	*(world + (world_height_blocks-WORLD_MARGIN)*world_width_blocks + WORLD_MARGIN) = 27;

	/* Draw lower right curve of WatEotW */
	*(world + (world_height_blocks-WORLD_MARGIN)*world_width_blocks + world_width_blocks-WORLD_MARGIN) = 30;

	/* Set the window name */
	SetWTitle (main_window, "\pUntitled World");

	/* Set the scroll bars to scroll through this world */
	SetCtlMax(horiz_scroll_bar, world_width_blocks - view_width_blocks - 1);
	SetCtlMax(vert_scroll_bar, world_height_blocks - view_height_blocks - 2);

	/* Set to update the map and the overview */
	SetPort(overview_window);
	InvalRect(&overview_window->portRect);
	SetPort(main_window);
	InvalRect(&main_window->portRect);

}	/* create_new_world() */



/*****************************************************************************\
* procedure open_world                                                        *
*                                                                             *
* Purpose: This procedure reads a world in from a world file.                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void open_world(void)
{

	StandardFileReply	get_file_reply;
	world_size_resource	size_resource;
	short				res_refnum;
	short				error;
	
	SFTypeList			types;
	
	/* Only accept wMap files */
	types[0] = 'wMap';

	/* Get the filename and info */
	StandardGetFile(NULL, 1, types, &get_file_reply);

	/* If the user didn't cancel, load this file */
	if (get_file_reply.sfGood)
		{
		
		/* Get rid of the current world */
		DisposeHandle(world_handle);
		
		/* Copy the file specification to a global variable */
		BlockMove(&get_file_reply.sfFile, &world_fsspec, sizeof(FSSpec));

		/* Set the window name to the filename */
		SetWTitle (main_window, world_fsspec.name);

		/* Open file as a resouce file */
		res_refnum = FSpOpenResFile(&world_fsspec, fsRdPerm);
		
		/* Read the size of the world map */
		size_resource = (world_size_resource) GetResource ('wSiz', 0);
		error = ResError();
		world_width_blocks = (*size_resource)->width;
		world_height_blocks = (*size_resource)->height;
		
		/* Read the world map and dereference it */
		world_handle = GetResource('wMap', 0);
		error = ResError();
		DetachResource(world_handle);
		error = ResError();
		
		/* Close the resource file */
		CloseResFile (res_refnum);
		error = ResError();

		/* Dereference the world handle for speed */
		HLock(world_handle);
		world = *world_handle;
		
		/* We have now saved all changes */
		changes_saved = TRUE;
		
		/* We have enough info to save this file later without a prompt */
		got_file_data = TRUE;
		
		/* Draw the new view */
		draw_background();
		
		/* Draw the new overview */
		draw_overview();
		
		/* Draw the frame */
		draw_overview_frame(TRUE);
		
		}

}	/* open_world() */



/*****************************************************************************\
* procedure save_world                                                        *
*                                                                             *
* Purpose: This procedure saves the world to a world file.                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void save_world(void)
{

	short				res_refnum;
	short				error;
	world_size_resource	size_resource;

	/* If we haven't saved before, use "Save As..." */
	if (!got_file_data)
		save_as_world();
	
	else
		{
		
		/* Delete the file if it exists */
		error = FSpDelete(&world_fsspec);
		
		/* Create the file */
		FSpCreateResFile(&world_fsspec, 'Spio', 'wMap', smSystemScript);
		error = ResError();
		
		/* Open file as a resouce file */
		res_refnum = FSpOpenResFile(&world_fsspec, fsWrPerm);
		error = ResError();
		
		/* Write the world map as a resource */
		AddResource (world_handle, 'wMap', 0, NULL);
		WriteResource (world_handle);
		DetachResource(world_handle);
		error = ResError();
		
		/* Read the size of the world map as a resource */
		size_resource = (world_size_resource) NewHandle(sizeof(world_size_resource));
		(*size_resource)->width = world_width_blocks;
		(*size_resource)->height = world_height_blocks;
		AddResource (size_resource, 'wSiz', 0, NULL);
		WriteResource (size_resource);
		error = ResError();
		
		/* Close the resource file */
		CloseResFile (res_refnum);
		error = ResError();
		
		/* We have now saved all changes */
		changes_saved = TRUE;
		
		}

}	/* save_world() */



/*****************************************************************************\
* procedure save_as_world                                                     *
*                                                                             *
* Purpose: This procedure saves the world to a world file, and prompts for a  *
*          filename even if this has already been saved.                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void save_as_world(void)
{

	Str255				filename;
	StandardFileReply	put_file_reply;
	
	/* Find the name of the window and use it as the filename */
	GetWTitle (main_window, filename);

	/* Prompt for location to save file */
	StandardPutFile("\pSave this world as:", filename, &put_file_reply);

	/* If user didn't cancel, save the file */
	if (put_file_reply.sfGood)
		{
		
		/* We now have enough info to save correctly */
		got_file_data = TRUE;
		
		/* Copy the file specification to a global variable */
		BlockMove(&put_file_reply.sfFile, &world_fsspec, sizeof(FSSpec));
		
		/* Set the window title to the filename */
		SetWTitle (main_window, world_fsspec.name);
		
		/* Save the file */
		save_world();
		
		}

}	/* save_as_world() */



/*****************************************************************************\
* procedure handle_key_down                                                   *
*                                                                             *
* Purpose: This procedure handles a key down event.                           *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 5, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void handle_key_down(EventRecord *event)
{

	short 	item_hit;
	long	menu_result;

	/* Check for menu shortcut */
	if (event->modifiers & cmdKey)
		{
				
		/* Find menu equivalent */
		menu_result = MenuKey(event->message);
		
		/* Pretend user clicked menu */
		if (HiWord(menu_result) != 0)	/* valid key? */
			do_menu (menu_result);		/* yes, map it to menu item */
		}
		

}	/* handle_key_down() */



/*****************************************************************************\
* procedure draw_overview                                                     *
*                                                                             *
* Purpose: This procedure draws the overview in the overview window.          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_overview(void)
{

	char	*this_block = world;
	long	i, j;

	/* Turn on the watch cursor */
	SetCursor(*watch_cursor);
	
	/* Draw in the overview window */
	SetPort(overview_window);

	for (i = 0; i < world_height_blocks; i++)
		for (j = 0; j < world_width_blocks; j++)
			{
			
			/* draw a pixel in the overview window */
			draw_overview_pixel(j, i, *(this_block++));

			}

	/* Remember that this has been drawn */
	ValidRect(&overview_window->portRect);

	/* Draw on main window again */
	SetPort(main_window);

	/* Turn on the arrow cursor */
	SetCursor(&arrow);
	
}	/* draw_overview() */




/*****************************************************************************\
* procedure draw_overview_frame                                               *
*                                                                             *
* Purpose: This procedure draws a frame in the overview window indicating the *
*          position of what we're looking at.                                 *
*                                                                             *
* Parameters: first_time: TRUE if we have not drawn a frame before.           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_overview_frame(Boolean first_time)
{

	/* Set up to draw in the overview window */
	SetPort(overview_window);

	/* Set pen to invert */
	PenMode(patXor);

	/* If we have drawn a frame before, erase it */
	if (!first_time)
		{
		FrameRect (&overview_frame);
		}
	
	/* Find the rectangle to frame */
	overview_frame.left = world_h - 1;
	overview_frame.right = world_h + view_width_blocks + 1;
	overview_frame.top = world_v - 1;
	overview_frame.bottom = world_v + view_height_blocks + 1;
	
	/* Frame the new rectangle */
	FrameRect (&overview_frame);

	/* Set pen to back to normal */
	PenMode(patCopy);

	/* Set up to draw in the main window */
	SetPort(main_window);

}	/* draw_overview_frame() */



/*****************************************************************************\
* procedure draw_overview_pixel                                               *
*                                                                             *
* Purpose: This procedure draws one pixel of the overview.                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_overview_pixel(short h, short v, char terrain_type)
{

	RGBColor	*color;

	/* Find the overview color equivalent of this terrain */
	switch (terrain_type)
		{
		
		case water:	color = &water_color;		break;
		case grass:	color = &grass_color;		break;
		case ice:	color = &ice_color;			break;
		case sand:	color = &sand_color;		break;
		default:	color = &other_color;		break;

		}

	/* Draw the pixel on the overview */				
	SetCPixel (h, v, color);
	
}	/* draw_overview_pixel() */
	


/*****************************************************************************\
* procedure draw_background_carefully                                         *
*                                                                             *
* Purpose: This procedure draws the background of the main window.  It does   *
*          it "properly," through QuickDraw, to ensure that there will be no  *
*          problems with window overlap.                                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_background_carefully(void)
{

	short		v, h;
	PicHandle	this_pict;
	Rect		pict_rect;
	Rect		terrain_size_rect;
	RgnHandle	temp_clip;
	Rect		map_rect;
	
	SetRect(&terrain_size_rect, 0, 0, block_size, block_size);
	
	/* Turn on the watch cursor */
	SetCursor(*watch_cursor);
	
	/* Draw in main window */
	SetPort(main_window);
	
	/* Don't draw over the scroll bars */
	temp_clip = NewRgn();
	GetClip(temp_clip);
	SetRect(&map_rect, 0, 0, view_width, view_height);
	ClipRect(&map_rect);
	
	/* Loop through all blocks in the window, drawing each one from PICT */
	for (v = 0; v <= view_height_blocks; v++)
		for (h = 0; h <= view_width_blocks; h++)
			{
			/* Draw the PICT for this block */
			this_pict = GetPicture (FIRST_TERRAIN_ID +
									*(world + (world_v + v) * world_width_blocks + (world_h + h)));
			BlockMove(&terrain_size_rect, &pict_rect, sizeof(Rect));
			OffsetRect(&pict_rect, h * (block_size + grid_lines), v * (block_size + grid_lines));
			DrawPicture(this_pict, &pict_rect);
			}

	/* Restore the clipping region */
	SetClip(temp_clip);
	DisposeRgn(temp_clip);

	/* Turn on the arrow cursor */
	SetCursor(&arrow);
	

}	/* draw_background_carefully() */



/*****************************************************************************\
* procedure draw_background                                                   *
*                                                                             *
* Purpose: This procedure draws the background of the main window.            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_background(void)
{

	register char	*block_base;		/* the base address of the current block */
	register char	*line_base;			/* the base address of the current line within this block */
	register char	*terrain_base;		/* the base address of the terrain pattern for this block */
	char	*current_world_map_block;/* Pointer to terrain or current block block in world map */
	char	*left_block_base;	/* the base address of the leftmost block on the current line */
	long 	vert;				/* counts vertical lines of blocks */
	long	block;				/* counts blocks horizontally within a line of blocks */
	long	num_vert_blocks;	/* number of blocks which fit vertically in the window */
	long	num_horiz_blocks;	/* number of blocks which fit horizontally in the window */
	long	start_line;			/* line to start drawing pattern at */
	long	end_line;			/* line to end drawing pattern at */
	long	hoffset;			/* offset of center of window into world map, horizontally */
	long	voffset;			/* offset of center of window into world map, vertically */
	long	bottom_edge_offset;	/* the number of lines below the screen the bottom pattern extendes */
	long	world_offset_h;		/* Current horizontal offset into the world map */
	long	world_offset_v;		/* Current vertical offset into the world map */
	long	total_left_bytes;	/* Number of bytes per line in a left block */
	long	total_right_bytes;	/* Number of bytes per line in a right block */
	long	left_longwords;		/* Number of longwords per line  in a left block */
	long	right_longwords;	/* Number of longwords per line in a right block */
	Boolean	left_word_flag;		/* True if there is a spare word per line in a left block */
	Boolean	left_byte_flag;		/* True if there is a spare byte per line in a left block */
	Boolean	right_word_flag;	/* True if there is a spare word per line in a right block */
	Boolean	right_byte_flag;	/* True if there is a spare byte per line in a right block */
	Boolean	topmost_block_flag; /* TRUE if we are currently drawing the top line of blocks */

	Ptr		*terrains;			/* Array of pointers to terrains */
	long	temp;

	/* If we should draw carefully (through QuickDraw), do it */
	if ((windows_intersect_flag && (FrontWindow() == overview_window)) || (in_background_flag))
		{
		draw_background_carefully();
		return;
		}

	/* Find the terrain array we should use */
	switch (block_size)
		{
		case (s64x64):
			terrains = (Ptr *) &terrains64x64;
			break;

		case (s32x32):
			terrains = (Ptr *) &terrains32x32;
			break;

		case (s16x16):
			terrains = (Ptr *) &terrains16x16;
			break;

		case (s8x8):
			terrains = (Ptr *) &terrains8x8;
			break;
		}


	/* Find the offset into the world map of the upper left corner of the window */
	hoffset = world_h * block_size;
	voffset = world_v * block_size;

	/* Find the offset of the upper left block into the world map */
	world_offset_h = hoffset/block_size;
	world_offset_v = voffset/block_size;
	
	/* Start reading terrain at the upper left of this segment of the world map */
	current_world_map_block = world + world_offset_v*world_width_blocks + world_offset_h;

	/* Find offsets of window into upper left block */
	hoffset %= block_size;
	voffset %= block_size;

	/* Find number of longwords in leftmost blocks, and any residual word or byte */
	total_left_bytes = block_size - hoffset;
	left_longwords = total_left_bytes >> 2;
	left_byte_flag = total_left_bytes & 0x1;
	left_word_flag = (total_left_bytes >> 1) & 0x1;

	/* Find number of longwords in rightmost blocks, and any residual word or byte */
	total_right_bytes = (view_width) % (block_size + grid_lines);
	right_longwords = total_right_bytes >> 2;
	right_byte_flag = total_right_bytes & 0x1;
	right_word_flag = (total_right_bytes >> 1) & 0x1;

	/* Find how far into its block the bottom line extends (skip all
		lower lines in that block) */
	bottom_edge_offset = (voffset+view_height) % (block_size + grid_lines);

	/* Find the number of blocks per screen vertically */
	num_vert_blocks = ((view_height+voffset)/(block_size + grid_lines)) + 1;

	/* Find the number of blocks in the window horizontally */
	num_horiz_blocks = ((view_width+hoffset)/(block_size + grid_lines)) + 1;

	/* Start at the upper left corner of the window */
	line_base = left_block_base = main_window_base;
	
	/* Remember that we're starting at the top of the window */
	topmost_block_flag = TRUE;

	HideCursor();

	/* loop through the vertical blocks */
	for (vert = 0; vert < num_vert_blocks; vert++)
		{
		

		/* Start the block pointer at the left */
		line_base = left_block_base;
		
		/* If we're at the top of the window, skip lines as necessary */
		start_line = (topmost_block_flag) ? voffset : 0;
		
		/* If we're at the bottom of the window, skip lines as necessary */
		end_line = (vert == (num_vert_blocks - 1)) ? bottom_edge_offset: block_size;
		
		/* Don't do anything if there's nothing to draw */
		if (start_line == end_line)
			continue;
		
		/* Draw a row of blocks */

		asm {
		
			moveq.l	#0, d0
			move.l	current_world_map_block, a0		; find the terrain type for this block
			move.b	(a0), d0
			move.l	terrains, a0
			asl.l	#2, d0
			add.l	d0, a0		
			move.l	(a0), terrain_base				; find the base address of the pattern
			move.l	start_line, d0					; offset by number of lines skipped
			asl.l	#6, d0
			add.l	d0, terrain_base
			
			move.l	end_line, d1					; count start_line - end_line lines
			sub.l	start_line, d1
			subq.l	#1, d1

		@left_line_loop:
		
			add.l hoffset, terrain_base;			; don't draw pattern left of window
	
			btst.b	#0, left_byte_flag				; If there is a byte at the left,
			beq		@no_left_byte					; draw it
			move.b (terrain_base)+, (line_base)+
			
		@no_left_byte:
		
			btst.b	#0, left_word_flag				; if there is a word at the left,
			beq		@no_left_word					; draw it
			move.w (terrain_base)+, (line_base)+
			
		@no_left_word:				
			
			move.l	left_longwords, d0				; set counter to draw long words
			subq.l	#1, d0
			
			bmi		@no_left_lws					; check for no left longwords
			
		@left_lw_loop:								; draw the long words
			move.l	(terrain_base)+, (line_base)+
			dbra	d0, @left_lw_loop
			
		@no_left_lws:	
		
			add.l	screen_width, line_base			; Point to next line of blocks */
			sub.l	total_left_bytes, line_base;
			
			dbra	d1, @left_line_loop
		
			move.l	left_block_base, block_base		; point to next destination block
			add.l	total_left_bytes, block_base
			add.l	grid_lines, block_base
			
			addq.l	#1, current_world_map_block		; point to next world block
			
			
			
			/* Do the middle blocks of this line:
			   d1 counts lines in a block, and
			   d0 counts longwords in a line */
			
			move.l	num_horiz_blocks, block			; loop through horizontal blocks
			subq.l	#2, block
		
		@middle_block_loop:
		
			moveq.l	#0, d0
			move.l	current_world_map_block, a0		; find the terrain type for this block
			move.b	(a0), d0
			move.l	terrains, a0
			asl.l	#2, d0
			add.l	d0, a0		
			move.l	(a0), terrain_base				; find the base address of the pattern
			move.l	start_line, d0					; offset by number of lines skipped
			asl.l	#6, d0
			add.l	d0, terrain_base
			
			move.l	block_base, line_base			; start at top of block
			
			/* Draw a middle block */
			
			move.l	end_line, d1					; count start_line - end_line lines
			sub.l	start_line, d1
			subq.l	#1, d1
			
		@middle_line_loop:
		
			move.l	block_size, d0					; loop through longwords of the block
			asr.l	#2, d0
			subq.l	#1, d0
			
		@middle_lw_loop:							; draw a line of the block
			move.l	(terrain_base)+, (line_base)+
			dbra	d0, @middle_lw_loop
			
			add.l	screen_width, line_base 		; point to next line of block
			sub.l	block_size, line_base
			
			dbra	d1, @middle_line_loop
			
			add.l	block_size, block_base			; point to next destination block
			add.l	grid_lines, block_base
			addq.l	#1, current_world_map_block		; point to next world block
			
			subq.l	#1, block						; go to next block
			bne		@middle_block_loop
			
			
			/* Draw the right block */

			move.l	block_base, line_base			; point to next destination block
			
			moveq.l	#0, d0
			move.l	current_world_map_block, a0		; find the terrain type for this block
			move.b	(a0), d0
			move.l	terrains, a0
			asl.l	#2, d0
			add.l	d0, a0		
			move.l	(a0), terrain_base				; find the base address of the pattern
			move.l	start_line, d0					; offset by number of lines skipped
			asl.l	#6, d0
			add.l	d0, terrain_base
			
			move.l	end_line, d1					; count start_line - end_line lines
			sub.l	start_line, d1
			subq.l	#1, d1
			
		@right_line_loop:
		
		
			btst.b	#0, right_byte_flag				; if there is a byte at the right,
			beq		@no_right_byte					;  draw it
			move.b (terrain_base)+, (line_base)+
			
		@no_right_byte:
		
			btst.b	#0, right_word_flag				; if there is a word at the right,
			beq		@no_right_word					; draw it
			move.w (terrain_base)+, (line_base)+
			
		@no_right_word:				

			move.l	right_longwords, d0				; set counter to draw long words
			subq.l	#1, d0
			
			bmi		@no_right_lws					; There are no lws at right
		
			; draw the long words
			
		@right_lw_loop:
			move.l	(terrain_base)+, (line_base)+
			dbra	d0, @right_lw_loop
			
		@no_right_lws:
		
			add.l	block_size, terrain_base		; skip pattern to right of window
			sub.l	total_right_bytes, terrain_base
			
			add.l	screen_width, line_base			; Point to next line of blocks */
			sub.l	total_right_bytes, line_base;
			
			dbra	d1, @right_line_loop
			
			}

		/* We're no longer at the topmost block */
		topmost_block_flag = FALSE;
		
		/* update the vertical block counter to point to the next block */
		left_block_base += screen_width*(end_line-start_line + grid_lines);
		
		/* Go to the next horizontal world map block */
		current_world_map_block += (world_width_blocks - num_horiz_blocks + 1);

		}	/* End of vert line loop */


	ShowCursor();

}	/* draw_background() */



/*****************************************************************************\
* procedure handle_activate                                                   *
*                                                                             *
* Purpose: This procedure handles activate events for the windows.            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_activate(EventRecord *event)
{

	/* If it's the main window, activate/deactivate the scroll bars */
	if ((WindowPtr) event->message == main_window)
		{
		HiliteControl(vert_scroll_bar, (event->modifiers & activeFlag) ? 0 : 255);
		HiliteControl(horiz_scroll_bar, (event->modifiers & activeFlag) ? 0 : 255);
		}

}	/* handle_activate() */



/*****************************************************************************\
* procedure handle_update                                                     *
*                                                                             *
* Purpose: This procedure handles update events for the windows.              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 18, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void handle_update(EventRecord *event)
{

	/* Start the update */
	BeginUpdate ((WindowPtr) event->message);
	
	/* If this is the overview window, draw the overview */
	if ((WindowPtr) event->message == overview_window)
		{
		draw_overview();
		draw_overview_frame(TRUE);
		}
	
	else	/* it's the map window */
		{
		
		/* Update the background. */
		draw_background();
			
		/* Draw the scroll bars */
		DrawControls(main_window);
		
		/* Draw the grow region */
		DrawGrowIcon (main_window);				

		}
	
	
	/* End the update */
	EndUpdate ((WindowPtr) event->message);

}	/* handle_update() */



/*****************************************************************************\
* procedure handle_multifinder_event                                          *
*                                                                             *
* Purpose: This procedure handles a multifinder event.                        *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 14, 1992                                              *
* Modified:                                                                   *
\*****************************************************************************/

void handle_multifinder_event(EventRecord *event)
{

	/* Check for multifinder event */
	if (event->message & 0x01000000)
		{

		/* Check for coming to foreground event */
		if (event->message & 0x00000001)
			{
			
			/* Coming to foreground... */
			in_background_flag = FALSE;
			
			}
			
		else
			{
			
			/* Going to background... */
			in_background_flag = TRUE;
			
			}
		}
		
}	/* handle_multifinder_event() */



/*****************************************************************************\
* procedure event_loop                                                        *
*                                                                             *
* Purpose: This procedure repeatedly checks for events, and handles them when *
*          they occur.  It does not exit until ExitToShell().                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void event_loop(void)

{

	Boolean		good;
	EventRecord	event;

	while (1)
		{		
			
		good = WaitNextEvent (everyEvent, &event, 0xFFFFFFFF, (RgnHandle) 0L);
		
		if (good)
		  switch (event.what)
		    {
			case mouseDown:
				handle_mouse_down(&event);
				break;
				
			case keyDown: 
			case autoKey:
				handle_key_down(&event);
				break;
				
			case updateEvt:
				handle_update(&event);
			    break;
			    
			case activateEvt:
				handle_activate(&event);
				break;

			case app4Evt:
				handle_multifinder_event(&event);
				break;
			
			default:;
		
		    }
		}
	    
}	/* event_loop() */



/*****************************************************************************\
* procedure main                                                              *
*                                                                             *
* Purpose: This is the main procedure which is called when the program starts.*
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void main(void)

{

	long	i;
	Rect	bounds;
	CTabHandle	clut_handle;
	
	/* Initialize managers and other mac stuff */
	init_program();

	/* Initialize menus */
	init_menus();

	/* Initialize windows */
	init_windows();

	/* Initialize the graphical data */
	init_graphics_data();

	/* Create a blank world */
	create_new_world();

	/* Go to the event loop, where everything actually happens */
	event_loop();
	
}	/* main() */