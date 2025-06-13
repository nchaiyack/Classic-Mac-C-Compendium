/* Graphics support not specific to any Xconq interface.
   Copyright (C) 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This file includes some very general graphics-related functionality
   that interfaces can (but are not required to) use.  For instance,
   the size and shapes of hex cells have been precalculated to provide
   a reasonable appearance at several magnifications.  Note that some
   of the algorithms in this file are abstracted from code that has been
   tuned and tweaked over many years, so it is strongly recommended that
   all new graphical interfaces use these. */

#include "conq.h"
extern UnitVector *actionvector;
#include "imf.h"
#include "ui.h"

#ifdef MAC /* temporary */
#include <Values.h>
#include <Types.h>
#include <Resources.h>
#endif

static ImageFamily *get_generic_images PROTO ((Side *side, char *name, 
					      void (*interp_hook)(ImageFamily *imf),
					      void (*load_hook)(ImageFamily *imf)));

/* The two games that should be always be available. */

char *first_game_name = "intro";

char *second_game_name = "standard";

/* The following magical arrays set all the sizes at each magnification. */

/* This is the basic cell size. */

short mags[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

/* These give the total dimensions of each hex cell, plus the vertical
   distance center-to-center.  This is all carefully calculated to make
   the cells overlap perfectly at each different magnification, assuming
   that the icons have the right shape and size. */

short hws[] = { 1, 2, 4, 8, 24, 44, 88, 174 };
short hhs[] = { 1, 2, 4, 8, 26, 48, 96, 192 };
short hcs[] = { 1, 2, 4, 8, 20, 37, 75, 148 };

/* The sizes of the unit subcells.  This is available drawing area, exact
   unit icon sizes depends on what's available. */

short uhs[] = { 1, 2, 3, 8, 16, 32, 64, 128 };
short uws[] = { 1, 2, 3, 8, 16, 32, 64, 128 };

/* Widths of borders and connections (0 implies don't draw at all). */

/* Full border width. */

short bwid[NUMPOWERS]  = { 0, 0, 1, 1, 3, 5, 7, 9 };

/* Half-width, for narrower inset borders. */

short bwid2[NUMPOWERS] = { 0, 0, 1, 1, 2, 3, 4, 5 };

/* Full connection width. */

short cwid[NUMPOWERS] = { 0, 0, 1, 1, 3, 5, 7, 9 };

/* Coordinates of the hex borders. */
/* Note that array has extra column so don't need to wrap index. */

short bsx[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  2,   4,   4,  2,  0,  0,  2 },
    {  4,   8,   8,  4,  0,  0,  4 },
    { 12,  24,  24, 12,  0,  0, 12 },
    { 22,  44,  44, 22,  0,  0, 22 },
    { 44,  88,  88, 44,  0,  0, 44 },
    { 87, 174, 174, 87,  0,  0, 87 }
};
short bsy[NUMPOWERS][7] = {
    { 0 },
    { 0 },
    {  0,   0,   4,   4,   4,  0,  0 },
    {  0,   0,   8,   8,   8,  0,  0 },
    {  0,   6,  20,  26,  20,  6,  0 },
    {  0,  11,  37,  48,  37, 11,  0 },
    {  0,  21,  75,  96,  75, 21,  0 },
    {  0,  44, 148, 192, 148, 44,  0 }
};

/* Coords of middles of each hex border (half a connection, basically). */
 
short lsx[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  1,  2,   1,  -1,  -2,  -1 },
    {  2,  4,   2,  -2,  -4,  -2 },
    {  6, 12,   6,  -6, -12,  -6 },
    { 11, 22,  11, -11, -22, -11 },
    { 22, 44,  22, -22, -44, -22 },
    { 44, 87,  44, -44, -87, -44 }
};
short lsy[NUMPOWERS][6] = {
    { 0 },
    { 0 },
    {  -2,  0,   2,   2,   0,  -2 },
    {  -4,  0,   4,   4,   0,  -4 },
    {  -9,  0,   9,   9,   0,  -9 },
    { -18,  0,  18,  18,   0, -18 },
    { -36,  0,  36,  36,   0, -36 },
    { -74,  0,  74,  74,   0, -74 }
};

short qx[NUMPOWERS][7], qy[NUMPOWERS][7];

/* The traditional direction characters. */

char *dirchars = "ulnbhy";

/* The image family for regions that are not yet discovered. */

ImageFamily *unseen_image = NULL;

/* This routine finds and lists all the games that should be listed as choices for
   the user. */

/* Note that we don't actually scan library folders looking for all possible game
   designs therein. */

/* (should go into kernel, use an MI way of collecting games) */

Module **possible_games = NULL;

int numgames = 0;

/* The comparison function for the game list puts un-formally-named
   modules at the end, plus the default sorting puts initial-lowercased
   names after uppercased ones. */

static int
module_name_compare(a1, a2)
CONST void *a1, *a2;
{
    Module *mp1, *mp2;
    int rslt;

    mp1 = *((Module **) a1);
    mp2 = *((Module **) a2);
    if (mp1->basemodulename == NULL) {
	if (mp2->basemodulename == NULL) {
	    /* Modules must always have a non-NULL name. */
	    return strcmp(mp1->name, mp2->name);
	} else {
	    rslt = strcmp(mp1->name, mp2->basemodulename);
	    if (rslt == 0) rslt = -1;
	    return rslt;
	}
    } else {
	if (mp2->basemodulename == NULL) {
	    rslt = strcmp(mp1->basemodulename, mp2->name);
	    if (rslt == 0) rslt = 1;
	    return rslt;
	} else {
	    rslt = strcmp(mp1->basemodulename, mp2->basemodulename);
	    if (rslt != 0)
	      return rslt;
	    if (mp1->title == NULL) {
		if (mp2->title == NULL) {
		    /* Modules must always have a non-NULL name. */
		    return strcmp(mp1->name, mp2->name);
		} else {
		    return 1;
		}
	    } else {
		if (mp2->title == NULL) {
		    return (-1);
		} else {
		    return strcmp(mp1->title, mp2->title);
		}
	    }
	}
    }
}

static int max_possible_games;

void
collect_possible_games()
{
    int numresources = 0, len, i;
    char *modulename = NULL, *modulecontents = NULL;
    Obj *lis;
    Module *module, *basemodule;
    FILE *fp;
    int startline = 0, endline = 0;

    if (numgames == 0 && numutypes == 0 /* !game_already_loaded() */) {
	len = 0;
	fp = open_library_file("game.dir");
	if (fp != NULL) {
	    lis = read_form(fp, &startline, &endline);
	    if (consp(lis))
	      len = length(lis);
	    fclose(fp);
	}
#ifdef MAC
	numresources = CountResources('XCgm');
#endif /* MAC */
	max_possible_games = 2 + (len + numresources) * 2;
	/* Make enough room to record all the possible games. */
	possible_games = (Module **) xmalloc(max_possible_games * sizeof(Module *));
	/* Collect the intro and standard game modules and put at head
	   of list. */
	module = get_game_module(first_game_name);
	add_to_possible_games(module);
	module = get_game_module(second_game_name);
	add_to_possible_games(module);
	/* Pick up game modules that are specified as resources. */
	for (; lis != lispnil; lis = cdr(lis)) {
	    if (!(symbolp(car(lis)) || stringp(car(lis)))) {
		/* (should warn about garbage in list) */
		continue;
	    }
	    modulename = c_string(car(lis));
	    if (modulename != NULL) {
		module = get_game_module(modulename);
		module->contents = modulecontents;
		add_to_possible_games(module);
		if (module->basemodulename != NULL) {
		    basemodule = get_game_module(module->basemodulename);
		    add_to_possible_games(basemodule);
		}
	    }
	}
#ifdef MAC
	/* Pick up game modules that are specified as resources. */
	for (i = 0; i < numresources; ++i) {
	    Handle modulehandle;
	    short moduleid;
	    ResType restype;
	    Str255 resname;

	    modulehandle = GetIndResource('XCgm', i + 1);
	    /* (should test for resource validity?) */
	    if (0 /* size > 0 */) {
		/* set modulecontents from resource */
		modulecontents = NULL;
	    }
	    /* Try to pick up module name from its resource name, otherwise
	       assume its name in its content. */
	    GetResInfo(modulehandle, &moduleid, &restype, resname);
	    if (resname[0] > 0) {
		resname[resname[0]+1] = '\0';
		modulename = copy_string((char *) resname+1);
	    } else {
		modulename = NULL;
	    }
	    if (modulename != NULL) {
		module = get_game_module(modulename);
		module->contents = modulecontents;
		add_to_possible_games(module);
		if (module->basemodulename != NULL) {
		    basemodule = get_game_module(module->basemodulename);
		    add_to_possible_games(basemodule);
		}
	    }
	}
#endif /* MAC */
	if (numgames > 3) {
	    /* Sort all but the first two games into alphabetical order
	       by displayed name. */
	    qsort(&(possible_games[2]), numgames - 2, sizeof(Module *),
		  module_name_compare);
	}
    }
}

/* Load a game's description and add it to the list of games. */

void
add_to_possible_games(module)
Module *module;
{
    int i;

    if (module != NULL) {
	if (load_game_description(module)) {
	    /* It might be that the module description supplies the real name,
	       and that the module already exists. (work on this) */
	    /* Don't add duplicate modules. */
	    for (i = 0; i < numgames; ++i) {
		if (possible_games[i] == module)
		  return;
	    }
	    if (numgames < max_possible_games) {
		possible_games[numgames++] = module;
	    }
	}
    }
}

/* Choose and return a reasonable location for map displays to start out
   centered on. */

void
pick_a_focus(side, xp, yp)
Side *side;
int *xp, *yp;
{
    int num = 0, sumx = 0, sumy = 0, tmpx, tmpy, dist, closest = area.maxdim;
    Unit *unit, *closestunit = NULL;

    if (side->startx < 0 || side->starty < 0)
      calc_start_xy(side);
    if (side->startx < 0 || side->starty < 0) {
	*xp = area.width / 2 - area.height / 4;  *yp = area.height / 2;
    } else {
	tmpx = side->startx;  tmpy = side->starty;
	/* Rescan the units to find a closest one. */
	for_all_side_units(side, unit) {
	    if (in_play(unit)) {
		/* If already got one right there, just return. */
		if (unit->x == tmpx && unit->y == tmpy) {
		    *xp = tmpx;  *yp = tmpy;
		    return;
		} else {
		    dist = distance(unit->x, unit->y, tmpx, tmpy);
		    if (dist < closest) {
			closest = dist;
			closestunit = unit;
		    }
		}
	    }
	}
	if (closestunit != NULL) {
	    /* Return the position of the unit closest to the avg position. */
	    *xp = closestunit->x;  *yp = closestunit->y;
	} else {
	    *xp = tmpx;  *yp = tmpy;
	}
    }
}

int
num_active_displays()
{
    int n = 0;
    Side *side;

    for_all_sides(side) {
	if (active_display(side))
	  ++n;
    }
    return n;
}

int cellwidthguess = -1;
int avgelev;
int vertexagg = 1;

void
guess_elev_stuff()
{
    int x, y, el;
    long count = 0, sum = 0, maxelev = -9999;

    for_all_cells(x, y) {
	++count;
	el = elev_at(x, y);
	sum += el;
	if (el > maxelev) maxelev = el;
    }
    avgelev = sum / count;
    cellwidthguess = (area.cellwidth > 1 ? area.cellwidth : max(2, (maxelev - avgelev) * 2));
}

static q_computed = FALSE;

void compute_q PROTO ((void));

void compute_q()
{
    int d, p, w;

    if (q_computed)
      return;
    for (p = 0; p < NUMPOWERS; ++p) {
	    if (p < 2)
	      continue;
	    w = bwid[p] + 1;
	    for_all_directions(d) {
		qx[p][d] = bsx[p][d] + ((hws[p] - 2 * bsx[p][d]) * w) / (2 * mags[p]);
		qy[p][d] = bsy[p][d] + ((hhs[p] - 2 * bsy[p][d]) * w) / (2 * mags[p]);
	    }
	    qx[p][NUMDIRS] = qx[p][0];
	    qy[p][NUMDIRS] = qy[p][0];
    }
    q_computed = TRUE;
}

VP *
new_vp()
{
    VP *vp;

    compute_q();  /* a hack */
    vp = (VP *) xmalloc(sizeof(VP));
    /* View at a 90 degree angle by default. */
    vp->angle = 90;
    return vp;
}

/* Given a viewport and a cell, figure out where it UL corner will be. */

void
xform_cell(vp, x, y, sxp, syp)
VP *vp;
int x, y, *sxp, *syp;
{
    extern int cellwidthguess, avgelev, vertexagg;

    if (in_area(x, y)) {
	*sxp = x * vp->hw + (y * vp->hw) / 2 - vp->sx;
#if 0 /* if two possibilities for sx, let caller make the choice and do the work */
	if (area.xwrap && *sxp < (- vp->hw))
	  *sxp += area.width * vp->hw;
#endif
	*syp = (vp->totsh - (vp->hh + y * vp->hch)) - vp->sy;
	if (vp->angle != 90 && elevations_defined()) {
	    int elev, offset;
	    if (cellwidthguess < 0) {
		guess_elev_stuff();
	    }
	    elev = elev_at(x, y) - avgelev + t_thickness(terrain_at(x, y));
	    elev *= vertexagg;
	    offset = (elev * vp->hh) / cellwidthguess;
	    *syp -= offset;
	}
    } else {
	/* Always die on this, indicates bugs that must be fixed. */
	run_error("attempting to xform %d,%d", x, y);
    }
}

void
xform_unit(vp, unit, sxp, syp, swp, shp)
VP *vp;
Unit *unit;
int *sxp, *syp, *swp, *shp;
{
    int num = 0, n = -1, sq, sx, sy, sx1, sy1, sw1, sh1;
    int x = unit->x, y = unit->y;
    Unit *unit2;

    if (unit->transport == NULL) {
	xform_cell(vp, x, y, &sx, &sy);
	/* Adjust to the unit box within the cell. */
	sx += (vp->hw - vp->uw) / 2;  sy += (vp->hh - vp->uh) / 2;
	/* Figure out our position in this cell's stack. */
	for_all_stack(x, y, unit2) {
	    if (unit == unit2) n = num;
	    ++num;
	}
	if (n < 0) run_error("xform_unit weirdness");
	if (num <= 1) {
	    sq = 1;
	} else if (num <= 4) {
	    sq = 2;
	} else if (num <= 16) {
	    sq = 4;
	} else if (num <= 256) {
	    sq = 8;
	} else {
	    /* This is room for 65,536 units in a stack. */
	    sq = 16;
	}
	*swp = vp->uw / sq;  *shp = vp->uh / sq;
	*sxp = sx + *swp * (n / sq);  *syp = sy + *shp * (n % sq);
    } else {
	/* Go up the transport chain to get the bounds for this unit. */
	xform_unit(vp, unit->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant(vp, unit->transport, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
    }
}

void
xform_unit_self(vp, unit, sxp, syp, swp, shp)
VP *vp;
Unit *unit;
int *sxp, *syp, *swp, *shp;
{
    int sx1, sy1, sw1, sh1;

    if (unit->transport == NULL) {
	if (unit->occupant == NULL) {
	    xform_unit(vp, unit, sxp, syp, swp, shp);
	} else {
	    xform_unit(vp, unit, &sx1, &sy1, &sw1, &sh1);
	    xform_occupant(vp, unit, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
	}
    } else {
	xform_unit(vp, unit->transport, &sx1, &sy1, &sw1, &sh1);
	xform_occupant(vp, unit->transport, unit, sx1, sy1, sw1, sh1, sxp, syp, swp, shp);
    }
}

void
xform_occupant(vp, transport, unit, sx, sy, sw, sh, sxp, syp, swp, shp)
VP *vp;
Unit *transport, *unit;
int sx, sy, sw, sh, *sxp, *syp, *swp, *shp;
{
    int num = 0, n = -1, nmx, nmy;
    Unit *unit2;

    /* Figure out the position of this unit amongst all the occupants. */
    for_all_occupants(transport, unit2) {
	if (unit2 == unit) n = num;
	++num;
    }
    if (unit == transport) {
	if (num > 0) {
	    /* Transport image shrinks by half in each dimension. */
	    *swp = sw / 2;  *shp = sh / 2;
	}
	/* Transport is always in the UL corner. */
	*sxp = sx;  *syp = sy;
    } else {
	if (n < 0) run_error("xform_occupant weirdness");
	/* Compute how the half-box will be subdivided.  Only use powers of two,
	   so image scaling works better. */
	if (num <= 2) {
	    nmx = 2;
	} else if (num <= 8) {
	    nmx = 4;
	} else if (num <= 128) {
	    nmx = 8;
	} else {
	    /* This is room for 32,768 units in a stack. */
	    nmy = 16;
	}
	nmy = nmx / 2;
	*swp = sw / nmx;  *shp = (sh / 2) / nmy;
	*sxp = sx + *swp * (n / nmy);  *syp = sy + sh / 2 + *shp * (n % nmy);
    }
}

/* Scale one viewport box to its position in another. */

void
scale_vp(vp, vp2, sxp, syp, swp, shp)
VP *vp, *vp2;
int *sxp, *syp, *swp, *shp;
{
    *sxp = (vp2->sx * vp->hw) / vp2->hw - vp->sx;
    *syp = (vp2->sy * vp->hch) / vp2->hch - vp->sy;
    *swp = (vp2->pxw * vp->hw) / vp2->hw;
    *shp = (vp2->pxh * vp->hch) / vp2->hch;
}

int
nearest_cell(vp, sx, sy, xp, yp)
VP *vp;
int sx, sy, *xp, *yp;
{
    /* Flip the raw y and then scale to hex coords. */
    *yp = (vp->totsh - (vp->sy + sy)) / vp->hch;
    /* Scale adjusted x to hex coord. */
    *xp = (sx + vp->sx - (*yp * vp->hw) / 2) / vp->hw;
    /* If the magnification of the map is large enough that the top and bottom
       edges of a hex are visibly sloping, then we have to take those edges
       int account, and accurately. */
    if ((vp->hh - vp->hch) / 2 > 1) {
	/* (should adjust according to hex boundaries correctly here) */
    }
    /* Wrap coords as usual. */
    if (area.xwrap)
      *xp = wrapx(*xp);
    DGprintf("Pixel %d,%d -> hex %d,%d\n", sx, sy, *xp, *yp);
    return (in_area(*xp, *yp));
}

int
nearest_boundary(vp, sx, sy, xp, yp, dirp)
VP *vp;
int sx, sy, *xp, *yp, *dirp;
{
    int sx2, sy2, ydelta, hexslope;

    /* Get the nearest cell... */
    if (nearest_cell(vp, sx, sy, xp, yp)) {
	/* ... and xform it back to get the pixel coords. */ 
	xform_cell(vp, *xp, *yp, &sx2, &sy2);
	ydelta = sy - sy2;
	hexslope = (vp->hh - vp->hch) / 2;
	if (sx - sx2 > vp->hw / 2) {
	    *dirp = ((ydelta < hexslope) ? NORTHEAST : (ydelta > vp->hch ? SOUTHEAST : EAST));
	} else {
	    *dirp = ((ydelta < hexslope) ? NORTHWEST : (ydelta > vp->hch ? SOUTHWEST : WEST));
	}
	DGprintf("Pixel %d,%d -> hex %d,%d dir %d\n", sx, sy, *xp, *yp, *dirp);
	return TRUE;
    } else {
	return FALSE;
    }
}

Unit *
find_unit_or_occ(vp, unit, usx, usy, usw, ush, sx, sy)
VP *vp;
Unit *unit;
int usx, usy, usw, ush, sx, sy;
{
    int usx1, usy1, usw1, ush1;
    Unit *occ, *rslt;

    /* See if the point might be over an occupant. */
    if (unit->occupant != NULL) {
	for_all_occupants(unit, occ) {
	    xform_unit(vp, occ, &usx1, &usy1, &usw1, &ush1);
	    rslt = find_unit_or_occ(vp, occ, usx1, usy1, usw1, ush1, sx, sy);
	    if (rslt)
	      return rslt;
	}
    }
    /* Otherwise see if it could be the unit itself.  This has the effect of
       "giving" the transport everything in its box that is not in an occ. */
    xform_unit(vp, unit, &usx1, &usy1, &usw1, &ush1);
    if (between(usx1, sx, usx1 + usw1) && between(usy1, sy, usy1 + ush1))
      return unit;
    return NULL;
}

Unit *
find_unit_at(vp, x, y, sx, sy)
VP *vp;
int x, y, sx, sy;
{
    int usx, usy, usw, ush;
    Unit *unit, *rslt;
    
    for_all_stack(x, y, unit) {
	xform_unit(vp, unit, &usx, &usy, &usw, &ush);
	rslt = find_unit_or_occ(vp, unit, usx, usy, usw, ush, sx, sy);
	if (rslt)
	  return rslt;
    }
    return NULL;
}

int
nearest_unit(vp, sx, sy, unitp)
VP *vp;
int sx, sy;
Unit **unitp;
{
    int x, y;
    
    if (!nearest_cell(vp, sx, sy, &x, &y)) {
	*unitp = NULL;
    } else if (vp->power > 4) {
	*unitp = find_unit_at(vp, x, y, sx, sy);
    } else {
	*unitp = unit_at(x, y);
    }
    DGprintf("Pixel %d,%d -> unit %s\n", sx, sy, unit_desig(*unitp));
    return 0;
}

int
cell_is_visible(vp, x, y)
VP *vp;
int x, y;
{
    int sx, sy;
 
 	if (!in_area(x, y))
 	  return FALSE;   
    xform_cell(vp, x, y, &sx, &sy);
    if (area.xwrap && sx > vp->totsw)
      sx -= vp->totsw;
    if (sx + vp->hw < 0)
      return FALSE;
    if (sx > vp->pxw) 
      return FALSE;
    if (sy + vp->hh < 0)
      return FALSE;
    if (sy > vp->pxh)
      return FALSE;
    return TRUE;
}

/* Decide whether given location is away from the edge of the map's window. */

int
cell_is_in_middle(vp, x, y)
VP *vp;
int x, y;
{
    int sx, sy, insetx1, insety1, insetx2, insety2;
    
 	if (!in_area(x, y))
 	  return FALSE;   
    xform_cell(vp, x, y, &sx, &sy);
    /* Adjust to be the center of the cell, more reasonable if large. */
    sx += vp->hw / 2;  sy += vp->hh / 2;
    insetx1 = min(vp->pxw / 4, 1 * vp->hw);
    insety1 = min(vp->pxh / 4, 1 * vp->hch);
    insetx2 = min(vp->pxw / 4, 2 * vp->hw);
    insety2 = min(vp->pxh / 4, 2 * vp->hch);
    if (sx < insetx2)
      return FALSE;
    if (sx > vp->pxw - insetx2)
      return FALSE;
    if (sy < (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    if (sy > vp->pxh - (between(2, y, area.height-3) ? insety2 : insety1))
      return FALSE;
    return TRUE;
}

/* Set vcx/vcy to point to the center of the view. */

void
focus_on_center(vp)
VP *vp;
{
    vp->vcy = (vp->totsh - (vp->sy + vp->pxh / 2)) / vp->hch;
    vp->vcx = vp->sx / vp->hw - (vp->vcy / 2) + (vp->pxw / vp->hch) / 2;
    /* Restrict the focus to be *inside* the area. */
    vp->vcy = max(1, min(vp->vcy, area.height - 2));
    if (area.xwrap) {
	vp->vcx = wrapx(vp->vcx);
    } else {
	vp->vcx = max(1, min(vp->vcx, area.width - 2));
	if (vp->vcx + vp->vcy < area.height / 2 + 1)
	  vp->vcx = area.height / 2 + 1;
	if (vp->vcx + vp->vcy > area.width + area.height / 2 - 1)
	  vp->vcx = area.width + area.height / 2 - 1;
    }
}

void
center_on_focus(vp)
VP *vp;
{
    /* Scale, add hex offset adjustment, translate to get left edge. */
    vp->sx = vp->vcx * vp->hw + (vp->vcy * vp->hw) / 2 - vp->pxw / 2 + vp->hw / 2;
    /* Scale, translate to top edge, flip. */
    vp->sy = vp->totsh - (vp->vcy * vp->hch + vp->pxh / 2 + vp->hh / 2);
    /* Weird vcx,vcy might make sx,sy nonsensical, so clip to rational
       limits. */
    vp->sx = max(hexagon_adjust(vp), min(vp->sx, vp->totsw - vp->pxw));
    vp->sy = max(0, min(vp->sy, vp->totsh - vp->pxh));
    DGprintf("View at %d,%d, focused at %d,%d\n",
	     vp->sx, vp->sy, vp->vcx, vp->vcy);
}

int
set_view_size(vp, w, h)
VP *vp;
int w, h;
{
    /* (should limit these values) */
    vp->pxw = w;  vp->pxh = h;
    return TRUE;
}

int
set_view_position(vp, sx, sy)
VP *vp;
int sx, sy;
{
    /* (should limit these values) */
    vp->sx = sx;  vp->sy = sy;
    /* Clip to rational limits. */
    vp->sx = max(hexagon_adjust(vp), min(vp->sx, vp->totsw - vp->pxw));
    vp->sy = max(0, min(vp->sy, vp->totsh - vp->pxh));
    return TRUE;
}

/* Given a magnification power, calculate and cache the sizes within a cell,
   and the scaled size in pixels of the entire world. */

int
set_view_power(vp, power)
VP *vp;
int power;
{
    vp->power = power;
    vp->mag = mags[power]; /* is this used?? */
    vp->hw = hws[power];  vp->hh = hhs[power];
    vp->hch = hcs[power];
    vp->uw = uws[power];  vp->uh = uhs[power];
    if (vp->angle == 30) {
	vp->hh /= 2;
	vp->hch /= 2;
    } else if (vp->angle == 15) {
	vp->hh /= 4;
	vp->hch /= 4;
    }
    /* Calculate and cache the width in pixels of the whole area, adding an
       an adjustment to account for the "bulge" of hexagon-shaped areas. */
    vp->totsw = area.width * vp->hw + hexagon_adjust(vp);
    /* Total scaled height is based on center-to-center height, plus an adjustment
       to include the bottom parts of the bottom row. */
    vp->totsh = area.height * vp->hch + (vp->hh - vp->hch);
    DGprintf("Power is now %d, total scaled area is %d x %d\n",
	     vp->power, vp->totsw, vp->totsh);
    return TRUE;
}

int
set_view_focus(vp, x, y)
VP *vp;
int x, y;
{
    vp->vcx = x;  vp->vcy = y;
    return TRUE;
}

int
set_view_angle(vp, angle)
VP *vp;
int angle;
{
    if (!(angle == 90 || angle == 30 || angle == 15)) {
	run_warning("Bad angle %d, setting to 90", angle);
	angle = 90;
    }
    vp->angle = angle;
    vp->hh = hhs[vp->power];
    vp->hch = hcs[vp->power];
    vp->uh = uhs[vp->power];
    if (vp->angle == 30) {
	vp->hh /= 2;
	vp->hch /= 2;
	vp->uh /= 2;
    } else if (vp->angle == 15) {
	vp->hh /= 4;
	vp->hch /= 4;
	vp->uh /= 4;
    }
    /* Total scaled height is based on center-to-center height, plus an
       adjustment to include the bottom parts of the bottom row. */
    vp->totsh = area.height * vp->hch + (vp->hh - vp->hch);
    DGprintf("Angle is now %d, total scaled area is %d x %d\n",
	     vp->angle, vp->totsw, vp->totsh);
    return TRUE;
}

int
set_view_direction(vp, dir)
VP *vp;
int dir;
{
    return TRUE;
}


/* (needs a better home?) */

/* Given a side and a unit, calculate the correct "next unit".  Typically
   used by autonext options, thus the name. */

Unit *
autonext_unit(side, unit)
Side *side;
Unit *unit;
{
    int i, uniti = -1, n;
    Unit *nextunit;

    if (!side->ingame
	|| side->finishedturn
	|| actionvector == NULL)
      return NULL;
    if (could_be_next_unit(unit) && side_controls_unit(side, unit))
      return unit;
    for (i = 0; i < actionvector->numunits; ++i) {
    	nextunit = (actionvector->units)[i].unit;
    	if (in_play(nextunit) && side_controls_unit(side, nextunit)) {
	    if (unit == NULL || unit == nextunit) {
		uniti = i;
		break;
	    }
    	}
    }
    if (uniti < 0)
      return NULL;
    /* (should scan for both a preferred and an alternate - preferred
       could be within a supplied bbox so as to avoid scrolling) */
    for (i = uniti; i < uniti + actionvector->numunits; ++i) {
    	n = i % actionvector->numunits;
    	nextunit = (actionvector->units)[n].unit;
    	if (could_be_next_unit(nextunit) && side_controls_unit(side, nextunit))
    	  return nextunit;
    }
    return NULL;
}

#if 0
int
in_box(x, y, lx, ly, w, h)
int x, y, lx, ly, w, h;
{
    if ( !between(ly, y, ly+h) )
      return FALSE;
    lx -= (y - ly) / 2;
    return between(lx, x, lx+w);
}
#endif

/*
 * This should really be called autonext_unit and the decision
 * whether to check inbox or not should depend on the bbox being
 * valid. i.e. could be called with -1,-1,-1,-1 to disable the bbox.
 */
Unit *
autonext_unit_inbox(side, unit, vp)
Side *side;
Unit *unit;
VP *vp;
{
    int i, u, mx, my, val, prefval = -999, v = 10;
    Unit *nextunit = NULL, *prefunit = NULL;

    if (!side->ingame || side->finishedturn || actionvector == NULL)
      return NULL;

    /* degenerate case... this unit still has stuff to do. */
    if (could_be_next_unit(unit) && side_controls_unit(side, unit))
	return unit;

    if (unit == NULL) {
	u = 0;
	if (!nearest_cell(vp, vp->sx + vp->pxw / 2, vp->sy + vp->pxh / 2, &mx, &my)) {
	    mx = area.width / 2;  my = area.height / 2;
	}
    } else {
	u = unit->type;
	mx = unit->x;  my = unit->y;
    }

    for (i = 0; i < actionvector->numunits; ++i) {
    	nextunit = (actionvector->units)[i].unit;
	if (side_controls_unit(side, nextunit) && could_be_next_unit(nextunit)) {
	    val = v - distance(nextunit->x, nextunit->y, mx, my);
	    if (cell_is_in_middle(vp, nextunit->x, nextunit->y))
	      val += v;
	    if (nextunit->type == u)
	      val += 2;

	    if (val > prefval) {
		prefval = val;
		prefunit = nextunit;
	    }
	}
    }
    return prefunit;
}

int
could_be_next_unit(unit)
Unit *unit;
{
    return (unit != NULL
	    && alive(unit)
	    && inside_area(unit->x, unit->y)
	    && (unit->act
		&& unit->act->acp > 0 /*
		&& !has_pending_action(unit) */)
	    && (unit->plan
		&& !unit->plan->asleep
		&& !unit->plan->reserve
		&& !unit->plan->delayed
		&& unit->plan->waitingfortasks));
}

/* Given that the player desires to move the given unit into the given
   cell/other unit, prepare a "most appropriate" action. */
   /* (should share diff cell and same cell interaction code) */
/* (should reindent) */
int
advance_into_cell(side, unit, x, y, other)
Side *side;
Unit *unit, *other;
int x, y;
{
#ifdef DESIGNERS
    /* Designers use this function to push units around, bound only by the
       limits on occupancy. */
    if (side->designer) {
	if (other != NULL && can_occupy(unit, other)) {
	    /* Teleport into a transport. */
	    leave_cell(unit);
	    enter_transport(unit, other);
	} else if (can_occupy_cell(unit, x, y)) {
	    designer_teleport(unit, x, y);
	} else {
	    return FALSE;
	}
	return TRUE;
    }
#endif /* DESIGNERS */
    if (x != unit->x || y != unit->y) {
	if (unit->act && unit->plan) { /* (should be more sophisticated test?) */
	    if (distance(unit->x, unit->y, x, y) == 1) {
		if (other == NULL) {
		    if (can_occupy_cell(unit, x, y)
			&& valid(check_move_action(unit, unit, x, y, unit->z))) {
			prep_move_action(unit, unit, x, y, unit->z);
			return TRUE;
		    } else {
			return FALSE;
		    }
		}
		if (unit_trusts_unit(unit, other)) {
		    /* A friend, maybe get on it. */
		    if (can_occupy(unit, other)) {
			if (valid(check_enter_action(unit, unit, other))) {
			    prep_enter_action(unit, unit, other);
			} else {
			    /* (should schedule for next turn?) */
			}
		    } else if (can_occupy(other, unit)) {
			if (u_acp(other->type) > 0) {
			    /* Have other unit do an enter action,
                               then move. */
			    /* (not quite right, move should happen
			       after other unit is actually inside, in
			       case it fills dest) */
			    prep_enter_action(other, other, unit);
			    order_moveto(unit, x, y);
			} else {
			    prep_enter_action(unit, other, unit);
			    order_moveto(unit, x, y);
			}
		    } else if (other->transport != NULL
			       && can_occupy(unit, other->transport)) {
			if (valid(check_enter_action(unit, unit, other->transport))) {
			    prep_enter_action(unit, unit, other->transport);
			} else {
			    /* (should schedule for next turn?) */
			}
		    } else if (other->transport != NULL
			       && other->transport->transport != NULL
			       && can_occupy(unit, other->transport->transport)) {
			/* two levels up should be sufficient */
			if (valid(check_enter_action(unit, unit, other->transport->transport))) {
			    prep_enter_action(unit, unit, other->transport->transport);
			} else {
			    /* (should schedule for next turn?) */
			}
		    } else if (valid(check_transfer_part_action(unit, unit, unit->hp, other))) {
			prep_transfer_part_action(unit, unit, unit->hp, other);
		    } else if (can_occupy_cell(unit, x, y)
			       && valid(check_move_action(unit, unit, x, y, unit->z))) {
			prep_move_action(unit, unit, x, y, unit->z);
		    } else {
			return FALSE;
		    }
		} else {
		    /* Somebody else's unit, try to victimize it in various ways,
		       trying coexistence only as a last resort. */
		    if (valid(check_capture_action(unit, unit, other))) {
			prep_capture_action(unit, unit, other);
		    } else if (valid(check_overrun_action(unit, unit, x, y, unit->z, 100))) {
			prep_overrun_action(unit, unit, x, y, unit->z, 100);
		    } else if (valid(check_attack_action(unit, unit, other, 100))) {
			prep_attack_action(unit, unit, other, 100);
		    } else if (valid(check_fire_at_action(unit, unit, other, -1))) {
			prep_fire_at_action(unit, unit, other, -1);
		    } else if (valid(check_detonate_action(unit, unit, x, y, unit->z))) {
			prep_detonate_action(unit, unit, x, y, unit->z);
		    } else if (valid(check_move_action(unit, unit, x, y, unit->z))) {
			prep_move_action(unit, unit, x, y, unit->z);
		    } else {
			return FALSE;
		    }
		}
	    } else {
		/* We're not adjacent to the destination, set up a move task. */
		order_moveto(unit, x, y);
	    }
	} else {
	    /* ??? can't act ??? */
	}
    } else {
	/* Destination is in the unit's own cell. */
	if (other != NULL) {
	    if (unit_trusts_unit(unit, other)) {
		    if (valid(check_transfer_part_action(unit, unit, unit->hp, other))) {
			prep_transfer_part_action(unit, unit, unit->hp, other);
		    } else if (valid(check_enter_action(unit, unit, other))) {
			prep_enter_action(unit, unit, other);
		    } else {
			return FALSE;
		    }
	    } else {
		    /* Somebody else's unit, try to victimize it in various ways,
		       trying coexistence only as a last resort. */
		    if (valid(check_capture_action(unit, unit, other))) {
			prep_capture_action(unit, unit, other);
		    } else if (valid(check_attack_action(unit, unit, other, 100))) {
			prep_attack_action(unit, unit, other, 100);
		    } else if (valid(check_fire_at_action(unit, unit, other, -1))) {
			prep_fire_at_action(unit, unit, other, -1);
		    } else if (valid(check_detonate_action(unit, unit, x, y, unit->z))) {
			prep_detonate_action(unit, unit, x, y, unit->z);
		    } else {
			return FALSE;
		    }
	    }
	} else if (unit->transport != NULL) {
	    /* Unit is an occupant wanting to leave, but yet remain in
	       the same cell as the transport. */
	    if (valid(check_move_action(unit, unit, x, y, unit->z))) {
		prep_move_action(unit, unit, x, y, unit->z);
	    } else {
		return FALSE;
	    }
	} else {
	    /* This is a no-op, don't do anything. */
	}
    }
    return TRUE;
}

int
give_supplies(unit, amts, rslts)
Unit *unit;
short *amts, *rslts;
{
    int m, gift, maxgift, actual, didsome;
    Unit *unit2;

    didsome = FALSE;
    unit2 = unit->transport;
    if (!(in_play(unit2) && completed(unit2)))
      return didsome;
    for_all_material_types(m) {
	if (rslts) rslts[m] = 0;
	maxgift = min(unit->supply[m], um_storage_x(unit2->type, m) - unit2->supply[m]);
	gift = ((amts == NULL || amts[m] == -1) ? (maxgift / 2) : amts[m]);
	if (gift > 0) {
	    if (1 /* can do immed transfer */) {
		/* Be stingy if giver is low */
		if (2 * unit->supply[m] < um_storage_x(unit->type, m))
		  gift = max(1, gift/2);
		actual = transfer_supply(unit, unit2, m, gift);
		if (rslts)
		  rslts[m] = actual;
		if (actual > 0)
		  didsome = TRUE;
	    }
	}
    }
    return didsome;
}

int
take_supplies(unit, amts, rslts)
Unit *unit;
short *amts, *rslts;
{
    int m, want, actual, didsome;
    Unit *unit2;

    didsome = FALSE;
    for_all_material_types(m) {
	if (rslts) rslts[m] = 0;
	want = ((amts == NULL || amts[m] == -1)
	        ? (um_storage_x(unit->type, m) - unit->supply[m])
	        : amts[m]);
	if (want > 0) {
	    unit2 = unit->transport;
	    if (in_play(unit2) && completed(unit2)) {
		/* Be stingy if transport is low */
		if (2 * unit2->supply[m] < um_storage_x(unit2->type, m))
		  want = max(1, want/2);
		actual = transfer_supply(unit2, unit, m, want);
		if (rslts)
		  rslts[m] = actual;
		if (actual > 0)
		  didsome = TRUE;
	    }
	}
    }
    return didsome;
}

/* Generic image setup. */

static ImageFamily *
get_generic_images(side, name, interp_hook, load_hook)
Side *side;
char *name;
void (*interp_hook) PROTO ((ImageFamily *imf));
void (*load_hook) PROTO ((ImageFamily *imf));
{
    FILE *fp;
    int cloned = FALSE;
    ImageFamily *imf;

    imf = get_imf(name);
    if (imf == NULL)
      return NULL;
    if (imf->numsizes > 0)
      if (interp_hook != NULL) {
	  imf = clone_imf(imf);
	  cloned = TRUE;
	  (*interp_hook)(imf);
      }
    if (load_hook != NULL) {
	if (!cloned) {
	    imf = clone_imf(imf);
	    cloned = TRUE;
	}
	(*load_hook)(imf);
    }
    if (imf->numsizes == 0) {
	/* Collect the names/locations of all image families. */
	fp = open_library_file("imf.dir");
	if (fp != NULL) {
	    load_image_families(fp, FALSE, NULL);
	    fclose(fp);
	} else {
	    init_warning("Cannot open \"%s\", will not use it", "imf.dir");
	}
	/* Get a (possibly empty) family. */
	imf = get_imf(name);
	if (imf == NULL)
	  return NULL;
	if (imf->location != NULL) {
	    /* Load data filling in the family. */
	    make_pathname(xconqlib, imf->location->name, "", spbuf);
	    if (load_imf_file(spbuf, NULL)) {
	    } else if (load_imf_file(imf->location->name, NULL)) {
	    } else {
		/* complain here, or not? */
	    }
	    if (interp_hook != NULL) {
		imf = clone_imf(imf);
		cloned = TRUE;
		(*interp_hook)(imf);
	    }
	}
    }
    return imf;
}

ImageFamily *
get_unit_type_images(side, u, interp_hook, load_hook, default_hook)
Side *side;
int u;
void (*interp_hook) PROTO ((ImageFamily *imf));
void (*load_hook) PROTO ((ImageFamily *imf));
void (*default_hook) PROTO ((ImageFamily *imf, int n, char *name));
{
    char *name;
    ImageFamily *imf;

    if (!empty_string(u_image_name(u)))
      name = u_image_name(u);
    else
      name = u_internal_name(u);
    imf = get_generic_images(side, name, interp_hook, load_hook);
    if (imf != NULL && imf->numsizes == 0 && default_hook != NULL) {
	imf->ersatz = TRUE;
	(*default_hook)(imf, u, u_type_name(u));
    }
    return imf;
}

ImageFamily *
get_terrain_type_images(side, t, interp_hook, load_hook, default_hook)
Side *side;
int t;
void (*interp_hook) PROTO ((ImageFamily *imf));
void (*load_hook) PROTO ((ImageFamily *imf));
void (*default_hook) PROTO ((ImageFamily *imf, int n, char *name));
{
    char *name;
    ImageFamily *imf;

    if (!empty_string(t_image_name(t)))
      name = t_image_name(t);
    else
      name = t_type_name(t);
    imf = get_generic_images(side, name, interp_hook, load_hook);
    if (imf != NULL && imf->numsizes == 0 && default_hook != NULL) {
	imf->ersatz = TRUE;
	(*default_hook)(imf, t, t_type_name(t));
    }
    return imf;
}

ImageFamily *
get_unseen_images(side, interp_hook, load_hook, default_hook)
Side *side;
void (*interp_hook) PROTO ((ImageFamily *imf));
void (*load_hook) PROTO ((ImageFamily *imf));
void (*default_hook) PROTO ((ImageFamily *imf, char *name));
{
    if (!empty_string(g_unseen_image_name())) {
	unseen_image = get_generic_images(side, g_unseen_image_name(), interp_hook, load_hook); 
	if (unseen_image != NULL && unseen_image->numsizes == 0) {
	    /* Appears to have failed - clear the unseen image then. */
	    unseen_image = NULL;
	    /* Note that we shouldn't try to free the imf, because it may be use
	       elsewhere. */
	}
    }
    return unseen_image;
}

ImageFamily *
get_emblem_images(side, side2, interp_hook, load_hook, default_hook)
Side *side, *side2;
void (*interp_hook) PROTO ((ImageFamily *imf));
void (*load_hook) PROTO ((ImageFamily *imf));
void (*default_hook) PROTO ((ImageFamily *imf, int n, char *name));
{
    char *name, tmpbuf[BUFSIZE];
    int s2 = side_number(side2);
    ImageFamily *imf;

    if (side2 != NULL && !empty_string(side2->emblemname))
      name = side2->emblemname;
    else {
	sprintf(tmpbuf, "s%d", s2);
	name = copy_string(tmpbuf);
    }
    imf = get_generic_images(side, name, interp_hook, load_hook);
    if (imf != NULL && imf->numsizes == 0 && default_hook != NULL && strcmp(name, "none") != 0) {
	(*default_hook)(imf, s2, name);
    }
    return imf;
}
