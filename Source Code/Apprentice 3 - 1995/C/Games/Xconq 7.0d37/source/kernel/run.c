/* The main simulation-running code in Xconq.
   Copyright (C) 1986, 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is the main simulation-running code. */

#include "conq.h"

extern int visible_to PROTO ((Unit *unit, Unit *unit2));

static void maybe_surrender_to PROTO ((Unit *unit, Unit *unit2));

static void run_turn_start PROTO ((void));
static void run_restored_turn_start PROTO ((void));
static void init_movement PROTO ((void));
static void init_actionvector PROTO ((void));
static void run_tech_leakage PROTO ((void));
static void cache_init_tech_levels PROTO ((void));
static void run_tooling_attrition PROTO ((void));
static void reset_all_reserves PROTO ((void));
static void compute_moves PROTO ((void));
static int move_some_units PROTO ((int lim));
static int unit_still_acting PROTO ((Unit *unit, int checkwaiting));
static int move_one_unit_multiple PROTO ((Unit *unit, int lim));
static void finish_movement PROTO ((void));
static void test_agreements PROTO ((void));
static void compute_sun PROTO ((void));
static void run_sun PROTO ((void));
static void compute_season PROTO ((void));
static void run_environment PROTO ((void));
static void damage_unit_with_temperature PROTO ((Unit *unit, int n));
static void mix_winds PROTO ((void));
static int spying_possible PROTO ((void));
static void run_spies PROTO ((void));
static void run_random_events PROTO ((void));
static int init_accidents PROTO ((void));
static void run_accidents PROTO ((void));
static int init_attrition PROTO ((void));
static void run_attrition PROTO ((void));
static int init_revolts PROTO ((void));
static void run_revolts PROTO ((void));
static void unit_revolt PROTO ((Unit *unit));
static int init_surrenders PROTO ((void));
static void run_surrenders PROTO ((void));
static void unit_surrender PROTO ((Unit *unit));
static int excess_left PROTO ((int x, int y));
static void try_transfer_to_cell PROTO ((int x, int y));
static int sharable_left PROTO ((int x, int y));
static void try_sharing_with_cell PROTO ((int x, int y));
static void run_economy PROTO ((void));
static int base_production PROTO ((Unit *unit, int m));
static void try_transfer PROTO ((Unit *from, Unit *to, int r));
static void try_sharing PROTO ((Unit *from, Unit *to, int m));
static void try_transfer_aux PROTO ((Unit *from, Unit *to, int r));
static int can_satisfy_need PROTO ((Unit *unit, int r, int need));
static void run_turn_end PROTO ((void));
static void run_people_consumption PROTO ((void));
static void run_cell_consumption PROTO ((void));
static void run_unit_base_consumption PROTO ((void));
static void unit_consumes PROTO ((Unit *unit));
static int in_supply PROTO ((Unit *unit, int m));
static void run_self_builds PROTO ((void));
static void run_people_side_changes PROTO ((void));
static void update_cell_display_all_sides PROTO ((int x, int y, int rightnow));
static void run_appearances PROTO ((void));
static void run_disappearances PROTO ((void));
static void run_hp_recovery PROTO ((void));
static int season_effect PROTO ((int u));
static void run_detonation_accidents PROTO ((void));
static void run_people_limits PROTO ((void));
static void maybe_detonate_accidently PROTO ((Unit *unit));
static void spy_on_location PROTO ((int x, int y));

int num_people_at PROTO ((int x, int y));

/* The number of the current turn within a year. */

int curyearpart = -1;

/* The season name for the current turn. */

char *curseasonname = NULL;

/* This is the main array that tracks units acting during this turn. */

UnitVector *actionvector = NULL;

int curpriority = 0;

/* The table of all types of random events. */

struct randomeventtype {
    int key;
    int (*initfn) PROTO ((void));
    void (*fn) PROTO ((void));
} randomeventmethods[] = {
    { K_ACCIDENTS_IN_TERRAIN, init_accidents, run_accidents },
    { K_ATTRITION_IN_TERRAIN, init_attrition, run_attrition },
    { K_UNITS_REVOLT, init_revolts, run_revolts },
    { K_UNITS_SURRENDER, init_surrenders, run_surrenders },
    { 0, NULL, NULL }
};

int numrandomevents = 0;

int randomeventindices[10]; /* this must be >= number of diff methods */

int maintimeout = -1;

int paused = FALSE;

/* State variables. */
/* (I don't think all of these are strictly necessary) */

/* This becomes TRUE the first time run_game is executed. */

int gameinited = FALSE;

/* This is true only before the game actually starts. */

int beforestart = TRUE;

/* This is true only at the beginning of a turn. */

int at_turn_start = FALSE;

/* This is true after the game is over. */

int endofgame = FALSE;

/* How often to do saves while playing. */

int checkpointinterval = 0;

/* This is set FALSE whenever the game state changes, and TRUE whenever
   the game has been saved. */

int gamestatesafe = TRUE;

/* This is TRUE after the designer has been mucking around, or if
   networked versions are found to be inconsistent. */

int compromised = FALSE;

/* True whenever the game has both day and night. */

int daynight = FALSE;

/* The location of the sun, as a position relative to the area.  The
   actual values may be far outside the area. */

int sunx, suny;

/* The sun's previous location. */

int lastsunx = -1, lastsuny = -1;

/* The time at which the game actually starts. */

time_t game_start_in_real_time;

/* The point in the turn at which players can actually do things. */

time_t turn_play_start_in_real_time;

int planexecs;

int taskexecs;

int any_tooling_attrition = -1;

int any_self_builds = -1;

int any_appearances = -1;

int any_people_side_changes = -1;

int *any_people_surrenders = NULL;

int any_hp_recovery = -1;

int any_tech_leakage = -1;

int any_detonation_accidents = -1;

int any_unit_production = -1;

int any_terrain_production = -1;

int any_people_production = -1;

int any_people_consumption = -1;

int any_cell_consumption = -1;

int any_unit_base_consumption = -1;

int any_people_max = -1;

int any_spying = -1;

int any_disappearances = -1;

/* This function does a (small, usually) amount of simulation, then returns.
   It can be run multiple times at any time, will not go "too far".
   It returns the max time in seconds that the interface should do
   things before running this again. */

int
run_game(maxactions)
int maxactions;
{
    int numacted, othersdone, runtime;
    time_t rungamestart, rungameend;
    Side *side;

    gameinited = TRUE;
    time(&rungamestart);
    if (beforestart) {
	/* If we haven't started yet, see if it's time. */
	test_for_game_start();
	Dprintf("run_game: tested for game start.\n");
    } else if (endofgame) {
	/* Nothing to do except wait for users to do exit commands. */
    	Dprintf("run_game: at end of game.\n");
    } else if (paused) {
	/* Don't do anything if we're paused. */
    	Dprintf("run_game: paused.\n");
    } else {
	if (at_turn_start) {
	    if (midturnrestore)
	      run_restored_turn_start();
	    else
	      run_turn_start();
	    check_all_units();
	    compose_actionvector();
	    init_movement();
	    update_all_progress_displays("", -1);
	    /* Game might have been ended by new turn init. */
	    if (endofgame) {
	    	Dprintf("run_game: game ended by new turn init.\n");
	    	return 0;
	    }
	    time(&turn_play_start_in_real_time);
	    at_turn_start = FALSE;
	}
	for_all_sides(side) {
	    /* If this is running in realtime, update all clock displays. */
	    if (side->ingame && side_has_display(side) && realtime_game()) {
		update_clock_display(side, TRUE);
	    }
	    /* Non-participating sides are automatically "finished". */
	    if (side->ingame
	        && !side->finishedturn
		&& !side_has_ai(side)
		&& !side_has_display(side)) {
		Dprintf("run_game: %s finished - no AI and/or display.\n",
			side_desig(side));
		finish_turn(side);
	    }
	}
	/* If all sides are done acting, end the turn.  This won't be true
	   right at the start of a turn. */
	if (all_sides_finished() || exceeded_rt_per_turn()) {
	    run_turn_end();
	    Dprintf("run_game: at turn end.\n");
	    at_turn_start = TRUE;
	} else {
	    /* Move some units around. */
	    numacted = move_some_units(maxactions);
	    othersdone = TRUE;
	    for_all_sides(side) {
		if (!side->finishedturn) {
		    /* Display and/or AI might have vanished? */
		    if (!side_has_ai(side) && !side_has_display(side)) {
		    	Dprintf("run_game: %s finished - AI and/or display gone.\n",
				side_desig(side));
			finish_turn(side);
		    }
		    /* See if any sides auto-finish. */
		    if (!units_still_acting(side)
			&& side->autofinish
			&& !side->designer) {
		    	Dprintf("run_game: %s auto-finishes.\n", side_desig(side));
			finish_turn(side);
		    }
		}
		if (!side->finishedturn && side_has_display(side)) {
		    othersdone = FALSE;
		}
	    }
	    if (numacted == 0 && othersdone) {
		for_all_sides(side) {
		    if (!side->finishedturn
			&& side_has_ai(side)
			&& !side_has_display(side)) {
			Dprintf("run_game: %s can't think of what to do, finishes.\n",
				side_desig(side));
			finish_turn(side);
		    }
		}
	    }
	    if (Debug) {
	    	Dprintf("run_game: ");
		switch (maxactions) {
		  case -1:
		    Dprintf("%d actions.", numacted);
		    break;
		  case 0:
		    Dprintf("No actions.");
		    break;
		  case 1:
		    Dprintf("%d/1 action.", numacted);
		    break;
		  default:
		    Dprintf("%d/%d actions.", numacted, maxactions);
		    break;
		}
		if (planexecs > 0)
		  Dprintf(" (%d plan execs)", planexecs);
		if (taskexecs > 0)
		  Dprintf(" (%d task execs)", taskexecs);
		/* (also number of units considered?) */
	    	Dprintf("\n");
	    }
	}
	check_realtime();
	test_for_game_end();
    }
    time(&rungameend);
    runtime = idifftime(rungameend, rungamestart);
    if (runtime > 0)
      Dprintf("run_game: took %d seconds\n", runtime);
    return 0;
}

/* See if game is ready to get underway for real.  Note that displays will
   work before the game has started, but game time doesn't move. */

void
test_for_game_start()
{
    int anydisplays = FALSE;
    Side *side;

    /* We must have at least one unit, on a side that is being displayed,
       before the game can start for real. */
    for_all_sides(side) {
	if (side_has_display(side)) {
	    anydisplays = TRUE;
	}
	if (side_has_units(side) && side_has_display(side)) {
	    /* Now we're really ready to roll. */
	    beforestart = FALSE;
	    at_turn_start = TRUE;
	    if (midturnrestore)
	      record_event(H_GAME_RESTARTED, ALLSIDES);
	    else
	      record_event(H_GAME_STARTED, ALLSIDES);
	    /* Record the game as starting NOW in real time. */
	    time(&game_start_in_real_time);
	    /* No need to look at any more sides, just get on with the game. */
	    return;
	}
    }
    if (!anydisplays) {
	init_warning("No sides have a display");
    }
}

/* This routine looks to see if the game is completely over. */

void
test_for_game_end()
{
    Side *side;

    /* Declare a draw if everybody is amenable. */
    if (all_others_willing_to_quit(NULL)) {
    	/* (should remove all sides first?) */
	end_the_game();
    }
    for_all_sides(side) {
    	/* If we have an active side being displayed, we're not done yet. */
	if (side->ingame && side_has_display(side))
	  return;
	/* (If no displayed sides have units, turns will whiz by) */
    }
    end_the_game();
}

#if 0
	/* Add this time interval to all sides's time consumption. */
	for_all_sides(side) {
	    int delta_time = (time(0) - side->lasttime);

	    if (side->more_units) {
		side->timeleft -= delta_time;
		side->timetaken += delta_time;
		update_clock_display(side, TRUE);
	    }
	}
#endif


/* This is true when all participating sides have finished their turn. */

int
all_sides_finished()
{
    Side *side;

    for_all_sides(side) {
	if (side->ingame && !side->finishedturn) {
	    return FALSE;
	}
    }
    return TRUE;
}

/* Call this from interfaces to check on realtime details without actually
   going into run_game.  Will call back to interface if necessary. */

void
check_realtime()
{
    Side *side;

    if (!realtime_game())
      return;
    if (exceeded_rt_for_game())
      end_the_game();
    if (g_rt_per_side() > 0) {
	for_all_sides(side) {
	    if (side->ingame && side->totaltimeused > g_rt_per_side()) {
		remove_side_from_game(side);
	    }
	}
    }
}

int
exceeded_rt_for_game()
{
    time_t now;

    if (g_rt_for_game() <= 0)
      return FALSE;
    time(&now);
    return (idifftime(now, game_start_in_real_time) + g_elapsed_time() > g_rt_for_game());
}

int
exceeded_rt_per_turn()
{
    time_t now;

    if (g_rt_per_turn() <= 0)
      return FALSE;
    time(&now);
    return (idifftime(now, turn_play_start_in_real_time) > g_rt_per_turn());
}

/* This returns true if the given side is still wanting to do stuff. */

int
units_still_acting(side)
Side *side;
{
    Unit *unit;

    if (side->ingame) {
	for_all_side_units(side, unit) {
	    if (unit_still_acting(unit, FALSE)) {
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/* Do everything that would happen before movement in a turn. */

static void
run_turn_start()
{
    int curturn;
    time_t turncalcstart, turncalcend;
    Side *side;

    /* Increment the turn number. */
    curturn = g_turn();
    ++curturn;
    set_g_turn(curturn);
    /* See if we've hit the preset end of the game. */
    if (curturn > g_last_turn() && !probability(g_extra_turn())) {
	end_the_game();
	/* The game is over, don't bother with the other calcs. */
	return;
    }
    time(&turncalcstart);
    update_all_progress_displays("turn start calcs", -1);
    compute_season();
    Dprintf("##### TURN %d (%s) #####\n",
	    curturn, absolute_date_string(curturn));
    for_all_sides(side) {
	side->finishedturn = FALSE;
	update_turn_display(side, TRUE);
	if (realtime_game()) {
	    update_clock_display(side, TRUE);
	}
    }
    run_sun();
    run_environment();
    run_economy();
    run_hp_recovery();
    run_self_builds();
    run_appearances();
    run_random_events();
    run_detonation_accidents();
    sort_units();
    init_actionvector();
    curpriority = 0;
    compute_moves();
    run_spies();
    run_tech_leakage();
    run_tooling_attrition();
    cache_init_tech_levels();
    reset_all_reserves();
    gamestatesafe = FALSE;
    if ((checkpointinterval > 0) && (curturn % checkpointinterval == 0)) {
	write_entire_game_state(checkpoint_filename());
    }
    time(&turncalcend);
    Dprintf("%d seconds to calc at turn start\n",
	    idifftime(turncalcend, turncalcstart));
}

/* Do computations to start the first turn of a restored game. */

static void
run_restored_turn_start()
{
    Side *side;

    Dprintf("##### TURN %d (%s) #####\n",
	    g_turn(), absolute_date_string(g_turn()));
    for_all_sides(side) {
	update_turn_display(side, TRUE);
	if (realtime_game()) {
	    update_clock_display(side, TRUE);
	}
    }
    compute_sun();
    sort_units();
    init_actionvector();
    /* We can set the priority arbitrarily now, since calculations will
       shortly move it to the correct value. */
    curpriority = 0;
    /* We're done with restore-specific tweaks, turn the flag off. */
    midturnrestore = FALSE;
}

static void
init_actionvector()
{
    if (actionvector == NULL)
      actionvector = make_unit_vector(max(numunits * 2, 100));
    clear_unit_vector(actionvector);
}

static void
init_movement()
{
    int i;
    Side *side, *side2;

    for_all_sides(side) {
	if (side->ingame) {
	    /* No units are waiting for orders initially. */
	    side->numwaiting = 0;
	}
    }
    i = 0;
    for_all_sides(side) {
	side->turnstarttime = time(0);
	/* Didn't really do input, but useful to pretend so. */
	side->lasttime = time(0);
	if (g_use_side_priority())
	  side->priority = i++;
	if (side_has_ai(side))
	  ai_init_turn(side);
	side->busy = FALSE;
	if (side_has_display(side))
	  update_action_display(side, TRUE);
    }
    /* Inform sides with displays that all units are ready to act. */
    for_all_sides(side) {
	if (side_has_display(side)) {
	    for_all_sides(side2) {
		update_side_display(side, side2, TRUE);
	    }
	}
    }
}

/* Compute the leakage of technology from one side to another. */

static void
run_tech_leakage()
{
    int u;
    Side *side, *side2;

    if (any_tech_leakage < 0) {
	any_tech_leakage = FALSE;
	for_all_unit_types(u) {
	    if (u_tech_leakage(u) > 0) {
		any_tech_leakage = TRUE;
		break;
	    }
	}
    }
    if (!any_tech_leakage)
      return;
    Dprintf("Running tech leakage\n");
    for_all_sides(side) {
	for_all_sides(side2) {
	    if (side != side2 /* and some contact between sides */) {
		for_all_unit_types(u) {
		    if (side->tech[u] < side2->tech[u]
			&& u_tech_leakage(u) > 0) {
			side->tech[u] += prob_fraction(u_tech_leakage(u));
		    }
		}
	    }
	}
    }
}

/* Remember each side's tech levels before it does any research actions
   during the turn.  This can be used to keep tech level from going up too
   fast if the player has lots of units doing research. */

static void
cache_init_tech_levels()
{
    int u;
    Side *side;

    if (using_tech_levels()) {
	for_all_sides(side) {
	    for_all_unit_types(u) {
		side->inittech[u] = side->tech[u];
	    }
	}
    }
}

/* Reduce some units' construction tooling randomly. */

static void
run_tooling_attrition()
{
    int u, u2, att;
    Unit *unit;

    /* Test whether tooling attrition is ever possible. */
    if (any_tooling_attrition < 0) {
	any_tooling_attrition = FALSE;
	for_all_unit_types(u) {
	    for_all_unit_types(u2) {
		if (uu_tp_attrition(u, u2) > 0) {
		    any_tooling_attrition = TRUE;
		    break;
		}
	    }
	    if (any_tooling_attrition)
	      break;
	}
    }
    if (!any_tooling_attrition)
      return;
    for_all_units(unit) {
	if (is_active(unit) && unit->tooling != NULL) {
	    for_all_unit_types(u2) {
		att = uu_tp_attrition(unit->type, u2);
		if (att > 0) {
		    unit->tooling[u2] -= prob_fraction(att);
		}
		if (unit->tooling[u2] < 0) unit->tooling[u2] = 0;
	    }
	}
    }
}

static void
reset_all_reserves()
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->plan != NULL) {
	    unit->plan->reserve = FALSE;
	}
    }
}

/* Compute moves and actions for all the units at once, put everybody that
   can do anything into a list. */

static void
compute_moves()
{
    int curturn = g_turn();
    Unit *unit;
    Side *side;

    for_all_sides(side) {
	side->numacting = 0;
	side->numfinished = 0;
	for_all_side_units(side, unit) {
	    if (unit->act) {
		/* Unit acp is set to -1 to indicate uninitialization,
		   but acp is computed by adding to the previous acp,
		   so when starting a new game (as opposed to
		   restoring an old one), acp should be inited to
		   zero.  (This could maybe be done better.) */
		if (curturn == 1)
		  unit->act->acp = 0;
		compute_acp(unit);
		update_unit_acp_display(side, unit, FALSE);
	    }
	}
    }
}

void
compose_actionvector()
{
    Unit *unit;

    for_all_units(unit) {
	if (unit->act && unit->act->initacp > 0 && unit_priority(unit) == curpriority) {
	    actionvector = add_unit_to_vector(actionvector, unit, 0);
	    /* Clear all delay flags. */
	    if (unit->plan)
	      unit->plan->delayed = FALSE;
	}
    }
    Dprintf("Action vector has %d units, at priority %d\n",
	    actionvector->numunits, curpriority);
}

int
unit_priority(unit)
Unit *unit;
{
    if (unit->side)
      return unit->side->priority;
    return 0;
}

/* Compute the action points available to the unit this turn. */

void
compute_acp(unit)
Unit *unit;
{
    int u = unit->type, t, acp, maxacp, minacp;
    Obj *dmgeffect;
    Unit *occ;

    /* Units still under construction or off-area can't do anything. */
    if (!completed(unit) || !inside_area(unit->x, unit->y)) {
	unit->act->initacp = unit->act->acp = unit->act->actualactions = 0;
	return;
    }
    /* First compute how many action points are available. */
    /* Start with basic acp, normal or damaged as appropriate. */
    if (unit->hp < u_hp_max(u)
	&& (dmgeffect = u_acp_damage_effect(u)) != lispnil) {
	acp = damaged_acp(unit, dmgeffect);
    } else {
	acp = u_acp(u);
    }
    /* Adjust for occupants. */
    for_all_occupants(unit, occ) {
	if (is_active(occ)) {
	    acp = (acp * uu_acp_occ_effect(occ->type, u)) / 100;
	}
    }
    /* Adjust for night time. */
    if (night_at(unit->x, unit->y)) {
    	/* (should account for unit being on a road at night, etc) */
	t = terrain_at(unit->x, unit->y);
    	acp = (acp * ut_acp_night_effect(u, t)) / 100;
    }
    /* (should) Adjust for season. */
    if (u_acp_season_effect(u) != lispnil) {
    	acp = (acp * season_effect(u)) / 100;
    }
    /* Clip to upper and lower acp-per-turn limits. */
    acp = max(acp, u_acp_turn_min(u));
    if (u_acp_turn_max(u) >= 0)
      acp = min(acp, u_acp_turn_max(u));
    /* Increment the unit's available acp by the acp we get for this turn. */
    unit->act->initacp = unit->act->acp + acp;
    /* Now clip the unit's accumulated acp to its limits. */
    minacp = u_acp_min(u);
    maxacp = (u_acp_max(u) < 0 ? acp : u_acp_max(u));
    unit->act->initacp = min(max(minacp, unit->act->initacp), maxacp);
    /* Current acp is now the initial acp. */
    unit->act->acp = unit->act->initacp;
    unit->act->actualactions = unit->act->actualmoves = 0;
}

/* Compute and return the acp of a damaged unit, using a list of (hp acp) pairs
   and interpolating between them. */

int
damaged_acp(unit, effect)
Unit *unit;
Obj *effect;
{
    int u = unit->type, hp = unit->hp, thishp, thisacp, nexthp, nextacp, rslt;
    Obj *rest;

    for (rest = effect; rest != lispnil; rest = cdr(rest)) {
	thishp = c_number(car(car(rest)));
	thisacp = c_number(cadr(car(rest)));
	if (cdr(rest)) {
	    nexthp = c_number(car(cadr(rest)));
	    nextacp = c_number(cadr(cadr(rest)));
	} else {
	    nexthp = u_hp_max(u);
	    nextacp = u_acp(u);
	}
	if (unit->hp < thishp) {
	    /* Interpolate between thishp and 0. */
	    return (thisacp * hp) / thishp;
	} else if (between(thishp, unit->hp, nexthp)) {
	    rslt = thisacp;
	    if (unit->hp != thishp) {
		/* Add the linear interpolation. */
	    	rslt += ((nextacp - thisacp) * (hp - thishp)) / (nexthp - thishp);
	    }
	    return rslt;
	}
    }
    return u_acp(u);
}

static void
compute_season()
{
    Obj *names, *rest, *elt;

    curseasonname = NULL;
    if (world.yearlength > 1) {
	curyearpart = (g_turn() + g_initial_year_part()) % world.yearlength;
	/* Determine the name of the season, if defined. */
	names = g_season_names();
	if (names != NULL && names != lispnil && consp(names)) {
	    for (rest = names; rest != lispnil; rest = cdr(rest)) {
		elt = car(rest);
		if (consp(elt)
		    && numberp(car(elt))
		    && numberp(cadr(elt))
		    && between(c_number(car(elt)), curyearpart, c_number(cadr(elt)))
		    && stringp(car(cddr(elt))))
		  curseasonname = c_string(car(cddr(elt)));
	    }
	}
    } else {
	curyearpart = 0;
    }
}

static int
season_effect(u)
int u;
{
    int rslt = 100, thisyearpart, thiseffect, nextyearpart, nexteffect;
    Obj *effects, *rest;

    if (curyearpart < 0)
      compute_season();
    effects = u_acp_season_effect(u);
    for (rest = effects; rest != lispnil; rest = cdr(rest)) {
    	if (!numberp(car(car(rest))))
    	  run_warning("not a number");
	thisyearpart = c_number(car (car(rest)));
    	if (!numberp(cadr(car(rest))))
    	  run_warning("not a number");
	thiseffect = c_number(cadr(car(rest)));
	if (cdr(rest)) {
	    nextyearpart = c_number(car (cadr(rest)));
	    nexteffect = c_number(cadr(cadr(rest)));
	} else {
	    /* Cycles around to the beginning of the list. */
	    nextyearpart = c_number(car(car(effects))) + world.yearlength;
	    nexteffect = c_number(cadr(car(effects)));
	}
	if (between(thisyearpart, curyearpart, nextyearpart)) {
	    rslt = thiseffect;
	    if (curyearpart != thisyearpart) {
		/* Add the linear interpolation. */
	    	rslt += ((nexteffect - thiseffect) * (curyearpart - thisyearpart))
	    	         / (nextyearpart - thisyearpart);
	    }
	    return rslt;
	}
    }
    return rslt;
}

/* Do some number of actions. */

static int
move_some_units(lim)
int lim;
{
    int num = 0, foundanytomove, curactor, numdelayed;
    Unit *unit;

    /* Negative limits are effectively infinite. */
    if (lim < 0)
      lim = 100000000;
  tryagain:
    foundanytomove = FALSE;
    numdelayed = 0;
    for (curactor = 0; curactor < actionvector->numunits; ++curactor) {
	unit = (actionvector->units)[curactor].unit;
	if (unit->plan && unit->plan->delayed) {
	    ++numdelayed;
	    continue;
	}
	/* If the unit is keeping formation, then give it a chance to
	   adjust its position. */
	if (is_active(unit)
	    && (unit->side ?
		(unit->side->ingame && !unit->side->finishedturn) : TRUE)
	    && (unit->act && unit->act->acp > 0)
	    && (unit->plan && unit->plan->formation)) {
	    num += move_one_unit_multiple(unit, lim - num);
	    foundanytomove = TRUE;
	}
	if (unit_still_acting(unit, TRUE)) {
	    num += move_one_unit_multiple(unit, lim - num);
	    foundanytomove = TRUE;
	}
	if (unit_still_acting(unit, FALSE)) {
	    foundanytomove = TRUE;
	}
	if (num >= lim) 
	  return num;
    }
    if (!foundanytomove && numdelayed > 0) {
	for (curactor = 0; curactor < actionvector->numunits; ++curactor) {
	    unit = (actionvector->units)[curactor].unit;
	    if (unit->plan)
	      unit->plan->delayed = FALSE;
	}
	goto tryagain;
    }
    if (!foundanytomove) {
    	if (g_use_side_priority() && curpriority < 2 * numsides) {
		++curpriority;
		compose_actionvector();
		goto tryagain;
	}
    }
    return num;
}

static int
unit_still_acting(unit, checkwaiting)
Unit *unit;
int checkwaiting;
{
    return (is_active(unit)
	    && (unit->side
		&& unit->side->ingame
		&& !unit->side->finishedturn)
	    && (unit->act
		&& unit->act->acp > 0)
	    && ((unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& (checkwaiting ? !unit->plan->waitingfortasks : TRUE))
		|| has_pending_action(unit)));
}

/* Do a single unit's actions, up to the given limit or until it runs
   out of things it wants to do (or something happens to it). */

static int
move_one_unit_multiple(unit, lim)
Unit *unit;
int lim;
{
    int num = 0, buzz = 0, acp1;
    int rslt;

    if (unit->act == NULL || unit->act->initacp < 1)
      return 0;
    acp1 = unit->act->acp;
    while (is_active(unit)
	   && (unit->act
	       && unit->act->acp > u_acp_min(unit->type))
	   && ((unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& !unit->plan->delayed)
	       || has_pending_action(unit))
	   && num < lim
	   && buzz < lim) {
	if (has_pending_action(unit)) {
	    /* Execute the action directly. */
	    rslt = execute_action(unit, &(unit->act->nextaction));
	    /* Clear the action.  Note that the unit might have changed
	       to a non-acting type, so we have to check for act struct. */
	    if (unit->act)
	      unit->act->nextaction.type = A_NONE;
	    /* In any case, the game state is irrevocably altered. */
	    gamestatesafe = FALSE;
	    ++num;
	} else if (unit->plan) {
	    if (unit->plan->formation && move_into_formation(unit)) {
		execute_plan(unit, 1);
		gamestatesafe = FALSE;
		++buzz;
	    }
	    if (unit->plan->waitingfortasks
		|| unit->plan->asleep
		|| unit->plan->reserve
		|| unit->plan->delayed)
	      break;
	    execute_plan(unit, 1);
	    gamestatesafe = FALSE;
	    ++buzz;
	} else {
	    run_warning("Planless \"%s\" was asked to act", unit_desig(unit));
	    ++buzz;
	}
	/* This should never happen? */
	if (unit->act && unit->act->acp == acp1 && num > 1) {
	    /* Blast the action. */
	    unit->act->nextaction.type = A_NONE;
	    /* Blast the plan. */
	    if (unit->plan)
	      unit->plan->type = PLAN_NONE;
	    if (unit->plan && probability(5))
	      unit->plan->asleep = TRUE;
	    Dprintf("%s was confused, starting over\n", unit_desig(unit));
	}
    }
    return num;
}

/* This explicitly finishes out a side's activity for the turn. */

void
finish_turn(side)
Side *side;
{
    Side *side2;

    /* Flag the side as being done for this turn. */
    side->finishedturn = TRUE;
    /* Stop counting down our time consumption. */
    side->totaltimeused += (time(0) - side->turnstarttime);
    /* Clue everybody in. */
    for_all_sides(side2) {
	update_side_display(side2, side, TRUE);
    }
    Dprintf("%s finished its turn.\n", side_desig(side));
}

/* Take care of details that no longer require any interaction, at least
   none that can't wait until the next turn. */

static void
finish_movement()
{
    int lostacp;
    Unit *unit;
    Side *side, *side2;

    for_all_sides (side) {
	/* Remove dead units that have been dead more than a turn. (?) */
	flush_side_dead(side);
	if (Debug) {
	    lostacp = 0;
	    for_all_side_units(side, unit) {
		if (is_active(unit) && unit->act && unit->act->acp > 0) {
		    lostacp += unit->act->acp;
		}
	    }
	    if (lostacp > 0) {
		Dprintf("%s forfeited %d acp overall.\n",
			side_desig(side), lostacp);
	    }
	}
	if (side_has_ai(side)) {
	    ai_finish_movement(side);
	}
    }
    for_all_sides(side)  {
	for_all_sides(side2) {
	    update_side_display(side, side2, TRUE);
	}
    }
}

/* See how any agreements' terms are holding up. */

static void
test_agreements()
{
    Agreement *ag;
    Side *side;
    Unit *unit;

    for_all_agreements(ag) {
	if (ag->state == in_force) {
		/* what? */
	}
    }
}

/* Compute sun-related data. */

static void
compute_sun()
{
    int curtime;

    switch (world.daylength) {
      case 0:
	/* Sun is at a fixed position. */
	daynight = TRUE;
	/* (should be possible to set explicitly somehow? implicitly from lat/long?) */
	sunx = area.width / 2;  suny = area.height / 2;
	break;
      case 1:
	/* No sun effects at all, every place uniformly lit. */
      	daynight = FALSE;
	break;
      default:
	/* Normal days and nights. */
	daynight = TRUE;
	/* If world has a appropriate circumference, the sun moves over
	   it at a regular pace. */
	if (world.circumference >= area.width) {
	    lastsunx = sunx;  lastsuny = suny;
	    curtime = (g_turn() + g_initial_day_part()) % world.daylength;
	    sunx = (curtime * world.circumference) / world.daylength + area.width / 2;
	    /* (should adjust suny for nonzero latitudes and axial tilt) */
	    suny = area.height / 2;
	}
	break;
    }
    if (daynight) {
	Dprintf("Sun is now at %d,%d\n", sunx, suny);
    }
}
/* Compute the position of the sun for this turn. */

static void
run_sun()
{
    int x, y;
    Side *side;

    compute_sun();
    if (world.daylength > 1 && world.circumference >= area.width) {
	/* Update the appearance of any cells whose lighting has changed. */
	for_all_cells(x, y) {
	    if (lighting(x, y, sunx, suny) != lighting(x, y, lastsunx, lastsuny)) {
		for_all_sides(side) {
		    update_cell_display(side, x, y, FALSE);
	    	}
	    }
	}
    }
}

/* Seasonal change moves the average temperature up and down gradually,
   modifying it according to the terrain underneath. */

/* This should check that equator and northpole are not equal. */

static void
run_environment()
{
    int yrlen = world.yearlength;
    int season, x, y, dir, t, lattemp, celltemp;
    int u, temp, comfmin, comfmax, range;
    Unit *unit;
    Side *side;

    if (mintemp == maxtemp && !any_wind_variation_in_layer)
      return;
    if (!temperatures_defined()) {
        allocate_area_temperatures();
        allocate_area_scratch(2);
    }
#if 0
    float spoletemp, npoletemp, equatortemp;
    float poledelta, equatordelta, ntempdelta, stempdelta;

    if (yrlen > 1 /* and axial tilt... */) {
	/* Compute the difference between summer and winter temps. */
	/* (Should be precomputed.) */
	poledelta = (g_summer_pole() - g_winter_pole()) / (yrlen / 2);
	equatordelta = (g_summer_equator() - g_winter_equator()) / (yrlen / 2);
	/* Compute where we are in the cycle of seasons. */
	/* Compute temps at poles and equator. */
	if (curyearpart <= yrlen / 2) {
	    npoletemp = g_winter_pole() + curyearpart * poledelta;
	    equatortemp = g_winter_equator() + curyearpart * equatordelta;
	    spoletemp = g_winter_pole() + (yrlen/2 - curyearpart) * poledelta;
	} else {
	    npoletemp = g_winter_pole() + (yrlen - curyearpart) * poledelta;
	    equatortemp = g_winter_equator() + (yrlen - curyearpart) * equatordelta;
	    spoletemp = g_winter_pole() + (curyearpart - yrlen/2) * poledelta;
	}
	Dprintf("Temps N %f Eq %f S %f\n", npoletemp, equatortemp, spoletemp);
	/* Compute the temperature variation from one lat to the next. */
	ntempdelta = (equatortemp-npoletemp)/(world.northpole-world.equator);
	stempdelta = (equatortemp-spoletemp)/(world.equator-world.southpole);
	}
#endif
    /* Compute the average temperature at each point in the world. */
    for (y = area.height-1; y >= 0; --y) {
	for (x = 0; x < area.width; ++x) {
	    if (!in_area(x, y)) continue;
	    /* Save the prev temp. */
	    set_tmp1_at(x, y, temperature_at(x, y));
	    t = terrain_at(x, y);
	    celltemp = t_temp_avg(t);
	    /* (should account for latitude/season effect here somehow) */
	    /* Add in a random variation if specified. */
	    if (t_temp_variability(t) > 0) {
		celltemp += (xrandom(t_temp_variability(t))
			    - t_temp_variability(t)/2);
	    }
	    /* Higher elevations can be much colder. */
	    /* (In this pos, will influence lowlands via moderation - realistic?) */
	    if (elevations_defined()
	        && g_temp_floor_elev() != 0
	        && elev_at(x, y) < g_temp_floor_elev()) {
		celltemp -=
		    ((celltemp - g_temp_floor()) * elev_at(x, y))
		    / g_temp_floor_elev();
	    }
	    /* Record the (unmoderated) temperature of the cell. */
	    set_temperature_at(x, y, celltemp);
	}
    }
    /* Sometimes the scale of the world is such that neighboring cells
       influence each other's temperatures. */
    if (g_temp_mod_range() > 0) {
	/* only doing a range of 1... */
	for_all_interior_cells(x, y) {
	    set_tmp2_at(x, y, temperature_at(x, y));
	    for_all_directions(dir) {
		set_tmp2_at(x, y,
			    (tmp2_at(x, y)
			     + temperature_at(x+dirx[dir], y+diry[dir])));
	    }
	}
	for_all_interior_cells(x, y) {
	    set_temperature_at(x, y, (tmp2_at(x, y) / (NUMDIRS + 1)));
	}
    }
    /* See if any displays should change and report if so. */
    for_all_sides(side) {
	if (side_has_display(side)) {
            for_all_cells(x, y) {
	        if (temperature_at(x, y) != tmp1_at(x, y) &&
	            (g_see_all() || terrain_view(side, x, y) != UNSEEN)) {
		    update_cell_display(side, x, y, 34);
		}
	    }
	    flush_display_buffers(side);
	}
    }
    /* Affect any temperature-sensitive units. */
    for_all_units(unit) {
	if (in_play(unit) &&
	    !(unit->transport != NULL &&
	      uu_temp_protect(unit->transport->type, unit->type))) {
	    u = unit->type;
	    temp = temperature_at(unit->x, unit->y);
#if 0
	    if (between(u_survival_min(u), temp, u_survival_max(u))) {
		if (temp < (comfmin = u_comfort_min(u))) {
		    range = comfmin - u_survival_min(u);
		    if (probability((100 * (comfmin - temp)) / range)) {
			damage_unit_with_temperature(unit, 1);
		    }
		} else if (temp > (comfmax = u_comfort_max(u))) {
		    range = u_survival_max(u) - comfmax;
		    if (probability((100 * (temp - comfmax)) / range)) {
			damage_unit_with_temperature(unit, 1);
		    }
		} else {
		    /* Unit gets off scot-free - this time... */
		}
	    } else {
		damage_unit_with_temperature(unit, unit->hp);
	    }
#endif
	}
    }
    /* Do wind changes. */
    if (any_wind_variation_in_layer) {
	for_all_interior_cells(x, y) {
	    set_tmp1_at(x, y, raw_wind_at(x, y));
	}
	for_all_interior_cells(x, y) {
	    int winddir = wind_dir_at(x, y);
	    int windforce = wind_force_at(x, y);
	    int t = terrain_at(x, y);
	    int anychange;

	    anychange = FALSE;
	    if (probability(t_wind_variability(t))) {
		winddir = (flip_coin() ? right_dir(winddir) : left_dir(winddir));
		anychange = TRUE;
	    }
	    if (probability(t_wind_force_variability(t))) {	
		windforce += (flip_coin() ? 1 : -1);
		windforce = max(windforce, t_wind_force_min(t));
		windforce = min(windforce, t_wind_force_max(t));
		anychange = TRUE;
	    }
	    if (anychange)
	      set_wind_at(x, y, winddir, windforce);
	}
	/* See if any displays should change and report if so. */
	for_all_sides(side) {
	    if (side_has_display(side)) {
		for_all_cells(x, y) {
		    if (raw_wind_at(x, y) != tmp1_at(x, y)
			&& (g_see_all() || terrain_view(side, x, y) != UNSEEN)) {
			update_cell_display(side, x, y, 35);
		    }
		}
		flush_display_buffers(side);
	    }
	}
    }
}

static void
damage_unit_with_temperature(unit, n)
Unit *unit;
int n;
{
    if (n >= unit->hp) {
	Dprintf("%s dies from excessive temps\n", unit_desig(unit));
	kill_unit(unit, H_UNIT_KILLED);
    } else {
	Dprintf("%s damaged by excessive temps\n", unit_desig(unit));
	unit->hp -= n;
    }
}

static void
mix_winds()
{
}

/* Given that the spying unit is going to get info about other units at this
   location, figure out just what it is going to see. */

static void
spy_on_location(x, y)
int x, y;
{
    int qual;
    Unit *unit2, *occ;

    for_all_stack(x, y, unit2) {
    	if (unit2->side != tmpunit->side) {
    	    qual = uu_spy_quality(tmpunit->type, unit2->type);
    	    if (probability(qual)) {
    	    	/* Spy got something, report it. */
    	    	/* (should be more worked-out, dunno exactly how) */
    	    	see_exact(tmpunit->side, x, y);
    	    	for_all_occupants(unit2, occ) {
    	    	    /* (should get info about occupants) */
    	    	}
    	    }
    	}
    }
}

/* Certain kinds of units can do spying for the side they're on. */
/* (should reindent) */
static void
run_spies()
{
    int chance;
    Unit *unit;

    if (any_spying < 0) {
	int u;

	any_spying = FALSE;
	for_all_unit_types(u) {
	    if (u_spy_chance(u) > 0) {
		any_spying = TRUE;
		break;
	    }
	}
	if (g_see_all())
	  any_spying = FALSE;  /* override */
    }
    if (!any_spying)
      return;
    Dprintf("Running spies\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    chance = u_spy_chance(unit->type);
	    if (chance > 0) {
		    if (xrandom(10000) < chance) {
			/* Spying is successful, decide how much was seen. */
			tmpunit = unit;
			apply_to_area(unit->x, unit->y, u_spy_range(unit->type),
				      spy_on_location);
		    }
	    }
	}
    }
}

/* Figure out ahead of time which random event methods to run. */

void
init_random_events()
{
    int i, k;
    Obj *randomeventlist, *rest, *evttype;

    numrandomevents = 0;
    randomeventlist = g_random_events();
    if (randomeventlist == NULL || randomeventlist == lispnil)
      return;
    for (rest = randomeventlist; rest != lispnil; rest = cdr(rest)) {
	evttype = car(rest);
	if (symbolp(evttype)) {
	    k = keyword_code(c_string(evttype));
	    for (i = 0; randomeventmethods[i].key != 0; ++i) {
		if (k == randomeventmethods[i].key
		    && randomeventmethods[i].fn != NULL
		    && (randomeventmethods[i].initfn == NULL
			|| (*(randomeventmethods[i].initfn))())) {
		    randomeventindices[numrandomevents++] = i;
		}
	    }
	}
    }
}

/* Run the current cache of random event methods. */

static void
run_random_events()
{
    int i;

    if (numrandomevents > 0) {
	Dprintf("Running random events\n");
	for (i = 0; i < numrandomevents; ++i) {
	    (*(randomeventmethods[randomeventindices[i]].fn))();
	}
    }
}

/* Test whether accidents can happen in this game. */

static int
init_accidents()
{
    int u, t;
    
    for_all_unit_types(u) {
    	for_all_terrain_types(t) {
    	    if (ut_accident_vanish(u, t) > 0
		|| ut_accident_hit(u, t) > 0)
	      return TRUE;
    	}
    }
    return FALSE;
}

/* Test each unit that is out in the open to see if a terrain-related
   accident happens to it.  Accidents can either kill the unit instantly or
   just damage it. */

static void
run_accidents()
{
    int t;
    Unit *unit;

    for_all_units(unit) {
	if (in_play(unit) && unit->transport == NULL) {
	    t = terrain_at(unit->x, unit->y);
	    if (xrandom(10000) < ut_accident_vanish(unit->type, t)) {
	    	/* Kill the unit outright. */
		kill_unit(unit, H_UNIT_VANISHED);
		/* should make a hevt */
	    } else if (xrandom(10000) < ut_accident_hit(unit->type, t)) {
		/* Damage the unit. */
		unit->hp2 -= ut_accident_damage(unit->type, t);
		/* (should be able to pass reason to damage_unit) */
		damage_unit(unit);
	    }
	}
    }
}

/* Test whether attrition can ever happen in this game. */

static int
init_attrition()
{
    int u, t;
    
    for_all_unit_types(u) {
    	for_all_terrain_types(t) {
    	    if (ut_attrition(u, t) > 0)
	      return TRUE;
    	}
    }
    return FALSE;
}

/* Attrition only takes out a few hp at a time, but can be deadly... */

static void
run_attrition()
{
    int u, dmg;
    Unit *unit;

    for_all_units(unit) {
	if (in_play(unit)) {
	    u = unit->type;
	    dmg = prob_fraction(ut_attrition(u, terrain_at(unit->x, unit->y)));
	    /* This is like hit_unit but doesn't have other effects. */
	    unit->hp2 -= dmg;
	    /* (should be able to pass reason to damage_unit) */
	    damage_unit(unit);
	}
    }
}

/* Test whether revolts can ever happen in this game. */

static int
init_revolts()
{
    int u;

    for_all_unit_types(u) {
	if (u_revolt(u) > 0)
	  return TRUE;
    }
    return FALSE;
}

/* Check each unit to see whether it revolts spontaneously.  While
   surrender is influenced by nearby units, revolt takes only the
   overall state of the world into account. */

static void
run_revolts()
{
    Unit *unit;

    for_all_units(unit) {
	if (in_play(unit) && u_revolt(unit->type) > 0) {
	    unit_revolt(unit);
	}
    }
}

static void
unit_revolt(unit)
Unit *unit;
{
    int u = unit->type, ux = unit->x, uy = unit->y, chance, count;
    Side *oldside = unit->side, *newside;

    chance = u_revolt(u);
    if (0 /* affected by politics */) {
    }
    if (xrandom(10000) < chance) {
	if (0 /* leanings towards various sides */) {
	    /* (should want to change to best-liked side) */
	} else {
	    count = 0;
	    while (count++ < 200) {
		newside = side_n(xrandom(numsides + 1));
		if (unit_allowed_on_side(unit, newside)
		    && newside != oldside) {
		    break;
		}
	    }
	}
	/* Might not have been much of a revolt. :-) */
	if (newside == oldside)
	  return;
	unit_changes_side(unit, newside, H_UNIT_ACQUIRED, 0 /* H_UNIT_REVOLTED */);
	see_exact(oldside, ux, uy);
	update_cell_display(oldside, ux, uy, TRUE);
	all_see_cell(ux, uy);
    }
}

/* Test whether surrenders can happen in this game. */

int *surrender_ranges;

static int
init_surrenders()
{
    int u1, u2, u3, range, rslt = FALSE;
    
    for_all_unit_types(u1) {
    	for_all_unit_types(u2) {
    	    if (uu_surrender_chance(u1, u2) > 0) {
		rslt = TRUE;
		if (surrender_ranges == NULL)
		  surrender_ranges = (int *) xmalloc(MAXUTYPES * sizeof(int));
		for_all_unit_types(u3) surrender_ranges[u3] = -1;
		range = uu_surrender_range(u1, u2);
		surrender_ranges[u1] = max(range, surrender_ranges[u1]);
    	    }
    	}
    }
    return rslt;
}

static void
run_surrenders()
{
    Unit *unit;

    for_all_units(unit) {
	if (in_play(unit)) {
	    unit_surrender(unit);
	}
    }
}

/* Units may surrender to enemy units that are visible nearby.
   Independents have to be treated specially, since they don't have a view
   to work from.  We sort of compute the view "on the fly". */

static void
unit_surrender(unit)
Unit *unit;
{
    int u = unit->type, dir, x1, y1, range, surrounded = TRUE;
    Unit *unit2;

    range = surrender_ranges[u];
    if (range < 0) {
	/* This unit won't surrender, nothing to do. */
    } else if (range > 1) {
	/* (should write general case) */
    } else {
	/* Range is 0 or 1; check other units in this cell. */
	for_all_stack(unit->x, unit->y, unit2) {
	    if (in_play(unit2)
		&& unit2->side != unit->side
		&& uu_surrender_chance(u, unit2->type) > 0
		&& visible_to(unit, unit2)) {
		maybe_surrender_to(unit, unit2);
	    }
	}
	/* Check on adjacent units. */
        if (range == 1) {
	  for_all_directions(dir) {
	    if (interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
		for_all_stack(unit->x, unit->y, unit2) {
		    if (in_play(unit2)
			&& unit2->side != unit->side
			&& uu_surrender_chance(u, unit2->type) > 0
			&& visible_to(unit, unit2)) {
			maybe_surrender_to(unit, unit2);
		    }
		}
	    }
	  }
	}
    }
}

/* Calculate whether one unit is visible to another, even if the other is independent. */

int
visible_to(unit, unit2)
Unit *unit, *unit2;
{
    int uview;

    if (g_see_all()) {
	return TRUE;
    } else if (unit->side != NULL) {
    	uview = unit_view(unit->side, unit2->x, unit2->y);
	return (uview != EMPTY && vtype(uview) == unit2->type);
    } else {
	/* (should be more careful to check see-chances) */
    	if (distance(unit->x, unit->y, unit2->x, unit2->y) <= u_vision_range(unit->type))
    	  return TRUE;
    	else
    	  return FALSE;
    }
}

static void
maybe_surrender_to(unit, unit2)
Unit *unit, *unit2;
{
    int chance;

    chance = uu_surrender_chance(unit->type, unit2->type);
    if (xrandom(10000) < chance) {
	/* (not really correct) */
	set_unit_side(unit, unit2->side);
    }
}

int tmpexcess;

/* We've "found what we were searching for" when the excess to distribute
   is gone. */

static int
excess_left(x, y)
int x, y;
{
    return (tmpexcess > 0);
}

static void
try_transfer_to_cell(x, y)
int x, y;
{
    Unit *unit2, *occ;

    if (tmpexcess <= 0)
      return;
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {
	    try_transfer(tmpunit, unit2, tmpmtype);
	}
    }
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {
	    for_all_occupants(unit2, occ) {
		if (in_play(occ) && occ->side == tmpunit->side) {
		    try_transfer(tmpunit, occ, tmpmtype);
		}
	    }
	}
    }
}

static int
sharable_left(x, y)
int x, y;
{
    return tmpunit->supply[tmpmtype] > (um_storage_x(tmpunit->type, tmpmtype) / 2);
}

static void
try_sharing_with_cell(x, y)
int x, y;
{
    Unit *unit2, *occ;

    if (!sharable_left(x, y))
      return;
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {
	    try_sharing(tmpunit, unit2, tmpmtype);
	}
    }
    for_all_stack(x, y, unit2) {
	if (in_play(unit2) && unit2->side == tmpunit->side) {
	    for_all_occupants(unit2, occ) {
		if (in_play(occ) && occ->side == tmpunit->side) {
		    try_sharing(tmpunit, occ, tmpmtype);
		}
	    }
	}
    }
}

/* The main routine does production, distribution, and discarding in order. */

static void
run_economy()
{
    int u, m, t, amt, dist, x, y, x1, y1, m1, m2;
    int prod, ptivity, stor, oldamt, newamt;
    int ttotals[MAXMTYPES], utotals[MAXMTYPES];
    Unit *unit;
    
    if (nummtypes == 0)
      return;
    if (any_unit_production < 0) {
	any_unit_production = FALSE;
	for_all_unit_types(u) {
	    for_all_material_types(m) {
		if (max(um_base_production(u, m),
			um_occ_production(u, m)) > 0) {
		    any_unit_production = TRUE;
		    break;
		}
	    }
	}
    }
    if (any_terrain_production < 0) {
	any_terrain_production = FALSE;
	for_all_terrain_types(t) {
	    for_all_material_types(m) {
		if (tm_production(t, m) > 0) {
		    any_terrain_production = TRUE;
		    break;
		}
	    }
	}
    }
    if (any_people_production < 0) {
	any_people_production = FALSE;
	for_all_material_types(m1) {
	    if (m_people(m1) > 0) {
		for_all_material_types(m2) {
		    if (mm_people_production(m1, m2) > 0) {
			any_people_production = TRUE;
			break;
		    }
		}
	    }
	}
    }
    if (!any_unit_production
	&& !any_terrain_production
	&& !any_people_production)
      return;
    /* (should find other reasons not to run this) */
    Dprintf("Running economy\n");
    for_all_material_types(m)
      ttotals[m] = utotals[m] = 0;
    /* Make new materials but don't clip to storage capacities yet. */
    if ((any_terrain_production || any_people_production)
	&& any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		for_all_cells(x, y) {
		    if (any_terrain_production) {
			t = terrain_at(x, y);
			prod = tm_production(t, m);
			if (prod > 0) {
			    oldamt = material_at(x, y, m);
			    newamt = oldamt + prod;
			    set_material_at(x, y, m, newamt);
			    if (Debug) {
				stor = tm_storage_x(t, m);
				if (newamt > stor)
				  ttotals[m] += (newamt - stor);
			    }
			}
		    }
		    if (any_people_production) {
			for_all_material_types(m1) {
			    if (m_people(m1) > 0) {
				prod = mm_people_production(m1, m);
				if (prod > 0) {
				    oldamt = material_at(x, y, m);
				    newamt = oldamt + prod;
				    set_material_at(x, y, m, newamt);
				    if (Debug) {
					stor = tm_storage_x(t, m);
					if (newamt > stor)
					  ttotals[m] += (newamt - stor);
				    }
				}
			    }
			}
		    }
		}
	    }
	}
    }
    if (any_unit_production) {
	for_all_units(unit) {
	    if (in_play(unit) && completed(unit)) {
		u = unit->type;
		for_all_material_types(m) {
		    t = terrain_at(unit->x, unit->y);
		    prod = base_production(unit, m);
		    if (prod > 0) {
			ptivity = ut_productivity(u, t);
			/* Note that we've switched to hundredths. */
			ptivity = max(ptivity, um_productivity_min(u, m));
			ptivity = min(ptivity, um_productivity_max(u, m));
			amt = prob_fraction(prod * ptivity);
			unit->supply[m] += amt;
			if (Debug && unit->supply[m] > um_storage_x(u, m))
			  utotals[m] += (unit->supply[m] - um_storage_x(u, m));
		    }
		}
	    }
	}
    }
    Dprintf("Overflow is:");
    Dprintf("  (for terrain)");
    for_all_material_types(m) Dprintf(" %d", ttotals[m]);
    Dprintf("  (for units)");
    for_all_material_types(m) Dprintf(" %d", utotals[m]);
    Dprintf("\n");
    /* Move stuff around - try to get rid of any excess. */
    /* (should also do cell-cell, cell-unit, unit-cell xfers) */
    for_all_units(unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		stor = um_storage_x(u, m);
		if (unit->supply[m] > stor) {
		    dist = um_outlength(u, m);
		    if (dist >= 0) {
		    	tmpunit = unit;
		    	tmpmtype = m;
		    	tmpexcess = unit->supply[m] - stor;
			search_and_apply(unit->x, unit->y, dist, excess_left,
					 &x1, &y1, 1,
					 try_transfer_to_cell, 999999);
		    }
		}
	    }
	}
    }
    /* (should) Limit overall people density. */
    if (0 /* any people limits */) {
    	int numpeople;
	
	for_all_cells(x, y) {
	    numpeople = 0;
	    for_all_material_types(m) {
		if (0 /* repns people */) {
		    numpeople += m_people(m) * material_at(x, y, m);
		    /* (should save in scratch layer) */
		}
	    }
	}
	/* (should have excess people migrate to cells with room) */
	/* (eliminate any leftover) */
    }
    /* Throw away excess that can't be stored anywhere. */
    for_all_material_types(m)
      ttotals[m] = utotals[m] = 0;
    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (cell_material_defined(m)) {
		for_all_cells(x, y) {
		    t = terrain_at(x, y);
		    stor = tm_storage_x(t, m);
		    oldamt = material_at(x, y, m);
		    newamt = min(oldamt, stor);
		    set_material_at(x, y, m, newamt);
		    if (Debug && newamt < oldamt)
		      ttotals[m] += (oldamt - newamt);
		}
	    }
	}
    }
    for_all_units(unit) {
	u = unit->type;
	for_all_material_types(m) {
	    stor = um_storage_x(u, m);
	    oldamt = unit->supply[m];
	    newamt = min(oldamt, stor);
	    unit->supply[m] = newamt;
	    if (Debug && newamt < oldamt)
	      utotals[m] += (oldamt - newamt);
	}
    }
    Dprintf("Discarded ");
    Dprintf("  (for terrain)");
    for_all_material_types(m) Dprintf(" %d", ttotals[m]);
    Dprintf("  (for units)");
    for_all_material_types(m) Dprintf(" %d", utotals[m]);
    Dprintf("\n");
    /* This next phase is for sharing of scarcer supplies. */
    for_all_units(unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		dist = um_outlength(u, m);
		if (dist >= 0) {
		    tmpunit = unit;
		    tmpmtype = m;
		    search_and_apply(unit->x, unit->y, dist, sharable_left,
				     &x1, &y1, 1,
				     try_sharing_with_cell, 999999);
		}
	    }
	}
    }
    /* Finally, reset supply alarms. */
    for_all_units(unit) {
	if (in_play(unit) && unit->plan != NULL) {
	    /* (should probably be a subr) */
	    if (unit->plan->supply_is_low
		&& !past_halfway_point(unit)) {
		unit->plan->supply_alarm = TRUE;
		unit->plan->supply_is_low = FALSE;
    		update_unit_display(unit->side, unit, TRUE); 
	    }
	}
    }
}

static int
base_production(unit, m)
Unit *unit;
int m;
{
    int u = unit->type, occprod;

    if (unit->transport) {
	occprod = um_occ_production(u, m);
	return (occprod >= 0 ? occprod : um_base_production(u, m));
    } else {
	return um_base_production(u, m);
    }
}

/* Give away supplies, but save enough to stay alive for a couple turns. */

static void
try_transfer(from, to, m)
Unit *from, *to;
int m;
{
    int u = from->type;
    int oldsupply = from->supply[m];

    try_transfer_aux(from, to, m);
    tmpexcess -= (oldsupply - from->supply[m]);
}

static void
try_sharing(from, to, m)
Unit *from, *to;
int m;
{
    int u = from->type;
    int oldsupply = from->supply[m];
  
    try_transfer_aux(from, to, m);
}

/* Material redistribution uses this routine to move supplies around
   between units far apart or on the same cell. Try to do reasonable
   things with the materials.  Net producers are much more willing to
   give away supplies than net consumers. */

static void
try_transfer_aux(from, to, m)
Unit *from, *to;
int m;
{
    int nd, u = from->type, u2 = to->type, fromrate, torate;
    Unit *occ;

    if (from != to &&
	um_inlength(u2, m) >= distance(from->x, from->y, to->x, to->y)) {
      if (completed(to)) {
	/* Try for the transfer only if we're below capacity. */
	if ((nd = um_storage_x(u2, m) - to->supply[m]) > 0) {
	    if ((um_base_production(u, m) > um_base_consumption(u, m))
		|| (survival_time(to) < 3)
		|| (um_storage_x(u, m) * 4 >= um_storage_x(u2, m))) {
		if (can_satisfy_need(from, m, nd)) {
		    transfer_supply(from, to, m, nd);
		} else if (can_satisfy_need(from, m, max(1, nd/2))) {
		    transfer_supply(from, to, m, max(1, nd/2));
		} else if (from->supply[m] > um_storage_x(u, m)) {
		    transfer_supply(from, to, m,
				    (from->supply[m] - um_storage_x(u, m)));
		}
	    } else {
		fromrate = u_speed(u) * um_consumption_per_move(u, m) * 3;
		fromrate = max(1, fromrate);
		torate = u_speed(u2) * um_consumption_per_move(u2, m) * 3;
		torate = max(1, torate);
		if ((from->supply[m] / fromrate) > (to->supply[m] / torate)) {
		    transfer_supply(from, to, m,
				    min(nd, (8 + from->supply[m]) / 9));
		} 
	    }
	}
      } else {
	/* Incomplete units don't need supply, but they are a handy overflow
	   mepository. */
	if (from->supply[m] > um_storage_x(u, m)
	    && to->supply[m] < um_storage_x(u2, m)) {
	    /* Calculate the limit on how much we can transfer usefully. */
	    nd = min(um_storage_x(u2, m) - to->supply[m],
		     from->supply[m] - um_storage_x(u, m));
	    transfer_supply(from, to, m, nd);
	}
      }
    }
}

/* This estimates if a need can be met.  Note that total transfer of requested */
/* amount is not a good idea, since the supplies might be essential to the */
/* unit that has them first.  If we're more than half full, or the request */
/* is less than 1/5 of our supply, then we can spare it. */

/* (should replace with doctrine/plan controls, check underlying terrain) */

static int
can_satisfy_need(unit, m, need)
Unit *unit;
int m, need;
{
    int supp = unit->supply[m];
    int stor = um_storage_x(unit->type, m);

    return (((2 * supp > stor) && (((supp * 4) / 5) > need)) ||
	    (need < stor / 5));
}

/* Do everything associated with the end of a turn. */

static void
run_turn_end()
{
    finish_movement();
    run_people_consumption();
    run_cell_consumption();
    run_unit_base_consumption();
    run_people_side_changes();
    /* This should come after other people-related computations,
       since this only constrains generic overcrowding. */
    run_people_limits();
    flush_dead_units();
    check_post_turn_scores();
    test_agreements();
    run_disappearances();
    if (Debug) report_malloc();
}

/* Handle consumption by people. */

static void
run_people_consumption()
{
    int x, y, m1, m2, t, consum, oldamt, newamt, newtype;

    /* Precompute whether any people consumption ever happens. */
    if (any_people_consumption < 0) {
	int mm1, mm2;

	any_people_consumption = FALSE;
	for_all_material_types(mm1) {
	    for_all_material_types(mm2) {
	    	if (mm_people_consumption(mm1, mm2) > 0) {
		    any_people_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_people_consumption)
	      break;
	}
	Dprintf("Have consumption by people.\n");
    }
    if (!any_people_consumption)
      return;
    Dprintf("Running people consumption\n");
    if (any_cell_materials_defined()) {
	for_all_material_types(m1) {
	    if (cell_material_defined(m1)) {
		for_all_material_types(m2) {
		    if (cell_material_defined(m2)) {
			consum = mm_people_consumption(m1, m2);
			if (consum > 0) {
			    for_all_cells(x, y) {
				oldamt = material_at(x, y, m2);
				newamt = oldamt - consum;
				if (newamt < 0) {
				    newamt = 0;
				    /* Check for exhaustion. */
				    /* (should share with cell consumption) */
				    t = terrain_at(x, y);
				    if (probability(tm_change_on_exhaust(t, m2)) &&
					tm_exhaust_type(t, m2) != NONTTYPE) {
					newtype = tm_exhaust_type(t, m2);
					/* Change the terrain's type. */
					set_terrain_at(x, y, newtype);
					/* (should) update sides, make units vanish, etc */
				    }
				}
				set_material_at(x, y, m2, newamt);
			    }
			}
		    }
		}
	    }
	}
    }
}

/* Handle consumption by terrain. */

static void
run_cell_consumption()
{
    int x, y, t, m, consum, oldamt, newamt, willchange, newtype;

    /* Precompute whether any cell base consumption ever happens. */
    if (any_cell_consumption < 0) {
	int t2, m2;

	any_cell_consumption = FALSE;
	for_all_terrain_types(t2) {
	    for_all_material_types(m2) {
	    	if (tm_consumption(t2, m2) > 0) {
		    any_cell_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_cell_consumption)
	      break;
	}
	Dprintf("Have consumption by cells.\n");
    }
    if (!any_cell_consumption)
      return;
    Dprintf("Running cell consumption\n");
    if (any_cell_materials_defined()) {
	for_all_cells(x, y) {
	    t = terrain_at(x, y);
	    willchange = FALSE;
	    for_all_material_types(m) {
		if (cell_material_defined(m)) {
		    consum = tm_consumption(t, m);
		    oldamt = material_at(x, y, m);
		    newamt = oldamt - consum;
		    if (newamt < 0) {
			newamt = 0;
			/* Check for exhaustion. */
			if (!willchange &&
			    probability(tm_change_on_exhaust(t, m)) &&
			    tm_exhaust_type(t, m) != NONTTYPE) {
			    willchange = TRUE;
			    newtype = tm_exhaust_type(t, m);
			}
		    }
		    set_material_at(x, y, m, newamt);
		}
	    }
	    if (willchange) {
	    	/* Change the terrain's type. */
		set_terrain_at(x, y, newtype);
		/* (should) update sides, make units vanish, etc */
	    }
	}
    }
}

/* Handle base consumption by units. */

static void
run_unit_base_consumption()
{
    Unit *unit;

    /* Precompute whether any base consumption ever happens. */
    if (any_unit_base_consumption < 0) {
	int u, m;

	any_unit_base_consumption = FALSE;
	for_all_unit_types(u) {
	    for_all_material_types(m) {
	    	if (um_base_consumption(u, m) > 0) {
		    any_unit_base_consumption = TRUE;
		    break;
	    	}
	    }
	    if (any_unit_base_consumption)
	      break;
	}
	Dprintf("Have consumption by units.\n");
    }
    if (!any_unit_base_consumption)
      return;
    Dprintf("Running unit consumption\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    unit_consumes(unit);
	}
    }
}

/* Consume the constant overhead part of supply consumption. */
/* Usage by movement is subtracted from overhead first. */

static void
unit_consumes(unit)
Unit *unit;
{
    int u = unit->type, m, usedup, consump, checkstarve = FALSE;

    if (alive(unit)) {    
      for_all_material_types(m) {
	if (um_base_consumption(u, m) > 0 &&
	    !(unit->transport != NULL && um_consumption_as_occupant(u, m) == 0)) {
	    /* Calculate what was already consumed by movement. */
	    usedup = 0;
	    if (unit->act != NULL)
	      usedup = unit->act->actualmoves * um_consumption_per_move(u, m);
	    consump = um_base_consumption(u, m);
	    /* If being transported, modify the base consumption. */
	    if (unit->transport != NULL)
	      consump = (consump * um_consumption_as_occupant(u, m)) / 100;
	    /* Subtract consumption that was not already used up in movement. */
	    if (usedup < consump)
	      unit->supply[m] -= (consump - usedup);
	    /* Don't let supply go below zero. */
	    if (unit->supply[m] <= 0) {
		unit->supply[m] = 0;
		checkstarve = TRUE;
	    }
	}
      }
    }
    if (checkstarve)
      maybe_starve(unit, TRUE);
    if (alive(unit)
    	&& unit->plan
    	&& !unit->plan->supply_is_low
    	&& past_halfway_point(unit)
    	) {
    	unit->plan->supply_is_low = TRUE;
    	update_unit_display(unit->side, unit, TRUE); 
    }
}

/* What happens to a unit that runs out of supplies.  If it can survive
   on nothing, then there may be a few turns of grace, depending on
   how the dice roll... */

void
maybe_starve(unit, partial)
Unit *unit;
int partial;
{
    int u = unit->type, m, starv, oneloss, hploss = 0;

    for_all_material_types(m) {
      if (unit->supply[m] <= 0 && !in_supply(unit, m)) {
	starv = um_hp_per_starve(u, m);
	if (starv > 0) {
	    oneloss = prob_fraction(starv);
	    hploss = max(hploss, oneloss);
	}
      }
    }
    if (hploss > 0) {
      if (hploss >= unit->hp) {
	/* (should let occupants try to escape first) */
	kill_unit(unit, H_UNIT_STARVED);
      } else if (partial) {
    	unit->hp -= hploss;
    	/* (should do other hp loss consequences) */
	/* (use generic damage routine?) */
      }
    }
}

/* Check if the unit has ready access to a source of supplies. */

/* (should be more sophisticated and account for supply lines etc) */

static int
in_supply(unit, m)
Unit *unit;
int m;
{
    if (unit->transport != NULL) {
    	if (unit->transport->supply[m] > 0)
	  return TRUE;
    }
    return FALSE;
}

/* Some types of units can become completed and grow to full size
   automatically when they get to a certain point. */

static void
run_self_builds()
{
    int u, cpper;
    Unit *unit;

    if (any_self_builds < 0) {
	any_self_builds = FALSE;
	for_all_unit_types(u) {
	    if (u_cp_per_self_build(u) > 0) {
		any_self_builds = TRUE;
		break;
	    }
	}
    }
    if (!any_self_builds)
      return;
    Dprintf("Running self builds\n");
    for_all_units(unit) {
	u = unit->type;
	if (in_play(unit)
	    && !fullsized(unit)
	    && (cpper = u_cp_per_self_build(u)) > 0
	    && unit->cp >= u_cp_to_self_build(u)) {
	    unit->cp += cpper;
	    if (unit->cp > u_cp(u)) unit->cp = u_cp(u);
	    if (completed(unit)) {
		make_unit_complete(unit);
	    } else {
	    	/* Let the player know that progress was made. */
		update_unit_display(unit->side, unit, TRUE);
	    }
	}
    }
}

static void
run_people_side_changes()
{
    int x, y, u, t;
    Unit *unit;

    if (!people_sides_defined())
      return;
    if (any_people_side_changes < 0) {
	any_people_surrenders = (int *) xmalloc(numutypes * sizeof(int));
	any_people_side_changes = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_people_surrender(u, t) > 0) {
		    any_people_side_changes = TRUE;
		    any_people_surrenders[u] = TRUE;
		    break;
		}
	    }
	}
    }
    if (!any_people_side_changes)
      return;
    for_all_cells(x, y) {
	if (unit_at(x, y) != NULL) {
	    for_all_stack(x, y, unit) {
		/* The people here may change sides. */
	        u = unit->type;
		if (any_people_surrenders[u]
		    && probability(people_surrender_chance(u, x, y))) {
		    change_people_side_around(x, y, u, unit->side);
		}
	    }
	} else {
	    /* Unoccupied cells might see population revert. */
	    /* (this would need multiple-loyalty pops) */
	}
    }
}

int
people_surrender_chance(u, x, y)
int u, x, y;
{
    int m, chance, peop;

    chance = ut_people_surrender(u, terrain_at(x, y));
    /* Modify the basic chance according to people types, if present. */
    if (any_cell_materials_defined()) {
	for_all_material_types(m) {
	    if (m_people(m) > 0
		&& cell_material_defined(m)) {
		peop = material_at(x, y, m);
		if (peop > 0) {
		    chance = (chance * um_people_surrender(u, m)) / 100;
		}
	    }
	}
    }
    return chance;
}

void
change_people_side_around(x, y, u, side)
int x, y, u;
Side *side;
{
    int pop = people_side_at(x, y), s = side_number(side), dir, x1, y1;

    if (pop != NOBODY
        && pop != s
        && !trusted_side(side, side_n(pop))) {
	set_people_side_at(x, y, s);
	update_cell_display_all_sides(x, y, TRUE);
	/* (should decrement coverage of previous owner somewhere?) */
	for_all_directions(dir) {
	    if (interior_point_in_dir(x, y, dir, &x1, &y1)) {
		update_cell_display_all_sides(x1, y1, TRUE);
	    }
	}
    }
    /* (should be able to do adjacent cells also) */
}

/* See if the numbers of individuals in a cell exceeds the max, and migrate or
   remove so as to bring the numbers back in line. */

static void
run_people_limits()
{
    int m, t, x, y, num, ratio, amt, newamt;
    
    if (any_people_max < 0) {
	any_people_max = FALSE;
	for_all_terrain_types(t) {
	    if (t_people_max(t) >= 0) {
		any_people_max = TRUE;
		break;
	    }
	}
    }
    if (!any_people_max)
      return;
    if (!any_cell_materials_defined())
      return;
    for_all_cells(x, y) {
	t = terrain_at(x, y);
	if (t_people_max(t) >= 0) {
	    num = num_people_at(x, y);
	    if (num > t_people_max(t)) {
		/* Too many people here, (should) trim them down. */
		/* Compute the ratio of limit to actual number.
		   (Note that actual number is guaranteed to be nonzero.) */
		ratio = (t_people_max(t) * 100) / num;
		for_all_material_types(m) {
		    if (m_people(m) > 0
			&& cell_material_defined(m)) {
			amt = material_at(x, y, m);
			if (amt > 0) {
			    newamt = (amt * ratio) / 100;
			    set_material_at(x, y, m, newamt);
			}
		    }
		}
	    }
	}
    }
}

/* (generic routine) */

int
num_people_at(x, y)
int x, y;
{
    int m, num;

    num = 0;
    for_all_material_types(m) {
	if (cell_material_defined(m)) {
	    num += material_at(x, y, m) * m_people(m);
	}
    }
    return num;
}

static void
update_cell_display_all_sides(x, y, rightnow)
int x, y, rightnow;
{
    Side *side;

    for_all_sides(side) {
	if (side->ingame && (g_see_all() || cover(side, x, y) > 0)) {
	    update_cell_display(side, x, y, rightnow);
	}
    }
}

/* See if it's time for any scheduled arrivals to appear. */

static void
run_appearances()
{
    int curturn, nx, ny, nw, nh, nx1, ny1;
    Obj *appear;
    Unit *unit, *transport;

    if (any_appearances < 0) {
    	any_appearances = FALSE;
	for_all_units(unit) {
	    if (unit->cp < 0 && unit->hook != lispnil) {
		appear = get_x_property(unit, K_APPEAR);
		if (appear != lispnil) {
		    any_appearances = TRUE;
		    break;
		}
	    }
	}
    }
    if (!any_appearances)
      return;
    Dprintf("Running appearances\n");
    curturn = g_turn();
    for_all_units(unit) {
    	/* See if now time for a unit to appear. */
    	if (unit->cp < 0 && unit->hook != lispnil) {
	    appear = get_x_property(unit, K_APPEAR);
	    if (appear != lispnil && c_number(car(appear)) <= curturn) {
		/* Set the unit to its correct cp. */
		unit->cp = (- unit->cp);
		/* Get the base location at which it will appear. */
		nx = (- unit->prevx);  ny = (- unit->prevy);
		if (cdr(appear) != lispnil && numberp(cadr(appear))) {
		    /* Appear at a random location around nx,ny. */
		    nw = c_number(cadr(appear));
		    if (cddr(appear) != lispnil && numberp(caddr(appear))) {
			nh = c_number(caddr(appear));
		    } else {
			nh = nw;
		    }
		    if (random_point_in_area(nx, ny, nw, nh, &nx1, &ny1)) {
			nx = nx1;  ny = ny1;
		    }
		}
		/* Do the usual steps to place the unit. */
		/* (should be able to retry with diff loc if nw or nh > 0) */
		if (inside_area(nx, ny)) {
		    if (can_occupy_cell(unit, nx, ny)) {
			enter_cell(unit, nx, ny);
		    } else {
			/* Search this cell for units to enter. */
			for_all_stack(nx, ny, transport) {
			    if (unit->side == transport->side
				&& can_occupy(unit, transport)) {
				enter_transport(unit, transport);
				break;
			    }
			}
			/* We got a problem, make the unit wait for next turn;
			   will just try again. */
			unit->cp = (- unit->cp);
		    }
		} else {
		    /* loc of reinforcement is messed up */
		}
		init_unit_actorstate(unit);
		init_unit_plan(unit);
	    }
    	}
    }
}

static void
run_disappearances()
{
    int curturn;
    Obj *disappear;
    Unit *unit;

    if (any_disappearances < 0) {
    	any_disappearances = FALSE;
	for_all_units(unit) {
	    if (unit->hook != lispnil) {
		disappear = get_x_property(unit, K_DISAPPEAR);
		if (disappear != lispnil) {
		    any_disappearances = TRUE;
		    break;
		}
	    }
	}
    }
    if (!any_disappearances)
      return;
    Dprintf("Running disappearances\n");
    curturn = g_turn();
    for_all_units(unit) {
    	/* See if now time for a unit to disappear. */
    	if (in_play(unit) && unit->hook != lispnil) {
    	    disappear = get_x_property(unit, K_DISAPPEAR);
    	    if (disappear != lispnil && c_number(car(disappear)) <= curturn) {
	    /* (should eject occupants first if possible) */
		kill_unit(unit, H_UNIT_KILLED);
	    }
	}
    }
}

/* Some types of units recover lost hp spontaneously. */

static void
run_hp_recovery()
{
    int u, hprecovery, hpmax, oldhp;
    Unit *unit;

    if (any_hp_recovery < 0) {
	any_hp_recovery = FALSE;
    	for_all_unit_types(u) {
	    if (u_hp_recovery(u) > 0) {
		any_hp_recovery = TRUE;
		break;
	    }
	}
    }
    if (!any_hp_recovery)
      return;
    Dprintf("Running hp recovery\n");
    for_all_units(unit) {
	if (is_active(unit)) {
	    u = unit->type;
	    hprecovery = u_hp_recovery(u);
	    hpmax = u_hp(u);
	    /* (should only do for one part of multi-part unit?) */
	    if (hprecovery > 0 && unit->hp < hpmax) {
		oldhp = unit->hp;
		unit->hp += prob_fraction(hprecovery);
		if (unit->hp > hpmax) unit->hp = hpmax;
		/* Inform the player if the unit's hp changed. */
		if (unit->hp != oldhp) {
		    update_unit_display(unit->side, unit, TRUE);
		}
	    }
	}
    }
}

static void
run_detonation_accidents()
{
    int u, t, x, y, z, chance;
    Unit *unit;

    if (any_detonation_accidents < 0) {
	any_detonation_accidents = FALSE;
	for_all_unit_types(u) {
	    for_all_terrain_types(t) {
		if (ut_detonation_accident(u, t) > 0) {
		    any_detonation_accidents = TRUE;
		    break;
		}
	    }
	}
    }
    if (!any_detonation_accidents)
      return;
    for_all_units(unit) {
	if (in_play(unit) && completed(unit)) {
	    x = unit->x;  y = unit->y;  z = unit->z;
	    t = terrain_at(x, y);
	    chance = ut_detonation_accident(unit->type, t);
	    if (chance > 0) {
	    	maybe_detonate_accidently(unit);
	    }
	}
    }
}

static void
maybe_detonate_accidently(unit)
Unit *unit;
{
    int x = unit->x, y = unit->y, chance, t;
    extern int maxudetonaterange;

    t = terrain_at(x, y);
    chance = ut_detonation_accident(unit->type, t);
    if (xrandom(10000) < chance) {
	/* Detonate the unit right where it is. */
	detonate_unit(unit, x, y, unit->z);
	reckon_damage_around(x, y, maxudetonaterange);
    }
}

/* Unconditional surrender. */

void
resign_game(side, side2)
Side *side, *side2;
{
    /* Nothing to do if we're not in the game. */
    if (!side->ingame)
      return;
    /* Tell everybody about the resignation. */
    /* (should record an appropriate event, will be textified by event handler) */
    side_loses(side, side2, -1);
}

/* This is true if there is any kind of realtime limit on the game. */

int
realtime_game()
{
    return (g_rt_for_game() > 0
    	    || g_rt_per_side() > 0
    	    || g_rt_per_turn() > 0);
}

/* Pass NULL to see if all sides are now willing to save the game. */

int
all_others_willing_to_save(side)
Side *side;
{
    Side *side2;

    for_all_sides(side2) {
	if (side != side2 && !side2->willingtosave)
	  return FALSE; 
    }
    return TRUE;
}

/* Pass NULL to see if all sides are now willing to declare a draw. */

int
all_others_willing_to_quit(side)
Side *side;
{
    Side *side2;

    for_all_sides(side2) {
	if (side != side2 && !side2->willingtodraw)
	  return FALSE; 
    }
    return TRUE;
}

/* This forces an end to the game directly. */

void
end_the_game()
{
    Side *side;

    Dprintf("The game is over.\n");
    record_event(H_GAME_ENDED, ALLSIDES);
    /* Set the global that indicates the game is over for everybody. */
    endofgame = TRUE;
    for_all_sides(side) {
    	/* (should there be any other effects on sides, like final scoring?) */
    	if (side_has_display(side)) {
    	    update_turn_display(side, TRUE);
    	    update_side_display(side, side, TRUE);
    	}
    }
    end_history();
	dump_statistics();
}
