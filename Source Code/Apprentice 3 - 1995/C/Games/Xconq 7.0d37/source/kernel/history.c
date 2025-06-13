/* Management of historical information about an Xconq game.
   Copyright (C) 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include "conq.h"

static void rewrite_unit_references PROTO ((int oldid, int newid));
static void rewrite_unit_references_in_event PROTO ((HistEvent *hevt, int oldid, int newid));

HevtDefn hevtdefns[] = {

#undef  DEF_HEVT
#define DEF_HEVT(NAME, code, DATADESCS) { NAME, DATADESCS },

#include "history.def"

    { NULL, NULL }
};

/* Head of the list of events. */

HistEvent *history;

/* The list of all past unit records. */

PastUnit *past_unit_list;

/* The id of the next past unit to be synthesized.  -1 is reserved, so
   start counting down from -2. */

int next_past_unit_id = -2;

/* Buffers for descriptions of past units. */

#define NUMPASTBUFS 3

int curpastbuf = 0;

char *pastbufs[NUMPASTBUFS] = { NULL, NULL, NULL };

/* True if events are being recorded into the history. */

int recording_events;

void
init_history()
{
    /* The first "event" is just a marker. */
    history = create_historical_event(0);
    /* Give it an impossible date. */
    history->startdate = -1;
    history->next = history->prev = history;
    /* This event cannot have any observers, since it occurs
       before the beginning of the game. */
    /* (should do later perhaps?) */
    recording_events = TRUE;
    record_event(H_LOG_STARTED, NOSIDES);
    /* Initialize the past unit record. */
    past_unit_list = NULL;
    next_past_unit_id = -2;
}

HistEvent *
create_historical_event(type)
HistEventType type;
{
    HistEvent *hevt = (HistEvent *) xmalloc(sizeof(HistEvent));

    if (!between(0, type, NUMHEVTTYPES))
      run_warning("unknown hist event type %d", type);
    hevt->type = type;
    hevt->observers = ALLSIDES;
    hevt->next = hevt->prev = NULL;
    return hevt;
}

HistEvent *
#ifdef __STDC__
record_event(HistEventType type, SideMask observers, ...)
#else
record_event(type, observers, d1, d2, d3, d4)
HistEventType type;
SideMask observers;
long d1, d2, d3, d4;
#endif
{
    int i, val;
    char *descs;
    HistEvent *hevt;
    Side *side;

    if (!recording_events)
      return NULL;
    hevt = create_historical_event(type);
    hevt->startdate = g_turn();
    /* (should set sequence number also?) */
    hevt->enddate = g_turn();
    hevt->observers = observers;
    descs = hevtdefns[type].datadescs;
#ifdef __STDC__
    {
	va_list ap;

	va_start(ap, observers);
	for (i = 0; descs[i] != '\0'; ++i) {
	    val = va_arg(ap, long);
	    switch (descs[i]) {
	      case 'S':
	        if (!between(0, val, numsides))
	          run_warning("invalid side number %d in hist event", val);
	        break;
	      case 'U':
	        /* (should check unit validity?) */
	        break;
	    }
	    hevt->data[i] = val;
	}
	va_end(ap);
    }
#else
    hevt->data[0] = d1;
    hevt->data[1] = d2;
    hevt->data[2] = d3;
    hevt->data[3] = d4;
#endif
    /* Insert the newly created event. */
    hevt->next = history;
    hevt->prev = history->prev;
    history->prev->next = hevt;
    history->prev = hevt;
    Dprintf("Recorded event %s (observed by %d)\n",
	    hevtdefns[hevt->type].name, hevt->observers);
    if (observers != NOSIDES) {
	/* Let all the observers' interfaces look at this event. */
	for_all_sides(side) {
	    if (side_in_set(side, observers)) {
		update_event_display(side, hevt, TRUE);
	    }
	}
    }
    if (any_post_event_scores)
      check_post_event_scores(hevt);
    return hevt;
}

void
record_unit_loss(unit, reason)
Unit *unit;
int reason;
{
    int lossreason;
    HistEvent *hevt;
    PastUnit *pastunit;

    pastunit = create_past_unit(unit->type, 0);
    pastunit->name = unit->name;
    pastunit->number = unit->number;
    pastunit->x = unit->x;  pastunit->y = unit->y;  pastunit->z = unit->z;
    pastunit->side = unit->side;
    rewrite_unit_references(unit->id, pastunit->id);
    hevt = record_event(reason, add_side_to_set(unit->side, NOSIDES), pastunit->id);
    lossreason = -1;
    if (unit->side) {
	switch (reason) {
	  /* (should add all loss reasons here) */
	  case H_UNIT_CAPTURED:
	    break;
	  case H_UNIT_KILLED:
	  case H_UNIT_WRECKED:
	    lossreason = 0;
	    break;
	  case H_UNIT_STARVED:
	    lossreason = 1;
	    break;
	  case H_UNIT_DISBANDED:
	    lossreason = 2;
	    break;
	  case H_UNIT_LEFT_WORLD:
	    break;
	  default:
	    break;
	}
	if (lossreason >= 0) {
	    ++(unit->side->losscounts[3 * unit->type + lossreason]);
	}
    }
}

PastUnit *
change_unit_to_past_unit(unit)
Unit *unit;
{
    PastUnit *pastunit;

    pastunit = create_past_unit(unit->type, 0);
    pastunit->name = unit->name;
    pastunit->number = unit->number;
    pastunit->x = unit->x;  pastunit->y = unit->y;  pastunit->z = unit->z;
    pastunit->side = unit->side;
    rewrite_unit_references(unit->id, pastunit->id);
    return pastunit;
}

void
record_unit_name_change(unit, newname)
Unit *unit;
char *newname;
{
    PastUnit *pastunit;

    pastunit = create_past_unit(unit->type, 0);
    pastunit->name = unit->name;
    pastunit->number = unit->number;
    pastunit->x = unit->x;  pastunit->y = unit->y;  pastunit->z = unit->z;
    pastunit->side = unit->side;
    rewrite_unit_references(unit->id, pastunit->id);
    record_event(H_UNIT_NAME_CHANGED, -1, pastunit->id, unit->id);
}

PastUnit *
create_past_unit(type, id)
int type, id;
{
    PastUnit *pastunit = (PastUnit *) xmalloc(sizeof(PastUnit));

    pastunit->type = type;
    if (id == 0)
      id = next_past_unit_id--;
    else
      next_past_unit_id = min(next_past_unit_id, id - 1);
    pastunit->id = id;
    pastunit->next = past_unit_list;
    past_unit_list = pastunit;
    return pastunit;
}

PastUnit *
find_past_unit(n)
int n;
{
    PastUnit *pastunit;

    for (pastunit = past_unit_list; pastunit != NULL; pastunit = pastunit->next) {
	if (pastunit->id == n)
	  return pastunit;
    }
    return NULL;
}

static void
rewrite_unit_references(oldid, newid)
int oldid, newid;
{
    HistEvent *hevt;

    rewrite_unit_references_in_event(history, oldid, newid);
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	rewrite_unit_references_in_event(hevt, oldid, newid);
    }
}

static void
rewrite_unit_references_in_event(hevt, oldid, newid)
HistEvent *hevt;
int oldid, newid;
{
    int i;
    char *descs;

    descs = hevtdefns[hevt->type].datadescs;
    /* Scan through the data description, looking for
       values that are references to actual units. */
    for (i = 0; descs[i] != '\0'; ++i) {
	if (descs[i] == 'U' && hevt->data[i] == oldid)
	  hevt->data[i] = newid;
    }
}

/* Indicate that history is no longer being recorded. */

void
end_history()
{
    record_event(H_LOG_ENDED, ALLSIDES);
    recording_events = FALSE;
}

HistEvent *
get_nth_history_line(side, n, nextevt)
Side *side;
int n;
HistEvent **nextevt;
{
    int i = 0;
    HistEvent *hevt;
	
    for (hevt = history->next; hevt != history; hevt = hevt->next) {
	if (side_in_set(side, hevt->observers)) {
	    if (n == 0) {
		*nextevt = hevt;
		return NULL;
	    }
	    if (i == n) {
		*nextevt = hevt->next;
		return hevt;
	    }
	    if (hevt->startdate != hevt->prev->startdate) {
		++i;
		if (i == n) {
		    *nextevt = hevt->next;
		    return NULL;
		}
	    }
	    ++i;
	}
    }
    /* Return the last event. */
    *nextevt = history;
    return history->prev;
}

/* Summarize various aspects of performance in the game, writing it
   all to a file. */

void
dump_statistics()
{
    Side *side;
    FILE *fp;

    fp = fopen(statistics_filename(), "w");
    if (fp != NULL) {
	if (1 /* records exist */) {
	    write_side_results(fp, NULL);
	    write_unit_record(fp, NULL);
	    write_combat_results(fp, NULL);
	    fprintf(fp, "\f\n");
	    for_all_sides(side) {
		write_side_results(fp, side);
		write_unit_record(fp, side);
		write_combat_results(fp, side);
		if (side->next != NULL)
		  fprintf(fp, "\f\n");
	    }
	} else {
	    fprintf(fp, "No statistics were kept.\n");
	}
	fclose(fp);
    } else {
	run_warning("Can't open statistics file \"%s\"",
		    statistics_filename());
    }
}

/* Short, unreadable, but greppable listing of past unit.  Primarily useful
   for debugging and warnings.  We use several buffers and rotate between
   them so we can call this more than once in a single printf. */

char *
past_unit_desig(pastunit)
PastUnit *pastunit;
{
    char *shortbuf;

    if (pastunit == NULL)
      return "no pastunit";
    /* Allocate if not yet done so. */
    if (pastbufs[curpastbuf] == NULL)
      pastbufs[curpastbuf] = xmalloc(BUFSIZE);
    shortbuf = pastbufs[curpastbuf];
    curpastbuf = (curpastbuf + 1) % NUMPASTBUFS;
    if (pastunit->id == -1) {
	sprintf(shortbuf, "s%d head", side_number(pastunit->side));
	return shortbuf;
    } else if (is_unit_type(pastunit->type)) {
	sprintf(shortbuf, "s%d %-3.3s %d (%d,%d",
		side_number(pastunit->side),
		shortest_unique_name(pastunit->type),
		pastunit->id, pastunit->x, pastunit->y);
	if (pastunit->z != 0)
	  tprintf(shortbuf, ",%d", pastunit->z);
	strcat(shortbuf, ")");  /* close out the pastunit location */
	return shortbuf;
    } else {
	return "!garbage pastunit!";
    }
}

/* Would be faster to stash these, but enough difference to care? */

int
total_gain(side, u)
Side *side;
int u;
{
    int i, total = 0;

    for (i = 0; i < 3; ++i)
      total += side_gain_count(side, u, i);
    return total;
}

int
total_loss(side, u)
Side *side;
int u;
{
    int i, total = 0;

    for (i = 0; i < 3; ++i)
      total += side_loss_count(side, u, i);
    return total;
}


