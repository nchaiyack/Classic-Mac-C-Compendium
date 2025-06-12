/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Management of world data.  Most of the action is within a single area
   of the world (at least for now). */

#include "conq.h"

/* The main world structure. */

World world;

/* The current area of the world. */

Area area;

/* This is the number of terrain types that can fill a cell. */

int numcelltypes = 0;

/* This is the number of terrain types that can be border terrain. */

int numbordtypes = 0;

/* This is the number of types that can be connections. */

int numconntypes = 0;

/* This is the number of types that can be coatings. */

int numcoattypes = 0;

/* (Features could also be used to implement the region 
   labeling scheme now used only in the machine player) */

Feature *featurelist = NULL;

Feature *lastfeature = NULL;

/* Feature id 0 means no geographical feature defined. */

int nextfid = 1;

int minelev;
int maxelev;

int mintemp;
int maxtemp;
int tempscanvary = FALSE;
int tempscanvaryinlayer = FALSE;

int minwindforce;
int maxwindforce;

int windscanvary = TRUE;

int windscanvaryinlayer = TRUE;

int minclouds;
int maxclouds;

int anyclouds = FALSE;

/* This is true if it is ever possible to have quantities of material
   in cells. */

int anymaterialsinterrain = FALSE;

/* Clean out all the world and area data. */

init_world()
{
    bzero(&world, sizeof(World));
    /* These default values effectively disable day and year effects. */
    world.daylength = 1;
    world.yearlength = 1;
    /* Init the current (default) area. */
    bzero(&area, sizeof(Area));
    /* Note especially that area width and height dimensions are now zero. */
}

set_world_circumference(circum, warn)
int circum, warn;
{
    /* All world circumferences are valid, no checks necessary. */
    world.circumference = circum;
    area.xwrap = (world.circumference == area.width);
    return TRUE;
}

set_area_shape(width, height, warn)
int width, height, warn;
{
    if (!valid_area_shape(width, height, warn)) return FALSE;
    area.width = width;  area.height = height;
    area.maxdim = max(area.width, area.height);
    area.xwrap = (world.circumference == area.width);
    return TRUE;
}

valid_area_shape(width, height, warn)	
int width, height, warn;
{
    if (width < MINWIDTH || height < MINHEIGHT) {
	if (warn) init_warning("area is too small");
    	return FALSE;
    }
    if (width != world.circumference && width * 2 < height) {
	if (warn) init_warning("hexagon area is %dx%d, impossible dimensions",
			       width, height);
    	return FALSE;
    }
    return TRUE;
}

check_area_shape()
{
    /* should use the above routine... */
    if (!area.width || !area.height) run_error("0x0 area!");
}

/* Calculate globals that we can use later to decide if particular
   classes of calculations need to be done, such as weather changes. */

calculate_world_globals()
{
    int t, m;

    /* This is a last chance to set a default world size.
       Will usually be set already, by players or by module. */
    if (area.width <= 0 && area.height <= 0) {
	area.width = DEFAULTWIDTH;  area.height = DEFAULTHEIGHT;
    }
    minelev = t_elev_min(0);
    maxelev = t_elev_max(0);
    mintemp = t_temp_min(0);
    maxtemp = t_temp_max(0);
    minwindforce = t_wind_force_min(0);
    maxwindforce = t_wind_force_max(0);
    minclouds = t_clouds_min(0);
    maxclouds = t_clouds_max(0);
    for_all_terrain_types(t) {
	if (t_elev_min(t) < minelev) minelev = t_elev_min(t);
	if (t_elev_max(t) > maxelev) maxelev = t_elev_max(t);
	if (t_temp_min(t) < mintemp) mintemp = t_temp_min(t);
	if (t_temp_max(t) > maxtemp) maxtemp = t_temp_max(t);
	if (t_wind_force_min(t) < maxwindforce)
	  minwindforce = t_wind_force_min(t);
	if (t_wind_force_max(t) > maxwindforce)
	  maxwindforce = t_wind_force_max(t);
	if (t_clouds_min(t) < minclouds)
	  minclouds = t_clouds_min(t);
	if (t_clouds_max(t) > maxclouds)
	  maxclouds = t_clouds_max(t);	
	/* Decide if materials can ever be accumulated in cells. */
	if (t_storage(t) > 0) {
	    anymaterialsinterrain = TRUE;
	} else {
	    for_all_material_types(m) {
		if (tm_storage_x(t, m) > 0) anymaterialsinterrain = TRUE;
	    }
	}
    }
    if (mintemp != maxtemp) tempscanvary = TRUE;
    if (minwindforce != maxwindforce) windscanvary = TRUE;
    if (minclouds != maxclouds) anyclouds = TRUE;
}

count_terrain_subtypes()
{
    int t;

    numcelltypes = numbordtypes = numconntypes = numcoattypes = 0;
    for_all_terrain_types(t) {
	switch (t_subtype(t)) {
	  case cellsubtype:
	    ++numcelltypes;
	    break;
	  case bordersubtype:
	    ++numbordtypes;
	    break;
	  case connectionsubtype:
	    ++numconntypes;
	    break;
	  case coatingsubtype:
	    ++numcoattypes;
	    break;
	}
    }
}

/* Make space for an area's terrain, and for the unit cache. */

/* This can be postponed until it is actually needed. */

allocate_area_terrain()
{
    check_area_shape();
    /* Get rid of old stuff maybe. (is this desirable?) */
    if (area.terrain != NULL) {
	free(area.terrain);
	free(area.units);
    }
    /* Allocate the basic terrain layer. */
    /* It doesn't matter what ttype 0 is, we're guaranteed that it
       will be defined eventually. */
    area.terrain = malloc_area_layer(char);
    /* Allocate and null out the unit cache. */
    area.units = malloc_area_layer(Unit *);
}

/* Set up the auxiliary terrain layer of the area. */

allocate_area_aux_terrain(t)
int t;
{
    if (!any_aux_terrain_defined()) {
	area.auxterrain = (char **) xmalloc(numttypes * sizeof(char *));
    }
    if (!aux_terrain_defined(t)) {
	area.auxterrain[t] = malloc_area_layer(char);
    }
}

/* Allocate some number of scratch layers.  These are used as temporaries
   in calculations, etc. */

allocate_area_scratch(n)
int n;
{
    check_area_shape();
    if (n >= 1 && !area.tmp1) {
	area.tmp1 = malloc_area_layer(short);
    }
    if (n >= 2 && !area.tmp2) {
	area.tmp2 = malloc_area_layer(short);
    }
    if (n >= 3 && !area.tmp3) {
	area.tmp3 = malloc_area_layer(short);
    }
}

/* Should have something to free scratch layers up also maybe. */

/* Allocate and init the elevation layer. */

allocate_area_elevations()
{
    if (!elevations_defined()) {
	check_area_shape();
	area.elevations = malloc_area_layer(short);
    }
}

/* Allocate and init the temperature layer. */

allocate_area_temperatures()
{
    if (!temperatures_defined()) {
	check_area_shape();
	area.temperature = malloc_area_layer(short);
    }
    /* We'll need one scratch layer too. */
    allocate_area_scratch(1);
}

/* Allocate a layer indicating the side of the people living in each cell. */

allocate_area_people_sides()
{
    int x, y;

    if (!people_sides_defined()) {
	check_area_shape();
	area.peopleside = malloc_area_layer(char);
	for_all_hexes(x, y) {
	    set_people_side_at(x, y, NOBODY);
	}
    }
}

/* Set up a cell material layer of the area. */

allocate_area_material(m)
int m;
{
    check_area_shape();
    if (!any_cell_materials_defined()) {
	area.materials = (short **) xmalloc(nummtypes * sizeof(short *));
    }
    if (!cell_material_defined(m)) {
	area.materials[m] = malloc_area_layer(short);
    }
}

allocate_area_clouds()
{
    if (!clouds_defined()) {
	check_area_shape();
	area.clouds = malloc_area_layer(short);
    }
}

allocate_area_cloud_altitudes()
{
	allocate_area_cloud_bottoms();
	allocate_area_cloud_heights();
}

allocate_area_cloud_bottoms()
{
    if (!cloud_bottoms_defined()) {
	check_area_shape();
	area.cloudbottoms = malloc_area_layer(short);
    }
}

allocate_area_cloud_heights()
{
    if (!cloud_heights_defined()) {
	check_area_shape();
	area.cloudheights = malloc_area_layer(short);
    }
}

allocate_area_winds()
{
    if (!winds_defined()) {
	check_area_shape();
	area.winds = malloc_area_layer(short);
    }
}

/* Generalized area search routine.  It starts in the immediately adjacent
   hexes and expands outwards.  The basic structure is to examine successive
   "rings" out to the max distance; within each ring, we must scan each of
   six faces (picking a random one to start with) by iterating along that
   face, in a direction 120 degrees from the direction out to one corner of
   the face.  Draw a picture if you want to understand it... */

/* Incr is normally one.  It is set to area_size to search on areas
   instead of hexes. */

/* Note that points far outside the map may be generated, but the predicate
   will not be called on them.  It may be applied to the same point several
   times, however, if the distance is enough to wrap around the area. */

/* This needs to be changed to understand different world shapes. */

search_around(x0, y0, maxdist, pred, rxp, ryp, incr)
int x0, y0, maxdist, (*pred)(), *rxp, *ryp, incr;
{
    int clockwise, dist, dd, d, dir, x1, y1, i, dir2, x, y;

    maxdist = max(min(maxdist, area.width), min(maxdist, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    for (dist = 1; dist <= maxdist; dist += incr) {
	dd = random_dir();
	for_all_directions(d) {
	    dir = (d + dd) % NUMDIRS;
	    x1 = x0 + dist * dirx[dir];
	    y1 = y0 + dist * diry[dir];
	    for (i = 0; i < dist; ++i) {
		dir2 = opposite_dir(dir + clockwise);
		x = x1 + i * dirx[dir2] * incr;
		y = y1 + i * diry[dir2] * incr;
		if (inside_area(x, y) && (*pred)(wrap(x), y)) {
		    *rxp = wrap(x);  *ryp = y;
		    return TRUE;
		}
	    }
	}
    }
    return FALSE;
}

search_and_apply(x0, y0, maxdist, pred, rxp, ryp, incr, fn, num)
int x0, y0, maxdist, (*pred)(), *rxp, *ryp, incr, (*fn)(), num;
{
    int clockwise, dist, dd, d, dir, x1, y1, i, dir2, x, y;

    maxdist = max(min(maxdist, area.width), min(maxdist, area.height));
    clockwise = (flip_coin() ? 1 : -1);
    if (maxdist >= 0) {
	    if (inside_area(x0, y0) && (*pred)(wrap(x0), y0)) {
		*rxp = wrap(x0);  *ryp = y0;
		(*fn)(wrap(x0), y0);
		if (--num <= 0) return TRUE;
	    }
    }
    for (dist = 1; dist <= maxdist; dist += incr) {
	dd = random_dir();
	for_all_directions(d) {
	    dir = (d + dd) % NUMDIRS;
	    x1 = x0 + dist * dirx[dir];
	    y1 = y0 + dist * diry[dir];
	    for (i = 0; i < dist; ++i) {
		dir2 = opposite_dir(dir + clockwise);
		x = x1 + i * dirx[dir2] * incr;
		y = y1 + i * diry[dir2] * incr;
		if (between(0, y, area.height-1)) {
		    if (inside_area(x, y) && (*pred)(wrap(x), y)) {
			*rxp = wrap(x);  *ryp = y;
			(*fn)(wrap(x), y);
			if (--num <= 0) return TRUE;
		    }
		}
	    }
	}
    }
    return FALSE;
}

/* Apply a function to every hex within the given radius, being careful (for
   both safety and efficiency reasons) not to go past edges.  Note that the
   distance is inclusive, and that distance of 0 means x0,y0 only.  Also,
   if the distance is greater than either map dimension, this routine still
   operates on a properly hexagonal area.  */

/* This routine should be avoided in time-critical code. */

apply_to_area(x0, y0, dist, fn)
int x0, y0, dist, (*fn)();
{
    int x, y, x1, y1, x2, y2;

    dist = min(dist, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    for (y = y1; y <= y2; ++y) {
	if (between(1, y, area.height-2)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(wrap(x), y)) {
		    ((*fn)(wrap(x), y));
		}
	    }
	}
    }
}

apply_to_area_plus_edge(x0, y0, dist, fn)
int x0, y0, dist, (*fn)();
{
    int x, y, x1, y1, x2, y2;

    dist = min(dist, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, area.height-1)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(wrap(x), y)) {
		    ((*fn)(wrap(x), y));
		}
	    }
	}
    }
}

apply_to_ring(x0, y0, distmin, distmax, fn)
int x0, y0, distmin, distmax, (*fn)();
{
    int dist, x, y, x1, y1, x2, y2;

    dist = min(distmax, area.maxdim);
    y1 = y0 - dist;
    y2 = y0 + dist;
    for (y = y1; y <= y2; ++y) {
	if (between(1, y, area.height-2)) {
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - (y < y0 ? (y - y1) : dist);
	    x2 = x0 + (y > y0 ? (y2 - y) : dist);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(wrap(x), y) && distance(x, y, x0, y0) >= distmin) {
		    ((*fn)(wrap(x), y));
		}
	    }
	}
    }
}

/* Apply the function to the hexagon bounded by w,h. */

apply_to_hexagon(x0, y0, w2, h2, fn)
int x0, y0, w2, h2, (*fn)();
{
    int x, y, x1, y1, x2, y2;

    y1 = limit(y0 - h2);
    y2 = limit(y0 + h2);
    for (y = y1; y <= y2; ++y) {
	if (between(0, y, area.height-1)) {  /* always true? */
	    /* Compute endpoints of row, but don't wrap or loop will confuse */
	    x1 = x0 - w2 + (y < y0 ? (y0 - y) : 0);
	    x2 = x0 + w2 - (y > y0 ? (y - y0) : 0);
	    for (x = x1; x <= x2; ++x) {
		/* not real efficient, sigh... */
		if (in_area(wrap(x), y)) {
		    ((*fn)(wrap(x), y));
		}
	    }
	}
    }
}

/* Apply a function all along a path. */

apply_to_path(fx, fy, tx, ty, dirfn, fn, shortest)
int fx, fy, tx, ty, (*dirfn)(), (*fn)(), shortest;
{
    int i = 500, j, x = fx, y = fy;
    int dx, dxa, dy, d[NUMDIRS], axis, hextant, tmp, sig;
    int numchoices;

    while (!(x == tx && y == ty) && i-- > 0 /* safety */) {
	dx = tx - x;  dy = ty - y;
	/* If in a wrapping world, choose the shortest of directions. */
	if (area.xwrap) {
	    dxa = (tx + area.width) - fx;
	    if (abs(dx) > abs(dxa)) dx = dxa;
	    dxa = (tx - area.width) - fx;
	    if (abs(dx) > abs(dxa)) dx = dxa;
	}
	/* Figure out the axis or hextant of this delta. */
	axis = hextant = -1;
	/* Decode the delta values. */
	if (dx == 0) {
	    axis = (dy > 0 ? NORTHEAST : SOUTHWEST);
	} else if (dy == 0) {
	    axis = (dx > 0 ? EAST : WEST);
	} else if (dx == (0 - dy)) {
	    axis = (dy > 0 ? NORTHWEST : SOUTHEAST);
	} else if (dx > 0) {
	    hextant = (dy > 0 ? EAST :
		       (abs(dx) > abs(dy) ? SOUTHEAST : SOUTHWEST));
	} else {
	    hextant = (dy < 0 ? WEST :
		       (abs(dx) > abs(dy) ? NORTHWEST : NORTHEAST));
	}
	/* On an axis, there's no choice. */
	if (axis >= 0) {
	    d[0] = d[1] = axis;
	    numchoices = (shortest ? 1 : 3);
	}
	/* Two choices in the middle of a hextant. */
	if (hextant >= 0) {
	    d[0] = left_dir(hextant);
	    d[1] = hextant;
	    numchoices = (shortest ? 2 : 4);
	}
	/* If we don't have to pick a shortest path, we have two more
	   directions to try. */
	if (!shortest) {
	    d[2] = left_dir(d[0]);
	    d[3] = right_dir(d[1]);
	}
	if (dirfn != NULL) {
		if ((numchoices = (*dirfn)(x, y, d, numchoices)) <= 0) return;
	} else {
	    /* Be a little random in our choice of directions to try first. */
	    if (flip_coin()) {
		tmp = d[0];  d[0] = d[1];  d[1] = tmp;
		tmp = d[2];  d[2] = d[3];  d[3] = tmp;
	    }
	}
	/* Try each of the directions. */
	if (!inside_area(x, y)) return;
	for (j = 0; j < numchoices; ++j) {
	    sig = (*fn)(x, y, d[j], j, numchoices);
	    if (sig > 0) {
		/* It's cool - go with this dir. */
		/* Jump along to the new spot on the path. */
		x += dirx[d[j]];  y += diry[d[j]];
		/* Out of the loop. */
		break;
	    } else if (sig < 0) {
		return;
	    } else {
		/* Try another. */
	    }
	}
    }
}

/* Find a path between the two given points. */

/* The chooser function gets passed an small array for directions;
   it is expected to fill it with directions sorted in order of
   preference, and to return the number of directions it found. */

/* (the chooser also needs to respect already-marked cells) */
/* (marking should account for all directions in?) */
/* (should return a "figure of merit" sometimes) */
/* (main prog should test vicinity of dest, might not be reachable
   anyway, but maybe should have "reach within n hexes") */

find_path(fx, fy, tx, ty, chooser, maxwps, waypoints, numwpsp)
int fx, fy, tx, ty, (*chooser)(), maxwps, *numwpsp;
Waypoint *waypoints;
{
    int ndirs, trythese[NUMDIRS], i;
    int x1, y1, x2, y2;

    if (fx == fy && tx == ty) {
	return TRUE;
    }
    ndirs = (*chooser)(x1, y1, x2, y2, trythese);
    if (ndirs == 0) {
	/* We're totally blocked. */
	return FALSE;
    } else {
	for (i = 0; i < ndirs; ++i) {
	    /* try this direction with find_path_aux */
	}
    }
    return FALSE;
}

/* Test whether x,y is a valid position anywhere in the current area. */

in_area(x, y)
int x, y;
{
    return (between(0, y, area.height-1) &&
	    (area.xwrap ? TRUE : (between(0, x, area.width-1) &&
				   between(area.height/2+0,
					   x+y,
					   area.width+area.height/2-1))));
}

/* This is true if the given x, y position is a valid position for units. */

/* Does x testing work right for even/odd heights? */

inside_area(x, y)
int x, y;
{
    return (between(1, y, area.height-2) &&
	    (area.xwrap ? TRUE : (between(1, x, area.width-2) &&
				   between(area.height/2+1,
					   x+y,
					   area.width+area.height/2-2))));
}

area_cells()
{
    int rslt = area.width * area.height;

    if (!area.xwrap) rslt = (rslt * 3) / 4;
    return rslt;
}
	    
/* Find the type of the border on the given side of the given hex. */

border_at(x, y, dir, t)
int x, y, dir, t;
{
    int bord;

    if (!inside_area(x, y)
	|| !t_is_border(t)
	|| !aux_terrain_defined(t))
      return FALSE;
    bord = aux_terrain_at(x, y, t);
    return (bord & (1 << dir));
}

/* For now, set a bit on both sides of a border. */

set_border_at(x, y, dir, t, onoff)
int x, y, dir, t, onoff;
{
    int ox = x + dirx[dir], oy = y + diry[dir], bord, obord;
    int odir = opposite_dir(dir);

    if (!inside_area(x, y) || !t_is_border(t)) return;
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    allocate_area_aux_terrain(t);
    bord = aux_terrain_at(x, y, t);
    bord = ((onoff << dir) | (bord & ~(1 << dir)));
    obord = aux_terrain_at(ox, oy, t);
    obord = ((onoff << odir) | (obord & ~(1 << odir)));
    set_aux_terrain_at(x, y, t, bord);
    set_aux_terrain_at(ox, oy, t, obord);
}

/* Find the type of the connection on the given side of the given hex. */

connection_at(x, y, dir, t)
int x, y, dir, t;
{
    int conn;

    if (!inside_area(x, y)
	|| !t_is_connection(t)
	|| !aux_terrain_defined(t))
      return FALSE;
    conn = aux_terrain_at(x, y, t);
    return (conn & (1 << dir));
}

/* For now, set a bit on both sides of a connection. */

set_connection_at(x, y, dir, t, onoff)
int x, y, dir, t, onoff;
{
    int ox = x + dirx[dir], oy = y + diry[dir], conn, oconn;
    int odir = opposite_dir(dir);

    if (!inside_area(x, y) || !t_is_connection(t)) return;
    allocate_area_aux_terrain(t);
    onoff = (onoff ? 1 : 0);  /* make sure it's one bit */
    conn = aux_terrain_at(x, y, t);
    conn = ((onoff << dir) | (conn & ~(1 << dir)));
    oconn = aux_terrain_at(ox, oy, t);
    oconn = ((onoff << odir) | (oconn & ~(1 << odir)));
    set_aux_terrain_at(x, y, t, conn);
    set_aux_terrain_at(ox, oy, t, oconn);
}

/* If there might be any inconsistencies in borders or connections,
   this fixes them.  Basically this just detects if a bit is set on
   either side, and sets the bits on both sides if so. */

patch_linear_terrain(t)
int t;
{
    int x, y, dir;
	
    if (t_is_border(t)) {
	for_all_hexes(x, y) {
	    /* This test is a hack to save some time.  If a cell has no border
	       flags in any direction, then either it has no borders or else it
	       will be fixed up later on, when an adjacent cell is patched. */
	    if (aux_terrain_at(x, y, t) != 0) {
		for_all_directions(dir) {
		    if (border_at(x, y, dir, t))
		      set_border_at(x, y, dir, t, TRUE);
		}
	    }
	}
    } else if (t_is_connection(t)) {
	for_all_hexes(x, y) {
	    if (aux_terrain_at(x, y, t) != 0) {
		for_all_directions(dir) {
		    if (connection_at(x, y, dir, t))
		      set_connection_at(x, y, dir, t, TRUE);
		}
	    }
	}
    }
}

/* Make space to record named features. */

init_features()
{
    int x, y;

    featurelist = lastfeature = NULL;
    area.features = malloc_area_layer(short);
    for_all_hexes(x, y) set_raw_feature_at(x, y, 0);
}

Feature *
create_feature(typename, name)
char *typename, *name;
{
    Feature *newfeature = (Feature *) xmalloc(sizeof(Feature));

    newfeature->id = nextfid++;
    newfeature->typename = typename;
    newfeature->name = name;
    /* Add to the end of the feature list. */
    if (lastfeature != NULL) {
    	lastfeature->next = newfeature;
    } else {
    	featurelist = newfeature;
    }
    lastfeature = newfeature;
    return newfeature;
}

Feature *
find_feature(fid)
int fid;
{
    Feature *feature;

    for (feature = featurelist; feature != NULL; feature = feature->next) {
	if (feature->id == fid) return feature;
    }
    return NULL;
}

Feature *
feature_at(x, y)
int x, y;
{
    int fid;

    if (!features_defined()) return NULL;

    fid = raw_feature_at(x, y);
    if (fid == 0) {
	return NULL;
    } else {
	return find_feature(fid);
    }
}

set_feature_type_name(feature, typename)
Feature *feature;
char *typename;
{
	if (feature == NULL) return;
	feature->typename = copy_string(typename);
	/* (should ping all displays) */
}

set_feature_name(feature, name)
Feature *feature;
char *name;
{
	if (feature == NULL) return;
	feature->name = copy_string(name);
	/* (should ping all displays) */
}

renumber_features()
{
    int newfid = 1, maxoldfid = 0, x, y;
    short *newlabels;
    Feature *feature;

    if (!features_defined()) return;
    for (feature = featurelist; feature != NULL; feature = feature->next) {
    	maxoldfid = max(maxoldfid, feature->id);
    	feature->relabel = newfid++;
    }
    if (maxoldfid > 1000) return; /* don't risk it */
    newlabels = (short *) xmalloc((maxoldfid + 1) * sizeof(short));
    for (feature = featurelist; feature != NULL; feature = feature->next) {
    	newlabels[feature->id] = feature->relabel;
    }
    for_all_hexes(x, y) {
    	set_raw_feature_at(x, y, newlabels[raw_feature_at(x, y)]);
    }
    for (feature = featurelist; feature != NULL; feature = feature->next) {
    	feature->id = feature->relabel;
    }
}

#if 0  /* never used, but seems useful? */
/* Returns shortest distance around the area (can be either direction). */

cyldist(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    int dist = distance(x1, y1, x2, y2), dist2;

    if ((dist2 = distance(x1+area.width, y1, x2, y2)) < dist) {
	dist = dist2;
    } else if ((dist2 = distance(x1, y1, x2+area.width, y2)) < dist) {
	dist = dist2;
    }
    return dist;
}
#endif

/* Compute the coords of a point in the given direction. */

point_in_dir(x, y, dir, xp, yp)
int x, y, dir, *xp, *yp;
{
    *xp = wrapx(x + dirx[dir]);  *yp = y + diry[dir];
    return (in_area(*xp, *yp));
}

interior_point_in_dir(x, y, dir, xp, yp)
int x, y, dir, *xp, *yp;
{
    *xp = wrapx(x + dirx[dir]);  *yp = y + diry[dir];
    return (inside_area(*xp, *yp));
}

/* Return a random point guaranteed inside the area. */

random_point(xp, yp)
int *xp, *yp;
{
    int tries = 500;

    while (tries-- > 0) {
	*xp = xrandom(area.width);  *yp = xrandom(area.height - 2) + 1;
	if (inside_area(*xp, *yp)) return TRUE;
    }
    return FALSE;
}

/* Return a random point guaranteed to be within a given radius of
   a given point. */

random_point_near(cx, cy, radius, xp, yp)
int cx, cy, radius, *xp, *yp;
{
    int tries = 500;

    if (radius <= 0) return FALSE;
    while (tries-- > 0) {
	*xp = cx + xrandom(2 * radius + 1) - radius;
	*yp = cy + xrandom(2 * radius + 1) - radius;
	if (inside_area(*xp, *yp)
	    && distance(cx, cy, *xp, *yp) <= radius) return TRUE;
    }
    return FALSE;
}

/* Return a random point guaranteed to be within a given radius of
   a given point. */

random_point_in_area(cx, cy, rx, ry, xp, yp)
int cx, cy, rx, ry, *xp, *yp;
{
    int tries = 500;

    while (tries-- > 0) {
	*xp = cx + xrandom(2 * rx + 1) - rx;
	*yp = cy + xrandom(2 * ry + 1) - ry;
	if (inside_area(*xp, *yp)
	    && distance(cx, cy, *xp, *yp) <= max(rx - ry, ry - rx)) return TRUE;  /* (should fix test?) */
    }
    return FALSE;
}

/* Generic warning that a terrain subtype is incorrect. */

terrain_subtype_warning(context, t)
char *context;
int t;
{
    run_warning("In %s: Garbage t%d (%s) subtype %d",
		context, t, t_type_name(t), t_subtype(t));
}

approx_dir(dx, dy)
int dx, dy;
{
    if (area.xwrap) {
    	dx = (dx < 0 ? (dx < 0 - area.width / 2 ? area.width + dx : dx)
	  	: (dx > area.width / 2 ? dx - area.width : dx));
    }
    if (dx == 0) {
	if (dy == 0) return -1; /* should flag so can use special cursor */
	if (dy > 0) return NORTHEAST;
	return SOUTHWEST;
    } else if (dx > 0) {
    	/* Check for the axes first. */
	if (dy == 0) return EAST;
	if (dy == (-dx)) return SOUTHEAST;
	if (dy > dx) return NORTHEAST;
	if ((-dy) <= dx / 2) return EAST;
	if ((-dy) < dx * 2) return SOUTHEAST;
	return SOUTHWEST;
    } else {
    	/* Check for the axes first. */
	if (dy == 0) return WEST;
    	if (dy == (-dx)) return NORTHWEST;
	if (dy > (-dx) * 2) return NORTHEAST;
	if (dy >= (-dx) / 2) return NORTHWEST;
	if (dy > dx) return WEST;
	return SOUTHWEST;
    }
}

hextant(dx, dy)
int dx, dy;
{
    if (area.xwrap) {
    	dx = (dx < 0 ? (dx < 0 - area.width / 2 ? area.width + dx : dx)
	  	: (dx > area.width / 2 ? dx - area.width : dx));
	}
    if (dx < 0) {
	if (dy < 0) return SOUTHWEST;
	if (dy == 0) return WEST;
	return NORTHWEST;
    } else if (dx == 0) {
	if (dy > 0) return NORTHEAST;
	if (dy == 0) return 0; /* should flag so can use special cursor */
	return SOUTHWEST;
    } else {
	if (dy < 0) return SOUTHEAST;
	if (dy == 0) return EAST;
	return NORTHEAST;
    }
}

/* should put date-handling code here? */

#ifdef DESIGNERS

/* Cell painting. */

paint_cell_1(x, y)
int x, y;
{
    /* Only do anything if we're actually changing to a different type. */
    if (terrain_at(x, y) != tmpttype) {
	set_terrain_at(x, y, tmpttype);
	see_exact(tmpside, x, y);
    }
}

paint_cell(side, x, y, r, t)
Side *side;
int x, y, r, t;
{
    tmpside = side;
    tmpttype = t;
    apply_to_area_plus_edge(x, y, r, paint_cell_1);
}

paint_border(side, x, y, dir, t, mode)
Side *side;
int x, y, dir, t, mode;
{
    int oldbord;

    allocate_area_aux_terrain(t);
    oldbord = border_at(x, y, dir, t);
    set_border_at(x, y, dir, t, (mode < 0 ? !oldbord : mode));
    if (oldbord != border_at(x, y, dir, t)) {
	see_exact(side, x, y);
	see_exact(side, x+dirx[dir], y+diry[dir]);
    }
}

paint_connection(side, x, y, dir, t, mode)
Side *side;
int x, y, dir, t, mode;
{
    int oldconn;

    allocate_area_aux_terrain(t);
    oldconn = connection_at(x, y, dir, t);
    set_connection_at(x, y, dir, t, (mode < 0 ? !oldconn : mode));
    if (oldconn != connection_at(x, y, dir, t)) {
	see_exact(side, x, y);
	see_exact(side, x+dirx[dir], y+diry[dir]);
    }
}

/* Coating painting. */

int tmpint;

paint_coating_1(x, y)
int x, y;
{
    int olddepth = aux_terrain_at(x, y, tmpttype);

    if (olddepth != tmpint) {
	set_aux_terrain_at(x, y, tmpttype, tmpint);
	see_exact(tmpside, x, y);
    }
}

paint_coating(side, x, y, r, t, depth)
Side *side;
int x, y, r, t, depth;
{
    allocate_area_aux_terrain(t);
    tmpside = side;
    tmpttype = t;
    tmpint = depth;
    apply_to_area_plus_edge(x, y, r, paint_coating_1);
}

paint_people_1(x, y)
int x, y;
{
    int oldpeop = people_side_at(x, y);

    if (oldpeop != tmpint) {
	set_people_side_at(x, y, tmpint);
	see_exact(tmpside, x, y);
    }
}

paint_people(side, x, y, r, s)
Side *side;
int x, y, r, s;
{
    allocate_area_people_sides();
    tmpside = side;
    tmpint = s;
    apply_to_area(x, y, r, paint_people_1);
}

paint_feature_1(x, y)
int x, y;
{
    int oldfid = raw_feature_at(x, y);

    if (oldfid != tmpint) {
	set_raw_feature_at(x, y, tmpint);
	see_exact(tmpside, x, y);
    }
}

paint_feature(side, x, y, r, f)
Side *side;
int x, y, r, f;
{
    Feature *oldfeature, *newfeature;

    if (!features_defined()) init_features();
    oldfeature = feature_at(x, y);
    if ((newfeature = find_feature(f)) != NULL) {
	if (newfeature != oldfeature) {
	    if (oldfeature != NULL) --oldfeature->size;
	    ++newfeature->size;
	    tmpside = side;
	    tmpint = f;
	    apply_to_area(x, y, r, paint_feature_1);
	}
    }
}

paint_elev_1(x, y)
int x, y;
{
    int n, t = terrain_at(x, y), oldelev = elev_at(x, y);

    n = max(t_elev_min(t), min(tmpint, t_elev_max(t)));
    if (n != oldelev) {
	set_elev_at(x, y, n);
	see_exact(tmpside, x, y);
    }
}

paint_elevation(side, x, y, r, elev)
Side *side;
int x, y, r, elev;
{
    allocate_area_elevations();
    tmpside = side;
    tmpint = elev;
    apply_to_area_plus_edge(x, y, r, paint_elev_1);
}

paint_temp_1(x, y)
int x, y;
{
    int n, t = terrain_at(x, y), oldtemp = temperature_at(x, y);
    
    n = max(t_temp_min(t), min(tmpint, t_temp_max(t)));
    if (n != oldtemp) {
	set_temperature_at(x, y, n);
	see_exact(tmpside, x, y);
    }
}

paint_temperature(side, x, y, r, temp)
Side *side;
int x, y, r, temp;
{
    allocate_area_temperatures();
    tmpside = side;
    tmpint = temp;
    apply_to_area_plus_edge(x, y, r, paint_temp_1);
}

paint_material_1(x, y)
int x, y;
{
    int oldm = material_at(x, y, tmpmtype);

    if (oldm != tmpint) {
	set_material_at(x, y, tmpmtype, tmpint);
	see_exact(tmpside, x, y);
    }
}

paint_material(side, x, y, r, m, amt)
Side *side;
int x, y, r, m, amt;
{
    allocate_area_material(m);
    tmpside = side;
    tmpmtype = m;
    tmpint = amt;
    apply_to_area_plus_edge(x, y, r, paint_material_1);
}

/* Cloud painting is more complicated because up to three separate
   layers are involved. */

int tmpint2, tmpint3;

paint_clouds_1(x, y)
int x, y;
{
    int oldcl = raw_cloud_at(x, y);
    int oldbot = raw_cloud_bottom_at(x, y);
    int oldhgt = raw_cloud_height_at(x, y);
    int changed = FALSE;

    if (oldcl != tmpint) {
	set_raw_cloud_at(x, y, tmpint);
	changed = TRUE;
    }
    if (oldbot != tmpint2) {
	set_raw_cloud_bottom_at(x, y, tmpint2);
	changed = TRUE;
    }
    if (oldhgt != tmpint3) {
	set_raw_cloud_height_at(x, y, tmpint3);
	changed = TRUE;
    }
    if (changed) see_exact(tmpside, x, y);
}

paint_clouds(side, x, y, r, cloudtype, bot, hgt)
Side *side;
int x, y, r, cloudtype, bot, hgt;
{
    allocate_area_clouds();
    /* (should not always do altitudes) */
    allocate_area_cloud_altitudes();
    tmpside = side;
    tmpint = cloudtype;
    tmpint2 = bot;
    tmpint3 = hgt;
    apply_to_area_plus_edge(x, y, r, paint_clouds_1);
}

paint_winds_1(x, y)
int x, y;
{
    int oldw = raw_wind_at(x, y);

    if (oldw != tmpint) {
	set_raw_wind_at(x, y, tmpint);
	see_exact(tmpside, x, y);
    }
}

paint_winds(side, x, y, r, dir, force)
Side *side;
int x, y, r, dir, force;
{
    allocate_area_winds();
    tmpside = side;
    tmpint = force << 3 | dir;
    apply_to_area_plus_edge(x, y, r, paint_winds_1);
}

#endif /* DESIGNERS */

/* Computing distance in a hexagonal system is a little peculiar, since it's
   sometimes just delta x or y, and other times is the sum.  Basically there
   are six formulas to compute distance, depending on the direction between
   the two points. */

distance(x1, y1, x2, y2)
int x1, y1, x2, y2;
{
    int dx = x2 - x1, dy = y2 - y1;

    if (area.xwrap) {
    	/* Choose the shortest way around a cylinder. */
    	dx = (dx < 0 ? (dx < 0 - area.width / 2 ? area.width + dx : dx)
	        	 : (dx > area.width / 2 ? dx - area.width : dx));
	}
    if (dx >= 0) {
	if (dy >= 0) {
	    return (dx + dy);
	} else if ((0 - dy) <= dx) {
	    return dx;
	} else {
	    return (0 - dy);
	}
    } else {
	if (dy <= 0) {
	    return (0 - (dx + dy));
	} else if (dy <= (0 - dx)) {
	    return (0 - dx);
	} else {
	    return dy;
	}
    }
}

/* Convert any vector into a direction (not necessarily the closest one). */
/* Fail horribly on zero vectors. */

find_dir(dx, dy)
int dx, dy;
{
    if (area.xwrap) {
    	dx = (dx < 0 ? (dx < 0 - area.width / 2 ? area.width + dx : dx)
	  	: (dx > area.width / 2 ? dx - area.width : dx));
	}
    if (dx < 0) {
	if (dy < 0) return SOUTHWEST;
	if (dy == 0) return WEST;
	return NORTHWEST;
    } else if (dx == 0) {
	if (dy > 0) return NORTHEAST;
	if (dy == 0) abort();
	return SOUTHWEST;
    } else {
	if (dy < 0) return SOUTHEAST;
	if (dy == 0) return EAST;
	return NORTHEAST;
    }
}
