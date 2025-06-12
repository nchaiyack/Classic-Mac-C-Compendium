/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Unit plan execution. */

#include "conq.h"

/* (should have a generic struct for all plan type attrs) */

char *plantypenames[] = {

#undef  DEF_PLAN
#define DEF_PLAN(NAME,code) NAME,

#include "plan.def"

    NULL
};

char *task_desig();
TaskOutcome execute_task();

/* Execute the plan. */

execute_plan(unit, try)
Unit *unit;
int try;
{
    Plan *plan = unit->plan;
    int u = unit->type, u2;
    Side *us = unit->side;

    if (!in_play(unit) || !completed(unit)) {
	DMprintf("%s shouldn't be planning yet\n", unit_desig(unit));
	return; 
    }
    DMprintf("%s using plan %s", unit_desig(unit), plan_desig(plan));
    if (try > 1) DMprintf(" (try #%d)", try);
    DMprintf("\n");
    /* Units that are asleep or in reserve do nothing. */
    if (plan->asleep || plan->reserve) return;
    if (try > 5) {
	DMprintf("%s redoing plan too often, going into reserve\n",
		 unit_desig(unit));
	plan->reserve = TRUE;
	return;
    }
    /* Unit actually has a plan, dispatch on its type. */
    switch (plan->type) {
      case PLAN_NONE:
	/* We get one chance to form a plan, putting the unit to sleep
	   if it doesn't work. */
	decide_plan(unit->side, unit);
	if (plan->type != PLAN_NONE) {
	    execute_plan(unit, ++try);
	} else {
	    DMprintf("%s could not form a plan, going to sleep\n",
		     unit_desig(unit));
	    plan->asleep = TRUE;
	}
	break;
      case PLAN_PASSIVE:
	/* Passive units just work from the task queue or wait
	   to be told what to do. */
	if (plan->alarms & 0x01) {
		set_resupply_task(unit);
		plan->alarmmask &= ~(0x01);
	}
	if (plan->tasks) {
	    /* (should check that doctrine being followed correctly) */
	    execute_task(unit);
	} else if (plan->autotask) {
	    decide_tasks(unit);
	    execute_plan(unit, ++try);
	} else if (unit->side && side_has_ai(unit->side)) {
	    /* Give AI a chance to dream up a more interesting plan. */
	    decide_plan(unit->side, unit);
	    execute_plan(unit, ++try);
	} else if (doctrine_allows_wait(unit)) {
	    /* Our goal is now to get guidance from the side. */
	    wait_for_orders(unit);
	} else {
	    /* We just are not getting any guidance at all... */
	    DMprintf("%s passive plan not working, going to sleep\n",
		     unit_desig(unit));
	    plan->asleep = TRUE;
	}
	break;
      case PLAN_OFFENSIVE:
	plan_offense(unit);
	break;
      case PLAN_DEFENSIVE:
	plan_defense(unit);
	break;
      case PLAN_EXPLORATORY:
	plan_exploration(unit);
	break;
      case PLAN_RANDOM:
	plan_random(unit);
	break;
      default:
	case_panic("plan type", plan->type);
	break;
    }
}

/* A unit operating offensively advances and attacks when possible. */

plan_offense(unit)
Unit *unit;
{
    int u = unit->type, u2;
    int x, y, w, h, range, x1, y1;
    Plan *plan = unit->plan;
    Task *task;
    Side *us = unit->side;

    if (resupply_if_low(unit)) return;
    if (rearm_if_low(unit)) return;
    if (plan->tasks) {
    	execute_task(unit);
    	return;
    }
    if (plan->maingoal && mobile(u)) {
	switch (plan->maingoal->type) {
	  case VICINITY_HELD:
	    x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
	    w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
	    if (distance(x, y, unit->x, unit->y) > w) {
	    	if (random_point_near(x, y, w / 2, &x1, &y1)) {
		    x = x1;  y = y1;
	    	}
		DMprintf("%s to go on offensive to %d,%d\n",
			 unit_desig(unit), x, y);
		push_movenear_task(unit, x, y, w / 2);
		if (unit->transport
		    && mobile(unit->transport->type)
		    && unit->transport->plan) {
		    push_movenear_task(unit->transport, x, y, w / 2);
		}
	    } else {
		range = w;
		/* No special goal, look for something to fight with. */
		/* Sometimes be willing to look a little farther out. */
		if (probability(50)) range *= 2;
		if (do_for_occupants(unit)) {
		} else if (go_after_victim(unit, range)) {
		} else if (!g_see_all()) {
		    DMprintf("%s will explore instead\n", unit_desig(unit));
		    plan_exploration(unit); /* or patrol */
		    /* Running under exploration rules now. */
		    return;
		}
	    }
	    break;
	  default:
	    DMprintf("offensive unit has some goal\n");
	    break;
	}
    } else if (mobile(u)) {
	range = operating_range_best(u);
	if (probability(50)) range = min(range, 2 * u_acp(u));
	if (do_for_occupants(unit)) {
	} else if (go_after_victim(unit, range)) {
	    /* No special goal, but found something to fight with. */
	} else if (!g_see_all()) {
	    DMprintf("%s will explore instead\n", unit_desig(unit));
	    plan_exploration(unit); /* or patrol */
	    /* Running under exploration rules now. */
	    return;
	} else {
	    /* should go to a "best location" if possible. */
	    /* (should do a sentry task) */
	}
    } else if (can_fire(unit) && fire_at_opportunity(unit)) {
    } else {
	plan_offense_support(unit);
    }
    if (plan->tasks) {
    	execute_task(unit);
    } else {
    	DMprintf("%s found nothing to do offensively", unit_desig(unit));
    	if (flip_coin()) {
    	    DMprintf("- going into reserve");
    	    plan->reserve = TRUE;
    	} else if (probability(5)) {
    	    DMprintf("- going to sleep");
    	    plan->asleep = TRUE;
    	}
    	DMprintf("\n");
    }
}

do_for_occupants(unit)
Unit *unit;
{
    Unit *occ;
    Goal *goal;
    Task *task;

    for_all_occupants(unit, occ) {
	if (occ->plan) {
	    /* Get the unit towards its goal, if it has one. */
	    if ((goal = occ->plan->maingoal) != NULL
		&& goal->type == VICINITY_HELD
		&& distance(goal->args[0], goal->args[1], unit->x, unit->y) > goal->args[2]) {
		order_movenear(unit, goal->args[0], goal->args[1], goal->args[2] / 2);
		DMprintf("%s will go where occupant %s wants to go\n",
			 unit_desig(unit), unit_desig(occ));
		return TRUE;
	    }
	    /* If the unit does not have a goal, see if it has a task. */
	    for (task = occ->plan->tasks; task != NULL; task = task->next) {
		if ((task->type == MOVETO_TASK
		     || task->type == HIT_UNIT_TASK)
		    && (task->args[0] != unit->x
			|| task->args[1] != unit->y)) {
		    order_movenear(unit, task->args[0], task->args[1], 1);
		    DMprintf("%s will go where occupant %s wants to go\n",
			     unit_desig(unit), unit_desig(occ));
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

plan_offense_support(unit)
Unit *unit;
{
    int u = unit->type, u2, u3 = NONUTYPE, backup = NONUTYPE;
    Task *task;

    if (side_has_ai(unit->side)) {
	u3 = mplayer_preferred_build_type(unit->side, unit, PLAN_OFFENSIVE);
    } else {
	for_all_unit_types(u2) {
	    if (mobile(u2)
		&& (can_attack(u2) || can_fire(u2))
		&& uu_acp_to_create(u, u2) > 0) {
		backup = u2;
		if (flip_coin()) {
		    u3 = u2;
		    break;
		}
	    }
	}
    }
    if (u3 == NONUTYPE) u3 = backup;
    if (is_unit_type(u3)) {
	task = unit->plan->tasks;
    	if (task == NULL || task->type != BUILD_TASK) {
	    DMprintf("%s supporting offense by building %s\n",
		     unit_desig(unit), u_type_name(u3));
	    set_construction(unit, u3, 2);
	} else {
	    DMprintf("%s already building, leaving alone\n",
		     unit_desig(unit));
	}
    } else {
    	DMprintf("%s has no way to support an offensive\n", unit_desig(unit));
    }
}

set_construction(unit, u, num)
Unit *unit;
int u, num;
{
    Task *task = unit->plan->tasks;

    if (task != NULL && task->type == BUILD_TASK) {
	task->args[0] = u;
	task->args[1] = num;
	task->args[2] = 0;
    } else {
	push_build_task(unit, u, num);
    }
}

/* Defensive units don't go out looking for trouble, but they should
   react strongly to threats. */

plan_defense(unit)
Unit *unit;
{
    if (resupply_if_low(unit)) return;
    if (rearm_if_low(unit)) return;
    if (unit->plan->tasks) {
    	/* (should analyze and maybe decide to change task) */
    	execute_task(unit);
    	return;
    }
    if (0 /* has specific goal */) {
    } else if (inside_area(unit->plan->x, unit->plan->y)) {
	DMprintf("%s should be guarding area around %d,%d\n",
		 unit_desig(unit), unit->plan->x, unit->plan->y);
    } else if (unit->plan->protectee != 0) {
	DMprintf("%s should be protecting %s\n",
		 unit_desig(unit),
		 unit_desig(find_unit(unit->plan->protectee)));
    } else if (can_fire(unit)) {
	/* No special goal, look for something to shoot at. */
	if (fire_at_opportunity(unit)) {
	    execute_task(unit);
	    return;
	}
    } else if (can_attack(unit)) {
	/* can move a short ways to attack an interloper */
    }
    /* (might be able to defend by interposing self?) */
    if (!unit->plan->reserve) {
	/* Just stay in reserve for now. */
	DMprintf("%s going into defensive reserve\n", unit_desig(unit));
	unit->plan->reserve = TRUE;
	/* should set all alarms? */
    }
}

plan_exploration(unit)
Unit *unit;
{
    Plan *plan = unit->plan;
    int u = unit->type, u2;
    int x, y, w, h, range, x1, y1;
    Side *us = unit->side;

    /* If the world has no secrets, exploration is sort of pointless. */
    if (g_see_all()) {
    	plan->reserve = TRUE;
    	return;
    }
    if (resupply_if_low(unit)) return;
    if (capture_indep_if_nearby(unit)) return;
    if (plan->tasks) {
    	/* (should see if a change of task is worthwhile) */
    	execute_task(unit);
    	return;
    }
    if (plan->maingoal) {
	switch (plan->maingoal->type) {
	  case VICINITY_KNOWN:
	  case VICINITY_HELD:
	    if (mobile(u)) {
		x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
		w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
		if (distance(x, y, unit->x, unit->y) > max(w, h)) {
	    	    if (random_point_near(x, y, w / 2, &x1, &y1)) {
	    		x = x1;  y = y1;
	    	    }
		    DMprintf("%s to explore towards %d,%d\n",
			     unit_desig(unit), x, y);
		    push_movenear_task(unit, x, y, w / 2);
		} else {
		    if (explore_reachable_hex(unit, w + 2)) {
		    } else {
		    	if (flip_coin()) {
			    DMprintf("%s clearing goal\n", unit_desig(unit));
			    plan->maingoal = NULL;
		    	}
			DMprintf("%s to walk randomly\n", unit_desig(unit));
			random_walk(unit);
		    }
		}
	    } else {
		plan_explorer_support(unit);
	    }
	    break;
	  case WORLD_KNOWN:
	    if (mobile(u)) {
		if (explore_reachable_hex(unit, area.maxdim)) {
		} else {
		    DMprintf("%s to walk randomly\n", unit_desig(unit));
		    random_walk(unit);
		}
	    } else {
		plan_explorer_support(unit);
	    }
	    break;
	  default:
	    DMprintf("%s goal %s?\n",
		     unit_desig(unit), goal_desig(unit->plan->maingoal));
	    break;
	}
    } else {
	/* No specific goal, just poke around. */
	if (mobile(u)) {
	    range = area.maxdim / 2;
	    if (explore_reachable_hex(unit, range)) {
	    } else {
	    	if (flip_coin()) {
	    		/* (should call a plan eraser) */
	    		unit->plan->type = PLAN_NONE;
	    	}
		DMprintf("%s to walk randomly\n", unit_desig(unit));
		random_walk(unit);
	    }
	} else {
	    plan_explorer_support(unit);
	}
    }
    if (plan->tasks) {
        execute_task(unit);
    } else {
        if (probability(10)) {
    	    DMprintf("no tasks, going to sleep (dunno why)\n");
    	    plan->asleep = TRUE;
        }
    }
}

plan_explorer_support(unit)
Unit *unit;
{
    int u = unit->type, u2, u3, backup;
    Task *task;

    if (side_has_ai(unit->side)) {
	u3 = mplayer_preferred_build_type(unit->side, unit, PLAN_EXPLORATORY);
    } else {
	for_all_unit_types(u2) {
	    if (mobile(u2)
		&& 1 /* better on more kinds of terrain? */
		&& uu_acp_to_create(u, u2) > 0) {
		backup = u2;
		if (flip_coin()) {
		    u3 = u2;
		    break;
		}
	    }
	}
    }
    if (u3 == NONUTYPE) u3 = backup;
    if (u3 != NONUTYPE) {
	task = unit->plan->tasks;
    	if (task == NULL || task->type != BUILD_TASK) {
	    DMprintf("%s supporting exploration by building %s\n",
		     unit_desig(unit), u_type_name(u3));
	    push_build_task(unit, u3, 2);
	} else {
	    DMprintf("%s already building, leaving alone\n",
		     unit_desig(unit));
	}
    } else {
    	DMprintf("%s has no way to support exploration\n", unit_desig(unit));
    }
}

int victimx, victimy, victimrating;

victim_here(x, y)
int x, y;
{
    int u2 = NONUTYPE, uview, rating, dist;
    Unit *unit2;
    Side *side = tmpunit->side, *oside = NULL;

    if (g_see_all()
	|| cover(side, x, y) > 0 /* should be "guaranteed to see" */) {
	if ((unit2 = unit_at(x, y)) != NULL) {
	    u2 = unit2->type;
	    oside = unit2->side;
	} else {
	    return FALSE;
	}
    } else if (terrain_view(side, x, y) != UNSEEN
	       && (uview = unit_view(side, x, y)) != EMPTY) {
	u2 = vtype(uview);
	oside = side_n(vside(uview));
    } else {
	return FALSE;
    }
    if (is_unit_type(u2)
	&& oside != side
	&& could_hit(tmpunit->type, u2)
	&& uu_damage(tmpunit->type, u2) > 0
	&& (!worth_capturing(side, u2, oside, x, y)
	    || could_capture(tmpunit->type, u2))) {
	rating = uu_bhw(tmpunit->type, u2);
	if (could_capture(tmpunit->type, u2)) {
	    return TRUE;
	}
	if (tmpunit->occupant != NULL
	    && could_hit(u2, tmpunit->type)
	    && uu_damage(u2, tmpunit->type) > 0
	    /* and valuable occupants not protected... */
	    ) {
	    return FALSE;
	}
	/* Further-away units are less interesting than closer ones. */
	dist = distance(tmpunit->x, tmpunit->y, x, y);
	if (dist > u_acp(tmpunit->type)) {
	    rating /= max(1, isqrt(dist - u_acp(tmpunit->type)));
	}
	if (rating > victimrating || (rating == victimrating && flip_coin())) {
	    victimx = x;  victimy = y;
	    victimrating = rating;
	}
    }
    return FALSE;
}

/* This decides whether a given unit type seen at a given location is worth
   trying to capture. */

worth_capturing(side, u2, oside, x, y)
Side *side, *oside;
int u2, x, y;
{
    int u, bestchance = 0;

    /* See how likely we are to be able to capture the type. */
    for_all_unit_types(u) {
	bestchance = max(uu_capture(u, u2), bestchance);
    }
    if (bestchance == 0) return FALSE;
    /* (should account for other considerations too) */
    return TRUE;
}

/* This routine looks for somebody, anybody to attack. */

go_after_victim(unit, range)
Unit *unit;
int range;
{
    int x, y, rslt;

    tmpunit = unit;
    DMprintf("%s seeking victim within %d; found ",
	     unit_desig(unit), range);
    victimrating = -9999;
    rslt = search_around(unit->x, unit->y, range, victim_here, &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
	/* (should be able to set capture task if better) */
	set_hit_task(unit, x, y);
	if (unit->transport != NULL
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    order_movenear(unit->transport, x, y, 1);
	}
    } else if (victimrating > -9999) {
	DMprintf("one (rated %d) at %d,%d\n", victimrating, victimx, victimy);
	/* Set up a task to go after the unit found. */
	/* (should be able to set capture task if better) */
	set_hit_task(unit, victimx, victimy);
	if (unit->transport != NULL
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
		order_movenear(unit->transport, victimx, victimy, 1);
	}
    } else {
    DMprintf("nothing\n");
    }
    return rslt;
}

int targetx, targety, targetrating;

target_here(x, y)
int x, y;
{
    int u2 = NONUTYPE, uview, rating, dist;
    Unit *unit2;
    Side *side = tmpunit->side, *oside = NULL;

    if (g_see_all()
	|| cover(side, x, y) > 0 /* should be "guaranteed to see" */) {
	if ((unit2 = unit_at(x, y)) != NULL) {
	    u2 = unit2->type;
	    oside = unit2->side;
	} else {
	    /* Nothing to shoot at here. */
	    return FALSE;
	}
    } else if (terrain_view(side, x, y) != UNSEEN
	       && (uview = unit_view(side, x, y)) != EMPTY) {
	u2 = vtype(uview);
	oside = side_n(vside(uview));
    } else {
	/* Nothing to shoot at here. */
	return FALSE;
    }
    if (is_unit_type(u2)
	&& oside != side
	&& could_hit(tmpunit->type, u2)
	&& uu_damage(tmpunit->type, u2) > 0
	/* and have correct ammo */
	) {
	rating = uu_hit(tmpunit->type, u2);
	/* Further-away units are less interesting than closer ones. */
	dist = distance(tmpunit->x, tmpunit->y, x, y);
	/* (effect should be quality of fire) */
	if (rating > targetrating || (rating == targetrating && flip_coin())) {
	    targetx = x;  targety = y;
	    targetrating = rating;
	}
    }
    return FALSE;
}

fire_at_opportunity(unit)
Unit *unit;
{
    int x, y, range, rslt;

    tmpunit = unit;
    range = u_range(unit->type);
    targetrating = -9999;
    DMprintf("%s seeking target within %d; found ",
             unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, target_here, &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to shoot at the unit found. */
	set_hit_task(unit, x, y);
    } else if (targetrating > -9999) {
	DMprintf("one (rated %d) at %d,%d\n", targetrating, x, y);
	/* Set up a task to shoot at the unit found. */
	set_hit_task(unit, targetx, targety);
    } else {
	DMprintf("nothing\n");
    }
    return rslt;
}

/* Check to see if our grand plans are at risk of being sideswiped by lack of
   supply, and set up a resupply task if so. */

resupply_if_low(unit)
Unit *unit;
{
    int u = unit->type, m, lowm = NONMTYPE;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u)) return FALSE;
    for_all_material_types(m) {
	if ((um_consume(u, m) > 0 || um_tomove(u, m) > 0)
	    && 2 * unit->supply[m] < um_storage(u, m)) {
	    lowm = m;
	    break;
	}
    }
    if (lowm != NONMTYPE) {
	if (curtask != NULL
	    && curtask->type == MOVETO_TASK
	    && supplies_here(unit, curtask->args[0], curtask->args[1], lowm))
	  /* Let the movement task execute. */
	  return FALSE;
	/* Otherwise set up a task. */
	DMprintf("%s low on %s, looking for source\n",
		 unit_desig(unit), m_type_name(m));
	set_resupply_task(unit);
	return (execute_task(unit) != TASK_FAILED);
    }
    return FALSE;
}

rearm_if_low(unit)
Unit *unit;
{
    int u = unit->type, m, lowm = NONMTYPE;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u)) return FALSE;
    for_all_material_types(m) {
	if (um_hitswith(u, m) > 0
	    && unit->supply[m] == 0
	    && um_storage_x(u, m) > 0) {
	    lowm = m;
	    break;
	}
    }
    if (lowm != NONMTYPE) {
	if (curtask != NULL
	    && curtask->type == MOVETO_TASK
	    && supplies_here(unit, curtask->args[0], curtask->args[1], lowm))
	  /* Let the movement task execute. */
	  return FALSE;
	/* Otherwise set up a task. */
	DMprintf("%s low on %s, looking for source\n",
		 unit_desig(unit), m_type_name(m));
	set_resupply_task(unit);
	return (execute_task(unit) != TASK_FAILED);
    }
    return FALSE;
}

supplies_here(unit, x, y, m)
Unit *unit;
int x, y, m;
{
    Unit *unit2;

    for_all_stack(x, y, unit2) {
	if (unit2->side == unit->side
	    && unit2->supply[m] > 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

indep_captureable_here(x, y)
int x, y;
{
    int u2 = NONUTYPE, uview;
    Unit *unit2;
    Side *side = tmpunit->side, *oside = NULL;

    if (g_see_all()
	|| cover(side, x, y) > 0 /* should be "guaranteed to see" */) {
	if ((unit2 = unit_at(x, y)) != NULL) {
	    u2 = unit2->type;
	    oside = unit2->side;
	} else {
	    return FALSE;
	}
    } else if (terrain_view(side, x, y) != UNSEEN
	       && (uview = unit_view(side, x, y)) != EMPTY) {
	u2 = vtype(uview);
	oside = side_n(vside(uview));
    } else {
	return FALSE;
    }
    return (is_unit_type(u2)
    	    && oside == NULL
	    && could_capture(tmpunit->type, u2));
}

/*  */

capture_indep_if_nearby(unit)
Unit *unit;
{
    int u = unit->type, range = 3;
    int x, y, rslt;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u)) return FALSE;
    if (!could_capture_any(unit->type)) return FALSE;
    tmpunit = unit;
    DMprintf("%s searching for easy capture within %d; found ",
	     unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, indep_captureable_here,
			 &x, &y, 1);
    if (rslt) {
	DMprintf("one at %d,%d\n", x, y);
	/* Set up a task to go after the unit found. */
	set_capture_task(unit, x, y);
	if (unit->transport
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    push_movenear_task(unit->transport, x, y, 1);
	}
	return (execute_task(unit) != TASK_FAILED);
    } else {
	DMprintf("nothing\n");
    }
    return FALSE;
}

could_capture_any(u)
int u;
{
    int u2;

    for_all_unit_types(u2) {
	if (could_capture(u, u2)) return TRUE;
	/* also check if u2 in game, on other side, etc? */
    }
    return FALSE;
}

/* This is a semi-testing routine that basically picks something for the
   unit to do without worrying about its validity.  The rest of the
   system should function correctly no matter what this thing comes up with. */

plan_random(unit)
Unit *unit;
{
    if (flip_coin()) {
	/* Pick a random task. */
	switch (xrandom((int) NUMTASKTYPES)) {
	  case 0:
	  case 1:
	  default:
	    if (1) {
		int dir = random_dir(), x1, y1;

		point_in_dir(unit->x, unit->y, dir, &x1, &y1);
		order_moveto(unit, x1, y1, 0);
	    } else {
		/* (eventually gen non-adj and up/down moves also) */
	    }
	    break;
	}
    } else {
	/* Pick a random action. */
	Action action;
	char *argtypestr;
	int rslt;

	bzero(&action, sizeof(Action));
	action.type = (ActionType) xrandom((int) NUMACTIONTYPES);
	argtypestr = actiondefns[(int) action.type].argtypes;
	make_plausible_random_args(argtypestr, 0, action.args, unit);
	if (flip_coin()) {
	    action.actee = unit->id;
	} else {
	    while (find_unit(action.actee = xrandom(numunits)+1) == NULL
		   && probability(98));
	}
	unit->act->nextaction = action;
	DMprintf("%s will randomly try %s\n",
		 unit_desig(unit), action_desig(&action));
    }
}

/* This attempts to make some vaguely plausible arguments for an action,
   using the types of each arg as a guide.  It also generates *invalid*
   arguments occasionally, which tests error checking in the actions' code. */

make_plausible_random_args(argtypestr, i, args, unit)
char *argtypestr;
short i, args[4];
Unit *unit;
{
    char argch;
    long arg;

    while (i < strlen(argtypestr) && i < 10) {
	argch = argtypestr[i];
	switch (argch) {
	  case 'n':
	    arg = (flip_coin() ? xrandom(10) :
		   (flip_coin() ? xrandom(100) :
		    (xrandom(20000) - 10000)));
	    break;
	  case 'u':
	    /* Go a little outside range, so as to get some invalid types. */
	    arg = xrandom(numutypes + 2) - 1;
	    break;
	  case 'm':
	    arg = xrandom(nummtypes + 2) - 1;
	    break;
	  case 't':
	    arg = xrandom(numttypes + 2) - 1;
	    break;
	  case 'x':
	    arg = (unit != NULL && flip_coin() ? (unit->x + xrandom(5) - 2) :
		   (xrandom(area.width + 4) - 2));
	    break;
	  case 'y':
	    arg = (unit != NULL && flip_coin() ? (unit->y + xrandom(5) - 2) :
		   (xrandom(area.height + 4) - 2));
	    break;
	  case 'z':
	    arg = (flip_coin() ? 0 : xrandom(10));
	    break;
	  case 'd':
	    arg = random_dir();
	    break;
	  case 'U':
	    /* Cast around for a valid unit. */
	    while (find_unit(arg = xrandom(numunits)+1) == NULL
		   && probability(98));
	    break;
	  case 'S':
	    arg = xrandom(numsides + 3) - 1;
	    break;
	  default:
	    run_warning("Garbled action arg type '%c'\n", argch);
	    arg = 0;
	    break;
	}
	args[i++] = arg;
    }
}

/* This routine calculates a basic plan for the given unit. */

decide_plan(side, unit)
Side *side;
Unit *unit;
{
    /* (should be able to make plan object if missing) */
    if (side == NULL) return;
    DMprintf("%s deciding its plan", unit_desig(unit));
    if (side_has_ai(side)) {
	    DMprintf(", using %s AI", side_desig(side));
	    ai_decide_plan(side, unit);
	    DMprintf(" - plan is now %s", plan_desig(unit->plan));
    }
    /* If the AI didn't decide anything, or if this is a human-run
       unit, then just say the unit will be following orders. */
    /* (doctrine should be able to ask for assortment of plans.) */
    if (unit->plan->type == PLAN_NONE) {
	if (side_has_display(side))  {
		DMprintf(", but will just be following orders");
		unit->plan->type = PLAN_PASSIVE;
    		clear_task_agenda(unit->plan);
    		/* Special-case cities in the intro game to automatically
    		   start producing infantry initially. */
    		if (curturn <= 1
		    && mainmodule != NULL
    		    && mainmodule->name != NULL
    		    && strcmp(mainmodule->name, "intro") == 0
    		    && strcmp(u_type_name(unit->type), "city") == 0) {
    		    push_build_task(unit, 0, 99);
    		}
	} else {
		/* or PLAN_NONE? */
		unit->plan->type = PLAN_PASSIVE;
    		clear_task_agenda(unit->plan);
		unit->plan->asleep = TRUE;
	}
    }
    DMprintf("\n");
}

doctrine_allows_wait(unit)
Unit *unit;
{
    int everask = units_doctrine(unit, everaskside);

    return (everask);
}

/* Record the unit as waiting for orders about what to do. */

wait_for_orders(unit)
Unit *unit;
{
    if (!unit->plan->waitingfortasks) {
	++(unit->side->numwaiting);
    }
    unit->plan->waitingfortasks = TRUE;
}

/* This runs if an order-following unit has been told to just pick something
   random to do. */

decide_tasks(unit)
Unit *unit;
{
    random_walk(unit);
}

/* Random walking just attempts to move around. */

random_walk(unit)
Unit *unit;
{
    int dir = random_dir(), x1, y1, tries = 0;

    while (!interior_point_in_dir(unit->x, unit->y, dir, &x1, &y1)) {
    	if (++tries > 500) {  run_warning("something is wrong");  break;  }
    	dir = random_dir();
    }
    set_moveto_task(unit, x1, y1);
}

/* Put a unit in reserve. */

reserve_unit(side, unit)
Side *side;
Unit *unit;
{
    if (unit->plan) {
	unit->plan->reserve = TRUE;
	update_unit_display(side, unit, TRUE);
    }
}

/* General routine to wake a unit up (and maybe all its cargo). */

wake_unit(unit, wakeocc, reason, unit2)
Unit *unit, *unit2;
int wakeocc, reason;
{
    Unit *occ;

    if (unit->plan) {
	unit->plan->asleep = unit->plan->reserve = FALSE;
	/* (echo to interface?) */
    }
    if (wakeocc) {
	for_all_occupants(unit, occ) wake_unit(occ, wakeocc, reason, unit2);
    }
    /* (should do something with other args) */
}

/* The area wakeup. */

int tmpflag;

wake_at(x, y)
int x, y;
{
    Unit *unit;

    for_all_stack(x, y, unit) {
	if (side_controls_unit(tmpside, unit)) {
	    wake_unit(unit, tmpflag, 0, NULL);
	}
    }
}

wake_area(side, x, y, n, occs)
Side *side;
int x, y, n, occs;
{
    tmpside = side;
    tmpflag = occs;
    apply_to_area(x, y, n, wake_at);
}

/* (should be subsumed in general alarm stuff) */
/* True if unit sees that is is adjacent to an unfriendly. */

adj_enemy(unit)
Unit *unit;
{
#if 0
    int d, x, y;
    viewdata view;

    for_all_directions(d) {
	x = wrap(unit->x + dirx[d]);  y = unit->y + diry[d];
	if (!indep(unit)) {
	    view = side_view(unit->side, x, y);
	    if (view != EMPTY && enemy_side(side_n(vside(view)), unit->side))
	      return TRUE;
	}
    }
#endif
    return FALSE;
}

/* Search for a base to move to.  "Extra" is our optimism level, i.e.
   extra distance we might be willing to consider. */

find_base(unit, pred, extra)
Unit *unit;
int (*pred)();
int extra;
{
#if 0
    int range;
    int ox, oy, ux = unit->x, uy = unit->y;
    
    route_max_distance = range = range_left(unit) + extra;
    route_max_distance = min(area.maxdim, range);
    if ((range * range < mside->numunits) ? 
	(search_around(ux, uy, range, pred, &ox, &oy, 1))  :
	(find_closest_unit(ux, uy, range, pred, &ox, &oy))) {
	order_moveto(unit, ox, oy);
/*	if (munit->plan->move_tries < 3)
	  unit->plan->orders.flags |= SHORTESTPATH;
	unit->plan->orders.flags &=
	  ~(ENEMYWAKE|NEUTRALWAKE|SUPPLYWAKE|ATTACKUNIT);
*/
	DMprintf("(found base at %d,%d)", ox, oy);
	return TRUE;
    }
#endif
    return FALSE;
}

/* See if we're in a bad way, either on supply or hits, and get to safety
   if possible.  */

maybe_return_home(unit)
Unit *unit;
{
#if 0
    int u = unit->type;
    
    DMprintf("%s checking return (low %d moves %d); ",
	     unit_desig(unit),
	     low_supplies(unit), moves_till_low_supplies(unit));
    if (0 /* (low_supplies(unit)
	 || munit->plan->move_tries > 6
	 || (moves_till_low_supplies(unit) < min(3, unit->mp) &&
	     unit->plan->wakeup_reason != WAKEENEMY))
	&& !can_capture_neighbor(unit)
	&& probability(100) */) {
	DMprintf("low supply, ");
	if (building(unit) && survival_time(unit) > 1 /* unit->schedule */) {
	    DMprintf("decided to sit and produce\n");
	    order_sentry(unit, 1);
	    return TRUE;
	}
	if (find_base(unit, good_haven_p, 0)) {
	    DMprintf("found a good base\n");
	    return TRUE;
	} else if (find_base(unit, haven_p, 0)) {
	    DMprintf("found a base\n");
	    return TRUE;
	} else if (unit->transport != NULL) {
	    DMprintf("in a transport, will sit\n");
	    order_sentry(unit, 1);
#if 0
	} else if (survive_to_build_base(unit) && unit->transport == NULL) {
	    DMprintf("going to build a base\n");
	    push_build_task(unit, machine_product(unit));
	    order_sentry(unit, 1 /* unit->schedule */+1);
	    return TRUE;
#endif
	} else if (find_base(unit, haven_p, 1)) {
	    DMprintf("found a base\n");
	    return TRUE;
	} else {
	    DMprintf("but can't do anything about it\n");
	}
    }
#if 0
    if ((cripple(unit) && probability(98))
	|| probability(100 - ((100 * unit->hp) / u_hp(u)))) {
	/* note that crippled units cannot repair themselves */
	DMprintf("badly damaged, ");
	if (unit->transport && could_repair(u, unit->transport->type)) {
	    DMprintf("%s will repair\n", unit_desig(unit->transport));
	    order_sentry(unit, 1);
	    return TRUE;
	} else {
	    if (find_base(unit, shop_p, 0)) {
		DMprintf("found a base\n");
		return TRUE;
	    } else {
		DMprintf("but no place to repair\n");
	    }
	}
    }
#endif
    if (out_of_ammo(unit) >= 0 && probability(80)) {
	DMprintf("should reload, ");
	if (find_base(unit, haven_p, 0)) {
	    DMprintf("found a base\n");
	    return TRUE;
	} else {
#if 0
	    if (/* hack */ 0 && survive_to_build_base(unit) && 
		unit->transport == NULL && probability(90)) {
		DMprintf("going to build something\n");
		push_build_task(unit, machine_product(unit));
		order_sentry(unit, 1 /* unit->schedule */+1);
	    } else {
		DMprintf("but can't\n");
	    }
#endif
	}
    }
#endif
    DMprintf("no need to return\n");
    return FALSE;
}

/* Return the distance that we can go by shortest path before running out */
/* of important supplies.  Will return at least 1, since we can *always* */
/* move one hex to safety.  This is a worst-case routine, too complicated */
/* to worry about units getting refreshed by terrain or whatever. */

range_left(unit)
Unit *unit;
{
    int u = unit->type, m, least = 12345; /* bigger than any real value */
    int tmp;
    
    for_all_material_types(m) {
	if (um_tomove(u, m) > 0) {
	    least = min(least, unit->supply[m] / um_tomove(u, m));
	}
#if 0
	if (um_consume(u, m) > 0) {
	    tmp = (u_speed(u) * unit->supply[m]) / um_consume(u, m);
	    least = min(least, tmp);
	}
#endif
    }
    return (least == 12345 ? 1 : least);
}

/* Make up a chain of tasks that define how to get to the (possibly
   unexplored) destination without running any undue hazards. */

plan_exploration_route(unit, x, y)
Unit *unit;
int x,y;
{
    int maxdist = distance(unit->x, unit->y, x, y);
    Plan *plan = unit->plan;
    Task *taskchain;

    /* Nothing to do? */
    if (maxdist == 0) return;
    /* (should limit maxdist by various heuristics) */

    if ((taskchain = (Task *) find_route(unit, maxdist, x, y)) != NULL) {
    clear_task_agenda(plan);
	plan->tasks = taskchain;
	DMprintf("%s will explore using %s\n",
		 unit_desig(unit), plan_desig(plan));
    }
}

/* Try to get to the given destination using the router in preference
   to the order_moveto command */

route_to(unit, x, y)
Unit *unit;
int x,y;
{
#if 0
    int maxdist = moves_till_low_supplies(unit);
    Unit *dest_unit = unit_at(x,y);
    Plan *plan;
    Task *taskchain, *task;

    munit = unit;
    if (dest_unit != NULL && isbase(dest_unit))
      maxdist = range_left(unit);
    if (route_max_distance > maxdist)
      maxdist = route_max_distance;
    maxdist = min(area.maxdim, maxdist);
    if ((taskchain = find_route(munit, maxdist, x, y)) != NULL) {
    	clear_task_agenda(unit->plan);
	unit->plan->tasks = taskchain;
	/* Find the last plan step that we want to use */
	task = unit->plan->tasks;
	while (maxdist-- > 1 && task != NULL)
	  task = task->next;
	if (task != NULL) {
/*	    free_task(task->next);  for now */
	    task->next = NULL;
	}
	DMprintf("%s has tasks going to (%d,%d)\n", unit_desig(unit), x, y);
    } else {
	DMprintf("%s no tasks, moving directly to (%d,%d)\n",
		 unit_desig(unit), x, y);
	order_moveto(unit, x, y);
    }
#endif
}

/* Estimate the goodness and badness of hexes in the immediate vicinity. */

find_worths(range)
int range;
{
}

/* This is a heuristic estimation of the value of one unit type hitting */
/* on another.  Should take cost of production into account as well as the */
/* chance and significance of any effect. */

attack_worth(unit, e)
Unit *unit;
int e;
{
    int u = unit->type, worth;

    worth = uu_bhw(u, e);
    /* Risk of death? */
/*    if (uu_damage(e, u) >= unit->hp)
	worth /= (could_capture(u, e) ? 1 : 4);
    if (could_capture(u, e)) worth *= 4; */
    return worth;
}

/* Support functions. */

/* Return true if the given position is threatened by the given unit type. */

threat(side, u, x0, y0)
Side *side;
int u, x0, y0;
{
    int d, x, y, thr = 0;
    Side *side2;
    int view;

    for_all_directions(d) {
    	point_in_dir(x0, y0, d, &x, &y);
	view = 0 /* side_view(side, x, y) */;
	if (view != UNSEEN && view != EMPTY) {
	    side2 = side_n(vside(view));
	    if (allied_side(side, side2)) {
		if (could_capture(u, vtype(view))) thr += 1000;
		if (uu_bhw(u, vtype(view)) > 0) thr += 100;
	    }
	}
    }
    return thr;
}

/* Heuristics for units to build "bases". */

should_build_base(unit)
Unit *unit;
{
#if 0
    DMprintf(
        "%s should build base survive %d base near %d wakeup %d - ",
		       unit_desig(unit),
		       survive_to_build_base(unit), base_nearby(unit,1),
		       unit->plan->wakeup_reason);
    if ((exact_survive_to_build_base(unit)) && 0 &&
	unit->transport == NULL &&
	!any_base_nearby(unit,1) &&
	probability(90) &&
	unit->plan->wakeup_reason != WAKEENEMY) {
#if 0
	push_build_task(unit, machine_product(unit));
	order_sentry(unit, 1 /* unit->schedule */+1);
	DMprintf("going to build a %d\n", unit->product);
#endif
	return TRUE;
    }
    if ((survive_to_build_base(unit) && 0 &&  probability(20)) &&
	unit->transport == NULL &&
	!any_base_nearby(unit,1) &&
	probability(90) &&
	!neutral_base_nearby(unit, 5) &&
	unit->plan->wakeup_reason != WAKEENEMY) {
#if 0
	push_build_task(unit, machine_product(unit));
	order_sentry(unit, 1 /* unit->schedule */+1);
	DMprintf("going to build a %d\n", unit->product);
#endif
	return TRUE;
    }
    DMprintf("decided not to.\n");
#endif
    return FALSE;
}

#define UNITPRODUCT 0

/* Take into account other units already in this region.  This will
   tend to increase the unit count of regions which already have more
   units than the portion of the land they account for. */

int
region_portion(n, u, units_close, adjterr)
int n, u, units_close[MAXUTYPES], adjterr[MAXUTYPES];
{
    int result;
#if 0
    if (unit_hexes[u] != 0) 
      result = n - (n * adjterr[u]) / (3 * unit_hexes[u]) +
	(n * units_close[u]) / 
	  (3 * max(side_strategy(mside)->unitlistcount[u], 1));
    else 
      result = n + (n * units_close[u]) /
	(3 * max(side_strategy(mside)->unitlistcount[u], 1));
    return result;
#endif
}

pop_task(plan)
Plan *plan;
{
    Task *oldtask;

    if (plan->tasks) {
	oldtask = plan->tasks;
	plan->tasks = plan->tasks->next;
	free_task(oldtask);
    }
}

react_to_enemies(unit)
Unit *unit;
{
    return (unit->plan ? TRUE : FALSE);
}

/* Patrol just does move_to, but cycling waypoints around when the first */
/* one has been reached. */

move_patrol(unit)
Unit *unit;
{
#if 0
    int tx, ty;

    if (unit->plan->orders.rept-- > 0) {
	if (unit->x == unit->plan->orders.p.pt[0].x &&
	    unit->y == unit->plan->orders.p.pt[0].y) {
	    tx = unit->plan->orders.p.pt[0].x;
	    ty = unit->plan->orders.p.pt[0].y;
	    unit->plan->orders.p.pt[0].x = unit->plan->orders.p.pt[1].x;
	    unit->plan->orders.p.pt[0].y = unit->plan->orders.p.pt[1].y;
	    unit->plan->orders.p.pt[1].x = tx;
	    unit->plan->orders.p.pt[1].y = ty;
	}
	return move_to(unit, unit->plan->orders.p.pt[0].x, unit->plan->orders.p.pt[0].y,
		       (unit->plan->orders.flags & SHORTESTPATH));
    }
#endif
    return TRUE;
}

/* Check for units waiting to embark */

check_for_embarkies(unit, top_unit)
Unit *unit, *top_unit;
{
    Unit *occ, *next;
    int timeleft;

    next = top_unit->occupant;
    while (next != NULL) {
	occ = next;
	next = occ->nexthere;
#if 0
	if (occ->plan->orders.type == EMBARK && can_carry(unit, occ)) {
	    timeleft = occ->plan->orders.rept;
	    leave_hex(occ);
	    enter_transport(occ, unit);
/*	      order_sentry(occ, min(timeleft, 5));  */
	}
#endif
    }
}

#if 0
    if (!indep(unit)  /* why this restriction? */
	&& type != NONUTYPE
	&& period_setproduct()
	&& could_make(unit->type, type)
	&& type != unit->product) {
	/* Set this as our new product. */
	unit->product = type;
	unit->next_product = type;
	unit->built = 0;
	/* Update the relevant displays. */
	if (unit->product != NONUTYPE) {
	    unit->side->unitsbuilding[unit->product]--;
/*	    update_unit_type_lists(unit->side, unit->product);  */
	}
	if (type != NONUTYPE) {
	    unit->side->unitsbuilding[type]++;
/*	    update_unit_type_lists(unit->side, type);  */
	}
    }
#endif

/* Basic routine to compute how long a unit will take to build something. */

build_time(unit, prod)
Unit *unit;
int prod;
{
    int schedule = 1 /* uu_make(unit->type, prod) */;
    int u, research_delay;

    /* Add penalty (or unpenalty!) for first unit of a type. */
    /* is "counts" a reliable way to test? */
    if (unit->side->counts[prod] <= 1) {
/*	research_delay = ((schedule * u_research(prod)) / 100);  */
	for_all_unit_types(u) {
	    if (unit->side->counts[u] > 1) {
		research_delay -=
		  (1 /*uu_make(unit->type, u)*/ * 
		   uu_tech_crossover(prod, u)) / 100;
	    }
	    if (research_delay > 0) {
		schedule += research_delay;
	    }
	}
    }
    return schedule;
}

clear_task_agenda(plan)
Plan *plan;
{
    int numcleared = 0;
    Task *oldtask;

    if (plan == NULL || plan->tasks == NULL) return;
    while (plan->tasks != NULL) {
    	oldtask = plan->tasks;
    	plan->tasks = plan->tasks->next;
    	free_task(oldtask);
    	++numcleared;
    }
    if (numcleared > 0) Dprintf("Cleared %d tasks from %s\n", numcleared, plan_desig(plan));
}

void 
free_plan(plan)
Plan *plan;
{
    /* don't do anything yet */
}

/* Describe a plan succinctly. */

char *planbuf = NULL;

char *
plan_desig(plan)
Plan *plan;
{
    Task *task;
    int extra = 0;

    if (planbuf == NULL) planbuf = xmalloc(1000);
    if (plan == NULL) {
	sprintf(planbuf, "no plan");
    } else if (plan->type == PLAN_NONE) {
	sprintf(planbuf, "unformed plan");
    } else {
	if (plan->tasks) {
	    sprintf(tmpbuf, "");
	    for (task = plan->tasks; task != NULL; task = task->next) {
		if (strlen(tmpbuf) < 100) {
		    strcat(tmpbuf, " ");  strcat(tmpbuf, task_desig(task));
		} else {
		    ++extra;
		}
	    }
	    if (extra > 0) {
		sprintf(tmpbuf+strlen(tmpbuf), " ... %d more ...", extra);
	    }
	} else {
	    sprintf(tmpbuf, "no tasks");
	}
	/* Should be able to say more about the plan */
	sprintf(planbuf, "type %s %s",
		plantypenames[plan->type], goal_desig(plan->maingoal));
	if (plan->asleep) strcat(planbuf, " [asleep]");
	if (plan->reserve) strcat(planbuf, " [reserve]");
	if (plan->reserve) strcat(planbuf, " [waiting for tasks]");
	strcat(planbuf, tmpbuf);
    }
    return planbuf;
}

/* True if unit is in immediate danger of being captured. */
/* Needs check on capturer transport being seen. */

might_be_captured(unit)
Unit *unit;
{
    int d, x, y;
    Unit *unit2;

    for_all_directions(d) {
	x = wrap(unit->x + dirx[d]);  y = unit->y + diry[d];
	if (((unit2 = unit_at(x, y)) != NULL) &&
	    (enemy_side(unit->side, unit2->side)) &&
	    (could_capture(unit2->type, unit->type))) return TRUE;
    }
    return FALSE;
}
