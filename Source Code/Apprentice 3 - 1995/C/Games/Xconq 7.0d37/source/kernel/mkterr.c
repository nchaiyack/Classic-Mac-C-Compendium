/* Terrain generation for Xconq.
   Copyright (C) 1986, 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is the collection of terrain generation methods. */

/* Fractal terrain generation. */

/* The process is actually done for elevation and water separately, then
   the terrain type is derived from looking at both together. */

#include "conq.h"

/* This bounds the range of high and low spots. */

#define MAXALT 4000

/* The following dynamically allocated arrays must be ints, since a area
   may have >32K cells. */

int *histo;             /* histogram array */
int *alts;              /* percentile for each elevation */
int *wets;              /* percentile for level of moisture */

/* Area scratch layers are used as: relief = tmp1, moisture = tmp2,
   aux = tmp3 */

int stepsize = 20;

int partdone;

/* This variable records the number of cells that didn't match any of the
   terrain type percentiles. */

static int numholes;

static void make_blobs PROTO ((short *layer, int numblobs, int blobradius, int blobalt));
static void limit_layer PROTO ((short *layer, int hi, int lo));
static void smooth_layer PROTO ((short *layer, int times));
static void percentile PROTO ((short *layer, int *percentiles));
static void compose_area PROTO ((void));
static int terrain_from_percentiles PROTO ((int x, int y));
static int dig_maze_path PROTO ((int x1, int y1, int dir1));
static int num_open_adj PROTO ((int x, int y));
static int random_solid_terrain PROTO ((void));
static int random_room_terrain PROTO ((void));
static int random_passage_terrain PROTO ((void));
static int high_point PROTO ((int x, int y));
static int water_point PROTO ((int x, int y));
static int bay_point PROTO ((int x, int y));
static void set_room_interior PROTO ((int x, int y));
static void name_highest_peaks PROTO ((Obj *parms));
static void name_lakes PROTO ((Obj *parms));
static char *name_feature_at PROTO ((int x, int y, char *typename));

/* The main function goes through a heuristically-determined process */
/* (read: I dinked until I liked the results) to make a area. */

/* Should add a cheap erosion simulator. */

int
make_fractal_terrain()
{
    int actualcells, altnumblobs, altblobradius, altblobalt;
    int wetnumblobs, wetblobradius, wetblobalt;

    /* Don't run if terrain is already present. */
    if (terrain_defined()) return FALSE;
    /* Note that we may still want this even if only one ttype defined,
       since elevs may still vary usefully. */
    /* Heuristic limit - this algorithm would get weird on small areas */
    if (area.width < 9 || area.height < 9) {
	init_warning("cannot generate fractal terrain for a %d x %d area",
		     area.width, area.height);
	return FALSE;
    }
    Dprintf("Going to make fractal terrain ...\n");
    allocate_area_scratch(3);
    histo  = (int *) xmalloc(MAXALT * sizeof(int));
    alts   = (int *) xmalloc(MAXALT * sizeof(int));
    wets   = (int *) xmalloc(MAXALT * sizeof(int));
    announce_lengthy_process("Making fractal terrain");
    /* Need a rough estimate of how much work involved, so can do progress. */
    if (g_alt_blob_density() > 0) {
	actualcells = (g_alt_blob_size() * area_cells()) / 10000;
	altblobradius = isqrt((actualcells * 4) / 3) / 2;
	altnumblobs = (g_alt_blob_density() * area_cells()) / 10000;
	altblobalt = g_alt_blob_height();
    }
    if (g_wet_blob_density() > 0) {
	actualcells = (g_wet_blob_size() * area_cells()) / 10000;
	wetblobradius = isqrt((actualcells * 4) / 3) / 2;
	wetnumblobs = (g_wet_blob_density() * area_cells()) / 10000;
	wetblobalt = g_wet_blob_height();
    }
    if (g_alt_blob_density() > 0) {
	/* Build a full relief area. */
	partdone = 0;
	make_blobs(area.tmp1, altnumblobs, altblobradius, altblobalt);
	/* Run the requested number of smoothing steps. */
	partdone += stepsize;
	smooth_layer(area.tmp1, g_alt_smoothing());
	percentile(area.tmp1, alts);
    }
    if (g_wet_blob_density() > 0) {
	/* Build a "moisture relief" area. */
	partdone += stepsize;
	make_blobs(area.tmp2, wetnumblobs, wetblobradius, wetblobalt);
	partdone += stepsize;
	smooth_layer(area.tmp2, g_wet_smoothing());
	percentile(area.tmp2, wets);
    }
    /* Put it all together. */
    partdone += stepsize;
    compose_area();
    add_edge_terrain();
    /* Free up what we don't need anymore. */
    free(histo);
    free(alts);
    free(wets);
    finish_lengthy_process();
    /* Report on the substitutions made. */
    if (numholes > 0) {
	init_warning("no possible terrain for %d cells, made them into %s",
		     numholes, t_type_name(0));
    }
    return TRUE;
}

static void
make_blobs(layer, numblobs, blobradius, blobalt)
short *layer;
int numblobs, blobradius, blobalt;
{
    int x0, y0, x1, y1, x2, y2, updown, x, y, xw;
    long maxdz, i, dz, oz;

    /* Init everything to the middle of the raw altitude range. */
    for_all_cells(x, y) aset(layer, x, y, MAXALT/2);
    numblobs = max(1, numblobs);
    maxdz = min(max(1, blobalt), MAXALT/2);
    Dprintf("Making %d blobs of radius %d max-dz %d...\n",
	    numblobs, blobradius, maxdz);
    /* Now lay down blobs. */
    for (i = 0; i < numblobs; ++i) {
	if (i % 100 == 0) {
	    announce_progress(partdone + (stepsize * i) / numblobs);
	}
	/* Decide whether we're making a hole or a hill. */
	updown = (flip_coin() ? 1 : -1);
	/* Pick a center for the bump. */
	random_point(&x0, &y0);
	if (blobradius <= 0) {
	    /* Special case for one-cell blobs. */
	    aadd(layer, x0, y0, updown * maxdz);
	} else {
	    /* Compute the LL corner. */
	    x1 = x0 - blobradius;  y1 = y0 - blobradius;
	    /* Compute the UR corner. */
	    x2 = x0 + blobradius;  y2 = y0 + blobradius;
	    /* Raise/lower all the cells within this bump. */
	    for (y = y1; y <= y2; ++y) {
		for (x = x1; x <= x2; ++x) {
		    xw = wrapx(x);
		    if ((x - x1 + y - y1 > blobradius)
			&& (x2 - x + y2 - y > blobradius)) {
			/* skip points outside the area */
			if (in_area(x, y)) {
			    oz = aref(layer, xw, y);
#if 1 /* ndef MPW_C */
			    /* Add some variation within the bump. */ 
			    dz = updown * (maxdz + xrandom(maxdz/2));
			    /* If dz is really extreme, cut it down. */
			    if (!between(0, oz + dz, MAXALT)) dz /= 2;
#else
			    dz = updown * maxdz / 2;
#endif
			    aset(layer, xw, y, oz + dz);
			}
		    }
		}
	    }
	}
    }
    /* Adding and subtracting might have got out of hand. */
    limit_layer(layer, MAXALT-1, 0); 
}

/* Ensure that area values stay within given range. */

static void
limit_layer(layer, hi, lo)
short *layer;
int hi, lo;
{
    int x, y, m;
    
    for_all_cells(x, y) {
	m = aref(layer, x, y);
	aset(layer, x, y, max(lo, min(m, hi)));
    }
}

/* Average each cell with its neighbors, using tmp3 as scratch layer. */

static void
smooth_layer(layer, times)
short *layer;
int times;
{
    int i, x, y, nx, px, dir, x1, y1, ndirs;
    long sum;

    for (i = 0; i < times; ++i) {
	Dprintf("Smoothing...\n");
	for (x = 0; x < area.width; ++x) {
	    nx = wrapx(x + 1);
	    px = wrapx(x - 1);
	    for (y = 0; y < area.height; ++y) {
		if (in_area(x, y)) {
		    sum = aref(layer, x, y);
		    if (inside_area(x, y) /* and hex geometry */) {
			sum += aref(layer, x, y+1);
			sum += aref(layer, nx, y);
			sum += aref(layer, nx, y-1);
			sum += aref(layer, x, y-1);
			sum += aref(layer, px, y);
			sum += aref(layer, px, y+1);
			sum /= (NUMDIRS + 1);
		    } else {
			/* Otherwise, use a slower but more general algorithm. */
			ndirs = 0;
			for_all_directions(dir) {
			    if (point_in_dir(x, y, dir, &x1, &y1)) {
				sum += aref(layer, x1, y1);
				++ndirs;
			    }
			}
			if (ndirs > 0)
			  sum /= ndirs;
		    }
		    set_tmp3_at(x, y, sum);
		}
	    }
	}
	for (x = 0; x < area.width; ++x) {
	    for (y = 0; y < area.height; ++y) {
		aset(layer, x, y, tmp3_at(x, y));
	    }
	}
	announce_progress(partdone + (stepsize * i) / times);
    }
}

/* Terrain types are specified in terms of percentage cover on a area, so
   for instance the Earth is 70% sea.  Since each of several types will have
   its own percentages (both for elevation and moisture), the simplest thing
   to do is to calculate the percentile for each raw elevation and moisture
   value, and save them all away.  */

/* Percentile computation is inefficient, should be done incrementally
   somehow instead of with * and / */

static void
percentile(layer, percentiles)
short *layer;
int *percentiles;
{
    int i, x, y, total;
    
    Dprintf("Computing percentiles...\n");
    limit_layer(layer, MAXALT-1, 0);
    for (i = 0; i < MAXALT; ++i) {
	histo[i] = 0;
	percentiles[i] = 0;
    }
    /* Make the basic histogram, counting only the inside. */
    for_all_interior_cells(x, y) {
	if (inside_area(x, y)) {
	    ++histo[aref(layer, x, y)];
	}
    }
    /* Integrate over the histogram */
    for (i = 1; i < MAXALT; ++i)
	histo[i] += histo[i-1];
    /* Total here should actually be same as number of cells in the area */
    total = histo[MAXALT-1];
    /* Compute the percentile position */
    for (i = 0; i < MAXALT; ++i) {
	percentiles[i] = (100 * histo[i]) / total;
    }
}

/* Final creation and output of the area. */

static void
compose_area()
{
    int x, y, rawelev, elev, t, elevrange[MAXTTYPES];

    Dprintf("Assigning terrain types to cells...\n");
    /* Make the terrain layer itself. */
    allocate_area_terrain();
    numholes = 0;
    for_all_interior_cells(x, y) {
	t = terrain_from_percentiles(x, y);
	set_terrain_at(x, y, t);
    }
    if (!world_is_flat()) {
    	/* Compute elevation variations of terrain. */
    	for_all_terrain_types(t) {
    		elevrange[t] = t_elev_max(t) - t_elev_min(t);
    	}
    	if (!elevations_defined()) {
	    allocate_area_elevations();
   	}
	for_all_cells(x, y) {
	    t = terrain_at(x, y);
	    rawelev = tmp1_at(x, y);
	    elev = ((rawelev * elevrange[t]) / MAXALT) + t_elev_min(t);
	    set_elev_at(x, y, elev);
	}
    }
}

/* Compute the actual terrain types.  This is basically a process of
   checking the percentile limits on each type against what is actually
   there. */

static int
terrain_from_percentiles(x, y)
int x, y;
{
    int t, rawalt = tmp1_at(x, y), rawwet = tmp2_at(x, y);

    if (numttypes == 1)
      return 0;
    for_all_terrain_types(t) {
	if (t_is_cell(t)
	    && between(t_alt_min(t), alts[rawalt], t_alt_max(t))
	    && between(t_wet_min(t), wets[rawwet], t_wet_max(t))) {
	    return t;
	}
    }
    /* No terrain maybe not an error, so just count and summarize later. */
    ++numholes;
    return 0;
}

/* Totally random (with weighting) terrain generation, as well as
   random elevations. */

int
make_random_terrain()
{
    int t, x, y, i, j, divvy, occurtable[100], elevrange;

    if (terrain_defined()) return FALSE;
    announce_lengthy_process("Making random terrain");
    Dprintf("Assigning terrain types...\n");
    /* (should fix this to allow any range of occurrence values) */
    i = 0;
    for_all_terrain_types(t) {
	if (t_occurrence(t) > 0) {
	    for (j = 0; j < t_occurrence(t); ++j) {
		if (i < 100) occurtable[i++] = t;
	    }
	} else {
	    divvy++;
	}
    }
    if (i == 0) {
    	for (i = 0; i < 100; ++i)
    	  occurtable[i] = 0;
    }
    allocate_area_terrain();
    /* Overwrite already-defined elevs? */
    if (!elevations_defined() && !world_is_flat())
      allocate_area_elevations();
    for_all_interior_cells(x, y) {
	t = occurtable[xrandom(100)];
	set_terrain_at(x, y, t);
	if (elevations_defined() && !world_is_flat()) {
	    elevrange = t_elev_max(t) - t_elev_min(t);
	    set_elev_at(x, y, xrandom(elevrange) + t_elev_min(t));
	}
    }
    /* Make sure the border of the area has something in it. */
    add_edge_terrain();
    finish_lengthy_process();
    return TRUE;
}

/* Method that is wired to be as close to earth as possible. */

/* (need to compute scale, get avg elevation and rainfall) */

int seatype = -1;
int landtype = -1;

int
make_earthlike_terrain()
{
    int t, x, y, elevrange;

    if (terrain_defined()) return FALSE;
    Dprintf("Categorizing terrain types...\n");
    for_all_terrain_types(t) {
    	if (strcmp("sea", t_type_name(t)) == 0) seatype = t;
    	if (strcmp("plains", t_type_name(t)) == 0) landtype = t;
    	/* etc */
    }
    if (seatype <= 0 || landtype <= 0) {
    	Dprintf("can't find earthlike terrain types");
    	return FALSE;
    }
    announce_lengthy_process("Making Earth-like terrain");
    allocate_area_terrain();
    /* Overwrite already-defined elevs? */
    if (!elevations_defined() && !world_is_flat())
      allocate_area_elevations();
    elevrange = maxelev - minelev;
    for_all_cells(x, y) {
	if (inside_area(x, y)) {
	    set_terrain_at(x, y, (flip_coin() ? seatype : landtype));
	    if (elevations_defined() && !world_is_flat()) {
	    	if (terrain_at(x, y) == seatype) {
		    set_elev_at(x, y, 0);
	    	} else {
		    set_elev_at(x, y, xrandom(elevrange - 1) + 1);
	    	}
	    }
	}
    }
    /* Make sure the border of the area has something in it. */
    add_edge_terrain();
    finish_lengthy_process();
    return TRUE;
}

/* Maze terrain generation.  */

int numsolidtypes = 0;
int numroomtypes = 0;
int numpassagetypes = 0;

int sumsolidoccur = 0;
int sumroomoccur = 0;
int sumpassageoccur = 0;

int solidtype = NONTTYPE;
int roomtype = NONTTYPE;
int passagetype = NONTTYPE;

int numpassagecells = 0;

static void
set_room_interior(x, y)
int x, y;
{
    set_terrain_at(x, y, random_room_terrain());
}

int
make_maze_terrain()
{
    int t, x, y, x1, y1, i;
    int dir, n;
    int numcells, tries = 0;
    int roomcells, roomradius, numrooms;
    int numsolidcells = 0, numpassagecellsneeded;

    if (terrain_defined()) return FALSE;
    for_all_terrain_types(t) {
    	if ((n = t_occurrence(t)) > 0) {
	    sumsolidoccur += n;
	    ++numsolidtypes;
	    solidtype = t;
    	}
    	if ((n = t_maze_room_occurrence(t)) > 0) {
	    sumroomoccur += n;
	    ++numroomtypes;
	    roomtype = t;
    	}
    	if ((n = t_maze_passage_occurrence(t)) > 0) {
	    sumpassageoccur += n;
	    ++numpassagetypes;
	    passagetype = t;
    	}
    }
    if (numsolidtypes + numroomtypes + numpassagetypes < 2) {
    	init_warning("No types to make maze with");
    	return FALSE;
    }
    announce_lengthy_process("Making maze terrain");
    allocate_area_terrain();
    /* Fill in the area with solid terrain. */
    for_all_cells(x, y) {
	set_terrain_at(x, y, random_solid_terrain());
    }
    /* Set the edges properly. */
    add_edge_terrain();
    numcells = area_cells();
    if (g_maze_room() > 0) {
	roomcells = 7;
	roomradius = 1;
	numrooms = ((numcells * g_maze_room()) / 10000) / roomcells;
	for (i = 0; i < numrooms; ++i) {
	    random_point(&x1, &y1);
	    apply_to_area(x1, y1, roomradius, set_room_interior);
	}
    }
    for_all_interior_cells(x, y) {
    	if (t_occurrence(terrain_at(x, y)) > 0) ++numsolidcells;
    }
    if (g_maze_passage() > 0) {
      numpassagecellsneeded = (numcells * g_maze_passage()) / 10000;
      while (numpassagecells < numpassagecellsneeded && tries++ < 500) {
    	random_point(&x1, &y1);
    	if (t_occurrence(terrain_at(x1, y1)) > 0) {
	    set_terrain_at(x1, y1, random_passage_terrain());
	    dir = random_dir();
	    dig_maze_path(x1, y1, dir);
	    dig_maze_path(x1, y1, opposite_dir(dir));
    	}
      }
    }
    /* Make sure the border of the area is fixed up. */
    add_edge_terrain();
    finish_lengthy_process();
    return TRUE;
}

static int
dig_maze_path(x1, y1, dir1)
int x1, y1, dir1;
{
    int found;
    int x, y, iter = 0, dir, dir2, nx, ny, nx2, ny2;
    int dug = 0;
    
    while (!inside_area(x1+dirx[dir1], y1+diry[dir1])) {
	dir1 = random_dir();
    }
    dir = dir1;
    x = x1;  y = y1;
    while (++iter < 500) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (!inside_area(nx, ny)) break;
	if (t_occurrence(terrain_at(nx, ny)) > 0
	    && num_open_adj(nx, ny) == 1) {
	    set_terrain_at(nx, ny, random_passage_terrain());
	    ++numpassagecells;
	    ++dug;
	} else {
	    found = FALSE;
	    for_all_directions(dir2) {
		point_in_dir(x, y, dir2, &nx2, &ny2);
		if (inside_area(nx2, ny2)
		    && t_occurrence(terrain_at(nx2, ny2)) > 0
		    && num_open_adj(nx2, ny2) == 1) {
		    set_terrain_at(nx2, ny2, random_passage_terrain());
		    ++numpassagecells;
		    ++dug;
		    found = TRUE;
		    dir = dir2;
		    break;
		}
	    }
	    if (!found) {
		return dug;
	    }
	}
	if (probability(20)) {
	    dig_maze_path(nx, ny, left_dir(dir));
	    dig_maze_path(nx, ny, right_dir(dir));
	    return dug;
	} else {
	    x = nx;  y = ny;
	    dir = (probability(50) ? dir : random_dir());
	}
    }
    return dug;
}

static int
num_open_adj(x, y)
int x, y;
{
    int dir, rslt = 0, nx, ny;

    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (t_maze_room_occurrence(terrain_at(nx, ny)) > 0
	    || t_maze_passage_occurrence(terrain_at(nx, ny)) > 0) ++rslt;
    }
    return rslt;
}

static int
random_solid_terrain()
{
    if (numsolidtypes == 1) return solidtype;
    return (xrandom(numttypes));
}

static int
random_room_terrain()
{
    if (numroomtypes == 1) return roomtype;
    return (xrandom(numttypes));
}

static int
random_passage_terrain()
{
    if (numpassagetypes == 1) return passagetype;
    return (xrandom(numttypes));
}

/* This method adds some randomly named geographical features. */

/* (This needs to interact properly with convex region finder eventually) */

int
name_geographical_features()
{
    char *classname;
    Obj *rest;

    /* If we got features from file or somewhere, don't overwrite them. */
    if (features_defined())
      return FALSE;
    /* We need to have some terrain to work from. */
    if (!terrain_defined())
      return FALSE;
    /* If no feature types requested, don't make any. */
    if (g_feature_types() == lispnil)
      return FALSE;
    announce_lengthy_process("Adding geographical features");
    Dprintf("Adding geographical features...\n");
    /* Set up the basic layer of data. */
    init_features();
    /* Scan through list to see what's being requested. */
    for (rest = g_feature_types(); rest != lispnil; rest = cdr(rest)) {
	if (consp(car(rest)) && stringp(car(car(rest)))) {
	    classname = c_string(car(car(rest)));
	    if (strcmp(classname, "peak") == 0) {
	    	name_highest_peaks(cdr(car(rest)));
	    } else if (strcmp(classname, "lake") == 0) {
		name_lakes(cdr(car(rest)));
	    } else {
		run_warning("Don't know to identify \"%s\" features", classname);
	    }
	} else {
	    run_warning("Clause not recognized");
	}
    }
    finish_lengthy_process();
    return TRUE;
}

/* Identify the highest high points as "peaks". */

static void
name_highest_peaks(parms)
Obj *parms;
{
    int x, y;
    int maxpeaks = (area.width * area.height) / 200;
    int numpeaks, *peakx, *peaky, i, lo;
    char *name;
    Feature *mountain;

    if (!elevations_defined() || world_is_flat()) {
	run_warning("Can't identify peaks, world is flat");
	return;
    }
    peakx = (int *) xmalloc(maxpeaks * sizeof(int));
    peaky = (int *) xmalloc(maxpeaks * sizeof(int));
    numpeaks = 0;

    for_all_interior_cells(x, y) {
	if (high_point(x, y)) {
		    if (numpeaks < maxpeaks) {
			peakx[numpeaks] = x;  peaky[numpeaks] = y;
			++numpeaks;
		    } else {
			/* Find the lowest of existing peaks. */
			lo = 0;
			for (i = 0; i < numpeaks; ++i) {
			    if (elev_at(peakx[i], peaky[i]) <
				elev_at(peakx[lo], peaky[lo])) {
				lo = i;
			    }
			}
			/* If less than our new candidate, replace. */
			if (elev_at(x, y) > elev_at(peakx[lo], peaky[lo])) {
			    peakx[lo] = x;  peaky[lo] = y;
			}
		    }
	}
    }
    for (i = 0; i < numpeaks; ++i) {
	name = name_feature_at(x, y, "peak");
	if (name == NULL) {
	    sprintf(tmpbuf, "Pk %d", elev_at(peakx[i], peaky[i]));
	    name = copy_string(tmpbuf);
	}
	mountain = create_feature("peak", name);
	mountain->size = 1;
	set_raw_feature_at(peakx[i], peaky[i], mountain->id);
    }
}

/* True if xy is a local high point. */

static int
high_point(x, y)
int x, y;
{
    int dir, nx, ny;

    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (elev_at(nx, ny) >= elev_at(x, y)) {
	    return FALSE;
	}
    }
    return TRUE;
}

static void
name_lakes(parms)
Obj *parms;
{
    int x, y;
    char *name;
    Feature *lake, *bay;

    for_all_interior_cells(x, y) {
	if (water_point(x, y)) {
	    name = name_feature_at(x, y, "lake");
	    if (name != NULL) {
		lake = create_feature("lake", name);
		lake->size = 1;
		set_raw_feature_at(x, y, lake->id);
	    }
	} else if (bay_point(x, y)) {
	    name = name_feature_at(x, y, "bay");
	    if (name != NULL) {
		bay = create_feature("bay", name);
		bay->size = 1;
		set_raw_feature_at(x, y, bay->id);
	    }
	}
    }
}

/* True if xy is isolated water. */

static int
water_point(x, y)
int x, y;
{
    int dir, nx, ny;

    if (strcmp(t_type_name(terrain_at(x, y)), "sea") != 0
        && strcmp(t_type_name(terrain_at(x, y)), "shallows") != 0)
      return FALSE;
    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (terrain_at(x, y) == terrain_at(nx, ny) || t_liquid(terrain_at(nx, ny))) {
	    return FALSE;
	}
    }
    return TRUE;
}

static int
bay_point(x, y)
int x, y;
{
    int dir, nx, ny;
    int seacount = 0, landcount = 0;

    if (strcmp(t_type_name(terrain_at(x, y)), "shallows") != 0)
      return FALSE;
    for_all_directions(dir) {
	point_in_dir(x, y, dir, &nx, &ny);
	if (strcmp(t_type_name(terrain_at(x, y)), "sea") == 0)
	  ++seacount;
	else
	  ++landcount;
    }
    return (seacount > 0 && landcount > 2);
}

static char *name_feature_at_using PROTO ((Obj *namerlist, int x, int y, char *typename));
/* (should reindent) */
static char *
name_feature_at(x, y, typename)
int x, y;
char *typename;
{
    char *rslt;
    Obj *namerlist;
    Side *side;

    /* Look for any side-specific namers. */
    if (people_sides_defined()) {
      for_all_sides(side) {
	if (side->featurenamers != NULL
	    && side->featurenamers != lispnil
	    && people_side_at(x, y) == side->id) {
	    rslt = name_feature_at_using(side->featurenamers, x, y, typename);
	    if (rslt != NULL)
	      return rslt;
	}
      }
    }
    /* Now try generic namer list. */
    namerlist = g_feature_namers();
    /* If no generic namers, get out of here. */
    if (namerlist == lispnil)
      return NULL;
    return name_feature_at_using(namerlist, x, y, typename);
}

static char *
name_feature_at_using(namerlist, x, y, typename)
Obj *namerlist;
int x, y;
char *typename;
{
    Obj *rest, *namerspec;

    /* If no namers found, get out of here. */
    if (namerlist == lispnil)
      return NULL;
    for (rest = namerlist; rest != lispnil; rest = cdr(rest)) {
	if (consp(car(rest))
	    && stringp(car(car(rest)))
	    && strcmp(c_string(car(car(rest))), typename) == 0) {
	    namerspec = cadr(car(rest));
	    /* This might be a string naming a namer, try making it into
	       a symbol. */
	    if (stringp(namerspec))
	      namerspec = intern_symbol(c_string(namerspec));
    	    if (boundp(namerspec))
	      return run_namer(symbol_value(namerspec));
	    /* If the namer doesn't exist, then this will continue looking for
	       one that does, which is maybe good, because this might fall back
	       from a side-specific to a generic namer, but can be confusing to
	       game designers, because then a feature might be created with no
	       name, with no warning of a problem with namers. */
	}
    }
    return NULL;
}

/* For efficiency and semantics reasons, the methods might not assign values
   to the cells around the edge of the area (if there *are* edges; neither
   a torus nor sphere will have any).  Note that there is no way to
   disable this from the game module; if having nonconstant edges is important
   enough to be worth the user confusion, don't call this from your
   area generation method. */

/* (this needs to set elevs too, maybe other stuff on edges?) */

void
add_edge_terrain()
{
    int x, y, t = g_edge_terrain(), halfheight = area.height / 2;

    /* Use ttype 0 if edge terrain is nonsensical. */
    if (!between(0, t, numttypes-1))
      t = 0;
    /* Right/left upper/lower sides of a hexagon. */
    if (!area.xwrap) {
	for (y = 0; y < halfheight; ++y) {
	    /* SW edge */
	    set_terrain_at(halfheight - y, y, t);
	    /* NW edge */
	    set_terrain_at(0, halfheight + y, t);
	    /* SE edge */
	    set_terrain_at(area.width-1, y, t);
	    /* NE edge */
	    set_terrain_at(area.width-1 - y, halfheight + y, t);
	}
    }
    /* Top and bottom edges of the area. */
    for (x = 0; x < area.width; ++x) {
	set_terrain_at(x, 0, t);
	set_terrain_at(x, area.height-1, t);
    }
}
