/* Help-related definitions for Xconq.
   Copyright (C) 1991, 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

enum nodeclass {
    miscnode,
    utypenode,
    mtypenode,
    ttypenode
};

typedef struct a_helpnode {
    char *key;
    void (*fn) PROTO ((int arg, char *key, char *buf));
    enum nodeclass nclass;
    int arg;
    char *text;
    int textend;
    int textsize;
    struct a_helpnode *prev;
    struct a_helpnode *next;
} HelpNode;

extern HelpNode *firsthelpnode;

extern HelpNode *create_help_node PROTO ((void));
extern HelpNode *add_help_node PROTO ((char *key, void (*fn)(int, char *, char *), int arg, HelpNode *prevnode));
extern HelpNode *find_help_node PROTO ((HelpNode *node, char *str));
extern void describe_copyright PROTO ((int arg, char *key, char *buf));
extern void describe_topics PROTO ((int arg, char *key, char *buf));
extern void describe_news PROTO ((int arg, char *key, char *buf));
extern void init_help PROTO ((void));
extern void create_game_help_nodes PROTO ((void));
extern char *get_help_text PROTO ((HelpNode *node));
extern void describe_concepts PROTO ((int arg, char *key, char *buf));
extern void describe_game_design PROTO ((int arg, char *key, char *buf));
extern void describe_utype PROTO ((int u, char *key, char *buf));
extern void append_help_phrase PROTO ((char *buf, char *phrase));
extern void append_notes PROTO ((char *buf, Obj *notes));
extern void append_number PROTO ((char *buf, int value, int dflt));
extern void describe_mtype PROTO ((int m, char *key, char *buf));
extern void describe_ttype PROTO ((int t, char *key, char *buf));
extern void describe_scorekeepers PROTO ((int arg, char *key, char *buf));
/* extern int histogram_compare PROTO ((struct histo *h1, struct histo *h2)); */
extern void uu_table_row_desc PROTO ((char *buf, int u, int (*fn)(int, int), void (*formatter)(char *, int)));
extern void um_table_row_desc PROTO ((char *buf, int u, int (*fn)(int, int), void (*formatter)(char *, int)));
extern void ut_table_row_desc PROTO ((char *buf, int u, int (*fn)(int, int), void (*formatter)(char *, int)));
extern void print_any_news PROTO ((void));
extern void print_game_description_to_file PROTO ((FILE *fp));
