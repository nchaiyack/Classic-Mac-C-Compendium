/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This river generator requires elevations. */

#include "conq.h"

static long totalrivers;

/* Add rivers by picking a headwater randomly, then running downhill. */

/* Can do as either a border or connection type, depending on which
   terrain type is called "river". */

make_rivers()
{
    int x, y, t;

    totalrivers = 0;
    for_all_hexes(x, y) {
	totalrivers += t_river_chance(terrain_at(x, y));
    }
    if (totalrivers <= 0) return;
    totalrivers /= 10000;
    totalrivers = max(1, totalrivers);
    if (elevations_defined()) {
	for_all_terrain_types(t) {
	    if (t_subtype_x(t) == keyword_value(K_RIVER_X)
		&& !aux_terrain_defined(t)) {
		if (t_is_border(t)) {
		    make_up_river_borders(t);
		} else if (t_is_connection(t)) {
		    make_up_river_connections(t);
		}
	    }
	}
    }
}

int touchedwater;

int lastrx, lastry, lastrvdir;

make_up_river_borders(rivertype)
int rivertype;
{
    int i, j, x, y, dir, elev, len, x1, y1, d1, e1, x2, y2, d2, e2;
    int x0, y0, numrivers = 0, numrivs;
    int low, lowdir;
    int t, watery[MAXTTYPES];

    announce_lengthy_process("Creating rivers (as borders)");
    allocate_area_aux_terrain(rivertype);
    for_all_interior_hexes(x0, y0) {
    	if (y0 % 2 != 0 || x0 % 2 != 0) continue;
	if (probability(t_river_chance(terrain_at(x0, y0)) / 100)) {
	    ++numrivers;
	    touchedwater = FALSE;
	    if (numrivers % 5 == 0)
	      announce_progress((100 * numrivers) / totalrivers);
	    x = x0;  y = y0;
	    /* Set the initial river direction. */
	    lowdir = 0;
	    low = elev_at_meet(x, y, lowdir);
	    for_all_directions(dir) {
		if (elev_at_meet(x, y, dir) < low) {
		    lowdir = dir;
		    low = elev_at_meet(x, y, lowdir); 
		}
	    }
	    dir = lowdir;
	    /* We do a right-hand -> downhill rule, so may need to flip around
	       and look at/work with river on opposite side. */
	    if (elev_at_meet(x, y, dir) <
		elev_at_meet(wrap(x+dirx[dir]),y+diry[dir], opposite_dir(dir))) {
		x += dirx[dir];  y += diry[dir];
		dir = opposite_dir(dir);
	    }
	    if (border_is_compatible(x, y, dir, rivertype)) {
		set_river_at(x, y, dir, rivertype);
	    } else {
		continue;
	    }
	    for (j = 0; j < 20; ++j) {  /* why 20?? */
		if (!inside_area(x, y)) break;
		elev = elev_at_meet(x, y, dir);
		/* Compute hex and dir of the two possible ways to flow. */
		x1 = x + dirx[right_dir(dir)];  y1 = y + diry[right_dir(dir)];
		d1 = left_dir(dir);
		e1 = elev_at_meet(x1, y1, d1);
		x2 = x;  y2 = y;
		d2 = right_dir(dir);
		e2 = elev_at_meet(x2, y2, d2);
		if (elev < e1 && elev < e2) {
		    init_warning("river going uphill??");
		    break;
		}
		/* Pick the lower of the two. */
		if (e1 < e2) {
		    x = x1;  y = y1;  dir = d1;
		} else {
		    x = x2;  y = y2;  dir = d2;
		}
		/* Rivers always follow the same paths, so if we see a river
		   here already, we're done. */
		if (border_at(x, y, dir, rivertype)) break;
		if (border_is_compatible(x, y, dir, rivertype)) {
		    set_river_at(x, y, dir, rivertype);
		}
	    }
	}
    }
    /* Cells surrounded by river should maybe be set specially. */
    if (g_river_sink_terrain() != NONTTYPE) {
	for_all_hexes(x, y) {
	    if (inside_area(x, y)) {
		numrivs = 0;
		for_all_directions(dir) {
		    if (border_at(x, y, dir, rivertype)) ++numrivs;
		}
		if (numrivs >= NUMDIRS) {
		    set_terrain_at(x, y, g_river_sink_terrain());
		}
	    }
	}
    }
    /* Fix any bad adjacencies that got through. */
    for_all_terrain_types(t) {
    	watery[t] = t_liquid(t);
    }
    for_all_interior_hexes(x, y) {
	    if (watery[terrain_at(x, y)]) {
		for_all_directions(dir) {
		    set_border_at(x, y, dir, rivertype, FALSE);
		}
	    }
    }
    finish_lengthy_process();
}

border_is_compatible(x, y, dir, b)
int x, y, dir, b;
{
    int x1, y1;

    return (tt_adj_terr_effect(terrain_at(x, y), b) < 0
            && point_in_dir(x, y, dir, &x1, &y1)
	    && tt_adj_terr_effect(terrain_at(x1, y1), b) < 0);
}

elev_in_dir(x, y, dir)
int x, y, dir;
{
    int x1, y1;

    point_in_dir(x, y, dir, &x1, &y1);

    return elev_at(x1, y1);
}

/* The elevation at the junction of three hexes is the lowest of the three. */

elev_at_meet(x, y, dir)
int x, y, dir;
{
    int elev = elev_at(x, y);

    if (elev_in_dir(x, y, dir) < elev)
      elev = elev_in_dir(x, y, dir);
    if (elev_in_dir(x, y, right_dir(dir)) < elev)
      elev = elev_in_dir(x, y, right_dir(dir));
    return elev;
}

/* should have various conditions */

set_river_at(x, y, dir, t)
int x, y, dir, t;
{
    int x1, y1;

    if (touchedwater) {
    } else if (t_liquid(terrain_at(x, y))) {
	touchedwater = TRUE;
    } else if (point_in_dir(x, y, dir, &x1, &y1)
	       && t_liquid(terrain_at(x1, y1))) {
	touchedwater = TRUE;
    } else {
	set_border_at(x, y, dir, t, TRUE);
    }
}

make_up_river_connections(rivertype)
int rivertype;
{
    int x0, y0, i, x, y, x1, y1, dir;
    int numrivers = 0, lowdir, low;

    allocate_area_aux_terrain(rivertype);
    announce_lengthy_process("Creating rivers (as connections)");
    for_all_interior_hexes(x0, y0) {
      if (probability(t_river_chance(terrain_at(x0, y0)) / 100)) {
        ++numrivers;
	if (numrivers % 5 == 0)
	  announce_progress((100 * numrivers) / totalrivers);
	x = x0;  y = y0;
	for (i = 0; i < 20; ++i) {
		/* Find the direction to the lowest adjacent cell. */
		lowdir = -1;
		low = elev_at(x, y);
		for_all_directions(dir) {
		    if (point_in_dir(x, y, dir, &x1, &y1)) {
			if (elev_at(x1, y1) <= low) {
			    lowdir = dir;
			    low = elev_at(x1, y1); 
			}
		    }
		}
		if (lowdir < 0) break;
		point_in_dir(x, y, lowdir, &x1, &y1);
		if (!(t_liquid(terrain_at(x, y))
		      && t_liquid(terrain_at(x1, y1)))) {
		   set_connection_at(x, y, lowdir, rivertype, TRUE);
		}
		x = x1;  y = y1;
		if (!inside_area(x, y)) break;
	}
      }
    }
    finish_lengthy_process();
}
