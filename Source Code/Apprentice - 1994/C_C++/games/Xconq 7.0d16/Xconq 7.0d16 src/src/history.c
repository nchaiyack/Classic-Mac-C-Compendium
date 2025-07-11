/* Copyright (c) 1992  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Management of world data and game modules. */

#include "conq.h"

extern int anyposteventscores;

/* This file gets the game statistics when it's all over.  It will
   be created in the current directory. */

#define STATSFILE "stats.xconq"

HevtDefn hevtdefns[] = {

#undef  DEF_HEVT
#define DEF_HEVT(NAME, code, DATADESCS) { NAME, DATADESCS },

#include "history.def"

    { NULL, NULL }
};

/* Head of the list of events. */

HistEvent *history;

/* Game logging. */

/* (eventually allow part of log to be kept on disk instead of in memory) */
/* (log file should be appended to, name should be tweakable) */
/* (log all name changes, record old name in event) */
/* (log all relationship changes (need formal actions for these?)) */
/* (summarize some event types rather than recording each, need to designate
   duration of summarizing behavior) */
/* (a game design should be able to control size/complexity of log) */
/* (full data can go back <n> turns, then summarize data further back) */
/* (eventually log mplayer decisions instead of doing debug prints) */

/* (doctrine or general prefs specify what sorts of events get presented
   to players) */

init_history()
{
    /* The first "event" is just a marker. */
    history = create_historical_event(0);
    history->next = history->prev = history;
    record_event(H_LOG_STARTED, 0);
}

HistEvent *
create_historical_event(type)
HistEventType type;
{
    HistEvent *hevt = (HistEvent *) xmalloc(sizeof(HistEvent));

    hevt->type = type;
    hevt->observers = -1;
    hevt->next = hevt->prev = NULL;
    return hevt;
}

HistEvent *
record_event(type, observers, d1, d2, d3, d4)
HistEventType type;
int observers;
long d1, d2, d3, d4;
{
    HistEvent *hevt = create_historical_event(type);
    Side *side;

    hevt->startdate = curturn;
    /* (set sequence number also) */
    hevt->enddate = curturn;
    /* (should fill in observers of this event correctly) */
    /* Insert the newly created event. */
    hevt->next = history;
    hevt->prev = history->prev;
    history->prev->next = hevt;
    history->prev = hevt;
    Dprintf("Recorded event %s (observed by %d)\n",
	    hevtdefns[hevt->type].name, hevt->observers);
    hevt->observers = observers;
    hevt->data[0] = d1;
    hevt->data[1] = d2;
    hevt->data[2] = d3;
    hevt->data[3] = d4;
    if (observers != 0) {
    /* Let all the observers' interfaces look at this event. */
    for_all_sides(side) {
	if (1 /* side observes this */) {
	    update_event_display(side, hevt, TRUE);
	}
    }
    }
    if (anyposteventscores) {
    	check_post_event_scores(hevt);
    }
    return hevt;
}

record_unit_loss(unit, reason)
Unit *unit;
int reason;
{
    HistEvent *hevt;

    hevt = record_event(reason, -1, unit->type, 0, 0);
}

/* This is to find out how everybody did. */
/* (this is really part of history display, how to do in general?) */

print_statistics()
{
    Side *side;
    FILE *fp;

    if ((fp = fopen(STATSFILE, "w")) != NULL) {
	for_all_sides(side) {
	    print_side_results(fp, side);
	    print_unit_record(fp, side);
/*	    print_combat_results(fp, side);  */
	    if (side->next != NULL) fprintf(fp, "\f\n");
	}
	fclose(fp);
    } else {
#ifdef USE_CONSOLE
	fprintf(stderr, "Can't open statistics file \"%s\"\n", STATSFILE);
#endif
    }
}

#if 0

/* Need some code to translate history into short codes. */

struct xxx {
DEF_HEVT("unit-started-with/Ini", H_UNIT_STARTED_WITH, "U")
DEF_HEVT("unit-created/Crt", H_UNIT_CREATED, "U")
DEF_HEVT("unit-completed/Com", H_UNIT_COMPLETED, "U")
DEF_HEVT("unit-acquired/Acq", H_UNIT_ACQUIRED, "U")
DEF_HEVT("unit-captured/Cap", H_UNIT_CAPTURED, "U")
DEF_HEVT("unit-killed/Cbt", H_UNIT_KILLED, "Uu")
DEF_HEVT("unit-wrecked/Cbt", H_UNIT_WRECKED, "Uu")
DEF_HEVT("unit-vanished/Van", H_UNIT_VANISHED, "U")
DEF_HEVT("unit-garrisoned/Gar", H_UNIT_GARRISONED, "U")
DEF_HEVT("unit-disbanded/Dis", H_UNIT_DISBANDED, "U")
DEF_HEVT("unit-starved/Stv", H_UNIT_STARVED, "Um")
DEF_HEVT("unit-left-world/Lft", H_UNIT_LEFT_WORLD, "U")

}
#endif

#if 0
/* Nearly-raw combat statistics are hard to interpret, but they provide */
/* a useful check against subjective evaluation of performance. */

print_combat_results(fp, side)
FILE *fp;
Side *side;
{
    int a, d;

    fprintf(fp,
	    "Unit Performance (successes and attacks against enemy by type\n");
    fprintf(fp, "   ");
    for_all_unit_types(d) {
	fprintf(fp, "  %1s  ", u_short_name(d));
    }
    fprintf(fp, "\n");
    for_all_unit_types(a) {
	fprintf(fp, " %1s ", u_short_name(a));
	for_all_unit_types(d) {
	    if (side_atkstats(side, a, d) > 0) {
		fprintf(fp, " %4.2f",
			((float) side_hitstats(side, a, d) /
			         side_atkstats(side, a, d)));
	    } else {
		fprintf(fp, "     ");
	    }
	}
	fprintf(fp, "\n   ");
	for_all_unit_types(d) {
	    if (side_atkstats(side, a, d) > 0) {
		fprintf(fp, " %3d ", side_atkstats(side, a, d));
	    } else {
		fprintf(fp, "     ");
	    }
	}
	fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}
#endif

end_history()
{
    record_event(H_LOG_ENDED, -1, NULL);
    /* (should turn off logger entirely now?) */
}

