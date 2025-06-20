/***************************** External Globals ****************************/

extern MenuHandle		apple_menu;			/* The apple menu */
extern MenuHandle		file_menu;			/* The file menu */
extern MenuHandle		edit_menu;			/* The edit menu */

extern WindowPtr		main_window;		/* the main window */
extern CWindowRecord	main_window_rec;	/* storage for the main window's info */

extern long			main_window_left;	/* left edge of the window (in bytes from left) */
extern long			main_window_top;	/* top edge of the main window */
extern Rect			main_window_bounds;	/* content rect of main window */
extern long			main_window_width;	/* width of main window in bytes */
extern long			main_window_height;	/* height of main window */
extern char			*main_window_base;	/* base address of the window */
extern PixMapHandle	main_window_pixmap;	/* the Pixmap for the main window */
extern Boolean		screen_pixmap_32_flag;/* TRUE if we need 32-bit mode to write to screen */
extern long			screen_width;		/* width of screen in bytes */

extern PicHandle	welcome_to_spion_pict;	/* "Welcome to Spion" picture */
extern Rect			welcome_rect;			/* Rect where "Welcome to Spion is drawn */
extern PicHandle	game_over_pict;			/* "Game Over" picture */
extern Rect			game_over_rect;			/* Rect where "Game Over" is drawn */

extern long			world_width;		/* width of the current world */
extern long			world_height;		/* height of the current world */

extern char			*radar_base;		/* Base address of radar on screen */

extern Fixed		throttle_const;			/* throttle constant */
extern Fixed		friction;				/* air friction constant */
extern Fixed		rotation_const;			/* rotational constant */

extern Rect			radar_rect;

extern enemy_handle	the_ship;				/* our ship, also known as enemies[0] */

extern char			**world_handle, *world;	/* Pointer to world map data */

extern Fixed		h_vector[NUM_SHIPS];	/* h coordinate of a direction */
extern Fixed		v_vector[NUM_SHIPS];	/* v coordinate of a direction */

extern char			turn_left_key;		/* The keys used to control the ship */
extern char			turn_right_key;
extern char			turn_hard_left_key;
extern char			turn_hard_right_key;
extern char			turret_left_key;
extern char			turret_right_key;
extern char			throttle_up_key;
extern char			throttle_down_key;
extern char			throttle_stop_key;
extern char			fire_primary_key;
extern char			shield_key;
extern char			pause_key;
extern char			fire_secondary_key;

extern Boolean		primary_firing;			/* TRUE if the primary weapon is now firing */
extern Boolean		secondary_firing;		/* TRUE if the secondary weapon is now firing */

extern short		num_weapons_locks;		/* TRUE if a weapons lock on US has been detected */
extern short		num_locks_detected;		/* TRUE if our weapon has locked onto an enemy */

extern PicHandle	weapons_lock_on_pict;	/* Pictures for status lights */
extern PicHandle	weapons_lock_off_pict;
extern PicHandle	lock_detected_on_pict;
extern PicHandle	lock_detected_off_pict;

extern long			lives;					/* number of lives remaining */
extern long			score;					/* Current score */
extern long			high_score;				/* Current high score */
extern long			level;					/* Current level */
extern long			crystals_found;			/* Crystals found */
extern long			total_crystals;			/* Total crystals to be found */

extern char		*biplanes[];			/* Array of pointers to biplanes */
extern char		*biplanes_masks[];		/* Array of pointers to biplanes masks */
extern char		*enemy_jets[];			/* Array of pointers to enemy jets */
extern char		*enemy_jets_masks[];	/* Array of pointers to enemy jets masks */
extern char		*flying_wings[];		/* Array of pointers to flying wings */
extern char		*flying_wings_masks[];	/* Array of pointers to flying wings masks */
extern char		*blaster_enemies[];		/* Array of pointers to blaster enemies */
extern char		*blaster_enemies_masks[];/* Array of pointers to blaster enemies masks */
extern char		*tracer_enemies[];		/* Array of pointers to tracer enemies */
extern char		*tracer_enemies_masks[];/* Array of pointers to tracer enemies masks */
extern char		*circle_enemies[];		/* Array of pointers to circle enemies */
extern char		*circle_enemies_masks[];/* Array of pointers to circle enemies masks */
extern char		*laser_guns[];			/* Array of pointers to laser guns */
extern char		*laser_guns_masks[];	/* Array of pointers to laser guns masks */
extern char		*fireballs[];			/* Array of pointers to fireballs */
extern char		*fireballs_masks[];		/* Array of pointers to fireballs masks */
extern char		*guns[];				/* Array of pointers to guns */
extern char		*guns_masks[];			/* Array of pointers to guns masks */
extern char		*cannons[];				/* Array of pointers to cannons */
extern char		*cannons_masks[];		/* Array of pointers to cannons masks */
extern char		*silos[];				/* Array of pointers to silos */
extern char		*silos_masks[];			/* Array of pointers to silos masks */
extern char		*ships[];				/* Array of pointers to ship */
extern char		*ship_masks[];			/* Array of pointers to masks for ship */
extern char		*terrains[];			/* Array of pointers to terrains */
extern char		*tracers_picts[];		/* Array of pointers to tracers */
extern char		*tracers_masks[];		/* Array of pointers to tracers masks */
extern char		*explosions_picts[];	/* Array of pointers to explosions */
extern char		*explosions_masks[];	/* Array of pointers to explosions masks */
extern char		*base_tops[];			/* Array of pointers to tops of bases */
extern char		*base_tops_masks[];		/* Array of pointers to tops of bases masks */
extern char		*base_bottoms[];		/* Array of pointers to bottoms of bases */
extern char		*base_bottoms_masks[];	/* Array of pointers to bottoms of bases masks */
extern char		*dead_bases[];			/* Array of pointers to dead bases */
extern char		*dead_bases_masks[];	/* Array of pointers to dead bases masks */
extern char		*goodies_picts[];		/* Array of pointers to goodies */
extern char		*goodies_masks[];		/* Array of pointers to goodies masks */
extern char		*missiles_picts[];		/* Array of pointers to missiles */
extern char		*missiles_masks[];		/* Array of pointers to missiles masks */
extern char		*bomb_pict;				/* Pointer to the bomb */
extern char		*bomb_mask;				/* Pointer to the bomb mask */
extern char		*large_bomb_pict;		/* Pointer to the large bomb */
extern char		*large_bomb_mask;		/* Pointer to the large bomb mask */
extern char		*shell_pict;			/* Pointer to the shell */
extern char		*shell_mask;			/* Pointer to the shell mask */
extern char		*spiked_ball_pict;		/* Pointer to the spiked ball */
extern char		*spiked_ball_mask;		/* Pointer to the spiked ball mask */

extern CGrafPort	offscreen_view_port;	/* Pointer to the offscreen port */
extern char			*offscreen_bitmap;		/* Pointer to the offscreen bitmap */

extern Boolean		refresh_flag;			/* TRUE when a Vertical Blanking Interval has begun */

extern Rect			vis_world_rect;			/* Part of world which is currently visible */

extern short				num_bases;		/* Number of bases in the world */
extern base_handle			bases[];		/* Bases now in the world */

extern short				num_bullets;	/* Number of bullets now on the screen */
extern bullet_handle		bullets[];		/* Bullets now in the world */

extern short				num_blasters;	/* Number of blasters now in the world */
extern blaster_handle		blasters[];		/* Blasters now in the world */

extern short				num_circles;	/* Number of circles now in the world */
extern circle_handle		circles[];		/* Circles now in the world */

extern short				num_laser_beams;/* Number of laser beams now in the world */
extern laser_beam_handle	laser_beams[];	/* Laser beams now in the world */

extern short				num_bombs;		/* Number of bombs now in the world */
extern bomb_handle			bombs[];		/* Bombs now in the world */

extern short				num_large_bombs;/* Number of large bombs now in the world */
extern large_bomb_handle	large_bombs[];	/* Large bombs now in the world */

extern short				num_explosions;	/* Number of explosions now in the world */
extern explosion_handle		explosions[];	/* Explosions now in the world */

extern short				num_shells;		/* Number of shells now in the world */
extern shell_handle			shells[];		/* Shells now in the world */

extern short				num_tracers;	/* Number of tracers now in the world */
extern tracer_handle 		tracers[];		/* Tracers now in the world */

extern short				num_missiles;	/* Number of missiles now in the world */
extern missile_handle 		missiles[];		/* Missiles now in the world */

extern short				num_enemies;	/* Number of enemies now in the world */
extern short				num_live_enemies; /* Number of enemies now alive */
extern enemy_handle			enemies[];		/* Enemies now in the world */

extern short				num_goodies;	/* Number of goodies now in the world */
extern goodie_handle		goodies[];		/* Goodies now in the world */

extern Handle		explosion_sound;		/* 'snd' resource for explosion sound */
extern Handle		bullet_sound;			/* 'snd' resource for bullet sound */
extern Handle		cannon_sound;			/* 'snd' resource for cannon sound */
extern Handle		crystal_sound;			/* 'snd' resource for crystal sound */
extern Handle		goodie_sound;			/* 'snd' resource for goodie sound */
extern Handle		laser_sound;			/* 'snd' resource for laser sound */
extern Handle		blaster_sound;			/* 'snd' resource for blaster sound */
extern Handle		circle_sound;			/* 'snd' resource for circle sound */
extern Handle		new_level_sound;		/* 'snd' resource for new level sound */
extern Handle		tracer_sound;			/* 'snd' resource for tracer sound */
extern Handle		clunk_sound;			/* 'snd' resource for clunk sound */

extern SndChannelPtr effects_sound_channel;		/* Channel we use for general sound effects */
extern SndChannelPtr explosion_sound_channel;	/* Channel we use for explosion sounds */
extern SndChannelPtr weapons_sound_channel;		/* Channel we use for our weapons sounds */

/* The rectangles containing the various status displays */
extern Rect			throttle_rect;
extern Rect			bank_rect;
extern Rect			fuel_rect;
extern Rect			radar_rect;
extern Point		turret_center;

/* Colors for status displays */
extern RGBColor	danger_color;	/* red */
extern RGBColor	status_color;	/* green */
extern RGBColor	turret_color;	/* white */
extern RGBColor	empty_color;	/* black */

extern RGBColor	wall_color;		/* orange */

extern short	ship_color_word;		/* word to write to radar to represent ship */
extern short	biplane_color_word;		/* word to write to radar to represent biplane */
extern short	cannon_color_word;		/* word to write to radar to represent cannon */
extern short	ground_gun_color_word;	/* word to write to radar to represent ground gun */
extern short	fireball_color_word;	/* word to write to radar to represent explosion */
extern short	silo_color_word;		/* word to write to radar to represent silo */
extern short	missile_color_word;		/* word to write to radar to represent missile */

extern short	weapons[NUM_WEAPON_SLOTS];	/* the weapons we've got now */
extern short	abilities[NUM_WEAPON_SLOTS];/* the abilities we've got now */
extern short	primary_weapon;				/* which is our primary weapon */
extern short	secondary_weapon;			/* which weapon is our secondary */
