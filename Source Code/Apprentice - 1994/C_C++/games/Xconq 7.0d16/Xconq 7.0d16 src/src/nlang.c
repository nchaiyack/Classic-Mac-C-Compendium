/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Natural-language handling in the kernel. */
/* This file should be entirely replaced for non-English Xconqs. */

#include "conq.h"

#define g_initial_date() ((char *) g_base_date())

int baseday = -1, basemonth = 0, baseyear = 1900;

int basehour;

/* Short names of directions. */

char *dirnames[] = DIRNAMES;

/* names of columns in unit record */

char *reasonnames[] = { NULL } /* REASONNAMES */;

char *unitbuf = NULL;

char *sideshorttitle = NULL;

pad_out(buf, n)
char *buf;
int n;
{
    int i, len = strlen(buf);

    if (n < 1) return;
    for (i = len; i < n; ++i) {
	buf[i] = ' ';
    }
    buf[n - 1] = '\0';
}

char *
short_side_title(side)
Side *side;
{
    if (sideshorttitle == NULL) sideshorttitle = (char *) xmalloc(BUFSIZE);
    if (side == NULL) {
	return " - ";
    } else if (side->name) {
	return side->name;
    } else if (side->pluralnoun) {
	sprintf(sideshorttitle, "The %s", side->pluralnoun);
    } else if (side->noun) {
	sprintf(sideshorttitle, "The %s", plural_form(side->noun));
    } else if (side->adjective) {
	sprintf(sideshorttitle, "The %s side", side->adjective);
    } else {
	return " - ";
    }
    return sideshorttitle;
}

char *
shortest_side_title(side2, buf)
Side *side2;
char *buf;
{
    if (side2 == NULL) {
	sprintf(buf, "-");
    } else if (side2->name) {
	sprintf(buf, "%s", side2->name);
    } else if (side2->adjective) {
	sprintf(buf, "%s", side2->adjective);
    } else if (side2->noun) {
	sprintf(buf, "%s", side2->noun);
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

void
long_player_title(buf, player, thisdisplayname)
char *buf, *thisdisplayname;
Player *player;
{
    if (player == NULL) {
	sprintf(buf, "");
    } else if (player->displayname != NULL) {
	if (strcmp(player->displayname, thisdisplayname) == 0) {
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
}

void
short_player_title(buf, player, thisdisplayname)
char *buf, *thisdisplayname;
Player *player;
{
    long_player_title(buf, player, thisdisplayname);
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

    if (unitbuf == NULL) unitbuf = (char *) xmalloc(BUFSIZE);
    /* Handle various weird situations. */
    if (unit == NULL)
      return "???";
    if (!alive(unit) && side)
      return "?dead?";
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

    if (unitbuf == NULL) unitbuf = (char *) xmalloc(BUFSIZE);
    if (unit == NULL) {
	return "???";
    }
    u = unit->type;
    if (unit->name && strlen(unit->name) > 0) {
	sprintf(unitbuf, "%s", unit->name);
    } else if (u_short_name(u) && strlen(u_short_name(u)) > 0) {
	sprintf(unitbuf, "%d%s %s",
		unit->number, ordinal_suffix(unit->number), u_short_name(u));
    } else {
	sprintf(unitbuf, "%d%s %s",
		unit->number, ordinal_suffix(unit->number), u_type_name(u));
    }
    return unitbuf;
}

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

/* Given a unit and optional type u, summarize construction status
   and timing. */

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
      if (task->type == BUILD_TASK) {
	u2 = task->args[0];
	tprintf(buf, " %s ", (is_unit_type(u2) ? u_type_name(u2) : "?"));
	unit2 = find_unit(task->args[1]);
	if (in_play(unit2) && unit2->type == u2) {
	    tprintf(buf, "%d/%d done ", unit2->cp, u_cp(unit2->type));
	}
	tprintf(buf, "(%d of %d)", task->args[2] + 1, task->args[3]);
      } else if (task->type == RESEARCH_TASK) {
	u2 = task->args[0];
        if (is_unit_type(u2)) {
		tprintf(buf, " %s tech %d/%d",
			u_type_name(u2), unit->side->tech[u2], task->args[1]);
        }
      }
    }
}

constructible_desc(buf, side, u, unit)
char *buf;
Side *side;
int u;
Unit *unit;
{
    char estbuf[10];
    int est, num;

    if (unit != NULL) {
    	if ((est = est_completion_time(unit, u)) >= 0) {
	    sprintf(estbuf, "[%2d] ", est); 
    	} else {
	    strcpy(estbuf, " --  ");
    	}
    } else {
    	strcpy(estbuf, "");
    }
    sprintf(buf, "%s%-16.16s", estbuf, u_type_name(u));
    if ((num = num_units_in_play(side, u)) > 0) {
	tprintf(buf, "  %3d", num);
    }
    if ((num = num_units_incomplete(side, u)) > 0) {
	tprintf(buf, "(%d)", num);
    }
}

est_completion_time(unit, u2)
Unit *unit;
int u2;
{
    if (uu_acp_to_create(unit->type, u2) < 1) return (-1);
    return normal_completion_time(unit->type, u2);
}

historical_event_date_desc(hevt, buf)
HistEvent *hevt;
char *buf;
{
    sprintf(buf, "%d: ", hevt->startdate);
}

historical_event_desc(hevt, buf)
HistEvent *hevt;
char *buf;
{
	int data0 = hevt->data[0];

    switch (hevt->type) {
      case H_LOG_STARTED:
	sprintf(buf, "started recording events");
	break;
      case H_GAME_STARTED:
	sprintf(buf, "started the game");
	break;
      case H_SIDE_JOINED:
	sprintf(buf, "%s joined the game", side_desig(side_n(data0)));
	break;
      case H_SIDE_LOST:
	sprintf(buf, "%s lost", side_desig(side_n(data0)));
	break;
      case H_SIDE_WON:
	sprintf(buf, "%s won", side_desig(side_n(data0)));
	break;
#if 0
      case H_UNIT_LOST:
	sprintf(buf, "lost %s (reason %d)", u_type_name(data0), hevt->data[1]);
	break;
#endif
      case H_UNIT_CREATED:
	sprintf(buf, "created %s", u_type_name(data0));
	break;
      case H_UNIT_COMPLETED:
	sprintf(buf, "completed %s", u_type_name(data0));
	break;
      case H_UNIT_CAPTURED:
	sprintf(buf, "%s captured %s", u_type_name(data0), u_type_name(hevt->data[1]));
	break;
      default:
	sprintf(buf, "%s", hevtdefns[hevt->type].name);
	break;
    }
}

linear_desc(buf, x, y)
char *buf;
int x, y;
{
    int t, t2, dir;

    if (any_aux_terrain_defined()) {
	for_all_terrain_types(t2) {
	    if (t_is_border(t2)
		&& aux_terrain_defined(t2)
		&& any_borders_at(x, y, t2)) {
		tprintf(buf, " %s", t_type_name(t2)); 
		for_all_directions(dir) {
		    if (border_at(x, y, dir, t2)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
	    }
	    if (t_is_connection(t2)
		&& aux_terrain_defined(t2)
		&& any_connections_at(x, y, t2)) {
		tprintf(buf, " %s", t_type_name(t2)); 
		for_all_directions(dir) {
		    if (connection_at(x, y, dir, t2)) {
			tprintf(buf, "/%s", dirnames[dir]);
		    }
		}
	    }
	}
    }
}

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
	if (feature == NULL) return NULL;
	if (feature->name) {
	    if (feature->typename) {
		if (0 /* special format for name/typename */) {
		} else {
		    sprintf(buf, "%s %s", feature->name, feature->typename);
		    return buf;
		}
	    } else {
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

    if (fid == 0) return NULL;
    if ((feature = find_feature(fid)) != NULL) {
	if (featurebuf == NULL) featurebuf = xmalloc(BUFSIZE);
	return feature_desc(feature, featurebuf);
    }
    /* No description of the location is available. */
    return NULL;
}

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

    /* Compose and display view history of this hex. */
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

plan_desc(buf, unit)
char *buf;
Unit *unit;
{
    int u = unit->type;
    Plan *plan = unit->plan;
    Task *task = plan->tasks;

    if (plan == NULL) {
    	sprintf(buf, "");
    	return;
    }
    sprintf(buf, "%s%s%s%s",
	    plantypenames[plan->type],
    	    (plan->waitingfortasks ? " Waiting" : ""),
	    (plan->asleep ? " Asleep" : ""),
	    (plan->reserve ? " Reserve" : ""),
	    (plan->aicontrol ? " Delegated" : "")
	    );
    if (task) {
    	task_desc(buf+strlen(buf), task);
    	if (task->next) tprintf(buf, " ...");
    } 
}

task_desc(buf, task)
char *buf;
Task *task;
{
    int i;
    char *argtypes;

    sprintf(buf, "%s", taskdefns[task->type].name);
    switch (task->type) {
      case BUILD_TASK:
	tprintf(buf, " %s", u_type_name(task->args[0]));
	if (task->args[1] != 0) {
	    Unit *unit = find_unit(task->args[1]);

	    if (unit != NULL) {
		tprintf(buf, " (%d cp)", unit->cp);
	    }
	}
	tprintf(buf, ", %d of %d", task->args[2], task->args[3]);
	break;
      default:
	argtypes = taskdefns[task->type].argtypes;
	for (i = 0; i < strlen(argtypes); ++i) {
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
#ifdef RUDE
    notify(side, "You lost, %s!", (flip_coin() ? "sucker" : "turkey"));
#else
    notify(side, "You lost!");
#endif /* RUDE */
#endif

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

    if (pluralbuf == NULL) pluralbuf = xmalloc(BUFSIZE);
    if (word == NULL) {
	run_warning("plural_form given NULL string");
	pluralbuf[0] = '\0';
	return pluralbuf;
    }
    if (strlen(word) > 0) endch   = word[strlen(word)-1];
    if (strlen(word) > 1) nextend = word[strlen(word)-2];
    if (endch == 'h' || endch == 's' || (endch == 'e' && nextend == 's')) {
	sprintf(pluralbuf, "%s", word);
    } else {
	sprintf(pluralbuf, "%ss", word);
    }
    return pluralbuf;
}


/* Date/time parsing and formatting. */

/* Compose a readable form of the given date. */

int calendartype = -1;

char datebuf[40];

char *turnname;
char *datestepname;
int datestep;

char *
absolute_date_string(date)
long date;
{
    int caltypeunknown = FALSE;
    Obj *cal, *caltype, *stepname, *step;

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
    /* do this for real eventually */
    sprintf(datebuf, "%d(%d)", date, base);
    return datebuf;
}

char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", "???" };

parse_usual_initial_date()
{
    char *date = g_initial_date();
    char monthname[BUFSIZE];
    int i;

    if (baseday < 0) {
      if (date != NULL && strlen(date) > 0) {
	/* Assume it's in a standard date format. */
    	if (strcmp(datestepname, "hour") == 0) {
	    sscanf(date, "%d %d %s %d", &basehour, &baseday, monthname, &baseyear);
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

char *
usual_date_string(date)
int date;
{
    int year, month = 0, week = 0, day = 0, hour = -1;

    --date;  /* first displayed date is turn 1, this adjusts */
    parse_usual_initial_date();
    /* (Short intervals should be done clock-style.) */
    if (strcmp(datestepname, "second") == 0) {
	sprintf(datebuf, "Second %d", date);
	return datebuf;
    } else if (strcmp(datestepname, "minute") == 0) {
	sprintf(datebuf, "Minute %d", date);
	return datebuf;
    } else if (strcmp(datestepname, "hour") == 0) {
 	hour = (date + basehour) % 24;
 	date = (date + basehour) / 24;
	day = date % 365;
	day = (day + baseday) % 30 + 1;
	month = day / 30 + basemonth;
	year = date / 365 + baseyear;
	sprintf(datebuf, "%d:00 %2d %s %d", hour, day, months[month], abs(year));
   } else if (strcmp(datestepname, "day") == 0) {
	/* Should do this more accurately... */
	day = date % 365;
	day = (day + baseday) % 30 + 1;
	month = (day - 1) / 30 + basemonth;
	year = date / 365 + baseyear;
	sprintf(datebuf, "%2d %s %d", day, months[month], abs(year));
    } else if (strcmp(datestepname, "week") == 0) {
	day = (date * 7) % 365;
	month = day / 30;
	day = day % 30 + 1;
	year = baseyear + (date * 7) / 365;
	sprintf(datebuf, "%2d %s %d", day, months[month], abs(year));
    } else if (strcmp(datestepname, "month") == 0) {
	year = baseyear + date / 12;
	sprintf(datebuf, "%s %d", months[date % 12], abs(year));
    } else if (strcmp(datestepname, "year") == 0) {
	year = baseyear + date;
	sprintf(datebuf, "%d", abs(year));
    } else {
	sprintf(datebuf, "?what's a %s?", datestepname);
    }
    if (year < 0) {
	strcat(datebuf, " BC");
    }
    return datebuf;
}

/* Return a string naming the season.  The names are wired in; for extra
   flavor, this should be settable. */

char *
season_name(turn)
int turn;
{
    int year = world.yearlength;
    int num;

    num = ((turn - 1) + (year - 0 /* g_first_midwinter() */)) % year;

    if (num <= year/2) {
	return ((num <= (1 * year) / 4) ? "winter" : "spring");
    } else {
	return ((num <= (3 * year) / 4) ? "summer" : "autumn");
    }
}

/* Display what is essentially a double-column bookkeeping of unit gains */
/* and losses.  Tricks here include the use of "dummy reason" flags to */
/* display sums of several columns. */

print_unit_record(fp, side)
FILE *fp;
Side *side;
{
#if 0
    int atype, reason, sum;

    fprintf(fp, "Unit Record (gains and losses by cause and unit type)\n");
    fprintf(fp, "   ");
    for (reason = 0; reason < NUMREASONS; ++reason) {
	fprintf(fp, " %3s", reasonnames[reason]);
    }
    fprintf(fp, "  Total\n");
    for_all_unit_types(atype) {
	sum = 0;
	fprintf(fp, " %s ", utype_name_n(atype, 1));
	for (reason = 0; reason < NUMREASONS; ++reason) {
	    if (side_balance(side, atype, reason) > 0) {
		fprintf(fp, " %3d", side_balance(side, atype, reason));
		sum += side_balance(side, atype, reason);
	    } else if (reason == DUMMYREAS) {
		fprintf(fp, " %3d", sum);
		sum = 0;
	    } else {
		fprintf(fp, "    ");
	    }
	}
	fprintf(fp, "   %3d\n", sum);
    }
    fprintf(fp, "\n");
#endif
}

/* Show some overall numbers on performance of a side. */

print_side_results(fp, side)
FILE *fp;
Side *side;
{
}

#if 0
    int occs[MAXUTYPES], gains[MAXUTYPES], kills[MAXUTYPES];
	char hitbuf[BUFSIZE], killbuf[BUFSIZE];
    for_all_unit_types(u2) occs[u2] = gains[u2] = kills[u2] = 0;
/*    notify(us, "You captured %s!", unit_handle(us, pris));  */
/*    notify(ps, "%s has been captured by the %s!",
	   unit_handle(ps, pris), plural_form(us->name)); */

    summarize_units(hitbuf,  gains);
    summarize_units(killbuf, kills);
    summarize_units(spbuf, occs);
    if (strlen(hitbuf) > 0) {
	if (strlen(killbuf) > 0) {
	    notify(us, "   (Also captured%s, killed%s)", hitbuf, killbuf);
	} else {
	    notify(us, "   (Also captured%s)", hitbuf);
	}
    } else if (strlen(killbuf) > 0) {
	notify(us, "   (Also killed%s)", killbuf);
    }
    if (strlen(spbuf) > 0) {
	notify(ps, "   (Lost%s)", spbuf);
    }
#endif


/* The following code formats a list of types that are missing images. */

/* This is the number of of types to mention by name; any others will
   just be included in the count of missing images. */

#define NUMTOLIST 5

char *missinglist = NULL;

int missing[4];

int totlisted = 0;

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
    if (between(1, totlisted, NUMTOLIST)) strcat(missinglist, ",");
    if (totlisted < NUMTOLIST) {
	strcat(missinglist, str);
    } else if (totlisted == NUMTOLIST) {
	strcat(missinglist, "...");
    }
    ++totlisted;
}

/* Return true if any images could not be found, and provide some helpful info
   into the supplied buffer. */

missing_images(buf)
char *buf;
{
    if (missinglist == NULL) return FALSE;
    sprintf(buf, "");
    /* (should make into a loop?) */
    if (missing[UTYP] > 0)
      tprintf(buf, " %d unit images", missing[UTYP]);
    if (missing[TTYP] > 0)
      tprintf(buf, " %d terrain images", missing[TTYP]);
    if (missing[3] > 0)
      tprintf(buf, " %d emblems", missing[3]);
    tprintf(buf, " - %s", missinglist);
    return TRUE;
}

