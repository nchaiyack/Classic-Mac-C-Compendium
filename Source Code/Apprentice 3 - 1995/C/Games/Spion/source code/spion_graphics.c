/*****
 * spion_graphics.c
 *
 *	Spion is a game under development.  This segment performs the drawing for the game
 *
 *
 *****/


#include "spion_resources.h"
#include "spion_types.h"
#include "spion_proto.h"
#include "spion_defines.h"
#include "spion_externs.h"


/**************************** Globals ***************************/

char			*ships[NUM_SHIPS];						/* Array of pointers to our ships */
char			*ship_masks[NUM_SHIPS];					/* Array of pointers to masks for ship */
char			*biplanes[NUM_BIPLANES];				/* Array of pointers to biplanes */
char			*biplanes_masks[NUM_BIPLANES];			/* Array of pointers to biplanes masks */
char			*enemy_jets[NUM_ENEMY_JETS];			/* Array of pointers to enemy jets */
char			*enemy_jets_masks[NUM_ENEMY_JETS];		/* Array of pointers to enemy jets masks */
char			*flying_wings[NUM_FLYING_WINGS];		/* Array of pointers to flying wings */
char			*flying_wings_masks[NUM_FLYING_WINGS];	/* Array of pointers to flying wings masks */
char			*blaster_enemies[NUM_BLASTER_ENEMIES];	/* Array of pointers to blaster enemies */
char			*blaster_enemies_masks[NUM_BLASTER_ENEMIES];/* Array of pointers to blaster enemies masks */
char			*tracer_enemies[NUM_TRACER_ENEMIES];	/* Array of pointers to tracer enemies */
char			*tracer_enemies_masks[NUM_TRACER_ENEMIES];/* Array of pointers to tracer enemies masks */
char			*circle_enemies[NUM_CIRCLE_ENEMIES];	/* Array of pointers to circle enemies */
char			*circle_enemies_masks[NUM_CIRCLE_ENEMIES];/* Array of pointers to circle enemies masks */
char			*laser_guns[NUM_LASER_GUNS];			/* Array of pointers to laser guns */
char			*laser_guns_masks[NUM_LASER_GUNS];		/* Array of pointers to laser guns masks */
char			*fireballs[NUM_FIREBALLS];				/* Array of pointers to fireballs */
char			*fireballs_masks[NUM_FIREBALLS];		/* Array of pointers to fireballs masks */
char			*tracers_picts[NUM_TRACERS];			/* Array of pointers to tracers */
char			*tracers_masks[NUM_TRACERS];			/* Array of pointers to tracers masks */
char			*missiles_picts[NUM_MISSILES];			/* Array of pointers to tracers */
char			*missiles_masks[NUM_MISSILES];			/* Array of pointers to tracers masks */
char			*guns[NUM_GUNS];						/* Array of pointers to guns */
char			*guns_masks[NUM_GUNS];					/* Array of pointers to guns masks */
char			*cannons[NUM_CANNONS];					/* Array of pointers to cannons */
char			*cannons_masks[NUM_CANNONS];			/* Array of pointers to cannons masks */
char			*silos[NUM_SILOS];						/* Array of pointers to silos */
char			*silos_masks[NUM_SILOS];				/* Array of pointers to silos masks */
char			*explosions_picts[NUM_EXPLOSIONS];		/* Array of pointers to explosions */
char			*explosions_masks[NUM_EXPLOSIONS];		/* Array of pointers to explosions masks */
char			*base_tops[NUM_BASES];					/* Array of pointers to tops of bases */
char			*base_tops_masks[NUM_BASES];			/* Array of pointers to tops of bases masks */
char			*base_bottoms[NUM_BASES];				/* Array of pointers to bottoms of bases */
char			*base_bottoms_masks[NUM_BASES];			/* Array of pointers to bottoms of bases masks */
char			*dead_bases[NUM_BASES];					/* Array of pointers to dead bases */
char			*dead_bases_masks[NUM_BASES];			/* Array of pointers to dead bases masks */
char			*goodies_picts[NUM_GOODIES];			/* Array of pointers to goodies */
char			*goodies_masks[NUM_GOODIES];			/* Array of pointers to goodies masks */
char			*terrains[NUM_TERRAINS];				/* Array of pointers to terrains */
char			*bomb_pict;								/* Pointer to the bomb */
char			*bomb_mask;								/* Pointer to the bomb mask */
char			*large_bomb_pict;						/* Pointer to the large bomb */
char			*large_bomb_mask;						/* Pointer to the large bomb mask */
char			*shell_pict;							/* Pointer to the shell */
char			*shell_mask;							/* Pointer to the shell mask */
char			*spiked_ball_pict;						/* Pointer to the spiked ball */
char			*spiked_ball_mask;						/* Pointer to the spiked ball mask */

short			ship_color_word;		/* word to write to radar to represent ship */
short			biplane_color_word;		/* word to write to radar to represent biplane */
short			cannon_color_word;		/* word to write to radar to represent cannon */
short			ground_gun_color_word;	/* word to write to radar to represent ground gun */
short			fireball_color_word;	/* word to write to radar to represent explosion */
short			silo_color_word;		/* word to write to radar to represent silo */
short			missile_color_word;		/* word to write to radar to represent missile */

CGrafPort		offscreen_view_port;/* Pointer to the offscreen port */
char			*offscreen_bitmap;	/* Pointer to the offscreen bitmap */
char			*radar_base;		/* Base address of radar on screen */




/**************************** Constants ***************************/


/* The rectangles containing the various status displays */
Rect		throttle_rect =	{271, 505, 282, 605};
Rect		damage_rect =	{296, 505, 307, 605};
Rect		energy_rect =	{321, 505, 332, 605};
Rect		radar_rect =	{113, 465, 241, 593};
Point		turret_center = {393, 520};

Rect		weapons_lock_rect = {340, 443, 367, 507};
Rect		lock_detected_rect = {340, 551, 367, 614};

Rect		leftmost_weapon_rect = 	{4, 140, 36, 172};
Rect		topmost_ability_rect =	{62, 11, 94, 43};
short		weapons_spacing = 48;
short		abilities_spacing = 44;

/* Colors for status displays */
RGBColor	danger_color =	{0xFFFF, 0x0000, 0x0000};	/* red */
RGBColor	status_color =	{0x0000, 0xFFFF, 0x0000};	/* green */
RGBColor	turret_color =	{0xFFFF, 0xFFFF, 0xFFFF};	/* white */
RGBColor	empty_color =	{0x0000, 0x0000, 0x0000};	/* black */

/* Color for laser beam */
RGBColor	laser_beam_color = {0xFFFF, 0x0000, 0x0000};	/* red */


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
	hoffset = ((*the_ship)->h >> 16) - (VIEW_WIDTH >> 1);
	voffset = ((*the_ship)->v >> 16) - (VIEW_HEIGHT >> 1);

	/* Set up the visible region of the world, in world coordinates */
	vis_world_rect.left = hoffset;
	vis_world_rect.top = voffset;
	vis_world_rect.right = hoffset + VIEW_WIDTH;
	vis_world_rect.bottom = voffset + VIEW_HEIGHT;

	/* Find the offset of the upper left block into the world map */
	world_offset_h = hoffset/BLOCK_WIDTH;
	world_offset_v = voffset/BLOCK_HEIGHT;
	
	/* Start reading terrain at the upper left of this segment of the world map */
	current_world_map_block = world + world_offset_v*world_width + world_offset_h;

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
		current_world_map_block += (world_width - num_horiz_blocks + 1);

		}	/* End of vert line loop */


}	/* draw_background() */



/*****************************************************************************\
* procedure draw_laser_beams                                                  *
*                                                                             *
* Purpose: This procedure draws any visible laser beams.                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void draw_laser_beams(void)
{

	short			i;
	register laser_beam_pointer	this_laser_beam;	/* Laser beam we're currently look at */

	/* Draw again in the main window */
	SetPort(&offscreen_view_port);

	/* Draw with the laser color */
	RGBForeColor(&laser_beam_color);
		
	/* Loop through the laser beams, and draw each one */
	for (i = 0; i < num_laser_beams; i++)
		{
		
		/* Dereference this laser beam */
		this_laser_beam = *laser_beams[i];
		
		/* Set the pattern to greyed if phased */
		PenPat(this_laser_beam->phased ? gray : black);

		/* Draw this laser_beam */
		MoveTo((this_laser_beam->start_h >> 16) - vis_world_rect.left,
				(this_laser_beam->start_v >> 16) - vis_world_rect.top);
		LineTo((this_laser_beam->end_h >> 16) - vis_world_rect.left,
				(this_laser_beam->end_v >> 16) - vis_world_rect.top);
		
		}	/* loop through laser_beams */
		
	/* Draw again in the main window */
	SetPort(main_window);

}	/* draw_laser_beams() */



/*****************************************************************************\
* procedure draw_blasters                                                     *
*                                                                             *
* Purpose: This procedure draws any visible blasters.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void draw_blasters(void)
{

	short						i;
	register blaster_pointer	this_blaster;	/* Blaster we're currently look at */

	/* Draw offscreen */
	SetPort(&offscreen_view_port);

	/* Draw with the laser color */
	RGBForeColor(&laser_beam_color);
		
	/* Loop through the blasters, and draw each one */
	for (i = 0; i < num_blasters; i++)
		{
		
		/* Dereference this blaster */
		this_blaster = *blasters[i];
		
		/* Set the pattern to greyed if phased */
		PenPat(this_blaster->phased ? gray : black);
		
		/* Draw this blaster */
		MoveTo((this_blaster->start_h >> 16) - vis_world_rect.left,
				(this_blaster->start_v >> 16) - vis_world_rect.top);
		LineTo((this_blaster->end_h >> 16) - vis_world_rect.left,
				(this_blaster->end_v >> 16) - vis_world_rect.top);
				
		}	/* loop through blasters */
		
	/* Draw again in the main window */
	SetPort(main_window);

}	/* draw_blasters() */



/*****************************************************************************\
* procedure draw_circles                                                      *
*                                                                             *
* Purpose: This procedure draws any visible circles.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void draw_circles(void)
{

	short					i;
	register circle_pointer	this_circle;	/* Circle we're currently look at */
	Rect					circle_rect;

	/* Draw again in the main window */
	SetPort(&offscreen_view_port);

	/* Draw with the laser color */
	RGBForeColor(&laser_beam_color);
		
	/* Loop through the circles, and draw each one */
	for (i = 0; i < num_circles; i++)
		{
		
		/* Dereference this circle */
		this_circle = *circles[i];
		
		/* Set the pattern to greyed if phased */
		PenPat(this_circle->phased ? gray : black);
		
		/* Draw this circle */
		circle_rect.left = ((this_circle->h >> 16) - this_circle->radius) - vis_world_rect.left;
		circle_rect.top = ((this_circle->v >> 16) - this_circle->radius) - vis_world_rect.top;
		circle_rect.right = circle_rect.left + (this_circle->radius << 1);
		circle_rect.bottom = circle_rect.top + (this_circle->radius << 1);
		FrameOval (&circle_rect);
		
		}	/* loop through circles */
		
	/* Draw again in the main window */
	SetPort(main_window);

}	/* draw_circles() */



/*****************************************************************************\
* procedure draw_bullets                                                      *
*                                                                             *
* Purpose: This procedure draws any visible bullets.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 5, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bullets(void)
{

	short			i;
	register bullet_pointer	this_bullet;	/* Bullet we're currently look at */
	register char	*bullet_base;	/* place in bitmap to draw bullet */
	short			bullet_word1, bullet_word2;

	/* Loop through the bullets, and draw each one */
	for (i = 0; i < num_bullets; i++)
		{
		
		/* Dereference this bullet */
		this_bullet = *bullets[i];
		
		/* Only pay attention to bullets in the view */
		if (((this_bullet->h >> 16) < vis_world_rect.right) &&
			((this_bullet->h >> 16) + 1 > vis_world_rect.left) &&
			((this_bullet->v >> 16) < vis_world_rect.bottom) &&
			((this_bullet->v >> 16) + 1 > vis_world_rect.top))
		
			{
		
			/* Find what to write to the bitmap */
			if (this_bullet->phased)
				{
				bullet_word1 = this_bullet->phase_mask;
				bullet_word2 = bullet_word1 ^ 0xFFFF;
				this_bullet->phase_mask ^= 0xFFFF;
				}
			else
				{
				bullet_word1 = 0x0000;
				bullet_word2 = 0x0000;
				}
				
			/* Draw this bullet */
			bullet_base = offscreen_bitmap +
							((this_bullet->v >> 16) - vis_world_rect.top) * VIEW_WIDTH +
							((this_bullet->h >> 16) - vis_world_rect.left);
			
			asm	{
				move.w	bullet_word1, (bullet_base)		; draw the first word
				add.l	#VIEW_WIDTH, bullet_base		; move to directly below it
				move.w	bullet_word2, (bullet_base)		; draw the second word
				}
			
			}
		
		}	/* loop through bullets */
		

}	/* draw_bullets() */



/*****************************************************************************\
* procedure draw_bombs                                                        *
*                                                                             *
* Purpose: This procedure draws any visible bombs.                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 22, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bombs(void)
{

	short			i, j;
	register bomb_pointer	this_bomb;		/* Bomb we're currently looking at */
	register char	*bomb_dest_base;/* place in bitmap to draw bomb */
	register char	*bomb_base;		/* bomb bitmap in memory */
	char	*mask_base;		/* bomb mask in memory */
	short			phase_mask1, phase_mask2;

	/* Loop through the bombs, and draw each one */
	for (i = 0; i < num_bombs; i++)
		{
		
		/* Dereference this bomb */
		this_bomb = *bombs[i];
		
		/* Only pay attention to bombs in the view */
		if ((((this_bomb->h >> 16) + (BOMB_SIZE >> 1)) < vis_world_rect.right) &&
			(((this_bomb->h >> 16) - (BOMB_SIZE >> 1)) > vis_world_rect.left) &&
			(((this_bomb->v >> 16) + (BOMB_SIZE >> 1)) < vis_world_rect.bottom) &&
			(((this_bomb->v >> 16) - (BOMB_SIZE >> 1)) > vis_world_rect.top))
			
			{
					
			/* Get base address of bomb */
			bomb_base = bomb_pict;
			mask_base = bomb_mask;
			
			/* Find what to write to the bitmap */
			if (this_bomb->phased)
				{
				phase_mask1 = this_bomb->phase_mask;
				phase_mask2 = phase_mask1 ^ 0xFFFFFFFF;
				this_bomb->phase_mask ^= 0xFFFFFFFF;
				}
				
			/* Find address to write to */
			bomb_dest_base = offscreen_bitmap +
						(((this_bomb->v) >> 16) - vis_world_rect.top - (BOMB_SIZE >> 1)) * VIEW_WIDTH +
							(((this_bomb->h) >> 16) - vis_world_rect.left - (BOMB_SIZE >> 1));
			
			asm{
				move.l	mask_base, a0				; put mask_base in a register
				}

			/* Write the bomb to the bitmap */
			for (j=0; j<BOMB_SIZE; j++)
				{
				
				if (this_bomb->phased)
					{
					
					/* Mask this line of the bomb phased into memory */
					asm	{
					
						; Repeat four times, once for each longword
						
						move.l	#3, d2
						
					@lw_loop:
					
						move.l	(a0)+, d0
						or.l	phase_mask1, d0
						and.l	(bomb_dest_base), d0
						move.l	(bomb_base)+, d1
						and.l	phase_mask2, d1
						or.l	d1, d0
						move.l	d0, (bomb_dest_base)+
			
						dbra	d2, @lw_loop
	
						eor.l	#0xFFFFFFFF, phase_mask1
						eor.l	#0xFFFFFFFF, phase_mask2
						
						}
					}
				
				else
					{
					
					/* Mask this line of the bomb unphased into memory */
					asm	{
					
						; Repeat four times, once for each longword
						
						move.l	#3, d2
						
					@Plw_loop:
					
						move.l	(bomb_dest_base), d0
						and.l	(a0)+, d0
						or.l	(bomb_base)+, d0
						move.l	d0, (bomb_dest_base)+
	
						dbra	d2, @Plw_loop
	
						}
					}
				
				/* Go to next line on screen */
				bomb_dest_base += VIEW_WIDTH - BOMB_SIZE;
			
				}
			
			}
		
		}	/* loop through bombs */
		

}	/* draw_bombs() */



/*****************************************************************************\
* procedure draw_goodies                                                      *
*                                                                             *
* Purpose: This procedure draws any visible goodies.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 26, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void draw_goodies(void)
{

	register char	*goodie_base;		/* Base address of goodie bitmap */
	register char	*goodie_dest_base;	/* Base address of goodie location in view bitmap */
	register char	*goodie_mask_base;	/* Base address of goodie mask location in view bitmap */
	register goodie_pointer	this_goodie;/* goodie we're currently look at */
	Rect			goodie_rect;		/* Rectangle containing goodie */
	Rect			clipped_goodie_rect;/* Rectangle containing visible portion of goodie */
	long			skip_lines;			/* number of lines invisible above view */
	long			skip_bytes;			/* number of bytes invisible to left of view */
	long			goodie_width;		/* Width of visible portion of goodie */
	long			goodie_height;		/* Height of visible portion of goodie */
	long			goodie_longwords;	/* number of longwords in one line of goodie */
	Boolean			extra_byte_flag;	/* TRUE if there is an extra byte in a line of the goodie */
	Boolean			extra_word_flag;	/* TRUE if there is an extra word in a line of the goodie */
	long			line, byte, i;
	long			goodie_bitmap_offset;
	char			temp;

	/* Loop through the goodies, and draw each one */
	for (i = 0; i < num_goodies; i++)
		{
		
		/* Dereference this goodie */
		this_goodie = *goodies[i];
				
		/* Find the goodie's rectangle */
		goodie_rect.left = (this_goodie->h >> 16) - (GOODIE_SIZE >> 1);
		goodie_rect.top = (this_goodie->v >> 16) - (GOODIE_SIZE >> 1);
		goodie_rect.bottom = goodie_rect.top + GOODIE_SIZE;
		goodie_rect.right = goodie_rect.left + GOODIE_SIZE;
		
		/* Find the intersection between the goodie's rect and the visible region.
			If there is no overlap at all, ignore this goodie. */
		if (!SectRect (&vis_world_rect, &goodie_rect, &clipped_goodie_rect))
			continue;
			
		/* There is overlap-- find goodie bitmap base address. */
		goodie_base = goodies_picts[this_goodie->type];
		goodie_mask_base = goodies_masks[this_goodie->type];

		/* Adjust the goodie bitmap base address so we start at the right place */
		skip_lines = clipped_goodie_rect.top - goodie_rect.top;
		skip_bytes = clipped_goodie_rect.left - goodie_rect.left;
		goodie_bitmap_offset = skip_lines * GOODIE_SIZE + skip_bytes;
		goodie_base += goodie_bitmap_offset;
		goodie_mask_base += goodie_bitmap_offset;

		/* Convert the clipped goodie rect to bitmap coordinates */
		OffsetRect (&clipped_goodie_rect, -vis_world_rect.left, -vis_world_rect.top);

		/* Find the width and height of the portion of the goodie to draw */
		goodie_width = clipped_goodie_rect.right - clipped_goodie_rect.left;
		goodie_height = clipped_goodie_rect.bottom - clipped_goodie_rect.top;
		
		/* Find the number of longwords we will be drawing, and find whether there's a
			spare word or byte */
		goodie_longwords = goodie_width >> 2;
		extra_byte_flag = goodie_width & 0x1;
		extra_word_flag = (goodie_width >> 1) & 0x1;
		
		/* Find the where on the bitmap this should be drawn */
		goodie_dest_base = offscreen_bitmap + clipped_goodie_rect.top * VIEW_WIDTH +
							clipped_goodie_rect.left;
		
		/* Draw the goodie */
		asm {

			move.l	goodie_height, d1				; count number of lines lines
			subq.l	#1, d1

		@line_loop:
		
			btst.b	#0, extra_byte_flag				; If there is an extra byte,
			beq		@no_extra_byte					; draw it
			
			move.b	(goodie_dest_base), d2			; mask this byte into the bitmap
			and.b	(goodie_mask_base)+, d2
			or.b	(goodie_base)+, d2
			move.b	d2, (goodie_dest_base)+
			
		@no_extra_byte:
		
			btst.b	#0, extra_word_flag				; if there is an extra word,
			beq		@no_extra_word					; draw it

			move.w	(goodie_dest_base), d2			; mask this word into the bitmap
			and.w	(goodie_mask_base)+, d2
			or.w	(goodie_base)+, d2
			move.w	d2, (goodie_dest_base)+
			
		@no_extra_word:				
			
			move.l	goodie_longwords, d0			; set counter to draw long words
			subq.l	#1, d0
			
			bmi		@no_longwords					; check for no longwords
			
		@longword_loop:								; draw the long words

			move.l	(goodie_dest_base), d2			; mask this longword into the bitmap
			and.l	(goodie_mask_base)+, d2
			or.l	(goodie_base)+, d2
			move.l	d2, (goodie_dest_base)+
			
			dbra	d0, @longword_loop
			
			addi.l	#VIEW_WIDTH, goodie_dest_base	; point to next view bitmap line
			sub.l	goodie_width, goodie_dest_base
			
			addi.l	#GOODIE_SIZE, goodie_base		; point to next source bitmap line
			sub.l	goodie_width, goodie_base
			
			addi.l	#GOODIE_SIZE, goodie_mask_base	; point to next mask line
			sub.l	goodie_width, goodie_mask_base
			
			dbra	d1, @line_loop

		@no_longwords:
		
			}
		
		}
		
}	/* draw_goodies() */



/*****************************************************************************\
* procedure draw_large_bombs                                                  *
*                                                                             *
* Purpose: This procedure draws any visible large bombs.                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_large_bombs(void)
{

	short			i, j;
	register large_bomb_pointer	this_large_bomb;		/* Large bomb we're currently looking at */
	register char	*large_bomb_dest_base;	/* place in bitmap to draw large_bomb */
	register char	*large_bomb_base;		/* large_bomb bitmap in memory */
	char	*mask_base;						/* large_bomb mask in memory */
	short			phase_mask1, phase_mask2;

	/* Loop through the large_bombs, and draw each one */
	for (i = 0; i < num_large_bombs; i++)
		{
		
		/* Dereference this large_bomb */
		this_large_bomb = *large_bombs[i];
		
		/* Only pay attention to large_bombs in the view */
		if ((((this_large_bomb->h >> 16) + (LARGE_BOMB_SIZE >> 1)) < vis_world_rect.right) &&
			(((this_large_bomb->h >> 16) - (LARGE_BOMB_SIZE >> 1)) > vis_world_rect.left) &&
			(((this_large_bomb->v >> 16) + (LARGE_BOMB_SIZE >> 1)) < vis_world_rect.bottom) &&
			(((this_large_bomb->v >> 16) - (LARGE_BOMB_SIZE >> 1)) > vis_world_rect.top))
			
			{
					
			/* Get base address of large_bomb */
			large_bomb_base = large_bomb_pict;
			mask_base = large_bomb_mask;
			
			/* Find what to write to the bitmap */
			if (this_large_bomb->phased)
				{
				phase_mask1 = this_large_bomb->phase_mask;
				phase_mask2 = phase_mask1 ^ 0xFFFFFFFF;
				this_large_bomb->phase_mask ^= 0xFFFFFFFF;
				}
				
			/* Find address to write to */
			large_bomb_dest_base = offscreen_bitmap +
						(((this_large_bomb->v) >> 16) - vis_world_rect.top - (LARGE_BOMB_SIZE >> 1)) * VIEW_WIDTH +
							(((this_large_bomb->h) >> 16) - vis_world_rect.left - (LARGE_BOMB_SIZE >> 1));
			
			asm{
				move.l	mask_base, a0				; put mask_base in a register
				}

			/* Write the large_bomb to the bitmap */
			for (j=0; j<LARGE_BOMB_SIZE; j++)
				{
				
				if (this_large_bomb->phased)
					{
					
					/* Mask this line of the large_bomb phased into memory */
					asm	{
					
						; Repeat four times, once for each longword
						
						move.l	#3, d2
						
					@lw_loop:
					
						move.l	(a0)+, d0
						or.l	phase_mask1, d0
						and.l	(large_bomb_dest_base), d0
						move.l	(large_bomb_base)+, d1
						and.l	phase_mask2, d1
						or.l	d1, d0
						move.l	d0, (large_bomb_dest_base)+
			
						dbra	d2, @lw_loop
	
						eor.l	#0xFFFFFFFF, phase_mask1
						eor.l	#0xFFFFFFFF, phase_mask2
						
						}
					}
				
				else
					{
					
					/* Mask this line of the large_bomb unphased into memory */
					asm	{
					
						; Repeat four times, once for each longword
						
						move.l	#3, d2
						
					@Plw_loop:
					
						move.l	(large_bomb_dest_base), d0
						and.l	(a0)+, d0
						or.l	(large_bomb_base)+, d0
						move.l	d0, (large_bomb_dest_base)+
	
						dbra	d2, @Plw_loop
	
						}
					}
				
				/* Go to next line on screen */
				large_bomb_dest_base += VIEW_WIDTH - LARGE_BOMB_SIZE;
			
				}
			
			}
		
		}	/* loop through large large_bombs */
		

}	/* draw_large_bombs() */



/*****************************************************************************\
* procedure draw_shells                                                       *
*                                                                             *
* Purpose: This procedure draws any visible shells.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_shells(void)
{

	short			i, j;
	register shell_pointer	this_shell;	/* Shell we're currently looking at */
	register char	*shell_dest_base;	/* place in bitmap to draw shell */
	register char	*shell_base;		/* shell bitmap in memory */
	char			*mask_base;			/* shell mask in memory */
	short			phase_mask1, phase_mask2;

	/* Loop through the shells, and draw each one */
	for (i = 0; i < num_shells; i++)
		{
		
		/* Dereference this shell */
		this_shell = *shells[i];
		
		/* Only pay attention to shells in the view */
		if ((((this_shell->h >> 16) + (SHELL_SIZE >> 1)) < vis_world_rect.right) &&
			(((this_shell->h >> 16) - (SHELL_SIZE >> 1)) > vis_world_rect.left) &&
			(((this_shell->v >> 16) + (SHELL_SIZE >> 1)) < vis_world_rect.bottom) &&
			(((this_shell->v >> 16) - (SHELL_SIZE >> 1)) > vis_world_rect.top))
			
			{
					
			/* Get base address of shell */
			shell_base = shell_pict;
			mask_base = shell_mask;
			
			/* Find what to write to the bitmap */
			if (this_shell->phased)
				{
				phase_mask1 = this_shell->phase_mask;
				phase_mask2 = phase_mask1 ^ 0xFFFFFFFF;
				this_shell->phase_mask ^= 0xFFFFFFFF;
				}
				
			/* Find address to write to */
			shell_dest_base = offscreen_bitmap +
						(((this_shell->v) >> 16) - vis_world_rect.top - (SHELL_SIZE >> 1)) * VIEW_WIDTH +
							(((this_shell->h) >> 16) - vis_world_rect.left - (SHELL_SIZE >> 1));
			
			asm{
				move.l	mask_base, a0				; put mask_base in a register
				}

			/* Write the shell to the bitmap */
			for (j=0; j<SHELL_SIZE; j++)
				{
				
				if (this_shell->phased)
					{
					
					/* Mask this line of the bomb phased into memory */
					asm	{
						
						move.l	#1, d2
						
					@lw_loop:
					
						move.l	(a0)+, d0
						or.l	phase_mask1, d0
						and.l	(shell_dest_base), d0
						move.l	(shell_base)+, d1
						and.l	phase_mask2, d1
						or.l	d1, d0
						move.l	d0, (shell_dest_base)+
			
						dbra	d2, @lw_loop
	
						eor.l	#0xFFFFFFFF, phase_mask1
						eor.l	#0xFFFFFFFF, phase_mask2
						
						}
					}
				
				else
					{
					
					/* Mask this line of the shell unphased into memory */
					asm	{
					
						move.l	#1, d2
						
					@Plw_loop:
					
						move.l	(shell_dest_base), d0
						and.l	(a0)+, d0
						or.l	(shell_base)+, d0
						move.l	d0, (shell_dest_base)+
	
						dbra	d2, @Plw_loop
	
						}
					}
				
				/* Go to next line on screen */
				shell_dest_base += VIEW_WIDTH - SHELL_SIZE;
				
				}	/* loop through shell lines */
			
			}
	
		}	/* loop through shells */
		

}	/* draw_shells() */



/*****************************************************************************\
* procedure draw_tracers                                                      *
*                                                                             *
* Purpose: This procedure draws any visible tracers.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_tracers(void)
{

	short			i, j;
	register tracer_pointer	this_tracer;	/* Tracer we're currently looking at */
	register char	*tracer_dest_base;		/* place in bitmap to draw tracer */
	register char	*tracer_base;			/* tracer bitmap in memory */
	char			*mask_base;				/* tracer mask in memory */
	short			phase_mask1, phase_mask2;

	/* Loop through the tracer, and draw each one */
	for (i = 0; i < num_tracers; i++)
		{
		
		/* Dereference this tracer */
		this_tracer = *tracers[i];
		
		/* Only pay attention to shells in the view */
		if ((((this_tracer->h >> 16) + (TRACER_SIZE >> 1)) < vis_world_rect.right) &&
			(((this_tracer->h >> 16) - (TRACER_SIZE >> 1)) > vis_world_rect.left) &&
			(((this_tracer->v >> 16) + (TRACER_SIZE >> 1)) < vis_world_rect.bottom) &&
			(((this_tracer->v >> 16) - (TRACER_SIZE >> 1)) > vis_world_rect.top))
			
			{
					
			/* Get base address of tracer */
			tracer_base = tracers_picts[this_tracer->frame];
			mask_base = tracers_masks[this_tracer->frame];
			
			/* Check for phased tracer */
			if (this_tracer->phased)
				{
				phase_mask1 = this_tracer->phase_mask;
				phase_mask2 = phase_mask1 ^ 0xFFFFFFFF;
				this_tracer->phase_mask ^= 0xFFFFFFFF;
				}
				
			/* Find address to write to */
			tracer_dest_base = offscreen_bitmap +
						(((this_tracer->v) >> 16) - vis_world_rect.top - (TRACER_SIZE >> 1)) * VIEW_WIDTH +
							(((this_tracer->h) >> 16) - vis_world_rect.left - (TRACER_SIZE >> 1));
			
			asm{
				move.l	mask_base, a0				; put mask_base in a register
				}

			/* Write the tracer to the bitmap */
			for (j=0; j<TRACER_SIZE; j++)
				{
				
				if (this_tracer->phased)
					{
					
					/* Mask this line of the tracer phased into memory */
					asm	{
						
						move.l	#1, d2
						
					@lw_loop:
					
						move.l	(a0)+, d0
						or.l	phase_mask1, d0
						and.l	(tracer_dest_base), d0
						move.l	(tracer_base)+, d1
						and.l	phase_mask2, d1
						or.l	d1, d0
						move.l	d0, (tracer_dest_base)+
			
						dbra	d2, @lw_loop
	
						eor.l	#0xFFFFFFFF, phase_mask1
						eor.l	#0xFFFFFFFF, phase_mask2
						
						}
					}
				
				else
					{
					
					/* Mask this line of the tracer unphased into memory */
					asm	{
					
						move.l	#1, d2
						
					@Plw_loop:
					
						move.l	(tracer_dest_base), d0
						and.l	(a0)+, d0
						or.l	(tracer_base)+, d0
						move.l	d0, (tracer_dest_base)+
	
						dbra	d2, @Plw_loop
	
						}
					}
				
				/* Go to next line on screen */
				tracer_dest_base += VIEW_WIDTH - TRACER_SIZE;
				
				}	/* loop through tracer lines */
			
			}
	
		}	/* loop through tracers */
		

}	/* draw_tracers() */



/*****************************************************************************\
* procedure draw_missiles                                                     *
*                                                                             *
* Purpose: This procedure draws any visible missiles.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_missiles(void)
{

	short			i, j;
	register missile_pointer	this_missile;	/* Missile we're currently looking at */
	register char	*missile_dest_base;	/* place in bitmap to draw missile */
	register char	*missile_base;		/* missile bitmap in memory */
	char			*mask_base;			/* missile mask in memory */
	short			phase_mask1, phase_mask2;

	/* Loop through the missiles, and draw each one */
	for (i = 0; i < num_missiles; i++)
		{
		
		/* Dereference this missile */
		this_missile = *missiles[i];
		
		/* Only pay attention to missiles in the view */
		if ((((this_missile->h >> 16) + (MISSILE_SIZE >> 1)) < vis_world_rect.right) &&
			(((this_missile->h >> 16) - (MISSILE_SIZE >> 1)) > vis_world_rect.left) &&
			(((this_missile->v >> 16) + (MISSILE_SIZE >> 1)) < vis_world_rect.bottom) &&
			(((this_missile->v >> 16) - (MISSILE_SIZE >> 1)) > vis_world_rect.top))
			

			{
					
			/* Get base address of missile */
			missile_base = missiles_picts[this_missile->direction >> 3];
			mask_base = missiles_masks[this_missile->direction >> 3];
			
			/* Find the phase mask, if any */
			if (this_missile->phased)
				{
				phase_mask1 = this_missile->phase_mask;
				phase_mask2 = phase_mask1 ^ 0xFFFFFFFF;
				this_missile->phase_mask ^= 0xFFFFFFFF;
				}
				
			/* Find address to write to */
			missile_dest_base =
				offscreen_bitmap +
				(((this_missile->v) >> 16) - vis_world_rect.top - (MISSILE_SIZE >> 1)) * VIEW_WIDTH +
					(((this_missile->h) >> 16) - vis_world_rect.left - (MISSILE_SIZE >> 1));
			
			asm{
				move.l	mask_base, a0				; put mask_base in a register
				}

			/* Write the missile to the bitmap */
			for (j=0; j<MISSILE_SIZE; j++)
				{
				
				if (this_missile->phased)
					{
					
					/* Mask this line of the missile phased into memory */
					asm	{
						
						move.l	#3, d2
						
					@lw_loop:
					
						move.l	(a0)+, d0
						or.l	phase_mask1, d0
						and.l	(missile_dest_base), d0
						move.l	(missile_base)+, d1
						and.l	phase_mask2, d1
						or.l	d1, d0
						move.l	d0, (missile_dest_base)+
			
						dbra	d2, @lw_loop
	
						eor.l	#0xFFFFFFFF, phase_mask1
						eor.l	#0xFFFFFFFF, phase_mask2
						
						}
					}
				
				else
					{
					
					/* Mask this line of the missile unphased into memory */
					asm	{
					
						move.l	#3, d2
						
					@Plw_loop:
					
						move.l	(missile_dest_base), d0
						and.l	(a0)+, d0
						or.l	(missile_base)+, d0
						move.l	d0, (missile_dest_base)+
	
						dbra	d2, @Plw_loop
	
						}
					}
				
				/* Go to next line on screen */
				missile_dest_base += VIEW_WIDTH - MISSILE_SIZE;
				}
			
			}
		
		}	/* loop through missiles */

}	/* draw_missiles() */



/*****************************************************************************\
* procedure draw_enemies                                                      *
*                                                                             *
* Purpose: This procedure draws any visible enemies.                          *
*                                                                             *
* Parameters: level: which level of enemies to draw.  Level may be air,       *
*                    ground, or ground_fireball.  ground_fireball draws only  *
*                    fireballs at the ground level.  ground draws all other   *
*                    ground level enemies.                                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_enemies(short level)
{

	register char	*enemy_base;		/* Base address of enemy bitmap */
	register char	*enemy_dest_base;	/* Base address of enemy location in view bitmap */
	register char	*enemy_mask_base;	/* Base address of enemy mask location in view bitmap */
	register enemy_pointer	this_enemy;	/* Enemy we're currently look at */
	Rect			enemy_rect;			/* Rectangle containing enemy */
	Rect			clipped_enemy_rect;	/* Rectangle containing visible portion of enemy */
	long			skip_lines;			/* number of lines invisible above view */
	long			skip_bytes;			/* number of bytes invisible to left of view */
	long			enemy_width;		/* Width of visible portion of enemy */
	long			enemy_height;		/* Height of visible portion of enemy */
	long			enemy_longwords;	/* number of longwords in one line of enemy */
	Boolean			extra_byte_flag;	/* TRUE if there is an extra byte in a line of the enemy */
	Boolean			extra_word_flag;	/* TRUE if there is an extra word in a line of the enemy */
	Boolean			explosion_dead;		/* TRUE if enemy is mostly exploded */
	long			line, byte, i;
	long			enemy_bitmap_offset;
	char			temp;
	short			frame;
	long			phase_lw_1, phase_lw_2;
	short			phase_word_1, phase_word_2;
	char			phase_byte_1, phase_byte_2;

	static char	silo_frames[SILO_FRAMES+1] = {
				0,	1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3,
					4, 4, 4, 5, 5, 5, 4, 4, 4, 3, 3, 3, 5, 5, 5, 6, 6, 6,
					7, 7, 7, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 3, 3, 3,
					4, 4, 4, 5, 5, 5, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 6, 6,
					7, 7, 7, 6, 6, 6, 3, 3, 3, 3, 3, 3, 8, 8, 8, 8, 8, 8,
					2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1};


/* We use the fireball_* names when dealing with fireballs, but they are really the
	same register variables */
#define	fireball_base		enemy_base
#define	fireball_dest_base	enemy_dest_base
#define	fireball_mask_base	enemy_mask_base

	/* Loop through the enemies, and draw each one */
	for (i = 0; i < num_enemies; i++)
		{
		
		/* Dereference this enemy */
		this_enemy = *enemies[i];
				
		/* If this enemy is on the wrong level, ignore it */
		switch (level)
			{
			case ground_fireball:
				if ((this_enemy->type != fireball) || (this_enemy->level == air)) continue;
				break;
			
			case ground:
				if ((this_enemy->type == fireball) || (this_enemy->level == air)) continue;
				break;
				
			case air:
				if (this_enemy->level == ground) continue;
				break;
			}
		
		/* Find if this enemy is more than half way through an explosion */
		explosion_dead = ((this_enemy->exploding) && (this_enemy->exploding < 24));
		
		/* Find the enemy's rectangle */
		enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
		enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
		enemy_rect.bottom = enemy_rect.top + SHIP_SIZE;
		enemy_rect.right = enemy_rect.left + SHIP_SIZE;
		
		/* Find the intersection between the enemy's rect and the visible region.
			If there is no overlap at all, ignore this enemy. */
		if (!SectRect (&vis_world_rect, &enemy_rect, &clipped_enemy_rect))
			continue;
			
		/* There is overlap-- find enemy bitmap base address */
		switch (this_enemy->type)
			{
			case ship:
				enemy_base = ships[(this_enemy->direction >> 16)];
				enemy_mask_base = ship_masks[(this_enemy->direction >> 16)];
				break;

			case biplane:
				enemy_base = biplanes[this_enemy->direction >> 19];
				enemy_mask_base = biplanes_masks[this_enemy->direction >> 19];
				break;

			case ground_gun:
			case smart_ground_gun:
				if ((this_enemy->dead) || (explosion_dead))
					this_enemy->turret = NUM_DIRECTIONS;
				enemy_base = guns[this_enemy->turret >> 3];
				enemy_mask_base = guns_masks[this_enemy->turret >> 3];
				break;
				
			case cannon:
				if ((this_enemy->dead) || (explosion_dead))
					this_enemy->turret = NUM_DIRECTIONS;
				enemy_base = cannons[this_enemy->turret >> 3];
				enemy_mask_base = cannons_masks[this_enemy->turret >> 3];
				break;

			case silo:
				if ((this_enemy->dead) || (explosion_dead))
					{
					enemy_base = silos[NUM_SILOS - 1];
					enemy_mask_base = silos_masks[NUM_SILOS - 1];
					}
					
				else
					{
					enemy_base = silos[silo_frames[this_enemy->silo_frame]];
					enemy_mask_base = silos_masks[silo_frames[this_enemy->silo_frame]];
					}
					
				break;

			}

		/* Adjust the enemy bitmap base address so we start at the right place */
		skip_lines = clipped_enemy_rect.top - enemy_rect.top;
		skip_bytes = clipped_enemy_rect.left - enemy_rect.left;
		enemy_bitmap_offset = skip_lines * SHIP_SIZE + skip_bytes;
		enemy_base += enemy_bitmap_offset;
		enemy_mask_base += enemy_bitmap_offset;

		/* Convert the clipped enemy rect to bitmap coordinates */
		OffsetRect (&clipped_enemy_rect, -vis_world_rect.left, -vis_world_rect.top);

		/* Find the width and height of the portion of the enemy to draw */
		enemy_width = clipped_enemy_rect.right - clipped_enemy_rect.left;
		enemy_height = clipped_enemy_rect.bottom - clipped_enemy_rect.top;
		
		/* Find the number of longwords we will be drawing, and find whether there's a
			spare word or byte */
		enemy_longwords = enemy_width >> 2;
		extra_byte_flag = enemy_width & 0x1;
		extra_word_flag = (enemy_width >> 1) & 0x1;
		
		/* Find the where on the bitmap this should be drawn */
		enemy_dest_base = offscreen_bitmap + clipped_enemy_rect.top * VIEW_WIDTH +
							clipped_enemy_rect.left;
		
		/* Don't draw the ship if it is exploding and past the half-way point */
		if (((!explosion_dead) || (this_enemy->level == ground)) && (this_enemy->type != fireball))
		
			{
			
			/* Draw this enemy normally if it is not phased */
			if (!this_enemy->phased)
				
				{
				/* Draw the ship */
				asm {
		
					move.l	enemy_height, d1					; count number of lines lines
					subq.l	#1, d1
		
				@line_loop:
				
					btst.b	#0, extra_byte_flag				; If there is an extra byte,
					beq		@no_extra_byte					; draw it
					
					move.b	(enemy_dest_base), d2			; mask this byte into the bitmap
					and.b	(enemy_mask_base)+, d2
					or.b	(enemy_base)+, d2
					move.b	d2, (enemy_dest_base)+
					
				@no_extra_byte:
				
					btst.b	#0, extra_word_flag				; if there is an extra word,
					beq		@no_extra_word					; draw it
		
					move.w	(enemy_dest_base), d2			; mask this word into the bitmap
					and.w	(enemy_mask_base)+, d2
					or.w	(enemy_base)+, d2
					move.w	d2, (enemy_dest_base)+
					
				@no_extra_word:				
					
					move.l	enemy_longwords, d0				; set counter to draw long words
					subq.l	#1, d0
					
					bmi		@no_longwords					; check for no longwords
					
				@longword_loop:								; draw the long words
		
					move.l	(enemy_dest_base), d2			; mask this longword into the bitmap
					and.l	(enemy_mask_base)+, d2
					or.l	(enemy_base)+, d2
					move.l	d2, (enemy_dest_base)+
					
					dbra	d0, @longword_loop
					
					addi.l	#VIEW_WIDTH, enemy_dest_base	; point to next view bitmap line
					sub.l	enemy_width, enemy_dest_base
					
					addi.l	#SHIP_SIZE, enemy_base			; point to next source bitmap line
					sub.l	enemy_width, enemy_base
					
					addi.l	#SHIP_SIZE, enemy_mask_base	; point to next mask line
					sub.l	enemy_width, enemy_mask_base
					
					dbra	d1, @line_loop
		
				@no_longwords:
				
					}
				}


			/* Enemy is phased-- draw it phased */
			else
				
				{

				/* Find the phase masks */
				phase_lw_2 = this_enemy->phase_mask_lw;
				phase_lw_1 = phase_lw_2 ^ 0xFFFFFFFF;
				phase_word_2 = this_enemy->phase_mask_word;
				phase_word_1 = phase_word_2 ^ 0xFFFF;
				phase_byte_2 = this_enemy->phase_mask_byte;
				phase_byte_1 = phase_byte_2 ^ 0xFF;

				asm {
		
					move.l	d4, a1							; save d4
					move.l	d3, a0							; save d3
		
					move.l	enemy_height, d1				; count number of lines lines
					subq.l	#1, d1
		
				@Pline_loop:
				
					btst.b	#0, extra_byte_flag				; If there is an extra byte,
					beq		@Pno_extra_byte					; draw it
					
					move.b	(enemy_mask_base)+, d3			; mask this byte into the bitmap
					or.b	phase_byte_1, d3
					and.b	(enemy_dest_base), d3
					move.b	(enemy_base)+, d4
					and.b	phase_byte_2, d4
					or.b	d4, d3
					move.b	d3, (enemy_dest_base)+
					
				@Pno_extra_byte:
				
					btst.b	#0, extra_word_flag				; if there is an extra word,
					beq		@Pno_extra_word					; draw it
		
					move.w	(enemy_mask_base)+, d3			; mask this word into the bitmap
					or.w	phase_word_1, d3
					and.w	(enemy_dest_base), d3
					move.w	(enemy_base)+, d4
					and.w	phase_word_2, d4
					or.w	d4, d3
					move.w	d3, (enemy_dest_base)+
					
				@Pno_extra_word:				
					
					move.l	enemy_longwords, d0				; set counter to draw long words
					subq.l	#1, d0
					
					bmi		@Pno_longwords					; check for no longwords
					
				@Plongword_loop:								; draw the long words
		
					move.l	(enemy_mask_base)+, d3			; mask this longword into the bitmap
					or.l	phase_lw_1, d3
					and.l	(enemy_dest_base), d3
					move.l	(enemy_base)+, d4
					and.l	phase_lw_2, d4
					or.l	d4, d3
					move.l	d3, (enemy_dest_base)+
					
					dbra	d0, @Plongword_loop
					
					addi.l	#VIEW_WIDTH, enemy_dest_base	; point to next view bitmap line
					sub.l	enemy_width, enemy_dest_base
					
					addi.l	#SHIP_SIZE, enemy_base			; point to next source bitmap line
					sub.l	enemy_width, enemy_base
					
					addi.l	#SHIP_SIZE, enemy_mask_base	; point to next mask line
					sub.l	enemy_width, enemy_mask_base
					
					eor.l	#0xFFFFFFFF, phase_lw_1
					eor.l	#0xFFFFFFFF, phase_lw_2

					eor.w	#0xFFFF, phase_word_1
					eor.w	#0xFFFF, phase_word_2

					eor.b	#0xFF, phase_byte_1
					eor.b	#0xFF, phase_byte_2

					dbra	d1, @Pline_loop
		
				@Pno_longwords:
				
					move.l	a1, d4							; restore d4
					move.l	a0, d3							; restore d3

					}
				}
			}


		/* If the enemy is exploding, draw the explosion frame */
		if (this_enemy->exploding)
			{
			
			/* Find the frame */
			frame = this_enemy->exploding;
			if (frame > 24)
				frame = FIREBALL_FRAMES - frame;
			
			/* Find the explosion base addresses */
			fireball_base = fireballs[((frame + 1) >> 1) - 1];
			fireball_mask_base = fireballs_masks[((frame + 1) >> 1) - 1];

			/* Adjust the fireball base address so we start at the right place */
			fireball_base += enemy_bitmap_offset;
			fireball_mask_base += enemy_bitmap_offset;

			/* Find the where on the bitmap this fireball should be drawn */
			fireball_dest_base = offscreen_bitmap + clipped_enemy_rect.top * VIEW_WIDTH +
									clipped_enemy_rect.left;
			
			/* Draw this fireball normally if it is not phased */
			if (!this_enemy->phased)
				{
				
				asm {
		
					move.l	enemy_height, d1					; count number of lines lines
					subq.l	#1, d1
		
				@fline_loop:
				
					btst.b	#0, extra_byte_flag				; If there is an extra byte,
					beq		@fno_extra_byte					; draw it
					
					move.b	(fireball_dest_base), d2		; mask this byte into the bitmap
					and.b	(fireball_mask_base)+, d2
					or.b	(fireball_base)+, d2
					move.b	d2, (fireball_dest_base)+
					
				@fno_extra_byte:
				
					btst.b	#0, extra_word_flag				; if there is an extra word,
					beq		@fno_extra_word					; draw it
		
					move.w	(fireball_dest_base), d2		; mask this word into the bitmap
					and.w	(fireball_mask_base)+, d2
					or.w	(fireball_base)+, d2
					move.w	d2, (fireball_dest_base)+
					
				@fno_extra_word:				
					
					move.l	enemy_longwords, d0				; set counter to draw long words
					subq.l	#1, d0
					
					bmi		@fno_longwords					; check for no longwords
					
				@flongword_loop:							; draw the long words
		
					move.l	(fireball_dest_base), d2		; mask this longword into the bitmap
					and.l	(fireball_mask_base)+, d2
					or.l	(fireball_base)+, d2
					move.l	d2, (fireball_dest_base)+
					
					dbra	d0, @flongword_loop
					
					addi.l	#VIEW_WIDTH, fireball_dest_base	; point to next view bitmap line
					sub.l	enemy_width, fireball_dest_base
					
					addi.l	#SHIP_SIZE, fireball_base		; point to next source bitmap line
					sub.l	enemy_width, fireball_base
					
					addi.l	#SHIP_SIZE, fireball_mask_base	; point to next mask line
					sub.l	enemy_width, fireball_mask_base
					
					dbra	d1, @fline_loop
		
				@fno_longwords:
				
					}
				}
				
			/* Fireball is phased -- draw it phased */
			else
				{

				/* Find the phase masks */
				phase_lw_2 = this_enemy->phase_mask_lw;
				phase_lw_1 = phase_lw_2 ^ 0xFFFFFFFF;
				phase_word_2 = this_enemy->phase_mask_word;
				phase_word_1 = phase_word_2 ^ 0xFFFF;
				phase_byte_2 = this_enemy->phase_mask_byte;
				phase_byte_1 = phase_byte_2 ^ 0xFF;

				asm {
		
					move.l	d4, a1							; save d4
					move.l	d3, a0							; save d3
		
					move.l	enemy_height, d1				; count number of lines lines
					subq.l	#1, d1
		
				@Pfline_loop:
				
					btst.b	#0, extra_byte_flag				; If there is an extra byte,
					beq		@Pfno_extra_byte				; draw it
					
					move.b	(fireball_mask_base)+, d3		; mask this byte into the bitmap
					or.b	phase_byte_1, d3
					and.b	(fireball_dest_base), d3
					move.b	(fireball_base)+, d4
					and.b	phase_byte_2, d4
					or.b	d4, d3
					move.b	d3, (fireball_dest_base)+
					
				@Pfno_extra_byte:
				
					btst.b	#0, extra_word_flag				; if there is an extra word,
					beq		@Pfno_extra_word				; draw it
		
					move.w	(fireball_mask_base)+, d3		; mask this word into the bitmap
					or.w	phase_byte_1, d3
					and.w	(fireball_dest_base), d3
					move.w	(fireball_base)+, d4
					and.w	phase_byte_2, d4
					or.w	d4, d3
					move.w	d3, (fireball_dest_base)+
					
				@Pfno_extra_word:				
					
					move.l	enemy_longwords, d0				; set counter to draw long words
					subq.l	#1, d0
					
					bmi		@Pfno_longwords					; check for no longwords
					
				@Pflongword_loop:								; draw the long words
		
					move.l	(fireball_mask_base)+, d3		; mask this longword into the bitmap
					or.l	phase_byte_1, d3
					and.l	(fireball_dest_base), d3
					move.l	(fireball_base)+, d4
					and.l	phase_byte_2, d4
					or.l	d4, d3
					move.l	d3, (fireball_dest_base)+
					
					dbra	d0, @Pflongword_loop
					
					addi.l	#VIEW_WIDTH, fireball_dest_base	; point to next view bitmap line
					sub.l	enemy_width, fireball_dest_base
					
					addi.l	#SHIP_SIZE, fireball_base		; point to next source bitmap line
					sub.l	enemy_width, fireball_base
					
					addi.l	#SHIP_SIZE, fireball_mask_base	; point to next mask line
					sub.l	enemy_width, fireball_mask_base
					
					eor.l	#0xFFFFFFFF, phase_lw_1
					eor.l	#0xFFFFFFFF, phase_lw_2

					eor.w	#0xFFFF, phase_word_1
					eor.w	#0xFFFF, phase_word_2

					eor.b	#0xFF, phase_byte_1
					eor.b	#0xFF, phase_byte_2

					dbra	d1, @Pfline_loop
		
				@Pfno_longwords:
				
					move.l	a1, d4							; restore d4
					move.l	a0, d3							; restore d3

					}
				}
			}
		
		}
		
}	/* draw_enemies() */



/*****************************************************************************\
* procedure draw_bases                                                        *
*                                                                             *
* Purpose: This procedure draws any visible bases.                            *
*                                                                             *
* Parameters: level: ground if we should draw the bottom half of the base,    *
*                    air if we should draw the top half.                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 26, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void draw_bases(short level)
{

	register char	*base_base;			/* Base address of base bitmap */
	register char	*base_dest_base;	/* Base address of base location in view bitmap */
	register char	*base_mask_base;	/* Base address of base mask location in view bitmap */
	register base_pointer	this_base;	/* Base we're currently look at */
	Rect			base_rect;			/* Rectangle containing base */
	Rect			clipped_base_rect;	/* Rectangle containing visible portion of base */
	long			skip_lines;			/* number of lines invisible above view */
	long			skip_bytes;			/* number of bytes invisible to left of view */
	long			base_width;			/* Width of visible portion of base */
	long			base_height;		/* Height of visible portion of base */
	long			base_longwords;		/* number of longwords in one line of base */
	Boolean			extra_byte_flag;	/* TRUE if there is an extra byte in a line of the base */
	Boolean			extra_word_flag;	/* TRUE if there is an extra word in a line of the base */
	long			line, byte, i;
	long			base_bitmap_offset;
	char			temp;
	short			frame;
	long			phase_lw_1, phase_lw_2;
	short			phase_word_1, phase_word_2;
	char			phase_byte_1, phase_byte_2;

	/* Loop through the bases, and draw each one */
	for (i = 0; i < num_bases; i++)
		{
		
		/* Dereference this base */
		this_base = *bases[i];
				
		/* Find the base's rectangle */
		base_rect.left = (this_base->h >> 16) - (BASE_SIZE >> 1);
		base_rect.top = (this_base->v >> 16) - (BASE_SIZE >> 1);
		base_rect.bottom = base_rect.top + BASE_SIZE;
		base_rect.right = base_rect.left + BASE_SIZE;
		
		/* Find the intersection between the base's rect and the visible region.
			If there is no overlap at all, ignore this base. */
		if (!SectRect (&vis_world_rect, &base_rect, &clipped_base_rect))
			continue;
			
		/* There is overlap-- find base bitmap base address.  If this base is dead,
			use the dead base picture */
		if (this_base->dead)
			{
			base_base = dead_bases[this_base->type];
			base_mask_base = dead_bases_masks[this_base->type];
			}
		
		else	/* Draw the top or bottom, depending on the value of the calling
					parameter */
			{
			
			base_base = (level == air) ? base_tops[this_base->type] :
											base_bottoms[this_base->type];
			base_mask_base = (level == air) ? base_tops_masks[this_base->type] :
												base_bottoms_masks[this_base->type];
			
			}

		/* Adjust the base bitmap base address so we start at the right place */
		skip_lines = clipped_base_rect.top - base_rect.top;
		skip_bytes = clipped_base_rect.left - base_rect.left;
		base_bitmap_offset = skip_lines * BASE_SIZE + skip_bytes;
		base_base += base_bitmap_offset;
		base_mask_base += base_bitmap_offset;

		/* Convert the clipped base rect to bitmap coordinates */
		OffsetRect (&clipped_base_rect, -vis_world_rect.left, -vis_world_rect.top);

		/* Find the width and height of the portion of the base to draw */
		base_width = clipped_base_rect.right - clipped_base_rect.left;
		base_height = clipped_base_rect.bottom - clipped_base_rect.top;
		
		/* Find the number of longwords we will be drawing, and find whether there's a
			spare word or byte */
		base_longwords = base_width >> 2;
		extra_byte_flag = base_width & 0x1;
		extra_word_flag = (base_width >> 1) & 0x1;
		
		/* Find the where on the bitmap this should be drawn */
		base_dest_base = offscreen_bitmap + clipped_base_rect.top * VIEW_WIDTH +
							clipped_base_rect.left;
		
		/* Draw the base */
		asm {

			move.l	base_height, d1					; count number of lines lines
			subq.l	#1, d1

		@line_loop:
		
			btst.b	#0, extra_byte_flag				; If there is an extra byte,
			beq		@no_extra_byte					; draw it
			
			move.b	(base_dest_base), d2			; mask this byte into the bitmap
			and.b	(base_mask_base)+, d2
			or.b	(base_base)+, d2
			move.b	d2, (base_dest_base)+
			
		@no_extra_byte:
		
			btst.b	#0, extra_word_flag				; if there is an extra word,
			beq		@no_extra_word					; draw it

			move.w	(base_dest_base), d2			; mask this word into the bitmap
			and.w	(base_mask_base)+, d2
			or.w	(base_base)+, d2
			move.w	d2, (base_dest_base)+
			
		@no_extra_word:				
			
			move.l	base_longwords, d0				; set counter to draw long words
			subq.l	#1, d0
			
			bmi		@no_longwords					; check for no longwords
			
		@longword_loop:								; draw the long words

			move.l	(base_dest_base), d2			; mask this longword into the bitmap
			and.l	(base_mask_base)+, d2
			or.l	(base_base)+, d2
			move.l	d2, (base_dest_base)+
			
			dbra	d0, @longword_loop
			
			addi.l	#VIEW_WIDTH, base_dest_base		; point to next view bitmap line
			sub.l	base_width, base_dest_base
			
			addi.l	#BASE_SIZE, base_base			; point to next source bitmap line
			sub.l	base_width, base_base
			
			addi.l	#BASE_SIZE, base_mask_base	; point to next mask line
			sub.l	base_width, base_mask_base
			
			dbra	d1, @line_loop

		@no_longwords:
		
			}
		
		}
		
}	/* draw_bases() */



/*****************************************************************************\
* procedure draw_explosions                                                   *
*                                                                             *
* Purpose: This procedure draws any visible explosions.                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_explosions(void)
{

	register char	*explosion_base;		/* Base address of explosion bitmap */
	register char	*explosion_dest_base;	/* Base address of explosion location in view bitmap */
	register char	*explosion_mask_base;	/* Base address of explosion location in view bitmap */
	register explosion_pointer	this_explosion;	/* explosion we're currently look at */
	Rect			explosion_rect;			/* Rectangle containing explosion */
	Rect			clipped_explosion_rect;	/* Rectangle containing visible portion of explosion */
	long			skip_lines;			/* number of lines invisible above view */
	long			skip_bytes;			/* number of bytes invisible to left of view */
	long			explosion_width;		/* Width of visible portion of explosion */
	long			explosion_height;		/* Height of visible portion of explosion */
	long			explosion_longwords;	/* number of longwords in one line of explosion */
	Boolean			extra_byte_flag;	/* TRUE if there is an extra byte in a line of the explosion */
	Boolean			extra_word_flag;	/* TRUE if there is an extra word in a line of the explosion */
	long			line, byte, i;
	long			explosion_bitmap_offset;
	char			temp;
	short			frame;
	long			phase_lw_1, phase_lw_2;
	short			phase_word_1, phase_word_2;
	char			phase_byte_1, phase_byte_2;

	static char	frames[EXPLOSION_FRAMES] =
			{	0, 0, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 1, 1, 1,
				2, 2, 2, 3, 3, 3, 4, 4, 4, 2, 2, 2, 3, 3, 3,
				4, 4, 4, 1, 1, 1, 3, 3, 3, 5, 5, 4, 4, 4,
				1, 1, 1, 1, 3, 3, 3, 3, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7 	};


	/* Loop through the explosions, and draw each one */
	for (i = 0; i < num_explosions; i++)
		{
		
		/* Dereference this explosion */
		this_explosion = *explosions[i];
				
		/* Find the explosion's rectangle */
		explosion_rect.left = this_explosion->h >> 16;
		explosion_rect.top = this_explosion->v >> 16;
		explosion_rect.bottom = explosion_rect.top + EXPLOSION_SIZE;
		explosion_rect.right = explosion_rect.left + EXPLOSION_SIZE;
		OffsetRect (&explosion_rect, -(EXPLOSION_SIZE >> 1), -(EXPLOSION_SIZE >> 1));
		
		/* Find the intersection between the explosion's rect and the visible region.
			If there is no overlap at all, ignore this explosion. */
		if (!SectRect (&vis_world_rect, &explosion_rect, &clipped_explosion_rect))
			continue;
			
		/* There is overlap-- find explosion bitmap base address */
		explosion_base = explosions_picts[frames[this_explosion->frame]];
		explosion_mask_base = explosions_masks[frames[this_explosion->frame]];
	
		/* Adjust the explosion bitmap base address so we start at the right place */
		skip_lines = clipped_explosion_rect.top - explosion_rect.top;
		skip_bytes = clipped_explosion_rect.left - explosion_rect.left;
		explosion_bitmap_offset = skip_lines * EXPLOSION_SIZE + skip_bytes;
		explosion_base += explosion_bitmap_offset;
		explosion_mask_base += explosion_bitmap_offset;

		/* Convert the clipped explosion rect to bitmap coordinates */
		OffsetRect (&clipped_explosion_rect, -vis_world_rect.left, -vis_world_rect.top);

		/* Find the width and height of the portion of the explosion to draw */
		explosion_width = clipped_explosion_rect.right - clipped_explosion_rect.left;
		explosion_height = clipped_explosion_rect.bottom - clipped_explosion_rect.top;
		
		/* Find the number of longwords we will be drawing, and find whether there's a
			spare word or byte */
		explosion_longwords = explosion_width >> 2;
		extra_byte_flag = explosion_width & 0x1;
		extra_word_flag = (explosion_width >> 1) & 0x1;
		
		/* Find the where on the bitmap this should be drawn */
		explosion_dest_base = offscreen_bitmap + clipped_explosion_rect.top * VIEW_WIDTH +
								clipped_explosion_rect.left;
		
		/* Draw this explosion normally if it is not phased */
		if (!this_explosion->phased)
			
			{

			asm {
	
				move.l	explosion_height, d1					; count number of lines lines
				subq.l	#1, d1
	
			@line_loop:
			
				btst.b	#0, extra_byte_flag					; If there is an extra byte,
				beq		@no_extra_byte						; draw it
				
				move.b	(explosion_dest_base), d2			; mask this byte into the bitmap
				and.b	(explosion_mask_base)+, d2
				or.b	(explosion_base)+, d2
				move.b	d2, (explosion_dest_base)+
				
			@no_extra_byte:
			
				btst.b	#0, extra_word_flag					; if there is an extra word,
				beq		@no_extra_word						; draw it
	
				move.w	(explosion_dest_base), d2			; mask this word into the bitmap
				and.w	(explosion_mask_base)+, d2
				or.w	(explosion_base)+, d2
				move.w	d2, (explosion_dest_base)+
				
			@no_extra_word:				
				
				move.l	explosion_longwords, d0				; set counter to draw long words
				subq.l	#1, d0
				
				bmi		@no_longwords						; check for no longwords
				
			@longword_loop:									; draw the long words
	
				move.l	(explosion_dest_base), d2			; mask this longword into the bitmap
				and.l	(explosion_mask_base)+, d2
				or.l	(explosion_base)+, d2
				move.l	d2, (explosion_dest_base)+
				
				dbra	d0, @longword_loop
				
				addi.l	#VIEW_WIDTH, explosion_dest_base	; point to next view bitmap line
				sub.l	explosion_width, explosion_dest_base
				
				addi.l	#EXPLOSION_SIZE, explosion_base		; point to next source bitmap line
				sub.l	explosion_width, explosion_base
				
				addi.l	#EXPLOSION_SIZE, explosion_mask_base ; point to next mask line
				sub.l	explosion_width, explosion_mask_base
				
				dbra	d1, @line_loop
	
			@no_longwords:
			
				}
			}


		/* explosion is phased-- draw it phased */
		else
			
			{

			/* Find the phase masks */
			phase_lw_2 = this_explosion->phase_mask_lw;
			phase_lw_1 = phase_lw_2 ^ 0xFFFFFFFF;
			phase_word_2 = this_explosion->phase_mask_word;
			phase_word_1 = phase_word_2 ^ 0xFFFF;
			phase_byte_2 = this_explosion->phase_mask_byte;
			phase_byte_1 = phase_byte_2 ^ 0xFF;

			asm {
	
				move.l	d4, a1								; save d4
				move.l	d3, a0								; save d3
	
				move.l	explosion_height, d1				; count number of lines lines
				subq.l	#1, d1
	
			@Pline_loop:
			
				btst.b	#0, extra_byte_flag					; If there is an extra byte,
				beq		@Pno_extra_byte						; draw it
				
				move.b	(explosion_mask_base)+, d3			; mask this byte into the bitmap
				or.b	phase_byte_1, d3
				and.b	(explosion_dest_base), d3
				move.b	(explosion_base)+, d4
				and.b	phase_byte_2, d4
				or.b	d4, d3
				move.b	d3, (explosion_dest_base)+
				
			@Pno_extra_byte:
			
				btst.b	#0, extra_word_flag					; if there is an extra word,
				beq		@Pno_extra_word						; draw it
	
				move.w	(explosion_mask_base)+, d3			; mask this word into the bitmap
				or.w	phase_word_1, d3
				and.w	(explosion_dest_base), d3
				move.w	(explosion_base)+, d4
				and.w	phase_word_2, d4
				or.w	d4, d3
				move.w	d3, (explosion_dest_base)+
				
			@Pno_extra_word:				
				
				move.l	explosion_longwords, d0				; set counter to draw long words
				subq.l	#1, d0
				
				bmi		@Pno_longwords						; check for no longwords
				
			@Plongword_loop:								; draw the long words
	
				move.l	(explosion_mask_base)+, d3			; mask this longword into the bitmap
				or.l	phase_lw_1, d3
				and.l	(explosion_dest_base), d3
				move.l	(explosion_base)+, d4
				and.l	phase_lw_2, d4
				or.l	d4, d3
				move.l	d3, (explosion_dest_base)+
				
				dbra	d0, @Plongword_loop
				
				addi.l	#VIEW_WIDTH, explosion_dest_base	; point to next view bitmap line
				sub.l	explosion_width, explosion_dest_base
				
				addi.l	#EXPLOSION_SIZE, explosion_base		; point to next source bitmap line
				sub.l	explosion_width, explosion_base
				
				addi.l	#EXPLOSION_SIZE, explosion_mask_base ; point to next mask line
				sub.l	explosion_width, explosion_mask_base
				
				eor.l	#0xFFFFFFFF, phase_lw_1
				eor.l	#0xFFFFFFFF, phase_lw_2

				eor.w	#0xFFFF, phase_word_1
				eor.w	#0xFFFF, phase_word_2

				eor.b	#0xFF, phase_byte_1
				eor.b	#0xFF, phase_byte_2

				dbra	d1, @Pline_loop
	
			@Pno_longwords:
			
				move.l	a1, d4							; restore d4
				move.l	a0, d3							; restore d3

				}
			}
		
		}	/* loop through explosions */
		
}	/* draw_explosions() */



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
		
		moveq.l	#95, d0
	
	@longword_loop:
	
		move.l	(source)+, (dest)+
		dbra	d0, @longword_loop
		
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

	char		*enemy_position;	/* position of enemy on radar */
	char		*missile_position;	/* position of missile on radar */
	char		*explosion_position;/* position of explosion on radar */
	short		i;
	char		*radar_position;
	register short				color_word;
	register enemy_pointer 		this_enemy;

#define this_explosion	this_enemy
#define this_missile	this_enemy
	
#if 0
	/* If this is not the first time, get rid of the previous ship */
	if (!first_time)
		{
		/* Erase the ship blip */
		asm {
			move.l	ship_position, a0
			eor.w	#SHIP_COLOR, (a0)
			add.l	screen_width, a0
			eor.w	#SHIP_COLOR, (a0)
			}		
		}

	/* Find base address of current ship position */
	ship_position = radar_base + ((*enemies[0])->v >> 22)*screen_width + ((*enemies[0])->h >> 22);
	
	/* Draw the ship blip */
	asm {
		move.l	ship_position, a0
		eor.w	#SHIP_COLOR, (a0)
		add.l	screen_width, a0
		eor.w	#SHIP_COLOR, (a0)
		}		
#endif
	
	/* Draw enemies */
	for (i = 0; i < num_enemies; i++)
		{

		/* Dereference this enemy */
		this_enemy = *enemies[i];
		
		/* Find the color word to use to represent this enemy */
		switch (this_enemy->type)
			{
			
			case ship:
				color_word = ship_color_word;
				break;
			
			case biplane:
				color_word = biplane_color_word;
				break;
			
			case ground_gun:
			case smart_ground_gun:
				color_word = ground_gun_color_word;
				break;
			
			case cannon:
				color_word = cannon_color_word;
				break;
			
			case fireball:
				color_word = fireball_color_word;
				break;
			
			case silo:
				color_word = silo_color_word;
				break;
			
			}
		
		/* Find base address of current enemy position */
		enemy_position = radar_base + (this_enemy->v >> 22)*screen_width + (this_enemy->h >> 22);
	
		/* If the position is unchanged, don't do anything */
		if (enemy_position == this_enemy->radar_position)
			continue;

		/* If this enemy has been drawn before, erase the old position */
		if (radar_position = this_enemy->radar_position)
			{
			asm {
				move.l	radar_position, a0
				eor.w	color_word, (a0)
				add.l	screen_width, a0
				eor.w	color_word, (a0)
				}
			}
	
		/* Draw the new position */
		asm {
			move.l	enemy_position, a0
			eor.w	color_word, (a0)
			add.l	screen_width, a0
			eor.w	color_word, (a0)
			}		
	
		/* Remember the old position */
		this_enemy->radar_position = enemy_position;
	
		}


	/* Draw missiles */
	for (i = 0; i < num_missiles; i++)
		{

		/* Dereference this missile */
		this_missile = (enemy_pointer) *missiles[i];
		
		/* Find the color word to use to represent this enemy */
		color_word = missile_color_word;
		
		/* Find base address of current missile position */
		missile_position = radar_base + (((missile_pointer) this_missile)->v >> 22)*screen_width +
										(((missile_pointer) this_missile)->h >> 22);
	
		/* If the position is unchanged, don't do anything */
		if (missile_position == ((missile_pointer) this_missile)->radar_position)
			continue;

		/* If this missile has been drawn before, erase the old position */
		if (radar_position = ((missile_pointer) this_missile)->radar_position)
			{
			asm {
				move.l	radar_position, a0
				eor.w	color_word, (a0)
				add.l	screen_width, a0
				eor.w	color_word, (a0)
				}
			}
	
		/* Draw the new position */
		asm {
			move.l	missile_position, a0
			eor.w	color_word, (a0)
			add.l	screen_width, a0
			eor.w	color_word, (a0)
			}		
	
		/* Remember the old position */
		((missile_pointer) this_missile)->radar_position = missile_position;
	
		}


	/* Draw explosions */
	for (i = 0; i < num_explosions; i++)
		{

		/* Dereference this explosion */
		this_explosion = (enemy_pointer) *explosions[i];
		
		/* Find the color word to use to represent this explosion */
		color_word = fireball_color_word;
		
		/* Find base address of current explosion position */
		explosion_position = radar_base +
							(((explosion_pointer) this_explosion)->v >> 22)*screen_width +
								(((explosion_pointer) this_explosion)->h >> 22);
	
		/* If the position is unchanged, don't do anything */
		if (explosion_position == ((explosion_pointer) this_explosion)->radar_position)
			continue;

		/* If this missile has been drawn before, erase the old position */
		if (radar_position = ((explosion_pointer) this_explosion)->radar_position)
			{
			asm {
				move.l	radar_position, a0
				eor.w	color_word, (a0)
				add.l	screen_width, a0
				eor.w	color_word, (a0)
				}
			}
	
		/* Draw the new position */
		asm {
			move.l	explosion_position, a0
			eor.w	color_word, (a0)
			add.l	screen_width, a0
			eor.w	color_word, (a0)
			}		
	
		/* Remember the old position */
		((explosion_pointer) this_explosion)->radar_position = explosion_position;
	
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
	if (last_turret == (*the_ship)->turret)
		return;

	/* Erase the last turret indicator */
	RGBForeColor (&empty_color);
	draw_turret_indicator(last_turret);

	/* Draw the current turret indicator */
	RGBForeColor (&turret_color);
	draw_turret_indicator((*the_ship)->turret);

	/* Remember the current turret */
	last_turret = (*the_ship)->turret;
	
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
	draw_thermometer(&throttle_rect, (*the_ship)->throttle, MAX_THROTTLE, FALSE);
	
}	/* draw_throttle() */



/*****************************************************************************\
* procedure draw_damage_indicator                                             *
*                                                                             *
* Purpose: This procedure draws the damage indicator at its current position. *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_damage_indicator(void)
{

	/* Draw the damage indicator */
	draw_thermometer(&damage_rect, SHIP_HP - (*the_ship)->hit_points, SHIP_HP,
						((*the_ship)->hit_points <= 10));
	
}	/* draw_damage_indicator() */



/*****************************************************************************\
* procedure draw_energy_status                                                *
*                                                                             *
* Purpose: This procedure draws the energy status thermometer at its current  *
*          value.                                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void draw_energy_status(void)
{

	/* Draw the energy status thermometer */
	draw_thermometer(&energy_rect, (*the_ship)->energy, MAX_ENERGY,
						((*the_ship)->energy < DANGER_ENERGY));
	
}	/* draw_energy_status() */



#if 0
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
	mark = midpoint + chunk * (*the_ship)->rotation;
	
	/* Fill to the left of the mark */
	BlockMove(&bank_rect, &rect, sizeof(Rect));
	rect.right = rect.left + mark - 3;
	PaintRect (&rect);
	
	/* Fill to the right of the mark */
	BlockMove(&bank_rect, &rect, sizeof(Rect));
	rect.left += (mark + 3);
	PaintRect (&rect);
		
	/* Now do the filling */
	RGBForeColor(&status_color);
	
	/* Fill in the mark */
	BlockMove(&bank_rect, &rect, sizeof(Rect));
	rect.left += (mark - 3);
	rect.right = rect.left + 6;
	PaintRect (&rect);
	
}	/* draw_bank_status() */
#endif



/*****************************************************************************\
* procedure draw_status_lights                                                  *
*                                                                             *
* Purpose: This procedure draws the status buttons.                           *
*                                                                             *
* Parameters: draw_all: TRUE if we should draw all buttons.                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 29, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_status_lights(Boolean draw_all)
{

	static Boolean	last_weapons_lock_flag;
	static Boolean	last_lock_detected_flag;

	/* Draw the weapons lock light if it has changed */
	if (((!num_weapons_locks) != (!last_weapons_lock_flag)) || draw_all)
		{
		DrawPicture((num_weapons_locks) ? weapons_lock_on_pict : weapons_lock_off_pict,
						&weapons_lock_rect);
		
		last_weapons_lock_flag = num_weapons_locks;
		}

	/* Draw the lock detected light if it has changed */
	if (((!num_locks_detected) != (!last_lock_detected_flag)) || draw_all)
		{
		DrawPicture((num_locks_detected) ? lock_detected_on_pict : lock_detected_off_pict,
						&lock_detected_rect);
		
		last_lock_detected_flag = num_locks_detected;
		}

}	/* draw_status_lights() */



/*****************************************************************************\
* procedure draw_weapon                                                       *
*                                                                             *
* Purpose: This procedure draws one of the available weapons.                 *
*                                                                             *
* Parameters: which_weapon: the slot number of the weapon to draw             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 29, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_weapon(short which_weapon)
{

	Rect		weapon_rect;
	PicHandle	weapon_pict;
	
	/* Get the weapon's rect */
	BlockMove(&leftmost_weapon_rect, &weapon_rect, sizeof(Rect));
	OffsetRect(&weapon_rect, weapons_spacing * which_weapon, 0);

	/* Draw the picture for this weapon in the weapon slot */
	if (weapons[which_weapon])
		{
		if (which_weapon == primary_weapon)
			weapon_pict = GetPicture(FIRST_PRIMARY_WEAPON_ID + weapons[which_weapon] - 1);
		else if (which_weapon == secondary_weapon)
			weapon_pict = GetPicture(FIRST_SECONDARY_WEAPON_ID + weapons[which_weapon] - 1);
		else
			weapon_pict = GetPicture(FIRST_UNUSED_WEAPON_ID + weapons[which_weapon] - 1);
		
		DrawPicture(weapon_pict, &weapon_rect);
		}

}	/* draw_weapon() */



/*****************************************************************************\
* procedure draw_ability                                                      *
*                                                                             *
* Purpose: This procedure draws one of the available abilities.               *
*                                                                             *
* Parameters: which_ability: the slot number of the ability to draw           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 29, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void draw_ability(short which_ability)
{

	Rect		ability_rect;
	PicHandle	ability_pict;
	
	/* Get the ability's rect */
	BlockMove(&topmost_ability_rect, &ability_rect, sizeof(Rect));
	OffsetRect(&ability_rect, 0, abilities_spacing * which_ability);

	/* Draw the picture for this ability in the ability slot */
	if (abilities[which_ability])
		{
		ability_pict = GetPicture(FIRST_ABILITY_ID + abilities[which_ability] - 1);
		DrawPicture(ability_pict, &ability_rect);
		}

}	/* draw_ability() */



/*****************************************************************************\
* procedure update_score_bar                                                  *
*                                                                             *
* Purpose: This procedure draws the score bar.                                *
*                                                                             *
* Parameters: draw_all: TRUE if we should draw all values.                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_score_bar(Boolean draw_all)
{

	static Rect	score_rect = 			{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 55+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 120+ABILITIES_BAR_WIDTH};
	static Rect	high_score_rect =		{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 165+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 230+ABILITIES_BAR_WIDTH};
	static Rect	lives_rect =			{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 368+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 398+ABILITIES_BAR_WIDTH};
	static Rect	level_rect =			{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 285+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 315+ABILITIES_BAR_WIDTH};
	static Rect	crystals_found_rect =	{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 472+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 497+ABILITIES_BAR_WIDTH};
	static Rect	total_crystals_rect =	{VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+3, 511+ABILITIES_BAR_WIDTH, VIEW_HEIGHT+WEAPONS_BAR_HEIGHT+17, 536+ABILITIES_BAR_WIDTH};
	
	static RGBColor	erase_color = {0x3000, 0x3000, 0x3000};
	static RGBColor	text_color = {0xFFFF, 0xFFFF, 0xFFFF};
	
	static	last_score = -1;
	static	last_high = -1;
	static	last_level = -1;
	static	last_lives = -1;
	static	last_crystals_found = -1;
	static	last_total_crystals = -1;
	
	Str255	num_string;

	/* Set the default font to Chicago */
	TextFont (systemFont);

	if ((score != last_score) || (draw_all))
		{
		/* Erase the previous score */
		RGBForeColor(&erase_color);
		PaintRect(&score_rect);
	
		/* Draw the score */
		RGBForeColor(&text_color);
		MoveTo(score_rect.left+3, score_rect.bottom-3);
		NumToString(score, num_string);
		DrawString (num_string);
		
		last_score = score;
		}

	if ((high_score != last_high) || (draw_all))
		{
		/* Erase the previous high score */
		RGBForeColor(&erase_color);
		PaintRect(&high_score_rect);
	
		/* Draw the high score */
		RGBForeColor(&text_color);
		MoveTo(high_score_rect.left+3, high_score_rect.bottom-3);
		NumToString(high_score, num_string);
		DrawString (num_string);

		last_high = high_score;
		}
	
	if ((last_lives != lives) || (draw_all))
		{
		/* Erase the previous number of lives */
		RGBForeColor(&erase_color);
		PaintRect(&lives_rect);
		
		/* Draw the high score */
		RGBForeColor(&text_color);
		MoveTo(lives_rect.left+3, lives_rect.bottom-3);
		NumToString(lives, num_string);
		DrawString (num_string);
		
		last_lives = lives;
		}
		
	if ((last_level != level) || (draw_all))
		{
		/* Erase the previous high score */
		RGBForeColor(&erase_color);
		PaintRect(&level_rect);
		
		/* Draw the high score */
		RGBForeColor(&text_color);
		MoveTo(level_rect.left+3, level_rect.bottom-3);
		NumToString(level, num_string);
		DrawString (num_string);
	
		last_level = level;
		}
	
	if ((last_crystals_found != crystals_found) || (draw_all))
		{
		/* Erase the previous crystal count */
		RGBForeColor(&erase_color);
		PaintRect(&crystals_found_rect);
		
		/* Draw the high score */
		RGBForeColor(&text_color);
		MoveTo(crystals_found_rect.left+3, crystals_found_rect.bottom-3);
		NumToString(crystals_found, num_string);
		DrawString (num_string);
	
		last_crystals_found = crystals_found;
		}
	
	if ((last_total_crystals != total_crystals) || (draw_all))
		{
		/* Erase the previous crystal count */
		RGBForeColor(&erase_color);
		PaintRect(&total_crystals_rect);
		
		/* Draw the high score */
		RGBForeColor(&text_color);
		MoveTo(total_crystals_rect.left+3, total_crystals_rect.bottom-3);
		NumToString(total_crystals, num_string);
		DrawString (num_string);
	
		last_total_crystals = total_crystals;
		}
	
}	/* update_score_bar() */



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
	BlockMove(therm_rect, &rect, sizeof(Rect));
	rect.right = rect.left + bar_length;
	
	/* Set the color to danger for a danger display or to status for a status display */
	RGBForeColor ( (danger) ? &danger_color : &status_color );
	
	/* Fill in the full part */
	PaintRect (&rect);
	
	/* Find the rectangle for the empty part */
	BlockMove(therm_rect, &rect, sizeof(Rect));
	rect.left += bar_length;
	
	/* Set the color to the empty color */
	RGBForeColor (&empty_color);
	
	/* Fill the empty part */
	PaintRect (&rect);
		
}	/* draw_thermometer() */



