/*****
 * spion.c
 *
 *	A game under development
 *
 *
 *****/

#include "spion_resources.h"
#include "spion_types.h"
#include "spion_proto.h"
#include "spion_defines.h"
#include "spion_weapons.h"
#include "spion_externs.h"
#include <FixMath.h>
#include <Profile.h>
#include <stdio.h>


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

long			world_width;		/* width of the current world */
long			world_height;		/* height of the current world */

Boolean			refresh_flag;		/* TRUE when a Vertical Blanking Interval has begun */
Boolean			game_in_progress;	/* TRUE when there is a game going on */
Boolean			game_paused;		/* TRUE when the game is paused */

char			turn_left_key;		/* The keys used to control the ship */
char			turn_right_key;
char			turn_hard_left_key;
char			turn_hard_right_key;
char			stop_turn_key;
char			turret_left_key;
char			turret_right_key;
char			throttle_up_key;
char			throttle_down_key;
char			throttle_stop_key;
char			shield_key;
char			fire_primary_key;
char			pause_key;
char			fire_secondary_key;

PicHandle		welcome_to_spion_pict;	/* "Welcome to Spion" picture */
Rect			welcome_rect;			/* Rect where "Welcome to Spion is drawn */
PicHandle		game_over_pict;			/* "Game Over" picture */
Rect			game_over_rect;			/* Rect where "Game Over" is drawn */

MenuHandle		apple_menu;			/* The apple menu */
MenuHandle		file_menu;			/* The file menu */
MenuHandle		edit_menu;			/* The edit menu */

long			lives;					/* number of lives remaining */
long			score;					/* Current score */
long			high_score;				/* Current high score */
long			level;					/* Current level */
long			crystals_found;			/* Crystals found */
long			total_crystals;			/* Total crystals to be found */

enemy_handle	the_ship;				/* our ship, also known as enemies[0] */

Fixed			throttle_const;			/* throttle constant */
Fixed			friction;				/* air friction constant */
Fixed			rotation_const;			/* rotational constant */
Fixed			h_vector[NUM_DIRECTIONS];/* h coordinate of a direction */
Fixed			v_vector[NUM_DIRECTIONS];	/* v coordinate of a direction */

short			num_enemies;			/* Number of enemies now in the world */
short			num_live_enemies;		/* Number of live enemies now in the world */
enemy_handle	enemies[MAX_ENEMIES];	/* Enemies now in the world */

short			num_goodies;			/* Number of goodies now in the world */
goodie_handle	goodies[MAX_GOODIES];	/* Goodies now in the world */

short			num_bases;				/* Number of bases in the world */
base_handle		bases[MAX_BASES];		/* Bases now in the world */

Rect			vis_world_rect;			/* Part of world which is currently visible */
Rect			ship_rect;				/* rectangle containing ship, world coordinates */

char			**world_handle, *world;	/* Pointer to world map data */

Handle			explosion_sound;		/* 'snd' resource for explosion sound */
Handle			bullet_sound;			/* 'snd' resource for bullet sound */
Handle			cannon_sound;			/* 'snd' resource for cannon sound */
Handle			crystal_sound;			/* 'snd' resource for crystal sound */
Handle			goodie_sound;			/* 'snd' resource for goodie sound */
Handle			laser_sound;			/* 'snd' resource for laser sound */
Handle			blaster_sound;			/* 'snd' resource for blaster sound */
Handle			circle_sound;			/* 'snd' resource for circle sound */
Handle			new_level_sound;		/* 'snd' resource for new level sound */
Handle			tracer_sound;			/* 'snd' resource for tracer sound */
Handle			clunk_sound;			/* 'snd' resource for clunk sound */

SndChannelPtr effects_sound_channel;		/* Channel we use for general sound effects */
SndChannelPtr explosion_sound_channel;		/* Channel we use for explosion sounds */
SndChannelPtr weapons_sound_channel;		/* Channel we use for our weapons sounds */

short			weapons[NUM_WEAPON_SLOTS];	/* the weapons we've got now */
short			abilities[NUM_WEAPON_SLOTS];/* the abilities we've got now */
short			primary_weapon;				/* which is our primary weapon */
short			secondary_weapon;			/* which weapon is our secondary */



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
* procedure handle_update_event                                               *
*                                                                             *
* Purpose: This procedure updates the main window.                            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: August 22, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void handle_update_event(void)
{

	PicHandle	picture;
	Rect		rect;
	short		i;

	/* Begin the update */
	BeginUpdate(main_window);

	/* Draw the controls in the window */
	picture = GetPicture(CONTROLS_PICT);
	BlockMove(&((*picture)->picFrame), &rect, sizeof(Rect));
	OffsetRect(&rect, VIEW_WIDTH + ABILITIES_BAR_WIDTH, WEAPONS_BAR_HEIGHT);
	DrawPicture(picture, &rect);

	/* Draw the status lights */
	draw_status_lights(TRUE);

	/* Draw the abilities bar in the window */
	picture = GetPicture(ABILITIES_BAR_PICT);
	BlockMove(&((*picture)->picFrame), &rect, sizeof(Rect));
	DrawPicture(picture, &rect);

	/* Draw the weapons bar in the window */
	picture = GetPicture(WEAPONS_BAR_PICT);
	BlockMove(&((*picture)->picFrame), &rect, sizeof(Rect));
	OffsetRect(&rect, ABILITIES_BAR_WIDTH, 0);
	DrawPicture(picture, &rect);

	/* Draw the score bar in the window */
	picture = GetPicture(SCORE_BAR_PICT);
	BlockMove(&((*picture)->picFrame), &rect, sizeof(Rect));
	OffsetRect(&rect, ABILITIES_BAR_WIDTH, VIEW_HEIGHT + WEAPONS_BAR_HEIGHT);
	DrawPicture(picture, &rect);

	/* Draw any numbers currently in the score bar */
	update_score_bar(TRUE);

	/* Draw the abilities */
	for (i = 0; i < NUM_ABILITY_SLOTS; i++)
		draw_ability(i);
	
	/* Draw the weapons */
	for (i = 0; i < NUM_WEAPON_SLOTS; i++)
		draw_weapon(i);
	
	/* End the update */
	EndUpdate(main_window);

}	/* handle_update_event() */



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
				
					/* Start a new game */
					init_new_game();

					/* Initialize the first level */
					init_new_level(level);
					
					/* There is now a game in progress */
					game_in_progress = TRUE;
					
					break;

				case QUIT_ITEM:
									
					/* Remove VBL task */
			/*		remove_vbl_task();	*/
				
					/* Stop recognizing key-up events */
					SetEventMask (SysEvtMask);
				
			/*		DumpProfile();		*/
	
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
* procedure create_goodie                                                     *
*                                                                             *
* Purpose: This procedure creates a new goodie and adds it to the list.       *
*                                                                             *
* Parameters: type:      goodie type                                          *
*             h, v:      position of goodie in world coordinates              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 26, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void create_goodie(short type, Fixed h, Fixed v)
{

	register goodie_pointer	new_goodie;

	/* If we already have the max number of goodies, don't do anything */
	if (num_goodies == MAX_GOODIES)
		return;
	
	/* Allocate space for the new goodie */
	goodies[num_goodies] = (goodie_handle) NewHandle(sizeof(goodie));

	/* Dereference the goodie */
	new_goodie = *goodies[num_goodies];

	/* Initialize a new goodie */
	new_goodie->h = h;
	new_goodie->v = v;
	new_goodie->type = type;
		
	/* We now have another goodie */
	num_goodies++;

}	/* create_goodie() */



/*****************************************************************************\
* procedure create_enemy                                                      *
*                                                                             *
* Purpose: This procedure creates a new enemy and adds it to the list.        *
*                                                                             *
* Parameters: type:      enemy type                                           *
*             h, v:      position of enemy in world coordinates               *
*             speed:     speed of enemy                                       *
*             direction: bearing of enemy                                     *
*             launched_from: base this enemy was launched from; -1 if none.   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void create_enemy(short type, Fixed h, Fixed v, Fixed speed, Fixed direction,
					short launched_from)
{

	register enemy_pointer	new_enemy;
	short					hit_points_array[fireball+1] = {SHIP_HP, BIPLANE_HP, GROUND_GUN_HP,
															CANNON_HP, SILO_HP, 0};

	/* If we already have the max number of enemies, don't do anything */
	if (num_enemies == MAX_ENEMIES)
		return;
	
	/* Allocate space for the new enemy */
	enemies[num_enemies] = (enemy_handle) NewHandle(sizeof(enemy));

	/* Dereference the enemy */
	new_enemy = *enemies[num_enemies];

	/* Initialize a new enemy */
	new_enemy->h = h;
	new_enemy->v = v;
	new_enemy->speed = speed;
	new_enemy->direction = direction;
	new_enemy->type = type;
	new_enemy->radar_position = (char *) 0L;
	new_enemy->exploding = 0;
	new_enemy->launching = (launched_from != -1) ? LAUNCH_TIME : 0;
	new_enemy->launched_from = launched_from;
	new_enemy->phased = FALSE;
	new_enemy->phase_mask_lw = 0x00FF00FF;
	new_enemy->phase_mask_word = 0x00FF;
	new_enemy->phase_mask_byte = 0xFF;
	new_enemy->dead = FALSE;
	new_enemy->fire_countdown = 0;
	new_enemy->silo_frame = 0;
	new_enemy->rotation = 0;
	new_enemy->has_rotating_turret = ((type == ship) || (type == ground_gun) ||
										(type == smart_ground_gun) || (type == cannon));
	new_enemy->turret = 0;
	new_enemy->turret_rotation = 0;
	new_enemy->use_air_resistance = (type == ship);
	new_enemy->energy = MAX_ENERGY;
	new_enemy->tracer_lock_enemy = NULL;
	new_enemy->missile_lock_enemy = ((type == silo) ? the_ship : NULL);
	new_enemy->smart_bomb_lock_enemy = NULL;
	
	/* Mark airborn enemies as airborn, unless they are just now being launched */
	if (((type == biplane) || (type == ship)) && (launched_from == -1))
		new_enemy->level = air;
	else
		new_enemy->level = ground;
	
	new_enemy->missile_lock_enemy = (type == silo) ? the_ship : NULL;
	
	/* Set the hit points according to the enemy type */
	new_enemy->hit_points = hit_points_array[type];
	
	/* We now have another enemy */
	num_enemies++;
	num_live_enemies++;

}	/* create_enemy() */



/*****************************************************************************\
* procedure launch_enemy                                                      *
*                                                                             *
* Purpose: This procedure launches a new enemy from a base.                   *
*                                                                             *
* Parameters: which_base: the base to launch from                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void launch_enemy(short which_base)
{

	short direction;

	/* Choose a random direction for the enemy to take at the start */
	direction = ((unsigned short) Random()) * (NUM_DIRECTIONS/65535);

	/* Create the enemy launching at the base's current location */	
	create_enemy(biplane, (*bases[which_base])->h, (*bases[which_base])->h,
					(4L << 16), (direction << 16), which_base);
	
	/* Remember that the base which launched it can't launch again until this
		launch is done */
	(*bases[which_base])->launching = TRUE;
	
} /* launch_enemy() */


/*****************************************************************************\
* procedure create_base                                                       *
*                                                                             *
* Purpose: This procedure creates a new base and adds it to the list.         *
*                                                                             *
* Parameters: type:      base type                                            *
*             h, v:      position of base in world coordinates                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void create_base(short type, Fixed h, Fixed v)
{

	register base_pointer	new_base;

	/* If we already have the max number of bases, don't do anything */
	if (num_bases == MAX_BASES)
		return;
	
	/* Allocate space for the new base */
	bases[num_bases] = (base_handle) NewHandle(sizeof(base));

	/* Dereference the base */
	new_base = *bases[num_bases];

	/* Initialize a new base */
	new_base->h = h;
	new_base->v = v;
	new_base->type = type;
	new_base->radar_position = (char *) 0L;
	new_base->dead = FALSE;
	new_base->launching = FALSE;
	new_base->exploding = FALSE;
	new_base->hit_points = BASE_HIT_POINTS;
	
	/* We now have another base */
	num_bases++;

}	/* create_base() */



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
	char	the_char, the_key;
	Boolean	set_primary_flag;
	short	the_weapon, temp;
	short	digit_keys[11] = {1, 2, 3, 4, 6, 5, 0, 9, 7, 0, 8};

	/* Check for menu shortcut */
	if (event->modifiers & cmdKey)
		{
				
		/* Find menu equivalent */
		menu_result = MenuKey(event->message);
		
		/* Pretend user clicked menu */
		if (HiWord(menu_result) != 0)	/* valid key? */
			do_menu (menu_result);		/* yes, map it to menu item */
		}
	
	/* Handle control keys */
	else
		{
		the_char = event->message & charCodeMask;
		the_key = (event->message & keyCodeMask) >> 8;
		
		/* If the game is paused, allow only un-pause */
		if (game_paused)
			{
			if (the_char == pause_key)
				game_paused = FALSE;

			return;
			}

		/* Check for digit key (switch weapon) */
		if ((the_key >= 0x12) && (the_key <= 0x1C) && (the_key != 0x18) && (the_key != 0x1B))
			{
			
			/* Decide whether it's setting primary or secondary weapon */
			set_primary_flag = !(event->modifiers & optionKey);
			
			/* Find which weapon corresponds to this key */
			the_weapon = digit_keys[the_key - 0x12] - 1;
			
			/* Don't let user set weapon to an unused weapon */
			if (!weapons[the_weapon])
				return;
			
			/* If user is setting something which is already set, ignore it */
			if (((the_weapon == primary_weapon) && (set_primary_flag)) ||
					((the_weapon == secondary_weapon) && (!set_primary_flag)))
				return;
				
			/* If user is changing a primary weapon to secondary, or vice versa, just
				switch the primary and secondaries */
			if ((the_weapon == primary_weapon) || (the_weapon == secondary_weapon))
				{
				
				/* Swap primary and secondary weapons */
				temp = primary_weapon;
				primary_weapon = secondary_weapon;
				secondary_weapon = temp;
				
				/* Update both primary and secondary weapons */
				draw_weapon(primary_weapon);
				draw_weapon(secondary_weapon);
				
				}
			
			/* User is setting a weapon which is neither primary nor secondary-- just
				set the weapon */
			else
				{
				if (set_primary_flag)
					{
					temp = primary_weapon;
					primary_weapon = the_weapon;
					}
				else
					{
					temp = secondary_weapon;
					secondary_weapon = the_weapon;
					}

				/* Redraw both the new weapon and the old weapon */
				draw_weapon(the_weapon);
				draw_weapon(temp);
				
				}
			
			/* Get rid of tracer lock, if any */
			if ((*the_ship)->tracer_lock_enemy)
				{
				(*the_ship)->tracer_lock_enemy = NULL;
				num_weapons_locks--;
				}
				
			/* Get rid of smart bomb lock, if any */
			if ((*the_ship)->smart_bomb_lock_enemy)
				{
				(*the_ship)->smart_bomb_lock_enemy = NULL;
				num_locks_detected--;
				}
				
			return;
				
			}

		if (the_char == pause_key)
			game_paused = TRUE;
			
		if (the_char == turn_left_key)
			(*the_ship)->rotation = -1;

		else if (the_char == turn_right_key)
			(*the_ship)->rotation = 1;
			
		if (the_char == turn_hard_left_key)
			(*the_ship)->rotation = -2;

		else if (the_char == turn_hard_right_key)
			(*the_ship)->rotation = 2;
			
		if (the_char == turret_left_key)
			(*the_ship)->turret_rotation = -2;
			
		else if (the_char == turret_right_key)
			(*the_ship)->turret_rotation = 2;
			
		else if (the_char == throttle_up_key)
			(*the_ship)->throttle++;
			
		else if (the_char == throttle_down_key)
			(*the_ship)->throttle--;
			
		else if (the_char == throttle_stop_key)
			(*the_ship)->throttle = 2;
			
		else if (the_char == fire_primary_key)
			{
			fire_weapon(the_ship, weapons[primary_weapon], 0, FALSE);
			primary_firing = TRUE;
			}
			
		else if (the_char == fire_secondary_key)
			{
			fire_weapon(the_ship, weapons[secondary_weapon], 0, FALSE);
			secondary_firing = TRUE;
			}
			
		else if (the_char == shield_key)
			(*the_ship)->phased = TRUE;
			
		/* Throttle may not exceed MAX_THROTTLE */
		if ((*the_ship)->throttle > MAX_THROTTLE)
			(*the_ship)->throttle = MAX_THROTTLE;
		
		/* No throttles less than 0 allowed */
		if ((*the_ship)->throttle < 0)
			(*the_ship)->throttle = 0;
		
		/* Rotation may not exceed MAX_ROTATION or drop below -MAX_ROTATION */
		if ((*the_ship)->rotation > MAX_ROTATION)
			(*the_ship)->rotation = MAX_ROTATION;
		if ((*the_ship)->rotation < -MAX_ROTATION)
			(*the_ship)->rotation = -MAX_ROTATION;
		
		/* Update the turret display */
		draw_turret();

		/* Update the turret display */
		draw_throttle();

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
		(*the_ship)->turret_rotation = 0;

	/* Stop firing when the user lets up the fire key */		
	else if (the_char == fire_primary_key)
		primary_firing = FALSE;
	else if (the_char == fire_secondary_key)
		secondary_firing = FALSE;
			
	else if (the_char == shield_key)
		(*the_ship)->phased = FALSE;

	else if ((the_char == turn_right_key) || (the_char == turn_left_key) ||
				(the_char == turn_hard_right_key) || (the_char == turn_hard_left_key))
		(*the_ship)->rotation = 0;
		
}	/* handle_key_up() */



/*****************************************************************************\
* procedure explode_enemy                                                     *
*                                                                             *
* Purpose: This procedure starts to explode an enemy.                         *
*                                                                             *
* Parameters: which_enemy: the enemy to explode.                              *
*             ours:        TRUE if we get credit for this enemy's destruction.*
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void explode_enemy(register enemy_pointer which_enemy, Boolean ours)
{

	/* If enemy is already exploding, don't do anything */
	if (which_enemy->exploding)
		return;

	/* Start the enemy exploding */
	which_enemy->exploding = FIREBALL_FRAMES - 1;
	
	/* Give us some points if we did this */
	if (ours)
		{
		switch (which_enemy->type)
			{
			
			case biplane:
				score += BIPLANE_POINTS;
				break;
			
			case ground_gun:
				score += GROUND_GUN_POINTS;
				break;
			
			case smart_ground_gun:
				score += SMART_GROUND_GUN_POINTS;
				break;
			
			case cannon:
				score += CANNON_POINTS;
				break;

			case silo:
				score += SILO_POINTS;
				break;

			}
		}
	
	/* Redraw the score bar to show the new score */
	update_score_bar(FALSE);

	/* Play the explosion sound */
	play_explosion_sound();
	
}	/* explode_enemy() */



/*****************************************************************************\
* procedure explode_base                                                      *
*                                                                             *
* Purpose: This procedure starts to explode a base.                           *
*                                                                             *
* Parameters: which_base:   the base to explode.                              *
*             ours:        TRUE if we get credit for this base's destruction. *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void explode_base(register base_pointer which_base, Boolean ours)
{

	/* If base is already exploding, don't do anything */
	if (which_base->exploding)
		return;

	/* Start the base exploding */
	which_base->exploding = BASE_FRAMES - 1;
	
	/* Give us some points if we did this */
	if (ours)
		{
		switch (which_base->type)
			{
			
			case 0:
				score += BASE_POINTS;
				break;
			
			case 1:
				score += BASE_POINTS;
				break;
			
			case 2:
				score += BASE_POINTS;
				break;

			}
		}
	
	/* Redraw the score bar to show the new score */
	update_score_bar(FALSE);

	/* Play the explosion sound */
	play_explosion_sound();
	
}	/* explode_base() */



/*****************************************************************************\
* procedure play_explosion_sound                                              *
*                                                                             *
* Purpose: This procedure plays the explosion sound.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void play_explosion_sound(void)
{

	play_sound(explosion_sound, explosion_sound_channel);

}	/* play_explosion_sound() */



/*****************************************************************************\
* procedure play_sound                                                        *
*                                                                             *
* Purpose: This procedure plays a sound through the explosion channel.        *
*                                                                             *
* Parameters: sound:   handle to a 'snd ' resource.                           *
*             channel: sound channel to play through.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void play_sound(Handle sound, SndChannelPtr channel)
{

	SndCommand	snd_command;

	/* Stop playing anything which is currently playing on this channel */
	snd_command.cmd = quietCmd;
	snd_command.param1 = 0;
	snd_command.param2 = 0;
	SndDoImmediate(channel, &snd_command);

	/* Play the sound */
	SndPlay(channel, sound, TRUE);

}	/* play_sound() */



/*****************************************************************************\
* procedure update_enemy_rotation                                             *
*                                                                             *
* Purpose: This procedure updates the rotation of the enemy to head towards   *
*          the ship.                                                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 13, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_enemy_rotation(register enemy_handle this_enemy_handle)
{

	Fixed	slope_to_ship;		/* Slope of line connecting this enemy to ship */
	Fixed	vector_to_ship_h;
	Fixed	vector_to_ship_v;
	Fixed	next_frame_h, next_frame_v;
	short	dir;
	short	direction_difference;
	short	ship_distance;
	long	horiz_distance;
	long	vert_distance;
	long	distance_squared;
	long	distance;
	Boolean	in_range;
	register enemy_pointer this_enemy = *this_enemy_handle;
	
	/* If this enemy is launching, it has no rotational control */
	if (this_enemy->launching)
		return;
	
	/* Find the vector from this enemy to the ship */
	vector_to_ship_h = (*the_ship)->h - this_enemy->h;
	vector_to_ship_v = (*the_ship)->v - this_enemy->v;

	/* Find ROUGHLY the distance to the ship */
	ship_distance = ((vector_to_ship_h > 0) ? vector_to_ship_h : -vector_to_ship_h) >> 16;
	ship_distance += ((vector_to_ship_v > 0) ? vector_to_ship_v : -vector_to_ship_v) >> 16;

	/* If this is a silo, and the ship's within range, and we don't already have the
		maximum number of missiles, and we are able to fire another missile now, fire */
	if ((this_enemy->type == silo) &&
			(ship_distance < SILO_FIRING_RANGE) &&
				(num_missiles != MAX_MISSILES) &&
					(!this_enemy->fire_countdown))
		{
	
		/* Start the chain of events which will evenually fire a missile */
		this_enemy->silo_frame = 1;
		this_enemy->fire_countdown = MISSILE_INTERVAL;
		return;
		
		}

	/* If the ship is out of our sight range, don't do anything */
	if (ship_distance > BIPLANE_SIGHT_RANGE)
		return;

	/* Find the direction to go to close in on the ship */
	if (this_enemy->type == smart_ground_gun)
		{
		next_frame_h = (*the_ship)->h +
					((*the_ship)->speed >> 8) * (h_vector[((*the_ship)->direction >> 16)] >> 8);
		next_frame_v = (*the_ship)->v +
					((*the_ship)->speed >> 8) * (v_vector[((*the_ship)->direction >> 16)] >> 8);
		dir = find_intercept_direction((*the_ship)->h, (*the_ship)->v, next_frame_h, next_frame_v,
										this_enemy->h, this_enemy->v, BULLET_SPEED << 16, TRUE,
										&in_range);
		}
	else
		{
		dir = find_intercept_direction((*the_ship)->h, (*the_ship)->v, 0, 0,
										this_enemy->h, this_enemy->v, 0, FALSE, &in_range);
		}
	
	in_range = (ship_distance < GROUND_GUN_FIRING_RANGE);

	/* For biplanes, find the distance between the current direction and the direction towards the ship */
	if (this_enemy->type == biplane)
		direction_difference = dir - (this_enemy->direction >> 16);

	else	/* Otherwise, use the gun turret direction */
		direction_difference = dir - ((this_enemy->direction >> 16) + this_enemy->turret) % NUM_DIRECTIONS;
	
	/* Handle special case crossing over 0/NUM_DIRECTIONS line */
	if (direction_difference < 0)
		direction_difference = NUM_DIRECTIONS + direction_difference;
	if (direction_difference > (NUM_DIRECTIONS >> 1))
		direction_difference = -NUM_DIRECTIONS + direction_difference;

	/* Fire if appropriate */
	switch (this_enemy->type)
		{
		case biplane:
			
			/* If we're facing right at the ship, and it's within range, fire */
			if ((ship_distance < BIPLANE_FIRING_RANGE) &&
				(direction_difference < 7) && (direction_difference > -7))
				{			
				fire_weapon(this_enemy_handle, rapid_fire_gun, 0, FALSE);
				}
				
			break;

		case ground_gun:
		case smart_ground_gun:

			/* If we're facing right at the ship, and it's within range, fire */
			if (in_range && (direction_difference < 7) && (direction_difference > -7))
				{			
				fire_weapon(this_enemy_handle, rapid_fire_gun, 0, FALSE);
				}
				
			break;

		case cannon:
			
			/* If we're facing right at the ship, and it's within range, and we don't
				already have the maximum number of shells, and we are able to fire another
				shell now, fire */
			if ((ship_distance < CANNON_FIRING_RANGE) &&
					((direction_difference < 7) && (direction_difference > -7)) &&
						(num_shells != MAX_SHELLS) &&
							(!this_enemy->fire_countdown))
				{
			
				/* Find the real distance to the ship */
				horiz_distance = ((*enemies[0])->h >> 16) - (this_enemy->h >> 16);
				vert_distance = ((*enemies[0])->v >> 16) - (this_enemy->h >> 16);
				distance_squared = horiz_distance * horiz_distance + vert_distance * vert_distance;
				distance = FracSqrt(distance_squared << 14) >> 22;
		
				/* Fire a shell at the ship */	
				fire_weapon(this_enemy_handle, cannon_gun, distance/SHELL_SPEED, FALSE);
				
				}
				
			break;
			
		}

	/* For air enemies, turn if we should */
	if (this_enemy->level == air)
		{
		if (direction_difference > 3)
			this_enemy->rotation = 16;
	
		else if (direction_difference < -3)
			this_enemy->rotation = -16;
	
		else
			this_enemy->rotation = 0;
		}
	
	else	/* ground enemies-- rotate the gun turret if we should */
		{
		if (direction_difference > 3)
			this_enemy->turret_rotation = (this_enemy->type == cannon) ? 1 : 2;
	
		else if (direction_difference < -3)
			this_enemy->turret_rotation = (this_enemy->type == cannon) ? -1 : -2;
	
		else
			this_enemy->turret_rotation = 0;
		}

	if ((this_enemy->type == ground_gun) || (this_enemy->type == smart_ground_gun))
		{
		this_enemy->turret = dir;
		this_enemy->turret_rotation = 0;
		}

}	/* update_enemy_rotation() */



/*****************************************************************************\
* procedure find_intercept_direction                                          *
*                                                                             *
* Purpose: This procedure plots intercept courses.                            *
*                                                                             *
* Parameters: ax, ay: position of the object to intercept.                    *
*             bx, by: position of object in one time unit.                    *
*             Ex, Ey: position of object which intents to intercept.          *
*             se:     speed of intercepting object (E)                        *
*             smart:  TRUE if we plot the future course of the object to      *
*                     intercept, and head on a true intercept course.  FALSE  *
*                     if we just head straight at the object.                 *
*             in_range: returns TRUE if smart and if object is in range.      *
*             returns direction object E should go to intercept.              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

short find_intercept_direction(Fixed ax, Fixed ay, Fixed bx, Fixed by, Fixed Ex, Fixed Ey,
								Fixed se, Boolean smart, Boolean *in_range)
{

	Fixed	A, B;
	long	C;			/* These are the A, B, and C of the Quadratic Equation */
	long	aim_point_x, aim_point_y;
	long	bx_minus_ax;
	long	by_minus_ay;
	long	ax_minus_Ex;
	long	ay_minus_Ey;
	long	square_root;
	Fixed	ti;
	long	aim_vector_x, aim_vector_y;
	
	/* If this is a smart interception, plot the course and head for where the object
		WILL be, not where it is */
	if (smart)
		{	
		/* Evaluate recurring subexpressions */
		bx_minus_ax = (bx - ax) >> 16;
		by_minus_ay = (by - ay) >> 16;
		ax_minus_Ex = (ax - Ex) >> 16;
		ay_minus_Ey = (ay - Ey) >> 16;
	
		/* Find A (Fixed)*/
		A = ((bx-ax)>>8)*((bx-ax)>>8) + ((by-ay)>>8)*((by-ay)>>8) - (se >> 8)*(se >> 8);
					
		/* Find B (an integer) */
		B = 2*ax_minus_Ex*bx_minus_ax + 2*ay_minus_Ey*by_minus_ay;
	
		/* Find C (an integer) */
		C = ax_minus_Ex*ax_minus_Ex + ay_minus_Ey*ay_minus_Ey;
	
		/* Find ti, the time to intercept.  We use the Quadratic Equation. */
		square_root = FracSqrt((B*B - 4*(A >> 16)*C) << 2) >> 16;
		
		/* Use the solution which is in FRONT of the object to intercept */
		if (!(A >> 16))
			A = 1L << 16;
		/* else	*/
			{
			ti = ((-B + square_root) << 16) / (2*A >> 16);
		
			if (ti < 0)
				ti = ((-B - square_root) << 16) / (2*A >> 16);
			}
				
		/* Find the point to aim at */
		aim_vector_x = ax + ((bx - ax) >> 8)*(ti >> 8) - Ex;
		aim_vector_y = ay + ((by - ay) >> 8)*(ti >> 8) - Ey;
		
		}
	
	else	/* Not a smart interception-- just head for the object */
		{
		aim_vector_x = ax - Ex;
		aim_vector_y = ay - Ey;
		}
	
	return (vector_to_direction(aim_vector_x, aim_vector_y));
	
}	/* find_intercept_direction() */



/*****************************************************************************\
* procedure vector_to_direction                                               *
*                                                                             *
* Purpose: This procedure converts a vector to the closest direction.         *
*                                                                             *
* Parameters: x, y: the vector                                                *
*             returns the direction which most closely matches the vector.    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

short vector_to_direction(long x, long y)
{

	Fixed	vector_slope;
	short	angle;

	/* Find the slope of the vector.  If it is vertical, use "infinite" slope. */
	if (!(y >> 16))
		vector_slope = 0x7FFFFFFF;
	else
		vector_slope = (x) / (y >> 16);

	/* Find the angle from vertical of the vector, in degrees */
	angle = AngleFromSlope(vector_slope);
	if (x < 0) angle = 180 + angle;

	/* Convert this to a direction */
	return (angle * NUM_DIRECTIONS / 360);

}	/* vector_to_direction() */



/*****************************************************************************\
* procedure update_enemy_position                                             *
*                                                                             *
* Purpose: This procedure updates the positions of an enemy, and handles      *
*          speed, rotation, and turret changes.                               *
*                                                                             *
* Parameters: which_enemy: the enemy to update                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_enemy_position(enemy_pointer which_enemy)
{

	Fixed		speed_squared;
	Fixed		new_speed;
	Fixed		decel;
	Fixed		accel;
	char		block_below;

	/* Find the block below this enemy */
	block_below = *(world + ((which_enemy->v >> 16) / BLOCK_HEIGHT) * world_width +
						((which_enemy->h >> 16) / BLOCK_WIDTH));

	/* Check whether this enemy has hit a wall */
	if ((block_below >= 25 ) && (block_below <= 31))
		{

		which_enemy->speed = 1L << 16;

		/* Explode the enemy */
		explode_enemy(which_enemy, FALSE);
		
		}

	/* Move enemy to new position */
	which_enemy->h += (which_enemy->speed >> 8) * (h_vector[(which_enemy->direction >> 16)] >> 8);
	which_enemy->v += (which_enemy->speed >> 8) * (v_vector[(which_enemy->direction >> 16)] >> 8);

	/* If the enemy's speed is computed the long way (with air resistance), do it now.
		Otherwise, don't do anything-- without resistance or throttle acceleration, the
		enemy's speed will stay the same. */
	if (which_enemy->use_air_resistance)
		{

		/* Find the air resistance on the enemy */
		speed_squared = (which_enemy->speed >> 8) * (which_enemy->speed >> 8);
		decel = (speed_squared >> 8) * (friction >> 8);
		new_speed = which_enemy->speed - decel;
		
		/* Find the throttle acceleration on the enemy */
		accel = which_enemy->throttle * which_enemy->throttle * throttle_const;
	
		/* Find new speed due to thrust */
		new_speed += accel;
		
		/* Find new velocity */
		which_enemy->speed = new_speed;

		}

	/* If the enemy is rotating, rotate it according to the speed */
	if (which_enemy->rotation && ((!which_enemy->exploding) || (which_enemy->exploding > 24)))
		{
		which_enemy->direction += which_enemy->rotation * (rotation_const >> 8) * (which_enemy->speed >> 8);

		/* Constrain enemy rotation to number of available directions */
		if (which_enemy->direction < 0)
			which_enemy->direction += (NUM_DIRECTIONS << 16);
		else if (which_enemy->direction >= (NUM_DIRECTIONS << 16))
			which_enemy->direction -= (NUM_DIRECTIONS << 16);
		}

	/* If the enemy's gun turret is rotating, rotate it another notch */
	if (which_enemy->turret_rotation && ((!which_enemy->exploding) || (which_enemy->exploding > 24)))
		{
		which_enemy->turret += which_enemy->turret_rotation;
	
		/* Constrain turret to number of directions */
		if (which_enemy->turret < 0)
			which_enemy->turret += NUM_DIRECTIONS;
		if (which_enemy->turret >= NUM_DIRECTIONS)
			which_enemy->turret -= NUM_DIRECTIONS;
			
		/* If this is us, draw the turret indicator in its new position */
		if (which_enemy->type == ship)
			draw_turret();
		}
	
}	/* update_enemy_position() */



/*****************************************************************************\
* procedure check_enemy_collisions                                            *
*                                                                             *
* Purpose: This procedure checks for collisions between the passed Rect and   *
*          all enemies.  If it detects such a collision, it destroys the      *
*          enemy it found, and returns TRUE.                                  *
*                                                                             *
* Parameters: check_rect: rectangle to check for collisions                   *
*             phased:     phase of the rectangle                              *
*             level:      level of the rectangle                              *
*             start_enemy:first enemy to check (does not check earlier ones)  *
*             ours:       TRUE we get credit for any collisions.              *
*             hit_ours:   is returned TRUE if the enemy struck our ship       *
*             returns TRUE if a collision was found.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 13, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_enemy_collisions(Rect *check_rect, short phased, short level,
								short start_enemy, Boolean ours, Boolean *hit_ours)
{

	short					i;
	register enemy_pointer	this_enemy;
	Rect					this_enemy_rect;
	Rect					intersect_rect;
	short					collisions_found = 0;

	/* Nobody has hit us yet */
	*hit_ours = 0;

	/* Check this enemy for collisions with other enemies */
	for (i = start_enemy; i<num_enemies; i++)
		{
		
		/* Dereference this enemy */
		this_enemy = *enemies[i];
		
		/* If this enemy is in a different phase, or at a different level, ignore it */
		if ((this_enemy->phased != phased) ||
				(this_enemy->level != level))
			continue;
		
		/* Find this enemy's rect */
		this_enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
		this_enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
		this_enemy_rect.bottom = this_enemy_rect.top + SHIP_SIZE;
		this_enemy_rect.right = this_enemy_rect.left + SHIP_SIZE;
		InsetRect (&this_enemy_rect, 2, 2);

		/* If the rectangles intersect, we have a collision */
		if (SectRect(&this_enemy_rect, check_rect, &intersect_rect))
			{
			
			/* Remember if this enemy hit us */
			*hit_ours += (this_enemy->type == ship);
			
			/* Register a hit with this enemy */
			hit_enemy(this_enemy, ENEMY_DAMAGE);
			
			/* remember that we found at least one collision */
			collisions_found++;
			
			}
		
		}	/* loop through enemies */

	/* Return the number of collision found */
	return collisions_found;

}	/* check_enemy_collisions() */



/*****************************************************************************\
* procedure check_bullet_collisions                                           *
*                                                                             *
* Purpose: This procedure checks for collisions between the passed Rect and   *
*          all bullets.  If it detects such a collision, it returns TRUE.     *
*                                                                             *
* Parameters: check_rect: rectangle to check for collisions                   *
*             phased:     phase of the rectangle                              *
*             ours:       is returned TRUE if the bullet was ours.            *
*             returns TRUE if a collision was found.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_bullet_collisions(Rect *check_rect, short phased, Boolean *ours)
{

	short					i;
	register bullet_pointer	this_bullet;
	Point					bullet_point;
	Rect					intersect_rect;
	short					collisions_found = 0;

	/* We have no destruction credit yet */
	*ours = FALSE;

	/* Check this rectangle for collisions with bullets */
	for (i=0; i<num_bullets; i++)
		{
		
		/* Dereference the bullet */
		this_bullet = *bullets[i];
		
		/* If this bullet is in a different phase, ignore it */
		if (this_bullet->phased != phased)
			continue;
		
		/* Find the bullet's location (world coordinates) */
		bullet_point.v = this_bullet->v >> 16; 
		bullet_point.h = this_bullet->h >> 16; 
		
		/* If the bullet is in the rectangle, we have a collisions */
		if (PtInRect (bullet_point, check_rect))
			{
			
			/* Remember if this bullet is ours */
			*ours = this_bullet->ours;

			/* Destroy this bullet */
			destroy_bullet(i);

			return TRUE;
			}
		
		}	/* for bullets */

	/* No collisions found */
	return FALSE;

}	/* check_bullet_collisions() */



/*****************************************************************************\
* procedure check_tracer_collisions                                           *
*                                                                             *
* Purpose: This procedure checks for collisions between the passed Rect and   *
*          all tracers.  If it detects such a collision, it returns TRUE.     *
*                                                                             *
* Parameters: check_rect: rectangle to check for collisions                   *
*             phased:     phase of the rectangle                              *
*             ours:       is returned TRUE if the tracer was ours.            *
*             returns TRUE if a collision was found.                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_tracer_collisions(Rect *check_rect, short phased, Boolean *ours)
{

	short					i;
	register tracer_pointer	this_tracer;
	Point					tracer_point;
	Rect					intersect_rect;
	short					collisions_found = 0;

	/* We have no destruction credit yet */
	*ours = FALSE;

	/* Check this rectangle for collisions with tracers */
	for (i=0; i<num_tracers; i++)
		{
		
		/* Dereference the tracer */
		this_tracer = *tracers[i];
		
		/* If this tracer is in a different phase, ignore it */
		if (this_tracer->phased != phased)
			continue;
		
		/* Find the tracer's location (world coordinates) */
		tracer_point.v = this_tracer->v >> 16; 
		tracer_point.h = this_tracer->h >> 16; 
		
		/* If the tracer is in the rectangle, we have a collision */
		if (PtInRect (tracer_point, check_rect))
			{
			
			/* Remember if this tracer is ours */
			*ours = this_tracer->ours;

			/* Destroy this tracer */
			destroy_tracer(i);

			return TRUE;

			}
		
		}	/* for tracers */

	/* No collisions found */
	return FALSE;

}	/* check_tracer_collisions() */



/*****************************************************************************\
* procedure update_enemies                                                    *
*                                                                             *
* Purpose: This procedure updates the position of the update_enemies, handles *
*          explosions and destructions.                                       *
*                                                                             *
* Parameters: returns TRUE if we are now dead.                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 13, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

Boolean update_enemies(void)
{

	long			i;
	register short	color_word;
	register enemy_pointer	this_enemy;  
	Rect			this_enemy_rect;
	Boolean			hit_us, our_tracer, our_bullet;

#define this_tracer		((tracer_pointer) this_bullet)

	/* Loop through the enemies, updating as we go */
	for (i=0; i<num_enemies; i++)
		{

		/* Dereference this enemy */
		this_enemy = *enemies[i];

		/* If this enemy is dead, ignore it */
		if (this_enemy->dead)
			continue;

		/* If this enemy is exploding, go to the next frame of the explosion */
		if (this_enemy->exploding)
			{
			
			/* Find next frame; if enemy is now totally gone, remove it from list */
			if (!(--this_enemy->exploding))
				{
				
				/* Find the color word to use to erase this enemy from the radar */
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
					
					case silo:
						color_word = silo_color_word;
						break;
					
					case fireball:
						color_word = fireball_color_word;
						break;
					
					}
				
				/* Erase the enemy from the radar */
				asm {
					move.l	this_enemy->radar_position, a0
					eor.w	color_word, (a0)
					add.l	screen_width, a0
					eor.w	color_word, (a0)
					}

				/* If this enemy was a ground enemy, don't get rid of it altogether--
					just mark it as dead (note that enemies launching may be technically
					"ground" enemies, but we consider them air enemies here) */
				if ((this_enemy->level == ground) && (this_enemy->type != fireball) &&
						(!this_enemy->launching))
					{
					
					this_enemy->dead = TRUE;
					this_enemy->exploding = 0;
					
					}
				
				else
				
					{
					
					/* get rid of this enemy completely */
					destroy_enemy(i);
				
					/* If this "enemy" was us, we're dead. */
					if (!i)
						return TRUE;
				
					/* Remember to update the enemy which WAS last */
					i--;
					
					}
				
				/* Whether this is now a "dead" enemy, or it's gone completely, we
					now have one fewer live enemies */
				num_live_enemies--;
				
				continue;
								
				}
			}

		/* If it was just a fireball, we're done */
		if (this_enemy->type == fireball)
			continue;

		/* If this enemy is being launched, go to next launch frame */
		if (this_enemy->launching)
			{
			this_enemy->launching--;

			/* If the launch is over, make this a normal, non-launching enemy */
			if (!this_enemy->launching)
				{
				this_enemy->level = air;
				
				/* The base which launched it may now launch another */
				(*bases[this_enemy->launched_from])->launching = FALSE;
				
				}
			}


		/* If it is a cannon or a silo, decrement the firing countdown if necessary */
		if (((this_enemy->type == cannon) || (this_enemy->type == silo)) &&
				(this_enemy->fire_countdown))
			this_enemy->fire_countdown--;

		/* If it's a silo and it's beginning to fire, go to the next frame */
		if ((this_enemy->type == silo) && (this_enemy->silo_frame))
			{
			
			/* Go to next frame-- if we're done, fire a missile */
			if (++this_enemy->silo_frame == SILO_FRAMES)
				fire_weapon(enemies[i], missile_launcher, 0, FALSE);
			}
			
		/* If this is not us, compute new rotation for this enemy */
		if (this_enemy->type != ship)
			update_enemy_rotation(enemies[i]);

		/* Move this enemy to its new position */
		update_enemy_position(this_enemy);
			
		/* If the enemy is off the edge of the world, destroy it */
		if ((this_enemy->h < 0) || (this_enemy->v < 0) ||
			(this_enemy->h > ((BLOCK_WIDTH*world_width) << 16)) ||
			(this_enemy->v > ((BLOCK_HEIGHT*world_height) << 16)))
				{
				destroy_enemy(i);
				}

		/* If this is an airborn enemy, check for collisions with other airborn enemies */
		if (this_enemy->level == air)
			{

			/* Find the enemy's rectangle */
			this_enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
			this_enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
			this_enemy_rect.bottom = this_enemy_rect.top + SHIP_SIZE;
			this_enemy_rect.right = this_enemy_rect.left + SHIP_SIZE;
			InsetRect (&this_enemy_rect, 2, 2);
			
			/* Check for collisions with bullets */
			if (check_bullet_collisions(&this_enemy_rect, this_enemy->phased, &our_bullet))
				{
				hit_enemy(this_enemy, BULLET_DAMAGE);
				}
			
			/* Check for collisions with enemies */
			else if (check_enemy_collisions(&this_enemy_rect, this_enemy->phased, this_enemy->level,
										i+1, (this_enemy->type == ship), &hit_us))
				{
				hit_enemy(this_enemy, ENEMY_DAMAGE);
				}
			
			/* Check for collisions with tracers */
			else if (check_tracer_collisions(&this_enemy_rect, this_enemy->phased, &our_tracer))
				{
				hit_enemy(this_enemy, TRACER_DAMAGE);
				}
			
			}
					
		}	/* for enemies */

	/* We're still alive */
	return FALSE;

}	/* update_enemies() */



/*****************************************************************************\
* procedure hit_enemy                                                         *
*                                                                             *
* Purpose: This procedure is called when an enemy is hit by something.  It    *
*          deducts hit points and destroys the enemy if it is dead.           *
*                                                                             *
* Parameters: which_enemy: the enemy which was hit.                           *
*             damage:      amount of damage done to enemy.                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void hit_enemy(enemy_pointer which_enemy, short damage)
{
	
	/* Take ofd the hit points */
	which_enemy->hit_points -= damage;
	if (which_enemy->hit_points < 0)
		which_enemy->hit_points = 0;
	
	/* If this enemy is us, redraw the damage indicator */
	if (which_enemy->type == ship)
		draw_damage_indicator();
	
	/* If the hit points are not negative, this enemy is dead */
	if (which_enemy->hit_points == 0)
		{

		/* Start this enemy exploding */
		if (!which_enemy->exploding)
			explode_enemy(which_enemy, TRUE);
		
		}
	
	/* If the enemy is not yet dead, just play the clunk sound */
	else
		play_sound(clunk_sound, effects_sound_channel);
		
}	/* hit_enemy() */



/*****************************************************************************\
* procedure hit_base                                                          *
*                                                                             *
* Purpose: This procedure is called when a base is hit by something.  It      *
*          deducts hit points and destroys the base if it is dead.            *
*                                                                             *
* Parameters: which_base:  the base which was hit.                            *
*             damage:      amount of damage done to base.                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void hit_base(base_pointer which_base, short damage)
{
	
	/* Take of the hit points */
	which_base->hit_points -= damage;
	
	/* If the hit points are not negative, this enemy is dead */
	if (which_base->hit_points < 0)
		{

		/* Start this base exploding */
		if (!which_base->exploding)
			explode_base(which_base, TRUE);
		
		}
		
}	/* hit_base() */



/*****************************************************************************\
* procedure update_bases                                                      *
*                                                                             *
* Purpose: This procedure updates the bases.                                  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void update_bases(void)
{

	long					i;
	Fixed					fireball_h, fireball_v;
	register base_pointer	this_base;  

	/* Loop through the bases, updating as we go */
	for (i=0; i<num_bases; i++)
		{

		/* Dereference this base */
		this_base = *bases[i];

		/* If this base is dead, ignore it */
		if ((this_base->dead) && (!this_base->exploding))
			continue;

		/* Occasionally, launch an enemy from a base */
		if ((!(*bases[i])->launching) && (Random() > 32600))
			launch_enemy(i);

		/* If this base is exploding, go to the next frame of the explosion */
		if (this_base->exploding)
			{
			
			/* Do the next frame of the explosion */
			--this_base->exploding;
			
			/* If the explosion has burned out, we may want to put a goodie or two there */
			if (!this_base->exploding)
				{
				
				/* generate a crystal goodie */
				get_random_point(&fireball_h, &fireball_v);
				create_goodie(crystal_goodie, this_base->h + fireball_h, this_base->v + fireball_v);
				
				/* possibly generate a weapons goodie */
				if (Random() > -50000)
					{
					get_random_point(&fireball_h, &fireball_v);
					create_goodie(weapons_goodie, this_base->h + fireball_h, this_base->v + fireball_v);
					}
				
				/* possibly generate an ability goodie */
				if (Random() > -50000)
					{
					get_random_point(&fireball_h, &fireball_v);
					create_goodie(ability_goodie, this_base->h + fireball_h, this_base->v + fireball_v);
					}
				
				}
			
			/* In the first phase of the explosion, there are a few spatterings
				of small fireballs */
			else if (this_base->exploding > BASE_FRAMES_P1)
				{
				
				if (Random() > 25000)
					{
					
					/* Make a fireball */
					get_random_point(&fireball_h, &fireball_v);
					create_fireball(this_base->h + fireball_h, this_base->v + fireball_v, FALSE);

					/* Sometimes play the explosion sound */
					if (Random() > -10000)
						play_explosion_sound();
	
					}
				
				}
			
			/* Phase 2 increases the number of small explosions, and adds a few large ones */
			else if (this_base->exploding > BASE_FRAMES_P2)
				{
				
				if (Random() > 15000)
					{
					
					/* Make a fireball */
					get_random_point(&fireball_h, &fireball_v);
					create_fireball(this_base->h + fireball_h, this_base->v + fireball_v, FALSE);

					/* Sometimes play the explosion sound */
					if (Random() > -20000)
						play_explosion_sound();
	
					}
				
				if (Random() > 25000)
					{
					
					/* Make an explosion */
					get_random_point(&fireball_h, &fireball_v);
					create_explosion(this_base->h + fireball_h, this_base->v + fireball_v, FALSE);

					/* Play the explosion sound */
					play_explosion_sound();
	
					}
				
				}
			
			/* Phase 3 eliminates small explosions, and makes a few more large ones */
			else if (this_base->exploding > BASE_FRAMES_P3)
				{
			
				this_base->dead = TRUE;
			
				if (Random() > 28000)
					{
					
					/* Make an explosion */
					get_random_point(&fireball_h, &fireball_v);
					create_explosion(this_base->h + fireball_h, this_base->v + fireball_v, FALSE);

					/* Play the explosion sound */
					play_explosion_sound();
	
					}
				
				}
			
			/* Phase 4 has a small spattering of small explosions */
			else
				{
				
				if (Random() > 28000)
					{
					
					/* Make a fireball */
					get_random_point(&fireball_h, &fireball_v);
					create_fireball(this_base->h + fireball_h, this_base->v + fireball_v, FALSE);

					/* Sometimes play the explosion sound */
					if (Random() > -10000)
						play_explosion_sound();
	
					}
				
				}
			
			
			
			}
					
		}	/* for bases */

}	/* update_bases() */



/*****************************************************************************\
* procedure update_goodies                                                    *
*                                                                             *
* Purpose: This procedure updates the goodies.                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void update_goodies(void)
{

	long					i;
	register goodie_pointer	this_goodie;  
	Rect					goodie_rect, ship_rect, intersect_rect;

	/* Loop through the goodies, updating as we go */
	for (i=0; i<num_goodies; i++)
		{

		/* Dereference this goodie */
		this_goodie = *goodies[i];

		/* Find our ship's rect */
		ship_rect.left = ((*enemies[0])->h >> 16) - (SHIP_SIZE >> 1);
		ship_rect.top = ((*enemies[0])->v >> 16) - (SHIP_SIZE >> 1);
		ship_rect.bottom = ship_rect.top + SHIP_SIZE;
		ship_rect.right = ship_rect.left + SHIP_SIZE;

		/* Find the goodie's rect */
		goodie_rect.left = (this_goodie->h >> 16) - (GOODIE_SIZE >> 1);
		goodie_rect.top = (this_goodie->v >> 16) - (GOODIE_SIZE >> 1);
		goodie_rect.bottom = goodie_rect.top + GOODIE_SIZE;
		goodie_rect.right = goodie_rect.left + GOODIE_SIZE;

		/* If the rectangles intersect, we have a collision */
		if (SectRect(&ship_rect, &goodie_rect, &intersect_rect))
			{
			
			/* If this is a crystal, play the crystal sound */
			if (this_goodie->type == crystal_goodie)
				{
				play_sound(crystal_sound, effects_sound_channel);
				}
			
			else
				{
				play_sound(goodie_sound, effects_sound_channel);
				}
			
			/* Get rid of this goodie */
			destroy_goodie(i);
			
			}
					
		}	/* for goodies */

}	/* update_goodies() */



/*****************************************************************************\
* procedure get_random_point                                                  *
*                                                                             *
* Purpose: This procedure generates a random point (h, v) where 0 < h < 128   *
*          and 0 < v < 128.                                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void get_random_point(Fixed *h, Fixed *v)
{

	*h = ((long) Random()) << 7;
	*v = ((long) Random()) << 7;

}	/* get_random_point() */



/*****************************************************************************\
* procedure destroy_enemy                                                     *
*                                                                             *
* Purpose: This procedure gets rid of an enemy.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_enemy(short which_enemy)
{

	/* We now have one fewer enemies */
	num_enemies--;
	
	/* If this was the last on the list, we're done */
	if (num_enemies == which_enemy)
		return;
	
	/* Otherwise, get rid of this enemy and replace its entry in the
		list with that of the last enemy */
	DisposeHandle(enemies[which_enemy]);
	enemies[which_enemy] = enemies[num_enemies];

}	/* destroy_enemy() */



/*****************************************************************************\
* procedure destroy_base                                                      *
*                                                                             *
* Purpose: This procedure gets rid of a base.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_base(short which_base)
{

	/* Get rid of this base */
	DisposeHandle(bases[which_base]);

	/* We now have one fewer bases */
	num_bases--;
	
	/* If this was the last on the list, we're done */
	if (num_bases == which_base)
		return;
	
	/* Otherwise, replace this base's entry in the list with that of the last base */
	bases[which_base] = bases[num_bases];

}	/* destroy_base() */



/*****************************************************************************\
* procedure destroy_goodie                                                    *
*                                                                             *
* Purpose: This procedure gets rid of a goodie.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_goodie(short which_goodie)
{

	/* Get rid of this goodie */
	DisposeHandle(goodies[which_goodie]);

	/* We now have one fewer goodies */
	num_goodies--;
	
	/* If this was the last on the list, we're done */
	if (num_goodies == which_goodie)
		return;
	
	/* Otherwise, replace this goodie's entry in the list with that of the last goodie */
	goodies[which_goodie] = goodies[num_goodies];

}	/* destroy_goodie() */



/*****************************************************************************\
* procedure drain_energy                                                      *
*                                                                             *
* Purpose: This procedure drains some energy from the ship.                   *
*                                                                             *
* Parameters: how_much: amount of energy to drain                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 27, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void drain_energy(long how_much)
{

	/* Drain this energy from the ship's supply */
	(*the_ship)->energy -= how_much;

	/* If the energy is all gone, explode the ship */
	if ((*the_ship)->energy <= 0)
		{
		(*the_ship)->energy = 0;
		explode_enemy(*the_ship, FALSE);
		}

}	/* drain_energy() */



/*****************************************************************************\
* procedure update_frame                                                      *
*                                                                             *
* Purpose: This procedure calculates and draws one frame of the game.         *
*                                                                             *
* Parameters: returns died if we died, or completed if we have completed the  *
*             level.  Otherwise, returns none.                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 6, 1992                                                *
* Modified:                                                                   *
\*****************************************************************************/

short update_frame(void)
{

	long		last_time;
	short		i;
	static short	energy_update_count = 1;

	/* Scroll another notch */
	draw_background();

	/* Draw the bottom halves of the bases */
	draw_bases(ground);

	/* Draw the ground enemies, except the fireballs */
	draw_enemies(ground);

	/* Draw the top halves of the bases */
	draw_bases(air);

	/* Draw the ground level fireballs.  This is done separately because these
		fireballs may be explosions on top of a base, and therefore must
		appear above the top of the base, whereas all other ground enemies
		must appear below the top of the base */
	draw_enemies(ground_fireball);

	/* Draw the goodies */
	if (num_goodies)
		draw_goodies();

	/* Draw the explosions */
	if (num_explosions)
		draw_explosions();

	/* Draw the bullets */
	if (num_bullets)
		draw_bullets();

	/* Draw the blasters */
	if (num_blasters)
		draw_blasters();

	/* Draw the laser beams */
	if (num_laser_beams)
		draw_laser_beams();

	/* Draw the circles */
	if (num_circles)
		draw_circles();

	/* Draw the tracers */
	if (num_tracers)
		draw_tracers();

	/* Draw the bombs */
	if (num_bombs)
		draw_bombs();

	/* Draw the large bombs */
	if (num_large_bombs)
		draw_large_bombs();

	/* Draw the shells */
	if (num_shells)
		draw_shells();

	/* Draw the missiles */
	if (num_missiles)
		draw_missiles();

	/* Draw the enemies */
	draw_enemies(air);

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

	/* update the bases */
	if (num_bases)
		update_bases();

	/* update the goodies */
	if (num_goodies)
		update_goodies();

	/* Update the position (and existence) of bullets */
	if (num_bullets)
		update_bullets();
	
	/* Update the position (and existence) of blasters */
	if (num_blasters)
		update_blasters();
	
	/* Update the laser beams */
	if (num_laser_beams)
		update_laser_beams();
	
	/* Update the circles */
	if (num_circles)
		update_circles();
	
	/* Update the position (and existence) of tracers */
	if (num_tracers)
		update_tracers();
	
	/* Update the position (and existence) of bombs */
	if (num_bombs)
		update_bombs();
	
	/* Update the position (and existence) of large_bombs */
	if (num_large_bombs)
		update_large_bombs();
	
	/* Update the position (and existence) of shells */
	if (num_shells)
		update_shells();
	
	/* Update the position (and existence) of missiles */
	if (num_missiles)
		update_missiles();
	
	/* Update the frame of explosions */
	if (num_explosions)
		update_explosions();
	
	/* Update the positions and status of enemies.  If we just died, return */
	if (update_enemies())
		return died;
	
	/* Update the primary and secondary weapons */
	update_weapon(*the_ship, weapons[primary_weapon]);
	update_weapon(*the_ship, weapons[secondary_weapon]);
	
	/* Decrease our energy supply according to our speed */
	drain_energy ((*the_ship)->speed >> 16);
	
	/* Decrease our energy supply if we are phased */
	if ((*the_ship)->phased)
		drain_energy (PHASE_ENERGY_DRAIN);
	
	/* Every few frames, update our energy thermometer */
	if (!(--energy_update_count))
		{
		draw_energy_status();
		energy_update_count = 20;
		}
	
	/* If all the enemies are dead (only we remain), start a new level */
	if (num_live_enemies == 1)
		return (completed);

	/* If the primary weapon is firing, fire again */
	if (primary_firing)
		fire_weapon(the_ship, weapons[primary_weapon], 0, TRUE);

	/* If the secondary weapon is firing, fire again */
	if (secondary_firing)
		fire_weapon(the_ship, weapons[secondary_weapon], 0, TRUE);

	/* Game not over yet */
	return none;

}	/* update_frame() */



/*****************************************************************************\
* procedure event_loop                                                        *
*                                                                             *
* Purpose: This procedure repeatedly checks for events, and handles them when *
*          they occur.  It does not exit until ExitToShell().                 *
*                                                                             *
* Parameters: returns died if we died or completed if we finished the level.  *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: November 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short event_loop(void)

{

	Boolean		good;
	EventRecord	event;
	short		status;
	Rect		dest_rect;
	long		time_before, total_time;

	/* No game at first */
	game_in_progress = FALSE;

	while (1)
		{
		
		/* If there's an unpaused game in progress, do another frame */
		if ((game_in_progress) && (!game_paused))
			{

			total_time = Ticks - time_before;
			time_before = Ticks;

			status = update_frame();

			/* Check to see if anything interesting happened */
			switch (status)
				{
				case died:		/* We died */
				
					/* Kill off a life */
					lives--;
					
					/* If we're dead for good, the game is over */
					if (lives == -1)
						{
						
						/* Game is over */
						game_in_progress = FALSE;
						
						/* Draw the Game Over picture */
						DrawPicture(game_over_pict, &game_over_rect);	
						
						}
					
					else	/* Still have lives remaining */
						{
						
						/* Update the bar to show new life count */
						update_score_bar(FALSE);

						/* Restart the level */
						init_new_level(level);
						
						}
						
						break;					
				
				case completed:	/* We completed a level */
					
					/* Go to next level */
					level++;
					
					/* Update the level indicator on the score bar */
					update_score_bar(FALSE);
					
					/* Start the next level */
					init_new_level(level);
					
					break;
					
				default:;
				}
			}
			
			
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
/*			case autoKey:	*/
				handle_key_down(&event);
				break;
				
			case updateEvt:
				handle_update_event();
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
	
#if 0
	InitProfile(200,200);
	_profile = 1;
	freopen("profiler report", "w", stdout);
#endif
	
	/* Go to the event loop, where all the action happens */
	event_loop();

}	/* main() */