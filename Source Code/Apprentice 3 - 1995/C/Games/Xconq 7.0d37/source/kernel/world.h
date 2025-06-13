/* Definitions relating to worlds and areas in Xconq.
   Copyright (C) 1987, 1988, 1989, 1991, 1992, 1993, 1994, 1995
   Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

typedef struct a_world {
    int circumference;		/* number of cells going around the world */
    int daylength;		/* number of turns in a day */
    int yearlength;		/* number of turns in a year */
    int axialtilt;		/* controls extrema of seasons */
    int firstmidwinter;
    int summernorthpoletemperature;
    int winternorthpoletemperature;
    int summernorthequatortemperature;
    int winternorthequatortemperature;
} World;

/* Theoretically, there is no maximum size to Xconq areas, but the minimum
   size is set by mystical properties, and is not negotiable. */

#define MINWIDTH 3
#define MINHEIGHT 3

/* An "area" is always basically a rectangular array of positions.  The
   hex effect can be achieved by interpreting neighborliness and direction
   differently, but that's all that's needed - storagewise the rect
   array is still the best choice.  All of the "layers" are dynamically
   allocated as needed, to save (considerable!) space. */

typedef struct a_area {
    int width, height;		/* size of the area */
    int maxdim;			/* max of the two dims */
    int xwrap;			/* true if x coords wrap around */
    long latitude, longitude;	/* position within whole world */
    long cellwidth;		/* distance across one cell */
    short fullwidth, fullheight; /* size of the area being used for data */
    short fullx, fully;		/* offset within full area to get data from */
    /* Pointers to the various "layers". */
    struct a_unit **units;	/* pointer to units if any */
    char *terrain;		/* terrain type at this spot */
    char **auxterrain;		/* vector of extra types */
    char *peopleside;		/* the overt side alignment of the locals */
    short *features;		/* layer of ids of features */
    short *elevations;		/* layer of elevations */
    short constelev;		/* a constant elevation */
    short **materials;		/* layer of materials in each cell */
    short *temperature;		/* layer of cell temperatures */
    short consttemp;		/* a constant temperature */
    short *clouds;		/* types of clouds in the layer */
    short *cloudbottoms;	/* altitudes of clouds of cloud layer */
    short *cloudheights;	/* heights of clouds in the cloud layer */
    short *winds;		/* layer of force/dir of winds. */
    short constwinds;		/* constant wind */
    /* These layers are for temporary use in calculations. */
    short *tmp1;
    short *tmp2;
    short *tmp3;
} Area;

/* Named geographical features. */

typedef struct a_feature {
    int type;			/* index of the general type */
    short id;			/* which one this is */
    char *name;			/* the name of the region */
    char *typename;		/* its category, such as "island" or "bay" */
    /* something for syntax? "foo bay" vs "bay of foo" */
    short superid[3];		/* indices of containing regions */
    struct a_feature *next;	/* arranged in a linked list */
    /* cached info about a region */
    int size;
    int x, y;
    int mindiam;
    int maxdiam;
    int relabel;
} Feature;

/* Paths. */

typedef struct a_waypoint {
    int x, y;
    int note;
} Waypoint;

typedef struct a_path {
    int numwps;
    Waypoint wps[100];
} Path;

/* Use this macro to get a area-spanning layer of the given type. */

#define malloc_area_layer(TYPE)  \
  ((TYPE *) xmalloc(area.width * area.height * sizeof(TYPE)))

#define zero_area_layer(ADDR, TYPE)  \
  (memset(ADDR, 0, area.width * area.height * sizeof(TYPE)))

/* General 2D malloced area array usage.  Names from Lisp. */

#define aref(m,x,y) ((m)[area.width * (y) + (x)])

#define aset(m,x,y,v) ((m)[area.width * (y) + (x)] = (v))

#define aadd(m,x,y,v) ((m)[area.width * (y) + (x)] += (v))

/* The unit is a raw pointer - this macro is used a *lot*.  This could
   be space-optimized by using a 16-bit unit id. */

#define unit_at(x,y) aref(area.units, x, y)

#define set_unit_at(x,y,u) aset(area.units, x, y, u)

/* Iterate through all units in this cell (but not their occs) . */

#define for_all_stack(x,y,var)  \
  for ((var) = unit_at((x), (y)); (var) != NULL;  (var) = (var)->nexthere)

/* Test if the terrain has been allocated yet. */

#define terrain_defined() (area.terrain != NULL)

/* The terrain at each cell is just the number of the terrain type. */

#define terrain_at(x,y) aref(area.terrain, x, y)

#define set_terrain_at(x,y,t) aset(area.terrain, x, y, t)

/* Auxiliary terrain array of layers. */

#define any_aux_terrain_defined() (area.auxterrain != NULL)

#define aux_terrain_defined(t)  \
  (any_aux_terrain_defined() && area.auxterrain[t] != NULL)

#define aux_terrain_at(x,y,t) aref(area.auxterrain[t], x, y)

#define set_aux_terrain_at(x,y,t,v) aset(area.auxterrain[t], x, y, v)

/* Not really correct, should finish. */

#define any_borders_at(x, y, b) (aux_terrain_at(x, y, b) != 0)

#define any_connections_at(x, y, c) (aux_terrain_at(x, y, c) != 0)

/* Elevation layer. */

#define world_is_flat() (minelev == maxelev)

#define elevations_defined() (area.elevations != NULL)

#define elev_at(x,y) aref(area.elevations, x, y)

#define set_elev_at(x,y,v) aset(area.elevations, x, y, v)

/* Feature layer. */

#define features_defined() (area.features != NULL)

/* The "raw feature" is its "short" identifier. */

#define raw_feature_at(x,y) aref(area.features, x, y)

#define set_raw_feature_at(x,y,f) aset(area.features, x, y, f)

/* Population layer. */

#define people_sides_defined() (area.peopleside != NULL)

#define people_side_at(x,y) aref(area.peopleside, x, y)

#define set_people_side_at(x,y,s) aset(area.peopleside, x, y, s)

/* A cell might be entirely uninhabited, so need an extra value to indicate. */

#define NOBODY (MAXSIDES+1)

#define populated(x,y) (people_side_at(x,y) != NOBODY)

/* Array of material layers. */

#define any_cell_materials_defined() (area.materials != NULL)

#define cell_material_defined(m) (area.materials[m] != NULL)

#define material_at(x,y,m) aref(area.materials[m], x, y)

#define set_material_at(x,y,m,v) aset(area.materials[m], x, y, v)

/* Temperature layer. */

#define temperatures_defined() (area.temperature != NULL)

#define temperature_at(x,y) aref(area.temperature, x, y)

#define set_temperature_at(x,y,v) aset(area.temperature, x, y, v)

/* Clouds layer. */

#define clouds_defined() (area.clouds != NULL)

#define raw_cloud_at(x,y) aref(area.clouds, x, y)

#define set_raw_cloud_at(x,y,v) aset(area.clouds, x, y, v)

#define cloud_bottoms_defined() (area.cloudbottoms != NULL)

#define raw_cloud_bottom_at(x,y) aref(area.cloudbottoms, x, y)

#define set_raw_cloud_bottom_at(x,y,v) aset(area.cloudbottoms, x, y, v)

#define cloud_heights_defined() (area.cloudheights != NULL)

#define raw_cloud_height_at(x,y) aref(area.cloudheights, x, y)

#define set_raw_cloud_height_at(x,y,v) aset(area.cloudheights, x, y, v)

/* Winds layer. */

#define winds_defined() (area.winds != NULL)

#define raw_wind_at(x,y) aref(area.winds, x, y)

#define set_raw_wind_at(x,y,v) aset(area.winds, x, y, v)

#define wind_dir_at(x,y) (raw_wind_at(x, y) & 0x07)

#define wind_force_at(x,y) (raw_wind_at(x, y) >> 3)

#define set_wind_at(x,y,d,f) (set_raw_wind_at(x, y, ((f) << 3) | (d)))

/* Handlers for scratch layers. */

#define tmp1_at(x,y) aref(area.tmp1, x, y)

#define set_tmp1_at(x,y,v) aset(area.tmp1, x, y, v)

#define tmp2_at(x,y) aref(area.tmp2, x, y)

#define set_tmp2_at(x,y,v) aset(area.tmp2, x, y, v)

#define tmp3_at(x,y) aref(area.tmp3, x, y)

#define set_tmp3_at(x,y,v) aset(area.tmp3, x, y, v)

/* This little macro implements wraparound in the x direction. */
/* The stupid add is for the benefit of brain-damaged mod operators */
/* that don't handle negative numbers properly. */

/* (This could use some general cleanup) */

#define wrap(x) (area.xwrap ? (((x) + area.width) % area.width) : (x))

#define wrapx(x) (area.xwrap ? (((x) + area.width) % area.width) : (x))

/* Constrain y to northern and southern edges. */

#define limit(y) (max(0, min((y), (area.height-1))))

#define interior(y) (max(1, min((y), (area.height-2))))

#define xy_in_dir(x,y,d,nx,ny) \
  (nx) = wrapx((x) + dirx[d]);  (ny) = (y) + diry[dir];

/* Iteration over all valid cell positions in a area.  These should be
   used carefully, since they don't (can't) have any grouping braces
   embedded. */

#define for_all_cells(x,y)  \
  for (x = 0; x < area.width; x++)  \
    for (y = 0; y < area.height; y++)  \
      if (in_area(x, y))

/* This doesn't generate positions along area edges.  Typically more
   useful within game. */

#define for_all_interior_cells(x,y)  \
  for (x = 0; x < area.width; x++)  \
    for (y = 1; y < area.height - 1; y++)  \
      if (inside_area(x, y))

/* Returns the lighting state of a given position. */
/* (should opencode distance call here) */

#define lighting(x,y,snx,sny)  \
  ((distance(x, y, snx, sny) < (3 * world.circumference) / 5) ? 2 : 0)

/* True if the given x,y is dark. */

#define night_at(x,y) (daynight && lighting((x), (y), (int) sunx, (int) suny) == 0)

/* World-related variables. */

extern World world;

extern Area area;

extern int midturnrestore;

extern int numcelltypes;
extern int numbordtypes;
extern int numconntypes;
extern int numcoattypes;

extern int minelev;
extern int maxelev;
extern int mintemp;
extern int maxtemp;
extern int minwindforce;
extern int maxwindforce;

extern int any_materials_in_terrain;
extern int any_temp_variation;
extern int any_temp_variation_in_layer;
extern int any_wind_variation;
extern int any_wind_variation_in_layer;
extern int any_clouds;

/* World-related functions. */

extern void init_world PROTO ((void));
extern int set_world_circumference PROTO ((int circum, int warn));
extern int set_area_shape PROTO ((int width, int height, int warn));
extern int valid_area_shape PROTO ((int width, int height, int warn));
extern void check_area_shape PROTO ((void));
extern void calculate_world_globals PROTO ((void));
extern void count_terrain_subtypes PROTO ((void));

extern void allocate_area_terrain PROTO ((void));
extern void allocate_area_aux_terrain PROTO ((int t));
extern void allocate_area_scratch PROTO ((int n));
extern void allocate_area_elevations PROTO ((void));
extern void allocate_area_temperatures PROTO ((void));
extern void allocate_area_people_sides PROTO ((void));
extern void allocate_area_material PROTO ((int m));
extern void allocate_area_clouds PROTO ((void));
extern void allocate_area_cloud_altitudes PROTO ((void));
extern void allocate_area_cloud_bottoms PROTO ((void));
extern void allocate_area_cloud_heights PROTO ((void));
extern void allocate_area_winds PROTO ((void));

extern int fn_terrain_at PROTO ((int x, int y));
extern int fn_aux_terrain_at PROTO ((int x, int y));
extern int fn_feature_at PROTO ((int x, int y));
extern int fn_elevation_at PROTO ((int x, int y));
extern int fn_people_side_at PROTO ((int x, int y));
extern int fn_material_at PROTO ((int x, int y));
extern int fn_temperature_at PROTO ((int x, int y));
extern int fn_raw_cloud_at PROTO ((int x, int y));
extern int fn_raw_cloud_bottom_at PROTO ((int x, int y));
extern int fn_raw_cloud_height_at PROTO ((int x, int y));
extern int fn_raw_wind_at PROTO ((int x, int y));

extern void fn_set_terrain_at PROTO ((int x, int y, int val));
extern void fn_set_aux_terrain_at PROTO ((int x, int y, int val));
extern void fn_set_people_side_at PROTO ((int x, int y, int val));
extern void fn_set_raw_feature_at PROTO ((int x, int y, int val));
extern void fn_set_elevation_at PROTO ((int x, int y, int val));
extern void fn_set_material_at PROTO ((int x, int y, int val));
extern void fn_set_temperature_at PROTO ((int x, int y, int val));
extern void fn_set_raw_wind_at PROTO ((int x, int y, int val));
extern void fn_set_raw_cloud_at PROTO ((int x, int y, int val));
extern void fn_set_raw_cloud_bottom_at PROTO ((int x, int y, int val));
extern void fn_set_raw_cloud_height_at PROTO ((int x, int y, int val));

extern int search_around PROTO ((int x0, int y0, int maxdist,
				 int (*pred)(int, int),
				 int *rxp, int *ryp, int incr));
extern int search_and_apply PROTO ((int x0, int y0, int maxdist,
				    int (*pred)(int, int),
				    int *rxp, int *ryp, int incr,
				    void (*fn)(int, int), long num));
extern void apply_to_area PROTO ((int x0, int y0, int dist,
				  void (*fn)(int, int)));
extern void apply_to_area_plus_edge PROTO ((int x0, int y0, int dist,
					    void (*fn)(int, int)));
extern void apply_to_ring PROTO ((int x0, int y0, int distmin, int distmax,
				  void (*fn)(int, int)));
extern void apply_to_hexagon PROTO ((int x0, int y0, int w2, int h2,
				     void (*fn)(int, int)));
extern void apply_to_path PROTO ((int fx, int fy, int tx, int ty,
				  int (*dirfn)(int, int, int *, int),
				  int (*fn)(int, int, int, int, int),
				  int shortest));
int find_path PROTO ((int fx, int fy, int tx, int ty,
		      int (*chooser)(int, int, int, int, int *),
		      int maxwps, Waypoint *waypoints, int *numwpsp));

extern int in_area PROTO ((int x, int y));
extern int inside_area PROTO ((int x, int y));
extern int area_cells PROTO ((void));
extern int border_at PROTO ((int x, int y, int dir, int t));
extern void set_border_at PROTO ((int x, int y, int dir, int t, int onoff));
extern int connection_at PROTO ((int x, int y, int dir, int t));
extern void set_connection_at PROTO ((int x, int y, int dir, int t, int onoff));
extern void patch_linear_terrain PROTO ((int t));
extern void init_features PROTO ((void));
extern Feature *create_feature PROTO ((char *typename, char *name));
extern Feature *find_feature PROTO ((int fid));
extern Feature *feature_at PROTO ((int x, int y));
extern void set_feature_type_name PROTO ((Feature *feature, char *typename));
extern void set_feature_name PROTO ((Feature *feature, char *name));
extern void destroy_feature PROTO ((Feature *feature));
extern void renumber_features PROTO ((void));
extern void compute_all_feature_centroids PROTO ((void));
extern void compute_feature_centroid PROTO ((Feature *feature));

extern int point_in_dir PROTO ((int x, int y, int dir, int *xp, int *yp));
extern int interior_point_in_dir PROTO ((int x, int y, int dir, int *xp, int *yp));
extern int point_in_dir_n PROTO ((int x, int y, int dir, int n, int *xp, int *yp));
extern int random_point PROTO ((int *xp, int *yp));
extern int random_point_near PROTO ((int cx, int cy, int radius, int *xp, int *yp));
extern int random_point_in_area PROTO ((int cx, int cy, int rx, int ry, int *xp, int *yp));
extern void terrain_subtype_warning PROTO ((char *context, int t));
extern int approx_dir PROTO ((int dx, int dy));
extern int hextant PROTO ((int dx, int dy));
extern int distance PROTO ((int x1, int y1, int x2, int y2));
extern int closest_dir PROTO ((int x, int y));

#ifdef DESIGNERS

extern void paint_cell PROTO ((Side *side, int x, int y, int r, int t));
extern void paint_border PROTO ((Side *side, int x, int y, int dir, int t, int mode));
extern void paint_connection PROTO ((Side *side, int x, int y, int dir, int t, int mode));
extern void paint_coating PROTO ((Side *side, int x, int y, int r, int t, int depth));
extern void paint_people PROTO ((Side *side, int x, int y, int r, int s));
extern void paint_feature PROTO ((Side *side, int x, int y, int r, int f));
extern void paint_elevation PROTO ((Side *side, int x, int y, int r, int elev));
extern void paint_temperature PROTO ((Side *side, int x, int y, int r, int temp));
extern void paint_material PROTO ((Side *side, int x, int y, int r, int m, int amt));
extern void paint_clouds PROTO ((Side *side, int x, int y, int r, int cloudtype, int bot, int hgt));
extern void paint_winds PROTO ((Side *side, int x, int y, int r, int dir, int force));

#endif /* DESIGNERS */
