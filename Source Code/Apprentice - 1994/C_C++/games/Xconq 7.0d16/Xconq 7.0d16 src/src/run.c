/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This is the main simulation-running code. */

#include "conq.h"

/* The number of the current turn. */

int curturn;

/* Sequencing number within the turn. */

int cursequence;

/* This is the main array that tracks units acting during this turn. */

UnitVector *actionvector = NULL;

/* The table of all types of random events. */

int init_accidents();
int init_attrition();
int init_revolts();
int init_surrenders();
int run_accidents();
int run_attrition();
int run_revolts();
int run_surrenders();

struct randomeventtype {
    int key;
    int (*initfn)();
    int (*fn)();
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

int gameinited = TRUE;

/* This is true only before the game actually starts. */

int beforestart = TRUE;

/* This is true only at the beginning of a turn. */

int startturn = FALSE;

/* This is true while units are acting. */

int movestuff = FALSE;

int finishturn = FALSE;

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

long sunx, suny;

/* The time at which the game actually starts. */

time_t realtimegamestart;

/* The point in the turn at which players can actually do things. */

time_t realtimeturnplaystart;

int planexecs;

int taskexecs;

int anyselfbuilds = -1;

int anypeoplesidechanges = -1;

int *anypeoplesurrenders = NULL;

/* This function does a (small, usually) amount of simulation, then returns.
   It can be run multiple times at any time, will not go "too far".
   It returns the max time in seconds that the interface should do
   things before running this again. */

run_game(maxactions)
int maxactions;
{
    int timeout = -1, numacted, othersdone, runtime;
    time_t rungamestart, rungameend;
    Side *side;

    gameinited = TRUE;
    if (maxactions > 0) maxactions = 1;
    if (maxactions < 0) maxactions = 1000000;
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
	/* shouldn't do if game being restored... */
	/* actually, if move allowances were restored. */
	if (startturn) {
	    run_new_turn();
	    /* Game might have been ended by new turn init. */
	    if (endofgame) {
	    	Dprintf("run_game: game ended by new turn init.\n");
	    	return 0;
	    }
	    time(&realtimeturnplaystart);
	    startturn = FALSE;
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
	    startturn = TRUE;
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
#if 0
	    /* Decide on the timeout for the next call of this routine. */
	    timeout = 0;
	    /* Sides that have no-activity-timeouts will adjust the
	       timeout to occur at the end of the latest timeout; sides
	       that have explicitly declared themselves done won't need
	       any additional waits. */
	    for_all_sides(side) {
		if (!side->finishedturn) {
		    timeout = max(timeout,
				  (side->realtimeout
				   - (time(0) - side->lasttime)));
		}
	    }
#endif
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
	    }
	    if (Debug) {
		if (planexecs > 0) {
		    Dprintf(" (%d plan execs)", planexecs);
		}
		if (taskexecs > 0) {
		    Dprintf(" (%d task execs)", taskexecs);
		}
		/* (also number of units considered?) */
	    }
	    Dprintf("\n");
	}
	test_for_game_end();
    }
    time(&rungameend);
    runtime = idifftime(rungameend, rungamestart);
    if (runtime > 0) Dprintf("run_game: took %d seconds\n", runtime);
    return 0 /* timeout */;
}

/* See if game is ready to get underway for real.  Note that displays will
   work before the game has started, but game time doesn't move. */

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
	    startturn = TRUE;
	    record_event(H_GAME_STARTED);
	    /* Record the game as starting NOW in real time. */
	    time(&realtimegamestart);
	    /* No need to look at any more sides, just get on with the game. */
	    return;
	}
    }
    if (!anydisplays) {
	init_warning("No sides have a display!");
    }
}

/* This routine looks to see if the game is completely over. */

test_for_game_end()
{
    Side *side;

    for_all_sides(side) {
    	/* If we have an active side being displayed, we're not done yet. */
	if (side->ingame && side_has_display(side)) return;
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

check_realtime()
{
    Side *side;

    if (!realtime_game()) return;
    if (exceeded_rt_for_game()) end_the_game();
    if (g_rt_per_side() > 0) {
	for_all_sides(side) {
	    if (side->ingame && side->totaltimeused > g_rt_per_side()) {
#if 0
		side_loses(side, NULL); /* (but should just go inactive?) */
		printf("%s ran out of time!\n", side_desig(side));
#endif
	    }
	}
    }
}

exceeded_rt_for_game()
{
    time_t now;

    if (g_rt_for_game() <= 0) return FALSE;
    time(&now);
    return (idifftime(now, realtimegamestart) > g_rt_for_game());
}

exceeded_rt_per_turn()
{
    time_t now;

    if (g_rt_per_turn() <= 0) return FALSE;
    time(&now);
    return (idifftime(now, realtimeturnplaystart) > g_rt_per_turn());
}

/* This returns true if the given side is still wanting to do stuff. */

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

run_new_turn()
{
    time_t turncalcstart, turncalcend;
    Side *side;

    if (!midturnrestore) {
	/* Increment the turn number. */
	++curturn;
	/* See if we've hit the preset end of the game. */
	if (curturn > g_last_turn() && !probability(g_extra_turn())) {
	    end_the_game();
	    /* The game is over, just get out of here. */
	    return;
	}
	time(&turncalcstart);
	cursequence = 0;
	update_all_progress_displays("turn start calcs", -1);
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
	run_reinforcements();
	run_random_events();
	sort_units();
	init_action_order();
	compute_moves();
	run_spies();
	run_tech_leakage();
	cache_init_tech_levels();
/*	reset_coverage();  */
	gamestatesafe = FALSE;
    }
    if ((checkpointinterval > 0) && (curturn % checkpointinterval == 0)) {
	write_entire_game_state(checkpoint_name());
    }
    init_movement();
    update_all_progress_displays("", -1);
    time(&turncalcend);
    Dprintf("%d seconds to calc at turn start\n",
	    idifftime(turncalcend, turncalcstart));
}

init_movement()
{
    Side *side, *side2;

    for_all_sides(side) {
	if (side->ingame) {
	    /* No units are waiting for orders initially. */
	    side->numwaiting = 0;
	}
    }
    for_all_sides(side) {
	side->turnstarttime = time(0);
	/* Didn't really do input, but useful to pretend so. */
	side->lasttime = time(0);
	if (side_has_ai(side)) {
	    ai_init_turn(side);
	}
	side->busy = FALSE;
	if (side_has_display(side)) {
	    update_action_display(side, TRUE);
	}
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

run_tech_leakage()
{
    int u;
    int anytechleakage = FALSE;
    Side *side, *side2;
	
    for_all_unit_types(u) {
	if (u_tech_leakage(u) > 0) {
	    anytechleakage = TRUE;
	    break;
	}
    }
    if (anytechleakage) {
	Dprintf("Running tech leakage\n");
	for_all_sides(side) {
	    for_all_sides(side2) {
		if (side != side2 /* and some contact between sides */) {
		    for_all_unit_types(u) {
			if (side->tech[u] < side2->tech[u]
			    && u_tech_leakage(u) > 0) {
			    side->tech[u] += u_tech_leakage(u) / 100
			      + probability(u_tech_leakage(u) % 100);
			}
		    }
		}
	    }
	}
    }
}

/* Remember each side's tech levels before it does any research actions
   during the turn.  This can be used to keep tech level from going up too
   fast if the player has lots of units doing research. */

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

/* Clean out all details about what order units will move in. */

init_action_order()
{
    Unit *unit;
    Side *side;

    if (actionvector == NULL) actionvector = make_unit_vector(2000);
    clear_unit_vector(actionvector);
    for_all_sides_plus_indep(side) {
	side->actorder = NULL;
    }
    for_all_units(side, unit) {
	/* Units start a turn *not* in reserve. */
	if (unit->plan != NULL) {
	    unit->plan->reserve = FALSE;
	}
    }
}

/* Compute moves and actions for all the units at once, put everybody that
   can do anything into a list. */

compute_moves()
{
    Unit *unit;
    Side *side;

    if (midturnrestore) { /* not necessary because already made? */
	midturnrestore = FALSE;
	/* but need to arrange numacting etc? */
    } else { /* for all sided units (why?). */
	for_all_sides(side) {
	    side->numacting = 0;
	    side->numfinished = 0;
	    for_all_side_units(side, unit) {
		if (unit->act) {
		    compute_acp(unit);
		    if (unit->act->initacp > 0) {
		        actionvector =
			  add_unit_to_vector(actionvector, unit, 0);
		    }
		}
	    }
	}
    }
}

/* Compute the action points available to the unit this turn. */

compute_acp(unit)
Unit *unit;
{
    int u = unit->type, acp, maxacp, minacp, acplimit;
    Unit *occ;

    /* Units still under construction or off-area can't do anything. */
    if (!completed(unit) || !inside_area(unit->x, unit->y)) {
	unit->act->initacp = unit->act->acp = unit->act->actualactions = 0;
	return;
    }
    /* First compute how many action points are available. */
    acp = u_acp(u);
    if (unit->hp < u_hp_at_max_acp_turn(u)) {
	if (unit->hp <= u_hp_at_min_acp_turn(u)) {
	    acp = u_acp_turn_min(u);
	} else if (u_hp_at_min_acp_turn(u) == u_hp_at_max_acp_turn(u)) {
	    /* ? */
	    acp = 0;
	} else {
	    /* Interpolate to get acp, rounding down. */
	    acp = u_acp_turn_min(u) + (u_acp(u) - u_acp_turn_min(u)) /
	      (u_hp_at_max_acp_turn(u) - u_hp_at_min_acp_turn(u));
	}
    }
    /* Add effect of occupants. */
    acplimit = u_acp(u);
    for_all_occupants(unit, occ) {
	if (is_active(occ)) {
	    acp = (acp * uu_occ_acp(occ->type, u)) / 100;
	    acplimit = max(acp, acplimit);
	}
    }
    /* Adjust for night time. */
    if (night_at(unit->x, unit->y)) {
    	/* (this doesn't account for unit being on a road at night) */
    	acp = (acp * ut_night_acp_effect(u, terrain_at(unit->x, unit->y))) / 100;
    }
    unit->act->initacp = unit->act->acp + acp;
    minacp = u_acp_min(u);
    maxacp = (u_acp_max(u) < 0 ? acplimit : u_acp_max(u));
    unit->act->initacp = min(max(minacp, unit->act->initacp), maxacp);
    unit->act->acp = unit->act->initacp;
    unit->act->actualactions = unit->act->actualmoves = 0;
}

/* Do some number of actions. */

move_some_units(lim)
int lim;
{
    int num = 0, curactor;
    Unit *unit;

    /* Negative limits are effectively infinite. */
    if (lim < 0) lim = 1000000;
    for (curactor = 0; curactor < actionvector->numunits; ++curactor) {
	unit = (actionvector->units)[curactor].unit;
	if (unit_still_acting(unit, TRUE)) {
	    num += move_one_unit_multiple(unit, lim - num);
	}
	if (num >= lim) break;
    }
    return num;
}

unit_still_acting(unit, checkwaiting)
Unit *unit;
int checkwaiting;
{
    return (in_play(unit)
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

move_one_unit_multiple(unit, lim)
Unit *unit;
int lim;
{
    int num = 0, buzz = 0, acp1;
    int rslt;

    if (unit->act == NULL || unit->act->initacp < 1) return 0;
    acp1 = unit->act->acp;
    while (in_play(unit)
	   && (unit->act
	       && unit->act->acp > u_acp_min(unit->type))
	   && ((unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve)
	       || has_pending_action(unit))
	   && num < lim
	   && buzz < lim) {
	if (has_pending_action(unit)) {
	    /* Execute the action directly. */
	    rslt = execute_action(unit, &(unit->act->nextaction));
	    /* Clear the action.  Note that the unit might have changed
	       to a non-acting type, so we have to check for act struct. */
	    if (unit->act) unit->act->nextaction.type = A_NONE;
	    /* In any case, the game state is irrevocably altered. */
	    gamestatesafe = FALSE;
	    ++num;
	} else if (unit->plan) {
	    if (unit->plan->waitingfortasks
		|| unit->plan->asleep
		|| unit->plan->reserve) break;
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
	    if (unit->plan) unit->plan->type = PLAN_NONE;
	    if (unit->plan && probability(5)) unit->plan->asleep = TRUE;
	    Dprintf("%s was confused, starting over\n", unit_desig(unit));
	}
    }
    return num;
}

/* This explicitly finishes out a side's activity for the turn. */

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
	if (side2->ingame) {
	    update_side_display(side2, side, TRUE);
	}
    }
    Dprintf("%s finished its turn.\n", side_desig(side));
}

/* Take care of details that no longer require any interaction, at least
   none that can't wait until the next turn. */

finish_movement()
{
    int lostacp;
    Unit *unit;
    Side *side, *side2;

    /* Remove dead units that have been dead more than a turn. */
    for_all_sides (side) {
	lostacp = 0;
	flush_side_dead(side);
	/* Clear wakeup reasons for non-movers. */
	/* need to fix this so that these stay around for a turn. */
	for_all_side_units(side, unit) {
/*	    side->laststarttime = time(0); */
	    if (Debug && in_play(unit) && unit->act && unit->act->acp > 0) {
		lostacp += unit->act->acp;
	    }
	}
	if (lostacp > 0) {
	    Dprintf("%s forfeited %d acp overall.\n", side_desig(side), lostacp);
	}
	if (side_has_ai(side)) {
	    mplayer_finish_movement(side);
	}
    }
    for_all_sides(side)  {
	for_all_sides(side2) {
	    update_side_display(side, side2, TRUE);
	}
    }
}

/* See how any agreements' terms are holding up. */

test_agreements()
{
    Agreement *ag;
    Side *side;
    Unit *unit;

    for_all_agreements(ag) {
	if (ag->active) {
		/* what? */
	}
    }
}

/* Compute the position of the sun for this turn. */

run_sun()
{
    int daylen = world.daylength, lastsunx, lastsuny, x, y;
    Side *side;

    switch (daylen) {
      case 0:
	/* Sun is at a fixed position. */
	/* (should be possible to set explicitly somehow? implicitly from lat/long?) */
	sunx = area.width / 2;  suny = area.height / 2;
	daynight = TRUE;
	break;
      case 1:
	/* No sun effects at all, every place uniformly lit. */
	break;
      default:
	/* If world has a appropriate circumference, the sun moves over
	   it at a regular pace. */
	if (world.circumference >= area.width) {
	    lastsunx = sunx;  lastsuny = suny;
	    sunx = (((curturn + g_initial_day_part() + daylen) % daylen)
	             * world.circumference) / daylen + area.width / 2;
	    /* (should adjust suny for nonzero latitudes and axial tilt) */
	    suny = area.height / 2;
	    daynight = TRUE;
	    	/* Update the appearance of any cells whose lighting has changed. */
	    	for_all_hexes(x, y) {
	    	    if (lighting(x, y, sunx, suny) != lighting(x, y, lastsunx, lastsuny)) {
	    	    	for_all_sides(side) {
	    	    	    update_cell_display(side, x, y, FALSE);
	    	    	}
	    	    }
	    	}
	}
	break;
    }
    if (daynight) {
	Dprintf("Sun is now at %d,%d\n", sunx, suny);
    }
}

/* Seasonal change moves the average temperate up and down gradually,
   modifying it according to the terrain underneath. */

/* This should check that equator and northpole are not equal. */

run_environment()
{
    int yrlen = world.yearlength;
    int season, x, y, dir, t, lattemp, celltemp;
    int u, temp, comfmin, comfmax, range;
    float spoletemp, npoletemp, equatortemp;
    float poledelta, equatordelta, ntempdelta, stempdelta;
    extern int mintemp, maxtemp;
    Unit *unit;
    Side *side;
    if (mintemp == maxtemp) return;
    if (!temperatures_defined()) {
        allocate_area_temperatures();
        allocate_area_scratch(2);
    }
#if 0
    if (yrlen > 1 /* and axial tilt... */) {
	/* Compute the difference between summer and winter temps. */
	/* (Should be precomputed.) */
	poledelta = (g_summer_pole() - g_winter_pole()) / (yrlen / 2);
	equatordelta = (g_summer_equator() - g_winter_equator()) / (yrlen / 2);
	/* Compute where we are in the cycle of seasons. */
	season = (curturn + (yrlen - g_first_midwinter())) % yrlen;
	/* Compute temps at poles and equator. */
	if (season <= yrlen / 2) {
	    npoletemp = g_winter_pole() + season * poledelta;
	    equatortemp = g_winter_equator() + season * equatordelta;
	    spoletemp = g_winter_pole() + (yrlen/2 - season) * poledelta;
	} else {
	    npoletemp = g_winter_pole() + (yrlen - season) * poledelta;
	    equatortemp = g_winter_equator() + (yrlen - season) * equatordelta;
	    spoletemp = g_winter_pole() + (season - yrlen/2) * poledelta;
	}
	Dprintf("Temps N %f Eq %f S %f\n", npoletemp, equatortemp, spoletemp);
	/* Compute the temperature variation from one lat to the next. */
	ntempdelta = (equatortemp-npoletemp)/(world.northpole-world.equator);
	stempdelta = (equatortemp-spoletemp)/(world.equator-world.southpole);
	}
#endif
    /* Compute the average temperature at each point in the world. */
    for (y = area.height-1; y >= 0; --y) {
#if 0
	/* For each latitude, compute the average temp. */
	if (y >= world.equator) {
	    lattemp = npoletemp
	      + ntempdelta * ((world.northpole - world.equator)
			      - (y - world.equator));
	} else {
	    lattemp = spoletemp
	      + stempdelta * ((world.equator - world.southpole)
			      - (world.equator - y));
	}
#endif
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
	for_all_interior_hexes(x, y) {
	    set_tmp2_at(x, y, temperature_at(x, y));
	    for_all_directions(dir) {
		set_tmp2_at(x, y,
			    (tmp2_at(x, y)
			     + temperature_at(x+dirx[dir], y+diry[dir])));
	    }
	}
	for_all_interior_hexes(x, y) {
	    set_temperature_at(x, y, (tmp2_at(x, y) / (NUMDIRS + 1)));
	}
    }
    /* See if any displays should change and report if so. */
    for_all_sides(side) {
	if (side_has_display(side)) {
            for_all_hexes(x, y) {
	        if (temperature_at(x, y) != tmp1_at(x, y)
		    && (g_see_all() || terrain_view(side, x, y) != UNSEEN)) {
		    update_cell_display(side, x, y, 34);
		}
	    }
	    flush_display_buffers(side);
	}
    }
    /* Affect any temperature-sensitive units. */
    for_all_units(side, unit) {
	if (in_play(unit)
	    && unit->transport == NULL /* or no protection */) {
	    u = unit->type;
	    temp = temperature_at(unit->x, unit->y);
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
	}
    }
}

damage_unit_with_temperature(unit, n)
Unit *unit;
int n;
{
    if (n >= unit->hp) {
	Dprintf("%s dies from excessive temps\n", unit_desig(unit));
	kill_unit(unit, -1);
    } else {
	Dprintf("%s damaged by excessive temps\n", unit_desig(unit));
	unit->hp -= n;
    }
}

/* Decide whether spying can happen in this game. */

spying_possible()
{
    int u;

    /* Not much point in spying if everything is always open to view. */
    if (g_see_all()) return FALSE;
    for_all_unit_types(u) {
	if (u_spy_chance(u) > 0) return TRUE;
    }
    return FALSE;
}

/* Given that the spying unit is going to get info about other units at this
   location, figure out just what it is going to see. */

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

run_spies()
{
    Side *side;
    Unit *unit;

    if (spying_possible()) {
	Dprintf("Running spies\n");
	for_all_sides(side) {
	    for_all_side_units(side, unit) {
		if (u_spy_chance(unit->type) > 0 && in_play(unit)) {
		    if (xrandom(10000) < u_spy_chance(unit->type)) {
			/* Spying is successful, decide how much was seen. */
			tmpunit = unit;
			apply_to_area(unit->x, unit->y, u_spy_range(unit->type),
				      spy_on_location);
		    }
		}
	    }
	}
    }
}

/* Figure out ahead of time which random event methods to run. */

init_random_events()
{
    int i, k;
    Obj *randomeventlist, *rest, *evttype;

    numrandomevents = 0;
    randomeventlist = g_random_events();
    if (randomeventlist == NULL || randomeventlist == lispnil) return;
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

init_accidents()
{
    int u, t;
    
    for_all_unit_types(u) {
    	for_all_terrain_types(t) {
    	    if (ut_accident_vanish(u, t) > 0
		|| ut_accident_hit(u, t) > 0) return TRUE;
    	}
    }
    return FALSE;
}

/* Test each unit that is out in the open to see if a terrain-related
   accident happens to it.  Accidents can either kill the unit instantly or
   just damage it. */
 
run_accidents()
{
    int t;
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
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

init_attrition()
{
    int u, t;
    
    for_all_unit_types(u) {
    	for_all_terrain_types(t) {
    	    if (ut_attrition(u, t) > 0) return TRUE;
    	}
    }
    return FALSE;
}

/* Attrition only takes out a few hp at a time, but can be deadly... */

run_attrition()
{
    int u, hipart, lopart, dmg;
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
	if (in_play(unit)) {
	    u = unit->type;
	    hipart = ut_attrition(u, terrain_at(unit->x, unit->y)) / 100;
	    lopart = ut_attrition(u, terrain_at(unit->x, unit->y)) % 100;
	    dmg = hipart + probability(lopart);
	    /* This is like hit_unit but doesn't have other effects. */
	    unit->hp2 -= dmg;
	    /* (should be able to pass reason to damage_unit) */
	    damage_unit(unit);
	}
    }
}

/* Test whether revolts can ever happen in this game. */

init_revolts()
{
    int u;

    for_all_unit_types(u) {
	if (u_revolt(u) > 0) return TRUE;
    }
    return FALSE;
}

/* Check each unit to see whether it revolts spontaneously.  While
   surrender is influenced by nearby units, revolt takes only the
   overall state of the world into account. */

run_revolts()
{
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
	if (in_play(unit) && u_revolt(unit->type) > 0) {
	    unit_revolt(unit);
	}
    }
}

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
		newside = side_n(random(numsides + 1));
		if (unit_allowed_on_side(unit, newside)
		    && newside != oldside) {
		    break;
		}
	    }
	}
	/* Might not have been much of a revolt. :-) */
	if (newside == oldside) return;
	unit_changes_side(unit, newside, H_UNIT_ACQUIRED, 0 /* H_UNIT_REVOLTED */);
	see_exact(oldside, ux, uy);
	update_cell_display(oldside, ux, uy, TRUE);
	all_see_hex(ux, uy);
    }
}

/* Test whether surrenders can happen in this game. */

init_surrenders()
{
    return FALSE;
}

run_surrenders()
{
    Unit *unit;
    Side *loopside;

    for_all_units(loopside, unit) {
	if (in_play(unit)) {
	    unit_surrender(unit);
	}
    }
}

/* Units may also surrender to adjacent enemy units, but only to a type */
/* that is both visible and capable of capturing the unit surrendering. */
/* Neutrals have to be treated differently, since they don't have a view */
/* to work from.  We sort of compute the view "on the fly". */

#define u_siege(u) 0
#define u_surrender(u) 0

unit_surrender(unit)
Unit *unit;
{
    int surrounded = TRUE;
    int u = unit->type, chance, d, x, y;
    int view;
    Unit *unit2, *eunit = NULL;
    Side *us = unit->side, *es;

    chance = 0;
    for_all_directions(d) {
	point_in_dir(unit->x, unit->y, d, &x, &y);
	if (indep(unit)) {
	    if (((unit2 = unit_at(x, y)) != NULL) &&
		(0 /* probability(u_visibility(unit2->type)) */) &&
		(could_capture(unit2->type, u))) {
		chance += u_surrender(u);
		eunit = unit2;
	    } else {
		surrounded = FALSE;
	    }
	} else {
#if 0
	    view = side_unit_view(us, x, y);
	    if (view == EMPTY || view == UNSEEN) {
		surrounded = FALSE;
	    } else {
		es = side_n(vside(view));
		if (enemy_side(us, es) && could_capture(vtype(view), u)) {
		    chance += u_surrender(u);
		    if (unit_at(x, y)) eunit = unit_at(x, y);
		}
	    }
#endif
	}
    }
    if (surrounded && eunit) chance += u_siege(u);
    if (xrandom(10000) < chance) {
	if (eunit) {
	    /*		capture_unit(eunit, unit); */
	}
    }
}

int tmpexcess;

/* We've "found what we were searching for" when the excess to distribute
   is gone. */

excess_left(x, y)
int x, y;
{
    return (tmpexcess > 0);
}

try_transfer_to_cell(x, y)
int x, y;
{
    Unit *unit2, *occ;

    if (tmpexcess <= 0) return;
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

sharable_left(x, y)
int x, y;
{
    return tmpunit->supply[tmpmtype] > (um_storage(tmpunit->type, tmpmtype) / 2);
}

try_sharing_with_cell(x, y)
int x, y;
{
    Unit *unit2, *occ;

    if (!sharable_left(x, y)) return;
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

run_economy()
{
    int u, m, r, t, amt, dist, x, y, x1, y1, x2, y2;
    int totals[MAXMTYPES];
    Unit *unit, *ounit;
    Side *loopside;

    if (nummtypes == 0) return;
    /* (should find other reasons not to run this) */
    Dprintf("Running economy\n");
    /* (should produce in terrain here) */
    for_all_material_types(m) totals[m] = 0;
    /* Make new materials but don't clip to storage capacity yet. */
    for_all_units(loopside, unit) {
    	if (in_play(unit) && completed(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		t = terrain_at(unit->x, unit->y);
		amt = (um_produce(u, m) * ut_productivity(u, t)) / 100 +
		  (probability((um_produce(u, m) * 
				ut_productivity(u, t)) % 100) ? 1 : 0);
		unit->supply[m] += amt;
		if (Debug && unit->supply[m] > um_storage(u, m))
		  totals[m] += (unit->supply[m] - um_storage(u, m));
	    }
	}
    }
    Dprintf("Overflow is");
    for_all_material_types(m) Dprintf(" %d", totals[m]);
    Dprintf("\n");
    /* Move stuff around - try to get rid of any excess. */
    for_all_units(loopside, unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		if (unit->supply[m] > um_storage(u, m)
		    && (dist = um_outlength(u, m)) >= 0) {
		    	tmpunit = unit;
		    	tmpmtype = m;
		    	tmpexcess = unit->supply[m] - um_storage(u, m);
			search_and_apply(unit->x, unit->y, dist, excess_left,
					 &x1, &y1, 1, try_transfer_to_cell, 999999);
		}
	    }
	}
    }
    /* (should move materials around) */
    /* (should move in/out of units as needed) */
    /* (should throw away excess in terrain here) */
    /* (should) Limit overall people density. */
    if (0 /* any people limits */) {
    	int numpeople;

	for_all_hexes(x, y) {
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
    for_all_material_types(m) totals[m] = 0;
    for_all_units(loopside, unit) {
	u = unit->type;
	for_all_material_types(m) {
	    if (Debug && unit->supply[m] > um_storage(u, m))
		  totals[m] += (unit->supply[m] - um_storage(u, m));
	    unit->supply[m] = min(unit->supply[m], um_storage(u, m));
	}
    }
    Dprintf("Discarded ");
    for_all_material_types(m) Dprintf(" %d", totals[m]);
    Dprintf("\n");
    /* The next phase is for sharing of scarce supplies. */
    for_all_units(loopside, unit) {
	if (in_play(unit) && !indep(unit)) {
	    u = unit->type;
	    for_all_material_types(m) {
		if ((dist = um_outlength(u, m)) >= 0) {
		    	tmpunit = unit;
		    	tmpmtype = m;
			search_and_apply(unit->x, unit->y, dist, sharable_left,
					 &x1, &y1, 1, try_sharing_with_cell, 999999);
		}
	    }
	}
    }
}

/* Give away supplies, but save enough to stay alive for a couple turns. */

try_transfer(from, to, r)
Unit *from, *to;
int r;
{
    int u = from->type;
    int oldsupply = from->supply[r];

#if 0  
    int save_amount;  /* Hang on to this much at least. */

    save_amount = max(3 * um_consume(u, r),
		      2 * u_speed(u) * um_tomove(u, r));
    save_amount = min(save_amount, from->supply[r]);
    from->supply[r] -= save_amount;
#endif
    try_transfer_aux(from, to, r);
/*    from->supply[r] += save_amount; */
    tmpexcess -= (oldsupply - from->supply[r]);
}

try_sharing(from, to, m)
Unit *from, *to;
int m;
{
    int u = from->type;
    int oldsupply = from->supply[m];
  
    try_transfer_aux(from, to, m);
}

/* Material redistribution uses this routine to move supplies around
   between units far apart or on the same hex. Try to do reasonable
   things with the materials.  Net producers are much more willing to
   give away supplies than net consumers. */

try_transfer_aux(from, to, r)
Unit *from, *to;
int r;
{
    int nd, u = from->type, u2 = to->type, fromrate, torate;
    Unit *occ;

    if (from != to &&
	um_inlength(u2, r) >= distance(from->x, from->y, to->x, to->y)) {
	/* Try for the transfer only if we're below capacity. */
	if ((nd = um_storage(u2, r) - to->supply[r]) > 0) {
	    if ((um_produce(u, r) > um_consume(u, r))
		|| (survival_time(to) < 3)
		|| (um_storage(u, r) * 4 >= um_storage(u2, r))) {
		if (can_satisfy_need(from, r, nd)) {
		    transfer_supply(from, to, r, nd);
		} else if (can_satisfy_need(from, r, max(1, nd/2))) {
		    transfer_supply(from, to, r, max(1, nd/2));
		} else if (from->supply[r] > um_storage(u, r)) {
		    transfer_supply(from, to, r,
				    (from->supply[r] - um_storage(u, r)));
		}
	    } else {
		fromrate = u_speed(u) * um_tomove(u, r) * 3;
		fromrate = max(1, fromrate);
		torate = u_speed(u2) * um_tomove(u2, r) * 3;
		torate = max(1, torate);
		if ((from->supply[r] / fromrate) > (to->supply[r] / torate)) {
		    transfer_supply(from, to, r,
				    min(nd, (8 + from->supply[r]) / 9));
		} 
	    }
	}
    }
}

/* This estimates what can be spared.  Note that total transfer of requested */
/* amount is not a good idea, since the supplies might be essential to the */
/* unit that has them first.  If we're more than half full, or the request */
/* is less than 1/5 of our supply, then we can spare it. */

/* (should replace with doctrine/plan controls, check underlying terrain) */

can_satisfy_need(unit, r, need)
Unit *unit;
int r, need;
{
    return ((((2 * unit->supply[r]) > (um_storage(unit->type, r))) &&
	     (((unit->supply[r] * 4) / 5) > need)) ||
	    (need < (um_storage(unit->type, r) / 5)));
}

/* Do everything associated with the end of a turn. */

run_turn_end()
{
    finish_movement();
    run_base_consumption();
    run_people_side_changes();
    flush_dead_units();
    check_post_turn_scores();
    test_agreements();
    if (Debug) report_malloc();
}

/* Handle base consumption by units. */

run_base_consumption()
{
    Unit *unit;
    Side *side;

    if (nummtypes == 0) return;
    if (0 /* any nonzero consumptions? */) return;
    Dprintf("Running base consumption\n");
    for_all_units(side, unit) {
	if (is_active(unit)) {
	    unit_consumes(unit);
	}
    }
}

/* Consume the constant overhead part of supply consumption. */
/* Usage by movement is subtracted from overhead first. */

unit_consumes(unit)
Unit *unit;
{
    int u = unit->type, m, usedup;
    
    for_all_material_types(m) {
	if (alive(unit) &&
	    um_consume(u, m) > 0 &&
	    (unit->transport == NULL /* || u_consume_as_occupant(u) */)) {
	    /* Calculate what was already consumed by movement. */
	    usedup = 0;
	    if (unit->act != NULL) {
	    	usedup = unit->act->actualmoves * um_consume_per_move(u, m);
	    }
	    if (usedup < um_consume(u, m)) {
		unit->supply[m] -= (um_consume(u, m) - usedup);
	    }
	    if (unit->supply[m] <= 0 && !in_supply(unit, m)) {
		exhaust_supply(unit, m, TRUE);
	    }
	}
    }
    if (alive(unit)
    	&& unit->plan
    	&& (unit->plan->alarmmask & 0x01)
    	&& !(unit->plan->alarms & 0x01)
    	&& past_halfway_point(unit)
    	) {
    	unit->plan->alarms &= 0x01;
    }
}

/* What happens to a unit that runs out of a material.  If it can survive
   on no supplies, then there may be a few turns of grace, depending on
   how the dice roll... */

exhaust_supply(unit, m, partial)
Unit *unit;
int m, partial;
{
    int u = unit->type, hploss, starv;

    starv = um_hp_per_starve(u, m); 
    hploss = starv / 100 + probability(starv % 100);
    if (hploss >= unit->hp) {
	/* (should let occupants try to escape first) */
	kill_unit(unit, H_UNIT_STARVED);
    } else if (partial) {
    	unit->hp -= hploss;
    	/* (should do other hp loss consequences) */
	/* (use generic damage routine?) */
    }
}

/* Check if the unit has ready access to a source of supplies. */

/* (should be more sophisticated and account for supply lines etc) */

in_supply(unit, m)
Unit *unit;
int m;
{
    if (unit->transport != NULL) {
    	if (unit->transport->supply[m] > 0) return TRUE;
    }
    return FALSE;
}

/* Some types of units can become completed and grow to full size
   automatically when they get to a certain point. */

run_self_builds()
{
    int u, cpper;
    Unit *unit;
    Side *loopside;

    if (anyselfbuilds < 0) {
	anyselfbuilds = FALSE;
	for_all_unit_types(u) {
	    if (u_cp_per_self_build(u) > 0) {
		anyselfbuilds = TRUE;
		break;
	    }
	}
    }
    if (!anyselfbuilds) return;
    Dprintf("Running self builds\n");
    for_all_units(loopside, unit) {
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

run_people_side_changes()
{
    int x, y, u, t;
    Unit *unit;

    if (!people_sides_defined()) return;
    if (anypeoplesidechanges < 0) {
	anypeoplesurrenders = (int *) xmalloc(numutypes * sizeof(int));
	anypeoplesidechanges = FALSE;
	for_all_unit_types(u) {
		for_all_terrain_types(t) {
	    		if (ut_people_surrender(u, t) > 0) {
				anypeoplesidechanges = TRUE;
				anypeoplesurrenders[u] = TRUE;
				break;
			}
	   	}
	}
    }
    if (!anypeoplesidechanges) return;
    for_all_hexes(x, y) {
	if (unit_at(x, y) != NULL) {
		t = terrain_at(x, y);
		for_all_stack(x, y, unit) {
		    /* The people here may change sides. */
		    if (probability(ut_people_surrender(unit->type, t))) {
			change_people_side_around(x, y, unit->type, unit->side);
		    }
		}
	} else {
		/* Unoccupied cells might see population revert. */
		/* (this would need multiple-loyalty pops) */
	}
    }
}

change_people_side_around(x, y, u, side)
int x, y, u;
Side *side;
{
    int pop = people_side_at(x, y), s = side_number(side), dir, x1, y1;

    if (pop != NOBODY && pop != s /* and not allied side */) {
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

Obj *
get_x_property(unit, subkey)
Unit *unit;
int subkey;
{
    Obj *lis, *bdg;

    for (lis = unit->hook; lis != lispnil; lis = cdr(lis)) {
	bdg = car(lis);
	if (symbolp(car(bdg)) && keyword_code(c_string(car(bdg))) == subkey)
	  return cdr(bdg);
    }
    return lispnil;
}

Obj *
get_x_property_by_name(unit, str)
Unit *unit;
char *str;
{
    Obj *lis, *bdg;

    for (lis = unit->hook; lis != lispnil; lis = cdr(lis)) {
	bdg = car(lis);
	if (symbolp(car(bdg)) && strcmp(c_string(car(bdg)), str) == 0) {
	    return cdr(bdg);
	}
    }
    return lispnil;
}

/* See if it's time for any scheduled arrivals to appear. */

run_reinforcements()
{
    int nx, ny, nw, nh, nx1, ny1;
    Obj *appear;
    Unit *unit, *transport;
    Side *loopside;

    if (0 /* any reinforcements? */) return;
    Dprintf("Running reinforcements\n");
    for_all_units(loopside, unit) {
    	/* See if now time for a unit to appear. */
    	if (unit->cp < 0
	    && unit->hook != lispnil
	    && (c_number(car(appear = get_x_property(unit, K_APPEAR)))
		<= curturn)) {
	    /* Set the unit to its correct cp. */
	    unit->cp = (- unit->cp);
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
	    /* (should be able to retry with different loc if nw or nh > 0) */
	    if (inside_area(nx, ny)) {
		if (can_occupy_cell(unit, nx, ny)) {
		    enter_hex(unit, nx, ny);
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

int anyhprecovery = -1;

/* Some types of units recover lost hp spontaneously. */

run_hp_recovery()
{
    int u, hprecovery, hpmax, oldhp;
    Unit *unit;
    Side *loopside;

    if (anyhprecovery < 0) {
	anyhprecovery = FALSE;
    	for_all_unit_types(u) {
	    if (u_hp_recovery(u) > 0) {
		anyhprecovery = TRUE;
		break;
	    }
	}
    }
    if (!anyhprecovery) return;
    Dprintf("Running hp recovery\n");
    for_all_units(loopside, unit) {
      if (in_play(unit)) {
	u = unit->type;
	hprecovery = u_hp_recovery(u);
	hpmax = u_hp(u);
	/* (should only do for one part of multi-part unit?) */
	if (hprecovery > 0 && unit->hp < hpmax) {
	    oldhp = unit->hp;
	    unit->hp += hprecovery / 100;
	    /* Maybe recover an additional hit point. */
	    if ((hprecovery % 100) != 0) {
		if (probability(hprecovery % 100)) ++unit->hp;
	    }
	    if (unit->hp > hpmax) unit->hp = hpmax;
	    /* Inform the player if the unit's hp changed. */
	    if (unit->hp != oldhp) {
		update_unit_display(unit->side, unit, TRUE);
	    }
	}
      }
    }
}

/* Unconditional surrender. */

resign_game(side, side2)
Side *side, *side2;
{
    /* Tell everybody about the resignation. */
    /* (should record an appropriate event, will be textified by event handler) */
    side_loses(side, side2);
}

/* This is true if there is any kind of realtime limit on the game. */

realtime_game()
{
    return (g_rt_for_game() > 0
    	    || g_rt_per_side() > 0
    	    || g_rt_per_turn() > 0);
}

all_others_willing_to_quit(side)
Side *side;
{
    Side *side2;

    for_all_sides(side2) {
	if (side != side2 && !side2->willingtodraw) return FALSE; 
    }
    return TRUE;
}

/* This forces an end to the game directly. */

end_the_game()
{
    Side *side;

    Dprintf("The game is over.\n");
    record_event(H_GAME_ENDED, -1, NULL);
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
}

recalc_caches()
{
    int u;
    Unit *unit;
    Side *side;

#if 0
    for_all_sides(side) {
	for_all_unit_types(u) {
	    side->units[u] = 0;
	    side->unitsbuilding[u] = 0;
	}
	for_all_side_units(side, unit) {
	    if (alive(unit)) {
		if (completed(unit)) {
		    ++(side->units[unit->type]);
		} else {
		    ++(side->unitsbuilding[unit->type]);
		}
	    }
	}
    }
#endif
}

/* (needs a better home?) */

/* Given a side and a unit, calculate the correct "next unit".  Typically
   used by autonext options, thus the name. */

Unit *
autonext_unit(side, unit)
Side *side;
Unit *unit;
{
    int i, uniti = -1, n;
    Unit *nextunit;

    if (!side->ingame
	|| side->finishedturn
	|| actionvector == NULL)
      return NULL;
    if (could_be_next_unit(unit) && side_controls_unit(side, unit)) return unit;
    for (i = 0; i < actionvector->numunits; ++i) {
    	nextunit = (actionvector->units)[i].unit;
    	if (in_play(nextunit) && side_controls_unit(side, nextunit)) {
	    if (unit == NULL || unit == nextunit) {
		uniti = i;
		break;
	    }
    	}
    }
    if (uniti < 0) return NULL;
    /* (should scan for both a preferred and an alternate - preferred
       could be within a supplied bbox so as to avoid scrolling) */
    for (i = uniti; i < uniti + actionvector->numunits; ++i) {
    	n = i % actionvector->numunits;
    	nextunit = (actionvector->units)[n].unit;
    	if (could_be_next_unit(nextunit) && side_controls_unit(side, nextunit)) return nextunit;
    }
    return NULL;
}

could_be_next_unit(unit)
Unit *unit;
{
    return (unit != NULL
	    && alive(unit)
	    && inside_area(unit->x, unit->y)
	    && (unit->act
		&& unit->act->acp > 0
		&& !has_pending_action(unit))
	    && (unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& unit->plan->waitingfortasks));
}
