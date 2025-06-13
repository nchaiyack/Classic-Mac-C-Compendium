/* Interface-independent natural language handling for Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file should be entirely replaced for non-English Xconq. */
/* (One way to do this would be to call this file "nlang-en.c", then
   symlink this or nlang-fr.c, etc to nlang.c when configuring;
   similarly for help.c.) */

#include "conq.h"

static char *usual_date_string PROTO ((int date));

static int gain_count PROTO ((Side *side, int u, int r));
static int loss_count PROTO ((Side *side, int u, int r));
static int atkstats PROTO ((Side *side, int a, int d));
static int hitstats PROTO ((Side *side, int a, int d));

int basehour;
int baseday = -1;
int basemonth = 0;
int baseyear = 1900;

/* Short names of directions. */

char *dirnames[] = DIRNAMES;

char *unitbuf = NULL;

char *past_unitbuf = NULL;

static char *side_short_title = NULL;

#define NUMGAINREASONS 3

static char *gain_reason_names[] = { "Ini", "Bld", "Cap" };

#define NUMLOSSREASONS 3

static char *loss_reason_names[] = { "Cbt", "Stv", "Dis" };

/* (should use an enum for date step names) */

int calendartype = -1;

char *datebuf = NULL;

char *turnname;
char *datestepname;
int datestep;

char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???" };

/* This is the number of types to mention by name; any others will
   just be included in the count of missing images. */

#define NUMTOLIST 5

static char *missinglist = NULL;

/* This array allows for counting up to 4 classes of missing images. */

static int missing[4];

static int totlisted = 0;

/* Pad a given buffer with blanks out to the given position. */

void
pad_out(buf, n)
char *buf;
int n;
{
    int i, len = strlen(buf);

    if (n < 1)
      return;
    for (i = len; i < n; ++i) {
	buf[i] = ' ';
    }
    buf[n - 1] = '\0';
}

char *
short_side_title(side)
Side *side;
{
    if (side_short_title == NULL)
      side_short_title = xmalloc(BUFSIZE);
    if (side == NULL) {
	return " - ";
    } else if (side->name) {
	return side->name;
    } else if (side->pluralnoun) {
	sprintf(side_short_title, "The %s", side->pluralnoun);
    } else if (side->noun) {
	sprintf(side_short_title, "The %s", plural_form(side->noun));
    } else if (side->adjective) {
	sprintf(side_short_title, "The %s side", side->adjective);
    } else {
	return " - ";
    }
    return side_short_title;
}

char *
shortest_side_title(side2, buf)
Side *side2;
char *buf;
{
    if (side2 == NULL) {
	return "-";
    } else if (side2->name) {
	return side2->name;
    } else if (side2->adjective) {
	return side2->adjective;
    } else if (side2->noun) {
	return side2->noun;
    } else {
	sprintf(buf, "(#%d)", side_number(side2));
    }
    return buf;
}

/* (should let NULL observer be assumed objective observer) */

char *
rel_short_side_title(side, side2, n)
Side *side, *side2;
int n;
{
    return "???";
}

char *
rel_long_side_title(side, side2)
Side *side, *side2;
{
    return "???";
}

char *
narrative_side_desc(side, side2)
Side *side, *side2;
{
    return "???";
}

char *
narrative_side_desc_first(side, side2)
Side *side, *side2;
{
    return "???";
}

char *
long_player_title(buf, player, thisdisplayname)
char *buf, *thisdisplayname;
Player *player;
{
    if (player == NULL) {
	sprintf(buf, "");
    } else if (player->displayname != NULL) {
	if (thisdisplayname != NULL && strcmp(player->displayname, thisdisplayname) == 0) {
	    sprintf(buf, "You");
	} else {
	    sprintf(buf, "%s", player->displayname);
	}
	if (player->aitypename != NULL) {
	    tprintf(buf, "(+ AI %s)", player->aitypename);
	}
    } else if (player->aitypename != NULL) {
	sprintf(buf, "AI %s", player->aitypename);
    } else {
	sprintf(buf, "-");
    }
    return buf;
}

char *
short_player_title(buf, player, thisdisplayname)
char *buf, *thisdisplayname;
Player *player;
{
    return long_player_title(buf, player, thisdisplayname);
}

void
side_and_type_name(buf, side, u, side2)
char *buf;
Side *side, *side2;
int u;
{
    /* Decide how to identify the side. */
    if (side2 == NULL) {
	sprintf(buf, "independent ");
    } else if (side == side2) {
	sprintf(buf, "your ");
    } else {
	/* (this could be more elaborate) */
	sprintf(buf, "%s ",
		(side2->adjective ? side2->adjective :
		 (side2->noun ? side2->noun : "?")));
    }
    /* Glue the pieces together and return it. */
    strcat(buf, u_type_name(u));
}

/* Build a short phrase describing a given unit to a given side,
   basically consisting of indication of unit's side, then of unit itself. */

char *
unit_handle(side, unit)
Side *side;
Unit *unit;
{
    char *utypename;
    Side *side2;

    if (unitbuf == NULL)
      unitbuf = xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    /* If this unit represents "yourself", say so. */
    if (side != NULL && unit == side->selfunit)
      return "you";
#if 0 /* how to do these days? */
    /* Sometimes a unit's description should be its name alone. */
    if (u_name_format(unit->type) == 2 && unit->name)
      return unit->name;
#endif
    /* Decide how to identify the side. */
    side2 = unit->side;
    if (side2 == NULL) {
	sprintf(unitbuf, "the ");
    } else if (side2 == side) {
	sprintf(unitbuf, "your ");
    } else {
	sprintf(unitbuf, "the %s ",
		(side2->adjective ? side2->adjective :
		 (side2->noun ? side2->noun : side_desig(side2))));
    }
    /* Now add the unit's unique description. */
    /* (Should this ever use short name?) */
    utypename = u_type_name(unit->type);
    if (unit->name) {
	tprintf(unitbuf, "%s %s", utypename, unit->name);
    } else if (unit->number > 0) {
	tprintf(unitbuf, "%d%s %s",
		unit->number, ordinal_suffix(unit->number), utypename);
    } else {
	tprintf(unitbuf, "%s", utypename);
    }
    return unitbuf;
}

/* Shorter unit description omits side name, but uses same buffer. */

char *
short_unit_handle(unit)
Unit *unit;
{
    int u;

    if (unitbuf == NULL)
      unitbuf = xmalloc(BUFSIZE);
    if (unit == NULL)
      return "???";
    if (!alive(unit)) {
    	sprintf(unitbuf, "dead #%d", unit->id);
        return unitbuf;
    }
    u = unit->type;
    if (!empty_string(unit->name)) {
	sprintf(unitbuf, "%s", unit->name);
    } else if (!empty_string(u_short_name(u))) {
	sprintf(unitbuf, "%d%s %s",
		unit->number, ordinal_suffix(unit->number), u_short_name(u));
    } else {
	sprintf(unitbuf, "%d%s %s",
		unit->number, ordinal_suffix(unit->number), u_type_name(u));
    }
    return unitbuf;
}

void
name_or_number(unit, buf)
Unit *unit;
char *buf;
{
    if (unit->name) {
	sprintf(buf, "%s", unit->name);
    } else if (unit->number > 0) {
	sprintf(buf, "%d%s", unit->number, ordinal_suffix(unit->number));
    } else {
	sprintf(buf, "");
    }
}

/* Build a short phrase describing a given past unit to a given side,
   basically consisting of indication of unit's side, then of unit itself. */

char *
past_unit_handle(side, past_unit)
Side *side;
PastUnit *past_unit;
{
    char *utypename;
    Side *side2;

    if (past_unitbuf == NULL)
      past_unitbuf = xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (past_unit == NULL)
      return "???";
    /* Decide how to identify the side. */
    side2 = past_unit->side;
    if (side2 == NULL) {
	sprintf(past_unitbuf, "the ");
    } else if (side2 == side) {
	sprintf(past_unitbuf, "your ");
    } else {
	sprintf(past_unitbuf, "the %s ",
		(side2->adjective ? side2->adjective :
		 (side2->noun ? side2->noun : side_desig(side2))));
    }
    /* Now add the past_unit's unique description. */
    /* (Should this ever use short name?) */
    utypename = u_type_name(past_unit->type);
    if (past_unit->name) {
	tprintf(past_unitbuf, "%s %s", utypename, past_unit->name);
    } else if (past_unit->number > 0) {
	tprintf(past_unitbuf, "%d%s %s",
		past_unit->number, ordinal_suffix(past_unit->number), utypename);
    } else {
	tprintf(past_unitbuf, "%s", utypename);
    }
    return past_unitbuf;
}

/* Shorter past_unit description omits side name, but uses same buffer. */

char *
short_past_unit_handle(past_unit)
PastUnit *past_unit;
{
    int u;

    if (past_unitbuf == NULL)
      past_unitbuf = xmalloc(BUFSIZE);
    if (past_unit == NULL)
      return "???";
    u = past_unit->type;
    if (!empty_string(past_unit->name)) {
	sprintf(past_unitbuf, "%s", past_unit->name);
    } else if (!empty_string(u_short_name(u))) {
	sprintf(past_unitbuf, "%d%s %s",
		past_unit->number, ordinal_suffix(past_unit->number), u_short_name(u));
    } else {
	sprintf(past_unitbuf, "%d%s %s",
		past_unit->number, ordinal_suffix(past_unit->number), u_type_name(u));
    }
    return past_unitbuf;
}

void
past_name_or_number(past_unit, buf)
PastUnit *past_unit;
char *buf;
{
    if (past_unit->name) {
	sprintf(buf, "%s", past_unit->name);
    } else if (past_unit->number > 0) {
	sprintf(buf, "%d%s", past_unit->number, ordinal_suffix(past_unit->number));
    } else {
	sprintf(buf, "");
    }
}

/* Given a unit and optional type u, summarize construction status
   and timing. */

void
construction_desc(buf, unit, u)
char *buf;
Unit *unit;
int u;
{
    int est, u2;
    char ubuf[10], tmpbuf[100];
    Task *task;
    Unit *unit2;

    if (u != NONUTYPE) {
	est = est_completion_time(unit, u);
	if (est >= 0) {
	    sprintf(ubuf, "[%2d] ", est);
	} else {
	    strcpy(ubuf, " --  ");
	}
    } else {
	strcpy(ubuf, "");
    }
    name_or_number(unit, tmpbuf);
    sprintf(buf, "%s%s %s", ubuf, u_type_name(unit->type), tmpbuf);
    pad_out(buf, 20);
    if (unit->plan
	&& unit->plan->tasks) {
      task = unit->plan->tasks;
      if (task->type == TASK_BUILD) {
	u2 = task->args[0];
	tprintf(buf, " %s ", (is_unit_type(u2) ? u_type_name(u2) : "?"));
	unit2 = find_unit(task->args[1]);
	if (in_play(unit2) && unit2->type == u2) {
	    tprintf(buf, "%d/%d done ", unit2->cp, u_cp(unit2->type));
	}
	tprintf(buf, "(%d of %d)", task->args[2] + 1, task->args[3]);
      } else if (task->type == TASK_RESEARCH) {
	u2 = task->args[0];
        if (is_unit_type(u2)) {
		tprintf(buf, " %s tech %d/%d",
			u_type_name(u2), unit->side->tech[u2], task->args[1]);
        }
      }
    }
}

void
constructible_desc(buf, side, u, unit)
char *buf;
Side *side;
int u;
Unit *unit;
{
    char estbuf[20];
    char techbuf[50];
    int est, tp, num;

    if (unit != NULL) {
	est = est_completion_time(unit, u);
    	if (est >= 0) {
	    sprintf(estbuf, "[%2d] ", est);
	    if (uu_tp_to_build(unit->type, u) > 0) {
		tp = (unit->tooling ? unit->tooling[u] : 0);
		tprintf(estbuf, "(%2d) ", tp);
	    }
    	} else {
	    strcpy(estbuf, " --  ");
    	}
    } else {
    	strcpy(estbuf, "");
    }
    if (u_tech_max(u) > 0) {
    	sprintf(techbuf, "[T %d/%d/%d] ", side->tech[u], u_tech_to_build(u), u_tech_max(u));
    } else {
    	strcpy(techbuf, "");
    }
    sprintf(buf, "%s%s%-16.16s", estbuf, techbuf, u_type_name(u));
    num = num_units_in_play(side, u);
    if (num > 0) {
	tprintf(buf, "  %3d", num);
    }
    num = num_units_incomplete(side, u);
    if (num > 0) {
	tprintf(buf, "(%d)", num);
    }
}

int
est_completion_time(unit, u2)
Unit *unit;
int u2;
{
    if (uu_acp_to_create(unit->type, u2) < 1)
      return (-1);
    return normal_completion_time(unit->type, u2);
}

void
historical_event_date_desc(hevt, buf)
HistEvent *hevt;
char *buf;
{
    sprintf(buf, "%d: ", hevt->startdate);
}

int find_event_type PROTO ((Obj *sym));

int
find_event_type(sym)
Obj *sym;
{
    int i;

    for (i = 0; hevtdefns[i].name != NULL; ++i) {
	if (strcmp(c_string(sym), hevtdefns[i].name) == 0)
	  return i;
    }
    return -1;
}

/* (should reindent) */
void
historical_event_desc(side, hevt, buf)
Side *side;
HistEvent *hevt;
char *buf;
{
    int data0 = hevt->data[0];
    int data1 = hevt->data[1];
    Obj *rest, *pattern, *text;
    Unit *unit, *unit2;
    PastUnit *pastunit, *pastunit2;
    Side *side2;
    
    if (g_event_messages() != lispnil) {
	for (rest = g_event_messages(); rest != lispnil; rest = cdr(rest)) {
	    if (consp(car(rest))) {
		pattern = car(car(rest));
		if (symbolp(pattern)
		    && find_event_type(pattern) == hevt->type) {
		    text = cadr(car(rest));
		    sprintf(buf, c_string(text));
		    return;
		} else if (consp(pattern)
			   && symbolp(car(pattern))
			   && find_event_type(car(pattern)) == hevt->type
			   /* && args match pattern args */
			   ) {
		    text = cadr(car(rest));
		    sprintf(buf, c_string(text));
		    return;
		}
	    }
	}
    }
    /* Generate a default description of the event. */
    switch (hevt->type) {
      case H_LOG_STARTED:
	sprintf(buf, "we started recording events");
	break;
      case H_LOG_ENDED:
	sprintf(buf, "we stopped recording events");
	break;
      case H_GAME_STARTED:
	sprintf(buf, "we started the game");
	break;
      case H_GAME_SAVED:
	sprintf(buf, "we saved the game");
	break;
      case H_GAME_RESTARTED:
	sprintf(buf, "we restarted the game");
	break;
      case H_GAME_ENDED:
	sprintf(buf, "we ended the game");
	break;
      case H_SIDE_JOINED:
      	side2 = side_n(data0);
	sprintf(buf, "%s joined the game",
		(side == side2 ? "you" : side_name(side2)));
	break;
      case H_SIDE_LOST:
      	side2 = side_n(data0);
	sprintf(buf, "%s lost!", (side == side2 ? "you" : side_name(side2)));
	/* Include an explanation of the cause, if there is one. */
	if (data1 == -1) {
	    tprintf(buf, " (resigned)");
	} else if (data1 == -2) {
	    tprintf(buf, " (self-unit died)");
	} else if (data1 > 0) {
	    tprintf(buf, " (scorekeeper %d)", data1);
	} else {
	    tprintf(buf, " (don't know why)");
	}
	break;
      case H_SIDE_WON:
      	side2 = side_n(data0);
	sprintf(buf, "%s won!", (side == side2 ? "you" : side_name(side2)));
	/* Include an explanation of the cause, if there is one. */
	if (data1 > 0) {
	    tprintf(buf, " (scorekeeper %d)", data1);
	} else {
	    tprintf(buf, " (don't know why)");
	}
	break;
      case H_UNIT_CREATED:
      	side2 = side_n(data0);
	sprintf(buf, "%s created ",
		(side == side2 ? "you" : side_name(side2)));
	unit = find_unit(data1);
	if (unit != NULL) {
	  tprintf(buf, "%s", unit_handle(side, unit));
	} else {
	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		tprintf(buf, "%s", past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_COMPLETED:
      	side2 = side_n(data0);
	sprintf(buf, "%s completed ",
		(side == side2 ? "you" : side_name(side2)));
	unit = find_unit(data1);
	if (unit != NULL) {
	  tprintf(buf, "%s", unit_handle(side, unit));
	} else {
	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		tprintf(buf, "%s", past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_CAPTURED:
	unit = find_unit(data0);
	if (unit != NULL) {
	    sprintf(buf, "%s", unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		sprintf(buf, "%s", past_unit_handle(side, pastunit));
	    } else {
		sprintf(buf, "%d??", data0);
	    }
	}
 	tprintf(buf, " captured ");
	unit = find_unit_dead_or_alive(data1);
	if (unit != NULL) {
	    tprintf(buf, "%s", unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data1);
	    if (pastunit != NULL) {
		tprintf(buf, "%s", past_unit_handle(side, pastunit));
	    } else {
		tprintf(buf, "%d??", data1);
	    }
	}
	break;
      case H_UNIT_DAMAGED:
	unit = find_unit_dead_or_alive(data0);
	if (unit != NULL) {
	    sprintf(buf, "%s", unit_handle(side, unit));
	} else {
 	    pastunit = find_past_unit(data0);
	    if (pastunit != NULL) {
		sprintf(buf, "%s", past_unit_handle(side, pastunit));
	    } else {
		sprintf(buf, "%d??", data0);
	    }
	}
	tprintf(buf, " damaged (%d -> %d hp)", data1, hevt->data[2]);
	break;
      case H_UNIT_KILLED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " was destroyed");
	break;
      case H_UNIT_STARVED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	tprintf(buf, " starved to death");
	break;
       case H_UNIT_NAME_CHANGED:
	pastunit = find_past_unit(data0);
	if (pastunit != NULL) {
	    sprintf(buf, "%s", past_unit_handle(side, pastunit));
	} else {
	    sprintf(buf, "%d??", data0);
	}
	unit = find_unit(data1);
	if (unit != NULL) {
	    if (unit->name != NULL)
	      tprintf(buf, " changed name to \"%s\"", unit->name);
	    else
	      tprintf(buf, " became anonymous");
	} else {
	pastunit2 = find_past_unit(data1);
 	  if (pastunit2 != NULL) {
	    if (pastunit2->name != NULL)
	      tprintf(buf, " changed name to \"%s\"", pastunit2->name);
	    else
	      tprintf(buf, " became anonymous");
	  }
	}
	break;
      default:
	run_warning("\"%s\" has no description", hevtdefns[hevt->type].name);
	break;
    }
}

/* Generate a description of the borders and connections in and around
   a location. */

void
linear_desc(buf, x, y)
char *buf;
int x, y;
{
    int t, dir;

    if (any_aux_terrain_defined()) {
	for_all_terrain_types(t) {
	    if (t_is_border(t)
		&& aux_terrain_defined(t)
		&& any_borders_at(x, y, t)) {
		tprintf(buf, " %s", t_type_name(t)); 
		for_all_directions(dir) {
		    if (border_at(x, y, dir, t)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
	    }
	    if (t_is_connection(t)
		&& aux_terrain_defined(t)
		&& any_connections_at(x, y, t)) {
		tprintf(buf, " %s", t_type_name(t)); 
		for_all_directions(dir) {
		    if (connection_at(x, y, dir, t)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
	    }
	}
    }
}

void
elevation_desc(buf, x, y)
char *buf;
int x, y;
{
    if (elevations_defined()) {
	sprintf(buf, "(Elev %d)", elev_at(x, y));
    }
}

char *
feature_desc(feature, buf)
Feature *feature;
char *buf;
{
    int i, capitalize = FALSE;
    char *str;

    if (feature == NULL)
      return NULL;
    if (feature->name) {
	/* Does the name need any substitutions done? */
	if (strchr(feature->name, '%')) {
	    i = 0;
	    for (str = feature->name; *str != '\0'; ++str) {
	    	if (*str == '%') {
		    /* Interpret substitution directives. */
		    switch (*(str + 1)) {
		      case 'T':
			capitalize = TRUE;
		      case 't':
			if (feature->typename) {
			    buf[i] = '\0';
			    strcat(buf, feature->typename);
			    if (capitalize && islower(buf[i]))
			      buf[i] = toupper(buf[i]);
			    i = strlen(buf);
			} else {
			    /* do nothing */
			}
			++str;
			break;
		      case '\0':
			break;  /* (should be error?) */
		      default:
			/* (error?) */
			break;
		    }
	    	} else {
		    buf[i++] = *str;
	    	}
	    }
	    /* Close off the string. */
	    buf[i] = '\0';
	    return buf;
	} else {
	    /* Return the name alone. */
	    return feature->name;
	}
    } else {
	if (feature->typename) {
	    sprintf(buf, "unnamed %s", feature->typename);
	    return buf;
	}
    }
    /* No description of the location is available. */
    return NULL;
}

/* Generate a string describing what is at the given location. */

char *featurebuf = NULL;

char *
feature_name_at(x, y)
int x, y;
{
    int fid = (features_defined() ? raw_feature_at(x, y) : 0);
    Feature *feature;

    if (fid == 0)
      return NULL;
    feature = find_feature(fid);
    if (feature != NULL) {
	if (featurebuf == NULL)
	  featurebuf = xmalloc(BUFSIZE);
	return feature_desc(feature, featurebuf);
    }
    /* No description of the location is available. */
    return NULL;
}

void
temperature_desc(buf, x, y)
char *buf;
int x, y;
{
    if (temperatures_defined()) {
	sprintf(buf, "(Temp %d)", temperature_at(x, y));
    }
}

#if 0
    int age, u;
    short view, prevview;
    Side *side2;

    /* Compose and display view history of this cell. */
    Dprintf("Drawing previous view info\n");
    age = side_view_age(side, curx, cury);
    prevview = side_prevview(side, curx, cury);
    if (age == 0) {
	if (prevview != view) {
	    if (prevview == EMPTY) {
		/* misleading if prevview was set during init. */
		sprintf(tmpbuf, "Up to date; had been empty.");
	    } else if (prevview == UNSEEN) {
		sprintf(tmpbuf, "Up to date; had been unexplored.");
	    } else {
		side2 = side_n(vside(prevview));
		u = vtype(prevview);
		if (side2 != side) {
		    sprintf(tmpbuf, "Up to date; had seen %s %s.",
			    (side2 == NULL ? "independent" :
			     side_name(side2)),
			    u_type_name(u));
		} else {
		    sprintf(tmpbuf,
			    "Up to date; had been occupied by your %s.",
			    u_type_name(u));
		}
	    }
	} else {
	    sprintf(tmpbuf, "Up to date.");
	}
    } else {
	if (prevview == EMPTY) {
	    sprintf(tmpbuf, "Was empty %d turns ago.", age);
	} else if (prevview == UNSEEN) {
	    sprintf(tmpbuf, "Terrain first seen %d turns ago.", age);
	} else {
	    side2 = side_n(vside(prevview));
	    u = vtype(prevview);
	    if (side2 != side) {
		sprintf(tmpbuf, "Saw %s %s, %d turns ago.",
			(side2 == NULL ? "independent" :
			 side_name(side2)),
			u_type_name(u), age);
	    } else {
		sprintf(tmpbuf, "Was occupied by your %s %d turns ago.",
			u_type_name(u), age);
	    }
	}
    }
#endif

void
hp_desc(buf, unit, label)
char *buf;
Unit *unit;
int label;
{
    if (label) {
	sprintf(buf, "HP ");
    } else {
	sprintf(buf, "");
    }
    /* (print '-' or some such for zero hp case?) */
    if (unit->hp == u_hp(unit->type)) {
	tprintf(buf, "%d", unit->hp);
    } else {
	tprintf(buf, "%d/%d", unit->hp, u_hp(unit->type));
    } 
}

void
acp_desc(buf, unit, label)
char *buf;
Unit *unit;
int label;
{
    int u = unit->type;

    if (!completed(unit)) {
	sprintf(buf, "%d/%d done", unit->cp, u_cp(u));
    } else if (unit->act && u_acp(u) > 0) {
    	if (label) {
    	    strcpy(buf, "ACP ");
    	} else {
    	    strcpy(buf, "");
    	}
	if (unit->act->acp == unit->act->initacp) {
	    tprintf(buf, "%d", unit->act->acp);
	} else {
	    tprintf(buf, "%d/%d", unit->act->acp, unit->act->initacp);
	}
    } else {
    	strcpy(buf, "");
    }
}

void
plan_desc(buf, unit)
char *buf;
Unit *unit;
{
    Plan *plan = unit->plan;
    Task *task = plan->tasks;

    if (plan == NULL) {
	buf[0] = '\0';
    	return;
    }
    sprintf(buf, "%s%s%s%s%s%s%s",
	    plantypenames[plan->type],
	    (plan->waitingfortasks ? " Waiting" : ""),
	    (plan->asleep ? " Asleep" : ""),
	    (plan->reserve ? " Reserve" : ""),
	    (plan->delayed ? " Delayed" : ""),
	    (plan->aicontrol ? " Delegated" : ""),
	    (plan->supply_is_low ? " Low" : ""),
	    (plan->supply_alarm ? " SAlarm" : "")
	    );
    if (task) {
	strcat(buf, " ");
    	task_desc(buf+strlen(buf), task);
    	if (task->next)
	  tprintf(buf, " ...");
    } 
}

void
task_desc(buf, task)
char *buf;
Task *task;
{
    int i, slen;
    char *argtypes;

    sprintf(buf, "%s", taskdefns[task->type].name);
    switch (task->type) {
      case TASK_BUILD:
	tprintf(buf, " %s", u_type_name(task->args[0]));
	if (task->args[1] != 0) {
	    Unit *unit = find_unit(task->args[1]);

	    if (unit != NULL) {
		tprintf(buf, " (%d cp)", unit->cp);
	    }
	}
	tprintf(buf, ", %d of %d", task->args[2], task->args[3]);
	break;
      case TASK_HIT_POSITION:
      	tprintf(buf, " %d,%d", task->args[0], task->args[1]);
	break;
      case TASK_HIT_UNIT:
      	tprintf(buf, " at %d,%d (type %d side %d)",
      		task->args[0], task->args[1], task->args[2], task->args[3]);
	break;
      case TASK_MOVETO:
        if (task->args[2] == 0) {
	    tprintf(buf, " %d,%d", task->args[0], task->args[1]);
        } else if (task->args[2] == 1) {
	    tprintf(buf, " adjacent %d,%d", task->args[0], task->args[1]);
        } else {
	    tprintf(buf, " within %d of %d,%d",
		    task->args[2], task->args[0], task->args[1]);
        }
	break;
      default:
	argtypes = taskdefns[task->type].argtypes;
	slen = strlen(argtypes);
	for (i = 0; i < slen; ++i) {
	    tprintf(buf, "%c%d", (i == 0 ? ' ' : ','), task->args[i]);
	}
	break;
    }
    tprintf(buf, " x %d", task->execnum);
    if (task->retrynum > 0) {
	tprintf(buf, " fail %d", task->retrynum);
    }
}

/* Format a clock time into a standard form.  This routine will omit the hours
   part if it will be uninteresting. */

void
time_desc(buf, time, maxtime)
char *buf;
int time, maxtime;
{
    int hour, minute, second;

    if (time >= 0) {
	hour = time / 3600;  minute = (time / 60) % 60;  second = time % 60;
    	if (between(1, maxtime, 3600) && hour == 0) {
	    sprintf(buf, "%.2d:%.2d", minute, second);
	} else {
	    sprintf(buf, "%.2d:%.2d:%.2d", hour, minute, second);
	}
    } else {
    	sprintf(buf, "??:??:??");
    }
}

/* General-purpose routine to take an array of anonymous unit types and
   summarize what's in it, using numbers and unit chars. */

char *
summarize_units(buf, ucnts)
char *buf;
int *ucnts;
{
    char tmp[BUFSIZE];  /* should be bigger? */
    int u;

    sprintf(buf, "");
    for_all_unit_types(u) {
	if (ucnts[u] > 0) {
	    sprintf(tmp, " %d %s", ucnts[u], utype_name_n(u, 3));
	    strcat(buf, tmp);
	}
    }
    return buf;
}

/* Compose a one-line comment on the game. */
/* (should reindent) */
char *
exit_commentary(side)
Side *side;
{
    int numingame = 0, numwon = 0, numlost = 0;
    Side *side2, *lastside, *winner, *loser;

    for_all_sides(side2) {
    if (side2->ingame) {
    	++numingame;
    	lastside = side2;
    }
	if (side_won(side2)) {
	    ++numwon;
	    winner = side2;
	}
	if (side_lost(side2)) {
	    ++numlost;
	    loser = side2;
	}
    }
    if (numingame > 0) {
    	if (0 /* could have been resolved, need to check scorekeepers */) {
			sprintf(spbuf, "The outcome remains undecided");
#ifdef RUDE
			if (numsides > 1) {
			    strcat(spbuf, ", but you're probably the loser!");
			} else {
			    strcat(spbuf, "...");
			}
#else
			strcat(spbuf, "...");
#endif /* RUDE */
		} else {
    		sprintf(spbuf, "Game is over.");
    	}
    } else if (numwon == numsides) {
	    sprintf(spbuf, "Everybody won!");
    } else if (numlost == numsides) {
	    sprintf(spbuf, "Everybody lost!");
	} else if (numwon == 1) {
		sprintf(spbuf, "%s won!", (side == winner ? "You" : side_desig(winner)));
	} else if (numlost == 1) {
		sprintf(spbuf, "%s lost!", (side == loser ? "You" : side_desig(loser)));
	} else {
		sprintf(spbuf, "Some won and some lost.");
	}
    return spbuf;
}

#if 0
    /* (get from nlang.c?) */
    notify_all(
#ifdef RUDE
	       "Those %s %s have given up!",
	       (flip_coin() ? "cowardly" : "wimpy"),
#else
	       "The %s have given up!",
#endif /* RUDE */
	       side->pluralnoun);
    if (side2 != NULL) {
	notify_all("... and they're giving all their stuff to the %s!",
		   side2->pluralnoun);
    }
#endif

/* The following routines should probably go into some kind of international
   type interface. */

/* Given a number, figure out what suffix should go with it. */

char *
ordinal_suffix(n)
int n;
{
    if (n % 100 == 11 || n % 100 == 12 || n % 100 == 13) {
	return "th";
    } else {
	switch (n % 10) {
	  case 1:   return "st";
	  case 2:   return "nd";
	  case 3:   return "rd";
	  default:  return "th";
	}
    }
}

/* Pluralize a word, attempting to be smart about various possibilities
   that don't have a different plural form (such as "Chinese" and "Swiss"). */

/* There should probably be a test for when to add "es" instead of "s". */

char *pluralbuf = NULL;

char *
plural_form(word)
char *word;
{
    char endch = ' ', nextend = ' ';

    if (pluralbuf == NULL)
      pluralbuf = xmalloc(BUFSIZE);
    if (word == NULL) {
	run_warning("plural_form given NULL string");
	pluralbuf[0] = '\0';
	return pluralbuf;
    }
    if (strlen(word) > 0)
      endch   = word[strlen(word)-1];
    if (strlen(word) > 1)
      nextend = word[strlen(word)-2];
    if (endch == 'h' || endch == 's' || (endch == 'e' && nextend == 's')) {
	sprintf(pluralbuf, "%s", word);
    } else {
	sprintf(pluralbuf, "%ss", word);
    }
    return pluralbuf;
}

/* General text generation. */

/*
Need some sort of default grammar used when game designs don't want
to add anything special.
Copy ^0, ... concept for format strings generated by grammars.
Use grammar idea for narrative generation.
Do for actions, notable backdrop events, summaries.
(text disband-narrative
  (self infantry "%2 goes home")
  (u* bomb "%1 dismantles %2")
  )
Match on action args to choose sentence, allow multiple weighted choices
for variety, be able to tailor for each side.
Should be able to do both present and past tense generation.
Routine is describe_thing(side(s), generator, parms[10]).
(Would be generally useful to have a side-to-bit-vector conversion routine...)
"Narrative" different from "message", is past sense, while "message"
describes ongoing things.
Need to do narrative descriptions / events for notable backdrop events
such as migrations, storms, etc.
*/

char *
make_text(buf, maker, a1, a2, a3, a4)
char *buf;
Obj *maker;
long a1, a2, a3, a4;
{
    if (buf == NULL)
      buf = xmalloc(BUFSIZE);
    if (maker != lispnil) {
    } else {
	sprintf(buf, "%d %d %d %d", a1, a2, a3, a4);
    }
    return buf;
}

/* Date/time parsing and formatting. */

/* Compose a readable form of the given date. */

char *
absolute_date_string(date)
long date;
{
    int caltypeunknown = FALSE;
    Obj *cal, *caltype, *stepname, *step;

    if (datebuf == NULL)
      datebuf = xmalloc(BUFSIZE);
    /* The first time we ask for a date, interpret the calendar. */
    if (calendartype < 0) {
    	cal = g_calendar();
    	if (consp(cal)) {
    	    caltype = car(cal);
    	    if ((symbolp(caltype) || stringp(caltype))
    	    	&& strcmp("usual", c_string(caltype)) == 0) {
    	    	calendartype = 1;
    		stepname = cadr(cal);
    		datestepname = (stringp(stepname) ? c_string(stepname) : "day");
    		step = car(cddr(cal));
    		datestep = (numberp(step) ? c_number(step) : 1);
    	    } else {
    	    	caltypeunknown = TRUE;
    	    }
    	} else {
    	    calendartype = 0;
    	    turnname = "Turn";
    	    if (stringp(cal)) {
    	    	turnname = c_string(cal);
    	    } else if (cal != lispnil) {
    	    	caltypeunknown = TRUE;
    	    }
	}
	if (caltypeunknown) {
	    init_warning("Unknown calendar type");
	}
    }
    switch (calendartype) {
      case 0:
	sprintf(datebuf, "%s%4d", turnname, date);
	return datebuf;
      case 1:
	return usual_date_string(date);
      default:
	case_panic("calendar type", calendartype);
    }
}

/* Compose a date, omitting components supplied by the base date. */

char *
relative_date_string(date, base)
long date, base;
{
    if (datebuf == NULL)
      datebuf = xmalloc(BUFSIZE);
    /* should do this for real eventually */
    sprintf(datebuf, "%d(%d)", date, base);
    return datebuf;
}

void
parse_usual_initial_date()
{
    char *date = g_base_date();
    char monthname[BUFSIZE];
    int i;

    if (baseday < 0) {
	if (!empty_string(date)) {
	    /* Assume it's in a standard date format. */
	    if (strcmp(datestepname, "hour") == 0) {
		sscanf(date, "%d %d %s %d",
		       &basehour, &baseday, monthname, &baseyear);
		--baseday;
	    } else if (strcmp(datestepname, "day") == 0) {
		sscanf(date, "%d %s %d", &baseday, monthname, &baseyear);
		--baseday;
	    } else if (strcmp(datestepname, "week") == 0) {
		sscanf(date, "%d %s %d", &baseday, monthname, &baseyear);
		--baseday;
	    } else if (strcmp(datestepname, "month") == 0) {
		sscanf(date, "%s %d", monthname, &baseyear);
	    } else if (strcmp(datestepname, "year") == 0) {
		sscanf(date, "%d", &baseyear);
	    }
	    for (i = 0; i < 12; ++i) {
		if (strcmp(monthname, months[i]) == 0) {
		    basemonth = i;
		    return;
		}
	    }
	} else {
	    baseday = 0;
	}
    }
}


/* Given a numeric data, convert it into something understandable.  Depends
   on the length of a turn. */

static char *
usual_date_string(date)
int date;
{
    int year = 0, season = 0, month = 0, week = 0, day = 0;
    int hour = 0, second = 0, minute = 0;

    /* The date, which is a turn number, should be 1 or more, but this
       routine may be called before the game really starts, so return
       something that will be distinctive if it's ever displayed. */
    if (date <= 0)
      return "?date<=0?";
    /* First displayed date is normally turn 1; be zero-based for
       the benefit of calculation. */
    --date;
    date *= datestep;
    parse_usual_initial_date();
    if (strcmp(datestepname, "second") == 0) {
	second = date % 60;
	minute = date / 60;
	sprintf(datebuf, "%d:%d", minute, second);
    } else if (strcmp(datestepname, "minute") == 0) {
	minute = date % 60;
	hour = date / 60 + basehour;
	sprintf(datebuf, "%d:%d", hour, minute);
    } else if (strcmp(datestepname, "hour") == 0) {
 	hour = (date + basehour) % 24;
	/* Convert to days, then proceed as for days. */
 	date = (date + basehour) / 24;
	date += 30 * basemonth;
	day = date % 365;
	month = day / 30;
	day = (day + baseday) % 30 + 1;
	year = date / 365 + baseyear;
	sprintf(datebuf, "%d:00 %2d %s %d", hour, day, months[month], ABS(year));
   } else if (strcmp(datestepname, "day") == 0) {
	/* Should do this more accurately... */
	date += 30 * basemonth;
	day = date % 365;
	month = day / 30;
	day = (day + baseday) % 30 + 1;
	year = date / 365 + baseyear;
	sprintf(datebuf, "%2d %s %d", day, months[month], ABS(year));
    } else if (strcmp(datestepname, "week") == 0) {
	/* Convert to days, then proceed as for days. */
	date *= 7;
	date += 30 * basemonth;
	day = date % 365;
	month = day / 30;
	day = (day + baseday) % 30 + 1;
	year = date / 365 + baseyear;
	sprintf(datebuf, "%2d %s %d", day, months[month], ABS(year));
    } else if (strcmp(datestepname, "month") == 0) {
    	month = date % 12;
	year = date / 12 + baseyear;
	sprintf(datebuf, "%s %d", months[month], ABS(year));
    } else if (strcmp(datestepname, "season") == 0) {
    	season = date % 4;
	year = date / 4 + baseyear;
	sprintf(datebuf, "%d %d", season, ABS(year));
    } else if (strcmp(datestepname, "year") == 0) {
	year = date + baseyear;
	sprintf(datebuf, "%d", ABS(year));
    } else {
	sprintf(datebuf, "?what's a %s?", datestepname);
    }
    if (year < 0) {
	strcat(datebuf, " BC");
    }
    return datebuf;
}

/* Show some overall numbers on performance of a side. */

void
write_side_results(fp, side)
FILE *fp;
Side *side;
{
    if (side == NULL) {
	fprintf(fp, "Results for game as a whole:\n\n");
    } else {
	fprintf(fp, "Results for %s, played by %s:\n\n",
		short_side_title(side), long_player_title(spbuf, side->player, NULL));
	/* (should mention win/loss and such) */
    }
}

/* Display what is essentially a double-column bookkeeping of unit gains
   and losses. */

/* (should reindent) */
void
write_unit_record(fp, side)
FILE *fp;
Side *side;
{
    int u, gainreason, lossreason, totgain, totloss, val;

    fprintf(fp, "Unit Record (gains and losses by cause and unit type)\n");
    fprintf(fp, " Unit Type ");
    for (gainreason = 0; gainreason < NUMGAINREASONS; ++gainreason) {
	fprintf(fp, " %3s", gain_reason_names[gainreason]);
    }
    fprintf(fp, " Gain |");
    for (lossreason = 0; lossreason < NUMLOSSREASONS; ++lossreason) {
	fprintf(fp, " %3s", loss_reason_names[lossreason]);
    }
    fprintf(fp, " Loss |");
    fprintf(fp, " Total\n");
    for_all_unit_types(u) {
      if (1 /* type was at least potentially in game */) {
	totgain = 0;
	fprintf(fp, " %9s ", utype_name_n(u, 9));
	for (gainreason = 0; gainreason < NUMGAINREASONS; ++gainreason) {
	    val = gain_count(side, u, gainreason);
	    if (val > 0) {
		fprintf(fp, " %3d", val);
		totgain += val;
	    } else {
		fprintf(fp, "    ");
	    }
	}
	fprintf(fp, "  %3d |", totgain);
	totloss = 0;
	for (lossreason = 0; lossreason < NUMLOSSREASONS; ++lossreason) {
	    val = loss_count(side, u, lossreason);
	    if (val > 0) {
		fprintf(fp, " %3d", val);
		totloss += val;
	    } else {
		fprintf(fp, "    ");
	    }
	}
	fprintf(fp, "  %3d |", totloss);
	fprintf(fp, "  %3d\n", totgain - totloss);
      }
    }
    fprintf(fp, "\n");
}

static int
gain_count(side, u, r)
Side *side;
int u, r;
{
    int sum;

    if (side != NULL)
      return side_gain_count(side, u, r);
    sum = 0;
    for_all_sides(side) {
	sum += side_gain_count(side, u, r);
    }
    return sum;
}

static int
loss_count(side, u, r)
Side *side;
int u, r;
{
    int sum;

    if (side != NULL)
      return side_loss_count(side, u, r);
    sum = 0;
    for_all_sides(side) {
	sum += side_loss_count(side, u, r);
    }
    return sum;
}

/* Nearly-raw combat statistics; hard to interpret, but they provide
   a useful check against subjective evaluation of performance. */
/* (should reindent) */
void
write_combat_results(fp, side)
FILE *fp;
Side *side;
{
    int a, d, atk;

    fprintf(fp,
	    "Unit Combat Results (average damage over # attacks against enemy, by type)\n");
    fprintf(fp, " A  D->");
    for_all_unit_types(d) {
      if (1 /* part of game */) {
	fprintf(fp, " %4s ", utype_name_n(d, 4));
      }
    }
    fprintf(fp, "\n");
    for_all_unit_types(a) {
      if (1 /* part of game */) {
	fprintf(fp, " %4s ", utype_name_n(a, 4));
	for_all_unit_types(d) {
	  if (1 /* part of game */) {
	    atk = atkstats(side, a, d);
	    if (atk > 0) {
		fprintf(fp, " %5.2f",
			((float) hitstats(side, a, d)) / atk);
	    } else {
		fprintf(fp, "      ");
	    }
	  }
	}
	fprintf(fp, "\n     ");
	for_all_unit_types(d) {
	  if (1 /* part of game */) {
	    atk = atkstats(side, a, d);
	    if (atk > 0) {
		fprintf(fp, " %4d ", atk);
	    } else {
		fprintf(fp, "      ");
	    }
	  }
	}
	fprintf(fp, "\n");
      }
    }
    fprintf(fp, "\n");
}

static int
atkstats(side, a, d)
Side *side;
int a, d;
{
    int sum;

    if (side != NULL)
      return side_atkstats(side, a, d);
    sum = 0;
    for_all_sides(side) {
	sum += side_atkstats(side, a, d);
    }
    return sum;
}

static int
hitstats(side, a, d)
Side *side;
int a, d;
{
    int sum;

    if (side != NULL)
      return side_hitstats(side, a, d);
    sum = 0;
    for_all_sides(side) {
	sum += side_hitstats(side, a, d);
    }
    return sum;
}

void
dice_desc(buf, dice)
char *buf;
int dice;
{
    int numdice, die, offset;

    if (dice >> 14 == 0 || dice >> 14 == 3) {
	sprintf(buf, "%d", dice);
    } else {
    	numdice = (dice >> 11) & 0x07;
    	die = (dice >> 7) & 0x0f;
    	offset = dice & 0x7f;
    	if (offset == 0) {
	    sprintf(buf, "%dd%d", numdice, die);
    	} else {
	    sprintf(buf, "%dd%d+%d", numdice, die, offset);
    	}
    }
}

/* The following code formats a list of types that are missing images. */

void
record_missing_image(typtyp, str)
int typtyp;
char *str;
{
    if (missinglist == NULL) {
	missinglist = xmalloc(BUFSIZE);
	missinglist[0] = '\0';
    }
    ++missing[typtyp];
    /* Add the name of the image-less type, but only if one of
       the first few. */
    if (between(1, totlisted, NUMTOLIST))
      strcat(missinglist, ",");
    if (totlisted < NUMTOLIST) {
	strcat(missinglist, str);
    } else if (totlisted == NUMTOLIST) {
	strcat(missinglist, "...");
    }
    ++totlisted;
}

/* Return true if any images could not be found, and provide some helpful info
   into the supplied buffer. */

int
missing_images(buf)
char *buf;
{
    if (missinglist == NULL)
      return FALSE;
    buf[0] = '\0';
    if (missing[UTYP] > 0)
      tprintf(buf, " %d unit images", missing[UTYP]);
    if (missing[TTYP] > 0)
      tprintf(buf, " %d terrain images", missing[TTYP]);
    if (missing[3] > 0)
      tprintf(buf, " %d emblems", missing[3]);
    tprintf(buf, " - %s", missinglist);
    return TRUE;
}

