/* Copyright (c) 1987-1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions relating to worlds and areas in them. */

typedef struct a_world {
    int circumference;     /* number of cells going around the world */
    int daylength;         /* number of turns in a day */
    int yearlength;        /* number of turns in a year */
    int axialtilt;         /* controls extrema of seasons */
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
   hex effect is achieved interpreting neighborliness and direction
   differently, but that's all that's needed - storagewise the rect
   array is still the best choice.  All of the "layers" are dynamically
   allocated as needed, to save (considerable!) space. */

typedef struct a_area {
    short width, height;    /* size of the area */
    short maxdim;           /* max of the two dims */
    short xwrap;            /* true if x coords wrap around */
    long latitude, longitude; /* position within whole world */
    long cellwidth;         /* distance across one cell */
    short fullwidth, fullheight; /* size of the area being used for data */
    short fullx, fully;     /* offset within full area to get data from */
    /* Pointers to the various "layers". */
    struct a_unit **units;  /* pointer to units if any */
    char *terrain;          /* terrain type at this spot */
    char **auxterrain;      /* vector of extra types */
    char *peopleside;       /* the overt side alignment of the locals */
    short *features;        /* layer of ids of features */
    short *elevations;      /* layer of elevations */
    short constelev;        /* a constant elevation */
    short **materials;      /* layer of materials in each cell */
    short *temperature;     /* layer of cell temperatures */
    short consttemp;        /* a constant temperature */
    short *clouds;          /* types of clouds in the layer */
    short *cloudbottoms;    /* altitudes of clouds of cloud layer */
    short *cloudheights;    /* heights of clouds in the cloud layer */
    short *winds;           /* layer of force/dir of winds. */
    short constwinds;       /* constant wind */
    /* These layers are for temporary use in calculations. */
    short *tmp1;
    short *tmp2;
    short *tmp3;
} Area;

/* Named geographical features. */

typedef struct a_feature {
    int type;               /* index of the general type */
    short id;               /* which one this is */
    char *name;             /* the name of the region */
    char *typename;         /* its category, such as "island" or "bay" */
    /* something for syntax? "foo bay" vs "bay of foo" */
    short superid[3];       /* indices of containing regions */
    struct a_feature *next; /* arranged in a linked list */
    /* cached info about a region */
    int size;
    int mindiam;
    int maxdiam;
    int relabel;
} Feature;

/* Use this macro to get a area-spanning layer of the given type. */

#define malloc_area_layer(TYPE)  \
  ((TYPE *) xmalloc(area.width * area.height * sizeof(TYPE)))

#define zero_area_layer(ADDR, TYPE)  \
  (bzero(ADDR, area.width * area.height * sizeof(TYPE)))

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

/* The terrain at each hex is just the number of the terrain type. */

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

/* Iteration over all valid hex positions in a area.  These should be
   used carefully, since they don't (can't) have any grouping braces
   embedded. */

#define for_all_hexes(x,y)  \
  for (x = 0; x < area.width; x++)  \
    for (y = 0; y < area.height; y++)  \
      if (in_area(x, y))

/* This doesn't generate positions along area edges.  Typically more
   useful within game. */

#define for_all_interior_hexes(x,y)  \
  for (x = 0; x < area.width; x++)  \
    for (y = 1; y < area.height - 1; y++)  \
      if (inside_area(x, y))

/* Returns the lighting state of a given position. */

#define lighting(x,y,snx,sny)  \
  ((distance(wrapx(x), y, snx, sny) < (3 * world.circumference) / 5) ? 2 : 0)

/* True if the given x,y is dark. */

#define night_at(x,y) (daynight && lighting((x), (y), sunx, suny) == 0)

/* Paths and path manipulation. */

typedef struct a_waypoint {
    int x, y;
    int note;
} Waypoint;

typedef struct a_path {
    int numwps;
    Waypoint wps[100];
} Path;

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

/* World-related functions. */

Feature *create_feature();
Feature *find_feature();
char *feature_name_at();
char *absolute_date_string();
char *greg_date_string();
