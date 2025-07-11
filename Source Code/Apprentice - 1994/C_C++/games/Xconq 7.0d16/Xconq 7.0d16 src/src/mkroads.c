/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* A road generation method that connects specified types of units
   and favors specified types of terrain. */

/* (should add some favoring of nearby rather than totally random units?) */

#include "conq.h"

static Unit *otherunit;

static int roadtype;

make_roads()
{
    int doanyroads = FALSE, doroads[MAXUTYPES];
    int t, u1, u2;
    Unit *unit1, *unit2;
    Side *loopside1, *loopside2;
    int i = 0, numroads = 0;

    /* Look for a suitable terrain type. */
    roadtype = NONTTYPE;
    for_all_terrain_types(t) {
	if (t_is_connection(t)
	    && !aux_terrain_defined(t)
	    && t_subtype_x(t) == keyword_value(K_ROAD_X)) {
		roadtype = t;
		break;
	}
    }
    if (roadtype == NONTTYPE) return;
    /* Don't run if road chance always zero. */
    for_all_unit_types(u1) doroads[u1] = FALSE;
    for_all_unit_types(u1) {
	for_all_unit_types(u2) {
	    if (uu_road_chance(u1, u2) > 0) {
		doroads[u1] = TRUE;
		doanyroads = TRUE;
		break;
	    }
	}
    }
    if (!doanyroads) return;
    /* Now we're ready to get down to work. */
    allocate_area_aux_terrain(roadtype);
    announce_lengthy_process("Laying down roads");
    /* should be able to announce progress by guessing at number of roads. */
    for_all_units(loopside1, unit1) {
	if (in_play(unit1) && doroads[unit1->type]) {
	    for_all_units(loopside2, unit2) {
		if (unit1 != unit2
		    && in_play(unit2)
		    && (g_see_all()
			|| unit1->side == unit2->side
			|| (unit_sees_other_unit(unit1, unit2)
			    && unit_sees_other_unit(unit2, unit1)))
		    && probability(uu_road_chance(unit1->type, unit2->type))) {
		    lay_connecting_road(unit1, unit2);
		}
	    }
	}
    }
    finish_lengthy_process();
}

unit_sees_other_unit(unit1, unit2)
Unit *unit1, *unit2;
{
    int u2 = unit2->type;
    Side *side = unit1->side;

    /* Independent units are considered to know about all other units
       (not realistic, but no one will notice, since sides with players
       must also be able to see the indep unit before they will have
       a road going to it) */
    if (indep(unit1)) return TRUE;
    /* This works sometimes, but views aren't always completely set up yet. */
    if (side->unitview != NULL
	&& unit_view(side, unit2->x, unit2->y) != EMPTY) return TRUE;
    if (side->terrview != NULL
        && terrain_view(side, unit2->x, unit2->y) != UNSEEN
        && (u_see_always(u2)
            || (indep(unit2) ? u_already_seen_indep(u2) : u_already_seen(u2))))
      return TRUE;
    if (people_sides_defined()
        && people_side_at(unit2->x, unit2->y) == side->id)
      return TRUE;
    return FALSE;
}

/* Given that a road is wanted between the two given units, attempt to
   route it favorably. */

lay_connecting_road(unit1, unit2)
Unit *unit1, *unit2;
{
    lay_road_between(unit1->x, unit1->y, unit2->x, unit2->y);
}

/* Lay down a single bit of road, tending to choose directions with more
   favorable underlying terrain and merging with already-existing roads. */

/* (should change to road-into-weight, return sorted list of direction choices
   to path selector) */

lay_road_segment(x, y, d, choice, numchoices)
int x, y, d, choice, numchoices;
{
    int x1, y1, roadchance, foundroad = FALSE;

    if (interior_point_in_dir(x, y, d, &x1, &y1)) {
	/* See if any road already in the other end of this conn. */
	if (aux_terrain_at(x1, y1, roadtype)) foundroad = TRUE;
    	roadchance = tt_road_into_chance(terrain_at(x, y), terrain_at(x1, y1));
        if (roadchance > 0) {
            if (probability(roadchance) || foundroad) {
		set_connection_at(x, y, d, roadtype, TRUE);
		/* Return whether to keep going or stop because of shared road. */
		return (foundroad ? -1 : 1);
	    }
	}
    }
    /* Try next or give up completely if no more to try. */
    return (choice < numchoices ? 0 : -1);
}

/* Do the whole path. */

lay_road_between(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    apply_to_path(x1, y1, x2, y2, NULL, lay_road_segment, FALSE);
}

can_be_connection_on(t1, t2)
int t1, t2;
{
    return 1 /* (t_road_density(t2) > 0) */;
}
