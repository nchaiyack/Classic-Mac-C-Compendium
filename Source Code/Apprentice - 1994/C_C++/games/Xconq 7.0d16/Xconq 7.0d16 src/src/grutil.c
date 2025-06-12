/* Copyright (c) 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Graphics utilities library. */

#include "conq.h"

#define NUMPOWERS 8

/* The following magical arrays set all the sizes at each mag. */

/* This is the basic cell size. */

int mags[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

/* These give the total dimensions of each hex cell, plus the vertical
   distance center-to-center.  This is all carefully calculated to make
   the cells overlap perfectly at each different magnification, assuming
   that the icons have the right shape and size. */

int hws[] = { 1, 2, 4, 8, 24, 44, 88, 174 };
int hhs[] = { 1, 2, 4, 8, 26, 48, 96, 192 };
int hcs[] = { 1, 2, 4, 8, 20, 37, 75, 148 };

/* The sizes of the unit subcells.  This is available drawing area, exact
   unit icon sizes depends on what's available. */

int uhs[] = { 1, 2, 3, 8, 16, 32, 64, 128 };
int uws[] = { 1, 2, 3, 8, 16, 32, 64, 128 };

/* Widths of borders and connections (0 implies don't draw at all). */

int bwid[NUMPOWERS]  = { 0, 0, 1, 1, 3, 5, 7, 9 };

int bwid2[NUMPOWERS] = { 0, 0, 1, 1, 2, 3, 4, 5 };

int cwid[NUMPOWERS] = { 0, 0, 1, 1, 3, 5, 7, 9 };

/* Coordinates of the hex borders. */
/* Note that array has extra column so don't need to wrap index. */

int bsx[NUMPOWERS][7] = {
	{ 0 },
	{ 0 },
	{  2,   4,   4,  2,  0,  0,  2 },
	{  4,   8,   8,  4,  0,  0,  4 },
	{ 12,  24,  24, 12,  0,  0, 12 },
	{ 22,  44,  44, 22,  0,  0, 22 },
	{ 44,  88,  88, 44,  0,  0, 44 },
	{ 87, 174, 174, 87,  0,  0, 87 }
};
int bsy[NUMPOWERS][7] = {
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
 
int lsx[NUMPOWERS][6] = {
	{ 0 },
	{ 0 },
	{  1,  2,   1,  -1,  -2,  -1 },
	{  2,  4,   2,  -2,  -4,  -2 },
	{  6, 12,   6,  -6, -12,  -6 },
	{ 11, 22,  11, -11, -22, -11 },
	{ 22, 44,  22, -22, -44, -22 },
	{ 44, 87,  44, -44, -87, -44 }
};

int lsy[NUMPOWERS][6] = {
	{ 0 },
	{ 0 },
	{  -2,  0,   2,   2,   0,  -2 },
	{  -4,  0,   4,   4,   0,  -4 },
	{  -9,  0,   9,   9,   0,  -9 },
	{ -18,  0,  18,  18,   0, -18 },
	{ -36,  0,  36,  36,   0, -36 },
	{ -74,  0,  74,  74,   0, -74 }
};

/* (should add generic layout and xform algorithms here) */

/* Choose and return a reasonable locations for map displays to start out
   centered on.  Usage is up to interfaces. */

pick_a_focus(side, xp, yp)
Side *side;
int *xp, *yp;
{
    int num = 0, sumx = 0, sumy = 0, tmpx, tmpy, dist, closest = area.maxdim;
    Unit *unit, *closestunit = NULL;

    if (side->startx > 0 && side->starty > 0) {
	*xp = side->startx;  *yp = side->starty;
    } else {
	/* Compute the average of all unit positions. */
    	for_all_side_units(side, unit) {
    	    if (in_play(unit)) {
    	    	++num;
    	    	sumx += unit->x;  sumy += unit->y;
    	    }
    	}
    	if (num > 0) {
	    tmpx = sumx / num;  tmpy = sumy / num;
	    /* Rescan the units to find a closest one. */
	    for_all_side_units(side, unit) {
		if (in_play(unit)) {
		    /* If already got one right there, just return. */
		    if (unit->x == tmpx && unit->y == tmpy) {
			*xp = tmpx;  *yp = tmpy;
			return;
		    } else {
		    	if ((dist = distance(unit->x, unit->y, tmpx, tmpy)) < closest) {
			    closest = dist;
			    closestunit = unit;
		    	}
		    }
		}
	    }
	    if (closestunit != NULL) {
		/* Return the position of the unit closest to the average position. */
		*xp = closestunit->x;  *yp = closestunit->y;
	    } else {
		*xp = tmpx;  *yp = tmpy;
	    }
    	} else {
	    *xp = area.width / 2 - area.height / 4;  *yp = area.height / 2;
	}
    }
}
