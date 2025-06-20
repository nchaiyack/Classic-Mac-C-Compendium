/* Unit plan execution for Xconq.
   Copyright (C) 1991, 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

static void wake_at PROTO ((int x, int y));

/* (should have a generic struct for all plan type attrs) */

char *plantypenames[] = {

#undef  DEF_PLAN
#define DEF_PLAN(NAME,code) NAME,

#include "plan.def"

    NULL
};

/* Execute the plan. */

void
execute_plan(unit, try)
Unit *unit;
int try;
{
    Plan *plan = unit->plan;

    if (!in_play(unit) || !completed(unit)) {
	DMprintf("%s shouldn't be planning yet\n", unit_desig(unit));
	return; 
    }
    DMprintf("%s using plan %s", unit_desig(unit), plan_desig(plan));
    if (try > 1)
      DMprintf(" (try #%d)", try);
    DMprintf("\n");
    /* Units that are asleep or in reserve do nothing. */
    if (plan->asleep || plan->reserve)
      return;
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
	if (plan->supply_is_low && plan->supply_alarm) {
	    plan->supply_alarm = FALSE;
	    if (0 /* auto resupply */) {
		set_resupply_task(unit);
	    } else if (plan->tasks
	    	       && (plan->tasks->type == TASK_RESUPPLY
	    	           || (plan->tasks->type == TASK_MOVETO
	    	               && plan->tasks->next
	    	               && plan->tasks->next->type == TASK_RESUPPLY))) {
		/* do nothing */
	    } else {           
		clear_task_agenda(plan);
		wait_for_orders(unit);
	    }
	}
	if (plan->tasks) {
	    /* (should check that doctrine being followed correctly) */
	    execute_task(unit);
	} else if (plan->formation && move_into_formation(unit)) {
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

/* See if we're too far away from an assigned position, set a task
   to move back if so. */

int
move_into_formation(unit)
Unit *unit;
{
    int nx, ny, dist; 
    Plan *plan = unit->plan;
    Goal *goal;
    Unit *leader;

    leader = plan->funit;
    /* (should doublecheck against leader id) */
    if (leader != NULL) {
	goal = plan->formation;
	nx = leader->x + goal->args[1];  ny = leader->y + goal->args[2];
	dist = goal->args[3];
	if (distance(unit->x, unit->y, nx, ny) > dist) {
	    /* (should perhaps insert after current task?) */
	    set_movenear_task(unit, nx, ny, dist);
	    return TRUE;
	}
    }
    return FALSE;
}

/* A unit operating offensively advances and attacks when possible. */

void
plan_offense(unit)
Unit *unit;
{
    int u = unit->type;
    int x, y, w, h, range, x1, y1;
    Plan *plan = unit->plan;

    if (resupply_if_low(unit))
      return;
    if (rearm_if_low(unit))
      return;
    if (plan->tasks) {
    	execute_task(unit);
    	return;
    }
    if (plan->maingoal && mobile(u)) {
	switch (plan->maingoal->type) {
	  case GOAL_VICINITY_HELD:
	    x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
	    w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
	    if (distance(x, y, unit->x, unit->y) > max(w, h)) {
		/* Outside the goal area - move in towards it. */
	    	if (random_point_near(x, y, w / 2, &x1, &y1)) {
		    x = x1;  y = y1;
	    	}
		DMprintf("%s to go on offensive to %d,%d\n",
			 unit_desig(unit), x, y);
		push_movenear_task(unit, x, y, max(w, h) / 2);
		if (unit->transport
		    && mobile(unit->transport->type)
		    && unit->transport->plan) {
		    push_movenear_task(unit->transport, x, y, max(w, h) / 2);
		}
	    } else {
		range = max(w, h);
		/* No special goal, look for something to fight with. */
		/* Sometimes be willing to look a little farther out. */
		if (probability(50))
		  range *= 2;
		if (do_for_occupants(unit)) {
		} else if (go_after_victim(unit, range)) {
		} else if (probability(20) && self_build_base_for_self(unit)) {
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
	if (probability(50))
	  range = min(range, 2 * u_acp(u));
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

int
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
		&& goal->type == GOAL_VICINITY_HELD
		&& distance(goal->args[0], goal->args[1], unit->x, unit->y)
		> goal->args[2]) {
		set_movenear_task(unit, goal->args[0], goal->args[1],
				  max(goal->args[2] / 2, 1));
		DMprintf("%s will go where occupant %s wants to go (goal %s)\n",
			 unit_desig(unit), unit_desig(occ), goal_desig(goal));
		return TRUE;
	    }
	    /* If the unit does not have a goal, see if it has a task. */
	    for (task = occ->plan->tasks; task != NULL; task = task->next) {
		if ((task->type == TASK_MOVETO
		     || task->type == TASK_HIT_UNIT)
		    && (task->args[0] != unit->x
			|| task->args[1] != unit->y)
			&& distance(task->args[0], task->args[1], unit->x, unit->y) > 1
			) {
		    set_movenear_task(unit, task->args[0], task->args[1], 1);
		    DMprintf("%s will go where occupant %s wants to go (task %s)\n",
			     unit_desig(unit), unit_desig(occ), task_desig(task));
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

int
self_build_base_for_self(unit)
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

void
plan_offense_support(unit)
Unit *unit;
{
    int u = unit->type, u2, u3 = NONUTYPE, backup = NONUTYPE;
    Task *task;

    if (side_has_ai(unit->side)) {
	u3 = ai_preferred_build_type(unit->side, unit, PLAN_OFFENSIVE);
    } else {
	for_all_unit_types(u2) {
	    if (mobile(u2)
		&& (type_can_attack(u2) || type_can_fire(u2))
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
    	if (task == NULL || task->type != TASK_BUILD) {
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

void
set_construction(unit, u, num)
Unit *unit;
int u, num;
{
    Task *task = unit->plan->tasks;

    if (task != NULL && task->type == TASK_BUILD) {
	task->args[0] = u;
	task->args[1] = num;
	task->args[2] = 0;
    } else {
	push_build_task(unit, u, num);
    }
}

/* Defensive units don't go out looking for trouble, but they should
   react strongly to threats. */

void
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
    }
}

void
plan_exploration(unit)
Unit *unit;
{
    Plan *plan = unit->plan;
    int u = unit->type;
    int x, y, w, h, range, x1, y1;
    Side *us = unit->side;

    /* If the world has no secrets, exploration is sort of pointless. */
    if (g_see_all()) {
    	plan->reserve = TRUE;
    	return;
    }
    if (resupply_if_low(unit))
      return;
    if (capture_indep_if_nearby(unit))
      return;
    if (capture_useful_if_nearby(unit))
      return;
    if (plan->tasks) {
    	/* (should see if a change of task is worthwhile) */
    	execute_task(unit);
    	return;
    }
    if (plan->maingoal) {
	switch (plan->maingoal->type) {
	  case GOAL_VICINITY_KNOWN:
	  case GOAL_VICINITY_HELD:
	    if (mobile(u)) {
		x = plan->maingoal->args[0];  y = plan->maingoal->args[1];
		w = plan->maingoal->args[2];  h = plan->maingoal->args[3];
		if (distance(x, y, unit->x, unit->y) > max(w, h)) {
	    	    if (random_point_near(x, y, max(w, h) / 2, &x1, &y1)) {
	    		x = x1;  y = y1;
	    	    }
		    DMprintf("%s to explore towards %d,%d\n",
			     unit_desig(unit), x, y);
		    push_movenear_task(unit, x, y, max(w, h) / 2);
		} else {
		    if (explore_reachable_cell(unit, max(w, h) + 2)) {
		    } else if (us != NULL && side_has_ai(us) && ai_guide_explorer(us, unit)) {
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
	  case GOAL_WORLD_KNOWN:
	    if (mobile(u)) {
		if (explore_reachable_cell(unit, area.maxdim)) {
		} else if (us != NULL && side_has_ai(us)
			   && ai_guide_explorer(us, unit)) {
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
	    if (explore_reachable_cell(unit, range)) {
	    } else if (us != NULL && side_has_ai(us) && ai_guide_explorer(us, unit)) {
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

void
plan_explorer_support(unit)
Unit *unit;
{
    int u = unit->type, u2, u3, backup;
    Task *task;

    if (side_has_ai(unit->side)) {
	u3 = ai_preferred_build_type(unit->side, unit, PLAN_EXPLORATORY);
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
    	if (task == NULL || task->type != TASK_BUILD) {
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

int
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
	&& enemy_side(side, oside)
	&& ((could_hit(tmpunit->type, u2)
	    && uu_damage(tmpunit->type, u2) > 0
	    && (!worth_capturing(side, u2, oside, x, y)
	        || uu_capture(tmpunit->type, u2) > 0))
	    || uu_capture(tmpunit->type, u2) > 0)
	) {
	rating = uu_zz_bhw(tmpunit->type, u2);
	if (capture_chance(tmpunit->type, u2, oside) > 0) {
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

int
worth_capturing(side, u2, side2, x, y)
Side *side, *side2;
int u2, x, y;
{
    int u, bestchance = 0;

    /* See how likely we are to be able to capture the type. */
    for_all_unit_types(u) {
	bestchance = max(capture_chance(u, u2, side2), bestchance);
    }
    if (bestchance == 0)
      return FALSE;
    /* (should account for other considerations too, like which types of units we have) */
    return TRUE;
}

/* This routine looks for somebody, anybody to attack. */

int
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
	    set_movenear_task(unit->transport, x, y, 1);
	}
    } else if (victimrating > -9999) {
	DMprintf("one (rated %d) at %d,%d\n", victimrating, victimx, victimy);
	/* Set up a task to go after the unit found. */
	/* (should be able to set capture task if better) */
	set_hit_task(unit, victimx, victimy);
	if (unit->transport != NULL
	    && mobile(unit->transport->type)
	    && unit->transport->plan) {
	    set_movenear_task(unit->transport, victimx, victimy, 1);
	}
    } else {
	DMprintf("nothing\n");
    }
    return rslt;
}

int targetx, targety, targetrating;

int
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
	/* Nothing (or at least nothing visible) to shoot at here. */
	return FALSE;
    }
    if (is_unit_type(u2)
	&& enemy_side(side, oside)
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

int
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

int
resupply_if_low(unit)
Unit *unit;
{
    int u = unit->type, m, lowm = NONMTYPE;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u)) return FALSE;
    for_all_material_types(m) {
	if ((um_base_consumption(u, m) > 0 || um_consumption_per_move(u, m) > 0)
	    && 2 * unit->supply[m] < um_storage_x(u, m)) {
	    lowm = m;
	    break;
	}
    }
    if (lowm != NONMTYPE) {
	if (curtask != NULL
	    && curtask->type == TASK_MOVETO
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

int
rearm_if_low(unit)
Unit *unit;
{
    int u = unit->type, m, lowm = NONMTYPE;
    Task *curtask = unit->plan->tasks;

    if (!mobile(u)) return FALSE;
    for_all_material_types(m) {
	if (um_consumption_per_attack(u, m) > 0
	    && unit->supply[m] == 0
	    && um_storage_x(u, m) > 0) {
	    lowm = m;
	    break;
	}
    }
    if (lowm != NONMTYPE) {
	if (curtask != NULL
	    && curtask->type == TASK_MOVETO
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

int
supplies_here(unit, x, y, m)
Unit *unit;
int x, y, m;
{
    Unit *unit2;

    for_all_stack(x, y, unit2) {
	if (unit_trusts_unit(unit2, unit)
	    && unit2->supply[m] > 0) {
	    return TRUE;
	}
    }
    return FALSE;
}

int
indep_captureable_here(x, y)
int x, y;
{
    int u2 = NONUTYPE, uview;
    Unit *unit2;
    Side *side = tmpunit->side, *side2 = NULL;

    if (g_see_all()
	|| cover(side, x, y) > 0 /* should be "guaranteed to see" */) {
	if ((unit2 = unit_at(x, y)) != NULL) {
	    u2 = unit2->type;
	    side2 = unit2->side;
	} else {
	    return FALSE;
	}
    } else if (terrain_view(side, x, y) != UNSEEN
	       && (uview = unit_view(side, x, y)) != EMPTY) {
	u2 = vtype(uview);
	side2 = side_n(vside(uview));
    } else {
	return FALSE;
    }
    return (is_unit_type(u2)
    	    && side2 == NULL
	    && capture_chance(tmpunit->type, u2, side2) > 0);
}

/*  */

int
capture_indep_if_nearby(unit)
Unit *unit;
{
    int u = unit->type, range = 5;
    int x, y, rslt;

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

int
useful_captureable_here(x, y)
int x, y;
{
    int u2 = NONUTYPE, uview;
    Unit *unit2;
    Side *side = tmpunit->side, *oside = NULL;

    if (g_see_all()
	|| cover(side, x, y) > 0 /* should be "guaranteed to see" */) {
	for_all_stack(x, y, unit2) {
	    u2 = unit2->type;
	    oside = unit2->side;
            if (is_unit_type(u2)
    	    && !trusted_side(side, oside)
	    && capture_chance(tmpunit->type, u2, oside) > 0
	    && useful_type(side, u2)
	    ) return TRUE;
	}
    } else if (terrain_view(side, x, y) != UNSEEN
	       && (uview = unit_view(side, x, y)) != EMPTY) {
	u2 = vtype(uview);
	oside = side_n(vside(uview));
        return (is_unit_type(u2)
    	    && !trusted_side(side, oside)
	    && capture_chance(tmpunit->type, u2, oside) > 0
	    && useful_type(side, u2)
	    );
    } else {
    }
    return FALSE;
}

/* Return true if the given type of unit is useful in some way to the
   given side.  This is almost always true. */

int
useful_type(side, u)
Side *side;
int u;
{
    /* (should be false for types we can't own, can't operate, etc) */
    return TRUE;
}

/*  */

int
capture_useful_if_nearby(unit)
Unit *unit;
{
    int u = unit->type, range = 2;
    int x, y, rslt;

    if (!mobile(u)) return FALSE;
    if (!could_capture_any(unit->type)) return FALSE;
    tmpunit = unit;
    DMprintf("%s searching for useful capture within %d; found ",
	     unit_desig(unit), range);
    rslt = search_around(unit->x, unit->y, range, useful_captureable_here,
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

int
could_capture_any(u)
int u;
{
    int u2;

    for_all_unit_types(u2) {
	if (uu_capture(u, u2) > 0 || uu_indep_capture(u, u2) > 0) return TRUE;
	/* also check if u2 in game, on other side, etc? */
    }
    return FALSE;
}

/* This is a semi-testing routine that basically picks something for the
   unit to do without worrying about its validity.  The rest of the
   system should function correctly no matter what this thing comes up with.
   Piles of warnings are likely, but that's OK. */

void
plan_random(unit)
Unit *unit;
{
    int dir, x1, y1;
    TaskType tasktype;
    Action action;
    char *argtypestr;

    if (flip_coin()) {
	if (unit->plan->tasks) {
	    execute_task(unit);
	    return;
	}
	/* Pick a random task. */
	tasktype = xrandom((int) NUMTASKTYPES);
	switch (tasktype) {
	  case TASK_NONE:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_BUILD:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_RESEARCH:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_CAPTURE_UNIT:
	    dir = random_dir();
	    point_in_dir(unit->x, unit->y, dir, &x1, &y1);
	    set_capture_task(unit, x1, y1);
	    break;
	  case TASK_DO_ACTION:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_HIT_POSITION:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_HIT_UNIT:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_MOVEDIR:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_MOVETO:
	    dir = random_dir();
	    point_in_dir(unit->x, unit->y, dir, &x1, &y1);
	    order_moveto(unit, x1, y1);
	    break;
	  case TASK_OCCUPY:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_PICKUP:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_REPAIR:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  case TASK_RESUPPLY:
	    set_resupply_task(unit);
	    break;
	  case TASK_SENTRY:
	    unit->plan->tasks = create_task (tasktype);
	    break;
	  default:
	    case_panic("task type", tasktype);
	    break;
	}
    }
    if (unit->plan && unit->plan->tasks)
      return;
    /* Otherwise go for a random action. */
    memset(&action, 0, sizeof(Action));
    action.type = (ActionType) xrandom((int) NUMACTIONTYPES);
    argtypestr = actiondefns[(int) action.type].argtypes;
    make_plausible_random_args(argtypestr, 0, &(action.args[0]), unit);
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

/* This attempts to make some vaguely plausible arguments for an action,
   using the types of each arg as a guide.  It also generates *invalid*
   arguments occasionally, which tests error checking in the actions' code. */

void
make_plausible_random_args(argtypestr, i, args, unit)
char *argtypestr;
int i;
short *args;
Unit *unit;
{
    char argch;
    int	slen;
    long arg;

    slen = strlen(argtypestr);
    while (i < slen && i < 10) {
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

void
decide_plan(side, unit)
Side *side;
Unit *unit;
{
    /* (should be able to make plan object if missing) */
    if (side == NULL)
      return;
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
	    if (g_turn() <= 1
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

int
doctrine_allows_wait(unit)
Unit *unit;
{
    int everask = units_doctrine(unit, everaskside);

    return (everask);
}

/* Record the unit as waiting for orders about what to do. */

void
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

void
decide_tasks(unit)
Unit *unit;
{
    random_walk(unit);
}

/* Random walking just attempts to move around. */

void
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

void
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

void
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

static int tmpflag;

static void
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

void
wake_area(side, x, y, n, occs)
Side *side;
int x, y, n, occs;
{
    tmpside = side;
    tmpflag = occs;
    apply_to_area(x, y, n, wake_at);
}

void
set_formation(unit, leader, x, y, dist, flex)
Unit *unit, *leader;
int x, y, dist, flex;
{
    Plan *plan = unit->plan;
    Goal *goal;

    if (plan == NULL)
      return;
    if (!in_play(unit))
      return;
    if (leader != NULL) {
	if (!in_play(leader))
	  return;
	goal = create_goal(GOAL_KEEP_FORMATION, unit->side, TRUE);
	goal->args[0] = leader->id;
    } else {
	if (!inside_area(x, y)) return;
	goal = create_goal(GOAL_KEEP_FORMATION, unit->side, TRUE);
	goal->args[0] = 0;
    }
    goal->args[1] = x;  goal->args[2] = y;
    goal->args[3] = dist;
    goal->args[4] = flex;
    plan->formation = goal;
    plan->funit = leader;
}

void
delay_unit(unit, flag)
Unit *unit;
int flag;
{
    if (in_play(unit) && unit->plan) {
	unit->plan->delayed = TRUE;
    }
}

/* Search for a base to move to.  "Extra" is our optimism level, i.e.
   extra distance we might be willing to consider. */

int
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

#if 0
/* See if we're in a bad way, either on supply or hits, and get to safety
   if possible.  */

int
maybe_return_home(unit)
Unit *unit;
{
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
	} else if (survive_to_build_base(unit) && unit->transport == NULL) {
	    DMprintf("going to build a base\n");
	    push_build_task(unit, machine_product(unit));
	    order_sentry(unit, 1 /* unit->schedule */+1);
	    return TRUE;
	} else if (find_base(unit, haven_p, 1)) {
	    DMprintf("found a base\n");
	    return TRUE;
	} else {
	    DMprintf("but can't do anything about it\n");
	}
    }
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
    if (out_of_ammo(unit) >= 0 && probability(80)) {
	DMprintf("should reload, ");
	if (find_base(unit, haven_p, 0)) {
	    DMprintf("found a base\n");
	    return TRUE;
	} else {
	    if (/* hack */ 0 && survive_to_build_base(unit) && 
		unit->transport == NULL && probability(90)) {
		DMprintf("going to build something\n");
		push_build_task(unit, machine_product(unit));
		order_sentry(unit, 1 /* unit->schedule */+1);
	    } else {
		DMprintf("but can't\n");
	    }
	}
    }
    DMprintf("no need to return\n");
    return FALSE;
}
#endif

/* Return the distance that we can go by shortest path before running out */
/* of important supplies.  Will return at least 1, since we can *always* */
/* move one cell to safety.  This is a worst-case routine, too complicated */
/* to worry about units getting refreshed by terrain or whatever. */

int
range_left(unit)
Unit *unit;
{
    int u = unit->type, m, least = 12345; /* bigger than any real value */
    
    for_all_material_types(m) {
	if (um_consumption_per_move(u, m) > 0) {
	    least = min(least, unit->supply[m] / um_consumption_per_move(u, m));
	}
#if 0
	if (um_base_consumption(u, m) > 0) {
	    tmp = (u_speed(u) * unit->supply[m]) / um_base_consumption(u, m);
	    least = min(least, tmp);
	}
#endif
    }
    return (least == 12345 ? 1 : least);
}

/* Estimate the goodness and badness of cells in the immediate vicinity. */

int
find_worths(range)
int range;
{
    return 0;
}

/* This is a heuristic estimation of the value of one unit type hitting */
/* on another.  Should take cost of production into account as well as the */
/* chance and significance of any effect. */

int
attack_worth(unit, e)
Unit *unit;
int e;
{
    int u = unit->type, worth;

    worth = uu_zz_bhw(u, e);
    /* Risk of death? */
/*    if (uu_damage(e, u) >= unit->hp)
	worth /= (could_capture(u, e) ? 1 : 4);
    if (could_capture(u, e)) worth *= 4; */
    return worth;
}

/* Support functions. */

/* Return true if the given position is threatened by the given unit type. */

int
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
		if (uu_capture(u, vtype(view)) > 0) thr += 1000;
		if (uu_zz_bhw(u, vtype(view)) > 0) thr += 100;
	    }
	}
    }
    return thr;
}

#if 0
/* Heuristics for units to build "bases". */

int
should_build_base(unit)
Unit *unit;
{
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
	push_build_task(unit, machine_product(unit));
	order_sentry(unit, 1 /* unit->schedule */+1);
	DMprintf("going to build a %d\n", unit->product);
	return TRUE;
    }
    if ((survive_to_build_base(unit) && 0 &&  probability(20)) &&
	unit->transport == NULL &&
	!any_base_nearby(unit,1) &&
	probability(90) &&
	!neutral_base_nearby(unit, 5) &&
	unit->plan->wakeup_reason != WAKEENEMY) {
	push_build_task(unit, machine_product(unit));
	order_sentry(unit, 1 /* unit->schedule */+1);
	DMprintf("going to build a %d\n", unit->product);
	return TRUE;
    }
    DMprintf("decided not to.\n");
    return FALSE;
}
#endif

#define UNITPRODUCT 0

void
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

int
react_to_enemies(unit)
Unit *unit;
{
    return (unit->plan ? TRUE : FALSE);
}

/* Patrol just does move_to, but cycling waypoints around when the first */
/* one has been reached. */

int
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

/* Basic routine to compute how long a unit will take to build something. */

int
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

void
clear_task_agenda(plan)
Plan *plan;
{
    int numcleared = 0;
    Task *oldtask;

    if (plan == NULL || plan->tasks == NULL)
      return;
    while (plan->tasks != NULL) {
    	oldtask = plan->tasks;
    	plan->tasks = plan->tasks->next;
    	free_task(oldtask);
    	++numcleared;
    }
    if (numcleared > 0)
      Dprintf("Cleared %d tasks from %s\n", numcleared, plan_desig(plan));
}

Plan *
create_plan()
{
    Plan *plan = (Plan *) xmalloc(sizeof(Plan));
    return plan;
}

void 
free_plan(plan)
Plan *plan;
{
    if (plan == NULL)
      run_error("no plan here?");
    /* Make tasks available for reallocation. */
    clear_task_agenda(plan);
    free(plan);
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
	    tmpbuf[0] = '\0';
	    for (task = plan->tasks; task != NULL; task = task->next) {
		if (strlen(tmpbuf) < 100) {
		    strcat(tmpbuf, " ");
		    strcat(tmpbuf, task_desig(task));
		} else {
		    ++extra;
		}
	    }
	    if (extra > 0) {
		tprintf(tmpbuf, " ... %d more ...", extra);
	    }
	} else {
	    sprintf(tmpbuf, "no tasks");
	}
	sprintf(planbuf, "type %s %s",
		plantypenames[plan->type], goal_desig(plan->maingoal));
	if (plan->asleep)
	  strcat(planbuf, " [asleep]");
	if (plan->reserve)
	  strcat(planbuf, " [reserve]");
	if (plan->delayed)
	  strcat(planbuf, " [delayed]");
	if (plan->waitingfortasks)
	  strcat(planbuf, " [waiting for tasks]");
	if (plan->supply_alarm)
	  strcat(planbuf, " [supply alarm]");
	if (plan->supply_is_low)
	  strcat(planbuf, " [supply is low]");
	strcat(planbuf, tmpbuf);
    }
    return planbuf;
}

/* True if unit is in immediate danger of being captured. */
/* Needs check on capturer transport being seen. */

int
might_be_captured(unit)
Unit *unit;
{
    int d, x, y;
    Unit *unit2;

    for_all_directions(d) {
      if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
	if (((unit2 = unit_at(x, y)) != NULL) &&
	    (enemy_side(unit->side, unit2->side)) &&
	    (uu_capture(unit2->type, unit->type) > 0)) return TRUE;
      }
    }
    return FALSE;
}

void
force_global_replan(side)
Side *side;
{
    Unit *unit;
    
    for_all_side_units(side, unit) {
	if (in_play(unit) && unit->plan != NULL) {
	    unit->plan->type = PLAN_NONE;
	    clear_task_agenda(unit->plan);
	    unit->plan->asleep = FALSE;
	    unit->plan->reserve = FALSE;
	    unit->plan->delayed = FALSE;
	}
    }
}

/* Auxiliary functions for unit planning in Xconq. */

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

#if 0
/* Can this unit build a base without dying. */

int
survive_to_build_base(unit)
Unit *unit;
{
  return (base_builder(unit) &&
	  survival_time(unit) > build_time(unit, base_builder(unit)));
}
#endif

#if 0
/* Is this the last chance for a unit to build a base without dying. */

int
exact_survive_to_build_base(unit)
Unit *unit;
{
  return (base_builder(unit) &&
	  survival_time(unit) == (1 + build_time(unit, base_builder(unit))));
}
#endif

#if 0
/* Is there a machine base here. */

int
base_here(x, y)
int x, y;
{
    Unit *unit = unit_at(x, y);

    return (0 /* unit != NULL && unit->side == mside && isbase(unit) */) ;
}
#endif

#if 0
/* Is there anybodies base here. */

int
any_base_here(x, y)
int x, y;
{
/*    int utype = vtype(side_view(mside, x, y)); */

    return (0 /* utype != EMPTY && utype != UNSEEN && u_is_base(utype) */);
}
#endif

#if 0
/* Is there anybodies base here. */

int
neutral_base_here(x, y)
int x, y;
{
    int view = side_view(mside, x, y);
    int utype = vtype(view);

    return (utype != EMPTY && utype != UNSEEN &&
	    u_is_base(utype) && vside_indep(view));
    return FALSE;
}
#endif

#if 0
/* Is there a base within the given range.  Generally range is small. */

int
base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, base_here, &x, &y, 1);
}
#endif

/* Is there a base within the given range.  Generally range is small. */

#if 0
int
any_base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, any_base_here, &x, &y, 1);
}
#endif

/* Is there a neutral base within the given range.  Generally range is
   small. */

#if 0
int
neutral_base_nearby(unit,range)
Unit *unit;
int range;
{
    int x,y;
  
    return search_around(unit->x, unit->y, range, neutral_base_here, &x, &y, 1);
}
#endif

int
occupant_could_capture(unit, u2)
Unit *unit;
int u2;
{
    Unit *occ;

    for_all_occupants(unit, occ)
      if (uu_capture(occ->type, u2) > 0)
	return TRUE;
    return FALSE;
}

/* Check to see if there is anyone around to capture. */

int
can_capture_neighbor(unit)
Unit *unit;
{
    int d, x, y;
    int view;
    Side *side2;

    for_all_directions(d) {
      if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
	view = unit_view(unit->side, x, y);
	if (view != UNSEEN && view != EMPTY) {
	    side2 = side_n(vside(view));
	    if (!allied_side(unit->side, side2)) {
		if (uu_capture(unit->type, vtype(view)) > 0) {
		    /* need some other way to change move order quickly */
		    return TRUE;
		}
	    }
	}
      }
    }
    return FALSE;
}

/* check if our first occupant can capture something.  Doesn't look at
   other occupants. */

int
occupant_can_capture_neighbor(unit)
Unit *unit;
{
    Unit *occ = unit->occupant;

    if (occ != NULL && occ->act && occ->act->acp > 0 && occ->side == unit->side) {
	if (can_capture_neighbor(occ)) {
	    return TRUE;
	}
    }
    return FALSE;
}

/* Find the closes unit, first prefering bases, and then transports. */

int
find_closest_unit(side, x0, y0, maxdist, pred, rxp, ryp)
Side *side;
int x0, y0, maxdist, (*pred)(), *rxp, *ryp;
{
#if 0    
    Unit *unit;
    int u, dist;
    int found = FALSE;

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

/* True if the given unit is a sort that can build other units. */

int
can_build(unit)
Unit *unit;
{
    int p;

    for_all_unit_types(p) {
	if (could_create(unit->type, p)) return TRUE;
    }
    return FALSE;
}

/* Test if unit can move out into adjacent cells. */

int
can_move(unit)
Unit *unit;
{
    int d, x, y;

    for_all_directions(d) {
      if (interior_point_in_dir(unit->x, unit->y, d, &x, &y)) {
        /* (should account for world-leaving options?) */
	if (could_move(unit->type, terrain_at(x, y))) return TRUE;
      }
    }
    return FALSE;
}

/* Returns the type of missing supplies. Not great routine if first */
/* material is a type of ammo. */

int
out_of_ammo(unit)
Unit *unit;
{
    int u = unit->type, r;

    for_all_material_types(r) {
	if (um_consumption_per_attack(u, r) > 0 && unit->supply[r] <= 0)
	    return r;
    }
    return (-1);
}

#if 0
/* Someplace that we can definitely get supplies at. */

int
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
		if (unit->supply[r] < um_storage_x(unit->type, r)) {
		    return FALSE;
		}
	    }
	}
    } 
    return FALSE;
}
#endif

#if 0
/* See if the location has a unit that can take us in for refueling */
/* (where's the check for refueling ability?) */

/* Is doing the side-effect a good idea here? */

int
haven_p(unit, x, y)
Unit *unit;
int x, y;
{
    Unit *unit2 = unit_at(x, y);
    Task *route;

      return FALSE;
}
#endif

#if 0
/* See if the location has a unit that can repair us */

int
shop_p(unit, x, y)
Unit *unit;
int x, y;
{
    Unit *unit2 = unit_at(x, y);
    Task *route;

      return FALSE;
}
#endif

/* Check how long a unit can sit where it is */

int
survival_time(unit)
Unit *unit;
{
    int u = unit->type, m, least = 99999, rate, tmp;
    int t = terrain_at(unit->x, unit->y);

    for_all_material_types(m) {
	rate = (um_base_consumption(u, m)
		- (um_base_production(u, m) * ut_productivity(u, t)) / 100);
	if (rate > 0) {
	    tmp = unit->supply[m];
	    if (unit->act) {
		tmp += unit->act->actualmoves * um_consumption_per_move(u, m);
	    }
	    least = min(least, tmp / rate);
	}
    }
    return least;
}

long
regions_around(u, x, y, center)
int u, x, y;
int center;
{
}

int
usable_cell(unit, x, y)
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

Task *explorechain;

int
explorable_cell(x, y)
int x, y;
{
    return (terrain_view(tmpside, x, y) == UNSEEN);
}

int
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

int
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

/* Go to the nearest cell that we can see how to get to. */

int
explore_reachable_cell(unit, range)
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

/* Check for any makers this unit should be capturing. */

int
should_capture_maker(unit)
Unit *unit;
{
    return 0;
}

/* Returns true if the given unit can't leave its cell for some reason. */

int
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

int
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

int
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

int
normal_completion_time(u, u2)
int u, u2;
{
    if (u_acp(u) == 0 || uu_cp_per_build(u, u2) == 0)
      return (-1);
    return (u_cp(u2) - uu_creation_cp(u, u2)) /
      (uu_cp_per_build(u, u2) * u_acp(u));
}


/* True if anybody at all is on any adjacent cell. */

int
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

/* A unit runs low on supplies at the halfway point.  Formula is the same
   no matter how/if occupants eat transports' supplies. */

int
past_halfway_point(unit)
Unit *unit;
{
    int u = unit->type, m;

    for_all_material_types(m) {
	if (((um_base_consumption(u, m) > 0) || (um_consumption_per_move(u, m) > 0)) &&
	    /* should check that the transport is adequate for */
	    /* supplying the fuel */ 
	    (unit->transport == NULL)) {
	    if (2 * unit->supply[m] <= um_storage_x(u, m)) return TRUE;
	}
    }
    return FALSE;
}


/* This is the maximum distance from "home" that a unit can expect to get,
   travelling on its most hostile terrain type. */

int
operating_range_worst(u)
int u;
{
    int m, t, prod, range, worstrange = area.maxdim;

    for_all_material_types(m) {
    	if (um_base_consumption(u, m) > 0) {
	    for_all_terrain_types(t) {
	    	if (!terrain_always_impassable(u, t)) {
	    	    prod = (um_base_production(u, m) * ut_productivity(u, t)) / 100;
	    	    if (prod < um_base_consumption(u, m)) {
			range = um_storage_x(u, m) / (um_base_consumption(u, m) - prod);
			if (range < worstrange) worstrange = range;
		    }
		}
	    }
	}
    }
    return worstrange;
}

/* Same, but for best terrain. */

int
operating_range_best(u)
int u;
{
    int m, t, prod, range, tbestrange, tbest = 0, bestrange = 0;

    for_all_terrain_types(t) {
	if (!terrain_always_impassable(u, t)) {
	    tbestrange = area.maxdim;
	    for_all_material_types(m) {
		if (um_base_consumption(u, m) > 0) {
	    	    prod = (um_base_production(u, m) * ut_productivity(u, t)) / 100;
	    	    if (prod < um_base_consumption(u, m)) {
			range = um_storage_x(u, m) / (um_base_consumption(u, m) - prod);
			if (range < tbestrange) tbestrange = range;
		    }
		}
	    }
	    if (tbestrange > bestrange) {
		bestrange = tbestrange;
		tbest = t;
	    }
	}
    }
    return bestrange;
}

int
terrain_always_impassable(u, t)
int u, t;
{
    if (ut_vanishes_on(u, t))
      return TRUE;
    if (ut_wrecks_on(u, t))
      return TRUE;
    if (ut_mp_to_enter(u, t) > u_acp(u))
      return TRUE;
    return FALSE;
}
