/* Copyright (c) 1991-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions relating to game modules. */

/* A file module records relevant info about the module, what it included,
   how to write it out, etc. */

typedef struct a_module {
    char *name;                 /* the actual unique name of the module */
    char *title;                /* a readable display name */
    char *blurb;                /* a descriptive one-liner */
    char *picturename;          /* name of a descriptive image */
    char *basemodulename;       /* name of the module that this one includes */
    char *defaultbasemodulename;  /* what game to load if something missing */
    char *basegame;             /* what general game this is based on */
    Obj *instructions;          /* basic instructions */
    Obj *notes;                 /* player notes */
    Obj *designnotes;           /* designer notes */
    char *version;              /* the version of this module */
    char *programversion;       /* compatible program versions */
    struct a_obj *variants;     /* list of player-choosable variants */
    char *contents;             /* a string with the actual contents */
    char *sp;                   /* "string pointer" a la file pointer */
    char *filename;             /* the filename */
    FILE *fp;                   /* the stdio file buffer */
    char *hook;                 /* space for system-specific file info */
    int startlineno;            /* line number being read at start of form */
    int endlineno;              /* line number being read at end of form */
    int defall;
#ifdef DESIGNERS
    int readonly;               /* true if shouldn't be modified */
    int deftypes;               /* flags indicating what to put in a module */
    int deftables;
    int defglobals;
    int defscoring;
    int defworld;
    int defareas;
    int defareaterrain;
    int defareamisc;
    int defareaweather;
    int defareamaterial;
    int defsides;
    int defsideviews;
    int defsidedoctrines;
    int defplayers;
    int defunits;
    int defunitids;
    int defunitprops;
    int defunitacts;
    int defunitplans;
    int defhistory;
    int compresslayers;
    int maybereshape;
    int subareawidth;
    int subareaheight;
    int subareax;
    int subareay;
    int finalsubareawidth;
    int finalsubareaheight;
    int finalsubareax;
    int finalsubareay;
    int finalwidth;
    int finalheight;
    int finalcircumference;
    int filltype;
#endif /* DESIGNERS */
    int open;                  /* true if currently open */
    int loaded;                /* true if already loaded */
    struct a_module *next;      /* pointer to next module */
    struct a_module *include;   /* pointer to first included module */
    struct a_module *nextinclude;  /* pointer to next included module */
    struct a_module *lastinclude;  /* pointer to last included module */
} Module;

/* Iteration over the list of modules. */

#define for_all_modules(m)  \
  for (m = modulelist; m != NULL; m = m->next)

#define for_all_includes(m,sub)  \
  for (sub = m->include; sub != NULL; sub = sub->nextinclude)

extern Module *modulelist;
extern Module *mainmodule;

Module *create_game_module();
Module *get_game_module();
Module *add_game_module();
