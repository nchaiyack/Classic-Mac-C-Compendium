/* All the definitions needed by the mplayer AI.
   Copyright (C) 1987, 88, 89, 91, 92, 93, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#define MAXGOALS 10

/* Limit on the number of theaters a single side may have. */

#define MAXTHEATERS 98

/* Strategy is what a side uses to make decisions. */

typedef struct a_strategy {
    int type;  /* placeholder */
    int trytowin;
    int creationdate;
    short strengths[MAXSIDES][MAXUTYPES];  /* estimated numbers of units */
    short points[MAXSIDES];  /* estimated point value */
    short alstrengths[MAXSIDES][MAXUTYPES];  /* numbers in alliances */
    short alpoints[MAXSIDES];  /* points in alliances */
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
   of that much activity. */

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
    short makers;           /* Total number of makers */
    short unexplored;       /* Number of unseem cells in theater. */
    short allied_bases;     /* Total number of our bases, includes towns */
    short border;           /* True if this is a border theater. */
    short nearby;           /* Is this close to a location we have bases. */
    short reinforce;         /* Priority on request for units. */
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

extern int worths_known;

/* General collections of numbers used by all machine players. */

/* Some basic ranges for prioritizing a units tasks. */
#define EXPLORE_VAL 2500
#define FAVORABLE_COMBAT 5000
#define UNFAVORABLE_COMBAT 4000
#define CAPTURE_MAKER 50000
#define CAPTURE_OTHER 20000
#define PATROL_VAL 100
#define HEAD_FOR_GOAL 2300
#define MEET_TRANSPORT 3000

extern int base_building;             /* true if base building is possible. */

extern int route_max_distance;

#define theater_at(s,x,y)  \
  (mplayer(s)->theatertable[mplayer(s)->areatheaters[(x)+area.width*(y)]])

#define set_theater_at(s,x,y,th)  \
  ((mplayer(s)->areatheaters[(x)+area.width*(y)]) = (th)->id)

#define for_all_cells_in_theater(s,x,y,th)  \
  for ((x) = theater->xmin; (x) < theater->xmax; ++(x))  \
    for ((y) = theater->ymin; (y) < theater->ymax; ++(y))  \
      if (theater_at((s), (x), (y)) == (th)

extern void mplayer_init PROTO ((Side *side));
extern void mplayer_init_turn PROTO ((Side *side));
extern void create_strategy PROTO ((Side *side));
extern void reset_strategy PROTO ((Side *side));
extern void analyze_the_game PROTO ((Side *side));
extern void determine_subgoals PROTO ((Side *side));
extern void review_theaters PROTO ((Side *side));
extern int fn_set_theater PROTO ((int x, int y));
extern void create_initial_theaters PROTO ((Side *side));
extern Theater *create_theater PROTO ((Side *side));
extern void review_goals PROTO ((Side *side));
extern void review_units PROTO ((Side *side));
extern void update_side_strategy PROTO ((Side *side));
extern void decide_theater_needs PROTO ((Side *side, Theater *theater));
extern void update_unit_plans PROTO ((Side *side));
extern void update_unit_plans_randomly PROTO ((Side *side));
extern void decide_resignation PROTO ((Side *side));
extern void give_up PROTO ((Side *side));
extern void make_estimates PROTO ((Side *side));
extern void add_goal PROTO ((Side *side, Goal *goal));
extern Goal *has_goal PROTO ((Side *side, GoalType goaltype));
extern void mplayer_decide_plan PROTO ((Side *side, Unit *unit));
extern int need_this_type_to_explore PROTO ((Side *side, int u));
/* extern int compare_weights PROTO ((struct weightelt *w1, struct weightelt *w2)); */
extern void assign_to_exploration PROTO ((Side *side, Unit *unit));
extern void assign_explorer_to_theater PROTO ((Side *side, Unit *unit, Theater *theater));
extern int need_this_type_to_build_explorers PROTO ((Side *side, int u));
extern void assign_to_explorer_construction PROTO ((Side *side, Unit *unit));
extern void assign_to_offense PROTO ((Side *side, Unit *unit));
extern void assign_to_offense_support PROTO ((Side *side, Unit *unit));
extern int type_can_build_attackers PROTO ((Side *side, int u));
extern int mplayer_preferred_build_type PROTO ((Side *side, Unit *unit, int plantype));
extern int select_by_weight PROTO ((int *arr, int numvals));
extern int need_more_transportation PROTO ((Side *side));
extern void assign_to_defense_support PROTO ((Side *side, Unit *unit));
extern int mplayer_guide_explorer PROTO ((Side *side, Unit *unit));
extern int build_base_for_self PROTO ((Side *side, Unit *unit));
extern int build_base_for_others PROTO ((Side *side, Unit *unit));
extern int build_depot_for_self PROTO ((Side *side, Unit *unit));
extern void mplayer_react_to_action_result PROTO ((Side *side, Unit *unit, int rslt));
extern void mplayer_react_to_task_result PROTO ((Side *side, Unit *unit, Task *task, int rslt));
extern void change_to_adjacent_theater PROTO ((Side *side, Unit *unit));
extern int desired_direction_impassable PROTO ((Unit *unit, int x, int y));
extern int could_be_ferried PROTO ((Unit *unit, int x, int y));
extern int carryable PROTO ((int u));
extern int accelerable PROTO ((int u));
extern int blocked_by_enemy PROTO ((Unit *unit, int x, int y));
extern void attack_blockage PROTO ((Side *side, Unit *unit, int x, int y));
extern void mplayer_finish_movement PROTO ((Side *side));
extern Unit *search_for_available_transport PROTO ((Unit *unit));
extern void rethink_plan PROTO ((Unit *unit));
extern int enemy_close_by PROTO ((Side *side, Unit *unit, int dist, int *xp, int *yp));
extern void mplayer_receive_message PROTO ((Side *side, Side *sender, char *str));
extern char *mplayer_at_desig PROTO ((Side *side, int x, int y));
extern int mplayer_theater_at PROTO ((Side *side, int x, int y));
extern void mplayer_write_state PROTO ((FILE *fp, Side *side));

extern void mplayer_init_shared PROTO ((void));
extern int basic_worth PROTO ((int u));
extern int offensive_worth PROTO ((int u));
extern int basic_hit_worth PROTO ((int u, int e));
extern int basic_capture_worth PROTO ((int u, int e));
extern int unit_strength PROTO ((int u));
extern void display_assessment PROTO ((void));
extern void mplayer_react_to_unit_loss PROTO ((Side *side, Unit *unit));
extern int is_base_for PROTO ((int u1, int u2));
extern int is_carrier_for PROTO ((int u1, int u2));
extern void set_u_is_base PROTO ((int u, int n));
extern void set_u_is_transport PROTO ((int u, int n));
extern void set_u_is_carrier PROTO ((int u, int n));
extern void set_u_is_base_builder PROTO ((int u, int n));
extern void set_u_can_make PROTO ((int u, int n));
extern void set_u_can_capture PROTO ((int u, int n));
extern void set_u_bw PROTO ((int u, int n));
extern void set_t_fraction PROTO ((int t, int n));
extern void set_uu_bhw PROTO ((int u1, int u2, int v));
extern void set_uu_bcw PROTO ((int u1, int u2, int v));
