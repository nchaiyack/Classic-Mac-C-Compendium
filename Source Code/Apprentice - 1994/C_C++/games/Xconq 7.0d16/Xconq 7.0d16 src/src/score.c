/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This file manages the scorekeepers in a game. */

#include "conq.h"

/* This is true when the given side should be tested against the given scorekeeper. */

#define scorekeeper_applicable(side,sk)  \
  ((side)->ingame && ((sk)->who == lispnil || 1 /* in side mask */))

/* The head of the list of scorekeepers. */

Scorekeeper *scorekeepers = NULL;

/* The total number of scorekeepers defined. */

int numscorekeepers = 0;

int nextskid = 1;

/* The number of scorekeepers maintaining numeric scores. */

int numscores = 0;

/* True if any pre-turn scorekeepers are defined. */

int anypreturnscores = FALSE;

/* True if any post-turn scorekeepers are defined. */

int anypostturnscores = FALSE;

/* True if any post-action scorekeepers are defined. */

int anypostactionscores = FALSE;

/* True if any post-event scorekeepers are defined. */

int anyposteventscores = FALSE;

/* True if any turn-specific scorekeepers are defined. */

int anyturnspecificscores = FALSE;

/* the winning side (allies are also winners) */

Side *winner = NULL;

/* Clear out any possible scorekeepers. */

init_scorekeepers()
{
    scorekeepers = NULL;
    numscorekeepers = 0;
    anypreturnscores = FALSE;
    anypostturnscores = FALSE;
    anypostactionscores = FALSE;
    anyposteventscores = FALSE;
    anyturnspecificscores = FALSE;
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

init_scores()
{
    Side *side;
    Scorekeeper *sk;
    Obj *when;

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
	if (numscores > 0) {
	    side->scores = (short *) xmalloc(numscores * sizeof(short));
	    for_all_scorekeepers(sk) {
		side->scores[sk->scorenum] = sk->initial;
	    }
	}
    }
    /* Some kinds of scorekeepers are expensive to run, so we set flags to
       indicate directly that we need to make the check. */
    for_all_scorekeepers(sk) {
    	when = sk->when;
    	if (consp(when)) {
	    if (cdr(when) != lispnil) {
		anyturnspecificscores = TRUE;
	    }
	    when = car(when);
	}
	if (match_keyword(when, K_BEFORE_TURN)) {
	     anypreturnscores = TRUE;
	}
	if (when == lispnil || match_keyword(when, K_AFTER_TURN)) {
	     anypostturnscores = TRUE;
	}
	if (match_keyword(when, K_AFTER_ACTION)) {
	     anypostactionscores = TRUE;
	}
	if (match_keyword(when, K_AFTER_EVENT)) {
	     anyposteventscores = TRUE;
	}
    }
}

/* Test all the scorekeepers that should be run immediately before any
   side moves anything. */

check_pre_turn_scores()
{
    Side *side;
    Scorekeeper *sk;

    if (anypreturnscores) {
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

check_post_turn_scores()
{
    Side *side;
    Scorekeeper *sk;

    if (anypostturnscores) {
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

check_post_action_scores(unit, action, rslt)
Unit *unit;
Action *action;
int rslt;
{
    Side *side;
    Scorekeeper *sk;

    if (anypostactionscores) {
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

check_post_event_scores(hevt)
HistEvent *hevt;
{
    Side *side;
    Scorekeeper *sk;

    if (anyposteventscores) {
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

run_scorekeeper(side, sk)
Side *side;
Scorekeeper *sk;
{
    eval_sk_form(side, sk, sk->body);
}

eval_sk_form(side, sk, form)
Side *side;
Scorekeeper *sk;
Obj *form;
{
    int val;
    char *formtype;
    Obj *test;

    if (symbolp(form)) {
        if (match_keyword(form, K_LAST_SIDE_WINS)) {
            return eval_sk_last_side_wins(sk);
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
	    /* (interpret usual cond form) */
/*
	    test = cadr(form);
	    if (eval_sk_test(side, sk, test)) {
	    	eval_sk_form(side, sk, car(cddr(form)));
	    }
*/
	    break;
	  case K_STOP:
	    /* (should stop the game altogether, a semi-error situation) */
	    break;
	  case K_WIN:
	    side_wins(side);
	    break;
	  case K_LOSE:
	    side_loses(side, NULL);
	    break;
	  case K_DRAW:
	    /* (should declare a draw) */
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
	/* This is for those forms that are really "void", but may appear in
	   a value-using context anyway. */
	return 0;
    } else if (numberp(form)) {
    	return c_number(form);
    } else {
        syntax_error(form, "scorekeeper body");
	return 0;
    }
}

sum_property(side, form)
Side *side;
Obj *form;
{
    int sum = 0, i, typevec[200];
    Obj *sidelist = lispnil, *typelist = lispnil, *prop = lispnil, *filter = lispnil;
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
    		    && (filter == lispnil || TRUE /* filter allows through */)) {
    			if (prop != lispnil) {
    				if (symbolp(prop) && strcmp(c_string(prop), "point-value") == 0) {
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

unit_point_value(unit)
Unit *unit;
{
    Obj *val;

    /* (should return 1 if all point values are zero) */
    /* Look for an extension property and use it as point value. */
    if (unit->hook != lispnil) {
	val = (Obj *) get_x_property_by_name(unit, "point-value");
	if (val != lispnil) {
	    val = eval(car(val));
	    if (numberp(val)) return c_number(val);
	    run_warning("non-numeric point value for %s", unit_desig(unit));
	}
    }
    return u_point_value(unit->type);
}

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
	    run_warning(form, "not a proper test");
	    return FALSE;
	}
    } else if (symbolp(form)) {
    	eval_symbol(form);
    }
    return FALSE;
}

eval_sk_last_side_wins(sk)
Scorekeeper *sk;
{
    Side *side2, *winner = NULL;
    Unit *unit;
    int numleft = 0, points;

    /* This is only meaningful in games with at least two sides. */
    if (num_sides_in_game() >= 2) {
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
		    side_loses(side2, NULL);
		} else {
		    ++numleft;
		    winner = side2;
		}
	    }
	}
	if (numleft == 1) {
	    side_wins(winner);
	}
    }
}

num_sides_in_game()
{
    int rslt = 0;
    Side *side;

    for_all_sides(side) {
	if (side->ingame) ++rslt;
    }
    return rslt;
}

/* Implement the effects of a side winning. */

side_wins(side)
Side *side;
{
    if (!side->ingame) return;
    /* Nothing happens to the side's units or people. */
    side->status = 1;
    record_event(H_SIDE_WON, -1, side_number(side));
    remove_side_from_game(side);
}

/* Implement the effects of a side losing. */

side_loses(side, side2)
Side *side, *side2;
{
    int x, y, s, s2, ux, uy;
    Unit *unit;

    /* This should never occur (other code needs cleanup) */
    if (!side->ingame) return;
    if (side == side2) run_error("can't lose to ourselves");
    if (side2 != NULL && !side2->ingame) run_error("losing to side not in game");
    /* Dispose of all of a side's units. */
    while (side->unithead->next != side->unithead) {
	unit = side->unithead->next;
	if (alive(unit)) {
	    ux = unit->x;  uy = unit->y;
	    unit_changes_side(unit, side2, H_UNIT_CAPTURED, H_SIDE_LOST);
	    /* Everybody gets to see this change. */
	    all_see_hex(ux, uy);
	} else {
	    /* Even dead units need to have their side set. */
	    set_unit_side(unit, side2);
	}
	if (unit->side == side) run_error("couldn't get rid of unit");
    }
    /* The people also change sides. */
    if (people_sides_defined()) {
	s = side_number(side);
	s2 = side_number(side2);
	for_all_hexes(x, y) {
	    if (people_side_at(x, y) == s) {
		set_people_side_at(x, y, s2);
	    } 
	}
    }
    /* Add the mark of shame itself. */
    side->status = -1;
    record_event(H_SIDE_LOST, -1, side_number(side));
    remove_side_from_game(side);
}

/* Add scorefile handling here. */

/* (only done if some scorekeeper says how?) */

record_into_scorefile()
{
}

/* This is a general test of whether the given side should be trying
   to win or should just be goofing off. */

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
