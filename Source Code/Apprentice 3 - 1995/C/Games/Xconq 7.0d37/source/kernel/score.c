/* Scores and scoring in Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* Scoring in Xconq does not happen by default; instead a game design
   may include one or more "scorekeepers", which are objects with properties
   that define how winning and losing will be decided.

   There are two kinds of functionality in scorekeepers.  One kind is the
   ability to end a game based on a specified condition, such as "the Star
   of Satyria enters Noblum".  The other is the accumulation of a numeric score.
   A scorekeeper may include both, so it can (for instance) end the game the
   instant that a player's score reaches 100, or 100 more than another player,
   or whatever.  */

#include "conq.h"

/* This is true when the given side should be tested against the given scorekeeper. */

#define scorekeeper_applicable(side,sk)  \
  ((side)->ingame && ((sk)->who == lispnil || 1 /* in side mask */))

static void eval_sk_last_side_wins PROTO ((Scorekeeper *sk));

/* The head of the list of scorekeepers. */

Scorekeeper *scorekeepers = NULL;

/* The total number of scorekeepers defined. */

int numscorekeepers = 0;

int nextskid = 1;

/* The number of scorekeepers maintaining numeric scores. */

int numscores = 0;

/* True if any pre-turn scorekeepers are defined. */

int any_pre_turn_scores = FALSE;

/* True if any post-turn scorekeepers are defined. */

int any_post_turn_scores = FALSE;

/* True if any post-action scorekeepers are defined. */

int any_post_action_scores = FALSE;

/* True if any post-event scorekeepers are defined. */

int any_post_event_scores = FALSE;

/* True if any turn-specific scorekeepers are defined. */

int any_turn_specific_scores = FALSE;

/* The count of sides in the game when the last-side-wins scorekeeper
   first tested. */

static int num_sides_originally = -1;

/* Clear out any possible scorekeepers. */

void
init_scorekeepers()
{
    scorekeepers = NULL;
    numscorekeepers = 0;
    any_pre_turn_scores = FALSE;
    any_post_turn_scores = FALSE;
    any_post_action_scores = FALSE;
    any_post_event_scores = FALSE;
    any_turn_specific_scores = FALSE;
}

Scorekeeper *
create_scorekeeper()
{
    Scorekeeper *sk = (Scorekeeper *) xmalloc(sizeof(Scorekeeper));

    /* Initialize any nonzero fields. */
    sk->id = nextskid++;
    sk->when = lispnil;
    sk->who = lispnil;
    sk->knownto = lispnil;
    sk->trigger = lispnil;
    sk->body = lispnil;
    sk->record = lispnil;
    sk->notes = lispnil;
    sk->scorenum = -1;
    /* Init the initial score to a disallowed value, this keeps it off the
       side's scorecard. */
    sk->initial = -10001;
    sk->triggered = FALSE;
    /* Glue into the linked list of scorekeepers. */
    sk->next = scorekeepers;
    scorekeepers = sk;
    ++numscorekeepers;
    return sk;
}

/* Allocate and fill in the initial score records for each side.  This
   must happen after all scorekeepers have been defined. */

void
init_scores()
{
    int score;
    Side *side;
    Scorekeeper *sk;
    Obj *savedscores, *when;

    /* First count and index all the scorekeepers that maintain
       a numeric score. */
    numscores = 0;
    for_all_scorekeepers(sk) {
    	if (sk->initial != -10001) {
    	    sk->scorenum = numscores++;
    	}
    }
    /* Allocate an appropriately-sized scorecard for each side.  Note that a
       particular position in the scorecard might not apply to all sides. */
    for_all_sides(side) {
	/* Collect any Lisp object that might have been stashed here
	   while reading a game module. */
	savedscores = (Obj *) side->scores;
	side->scores = NULL;
	if (numscores > 0) {
	    side->scores = (short *) xmalloc(numscores * sizeof(short));
	    for_all_scorekeepers(sk) {
		score = sk->initial;
		/* Collect a saved score if there is one to collect. */
		if (savedscores != NULL
		    && savedscores != lispnil
		    && numberp(car(savedscores))) {
		    score = c_number(car(savedscores));
		    savedscores = cdr(savedscores);
		}
		side->scores[sk->scorenum] = score;
	    }
	}
    }
    /* Some kinds of scorekeepers are expensive to run, so we set flags to
       indicate directly that we need to make the check. */
    for_all_scorekeepers(sk) {
    	when = sk->when;
    	if (consp(when)) {
	    if (cdr(when) != lispnil) {
		any_turn_specific_scores = TRUE;
	    }
	    when = car(when);
	}
	if (match_keyword(when, K_BEFORE_TURN)) {
	     any_pre_turn_scores = TRUE;
	}
	if (when == lispnil || match_keyword(when, K_AFTER_TURN)) {
	     any_post_turn_scores = TRUE;
	}
	if (match_keyword(when, K_AFTER_ACTION)) {
	     any_post_action_scores = TRUE;
	}
	if (match_keyword(when, K_AFTER_EVENT)) {
	     any_post_event_scores = TRUE;
	}
    }
}

/* Test all the scorekeepers that should be run immediately before any
   side moves anything. */

void
check_pre_turn_scores()
{
    Side *side;
    Scorekeeper *sk;

    if (any_pre_turn_scores) {
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_BEFORE_TURN)) {
		for_all_sides(side) {
		    if (scorekeeper_applicable(side, sk)) {
			run_scorekeeper(side, sk); 
		    } else {
		    	Dprintf("sk %d not applicable to %s\n",
		    		sk->id, side_desig(side));
		    }
		}
	    }
	}
    }
}

/* Test all the scorekeepers that should be run only at the end of a turn. */

void
check_post_turn_scores()
{
    Side *side;
    Scorekeeper *sk;

    if (any_post_turn_scores) {
	for_all_scorekeepers(sk) {
	    Dprintf("Checking post-turn scorekeeper %d\n", sk->id);
	    if (sk->when == lispnil || match_keyword(sk->when, K_AFTER_TURN)) {
		/* Decide whether the scorekeeper applies to one side or to the whole game. */
	    	if (symbolp(sk->body)
	    	    && match_keyword(sk->body, K_LAST_SIDE_WINS)) {
			eval_sk_last_side_wins(sk);
	    	} else {
			for_all_sides(side) {
			    if (scorekeeper_applicable(side, sk)) {
				run_scorekeeper(side, sk); 
			    } else {
			    	Dprintf("sk %d not applicable to %s\n",
			    		sk->id, side_desig(side));
			    }
			}
		}
	    }
	}
    }
}

void
check_post_action_scores(unit, action, rslt)
Unit *unit;
Action *action;
int rslt;
{
    Side *side;
    Scorekeeper *sk;

    if (any_post_action_scores) {
	Dprintf("Checking post-action scorekeepers\n");
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_AFTER_ACTION)) {
		if (sk->trigger == lispnil || sk->triggered) {
		    for_all_sides(side) {
			if (scorekeeper_applicable(side, sk)) {
			    run_scorekeeper(side, sk);
		        } else {
		    	    Dprintf("sk %d not applicable to %s\n",
		    		    sk->id, side_desig(side));
			}
		    }
		}
	    }
	}
    }
}

void
check_post_event_scores(hevt)
HistEvent *hevt;
{
    Side *side;
    Scorekeeper *sk;

    if (any_post_event_scores) {
	Dprintf("Checking post-event scorekeepers\n");
	for_all_scorekeepers(sk) {
	    if (match_keyword(sk->when, K_AFTER_EVENT)) {
		if (sk->trigger == lispnil || sk->triggered) {
		    for_all_sides(side) {
			if (scorekeeper_applicable(side, sk)) {
			    run_scorekeeper(side, sk); 
		        } else {
		    	    Dprintf("sk %d not applicable to %s\n",
		    		    sk->id, side_desig(side));
			}
		    }
		}
	    }
	}
    }
}

/* This is what actually does the test and effect of the scorekeeper
   on the given side.  This can be expensive to run. */

void
run_scorekeeper(side, sk)
Side *side;
Scorekeeper *sk;
{
    eval_sk_form(side, sk, sk->body);
}

int
eval_sk_form(side, sk, form)
Side *side;
Scorekeeper *sk;
Obj *form;
{
    int val;
    char *formtype;
    Obj *test, *clauses, *clause, *rest;

    if (symbolp(form)) {
        if (match_keyword(form, K_LAST_SIDE_WINS)) {
            eval_sk_last_side_wins(sk);
	    return 0;
        } else if (boundp(form)) {
            return 0;
        } else {
            syntax_error(form, "scorekeeper body");
            return 0;
        }
    } else if (consp(form) && symbolp(car(form))) {
	formtype = c_string(car(form));
	switch (keyword_code(formtype)) {
	  case K_IF:
	    test = cadr(form);
	    if (eval_sk_test(side, sk, test)) {
	    	eval_sk_form(side, sk, car(cddr(form)));
	    }
	    break;
	  case K_COND:
	    /* Interpret the traditional cond form. */
	    for (clauses = cdr(form); clauses != lispnil; clauses = cdr(clauses)) {
		clause = car(clauses);
		test = car(clause);
	    	if (eval_sk_test(side, sk, test)) {
	    	    for (rest = cdr(clause); rest != lispnil; rest = cdr(rest)) {
	    		eval_sk_form(side, sk, car(rest));
	    	    }
	    	    /* Don't look at any more clauses. */
	    	    break;
	    	}
	    }
	    break;
	  case K_STOP:
	    run_error("game stopped by scorekeeper %d", sk->id);
	    break;
	  case K_WIN:
	    if (side->ingame)
	      side_wins(side, sk->id);
	    break;
	  case K_LOSE:
	    if (side->ingame)
	      side_loses(side, NULL, sk->id);
	    break;
	  case K_DRAW:
	    all_sides_draw();
	    break;
	  case K_END:
	    /* (should end the game, assign scores to all sides) */
	    break;
	  case K_SET:
	    /* (should only do if this actually *is* a numeric scorekeeper) */
	    side->scores[sk->scorenum] = eval_sk_form(side, sk, cadr(form));
	    return side->scores[sk->scorenum];
	  case K_ADD:
	    if (cdr(form) != lispnil) {
		val = eval_sk_form(side, sk, cadr(form));
	    } else {
	    	val = 1;
	    }
	    /* (should only do if this actually *is* a numeric scorekeeper) */
	    side->scores[sk->scorenum] += val;
	    return side->scores[sk->scorenum];
	  case K_SUM:
	    return sum_property(side, cdr(form));
	  default:
	    run_warning("unknown form type `%s' in scorekeeper %d", formtype, sk->id);
	}
	/* This is for those forms that don't really have values, but may appear in
	   a value-using context anyway. */
	return 0;
    } else if (numberp(form)) {
    	return c_number(form);
    } else {
        syntax_error(form, "scorekeeper body");
	return 0;
    }
}

int
sum_property(side, form)
Side *side;
Obj *form;
{
    int sum = 0, i, typevec[200];
    Obj *sidelist = lispnil, *typelist = lispnil, *prop = lispnil;
    Obj *filter = lispnil;
    Unit *unit;
    
    if (typelist != lispnil) {
    	/* (should decide overall type of objects first) */
	for (i = 0; i < 200; ++i) typevec[i] = FALSE;
    } else {
	for (i = 0; i < 200; ++i) typevec[i] = TRUE;
    }
    if (consp(form)) {
	prop = car(form);
	form = cdr(form);
    }
    if (sidelist != lispnil) {
    	/* (should let optional cadr designate some other side) */
    } else {
	if (1 /* iterating over units */) {
	    for_all_side_units(side, unit) {
    		if (in_play(unit)
    		    && typevec[unit->type]
    		    && (filter == lispnil || TRUE /*filter allows through*/)) {
		    if (prop != lispnil) {
			if (symbolp(prop)
			    && strcmp(c_string(prop), "point-value") == 0) {
			    sum += unit_point_value(unit);
			} else {
			}
		    } else {
			sum += 1;
		    }
		}
	    }
	}
    }   
    return sum;
}

int
unit_point_value(unit)
Unit *unit;
{
    Obj *val;

    /* Incomplete units are always worthless. */
    if (!completed(unit))
      return 0;
    /* Look for an extension property and use it as point value. */
    if (unit->hook != lispnil) {
	val = get_x_property_by_name(unit, "point-value");
	if (val != lispnil) {
	    val = eval(car(val));
	    if (numberp(val))
	      return c_number(val);
	    run_warning("non-numeric point value for %s", unit_desig(unit));
	    /* Just fall through and use point value for type. */
	}
    }
    return u_point_value(unit->type);
}

int
eval_sk_test(side, sk, form)
Side *side;
Scorekeeper *sk;
Obj *form;
{
    char *formtype;
    Obj *arg1 = lispnil, *arg2 = lispnil, *rest;

    if (consp(form)) {
	formtype = c_string(car(form));
	if (consp(cdr(form)))  arg1 = cadr(form);
	if (consp(cddr(form))) arg2 = car(cddr(form));
	switch (keyword_code(formtype)) {
	  case K_AND:
	    for (rest = cdr(form); rest != lispnil; rest = cdr(rest)) {
		if (!eval_sk_form(side, sk, car(rest))) return FALSE;
	    }
	    return TRUE;
	  case K_OR:
	    for (rest = cdr(form); rest != lispnil; rest = cdr(rest)) {
		if (eval_sk_form(side, sk, car(rest))) return TRUE;
	    }
	    return FALSE;
	  case K_NOT:
	    return (!eval_sk_form(side, sk, arg1));
	  case K_EQ:
	    return (eval_sk_form(side, sk, arg1) ==
		    eval_sk_form(side, sk, arg2));
	  case K_LT:
	    return (eval_sk_form(side, sk, arg1) <
		    eval_sk_form(side, sk, arg2));
	  case K_LE:
	    return (eval_sk_form(side, sk, arg1) <=
		    eval_sk_form(side, sk, arg2));
	  case K_GT:
	    return (eval_sk_form(side, sk, arg1) >
		    eval_sk_form(side, sk, arg2));
	  case K_GE:
	    return (eval_sk_form(side, sk, arg1) >=
		    eval_sk_form(side, sk, arg2));
	  default:
	    run_warning("not a proper test");
	    return FALSE;
	}
    } else if (symbolp(form)) {
    	eval_symbol(form);
    }
    return FALSE;
}

static void
eval_sk_last_side_wins(sk)
Scorekeeper *sk;
{
    Side *side2, *winner = NULL;
    Unit *unit;
    int numleft = 0, points;

    /* Compute the number of sides in the game the first time this
       scorekeeper is evaluated.  This is so we don't force the game
       to end because (for whatever reason) only one side was active
       from the beginning. */
    if (num_sides_originally < 0) {
    	num_sides_originally = 0;
	for_all_sides(side2)
	  if (side2->ingame)
	    ++num_sides_originally;
    }
    /* This is only meaningful in games with at least two sides. */
    if (num_sides_originally >= 2) {
	for_all_sides(side2) {
	    if (side2->ingame) {
		points = 0;
		for_all_side_units(side2, unit) {
		    if (in_play(unit) && completed(unit)) {
			points += unit_point_value(unit);
		    }
		}
		Dprintf("%s has %d points worth of units\n",
			side_desig(side2), points);
		if (points == 0) {
		    side_loses(side2, NULL, sk->id);
		} else {
		    ++numleft;
		    winner = side2;
		}
	    }
	}
	if (numleft == 1) {
	    side_wins(winner, sk->id);
	}
    }
}

/* Implement the effects of a side winning. */

void
side_wins(side, why)
Side *side;
int why;
{
    /* Nothing happens to the side's units or people. */
    side->status = 1;
    record_event(H_SIDE_WON, ALLSIDES, side_number(side), why);
    remove_side_from_game(side);
}

/* Implement the effects of a side losing. */

void
side_loses(side, side2, why)
Side *side, *side2;
int why;
{
    int x, y, s, s2, ux, uy;
    Unit *unit;

    if (side == side2)
      run_error("losing to self");
    if (side2 != NULL && !side2->ingame)
      run_error("losing to side not in game");
    /* Dispose of all of a side's units. */
    for_all_units(unit) {
	if (unit->side == side) {
		if (in_play(unit)) {
		    ux = unit->x;  uy = unit->y;
		    unit_changes_side(unit, side2, H_UNIT_CAPTURED, H_SIDE_LOST);
		    /* Everybody gets to see this change. */
		    all_see_cell(ux, uy);
		} else {
		    /* Even out-of-play units need to have their side set. */
		    set_unit_side(unit, side2);
		}
	}
    }
    /* The people also change sides. */
    if (people_sides_defined()) {
	s = side_number(side);
	s2 = side_number(side2);
	for_all_cells(x, y) {
	    if (people_side_at(x, y) == s) {
		set_people_side_at(x, y, s2);
	    } 
	}
    }
    /* Reset view coverage everywhere. */
    /* (should move prior to unit loss perhaps?  then can see final positions) */
    if (!g_see_all()) {
	for_all_cells(x, y) {
	    if (0 /* some reason to retain coverage */) {
	    } else {
		set_cover(side, x, y, 0);
	    }
	}
    }
    /* Add the mark of shame itself. */
    side->status = -1;
    record_event(H_SIDE_LOST, ALLSIDES, side_number(side), why);
    remove_side_from_game(side);
}

void
all_sides_draw()
{
    Side *side;

    for_all_sides(side) {
    	if (side->ingame) {
	    side->status = 0;
	    record_event(H_SIDE_WITHDREW, ALLSIDES, side_number(side));
	    remove_side_from_game(side);
	}
    }
}

/* Add scorefile handling here. */

/* (should only be done if some scorekeeper says how?) */

void
record_into_scorefile()
{
    char *filename = SCOREFILE;
    FILE *fp;

    if (!empty_string(g_scorefile_name()))
      filename = g_scorefile_name();
    /* (should assemble a path to specifiable scorefile dir) */
    fp = fopen(filename, "a");
    if (fp != NULL) {
	fprintf(fp, "(game");
	fprintf(fp, " \"%s\"", mainmodule->name);
	/* (should write players, scores, game facts, etc) */
	fprintf(fp, ")\n");
	fclose(fp);
    }
}

/* This is a general test of whether the given side should be trying
   to win or should just be goofing off. */

int
should_try_to_win(side)
Side *side;
{
    Scorekeeper *sk;

    for_all_scorekeepers(sk) {
	if (1 /* sk applies to this side? */) {
	    return TRUE;
	}
    }
    return FALSE;
}
