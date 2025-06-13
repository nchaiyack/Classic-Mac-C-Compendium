/* Definitions for game modules in Xconq.
   Copyright (C) 1991, 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* A variant describes an option that is available to players starting
   up a game, as well as modules including each other. */

typedef struct a_variant {
    Obj *id;                    /* unique id */
    char *name;                 /* displayable name */
    Obj *dflt;                  /* default value */
    Obj *range;                 /* description of range of values */
    Obj *cases;                 /* actions to do on matches */
    int used;                   /* true if the variant has been set to a value already */
} Variant;

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
    Variant *variants;          /* array of player-choosable variants */
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

/* Declarations of module functions. */

extern void clear_game_modules PROTO ((void));
extern Module *create_game_module PROTO ((char *name));
extern Module *find_game_module PROTO ((char *name));
extern Module *get_game_module PROTO ((char *name));
extern Module *add_game_module PROTO ((char *name, Module *includer));
extern void describe_game_modules PROTO ((int arg, char *key, char *buf));
extern void describe_game_module_aux PROTO ((char *buf, Module *module, int level));
extern void describe_module_notes PROTO ((char *buf, Module *module));
extern void load_default_game PROTO ((void));
extern int load_game_description PROTO ((Module *module));
extern void load_game_module PROTO ((Module *module, int dowarn));
extern void load_base_module PROTO ((Module *module));
extern int open_module PROTO ((Module *module, int dowarn));
extern void read_forms PROTO ((Module *module));
extern void init_module_reshape PROTO ((Module *module));
extern int reshape_the_output PROTO ((Module *module));
extern int valid_reshape PROTO ((Module *module));
extern void close_module PROTO ((Module *module));
extern char *module_desig PROTO ((Module *module));







