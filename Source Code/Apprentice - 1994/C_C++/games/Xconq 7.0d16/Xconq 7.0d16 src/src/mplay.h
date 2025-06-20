/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* All the definitions needed by the mplayer AI. */

#define MAXGOALS 10

#define MAXTHEATERS 98

/* Strategy is what a side uses to make decisions. */

typedef struct a_strategy {
    int type;  /* placeholder */
    int trytowin;
    int creationdate;
    short strengths[MAXSIDES][MAXUTYPES];  /* estimated numbers of units */
    short alstrengths[MAXSIDES][MAXUTYPES];  /* numbers in alliances */
    short contacted[MAXSIDES+1];
    short homefound[MAXSIDES+1];
    int analyzegame;
    struct a_theater *theaters;
    struct a_theater **theatertable;
    short numtheaters;
    char *areatheaters;
    struct a_theater *homefront;
    struct a_theater *perimeters[NUMDIRS];
    struct a_theater *midranges[NUMDIRS];
    struct a_theater *remotes[NUMDIRS];
    int numgoals;
    struct a_goal *goals[MAXGOALS];
    /* Exploration and search slots. */
    int zonewidth, zoneheight;
    int numzonex, numzoney;     /* dimensions of search zone array */
    int numzones;
    struct a_searchzone *searchzones;
    short *explorertypes;
    short explorersneeded;
    short *terrainguess;
    short cx, cy;               /* "centroid" of all our units */
    short *demand;              /* worth of each utype w.r.t. strategy */
    int explore_priority;
    int defend_priority;
    int attack_priority;
    struct a_unit *unitlist[MAXUTYPES];   /* lists to help mplay efficiency */
    short unitlistcount[MAXUTYPES];  /* counts of above lists */
    short *actualmix;
    short *expectedmix;
    short *idealmix;
} Strategy;

#define mplayer(s) ((Strategy *) (s)->ai)

extern int delaymove;

/* A Theater is a sub-area that can be planned for all at once. */

/* To save space in theater layer, no more than 127 theaters may exist at once.
   This should be sufficient, even a Napoleon would have trouble keeping track
   of so much activity. */

typedef struct a_theater {
    short id;
    char *name;             /* an informative name for this theater */
    short x, y;             /* center of the theater */
    short xmin, ymin;       /*  */
    short xmax, ymax;       /*  */
    long size;		    /* number of cells in the theater */
    short importance;       /* 0 = shrug, 100 = critical */
    Goal *maingoal;
    short allied_units;     /* How many units on our side here. */
    short allied_makers;    /* How many of our makers */
    short neutral_makers;    /* How many of neutral makers */
    short makers;           /* Total number of makers */
    short unexplored;       /* Number of unseem hexes in theater. */
    short allied_bases;     /* Total number of our bases, includes towns */
    short border;           /* True if this is a border theater. */
    short nearby;           /* Is this close to a location we have bases. */
    short reinforce;         /* Priority on request for units. */
    int enemy_seen_recently; /* How many enemy have we seen in last */
    /* turn. */
    short numassigned[MAXUTYPES]; /* num of each type assigned to theater */
    short numneeded[MAXUTYPES];  /* units we should move to theater. */
    short numtotransport[MAXUTYPES]; /* types needing transportation. */
    short numenemies[MAXUTYPES];
    short numsuspected[MAXUTYPES];
    short numsuspectedmax[MAXUTYPES];
    long *people;           /* number of populated cells seen */
    int enemystrengthmin;   /* Estimate of enemy unit strength */
    int enemystrengthmax;   /* Estimate of enemy unit strength */
    short units_lost;       /* How many units have we lost here. */
    /* Weighted average of last few turns. */
    short capturers_approaching; /* number of units comming to capture bases */
    short capture_time;     /* When do we expect to have capturers here */
    short safe_area;        /* do we really need to move units here */
    struct a_theater *next;
} Theater;

#define for_all_theaters(s,th)  \
  for ((th) = mplayer(s)->theaters; (th) != NULL; (th) = (th)->next)

/* values for unit_request */
#define NO_UNITS 0
#define GUARD_BASE 1
#define PATROL_OPAREA 2
#define EXPLORE_OPAREA 3
#define DEFEND_OPAREA 4
#define GUARD_TOWN 6
#define GUARD_BORDER 2
#define GUARD_BORDER_TOWN 10
#define DEFEND_BASE 20
#define DEFEND_TOWN 50

/* Var and fn decls. */

extern int unit_count;
extern int worths_known;

extern int evaluate_hex(), maximize_worth();

/* General collections of numbers used by all machine players. */

extern int *localworth;                /* for evaluation of nearby hexes */

extern int can_move_in_dir[NUMDIRS];

/* Some basic ranges for prioritizing a units tasks. */
#define EXPLORE_VAL 2500
#define FAVORABLE_COMBAT 5000
#define UNFAVORABLE_COMBAT 4000
#define CAPTURE_MAKER 50000
#define CAPTURE_OTHER 20000
#define PATROL_VAL 100
#define HEAD_FOR_GOAL 2300
#define MEET_TRANSPORT 3000

extern long bestworth, bestx, besty;

extern Unit *munit;                    /* Unit being decided about */

extern Side *mside;                    /* Side whose unit is being decided about */
extern int base_building;             /* true if base building is possible. */

extern int worths_known;
extern int route_max_distance;

extern int units_nearby();
extern int survive_to_build_base(), exact_survive_to_build_base();
extern int base_nearby(), any_base_nearby();
extern int occupant_can_capture();
extern int occupant_can_capture_neighbor();
extern int find_closest_unit(), can_produce(), can_move();
extern int out_of_ammo();
extern int fullness(), survival_time();
extern int haven_p(), shop_p(), good_haven_p();
extern Task *find_route(), *find_route_aux(), *make_route_chain();
extern Task *make_route_step();
extern int follow_plan();
extern int unit_strength();
extern void update_opareas();

#define theater_at(s,x,y)  \
  (mplayer(s)->theatertable[mplayer(s)->areatheaters[(x)+area.width*(y)]])

#define set_theater_at(s,x,y,th)  \
  ((mplayer(s)->areatheaters[(x)+area.width*(y)]) = (th)->id)

#define for_all_hexes_in_theater(s,x,y,th)  \
  for ((x) = theater->xmin; (x) < theater->xmax; ++(x))  \
    for ((y) = theater->ymin; (y) < theater->ymax; ++(y))  \
      if (theater_at((s), (x), (y)) == (th)

Theater *create_theater();
