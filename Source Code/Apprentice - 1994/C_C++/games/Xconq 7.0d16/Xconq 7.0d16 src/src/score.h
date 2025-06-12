/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions about scorekeepers. */

typedef struct a_scorekeeper {
    short id;                   /* unique id number */
    char *title;                /* title by which this is displayed */
    Obj *when;                  /* times at which this will run */
    Obj *who;                   /* which sides this applies to */
    Obj *knownto;               /* which sides know about this scorekeeper */
    long initial;               /* initial value of a numeric score */
    Obj *trigger;               /* test that decides triggering */
    short triggered;            /* true when scorekeeper has been triggered */
    Obj *body;                  /* the actual effect-causing stuff */
    Obj *messages;              /* textual messages to display */
    Obj *specialeffects;        /* other things to do (raspberries, etc) */
    Obj *record;                /* how to record this in scorefile */
    Obj *notes;                 /* random notes about the scorekeeper */
    short scorenum;             /* index of this scorekeeper's score value */
    struct a_scorekeeper *next; /* pointer to the next scorekeeper */
} Scorekeeper;

/* Iteration over all scorekeepers. */

#define for_all_scorekeepers(sk)  \
  for ((sk) = scorekeepers;  (sk) != NULL;  (sk) = (sk)->next)

#define keeping_score() (numscorekeepers > 0 && numdesigners == 0)

#define recording_scores() (0)

extern Scorekeeper *scorekeepers;

extern int numscorekeepers;

Scorekeeper *create_scorekeeper();
