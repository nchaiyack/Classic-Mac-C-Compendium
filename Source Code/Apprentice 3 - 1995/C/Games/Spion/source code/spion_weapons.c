/*****
 * spion_weapons.c
 *
 *	This segment handles weapons
 *
 *
 *****/

#include "spion_resources.h"
#include "spion_types.h"
#include "spion_proto.h"
#include "spion_weapons.h"
#include "spion_defines.h"
#include "spion_externs.h"
#include <FixMath.h>
#include <Profile.h>
#include <stdio.h>


/**************************** Globals ***************************/

short			num_bullets;			/* Number of bullets now on the screen */
bullet_handle	bullets[MAX_BULLETS];	/* Bullets now on the screen */

short			num_blasters;			/* Number of blasters now on the screen */
blaster_handle	blasters[MAX_BLASTERS];	/* Blasters now on the screen */

short			num_circles;			/* Number of circles now on the screen */
circle_handle	circles[MAX_CIRCLES];	/* Circles now on the screen */

short			num_laser_beams;		/* Number of laser beams now on the screen */
laser_beam_handle laser_beams[MAX_LASER_BEAMS];/* Laser beams now on the screen */

short			num_bombs;				/* Number of bombs now on the screen */
bomb_handle		bombs[MAX_BOMBS];		/* Bombs now on the screen */

short			num_large_bombs;				/* Number of large bombs now on the screen */
large_bomb_handle large_bombs[MAX_LARGE_BOMBS];	/* Large bombs now on the screen */

short			num_explosions;					/* Number of explosions now on the screen */
explosion_handle explosions[MAX_EXPLOSIONS];	/* Explosions now on the screen */

short			num_shells;				/* Number of shells now on the screen */
shell_handle	shells[MAX_SHELLS];		/* Shells now on the screen */

short			num_missiles;			/* Number of missiles now on the screen */
missile_handle	missiles[MAX_MISSILES];	/* Missiles now on the screen */

short			num_tracers;			/* Number of tracers now on the screen */
tracer_handle	tracers[MAX_TRACERS];	/* Tracers now on the screen */

PicHandle		weapons_lock_on_pict;	/* Pictures for status lights */
PicHandle		weapons_lock_off_pict;
PicHandle		lock_detected_on_pict;
PicHandle		lock_detected_off_pict;

Boolean			primary_firing;			/* TRUE if the primary weapon is now firing */
Boolean			secondary_firing;		/* TRUE if the secondary weapon is now firing */

short			num_weapons_locks;		/* TRUE if a weapons lock on us has been detected */
short			num_locks_detected;		/* TRUE if our weapon has locked onto an enemy */




/*****************************************************************************\
* procedure fire_weapon                                                       *
*                                                                             *
* Purpose: This procedure fires a weapon of the specified type.               *
*                                                                             *
* Parameters: source_enemy: the enemy which fired                             *
*             weapon_type:  the type of weapon to fire                        *
*             param:        an additional parameter-- meaning varies.         *
*             repeat:       TRUE if the weapon is now repeating               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 29, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void fire_weapon(enemy_handle source_enemy_handle, short weapon_type, long param, Boolean repeat)
{
	
	static short			blaster_countdown = 1;
	register enemy_pointer	source_enemy = *source_enemy_handle;
	
	/* If the enemy is beyond the halfway into an explosion, it is in no condition to fire */
	if ((source_enemy->exploding) && (source_enemy->exploding < 24)) return;
	
	switch (weapon_type)
		{
		
		case single_fire_gun:
			if (!repeat)
				{
				fire_bullet(source_enemy);
				play_sound(bullet_sound, weapons_sound_channel);
				}
			break;
		
		case rapid_fire_gun:
			{
			fire_bullet(source_enemy);
			play_sound(bullet_sound, weapons_sound_channel);
			}
			
			break;
		
		case tracer_gun:
			if (!repeat)
				{
				fire_tracer(source_enemy);
				play_sound(tracer_sound, weapons_sound_channel);
				}
			break;
		
		case laser_gun:
			if (!repeat)
				play_sound(laser_sound, weapons_sound_channel);
			fire_laser(source_enemy_handle);
			break;
		
		case bomb_dropper:
			if (!repeat)
				drop_bomb(source_enemy, FALSE);
			break;
		
		case large_bomb_dropper:
			if (!repeat)
				drop_large_bomb(source_enemy);
			break;
		
		case smart_bomb_dropper:
			if (!repeat)
				drop_bomb(source_enemy, TRUE);
			break;
		
		case circle_weapon:
			if (!repeat)
				{
				play_sound(circle_sound, weapons_sound_channel);
				fire_circle(source_enemy);
				}
			break;
		
		case cannon_gun:
			play_sound(cannon_sound, weapons_sound_channel);
			if (!repeat)
				fire_shell(source_enemy, param);
			break;
		
		case missile_launcher:
			if (!repeat)
				fire_missile(source_enemy);
			break;
		
		case blaster_weapon:
			if (!repeat)
				play_sound(blaster_sound, weapons_sound_channel);
			if (!(--blaster_countdown))
				{
				fire_blaster(source_enemy);
				blaster_countdown = 2;
				}
			break;
		
		}

}	/* fire_weapon() */



/*****************************************************************************\
* procedure update_bullets                                                    *
*                                                                             *
* Purpose: This procedure updates the position of the bullets, and gets rid   *
*          of them when they expire.                                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 13, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_bullets(void)
{

	register long	i;
	register bullet_pointer	this_bullet;

	/* Loop through the bullets, updating as we go */
	for (i=0; i<num_bullets; i++)
		{
		
		/* Dereference this bullet for speed */
		this_bullet = *bullets[i];
		
		/* Shorten the fuse.  If this bullet's fuse has run out, kill it */
		if (!(--(this_bullet->fuse)))
			{
			
			/* Get rid of this bullet */
			destroy_bullet(i);
			
			/* Remember to update the bullet which WAS last */
			i--;
			
			}
			
		else	/* This is still a live bullet */
			{
			
			/* move it to its new location */
			this_bullet->h += this_bullet->speed_h;
			this_bullet->v += this_bullet->speed_v;
			
			}
		}

}	/* update_bullets() */



/*****************************************************************************\
* procedure update_blasters                                                   *
*                                                                             *
* Purpose: This procedure updates the position of the blasters, and gets rid  *
*          of them when they expire.                                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_blasters(void)
{

	register long				i;
	register blaster_pointer	this_blaster;
	Fixed						midpoint_h, midpoint_v;

	/* Loop through the blasters, updating as we go */
	for (i=0; i<num_blasters; i++)
		{
		
		/* Dereference this blaster for speed */
		this_blaster = *blasters[i];
		
		/* Check for collisions between this blaster and enemies */
		check_line_hits(this_blaster->start_h, this_blaster->start_v,
							this_blaster->end_h, this_blaster->end_v,
							this_blaster->phased, NULL, this_blaster->ours);

		/* Shorten the fuse.  If this blaster's fuse has run out, kill it */
		if (!(--(this_blaster->fuse)))
			{
			
			/* Get rid of this blaster */
			destroy_blaster(i);
			
			/* Remember to update the blaster which WAS last */
			i--;
			
			}
			
		else	/* This is still a live blaster */
			{
			
			/* move it to its new location */
			this_blaster->start_h += this_blaster->speed_h;
			this_blaster->end_h += this_blaster->speed_h;
			this_blaster->start_v += this_blaster->speed_v;
			this_blaster->end_v += this_blaster->speed_v;

			/* Make it longer */
			midpoint_h = (this_blaster->start_h + this_blaster->end_h) >> 1;
			midpoint_v = (this_blaster->start_v + this_blaster->end_v) >> 1;

			this_blaster->start_h = midpoint_h +
									((this_blaster->start_h - midpoint_h) >> 8) * 0x00000120;
			this_blaster->start_v = midpoint_v +
									((this_blaster->start_v - midpoint_v) >> 8) * 0x00000120;
			this_blaster->end_h = midpoint_h +
									((this_blaster->end_h - midpoint_h) >> 8) * 0x00000120;
			this_blaster->end_v = midpoint_v +
									((this_blaster->end_v - midpoint_v) >> 8) * 0x00000120;
			
			}
		}

}	/* update_blasters() */



/*****************************************************************************\
* procedure update_circles                                                    *
*                                                                             *
* Purpose: This procedure expands the circle.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 13, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_circles(void)
{

	long					i, j;
	register circle_pointer	this_circle;
	register enemy_pointer	this_enemy;
	short					horiz_distance, vert_distance;
	long					distance_squared;
	long					distance;
	Rect					circle_rect, enemy_rect, intersect_rect, missile_rect;

#define this_missile	this_enemy

	/* Loop through the circles, updating as we go */
	for (i=0; i<num_circles; i++)
		{
		
		/* Dereference this circle for speed */
		this_circle = *circles[i];
		
		/* Find the rect in which the circle is inscribed */
		circle_rect.left = (this_circle->h >> 16) - this_circle->radius;
		circle_rect.top = (this_circle->v >> 16) - this_circle->radius;
		circle_rect.right = circle_rect.left + (this_circle->radius << 1);
		circle_rect.bottom = circle_rect.top + (this_circle->radius << 1);

		/* Check for collisions with enemies */
		for (j=0; j<num_enemies; j++)
			{
			
			/* Dereference this enemy */
			this_enemy = *enemies[j];
	
			/* If this enemy is already exploding, or it is a ground enemy, or it is in
				a different phase from the circle, or if it is an fireball, ignore it */
			if (this_enemy->exploding ||
					(this_enemy->level == ground) ||
						(this_enemy->phased != this_circle->phased) ||
							(this_enemy->type == fireball) ||
								(this_enemy->dead))
				continue;
			
			/* Find this enemy's rect */
			enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
			enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
			enemy_rect.bottom = enemy_rect.top + SHIP_SIZE;
			enemy_rect.right = enemy_rect.left + SHIP_SIZE;

			/* If the enemy's rectangle does not intersect with the circle's rect,
				it is safe.  We do this to keep from having to do a FracSqrt for every
				enemy. */
			if (!SectRect(&circle_rect, &enemy_rect, &intersect_rect))
				{
				continue;
				}
				
			/* Find the distance from the center of the enemy to the center of the circle */
			horiz_distance = (this_circle->h >> 16) - (this_enemy->h >> 16);
			vert_distance = (this_circle->v >> 16) - (this_enemy->v >> 16);
			distance_squared = horiz_distance * horiz_distance + vert_distance * vert_distance;
			distance = FracSqrt(distance_squared << 14) >> 22;
		
			/* If the enemy is on top of the circle, destroy it. */
			if ((distance < (this_circle->radius + (SHIP_SIZE >> 1))) &&
				(distance > (this_circle->radius - (SHIP_SIZE >> 1))))
				{
				hit_enemy(this_enemy, CIRCLE_DAMAGE);
				}
			
			}	/* loop through enemies */
		
		/* Check for collisions with missiles */
		for (j=0; j<num_missiles; j++)
			{
			
			/* Dereference this missile */
			this_missile = (enemy_pointer) *missiles[j];
	
			/* If this missile is already exploding, or it is in a different phase
				from the circle, ignore it */
			if (this_missile->exploding ||
						(this_missile->phased != this_circle->phased))
				continue;
			
			/* Find this missile's rect */
			missile_rect.left = (((missile_pointer) this_missile)->h >> 16) - (MISSILE_SIZE >> 1);
			missile_rect.top = (((missile_pointer) this_missile)->v >> 16) - (MISSILE_SIZE >> 1);
			missile_rect.bottom = missile_rect.top + MISSILE_SIZE;
			missile_rect.right = missile_rect.left + MISSILE_SIZE;

			/* If the missile's rectangle does not intersect with the circle's rect,
				it is safe.  We do this to keep from having to do a FracSqrt for every
				missile. */
			if (!SectRect(&circle_rect, &missile_rect, &intersect_rect))
				{
				continue;
				}
				
			/* Find the distance from the center of the missile to the center of the circle */
			horiz_distance = (this_circle->h >> 16) - (((missile_pointer) this_missile)->h >> 16);
			vert_distance = (this_circle->v >> 16) - (((missile_pointer) this_missile)->v >> 16);
			distance_squared = horiz_distance * horiz_distance + vert_distance * vert_distance;
			distance = FracSqrt(distance_squared << 14) >> 22;
		
			/* If the missile is on top of the circle, destroy it. */
			if ((distance < (this_circle->radius + (MISSILE_SIZE >> 1))) &&
				(distance > (this_circle->radius - (MISSILE_SIZE >> 1))))
				{
				explode_missile((missile_pointer) this_missile, j, TRUE);
				}
			
			}	/* loop through missiles */
		
		/* Shorten the fuse.  If this circle's fuse has run out, kill it */
		if (!(--(this_circle->fuse)))
			{
			
			/* Get rid of this circle */
			destroy_circle(i);
			
			/* Remember to update the circle which WAS last */
			i--;
			
			}
			
		else	/* This is still a live circle */
			{
				
			/* move it to its new location */
			this_circle->h += this_circle->speed_h;
			this_circle->v += this_circle->speed_v;
			
			/* Make it bigger */
			this_circle->radius += CIRCLE_RADIUS_GROWTH;
			
			}
		}

}	/* update_circles() */



/*****************************************************************************\
* procedure update_laser_beams                                                *
*                                                                             *
* Purpose: This procedure updates a laser beam.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void update_laser_beams(void)
{

	register long	i;
	register laser_beam_pointer	this_laser_beam;

	/* Loop through the laser_beams, updating as we go */
	for (i=0; i<num_laser_beams; i++)
		{
		
		/* Dereference this laser_beams for speed */
		this_laser_beam = *laser_beams[i];
		
		/* Check for collisions between this beam and enemies */
		check_line_hits(this_laser_beam->start_h, this_laser_beam->start_v,
							this_laser_beam->end_h, this_laser_beam->end_v,
							this_laser_beam->phased, this_laser_beam->source_enemy,
							this_laser_beam->ours);

		/* Get rid of this laser_beam */
		destroy_laser_beam(i);
		
		/* Remember to update the bullet which WAS last */
		i--;
		
		}	/* loop through laser beams */

}	/* update_laser_beams() */



/*****************************************************************************\
* procedure destroy_laser_beam                                                *
*                                                                             *
* Purpose: This procedure gets rid of a laser beam.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_laser_beam(short which_laser_beam)
{

	/* We now have one fewer laser_beams */
	num_laser_beams--;
	
	/* If this was the last on the list, we're done */
	if (which_laser_beam == num_laser_beams)
		return;
	
	/* Otherwise, get rid of this laser_beam and replace its entry in the
		list with that of the last laser_beam */
	DisposeHandle(laser_beams[which_laser_beam]);
	laser_beams[which_laser_beam] = laser_beams[num_laser_beams];

}	/* destroy_laser_beam() */



/*****************************************************************************\
* procedure destroy_bullet                                                    *
*                                                                             *
* Purpose: This procedure gets rid of a bullet.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_bullet(short which_bullet)
{

	/* We now have one fewer bullets */
	num_bullets--;
	
	/* If this was the last on the list, we're done */
	if (which_bullet == num_bullets)
		return;
	
	/* Otherwise, get rid of this bullet and replace its entry in the
		list with that of the last bullet */
	DisposeHandle(bullets[which_bullet]);
	bullets[which_bullet] = bullets[num_bullets];

}	/* destroy_bullet() */



/*****************************************************************************\
* procedure destroy_blaster                                                   *
*                                                                             *
* Purpose: This procedure gets rid of a blaster.                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_blaster(short which_blaster)
{

	/* We now have one fewer blasters */
	num_blasters--;
	
	/* If this was the last on the list, we're done */
	if (which_blaster == num_blasters)
		return;
	
	/* Otherwise, get rid of this blaster and replace its entry in the
		list with that of the last blaster */
	DisposeHandle(blasters[which_blaster]);
	blasters[which_blaster] = blasters[num_blasters];

}	/* destroy_blaster() */



/*****************************************************************************\
* procedure destroy_circle                                                    *
*                                                                             *
* Purpose: This procedure gets rid of a circle.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_circle(short which_circle)
{

	/* We now have one fewer circles */
	num_circles--;
	
	/* If this was the last on the list, we're done */
	if (which_circle == num_circles)
		return;
	
	/* Otherwise, get rid of this circle and replace its entry in the
		list with that of the last circle */
	DisposeHandle(circles[which_circle]);
	circles[which_circle] = circles[num_circles];

}	/* destroy_circle() */



/*****************************************************************************\
* procedure update_tracers                                                    *
*                                                                             *
* Purpose: This procedure updates the position of the tracers, and gets rid   *
*          of them when they expire.                                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_tracers(void)
{

	register long	i;
	register tracer_pointer	this_tracer;

	/* Loop through the tracers, updating as we go */
	for (i=0; i<num_tracers; i++)
		{
		
		/* Dereference this tracer for speed */
		this_tracer = *tracers[i];
		
		/* Shorten the fuse.  If this tracer's fuse has run out, kill it */
		if (!(--(this_tracer->fuse)))
			{
			
			/* Get rid of this tracer */
			destroy_tracer(i);
			
			/* Remember to update the tracer which WAS last */
			i--;
			
			}
			
		else	/* This is still a live tracer */
			{
			
			/* Go to next frame */
			this_tracer->frame++;
			this_tracer->frame %= TRACER_FRAMES;
			
			/* Turn it towards its objective */
			update_tracer_direction(this_tracer);

			/* Move it along its directional vector */
			this_tracer->h += h_vector[this_tracer->direction] * TRACER_SPEED +
								this_tracer->base_speed_h;
			this_tracer->v += v_vector[this_tracer->direction] * TRACER_SPEED +
								this_tracer->base_speed_v;
			
			/* Add frictional drag on the base speed */
			this_tracer->base_speed_h = (this_tracer->base_speed_h >> 8) * (0xF000 >> 8);
			this_tracer->base_speed_v = (this_tracer->base_speed_v >> 8) * (0xF000 >> 8);
		
			}
		}

}	/* update_tracers() */



/*****************************************************************************\
* procedure destroy_tracer                                                    *
*                                                                             *
* Purpose: This procedure gets rid of a tracer.                               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_tracer(short which_tracer)
{

	/* We now have one fewer tracers */
	num_tracers--;
	
	/* If this was the last on the list, we're done */
	if (which_tracer == num_tracers)
		return;
	
	/* Otherwise, get rid of this tracer and replace its entry in the
		list with that of the last tracer */
	DisposeHandle(tracers[which_tracer]);
	tracers[which_tracer] = tracers[num_tracers];

}	/* destroy_tracer() */



/*****************************************************************************\
* procedure update_shells                                                     *
*                                                                             *
* Purpose: This procedure updates the position of the shells, and gets        *
*          them when their fuse runs out.                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_shells(void)
{

	register long	i, j;
	register bullet_pointer	new_bullet;
	register shell_pointer	this_shell;

	/* Loop through the shells, updating as we go */
	for (i=0; i<num_shells; i++)
		{
		
		/* Dereference the shell for speed */
		this_shell = *shells[i];
		
		/* Shorten the fuse.  If this shell's fuse has run out, explode it */
		if (!(--this_shell->fuse))
			{

			/* Play the explosion sound */
			play_explosion_sound();
			
			/* Generate eight bullets heading outward from the shell */
			for (j = 0; j < NUM_DIRECTIONS; j += 4)
				{
				/* If we already have the max number of bullets, don't do anything */
				if (num_bullets == MAX_BULLETS)
					continue;

				/* Allocate memory for this bullet */
				bullets[num_bullets] = (bullet_handle) NewHandle(sizeof(bullet));
				
				/* Dereference for speed */
				new_bullet = *bullets[num_bullets];

				/* Find the velocity of a bullet in the turret direction */
				new_bullet->speed_h = BULLET_SPEED * h_vector[j];
				new_bullet->speed_v = BULLET_SPEED * v_vector[j];

				/* Move the bullet away from the ship */
				new_bullet->h = this_shell->h;
				new_bullet->v = this_shell->v;
				
				/* Add the current shell velocity to that of the bullet */
				new_bullet->speed_h += this_shell->speed_h;
				new_bullet->speed_v += this_shell->speed_v;

				/* Start the bullet countdown */
				new_bullet->fuse = SHELL_BULLET_FUSE;		
			
				/* This is not our bullet */
				new_bullet->ours = FALSE;
			
				/* This bullet is phased the shell was */
				new_bullet->phased = this_shell->phased;
				new_bullet->phase_mask = 0xFF00;

				/* We now have one more bullet */
				num_bullets++;
				
				}
			
			/* Get rid of this shell */
			destroy_shell(i);
			
			/* Remember to update the shell which WAS last */
			i--;
			
			}
			
		else	/* This is still a live shell */
			{
			
			/* move it to its new location */
			this_shell->h += this_shell->speed_h;
			this_shell->v += this_shell->speed_v;
			
			}
			
		}	/* loop through shells */

}	/* update_shells() */



/*****************************************************************************\
* procedure destroy_shell                                                     *
*                                                                             *
* Purpose: This procedure gets rid of a shell.                                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_shell(short which_shell)
{

	/* We now have one fewer shells */
	num_shells--;
	
	/* If this was the last on the list, we're done */
	if (num_shells == which_shell)
		return;
	
	/* Otherwise, get rid of this shell and replace its entry in the
		list with that of the last shell */
	DisposeHandle(shells[which_shell]);
	shells[which_shell] = shells[num_shells];

}	/* destroy_shell() */



/*****************************************************************************\
* procedure update_missiles                                                   *
*                                                                             *
* Purpose: This procedure updates the position of the missiles, and destroys  *
*          them when their fuse runs out.                                     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_missiles(void)
{

	register long	i, j;
	register missile_pointer	this_missile;
	register bullet_pointer		this_bullet;
	Rect			missile_rect;
	Rect			ship_rect;
	Rect			intersect_rect;
	char			*radar_position;
	Point			bullet_point;
	Boolean			hit_us;
	Boolean			hit_our_bullet;

	/* Loop through the missiles, updating as we go */
	for (i=0; i<num_missiles; i++)
		{
		
		/* Dereference this missile */
		this_missile = *missiles[i];
		
		/* Find the missile's Rect */
		missile_rect.left = (this_missile->h >> 16) - (MISSILE_SIZE > 1);
		missile_rect.top = (this_missile->v >> 16) - (MISSILE_SIZE > 1);
		missile_rect.bottom = missile_rect.top + MISSILE_SIZE;
		missile_rect.right = missile_rect.left + MISSILE_SIZE;
		InsetRect (&ship_rect, 2, 2);

		if (check_bullet_collisions(&missile_rect, this_missile->phased, &hit_our_bullet) ||
			check_enemy_collisions(&missile_rect, this_missile->phased, air, 0,
									this_missile->ours, &hit_us))
			{							

			/* Explode the missile */
			explode_missile(this_missile, i, hit_our_bullet);
			
			/* Remember to update the missile which WAS last */
			i--;
			
			/* Next missile */
			continue;
			
			}
					
		/* Shorten the fuse.  If this missile's fuse has run out, explode it */
		if (!(--this_missile->fuse))
			{

			/* Explode the missile */
			explode_missile(this_missile, i, FALSE);

			/* Remember to update the missile which WAS last */
			i--;
			
			}
			
		else	/* This is still a live missile */
			{
			
			/* move it to its new location */
			this_missile->h += this_missile->speed_h;
			this_missile->v += this_missile->speed_v;
			
			/* Rotate it to close on ship */
			update_missile_direction(this_missile);
			
			}
		}

}	/* update_missiles() */



/*****************************************************************************\
* procedure destroy_missile                                                   *
*                                                                             *
* Purpose: This procedure gets rid of a missile.                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_missile(short which_missile)
{

	/* We have one fewer locks on us-- turn off the light if appropriate */
	num_locks_detected--;
	draw_status_lights(FALSE);

	/* We now have one fewer missiles */
	num_missiles--;
	
	/* If this was the last on the list, we're done */
	if (num_missiles == which_missile)
		return;
	
	/* Otherwise, get rid of this missile and replace its entry in the
		list with that of the last missile */
	DisposeHandle(missiles[which_missile]);
	missiles[which_missile] = missiles[num_missiles];

}	/* destroy_missile() */



/*****************************************************************************\
* procedure update_explosions                                                 *
*                                                                             *
* Purpose: This procedure updates the explosions.                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_explosions(void)
{

	long						i;
	register explosion_pointer	this_explosion;
	char						*radar_position;

	/* Loop through the explosions, updating as we go */
	for (i=0; i<num_explosions; i++)
		{
		
		/* Dereference this explosion */
		this_explosion = *explosions[i];
		
		/* Go to next frame */
		this_explosion->frame++;
		
		/* If we're done, get rid of this explosion */
		if (this_explosion->frame == FIREBALL_FRAMES)
			{
			
			/* Erase this explosion from the radar */
			radar_position = this_explosion->radar_position;
			asm {
				move.l	radar_position, a0
				move.w	fireball_color_word, d0
				eor.w	d0, (a0)
				add.l	screen_width, a0
				eor.w	d0, (a0)
				}
			
			destroy_explosion(i);

			/* Remember to update the explosion which WAS last */
			i--;
			}
		}

}	/* update_explosions() */



/*****************************************************************************\
* procedure create_explosion                                                  *
*                                                                             *
* Purpose: This procedure creates a new large explosion.                      *
*                                                                             *
* Parameters: h, v:   position of explosion in world coordinates              *
*             phased: true if the explosion is phased                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void create_explosion(Fixed h, Fixed v, Boolean phased)
{

	register explosion_pointer	new_explosion;

	/* If we already have the max number of explosions, don't do anything */
	if (num_explosions == MAX_EXPLOSIONS)
		return;
	
	/* Allocate space for the new explosion */
	explosions[num_explosions] = (explosion_handle) NewHandle(sizeof(explosion));

	/* Dereference the explosion */
	new_explosion = *explosions[num_explosions];

	/* Initialize a new explosion */
	new_explosion->h = h;
	new_explosion->v = v;
	new_explosion->phased = phased;
	new_explosion->frame = 0;
	new_explosion->phase_mask_lw = 0x00FF00FF;
	new_explosion->phase_mask_word = 0x00FF;
	new_explosion->phase_mask_byte = 0xFF;
	new_explosion->radar_position = NULL;
	
	/* We now have another explosion */
	num_explosions++;

}	/* create_explosion() */



/*****************************************************************************\
* procedure destroy_explosion                                                 *
*                                                                             *
* Purpose: This procedure gets rid of an explosion.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_explosion(short which_explosion)
{

	/* We now have one fewer explosions */
	num_explosions--;
	
	/* If this was the last on the list, we're done */
	if (num_explosions == which_explosion)
		return;
	
	/* Otherwise, get rid of this explosion and replace its entry in the
		list with that of the last explosion */
	DisposeHandle(explosions[which_explosion]);
	explosions[which_explosion] = explosions[num_explosions];

}	/* destroy_explosion() */



/*****************************************************************************\
* procedure update_bombs                                                      *
*                                                                             *
* Purpose: This procedure updates the position of the bombs, and explodes     *
*          them when they impact.                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 22, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_bombs(void)
{

	long			i, j;
	register bomb_pointer	this_bomb;
	register enemy_pointer	this_enemy;
	Rect			explosion_rect;
	Rect			enemy_rect;
	Rect			intersect_rect;

#define this_base	this_enemy

	/* Loop through the bombs, updating as we go */
	for (i=0; i<num_bombs; i++)
		{
		
		/* Dereference this bomb */
		this_bomb = *bombs[i];
		
		/* Shorten the fuse.  If this bomb's fuse has run out, explode it */
		if (!(--this_bomb->fuse))
			{
			
			/* Play the explosion sound */
			play_explosion_sound();
			
			/* Dereference this bomb again, in case SndPlay moved it */
			this_bomb = *bombs[i];
		
			/* Create a fireball */
			create_fireball(this_bomb->h, this_bomb->v, this_bomb->phased);

			/* Find this explosion's rect */
			explosion_rect.left = ((*enemies[num_enemies-1])->h >> 16) - (SHIP_SIZE >> 1);
			explosion_rect.top = ((*enemies[num_enemies-1])->v >> 16) - (SHIP_SIZE >> 1);
			explosion_rect.bottom = explosion_rect.top + SHIP_SIZE;
			explosion_rect.right = explosion_rect.left + SHIP_SIZE;

			/* Find all enemies which are hit by the explosion */
			for (j=0; j<num_enemies; j++)
				{
				
				/* Dereference this enemy */
				this_enemy = *enemies[j];
		
				/* If this enemy is already exploding, or it is an air enemy, or it is in
					a different phase from the bullet, or if it is an fireball, ignore it */
				if (this_enemy->exploding ||
						(this_enemy->level == air) ||
							(this_enemy->phased != this_bomb->phased) ||
								(this_enemy->type == fireball) ||
									(this_enemy->dead))
					continue;
				
				/* Find this enemy's rect */
				enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
				enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
				enemy_rect.bottom = enemy_rect.top + SHIP_SIZE;
				enemy_rect.right = enemy_rect.left + SHIP_SIZE;
		
				/* If the enemy's rectangle intersects with the explosion's rect, damage it */
				if (SectRect(&explosion_rect, &enemy_rect, &intersect_rect))
					{
					hit_enemy(this_enemy, BOMB_DAMAGE);
					}
				
				}	/* loop through enemies */
				
			/* Find all bases which are hit by the fireball */
			for (j=0; j<num_bases; j++)
				{
				
				/* Dereference this base */
				this_base = (enemy_pointer) *bases[j];
		
				/* If this base is already exploding, or it is already dead, ignore it */
				if (((base_pointer) this_base)->exploding || (this_base->dead))
					continue;
				
				/* Find this base's rect */
				enemy_rect.left = (((base_pointer) this_base)->h >> 16) - (BASE_SIZE >> 1);
				enemy_rect.top = (((base_pointer) this_base)->v >> 16) - (BASE_SIZE >> 1);
				enemy_rect.bottom = enemy_rect.top + BASE_SIZE;
				enemy_rect.right = enemy_rect.left + BASE_SIZE;
		
				/* If the base's rectangle intersects with the explosion's rect, knock off
					some hit points */
				if (SectRect(&explosion_rect, &enemy_rect, &intersect_rect))
					{
					
					/* Take off appropriate hit points */
					hit_base((base_pointer) this_base, BOMB_DAMAGE);
					
					}
				
				}	/* loop through bases */
				
			/* Get rid of this bomb */
			destroy_bomb(i);

			/* Remember to update the bomb which WAS last */
			i--;

			}	/* loop through bombs */
			
		else	/* This is still a live bomb */
			{
			
			/* If this is a smart bomb, move towards the enemy */
			if (this_bomb->smart)
				{
				
				/* Turn the smart bomb towards the enemy */
				update_smart_bomb_direction(this_bomb);
				
				/* Move towards the enemy */
				this_bomb->h += this_bomb->speed_h + this_bomb->smart_speed_h;
				this_bomb->v += this_bomb->speed_v + this_bomb->smart_speed_v;
				
				}
			
			else	/* just a normal bomb */
				{
				this_bomb->h += this_bomb->speed_h;
				this_bomb->v += this_bomb->speed_v;
				}
				
			}
		}

}	/* update_bombs() */


/*****************************************************************************\
* procedure create_fireball                                                   *
*                                                                             *
* Purpose: This procedure creates a fireball.                                 *
*                                                                             *
* Parameters: where_h: horizontal position of the fireball in the world.      *
*             where_v: vertical position of the fireball in the world.        *                                                                          *
*             phased:  TRUE if the fireball is phased.                        *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: March 25, 1993                                                  *
* Modified:                                                                   *
\*****************************************************************************/

void create_fireball(Fixed where_h, Fixed where_v, Boolean phased)
{

	/* Create an enemy of type fireball at the location */
	create_enemy(fireball, where_h, where_v, 0, 0, -1);
	
	/* Set the fireball frame to the beginning of the sequence */
	(*enemies[num_enemies-1])->exploding = FIREBALL_FRAMES >> 1;
	
	/* Make this fireball phased if desired */
	(*enemies[num_enemies-1])->phased = phased;

}	/* create_fireball() */


/*****************************************************************************\
* procedure destroy_bomb                                                      *
*                                                                             *
* Purpose: This procedure gets rid of a bomb.                                 *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_bomb(short which_bomb)
{

	/* We now have one fewer bombs */
	num_bombs--;
	
	/* If this was the last on the list, we're done */
	if (num_bombs == which_bomb)
		return;
	
	/* Otherwise, get rid of this bomb and replace its entry in the
		list with that of the last bomb */
	DisposeHandle(bombs[which_bomb]);
	bombs[which_bomb] = bombs[num_bombs];

}	/* destroy_bomb() */



/*****************************************************************************\
* procedure update_large_bombs                                                *
*                                                                             *
* Purpose: This procedure updates the position of the large bombs, and        *
*          explodes them when they impact.                                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_large_bombs(void)
{

	long						i, j;
	register large_bomb_pointer	this_large_bomb;
	register enemy_pointer		this_enemy;
	Rect						explosion_rect;
	Rect						enemy_rect;
	Rect						intersect_rect;

#define	this_base	this_enemy

	/* Loop through the large bombs, updating as we go */
	for (i=0; i<num_large_bombs; i++)
		{
		
		/* Dereference this large bomb */
		this_large_bomb = *large_bombs[i];
		
		/* Shorten the fuse.  If this large bomb's fuse has run out, explode it */
		if (!(--this_large_bomb->fuse))
			{
			
			/* Play the explosion sound */
			play_explosion_sound();
			
			/* Dereference this large bomb again, in case SndPlay moved it */
			this_large_bomb = *large_bombs[i];
		
			/* Create the explosion */
			create_explosion(this_large_bomb->h, this_large_bomb->v, this_large_bomb->phased);
			
			/* Find this explosion's rect */
			explosion_rect.left = ((*explosions[num_explosions-1])->h >> 16) - (EXPLOSION_SIZE >> 1);
			explosion_rect.top = ((*explosions[num_explosions-1])->v >> 16) - (EXPLOSION_SIZE >> 1);
			explosion_rect.bottom = explosion_rect.top + EXPLOSION_SIZE;
			explosion_rect.right = explosion_rect.left + EXPLOSION_SIZE;

			/* Find all enemies which are hit by the explosion */
			for (j=0; j<num_enemies; j++)
				{
				
				/* Dereference this enemy */
				this_enemy = *enemies[j];
		
				/* If this enemy is already exploding, or it is an air enemy, or it is in
					a different phase from the bullet, or if it is a fireball, ignore it */
				if (this_enemy->exploding ||
						(this_enemy->level == air) ||
							(this_enemy->phased != this_large_bomb->phased) ||
								(this_enemy->type == fireball) ||
									(this_enemy->dead))
					continue;
				
				/* Find this enemy's rect */
				enemy_rect.left = (this_enemy->h >> 16) - (SHIP_SIZE >> 1);
				enemy_rect.top = (this_enemy->v >> 16) - (SHIP_SIZE >> 1);
				enemy_rect.bottom = enemy_rect.top + SHIP_SIZE;
				enemy_rect.right = enemy_rect.left + SHIP_SIZE;
		
				/* If the enemy's rectangle intersects with the explosion's rect, kill it */
				if (SectRect(&explosion_rect, &enemy_rect, &intersect_rect))
					{
					hit_enemy(this_enemy, LARGE_BOMB_DAMAGE);
					}
				
				}	/* loop through enemies */
				
			/* Find all bases which are hit by the explosion */
			for (j=0; j<num_bases; j++)
				{
				
				/* Dereference this base */
				this_base = (enemy_pointer) *bases[j];
		
				/* If this base is already exploding, or it is already dead, ignore it */
				if (((base_pointer) this_base)->exploding || (this_base->dead))
					continue;
				
				/* Find this base's rect */
				enemy_rect.left = (((base_pointer) this_base)->h >> 16) - (BASE_SIZE >> 1);
				enemy_rect.top = (((base_pointer) this_base)->v >> 16) - (BASE_SIZE >> 1);
				enemy_rect.bottom = enemy_rect.top + BASE_SIZE;
				enemy_rect.right = enemy_rect.left + BASE_SIZE;
		
				/* If the base's rectangle intersects with the explosion's rect, knock off
					some hit points */
				if (SectRect(&explosion_rect, &enemy_rect, &intersect_rect))
					{
					
					/* hit and possibly destroy this base */
					hit_base((base_pointer) this_base, LARGE_BOMB_DAMAGE);
					
					}
				
				}	/* loop through bases */
				
			/* Get rid of this large bomb */
			destroy_large_bomb(i);

			/* Remember to update the large bomb which WAS last */
			i--;

			}	/* loop through large bombs */
			
		else	/* This is still a live large bomb */
			{
			
			/* move it to its new location */
			this_large_bomb->h += this_large_bomb->speed_h;
			this_large_bomb->v += this_large_bomb->speed_v;
			
			}
		}

}	/* update_large_bombs() */



/*****************************************************************************\
* procedure destroy_large_bomb                                                *
*                                                                             *
* Purpose: This procedure gets rid of a large bomb.                           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void destroy_large_bomb(short which_large_bomb)
{

	/* We now have one fewer large bombs */
	num_large_bombs--;
	
	/* If this was the last on the list, we're done */
	if (num_large_bombs == which_large_bomb)
		return;
	
	/* Otherwise, get rid of this large bomb and replace its entry in the
		list with that of the last large bomb */
	DisposeHandle(large_bombs[which_large_bomb]);
	large_bombs[which_large_bomb] = large_bombs[num_large_bombs];

}	/* destroy_large_bomb() */



/*****************************************************************************\
* procedure update_missile_direction                                          *
*                                                                             *
* Purpose: This procedure updates the direction of the missile.               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 24, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_missile_direction(register missile_pointer this_missile)
{

	Fixed	slope_to_lock;		/* Slope of line connecting this enemy to locked enemy */
	Fixed	vector_to_lock_h;
	Fixed	vector_to_lock_v;
	short	lock_angle;
	short	direction_angle;
	short	angle_difference;
	short	lock_distance;
	long	horiz_distance;
	long	vert_distance;
	long	distance_squared;
	long	distance;

	/* Find the vector from this missile to the locked enemy */
	vector_to_lock_h = (*this_missile->enemy_lock)->h - this_missile->h;
	vector_to_lock_v = (*this_missile->enemy_lock)->v - this_missile->v;

	/* Find the slope of the line connecting this missile to the locked enemy */
	if (!(vector_to_lock_v >> 16))
		slope_to_lock = 0x7FFFFFFF;
	else
		slope_to_lock = (vector_to_lock_h) / (vector_to_lock_v >> 16);

	/* Find the angle from vertical of the vector to the locked enemy */
	lock_angle = AngleFromSlope(slope_to_lock);
	if (vector_to_lock_h < 0) lock_angle = 180 + lock_angle;

	/* Find the angle from vertical along which the enemy is now travelling */
	direction_angle = 360L * this_missile->direction / NUM_DIRECTIONS;
	
	/* Find the angular distance between the current direction and the direction
		towards the locked enemy */
	angle_difference = lock_angle - direction_angle;
	
	/* Handle special case crossing over 0/360 line */
	if (angle_difference < 0)
		angle_difference = 360 + angle_difference;
	if (angle_difference > 180)
		angle_difference = -360 + angle_difference;

	/* If we're not quite on track-- turn so that we are */
	if ((angle_difference > 2) || (angle_difference < -2))
		{
		this_missile->direction += angle_difference * NUM_DIRECTIONS / 360;
	
		if (this_missile->direction >= NUM_DIRECTIONS)
			this_missile->direction -= NUM_DIRECTIONS;
		else if (this_missile->direction < 0)
			this_missile->direction += NUM_DIRECTIONS;
		}

	/* If we're right on track, apply thrust */
	this_missile->speed_h += (h_vector[this_missile->direction] >> 8) * MISSILE_ACCEL;
	this_missile->speed_v += (v_vector[this_missile->direction] >> 8) * MISSILE_ACCEL;
	
	/* Add frictional drag */
	this_missile->speed_h = (this_missile->speed_h >> 8) * (0xF000 >> 8);
	this_missile->speed_v = (this_missile->speed_v >> 8) * (0xF000 >> 8);

}	/* update_missile_direction() */



/*****************************************************************************\
* procedure check_tracer_lock                                                 *
*                                                                             *
* Purpose: This procedure checks whether an enemy is within tracer lock       *
*          range.                                                             *
*                                                                             *
* Parameters: check_enemy: enemy to check.                                    *
*             returns TRUE if this enemy is within tracer lock range.         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_tracer_lock(register enemy_pointer source_enemy, register enemy_pointer check_enemy)
{

	Fixed		slope_to_enemy;		/* Slope of line connecting this tracer to enemy */
	Fixed		vector_to_enemy_h;
	Fixed		vector_to_enemy_v;
	Fixed		distance_to_enemy;
	short		enemy_angle;
	short		direction_angle;
	short		angle_difference;
	

	/* Find the vector from the tracer to the locked enemy */
	vector_to_enemy_h = check_enemy->h - source_enemy->h;
	vector_to_enemy_v = check_enemy->v - source_enemy->v;

	/* If the enemy is our of range, there's no lock */
	distance_to_enemy = ((vector_to_enemy_h < 0) ? -vector_to_enemy_h : vector_to_enemy_h) +
						((vector_to_enemy_v < 0) ? -vector_to_enemy_v : vector_to_enemy_v);
	if (distance_to_enemy > (TRACER_LOCK_RANGE << 16))
		return FALSE;

	/* Find the slope of the line connecting the tracer to the locked enemy */
	if (!(vector_to_enemy_v >> 16))
		slope_to_enemy = 0x7FFFFFFF;
	else
		slope_to_enemy = (vector_to_enemy_h) / (vector_to_enemy_v >> 16);

	/* Find the angle from vertical of the vector to the enemy */
	enemy_angle = AngleFromSlope(slope_to_enemy);
	if (vector_to_enemy_h < 0) enemy_angle = 180 + enemy_angle;

	/* Find the angle of the gun turret */
	direction_angle = 360L * (((source_enemy->direction >> 16) + source_enemy->turret) % NUM_DIRECTIONS)
						/ NUM_DIRECTIONS;
	
	/* Find the angular distance between the turret and the vector to the enemy */
	angle_difference = enemy_angle - direction_angle;
	
	/* Handle special case crossing over 0/360 line */
	if (angle_difference < 0)
		angle_difference = 360 + angle_difference;
	if (angle_difference > 180)
		angle_difference = -360 + angle_difference;

	/* If the angular difference is within the angular tracer lock range, we have a lock */
	if ((angle_difference < TRACER_ANGULAR_LOCK_RANGE) &&
			(angle_difference > -TRACER_ANGULAR_LOCK_RANGE))
		return TRUE;
	
	/* There's no lock */
	return FALSE;	

}	/* check_tracer_lock() */



/*****************************************************************************\
* procedure update_tracer_lock                                                *
*                                                                             *
* Purpose: This procedure updates the weapons lock for the tracer gun.        *
*                                                                             *
* Parameters: source_enemy: the enemy with the tracer gun.                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_tracer_lock(enemy_pointer source_enemy)
{

	Fixed		slope_to_enemy;		/* Slope of line connecting this tracer to enemy */
	Fixed		vector_to_enemy_h;
	Fixed		vector_to_enemy_v;
	short		enemy_angle;
	short		i;
	register enemy_pointer	locked_enemy;

	/* If we already have a lock, verify it */
	if (source_enemy->tracer_lock_enemy)
		{
		
		/* If we're still locked, no problem */
		if (check_tracer_lock(source_enemy, *(source_enemy->tracer_lock_enemy)))
			return;
		
		else	/* lock is gone */
			{
			source_enemy->tracer_lock_enemy = NULL;

			/* If this is us, we have one fewer visible locks */
			if (source_enemy->type == ship)
				num_weapons_locks--;

			}
		
		}
	
	/* No lock-- loop through flying enemies looking for one to lock onto */
	for (i = 0; i < num_enemies; i++)
		{
		
		/* Dereference this enemy */
		locked_enemy = *enemies[i];
		
		/* Don't lock onto ourselves */
		if (locked_enemy == source_enemy)
			continue;
		
		/* If this enemy is in a different phase, or is on the ground, or is exploding,
			don't lock to it */
		if ((locked_enemy->phased != (*enemies[0])->phased) || (locked_enemy->level == ground) ||
			(locked_enemy->exploding))
			continue;
		
		if (check_tracer_lock(source_enemy, locked_enemy))
			{
			
			/* Found a lock */
			source_enemy->tracer_lock_enemy = enemies[i];
	
			/* If this is us, we have one fewer visible locks */
			if (source_enemy->type == ship)
				{
				num_weapons_locks++;
				draw_status_lights(FALSE);
				}
				
			return;

			}
		}	
	
	/* No lock */
	source_enemy->tracer_lock_enemy = NULL;
	
	/* If this is us, update the lock light if it changed */
	if (source_enemy->type == ship)
		draw_status_lights(FALSE);

}	/* update_tracer_lock() */



/*****************************************************************************\
* procedure check_smart_bomb_lock                                             *
*                                                                             *
* Purpose: This procedure checks whether an enemy is within smart bomb lock   *
*          range.                                                             *
*                                                                             *
* Parameters: check_enemy: enemy to check.                                    *
*             source_enemy: enemy with the smart bomb dropper.                *
*             returns TRUE if this enemy is within smart bomb lock range.     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_smart_bomb_lock(register enemy_pointer source_enemy, register enemy_pointer check_enemy)
{

	Fixed		dropped_speed_h;
	Fixed		dropped_speed_v;
	Fixed		bomb_landing_point_h;
	Fixed		bomb_landing_point_v;
	short		distance_to_enemy_h;
	short		distance_to_enemy_v;
	long		distance_to_enemy_squared;
	short		distance_to_enemy;
	
	/* Lock on if the bomb will be able to get to the target.  The areas which the
		bomb can reach are those within a square centered at the bomb's detonation
		site, and of "radius" BOMB_FUSE_LENGTH*SMART_BOMB_SPEED */
	
	/* Find the velocity of the bomb if it is dropped right now */
	dropped_speed_h = (h_vector[(source_enemy->direction >> 16)] >> 8) * ((source_enemy->speed - (1L << 16)) >> 8);
	dropped_speed_v = (v_vector[(source_enemy->direction >> 16)] >> 8) * ((source_enemy->speed - (1L << 16)) >> 8);
	
	/* Find where the bomb will land if it is dropped now and never adjusts */
	bomb_landing_point_h = source_enemy->h + dropped_speed_h*BOMB_FUSE_LENGTH;
	bomb_landing_point_v = source_enemy->v + dropped_speed_v*BOMB_FUSE_LENGTH;
	
	/* Find the distance from the enemy to the bomb's normal landing point */
	distance_to_enemy_h = (check_enemy->h - bomb_landing_point_h >> 16);
	distance_to_enemy_v = (check_enemy->v - bomb_landing_point_v >> 16);

	/* Get the absolute values of the distances */
	distance_to_enemy_h = (distance_to_enemy_h < 0) ? -distance_to_enemy_h : distance_to_enemy_h;
	distance_to_enemy_v = (distance_to_enemy_v < 0) ? -distance_to_enemy_v : distance_to_enemy_v;
	
	/* Check if the enemy is outside the square which is bomb can reach.  If it is,
		we have no lock. */
	if ((distance_to_enemy_h > BOMB_FUSE_LENGTH * SMART_BOMB_SPEED / 2) ||
		(distance_to_enemy_v > BOMB_FUSE_LENGTH * SMART_BOMB_SPEED / 2))
		return FALSE;
	else
		return TRUE;

	/* If the enemy is close enough that the bomb could get there before exploding,
		lock on */
	return (distance_to_enemy < BOMB_FUSE_LENGTH * SMART_BOMB_SPEED);
		
}	/* check_smart_bomb_lock() */



/*****************************************************************************\
* procedure update_smart_bomb_lock                                            *
*                                                                             *
* Purpose: This procedure updates the weapons lock for the smart bomb dropper.*
*                                                                             *
* Parameters: source_enemy: the enemy with the smart bomb dropper.            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 30, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_smart_bomb_lock(register enemy_pointer source_enemy)
{

	Fixed		slope_to_enemy;		/* Slope of line connecting the ship to enemy */
	Fixed		vector_to_enemy_h;
	Fixed		vector_to_enemy_v;
	short		enemy_angle;
	short		i;
	register enemy_pointer	locked_enemy;

	/* If we already have a lock, verify it */
	if (source_enemy->smart_bomb_lock_enemy)
		{
		
		/* If we're still locked, no problem */
		if (check_smart_bomb_lock(source_enemy, *source_enemy->smart_bomb_lock_enemy))
			return;
		
		else	/* lock is gone */
			{

			source_enemy->smart_bomb_lock_enemy = NULL;

			/* If this is us, we have one fewer visible locks */
			if (source_enemy == *the_ship)
				num_weapons_locks--;

			}
		
		}
	
	/* No lock-- loop through flying enemies looking for one to lock onto */
	for (i = 0; i < num_enemies; i++)
		{
		
		/* Dereference this enemy */
		locked_enemy = *enemies[i];
		
		/* If this enemy is in a different phase, or is in the air, or is exploding,
			don't lock to it */
		if ((locked_enemy->phased != (*enemies[0])->phased) || (locked_enemy->level == air) ||
			(locked_enemy->exploding))
			continue;
		
		if (check_smart_bomb_lock(source_enemy, locked_enemy))
			{
			
			/* Found a lock */
			source_enemy->smart_bomb_lock_enemy = enemies[i];
	
			/* If this is us, we have one fewer visible locks */
			if (source_enemy == *the_ship)
				{
				num_weapons_locks++;
				draw_status_lights(FALSE);
				}
				
			return;

			}
		}	
	
	/* No lock */
	source_enemy->smart_bomb_lock_enemy = NULL;
	
	/* If this is us, update the lock light if it changed */
	if (source_enemy == *the_ship)
		draw_status_lights(FALSE);

}	/* update_smart_bomb_lock() */



/*****************************************************************************\
* procedure update_weapon                                                     *
*                                                                             *
* Purpose: This procedure handles any updating required by this weapon.       *
*                                                                             *
* Parameters: source_enemy: the enemy with the weapon.                        *
*             weapon_type: the weapon type.                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_weapon(enemy_pointer source_enemy, short weapon_type)
{

	switch (weapon_type)
		{
		
		case tracer_gun:
			update_tracer_lock(source_enemy);
			break;
		
		case smart_bomb_dropper:
			update_smart_bomb_lock(source_enemy);
			break;
		
		default:;
		
		}
		
}	/* update_weapon() */



/*****************************************************************************\
* procedure update_tracer_direction                                           *
*                                                                             *
* Purpose: This procedure updates the direction of the tracer.                *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_tracer_direction(register tracer_pointer this_tracer)
{

	Fixed		slope_to_enemy;		/* Slope of line connecting this tracer to enemy */
	Fixed		vector_to_enemy_h;
	Fixed		vector_to_enemy_v;
	short		enemy_angle;
	register enemy_pointer	locked_enemy;

	/* If there is no lock, don't change direction */
	if (!this_tracer->locked_enemy)
		return;

	/* Find the enemy this tracer is locked on to */
	locked_enemy = *(this_tracer->locked_enemy);

	/* Find the vector from this tracer to the enemy */
	vector_to_enemy_h = locked_enemy->h - this_tracer->h;
	vector_to_enemy_v = locked_enemy->v - this_tracer->v;

	/* Find the slope of the line connecting this tracer to the enemy */
	if (!(vector_to_enemy_v >> 16))
		slope_to_enemy = 0x7FFFFFFF;
	else
		slope_to_enemy = (vector_to_enemy_h) / (vector_to_enemy_v >> 16);

	/* Find the angle from vertical of the vector to the enemy */
	enemy_angle = AngleFromSlope(slope_to_enemy);
	if (vector_to_enemy_h < 0) enemy_angle = 180 + enemy_angle;

	/* Turn towards the enemy */
	this_tracer->direction = enemy_angle * NUM_DIRECTIONS / 360;

}	/* update_tracer_direction() */



/*****************************************************************************\
* procedure update_smart_bomb_direction                                       *
*                                                                             *
* Purpose: This procedure updates the direction of the smart bomb.            *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 28, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void update_smart_bomb_direction(register bomb_pointer this_bomb)
{

	Fixed		slope_to_enemy;		/* Slope of line connecting this bomb to enemy */
	Fixed		vector_to_enemy_h;
	Fixed		vector_to_enemy_v;
	Fixed		impact_enemy_h;
	Fixed		impact_enemy_v;
	Fixed		bomb_speed_h, bomb_speed_v;
	short		enemy_angle;
	register enemy_pointer	locked_enemy;

	/* Find the enemy this bomb is locked on to */
	locked_enemy = *(this_bomb->smart_lock);

	/* Figure out where the enemy will be when the bomb impacts, in BOMB coordinates.
		In other words, assume the bomb is stationary, and pretend the enemy is moving. */
	impact_enemy_h = locked_enemy->h - (this_bomb->speed_h * this_bomb->fuse);
	impact_enemy_v = locked_enemy->v - (this_bomb->speed_v * this_bomb->fuse);
	
	/* Find the vector from this smart bomb to the enemy's future position */
	vector_to_enemy_h = impact_enemy_h - this_bomb->h;
	vector_to_enemy_v = impact_enemy_v - this_bomb->v;

	/* Find the speed we have to go to get there in time */
	bomb_speed_h = vector_to_enemy_h / this_bomb->fuse;
	bomb_speed_v = vector_to_enemy_v / this_bomb->fuse;
	
	/* We can't go faster than SMART_BOMB_SPEED */
	if (bomb_speed_h > (SMART_BOMB_SPEED << 16))
		bomb_speed_h = (SMART_BOMB_SPEED << 16);
	if (bomb_speed_v > (SMART_BOMB_SPEED << 16))
		bomb_speed_v = (SMART_BOMB_SPEED << 16);
	
	/* Set the bomb's "smart" speed to this speed */
	this_bomb->smart_speed_h = bomb_speed_h;
	this_bomb->smart_speed_v = bomb_speed_v;

}	/* update_smart_bomb_direction() */



/*****************************************************************************\
* procedure drop_bomb                                                         *
*                                                                             *
* Purpose: This procedure drops a bomb.                                       *
*                                                                             *
* Parameters: smart: TRUE if this bomb is a smart bomb.                       *
*             which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 22, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void drop_bomb(enemy_pointer which_enemy, Boolean smart)
{

	register bomb_pointer	new_bomb;   

	/* If we already have the max number of bullets, don't do anything */
	if (num_bombs == MAX_BOMBS)
		return;
	
	/* Allocate memory for this new bomb */
	bombs[num_bombs] = (bomb_handle) NewHandle(sizeof(bomb));
	
	/* Dereference the new bomb */
	new_bomb = *bombs[num_bombs];
	
	/* Find the position of the bomb (same as that of ship) */
	new_bomb->h = which_enemy->h;
	new_bomb->v = which_enemy->v;

	/* Find the velocity of the bomb */
	new_bomb->speed_h = (h_vector[(which_enemy->direction >> 16)] >> 8) * ((which_enemy->speed - (1L << 16)) >> 8);
	new_bomb->speed_v = (v_vector[(which_enemy->direction >> 16)] >> 8) * ((which_enemy->speed - (1L << 16)) >> 8);

	/* Start the bomb countdown (time until impact) */
	new_bomb->fuse = BOMB_FUSE_LENGTH;		

	/* This bomb is phased if the enemy is */
	new_bomb->phased = which_enemy->phased;
	new_bomb->phase_mask = 0xFF00FF00;

	/* If this bomb is smart, and the smart bomb dropper has locked on to a ground
		anamy, initialize it as a smart bomb */
	if (smart && which_enemy->smart_bomb_lock_enemy)
		{
	
		/* This bomb is smart */
		new_bomb->smart = TRUE;
		
		/* This bomb is not moving intelligently yet */
		new_bomb->smart_speed_h = 0;
		new_bomb->smart_speed_v = 0;
		
		/* This bomb is locked onto the smart lock enemy */
		new_bomb->smart_lock = which_enemy->smart_bomb_lock_enemy;
	
		}

	else
		{
		
		/* Not a smart bomb */
		new_bomb->smart = FALSE;
		
		}

	/* We now have a new bomb */
	num_bombs++;

}	/* drop_bomb() */



/*****************************************************************************\
* procedure drop_large_bomb                                                   *
*                                                                             *
* Purpose: This procedure drops a large bomb.                                 *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 22, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void drop_large_bomb(enemy_pointer which_enemy)
{

	register large_bomb_pointer	new_large_bomb;   
	short						large_bomb_direction;

	/* If we already have the max number of large bombs, don't do anything */
	if (num_large_bombs == MAX_LARGE_BOMBS)
		return;
	
	/* Allocate memory for this new large bomb */
	large_bombs[num_large_bombs] = (large_bomb_handle) NewHandle(sizeof(large_bomb));
	
	/* Dereference the new large bomb */
	new_large_bomb = *large_bombs[num_large_bombs];
	
	/* Find the direction this large bomb will be travelling */
	large_bomb_direction = (which_enemy->direction >> 16);
	
	/* Find the position of the large bomb (same as that of enemy) */
	new_large_bomb->h = which_enemy->h;
	new_large_bomb->v = which_enemy->v;

	/* Find the velocity of the bomb */
	new_large_bomb->speed_h = (h_vector[(which_enemy->direction >> 16)] >> 8) * ((which_enemy->speed - (1L << 16)) >> 8);
	new_large_bomb->speed_v = (v_vector[(which_enemy->direction >> 16)] >> 8) * ((which_enemy->speed - (1L << 16)) >> 8);

	/* Start the bomb countdown (time until impact) */
	new_large_bomb->fuse = BOMB_FUSE_LENGTH;		

	/* This large bomb is phased if the enemy is */
	new_large_bomb->phased = which_enemy->phased;
	new_large_bomb->phase_mask = 0xFF00FF00;

	/* We now have a new large bomb */
	num_large_bombs++;

}	/* drop_large_bomb() */



/*****************************************************************************\
* procedure fire_bullet                                                       *
*                                                                             *
* Purpose: This procedure fires another bullet.                               *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 12, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void fire_bullet(enemy_pointer which_enemy)
{

	register bullet_pointer	new_bullet;   
	short			bullet_direction;

	/* If we already have the max number of bullets, don't do anything */
	if (num_bullets == MAX_BULLETS)
		return;
	
	/* If it's us firing, deduct some energy for this bullet */
	if (which_enemy->type == ship)
		drain_energy(BULLET_ENERGY_DRAIN);
	
	/* Allocate memory for this new bullet */
	bullets[num_bullets] = (bullet_handle) NewHandle(sizeof(bullet));
	
	/* Dereference the new bullet */
	new_bullet = *bullets[num_bullets];
		
	/* Find the direction this bullet will be travelling */
	bullet_direction = ((which_enemy->direction >> 16) + which_enemy->turret) % NUM_DIRECTIONS;
	
	/* Find the velocity of a bullet in the turret direction */
	new_bullet->speed_h = BULLET_SPEED * h_vector[bullet_direction];
	new_bullet->speed_v = BULLET_SPEED * v_vector[bullet_direction];

	/* Move the bullet away from the enemy */
	new_bullet->h = which_enemy->h + h_vector[bullet_direction] * 25L - (1L << 16);
	new_bullet->v = which_enemy->v + v_vector[bullet_direction] * 25L - (1L << 16);
	
	/* Add the current enemy velocity to that of the bullet */
	new_bullet->speed_h += (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_bullet->speed_v += (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);

	/* Start the bullet countdown */
	new_bullet->fuse = BULLET_FUSE_LENGTH;		

	/* This is our bullet if this enemy is us */
	if (which_enemy->type == ship)
		new_bullet->ours = TRUE;

	/* This bullet is phased if enemy is */
	new_bullet->phased = which_enemy->phased;
	new_bullet->phase_mask = 0xFF00;

	/* We now have a new bullet */
	num_bullets++;

}	/* fire_bullet() */
	


/*****************************************************************************\
* procedure fire_blaster                                                      *
*                                                                             *
* Purpose: This procedure fires another blaster.                              *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void fire_blaster(enemy_pointer which_enemy)
{

	register blaster_pointer	new_blaster;   
	short						blaster_direction;
	short						perp_direction;
	Fixed						blaster_midpoint_v, blaster_midpoint_h;

	/* If we already have the max number of blasters, don't do anything */
	if (num_blasters == MAX_BLASTERS)
		return;
	
	/* If it's us firing, deduct some energy for this blaster */
	if (which_enemy->type == ship)
		drain_energy(BLASTER_ENERGY_DRAIN);
	
	/* Allocate memory for this new blaster */
	blasters[num_blasters] = (blaster_handle) NewHandle(sizeof(blaster));
	
	/* Dereference the new blaster */
	new_blaster = *blasters[num_blasters];
		
	/* Find the direction this blaster will be travelling */
	blaster_direction = ((which_enemy->direction >> 16) + which_enemy->turret) % NUM_DIRECTIONS;
	
	/* Find the velocity of a blaster in the turret direction */
	new_blaster->speed_h = BLASTER_SPEED * h_vector[blaster_direction];
	new_blaster->speed_v = BLASTER_SPEED * v_vector[blaster_direction];

	/* Find the midpoint of the new blaster */
	blaster_midpoint_h = which_enemy->h + h_vector[blaster_direction] * 30L - (1L << 16);
	blaster_midpoint_v = which_enemy->v + v_vector[blaster_direction] * 30L - (1L << 16);

	/* Find this blaster's line segment */
	perp_direction = (blaster_direction + (NUM_DIRECTIONS >> 2)) % NUM_DIRECTIONS;
	new_blaster->start_h = blaster_midpoint_h + h_vector[perp_direction] * INITIAL_BLASTER_LENGTH;
	new_blaster->start_v = blaster_midpoint_v + v_vector[perp_direction] * INITIAL_BLASTER_LENGTH;
	new_blaster->end_h = blaster_midpoint_h - h_vector[perp_direction] * INITIAL_BLASTER_LENGTH;
	new_blaster->end_v = blaster_midpoint_v - v_vector[perp_direction] * INITIAL_BLASTER_LENGTH;

	/* Add the current ship velocity to that of the blaster */
	new_blaster->speed_h += (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_blaster->speed_v += (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);

	/* Start the blaster countdown */
	new_blaster->fuse = BLASTER_FUSE_LENGTH;		

	/* This is our blaster */
	new_blaster->ours = TRUE;

	/* This blaster is phased if we are */
	new_blaster->phased = which_enemy->phased;

	/* We now have a new blaster */
	num_blasters++;

}	/* fire_blaster() */
	


/*****************************************************************************\
* procedure fire_laser                                                        *
*                                                                             *
* Purpose: This procedure fires the laser.                                    *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void fire_laser(enemy_handle which_enemy_handle)
{

	short					laser_direction;
	laser_beam_pointer		new_laser_beam;
	register enemy_pointer	which_enemy = *which_enemy_handle;
	
	/* If we already have the max number of laser beams, don't do anything */
	if (num_laser_beams == MAX_LASER_BEAMS)
		return;
	
	/* If it's us firing, deduct some energy for this laser */
	if (which_enemy->type == ship)
		drain_energy(LASER_ENERGY_DRAIN);
	
	/* Allocate memory for this new laser beam */
	laser_beams[num_laser_beams] = (laser_beam_handle) NewHandle(sizeof(laser_beam));
	
	/* Dereference the new laser_beam */
	new_laser_beam = *laser_beams[num_laser_beams];
		
	/* Find the direction the laser will be fired */
	laser_direction = ((which_enemy->direction >> 16) + which_enemy->turret) % NUM_DIRECTIONS;

	/* Find the line segment of the laser */
	new_laser_beam->start_h = which_enemy->h + 16 * h_vector[laser_direction];
	new_laser_beam->start_v = which_enemy->v + 16 * v_vector[laser_direction];
	new_laser_beam->end_h = which_enemy->h + LASER_LENGTH * h_vector[laser_direction];
	new_laser_beam->end_v = which_enemy->v + LASER_LENGTH * v_vector[laser_direction];
	
	/* This laser beam is phased if we are */
	new_laser_beam->phased = which_enemy->phased;

	/* Remember which enemy fired */
	new_laser_beam->source_enemy = which_enemy_handle;

	/* We now have one more laser beam */
	num_laser_beams++;

}	/* fire_laser_beam() */
	


/*****************************************************************************\
* procedure fire_circle                                                       *
*                                                                             *
* Purpose: This procedure fires the circle weapon.                            *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 2, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void fire_circle(enemy_pointer which_enemy)
{

	short				laser_direction;
	circle_pointer		new_circle;
	
	/* If we already have the max number of circles, don't do anything */
	if (num_circles == MAX_CIRCLES)
		return;
	
	/* If it's us firing, deduct some energy for this circle */
	if (which_enemy->type == ship)
		drain_energy(CIRCLE_ENERGY_DRAIN);
	
	/* Allocate memory for this new circle */
	circles[num_circles] = (circle_handle) NewHandle(sizeof(circle));
	
	/* Dereference the new circle */
	new_circle = *circles[num_circles];
		
	/* Find the center of the circle */
	new_circle->h = which_enemy->h;
	new_circle->v = which_enemy->v;

	/* Find the velocity of the circle */
	new_circle->speed_h = (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_circle->speed_v = (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	
	/* Start with small circle */
	new_circle->radius = 50;
	
	/* Start the circle countdown */
	new_circle->fuse = CIRCLE_FUSE_LENGTH;		
	
	/* This circle is phased if we are */
	new_circle->phased = which_enemy->phased;
	new_circle->phase_mask_lw = 0x00FF00FF;
	new_circle->phase_mask_word = 0x00FF;
	new_circle->phase_mask_byte = 0xFF;

	/* We now have one more circle */
	num_circles++;

}	/* fire_circle() */
	


/*****************************************************************************\
* procedure check_line_hits                                                   *
*                                                                             *
* Purpose: This procedure checks a line segment and damages any enemies       *
*          which it hits.                                                     *
*                                                                             *
* Parameters: x1, y1, x2, y2: endpoints of the line segment                   *
*             line_phased:    TRUE if the line segment is phased.             *
*             exempt_enemy:   An enemy which cannot collide with this segment *
*                             (typically, the one which fire the weapon).     *
*             ours:           TRUE if we get credit for any destructions.     *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1993                                                 *
* Modified:                                                                   *
\*****************************************************************************/

void check_line_hits(Fixed x1, Fixed y1, Fixed x2, Fixed y2, Boolean line_phased,
						enemy_handle exempt_enemy, Boolean ours)
{

	register enemy_pointer		this_enemy;
	Fixed					enemy_rect_left, enemy_rect_right, enemy_rect_top, enemy_rect_bottom;
	short					i;
	Rect					ship_rect;

#define this_missile	this_enemy

	/* A collision occurs when the laser beam line segment intersects the
		enemy's rectangle.  We check this by looking for intersections between
		the laser beam line segment and the two diagonals of the enemy's rect. */
	
	/* Loop through the enemies */
	for (i=0; i<num_enemies; i++)
		{

		/* If this is the exempt enemy, ignore it */
		if (enemies[i] == exempt_enemy)
			continue;

		/* Dereference this enemy */
		this_enemy = *enemies[i];
		
		/* If this enemy is already exploding, or it is an ground enemy, or it is in
			a different phase from the laser beam, or if it is an fireball, ignore it */
		if (this_enemy->exploding ||
				(this_enemy->level == ground) ||
					(this_enemy->phased != line_phased) ||
						(this_enemy->type == fireball) ||
							(this_enemy->dead))
			continue;
		
		/* Find this enemy's rect */
		enemy_rect_left = this_enemy->h - (SHIP_SIZE << 15);
		enemy_rect_top = this_enemy->v - (SHIP_SIZE << 15);
		enemy_rect_bottom = enemy_rect_top + (SHIP_SIZE << 16);
		enemy_rect_right = enemy_rect_left + (SHIP_SIZE << 16);

		/* Check for collision with this enemy */
		if (check_segment_intersection(x1, y1, x2, y2,
										enemy_rect_left, enemy_rect_bottom,
										enemy_rect_right, enemy_rect_top)
										
								||
			
			check_segment_intersection(x1, y1, x2, y2,
										enemy_rect_left, enemy_rect_top,
										enemy_rect_right, enemy_rect_bottom))
			{
			
			hit_enemy(this_enemy, LINE_DAMAGE);
			
			}
		
		}	/* loop through enemies */

	/* Loop through the missiles */
	for (i=0; i<num_missiles; i++)
		{

		/* Dereference this missile */
		this_missile = (enemy_pointer) *missiles[i];
		
		/* If this missile is in a different phase from the line, ignore it */
		if (this_missile->phased != line_phased)
			continue;
		
		/* Find this missile's rect (we use enemy_rect here) */
		enemy_rect_left = ((missile_pointer) this_missile)->h - (MISSILE_SIZE << 15);
		enemy_rect_top = ((missile_pointer) this_missile)->v - (MISSILE_SIZE << 15);
		enemy_rect_bottom = enemy_rect_top + (MISSILE_SIZE << 16);
		enemy_rect_right = enemy_rect_left + (MISSILE_SIZE << 16);

		/* Check for collision with this missile */
		if (check_segment_intersection(x1, y1, x2, y2,
										enemy_rect_left, enemy_rect_bottom,
										enemy_rect_right, enemy_rect_top)
										
								||
			
			check_segment_intersection(x1, y1, x2, y2,
										enemy_rect_left, enemy_rect_top,
										enemy_rect_right, enemy_rect_bottom))
			{
			
			/* Explode this missile */
			explode_missile(((missile_pointer) this_missile), i, ours);
						
			/* Remember to update the missile which WAS last */
			i--;
			
			/* Next missile */
			continue;
			
			}
		
		}	/* loop through missiles */

}	/* check_line_hits() */



/*****************************************************************************\
* procedure explode_missile                                                   *
*                                                                             *
* Purpose: This explodes a missile.                                           *
*                                                                             *
* Parameters: which_missile: missile to explode                               *
*             i:             missile number                                   *
*             ours:          TRUE if we should get credit for this.           *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 3, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean explode_missile(missile_pointer which_missile, short i, Boolean ours)
{

	char	*radar_position;

	/* Create a pseudo-enemy to make the visual fireball */
	create_enemy(fireball, which_missile->h, which_missile->v, (5L << 16),
					which_missile->direction, -1);
	(*enemies[num_enemies-1])->exploding = FIREBALL_FRAMES >> 1;
	(*enemies[num_enemies-1])->phased = which_missile->phased;
	
	/* Find position of missile on radar */
	radar_position = which_missile->radar_position;
			
	/* Erase the missile from the radar */
	asm {
		move.l	radar_position, a0
		move.w	missile_color_word, d0
		eor.w	d0, (a0)
		add.l	screen_width, a0
		eor.w	d0, (a0)
		}
			
	/* If the missile was destroyed by one of our weapons, give us some points */
	if (ours)
		{
		score += MISSILE_POINTS;
		update_score_bar(FALSE);
		}

	/* Get rid of this missile */
	destroy_missile(i);
	
	/* Play the explosion sound */
	play_explosion_sound();
	
}	/* explode_missile() */



/*****************************************************************************\
* procedure check_segment_intersection                                        *
*                                                                             *
* Purpose: This checks whether two line segments intersect.                   *
*                                                                             *
* Parameters: The first line segment runs from (a1x, a1y) to (b1x, b1y)       *
*             The second segment runs from (a2x, a2y) to (b2x, b2y)           *
*             returns TRUE if the segments intersect.                         *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: January 1, 1992                                                 *
* Modified:                                                                   *
\*****************************************************************************/

Boolean check_segment_intersection(Fixed a1x, Fixed a1y, Fixed b1x, Fixed b1y,
									Fixed a2x, Fixed a2y, Fixed b2x, Fixed b2y)
{

	Fixed	t2;
	Fixed 	Q1, Q2, Q3, Q4;
	Fixed	intersect_x;

	/* The first line segment is defined by X1 = A1 + (B1 - A1) * t1 
	   The second is defined by             X2 = A2 + (B2 - A2) * t2
	   
	   Taking X1 == X2 gives us two parametric equations, which allows us to
	   solve for t2. 	*/
	
	/* Find intermediate quotients so this doesn't look like such a mess */
	if (!((b1x - a1x) >> 16))
		{
		b1x += (1L << 16);
	/*	Q1 = (a2x - a1x > 0) ? 0x7FFFFFFF : 0xFFFFFFFF;
		Q4 = (b2x - a2x > 0) ? 0x7FFFFFFF : 0xFFFFFFFF;	*/
		}

	Q1 = (a2x - a1x) / ((b1x - a1x) >> 16);
	Q4 = (b2x - a2x) / ((b1x - a1x) >> 16);
			
	if (!((b1y - a1y) >> 16))
		{
		b1y += (1L << 16);
	/*	Q3 = (b2y - a2y > 0) ? 0x7FFFFFFF : 0xFFFFFFFF;
		Q2 = (a2y - a1y > 0) ? 0x7FFFFFFF : 0xFFFFFFFF;	*/
		}
	
	Q3 = (b2y - a2y) / ((b1y - a1y) >> 16);
	Q2 = (a2y - a1y) / ((b1y - a1y) >> 16);
		
	/* Find t2 */
	if (!((Q3 - Q4) >> 16))
		t2 = 0xFFFFFFFF;
	else
		t2 = (Q1 - Q2) / ((Q3 - Q4) >> 16);
	
	/* Find the intersection point */
	intersect_x = a2x + ((b2x - a2x) >> 8) * (t2 >> 8);
/*	intersect_y = a2y + ((b2y - a2y) >> 8) * (t2 >> 8);	*/  /* not needed */
	
	/* If the intersection point is not between the endpoints there is no
		intersection */
	if ((intersect_x > a1x) && (intersect_x > b1x) || (intersect_x < a1x) && (intersect_x < b1x))
		return FALSE;
	if ((intersect_x > a2x) && (intersect_x > b2x) || (intersect_x < a2x) && (intersect_x < b2x))
		return FALSE;

	/* The point is between the endpoints of both segments; hence the segments intersect */
	return TRUE;	

}	/* check_segment_intersection() */



/*****************************************************************************\
* procedure fire_tracer                                                       *
*                                                                             *
* Purpose: This procedure fires another tracer.                               *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 25, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void fire_tracer(enemy_pointer which_enemy)
{

	register tracer_pointer	new_tracer;   
	short			tracer_direction;

	/* If we already have the max number of tracers, don't do anything */
	if (num_tracers == MAX_TRACERS)
		return;
	
	/* If it's us firing, deduct some energy for this tracer */
	if (which_enemy->type == ship)
		drain_energy(TRACER_ENERGY_DRAIN);
	
	/* Allocate memory for this new tracer */
	tracers[num_tracers] = (tracer_handle) NewHandle(sizeof(tracer));
	
	/* Dereference the new tracer */
	new_tracer = *tracers[num_tracers];
		
	/* Find the direction this tracer will be travelling */
	new_tracer->direction = ((which_enemy->direction >> 16) + which_enemy->turret) % NUM_DIRECTIONS;
	
	/* Move the tracer away from the ship */
	new_tracer->h = which_enemy->h + h_vector[new_tracer->direction] * 25L - (1L << 16);
	new_tracer->v = which_enemy->v + v_vector[new_tracer->direction] * 25L - (1L << 16);
	
	/* Set the base velocity of the tracer to that of the ship */
	new_tracer->base_speed_h = (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_tracer->base_speed_v = (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);

	/* Start the tracer countdown */
	new_tracer->fuse = TRACER_FUSE_LENGTH;		

	/* This tracer is phased if we are */
	new_tracer->phased = which_enemy->phased;
	new_tracer->phase_mask = 0xFF00;

	/* Start with first frame */
	new_tracer->frame = 0;
	
	/* If we have a weapons lock, aim the tracer at that enemy.  Otherwise, no lock */
	new_tracer->locked_enemy = which_enemy->tracer_lock_enemy;
	
	/* We now have a new tracer */
	num_tracers++;

}	/* fire_tracer() */
	


/*****************************************************************************\
* procedure fire_shell                                                        *
*                                                                             *
* Purpose: This procedure fires a shell.                                      *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*             fuse:        frames to detonation of shell                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void fire_shell(enemy_pointer which_enemy, short fuse)
{

	register shell_pointer	new_shell;   
	short			shell_direction;

	/* Allocate memory for this new shell */
	shells[num_shells] = (shell_handle) NewHandle(sizeof(shell));
	
	/* Dereference the new shell */
	new_shell = *shells[num_shells];
		
	/* Find the direction this shell will be travelling */
	shell_direction = ((which_enemy->direction >> 16) + which_enemy->turret) % NUM_DIRECTIONS;
	
	/* Find the velocity of a shell in the firing direction */
	new_shell->speed_h = SHELL_SPEED * h_vector[shell_direction];
	new_shell->speed_v = SHELL_SPEED * v_vector[shell_direction];

	/* Add the current enemy velocity to that of the bullet */
	new_shell->speed_h += (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_shell->speed_v += (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);

	/* Move the shell away from the enemy */
	new_shell->h = which_enemy->h + h_vector[shell_direction] * 27L - (1L << 16);
	new_shell->v = which_enemy->v + v_vector[shell_direction] * 27L - (1L << 16);
	
	/* Start the shell countdown */
	new_shell->fuse = fuse;		

	/* Give us credit if this is our shell */
	new_shell->ours = (which_enemy->type == ship);

	/* This shell is phased if the enemy is */
	new_shell->phased = which_enemy->phased;
	new_shell->phase_mask = 0xFF00FF00;

	/* We now have a new shell */
	num_shells++;

	/* This enemy can't fire another shell for a while */
	which_enemy->fire_countdown = SHELL_INTERVAL;

}	/* fire_shell() */



/*****************************************************************************\
* procedure fire_missile                                                      *
*                                                                             *
* Purpose: This procedure fires a missile.                                    *
*                                                                             *
* Parameters: which_enemy: the enemy which is firing                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: December 23, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

void fire_missile(enemy_pointer which_enemy)
{

	register missile_pointer	new_missile;   
	short			missile_direction;

	/* Allocate memory for this new missile */
	missiles[num_missiles] = (missile_handle) NewHandle(sizeof(missile));
	
	/* Dereference the new missile */
	new_missile = *missiles[num_missiles];
		
	/* Start with missile pointing upward */
	new_missile->direction = 0;
	
	/* Remember we haven't drawn this one on the radar yet */
	new_missile->radar_position = NULL;
	
	/* Start with velocity equal to that of the launcher */
	new_missile->speed_h = (h_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);
	new_missile->speed_v = (v_vector[(which_enemy->direction >> 16)] >> 8) * (which_enemy->speed >> 8);

	/* Start with the missile right on top of the enemy */
	new_missile->h = which_enemy->h;
	new_missile->v = which_enemy->v;
	
	/* Start the missile countdown */
	new_missile->fuse = MISSILE_FUSE_LENGTH;		

	/* This missile is phased if the enemy is */
	new_missile->phased = which_enemy->phased;
	new_missile->phase_mask = 0xFF00FF00;

	/* We now have a new missile */
	num_missiles++;

	/* Stop the firing sequence */
	which_enemy->silo_frame = 0;
	
	/* Lock on to the correct enemy */
	new_missile->enemy_lock = which_enemy->missile_lock_enemy;
	
	/* If this missile has locked on to us, light up the lock detected light */
	if (new_missile->enemy_lock == the_ship)
		{
		num_locks_detected++;
		draw_status_lights(FALSE);
		}

}	/* fire_missile() */
