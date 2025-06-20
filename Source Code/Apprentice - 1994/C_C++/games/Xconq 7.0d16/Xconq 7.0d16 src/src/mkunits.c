/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

#include "conq.h"

Unit *find_occupant_place();

/* Tmp array for counting terrain. */

int *numcells;

int *favorite;

int *totnumcells;

/* This is the number of country placements that could not be found to meet
   all the constraints. */

int badcountryplaces = 0;

/* This says whether terrain alteration is permissible to make country
   placements work.  This only happens when normal placement starts to fail. */

int mungterrain;

int tmpradius;

int curmindistance;
int curmaxdistance;

int sideprogress;
int sidedeltahalf;

int growth;

/* Set the people inside the country to be on the country's side.
   Flip a coin along the edges, to make the border more interesting.
   Also don't always overwrite existing people. */
 
set_people_on_side(x, y)
int x, y;
{
    if ((distance(tmpside->startx, tmpside->starty, x, y) <= tmpradius
         || flip_coin())
        && probability(t_country_people(terrain_at(x, y)))
        && (people_side_at(x, y) == NOBODY || flip_coin())) {
	set_people_side_at(x, y, side_number(tmpside));
    }
}

/* Expand a country into the given cell. */

expand_country(x, y)
int x, y;
{
    int u, t = terrain_at(x, y);
    Unit *unit;

    if (people_side_at(x, y) == NOBODY) {
	if (probability(t_country_growth(t))) {
	    for_all_unit_types(u) {
	    	tmputype = u;
		if (u_unit_growth(u) > xrandom(10000)
		    && probability(ut_favored(u, t))
		    && valid_unit_place(x, y)) {
		    if ((unit = create_unit(u, TRUE)) != NULL) {
			set_unit_side(unit, tmpside);
			enter_hex(unit, x, y);
		    	break;
		    }
		}
		if (u_indep_growth(u) > xrandom(10000)
		    && probability(ut_favored(u, t))
		    && valid_unit_place(x, y)) {
		    if ((unit = create_unit(u, TRUE)) != NULL) {
			enter_hex(unit, x, y);
		    	break;
		    }
		}
	    }
	    if (probability(t_country_people(t))) {
		set_people_side_at(x, y, side_number(tmpside));
	    }
	    ++growth;
	}
    } else {
	if (probability(t_country_takeover(t))) {
	    if (probability(t_country_people(t))) {
		set_people_side_at(x, y, side_number(tmpside));
	    }
	    /* look at stack and grab units in it */
	    ++growth;
	}
    }
}

#define announce_unit_progress(n) \
  announce_progress(sideprogress + sidedeltahalf + (sidedeltahalf * (n)) / totnumtodo)

/* Place all the units belonging to countries. */

make_countries()
{
    int x0, y0, u, t, i, tot, num, x, y, advantage, sideadvantage, favor, x1, y1, dir;
    int canbeinopen[MAXUTYPES];  /* true if type need not be occ at start */
    int numtodo[MAXUTYPES], numindeptodo[MAXUTYPES], totnumtodo, totnumdone;
    int numleft[MAXUTYPES], numindepleft[MAXUTYPES], totleft, numlisted, numfails;
    int checkmins = FALSE;
    int dopeoplesides = FALSE, dopeopleindeps = FALSE;
    int maxradius;
    int numdone = 0;
    Unit *unit, *transport;
    Side *side;
    char tmpbuf2[BUFSIZE];

    /* Run this always, unless something important is missing. */
    if (!terrain_defined()) return;
    /* Calculate whether the minimum required terrain types are present. */
    totnumcells = (int *) xmalloc(numttypes * sizeof(int));
    for_all_terrain_types(t) {
    	if (t_country_min(t) > 0) checkmins = TRUE;
    }
    if (checkmins) {
    	/* Edge cells are useless for country placement. */
    	for_all_interior_hexes(x, y) {
    	    ++totnumcells[terrain_at(x, y)];
    	}
    	for_all_terrain_types(t) {
    	    if (t_country_min(t) * numsides > totnumcells[t]) {
    	    	init_warning("Not enough %s for all %d sides",
			     t_type_name(t), numsides);
    	    	/* Don't error out, might be extenuating circumstances. */
    	    }
    	}
    }
    announce_lengthy_process("Making countries");
    /* Precompute some important info */
    numcells = (int *) xmalloc(numttypes * sizeof(int));
    favorite = (int *) xmalloc(numutypes * sizeof(int));
    tot = 0;
    for_all_unit_types(u) {
	canbeinopen[u] = FALSE;
	favorite[u] = NONTTYPE;
	favor = 0;
	for_all_terrain_types(t) {
	    if (ut_favored(u, t) > 0) canbeinopen[u] = TRUE;
	    if (ut_favored(u, t) > favor) {
		favorite[u] = t;
		favor = ut_favored(u, t);
	    }
	}
	if (canbeinopen[u]) {
	    tot += u_start_with(u) + u_indep_near_start(u);
	}
    }
    /* Make space for people sides if we're going to have any. */
    if (!people_sides_defined()) {
	for_all_terrain_types(t) {
	    if (t_country_people(t) > 0) dopeoplesides = TRUE;
	    if (t_indep_people(t) > 0) dopeopleindeps = TRUE;
	}
	if (dopeoplesides || dopeopleindeps) {
	    allocate_area_people_sides();
	}
    }
    /* If no radius specified, pick something plausible. */
    if (g_min_radius() <= 0) {
	tmpradius = max(1, isqrt(3 * tot) / 2);
    } else {
	tmpradius = g_min_radius();
    }
    curmindistance = g_min_separation();
    curmaxdistance = g_max_separation();
    badcountryplaces = 0;
    for_all_sides(side) {
	sideprogress = (100 * numdone++) / numsides;
	sidedeltahalf = (100 / numsides) / 2;
	announce_progress(sideprogress);
	if (!country_is_complete(side)) {
	    sideadvantage = max(1, side->advantage);
	    advantage = (side->player ? side->player->advantage : sideadvantage);
	    /* Discover or generate the country's center. */
	    mungterrain = FALSE;
	    find_a_place(side);
	    x0 = side->startx;  y0 = side->starty;
	    Dprintf("%s starts around %d,%d\n", side_desig(side), x0, y0);
	    announce_progress(sideprogress + sidedeltahalf);
	    totnumtodo = totnumdone = 0;
	    for_all_unit_types(u) {
		numtodo[u] = numindeptodo[u] = 0;
		if (type_allowed_on_side(u, side)) {
		    numtodo[u] = (u_start_with(u) * advantage) / sideadvantage;
		    totnumtodo += numtodo[u];
		}
		if (type_allowed_on_side(u, NULL)) {
		    numindeptodo[u] = (u_indep_near_start(u) * advantage) / sideadvantage;
		    totnumtodo += numindeptodo[u];
		}
		numleft[u] = numindepleft[u] = 0;
	    }
	    /* First do units actually belonging to the side initially. */
	    for_all_unit_types(u) {
		if (canbeinopen[u]) {
		    for (i = 0; i < numtodo[u]; ++i) {
			if (find_unit_place(u, x0, y0, &x, &y)) {
			    if ((unit = create_unit(u, TRUE)) != NULL) {
				set_unit_side(unit, side);
				enter_hex(unit, x, y);
	   			announce_unit_progress(++totnumdone);
			    }
			} else {
			    /* If can't find places for this type, give up. */
		            numleft[u] = numtodo[u] - i;
			    break;
			}
		    }
		}
	    }
	    /* Now do independents in the initial country area. */
	    for_all_unit_types(u) {
		for (i = 0; i < numindeptodo[u]; ++i) {
		    if (find_unit_place(u, x0, y0, &x, &y)) {
			if ((unit = create_unit(u, TRUE)) != NULL) {
			    enter_hex(unit, x, y);
			    announce_unit_progress(++totnumdone);
			}
		    } else {
			/* If can't find places for this type, give up. */
			numindepleft[u] = numindeptodo[u] - i;
			break;
		    }
		}
	    }
	    /* Now do units that have to be occupants.  Note that if occupants
	       are allowed in independent units, then they might be
	       placed in one. */
	    for_all_unit_types(u) {
		if (!canbeinopen[u]) {
		    for (i = 0; i < numtodo[u]; ++i) {
			if ((transport = find_occupant_place(side, u, x0, y0))
			    != NULL) {
			    if ((unit = create_unit(u, TRUE)) != NULL) {
				set_unit_side(unit, side);
				enter_transport(unit, transport);
	   			announce_unit_progress(++totnumdone);
			    }
			} else {
		   	    numleft[u] = numtodo[u] - i;
			    break;
			}
		    }
		}
	    }
	    /* Now warn about what couldn't be placed. */
	    totleft = numlisted = numfails = 0;
	    sprintf(tmpbuf2, "");
	    for_all_unit_types(u) {
		totleft += numleft[u];
		if (numleft[u] > 0 && numlisted < 5) {
		    ++numfails;
		    strcat(tmpbuf2, " ");
		    strcat(tmpbuf2, u_type_name(u));
		}
	    }
	    if (totleft > 0) {
		char tmpbuf3[BUFSIZE];
		char *shortest_side_title();

		init_warning("could not put %d units in %s country (%s%s)",
			     totleft, shortest_side_title(side, tmpbuf3),
			     tmpbuf2, (numfails >= 5 ? " etc" : ""));
	    }
	}
	/* Now set the side of the people in this country. */
	tmpside = side;
	apply_to_area(side->startx, side->starty, tmpradius + 1,
		      set_people_on_side);
	/* Make sure each unit has people there, if allowed at all. */
	for_all_side_units(side, unit) {
	    if (inside_area(unit->x, unit->y)
	        && t_country_people(terrain_at(unit->x, unit->y)) > 0) {
		set_people_side_at(unit->x, unit->y, side_number(side));
	    }
	}
    }
    finish_lengthy_process();
    /* Grow each country out to a maximum radius. */
    announce_lengthy_process("Growing countries");
    maxradius = min(area.width, g_radius_max());
    for_all_sides(side) side->finalradius = maxradius;
    for (i = tmpradius; i < maxradius; ++i) {
    	announce_progress((100 * (i - tmpradius)) / (maxradius - tmpradius));
	for_all_sides(side) {
	    /* If side still allowed to grow, expand it. */
	    if (side->finalradius == maxradius) {
	    	tmpside = side;
	    	growth = 0;
	    	apply_to_ring(side->startx, side->starty, i - 2, i,
			      expand_country);
		/* If no actual growth happened in a step, the country
		   might have reached its natural size. */
	    	if (growth == 0 && probability(g_growth_stop())) {
		    side->finalradius = i;
	    	}
	    }
	}
    }
    /* Do a couple "consolidation" steps to fill in "jaggies". */
    for (i = 0; i < 2; ++i) {
	for_all_sides(side) {
	    tmpside = side;
	    apply_to_ring(side->startx, side->starty,
			  tmpradius, side->finalradius - 1,
			  expand_country);
	}
    }
    /* Also remove isolated populations surrounded by one other side. */
    if (people_sides_defined()) {
	for_all_interior_hexes(x, y) {
	    int pop, pop2, majoritypop;
	    
	    if ((pop = people_side_at(x, y)) != NOBODY) {
		majoritypop = NOBODY;
		for_all_directions(dir) {
		    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			if ((pop2 = people_side_at(x1, y1)) == pop)
			  goto nextcell;
			if (majoritypop == NOBODY) {
			    majoritypop = pop2;
			} else {
			    if (pop2 != majoritypop) goto nextcell;
			}
		    }
		}
		if (majoritypop != NOBODY) {
		    set_people_side_at(x, y, majoritypop);
		}
	    }
	  nextcell:
	    pop = pop;
	}
    }
    finish_lengthy_process();
    /* Warn about any difficulties encountered. */
    if (badcountryplaces > 0) {
    	init_warning("%d of %d sides have undesirable locations",
		     badcountryplaces, numsides);
    }
}

/* Test to see whether the side's pre-existing setup already suffices
   as a country in this game. */

country_is_complete(side)
Side *side;
{
    int u, numunits[MAXUTYPES];
    Unit *unit;
    int sideadvantage = max(1, side->advantage);
    int advantage = (side->player ? side->player->advantage : sideadvantage);

    for_all_unit_types(u) numunits[u] = 0;
    /* Count up all and only the completed and present units. */
    for_all_side_units(side, unit) {
    	if (in_play(unit)) {
	    ++numunits[unit->type];
	}
    }
    for_all_unit_types(u) {
	if (numunits[u] < ((u_start_with(u) * advantage) / sideadvantage)) return FALSE;
    }
    return TRUE;
}

/* Count the hex as being of a particular type. */

count_hexes(x, y)
int x, y;
{
    ++numcells[terrain_at(x, y)];
}

/* Test whether a given location is desirable for a country.  It should be
   in the right distance range from other countries, and have enough of
   the right sorts of terrain. */

good_place(cx, cy)
int cx, cy;
{
    int toofar = TRUE, notfirst = FALSE;
    int c, px, py, u, t;
    Side *side;

    /* Check the candidate position against the other countries' positions. */
    for_all_sides(side) {
	px = side->startx;  py = side->starty;
	if (inside_area(px, py)) {
	    notfirst = TRUE;
	    /* Min separation default allows any min separation. */
	    if (distance(cx, cy, px, py) < curmindistance) return FALSE;
	    /* Default max separation says pos can never be too far away. */
	    if (curmaxdistance < 0
	        || distance(cx, cy, px, py) < curmaxdistance) toofar = FALSE;
	}
    }
    if (toofar && notfirst) return FALSE;
    /* Count the types of terrain in the country. */
    for_all_terrain_types(t) numcells[t] = 0;
    apply_to_area(cx, cy, tmpradius, count_hexes);
    /* Check against our upper and lower limits on terrain. */
    for_all_terrain_types(t) {
    	if (numcells[t] < t_country_min(t)) return FALSE;
	if (t_country_max(t) >= 0
	    && numcells[t] > t_country_max(t)) return FALSE;
    }
    return TRUE;
}

/* Work hard to find a place for a side's country.  First make some random
   trials, then start searching from the center of the area outwards.
   Then just pick a place and plan to patch up the terrain later. */

find_a_place(side)
Side *side;
{
    int tries, x, y, maxtries = area_cells() / 5;
    Unit *unit;

    Dprintf("%s country ", side_desig(side));
    /* First see if any of our units is already at a good location
       to put the whole country. */
    /* (should choose randomly from units first?) */
    for_all_side_units(side, unit) {
	if (in_play(unit) && good_place(unit->x, unit->y)) {
	    side->startx = unit->x;  side->starty = unit->y;
	    Dprintf("placed at unit %s\n", unit_desig(unit));
	    return;
	}
    }
    /* Then try any of the independent units.  Probability of choosing
       one is inversely proportional to the total number of units. */
    for_all_side_units(indepside, unit) {
	if (in_play(unit)
	    && good_place(unit->x, unit->y)
	    && probability(max(1, 100 / numunits))) {
	    side->startx = unit->x;  side->starty = unit->y;
	    Dprintf("placed at indep unit %s\n", unit_desig(unit));
	    return;
	}
    }
    /* Then try some random locations. */
    for (tries = 0; tries < maxtries; ++tries) {
	if (tries % 10 == 0) {
	    announce_progress(sideprogress +
			      ((sidedeltahalf / 2) * tries) / maxtries);
	}
	random_point(&x, &y);
	/* Filter out points that are right on the edge.  If that's
	   where the only valid starting places are, the exhaustive
	   search will still find them. */
	if (between(2, y, area.height - 2)
	    && between(2, x, area.width - 2)
	    && good_place(x, y)) { 
	    side->startx = x;  side->starty = y;
	    Dprintf("placed on try %d\n", tries);
	    return;
	}
    }
    /* Then search exhaustively, starting from the center of the area. */
    if (search_around(area.width / 2, area.height / 2,
		      area.width / 2 + area.height / 2,
		      good_place, &x, &y, 1)) {
	side->startx = x;  side->starty = y;
	Dprintf("placed after search\n");
	return;
    }
    /* This should be a place in the area no matter what. */
    random_point(&x, &y);
    side->startx = x;  side->starty = y;
    Dprintf("placed randomly\n");
    ++badcountryplaces;
    /* Since placement has become difficult, we get permission to alter
       the terrain if necessary, while placing units. */
    mungterrain = TRUE;
    return;
}    

/* The basic conditions that *must* be met by an initial unit placement. */

valid_unit_place(x, y)
int x, y;
{
    int t = terrain_at(x, y);

    return (inside_area(x, y)
        && ut_favored(tmputype, t) > 0
        && !ut_vanishes_on(tmputype, t)
        && !ut_wrecks_on(tmputype, t)
        && type_can_occupy_cell(tmputype, x, y));
}

possible_unit_place(x, y)
int x, y;
{
    return (inside_area(x, y) && unit_at(x, y) == NULL);
}

/* Find a place somewhere in the designated area, following constraints
   on terrain and adjacency.  Returns success of placement. */

find_unit_place(u, cx, cy, xp, yp)
int u, cx, cy, *xp, *yp;
{
    int t, tries, x, y, dir, x1, y1;
    int maxtries = tmpradius * tmpradius * 5;

    Dprintf("%s place found ", u_type_name(u));
    tmputype = u;
    for (tries = 0; tries < maxtries; ++tries) {
	if (random_point_near(cx, cy, tmpradius, &x, &y)) {
	    if (mungterrain
	        && !valid_unit_place(x, y)
	        && possible_unit_place(x, y)
		&& favorite[u] != NONTTYPE
		&& probability(ut_favored(u, favorite[u]))) {
		Dprintf("(by munging) ");
		mung_terrain(x, y, u);
	    }
	    if (valid_unit_place(x, y)
	        && probability(ut_favored(u, terrain_at(x, y)))) {
	    	*xp = x;  *yp = y;
		Dprintf("on try %d of %d\n", tries, maxtries);
		return TRUE;
	    }
	}
    }
    /* Random points aren't working, switch to exhaustive search. */
    if (search_around(cx, cy, tmpradius, valid_unit_place, &x, &y, 1)) {
	*xp = x;  *yp = y;
	Dprintf("after search\n");
	return TRUE;
    }
    /* Search again, just find a location that we can alter to suit. */
    if (favorite[u] != NONTTYPE) {
        if (search_around(cx, cy, tmpradius, possible_unit_place, &x, &y, 1)) {
            mung_terrain(x, y, u);
	    *xp = x;  *yp = y;
	    Dprintf("(by munging) after search\n");
	    return TRUE;
	}
    }
    Dprintf("- NOT!\n");
    return FALSE;
}

mung_terrain(x, y, u)
int x, y, u;
{
    int dir, x1, y1;

    /* Alter the terrain to be compatible with the given unit type. */
    set_terrain_at(x, y, favorite[u]);
    /* Mung some empty adjacent cells also, improves appearance. */
    for_all_directions(dir) {
	if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
	    if (unit_at(x1, y1) == NULL && flip_coin()) {
		set_terrain_at(x1, y1, favorite[u]);
	    }
	}
    }
}

Unit *
find_occupant_place(side, u, x0, y0)
Side *side;
int u, x0, y0;
{
    Unit *transport;

    /* (should cast about randomly first, then do full search) */
    for_all_side_units(side, transport) {
	if (is_unit(transport)
	    && inside_area(transport->x, transport->y)
	    && type_can_occupy(u, transport)) {
	    return transport;
	}
    }
    /* (should be able to look at indep units in country) */
    return NULL;
}

/* A method that scatters independent units over the world. */

make_independent_units()
{
    int u, t, x, y;
    int doindeptype[MAXUTYPES], doanytype = FALSE, doindeppeople = FALSE;
    Unit *unit;

    /* Can't do anything without some terrain. */
    if (!terrain_defined()) return;
    /* Decide which types will be put down. */
    for_all_unit_types(u) {
    	doindeptype[u] = FALSE;
    	for_all_terrain_types(t) {
    	    if (ut_indep_density(u, t) > 0 && !ut_vanishes_on(u, t)) {
    	    	doindeptype[u] = TRUE;
    	    	doanytype = TRUE;
    	    	break;
    	    }
    	}
    }
    /* Make space for people sides if we're going to have any. */
    if (!people_sides_defined()) {
	for_all_terrain_types(t) {
	    if (t_indep_people(t) > 0) doindeppeople = TRUE;
	}
	if (doindeppeople) {
	    allocate_area_people_sides();
	}
    }
    /* If no units or peoples to do, don't waste time going through
       the world. */
    if (!doanytype && !doindeppeople) return;
    announce_lengthy_process("Making independents");
    /* Now apply the process to each hex individually. */
    for_all_interior_hexes(x, y) {
    	/* Progress is approx proportional to "column" being worked on. */
    	if (y == 1 && x % 5 == 0) announce_progress((x * 100) / area.width);
	t = terrain_at(x, y);
	for_all_unit_types(u) {
	    if (doindeptype[u]) {
		if (ut_indep_density(u, t) > xrandom(10000)) {
	    	    if (!ut_vanishes_on(u, t)
			&& type_can_occupy_cell(u, x, y)) {
			if ((unit = create_unit(u, TRUE)) != NULL) {
			    enter_hex(unit, x, y);
		    	}
		    }
		}
	    }
	}
	if (doindeppeople
	    /* Don't put indeps on top of people already on a side. */
	    && people_side_at(x, y) == NOBODY
	    && probability(t_indep_people(t))) {
	    /* 0 represents indep people present */
	    set_people_side_at(x, y, 0);
	}
    }
    finish_lengthy_process();
}
