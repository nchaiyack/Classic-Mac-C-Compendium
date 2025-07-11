/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file manages "sides", which mainly link units and players. */

/* (should sides and players be listed in arrays a la assignments array? */

#include "conq.h"

/* Head of the list of all sides. */

Side *sidelist;

/* Pointer to the side representing independence. */

Side *indepside;

/* Pointer to the last side of the list. */

Side *lastside;

/* Temporary used in many places. */
         
Side *tmpside;

/* The actual number of sides in a game.  This number never decreases;
   sides no longer playing need to be around for recordskeeping purposes. */

int numsides;

/* Used to generate the id number of the side. */

int nextsideid;

/* Pointer to buffer used for readable side description (for debugging). */

char *sidedesigbuf = NULL;

/* Pointer to the head of the list of players. */

Player *playerlist;

/* Pointer to the last player of the list, used to add players at end. */

Player *lastplayer;

/* The total number of players. */

int numplayers;

/* Use to generate the id number of each player. */

int nextplayerid;

/* Init side machinery.   We always have an "independent" side hanging around;
   it will be at the head of the list of sides, but normal side iteration
   finesses this by starting from the second list element.  The independent
   side is also a good placeholder for random stuff. */

init_sides()
{
    /* Set up the list of sides. */
    sidelist = lastside = (Side *) xmalloc(sizeof(Side));
    bzero(sidelist, sizeof(Side));
    /* The independent units' side will be the first one, is always created. */
    indepside = sidelist;
    /* Fill in some properties of the independents' side. */
    indepside->name = "- indep -";
    indepside->noun = "- indep -";
    indepside->adjective = "independent";
    indepside->colorscheme = NULL;
    /* By default, independent units are not identified by an emblem. */
    /* A game design can set this appropriately if desired, however. */
    indepside->emblemname = "none";
    indepside->id = 0;
    /* The independent side is not counted in numsides. */
    numsides = 0;
    /* Regular side ids start at 1 and go up. */
    nextsideid = 1;
    /* Set up the list of players. */
    playerlist = lastplayer = NULL;
    numplayers = 0;
    nextplayerid = 1;
    /* Set up the player/side assignment array. */
    assignments = (Assign *) xmalloc(MAXSIDES * sizeof(Assign));
}

/* Create an object representing a side. */

Side *
create_side()
{
    int i, u;
    Side *newside;

    if (numsides >= g_sides_max()) {
	run_error("Cannot have more than %d sides total!", g_sides_max());
    }
    /* Allocate the side object proper. */
    newside = (Side *) xmalloc(sizeof(Side));
    /* This will null or zero everything, so don't need to do each slot. */
    bzero(newside, sizeof(Side));
    /* Fill in various side slots.  Only those with non-zero/non-NULL
       defaults need have anything done to them. */
    newside->id = nextsideid++;
    /* Always start sides IN the game. */
    newside->ingame = TRUE;
    /* Set up the relationships with other sides. */
    newside->trusts = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    newside->trades = (short *) xmalloc((g_sides_max() + 1) * sizeof(short));
    /* (should flush if default is really going to be zero.) */
    for (i = 0; i < g_sides_max() + 1; ++i) {
	newside->trusts[i] = 0;
	newside->trades[i] = 0;
    }
    newside->startwith = NULL;
    newside->counts = (short *) xmalloc(numutypes * sizeof(short));
    newside->tech = (short *) xmalloc(numutypes * sizeof(short));
    newside->inittech = (short *) xmalloc(numutypes * sizeof(short));
    for_all_unit_types(u) {
	/* Start unit numbering at 1, not 0. */
	newside->counts[u] = 1;
    }
    newside->itertime = 100;
    newside->unitseveraskside = TRUE;
    newside->realtimeout = 0;
    newside->autofinish = TRUE;
    /* Set up the default doctrine substructure. */
    init_doctrine(newside);
    newside->startx = newside->starty = -1;
    newside->next = NULL;
    lastside->next = newside;
    lastside = newside;
    init_side_unithead(newside);
    ++numsides;
    return newside;
}

/* To make the double links work, we have to have one pseudo-unit to serve
   as a head.  This unit should not be seen by any outside code. */

init_side_unithead(side)
Side *side;
{
    if (side->unithead == NULL) {
	side->unithead = (Unit *) create_bare_unit(0);
	side->unithead->next = side->unithead;
	side->unithead->prev = side->unithead;
    }
}

/* Quick check to see if we have units. */

/* This should be improved to not be fooled by dead units? */

side_has_units(side)
Side *side;
{
    return (side->unithead != NULL
	    && (side->unithead->next != side->unithead));
}

/* Set up doctrine structures. */

init_doctrine(side)
Side *side;
{
    int u;

    side->doctrine = (Doctrine *) xmalloc(sizeof(Doctrine));
    /* Allocate just enough unit doctrines for all the defined types. */
    side->udoctrine = (Doctrine *) xmalloc(numutypes * sizeof(Doctrine));
    /* Fill each of those in with good defaults. */
    side->doctrine->everaskside = TRUE;
    for_all_unit_types(u) {
	u_doctrine(side, u, everaskside) = TRUE;
    }
}

/* Initialize basic viewing structures for a side.  This happens when the
   side is created (during/after reading but before synthesis). */

init_view(side)
Side *side;
{
    int x, y, terrainset = FALSE;

    if (g_see_all()) return;
    /* Allocate space for views. */
    /* (should check world validity/compatibility first) */
    if (side->terrview == NULL) {
	side->terrview = malloc_area_layer(short);
    } else {
	terrainset = TRUE;
    }
    if (side->unitview == NULL) {
	side->unitview = malloc_area_layer(short);
	side->unitviewdate = malloc_area_layer(short);
#if (MAXVIEWHISTORY > 0)
	side->viewhistory[MAXVIEWHISTORY] = malloc_area_layer(long);
#endif
    }
    /* Allocate the vision coverage cache if needed. */
    if (side->coverage == NULL) {
	side->coverage = malloc_area_layer(short);
    }
    /* Set it to a correct state for the units and people already present. */
    calc_coverage(side);
    tmpside = side;
    for_all_hexes(x, y) {
	if (g_terrain_seen()) {
	    init_view_hex(x, y);
	} else {
	    if (!terrainset) set_terrain_view(side, x, y, UNSEEN);
	    set_unit_view(side, x, y, EMPTY);
	    /* View date is 0, which is what we want. */
	}
    }
}

/* Calculate the centroid of all the starting units. */

calc_start_xy(side)
Side *side;
{
    int num = 0, sumx = 0, sumy = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && inside_area(unit->x, unit->y)) {
	    ++num;
	    sumx += unit->x;  sumy += unit->y;
	}
    }
    if (num > 0) {
	side->startx = wrapx(sumx / num);  side->starty = sumy / num;
    }
}

/* Get a char string naming the side.  Doesn't have to be pretty. */

/* (should synth complete name/adjective from other parts of speech) */

char *
side_name(side)
Side *side;
{
    return (side == NULL ? "independent" :
      (side->name ? side->name :
	    (side->adjective ? side->adjective : 
	     (side->pluralnoun ? side->pluralnoun :
	      (side->noun ? side->noun :
	       "")))));
}

char *
side_adjective(side)
Side *side;
{
    return (side == NULL ? "independent" :
       (side->adjective ? side->adjective :
	    (side->pluralnoun ? side->pluralnoun :
	     (side->noun ? side->noun :
	      (side->name ? side->name :
	       "")))));
}

/* Given a side, get its "number", which is same as its "id". */

side_number(side)
Side *side;
{
    return (side == NULL ? indepside->id : side->id);
}

/* The inverse function - given a number, figure out which side it is.
   Returns NULL for independent side number and for any failures. */

Side *
side_n(n)
int n;
{
    Side *rslt;

    if (n < 1) return NULL;
    for_all_sides(rslt) if (rslt->id == n) return rslt;
    return NULL;
}

/* This is true when one side controls another. */

side_controls_side(side, side2)
Side *side, *side2;
{
    if (side == NULL || side2 == NULL) return FALSE;
    return (side == side2 || side2->controlledby == side);
}

/* This is true if the given side may operate on the given unit. */

side_controls_unit(side, unit)
Side *side;
Unit *unit;
{
    if (side == NULL || unit == NULL) return FALSE;
    /* Most common case. */
    if (side == unit->side) return TRUE;
#ifdef DESIGNERS
    if (side->designer) return TRUE;
#endif
    if (side_controls_side(side, unit->side)) {
	/* should check if type is controllable and within control coverage */
	/* should check if unit's cmdr is controlled */
	/* The *unit* side must have the tech to use the unit, the controlling
	   side would have to actually take over the unit if it wants to use
	   its tech level to control the unit. */
	if (unit->side->tech[unit->type] < u_tech_to_use(unit->type)) return FALSE;
	return TRUE;
    } else {
	/* should check if in control coverage for non-owned units */
	return FALSE;
    }
}

/* This is true if the given side may examine the given unit. */

side_sees_unit(side, unit)
Side *side;
Unit *unit;
{
    if (side == NULL || unit == NULL) return FALSE;
    if (side->designer) return TRUE;
    if (side_controls_side(side, unit->side)) {
	return TRUE;
    } else {
	return FALSE;
    }
}

side_sees_image(side, unit)
Side *side;
Unit *unit;
{
    if (side == NULL || unit == NULL) return FALSE;
    if (side->designer) return TRUE;
    if (g_see_all()) return TRUE;
    if (side_controls_side(side, unit->side)) return TRUE;
    if (in_area(unit->x, unit->y) && cover(side, unit->x, unit->y) > 0) return TRUE;
    return FALSE;
}

num_units_in_play(side, u)
Side *side;
int u;
{
    int num = 0;
    Unit *unit;

    if (side == NULL || !side->ingame) return num;
    for_all_side_units(side, unit) {
	if (unit->type == u && in_play(unit) && completed(unit)) ++num;
    }
    return num;
}

num_units_incomplete(side, u)
Side *side;
int u;
{
    int num = 0;
    Unit *unit;

    if (side == NULL || !side->ingame) return num;
    for_all_side_units(side, unit) {
	if (unit->type == u && alive(unit) && !completed(unit)) ++num;
    }
    return num;
}

Unit *
find_next_unit(side, prevunit)
Side *side;
Unit *prevunit;
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (prevunit == NULL) prevunit = side->unithead;
	printf("Find_next_unit starting with %s\n", unit_desig(prevunit));
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    printf("Looking at %s", unit_desig(unit));
	    printf(", prev is %s", unit_desig(unit->prev));
	    printf(", next is %s", unit_desig(unit->next));
	    printf("\n");
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

Unit *
find_prev_unit(side, nextunit)
Side *side;
Unit *nextunit;
{
    Unit *unit = NULL;
    
    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

Unit *
find_next_actor(side, prevunit)
Side *side;
Unit *prevunit;
{
    Unit *unit = NULL;
    
    if (side != NULL) {
	if (prevunit == NULL) prevunit = side->unithead;
	for (unit = prevunit->next; unit != prevunit; unit = unit->next) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& (unit->act && unit->act->initacp)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

Unit *
find_prev_actor(side, nextunit)
Side *side;
Unit *nextunit;
{
    Unit *unit = NULL;

    if (side != NULL) {
	if (nextunit == NULL) nextunit = side->unithead;
	for (unit = nextunit->prev; unit != nextunit; unit = unit->prev) {
	    if (is_unit(unit) && unit->id > 0
		&& alive(unit)
		&& inside_area(unit->x, unit->y)
		&& (unit->act && unit->act->initacp)) {
		return unit;
	    }
	} 
    }
    return NULL;
}

/* Compute the total number of action points available to the side at the
   beginning of the turn. */

side_initacp(side)
Side *side;
{
    int totacp = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
	    totacp += unit->act->initacp;
	}
    }
    return totacp;
}

/* Return the total of acp still unused by the side. */

side_acp(side)
Side *side;
{
    int acpleft = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
	    acpleft += unit->act->acp;
	}
    }
    return acpleft;
}

side_acp_reserved(side)
Side *side;
{
    int acpleft = 0;
    Unit *unit;

    for_all_side_units(side, unit) {
	if (alive(unit) && unit->act) {
		if (unit->plan
		    && (unit->plan->reserve || unit->plan->asleep)) {
	    	acpleft += unit->act->acp;
	    }
	}
    }
    return acpleft;
}

/* (note that technology could be "factored out" of a game if all sides
   reach max tech at some point) */
/* (otherwise should compute once and cache) */
/* (note that once tech factors out, can never factor in again) */

using_tech_levels()
{
    int u;
    Side *side;

    for_all_sides(side) {
	for_all_unit_types(u) {
	    if (side->tech[u] < u_tech_max(u)) return TRUE;
	}
    }
    return FALSE;
}

/* Take the given side out of the game entirely.  This does not imply
   winning or losing, nor does it take down the side's display or AI. */

remove_side_from_game(side)
Side *side;
{
    Side *side2;

    /* Officially flag this side as being no longer in the game. */
    side->ingame = FALSE;
    /* Update imagery on everybody's views and displays, including
       those of the side just removed. */
    for_all_sides(side2) {
/*	    remove_images(side2, side_number(side));  */
	    update_side_display(side2, side);
    }
}

num_displayed_sides()
{
    int n = 0;
    Side *side;

    for_all_sides(side) {
	if (side_has_display(side)) ++n;
    }
    return n;
}

set_side_name(side, side2, newname)
Side *side, *side2;
char *newname;
{
    side2->name = newname;
    update_side_display(side, side2, TRUE);
}

#ifdef DESIGNERS

become_designer(side)
Side *side;
{
    if (!side->designer) ++numdesigners;
    side->designer = TRUE;
    /* Designers have godlike power in the game, so mark it (permanently)
       as no longer a normal game. */
    compromised = TRUE;
    update_everything();
}

become_nondesigner(side)
Side *side;
{
    if (side->designer) --numdesigners;
    side->designer = FALSE;
    update_everything();
}

#endif /* DESIGNERS */

trusted_side(side1, side2)
Side *side1, *side2;
{
    return (side1 != NULL && side1->trusts[side_number(side2)]);
}

/* What interfaces should use to tweak the autofinish flag. */

set_autofinish(side, value)
Side *side;
int value;
{
    side->autofinish = value;
}

/* Being at war requires only ones of the sides to consider itself so. */

/* (Should the other side's relationships be tweaked also?) */

enemy_side(s1, s2)
Side *s1, *s2;
{
    if (s1 == s2) return FALSE;
    return TRUE;
}

/* A formal alliance requires the agreement of both sides. */

allied_side(s1, s2)
Side *s1, *s2;
{
    if (s1 == s2) return TRUE;
    return FALSE;
}

/* Neutralness is basically anything else. */

neutral_side(s1, s2)
Side *s1, *s2;
{
    return TRUE;
}

/* Formal declarations of war need to do a transitive clsoure, as part of */
/* dragging allies in. */

declare_war(side1, side2)
Side *side1, *side2;
{
#if 0
    Side *side3;

    notify_all("The %s and the %s have declared war!!",
	       side1->pluralnoun, side2->pluralnoun);
    make_war(side1, side2);
    for_all_sides(side3) {
	if (allied_side(side3, side1)) make_war(side3, side2);
	if (allied_side(side3, side2)) make_war(side3, side1);
    }
#endif
}

/* Internal non-noisy function. */

make_war(side1, side2)
Side *side1, *side2;
{
#if 0
    side1->relationships[side_number(side2)] = ENEMY;
    side2->relationships[side_number(side1)] = ENEMY;
#endif
}

/* Establish neutrality for both sides. */

declare_neutrality(side1, side2)
Side *side1, *side2;
{
#if 0
    notify_all("The %s and the %s have agreed to neutrality.",
	       side1->pluralnoun, side2->pluralnoun);
    make_neutrality(side1, side2);
#endif
}

/* Internal non-noisy function. */

make_neutrality(side1, side2)
Side *side1, *side2;
{
#if 0
    side1->relationships[side_number(side2)] = NEUTRAL;
    side2->relationships[side_number(side1)] = NEUTRAL;
#endif
}

/* Establish the alliance for both sides, then extend it to include */
/* every other ally (only need one pass over sides to ensure transitive */
/* closure, because alliances formed one at a time). */

declare_alliance(side1, side2)
Side *side1, *side2;
{
#if 0
    Side *side3;

    notify_all("The %s and the %s enter into an alliance.",
	       side1->pluralnoun, side2->pluralnoun);
    make_alliance(side1, side2);
    for_all_sides(side3) {
	if (allied_side(side3, side1)) make_alliance(side3, side2);
	if (allied_side(side3, side2)) make_alliance(side3, side1);
    }
#endif
}

/* Internal non-noisy function. */

make_alliance(side1, side2)
Side *side1, *side2;
{
#if 0
    if (side1 != side2) {
	side1->relationships[side_number(side2)] = ALLY;
	side2->relationships[side_number(side1)] = ALLY;
    }
#endif
}

/* General method for passing along info about one side to another. */
/* If sender is NULL, it means to pass along info about *all* sides. */

reveal_side(sender, recipient, chance)
Side *sender, *recipient;
int chance;
{
    int x, y;
    Unit *unit;
    Side *loopside;

	if (g_see_all()) return;
#if 0
    if (chance >= 100) {
	for_all_hexes(x, y) {
	    short view = side_view(recipient, x, y);
	    int ts = side_view_timestamp(recipient, x, y);
	    if (view != EMPTY &&
		view != UNSEEN &&
		side_n(vside(view)) == sender) { 
		see_exact(recipient, x, y);
		side_view_timestamp(recipient, x, y) = ts;
	    }
	}
    }
    for_all_units(loopside, unit) {
	if (alive(unit) &&
	    (unit->side == sender || sender == NULL) &&
	    probability(chance)) {
	    see_exact(recipient, unit->x, unit->y);
	    update_cell_display(recipient, unit->x, unit->y, TRUE);
	}
    }
#endif
}

/* What happens when a unit appears on a given hex. */

/* An always-seen unit has builtin spies/tracers to inform everybody else of
   all its movements.  When such a unit occupies a hex, coverage is turned
   on and remains on until the unit leaves that hex. */

/* If this unit with onboard spies wanders into unknown territory,
   shouldn't that territory become known as well?  I think the unseen
   test should only apply during initialization. */
/* But if always-seen unit concealed during init, will magically appear
   when it first moves! */

all_see_occupy(unit, x, y, inopen)
Unit *unit;
int x, y;
int inopen;
{
    Side *side;
    int seeall = g_see_all(), always = u_see_always(unit->type);

    for_all_sides(side) {
	if (seeall) {
	    update_cell_display(side, x, y, TRUE);
	} if (side_sees_unit(side, unit)) {
	    see_hex(side, x, y);
	} else {
	    if (always && terrain_view(side, x, y) != UNSEEN) {
		if (side->coverage) add_cover(side, x, y, 1 /* should be "guaranteed visible" */);
	    }
	    if (inopen || always) {
		see_hex(side, x, y);
	    }
	}
    }
}

/* Some highly visible unit types cannot leave a hex without everybody
   knowing about the event.  The visibility is attached to the unit, not
   the hex, so first the newly-empty hex is viewed, then view coverage
   is decremented. */

all_see_leave(unit, x, y, inopen)
Unit *unit;
int x, y;
int inopen;
{
    Side *side;
    int seeall = g_see_all(), always = u_see_always(unit->type);
    int olduview;

    for_all_sides(side) {
	if (seeall) {
	    update_cell_display(side, x, y, TRUE);
	} else if (side_sees_unit(side, unit)) {
	    see_hex(side, x, y);
	} else {
	    if (always && terrain_view(side, x, y) != UNSEEN) {
		see_hex(side, x, y);
		if (side->coverage && cover(side, x, y) > 0) add_cover(side, x, y, -1);
	    }
	    /* Won't be called twice, because view coverage is 0 now. */
	    if (inopen) {
		see_hex(side, x, y);
	    }
	    /* special hack to flush images we *know* are garbage */
	    if (side->coverage && cover(side, x, y) < 1) {
	    	olduview = unit_view(side, x, y);
	    	
	    	if (vside(olduview) == side_number(side)) {
	    	    set_unit_view(side, x, y, EMPTY);
	    	    set_unit_view_date(side, x, y, curturn);
		    update_cell_display(side, x, y, FALSE);
	    	}
	    }
	}
    }
}

/* Unit's beady eyes are now covering (or not, depending on sign of onoff)
   the immediate area.  The view is a hexagonal area overlapping the area;
   since new things may be coming into view, we have to check and maybe
   draw lots of hexes (but only need the one output flush, fortunately). */

/* (LOS comes in here, to make irregular coverage areas) */

cover_area(unit, x0, y0, onoff)
Unit *unit;
int x0, y0, onoff;
{
    int u = unit->type, range, x, y, x1, y1, x2, y2, adj, diff, dist, cov;
    int coverdelta;
    Unit *eunit;
    Side *side = unit->side;

    if (indep(unit)
	|| g_see_all()
	|| !in_area(unit->x, unit->y)
	|| !completed(unit)
	|| side->coverage == NULL)
      return;
    range = u_vision_range(u);
    adj = u_vision_adj(u);
    diff = adj - u_vision_at_max_range(u);
    /* These may be outside the area - necessary since units may be able
       to see farther in x than the height of the area. */
    y1 = y0 - range;
    y2 = y0 + range;
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, area.height-1)) {
	    x1 = x0 - (y < y0 ? (y - y1) : range);
	    x2 = x0 + (y > y0 ? (y2 - y) : range);
	    for (x = x1; x <= x2; ++x) {
		/* Don't need coverage on edge hexes. */
		if (in_area(x, y)) {
		    dist = distance(x0, y0, x, y);
		    /* Add to the coverage of this hex. */
		    if (dist > 0) {
		        coverdelta = adj - ((dist - 1) * diff) / range;
		    } else {
		    	coverdelta = u_vision_at(u);
		    }
		    cov = onoff * coverdelta + cover(side, wrap(x), y);
		    set_cover(side, wrap(x), y, max(0, cov));
		    /* View the hex and maybe get excited about what we see. */
		    if (onoff > 0 && see_hex(side, wrap(x), y)) {
			react_to_seen_unit(side, unit, wrap(x), y);
		    }
		}
	    }
	}
    }
    /* As a special case, units with a vision
    /* If we're seeing new things, make sure they're on the display. */
    if (onoff > 0) {
	flush_display_buffers(side);
    }
}

/* Use this to clear out garbled view coverage. */

reset_coverage()
{
    Side *side;

    if (g_see_all()) return;
    for_all_sides(side) {
	calc_coverage(side);
    }
}

/* Calculate/recalculate the view coverage layers of a side. */

calc_coverage(side)
Side *side;
{
    int x, y, s = side_number(side);
    Unit *unit;

    if (side->coverage == NULL) return;
    Dprintf("Calculating all view coverage for %s\n", side_desig(side));
    if (people_sides_defined()) {
	for_all_hexes(x, y) {
	    set_cover(side, x, y, (people_side_at(x, y) == s ? 1 : 0));
	}
    } else {
	for_all_hexes(x, y) {
	    set_cover(side, x, y, 0);
	}
    }
    /* Compute coverage by the units already in place. */
    for_all_side_units(side, unit) {
	cover_area(unit, unit->x, unit->y, 1);
    }
}

reset_all_views()
{
    Side *side;

    if (g_see_all()) return;
    for_all_sides(side) {
	reset_view(side);
    }
}

reset_view(side)
Side *side;
{
    int x, y, uview;

    for_all_hexes(x, y) {
	if (cover(side, x, y) == 0
	    && ((uview = unit_view(side, x, y)) != EMPTY)
	    && vside(uview) == side_number(side)) {
	    set_unit_view(side, x, y, EMPTY);
	}
    }
}

react_to_seen_unit(side, unit, x, y)
Side *side;
Unit *unit;
int x, y;
{
    int uview, eu;
    Unit *eunit;
    Side *es;

    if (g_see_all() /* see real unit */) {
    	/* (should look at all of stack if can be mixed) */
	if ((eunit = unit_at(wrap(x), y)) != NULL) {
	    if (react_to_enemies(unit) && !allied_side(eunit->side, side)) {
		/* should do a more general alarm */
	        wake_unit(unit, TRUE);
	    }
	}
    } else if (side->coverage) {
    	if ((uview = unit_view(side, x, y)) != EMPTY) {
    	    eu = vtype(uview);  es = side_n(vside(uview));
    	    /* react only to certain utypes? */
	    if (react_to_enemies(unit) && !allied_side(es, side)) {
		/* should do a more general alarm */
		wake_unit(unit, TRUE);
	    }
    	}
    } else {
    	/* ??? */
    }
}

/* Update the view of this hex for everybody's benefit.  May have to write
   to many displays, sigh. */

all_see_hex(x, y)
int x, y;
{
    register Side *side;

    for_all_sides(side) see_hex(side, x, y);
}

/* Look at the given position, possibly not seeing anything.  Return true if
   a unit was spotted. */

see_hex(side, x, y)
Side *side;
int x, y;
{
    int update = FALSE, u, chance, t, curtview, newtview, curuview, newuview;
    Unit *unit, *unitseen = NULL;

    if (side == NULL || !in_area(x, y)) return FALSE;
    /* If we see everything, just pass through to updating the display. */
    if (g_see_all()) {
    	update = TRUE;
    	unitseen = unit_at(x, y);
    } else if (side->coverage && cover(side, x, y) > 0) {
    	/* Update our knowledge of the terrain. */
    	curtview = terrain_view(side, x, y);
 	if (curtview == UNSEEN) {
	    set_terrain_view(side, x, y, buildtview(terrain_at(x, y)));
	    update = TRUE;
	}
   	curuview = unit_view(side, x, y);
   	for_all_stack(x, y, unit) {
	    if (side_sees_unit(side, unit)) {
		unitseen = unit;
		break;
	    } else {
		t = terrain_at(x, y);
		chance = cover(side, x, y) * 100 /* should be scaled? */;
		chance = (chance * ut_visibility(unit->type, t)) / 100;
		if (probability(chance)) {
		    unitseen = unit;
		    break;
		}
	    }
	}
	if (unitseen) {
	    newuview = builduview(side_number(unitseen->side), unitseen->type);
	    if (curuview != newuview) {
		set_unit_view(side, x, y, newuview);
	    	set_unit_view_date(side, x, y, curturn);
		update = TRUE;
	    } else {
	    	/* Same as we already know, so not considered a change. */
	    	unitseen = NULL;
	    }
	} else if (curuview != EMPTY) {
	    set_unit_view(side, x, y, EMPTY);
	    set_unit_view_date(side, x, y, curturn);
	    update = TRUE;
	}
    }
    /* If there was any change in what was visible, tell the display. */
    if (update) {
	update_cell_display(side, x, y, FALSE);
    }
    return (unitseen != NULL);
}

/* "Bare-bones" viewing, for whenever you know exactly what's there.
   This is the lowest level of all viewing routines, and executed a *lot*. */

void
see_exact(side, x, y)
Side *side;
int x, y;
{
    register int newview;
    register Unit *unit;

    if (side == NULL || !in_area(x, y)) return;
    if (!g_see_all()) {
    	set_terrain_view(side, x, y, 1);
	if ((unit = unit_at(x, y)) != NULL) {
	    newview = builduview(side_number(unit->side), unit->type);
	    set_unit_view(side, x, y, newview);
	    set_unit_view_date(side, x, y, curturn);
	}
    }
    /* Display the hex, but don't flush it to the screen now. */
    update_cell_display(side, x, y, FALSE);
}

/* Utility to clean up images of units from a lost side.  (Since everybody
   heard about the loss, it's hardly a secret.) */

/* (What if hex was a mixed-side stack? should re-see hexes instead.) */

remove_images(side, s2)
Side *side;
int s2;
{
    int x, y, view;

    if (g_see_all()) return;
    for_all_hexes(x, y) {
	view = unit_view(side, x, y);
	if (view != EMPTY && vside(view) == s2) {
	    set_unit_view(side, x, y, EMPTY);
	    set_unit_view_date(side, x, y, curturn);
	    update_cell_display(side, x, y, TRUE);
	}
    }
}

/* A border has been seen if the hex on either side has been seen. */

seen_border(side, x, y, dir)
Side *side;
int x, y, dir;
{
    return (terrain_view(side, x, y) != UNSEEN
	    || terrain_view(side, wrap(x + dirx[dir]), y + diry[dir]) != UNSEEN);
}

/* Make a printable identification of the given side. */

char *
side_desig(side)
Side *side;
{
    if (sidedesigbuf == NULL) sidedesigbuf = xmalloc(BUFSIZE);
    if (side != NULL) {
	sprintf(sidedesigbuf, "s%d (%s)", side_number(side), side_name(side));
	if (side->selfunit) {
	    sprintf(sidedesigbuf+strlen(sidedesigbuf), "(self is #%d)", side->selfunit->id);
	}
    } else {
	sprintf(sidedesigbuf, "null side");
    }
    return sidedesigbuf;
}

/* Add a player into the list of players.  All values are defaults here. */

Player *
add_player()
{
    Player *player = (Player *) xmalloc(sizeof(Player));

    player->id = nextplayerid++;
    /* Note that all names and suchlike slots are NULL. */
    numplayers++;
    /* Add this one to the end of the player list. */
    if (lastplayer == NULL) {
	playerlist = lastplayer = player;
    } else {
	lastplayer->next = player;
	lastplayer = player;
    }
    Dprintf("Added a player\n");
    return player;
}

Player *find_player(n)
int n;
{
    Player *player;

    for (player = playerlist; player != NULL; player = player->next) {
	if (player->id == n) return player;
    }
    return NULL;
}

canonicalize_player(player)
Player *player;
{
    if (player == NULL) return;
    if (player->displayname && strlen(player->displayname) == 0) {
	player->displayname = NULL;
    }
    if (player->aitypename && strlen(player->aitypename) == 0) {
	player->aitypename = NULL;
    }
}

/* Make a printable identification of the given player. */

char playerdesigbuf[100];

char *
player_desig(player)
Player *player;
{
    if (player != NULL) {
	sprintf(playerdesigbuf, "%s,%s/%s@%s+%d",
		(player->name ? player->name : ""),
		(player->aitypename ? player->aitypename : ""),
		(player->configname ? player->configname : ""),
		(player->displayname ? player->displayname : ""),
		player->advantage);
    } else {
	sprintf(playerdesigbuf, "nullplayer");
    }
    return playerdesigbuf;
}

/* Agreement handling here for now. */

Agreement *agreementlist = NULL;

Agreement *lastagreement;

int numagreements = 0;

init_agreements()
{
    agreementlist = lastagreement = NULL;
    numagreements = 0;
}

Agreement *
create_agreement()
{
    Agreement *ag = (Agreement *) xmalloc(sizeof(Agreement));

    ag->terms = lispnil;
    ag->next = NULL;
    if (agreementlist != NULL) {
	lastagreement->next = ag;
    } else {
	agreementlist = lastagreement = ag;
    }
    ++numagreements;
    return ag;
}

char agreementdesigbuf[BUFSIZE];

char *
agreement_desig(ag)
Agreement *ag;
{
    sprintf(agreementdesigbuf, "<ag %s %s %s>",
	    (ag->typename ? ag->typename : "(null)"),
	    (ag->name ? ag->name : "(null)"),
	    (ag->terms == lispnil ? "(no terms)" : "...terms..."));
    return agreementdesigbuf;
}

#if 0
	} else if (strcmp(msg, "briefing") == 0) {
	    notify("Receiving a briefing from the %s...",
		   plural_form(side->name));
	    reveal_side(side, reqside, 100);
	    notify("You just briefed the %s on your position.",
		   plural_form(reqside->name));
	} else if (strcmp(msg, "alliance") == 0) {
	    notify("You propose a formal alliance with the %s.",
		   plural_form(reqside->name));
	    side->relationships[side_number(reqside)] = ALLY;
	    if (reqside->relationships[side_number(side)] >= ALLY) {
		declare_alliance(side, reqside);
		for_all_sides(side3) redraw(side3);
	    } else {
		notify("The %s propose a formal alliance.",
		       plural_form(side->name));
	    }
	} else if (strcmp(msg, "neutral") == 0) {
	    notify("You propose neutrality with the %s.",
		   plural_form(reqside->name));
	    side->relationships[side_number(reqside)] = NEUTRAL;
	    if (reqside->relationships[side_number(side)] == NEUTRAL) {
		declare_neutrality(side, reqside);
		for_all_sides(side3) redraw(side3);
	    } else {
		notify("The %s propose neutrality.",
		       plural_form(side->name));
	    }
	} else if (strcmp(msg, "war") == 0) {
	    notify("You declare war on the %s!",
		   plural_form(reqside->name));
	    declare_war(side, reqside);
	    for_all_sides(side3) redraw(side3);
#endif
