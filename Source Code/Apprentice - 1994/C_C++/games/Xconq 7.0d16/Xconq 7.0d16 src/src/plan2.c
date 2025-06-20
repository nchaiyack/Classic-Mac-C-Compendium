/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Auxilary functions for unit plans. */

#include "conq.h"

Task *find_route();

/* router flags */

#define SAMEPATH 1
#define EXPLORE_PATH 2

/* These macros are a cache used for planning purposes by machines. */

#define markloc(x, y) (set_tmp1_at(x, y, mark))

#define markedloc(x, y) (tmp1_at(x, y) == mark)

#define get_fromdir(x, y) (tmp2_at(x, y))

#define set_fromdir(x, y, dir) (set_tmp2_at(x, y, dir))

#define get_dist(x, y) (tmp3_at(x, y))

#define set_dist(x, y, d) (set_tmp3_at(x, y, d))

int route_max_distance;

/* Count how many units on our side of the given type are within the */
/* specified distance. */

int
units_nearby(x, y, dist, type)
int x, y;
{
    Unit *unit;
    int ux, uy;

    int unit_count = 0;
#if 0
    for (unit = side_strategy(mside)->unitlist[type]; unit != NULL; unit = unit->mlist) {
	ux = unit->x;  uy = unit->y;
	if (alive(unit) && distance(x, y, ux, uy) <= dist) unit_count++;
    }
#endif
    return unit_count;
}

/* Can this unit build a base without dying. */

survive_to_build_base(unit)
Unit *unit;
{
  return (base_builder(unit) &&
	  survival_time(unit) > build_time(unit, base_builder(unit)));
}

/* Is this the last chance for a unit to build a base without dying. */

exact_survive_to_build_base(unit)
Unit *unit;
{
  return (base_builder(unit) &&
	  survival_time(unit) == (1 + build_time(unit, base_builder(unit))));
}

/* Is there a machine base here. */

base_here(x, y)
int x, y;
{
    Unit *unit = unit_at(x, y);

    return (0 /* unit != NULL && unit->side == mside && isbase(unit) */) ;
}

/* Is there anybodies base here. */

any_base_here(x, y)
int x, y;
{
/*    int utype = vtype(side_view(mside, x, y)); */

    return (0 /* utype != EMPTY && utype != UNSEEN && u_is_base(utype) */);
}

/* Is there anybodies base here. */

neutral_base_here(x, y)
int x, y;
{
#if 0
    int view = side_view(mside, x, y);
    int utype = vtype(view);

    return (utype != EMPTY && utype != UNSEEN &&
	    u_is_base(utype) && vside_indep(view));
#endif
}

/* Is there a base within the given range.  Generally range is small. */

base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, base_here, &x, &y, 1);
}

/* Is there a base within the given range.  Generally range is small. */

any_base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, any_base_here, &x, &y, 1);
}

/* Is there a neutral base within the given range.  Generally range is small. */

neutral_base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, neutral_base_here, &x, &y, 1);
}


occupant_could_capture(unit,etype)
Unit *unit;
int etype;
{
    Unit *occ;

    for_all_occupants(unit,occ)
      if could_capture(occ->type, etype)
	return TRUE;
    return FALSE;
}

/* Check to see if there is anyone around to capture. */

can_capture_neighbor(unit)
Unit *unit;
{
    int d, x, y;
    int view;
    Side *side2;

    for_all_directions(d) {
	x = wrap(unit->x + dirx[d]);  y = limit(unit->y + diry[d]);
	view = unit_view(unit->side, x, y);
	if (view != UNSEEN && view != EMPTY) {
	    side2 = side_n(vside(view));
	    if (!allied_side(unit->side, side2)) {
		if (could_capture(unit->type, vtype(view))) {
		    /* need some other way to change move order quickly */
#if 0
		    mside->movunit = unit;
		    mside->last_unit = unit;
#endif
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

/* check if our first occupant can capture something.  Doesn't look at
other occupants. */

occupant_can_capture_neighbor(unit)
Unit *unit;
{
    Unit *occ = unit->occupant;

    if (occ != NULL && occ->act && occ->act->acp > 0 && occ->side == unit->side) {
	if (can_capture_neighbor(occ)) {
#if 0
	    mside->movunit = occ;
	    mside->last_unit = occ;
#endif
	    return TRUE;
	}
    }
    return FALSE;
}

/* Find the closes unit, first prefering bases, and then transports. */

find_closest_unit(side, x0, y0, maxdist, pred, rxp, ryp)
Side *side;
int x0, y0, maxdist, (*pred)(), *rxp, *ryp;
{
    Unit *unit;
    int u, dist;
    int found = FALSE;

#if 0    
    for_all_unit_types(u) {
	if (u_is_base(u)) {
	    for (unit = NULL /* side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit) &&
		    (dist = distance(x0, y0, unit->x, unit->y)) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
    for_all_unit_types(u) {
	if (!u_is_base(u) && u_is_transport(u)) {
	    for (unit = NULL /*side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit)
		    && distance(x0, y0, unit->x, unit->y) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
    /* (What's the point of finding a non-base/non-transport?) */
    for_all_unit_types(u) {
	if (!u_is_base(u) && !u_is_transport(u)) {
	    for (unit = NULL/*side_strategy(side)->unitlist[u]*/; unit != NULL; unit = unit->mlist) {
		if (alive(unit)
		    && distance(x0, y0, unit->x, unit->y) <= maxdist) {
		    if ((*pred)(unit->x, unit->y)) {
			maxdist = dist - 1;
			*rxp = unit->x;  *ryp = unit->y;
			found = TRUE;
		    }
		}
	    }
	}
    }
    if (found) {
	return TRUE;
    }
#endif
    return FALSE;
}


/* Return percentage of capacity. */

fullness(unit)
Unit *unit;
{
    int u = unit->type, o, cap = 0, num = 0, vol = 0;
    Unit *occ;

    for_all_unit_types(o) cap += uu_capacity_x(u, o);
    for_all_occupants(unit, occ) {
	num++;
    }
/*
    return (cap > 0) ? max(((u_holdvolume(u) > 0) ? (100 * vol) / u_holdvolume(u) : 100),
	        (100 * num) / cap) : 100; */
    return 0;

}

/* True if the given unit is a sort that can build other units. */

can_build(unit)
Unit *unit;
{
    int p;

    for_all_unit_types(p) {
	if (could_make(unit->type, p)) return TRUE;
    }
    return FALSE;
}

/* Test if unit can move out into adjacent hexes. */

can_move(unit)
Unit *unit;
{
    int d, x, y;

    for_all_directions(d) {
	x = wrap(unit->x + dirx[d]);  y = limit(unit->y + diry[d]);
	if (could_move(unit->type, terrain_at(x, y))) return TRUE;
    }
    return FALSE;
}

/* Returns the type of missing supplies. Not great routine if first */
/* material is a type of ammo. */

out_of_ammo(unit)
Unit *unit;
{
    int u = unit->type, r;

    for_all_material_types(r) {
	if (um_hitswith(u, r) > 0 && unit->supply[r] <= 0)
	    return r;
    }
    return (-1);
}

/* Someplace that we can definitely get supplies at. */

good_haven_p(side, x, y)
Side *side;
int x, y;
{
    Unit *unit = unit_at(x, y);
    int r;
    Task *route;

    if (unit != NULL) {
	if (allied_side(side, unit->side) && alive(unit) &&
	    can_carry(unit, unit) && !might_be_captured(unit)) {
	    for_all_material_types(r) {
		/* could also add in distance calculation to see how much we */
		/* really need. */
		if (unit->supply[r] < um_storage(unit->type, r)) {
		    return FALSE;
		}
	    }
	    if ((route = find_route(unit, route_max_distance, x, y))) {
    	clear_task_agenda(unit->plan);
		unit->plan->tasks = route;
		return TRUE;
	    }
	}
    } 
    return FALSE;
}

/* See if the location has a unit that can take us in for refueling */
/* (where's the check for refueling ability?) */

/* Is doing the side-effect a good idea here? */

haven_p(unit, x, y)
Unit *unit;
int x, y;
{
    Unit *unit2 = unit_at(x, y);
    Task *route;

    if (unit != NULL &&
	allied_side(unit->side, unit2->side) && alive(unit) &&
	can_carry(unit, unit2) && !might_be_captured(unit2) &&
	((route = find_route(unit, route_max_distance, x, y)))) {
    clear_task_agenda(unit->plan);
	unit->plan->tasks = route;
	return TRUE;
    } else
      return FALSE;
}

/* See if the location has a unit that can repair us */

shop_p(unit, x, y)
Unit *unit;
int x, y;
{
    Unit *unit2 = unit_at(x, y);
    Task *route;

    if (unit != NULL && allied_side(unit->side, unit2->side) && alive(unit) &&
	can_carry(unit, unit2) && could_repair(unit2->type, unit->type) &&
	!might_be_captured(unit2) &&
	((route = find_route(unit, route_max_distance, x, y)))) {
    clear_task_agenda(unit->plan);
	unit->plan->tasks = route;
	return TRUE;
    } else
      return FALSE;
}

/* Check how long a unit can sit where it is */

int
survival_time(unit)
Unit *unit;
{
    int u = unit->type, m, least = 99999, rate, tmp;
    int t = terrain_at(unit->x, unit->y);

    for_all_material_types(m) {
	rate = (um_consume(u, m)
		- (um_produce(u, m) * ut_productivity(u, t)) / 100);
	if (rate > 0) {
	    tmp = unit->supply[m];
	    if (unit->act) {
		tmp += unit->act->actualmoves * um_tomove(u, m);
	    }
	    least = min(least, tmp / rate);
	}
    }
    return least;
}

long regions_around(u, x, y, center)
int u, x, y;
int center;
{
#if 0
    int d, nx, ny;
    long result = 0;

    for_all_directions(d) {
	nx = wrap(x + dirx[d]);  ny = y + diry[d];
	if (aref(unit_region[u], nx, ny) >= 0)
	  result |= 1 << (aref(unit_region[u], nx, ny) % sizeof(long));
    }
    if (center && aref(unit_region[u], x, y) > 0)
      result |= 1 << (aref(unit_region[u], x, y) % sizeof(long));
    return result;
#endif
}

/* Make a single step of a movement chain. */

/* what to do with priority? */

Task *
make_route_step(x, y, priority)
int x, y, priority;
{
    Task *task = (Task *) create_moveto_task(NULL, x, y);

    return task;
}
  
/* Build the route from the chain of arrows left on the fromdir map. */

Task *
make_route_chain(sx, sy, tx, ty)
int sx, sy, tx, ty;
{
    Task *chain = NULL, *new;
    int x = tx, y = ty, d;

#if 0
    /* garbled here */
    while (x != sx || y != sy) {
	new = make_route_step(x, y, bestworth);
	return new;
	new->next = chain;
	chain = new;
	d = get_fromdir(x, y);
	x = wrap(x + dirx[d]);	y = y + diry[d];
    }
#endif
    return chain;
}  

/* should be using A* rather than this I think.? */

unsigned char mark = 255;
int counter = 0;

Task *
find_route_aux(unit, maxdist, curlen, fromdir, sx, sy, fx, fy, tx, ty, flags)
Unit *unit;
int maxdist, curlen, fromdir, sx, sy, fx, fy, tx, ty, flags;
{
    int u = unit->type;
    int d, i, x, y, td;
    int view;
    int baddirs;
    int distleft = distance(fx, fy, tx, ty);
    Task *chain;

    /* Init the repeated-path prevention */
    if (!(flags & SAMEPATH)) {
	baddirs = 0;
	mark++;
	if (mark == 0) {
	    for_all_hexes(x, y) markloc(x, y);
	    mark++;
	}
    } else {
	baddirs =
	  (1 << fromdir)
	    | (1 << right_dir(fromdir))
	      | (1 << left_dir(fromdir));
    }
    flags |= SAMEPATH; /* add samepath flag */
    markloc(fx, fy);
    set_fromdir(fx, fy, fromdir);
    /* Oops, we've wandered too far already - give up. */
    if (curlen + distleft > maxdist) {
	return NULL;
    }
/*    set_dist(fx, fy, curlen);  */
    curlen++;
    /* Yay, we're there!  Make a chain... */
    if (fx == tx && fy == ty)
      return make_route_chain(sx, sy, tx, ty);
    
    d = fromdir;
    do {
	d = right_dir(d);
	x = wrap(fx + dirx[d]);  y = fy + diry[d];
    } while (distance(x, y, tx, ty) >= distleft);
    
    td = left_dir(d);
    if (distance(wrap(fx + dirx[td]), fy + diry[td], tx, ty) < distleft) {
	d = td;
    }
    
    x = wrap(fx + dirx[d]);  y = fy + diry[d];
#if 0
    if ((flags & EXPLORE_PATH) && UNSEEN == unit_view(unit->side, x, y)) {
	return make_route_chain(sx, sy, fx, fy);
    }
#endif
    
    for (i = 0; i <= 3; ++i) {
	/* Look to the right */
	td = (d + i + 6) % 6;
	x = wrap(fx + dirx[td]);  y = fy + diry[td];
	if ((x == tx && y == ty) ||
	    (inside_area(x, y)
	     && !markedloc(x, y)
	     && (baddirs & (1 << td)) == 0
	     && usable_hex(unit, x, y))) {
	    chain = find_route_aux(unit, maxdist, curlen, opposite_dir(td),
				   sx, sy, x, y, tx, ty, flags);
	    if (chain != NULL)
	      return chain;
	}
	/* Look to the left */
	td = (d - i + 6) % 6;
	x = wrap(fx + dirx[td]);  y = fy + diry[td];
	if ((x == tx && y == ty) ||
	    (inside_area(x, y)
	     && between(1, i, 2)
	     && !markedloc(x, y)
	     && (baddirs & (1 << td)) == 0
	     && usable_hex(unit, x, y))) {
	    chain = find_route_aux(unit, maxdist, curlen, opposite_dir(td),
				   sx, sy, x, y, tx, ty, flags);
	    if (chain != NULL)
	      return chain;
	}
    }
    return NULL;
}

usable_hex(unit, x, y)
Unit *unit;
int x, y;
{
    int u = unit->type;
    int view = unit_view(unit->side, x, y);
    
    return (((view == EMPTY) || view == UNSEEN ||
	    (allied_side(side_n(vside(view)),unit->side) &&
	     could_carry(vtype(view), u))) &&
	       could_move(u, terrain_at(x, y)));
}

/* Given a unit, a destination, and a max allowed distance,
   compute a plausible route as a list of tasks and return it. */

Task *
find_route(unit, maxdist, tx, ty)
Unit *unit;
int maxdist, tx, ty;
{
    int u = unit->type;
    int fx = unit->x, fy = unit->y;
    int mindist = distance(fx, fy, tx, ty), i, flags;
    Task *route;

    DMprintf("%s finding route; %d,%d to %d,%d (%d < dist < %d)\n",
	     unit_desig(unit), fx, fy, tx, ty, mindist, maxdist);
    if (mindist > maxdist) {
	run_warning("can't possibly find a route!\n");
    }
#if 0 /* def REGIONS */
    if ((regions_around(u, tx, ty, TRUE) &
	 regions_around(u, fx, fy, FALSE)) == 0)
      {
	  int d, x, y;
	  Unit *base;
	  int basefound = FALSE;

	  for_all_directions(d) {
	      x = wrap(fx + dirx[d]);  y = limit(fy + diry[d]);
	      if ((base = unit_at(x, y)) != NULL &&
		  could_carry(base->type, u))
		basefound = TRUE;
	  }
	  if (!basefound) {
	      return NULL;
	  }
      }
#endif
#if 0
    /* we might have to be exploring to make it */
    if (UNSEEN == unit_view(unit->side, tx, ty))
      flags = EXPLORE_PATH;
#endif

    /* Try to find a route, gradually relaxing the distance constraint. */
    for (i = 0; (mindist + i * i) <= maxdist; i++) {
	route = find_route_aux(unit, min(maxdist, ((i+1)*(i+1)+mindist)),
			       0, -1, fx, fy,
			       fx, fy, tx, ty, flags);
	/* If we got something, return it. */
	if (route != NULL) {
	    return route;
	}
    }
    return NULL;
}

Task *
find_route_aux_nearest(unit, maxdist, curlen, fromdir, sx, sy, fx, fy, pathpred, destpred, flags)
Unit *unit;
int maxdist, curlen, fromdir, sx, sy, fx, fy, flags;
int (*pathpred)(), (*destpred)();
{
#if 0
    int u = unit->type;
    int d, i, x, y, td;
    int view;
    int baddirs;
    Task *chain;

    /* Init the repeated-path prevention */
    if (!(flags & SAMEPATH)) {
	baddirs = 0;
	mark++;
	if (mark == 0) {
	    for_all_hexes(x, y) markloc(x, y);
	    mark++;
	}
    } else {
	baddirs =
	  (1 << fromdir)
	    | (1 << right_dir(fromdir))
	      | (1 << left_dir(fromdir));
    }
    flags |= SAMEPATH; /* add samepath flag */
    markloc(fx, fy);
    set_fromdir(fx, fy, fromdir);
    /* Oops, we've wandered too far already - give up. */
    if (curlen > maxdist) {
	return NULL;
    }
/*    set_dist(fx, fy, curlen);  */
    curlen++;
    /* Yay, we're there!  Make a chain... */
    if ((*destpred)(unit, fx, fy)) {
	return make_route_chain(sx, sy, tx, ty);
    }
    
    d = fromdir;
    do {
	d = right_dir(d);
	x = wrap(fx + dirx[d]);  y = fy + diry[d];
    } while (distance(x, y, tx, ty) >= distleft);
    
    td = left_dir(d);
    if (distance(wrap(fx + dirx[td]), fy + diry[td], tx, ty) < distleft) {
	d = td;
    }
    
    x = wrap(fx + dirx[d]);  y = fy + diry[d];
#if 0
    if ((flags & EXPLORE_PATH) && UNSEEN == unit_view(unit->side, x, y)) {
	return make_route_chain(sx, sy, fx, fy);
    }
#endif
    
    for (i = 0; i <= 3; ++i) {
	/* Look to the right */
	td = (d + i + 6) % 6;
	x = wrap(fx + dirx[td]);  y = fy + diry[td];
	if ((x == tx && y == ty) ||
	    (inside_area(x, y)
	     && !markedloc(x, y)
	     && (baddirs & (1 << td)) == 0
	     && usable_hex(unit, x, y))) {
	    chain = find_route_aux_nearest(unit, maxdist, curlen, opposite_dir(td),
				   sx, sy, x, y, tx, ty, flags);
	    if (chain != NULL)
	      return chain;
	}
	/* Look to the left */
	td = (d - i + 6) % 6;
	x = wrap(fx + dirx[td]);  y = fy + diry[td];
	if ((x == tx && y == ty) ||
	    (inside_area(x, y)
	     && between(1, i, 2)
	     && !markedloc(x, y)
	     && (baddirs & (1 << td)) == 0
	     && usable_hex(unit, x, y))) {
	    chain = find_route_aux_nearest(unit, maxdist, curlen, opposite_dir(td),
				   sx, sy, x, y, tx, ty, flags);
	    if (chain != NULL)
	      return chain;
	}
    }
#endif
    return NULL;
}

/* Given a unit, a destination, and a max allowed distance,
   compute a plausible route as a list of tasks and return it. */

Task *
find_route_to_nearest(unit, fx, fy, maxdist, pathpred, destpred)
Unit *unit;
int fx, fy, maxdist;
int (*pathpred)(), (*destpred)();
{
    int u = unit->type;
    int mindist = 1;
    int flags;
    Task *route;

    DMprintf("%s finding route; %d,%d to ? (%d < dist < %d)\n",
	     unit_desig(unit), fx, fy, mindist, maxdist);
    if (mindist > maxdist) {
	run_warning("can't possibly find a route!\n");
    }
    route = find_route_aux(unit, maxdist,
			   0, -1, fx, fy,
			   fx, fy, pathpred, destpred, flags);
    /* If we got something, return it. */
    if (route != NULL) {
	return route;
    }
    return NULL;
}

Task *explorechain;

explorable_hex(x, y)
int x, y;
{
    return (terrain_view(tmpside, x, y) == UNSEEN);
}

reachable_unknown(x, y)
int x, y;
{
    if (!inside_area(x, y)) return FALSE;
    if (terrain_view(tmpside, x, y) == UNSEEN) {
    	if (adj_known_ok_terrain(x, y, tmpside, tmpunit->type)) {
	    return TRUE;
	} else {
	    return FALSE;
	}
    } else {
	return FALSE;
    }
}

/* Test whether the given location has an adjacent cell that is ok for
   the given type to be out in the open. */

adj_known_ok_terrain(x, y, side, u)
int x, y, u;
Side *side;
{
	int dir, x1, y1, t;

	if (!inside_area(x, y) || side == NULL) return FALSE;
	for_all_directions(dir) {
		if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		    if (terrain_view(side, x1, y1) == UNSEEN) continue;
		    t = terrain_at(x1, y1);
		    if (!terrain_always_impassable(u, t)) return TRUE;
		}
	}
	return FALSE;
}

/* Go to the nearest hex that we can see how to get to. */

explore_reachable_hex(unit, range)
Unit *unit;
int range;
{
    int x, y;

    if (g_see_all() || g_terrain_seen()) return FALSE;
    tmpunit = unit;
    tmpside = unit->side;
    DMprintf("%s searching within %d for cell to explore -", unit_desig(unit), range);
    if (search_around(unit->x, unit->y, range, reachable_unknown, &x, &y, 1)) {
	push_movenear_task(unit, x, y, 1);
	DMprintf("found one at %d,%d\n", x, y);
	return TRUE;
    }
    DMprintf("found nothing\n");
    return FALSE;
}

/* Optimize a plan to account for base building and running out of
supplies. */

optimize_plan(unit)
Unit *unit;
{
}

/* Check for any makers this unit should be capturing. */

should_capture_maker(unit)
Unit *unit;
{
}

/* Returns true if the given unit can't leave its hex for some reason. */

no_possible_moves(unit)
Unit *unit;
{
    int fx = unit->x, fy = unit->y, ut = unit->type;
    int d, x, y;
    int view;
    Side *side = unit->side;

    for_all_directions(d) {
	x = wrap(fx + dirx[d]);  y = limit(fy + diry[d]);
	view = unit_view(side, x, y);
	if (view == EMPTY) {
	    if (could_move(ut, terrain_at(x, y)))
	      return FALSE;
	} else if (enemy_side(side_n(vside(view)) , side)
		   && could_hit(ut, vtype(view))) {
	    return FALSE;
	} else if (could_carry(vtype(view), ut) &&
		   allied_side(side_n(vside(view)), side))
	  return FALSE;
    }
    return TRUE;
}

adj_known_passable(side, x, y, u)
Side *side;
int x, y, u;
{
    int dir;

    for_all_directions(dir) {
	if (unit_view(side, wrapx(x + dirx[dir]), y + diry[dir]) != UNSEEN
	    && could_occupy(u, terrain_at(wrapx(x + dirx[dir]), y + diry[dir])))
	  return TRUE;
    }
    return FALSE;
}

adj_obstacle(type, x, y)
int type, x, y;
{
    int d, x1, y1;

    for_all_directions(d) {
	x1 = wrap(x + dirx[d]);  y1 = limit(y + diry[d]);
	if (!could_move(type, terrain_at(x1, y1))) return TRUE;
    }
    return FALSE;
}

/* Estimate the usual number of turns to finish construction. */

normal_completion_time(u, u2)
int u, u2;
{
	if (u_acp(u) == 0 || uu_cp_per_build(u, u2) == 0) return (-1);
	return (u_cp(u2) - uu_creation_cp(u, u2)) / (uu_cp_per_build(u, u2) * u_acp(u));
}


/* True if anybody at all is on any adjacent hex. */

adj_unit(x, y)
int x, y;
{
    int d, x1, y1;

    for_all_directions(d) {
	if (interior_point_in_dir(x, y, d, &x1, &y1)) {
	    if (unit_at(x1, y1)) return TRUE;
	}
    }
    return FALSE;
}

/* A unit runs low on supplies at the halfway point.  Formula is the same no matter
   how/if occupants eat transports' supplies. */

past_halfway_point(unit)
Unit *unit;
{
    int u = unit->type, m;

    for_all_material_types(m) {
	if (((um_consume(u, m) > 0) || (um_tomove(u, m) > 0)) &&
	    /* should check that the transport is adequate for */
	    /* supplying the fuel */ 
	    (unit->transport == NULL)) {
	    if (2 * unit->supply[m] <= um_storage(u, m)) return TRUE;
	}
    }
    return FALSE;
}


