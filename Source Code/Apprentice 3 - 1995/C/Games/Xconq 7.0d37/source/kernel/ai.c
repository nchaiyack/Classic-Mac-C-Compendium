/* Functions common to all AIs.
   Copyright (C) 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

#include "mplay.h"

/* (should make "real" objects for each AI type) */

char *aitypenames[] = {
  "brainless",
  "mplayer",
  NULL
};

void
init_ai(side)
Side *side;
{
    if (side_wants_ai(side)) {
	if (strcmp("mplayer", side->player->aitypename) == 0) {
	    mplayer_init(side);
	} else {
	}
	if (!side_has_ai(side)) {
	    init_warning("might not have made an AI (type %s) for %s",
			 side->player->aitypename, side_desig(side));
	}
    }
}

/* Change the AI running a side.  This has to clean up if the AI is
   being turned off. */

void
set_side_ai(side, typename)
Side *side;
char *typename;
{
    Unit *unit;

    if (typename == NULL) {
	side->player->aitypename = NULL;
	/* (should just deactivate) */
	side->ai = NULL;
	/* Clear out everything that was set up by the AI. */
	for_all_side_units(side, unit) {
	    unit->aihook = NULL;
	    if (unit->plan && unit->plan->aicontrol) {
		unit->plan->type = PLAN_NONE;
		unit->plan->maingoal = NULL;
		unit->plan->formation = NULL;
		unit->plan->funit = NULL;
		clear_task_agenda(unit->plan);
		unit->plan->asleep = FALSE;
		unit->plan->reserve = FALSE;
		/* Don't touch delay, let action looping clear it eventually. */
		unit->plan->aicontrol = FALSE;
	    }
	    /* Still let units finish their currently buffered action. */
	}
    } else {
	side->player->aitypename = typename;
	for_all_side_units(side, unit) {
	    unit->aihook = NULL;
	    if (unit->plan && unit->plan->aicontrol) {
		unit->plan->type = PLAN_NONE;
		unit->plan->maingoal = NULL;
		unit->plan->formation = NULL;
		unit->plan->funit = NULL;
		clear_task_agenda(unit->plan);
		unit->plan->aicontrol = TRUE;
	    }
	    /* Still let units finish their currently buffered action. */
	}
	init_ai(side);
    }
}

void
ai_init_turn(side)
Side *side;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_init_turn(side);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

void
ai_decide_plan(side, unit)
Side *side;
Unit *unit;
{
    switch (side_ai_type(side)) {
      case nobrains:
	unit->plan->type = PLAN_PASSIVE;
	clear_task_agenda(unit->plan);
	break;
      case mplayertype:
	mplayer_decide_plan(side, unit);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

void
ai_react_to_unit_loss(side, unit)
Side *side;
Unit *unit;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_react_to_unit_loss(side, unit);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

/* Forward an action result to the appropriate AI routine. */

void
ai_react_to_action_result(side, unit, rslt)
Side *side;
Unit *unit;
int rslt;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_react_to_action_result(side, unit, rslt);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

/* Forward a task result to the appropriate AI routine. */

void
ai_react_to_task_result(side, unit, task, rslt)
Side *side;
Unit *unit;
Task *task;
int rslt;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_react_to_task_result(side, unit, task, rslt);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

int
ai_guide_explorer(side, unit)
Side *side;
Unit *unit;
{
    switch (side_ai_type(side)) {
      case nobrains:
	return FALSE;
      case mplayertype:
	return mplayer_guide_explorer(side, unit);
      default:
	case_panic("player AI type", side_ai_type(side));
      	return FALSE;
    }
}

int
ai_preferred_build_type(side, unit, plantype)
Side *side;
Unit *unit;
int plantype;
{
    switch (side_ai_type(side)) {
      case nobrains:
	return NONUTYPE;
      case mplayertype:
	return mplayer_preferred_build_type(side, unit, plantype);
      default:
	case_panic("player AI type", side_ai_type(side));
      	return NONUTYPE;
    }
}

void
ai_finish_movement(side)
Side *side;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_finish_movement(side);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

/* Forward a textual message to the appropriate AI routine. */

void
ai_receive_message(side, sender, str)
Side *side, *sender;
char *str;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_receive_message(side, sender, str);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

void
ai_write_state(fp, side)
FILE *fp;
Side *side;
{
    switch (side_ai_type(side)) {
      case nobrains:
	break;
      case mplayertype:
	mplayer_write_state(fp, side);
	break;
      default:
	case_panic("player AI type", side_ai_type(side));
	break;
    }
}

void
ai_read_state(side, data)
Side *side;
Obj *data;
{
#if 0
    /* (should do this eventually - tricky because need to save data until
       AI is actually being set up) */
    side->aidata = (void *) data;
#endif
}

int
ai_region_at(side, x, y)
Side *side;
int x, y;
{
    switch (side_ai_type(side)) {
      case nobrains:
	return 0;
      case mplayertype:
	return mplayer_theater_at(side, x, y);
      default:
	case_panic("player AI type", side_ai_type(side));
	return 0;
    }
}

char *
ai_at_desig(side, x, y)
Side *side;
int x, y;
{
    switch (side_ai_type(side)) {
      case nobrains:
	return NULL;
      case mplayertype:
	return mplayer_at_desig(side, x, y);
      default:
	case_panic("player AI type", side_ai_type(side));
	return NULL;
    }
}

/* (should go elsewhere eventually?) */

/* Goal handling. */

GoalDefn goaldefns[] = {

#undef  DEF_GOAL
#define DEF_GOAL(NAME,code,ARGTYPES) { NAME, ARGTYPES },

#include "goal.def"

    { NULL, NULL }
};


/* General handling of goals. */

Goal *
create_goal(type, side, tf)
GoalType type;
Side *side;
int tf;
{
    Goal *goal = (Goal *) xmalloc(sizeof(Goal));

    goal->type = type;
    goal->side = side;
    goal->tf = tf;
    return goal;
}

int
cell_unknown(x, y)
int x, y;
{
    return (!g_see_all() && terrain_view(tmpside, x, y) == UNSEEN);
}

int
enemies_present(x, y)
int x, y;
{
    if (g_see_all() /* or x,y under direct observation */) {
    	return (unit_at(x, y) != NULL && unit_at(x, y)->side != tmpside);
    } else if (terrain_view(tmpside, x, y) != UNSEEN) {
    	return (vside(unit_view(tmpside, x, y)) != side_number(tmpside));
    } else {
    	return FALSE;
    }
}

/* Test a goal to see if it is true for side, as specified. */

int
goal_truth(side, goal)
Side *side;
Goal *goal;
{
    int x, y;
    Side *side2 = NULL;

    if (goal == NULL) return 0;
    switch (goal->type) {
      case GOAL_WON_GAME:
	side2 = goal->side;
	return (side2 ? (side_won(side2) ? 100 : -100) : 0);
      case GOAL_LOST_GAME:
	side2 = goal->side;
	return (side2 ? (side_lost(side2) ? 100 : -100) : 0);
      case GOAL_POSITIONS_KNOWN:
	/* what if no enemies present? then this is undefined? */
	/* should goals have preconditions or prerequisites? */
	return 0;
      case GOAL_WORLD_KNOWN:
	tmpside = side;
	for_all_interior_cells(x, y) {
	    if (cell_unknown(x, y)) return -100;
	}
	return 100;
      case GOAL_VICINITY_KNOWN:
	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  cell_unknown, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      case GOAL_VICINITY_HELD:
      	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  enemies_present, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      case GOAL_CELL_OCCUPIED:
	return 0;
      case GOAL_HAS_UNIT_TYPE:
	return 0;
      case GOAL_HAS_UNIT_TYPE_NEAR:
	return 0;
      case GOAL_HAS_MATERIAL_TYPE:
	return 0;
      default:
	case_panic("goal type", goal->type);
	return 0;
    }
}

/* (might eventually want another evaluator that guesses at another
   side's goals) */

char *goalbuf = NULL;

char *
goal_desig(goal)
Goal *goal;
{
    int numargs, i, arg;
    char *argtypes;

    if (goal == NULL)
      return "<null goal>";
    if (goalbuf == NULL)
      goalbuf = xmalloc(BUFSIZE);
    sprintf(goalbuf, "<goal s%d %s%s",
	    side_number(goal->side), (goal->tf ? "" : "not "),
	    goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    numargs = strlen(argtypes);
    for (i = 0; i < numargs; ++i) {
	arg = goal->args[i];
	switch (argtypes[i]) {
	  case 'h':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  case 'm':
	    if (is_material_type(arg))
	      tprintf(goalbuf, " %s", m_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'S':
	    tprintf(goalbuf, " `%s'", side_desig(side_n(arg)));
	    break;
	  case 'u':
	    if (is_unit_type(arg))
	      tprintf(goalbuf, " %s", u_type_name(arg));
	    else
	      tprintf(goalbuf, " m%d?", arg);
	    break;
	  case 'U':
	    tprintf(goalbuf, " `%s'", unit_desig(find_unit(arg)));
	    break;
	  case 'w':
	    tprintf(goalbuf, " %dx", arg);
	    break;
	  case 'x':
	    tprintf(goalbuf, " %d,", arg);
	    break;
	  case 'y':
	    tprintf(goalbuf, "%d", arg);
	    break;
	  default:
	    tprintf(goalbuf, " %d", arg);
	    break;
	}
    }
    strcat(goalbuf, ">");
    return goalbuf;
}
