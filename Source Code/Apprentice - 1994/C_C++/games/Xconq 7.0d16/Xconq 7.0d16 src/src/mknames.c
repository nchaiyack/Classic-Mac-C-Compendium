/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Naming is a special class of init method, since it may run both during
   init and throughout a game.  Name generation has a very strong influence
   on the flavor of a game, so it has some extra flexibility, including
   a capability to use a simple context-free grammar to generate names. */

#include "conq.h"

int totalsideweights;

char *run_namer();
char *name_from_grammar();

init_namers()
{
    totalsideweights = 0;
}

/* The default side name list has the form
   (((name "A") (noun "Aian")) ... ). */

set_g_side_lib_default()
{
    int i;
    Obj *tmp;
    Obj *sidenamelist = lispnil, *sidenamelistend = lispnil;

    for (i = 0; i < MAXSIDES; ++i) {
	sprintf(spbuf, "%c", 'A' + i);
	sprintf(tmpbuf, "%cian", 'A' + i); /* should be in nlang.c? */
	tmp = lispnil;
	tmp = cons(cons(intern_symbol(keyword_name(K_NAME)),
			cons(new_string(copy_string(spbuf)), lispnil)),
		   tmp);
	tmp = cons(cons(intern_symbol(keyword_name(K_NOUN)),
			cons(new_string(copy_string(tmpbuf)), lispnil)),
		   tmp);
	tmp = cons(tmp, lispnil);
	if (sidenamelist == lispnil) {
	    sidenamelist = tmp;
	} else {
	    sidenamelistend->v.cons.cdr = tmp;
	}
	sidenamelistend = tmp;
    }
    set_g_side_lib(sidenamelist);
}

/* Pick a side name that is not already being used. */

make_up_side_name(side)
Side *side;
{
    int uniq = FALSE, tries = 0, n, sofar;
    char *str;
    Obj *sidelib, *subobj, *subelts, *lis, *filler;
    Side *side2;

    if ((sidelib = g_side_lib()) == lispnil) return;
    /* (what if all weights were explicitly 0?) */
    if (totalsideweights == 0) {
	for (lis = sidelib; lis != lispnil; lis = cdr(lis)) {
	    totalsideweights +=
	      (numberp(car(car(lis))) ? c_number(car(car(lis))) : 1);
	}
    }
    filler = lispnil;
    while (tries++ < 100 * numsides) {
	n = xrandom(totalsideweights);
	sofar = 0;
	subobj = lispnil;
	for (lis = sidelib; lis != lispnil; lis = cdr(lis)) {
	    sofar += (numberp(car(car(lis))) ? c_number(car(car(lis))) : 1);
	    if (sofar > n) {
		subobj = car(lis);
		break;
	    }
	}
	/* (should scan and preprocess subobj before using it) */
	/* Remove the weighting if present. */
	if (numberp(car(subobj))) {
	    subobj = cdr(subobj);
	}
	uniq = TRUE;
	for (subelts = subobj; subelts != lispnil; subelts = cdr(subelts)) {
	    if (stringp(cadr(car(subelts)))) {
		if (name_in_use(side, c_string(cadr(car(subelts))))) {
		    uniq = FALSE;
		    break;
		}
	    }
	}
	if (uniq) {
		filler = subobj;
		break;
	}
    }
    /* Now fill the side from the chosen obj - no effect if it is nil. */
    fill_in_side(side, filler, FALSE);
}

/* This tests whether a given string is already being used by a side. */

name_in_use(side, str)
Side *side;
char *str;
{
    Side *side2;

    if (str == NULL || strlen(str) == 0) return FALSE;
    for_all_sides(side2) {
	if (side2 != side) {
	    if ((side2->name && strcmp(str, side2->name) == 0)
		|| (side2->noun && strcmp(str, side2->noun) == 0)
		|| (side2->pluralnoun && strcmp(str, side2->pluralnoun) == 0)
		|| (side2->adjective && strcmp(str, side2->adjective) == 0)
		) return TRUE;
	}
    }
    return FALSE;
}

/* Method to add names to units that want them and don't have them already. */

name_units_randomly()
{
    Unit *unit;
    Side *loopside;

    /* There's never any reason not to run this method. */
    /* (should this announce progress?) */
    for_all_units(loopside, unit) {
	make_up_unit_name(unit);
	assign_unit_number(unit);
    }
}

/* Given a unit, return its naming method if it has one. */

char *
unit_namer(unit)
Unit *unit;
{
    Side *side;

    if (unit == NULL) return NULL;
    /* Look for and return a side-specific namer if found. */
    side = (unit->side ? unit->side : indepside);
    if (side->unitnamers != NULL && side->unitnamers[unit->type] != NULL) {
    	return side->unitnamers[unit->type];
    }
    return u_namer(unit->type);
}

/* Generate a name for a unit, using an appropriate method.

   It is possible (in fact encouraged) to add cool new unit name generation
   methods in here, especially when the grammar-based or thematic methods
   don't give the desired results. */

char *
propose_unit_name(unit)
Unit *unit;
{
    int u;
    char *method;

    if (unit == NULL) return NULL;
    u = unit->type;
    method = unit_namer(unit);
    if (method == NULL || strcmp(method, "") == 0) {
	/* Nothing to work with. */
    } else if (boundp(intern_symbol(method))) {
	return run_namer(symbol_value(intern_symbol(method)));
    } else {
	/* Do builtin naming methods. */
	switch (keyword_code(method)) {
	  case K_JUNKY:
	    /* Kind of a bizarre thing, but flavorful sometimes. */
	    if (unit->side) {
		sprintf(spbuf, "%c%c-%s-%02d",
			uppercase(unit->side->name[0]),
			uppercase(unit->side->name[1]),
			utype_name_n(u, 3), unit->number);
	    } else {
		sprintf(spbuf, "%s-%d", utype_name_n(u, 3), unit->id);
	    }
	    return copy_string(spbuf);
	  default:
	    init_warning("No naming method `%s', ignoring", method);
	    break;
	}
    }
    return NULL;
}

/* This names only units that do not already have names. */

make_up_unit_name(unit)
Unit *unit;
{
    if (unit == NULL || unit->name != NULL) return;
    /* (should check that proposed name is not in use by matching side and type?) */
    unit->name = propose_unit_name(unit);
}

/* Unit numbering only happens to designated types that are on a side. */

assign_unit_number(unit)
Unit *unit;
{
    if (u_assign_number(unit->type)
	&& unit->side != NULL
	&& unit->number == 0) {
	/* Give it the next available number and increment. */
	unit->number = (unit->side->counts)[unit->type]++;
    } else {
	/* Note that this will erase any already-assigned number,
	   if the type is one that is not supposed to be numbered. */
	unit->number = 0;
    }
}

/* Given a naming method, run it and get back a string. */

char *
run_namer(namer)
Obj *namer;
{
    int ix;
    Obj *prev;
    Obj *rslt;
    Obj *code = namer->v.meth.code;
    Obj *type;

    if (!consp(code)) return "?format?";
    type = car(code);
    if (!symbolp(type)) return "?type?";
    switch (keyword_code(c_string(type))) {
      case K_JUNKY:
      case K_RANDOM:
	if (namer->v.meth.len > 1) {
	    ix = xrandom(namer->v.meth.len - 1) + 1;
	    prev = code;
	    while (--ix) prev = cdr(prev);
	    rslt = cadr(prev);
	    /* Splice out our desired name. */
	    prev->v.cons.cdr = cddr(prev);
	    --(namer->v.meth.len);
	    return c_string(rslt);
	} else {
	    return "?no more names?";
	}
	break;
      case K_GRAMMAR:
	return name_from_grammar(code);
      default:
	return "?method?";
    }
}

static int maxdepth;

char *
name_from_grammar(grammar)
Obj *grammar;
{
    int i, len;
    char rslt[1000];  /* not really safe... */
    Obj *root = cadr(grammar);
    Obj *depth = caddr(grammar);
    Obj *rules = cdr(cddr(grammar));

    maxdepth = 5;
    if (numberp(depth)) maxdepth = c_number(depth);
    sprintf(rslt, "");
    gen_name(root, rules, 0, rslt);
    /* This should be optional maybe. */
    rslt[0] = uppercase(rslt[0]);
    return copy_string(rslt);
}

/* Given a nonterminal and a set of rules, find and apply the right rule. */

gen_name(nonterm, rules, depth, rslt)
Obj *nonterm, *rules;
int depth;
char *rslt;
{
    Obj *lis;

    for (lis = rules; lis != lispnil; lis = cdr(lis)) {
	if (equal(nonterm, car(car(lis)))) {
	    gen_from_rule(cadr(car(lis)), rules, depth, rslt);
	    return;
	}
    }
    if (symbolp(nonterm)
	&& boundp(nonterm)
	&& methodp(symbol_value(nonterm))) {
	strcat(rslt, run_namer(symbol_value(nonterm)));
    } else {
	/* Assume that the purported nonterm symbol is actually a terminal. */
	strcat(rslt, c_string(nonterm));
    }
}

/* Given a rule body, decide how to add to the output string.  This may
   recurse, so there is a limit check. */

gen_from_rule(rule, rules, depth, rslt)
Obj *rule, *rules;
int depth;
char *rslt;
{
    Obj *lis;
    int total, num, oldlen;

    if (depth >= maxdepth) return -1;

    switch (rule->type) {
      case NUMBER:
	break;  /* ignore for now.. */
      case SYMBOL:
	gen_name(rule, rules, depth, rslt);
	break;
      case STRING:
	strcat(rslt, c_string(rule));
	break;
      case CONS:
        if (symbolp(car(rule))) {
         switch (keyword_code(c_string(car(rule)))) {
	  case K_OR:
	    /* weighted selection */
	    total = 0;
	    for (lis = cdr(rule); lis != lispnil; lis = cdr(lis)) {
		if (numberp(car(lis))) {
		    total += c_number(car(lis));
		    lis = cdr(lis);
		} else {
		    total += 1;
		}
	    }
	    /* We now know the range, make a random index into it. */
	    num = xrandom(total);
	    /* Go through again to figure out which choice the index refs. */
	    total = 0;
	    for (lis = cdr(rule); lis != lispnil; lis = cdr(lis)) {
		if (numberp(car(lis))) {
		    total += c_number(car(lis));
		    lis = cdr(lis);
		} else {
		    total += 1;
		}
		if (total > num) {
		    gen_from_rule(car(lis), rules, depth + 1, rslt);
		    return;
		}
	    }
	    break;
	  case K_ANY:
	    /* uniform selection */
	    strcat(rslt, c_string(elt(cdr(rule),
				      xrandom(length(cdr(rule))))));
	    break;
	  case K_CAPITALIZE:
	    oldlen = strlen(rslt);
	    gen_name(rule, rules, depth, rslt);
	    rslt[oldlen] = uppercase(rslt[oldlen]);
	    break;
	  default:
	    /* Nested subsequence. */
	    for (lis = rule; lis != lispnil; lis = cdr(lis)) {
		gen_from_rule(car(lis), rules, depth + 1, rslt);
	    }
	 }
	} else {
	 	/* syntax error */
	}
	break;
    }
}
