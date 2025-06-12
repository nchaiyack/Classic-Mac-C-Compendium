/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* This is the general include file includes all the others, and adds
   a few more things that don't really go anywhere else. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"

#include "module.h"
#include "game.h"
#include "player.h"
#include "side.h"
#include "unit.h"
#include "world.h"
#include "score.h"
#include "history.h"

#include "ai.h"

/* (should be in game.h?) */

enum keywords {

#undef  DEF_KWD
#define DEF_KWD(name,CODE,value)  CODE,

#include "keyword.def"

    LAST_KEYWORD
};

#define match_keyword(ob,key) \
  (symbolp(ob) && strcmp(c_string(ob), keyword_name(key)) == 0)

/* (should be in a cmdline.h file or some such) */

enum parsestage {
    general_options,
    variant_options,
    player_options,
    interface_options,
    leftover_options
};

/* (should be in a help.h file) */

enum nodeclass {
    miscnode,
    utypenode,
    mtypenode,
    ttypenode
};

typedef struct a_helpnode {
    char *key;
    int (*fn)();
    enum nodeclass nclass;
    int arg;
    char *text;
    int textend;
    int textsize;
    struct a_helpnode *prev;
    struct a_helpnode *next;
} HelpNode;

/* (where should all these go?) */

extern int typesdefined;
extern int gamedefined;
extern int compromised;
extern HelpNode *firsthelpnode;

#include "protos.h"
