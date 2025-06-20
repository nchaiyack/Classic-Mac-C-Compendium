/* Definitions for graphics support not specific to any interface.
   Copyright (C) 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

#define NUMPOWERS 8

typedef struct a_vp {
	short sx, sy;
	long  totsw, totsh;
	short pxw, pxh;		    /* size of window in pixels */
	short power;			/* index to this map's magnification */
	short mag;				/* magnification of each cell (a power of 2) */
	short vcx, vcy;		    /* center of the view */
	short hw, hh;			/* pixel dims of a cell */
	short hch;				/* height of cell cell between centers */
	short uw, uh;			/* pixel dims of unit subcell */
	short angle;            /* Angle of perspective view. */
} VP;

#define hexagon_adjust(v) (area.xwrap ? 0 : ((area.height / 2) * (v)->hw) / 2)

#define REDRAW_CHAR '\001'
#define BACKSPACE_CHAR '\010'
#define ESCAPE_CHAR '\033'
#define DELETE_CHAR 0x7f

extern short mags[], hws[], hhs[], hcs[], uws[], uhs[];

extern short bwid[], bwid2[], cwid[];

extern short bsx[NUMPOWERS][7], bsy[NUMPOWERS][7];
extern short lsx[NUMPOWERS][6], lsy[NUMPOWERS][6];

extern short qx[NUMPOWERS][7], qy[NUMPOWERS][7];

extern char *dirchars;

extern ImageFamily *unseen_image;

extern Module **possible_games;

extern int numgames;

/* Function declarations. */

extern void collect_possible_games PROTO ((void));
extern void add_to_possible_games PROTO ((Module *module));

extern VP *new_vp PROTO ((void));

extern void xform_cell PROTO ((VP *vp, int x, int y, int *sxp, int *syp));
extern void xform_unit PROTO ((VP *vp, Unit *unit, int *sxp, int *syp, int *swp, int *shp));
extern void xform_unit_self PROTO ((VP *vp, Unit *unit, int *sxp, int *syp, int *swp, int *shp));
extern void xform_occupant PROTO ((VP *vp, Unit *transport, Unit *unit, int sx, int sy, int sw, int sh, int *sxp, int *syp, int *swp, int *shp));

extern void scale_vp PROTO ((VP *vp, VP *vp2, int *sxp, int *syp, int *swp, int *shp));

extern int nearest_cell PROTO ((VP *vp, int sx, int sy, int *xp, int *yp));
extern int nearest_boundary PROTO ((VP *vp, int sx, int sy, int *xp, int *yp, int *dirp));
extern int nearest_unit PROTO ((VP *vp, int sx, int sy, Unit **unitp));

extern int cell_is_visible PROTO ((VP *vp, int x, int y));
extern int unit_is_visible PROTO ((VP *vp, Unit *unit));
extern int cell_is_in_middle PROTO ((VP *vp, int x, int y));

extern int set_view_size PROTO ((VP *vp, int w, int h));
extern int set_view_position PROTO ((VP *vp, int sx, int sy));
extern int set_view_power PROTO ((VP *vp, int power));
extern int set_view_angle PROTO ((VP *vp, int angle));
extern int set_view_direction PROTO ((VP *vp, int dir));
extern int set_view_focus PROTO ((VP *vp, int x, int y));
extern void center_on_focus PROTO ((VP *vp));
extern void focus_on_center PROTO ((VP *vp));

extern void guess_elev_stuff PROTO ((void));
extern Unit *find_unit_or_occ PROTO ((VP *vp, Unit *unit, int usx, int usy, int usw, int ush, int sx, int sy));
extern Unit *find_unit_at PROTO ((VP *vp, int x, int y, int sx, int sy));

extern void pick_a_focus PROTO ((Side *side, int *xp, int *yp));

extern Unit *autonext_unit PROTO ((Side *side, Unit *unit));
extern int in_box PROTO ((int x, int y, int lx, int ly, int w, int h));
extern Unit *autonext_unit_inbox PROTO ((Side *side, Unit *unit, VP *vp));
extern int could_be_next_unit PROTO ((Unit *unit));

extern int num_active_displays PROTO ((void));

extern int advance_into_cell PROTO ((Side *side, Unit *unit, int x, int y,
				Unit *other));

extern int give_supplies PROTO ((Unit *unit, short *amts, short *rslts));
extern int take_supplies PROTO ((Unit *unit, short *amts, short *rslts));

extern ImageFamily *get_unit_type_images PROTO ((Side *side, int u,
						 void (*interp_hook)(ImageFamily *imf),
						 void (*load_hook)(ImageFamily *imf),
						 void (*default_hook)(ImageFamily *imf, int u, char *name)));
extern ImageFamily *get_terrain_type_images PROTO ((Side *side, int t,
						    void (*interp_hook)(ImageFamily *imf),
						    void (*load_hook)(ImageFamily *imf),
						    void (*default_hook)(ImageFamily *imf, int u, char *name)));
extern ImageFamily *get_unseen_images PROTO ((Side *side,
						    void (*interp_hook)(ImageFamily *imf),
						    void (*load_hook)(ImageFamily *imf),
						    void (*default_hook)(ImageFamily *imf, char *name)));
extern ImageFamily *get_emblem_images PROTO ((Side *side, Side *side2,
					      void (*interp_hook)(ImageFamily *imf),
					      void (*load_hook)(ImageFamily *imf),
						  void (*default_hook)(ImageFamily *imf, int u, char *name)));
