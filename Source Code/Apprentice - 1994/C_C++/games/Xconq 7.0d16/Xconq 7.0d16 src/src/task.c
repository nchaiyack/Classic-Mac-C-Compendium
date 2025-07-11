/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Unit task handling. */

#include "conq.h"

#ifndef INITMAXTASKS
#define INITMAXTASKS 400
#endif

#define can_see_actual_units(side, x, y) (g_see_all() || cover((side), (x), (y)) > 0)

#define push_task(unit, task)  \
    task->next = unit->plan->tasks;  \
    unit->plan->tasks = task;

TaskOutcome execute_task_aux();

/* A list of available task objects. */

Task *freetasks = NULL;

char *tasktypenames[] = {

#undef  DEF_TASK
#define DEF_TASK(NAME,code,argtypes,fn) NAME,

#include "task.def"

    NULL
};

/* Array of descriptions of task types. */

TaskDefn taskdefns[] = {

#undef  DEF_TASK
#define DEF_TASK(NAME,code,ARGTYPES,fn) { NAME, ARGTYPES },

#include "task.def"

    { NULL, NULL }
};

/* Pointer to a buffer that task debug info goes into. */

char *taskbuf = NULL;

do_none_task(unit, task)
Unit *unit;
Task *task;
{
    return TASK_COMPLETE;
}

do_approach_task(unit, task, uid2, dist)
Unit *unit;
Task *task;
int uid2, dist;
{
    return TASK_FAILED;
}

do_build_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type, dist, dx, dy, dir, tx, ty, nx, ny;
    int x = unit->x, y = unit->y;
    int u2 = task->args[0], run = task->args[3];
    Unit *unit2 = NULL, *occ;
    Side *us = unit->side;

    /* First see if we've already built all the units requested. */
    if (task->args[2] >= run) {
        return TASK_COMPLETE;
    }
    /* See if our technology needs improvement in order to build this type. */
    if (is_unit_type(u2)
	&& u_tech_to_build(u2) > 0
        && us->tech[u2] < u_tech_to_build(u2)) {
        if (uu_acp_to_research(u, u2) > 0) {
	    if (valid(check_research_action(unit, unit, u2))) {
		prep_research_action(unit, unit, u2);
		return TASK_PREPPED_ACTION;
	    } else {
		/* We get three trys to research before giving up. */
	    	return (task->execnum < 3 ? TASK_INCOMPLETE : TASK_FAILED);
	    }
        } else {
	    /* Can't do the necessary research. */
	    return TASK_FAILED;
        }
    }
    if (0 /* need to tool up */) {
    }
    /* Check out the unit supposedly in progress. */
    if (task->args[1] != 0) {
	unit2 = find_unit(task->args[1]);
	if (in_play(unit2) && unit2->type == u2) {
	    if (fullsized(unit2)) {
		++(task->args[2]);
		if (task->args[2] >= run) {
		    return TASK_COMPLETE;
		}
		unit2 = NULL;
	    } else {
		/* press on */
	    }
	} else {
	    unit2 = NULL;
	}
    }
    if (unit2 == NULL) {
	/* Search for any appropriate incomplete units nearby. */
	for_all_occupants(unit, occ) {
	    if (in_play(occ)
		&& !fullsized(occ)
		&& occ->type == u2) {
		unit2 = occ;
		break;
	    }
	}
    }
    if (unit2 == NULL) {
    	for_all_stack(x, y, occ) {
	    if (in_play(occ)
		&& !fullsized(occ)
		&& occ->type == u2) {
		unit2 = occ;
		break;
	    }
    	}
    }
    /* (should search within separation radius for matching unit type) */
    if (unit2 == NULL) {
	if (valid(check_create_in_action(unit, unit, u2, unit))) {
	    prep_create_in_action(unit, unit, u2, unit);
	    return TASK_PREPPED_ACTION;
	} else if (valid(check_create_at_action(unit, unit, u2, x, y, 0))) {
	    prep_create_at_action(unit, unit, u2, x, y, 0);
	    return TASK_PREPPED_ACTION;
	} else {
	    /* Try creating in an adjacent cell. */
	    for_all_directions(dir) {
		if (interior_point_in_dir(x, y, dir, &nx, &ny)
		    && valid(check_create_at_action(unit, unit, u2, nx, ny, 0))) {
		    prep_create_at_action(unit, unit, u2, nx, ny, 0);
		    return TASK_PREPPED_ACTION;
		}
	    }
	    return TASK_FAILED;
	}
    } else {
	/* Record the unit's id for use the next time around. */
	task->args[1] = unit2->id;
    }
    /* We have a unit to push towards completion. */
    if (valid(check_build_action(unit, unit, unit2))) {
	prep_build_action(unit, unit, unit2);
	return TASK_PREPPED_ACTION;
    } else {
	return TASK_FAILED;
    }
}

do_research_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type;
    int u2 = task->args[0], lev = task->args[1];
    Side *us = unit->side;

    /* Independents can never ever do research. */
    if (us == NULL) return TASK_FAILED;
    if (us->tech[u2] > u_tech_max(u2)) return TASK_FAILED; /* actually an error */
    if (us->tech[u2] >= lev) return TASK_COMPLETE;
    if (uu_acp_to_research(u, u2) <= 0) return TASK_FAILED;
    if (valid(check_research_action(unit, unit, u2))) {
	prep_research_action(unit, unit, u2);
	return TASK_PREPPED_ACTION;
    } else {
	/* We get three tries to research before giving up. */
	return (task->execnum < 3 ? TASK_INCOMPLETE : TASK_FAILED);
    }
}

do_capture_unit_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type, tx, ty, dist, movedist;
    Unit *unit2;
    Side *us = unit->side;

    /* This is to capture a given type/side of unit at a given place. */
    /* (need to be able to say how hard to try) */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    switch (dist) {
      case 0:
	/* huh? */
	return TASK_FAILED;
      case 1:
	if ((unit2 = unit_at(tx, ty)) != NULL) {
	    if (unit2->side != us /* should be "not a friendly side" */) {
		if (valid(check_capture_action(unit, unit, unit2))) {
		    prep_capture_action(unit, unit, unit2);
		    return TASK_PREPPED_ACTION;
		} else if (valid(check_attack_action(unit, unit, unit2, 100))) {
		    prep_attack_action(unit, unit, unit2, 100);
		    return TASK_PREPPED_ACTION;
		} else {
		    /* We get several tries to capture before giving up. */
		    unit->plan->reserve = TRUE;
		    return (task->execnum < 5 ? TASK_INCOMPLETE : TASK_FAILED);
		}
	    } else {
		/* Our victim apparently saw the light already. */
		return TASK_COMPLETE;
	    }
	} else {
	    /* Odd, nothing was here to capture. */
	    return TASK_FAILED;
	}
      default:
	    /* If on mobile transport, let it handle things. */
	    if (unit->transport != NULL
	        && mobile(unit->transport->type)
		/* and the transport is not blocked */
	        && flip_coin()) {
	        return TASK_INCOMPLETE;
	    }
	    /* If out of range and can move, push a task to get closer (maybe). */
	    if (mobile(u) && flip_coin()) {
		movedist = max(1 /* attack range */, u_range(u));
		if (dist > movedist + u_acp(u) /* or dist that could be covered in 1-2 turns */) {
		    movedist = dist - max(1, (dist - movedist) / 4);
			/* We're too far away to capture directly, add a moveto task. */
			push_movenear_task(unit, tx, ty, movedist);
			return TASK_INCOMPLETE;
		}
	    }
	    return TASK_FAILED;
    } 
}

/* This task just attempts to do the explicitly specified action repeatedly. */

do_action_task(unit, task)
Unit *unit;
Task *task;
{
    int i;

    if (task->args[0] > 0) {
	--(task->args[0]);
    	if (unit->act == NULL) return TASK_FAILED;
    	unit->act->nextaction.type = task->args[1];
    	for (i = 0; i < MAXACTIONARGS; ++i) {
	    unit->act->nextaction.args[i] = task->args[i + 2];
    	}
    	/* act on self always? */
    	unit->act->nextaction.actee = unit->id;
	return TASK_PREPPED_ACTION;
    } else {
	return TASK_COMPLETE;
    }
    return TASK_FAILED;
}

do_ask_side_task(unit, task)
Unit *unit;
Task *task;
{
    if (task->args[0] > 0) {
	--(task->args[0]);
	/* (how should this work exactly?) */
	return TASK_INCOMPLETE;
    } else {
	return TASK_COMPLETE;
    }
    return TASK_FAILED;
}

do_hit_position_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type, tx, ty, dist;

    /* This is to hit a given place. */
    /* (ask for a number of hits?) */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    if (valid(check_fire_into_action(unit, unit, tx, ty, 0, -1))) {
	prep_fire_into_action(unit, unit, tx, ty, 0, -1);
	return TASK_PREPPED_ACTION;
    } else if (mobile(u) && flip_coin()) {
	/* We're too far away to shoot directly, add a moveto task. */
	push_movenear_task(unit, tx, ty, max(1 /* attack range */, u_range(u)));
	return TASK_INCOMPLETE;
    }
    return TASK_FAILED;
}

do_hit_unit_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type, tx, ty, dist, movedist, enemythere, uview, u2, s2;
    Unit *unit2;
    Side *us = unit->side;

    /* This is to hit a (should be - given type/side of) unit at a given place. */
    /* (should add spec for number of hits to attempt?) */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    if (dist <= 1 /* direct attack range */) {
	for_all_stack(tx, ty, unit2) {
	    if (unit2->side != us) {
		if (valid(check_attack_action(unit, unit, unit2, 100))) {
		    prep_attack_action(unit, unit, unit2, 100);
		    return TASK_PREPPED_ACTION;
		}
	    }
	}
	return TASK_FAILED;
    }
    if (dist < u_range_min(u)) {
	/* should move further away */
	return TASK_FAILED;
    }
    if (dist < u_range(u)) {
    	/* Presumes target cell can be examined? */
	for_all_stack(tx, ty, unit2) {
	    if (unit2->side != us) {
		if (valid(check_fire_at_action(unit, unit, unit2, -1))
		    && fire_can_damage(unit, unit2)) {
		    prep_fire_at_action(unit, unit, unit2, -1);
		    return TASK_PREPPED_ACTION;
		}
	    }
	}
	return TASK_FAILED;	    
    }
    if (can_see_actual_units(us, tx, ty)) {
	enemythere = FALSE;
	for_all_stack(tx, ty, unit2) {
	    if (unit2->side != us) {
	    	enemythere = TRUE;
	    	break;
	    }
	}
	if (!enemythere) {
	    if (task->args[2] != NONUTYPE) {
		/* (should search area for unit of desired type) */
	    } else {
		return TASK_COMPLETE;
	    }
	}
    } else {
	/* Have to assess old image or non-image. */
	uview = unit_view(us, tx, ty);
	if (uview != EMPTY) {
	    if (task->args[2] == NONUTYPE) {
	    	/* Just keep going. */
	    }
	} else {
	    /* Not clear if disappearance of target means success or failure,
	       but go with failure. */
	    return TASK_FAILED;
	}
    }
    /* If on mobile transport, let it handle things. */
    if (unit->transport != NULL
        && mobile(unit->transport->type)
	/* and the transport is not blocked */
        && flip_coin()) {
        return TASK_INCOMPLETE;
    }
    /* If out of range and can move, push a task to get closer (maybe). */
    if (mobile(u) && flip_coin()) {
	movedist = max(1 /* attack range */, u_range(u));
	if (dist > movedist + u_acp(u) /* or dist that could be covered in 1-2 turns */) {
	    movedist = dist - max(1, (dist - movedist) / 4);
	}
	push_movenear_task(unit, tx, ty, movedist);
	return TASK_INCOMPLETE;
    }
    return TASK_FAILED;
}

/* Return true if the unit can actually damage the other unit. */

fire_can_damage(unit, unit2)
Unit *unit, *unit2;
{
    if (!alive(unit) || !alive(unit2)) return FALSE;
    /* (should check for right kind of ammo) */
    if (uu_hit(unit->type, unit2->type) <= 0) return FALSE;
    /* (this is dubious - a no-damage attack could still consume acp) */
    if (uu_damage(unit->type, unit2->type) <= 0) return FALSE;
    return TRUE;
}

do_movedir_task(unit, task)
Unit *unit;
Task *task;
{
    int dir, tx, ty;
    Unit *unit2;

    if ((task->args[1])-- > 0) {
	dir = task->args[0];
	tx = wrapx(unit->x + dirx[dir]);  ty = unit->y + diry[dir];
	if (unit_at(tx, ty)) {
	    for_all_stack(tx, ty, unit2) {
	    }
	    return TASK_FAILED;
	} else if (valid(check_move_action(unit, unit, tx, ty, 0))) {
	    prep_move_action(unit, unit, tx, ty, 0);
	    return TASK_PREPPED_ACTION;
	} else {
	    return TASK_FAILED;
	}
    } else {
	return TASK_COMPLETE;
    }
}

int plausible_move_dir();
int sort_directions();

enum choicestate {
	eitherway,
	leftthenright,
	rightthenleft,
	leftonly,
	rightonly
};

do_moveto_task(unit, task)
Unit *unit;
Task *task;
{
    int u = unit->type, dist, dx, dy, dir, tx, ty, nx, ny;
    int dirs[NUMDIRS], numdirs, i, numdirs2;
    Unit *unit2, *occ;
    Side *us = unit->side;

    /* This task is to get to a designated location somehow. */
    tx = task->args[0];  ty = task->args[1];
    dist = distance(tx, ty, unit->x, unit->y);
    if (dist <= task->args[2]) {
	return TASK_COMPLETE;
    }
    switch (dist) {
      case 0:
	/* We're there already, nothing more to do. */
	return TASK_COMPLETE;
      case 1:
	/* Adjacent hex, do a single move. */
	/* (similar to multi-move case, merge code?) */
	if (unit_at(tx, ty)) {
	    for_all_stack(tx, ty, unit2) {
		if (can_occupy(unit, unit2)) {
		    if (valid(check_enter_action(unit, unit, unit2))) {
			prep_enter_action(unit, unit, unit2);
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
		} else if (unit2->side != unit->side) {
		    if (valid(check_attack_action(unit, unit, unit2, 100))) {
			prep_attack_action(unit, unit, unit2, 100);
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
		}
	    }
	    return TASK_FAILED;
	}
	if (valid(check_move_action(unit, unit, tx, ty, unit->z))) {
	    /* Moving into an empty hex. */
	    prep_move_action(unit, unit, tx, ty, unit->z);
	    return TASK_PREPPED_ACTION;
	} else {
	    return TASK_FAILED;
	}
	break;
      default:
	/* Still some distance away, pick a way to go. */
	    /* If on mobile transport, let it handle things. */
	    if (unit->transport != NULL
	        && mobile(unit->transport->type)
	        /* and the transport is not blocked */
	        && flip_coin()) {
	        unit->plan->reserve = TRUE;
	        return TASK_INCOMPLETE;
	    }
	numdirs = choose_move_dirs(unit, tx, ty, TRUE,
				   plausible_move_dir, sort_directions, dirs);
	for (i = 0; i < numdirs; ++i) {
	    point_in_dir(unit->x, unit->y, dirs[i], &nx, &ny);
	    for_all_stack(nx, ny, unit2) {
		if (can_occupy(unit, unit2)) {
		    if (valid(check_enter_action(unit, unit, unit2))) {
			prep_enter_action(unit, unit, unit2);
			/* We (probably) made forward progress, so reopen choice of dirs. */
			task->args[3] = eitherway;
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
		} else if (unit2->side != unit->side) {
		    if (unit->occupant) {
		    	/* More important to find a way through. */
		    	continue;
		    } else {
		    	/* This will encourage some re-evaluation. */
		    	return TASK_FAILED;
		    }
#if 0 /* the following is rarely a good idea */
		    if (valid(check_attack_action(unit, unit, unit2, 100))) {
			prep_attack_action(unit, unit, unit2, 100);
			/* We (probably) made forward progress, so reopen choice of dirs. */
			task->args[3] = eitherway;
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
#endif
		}
	    }
	    if (valid(check_move_action(unit, unit, nx, ny, unit->z))) {
		prep_move_action(unit, unit, nx, ny, unit->z);
		/* We (probably) made forward progress, so reopen choice of dirs. */
		task->args[3] = eitherway;
		return TASK_PREPPED_ACTION;
	    }
	}
	/* Get both right and left non-decreasing dirs. */
	numdirs  = choose_move_dirs(unit, tx, ty, TRUE, NULL, NULL, dirs);
	numdirs2 = choose_move_dirs(unit, tx, ty, FALSE, NULL, NULL, dirs);
	for (i = numdirs; i < numdirs2; ++i) {
	  if (plausible_move_dir(unit, dirs[i])) {
	    switch (task->args[3]) {
	      case eitherway:
		if (i == numdirs) task->args[3] = leftonly /* leftthenright */;
	    	if (i == numdirs+1) task->args[3] = rightonly /* rightthenleft */;
		break;
	      case leftthenright:
		if (i == numdirs) task->args[3] = rightonly;
	    	if (i == numdirs+1) task->args[3] = rightonly;
	    	continue;
		break;
	      case rightthenleft:
	    	if (i == numdirs+1) task->args[3] = leftonly;
	    	continue;
		break;
	      case leftonly:
	    	if (i == numdirs+1) continue;
		break;
	      case rightonly:
	    	if (i == numdirs) continue;
		break;
	    }
	  } else {
	    switch (task->args[3]) {
	      case eitherway:
		    if (i == numdirs) task->args[3] = rightonly;
	    	    if (i == numdirs+1) task->args[3] = leftonly;
	    	    continue;
		break;
	      case leftthenright:
		    if (i == numdirs) task->args[3] = rightonly;
	    	    if (i == numdirs+1) task->args[3] = rightonly;
	    	    continue;
		break;
	      case rightthenleft:
	    	    if (i == numdirs+1) task->args[3] = leftonly;
	    	    continue;
		break;
	      case leftonly:
	    	if (i == numdirs) return TASK_FAILED;
	    	if (i == numdirs+1) continue;
		break;
	      case rightonly:
	    	if (i == numdirs) continue;
	    	if (i == numdirs+1) return TASK_FAILED;
		break;
	    }
	  }
	    point_in_dir(unit->x, unit->y, dirs[i], &nx, &ny);
	    for_all_stack(nx, ny, unit2) {
		if (can_occupy(unit, unit2)) {
		    if (valid(check_enter_action(unit, unit, unit2))) {
			prep_enter_action(unit, unit, unit2);
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
		} else if (unit2->side != unit->side) {
		    if (unit->occupant) {
		    	/* More important to find a way through. */
		    	continue;
		    } else {
		    	/* This will encourage some re-evaluation. */
		    	return TASK_FAILED;
		    }
#if 0 /* the following is rarely a good idea */
		    if (valid(check_attack_action(unit, unit, unit2, 100))) {
			prep_attack_action(unit, unit, unit2, 100);
			return TASK_PREPPED_ACTION;
		    } else {
			continue;
		    }
#endif
		}
	    }
	    if (valid(check_move_action(unit, unit, nx, ny, unit->z))) {
		prep_move_action(unit, unit, nx, ny, 0);
		return TASK_PREPPED_ACTION;
	    }
	}
    }
    return TASK_FAILED;
}

do_occupy_task(unit, task)
Unit *unit;
Task *task;
{
    Unit *transport = find_unit(task->args[0]);

    if (!in_play(transport)) return TASK_FAILED;
    if (unit->transport == transport) {
	return TASK_COMPLETE;
    }
    return TASK_FAILED;
}

/* Wait around for a particular unit.  Give up if the unit is not forthcoming. */

do_pickup_task(unit, task)
Unit *unit;
Task *task;
{
    Unit *occupant = find_unit(task->args[0]);

    if (!in_play(occupant)) return TASK_FAILED;
    wake_unit(occupant, FALSE, 0, NULL);
    if (occupant->transport == unit) {
	return TASK_COMPLETE;
    } else if (task->execnum > 10) {
	/* Waiting around isn't working for us, give up.  If the
	   prospective occupant still needs us, we'll get another
	   call. */
	return TASK_FAILED;
    } else {
	if (valid(check_enter_action(occupant, occupant, unit))) {
	    prep_enter_action(occupant, occupant, unit);
	    return TASK_PREPPED_ACTION;
	} else if (valid(check_enter_action(unit, occupant, unit))) {
	    prep_enter_action(unit, occupant, unit);
	    return TASK_PREPPED_ACTION;
	} else {
	    return (task->execnum < 5 ? TASK_INCOMPLETE : TASK_FAILED);
	}
    }
    return (task->execnum < 5 ? TASK_INCOMPLETE : TASK_FAILED);
}

Unit *
repair_here(x, y)
int x, y;
{
    Unit *unit;

    for_all_stack(x, y, unit) {
	/* what about allies? */
	if (unit->side == tmpside && can_carry(unit, tmpunit)) {
	    /* this should be controlled by doctrine? */
	    /* shouldn't wake up, should get a new task to "wait up"
	       or even approach if possible */
	/*    wake_unit(unit, FALSE, WAKEOWNER, NULL);  */
	    return unit;
	}
	/* should look at occupants in stack too */
    }
    return NULL;
}

do_repair_task(unit, task)
Unit *unit;
Task *task;
{
    int x, y, u = unit->type, m, range = area.maxdim;
    int ux = unit->x, uy = unit->y;
    Unit *unit2;

    for_all_material_types(m) {
	if (um_tomove(u, m) > 0) {
	    range = min(range, unit->supply[m] / um_tomove(u, m));
	}
    }
    tmpside = unit->side;
    tmpunit = unit;
    if (unit->hp == u_hp(u)) {  /* what if unit is multi-part? */
    	return TASK_COMPLETE;
    } else if (unit->transport != NULL) {
	set_unit_reserve(unit->side, unit, TRUE);
    	return TASK_INCOMPLETE;
    } else if ((unit2 = repair_here(ux, uy)) != NULL
    	&& unit2 != unit->transport) {	
    	prep_enter_action(unit, unit, unit2);
	return TASK_PREPPED_ACTION;
    } else if (search_around(ux, uy, range, repair_here, &x, &y, 1)) {
    	/* (should collect actual unit and chase it directly) */
	push_moveto_task(unit, x, y);
	return TASK_INCOMPLETE;
    } else {
    	/* (should be able to signal interface usefully somehow) */
	return TASK_FAILED;
    }
    return TASK_FAILED;
}

int *lowm = NULL, numlow; 

Unit *
aux_resupply_here(unit)
Unit *unit;
{
    int i, enough = TRUE;
    Unit *occ;

	/* what about allies? */
	if (unit->side == tmpside
	    && can_carry(unit, tmpunit)) {
	    for (i = 0; i < numlow; ++i) {
		if (unit->supply[lowm[i]] == 0) enough = FALSE;
	    }
	    /* should test that unit has desired supply too... */
	    /* this should be controlled by doctrine? */
	    /* shouldn't wake up, should get a new task to "wait up"
	       or even approach if possible */
	    /*    wake_unit(unit, FALSE, WAKEOWNER, NULL);  */
	    if (enough) return unit;
	}
	for_all_occupants(unit, occ) {
		return aux_resupply_here(occ);
	}
	return NULL;
}

Unit *
resupply_here(x, y)
int x, y;
{
    Unit *unit, *resupplier;

    for_all_stack(x, y, unit) {
    	resupplier = aux_resupply_here(unit);
    	if (resupplier) return resupplier;
    }
    return NULL;
}

/* Replenish our supplies, using one of several strategies, which as usual
   depends on the game, unit, terrain, etc.  Strategies include 1) wait for
   supply line or own production to replenish, 2) move to productive terrain
   and then wait, 3) move within range of a supplier, and 4) request a supplier
   to approach. */

do_resupply_task(unit, task)
Unit *unit;
Task *task;
{
    int x, y, u = unit->type, m, range = area.maxdim;
    int ux = unit->x, uy = unit->y;
    Unit *unit2;

    tmpside = unit->side;
    tmpunit = unit;
    if (lowm == NULL) lowm = (int *) xmalloc(nummtypes * sizeof(int));
    numlow = 0;
    for_all_material_types(m) {
    	if (unit->supply[m] < um_storage(u, m)) {
    	    lowm[numlow++] = m;
    	    /* (doesn't account for shared hold) */
    	}
	if (um_tomove(u, m) > 0) {
	    range = min(range, unit->supply[m] / um_tomove(u, m));
	}
    }
    /* We're all full up, must be OK. */
    if (numlow == 0) {
    	return TASK_COMPLETE;
    } else if (can_resupply_self(unit, lowm, numlow)) {
	set_unit_reserve(unit->side, unit, TRUE);
    } else if (unit->transport != NULL) {
    	/* (could attempt to resupply via direct action) */
	set_unit_reserve(unit->side, unit, TRUE);
    	return (probability(10) ? TASK_FAILED : TASK_INCOMPLETE);
    } else if ((unit2 = resupply_here(ux, uy)) != NULL
    	&& unit2 != unit->transport) {	
    	prep_enter_action(unit, unit, unit2);
	return TASK_PREPPED_ACTION;
    } else if (search_around(ux, uy, range, resupply_here, &x, &y, 1)) {
    	/* (should collect actual unit and chase it directly) */
    	/* (only need to get with outlength of supply) */
	push_moveto_task(unit, x, y);
	return TASK_INCOMPLETE;
    } else {
	set_unit_reserve(unit->side, unit, TRUE);
    	/* (should be able to signal interface usefully somehow) */
	return TASK_FAILED;
    }
}

can_resupply_self(unit, materials, numtypes)
Unit *unit;
int *materials, numtypes;
{
    int u = unit->type, i, m, rslt = TRUE, t = terrain_at(unit->x, unit->y);

    for (i = 0; i < numtypes; ++i) {
    	m = materials[i];
	if (um_produce(u, m) * ut_productivity(u, t) <= um_consume(u, m)) rslt = FALSE;
    }
    return rslt;
}

do_sentry_task(unit, task)
Unit *unit;
Task *task;
{
    if (task->args[0] > 0) {
	unit->plan->reserve = TRUE;
	--(task->args[0]);
	return TASK_INCOMPLETE;
    } else {
	/* Unit won't necessarily wake up, may just replan and
	   continue sleeping. */
	return TASK_COMPLETE;
    }
    return TASK_FAILED;
}

#if 0
(int (*taskfunctions)())[] = {

#undef  DEF_TASK
#define DEF_TASK(NAME,code,argtypes,FN) FN,

#include "task.def"

    NULL
};
#endif

/* Allocate an initial collection of task objects. */

void
init_tasks()
{
    int i;

    freetasks = (Task *) xmalloc(INITMAXTASKS * sizeof(Task));
    for (i = 0; i < INITMAXTASKS; ++i) {
      freetasks[i].next = &freetasks[i+1];
    }
    freetasks[INITMAXTASKS-1].next = NULL;
}

/* Create and return a new task. */

Task *
create_task(type)
TaskType type;
{
    int i;
    Task *task;

    if (freetasks) {
	task = freetasks;
	freetasks = task->next;
    } else {
	task = (Task *) xmalloc(sizeof(Task));
    }
    task->type = type;
    task->execnum = 0;
    task->retrynum = 0;
    for (i = 0; i < MAXTASKARGS; ++i) task->args[i] = 0;
    task->next = NULL;
    return task;
}

TaskOutcome
execute_task(unit)
Unit *unit;
{
    Plan *plan = unit->plan;
    TaskOutcome rslt;
    Task *task;

    if (unit->plan == NULL) run_error("???");
    task = plan->tasks;
    rslt = execute_task_aux(unit, task);
    DMprintf("%s did task %s: ", unit_desig(unit), task_desig(task));
    /* Now look at what happened with task execution. */
    switch (rslt) {
      case TASK_UNKNOWN:
	DMprintf("unknown outcome???");
	break;
      case TASK_FAILED:
        ++task->retrynum;
	DMprintf("failed try %d, ", task->retrynum);
	if (probability(20) || task->retrynum > 5) {
	    pop_task(plan);
	    DMprintf("removed it");
	    /* We might be buzzing, so maybe go into reserve. */
	    if (probability(20)) {
	    	plan->reserve = TRUE;
	    	DMprintf(" and went into reserve");
	    }
	} else {
	    DMprintf("will retry");
	}
	break;
      case TASK_INCOMPLETE:
	/* Leave the task alone. */
	DMprintf("incomplete");
	break;
      case TASK_PREPPED_ACTION:
	/* Mention the action prepared to execute. */
	DMprintf("prepped action %s", action_desig(&(unit->act->nextaction)));
	break;
      case TASK_COMPLETE:
	DMprintf("completed after %d executions", task->execnum);
	pop_task(plan);
	break;
      default:
	break;
    }
    DMprintf("\n");
    /* Give AIs a chance to decide what to do. */
    if (unit->side != NULL && side_has_ai(unit->side)) {
    	ai_react_to_task_result(unit->side, unit, task, rslt);
    }
    return rslt;
}

/* Perform a single given task. */

TaskOutcome
execute_task_aux(unit, task)
Unit *unit;
Task *task;
{
    if (!alive(unit) || task == NULL) return;
    DMprintf("%s doing task %s\n", unit_desig(unit), task_desig(task));
    /* Count this execution. */
    ++task->execnum;
    switch (task->type) {
      case NO_TASK:
	/* This is a no-op, useful as a placeholder.  Always "succeeds". */
	return TASK_COMPLETE;
      case BUILD_TASK:
	return do_build_task(unit, task);
      case RESEARCH_TASK:
	return do_research_task(unit, task);
      case CAPTURE_UNIT_TASK:
	return do_capture_unit_task(unit, task);
      case HIT_POSITION_TASK:
	return do_hit_position_task(unit, task);
      case HIT_UNIT_TASK:
	return do_hit_unit_task(unit, task);
      case MOVEDIR_TASK:
	return do_movedir_task(unit, task);
      case MOVETO_TASK:
	return do_moveto_task(unit, task);
      case SENTRY_TASK:
        return do_sentry_task(unit, task);
      case RESUPPLY_TASK:
        return do_resupply_task(unit, task);
      case PICKUP_TASK:
        return do_pickup_task(unit, task);
      default:
/*        case_panic("task type", task->type);  */
	run_warning("Unknown task type %d", task->type);
    	return TASK_FAILED;
    }
}

/* This weird-looking routine computes next directions for moving to a
   given spot.  The number of directions ranges from 1 to 4, depending
   on whether there is a straight-line path to the dest, and whether we are
   required to take a direct path or are allowed to move in dirs that don't
   the unit any closer (we never increase our distance though).
   Some trickinesses:  if area wraps, must resolve ambiguity about
   getting to the same place going either direction (we pick shortest). */

choose_move_dirs(unit, tx, ty, shortest, dirtest, dirsort, dirs)
Unit *unit;
int tx, ty, shortest, (*dirtest)(), (*dirsort)(), *dirs;
{
    int avoid = 0;
    int closer;
    int dx, dxa, dy, dist, d1, d2, d3, d4, axis = -1, hextant = -1, tmp;
    int numdirs = 0, shortestnumdirs;

    dist = distance(unit->x, unit->y, tx, ty);
    dx = tx - unit->x;  dy = ty - unit->y;

    if (area.xwrap) {
	dxa = (tx + area.width) - unit->x;
	if (abs(dx) > abs(dxa)) dx = dxa;
	dxa = (tx - area.width) - unit->x;
	if (abs(dx) > abs(dxa)) dx = dxa;
    }
    if (dx == 0 && dy == 0) {
	return -1;
    }
    axis = hextant = -1;
    if (dx == 0) {
	axis = (dy > 0 ? NORTHEAST : SOUTHWEST);
    } else if (dy == 0) {
	axis = (dx > 0 ? EAST : WEST);
    } else if (dx == (0 - dy)) {
	axis = (dy > 0 ? NORTHWEST : SOUTHEAST);
    } else if (dx > 0) {
	hextant = (dy > 0 ? EAST :
		   (abs(dx) > abs(dy) ? SOUTHEAST : SOUTHWEST));
    } else {
	hextant = (dy < 0 ? WEST :
		   (abs(dx) > abs(dy) ? NORTHWEST : NORTHEAST));
    }
    if (axis >= 0) {
	d1 = d2 = axis;
	if (dirtest == NULL || (*dirtest)(unit, d1)) {
	    dirs[numdirs++] = d1;
	}
    }
    if (hextant >= 0) {
	d1 = left_dir(hextant);
	d2 = hextant;
	if (dirtest == NULL || (*dirtest)(unit, d1)) {
	    dirs[numdirs++] = d1;
	}
	if (dirtest == NULL || (*dirtest)(unit, d2)) {
	    dirs[numdirs++] = d2;
	}
    }
    /* Check on other properties of the two choices. */
    if (numdirs > 1 && dirsort != NULL) {
    	(*dirsort)(unit, dirs, numdirs);
    }
    if (dist > 1 && !shortest) {
	shortestnumdirs = numdirs;
    	d3 = left_dir(d1);
    	d4 = right_dir(d2);
	if (dirtest == NULL || (*dirtest)(unit, d3)) {
	    dirs[numdirs++] = d3;
	}
	if (dirtest == NULL || (*dirtest)(unit, d4)) {
	    dirs[numdirs++] = d4;
	}
	if (numdirs > shortestnumdirs + 1 && dirsort != NULL) {
	    (*dirsort)(unit, dirs + shortestnumdirs, numdirs - shortestnumdirs);
	}
    }
    return numdirs;
}

/* A heuristic test for whether the given direction is a good one
   to move in. */

plausible_move_dir(unit, dir)
Unit *unit;
int dir;
{
    int u = unit->type, ux = unit->x, uy = unit->y, nx, ny, t, c;

    point_in_dir(ux, uy, dir, &nx, &ny);
    if (unit_at(nx, ny)) return TRUE;
    t = terrain_at(nx, ny);
    if ((ut_vanishes_on(u, t)
         || ut_wrecks_on(u, t))
        && !can_move_via_conn(unit, nx, ny)) return FALSE;
    if (ut_mp_to_enter(u, t) <= u_acp(u)) return TRUE;
    if (numconntypes > 0) {
	/* Try each connection type to see if it works. */
	for_all_terrain_types(c) {
	    if (t_is_connection(c)
		&& aux_terrain_defined(c)
		&& connection_at(ux, uy, dir, c)) {
		if ((ut_mp_to_enter(u, c)
		     + ut_mp_to_traverse(u, c)
		     + ut_mp_to_leave(u, c)) <= u_acp(u)) return TRUE;
	    }
	}
    }
    return FALSE;
}

/* This compares the desirability of two different directions.  This is
   somewhat tricky, because it should return < 0 if i0 designates a BETTER
   direction than i1. */

int xs[NUMDIRS], ys[NUMDIRS], terrs[NUMDIRS];

compare_directions(i0, i1)
int *i0, *i1;
{
    int u = tmputype, t0 = terrs[*i0], t1 = terrs[*i1];
    int ux = tmpunit->x, uy = tmpunit->y, u2 = NONUTYPE;
    int cost0 = 0, cost1 = 0, s, ps0, ps1, surr0, surr1, rslt;
    extern anypeoplesidechanges, *anypeoplesurrenders;

    if (tmpunit->transport) u2 = tmpunit->transport->type;
    cost0 = total_move_cost(u, u2, ux, uy, 0, xs[*i0], ys[*i0], 0);
    cost1 = total_move_cost(u, u2, ux, uy, 0, xs[*i1], ys[*i1], 0);
    if (cost0 != cost1) {
	return cost0 - cost1;
    }
#if 0
    if (ut_mp_to_enter(u, t0) > ut_mp_to_enter(u, t1)) {
        /* (should not use if movement is by conn only) */
	return 1;
    }
#endif
    if (1 /* not in supply */) {
	if ((rslt = ut_productivity(u, t1) - ut_productivity(u, t0)) != 0) {
	    return rslt;
	}
    }
    if ((rslt = ut_mp_to_leave(u, t1) - ut_mp_to_leave(u, t0)) != 0) {
	return rslt;
    }
    if ((rslt = ut_accident_hit(u, t1) - ut_accident_hit(u, t0)) != 0) {
	return rslt;
    }
    if ((rslt = ut_visibility(u, t1) - ut_visibility(u, t0)) != 0) {
	return rslt;
    }
    /* Prefer to go over cells that we can change to our side. */
    if (anypeoplesurrenders != NULL && anypeoplesurrenders[u]) {
    	s = side_number(tmpunit->side);
    	ps0 = people_side_at(xs[*i0], ys[*i0]);
    	ps1 = people_side_at(xs[*i1], ys[*i1]);
    	surr0 = ut_people_surrender(u, t0) * ((ps0 != NOBODY && s != ps0) ? 1 : 0);
    	surr1 = ut_people_surrender(u, t1) * ((ps1 != NOBODY && s != ps1) ? 1 : 0);
    	if (surr0 != surr1) {
	    return surr1 - surr0;
    	}
    }
    return 0;
}

sort_directions(unit, dirs, numdirs)
Unit *unit;
int *dirs, numdirs;
{
    int i, u = unit->type, m, tmp, i0 = 0, i1 = 1, compar;

    for (i = 0; i < numdirs; ++i) { 
	point_in_dir(unit->x, unit->y, dirs[i], &(xs[i]), &(ys[i]));
	terrs[i] = terrain_at(xs[i], ys[i]);
    }
    tmpunit = unit;
    tmputype = unit->type;
    if (numdirs == 2) {
	compar = compare_directions(&i0, &i1);
    	if (compar > 0 || (compar == 0 && flip_coin())) {
    	    tmp = dirs[0];  dirs[0] = dirs[1];  dirs[1] = tmp;
    	}
    } else if (numdirs > 2) {
    	qsort(dirs, numdirs, sizeof(int), compare_directions);
	if (compare_directions(&i0, &i1) == 0 && flip_coin()) {
	    tmp = dirs[0];  dirs[0] = dirs[1];  dirs[1] = tmp;
	}
    }
}

/* Put the given task back onto the list of free tasks. */

void 
free_task(task)
Task *task;
{
    task->next = freetasks;
    freetasks = task;
}

/* Make the given unit be able to execute its plan. */

be_active(unit)
Unit *unit;
{
    if (unit->plan == NULL) return;
    /* Shouldn't be asleep any longer. */
    unit->plan->asleep = FALSE;
    /* We're not in reserve. */
    unit->plan->reserve = FALSE;
    /* Obviously we're no longer waiting to be told what to do. */
    unit->plan->waitingfortasks = FALSE;
}

/* Order a unit to sit quietly for some number of turns. */

Task *
create_sentry_task(unit, n)
Unit *unit;
int n;
{
    Task *task = create_task(SENTRY_TASK);

    task->args[0] = n;
    return task;
}

push_sentry_task(unit, n)
Unit *unit;
int n;
{
    Task *task;

    if (!tasked_unit_valid(unit, "sentry")) return;
    task = create_sentry_task(unit, n);
    push_task(unit, task);
    /* Obviously we're no longer waiting to be told what to do. */
    unit->plan->waitingfortasks = FALSE;
}

Task *
create_moveto_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task = create_task(MOVETO_TASK);

    task->args[0] = x;  task->args[1] = y;
    return task;
}

order_sentry(unit, n)
Unit *unit;
int n;
{
    if (!tasked_unit_valid(unit, "sentry")) return;
    clear_task_agenda(unit->plan);
    unit->plan->tasks = create_task(SENTRY_TASK);
    unit->plan->tasks->args[0] = n;
    /* We're no longer waiting to be told what to do. */
    unit->plan->waitingfortasks = FALSE;
}

push_moveto_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task;

    if (!tasked_unit_valid(unit, "move")) return;
    task = create_moveto_task(unit, x, y);
    push_task(unit, task);
    /* Obviously we're no longer waiting to be told what to do. */
    unit->plan->waitingfortasks = FALSE;
}

/* Give the unit a task to move to a given place, erasing every other task. */

order_moveto(unit, x, y)
Unit *unit;
int x, y;
{
    if (!tasked_unit_valid(unit, "move")) return;
    if (!in_area(x, y)) {
    	run_warning("Trying to move %s to %d,%d", unit_desig(unit), x, y);
    	return;
    }
    clear_task_agenda(unit->plan);
    unit->plan->tasks = create_moveto_task(unit, x, y);
    be_active(unit);
}

set_moveto_task(unit, x, y)
Unit *unit;
int x, y;
{
    if (!tasked_unit_valid(unit, "move")) return;
    if (!in_area(x, y)) {
    	run_warning("Trying to move %s to %d,%d", unit_desig(unit), x, y);
    	return;
    }
    clear_task_agenda(unit->plan);
    unit->plan->tasks = create_moveto_task(unit, x, y);
    be_active(unit);
}

Task *
create_movenear_task(unit, x, y, dist)
Unit *unit;
int x, y, dist;
{
    Task *task = create_task(MOVETO_TASK);

    task->args[0] = x;  task->args[1] = y;
    task->args[2] = dist;
    return task;
}

push_movenear_task(unit, x, y, dist)
Unit *unit;
int x, y, dist;
{
    Task *task;

    if (!tasked_unit_valid(unit, "move")) return;
    task = create_movenear_task(unit, x, y, dist);
    push_task(unit, task);
    be_active(unit);
}

/* Give the unit a task to move to a given place, erasing every other task. */

order_movenear(unit, x, y)
Unit *unit;
int x, y;
{
    if (!tasked_unit_valid(unit, "move")) return;
    if (!in_area(x, y)) {
    	run_warning("Trying to move %s to %d,%d", unit_desig(unit), x, y);
    	return;
    }
    clear_task_agenda(unit->plan);
    unit->plan->tasks = create_moveto_task(unit, x, y, 1);
    be_active(unit);
}

/* Create a task to move in a given direction for a given distance. */

Task *
create_movedir_task(unit, dir, n)
Unit *unit;
int dir, n;
{
    Task *task = create_task(MOVEDIR_TASK);

    task->args[0] = dir;  task->args[1] = n;
    return task;
}

/* Fill in the given unit with direction-moving orders. */

order_movedir(unit, dir, n)
Unit *unit;
int dir, n;
{
    clear_task_agenda(unit->plan);
    unit->plan->tasks = create_movedir_task(unit, dir, n);
    unit->plan->tasks->args[0] = dir;  unit->plan->tasks->args[1] = n;
    be_active(unit);
}

/* A two-waypoint patrol suffices for many purposes. */
/* Should have a more general patrol routine eventually (> 2 waypoints). */

order_patrol(unit, x0, y0, x1, y1, n)
Unit *unit;
int x0, y0, x1, y1, n;
{
#if 0
    unit->plan->orders.type = PATROL;
    unit->plan->orders.rept = n;
    unit->plan->orders.p.pt[0].x = x0;
    unit->plan->orders.p.pt[0].y = y0;
    unit->plan->orders.p.pt[1].x = x1;
    unit->plan->orders.p.pt[1].y = y1;
#endif
}

/* Wait around to embark on some unit when it comes to our location. */

order_embark(unit, n)
Unit *unit;
int n;
{
}

/* This routine sets up a task to build a unit of the given type. */

Task *
create_build_task(unit, u2, run)
Unit *unit;
int u2, run;
{
    Task *task = create_task(BUILD_TASK);

    /* (should check plausibility first?) */
    task->args[0] = u2;
    task->args[3] = run;
    return task;
}

push_build_task(unit, u2, run)
Unit *unit;
int u2, run;
{
    Task *task;

    if (!tasked_unit_valid(unit, "build")) return;
    task = create_build_task(unit, u2, run);
    push_task(unit, task);
    be_active(unit);
}

/* This routine sets up a task to research a unit of the given type. */

Task *
create_research_task(unit, u2, n)
Unit *unit;
int u2, n;
{
    Task *task = create_task(RESEARCH_TASK);

    task->args[0] = u2;
    task->args[1] = n;
    return task;
}

push_research_task(unit, u2, n)
Unit *unit;
int u2, n;
{
    Task *task;

    if (!tasked_unit_valid(unit, "research")) return;
    task = create_research_task(unit, u2, n);
    push_task(unit, task);
    be_active(unit);
}

set_hit_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task = create_task(HIT_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    task->args[2] = NONUTYPE;  task->args[3] = -1;
    clear_task_agenda(unit->plan);
    unit->plan->tasks = task;
}

push_specific_hit_task(unit, x, y, u, s)
Unit *unit;
int x, y, u, s;
{
    Task *task = create_task(HIT_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    task->args[2] = u;  task->args[3] = s;
    push_task(unit, task);
    be_active(unit);
}

set_specific_hit_task(unit, x, y, u, s)
Unit *unit;
int x, y, u, s;
{
    Task *task = create_task(HIT_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    task->args[2] = u;  task->args[3] = s;
    clear_task_agenda(unit->plan);
    unit->plan->tasks = task;
}

push_hit_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task = create_task(HIT_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    task->args[2] = NONUTYPE;  task->args[3] = -1 /* allsidesmask */;
    push_task(unit, task);
    be_active(unit);
}

set_capture_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task = create_task(CAPTURE_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    clear_task_agenda(unit->plan);
    unit->plan->tasks = task;
}

push_capture_task(unit, x, y)
Unit *unit;
int x, y;
{
    Task *task = create_task(CAPTURE_UNIT_TASK);

    task->args[0] = x;  task->args[1] = y;
    push_task(unit, task);
    be_active(unit);
}

set_resupply_task(unit)
Unit *unit;
{
    Task *task;

    if (!tasked_unit_valid(unit, "resupply")) return;
    clear_task_agenda(unit->plan);
    task = create_task(RESUPPLY_TASK);
    unit->plan->tasks = task;
    be_active(unit);
}

Task *
create_occupy_task(unit, transp)
Unit *unit, *transp;
{
    Task *task = create_task(OCCUPY_TASK);

    task->args[0] = transp->id;
    /* add a waiting period also? */
    return task;
}

push_occupy_task(unit, transp)
Unit *unit, *transp;
{
    Task *task;

    if (!tasked_unit_valid(unit, "occupy")) return;
    task = create_occupy_task(unit, transp);
    push_task(unit, task);
    be_active(unit);
}

Task *
create_pickup_task(unit, occ)
Unit *unit, *occ;
{
    Task *task = create_task(PICKUP_TASK);

    task->args[0] = occ->id;
    /* add a waiting period also? */
    return task;
}

push_pickup_task(unit, occ)
Unit *unit, *occ;
{
    Task *task;

    if (!tasked_unit_valid(unit, "pickup")) return;
    task = create_pickup_task(unit, occ);
    push_task(unit, task);
    be_active(unit);
}

tasked_unit_valid(unit, tasktypename)
Unit *unit;
char *tasktypename;
{
    if (!in_play(unit) || unit->plan == NULL) {
	run_warning("Trying to do %s task with bad %s", tasktypename, unit_desig(unit));
	return FALSE;
    }
    return TRUE;
}

/* Describe a task succinctly - use for debugging only. */

char *
task_desig(task)
Task *task;
{
    int i;
    char *argtypes;

    if (taskbuf == NULL) taskbuf = xmalloc(BUFSIZE);
    if (task) {
	sprintf(taskbuf, "{%s", taskdefns[task->type].name);
	argtypes = taskdefns[task->type].argtypes;
	for (i = 0; i < strlen(argtypes); ++i) {
	    tprintf(taskbuf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	tprintf(taskbuf, " x %d", task->execnum);
	if (task->retrynum > 0) {
	    tprintf(taskbuf, " fail %d", task->retrynum);
	}
	strcat(taskbuf, "}");
    } else {
	sprintf(taskbuf, "no task");
    }
    return taskbuf;
}

