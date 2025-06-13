/* Implementation of the "mplayer" AI in Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"
#include "mplay.h"

#define unit_theater(unit) ((Theater *) (unit)->aihook)

#define set_unit_theater(unit,theater) ((unit)->aihook = (char *) (theater))

#define can_see_actual_units(side, x, y) (g_see_all() || cover((side), (x), (y)) > 0)

static int compare_weights PROTO ((const void *w1, const void *w2));

/* Flag to detect when shared mplayer init has been done. */

static int mplayerinited = FALSE;

void
mplayer_init(side)
Side *side;
{
    Unit *unit;

    /* (should do this only when absolutely needed - mplayer might
       never actually be used) */
    if (!mplayerinited) {
	mplayer_init_shared();
	mplayerinited = TRUE;
	Dprintf("One mplayer AI is %d bytes.\n", sizeof(Strategy));
    }
    /* Make sure a strategy object exists. */
    if (mplayer(side) == NULL)
      create_strategy(side);
    /* If the side has no units at the moment, it doesn't really need to
       plan. */
    if (!side_has_units(side))
      return;
    analyze_the_game(side);
    /* Reset plans of any units that were not doing anything. */
    for_all_side_units(side, unit) {
    	if (in_play(unit) && unit->plan && unit->plan->aicontrol) {
	    unit->plan->asleep = FALSE;
	    unit->plan->reserve = FALSE;
	    /* We might want to mess with the unit now, so clear all
	       delays. */
	    unit->plan->delayed = FALSE;
	    unit->plan->waitingfortasks = FALSE;
	    if (unit->plan->type == PLAN_PASSIVE) {
		unit->plan->type = PLAN_NONE;
	    }
    	}
    }
}

/* At the beginning of each turn, make plans and review the situation. */

void
mplayer_init_turn(side)
Side *side;
{
    /* Cases where we no longer need to run. */
    if (!side->ingame)
      return;
    /* A side without units hasn't got anything to do but wait. */
    /* (should account for possible units on controlled sides) */
    if (!side_has_units(side))
      return;
    /* Mplayers in a hacked game will not play,
       unless they're being debugged. */
    if (compromised && !DebugM)
      return;
    update_all_progress_displays("ai turn init start", side->id);
    DMprintf("%s mplayer init turn\n", side_desig(side));
    /* Make sure a strategy object exists. */
    if (mplayer(side) == NULL)
      create_strategy(side);
    /* Look over the game design we're playing with. */
    analyze_the_game(side);
    /* If this game is one that can be won, as opposed to
       just dinking around, figure how to win it. */
    if (mplayer(side)->trytowin) {
	/* Check out the current goal tree first. */
	review_goals(side);
	/* Goal analysis might have triggered resignation. */
	if (!side->ingame)
	  goto done;
	/* Check out all the theaters. */
	review_theaters(side);
	/* Check out all of our units. */
	review_units(side);
	/* Decide on the new current plan. */
	update_side_strategy(side);
	/* Propagate this to individual unit plans. */
	update_unit_plans(side);
    } else {
	update_unit_plans_randomly(side);
    }
  done:
    update_all_progress_displays("", side->id);
    DMprintf("%s mplayer init turn done\n", side_desig(side));
}

/* Create and install an entirely new strategy object for the side. */

void
create_strategy(side)
Side *side;
{
    Strategy *strategy = (Strategy *) xmalloc(sizeof(Strategy));

    /* Put the specific structure into a generic slot. */
    side->ai = (struct a_ai *) strategy;
    strategy->type = mplayertype;
    /* Allocate a table of pointers to theaters, for access via small numbers
       rather than full pointers. */
    strategy->theatertable = (Theater **) xmalloc(127 * sizeof(Theater *));
    /* Allocate a layer of indexes into the theater table. */
    strategy->areatheaters = malloc_area_layer(char);
    /* Allocate random things. */
    /* Arrays for unit types. */
    strategy->actualmix = (short *) xmalloc(numutypes * sizeof(short));
    strategy->expectedmix = (short *) xmalloc(numutypes * sizeof(short));
    strategy->idealmix = (short *) xmalloc(numutypes * sizeof(short));
    /* Arrays for terrain types. */
    strategy->terrainguess = (short *) xmalloc(numttypes * sizeof(short));
    /* Set everything to correct initial values. */
    reset_strategy(side);
}

/* Put all the right initial values into the strategy, but don't allocate anything. */

void
reset_strategy(side)
Side *side;
{
    int u, t, dir;
    Strategy *strategy = (Strategy *) side->ai;

    /* Remember when we did this. */
    strategy->creationdate = g_turn();
    /* Null out various stuff. */
    strategy->numgoals = 0;
    strategy->theaters = NULL;
    /* Actually we start with no theaters, but it's convenient to leave entry 0
       in the theater table pointing to NULL. */
    strategy->numtheaters = 1;
    /* Clear pointers to special-purpose theaters. */
    strategy->homefront = NULL;
    for_all_directions(dir) {
    	strategy->perimeters[dir] = strategy->midranges[dir] = strategy->remotes[dir] = NULL;
    }
    strategy->explorersneeded = 0;
    /* Reset the summation of our exploration needs. */
    for_all_unit_types(u) {
	strategy->actualmix[u] = 0;
	strategy->expectedmix[u] = 0;
	strategy->idealmix[u] = 0;
    }
    for_all_terrain_types(t) {
	strategy->terrainguess[t] = 0;
    }
    strategy->analyzegame = TRUE;
    /* Analyze the game and decide our basic goals. */
    analyze_the_game(side);
}

/* Look over the game design and decide what we're supposed to be doing,
   if anything at all.  This just sets up toplevel goals based on the
   game design, does not evaluate goals or any such. */

void
analyze_the_game(side)
Side *side;
{
    int maybedraw;
    Goal *goal;

    if (mplayer(side)->analyzegame) {
	if (should_try_to_win(side)) {
	    mplayer(side)->trytowin = TRUE;
	    /* This is our whole purpose in the game. */
	    goal = create_goal(GOAL_WON_GAME, side, TRUE);
	    add_goal(side, goal);
	    /* Now figure what exactly we have to do in order to win. */
	    determine_subgoals(side);
	    /* Machine will want to keep playing as long as it thinks
	       it has a chance to win. */
	    maybedraw = FALSE;
	} else {
	    mplayer(side)->trytowin = FALSE;
	    /* Since the side is not trying to win anything, it will be
	       pretty laidback about whether to keep the game going. */
	    maybedraw = TRUE;
	}
	/* Be trusting about game saves, at least for now. */
	set_willing_to_save(side, TRUE);
	set_willing_to_draw(side, maybedraw);
	mplayer(side)->analyzegame = FALSE;
	DMprintf("%s will try to %s this game\n",
		 side_desig(side),
		 mplayer(side)->trytowin ? "win" : "have fun in");
	/* (should summarize any per-side goals here) */
    }
}

void
determine_subgoals(side)
Side *side;
{
    Side *side2;
    Scorekeeper *sk;
    Goal *goal;

    /* Look at each scorekeeper and decide on appropriate goals. */
    for_all_scorekeepers(sk) {
        if (match_keyword(sk->body, K_LAST_SIDE_WINS)) {
	    /* We want to "kick butt" - *everybody* else's butt. */
	    for_all_sides(side2) {
		if (!trusted_side(side, side2) && side2->ingame) {
		    goal = create_goal(GOAL_WON_GAME, side2, FALSE);
		    add_goal(side, goal);
		    /* (should add "search-and-destroy" as corollaries) */
		    /* (should add protection of own valuable units) */
		}
	    }
        } else {
	    DMprintf("Don't understand a scorekeeper!");
        }
    }
    /* We might develop a sudden interest in exploration. */
    /* (but should only be if information is really important to winning) */
    if (!g_see_all()) {
	if (!g_terrain_seen()) {
	    add_goal(side, create_goal(GOAL_WORLD_KNOWN, side, TRUE));
	}
	/* It will be important to keep track of other sides' units
	   as much as possible. */
	for_all_sides(side2) {
	    if (side != side2) {
		goal = create_goal(GOAL_POSITIONS_KNOWN, side, TRUE);
		goal->args[0] = (long) side2;
		add_goal(side, goal);
	    }
	}
	/* Also add the general goal of knowing where indeps are. */
	goal = create_goal(GOAL_POSITIONS_KNOWN, side, TRUE);
	goal->args[0] = (long) NULL;
	add_goal(side, goal);
    }
}

/* Do a combination of analyzing existing theaters and creating new ones. */

void
review_theaters(side)
Side *side;
{
    int x, y, u, s, pop, totnumunits;
    int firstcontact = FALSE;
    int homefound = FALSE;
    short view;
    Unit *unit;
    Side *firstcontactside, *homefoundside, *otherside;
    Theater *theater;

    /* Create some theaters if none exist. */
    if (mplayer(side)->theaters == NULL) {
	create_initial_theaters(side);
    }
    for_all_theaters(side, theater) {
	theater->allied_units = 0;
	theater->makers = 0;
	theater->unexplored = 0;
	theater->border = FALSE;
	theater->allied_bases = 0;
	for_all_unit_types(u) {
	    theater->numassigned[u] = 0;
	    theater->numneeded[u] = 0;
	    theater->numenemies[u] = 0;
	    theater->numsuspected[u] = theater->numsuspectedmax[u] = 0;
	    theater->numtotransport[u] = 0;
	}
	if (people_sides_defined()) {
	    for (s = 0; s <= numsides; ++s) theater->people[s] = 0;
	}
/*	theater->enemy_strength /= 2; */
	theater->units_lost /= 2;
	theater->size = 0;
	theater->xmin = theater->ymin = -1;
	theater->xmax = theater->ymax = -1;
    }
    /* Now look at all the units that we can. */
    for_all_side_units(side, unit) {
    	if (in_play(unit)) {
	    theater = unit_theater(unit);
	    if (theater != NULL) {
		++(theater->allied_units);
		(theater->numassigned[unit->type])++;
		if (isbase(unit)) theater->allied_bases++;
	    }
	}
    }
    /* (should also analyze allies etc) */
    /* Now look at the whole world. */
    for_all_interior_cells(x, y) {
	if ((theater = theater_at(side, x, y)) != NULL) {
	    ++(theater->size);
	    if (theater->xmin < 0 || x < theater->xmin) theater->xmin = x;
	    if (theater->ymin < 0 || y < theater->ymin) theater->ymin = y;
	    if (theater->xmax < 0 || x > theater->xmax) theater->xmax = x;
	    if (theater->ymax < 0 || y > theater->ymax) theater->ymax = y;
	    if (g_see_all() || cover(side, x, y) > 0) {
	    	for_all_stack(x, y, unit) {
	    	    /* what about occupants? */
	    	    if (in_play(unit)
	    	    	&& side != unit->side
	    	    	&& (unit->side != NULL
	    	    	    || u_point_value(unit->type) > 0)) {
			if (enemy_side(side, unit->side))
			  ++(theater->numenemies[unit->type]);
	    	    	if (mplayer(side)->contacted[side_number(unit->side)] == 0) {
			    mplayer(side)->contacted[side_number(unit->side)] = 1;
			    if (unit->side != NULL) {
				firstcontact = TRUE;
				firstcontactside = unit->side;
			    }
	    	    	}
	    	    	if (mplayer(side)->homefound[side_number(unit->side)] == 0
	    	    	    && !mobile(unit->type)) {
			    mplayer(side)->homefound[side_number(unit->side)] = 1;
			    if (unit->side != NULL) {
				homefound = TRUE;
				homefoundside = unit->side;
			    }
	    	    	}
	    	    }
	    	}
		if (people_sides_defined()) {
		    if ((pop = people_side_at(x, y)) != NOBODY) {
			++(theater->people[pop]);
	    	    	if (mplayer(side)->homefound[pop] == 0) {
			    mplayer(side)->homefound[pop] = 1;
			    if (pop != 0) {
				homefound = TRUE;
				homefoundside = side_n(pop);
			    }
	    	    	}
		    }
		}
	    } else {
		if (terrain_view(side, x, y) == UNSEEN) {
		    ++(theater->unexplored);
		} else {
		    view = unit_view(side, x, y);
		    if (view != EMPTY) {
			if (side != side_n(vside(view))
			    && (vside(view) != 0
			    	/* (should count indep prizes separately) */
			        || u_point_value(vtype(view)) > 0)) {
			    u = vtype(view);
			    if (enemy_side(side, side_n(vside(view)))) {
				++(theater->numsuspected[u]);
				++(theater->numsuspectedmax[u]);
			    }
			}
		    }
		    if (people_sides_defined()) {
			pop = people_side_at(x, y);
			if (pop != NOBODY) {
			    ++(theater->people[pop]);
			}
		    }
		}
	    }
	}
    }
    for_all_theaters(side, theater) {
    	theater->x = (theater->xmin + theater->xmax) / 2;
    	theater->y = (theater->ymin + theater->ymax) / 2;
    	theater->enemystrengthmin = theater->enemystrengthmax = 0;
    	for_all_unit_types(u) {
	    theater->enemystrengthmin +=
	      theater->numenemies[u] + theater->numsuspected[u];
	}
	theater->enemystrengthmax = theater->enemystrengthmin;
    }
    if (firstcontact || homefound) {
    	for_all_side_units(side, unit) {
	    if (unit->plan && unit->plan->aicontrol) {
		unit->plan->maingoal = NULL;
		unit->plan->formation = NULL;
		unit->plan->funit = NULL;
		/* Force a replan. */
		unit->plan->type = PLAN_NONE;
		unit->plan->asleep = FALSE;
		unit->plan->reserve = FALSE;
		unit->plan->waitingfortasks = FALSE;
		set_unit_theater(unit, NULL);
		update_unit_display(side, unit, TRUE);
	    }
	}
    }
    for_all_theaters(side, theater) {
	DMprintf("%s theater \"%s\" at %d,%d from %d,%d to %d,%d (size %d)\n",
		 side_desig(side), theater->name, theater->x, theater->y,
		 theater->xmin, theater->ymin, theater->xmax, theater->ymax,
		 theater->size);
	/* Summarize what we know about the theater. */
	DMprintf("%s theater \"%s\"", side_desig(side), theater->name);
	if (!g_see_all() && theater->unexplored > 0) {
	    DMprintf(" unexplored %d", theater->unexplored);
	}
	DMprintf(" enemy %d", theater->enemystrengthmin);
	if (theater->enemystrengthmin != theater->enemystrengthmax) {
	    DMprintf("-%d", theater->enemystrengthmax);
	}
	for_all_unit_types(u) {
	    if (theater->numenemies[u] + theater->numsuspected[u] > 0) {
	    	DMprintf(" %3s %d", u_type_name(u), theater->numenemies[u]);
	    	if (theater->numsuspected[u] > 0) {
		    DMprintf("+%d", theater->numsuspected[u]);
		}
	    }
	}
	if (people_sides_defined()) {
	    DMprintf(" people");
	    for (s = 0; s <= numsides; ++s) {
		if (theater->people[s] > 0) {
		    DMprintf(" s%d %d", s, theater->people[s]);
		}
	    }
	}
	DMprintf("\n");
	totnumunits = 0;
	for_all_unit_types(u) {
	    totnumunits +=
	      (theater->numassigned[u] + theater->numneeded[u] + theater->numtotransport[u]);
	}
	if (totnumunits > 0) {
	    /* Summarize the status of our own units in this theater. */
	    DMprintf("%s theater \"%s\" has ", side_desig(side), theater->name);
	    for_all_unit_types(u) {
		if (theater->numassigned[u] + theater->numneeded[u] + theater->numtotransport[u] > 0) {
		    DMprintf(" %d %3s", theater->numassigned[u], u_type_name(u));
			if (theater->numneeded[u] > 0) {
			    DMprintf(" (of %d needed)", theater->numneeded[u]);
			}
			if (theater->numtotransport[u] > 0) {
			    DMprintf(" (%d awaiting transport)", theater->numtotransport[u]);
			}
		}
	    }
	    DMprintf("\n");
	}
    }
    /* Also summarize contacts. */
    for_all_sides(otherside) {
    	if (otherside != side) {
	    if (mplayer(side)->contacted[otherside->id]) {
		DMprintf("%s contacted s%d", side_desig(side), otherside->id);
		if (mplayer(side)->homefound[otherside->id]) {
		    DMprintf(", home found");
		}
		DMprintf("\n");
	    }
    	}
    }
}

Theater *tmptheater;

int
fn_set_theater(x, y)
int x, y;
{
    set_theater_at(tmpside, x, y, tmptheater);
    return 0;
}

/* Set up the initial set of theaters. */

void
create_initial_theaters(side)
Side *side;
{
    int x, y, dir, dist, i, j;
    int xmin, ymin, xmax, ymax;
    int homeradius, perimradius, midradius, xxx;
    int numthx, numthy, thwid, thhgt;
    Unit *unit;
    Theater *homefront, *enemyarea, *theater;
    Theater *gridtheaters[8][8];
    Strategy *strategy = mplayer(side);

    for (i = 0; i < 8; ++i) {
	for (j = 0; j < 8; ++j) {
	    gridtheaters[i][j] = NULL;
	}
    }
    /* Compute bbox of initial (should also do enemy?) units. */
    xmin = area.width;  ymin = area.height;  xmax = ymax = 0;
    for_all_side_units(side, unit) {
	if (alive(unit) /* and other preconditions? */) {
	    if (unit->x < xmin) xmin = unit->x;
	    if (unit->y < ymin) ymin = unit->y;
	    if (unit->x > xmax) xmax = unit->x;
	    if (unit->y > ymax) ymax = unit->y;
	}
    }
    /* Most games start with each side's units grouped closely together.
       If this is not the case, do something else. */
    if (xmax - xmin > area.width / 4 && ymax - ymin > area.height / 4) {
	/* (should do some sort of clustering of units) */
	if (0 /*people_sides_defined()*/) {
	    homefront = create_theater(side);
	    homefront->name = "Home Front";
	    enemyarea = create_theater(side);
	    enemyarea->name = "Enemy Area";
	    for_all_interior_cells(x, y) {
	        if (people_side_at(x, y) == side->id) {
		    set_theater_at(side, x, y, homefront);
	        } else {
		    set_theater_at(side, x, y, enemyarea);
	        }
	    }
	} else {
		/* Divide the world up along a grid. */
		numthx = (area.width  > 60 ? (area.width  > 120 ? 7 : 5) : 3);
		numthy = (area.height > 60 ? (area.height > 120 ? 7 : 5) : 3);
		thwid = max(8, area.width / numthx);
		thhgt = max(8, area.height / numthy);
	    	for_all_interior_cells(x, y) {
			i = x / thwid;  j = y / thhgt;
			if (gridtheaters[i][j] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Grid %d,%d", i, j);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    gridtheaters[i][j] = theater;
			} else {
			    theater = gridtheaters[i][j];
			}
		    set_theater_at(side, x, y, theater);
		}
	}
	return;
    } else {
	/* Always create a first theater that covers the starting area. */
	homefront = create_theater(side);
	homefront->name = "Home Front";
	/* Calculate startxy if not already available. */
	if (side->startx < 0 && side->starty < 0)
	  calc_start_xy(side);
	homefront->x = side->startx;  homefront->y = side->starty;
	strategy->homefront = homefront;
	homeradius = max(5, g_min_radius());
	perimradius = max(homeradius + 5, g_min_separation() - homeradius);
	midradius = max(perimradius + 10, g_min_separation() * 2);
	xxx = max((side->startx - perimradius), (area.width - side->startx - perimradius));
	xxx /= 2;
	midradius = min(midradius, perimradius + xxx);
	for_all_interior_cells(x, y) {
	    if (people_sides_defined()
		&& people_side_at(x, y) == side->id) {
		set_theater_at(side, x, y, homefront);
	    } else {
		dist = distance(x, y, side->startx, side->starty);
		if (dist < homeradius) {
		    set_theater_at(side, x, y, homefront);
		} else {
		    dir = approx_dir(x - side->startx, y - side->starty);
		    if (dist < perimradius) {
			if (strategy->perimeters[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Perimeter %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->perimeters[dir] = theater;
			} else {
			    theater = strategy->perimeters[dir];
			}
		    } else if (dist < midradius) {
			if (strategy->midranges[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Midrange %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->midranges[dir] = theater;
			} else {
			    theater = strategy->midranges[dir];
			}
		    } else {
			if (strategy->remotes[dir] == NULL) {
			    theater = create_theater(side);
			    sprintf(spbuf, "Remote %s", dirnames[dir]);
			    theater->name = copy_string(spbuf);
			    theater->x = x;  theater->y = y;
			    strategy->remotes[dir] = theater;
			} else {
			    theater = strategy->remotes[dir];
			}
		    }
		    set_theater_at(side, x, y, theater);
	    	}
	    }
	}  
    }
    /* Assign all units to the theater they're currently in. */
    /* (how do reinforcements get handled? mplayer should get hold of perhaps) */
    for_all_side_units(side, unit) {
	if (in_play(unit) /* and other preconditions? */) {
		set_unit_theater(unit, theater_at(side, unit->x, unit->y));
	}
    }
}

/* Create a single theater object and link it into the list of
   theaters. */

/* (should be able to re-use theaters in already in theater table) */

Theater *
create_theater(side)
Side *side;
{
    Theater *theater = (Theater *) xmalloc(sizeof(Theater));

    if (mplayer(side)->numtheaters > MAXTHEATERS) return NULL;
    theater->id = (mplayer(side)->numtheaters)++;
    theater->name = "?";
    theater->maingoal = NULL;
    theater->people = (long *) xmalloc ((numsides + 1) * sizeof(long));
    /* (should alloc other array slots too) */
    /* Connect theater into a linked list. */
    theater->next = mplayer(side)->theaters;
    mplayer(side)->theaters = theater;
    /* Install it into the theater table also. */
    mplayer(side)->theatertable[theater->id] = theater;
    return theater;
}

/* Examine the goals to see what has been accomplished and what still needs
   to be done. */

void
review_goals(side)
Side *side;
{
    int i;
    Scorekeeper *sk;
    Goal *goal;
    Strategy *strategy = mplayer(side);

    for (i = 0; i < strategy->numgoals; ++i) {
	goal = strategy->goals[i];
	DMprintf("%s has %s\n", side_desig(side), goal_desig(goal));
    }
    /* Should look at certainty of each goal and decide whether to keep or
       drop it, and mention in debug output also. */
    /* Also think about resigning. */
    if (keeping_score()) {
	for_all_scorekeepers(sk) {
	    if (symbolp(sk->body)
		&& match_keyword(sk->body, K_LAST_SIDE_WINS)) {
		decide_resignation(side);
	    }
	}
    }
}


/* Sometimes there is no point in going on, but be careful not to be too
   pessimistic.  Right now we only give up if no hope at all. */

void
decide_resignation(side)
Side *side;
{
    int u, couldwin = TRUE, coulddraw = TRUE, chance = 0;
    int sn1, x, y, uview;
    Side *side1, *side2;
    Strategy *strategy = mplayer(side);
    Unit *unit;
 
    for_all_sides(side1) {
	sn1 = side_number(side1);
	for_all_unit_types(u) {
	    strategy->strengths[sn1][u] = 0;
	}
	if (side1 == side || allied_side(side, side1)) {
	    for_all_side_units(side1, unit) {
		if (in_play(unit)) {  /* but what about reinforcements? */
		    ++(strategy->strengths[sn1][unit->type]);
		}
	    }
	}
    }
    if (g_see_all()) {
	    for_all_cells(x, y) {
		for_all_stack(x, y, unit) {
		    side2 = unit->side;
		    if (side2 != NULL && enemy_side(side, side2)) {
			++(strategy->strengths[side2->id][unit->type]);
		    }
		}
	    }
    } else {
	    /* Look at the current view to get enemy strength. */
	    /* This is too easily faked, and doesn't know about hiding units... */
	    /* Should also discount old data. */
	    for_all_cells(x, y) {
		uview = unit_view(side, x, y);
		if (uview != UNSEEN && uview != EMPTY) {
		    side2 = side_n(vside(uview));
		    if (side2 != NULL && enemy_side(side, side2)) {
			++(strategy->strengths[side2->id][vtype(uview)]);
		    }
		}
	    }
    }
    /* Estimate point values. */
    for_all_sides(side1) {
	sn1 = side1->id;
	strategy->points[sn1] = 0;
	for_all_unit_types(u) {
	    strategy->points[sn1] += strategy->strengths[sn1][u] * u_point_value(u);
	}
    }
    /* Estimate point values, and how many units of each type in allied group. */
    for_all_sides(side1) {
	sn1 = side1->id;
	for_all_unit_types(u) {
	    strategy->alstrengths[sn1][u] = strategy->strengths[sn1][u];
	    for_all_sides(side2) {
		if (side1 != side2 && allied_side(side1, side2)) {
		    strategy->alstrengths[sn1][u] +=
		      strategy->strengths[side2->id][u];
		}
	    }
	}
	for_all_sides(side2) {
		if (side1 != side2 && allied_side(side1, side2)) {
		    strategy->alpoints[sn1] += strategy->points[side2->id];
		}
	}
    }
    /* Dump out a detailed listing of our estimates. */
    if (DebugM) {
	for_all_sides(side1) {
	    DMprintf("%s strength estimate: ", side_desig(side1));
	    for_all_unit_types(u) {
		DMprintf(" %d", strategy->strengths[side1->id][u]);
	    }
	    DMprintf("\n");
	}
    }
    for_all_sides(side1) {
      if (side != side1 && !allied_side(side, side1)) {
	sn1 = side1->id;
	if (strategy->points[sn1] > 5 * strategy->points[side->id]) {
	    chance = 50;
	}
	if (strategy->points[sn1] > 8 * strategy->points[side->id]) {
	    chance = 90;
	}
      }
    }
    /* Whether or not we actually resign, we may be willing to
       go for a draw if other players want to. */
    /* (Note that toggling this flag is not exactly poker-faced
       behavior, but I doubt human players will be able to derive
       much advantage, since they'll already have a pretty good
       idea if the AI is in trouble or not.) */
    set_willing_to_draw(side, (chance > 0));
    /* Maybe resign. */
    if (chance > 0) {
	if (probability(chance)) {
	    give_up(side);
	}
    }
}

/* If a machine player resigns, it tries to help its friends. */

void
give_up(side)
Side *side;
{
    Side *side1;

    /* (should have to give units to controlling side if there is one) */
    /* Try to give away all of our units to an ally. */
    for_all_sides(side1) {
	if (side != side1 && allied_side(side, side1)) {
	    resign_game(side, side1);
	    return;
	}
    }
    /* (should give to any positively-regarded side?) */
    /* No allies, let everything become independent. */
    /* (should disband all units that can be disbanded?) */
    resign_game(side, NULL);
}

/* Go through all our units (and allied ones?). */

void
review_units(side)
Side *side;
{
    Unit *unit;
    Plan *plan;
    Theater *oldtheater, *theater;
    
    for_all_side_units(side, unit) {
	if (in_play(unit) && unit->plan && unit->plan->aicontrol) {
	    plan = unit->plan;
	    oldtheater = unit_theater(unit);
	    /* Goal might have become satisfied. */
	    if (plan->maingoal) {
		if (goal_truth(side, plan->maingoal) == 100) {
		    DMprintf("%s %s satisfied, removing\n",
			     unit_desig(unit), goal_desig(plan->maingoal));
		    plan->maingoal = NULL;
		    /* Force a replan. */
		    plan->type = PLAN_NONE;
		    set_unit_theater(unit, NULL);
		}
	    }
	    /* Theater might have become explored enough (90% known). */
	    if (plan->type == PLAN_EXPLORATORY
	        && (theater = unit_theater(unit)) != NULL
	        && theater->unexplored < theater->size / 10) {
		    DMprintf("%s theater %s is mostly known\n",
			     unit_desig(unit), theater->name);
		    plan->maingoal = NULL;
		    /* Force a replan. */
		    plan->type = PLAN_NONE;
		    set_unit_theater(unit, NULL);
	    }
	    theater = unit_theater(unit);
	    DMprintf("%s currently assigned to %s",
	    	     unit_desig(unit),
		     (theater ? theater->name : "no theater"));
	    if (oldtheater != theater) {
	    	DMprintf(" (was %s)",
			 (oldtheater ? oldtheater->name : "no theater"));
	    }
	    DMprintf("\n");
	}
    }
    /* Could notify display about unit plan mix? */
}

/* Look at our current overall strategy and hack it as needed. */

void
update_side_strategy(side)
Side *side;
{
    Theater *theater;

    Dprintf("%s updating strategy\n", side_desig(side));
    /* Add something to add/update theaters as things open up. (?) */
    for_all_theaters(side, theater) {
	decide_theater_needs(side, theater);
    }
}

/* Figure out how many units to request for each area. */

void
decide_theater_needs(side, theater)
Side *side;
Theater *theater;
{
    if (theater->unexplored > 0) {
    	/* Exploration is less important when 90% of a theater is known. */
    	if (theater->unexplored > (theater->size / 10)) {
		++(mplayer(side)->explorersneeded);
    	}
	/* Should look for good exploration units. */
	theater->importance = 50;  /* should depend on context */
/*	theater->reinforce = EXPLORE_AREA;  */
#if 0
    } else if (0 /* theater->enemy_strength < 5 */) {
	if (theater->allied_makers == 0
	    && theater->makers > 0
	    && theater->nearby) {
	    theater->reinforce = GUARD_BORDER_TOWN + 2 * theater->makers;
	} else if (theater->makers > 0) {
	    theater->reinforce = (theater->border ? GUARD_BORDER_TOWN :
				  GUARD_TOWN) + 2 * theater->allied_makers;
	} else if (theater->allied_bases > 0) {
	    theater->reinforce = (theater->border ? GUARD_BORDER: GUARD_BASE);
	} else if (theater->border) {
	    theater->reinforce = NO_UNITS;
	} else {
	    theater->reinforce = NO_UNITS;
	}
    } else {
	if (theater->allied_makers > 0) {
	    theater->reinforce = DEFEND_TOWN + 5 * theater->makers;
	} else if (theater->allied_bases > 0) {
	    theater->reinforce = DEFEND_BASE + theater->allied_bases;
	} else {
	    theater->reinforce = 0 /* DEFEND_AREA */;
	}
#endif
    }
}

/* For each unit, decide what it should be doing (if anything).  This is
   when a side takes the initiative;  a unit can also request info from
   its side when it is working on its individual plan. */

void
update_unit_plans(side)
Side *side;
{
    Unit *unit;

    for_all_side_units(side, unit) {
	if (is_active(unit) && unit->plan != NULL) {
	    mplayer_decide_plan(side, unit);
	}
    }
}

/* Randomly change a unit's plans.  (This is really more for
   debugging, exercising plan execution code in novel ways.) */

void
update_unit_plans_randomly(side)
Side *side;
{
    Unit *unit;

    for_all_side_units(side, unit) {
	if (is_active(unit) && unit->plan && unit->plan->aicontrol) {
	    if (probability(10)) {
		DMprintf("Randomly changed %s plan %s",
			 unit_desig(unit), plan_desig(unit->plan));
		unit->plan->type = xrandom((int) NUMPLANTYPES);
		DMprintf("to plan %s\n", plan_desig(unit->plan));
	    }
	    /* (should add/remove goals randomly) */
	    if (probability(10)) {
		unit->plan->reserve = FALSE;
	    }
	    if (probability(10)) {
		unit->plan->asleep = FALSE;
	    }
	}
    }
}

/* Push a new goal onto the side's list of goals. */

/* (this should only add goals that are not already present) */

void
add_goal(side, goal)
Side *side;
Goal *goal;
{
    if (mplayer(side)->numgoals < MAXGOALS) {
	mplayer(side)->goals[(mplayer(side)->numgoals)++] = goal;
    }
}

Goal *
has_goal(side, goaltype)
Side *side;
GoalType goaltype;
{
    int i;
    Goal *goal;

    for (i = 0; i < mplayer(side)->numgoals; ++i) {
	goal = mplayer(side)->goals[i];
	if (goal != NULL && goal->type == goaltype) {
	    return goal;
	}
    }
    return NULL;
}

/* This is for when a unit needs a plan and asks its side for one. */

void
mplayer_decide_plan(side, unit)
Side *side;
Unit *unit;
{
    Plan *plan = unit->plan;

    if (plan == NULL || !plan->aicontrol) return;
    if (!mplayer(side)->trytowin) {
	plan->type = PLAN_RANDOM;
	clear_task_agenda(unit->plan);
	return;
    }
    switch (plan->type) {
      case PLAN_PASSIVE:
      case PLAN_NONE:
	if (mobile(unit->type)) {
	    /* Maybe assign to exploration. */
	    if (!g_see_all() && !g_terrain_seen() /* should be "has goal" */) {
		if (need_this_type_to_explore(side, unit->type)) {
		    /* also limit to a total percentage, in case
		       exploration needs are very high */
		    assign_to_exploration(side, unit);
		} else {
		    assign_to_offense(side, unit);
		}
	    } else if (1) {
		assign_to_offense(side, unit);
	    } else {
/*		assign_to_defense(side, unit); */
	    }
	} else {
	    if ((!g_see_all() && !g_terrain_seen())
	        && need_this_type_to_build_explorers(side, unit->type)) {
		assign_to_explorer_construction(side, unit);
	    } else if (type_can_build_attackers(side, unit->type)) {
	    	assign_to_offense_support(side, unit);
	    } else {
		assign_to_defense_support(side, unit);
	    }
	}
	break;
      case PLAN_OFFENSIVE:
	/* leave plan alone */
	break;
      case PLAN_EXPLORATORY:
	/* leave plan alone */
	break;
      case PLAN_DEFENSIVE:
	/* leave plan alone */
	break;
      default:
	break;
    }
}

int
need_this_type_to_explore(side, u)
Side *side;
int u;
{
    int s, numcontacted = 0, numfound = 0;

    if (!mobile(u)) return FALSE;
    for (s = 1; s <= numsides; ++s) {
    	if (s == side->id) continue;
	if (mplayer(side)->contacted[s]) ++numcontacted;
	if (mplayer(side)->homefound[s]) ++numfound;
    }
    if (numcontacted == 0) {
	return TRUE;
    } else if (numfound == 0) {
	return probability(50);
    } else if (numfound < numsides - 1) {
	return probability(10);
    } else {
	return FALSE;
    }
}

struct weightelt {
    int weight;
    long data;
};

static int
compare_weights(w1, w2)
CONST void *w1, *w2;
{
    return (((struct weightelt *) w2)->weight - ((struct weightelt *) w1)->weight);
}

/* Set the unit up as an explorer and let it go. */

void
assign_to_exploration(side, unit)
Side *side;
Unit *unit;
{
    int numweights = 0, weight, i, dist;
    struct weightelt weights[MAXTHEATERS];
    Theater *theater;

    /* Unit's goal in life will be to see that the world is all known. */
    unit->plan->type = PLAN_EXPLORATORY;
    set_unit_theater(unit, NULL);
    /* Find the theater most in need of exploration. */
    for_all_theaters(side, theater) {
    	if (theater->size > 0 && theater->unexplored > 0) {
	    weight = (100 * theater->unexplored) / theater->size;
	    /* Downrate theaters that are far away. */
	    dist = distance(unit->x, unit->y, theater->x, theater->y)
	      - isqrt(theater->size) / 2;
	    if (dist < 0) dist = 0;
	    weight /= max(1, (4 * dist) / area.maxdim);
	    /* Flatten out 10% variations. */
	    weight = 10 * (weight / 10);
	    weights[numweights].weight = weight;
	    weights[numweights].data = theater->id;
	    ++numweights;
    	}
    }
    if (numweights > 0) {
    	qsort(weights, numweights, sizeof(struct weightelt), compare_weights);
    	/* Choose randomly among theaters of equal weight. */
    	for (i = 0; i < numweights; ++i)
	  if (weights[i].weight < weights[0].weight) break;
    	theater = mplayer(side)->theatertable[weights[xrandom(i)].data];
    } else {
    	theater = NULL;
    }
    assign_explorer_to_theater(side, unit, theater);
}

int probably_explorable PROTO ((Side *side, int x, int y, int u));

int
probably_explorable(side, x, y, u)
Side *side;
int x, y, u;
{
	int dir, x1, y1, tview, t;

	for_all_directions(dir) {
		if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
			tview = terrain_view(side, x, y);
			if (tview == UNSEEN)
			  return TRUE;
			t = vterrain(tview);
			if (could_move(u, t))
			  return TRUE;
		}
	}
	return FALSE;
}

void
assign_explorer_to_theater(side, unit, theater)
Side *side;
Unit *unit;
Theater *theater;
{
    int sq, x, y, tries = 0;
    Goal *goal;
    
    if (theater != NULL) {
	set_unit_theater(unit, theater);
	++(theater->numassigned[unit->type]);
	goal = create_goal(GOAL_VICINITY_KNOWN, side, TRUE);
	sq = isqrt(theater->size);
	while (++tries < 100) {
	    /* Select a random point within the theater. */
	    x = theater->xmin;  y = theater->ymin;
	    x += (xrandom(theater->xmax - theater->xmin)
		  + xrandom(theater->xmax - theater->xmin)) / 2;
	    y += (xrandom(theater->ymax - theater->ymin)
		  + xrandom(theater->ymax - theater->ymin)) / 2;
	    if (theater_at(side, x, y) == theater
		&& terrain_view(side, x, y) == UNSEEN
		&& probably_explorable(side, x, y, unit->type))
	      break;
	}
	goal->args[0] = x;  goal->args[1] = y;
	goal->args[2] = goal->args[3] = sq / 2;
	unit->plan->maingoal = goal;
	DMprintf("%s now assigned to exploration in %s, around %d,%d\n",
		 unit_desig(unit), theater->name, x, y);
    }
}

int
need_this_type_to_build_explorers(side, u)
Side *side;
int u;
{
    int s, u2;
	
    for (s = 1; s <= numsides; ++s) {
	if (mplayer(side)->contacted[s]) return FALSE;
	if (mplayer(side)->homefound[s]) return FALSE;
    }
    for_all_unit_types(u2) {
	if (mobile(u2)
	    /* should also check u2 is a useful explorer */
	    && uu_acp_to_create(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

/* Explorer constructors concentrate on building types that are good for
   exploration. */

void
assign_to_explorer_construction(side, unit)
Side *side;
Unit *unit;
{
    /* Unit's goal in life will be to help see that the world is all known. */
    unit->plan->type = PLAN_EXPLORATORY;
    DMprintf("%s assigned to explorer construction\n", unit_desig(unit));
}

void
assign_to_offense(side, unit)
Side *side;
Unit *unit;
{
    int numweights = 0, weight;
    struct weightelt weights[MAXTHEATERS];
    Goal *goal;
    Theater *homefront, *theater;

    unit->plan->type = PLAN_OFFENSIVE;
    clear_task_agenda(unit->plan);
    /* If our home area is being threatened, assign the unit to it. */
    if ((homefront = mplayer(side)->homefront) != NULL
        && homefront->enemystrengthmin > 0) {
	set_unit_theater(unit, homefront);
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	goal->args[0] = homefront->x;  goal->args[1] = homefront->y;
	goal->args[2] = goal->args[3] = isqrt(homefront->size);
	unit->plan->maingoal = goal;
	DMprintf("%s assigned to offensive in the home front\n",
		 unit_desig(unit));
	return;
    }
    /* If the theater the unit is currently in is being threatened, assign the unit to it. */
    /* (should just increase it weight in next calculation?) */
    if ((theater = theater_at(side, unit->x, unit->y)) != NULL
        && theater->enemystrengthmin > 0) {
	set_unit_theater(unit, theater);
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	/* (should randomize?) */
	goal->args[0] = theater->x;  goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	unit->plan->maingoal = goal;
	DMprintf("%s assigned to offensive in the theater where it's at now\n",
		 unit_desig(unit));
	return;
    }
    for_all_theaters(side, theater) {
    	if (theater->enemystrengthmin > 0 || theater->unexplored > 0) {
	    /* (should weight by strength relative to own units already there) */
	    weight = theater->enemystrengthmax * 20;
	    if (distance(unit->x, unit->y, theater->x, theater->y) > area.width / 2) {
		weight /= 2;
	    }
	    weight += (10 * theater->unexplored) / max(1, theater->size);
	    weights[numweights].weight = weight;
	    weights[numweights].data = theater->id;
	    ++numweights;
    	}
    }
    if (numweights > 0) {
    	qsort(weights, numweights, sizeof(struct weightelt), compare_weights);
    	theater = mplayer(side)->theatertable[weights[0].data];
    } else {
    	theater = theater_at(side, unit->x, unit->y);
    }
    set_unit_theater(unit, theater);
    if (theater != NULL) {
	++(theater->numassigned[unit->type]);
	goal = create_goal(GOAL_VICINITY_HELD, side, TRUE);
	/* (should randomize?) */
	goal->args[0] = theater->x;  goal->args[1] = theater->y;
	goal->args[2] = (theater->xmax - theater->xmin) / 2;
	goal->args[3] = (theater->ymax - theater->ymin) / 2;
	unit->plan->maingoal = goal;
	DMprintf("%s now assigned to offensive in %s",
		 unit_desig(unit), theater->name);
	if (numweights > 1) {
	    DMprintf(" (weight %d; runnerup was %s, weight %d)",
		     weights[0].weight,
		     (mplayer(side)->theatertable[weights[1].data])->name,
		     weights[1].weight);
	}
	DMprintf("\n");
    } else {
	DMprintf("%s now assigned to offensive in no theater",
		 unit_desig(unit));
    }
}

void
assign_to_offense_support(side, unit)
Side *side;
Unit *unit;
{
    unit->plan->type = PLAN_OFFENSIVE;
    clear_task_agenda(unit->plan);
}

int
type_can_build_attackers(side, u)
Side *side;
int u;
{
    int u2;
	
    for_all_unit_types(u2) {
	if (mobile(u2)
	    && (type_can_attack(u2) || type_can_fire(u2))
	    && uu_acp_to_create(u, u2) > 0) return TRUE;
    }
    return FALSE;
}

int
mplayer_preferred_build_type(side, unit, plantype)
Side *side;
Unit *unit;
int plantype;
{
    int u = unit->type, u2, u3, maxworth = 0;
    int prefs[MAXUTYPES];
    int t, knownterrain[MAXTTYPES], fringeterrain[MAXTTYPES], sumfringe, totfringe;
    int enemytypes[MAXUTYPES];
    int numtotransport[MAXUTYPES];
    int x, y, dir, x1, y1;
    int blockedallaround;
    int uview;
    Unit *unit2, *occ;
    Theater *theater;

    if (plantype == PLAN_EXPLORATORY) {
	/* Calculate the amount of each type of terrain at the edges
	   of the known world. */
	for_all_terrain_types(t) knownterrain[t] = fringeterrain[t] = 0;
	for_all_interior_cells(x, y) {
	    if (terrain_view(side, x, y) != UNSEEN) {
		++(knownterrain[(int) terrain_at(x, y)]);
		for_all_directions(dir) {
		    point_in_dir(x, y, dir, &x1, &y1);
		    if (terrain_view(side, x1, y1) == UNSEEN) {
			++(fringeterrain[(int) terrain_at(x, y)]);
			break;
		    }
		}
	    }
	}
    } else {
    	for_all_unit_types(u2) enemytypes[u2] = 0;
    	for_all_interior_cells(x, y) {
	    if (g_see_all() || cover(side, x, y) > 0) {
		for_all_stack(x, y, unit2) {
		    if (unit2->side != side) ++enemytypes[unit2->type];
		    /* (should count occ types recursively also) */
		    for_all_occupants(unit2, occ) {
			++enemytypes[occ->type];
		    }
		}
	    } else {
		if ((uview = unit_view(side, x, y)) != EMPTY) {
		    if (side_n(vside(uview)) != side) ++enemytypes[vtype(uview)];
		}
	    }
    	}
    }
    /* Calculate a basic preference for each possible type. */
    for_all_unit_types(u2) {
	prefs[u2] = 0;
	if (uu_acp_to_create(u, u2) > 0
	    /* tmp hack until mplayer can do research */
	    && (u_tech_to_build(u2) > 0 ? side->tech[u2] >= u_tech_to_build(u2) : TRUE)
	    && type_allowed_on_side(u2, side)) {
	    if (0 /* any demand in this unit's theater */) {
	    } else if (need_more_transportation(side)) {
    		for_all_unit_types(u3) {
		    numtotransport[u3] = 0;
    		}
	    	for_all_theaters(side, theater) {
		    for_all_unit_types(u3) {
			numtotransport[u3] += theater->numtotransport[u3];
		    }
	    	}
    		for_all_unit_types(u3) {
		    if (numtotransport[u3] > 0
			&& mobile(u2)
			&& could_carry(u2, u3)) {
			prefs[u2] += numtotransport[u3];
		    }
	    	}
	    } else {
		/* Prefer units by overall suitability for general plan. */
		if (plantype == PLAN_EXPLORATORY) {
		    sumfringe = totfringe = 0;
		    for_all_terrain_types(t) {
			totfringe += fringeterrain[t];
			if (!terrain_always_impassable(u2, t))
			  sumfringe += fringeterrain[t];
		    }
		    if (totfringe < 1) sumfringe = totfringe = 1;
		    /* Scale - so 5% diffs in amt of crossable terrain
		       don't affect result. */
		    prefs[u2] = (20 * sumfringe) / totfringe;
		    prefs[u2] /= max(1, normal_completion_time(u, u2) / 8);
		} else {
		    for_all_unit_types(u3) {
		       if (enemytypes[u3] > 0) {
		           if (uu_zz_bhw(u2, u3) > 0) {
		               prefs[u2] += uu_zz_bhw(u2, u3) * enemytypes[u3];
		           }
		           if (uu_zz_bcw(u2, u3) > 0) {
		               prefs[u2] += uu_zz_bcw(u2, u3) * enemytypes[u3];
		           }
		       }
		    }
		    prefs[u2] /= max(1, normal_completion_time(u, u2) / 8);
		}
		if (prefs[u2] < 1) prefs[u2] = 1;
	    }
	} else {
	}
    }
    /* Units that can't even get out of the builder get their preference
       cut.  This helps prevent the construction of large ships in Denver. */
    /* (should allow if units would have some other way to leave) */
    if (1 /* plantype == PLAN_EXPLORATORY */) {
	for_all_unit_types(u2) {
	    if (prefs[u2] > 1) {
		blockedallaround = TRUE;
		for_all_directions(dir) {
		    point_in_dir(unit->x, unit->y, dir, &x1, &y1);
		    if (!terrain_always_impassable(u2, terrain_at(x1, y1))) {
		    	blockedallaround = FALSE;
		    	break;
		    }
#if 0  /* for the moment */
		    if (unit_at(x1, y1) != NULL) {
		    	blockedallaround = FALSE;
		    	break;
		    }
#endif
		}
		if (blockedallaround) prefs[u2] = 0;
	    }
	}
    }
    /* Look for an existing incomplete occupant and prefer to build its type,
       if it is in the choices in the typelist. */
    for_all_occupants(unit, occ) {
	if (in_play(occ) && !completed(occ)) {
		if (prefs[occ->type] > 0 && flip_coin()) return occ->type;
	}
    }
    for_all_unit_types(u2) if (prefs[u2] < 0) prefs[u2] = 0;
    return select_by_weight(prefs, numutypes);
}

/* (should make this a generic routine) */

int
select_by_weight(arr, numvals)
int *arr, numvals;
{
    int sum = 0, i, n;

    sum = 0;
    for (i = 0; i < numvals; ++i) {
	sum += arr[i];
    }
    if (sum == 0) return -1;
    /* We now know the range, make a random index into it. */
    n = xrandom(sum);
    /* Go through again to figure out which choice the index refs. */
    sum = 0;
    for (i = 0; i < numvals; ++i) {
	sum += arr[i];
	if (sum >= n) {
	    return i;
	}
    }
    run_error("Ooh weird");
    return 0;
}

int
need_more_transportation(side)
Side *side;
{
    int u3, u2, anytransport;
    Theater *theater;

    for_all_theaters(side, theater) {
	for_all_unit_types(u3) {
	    if (theater->numtotransport[u3] > 0) {
		anytransport = FALSE;
		for_all_unit_types(u2) {
		    if (theater->numassigned[u2] > 0
			&& mobile(u2)
			&& could_carry(u2, u3))
		      anytransport = TRUE;
		}
		if (!anytransport) return TRUE;
	    }
	}
    }
    return FALSE;
}

void
assign_to_defense_support(side, unit)
Side *side;
Unit *unit;
{
    unit->plan->type = PLAN_DEFENSIVE;
    clear_task_agenda(unit->plan);
}

/* This is called when an exploring unit gets confused about what to do. */

int
mplayer_guide_explorer(side, unit)
Side *side;
Unit *unit;
{
    if (probability(10) && build_base_for_self(side, unit))
      return TRUE;
    if (probability(10) && build_base_for_others(side, unit))
      return TRUE;
    return FALSE;
}

/* Decide for the unit whether it should build a base for its own benefit. */

int
build_base_for_self(side, unit)
Side *side;
Unit *unit;
{
    int u = unit->type, u2, cando = FALSE;

    for_all_unit_types(u2) {
	if (uu_acp_to_create(u, u2) > 0
	    && (uu_creation_cp(u, u2) >= u_cp(u2)
	        || uu_acp_to_build(u, u2) > 0)
	    /* (should check if any advantage to building) */
	   ) {
	   cando = TRUE;
	   break;
	}
    }
    if (cando) {
	DMprintf("%s building %s as a base for itself\n",
		     unit_desig(unit), u_type_name(u2));
	set_construction(unit, u2, 1);
	return TRUE;
    }
    return FALSE;
}

/* Decide for the unit whether it should build a base to help other units. */

int
build_base_for_others(side, unit)
Side *side;
Unit *unit;
{
    return FALSE;
}

int
build_depot_for_self(side, unit)
Side *side;
Unit *unit;
{
    int u = unit->type, u2, cando = FALSE;

    for_all_unit_types(u2) {
	if (uu_acp_to_create(u, u2) > 0
	    && (uu_creation_cp(u, u2) >= u_cp(u2)
	        || uu_acp_to_build(u, u2) > 0)
	    /* (should check if any advantage to building) */
	   ) {
	   cando = TRUE;
	   break;
	}
    }
    if (cando) {
	DMprintf("%s building %s as a depot for itself\n",
		     unit_desig(unit), u_type_name(u2));
	set_construction(unit, u2, 1);
	return TRUE;
    }
    return FALSE;
}

void
mplayer_react_to_action_result(side, unit, rslt)
Side *side;
Unit *unit;
int rslt;
{
    /* (should check on supplies) */
}

/* This is a hook that runs after each task is executed. */

void
mplayer_react_to_task_result(side, unit, task, rslt)
Side *side;
Unit *unit;
Task *task;
int rslt;
{
    Unit *occ;
    Theater *theater;

    /* React to an apparent blockage. */
    if (rslt == TASK_FAILED
	&& task != NULL
	&& task->type == TASK_MOVETO
	&& task->retrynum > 2) {
	if (desired_direction_impassable(unit, task->args[0], task->args[1])) {
	    if (could_be_ferried(unit, task->args[0], task->args[1])) {
		if (unit->plan->type == PLAN_EXPLORATORY && flip_coin()) {
	      	    /* (could also change task within the same theater) */
		    DMprintf("%s blocked while exploring, changing theaters\n",
			     unit_desig(unit));
		    change_to_adjacent_theater(side, unit);
		} else {
		    DMprintf("%s blocked, will wait for transport\n",
			     unit_desig(unit));
		    theater = theater_at(side, unit->x, unit->y);
		    if (theater != NULL) {
			++(theater->numtotransport[unit->type]);
		    }
		    unit->plan->reserve = TRUE;
		    unit->plan->waitingfortransport = TRUE;
		}
	    } else {
	    	if (unit->occupant) {
		    DMprintf("%s blocked while transporting, will sit briefly\n",
			     unit_desig(unit));
		    unit->plan->reserve = TRUE;
		    for_all_occupants(unit, occ) {
		    	wake_unit(occ, FALSE, -1, NULL);
		    }
		    return;
	    	}
		/* Another option is to transfer to another theater.
		   This is especially useful when exploring. */
		if (unit->plan->type == PLAN_EXPLORATORY) {
		    DMprintf("%s blocked while exploring, changing theaters\n",
			     unit_desig(unit));
		    change_to_adjacent_theater(side, unit);
		}
	    }
	} else if (blocked_by_enemy(unit, task->args[0], task->args[1])) {
	    /* (should decide if allowable risk to passengers) */
	    DMprintf("%s blocked by enemy\n", unit_desig(unit));
	    attack_blockage(side, unit, task->args[0], task->args[1]); 
	}
	return;
    }
    /* React to inability to resupply by trying to build a base. */
    if (rslt == TASK_FAILED
	&& task != NULL
	&& task->type == TASK_RESUPPLY
	&& task->retrynum > 2) {
    	set_unit_reserve(side, unit, FALSE, FALSE);
    	build_depot_for_self(side, unit);
    }
#if 0    /* I'm not sure the following is really necessary - */
    /* - if the task fails, but a good victim is to be seen nearby, then the AI will
       eventually assign a unit to attack, quite likely the one whose task just failed. */
    /* React to a target unit trying to get away; if we can still see it nearby
       somewhere, adjust to pursue. */
    if (rslt == TASK_FAILED
	&& task != NULL
	&& task->type == TASK_HIT_UNIT
	) {
	int u = unit->type, tx, ty, dist, enemythere, uview, u2, s2;
	Unit *unit2, *target;

	tx = task->args[0];  ty = task->args[1];
	tu = task->args[2];  ts = task->args[3];
	if (can_see_actual_units(side, tx, ty)) {
		for_all_stack(tx, ty, unit2) {
	    	    if (unit2->side != side
			&& (tu == NONUTYPE || tu == unit2->type)
			) {
			/* A target is still there; task must have failed for some
			   other reason. */
			return;
		    }
		}
		/* They're gone! */
	} else {
		/* Assess old image or emptiness. */
		uview = unit_view(side, tx, ty);
		if (uview != EMPTY) {
		    if (tu == NONUTYPE || tu == vtype(uview)) {
		    	/* Target is still here. */
		    	return;
		    }
		}
		/* They're gone! */
	}
	/* If we get here, the target's location is empty. */
	return;
    }
#endif
}

void
change_to_adjacent_theater(side, unit)
Side *side;
Unit *unit;
{
    int dir;
    Theater *theater;

    if ((theater = unit_theater(unit)) != NULL) {
   	for_all_directions(dir) {
   	    if (theater == mplayer(side)->perimeters[dir]) {
   	    	assign_explorer_to_theater(side, unit,
   	    	    mplayer(side)->perimeters[flip_coin() ? left_dir(dir) : right_dir(dir)]);
   	    	break;
   	    }
   	    if (theater == mplayer(side)->midranges[dir]) {
   	    	assign_explorer_to_theater(side, unit,
   	    	    mplayer(side)->midranges[flip_coin() ? left_dir(dir) : right_dir(dir)]);
   	    	break;
   	    }
   	    if (theater == mplayer(side)->remotes[dir]) {
   	    	assign_explorer_to_theater(side, unit,
   	    	    mplayer(side)->remotes[flip_coin() ? left_dir(dir) : right_dir(dir)]);
   	    	break;
   	    }
   	}
    }
}

/* (should account for impassability because of borders, etc) */

int
desired_direction_impassable(unit, x, y)
Unit *unit;
int x, y;
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t, numbaddirs = 0;

    numdirs = choose_move_dirs(unit, x, y, TRUE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	if (terrain_always_impassable(unit->type, t)) ++numbaddirs;
    }
    return (numbaddirs == numdirs);
}

int
could_be_ferried(unit, x, y)
Unit *unit;
int x, y;
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t, u2;

    if (!carryable(unit->type)) return FALSE;
    numdirs = choose_move_dirs(unit, x, y, FALSE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	/* See if there is a type that can carry us through via this direction. */
	for_all_unit_types(u2) {
	    if (could_carry(u2, unit->type)
	        && mobile(u2)
	        && !terrain_always_impassable(u2, t)) {
	        return TRUE;
	    }
	}
    }
    return FALSE;
}

/* Note the recursion - should precalc this property. */

int
carryable(u)
int u;
{
    int u2;
	
    for_all_unit_types(u2) {
	if (could_carry(u2, u)
	    && (mobile(u2) /* || carryable(u2) */ )) return TRUE;
    }
    return FALSE;
}

int *accelerables = NULL;

int
accelerable(u)
int u;
{
    int u1, u2;

    if (accelerables == NULL) {
	accelerables = (int *) xmalloc(numutypes * sizeof(int));
	for_all_unit_types(u1) {	
	    for_all_unit_types(u2) {
		if (could_carry(u2, u1)
		    && mobile(u2)) {
		    if (u_acp(u2) * u_speed(u2) > u_acp(u1) * u_speed(u1)) {
#if 0
			sameterrain = TRUE;
			for_all_terrain_types(t) {
			    if (terrain_always_impassable(u2, t)
				&& !terrain_always_impassable(u1, t)) {
				return FALSE;
			    }
			}
#endif
			accelerables[u1] = TRUE;
			break;
		    }
		}
	    }
	}
    }
    return accelerables[u];
}

int
blocked_by_enemy(unit, x, y)
Unit *unit;
int x, y;
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t, numbaddirs = 0;
    Unit *unit2;

    numdirs = choose_move_dirs(unit, x, y, TRUE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	if (terrain_always_impassable(unit->type, t)) { ++numbaddirs; continue; }
	unit2 = unit_at(x1, y1);
	if (in_play(unit2) && unit2->side != unit->side) ++numbaddirs;
    }
    return (numbaddirs == numdirs);
}

void
attack_blockage(side, unit, x, y)
Side *side;
Unit *unit;
int x, y;
{
    int dirs[NUMDIRS], numdirs, i, x1, y1, t;
    Unit *unit2;

    numdirs = choose_move_dirs(unit, x, y, TRUE, NULL, NULL, dirs);
    for (i = 0; i < numdirs; ++i) {
	point_in_dir(unit->x, unit->y, dirs[i], &x1, &y1);
	t = terrain_at(x1, y1);
	if (terrain_always_impassable(unit->type, t))
	  continue;
	unit2 = unit_at(x1, y1);
	if (unit2->side != unit->side) {
	    push_hit_task(unit, x1, y1);
	    return;
	}
    }
}

/* At the end of a turn, re-evaluate the plans of some units in case
   the situation changed. */

void
mplayer_finish_movement(side)
Side *side;
{
    int u;
    Unit *unit;
    Theater *theater;

    for_all_theaters(side, theater) {
	for_all_unit_types(u) {
	    if (theater->numtotransport[u] > 0) {
		/* Find a unit needing transport. */
		for_all_side_units(side, unit) {
		    if (is_active(unit)
			&& unit->plan
			&& unit->plan->aicontrol
			&& unit->plan->waitingfortransport
			&& theater_at(side, unit->x, unit->y) == theater) {
			search_for_available_transport(unit);
			unit->plan->waitingfortransport = FALSE;
		    }
		}
		break;
	    }
	}
    }
    for_all_side_units(side, unit) {
	if (is_active(unit)
	    && unit->plan
	    && unit->plan->aicontrol) {
	    rethink_plan(unit);
	}
    }
}

Unit *
search_for_available_transport(unit)
Unit *unit;
{
    int dist, closestdist = area.maxdim;
    Unit *transport, *closesttransport = NULL;
    Theater *theater = unit_theater(unit);

    /* (more efficient to search adjacent cells first?) */
    for_all_side_units(unit->side, transport) {
	if (is_active(transport)
	    && mobile(transport->type)
	    && could_carry(transport->type, unit->type)
	    && transport->occupant == NULL /* (should be "has room") */
	    && transport->act != NULL /* not quite correct, but to fix bug */) {
	    /* Maybe this one is already coming to get somebody. */
	    if (transport->plan
		&& transport->plan->tasks != NULL
		&& transport->plan->tasks->type == TASK_PICKUP) {
		if (transport->plan->tasks->args[0] == unit->id) return transport;
		/* Picking up somebody else - don't hijack. */
		continue;
	    }
	    if (transport->plan
		&& transport->plan->tasks != NULL
		&& transport->plan->tasks->type == TASK_MOVETO
		&& transport->plan->tasks->next != NULL
		&& transport->plan->tasks->next->type == TASK_PICKUP) {
		if (transport->plan->tasks->next->args[0] == unit->id) return transport;
		/* Picking up somebody else - don't hijack. */
		continue;
	    }
	    dist = distance(unit->x, unit->y, transport->x, transport->y);
	    if (dist < closestdist || (dist == closestdist && flip_coin())) {
		closesttransport = transport;
		closestdist = dist;
	    }
	    /* If transport already adjacent, no need to keep looking. */
	    if (closestdist <= 1) break;
	}
    }
    if (closesttransport != NULL && closesttransport->plan != NULL) {
	clear_task_agenda(closesttransport->plan);
	/* (could inherit unit's goal, but not needed) */
	closesttransport->plan->maingoal = NULL;
	push_pickup_task(closesttransport, unit);
	push_movenear_task(closesttransport, unit->x, unit->y, 1);
	/* No longer count this unit as needing transport. */
	if (theater != NULL) {
	    --(theater->numtotransport[unit->type]);
	    set_unit_theater(closesttransport, theater);
	}
	DMprintf("%s will be picked up by closest transport %s\n",
	         unit_desig(unit), unit_desig(closesttransport));
	return closesttransport;
    }
    return NULL;
}

void
rethink_plan(unit)
Unit *unit;
{
    int dist, x1, y1;
    Task *toptask = unit->plan->tasks, *nexttask = NULL;
    Plan *plan = unit->plan;
    Unit *transport;

    if (toptask) nexttask = toptask->next;
    if (toptask != NULL
	&& (toptask->type == TASK_HIT_UNIT
	    || (toptask->type == TASK_MOVETO
		&& nexttask != NULL
		&& nexttask->type == TASK_HIT_UNIT))
        && !plan->reserve
        && !plan->asleep
        && !plan->waitingfortransport
        && (unit->transport == NULL || !mobile(unit->transport->type))
        && ((dist = distance(unit->x, unit->y,
			     toptask->args[0], toptask->args[1]))
            >= 4 * u_acp(unit->type))
        && accelerable(unit->type)
        ) {
        DMprintf("%s looking for transport to accelerate with", unit_desig(unit));
        DMprintf("\n");
        if ((transport = search_for_available_transport(unit)) != NULL) {
/*	    push_sentry_task(unit, max(1, dist / max(1, u_acp(transport->type)))); */
	    plan->reserve = TRUE;
	    plan->waitingfortransport = FALSE;
        }
    }
    if (unit->plan->type == PLAN_OFFENSIVE
        && toptask != NULL
        && toptask->type == TASK_MOVETO
        && distance(unit->x, unit->y, toptask->args[0], toptask->args[1])
		>= min(2, u_acp(unit->type))
        && enemy_close_by(unit->side, unit, 1 /* 2 would be better? */, &x1, &y1)
        ) {
        	push_hit_task(unit, x1, y1);
        	DMprintf("%s sees enemy close by, will attack it\n", unit_desig(unit));
    }
    /* (should also notice fire opportunities) */
}

extern int victimx, victimy, victimrating;

int
enemy_close_by(side, unit, dist, xp, yp)
Side *side;
Unit *unit;
int dist, *xp, *yp;
{
	int x = unit->x, y = unit->y, dir, x1, y1;

	victimrating = -9999;
	tmpunit = unit;
	victim_here(x, y);
	for_all_directions(dir) {
		if (point_in_dir(x, y, dir, &x1, &y1)) {
			victim_here(x, y);
		}
	}
	if (victimrating > -9999) {
		*xp = victimx;  *yp = victimy;
		return TRUE;
	} else {
		return FALSE;
	}
}

void
mplayer_receive_message(side, sender, str)
Side *side, *sender;
char *str;
{
    /* First detect standard messages. */
    if (strcmp(str, "Eh?") == 0) {
	/* Don't respond, otherwise we might infinitely recurse. */
    } else if (allied_side(side, sender)) {
    } else {
	/* (should) Detect insults and respond appropriately. */
	if (strstr(str, "idiot")) {
	} else {
	    /* No idea what the message was, be puzzled. */
	    send_message(side, add_side_to_set(sender, NOSIDES), "Eh?");
	}
    }
}

/* This is used by interfaces to display the theater in use at a given point. */

char *
mplayer_at_desig(side, x, y)
Side *side;
int x, y;
{
    Theater *theater;

    if (mplayer(side) == NULL)
      return "";
    theater = theater_at(side, x, y);
    return (theater ? theater->name : "<no theater>");
}

int
mplayer_theater_at(side, x, y)
Side *side;
int x, y;
{
    Theater *theater;

    if (mplayer(side) == NULL)
      return 0;
    theater = theater_at(side, x, y);
    return (theater ? theater->id : 0);
}

/* also have a state reader? */

void
mplayer_write_state(fp, side)
FILE *fp;
Side *side;
{
#if 0 /* garbages up save/restore but has no real purpose */
    Theater *theater;
	
    fprintf(fp, "\n");
    for_all_theaters(side, theater) {
	fprintf(fp, "; theater %s\n", theater->name);
    }
#endif
}

/* Supporting calculations for the "mplayer" AI in Xconq.  */

int basic_transport_worth PROTO ((int u1, int u2));
void set_uu_btw PROTO ((int u1, int u2, int v));

#define DICE(N,NUMDICE,SPOTS,OFFSET)  \
  (((N) >> 14 == 0 || (N) >> 14 == 3) ?  \
   (NUMDICE = 0, SPOTS = 0, OFFSET = (N)) :  \
   (NUMDICE = ((N) >> 11) & 0x07, SPOTS = ((N) >> 7) & 0x0f, OFFSET = (N) & 0x7f))

/* General collections of numbers used by all machine players. */

int base_building = FALSE;     /* can some unit build a base */

/* Init used by all machine players.  Precompute useful information
   relating to unit types in general, and that usually gets referenced
   in inner loops. */

void
mplayer_init_shared()
{
    int u, u1, u2, t, m1, numbuilders, tmp;
    
    /* Need 3 scratch layers for routefinding. */
    allocate_area_scratch(3);

    if (!g_see_all() && 1 /* terrain not all known */) {
	    /* Compute approx percentage occurrence of each terrain type */
	    /* in the area not yet seen */
	    /* (not useful if fractal gen not used) */
	    for_all_terrain_types(t) {
		set_t_fraction(t, ((t_alt_max(t) - t_alt_min(t)) *
			       (t_wet_max(t) - t_wet_min(t))) / 100);
	    }
    }
    /* Recognize unit types that are bases */
    for_all_unit_types(u1) {
	set_u_is_base(u1, FALSE);
	tmp = FALSE;
	for_all_material_types(m1) {
	    if (um_base_production(u1, m1) > 0) {
		tmp = TRUE;
		break;
	    }
	}
	if (tmp) {
	    for_all_unit_types(u2) {
		if ((u1 != u2) && could_carry(u1,u2)) {
		    set_u_is_base(u1, TRUE);
		    continue;
		}
	    }
	}
    }
    /* Note that is_base_builder is set to the type of base that can */
    /* be built.  That means that unit zero can not be a base which */
    /* can be built. */
    for_all_unit_types(u1) {
	set_u_is_transport(u1, FALSE);
	set_u_is_carrier(u1, FALSE);
	set_u_is_base_builder(u1, FALSE);
	set_u_can_make(u1, FALSE);
	set_u_can_capture(u1, FALSE);
	numbuilders = 0;
/*	ave_build_time[u1] = 0;  */
	for_all_unit_types(u2) {
	    if (u_is_base(u2) &&
		could_create(u1, u2) &&
		1 /* can be made quickly? */) {
		set_u_is_base_builder(u1, u2);
		base_building = TRUE;
	    }
	    if (u_speed(u1) > 0 && could_carry(u1, u2)) {
		set_u_is_transport(u1, TRUE);
	    }
	    if (could_create(u2, u1)) {
		numbuilders++;
/*		ave_build_time[u1] += uu_make(u2,u1);  */
		set_u_can_make(u2, TRUE);
	    }
	    if (uu_capture(u1, u2) > 0 || uu_indep_capture(u1, u2) > 0) {
		set_u_can_capture(u1, TRUE);
	    }
	}
/*	if (numbuilders > 0)
	  ave_build_time[u1] /= numbuilders;  */
    }
    /* a carrier is a unit that is a mobile base, but that cannot
       move a passenger anywhere the passenger could not go itself. */
    for_all_unit_types(u1) {
	if (u_is_transport(u1)) {
	    set_u_is_carrier(u1, TRUE);
	    for_all_unit_types(u2) {
		if (could_carry(u1, u2)) {
		    for_all_terrain_types(t) {
			if (could_move(u1, t) && !could_move(u2, t))
			  set_u_is_carrier(u1, FALSE);
		    }
		}
	    }
	}
    }
    for_all_unit_types(u) {
	set_u_bw(u, basic_worth(u));
    }
    for_all_unit_types(u) {
	for_all_unit_types(u2) {
	    set_uu_bhw(u, u2, basic_hit_worth(u, u2));
	    set_uu_bcw(u, u2, basic_capture_worth(u, u2));
	    set_uu_btw(u, u2, basic_transport_worth(u, u2));
	}
    }
    /* Tell how things rated. */
    if (DebugM) display_assessment();
}

/* A crude estimate of the worth of having one type of unit. */

int
basic_worth(u)
int u;
{
    int worth = 0, u2, r, range;
  
    worth += u_hp(u) * 10;
    for_all_unit_types(u2) {
	if (could_create(u, u2))
	  worth += (u_bw(u2) * (50)) / 1 /* uu_make(u, u2) */;
	if (could_carry(u, u2))
	  worth += (1 + u_speed(u)) * uu_capacity_x(u, u2) *
	    (u_is_base(u) ? 10 : 1) * u_bw(u2) / 30;
    }
    range = 12345;
    for_all_material_types(r) {
	worth += um_base_production(u, r) * (u_is_base(u) ? 4 : 1);
	if (um_consumption_per_move(u, r) > 0)
	  range = min(range, um_storage_x(u, r) / max(1, um_consumption_per_move(u, r)));
	if (um_base_consumption(u, r) > 0) 
	  range =
	    min(range, u_speed(u) * um_storage_x(u, r) / max(1, um_base_consumption(u, r)));
    }
    worth += u_speed(u) * u_hp(u);
    worth += (range == 12345 ? area.maxdim : range)
      * u_hp(u) / max(1, 10 - u_speed(u));
    for_all_unit_types(u2) {
	worth += (worth * uu_capture(u, u2)) / 150;
    }
    worth = isqrt(worth);
    DMprintf("unit type %s worth %d \n ", u_type_name(u), worth);
    return worth;
}

/* A crude estimate of the worth of having a type of unit for offense. */

int
offensive_worth(u)
int u;
{
    int worth = 0, u2 /* ,  r, range */;

    if (mobile(u)) {
    	for_all_unit_types(u2) {
    		if (basic_hit_worth(u, u2) > 0) worth += basic_hit_worth(u, u2);
    	}
    }
#if 0  
    worth += u_hp(u) * 10;
    for_all_unit_types(u2) {
	if (could_carry(u, u2))
	  worth += (1 + u_speed(u)) * uu_capacity_x(u, u2) *
	    (u_is_base(u) ? 10 : 1) * u_bw(u2) / 30;
    }
    worth += u_speed(u) * u_hp(u);
    worth += (range == 12345 ? area.maxdim : range)
      * u_hp(u) / max(1, 10 - u_speed(u));
    for_all_unit_types(u2) {
	worth += (worth * uu_capture(u, u2)) / 150;
    }
    worth = isqrt(worth);
#endif
    DMprintf("unit type %s worth %d \n ", u_type_name(u), worth);
    return worth;
}

/* A basic estimate of the payoff of one unit type attacking another type
   directly.  This is "context-free", does not account for overall goals etc. */

/* (should account for number of attacks possible in one turn) */

int
basic_hit_worth(u, e)
int u, e;
{
    int dam, numdice, numspots, offset, avgdamage, worth = 0, anti = 0;

    dam = uu_damage(u, e);
    DICE(dam, numdice, numspots, offset);
    avgdamage = offset + (numdice * numspots) / 2;
    if (avgdamage > u_hp(e)) avgdamage = u_hp(e);
    worth = (uu_hit(u, e) * avgdamage) / u_hp(e);
    if (1 /* strength of counterattack */) {
	dam = uu_damage(e, u);
	DICE(dam, numdice, numspots, offset);
	avgdamage = offset + (numdice * numspots) / 2;
	if (avgdamage > u_hp(u)) avgdamage = u_hp(u);
	anti = (uu_hit(e, u) * avgdamage) / u_hp(u);
    }
    return worth - (anti * 9) / 10;
}

/* A crude estimate of the payoff of one unit type trying to capture. */

int
basic_capture_worth(u, e)
int u, e;
{
    int worth1 = 0, worth2 = 0;

    if (uu_capture(u, e) > 0) {
	worth1 = uu_capture(u, e) * u_acp(u) /* divided by acp/attack */;
    }
    if (uu_indep_capture(u, e) > 0) {
	worth2 = uu_indep_capture(u, e) * u_acp(u) /* divided by acp/attack */;
    }
    return max(worth1, worth2);
}

int
basic_transport_worth(u, u2)
int u, u2;
{
    int worth = 0;

    if (could_carry(u, u2)) {
	worth += 1;
    }
    return worth;
}

/* Some notion of the unit's "strength"? */

int
unit_strength(u)
int u;
{
    return 1;
}

/* Display the results of our calculations. */

void
display_assessment()
{
    int t, u, u2;

    DMprintf("\n\nEstimated Terrain Percentages:\n");
    for_all_terrain_types(t) {
	DMprintf(" %3d%%", t_fraction(t));
    }
    DMprintf("\nUnit Attributes:\n");
    for_all_unit_types(u) {
	DMprintf(" %-3.3s : base %d, transport %d, carrier %d, worth %d\n",
	       shortest_unique_name(u), u_is_base(u),
	       u_is_transport(u), u_is_carrier(u), u_bw(u));
	DMprintf("    Operate between ranges %d and %d\n", operating_range_worst(u), operating_range_best(u));
    }
    DMprintf("\nUnit vs Unit Combat:\n");
    for_all_unit_types(u) {
	for_all_unit_types(u2) DMprintf("%5d", uu_zz_bhw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Capture:\n");
    for_all_unit_types(u) {
	for_all_unit_types(u2) DMprintf(" %4d", uu_zz_bcw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\nUnit vs Unit Transport:\n");
    for_all_unit_types(u) {
	for_all_unit_types(u2) DMprintf(" %4d", uu_zz_btw(u, u2));
	DMprintf("\n");
    }
    DMprintf("\n");
}

void
mplayer_react_to_unit_loss(side, unit)
Side *side;
Unit *unit;
{
    int x = unit->x, y = unit->y;
    Theater *th;

    if (!inside_area(x, y)) {
	x = unit->prevx;  y = unit->prevy;
    }
    if (!inside_area(x, y)) return;
    if (mplayer(side) && (th = theater_at(side, x, y)) != NULL) {
    	++(th->units_lost);
    }
}

int
is_base_for(u1, u2)
int u1, u2;
{
    return (u_speed(u1) == 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

int
is_carrier_for(u1, u2)
int u1, u2;
{
    return (u_speed(u1) > 0
	    && (uu_capacity_x(u2, u1) > 0
		|| (uu_size(u2, u1) <= u_capacity(u1))));
}

/* Since *.def parameters don't have setters usually, we have to supply
   some here.  These are very sensitive to how the parameters are organized,
   and they don't do any checking, so you have to careful about using them. */

void set_u_is_base(u, n) int u, n; {  utypes[u].is_base = n;  }
void set_u_is_transport(u, n) int u, n; {  utypes[u].is_transport = n;  }
void set_u_is_carrier(u, n) int u, n; {  utypes[u].is_carrier = n;  }
void set_u_is_base_builder(u, n) int u, n; {  utypes[u].is_base_builder = n;  }
void set_u_can_make(u, n) int u, n; {  utypes[u].can_make = n;  }
void set_u_can_capture(u, n) int u, n; {  utypes[u].can_capture = n;  }
void set_u_bw(u, n) int u, n; {  utypes[u].bw = n;  }
void set_t_fraction(t, n) int t, n; {  ttypes[t].fraction = n;  }

int bhwtab = -1;
int bcwtab = -1;
int btwtab = -1;

void
set_uu_bhw(u1, u2, v)
int u1, u2, v;
{
    if (bhwtab < 0) {
	for (bhwtab = 0; tabledefns[bhwtab].name != NULL; ++bhwtab) {
	    if (strcmp("zz-basic-hit-worth", tabledefns[bhwtab].name) == 0) {
		allocate_table(bhwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[bhwtab].table == NULL)
      run_error("no bhw table allocated");
    (*(tabledefns[bhwtab].table))[numutypes * u1 + u2] = v;
}

void
set_uu_bcw(u1, u2, v)
int u1, u2, v;
{
    if (bcwtab < 0) {
	for (bcwtab = 0; tabledefns[bcwtab].name != NULL; ++bcwtab) {
	    if (strcmp("zz-basic-capture-worth", tabledefns[bcwtab].name) == 0) {
		allocate_table(bcwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[bcwtab].table == NULL)
      run_error("no bcw table allocated");
    (*(tabledefns[bcwtab].table))[numutypes * u1 + u2] = v;
}


void
set_uu_btw(u1, u2, v)
int u1, u2, v;
{
    if (btwtab < 0) {
	for (btwtab = 0; tabledefns[btwtab].name != NULL; ++btwtab) {
	    if (strcmp("zz-basic-transport-worth", tabledefns[btwtab].name) == 0) {
		allocate_table(btwtab, FALSE);
		break;
	    }
	}
    }
    if (tabledefns[btwtab].table == NULL)
      run_error("no btw table allocated");
    (*(tabledefns[btwtab].table))[numutypes * u1 + u2] = v;
}
