/*****
 * spion.c
 *
 *	A game under development
 *
 *
 *****/

#include "spion_resources.h"
#include "spion_proto.h"
#include "spion_defines.h"
#include <FixMath.h>


/**************************** Constants ***************************/

/* The rectangles containing the various status displays */
Rect		throttle_rect =	{232, 453, 243, 553};
Rect		bank_rect =		{257, 453, 268, 553};
Rect		bullets_rect =	{282, 453, 293, 553};
Rect		fuel_rect =		{307, 453, 318, 553};
Rect		radar_rect =	{74, 413, 202, 541};
Point		turret_center = {355, 468};

/* Colors for status displays */
RGBColor	danger_color =	{0xFFFF, 0x0000, 0x0000};	/* red */
RGBColor	status_color =	{0x0000, 0xFFFF, 0x0000};	/* green */
RGBColor	turret_color =	{0xFFFF, 0xFFFF, 0xFFFF};	/* white */
RGBColor	empty_color =	{0x0000, 0x0000, 0x0000};	/* black */



/***************************** Globals ****************************/

WindowPtr		main_window;		/* the main window */
CWindowRecord	main_window_rec;	/* storage for the main window's info */

long			main_window_left;	/* left edge of the window (in bytes from left) */
long			main_window_top;	/* top edge of the main window */
Rect			main_window_bounds;	/* content rect of main window */
long			main_window_width;	/* width of main window in bytes */
long			main_window_height;	/* height of main window */
char			*main_window_base;	/* base address of the window */
PixMapHandle	main_window_pixmap;	/* the Pixmap for the main window */
Boolean			screen_pixmap_32_flag;/* TRUE if we need 32-bit mode to write to screen */
long			screen_width;		/* width of screen in bytes */
Boolean			refresh_flag;		/* TRUE when a Vertical Blanking Interval has begun */


char			turn_left_key;		/* The keys used to control the ship */
char			turn_right_key;
char			stop_turn_key;
char			turret_left_key;
char			turret_right_key;
char			throttle_up_key;
char			throttle_down_key;
char			throttle_stop_key;
char			fire_key;

char			*offscreen_bitmap;	/* Pointer to the offscreen bitmap */
char			*radar_base;		/* Base address of radar on screen */

MenuHandle		apple_menu;			/* The apple menu */
MenuHandle		file_menu;			/* The file menu */
MenuHandle		edit_menu;			/* The edit menu */

Fixed			ship_direction;			/* Direction ship is facing */
short			ship_turret;			/* Direction turret is facing (0=fore) */
long			ship_throttle;			/* Current throttle */
long			ship_fuel;				/* Current fuel */
long			ship_bullets;			/* Current bullets */
Fixed			ship_speed_h;			/* Current horizontal speed */
Fixed			ship_speed_v;			/* Current vertical speed */
long			ship_rotation;			/* Rotation of the ship */
Boolean			ship_firing;			/* TRUE if we are now firing */
short			ship_turret_rotation;	/* direction, if any that the turret is rotating */
Fixed			throttle_const;			/* throttle constant */
Fixed			friction;				/* air friction constant */
Fixed			rotation_const;			/* rotational constant */
Fixed			h_vector[NUM_SHIPS];	/* h coordinate of a direction */
Fixed			v_vector[NUM_SHIPS];	/* v coordinate of a direction */
char			*ships[NUM_SHIPS];		/* Array of pointers to our ships */
char			*biplanes[NUM_BIPLANES];/* Array of pointers to biplanes */
char			*biplanes_masks[NUM_BIPLANES];/* Array of pointers to biplanes masks */
char			*ship_masks[NUM_SHIPS];	/* Array of pointers to masks for ship */
char			*terrains[NUM_TERRAINS];/* Array of pointers to terrains */

Fixed			ship_h;					/* Current horizontal position of ship */
Fixed			ship_v;					/* Current vertical position of ship */

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	short	fuse;
	} bullet;

short			num_bullets;			/* Number of bullets now on the screen */
bullet			bullets[MAX_BULLETS];	/* Bullets now on the screen */

Rect			vis_world_rect;			/* Part of world which is currently visible */

char			**world_handle, *world;	/* Pointer to world map data */




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

	WindowPtr	window;
	short		part = FindWindow (event->where, &window);
	
    switch (part)
      {
	  case inSysWindow: 
	    SystemClick (event, window);
	    break;
	    
	  case inMenuBar:
	  	do_menu(MenuSelect(event->where));
	    break;
	    
	  case inDrag:
	  	  break;
	  	  
	  case inContent:
	  	break;
	  	
	  case inGoAway:
	  	  break;
      }
      
}	/* handle_mouse_down() */



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
			
				case OPEN_ITEM:
					break;

				case SAVE_ITEM:
					break;
					
				case QUIT_ITEM:
									
					/* Remove VBL task */
			/*		remove_vbl_task();	*/
				
					/* Stop recognizing key-up events */
					SetEventMask (SysEvtMask);
				
					/* Quit */
					ExitToShell();

			}
			
			break;

		case EDIT_MENU:

			break;

	}
	
	HiliteMenu(0);

}	/* do_menu() */



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

	long	temp;

	/* Find the offset into the world map of the upper left corner of the window */
	hoffset = (ship_h >> 16) - (VIEW_WIDTH >> 1);
	voffset = (ship_v >> 16) - (VIEW_HEIGHT >> 1);

	/* Set up the visible region of the world, in world coordinates */
	vis_world_rect.left = hoffset;
	vis_world_rect.top = voffset;
	vis_world_rect.right = hoffset + VIEW_WIDTH;
	vis_world_rect.bottom = voffset + VIEW_HEIGHT;

	/* Find the offset of the upper left block into the world map */
	world_offset_h = hoffset/BLOCK_WIDTH;
	world_offset_v = voffset/BLOCK_HEIGHT;
	
	/* Start reading terrain at the upper left of this segment of the world map */
	current_world_map_block = world + world_offset_v*WORLD_WIDTH + world_offset_h;

	/* Find offsets of window into upper left block */
	hoffset &= 0x3F;
	voffset &= 0x3F;

	/* Find number of longwords in leftmost blocks, and any residual word or byte */
	total_left_bytes = BLOCK_HEIGHT - hoffset;
	left_longwords = total_left_bytes >> 2;
	left_byte_flag = total_left_bytes & 0x1;
	left_word_flag = (total_left_bytes >> 1) & 0x1;

	/* Find number of longwords in rightmost blocks, and any residual word or byte */
	total_right_bytes = BLOCK_WIDTH - total_left_bytes;
	right_longwords = total_right_bytes >> 2;
	right_byte_flag = total_right_bytes & 0x1;
	right_word_flag = (total_right_bytes >> 1) & 0x1;

	/* Find how far into its block the bottom line extends (skip all
		lower lines in that block) */
	bottom_edge_offset = (voffset+VIEW_HEIGHT) & 0x3F;

	/* Find the number of blocks per screen vertically */
	num_vert_blocks = ((VIEW_HEIGHT+voffset)/BLOCK_HEIGHT) + 1;

	/* Find the number of blocks in the window horizontally */
	num_horiz_blocks = ((VIEW_WIDTH+hoffset)/BLOCK_WIDTH) + 1;

	/* Start at the upper left corner of the window */
	line_base = left_block_base = offscreen_bitmap;
	
	/* Remember that we're starting at the top of the window */
	topmost_block_flag = TRUE;
	
	/* loop through the vertical blocks */
	for (vert = 0; vert < num_vert_blocks; vert++)
		{
		

		/* Start the block pointer at the left */
		line_base = left_block_base;
		
		/* If we're at the top of the window, skip lines as necessary */
		start_line = (topmost_block_flag) ? voffset : 0;
		
		/* If we're at the bottom of the window, skip lines as necessary */
		end_line = (vert == (num_vert_blocks - 1)) ? bottom_edge_offset: BLOCK_HEIGHT;
		
		/* Don't do anything if there's nothing to draw */
		if (start_line == end_line)
			continue;
		
		/* Draw a row of blocks */

		asm {
		
			moveq.l	#0, d0
			move.l	current_world_map_block, a0		; find the terrain type for this block
			move.b	(a0), d0						
			move.l	(terrains, d0*4), terrain_base	; find the base address of the pattern
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
		
			addi.l	#VIEW_WIDTH, line_base			; Point to next line of blocks */
			sub.l	total_left_bytes, line_base;
			
			dbra	d1, @left_line_loop
		
			move.l	left_block_base, block_base		; point to next destination block
			add.l	total_left_bytes, block_base
			
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
			move.l	(terrains, d0*4), terrain_base	; find the base address of the pattern
			move.l	start_line, d0					; offset by number of lines skipped
			asl.l	#6, d0
			add.l	d0, terrain_base
			
			move.l	block_base, line_base			; start at top of block
			
			/* Draw a middle block */
			
			move.l	end_line, d1					; count start_line - end_line lines
			sub.l	start_line, d1
			subq.l	#1, d1
			
		@middle_line_loop:
		
			move.l	#15, d0							; loop through longwords of the block
			
		@middle_lw_loop:							; draw a line of the block
			move.l	(terrain_base)+, (line_base)+
			dbra	d0, @middle_lw_loop
			
			addi.l	#(VIEW_WIDTH-BLOCK_WIDTH), line_base ; point to next line of block
			
			dbra	d1, @middle_line_loop
			
			addi.l	#BLOCK_WIDTH, block_base		; point to next destination block
			addq.l	#1, current_world_map_block		; point to next world block
			
			subq.l	#1, block						; go to next block
			bne		@middle_block_loop
			
			
			/* Draw the right block */

			move.l	block_base, line_base			; point to next destination block
			
			moveq.l	#0, d0
			move.l	current_world_map_block, a0		; find the terrain type for this block
			move.b	(a0), d0						
			move.l	(terrains, d0*4), terrain_base	; find the base address of the pattern
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
		
			addi.l	#BLOCK_WIDTH, terrain_base		; skip pattern to right of window
			sub.l	total_right_bytes, terrain_base
			
			addi.l	#VIEW_WIDTH, line_base			; Point to next line of blocks */
			sub.l	total_right_bytes, line_base;
			
			dbra	d1, @right_line_loop
			
			}

		/* We're no longer at the topmost block */
		topmost_block_flag = FALSE;
		
		/* update the vertical block counter to point to the next block */
		left_block_base += VIEW_WIDTH*(end_line-start_line);
		
		/* Go to the next horizontal world map block */
		current_world_map_block += (WORLD_WIDTH - num_horiz_blocks + 1);

		}	/* End of vert line loop */


}	/* draw_background() */



/*****************************************************************************\
* procedure draw_ship                                                         *
*                                                                             *
* Purpose: This procedure draws the ship at the center of the main window.    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 5, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_ship(void)
{

	register char	*ship_image_base;	/* base address of ship image in memory */
	register char	*ship_mask_base;	/* base address of ship mask in memory */
	register char	*ship_dest_base;	/* base address of ship destination on screen */
	short			ship_left, ship_top;
	short			i;

	/* Find the upper left corner of the ship's location, in local coordinates */
	ship_left = (VIEW_WIDTH >> 1) - (SHIP_WIDTH >> 1);
	ship_top = (VIEW_HEIGHT >> 1) - (SHIP_HEIGHT >> 1);

	/* find the base address of the ship's location */
	ship_dest_base = offscreen_bitmap + ship_top * VIEW_WIDTH + ship_left;
	
	/* find the base address of the ship's image and mask in memory */
	ship_image_base = ships[ship_direction >> 16];
	ship_mask_base = ship_masks[ship_direction >> 16];

	/* Write the ship to the screen, a byte at a time */
	for (i=0; i<SHIP_HEIGHT; i++)
		{
		
		/* Mask this line of the ship into memory */
		asm	{
		
			; Repeat eight times, once for each longword per line
			
			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			move.l	(ship_dest_base), d0
			and.l	(ship_mask_base)+, d0
			or.l	(ship_image_base)+, d0
			move.l	d0, (ship_dest_base)+

			}
		
		/* Go to next line on screen */
		ship_dest_base += VIEW_WIDTH - SHIP_WIDTH;
		}
	

}	/* draw_ship() */



/*****************************************************************************\
* procedure draw_bullets                                                      *
*                                                                             *
* Purpose: This procedure draws the bullets at the center of the main window. *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 5, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bullets(void)
{

	short			i;
	bullet			this_bullet;	/* Bullet we're currently look at */
	register char	*bullet_base;	/* place in bitmap to draw bullet */

	/* Loop through the bullets, and draw each one */
	for (i = 0; i < num_bullets; i++)
		{
		
		/* Get this bullet */
		BlockMove(&bullets[i], &this_bullet, sizeof(bullet));
		
		/* Ignore off-screen bullets */
		if (((this_bullet.h >> 16) > vis_world_rect.right) ||
			((this_bullet.h >> 16) < vis_world_rect.left) ||
			((this_bullet.v >> 16) > vis_world_rect.bottom) ||
			((this_bullet.v >> 16) < vis_world_rect.top))
			
			continue;
		
		/* Draw this bullet */
		bullet_base = offscreen_bitmap +
						((this_bullet.v >> 16) - vis_world_rect.top) * VIEW_WIDTH +
						((this_bullet.h >> 16) - vis_world_rect.left);
		
		asm	{
			move.w	#BULLET_WORD, (bullet_base)		; draw the first word
			add.l	#VIEW_WIDTH, bullet_base		; move to directly below it
			move.w	#BULLET_WORD, (bullet_base)		; draw the second word
			}
			
		}
		

}	/* draw_bullets() */



/*****************************************************************************\
* procedure update_window                                                     *
*                                                                             *
* Purpose: This procedure copies the offscreen bitmap to the window.          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 5, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void update_window(Boolean offset)
{

	register char	*source;
	register char	*dest;
	register long	line_count;
	register long	bytes_between_dest_lines;
	char			mode = true32b;		/* Something about 32/24-bit mode */


	/* Switch to 32-bit mode if necessary */
	if (screen_pixmap_32_flag) SwapMMUMode( &mode );
	
	/* Hide the cursor so we don't draw over it */
/*	HideCursor();	*/

	asm{
	
		move.l	offscreen_bitmap, source	; copy from bitmap
		move.l	main_window_base, dest		; copy to window
		
		btst.b	#0, offset					; If there is an offset,
		beq		@no_offset					; draw the odd lines
		
		add.l	screen_width, dest
		addi.l	#VIEW_WIDTH, source
		
	@no_offset:	
		
		move.l	screen_width, bytes_between_dest_lines
		asl.l	#1, bytes_between_dest_lines
		sub.l	#384, bytes_between_dest_lines
		
		move.l	#191, line_count			; copy 384 lines
		
	@line_loop:

		; Copy 384 bytes (96 longwords) per line
		
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+
		move.l	(source)+, (dest)+

		add.l	bytes_between_dest_lines, dest		; point to next line
		addi.l	#VIEW_WIDTH, source

		dbra	line_count, @line_loop		; go to next line

		}
		
	/* Show the cursor again */
/*	ShowCursor();	*/

	/* Switch back to 24-bit mode if necessary */
    if (screen_pixmap_32_flag) SwapMMUMode( &mode );
	
}	/* update_window() */



/*****************************************************************************\
* procedure update_radar                                                      *
*                                                                             *
* Purpose: This procedure draws the radar screen.                             *
*                                                                             *
* Parameters: first_time: TRUE if the radar has never been drawn before       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void update_radar(Boolean first_time)
{

	static char	*ship_position;	/* position of ship on radar */
	
#define SHIP_COLOR	0xFFFF

	/* If this is not the first time, get rid of the previous ship */
	if (!first_time)
		{
		/* Draw the ship blip */
		asm {
			move.l	ship_position, a0
			eor.w	#SHIP_COLOR, (a0)
			add.l	screen_width, a0
			eor.w	#SHIP_COLOR, (a0)
			}		
		}

	/* Find base address of current ship position */
	ship_position = radar_base + (ship_v >> 22)*screen_width + (ship_h >> 22);
	
	/* Draw the ship blip */
	asm {
		move.l	ship_position, a0
		eor.w	#SHIP_COLOR, (a0)
		add.l	screen_width, a0
		eor.w	#SHIP_COLOR, (a0)
		}		
	
}	/* update_radar() */



/*****************************************************************************\
* procedure draw_turret                                                       *
*                                                                             *
* Purpose: This procedure draws the gun turret at its current position.       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_turret(void)
{

	static short	last_turret = 1;

	/* Don't do anything unless the turret has changed */
	if (last_turret == ship_turret)
		return;

	/* Erase the last turret indicator */
	RGBForeColor (&empty_color);
	draw_turret_indicator(last_turret);

	/* Draw the current turret indicator */
	RGBForeColor (&turret_color);
	draw_turret_indicator(ship_turret);

	/* Remember the current turret */
	last_turret = ship_turret;
	
}	/* draw_turret() */



/*****************************************************************************\
* procedure draw_turret_indicator                                             *
*                                                                             *
* Purpose: This procedure draws the indicator line for the gun turret         *
*          display.                                                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_turret_indicator(short direction)
{

	/* Set the pen to a larger size */
	PenSize(2, 2);

	/* Draw the line */
	MoveTo(turret_center.h, turret_center.v);
	LineTo(turret_center.h + ((h_vector[direction] * 19L) >> 16),
			turret_center.v + ((v_vector[direction] * 19L) >> 16));

	/* Set the pen to the normal size */
	PenSize(1, 1);

}	/* draw_turret_indicator() */



/*****************************************************************************\
* procedure draw_throttle                                                     *
*                                                                             *
* Purpose: This procedure draws the throttle at its current position.         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_throttle(void)
{

	/* Draw the throttle */
	draw_thermometer(&throttle_rect, ship_throttle, MAX_THROTTLE, FALSE);
	
}	/* draw_throttle() */



/*****************************************************************************\
* procedure draw_bullets_status                                               *
*                                                                             *
* Purpose: This procedure draws the bullets status thermometer at its current *
*          value.                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bullets_status(void)
{

	/* Draw the bullets status thermometer */
	draw_thermometer(&bullets_rect, ship_bullets, MAX_BULLETS, FALSE);
	
}	/* draw_bullets_status() */



/*****************************************************************************\
* procedure draw_fuel_status                                                  *
*                                                                             *
* Purpose: This procedure draws the fuel status thermometer at its current    *
*          value.                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_fuel_status(void)
{

	/* Draw the fuel status thermometer */
	draw_thermometer(&fuel_rect, ship_fuel, MAX_FUEL, FALSE);
	
}	/* draw_fuel_status() */



/*****************************************************************************\
* procedure draw_bank_status                                                  *
*                                                                             *
* Purpose: This procedure draws the bank status thermometer at its current    *
*          value.                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bank_status(void)
{

	Rect	rect;
	short	length = (bank_rect.right - bank_rect.left);
	short	midpoint = length >> 1;
	short	chunk = (length - 6)/(2*MAX_ROTATION);
	short	mark;

	/* Start with the whole rectangle */
	
	/* Do the blanking first */
	RGBForeColor (&empty_color);
	
	/* Find the mark */
	mark = midpoint + chunk * ship_rotation;
	
	/* Fill to the left of the mark */
	BlockMove(&bank_rect, &rect, 8);
	rect.right = rect.left + mark - 3;
	PaintRect (&rect);
	
	/* Fill to the right of the mark */
	BlockMove(&bank_rect, &rect, 8);
	rect.left += (mark + 3);
	PaintRect (&rect);
		
	/* Now do the filling */
	RGBForeColor(&status_color);
	
	/* Fill in the mark */
	BlockMove(&bank_rect, &rect, 8);
	rect.left += (mark - 3);
	rect.right = rect.left + 6;
	PaintRect (&rect);
	
}	/* draw_bank_status() */



/*****************************************************************************\
* procedure draw_thermometer                                                  *
*                                                                             *
* Purpose: This procedure draws a thermometer.                                *
*                                                                             *
* Parameters: therm_rect:	the rectangle in which to draw the thermometer    *
*             current_value:the current value of the thermometer              *
*             max_value:	the maximum possible value of the thermometer     *
*             danger:		TRUE if we should draw in danger color            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_thermometer(Rect *therm_rect, long current_value, long max_value, Boolean danger)
{

	Rect	rect;
	short	bar_length;

	/* Find the length of the bar */
	bar_length = (therm_rect->right - therm_rect->left) * current_value / max_value;

	/* Find the rectangle for the full part */
	BlockMove(therm_rect, &rect, 8);
	rect.right = rect.left + bar_length;
	
	/* Set the color to danger for a danger display or to status for a status display */
	RGBForeColor ( (danger) ? &danger_color : &status_color );
	
	/* Fill in the full part */
	PaintRect (&rect);
	
	/* Find the rectangle for the empty part */
	BlockMove(therm_rect, &rect, 8);
	rect.left += bar_length;
	
	/* Set the color to the empty color */
	RGBForeColor (&empty_color);
	
	/* Fill the empty part */
	PaintRect (&rect);
		
}	/* draw_thermometer() */



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
	char	the_char;

	/* Check for menu shortcut */
	if (event->modifiers & cmdKey)
		{
				
		/* Find menu equivalent */
		menu_result = MenuKey(event->message);
		
		/* Pretend user clicked menu */
		if (HiWord(menu_result) != 0)	/* valid key? */
			do_menu (menu_result);		/* yes, map it to menu item */
		}
		
	/* Check for rotation */
	else
		{
		the_char = event->message & charCodeMask;
		
		if (the_char == turn_left_key)
			ship_rotation--;
			
		else if (the_char == turn_right_key)
			ship_rotation++;
			
		else if (the_char == stop_turn_key)
			ship_rotation = 0;
			
		if (the_char == turret_left_key)
			ship_turret_rotation = -1;
			
		else if (the_char == turret_right_key)
			ship_turret_rotation = 1;
			
		else if (the_char == throttle_up_key)
			ship_throttle++;
			
		else if (the_char == throttle_down_key)
			ship_throttle--;
			
		else if (the_char == throttle_stop_key)
			ship_throttle = 0;
			
		else if (the_char == fire_key)
			ship_firing = TRUE;
			
		/* Throttle may not exceed MAX_THROTTLE */
		if (ship_throttle > MAX_THROTTLE)
			ship_throttle = MAX_THROTTLE;
		
		/* No negative throttles allowed */
		if (ship_throttle < 0)
			ship_throttle = 0;
		
		/* Rotation may not exceed MAX_ROTATION or drop below -MAX_ROTATION */
		if (ship_rotation > MAX_ROTATION)
			ship_rotation = MAX_ROTATION;
		if (ship_rotation < -MAX_ROTATION)
			ship_rotation = -MAX_ROTATION;
		
		/* Update the turret display */
		draw_turret();

		/* Update the turret display */
		draw_throttle();

		/* Update the bank thermometer */
		draw_bank_status();

		}

}	/* handle_key_down() */



/*****************************************************************************\
* procedure handle_key_up                                                     *
*                                                                             *
* Purpose: This procedure handles a key up event.                             *
*                                                                             *
* Parameters: event: the event.                                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void handle_key_up(EventRecord *event)
{

	short 	item_hit;
	long	menu_result;
	char	the_char;

	the_char = event->message & charCodeMask;
	
	/* Stop rotating the turret when the user lets up the rotate key */
	if ((the_char == turret_left_key) || (the_char == turret_right_key))
		ship_turret_rotation = 0;

	/* Stop firing when the user lets up the fire key */		
	else if (the_char == fire_key)
		ship_firing = FALSE;
			
}	/* handle_key_up() */



/*****************************************************************************\
* procedure update_frame                                                      *
*                                                                             *
* Purpose: This procedure calculates and draws one frame of the game.         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void update_frame(void)
{

	long	last_time;
	Fixed	speed_squared;
	Fixed	new_speed;
	Fixed	ship_speed;
	Fixed	decel;
	Fixed	speed_ratio;
	Fixed	accel_const;
	Fixed	accel;
	Fixed	h_accel;
	Fixed	v_accel;
	short	i;
	bullet	new_bullet;   
	short	bullet_direction;

	/* Scroll another notch */
	draw_background();

	/* Draw the bullets */
	draw_bullets();

	/* Draw our ship */
	draw_ship();
	
#if 0
	/* Wait for VBL */
	refresh_flag = FALSE;
	while (refresh_flag == FALSE);
#endif
	
	/* Update the screen from the offscreen bitmap */
	update_window(FALSE);
	
	/* Update the radar */
	update_radar(FALSE);

#if 0
	/* Wait for VBL */
	refresh_flag = FALSE;
	while (refresh_flag == FALSE);
#endif
	
	/* Update the screen from the offscreen bitmap */
	update_window(TRUE);

	/* Loop through the bullets, updating as we go */
	for (i=0; i<num_bullets; i++)
		{
		
		/* Shorten the fuse.  If this bullet's fuse has run out, kill it */
		if (!(--bullets[i].fuse))
			{
			
			/* We now have one fewer bullets */
			num_bullets--;
			
			/* If it was the last bullet on the list, no problem */
			if (i == num_bullets)
				continue;
				
			/* Otherwise, overwrite its record with that of the last bullet */
			BlockMove(&bullets[num_bullets], &bullets[i], sizeof(bullet));
			
			/* Remember to update the bullet which WAS last */
			i--;
			
			}
			
		else	/* This is still a live bullet */
			{
			
			/* move it to its new location */
			bullets[i].h += bullets[i].speed_h;
			bullets[i].v += bullets[i].speed_v;
			
			}
		}			

	/* Move ship to new position */
	ship_h += ship_speed_h;
	ship_v += ship_speed_v;

	/* Bounce the ship off the edges of the world */
	if (ship_h < (BLOCK_WIDTH * WORLD_MARGIN) << 16)
		
		/* Make sure ship is actually pointing left */
		if (ship_direction > (16L << 16))
			ship_direction = (32L << 16) - (ship_direction & 0xFFFF0000);
		

	if (ship_v < (BLOCK_HEIGHT * WORLD_MARGIN) << 16)

		/* Make sure ship is actually pointing up */
		if ((ship_direction < (8L << 16)) || (ship_direction > (24L << 16)))
			ship_direction = (ship_direction > (16L << 16)) ?
							((48L << 16) - (ship_direction & 0xFFFF0000)) :
							((16L << 16) - (ship_direction & 0xFFFF0000));

	if (ship_h > (BLOCK_WIDTH * (WORLD_WIDTH - WORLD_MARGIN)) << 16)

		/* Make sure ship is actually pointing right */
		if (ship_direction < (16L << 16))
			ship_direction = (32L << 16) - (ship_direction & 0xFFFF0000);

	if (ship_v > (BLOCK_HEIGHT * (WORLD_HEIGHT - WORLD_MARGIN)) << 16)
	
		/* Make sure ship is actually pointing down */
		if ((ship_direction > (8L << 16)) && (ship_direction < (24L << 16)))
			ship_direction = (ship_direction > (16L << 16)) ?
								((48L << 16) - (ship_direction & 0xFFFF0000)) :
								((16L << 16) - (ship_direction & 0xFFFF0000));

	/* Find the air resistance on the ship */
	speed_squared = (ship_speed_h >> 8)*(ship_speed_h >> 8) +
						(ship_speed_v >> 8)*(ship_speed_v >> 8);
	ship_speed = (FracSqrt((speed_squared/256) << 14) >> 14) * 16;

	/* Ensure that speed is non-zero */
	if (!ship_speed) ship_speed = 0x00000100;
	decel = (speed_squared >> 8) * (friction >> 8);
	new_speed = ship_speed - decel;
	
	/* Find the throttle acceleration on the ship */
	accel = ship_throttle * ship_throttle * throttle_const;

	/* Find new speed due to thrust */
	new_speed += accel;
	
	/* Find new velocity */
	ship_speed_h = (h_vector[ship_direction >> 16] >> 8) * (new_speed >> 8);
	ship_speed_v = (v_vector[ship_direction >> 16] >> 8) * (new_speed >> 8);

	/* If the ship is rotating, rotate it according to the speed */
	if (ship_rotation)
		{
		ship_direction += ship_rotation * (rotation_const >> 8) * (ship_speed >> 8);

		/* Constrain ship rotation to number of available frames */
		if (ship_direction < 0)
			ship_direction += NUM_SHIPS << 16;
		else if (ship_direction > (NUM_SHIPS << 16))
			ship_direction -= NUM_SHIPS << 16;
		}


	/* If the ship is firing, fire another bullet */
	if (ship_firing)
		{
		/* If we already have the max number of bullets, don't do anything */
		if (num_bullets == MAX_BULLETS)
			return;
		
		/* Find the direction this bullet will be travelling */
		bullet_direction = ((ship_direction >> 16) + ship_turret) % NUM_SHIPS;
		
		/* Find the velocity of a bullet in the turret direction */
		new_bullet.speed_h = BULLET_SPEED * h_vector[bullet_direction];
		new_bullet.speed_v = BULLET_SPEED * v_vector[bullet_direction];

		/* Move the bullet away from the ship */
		new_bullet.h = ship_h + h_vector[bullet_direction] * 20L - (1L << 16);
		new_bullet.v = ship_v + v_vector[bullet_direction] * 20L - (1L << 16);
		
		/* Add the current ship velocity to that of the bullet */
		new_bullet.speed_h += ship_speed_h;
		new_bullet.speed_v += ship_speed_v;

		/* Start the bullet countdown */
		new_bullet.fuse = BULLET_FUSE_LENGTH;		
	
		/* Copy this bullet to the bullet list */
		BlockMove(&new_bullet, &bullets[num_bullets], sizeof(bullet));
	
		/* We now have one more bullet */
		num_bullets++;
		
		}
	
	/* If the ship's gun turret is rotating, rotate it another notch */
	if (ship_turret_rotation)
		{
		ship_turret += ship_turret_rotation;
	
		/* Constrain turret to 360 degrees */
		if (ship_turret < 0)
			ship_turret += NUM_SHIPS;
		if (ship_turret >= NUM_SHIPS)
			ship_turret -= NUM_SHIPS;
			
		/* Draw the turret in its new position */
		draw_turret();
		}

}	/* update_frame() */



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

		/* Do another frame of the game */
		update_frame();

		good = WaitNextEvent (everyEvent, &event, 0, (RgnHandle) 0L);
		
		if (good)
		  switch (event.what)
		    {
			case mouseDown:
				handle_mouse_down(&event);
				break;
				
			case keyUp: 
				handle_key_up(&event);
				break;

			case keyDown: 
			case autoKey:
				handle_key_down(&event);
				break;
				
			case updateEvt:
			    break;
			    
			case activateEvt:
				break;
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

	/* Initialize the program data */
	init_data();
	
	/* Initialize menus */
	init_menus();

	/* Initialize windows */
	init_windows();

	/* Initialize the graphics data */
	init_graphics_data();
	
	/* Initialize for a new game */
	init_new_game();

	/* Go to the event loop */
	event_loop();
	
}	/* main() */
