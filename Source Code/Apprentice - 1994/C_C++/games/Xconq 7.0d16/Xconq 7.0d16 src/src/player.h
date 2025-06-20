/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions common to all players. */

typedef struct a_player {
    short id;                 /* unique id for the player */
    char *name;               /* proper name of the player */
    char *configname;         /* name of a particular configuration */
    char *displayname;        /* name of the desired display */
    char *aitypename;         /* name of an AI type */
    short advantage;          /* player's desired initial advantage */
    char *password;           /* encrypted password of the player */
    struct a_side *side;      /* the side being played */
    struct a_player *next;    /* pointer to the next player */
} Player;

/* This is the mapping between players and sides. */

typedef struct {
    struct a_side *side;      /* the side */
    struct a_player *player;  /* the player assigned to the side */
    int locked;               /* true if the assignment can't be changed */
} Assign;

extern Player *playerlist;
extern Player *lastplayer;

extern Assign *assignments;

Player *add_player();
Player *add_default_player();
Player *find_player();
char *player_desig();
