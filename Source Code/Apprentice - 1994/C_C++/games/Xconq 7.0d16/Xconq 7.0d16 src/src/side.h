/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Doctrine is policy info that units use to help decide behavior. */

typedef struct a_doctrine {
    int everaskside;            /* should this unit ever ask for orders? */
    int wakeintransport;     /* be awake while in transport? */
    int avoidbadterrain;     /* avoid dangerous/unproductive terrain? */
    /* need something generic for event wakeup */
    int lowsupplyreturn;     /* return to base if supplies low? */
    short supplyspare;        /* how close to cut low supplies */
    int repairreturn;        /* return to base for repairs if damaged? */
    int lowammoretreat;      /* run away if ammo is low? */
    int lowammoreturn;       /* return to base if ammo is low? */
    short generosity;         /* how much supply to share with others */
    short fillpercentage;     /* how many passengers to wait for */
    int expendable;          /* OK to risk destruction in combat? */
    short ifnearbycombat;
    short ifnearby[MAXUTYPES]; /* how to react to utype nearby */
    short aggressiveness;     /* general aggressiveness */
    short wakeupinterval;
    short replaninterval;     /* how often to recompute plans */
    short exploreimportance;  /* priority of exploration */
    int exploreedges;        /* explore by going around edges? */
    int buildalways;
    /* something for move order? */
    short maxsubordinates;    /* general limit on number of subords */
    short maxsubordtypes[MAXUTYPES];  /* per-type limit on subords */
    struct a_doctrine *fallback;  /* what to use if this one says nothing */
} Doctrine;

/* More convenient access of unit doctrine info. */

#define u_doctrine(side, u, slot)  \
  ((((side)->udoctrine)[u]).slot)

#define units_doctrine(unit, slot)  \
  ((((unit)->side->udoctrine)[(unit)->type]).slot)

/* Each Xconq player is a "side" - more or less one country.  A side may or
   may not be played by a person, and may or may not have a display attached
   to it.  Each side has a different view of the world.  */

typedef struct a_side {
    char *name;               /* proper name of this side */
    char *longname;           /* the long name of this side */
    char *shortname;          /* the long name of this side */
    char *noun;               /* the noun describing a member of this side */
    char *pluralnoun;         /* the noun describing several members */
    char *adjective;          /* adjective for members of this side */
    char *colorscheme;        /* names of the side's characteristic colors */
    char *emblemname;         /* name of the side's emblem */
    char *sideclass;          /* general type or class description */
    int id;                   /* a unique id */
    Obj *symbol;              /* a symbol bound to side's id */
    short controlledbyid;     /* id of controlling side */
    struct a_side *controlledby;
    short *trusts;            /* True if side trusts another side */
    short *trades;
    short *startwith;
    short *counts;            /* array of numbers for identifying units */
    short *tech;              /* tech level for each unit type */
    short *inittech;          /* tech level at beginning of turn */
    short selfunitid;
    struct a_unit *selfunit;  /* unit that embodies side */
    short *terrview;          /* ptr to view of terrain */
    short *unitview;          /* ptr to view of units */          
    short *unitviewdate;      /* ptr to dates of views of units */          
    short *mview;             /* ptr to view of cell materials */
    short *weatherview;       /* ptr to view of weather */
    short ingame;             /* true if side participating in game */
    short status;             /* -1/0/1 for lost/draw/won */
    short lost;               /* true if this side was knocked out */
    short *scores;            /* an array of scores managed by scorekeepers */
    short turntimeused;       /* seconds used this turn */
    short totaltimeused;      /* total seconds used */
    short timeouts;           /* total timeouts used */
    short timeoutsused;       /* total timeouts used */
    short finishedturn;       /* true when side wants to go to next turn */
    short designer;           /* true if side is doing scenario design */
    short startx, starty;     /* approx center of side's "country" */
    short nameslocked;
    char **unitnamers;
    Obj *featurenamers;
    short advantage;          /* actual advantage */
    short minadvantage;       /* max advantage requestable during init */
    short maxadvantage;       /* min of same */
    /* Never saved */
    short busy;               /* true when side state not saveable */
    short finalradius;
    /* Pointers to the other major structures of a side. */
    short playerid;
    struct a_player *player;  /* pointer to data about the player */
    struct a_ui *ui;          /* pointer to all the user interface data */
    struct a_ai *ai;          /* pointer to the AI making decisions. */
    /* These are all caches, should be possible to recalc as needed. */
    struct a_unit *unithead;  /* points to list of all units on this side */
    struct a_unit *actorder;  /* points to first unit to act each turn */
    short numwaiting;         /* number of units waiting to get orders */
    short *coverage;          /* indicates how many looking at this hex */
    short *spcoverage;        /* indicates num special sensors looking */
    short numunits;           /* number of units the side has */
    short numacting;          /* number of units that can do things */
    short numfinished;        /* number of units that have finished acting */

    long turnstarttime;       /* real time at start of turn */
    long lasttime;            /* when clock started counting down again */
    long turnfinishtime;      /* real time when we've finished interacting */
    long laststarttime;       /* ? */

    short itertime;           /* length of order repetition */
    Doctrine *doctrine;       /* general doctrine */
    Doctrine *udoctrine;
    short tasklimit;          /* max length of task queues */
    int respectneutrality;   /* attack neutrals automatically? */
    int trustallies;         /* rely on allies for transport etc? */
    short moverandomness;
    int movestraight;        /* try to move in a straight line? */
    short backtrackinglimit;  /* how far backwards to look for route */
    int unitseveraskside;
    int usemachinestrategy;  /* like setting humanp */
    short realtimeout;        /* how long to wait before just going ahead */
    int autofinish;          /* turn done when all units acted */
    long startbeeptime;       /* after this time, beep to signal turn start */
    int willingtodraw;       /* will this side go along with quitters? */
    struct a_side *next;      /* pointer to next in list */
} Side;

/* Some convenient macros. */

#define side_in_play(side) (side == NULL || side->ingame)

/* Iteration over all sides. */

/* The first "side" is just the independent units, don't usually look at it. */

#define for_all_sides(v) for (v = sidelist->next; v != NULL; v = v->next)

/* But sometimes we need to treat independents if they were on a side. */

#define for_all_sides_plus_indep(v)  \
  for (v = sidelist; v != NULL; v = v->next)

/* Manipulation of numbers encoding a side's view data. */

#define terrain_view(s,x,y)  \
 ((s)->terrview ? aref((s)->terrview, x, y) : terrain_at(x, y))

#define set_terrain_view(s,x,y,v) aset((s)->terrview, x, y, v)

#define buildtview(t) ((t) + 1)

#define vterrain(v) ((v) - 1)

#define UNSEEN (0)

#define terrain_view_date(s,x,y)  (-1)

#define set_terrain_view_date(s,x,y,v) /* aset((s)->unitviewdate, x, y, v) */

/* Basic view is encoded as hhhhhssssuuuuuuu + 1, where the u's are the unit
   type, s's are the side number, and h's are hp.  0 == empty. */
   
#define unit_view(s,x,y)  \
  ((s)->unitview ? aref((s)->unitview, x, y) : EMPTY)

#define set_unit_view(s,x,y,v) aset((s)->unitview, x, y, v)

#define builduview(s,u) ((((s) << 7) | (u)) + 1)

#define vside(v) ((((v) - 1) >> 7) & 0x0f)

#define vtype(v) (((v) - 1) & 0x7f)

#define EMPTY (0)

#define appears_empty(v) ((v) == EMPTY)

#define vside_indep(v) (vside(v) == 0)

#define unit_view_date(s,x,y)  \
  ((s)->unitviewdate ?  aref((s)->unitviewdate, x, y) : (-1))

#define set_unit_view_date(s,x,y,v) aset((s)->unitviewdate, x, y, v)

/* (should add material and weather views here) */

/* Basic manipulation of normal and specialized vision coverage. */

#define cover(s,x,y) (((s)->coverage)[area.width*(y)+(x)])

#define set_cover(s,x,y,v) (((s)->coverage)[area.width*(y)+(x)] = (v))

#define add_cover(s,x,y,v) (((s)->coverage)[area.width*(y)+(x)] += (v))

#define sp_cover(s,x,y) (((s)->spcoverage)[area.width*(y)+(x)])

#define set_sp_cover(s,x,y,v) (((s)->spcoverage)[area.width*(y)+(x)] = (v))

#define add_sp_cover(s,x,y,v) (((s)->spcoverage)[area.width*(y)+(x)] += (v))

/* Tests of who/what runs the side. */

#define side_wants_display(s) ((s)->player && (s)->player->displayname)

#define side_wants_ai(s) ((s)->player && (s)->player->aitypename)

#define side_has_display(s) (((s)->ui) != NULL)

#define side_has_ai(s) (((s)->ai) != NULL)

#define side_lost(s) ((s) != NULL && !(s)->ingame && (s)->status < 0)

#define side_drew(s) ((s) != NULL && !(s)->ingame && (s)->status == 0)

#define side_won(s) ((s) != NULL && !(s)->ingame && (s)->status > 1)

/* Side-related variables. */

extern Side *sidelist;
extern Side *lastside;
extern Side *indepside;
extern Side *curside;
extern Side *tmpside;

extern int numsides;
extern int numplayers;
extern int numdesigners;

/* Definition of a "agreement", which is the generic concept of an agreement
   between sides. */

typedef struct a_agreement {
    char *typename;           /* a descriptive general name */
    char *name;               /* the specific name of this agreement. */
    int active;               /* is this agreement in effect */
    int proposer;
#if 0 /* (MAXSIDES < 32) */
    long signers;
    long willing;
    long knownto;
#else
    int signers[MAXSIDES];
    int willing[MAXSIDES];
    int knownto[MAXSIDES];
#endif
    struct a_obj *terms;
    int enforcement;
    struct a_agreement *next;
} Agreement;

/* Iteration over all agreements in the game. */

#define for_all_agreements(v) for (v = agreementlist; v != NULL; v = v->next)

#define any_agreements() (agreementlist != NULL)

#define side_signed_agreement(side,ag) ((ag)->signers[side_n(side)])

#define side_willing_agreement(side,ag) ((ag)->willing[side_n(side)])

#define side_knows_about_agreement(side,ag) ((ag)->knownto[side_n(side)])

extern int numagreements;

extern Agreement *agreementlist;
extern Agreement *lastagreement;

Agreement *create_agreement();
