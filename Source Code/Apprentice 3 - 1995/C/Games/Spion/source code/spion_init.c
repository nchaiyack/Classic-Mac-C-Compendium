/*****
 * spion_init.c
 *
 *	Spion is a game under development.  This segment performs various initializations
 *
 *
 *****/

#include <QDOffscreen.h>
#include <FixMath.h>
#include <Retrace.h>
#include <Start.h>
#include "spion_types.h"
#include "spion_proto.h"
#include "spion_resources.h"
#include "spion_defines.h"
#include "spion_externs.h"


/***************************** Globals ****************************/

PicHandle		controls_pict;		/* The control panel picture */
PicHandle		score_bar_pict;		/* The score bar picture */

Boolean			sound_done;


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
	
	/* Accept key-up events */
	SetEventMask (SysEvtMask | keyUpMask);
	
/*	install_vbl_task();	*/
	
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
	DrawMenuBar();	/* Draw the new menu bar */
	
	/* add the desk accessories */
	AddResMenu(apple_menu, 'DRVR');

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
	Rect			dest_rect;

	/* Open the main window */
	main_window = GetNewCWindow (MAIN_WINDOW, &main_window_rec, (WindowPtr)-1L );
	
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
	
	/* Find the width of the window */
	main_window_width = main_window_bounds.right - main_window_bounds.left;
	
	/* Find the height of the window */
	main_window_height = main_window_bounds.bottom - main_window_bounds.top;

	/* Check whether we need to be in 32-bit mode to access the screen memory */
    screen_pixmap_32_flag = PixMap32Bit(main_window_pixmap);
    
	/* Get the pixmap for the main window */
	main_window_pixmap = ((CGrafPtr) main_window)->portPixMap;

	/* Lock down the screen */
    LockPixels(main_window_pixmap);

	/* Find the base address of the upper left corner of the window */
	main_window_base = ((char *) GetPixBaseAddr(main_window_pixmap)) +
						(main_window_top + WEAPONS_BAR_HEIGHT) * screen_width +
							main_window_left + ABILITIES_BAR_WIDTH;

	/* Find the base address of the radar screen */
	radar_base = (char *) GetPixBaseAddr(main_window_pixmap) +
					(main_window_top + radar_rect.top) * screen_width +
						main_window_left + radar_rect.left;

	/* Set friction constant */
	friction = X2Fix(0.008);
	
	/* Set throttle constant */
	throttle_const = X2Fix(0.03);

	/* Set rotation constant */
	rotation_const = X2Fix(0.06);

}	/* init_windows() */



/*****************************************************************************\
* procedure init_data                                                         *
*                                                                             *
* Purpose: This procedure initializes data used by the program.               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 4, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void init_data(void)
{

	unsigned short	i, j;
	extended		r;
	Fixed			fixed_r;
	short			error;
	unsigned short	island_h, island_v;
	short			islands_found;
	Boolean			overlap_flag;
	char			byte = 0;
	extended		Pi = 3.14159265358979323846;
	world_size_resource	size_resource;

	/* Initialize random numbers */
	GetDateTime(&randSeed);

	/* Read in the size info for world #0 */
	size_resource = (world_size_resource) GetResource ('wSiz', 0);
	world_width = (*size_resource)->width;
	world_height = (*size_resource)->height;
		
	/* Load in the world #0 */
	world_handle = GetResource('wMap', 0);
	DetachResource(world_handle);
		
	/* Dereference the world handle for speed */
	HLock(world_handle);
	world = *world_handle;
	
	/* Set up the directional vectors */
	for (i = 0, r = 0.; i < NUM_DIRECTIONS; i++, r += (2*Pi)/NUM_DIRECTIONS)
		{
		
		/* Convert r to to a fixed point number */
		fixed_r = X2Fix(r);
	
		/* Take the sin of fixed_r, and remember it */
		h_vector[i] = (FracSin (fixed_r) >> 14);
		v_vector[i] = (-FracCos (fixed_r) >> 14);
		
		}

	/* Read in the sound resources */
	explosion_sound = GetResource ('snd ', EXPLOSION_SOUND);
	bullet_sound = GetResource ('snd ', BULLET_SOUND);
	cannon_sound = GetResource ('snd ', CANNON_SOUND);
	crystal_sound = GetResource ('snd ', CRYSTAL_SOUND);
	goodie_sound = GetResource ('snd ', GOODIE_SOUND);
	laser_sound = GetResource ('snd ', LASER_SOUND);
	blaster_sound = GetResource ('snd ', BLASTER_SOUND);
	circle_sound = GetResource ('snd ', CIRCLE_SOUND);
	new_level_sound = GetResource ('snd ', NEW_LEVEL_SOUND);
	tracer_sound = GetResource ('snd ', TRACER_SOUND);
	clunk_sound = GetResource ('snd ', CLUNK_SOUND);
	HNoPurge(bullet_sound);
	HNoPurge(explosion_sound);
	HNoPurge(cannon_sound);
	HNoPurge(crystal_sound);
	HNoPurge(goodie_sound);
	HNoPurge(laser_sound);
	HNoPurge(blaster_sound);
	HNoPurge(circle_sound);
	HNoPurge(new_level_sound);
	HNoPurge(clunk_sound);

	/* Create our sound channels */
	explosion_sound_channel = NULL;
	SndNewChannel(&explosion_sound_channel, sampledSynth, initMono+initNoInterp, sound_done_proc);  
	effects_sound_channel = NULL;
	SndNewChannel(&effects_sound_channel, sampledSynth, initMono+initNoInterp, sound_done_proc);  
	weapons_sound_channel = NULL;
	SndNewChannel(&weapons_sound_channel, sampledSynth, initMono+initNoInterp, sound_done_proc);  

	/* Set the keys for rotating, thrusting, and firing */
	turn_left_key = '\34';
	turn_right_key = '\35';
	turn_hard_left_key = '.';
	turn_hard_right_key = '/';
	turret_left_key = 'q';
	turret_right_key = 'w';
	throttle_up_key = 'a';
	throttle_down_key = 'z';
	throttle_stop_key = 'x';
	shield_key = 's';
	fire_primary_key = ' ';
	fire_secondary_key = 'd';
	pause_key = 'p';

}	/* init_data() */


pascal void sound_done_proc(SndChannelPtr channel, SndCommand command)
{

	sound_done = TRUE;

}	/* sound_done_proc() */


/*****************************************************************************\
* procedure init_new_game                                                     *
*                                                                             *
* Purpose: This procedure initializes variables for a new game.               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void init_new_game(void)
{

	short	i;

	/* Set progress variables */
	score = 0;
	lives = 5;
	level = 1;
	crystals_found = 0;
	total_crystals = 1;
	
	/* Start with nothing of anything */
	num_bullets = 0;
	num_tracers = 0;
	num_bombs = 0;
	num_large_bombs = 0;
	num_explosions = 0;
	num_shells = 0;
	num_missiles = 0;
	num_laser_beams = 0;
	num_circles = 0;
	num_blasters = 0;
	num_enemies = 0;
	num_bases = 0;
	num_goodies = 0;
	num_live_enemies = 0;
	
	/* Draw the score bar */
	update_score_bar(TRUE);
	
	/* We have only a single-fire gun */
	for (i = 1; i < NUM_WEAPON_SLOTS; i++)
		weapons[i] = 0;
	weapons[0] = single_fire_gun;
	weapons[1] = rapid_fire_gun;
	weapons[2] = tracer_gun;
	weapons[3] = laser_gun;
	weapons[4] = bomb_dropper;
	weapons[5] = large_bomb_dropper;
	weapons[6] = smart_bomb_dropper;
	weapons[7] = circle_weapon;
	weapons[8] = blaster_weapon;
	primary_weapon = 1;
	secondary_weapon = 4;
	
	/* We have no abilities */
	for (i = 0; i < NUM_ABILITY_SLOTS; i++)
		abilities[i] = 0;
	abilities[0] = phase_ability;
	abilities[1] = water_energy_ability;
	abilities[2] = low_energy_ability;
	abilities[3] = medium_energy_ability;
	abilities[4] = high_energy_ability;
	abilities[5] = auto_aim_ability;
	
	/* Draw the abilities */
	for (i = 0; i < NUM_ABILITY_SLOTS; i++)
		draw_ability(i);
	
	/* Draw the weapons */
	for (i = 0; i < NUM_WEAPON_SLOTS; i++)
		draw_weapon(i);

}	/* init_new_game() */



/*****************************************************************************\
* procedure init_new_level                                                    *
*                                                                             *
* Purpose: This procedure initializes variables for a new level.              *
*                                                                             *
* Parameters: level: the level number we're on now                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

void init_new_level(short level)
{

	RGBColor	wall_color = {0xFFFF, 0xFFFF, 0x0000};
	Rect		wall_rect;
	short		i;
	Fixed		biplane_h, biplane_v;
	long		biplane_direction;

	/* get rid of anything which is left over from last live/level */
	while (num_bullets) destroy_bullet(0);
	while (num_tracers) destroy_tracer(0);
	while (num_bombs) destroy_bomb(0);
	while (num_large_bombs) destroy_large_bomb(0);
	while (num_explosions) destroy_explosion(0);
	while (num_shells) destroy_shell(0);
	while (num_missiles) destroy_missile(0);
	while (num_laser_beams) destroy_laser_beam(0);
	while (num_circles) destroy_circle(0);
	while (num_blasters) destroy_blaster(0);
	while (num_enemies) destroy_enemy(0);
	while (num_bases) destroy_base(0);
	while (num_goodies) destroy_goodie(0);

	/* Create ship at center of world */
	create_enemy(ship, ((world_width/2) * BLOCK_WIDTH) << 16,
					((world_height/2) * BLOCK_HEIGHT) << 16, (2L << 16), 0, -1);

	/* Get a working copy of our ship's handle */
	the_ship = enemies[0];

	/* Start with throttle of 2 */
	(*the_ship)->throttle = 2;

	/* Start with no rotation */
	(*the_ship)->rotation = 0;

	/* Start with full energy */
	(*the_ship)->energy = MAX_ENERGY;

	/* At the start, we're not firing or rotating the turret */
	primary_firing = FALSE;
	secondary_firing = FALSE;
	(*the_ship)->turret_rotation = 0;
	(*the_ship)->turret = 0;

	/* Draw the throttle */
	draw_throttle();

	/* Draw the turret */
	draw_turret();

	/* Draw the energy thermometer */
	draw_energy_status();

	/* Draw the damage thermometer */
	draw_damage_indicator();

	/* Draw the status lights */
	num_weapons_locks = num_locks_detected = 0;
	draw_status_lights(TRUE);

	/* Erase the radar */
	RGBForeColor(&empty_color);
	PaintRect(&radar_rect);

	/* Draw the Wall at the End of the World on the radar */
	RGBForeColor(&wall_color);
	BlockMove(&radar_rect, &wall_rect, sizeof(Rect));
	InsetRect(&wall_rect, WORLD_MARGIN, WORLD_MARGIN);
	FrameRect(&wall_rect);	

	/* Draw the radar */
	update_radar(TRUE);

	/* We are not currently exploding */
	(*the_ship)->exploding = 0;

	/* Add some enemies */
#if 0
	for (i = 0; i < level; i++)
		{
		/* Create a new biplane at a random location and direction */
		
		biplane_h = ((unsigned short) Random()) *
						((world_width-2*WORLD_MARGIN)*BLOCK_WIDTH) / 65536L +
							WORLD_MARGIN * BLOCK_WIDTH;
		biplane_v = ((unsigned short) Random()) *
						((world_width-2*WORLD_MARGIN)*BLOCK_WIDTH) / 65536L +
							WORLD_MARGIN * BLOCK_WIDTH;
		biplane_direction = ((unsigned short) Random()) * (NUM_DIRECTIONS/65535);
		
		create_enemy(biplane, biplane_h << 16, biplane_v << 16, (4L << 16), (biplane_direction << 16));

		}
#endif

	/* Add a base in the middle */
	create_base(1, 4000L << 16, 4000L << 16);

	/* Add a ground gun */
	create_enemy(ground_gun, 2000L << 16, 2000L << 16, 0L, 0L, -1);

	/* Add a cannon */
	create_enemy(cannon, 6000L << 16, 6000L << 16, 0L, 0L, -1);

	/* Add a silo */
	create_enemy(silo, 6000L << 16, 2000L << 16, 0L, 0L, -1);


}	/* init_new_level() */



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

#define	PIXMAP_SIZE	128

	short		i, j, k;
	Handle		handle, offscreen_bitmap_handle, ship_handle, ship_mask_handle;
	Ptr			ship_pointer, pixmap_ship_pointer, ship_mask_pointer, screen_pat_base;
	PicHandle	picture_handle;
	Rect		dest_rect;
	long		picture_size;
	Ptr			pointer;
	Rect		ship_rect = {0, 0, 33, 33};
	RGBColor	rgb_white = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor	rgb_black = {0x0000, 0x0000, 0x0000};
	RGBColor	ship_color = {0xFFFF, 0xFFFF, 0xFFFF};
	RGBColor	biplane_color = {0x0000, 0xFFFF, 0xFFFF};
	RGBColor	cannon_color = {0x0000, 0xFFFF, 0x0000};
	RGBColor	missile_color = {0xFFFF, 0x0000, 0x0000};
	RGBColor	ground_gun_color = {0xFFFF, 0xFFFF, 0x0000};
	RGBColor	explosion_color = {65535, 54998, 8481};
	RGBColor	silo_color = {0xFFFF, 0x0000, 0xFFFF};
	Rect		port_bounds = {0, 0, PIXMAP_SIZE, PIXMAP_SIZE};
	CGrafPort	offscreen_port;
	char		*pixmap_base;
	Rect		view_rect = {WEAPONS_BAR_HEIGHT, ABILITIES_BAR_WIDTH,
								VIEW_HEIGHT + WEAPONS_BAR_HEIGHT, VIEW_WIDTH + ABILITIES_BAR_WIDTH};
	Rect		init_bar_rect = {360, 72, 368, 412};
	long		init_counter = 0;
	StringHandle string_handle;

	Rect	offscreen_view_bounds = {0, 0, VIEW_HEIGHT, VIEW_WIDTH};

#define	NUM_MULTIFRAME_BITMAPS	18

	typedef struct
		{
		short	first_pict_id;
		short	num_picts;
		short	pict_size;
		Ptr		*bitmaps;
		Ptr		*masks;
		} bitmap_info;
	
	/* The table below is used to read multi-frame bitmaps into memory.  The columns are, in order, the resource
		if of the first PICT of the series (the rest must follow consecutively), the number of frames (number of
		PICTs), the size in pixels of one size of the (square) picture, the array of pointers to the bitmaps,
		and the array of pointers to the masks */
		
	bitmap_info	bitmap_init_info[NUM_MULTIFRAME_BITMAPS] =

		{
	
		{ FIRST_SHIP_ID, 			NUM_SHIPS, 			SHIP_SIZE, 			ships, 				ship_masks				},
		{ FIRST_BIPLANE_ID, 		NUM_BIPLANES, 		BIPLANE_SIZE, 		biplanes, 			biplanes_masks 			},
		{ FIRST_ENEMY_JET_ID, 		NUM_ENEMY_JETS, 	ENEMY_JET_SIZE, 	enemy_jets, 		enemy_jets_masks 		},
		{ FIRST_FLYING_WING_ID, 	NUM_FLYING_WINGS, 	FLYING_WING_SIZE, 	flying_wings, 		flying_wings_masks 		},
		{ FIRST_BLASTER_ENEMY_ID, 	NUM_BLASTER_ENEMIES,BLASTER_ENEMY_SIZE, blaster_enemies,	blaster_enemies_masks 	},
		{ FIRST_CIRCLE_ENEMY_ID, 	NUM_CIRCLE_ENEMIES, CIRCLE_ENEMY_SIZE, 	circle_enemies, 	circle_enemies_masks 	},
		{ FIRST_GUN_ID, 			NUM_GUNS, 			GUN_SIZE, 			guns, 				guns_masks 				},
		{ FIRST_LASER_GUN_ID, 		NUM_LASER_GUNS, 	LASER_GUN_SIZE, 	laser_guns, 		laser_guns_masks 		},
		{ FIRST_CANNON_ID, 			NUM_CANNONS, 		CANNON_SIZE, 		cannons, 			cannons_masks 			},
		{ FIRST_SILO_ID, 			NUM_SILOS, 			SILO_SIZE, 			silos, 				silos_masks 			},
		{ FIRST_TRACER_ID, 			NUM_TRACERS, 		TRACER_SIZE, 		tracers_picts,		tracers_masks 			},
		{ FIRST_EXPLOSION_ID, 		NUM_EXPLOSIONS,		EXPLOSION_SIZE, 	explosions_picts,	explosions_masks 		},
		{ FIRST_MISSILE_ID, 		NUM_MISSILES, 		MISSILE_SIZE, 		missiles_picts,		missiles_masks 			},
		{ FIRST_FIREBALL_ID, 		NUM_FIREBALLS, 		FIREBALL_SIZE, 		fireballs, 			fireballs_masks 		},
		{ FIRST_GOODIE_ID, 			NUM_GOODIES, 		GOODIE_SIZE, 		goodies_picts, 		goodies_masks 			},
		{ FIRST_BASE_TOP_ID, 		NUM_BASES, 			BASE_SIZE, 			base_tops, 			base_tops_masks 		},
		{ FIRST_BASE_BOTTOM_ID, 	NUM_BASES, 			BASE_SIZE, 			base_bottoms,		base_bottoms_masks 		},
		{ FIRST_DEAD_BASE_ID, 		NUM_BASES, 			BASE_SIZE, 			dead_bases, 		dead_bases_masks 		}

		 };

	long		total_init_count = 0;
	
	/* Read the Welcome to Spion picture into memory */
	welcome_to_spion_pict = GetPicture(WELCOME_PICT);
	BlockMove(&((*welcome_to_spion_pict)->picFrame), &welcome_rect, sizeof(Rect));
	OffsetRect(&welcome_rect,
				ABILITIES_BAR_WIDTH + VIEW_WIDTH/2  - ((welcome_rect.right - welcome_rect.left)/2),
				WEAPONS_BAR_HEIGHT + VIEW_HEIGHT/2 - ((welcome_rect.bottom - welcome_rect.top)/2) );

	/* Read the Game Over picture into memory */
	game_over_pict = GetPicture(GAME_OVER_PICT);
	BlockMove(&((*game_over_pict)->picFrame), &game_over_rect, sizeof(Rect));
	OffsetRect(&game_over_rect,
				ABILITIES_BAR_WIDTH + VIEW_WIDTH/2  - ((game_over_rect.right - game_over_rect.left)/2),
				WEAPONS_BAR_HEIGHT + VIEW_HEIGHT/2 - ((game_over_rect.bottom - game_over_rect.top)/2) );

	/* Read the weapons lock pictures into memory */
	weapons_lock_on_pict = GetPicture(WEAPONS_LOCK_ON_PICT);
	weapons_lock_off_pict = GetPicture(WEAPONS_LOCK_OFF_PICT);
	lock_detected_on_pict = GetPicture(LOCK_DETECTED_ON_PICT);
	lock_detected_off_pict = GetPicture(LOCK_DETECTED_OFF_PICT);

	/* Read the Game Over picture into memory */
	game_over_pict = GetPicture(GAME_OVER_PICT);
	BlockMove(&((*game_over_pict)->picFrame), &game_over_rect, sizeof(Rect));
	OffsetRect(&game_over_rect,
				ABILITIES_BAR_WIDTH + VIEW_WIDTH/2  - ((game_over_rect.right - game_over_rect.left)/2),
				WEAPONS_BAR_HEIGHT + VIEW_HEIGHT/2 - ((game_over_rect.bottom - game_over_rect.top)/2) );

	/* Draw the main window, and the Welcome to Spion picture */
	SetPort(main_window);
	InvalRect(&main_window->portRect);
	handle_update_event();
	RGBForeColor(&rgb_black);
	PaintRect(&view_rect);
	DrawPicture(welcome_to_spion_pict, &welcome_rect);	

	/* Draw the initialization status bar */
	RGBForeColor(&rgb_white);
	InsetRect(&init_bar_rect, -1, -1);
	FrameRect(&init_bar_rect);
	InsetRect(&init_bar_rect, 1, 1);

	/* Create an offscreen area to draw these graphics data */
	create_offscreen_grafport(&offscreen_port, &port_bounds, &pixmap_base);
	
	/* Count the number of notches on the thermometer */
	for (i = 0; i <	NUM_MULTIFRAME_BITMAPS; i++)
		total_init_count += bitmap_init_info[i].num_picts;
	
	total_init_count += NUM_TERRAINS;
	
	/* Read the terrains into memory */
	for (i = 0; i < NUM_TERRAINS; i++)
		{

		/* Transfer this pattern picture to memory */
		read_picture(FIRST_TERRAIN_ID + i, BLOCK_HEIGHT, pixmap_base, &terrains[i]);		
		
		/* Draw another thermometer notch */
		SetPort(main_window);
		draw_thermometer(&init_bar_rect, ++init_counter, total_init_count, FALSE);
		SetPort(&offscreen_port);

		}

	/* Fill in a square of white where the bitmaps will appear */
	RGBForeColor(&rgb_white);
	PaintRect(&ship_rect);
	
	/* Read the multi-frame bitmaps into memory */
	for (i = 0; i <	NUM_MULTIFRAME_BITMAPS; i++)
		read_pictures_and_masks(bitmap_init_info[i].first_pict_id,
								bitmap_init_info[i].num_picts,
								bitmap_init_info[i].pict_size,
								bitmap_init_info[i].bitmaps,
								bitmap_init_info[i].masks,
								&offscreen_port, pixmap_base, &init_bar_rect,
								&init_counter, total_init_count);		

	/* Read the shell, bomb, large bomb, and spiked ball pictures (each has only one frame) */
	read_picture_and_mask(SHELL_ID, SHELL_SIZE, pixmap_base, &shell_pict, &shell_mask);
	read_picture_and_mask(BOMB_ID, BOMB_SIZE, pixmap_base, &bomb_pict, &bomb_mask);
	read_picture_and_mask(LARGE_BOMB_ID, BOMB_SIZE, pixmap_base, &large_bomb_pict, &large_bomb_mask);
	read_picture_and_mask(SPIKED_BALL_ID, SPIKED_BALL_SIZE, pixmap_base, &spiked_ball_pict, &spiked_ball_mask);
		
	/* Find the color radar word for various radar objects */
	biplane_color_word = find_color_word(&biplane_color, pixmap_base);
	ship_color_word = find_color_word(&ship_color, pixmap_base);
	fireball_color_word = find_color_word(&explosion_color, pixmap_base);
	cannon_color_word = find_color_word(&cannon_color, pixmap_base);
	ground_gun_color_word = find_color_word(&ground_gun_color, pixmap_base);
	missile_color_word = find_color_word(&missile_color, pixmap_base);
	silo_color_word = find_color_word(&silo_color, pixmap_base);
	
	/* Create an offscreen area used for quick screen updating */
	create_offscreen_grafport(&offscreen_view_port, &offscreen_view_bounds, &offscreen_bitmap);
		
	/* Throw away the offscreen pixmap */
	dispose_offscreen_grafport(&offscreen_port);
	
	/* Set thePort back to the window */
	SetPort(main_window);

	/* Erase the thermometer */
	InsetRect(&init_bar_rect, -1, -1);
	RGBForeColor(&rgb_black);
	PaintRect(&init_bar_rect);
	
	/* Draw the instructions string */
	string_handle = GetString (FIRST_INSTRUCTIONS);
	MoveTo(init_bar_rect.left + 25, init_bar_rect.bottom);
	RGBForeColor(&biplane_color);
	DrawString(*string_handle);
	ReleaseResource(string_handle);

}	/* init_graphics_data() */



/*****************************************************************************\
* procedure find_color_word                                                   *
*                                                                             *
* Purpose: This procedure finds the word value associated with two pixels of  *
*          a given RGB color in the current palette of the current pixmap.    *
*                                                                             *
* Parameters: color: 		the color                                         *
*             pixmap_base:	the base address of the PixMap                    *
*             returns:      the word corresponding to two pixels of color.    *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 28, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

short find_color_word(RGBColor *color, Ptr pixmap_base)
{

	/* We simply draw a two dots of the color at coordinate (0, 0), and (1, 0),
		and then read the word directly */
	RGBForeColor(color);
	MoveTo(0, 0);
	Line(1, 0);
	return (*( (short *) pixmap_base) ^ 0xFFFF);

}	/* find_color_word() */
	


/*****************************************************************************\
* procedure read_pictures_and_masks                                           *
*                                                                             *
* Purpose: This procedure reads all PICTs of a certain type into memory, and  *
*          generates masks for them.                                          *
*                                                                             *
* Parameters: first_resource_id: the resource id of the first PICT            *
*             num_picts:         the number of PICTs to read into memory      *
*             bitmap_size:       the length of one size of the (square) PICT  *
*             bitmaps:           received array of handles to bitmaps         *
*             masks:             received array of handles to masks           *
*             offscreen_port:    an offscreen CGrafPort to be used as scratch *
*             pixmap_base:       base address of pixmap of offscreen port.    *
*             status_rect:       rectangle containing the status thermometer  *
*             status_count:      number of ticks on the status thermometer    *
*             total_count:       total possible ticks on the status therm.    *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 28, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void read_pictures_and_masks(short first_resource_id, short num_picts, short bitmap_size,
								Ptr *bitmaps, Ptr *masks, CGrafPtr offscreen_port,
								Ptr pixmap_base, Rect *status_rect, long *status_count,
								long total_count)
{

	long i;

	for (i = 0; i < num_picts; i++)
		{
		read_picture_and_mask(first_resource_id + i, bitmap_size, pixmap_base,
								&bitmaps[i], &masks[i]);		
		
		/* Draw another thermometer notch */
		SetPort(main_window);
		draw_thermometer(status_rect, ++(*status_count), total_count, FALSE);
		SetPort(offscreen_port);

		}

}	/* read_pictures_and_masks() */
		


/*****************************************************************************\
* procedure read_picture                                                      *
*                                                                             *
* Purpose: This procedure reads a PICT resource from the resource file, draws *
*          it in the offscreen bitmap, and copies it to its own bitmap.       *
*                                                                             *
* Parameters: pict_id:       the resource id of the PICT                      *
*             pict_size:     the length of one size of the (square) PICT      *
*             pixmap_base:   base address of offscreen pixmap                 *
*             bitmap_ptr:    receives pointer to new bitmap containing pict   *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void read_picture(short pict_id, long pict_size, Ptr pixmap_base, Ptr *bitmap_ptr)
{

	PicHandle	picture_handle;
	Rect		dest_rect;
	long		picture_length;
	Ptr			pixmap_pointer;
	short		i, j;
	Ptr			quick_bitmap_ptr;
	Handle		bitmap_handle;
	Ptr			screen_pat_base;

	/* Draw the picture in the bitmap */
	picture_handle = GetPicture(pict_id);
	BlockMove(&((*picture_handle)->picFrame), &dest_rect, 8);
	DrawPicture(picture_handle, &dest_rect);
	
	/* Allocate a block of memory for the bitmap */
	picture_length = pict_size*pict_size;
	bitmap_handle = NewHandle(picture_length);
	
	/* Lock down the handle and dereference it for speed */
	HLock(bitmap_handle);
	quick_bitmap_ptr = *bitmap_ptr = *bitmap_handle;
	
	/* Find the base address of the picture in the offscreen pixmap */
	screen_pat_base = pixmap_base;
			
	/* Copy the data from the offscreen bitmap to the picture bitmap in memory */
	for (j = 0; j < pict_size; j++)
		{
		/* Move this line */
		BlockMove(screen_pat_base, quick_bitmap_ptr, pict_size);
		
		/* Go to next line */
		screen_pat_base += PIXMAP_SIZE;
		quick_bitmap_ptr += pict_size;
		}
		
	/* Free the memory used by the PICT */
	ReleaseResource(picture_handle);
		
}	/* read_picture() */



/*****************************************************************************\
* procedure read_picture_and_mask                                             *
*                                                                             *
* Purpose: This procedure reads a PICT resource from the resource file, draws *
*          it in the offscreen bitmap, and copies it to its own bitmap.  It   *
*          also creates a mask from the picture, and copies that to a bitmap. *
*                                                                             *
* Parameters: pict_id:       the resource id of the PICT                      *
*             pict_size:     the length of one size of the (square) PICT      *
*             pixmap_base:   base address of offscreen pixmap                 *
*             bitmap_handle: receives pointer to new bitmap containing pict   *
*             mask_handle:   receives pointer to new bitmap containing mask   *
*                                                                             *
* Parameters: bounds_rect: the size of the desired bitmap.                    *                                                                             *
*                                                                             *                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void read_picture_and_mask(short pict_id, long pict_size, Ptr pixmap_base, 
							Ptr *bitmap_ptr, Ptr *mask_ptr)
{

	PicHandle	picture_handle;
	Rect		dest_rect;
	long		picture_length;
	Ptr			pixmap_pointer;
	short		i, j;
	Ptr			quick_bitmap_ptr;
	Ptr			quick_mask_ptr;
	Handle		bitmap_handle;
	Handle		mask_handle;

	/* Draw the picture on the bitmap */
	picture_handle = GetPicture(pict_id);
	BlockMove(&((*picture_handle)->picFrame), &dest_rect, 8);
	DrawPicture(picture_handle, &dest_rect);

	/* Allocate two blocks, one for the bitmap and one for the mask */
	picture_length = pict_size*pict_size;
	bitmap_handle = NewHandle(picture_length);
	mask_handle = NewHandle(picture_length);
	
	/* Lock down the handles and dereference them for speed */
	HLock(bitmap_handle);
	HLock(mask_handle);
	quick_bitmap_ptr = *bitmap_ptr = *bitmap_handle;
	quick_mask_ptr = *mask_ptr = *mask_handle;
		
	/* Find the base address of the picture in the offscreen pixmap */
	pixmap_pointer = pixmap_base;
			
	/* Copy the data from the screen bitmap to the bitmap in memory, and
		make a mask of it at the same time */
	for (i = 0; i < pict_size; i++)
		{
		for (j = 0; j < pict_size; j++)
			{
			
			/* Copy a byte to the ship bitmap in memory */
			*(quick_bitmap_ptr++) = *(pixmap_pointer);
			
			/* Convert this byte into a mask */
			*(quick_mask_ptr++) = *(pixmap_pointer++) ? 0x00 : 0xFF;
			
		}
		
		/* Go to the beginning of the next line */
		pixmap_pointer += PIXMAP_SIZE - pict_size;
		
		}
			
	/* Free the memory used by the PICT */
	ReleaseResource(picture_handle);

}	/* read_picture_and_mask() */



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
* procedure install_vbl_task                                                  *
*                                                                             *
* Purpose: This procedure installs the main screen VBL task which is used to  *
*          synch the animation with the screen refresh.                       *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 10, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

/* We use this structure so that the VBL task can find our A5 and use it to
	access our globals */
typedef struct
	{
	long application_A5;
	VBLTask the_vbl_task;
	} VBL_task_A5_struct;

VBL_task_A5_struct		refresh_task;	/* installed task */

#define	VBLS_BETWEEN_FRAMES	1

void install_vbl_task(void)
{

	short		error;
	DefVideoRec	video_info;

	/* Find the slot number of the video card */
	GetVideoDefault (&video_info);

	/* Install the screen refresh interrupt */
	refresh_task.the_vbl_task.qType = vType;
	refresh_task.the_vbl_task.vblAddr = (ProcPtr) set_refresh_flag;
	refresh_task.the_vbl_task.vblCount = VBLS_BETWEEN_FRAMES;
	refresh_task.the_vbl_task.vblPhase = 0; 
	
	/* Save the applicationsŐs A5 where the VBL tasks can find it; right 
	 	before the VBL Task structure.  We need the VBL Tasks to have access
	 	to the application globals, so they need to know what the applicationŐs
	 	A5 global pointer is. */
	refresh_task.application_A5 = (long) CurrentA5;	/* a Macintosh global */

	/* Install the task */		
	error = SlotVInstall((QElemPtr)&refresh_task.the_vbl_task, video_info.sdSlot);

}	/* install_vbl_task() */



/*****************************************************************************\
* procedure remove_vbl_task                                                   *
*                                                                             *
* Purpose: This procedure removes the VBL task which is used to synch the     *
*          animation with the screen refresh.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 10, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void remove_vbl_task(void)
{
		
	DefVideoRec video_info;

	/* Find the slot number of the video card */
	GetVideoDefault (&video_info);

	/* Remove the vbl task */
	SlotVRemove ((QElemPtr)&refresh_task.the_vbl_task, video_info.sdSlot);

}	/* remove_vbl_task() */



/*****************************************************************************\
* procedure set_refresh_flag                                                  *
*                                                                             *
* Purpose: This is the VBL task which makes refresh_flag TRUE every vbl.      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 10, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

pascal void set_refresh_flag(void)
{

	/* The base address of this tasks VBL structure is now in a0.  We can use
		this to get the applicationŐs a5 global pointer. */
	asm {
		move.l a5, -(SP)			; save current a5
		move.l -4(a0), a5			; get applicationsŐs a5
		}

	refresh_flag = TRUE;
	
	refresh_task.the_vbl_task.vblCount = VBLS_BETWEEN_FRAMES;
	

	asm	{
		move.l (SP)+, a5		; restore the old a5
		}

}	/* set_refresh_flag() */


