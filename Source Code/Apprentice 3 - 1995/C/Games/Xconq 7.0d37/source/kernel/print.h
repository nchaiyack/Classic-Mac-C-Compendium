/* Definitions for printing in Xconq.
   Copyright (C) 1994 Stanley T. Shebs and Massimo Campostrini.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

typedef struct a_legend {
    int ox, oy;  /* starting point */
    int dx, dy;  /* displacement */
    float angle, dist;  /* polar displacement (redundant, but useful) */
} Legend;

/* Parameters to control view printing. */

typedef struct a_print_parameters {
    /* binary flags */
    int corner_coord;
    int terrain_dither;
    int terrain_double;
    int features;
    int cell_summary;
    int cm;  /* otherwise inches */
    /* integer flags */
    int names;
    /* lengths */
    double cell_size;
    double cell_grid_width;
    double border_width;
    double connection_width;
    double page_width;
    double page_height;
    double top_margin;
    double bottom_margin;
    double left_margin;
    double right_margin;
    /* gray levels */
    double terrain_gray;
    double enemy_gray;
} PrintParameters;

/* Prototypes. */

extern void init_ascii_print PROTO ((PrintParameters *pp));
extern void dump_ascii_view PROTO ((Side *side, PrintParameters *pp, char *filename));

extern void init_ps_print PROTO ((PrintParameters *pp));
extern void dump_ps_view PROTO ((Side *side, PrintParameters *pp, char *filename));

extern int ps_load_bitmap PROTO ((Side *side, int u));
extern int store_bitmap PROTO ((int u));
extern int make_ps_images PROTO ((void));
extern char *add_esc_char PROTO ((char ch));
extern char *add_esc_string PROTO ((char *str));
char *pad_blanks PROTO ((char *str, int n));
