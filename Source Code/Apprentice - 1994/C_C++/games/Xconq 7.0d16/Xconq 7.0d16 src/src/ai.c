/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Functions common to all AIs. */

#include "conq.h"

/* (should make "real" objects for each AI type) */

char *aitypenames[] = {
  "brainless",
  "mplayer",
  NULL
};

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
		clear_task_agenda(unit->plan);
		unit->plan->asleep = FALSE;
		unit->plan->reserve = FALSE;
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
		clear_task_agenda(unit->plan);
		unit->plan->aicontrol = TRUE;
	    }
	    /* Still let units finish their currently buffered action. */
	}
	init_ai(side);
    }
}

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
	break;
    }
}

ai_react_to_unit_loss(side, unit)
Side *side;
Unit *unit;
{
    switch (side_ai_type(side)) {
      case mplayertype:
	mplayer_react_to_unit_loss(side, unit);
    }
}

ai_react_to_action_result(side, unit, rslt)
Side *side;
Unit *unit;
int rslt;
{
    switch (side_ai_type(side)) {
      case mplayertype:
	mplayer_react_to_action_result(side, unit, rslt);
    }
}

ai_react_to_task_result(side, unit, task, rslt)
Side *side;
Unit *unit;
Task *task;
int rslt;
{
    switch (side_ai_type(side)) {
      case mplayertype:
	mplayer_react_to_task_result(side, unit, task, rslt);
    }
}

ai_write_state(fp, side)
FILE *fp;
Side *side;
{
    switch (side_ai_type(side)) {
      case mplayertype:
	mplayer_write_state(fp, side);
    }
}

ai_read_state(side, data)
Side *side;
Obj *data;
{
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

hex_unknown(x, y)
{
    return (!g_see_all() && terrain_view(tmpside, x, y) == UNSEEN);
}

enemies_present(x, y)
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

goal_truth(side, goal)
Side *side;
Goal *goal;
{
    int x, y;
    Side *side2 = NULL;

    if (goal == NULL) return 0;
    switch (goal->type) {
      case WON_GAME:
	side2 = goal->side;
	return (side2 ? (side_won(side2) ? 100 : -100) : 0);
      case LOST_GAME:
	side2 = goal->side;
	return (side2 ? (side_lost(side2) ? 100 : -100) : 0);
      case POSITIONS_KNOWN:
	/* what if no enemies present? then this is undefined? */
	/* should goals have preconditions or prerequisites? */
	return 0;
      case WORLD_KNOWN:
	tmpside = side;
	for_all_interior_hexes(x, y) {
	    if (hex_unknown(x, y)) return -100;
	}
	return 100;
      case VICINITY_KNOWN:
	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  hex_unknown, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      case VICINITY_HELD:
      	tmpside = side;
	if (search_around(goal->args[0], goal->args[1], goal->args[2],
			  enemies_present, &x, &y, 1)) {
	    return -100;
	} else {
	    return 100;
	}
      default:
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
    int i;
    char *argtypes;

    if (goal == NULL) return "<null goal>";
    if (goalbuf == NULL) goalbuf = xmalloc(BUFSIZE);
    sprintf(goalbuf, "<goal s%d %s%s",
	    side_number(goal->side), (goal->tf ? "" : "not "),
	    goaldefns[goal->type].name);
    argtypes = goaldefns[goal->type].argtypes;
    for (i = 0; i < strlen(argtypes); ++i) {
	switch (argtypes[i]) {
	  default:
	    sprintf(goalbuf+strlen(goalbuf), " %d", goal->args[i]);
	}
    }
    strcat(goalbuf, ">");
    return goalbuf;
}
