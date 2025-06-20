/* Definitions for the historical record.
   Copyright (C) 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

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
    int observers[MAXSIDES];
#endif
    struct a_histevent *cause;
    struct a_histevent *next;
    struct a_histevent *prev;
    long data[4];
} HistEvent;

/* This is a snapshot of key bits of a unit's state at a particular
   moment. */

typedef struct a_pastunit {
    short type;                /* type */
    short id;                  /* truly unique id number */
    char *name;                /* the name, if given */
    long number;               /* semi-unique number */
    short x, y, z;             /* position of unit in world */
    struct a_side *side;       /* whose side this unit is on */
    struct a_pastunit *next;
} PastUnit;

extern HevtDefn hevtdefns[];

extern HistEvent *history;

extern PastUnit *past_unit_list;

extern void init_history PROTO ((void));
extern HistEvent *create_historical_event PROTO ((HistEventType type));
extern HistEvent *record_event PROTO ((HistEventType type, SideMask observers, ...));
extern void record_unit_loss PROTO ((Unit *unit, int reason));
extern void record_unit_name_change PROTO ((Unit *unit, char *newname));
extern void end_history PROTO ((void));
extern HistEvent *get_nth_history_line PROTO ((Side *side, int n, HistEvent **nextevt));
extern PastUnit *create_past_unit PROTO ((int type, int id));
extern PastUnit *find_past_unit PROTO ((int n));
extern char *past_unit_desig PROTO ((PastUnit *pastunit));
extern PastUnit *change_unit_to_past_unit PROTO ((Unit *unit));
extern void dump_statistics PROTO ((void));

