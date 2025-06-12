/* Copyright (c) 1992  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions for the historical record. */

typedef enum {

#undef  DEF_HEVT
#define DEF_HEVT(name, CODE, datadescs) CODE,

#include "history.def"

    NUMHEVTTYPES
} HistEventType;

/* This is the form of the definition of a event type. */

typedef struct a_hevt_defn {
    char *name;
    char *datadescs;
} HevtDefn;

typedef struct a_histevent {
    HistEventType type;
    int subtype;
    int summary;
    int startdate;
    int startseq;
    int enddate;
    int endseq;
#if (MAXSIDES < 32)
    long observers;
#else
    long observers[MAXSIDES];
#endif
    struct a_histevent *cause;
    struct a_histevent *next;
    struct a_histevent *prev;
    long data[4];
} HistEvent;

extern HevtDefn hevtdefns[];

extern HistEvent *history;

HistEvent *create_historical_event();
HistEvent *record_event();

/* Accessors (usable for both getting and setting) to the statistics
   arrays of a side. */

#define side_balance(s,u,r) ((s)->balance[NUMREASONS*(u) + (r)])

#define side_atkstats(s,u1,u2) ((s)->atkstats[numutypes*(u1) + (u2)])

#define side_hitstats(s,u1,u2) ((s)->hitstats[numutypes*(u1) + (u2)])

