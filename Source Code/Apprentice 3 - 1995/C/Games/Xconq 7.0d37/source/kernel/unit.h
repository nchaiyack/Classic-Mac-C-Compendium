/* Definitions relating to units in Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* The unit structure should be small, because there may be many of them.
   Unit semantics go in this structure, while unit brains go into the
   act/plan.  Test: a unit that is like a rock and can't do anything at all
   just needs basic slots, plan needn't be allocated.  Another test:
   unit should still function correctly after its current plan has been
   destroyed and replaced with another. */

typedef struct a_unit {
    short type;			/* type */
    short id;		  	/* truly unique id number */
    char *name;			/* the name, if given */
    long number;	       	/* semi-unique number */
    short x, y, z;	     	/* position of unit in world */
    struct a_side *side;	/* whose side this unit is on */
    short hp;		  	/* how much more damage each part can take */
    short hp2;		 	/* buffer for next value of hp */
    short cp;		  	/* state of construction */
    short cxp;		 	/* combat experience */
    short morale;	      	/* morale */
    struct a_unit *transport;	/* pointer to transporting unit if any */
#if (MAXMTYPES > 2)
    short *supply;	     	/* how much supply we're carrying (dynalloc) */
#else
    short supply[MAXMTYPES];	/* how much supply we're carrying */
#endif
#if (MAXSIDES < 30)
    long spotted;		/* which sides always see us (bit vector) */
#else
    char spotted[MAXSIDES];	/* which sides always see us (array) */
#endif
    short *tooling;	    	/* level of preparation for construction */
    short *opinions;		/* opinion of each side, own side and others */
    struct a_actorstate *act;	/* the unit's current actor state */
    struct a_plan *plan;	/* the unit's current plan */
    Obj *hook;		 	/* placeholder for optional stuff */
    char *aihook;		/* used by AI to keep info about this unit */
    char *uihook;		/* used by interfaces for their own purposes */
    /* Following slots are never saved. */
    struct a_unit *occupant;	/* pointer to first unit being carried */
    struct a_unit *nexthere;	/* pointer to fellow occupant */
    struct a_unit *prev;	/* previous unit in list of side's units */
    struct a_unit *next;	/* next unit in list of side's units */
    struct a_unit *unext;	/* next unit in list of all units */
    short prevx, prevy;		/* where were we last */
} Unit;

/* Some convenient macros. */

/* Since it is possible for a unit to change sides and therefore
   prev/next pointers while iterating using the macros below, one
   must be very careful either that unit sides can't change during
   the loop, or else to maintain a temp var that can be used to
   repair the iteration.  This also applies to units dying. */

/* Iteration over all units. */
/* Careful with this one, can run afoul of precedence rules since no
   brace to enclose the outer loop. */

#define for_all_units(v)  \
    for (v = unitlist; v != NULL; v = v->unext)

/* Iteration over all units on a given side. */

#define for_all_side_units(s,v) \
    for (v = (s)->unithead->next; v != (s)->unithead; v = v->next)

/* Iteration over all occupants of a unit (but not sub-occupants). */

#define for_all_occupants(u1,v) \
  for (v = (u1)->occupant; v != NULL; v = v->nexthere)

#define is_unit(unit) ((unit) != NULL && is_unit_type((unit)->type))

#define alive(unit) ((unit)->hp > 0)

#define indep(unit) ((unit)->side == NULL || (unit)->side == indepside)

#define completed(unit) \
  ((unit)->cp >= (u_cp((unit)->type) / u_parts((unit)->type)))

#define fullsized(unit) \
  ((unit)->cp >= u_cp((unit)->type))

/* Extractor for the actual altitude of an airborne unit. */

#define unit_alt(unit) ((unit)->z & 1 == 0 ? ((unit)->z >> 1) : 0)

/* Extractor for the connection a unit is on. */

#define unit_conn(unit) ((unit)->z & 1 == 1 ? ((unit)->z >> 1) : NONTTYPE)

/* This is true if the unit is on the board somewhere. */

#define is_present(unit) in_play(unit)

#define in_play(unit) \
  (is_unit(unit) && alive(unit) && inside_area((unit)->x, (unit)->y))

#define in_action(unit) is_active(unit)

#define is_acting(unit) is_active(unit)

#define is_active(unit) (in_play(unit) && completed(unit))

/* Extractions for the two parts of an attitude/feeling. */

#define intensity(att) (((att) >> 8) & 0xff)

#define bias(att) (((att) & 0xff) - 128)

/* A sortable vector of units, generally useful. */

/* The kinds of sort keys available for list windows. */

enum sortkeys {
    bynothing,
    bytype,
    byname,
    byactorder,
    bylocation,
    byside,
    numsortkeytypes
};

/* Can sort on as many as five keys. */

#define MAXSORTKEYS 5

typedef struct a_unitvectorentry {
    Unit *unit;
    int flag;
} UnitVectorEntry;

typedef struct a_unitvector {
    int size;
    int numunits;
    enum sortkeys sortkeys[MAXSORTKEYS];
    UnitVectorEntry units[1];
} UnitVector;

/* Types of primitive unit actions. */

typedef enum actiontype {

#undef  DEF_ACTION
#define DEF_ACTION(name,CODE,args,prepfn,dofn,checkfn,argdecl,doc) CODE,

#include "action.def"

    NUMACTIONTYPES

} ActionType;

typedef struct a_actiondefn {
    ActionType typecode;
    char *name;
    char *argtypes;
#ifdef THINK_C
    int (*dofn) PROTO ((Unit *unit, Unit *unit2, ...));
    int (*checkfn) PROTO ((Unit *unit, Unit *unit2, ...));
#else
    int (*dofn) PROTO (());
    int (*checkfn) PROTO (());
#endif
} ActionDefn;

#define MAXACTIONARGS 4

typedef struct a_action {
    ActionType type;		/* the type of the action */
    short args[MAXACTIONARGS];	/* assorted parameters */
    short actee;		/* the unit being affected by action */
    struct a_action *next;	/* chain to next action */
} Action;

typedef struct a_actorstate {
    short initacp;		/* how much we can still do */
    short acp;			/* how much we can still do */
    short actualactions;	/* actions actually done this turn */
    short actualmoves;		/* cells actually covered this turn */
    Action nextaction;
} ActorState;

#define valid(x) ((x) == A_ANY_OK)

#define has_pending_action(unit)  \
  ((unit)->act && (unit)->act->nextaction.type != A_NONE)

/* All the definitions that govern planning. */

/* A goal is a predicate object that can be tested to see whether it has
   been achieved.  As such, it is a relatively static object and may be
   shared. */

/* The different types of goals. */

typedef enum goaltype {

#undef  DEF_GOAL
#define DEF_GOAL(name,GOALTYPE,args) GOALTYPE,

#include "goal.def"

    g_t_dummy
} GoalType;

typedef struct a_goaldefn {
    char *name;
    char *argtypes;
} GoalDefn;

/* The goal structure proper. */

#define MAXGOALARGS 5

typedef struct a_goal {
    GoalType type;
    short tf;
    Side *side;
    short args[MAXGOALARGS];
} Goal;

extern Goal *create_goal PROTO ((GoalType type, Side *side, int tf));
extern int cell_unknown PROTO ((int x, int y));
extern int enemies_present PROTO ((int x, int y));
extern int goal_truth PROTO ((Side *side, Goal *goal));
extern char *goal_desig PROTO ((Goal *goal));

/* A task is a single executable element of a unit's plan.  Each task type
   is something that has been found useful or convenient to encapsulate as
   a step in a plan. */

typedef enum a_tasktype {

#undef  DEF_TASK
#define DEF_TASK(name,CODE,argtypes,fn) CODE,

#include "task.def"

    NUMTASKTYPES
} TaskType;

typedef enum a_taskoutcome {
  TASK_UNKNOWN,
  TASK_FAILED,
  TASK_IS_INCOMPLETE,
  TASK_PREPPED_ACTION,
  TASK_IS_COMPLETE
} TaskOutcome;

#define MAXTASKARGS 6

typedef struct a_task {
    TaskType type;		/* the kind of task we want to do */
    short args[MAXTASKARGS];	/* arguments */
    short execnum;		/* how many times this has been done */
    short retrynum;		/* number of immed failures so far */
    struct a_task *next;	/* the next task to undertake */
} Task;

typedef struct a_taskdefn {
    char *name;
    char *argtypes;
    TaskOutcome (*exec) PROTO ((Unit *unit, Task *task));
} TaskDefn;

/* A plan is what a single unit uses to make decisions, both for itself and
   for any other units it commands.  Any unit that can act at all has a
   plan object.  A plan collects lots of unit behavior, but its most
   important structure is the task queue, which contains a list of what
   to do next, in order. */

/* Plan types distinguish several kinds of usages. */

typedef enum plantype {

#undef  DEF_PLAN
#define DEF_PLAN(name,CODE) CODE,

#include "plan.def"

    NUMPLANTYPES
} PlanType;

typedef struct a_plan {
    PlanType type;		/* general type of plan that we've got here */
    short creationturn;		/* turn at which this plan was created */
    short startturn;		/* turn at which this plan is to be done */
    short endturn;		/* turn to deactivate this plan */
    short asleep;		/* true if the unit is doing nothing */
    short reserve;		/* true if unit waiting until next turn */
    short delayed;
    short waitingfortasks;	/* true if waiting to be given a task */
    short autotask;		/* true if will try to generate own tasks */
    short aicontrol;		/* true if an AI can operate on the unit */
    short supply_alarm;
    short supply_is_low;
    short waitingfortransport;
    struct a_goal *maingoal;	/* the main goal of this plan */
    struct a_goal *formation;	/* goal to keep in a formation */
    struct a_task *tasks;	/* pointer to chain of sequential tasks */
    /* Not saved/restored. (little value, some trouble to do) */
    struct a_unit *funit;	/* pointer to unit keeping formation */
    Action lastaction;	 	/* a copy of the last action attempted */
    short lastresult;		/* that action's outcome */
} Plan;

/* Global unit variables. */

extern Unit *unitlist;
extern Unit *tmpunit;

extern int numunits;

extern enum sortkeys tmpsortkeys[];

extern ActionDefn actiondefns[];

extern GoalDefn goaldefns[];

extern TaskDefn taskdefns[];

extern char *plantypenames[];

/* Declarations of unit-related functions. */

extern void allocate_unit_block PROTO ((void));
extern void init_units PROTO ((void));
extern Unit *create_bare_unit PROTO ((int type));
extern Unit *create_unit PROTO ((int type, int makebrains));
extern void init_unit_actorstate PROTO ((Unit *unit));
extern void init_unit_plan PROTO ((Unit *unit));
extern void init_unit_tooling PROTO ((Unit *unit));
extern void init_unit_opinions PROTO ((Unit *unit));
extern Unit *designer_create_unit PROTO ((Side *side, int u, int s, int x, int y));
extern void change_unit_type PROTO ((Unit *unit, int newtype, int reason));
extern int max_builds PROTO ((int u));
extern int enter_cell PROTO ((Unit *unit, int x, int y));
extern int can_occupy_cell PROTO ((Unit *unit, int x, int y));
extern int type_can_occupy_cell PROTO ((int u, int x, int y));
extern int can_occupy_cell_without PROTO ((Unit *unit, int x, int y, Unit *unit3));
extern int type_can_occupy_cell_without PROTO ((int u, int x, int y, Unit *unit3));
extern void enter_cell_aux PROTO ((Unit *unit, int x, int y));
extern int can_occupy PROTO ((Unit *unit, Unit *transport));
extern int can_carry PROTO ((Unit *transport, Unit *unit));
extern int type_can_occupy PROTO ((int u, Unit *transport));
extern int can_occupy_type PROTO ((Unit *unit, int u2));
extern int can_carry_type PROTO ((Unit *transport, int u));
extern void enter_transport PROTO ((Unit *unit, Unit *transport));
extern void leave_cell PROTO ((Unit *unit));
extern void leave_cell_aux PROTO ((Unit *unit));
extern void leave_transport PROTO ((Unit *unit));
extern void eject_excess_occupants PROTO ((Unit *unit));
extern void eject_occupant PROTO ((Unit *unit, Unit *occ));
extern void unit_changes_side PROTO ((Unit *unit, Side *newside, int reason1, int reason2));
extern int unit_allowed_on_side PROTO ((Unit *unit, Side *side));
extern int test_class_membership PROTO ((Obj *leaf));
extern int type_allowed_on_side PROTO ((int u, Side *side));
extern int unit_trusts_unit PROTO ((Unit *unit1, Unit *unit2));
extern int set_unit_side PROTO ((Unit *unit, Side *side));
extern void set_unit_plan_type PROTO ((Side *side, Unit *unit, int type));
extern void set_unit_asleep PROTO ((Side *side, Unit *unit, int flag, int recurse));
extern void set_unit_reserve PROTO ((Side *side, Unit *unit, int flag, int recurse));
extern void set_unit_ai_control PROTO ((Side *side, Unit *unit, int flag, int recurse));
extern void set_unit_name PROTO ((Side *side, Unit *unit, char *newname));
extern void kill_unit PROTO ((Unit *unit, int reason));
extern void kill_unit_aux PROTO ((Unit *unit, int reason));
extern void react_to_unit_loss PROTO ((Side *side, Unit *unit, int reason));
extern void dispose_of_plan PROTO ((Unit *unit));
extern void flush_dead_units PROTO ((void));
extern void put_unit_on_dead_list PROTO ((Unit *unit));
extern void flush_side_dead PROTO ((Side *side));
extern void flush_one_unit PROTO ((Unit *unit));
extern void sort_units PROTO ((void));
extern int moves_till_low_supplies PROTO ((Unit *unit));
extern char *unit_desig PROTO ((Unit *unit));
extern char *unit_desig_no_loc PROTO ((Unit *unit));
extern char *utype_name_n PROTO ((int u, int n));
extern char *shortest_unique_name PROTO ((int u));
extern char *actorstate_desig PROTO ((struct a_actorstate *as));
extern Unit *find_unit PROTO ((int n));
extern Unit *find_unit_by_name PROTO ((char *nm));
extern Unit *find_unit_by_number PROTO ((int nb));
extern Unit *find_unit_dead_or_alive PROTO ((int n));
extern int find_unit_name PROTO ((char *str));
extern Unit *first_unit PROTO ((Side *side));
extern void insert_unit PROTO ((Unit *unithead, Unit *unit));
extern void delete_unit PROTO ((Unit *unit));
extern int num_occupants PROTO ((Unit *unit));
extern int num_units_at PROTO ((int x, int y));
extern void check_all_units PROTO ((void));
extern void check_unit PROTO ((Unit *unit));
extern void designer_teleport PROTO ((Unit *unit, int x, int y));
extern UnitVector *make_unit_vector PROTO ((int initsize));
extern void clear_unit_vector PROTO ((UnitVector *vec));
extern UnitVector *add_unit_to_vector PROTO ((UnitVector *vec, Unit *unit, int flag));
extern void remove_unit_from_vector PROTO ((UnitVector *vec, Unit *unit, int pos));
extern void sort_unit_vector PROTO ((UnitVector *vec));
extern Obj *get_x_property PROTO ((Unit *unit, int subkey));
extern Obj *get_x_property_by_name PROTO ((Unit *unit, char *str));

/* Declarations of plan-related functions. */

extern void execute_plan PROTO ((Unit *unit, int try));
extern int move_into_formation PROTO ((Unit *unit));
extern void plan_offense PROTO ((Unit *unit));
extern int do_for_occupants PROTO ((Unit *unit));
extern void plan_offense_support PROTO ((Unit *unit));
extern void set_construction PROTO ((Unit *unit, int u, int num));
extern void plan_defense PROTO ((Unit *unit));
extern void plan_exploration PROTO ((Unit *unit));
extern void plan_explorer_support PROTO ((Unit *unit));
extern int victim_here PROTO ((int x, int y));
extern int worth_capturing PROTO ((Side *side, int u2, Side *oside, int x, int y));
extern int go_after_victim PROTO ((Unit *unit, int range));
extern int target_here PROTO ((int x, int y));
extern int fire_at_opportunity PROTO ((Unit *unit));
extern int resupply_if_low PROTO ((Unit *unit));
extern int rearm_if_low PROTO ((Unit *unit));
extern int supplies_here PROTO ((Unit *unit, int x, int y, int m));
extern int indep_captureable_here PROTO ((int x, int y));
extern int capture_indep_if_nearby PROTO ((Unit *unit));
extern int useful_captureable_here PROTO ((int x, int y));
extern int useful_type PROTO ((Side *side, int u));
extern int capture_useful_if_nearby PROTO ((Unit *unit));
extern int could_capture_any PROTO ((int u));
extern void plan_random PROTO ((Unit *unit));
extern void make_plausible_random_args PROTO ((char *argtypestr, int i, short *args, Unit *unit));
extern void decide_plan PROTO ((Side *side, Unit *unit));
extern int doctrine_allows_wait PROTO ((Unit *unit));
extern void wait_for_orders PROTO ((Unit *unit));
extern void decide_tasks PROTO ((Unit *unit));
extern void random_walk PROTO ((Unit *unit));
extern void reserve_unit PROTO ((Side *side, Unit *unit));
extern void wake_unit PROTO ((Unit *unit, int wakeocc, int reason, Unit *unit2));
extern void wake_area PROTO ((Side *side, int x, int y, int n, int occs));
extern void set_formation PROTO ((Unit *unit, Unit *leader, int x, int y, int dist, int flex));
extern void delay_unit PROTO ((Unit *unit, int flag));
extern int find_base PROTO ((Unit *unit, int (*pred)(void), int extra));
extern int maybe_return_home PROTO ((Unit *unit));
extern int range_left PROTO ((Unit *unit));
extern void plan_exploration_route PROTO ((Unit *unit, int x, int y));
extern int route_to PROTO ((Unit *unit, int x, int y));
extern int find_worths PROTO ((int range));
extern int attack_worth PROTO ((Unit *unit, int e));
extern int threat PROTO ((Side *side, int u, int x0, int y0));
extern int should_build_base PROTO ((Unit *unit));
extern int region_portion PROTO ((int n, int u, int *units_close, int *adjterr));
extern void pop_task PROTO ((Plan *plan));
extern int react_to_enemies PROTO ((Unit *unit));
extern int move_patrol PROTO ((Unit *unit));
extern int build_time PROTO ((Unit *unit, int prod));
extern void clear_task_agenda PROTO ((Plan *plan));
extern Plan *create_plan PROTO ((void));
extern void free_plan PROTO ((Plan *plan));
extern char *plan_desig PROTO ((Plan *plan));
extern int might_be_captured PROTO ((Unit *unit));
extern void force_global_replan PROTO ((Side *side));
extern int units_nearby PROTO ((int x, int y, int dist, int type));
extern int survive_to_build_base PROTO ((Unit *unit));
extern int exact_survive_to_build_base PROTO ((Unit *unit));
extern int base_here PROTO ((int x, int y));
extern int any_base_here PROTO ((int x, int y));
extern int neutral_base_here PROTO ((int x, int y));
extern int base_nearby PROTO ((Unit *unit, int range));
extern int any_base_nearby PROTO ((Unit *unit, int range));
extern int neutral_base_nearby PROTO ((Unit *unit, int range));
extern int occupant_could_capture PROTO ((Unit *unit, int etype));
extern int can_capture_neighbor PROTO ((Unit *unit));
extern int occupant_can_capture_neighbor PROTO ((Unit *unit));
extern int find_closest_unit PROTO ((Side *side, int x0, int y0, int maxdist, int (*pred)(void), int *rxp, int *ryp));
extern int fullness PROTO ((Unit *unit));
extern int can_build PROTO ((Unit *unit));
extern int can_move PROTO ((Unit *unit));
extern int out_of_ammo PROTO ((Unit *unit));
extern int good_haven_p PROTO ((Side *side, int x, int y));
extern int haven_p PROTO ((Unit *unit, int x, int y));
extern int shop_p PROTO ((Unit *unit, int x, int y));
extern int survival_time PROTO ((Unit *unit));
extern long regions_around PROTO ((int u, int x, int y, int center));
extern Task *make_route_step PROTO ((int x, int y, int priority));
extern Task *make_route_chain PROTO ((int sx, int sy, int tx, int ty));
extern Task *find_route_aux PROTO ((Unit *unit, int maxdist, int curlen, int fromdir, int sx, int sy, int fx, int fy, int tx, int ty, int flags));
extern int usable_cell PROTO ((Unit *unit, int x, int y));
extern Task *find_route PROTO ((Unit *unit, int maxdist, int tx, int ty));
extern Task *find_route_aux_nearest PROTO ((Unit *unit, int maxdist, int curlen, int fromdir, int sx, int sy, int fx, int fy, int (*pathpred)(void), int (*destpred)(void), int flags));
extern Task *find_route_to_nearest PROTO ((Unit *unit, int fx, int fy, int maxdist, int (*pathpred)(void), int (*destpred)(void)));
extern int explorable_cell PROTO ((int x, int y));
extern int reachable_unknown PROTO ((int x, int y));
extern int adj_known_ok_terrain PROTO ((int x, int y, Side *side, int u));
extern int explore_reachable_cell PROTO ((Unit *unit, int range));
extern int optimize_plan PROTO ((Unit *unit));
extern int should_capture_maker PROTO ((Unit *unit));
extern int no_possible_moves PROTO ((Unit *unit));
extern int adj_known_passable PROTO ((Side *side, int x, int y, int u));
extern int adj_obstacle PROTO ((int type, int x, int y));
extern int normal_completion_time PROTO ((int u, int u2));
extern int adj_unit PROTO ((int x, int y));
extern int past_halfway_point PROTO ((Unit *unit));

extern int self_build_base_for_self PROTO ((Unit *unit));

extern int operating_range_worst PROTO ((int u));
extern int operating_range_best PROTO ((int u));

extern int terrain_always_impassable PROTO ((int u, int t));

/* Declarations of task-related functions. */

extern void init_tasks PROTO ((void));
extern void allocate_task_block PROTO ((void));
extern Task *create_task PROTO ((TaskType type));
extern int fire_can_damage PROTO ((Unit *unit, Unit *unit2));
extern Unit *repair_here PROTO ((int x, int y));
extern Unit *aux_resupply_here PROTO ((Unit *unit));
extern Unit *resupply_here PROTO ((int x, int y));
extern int can_auto_resupply_self PROTO ((Unit *unit, int *materials, int numtypes));
extern TaskOutcome execute_task PROTO ((Unit *unit));
extern TaskOutcome execute_task_aux PROTO ((Unit *unit, Task *task));
extern int choose_move_dirs PROTO ((Unit *unit, int tx, int ty, int shortest, int (*dirtest)(Unit *, int), void (*dirsort)(Unit *, int *, int), int *dirs));
extern int plausible_move_dir PROTO ((Unit *unit, int dir));
extern void sort_directions PROTO ((Unit *unit, int *dirs, int numdirs));
extern void free_task PROTO ((Task *task));
extern void be_active PROTO ((Unit *unit));
extern Task *create_sentry_task PROTO ((int n));
extern void push_sentry_task PROTO ((Unit *unit, int n));
extern Task *create_moveto_task PROTO ((int x, int y));
extern void order_sentry PROTO ((Unit *unit, int n));
extern void push_moveto_task PROTO ((Unit *unit, int x, int y));
extern void order_moveto PROTO ((Unit *unit, int x, int y));
extern void set_moveto_task PROTO ((Unit *unit, int x, int y));
extern Task *create_movenear_task PROTO ((int x, int y, int dist));
extern void set_movenear_task PROTO ((Unit *unit, int x, int y, int dist));
extern void push_movenear_task PROTO ((Unit *unit, int x, int y, int dist));
extern void order_movenear PROTO ((Unit *unit, int x, int y));
extern Task *create_movedir_task PROTO ((int dir, int n));
extern void set_movedir_task PROTO ((Unit *unit, int dir, int n));
extern Task *create_build_task PROTO ((int u2, int run));
extern void push_build_task PROTO ((Unit *unit, int u2, int run));
extern Task *create_research_task PROTO ((int u2, int n));
extern void push_research_task PROTO ((Unit *unit, int u2, int n));
extern void set_hit_task PROTO ((Unit *unit, int x, int y));
extern void push_specific_hit_task PROTO ((Unit *unit, int x, int y, int u, int s));
extern void set_specific_hit_task PROTO ((Unit *unit, int x, int y, int u, int s));
extern void push_hit_task PROTO ((Unit *unit, int x, int y));
extern void set_capture_task PROTO ((Unit *unit, int x, int y));
extern void push_capture_task PROTO ((Unit *unit, int x, int y));
extern void set_resupply_task PROTO ((Unit *unit));
extern Task *create_occupy_task PROTO ((Unit *transport));
extern void push_occupy_task PROTO ((Unit *unit, Unit *transp));
extern Task *create_pickup_task PROTO ((Unit *occ));
extern void push_pickup_task PROTO ((Unit *unit, Unit *occ));
extern int tasked_unit_valid PROTO ((Unit *unit, char *tasktypename));
extern char *task_desig PROTO ((Task *task));

extern int repair_test PROTO ((int x, int y));
extern int resupply_test PROTO ((int x, int y));

#undef  DEF_ACTION
#define DEF_ACTION(name,code,args,PREPFN,dofn,CHECKFN,ARGDECL,doc)  \
  extern int PREPFN  PROTO (ARGDECL);  \
  extern int CHECKFN PROTO (ARGDECL);

#include "action.def"

