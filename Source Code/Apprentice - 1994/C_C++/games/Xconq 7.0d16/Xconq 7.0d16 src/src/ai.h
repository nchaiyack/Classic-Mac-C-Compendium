/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions common to all AIs. */

typedef enum {
    nobrains = 0,
    mplayertype = 1
    /* plus other types as desired */
} AIType;

/* Definition common to all ai type. (?) */

typedef struct a_ai {
  AIType type;
} AI;

#define side_ai_type(s) (((AI *) (s)->ai)->type)

/* Add declaration of AI hooks here. */

extern char *aitypenames[];
