/* Definitions for sides in Xconq.
   Copyright 1987, 1988, 1989, 1991, 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* A side mask is a bit vector, where the bit position corresponds to the
   side number. */

#if MAXSIDES < 31
typedef long SideMask;
#define NOSIDES (0)
#define ALLSIDES (-1)
#define add_side_to_set(side,mask) ((mask) | (1 << ((side) ? (side)->id : 0)))
#define side_in_set(side,mask) ((mask) & (1 << ((side) ? (side)->id : 0)))
#else
not implemented yet
#endif /* MAXSIDES */

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
    int expendable;          /* OK to risk destruction in combat? */
    short rearm;
    short repair;
    short resupply;
    short ifnearbycombat;
    short ifnearby[MAXUTYPES]; /* how to react to utype nearby */
    short aggressiveness;     /* general aggressiveness */
    short wakeupinterval;
    short replaninterval;     /* how often to recompute plans */
    short exploreimportance;  /* priority of exploration */
    int exploreedges;        /* explore by going around edges? */
    int buildalways;
    short locked;
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
    short selfunitid;         /* id of unit that embodies side */
    struct a_unit *selfunit;  /* unit that embodies side */
    short *terrview;          /* ptr to view of terrain */
    short *unitview;          /* ptr to view of units */          
    short *unitviewdate;      /* ptr to dates of views of units */          
    short *mview;             /* ptr to view of cell materials */
    short *weatherview;       /* ptr to view of weather */
    short ingame;             /* true if side participating in game */
    short priority;           /* overall action priority of this side */
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
    short playerid;           /* numeric id of the player */
    struct a_player *player;  /* pointer to data about the player */
    struct a_ui *ui;          /* pointer to all the user interface data */
    struct a_ai *ai;          /* pointer to the AI making decisions. */
    /* These are all caches, should be possible to recalc as needed. */
    struct a_unit *unithead;  /* points to list of all units on this side */
    struct a_unit *actorder;  /* points to first unit to act each turn */
    short numwaiting;         /* number of units waiting to get orders */
    short *coverage;          /* indicates how many looking at this cell */
    short *numunits;          /* number of units the side has */
    short *numlive;           /* number of live & in-play units the side has */
    short numacting;          /* number of units that can do things */
    short numfinished;        /* number of units that have finished acting */

    long turnstarttime;       /* real time at start of turn */
    long lasttime;            /* when clock started counting down again */
    long turnfinishtime;      /* real time when we've finished interacting */
    long laststarttime;       /* ? */

    short itertime;           /* length of order repetition */
    Doctrine *doctrine;       /* general doctrine */
    Doctrine *udoctrine;      /* array of per-unit-type doctrines */
    short doctrineslocked;
    short tasklimit;          /* max length of task queues */
    int respectneutrality;    /* attack neutrals automatically? */
    int trustallies;          /* rely on allies for transport etc? */
    short moverandomness;
    int movestraight;         /* try to move in a straight line? */
    short backtrackinglimit;  /* how far backwards to look for route */
    int unitseveraskside;
    int usemachinestrategy;   /* like setting humanp */
    short realtimeout;        /* how long to wait before just going ahead */
    int autofinish;           /* turn done when all units acted */
    long startbeeptime;       /* after this time, beep to signal turn start */
    int willingtosave;        /* will this side go along with saving the game? */
    int willingtodraw;        /* will this side go along with quitters? */

    short *gaincounts;
    short *losscounts;
    long **atkstats;
    long **hitstats;

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

/* Basic manipulation of vision coverage layer. */

#define cover(s,x,y) (((s)->coverage)[area.width*(y)+(x)])

#define set_cover(s,x,y,v) (((s)->coverage)[area.width*(y)+(x)] = (v))

#define add_cover(s,x,y,v) (((s)->coverage)[area.width*(y)+(x)] += (v))

/* Tests of who/what runs the side. */

#define side_wants_display(s) ((s)->player && (s)->player->displayname)

#define side_wants_ai(s) ((s)->player && (s)->player->aitypename)

#define side_has_display(s) (((s)->ui) != NULL)

#define side_has_ai(s) (((s)->ai) != NULL)

#define side_lost(s) ((s) != NULL && !(s)->ingame && (s)->status < 0)

#define side_drew(s) ((s) != NULL && !(s)->ingame && (s)->status == 0)

#define side_won(s) ((s) != NULL && !(s)->ingame && (s)->status > 1)

#define side_gain_count(s,u,r) (((s)->gaincounts)[3*(u)+(r)])

#define side_loss_count(s,u,r) (((s)->losscounts)[3*(u)+(r)])

#define side_atkstats(s,a,d) ((s)->atkstats[a] ? ((s)->atkstats[a])[d] : 0)

#define side_hitstats(s,a,d) ((s)->hitstats[a] ? ((s)->hitstats[a])[d] : 0)

/* Side-related variables. */

extern Side *sidelist;
extern Side *lastside;
extern Side *indepside;
extern Side *curside;
extern Side *tmpside;

extern int numsides;
extern int numplayers;
extern int numdesigners;

/* Definition of an agreement between sides. */

typedef struct a_agreement {
    short id;                 /* a unique id */
    char *typename;           /* a descriptive general name */
    char *name;               /* the specific name of this agreement */
    int state;                /* is this agreement in effect */
    SideMask drafters;        /* sides drafting the agreement */
    SideMask proposers;       /* sides ready to propose the draft agreement */
    SideMask signers;         /* proposed list of signers */
    SideMask willing;         /* sides that have indicated agreement so far */
    SideMask knownto;         /* sides that are aware of the signed agreement */
    struct a_obj *terms;      /* list of specific terms */
    short enforcement;        /* true if program should attempt to enforce terms */
    struct a_agreement *next;
} Agreement;

enum {
    draft,                    /* agreement is circulating among drafters */
    proposed,                 /* agreement is proposed to prospective signers */
    in_force,                 /* agreement is active */
    moribund                  /* agreement has expired */
};

/* Iteration over all agreements in the game. */

#define for_all_agreements(v) for (v = agreementlist; v != NULL; v = v->next)

#define any_agreements() (agreementlist != NULL)

#define side_signed_agreement(side,ag) ((ag)->signers[side_n(side)])

#define side_willing_agreement(side,ag) ((ag)->willing[side_n(side)])

#define side_knows_about_agreement(side,ag) ((ag)->knownto[side_n(side)])

/* Agreement-related variables. */

extern int numagreements;

extern Agreement *agreementlist;
extern Agreement *lastagreement;

/* Note: Can't use the "Unit" typedef below, must use "struct a_unit". */

extern void init_sides PROTO ((void));
extern Side *create_side PROTO ((void));
extern void init_side_unithead PROTO ((Side *side));
extern int side_has_units PROTO ((Side *side));
extern void init_doctrine PROTO ((Side *side));
extern int init_view PROTO ((Side *side));
extern void calc_start_xy PROTO ((Side *side));
extern char *side_name PROTO ((Side *side));
extern char *side_adjective PROTO ((Side *side));
extern int side_number PROTO ((Side *side));
extern Side *side_n PROTO ((int n));
extern int side_controls_side PROTO ((Side *side, Side *side2));
extern int side_controls_unit PROTO ((Side *side, struct a_unit *unit));
extern int side_sees_unit PROTO ((Side *side, struct a_unit *unit));
extern int side_sees_image PROTO ((Side *side, struct a_unit *unit));
extern int num_units_in_play PROTO ((Side *side, int u));
extern int num_units_incomplete PROTO ((Side *side, int u));
extern struct a_unit *find_next_unit PROTO ((Side *side, struct a_unit *prevunit));
extern struct a_unit *find_prev_unit PROTO ((Side *side, struct a_unit *nextunit));
extern struct a_unit *find_next_actor PROTO ((Side *side, struct a_unit *prevunit));
extern struct a_unit *find_prev_actor PROTO ((Side *side, struct a_unit *nextunit));
extern struct a_unit *find_next_mover PROTO ((Side *side, struct a_unit *prevunit));
extern struct a_unit *find_prev_mover PROTO ((Side *side, struct a_unit *nextunit));
extern struct a_unit *find_next_awake_mover PROTO ((Side *side, struct a_unit *prevunit));
extern struct a_unit *find_prev_awake_mover PROTO ((Side *side, struct a_unit *nextunit));
extern int side_initacp PROTO ((Side *side));
extern int side_acp PROTO ((Side *side));
extern int side_acp_reserved PROTO ((Side *side));
extern int using_tech_levels PROTO ((void));
extern void remove_side_from_game PROTO ((Side *side));
extern int num_displayed_sides PROTO ((void));
extern void set_side_name PROTO ((Side *side, Side *side2, char *newname));
extern void become_designer PROTO ((Side *side));
extern void become_nondesigner PROTO ((Side *side));
extern int trusted_side PROTO ((Side *side1, Side *side2));
extern void set_trust PROTO ((Side *side, Side *side2, int val));
extern void set_mutual_trust PROTO ((Side *side, Side *side2, int val));
extern void set_autofinish PROTO ((Side *side, int value));
extern int enemy_side PROTO ((Side *s1, Side *s2));
extern int allied_side PROTO ((Side *s1, Side *s2));
extern int neutral_side PROTO ((Side *s1, Side *s2));
extern void set_willing_to_save PROTO ((Side *side, int flag));
extern void set_willing_to_draw PROTO ((Side *side, int flag));
extern void reveal_side PROTO ((Side *sender, Side *recipient, int *types));
extern void send_message PROTO ((Side *side, long sidemask, char *str));
extern void receive_message PROTO ((Side *side, Side *sender, char *str));
extern void all_see_occupy PROTO ((struct a_unit *unit, int x, int y, int inopen));
extern void all_see_leave PROTO ((struct a_unit *unit, int x, int y, int inopen));
extern void cover_area PROTO ((Side *side, struct a_unit *unit, int x0, int y0, int onoff));
extern void reset_coverage PROTO ((void));
extern void calc_coverage PROTO ((Side *side));
extern void reset_all_views PROTO ((void));
extern void reset_view PROTO ((Side *side));
extern void react_to_seen_unit PROTO ((Side *side, struct a_unit *unit, int x, int y));
extern void all_see_cell PROTO ((int x, int y));
extern int see_cell PROTO ((Side *side, int x, int y));
extern void see_exact PROTO ((Side *side, int x, int y));
extern int seen_border PROTO ((Side *side, int x, int y, int dir));
extern char *side_desig PROTO ((Side *side));

extern Player *add_player PROTO ((void));
extern Player *find_player PROTO ((int n));
extern void canonicalize_player PROTO ((Player *player));
extern char *player_desig PROTO ((Player *player));

extern void init_agreements PROTO ((void));
extern Agreement *create_agreement PROTO ((int id));
extern char *agreement_desig PROTO ((Agreement *ag));

extern int load_side_config PROTO ((Side *side));

extern int set_side_selfunit PROTO ((Side *side, struct a_unit *unit));

#ifdef DESIGNERS
extern void paint_view PROTO ((Side *side, int x, int y, int r, int tview, int uview));
#endif /* DESIGNERS */
