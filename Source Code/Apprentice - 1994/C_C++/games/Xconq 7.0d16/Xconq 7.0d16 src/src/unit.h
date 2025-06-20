/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions about units. */

/* This structure should be small, because there may be many of them.
   Unit semantics go in this structure, while unit brains go into the
   act/plan.  Test: a unit that is like a rock and can't do anything at all
   just needs basic slots, plan needn't be allocated.  Another test:
   unit should still function correctly after its current plan has been
   destroyed and replaced with another. */

typedef struct a_unit {
    short type;                /* type */
    short id;                  /* truly unique id number */
    char *name;                /* the name, if given */
    short number;              /* semi-unique number */
    short x, y, z;             /* position of unit in world */
    struct a_side *side;       /* whose side this unit is on */
    short hp;                  /* how much more damage each part can take */
    short hp2;                 /* buffer for next value of hp */
    short cp;                  /* the state of construction of this unit */
    short cxp;                 /* the combat experience of this unit */
    struct a_unit *transport;  /* pointer to transporting unit if any */
#if (MAXMTYPES > 2)
    short *supply;             /* how much supply we're carrying (dynalloc) */
#else
    short supply[MAXMTYPES];   /* how much supply we're carrying */
#endif
#if (MAXSIDES < 30)
    long spotted;              /* which sides always see us (bit vector) */
#else
    char spotted[MAXSIDES];    /* which sides always see us */
#endif
    short *tooling;            /* level of preparation for construction */
    short *attitudes;          /* feelings towards/against sides */
    struct a_actorstate *act;  /* the unit's current actor state */
    struct a_plan *plan;       /* the unit's current plan */
    Obj *hook;                 /* placeholder for optional stuff */
    char *aihook;              /* used by AI to keep info about this unit */
    char *uihook;              /* used by interfaces for their own purposes */
    /* Following slots are never saved. */
    struct a_unit *occupant;   /* pointer to first unit being carried */
    struct a_unit *nexthere;   /* pointer to fellow occupant */
    struct a_unit *prev;       /* previous unit in list of side's units */
    struct a_unit *next;       /* next unit in list of side's units */
    short prevx, prevy;        /* where were we last */
} Unit;

/* Some convenient macros. */

/* Iteration over all units. */
/* Careful with this one, can run afoul of precedence rules since no
   brace to enclose the outer loop. */

#define for_all_units(s,v)  \
  for ((s) = sidelist; (s) != NULL; (s) = (s)->next) \
    for (v = (s)->unithead->next; v != (s)->unithead; v = v->next)

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
#define DEF_ACTION(name,CODE,args,fn,doc) CODE,

#include "action.def"

    NUMACTIONTYPES

} ActionType;

typedef struct a_actiondefn {
    ActionType typecode;
    char *name;
    char *argtypes;
    int (*dofn)();
} ActionDefn;

#define MAXACTIONARGS 4

typedef struct a_action {
    ActionType type;           /* the type of the action */
    short args[MAXACTIONARGS]; /* assorted parameters */
    short actee;               /* the unit being affected by action */
    struct a_action *next;     /* chain to next action */
} Action;

typedef struct a_actorstate {
    short initacp;         /* how much we can still do */
    short acp;         /* how much we can still do */
    short actualactions;         /* actions actually done this turn */
    short actualmoves;         /* hexes actually covered this turn */
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

Goal *create_goal();
char *goal_desig();

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
  TASK_INCOMPLETE,
  TASK_PREPPED_ACTION,
  TASK_COMPLETE
} TaskOutcome;

typedef struct a_taskdefn {
    char *name;
    char *argtypes;
} TaskDefn;

#define MAXTASKARGS 6

typedef struct a_task {
    TaskType type;             /* the kind of task we want to do */
    short args[MAXTASKARGS];   /* arguments */
    short execnum;             /* how many times this has been done */
    short retrynum;            /* number of immed failures so far */
    struct a_task *next;       /* the next task to undertake */
} Task;

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
    PlanType type;             /* general type of plan that we've got here */
    short subtype;             /* orders being followed */
    struct a_goal *maingoal;   /* the main goal of this plan */
    short active;              /* true if this plan is still being used */
    short creationtime;        /* time at which this plan was created */
    short starttime;           /* time at which this plan is to be done */
    short endtime;             /* time to deactivate this plan */
    short asleep;              /* true if the unit is doing nothing */
    short reserve;             /* true if unit waiting until next turn */
    short waitingfortasks;
    short autotask;
    struct a_task *tasks;      /* pointer to chain of sequential tasks */
    short boss;                /* id of who gives us commands */
    short deputy;              /* id of who inherits the plan if we die */
    short subords;             /* id of a unit being controlled by this one */
    short nextpeer;            /* who we like to hang out with */
    Action lastaction;         /* a copy of the last action attempted */
    short lastresult;          /* that action's outcome */
    short alarmmask;           /* bit vector of alarms currently set */
    short alarms;              /* bit vector of alarms that have triggered */
    short alarmdata[4];        /* data concerning the alarms */
    short x, y, radius;        /* dimensions of area being defended */
    short protectee;           /* id of unit being protected */
    short waitingfortransport;
    short aicontrol;
} Plan;

/* Global unit variables. */

extern Unit *unitlist;
extern Unit *tmpunit;

extern int numunits;

/* Declarations of unit-related functions. */

void init_units();
int type_can_occupy();
int can_occupy();
int can_carry();
int enter_hex();
void leave_hex();
void enter_transport();
void leave_transport();
int embark_unit();
void flush_dead_units();
void sort_units();
int low_supplies();
char *summarize_units();
Unit *create_unit PROTO((int u, int makeplan));
Unit *find_unit();
int find_unit_char();
int find_unit_name();
char *random_unit_name();
char *unit_handle();
char *short_unit_handle();
char *utype_name_n();
char *unit_desig();
Unit *first_unit();
Unit *next_unit();
void insert_unit();
void delete_unit();
int evaluate_hex();
int maximize_worth();

Unit *find_next_unit();
Unit *find_prev_unit();
Unit *find_next_actor();
Unit *find_prev_actor();

char *actorstate_desig();

extern enum sortkeys tmpsortkeys[];

UnitVector *make_unit_vector();
UnitVector *add_unit_to_vector();

/* (following should be elsewhere?) */

void free_plan();
char *plan_desig();

void init_tasks();
Task *create_task();
void free_task();
char *task_desig();

extern ActionDefn actiondefns[];

extern GoalDefn goaldefns[];

extern TaskDefn taskdefns[];

extern char *plantypenames[];

Action *create_action();
char *action_desig();

/* Declare all the action functions. */

#undef  DEF_ACTION
#define DEF_ACTION(code,name,args,FN,doc) int FN();

#include "action.def"
