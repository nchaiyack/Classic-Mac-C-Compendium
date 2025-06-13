/* Definitions for the curses interface to Xconq.
   Copyright (C) 1986, 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#include <curses.h>

#include "imf.h"
#include "ui.h"

#define VIEWFILE "view.ccq"

#define INFOLINES 4

#define ESCAPE '\033'           /* standard abort character */
#define BACKSPACE '\010'        /* for fixing strings being entered */

/* ^A does a "transparent redraw" - can be done in middle of another
   command for instance. */

#define REDRAW '\001'

struct ccwin {
  int x, y;
  int w, h;
};

/* The program can be in a number of different "modes", which affect
   both the appearance of the screen and the interpretation of input. */

enum mode {
    SURVEY,
    MOVE,
    HELP,
    MORE,
    PROMPT,
    PROMPTXY
  };

enum listsides {
    ourside,
    ourallies,
    allsides,
    numlistsides
  };

#define append_to_buffer(x,y) strcat(x,y)

#define terrain_visible(x, y)  \
  (g_see_all() || dside->designer || terrain_view(dside, wrapx(x), y) != UNSEEN)

#define units_visible(x, y)  \
  (g_see_all() || dside->designer || cover(dside, wrapx(x), y) >= 1)

extern Side *dside;
extern int use_both_chars;
extern int follow_action;
extern int curx, cury;
extern int tmpcurx, tmpcury;
extern Unit *curunit;
extern Unit *tmpcurunit;
extern enum mode mode;
extern enum mode prevmode;
extern int itertime;
extern char inpch;
extern int cmdarg;
extern char *ustr;
extern int *uvec;
extern int *bvec;
extern int nw, nh;
extern int lastx, lasty;
extern char *text1;
extern char *text2;
extern int reqstrbeg;
extern int reqstrend;
extern struct ccwin *helpwin;
extern HelpNode *cur_help_node;
extern HelpNode *help_help_node;
extern HelpNode *topics_help_node;
extern char *helptopic;
extern char *helpstring;
extern int helpscreentop;
extern struct ccwin *datewin;
extern struct ccwin *sideswin;
extern struct ccwin *toplineswin;
extern struct ccwin *clockwin;
extern struct ccwin *mapwin;
extern struct ccwin *listwin;
extern struct ccwin *closeupwin;
extern struct ccwin *sidecloseupwin;
extern int mw, mh;
extern int vx, vy;
extern int vw, vh;
extern int vw2, vh2;
extern VP *mvp;
extern int lastvcx, lastvcy;
extern int lw, lh;
extern int sh;
extern int drawterrain;
extern int drawunits;
extern int drawnames;
extern int drawpeople;
extern enum listsides listsides;
extern int test;
extern int value;
extern int sorton;
extern int sortorder;
extern int active;
extern char *terrchars;
extern char *unitchars;
extern char unseen_char_1, unseen_char_2;

extern void init_display PROTO ((void));
extern void init_interaction PROTO ((void));
extern int wait_for_char PROTO ((void));
extern void maybe_handle_input PROTO ((int));
extern void interpret_input PROTO ((void));
extern void do_dir_2 PROTO ((int dir, int n));
extern void move_survey PROTO ((int x, int y));
extern void put_on_screen PROTO ((int x, int y));

extern int ask_bool PROTO ((char *question, int dflt));
extern int ask_unit_type PROTO ((char *question, short *possibles));
extern int ask_terrain_type PROTO ((char *question, short *possibles));
extern int ask_position PROTO ((char *prompt, int *xp, int *yp));
extern void save_cur PROTO ((void));
extern void restore_cur PROTO ((void));
extern int ask_string PROTO ((char *prompt, char *dflt, char **strp));
extern Side *ask_side PROTO ((char *prompt, Side *dflt));
extern int ask_unit PROTO ((char *prompt, Unit **unitp));
extern void make_current PROTO ((Unit *unit));
extern void interpret_help PROTO ((void));

extern void exit_cconq PROTO ((Side *side));

extern struct ccwin *create_window PROTO ((int x, int y, int w, int h));

extern int in_middle PROTO ((int x, int y));
extern void set_map_viewport PROTO ((void));
extern void set_scroll PROTO ((void));

extern void redraw PROTO ((void));
extern void show_toplines PROTO ((void));
extern void clear_toplines PROTO ((void));
extern void show_closeup PROTO ((void));
extern void show_map PROTO ((void));
extern void draw_row PROTO ((int x, int y, int len));
extern void show_game_date PROTO ((void));
extern void show_clock PROTO ((void));
extern void show_side_list PROTO ((void));
extern void show_list PROTO ((void));
extern void show_cursor PROTO ((void));
extern void show_help PROTO ((void));

extern void clear_window PROTO ((struct ccwin *win));

extern int draw_text PROTO ((struct ccwin *win, int x, int y, char *str));
extern int cur_at PROTO ((struct ccwin *win, int x, int y));

extern void cycle_list_type PROTO ((void));
extern void cycle_list_filter PROTO ((void));
extern void cycle_list_order PROTO ((void));

extern void low_notify PROTO ((char *buf));

extern int auto_attack_on_move PROTO ((Unit *, Unit *));

extern Unit *find_next_and_look PROTO ((void));

extern void execute_command PROTO ((void));

extern void xbeep PROTO ((void));

extern void dump_text_view PROTO ((Side *side));
