/* PostScript(tm) printing for Xconq.
   Copyright (C) 1994, 1995 Massimo Campostrini & Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*
	Problems
	--------
   Terrain prints reasonably with terrain_gray = 0.5 and
terrain_dither = 0 (at least on a LaserWriter Pro 630 at 600dpi).
terrain_gray = 0 prints terrain icons which are easier to distinguish,
but make many terrains too dark.  Perhaps design new, lighter bitmaps
for terrains?
   Small feature legends are unreadable over dark terrains and connections. 
Legends over terrain improve if terrain_gray is set to 0.75, but then 
terrains are hard to distinguish.  Some kind of masking is in order, at 
least for small legends.

	TODO
	----
   How to mask feature legends?  Use "reverse outline" fonts (from 
"outline.ps" in the cookbook)?  Or just blank out a rectangle under small 
legends, like what is now done for unit names?
   The front end should announce "printing view to file..." and "done"
[done for xconq].
   Implement selection of the region to print.
   Read and interpret `embed' forms.
   Use border/connection bitmaps?  Or differentiate otherwise
between borders or connections?
   Print more fancy legends, e.g., list of side names/emblems 
(on a separate page?)
   Merge parameters better into xconq: initialize by resources and let
each front-end change them (MAC & X should pop-up a window  [X done]).
   If a bitmap is missing but a color image is present, convert it? How?
   Better choice of unit to display (for crowded cells).  How?
   Fix priniting of unit names spanning more than one cell.
   Mark friendly/enemy units.  How?
   More cleanup.

	How to customize
	----------------
   The output look is controlled by a bunch of variables in the
routine init_print.  Their default values are "wired in" into the code,
but xconq may pop-up a configuration window.  Check that the default 
`page_width' and `page_height' fit your paper sheets.

   Mail me suggestions and comments.  I would especially appreciate
suggestions for generating cleaner, faster, more compact PostScript.

        Massimo Campostrini,
Istituto Nazionale di Fisica Nucleare, Sezione di Pisa,
Piazza Torricelli 2, I56126 Pisa, Italy  ||  Phone: (+39)(50)911272
Internet: campo@sunthpi3.difi.unipi.it   ||  Fax:   (+39)(50)48277  */

#include "conq.h"
#include "print.h"
#include "imf.h"
#include <math.h>

PrintParameters *pp = NULL;

#define on_page(x,y) (((x)>=0) && ((x)<width) && ((y)>=0) && ((y)<height))

/* masks for name placing */
#define N_U  '\001'
#define N_D  '\002'
#define N_BK '\020'

#define name_at(x,y)  (name_layer[width*((y)+1)+(x)+1])

#define GRAY_SCALE 16	      /* precision for dithering */
/* length of a hexadecimal string encoding a w*h rectangle */
#define area_leng(w,h) ((((w)+7)/8) * (h) * 2)

/* this will allow images up to 64*64 */
#define PSBUFSIZE 1148

static int name_dir[] =
  { EAST, WEST, SOUTHEAST, NORTHEAST, SOUTHWEST, NORTHWEST };
/* maximgfam = initial estimate of number of image families in library */
static int maximgfam = 500;
static char *buffer;
static char *escbuf;
static char *padbuf;
static Side *ps_side;
static char zero[] = "00";
static int img_cooked = 0, terr_scale, terr_gray, x0, width, height;
static double ru, rtw, rth, asym, sqrt3,
              cell_grid_width, border_width, connection_width;

/* h = hex height; c = vert. distance between hex centers;
   w = hex width = horiz. distance between hex centers.
   For undeformed hexes, c = h*3/4, w = h*sqrt(3)/2;
   the following asymmetry allows 8x8 blocks to tile properly,
   i.e. hex_w%16==0 && hex_c%8==0
   Moreover, the code assumes  hex_w%4==0 && hex_c%2==0 && hex_h%4==0 */
static int hex_h = 32;
static int hex_c = 24;
static int hex_w = 32;
/* unit icon size (compare to hex_w, *not* to hex_h) */
static int unit_w = 16;

/* PostScript fonts description */

static char *ps_name[3] = {
  "Helvetica-Narrow",
  "Helvetica",
  "Helvetica-Bold"
};

/* height of a capital letter in PostScript Helvetica,
   in thousands of the "nominal size" */
static int ps_height = 729;

/* width of letters in PostScript Helvetica fonts,
   in thousands of the "nominal size" */
static short ps_width[3][256] = {
/* font 0 is Helvetica-Narrow */
  {
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
  228,  228,  291,  456,  456,  729,  547,  182,
  273,  273,  319,  479,  228,  273,  228,  228,
  456,  456,  456,  456,  456,  456,  456,  456,
  456,  456,  228,  228,  479,  479,  479,  456,
  832,  547,  547,  592,  592,  547,  501,  638,
  592,  228,  410,  547,  456,  683,  592,  638,
  547,  638,  592,  547,  501,  592,  547,  774,
  547,  547,  501,  228,  228,  228,  385,  456,
  182,  456,  456,  410,  456,  456,  228,  456,
  456,  182,  182,  410,  182,  683,  456,  456,
  456,  456,  273,  410,  228,  456,  410,  592,
  410,  410,  410,  274,  213,  274,  479,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,  273,  456,  456,  137,  456,  456,  456,
  456,  157,  273,  456,  273,  273,  410,  410,
    0,  456,  456,  456,  228,    0,  440,  287,
  182,  273,  273,  456,  820,  820,    0,  501,
    0,  273,  273,  273,  273,  273,  273,  273,
  273,    0,  273,  273,    0,  273,  273,  273,
  820,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,  820,    0,  303,    0,    0,    0,    0,
  456,  638,  820,  299,    0,    0,    0,    0,
    0,  729,    0,    0,    0,  228,    0,    0,
  182,  501,  774,  501,    0,    0,    0,    0 },
/* font 1 is Helvetica */
  {
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
  278,  278,  355,  556,  556,  889,  667,  222,
  333,  333,  389,  584,  278,  333,  278,  278,
  556,  556,  556,  556,  556,  556,  556,  556,
  556,  556,  278,  278,  584,  584,  584,  556,
 1015,  667,  667,  722,  722,  667,  611,  778,
  722,  278,  500,  667,  556,  833,  722,  778,
  667,  778,  722,  667,  611,  722,  667,  944,
  667,  667,  611,  278,  278,  278,  469,  556,
  222,  556,  556,  500,  556,  556,  278,  556,
  556,  222,  222,  500,  222,  833,  556,  556,
  556,  556,  333,  500,  278,  556,  500,  722,
  500,  500,  500,  334,  260,  334,  584,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,  333,  556,  556,  167,  556,  556,  556,
  556,  191,  333,  556,  333,  333,  500,  500,
    0,  556,  556,  556,  278,    0,  537,  350,
  222,  333,  333,  556, 1000, 1000,    0,  611,
    0,  333,  333,  333,  333,  333,  333,  333,
  333,    0,  333,  333,    0,  333,  333,  333,
 1000,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0, 1000,    0,  370,    0,    0,    0,    0,
  556,  778, 1000,  365,    0,    0,    0,    0,
    0,  889,    0,    0,    0,  278,    0,    0,
  222,  611,  944,  611,    0,    0,    0,    0 },
/* font 2 is Helvetica-Bold */
  {
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
  278,  333,  474,  556,  556,  889,  722,  278,
  333,  333,  389,  584,  278,  333,  278,  278,
  556,  556,  556,  556,  556,  556,  556,  556,
  556,  556,  333,  333,  584,  584,  584,  611,
  975,  722,  722,  722,  722,  667,  611,  778,
  722,  278,  556,  722,  611,  833,  722,  778,
  667,  778,  722,  667,  611,  722,  667,  944,
  667,  667,  611,  333,  278,  333,  584,  556,
  278,  556,  611,  556,  611,  556,  333,  611,
  611,  278,  278,  556,  278,  889,  611,  611,
  611,  611,  389,  556,  333,  611,  556,  778,
  556,  556,  500,  389,  280,  389,  584,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0,  333,  556,  556,  167,  556,  556,  556,
  556,  238,  500,  556,  333,  333,  611,  611,
    0,  556,  556,  556,  278,    0,  556,  350,
  278,  500,  500,  556, 1000, 1000,    0,  611,
    0,  333,  333,  333,  333,  333,  333,  333,
  333,    0,  333,  333,    0,  333,  333,  333,
 1000,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,
    0, 1000,    0,  370,    0,    0,    0,    0,
  611,  778, 1000,  365,    0,    0,    0,    0,
    0,  889,    0,    0,    0,  278,    0,    0,
  278,  611,  944,  611,    0,    0,    0,    0 }
};

typedef struct a_ps_image_family {
  char *name;
  ImageFamily *imf;
  Image *img;
  char *hexmonodata;
  char *hexmaskdata;
} PsImage;

static PsImage *unit_icon, *terr_icon, *side_icon;

static int conn_x[NUMDIRS], conn_y[NUMDIRS],
           hexc_x[NUMDIRS+1], hexc_y[NUMDIRS+1];

/* prototypes */

void ps_cook_imf PROTO ((void));
void ps_initialize_imf PROTO ((PsImage *psim));
char *ps_hex_dump PROTO ((Obj *data, int w, int h));
char *img_untile PROTO ((char *tiled, int w, int h, int istile,
			 int dither, int scale, int gray));
void misc_init PROTO ((void));
void ps_initialize PROTO ((FILE *fp));
void page_init PROTO ((FILE *fp, int page, int i, int j, char *stime));
int nearest_valid_x PROTO((int x, int y));
int seen_terrain_at PROTO ((int x, int y, Side *side));
Unit *seen_unit_at PROTO ((int x, int y, Side *side));
int seen_utype_at PROTO ((int x, int y, Side *side));
int sideno_of_seen_unit_at PROTO ((int x, int y, Side *side));
char *name_of_seen_unit_at PROTO ((int x, int y, Side *side));
char *summary_of_seen_units_at PROTO ((int x, int y, Side *side));
int ps_string_width PROTO ((char *str, int font));
void place_feature_legends
  PROTO ((Legend *leg, int nf, Side *side, int orient, int block));
int blocking_utype PROTO ((int u, int block));
int num_features PROTO ((void));
int print_unit_legends PROTO ((FILE *fp, char *name, char *summary, char *m,
	int dir, int cx, int cy));

void
ps_cook_imf()
{
    int i;
    Side *s;
    Image *img;
    
    if (buffer == NULL)
      buffer = xmalloc(PSBUFSIZE + 4);
    if (unit_icon == NULL)
      unit_icon = (PsImage *)
	xmalloc (numutypes * sizeof(PsImage));
    if (terr_icon == NULL)
      terr_icon = (PsImage *)
	xmalloc (numttypes * sizeof(PsImage));
    if (side_icon == NULL)
      side_icon = (PsImage *)
	xmalloc (MAXSIDES  * sizeof(PsImage));
    
    /* pick the images we need */
    for_all_unit_types(i) {
	unit_icon[i].imf = NULL;
	/* first choice */
	if (utypes[i].imagename) {
	    unit_icon[i].name = utypes[i].imagename;
	    unit_icon[i].imf = find_imf(utypes[i].imagename);
	}
	/* second choice */
	if (!unit_icon[i].imf && utypes[i].name) {
	    unit_icon[i].name = utypes[i].name;
	    unit_icon[i].imf = find_imf(utypes[i].name);
	}

	if (!unit_icon[i].imf || !unit_icon[i].imf->images) {
	    unit_icon[i].img = NULL;
	} else {
	    unit_icon[i].img = unit_icon[i].imf->images;
	    for (img=unit_icon[i].imf->images->next; img; img=img->next) {
		/* should we prefer the new image? */
		if ((!unit_icon[i].img->monodata && img->monodata) ||
		    (!unit_icon[i].img->maskdata && img->maskdata) ||
		    ( unit_icon[i].img->w < img->w)) {
		    unit_icon[i].img = img;
		}
	    }
	}
	ps_initialize_imf(&unit_icon[i]);
    }
    
    for_all_terrain_types(i) {
	terr_icon[i].imf = NULL;
	/* first choice */
	if (ttypes[i].imagename) {
	    terr_icon[i].name = ttypes[i].imagename;
	    terr_icon[i].imf = find_imf(ttypes[i].imagename);
	}
	/* second choice */
	if (!terr_icon[i].imf && ttypes[i].name) {
	    terr_icon[i].name = ttypes[i].name;
	    terr_icon[i].imf = find_imf(ttypes[i].name);
	}
	if (terr_icon[i].imf) {
	    terr_icon[i].img = terr_icon[i].imf->images;
	} else {
	    terr_icon[i].img = NULL;
	}
	ps_initialize_imf(&terr_icon[i]);
    }
    
    i = -1;
    for_all_sides_plus_indep(s) {
	i++;
	side_icon[i].imf = NULL;
	/* first choice */
	if (s->emblemname) {
	    side_icon[i].name = s->emblemname;
	    side_icon[i].imf = find_imf(s->emblemname);
	}
	if (side_icon[i].imf) {
	    side_icon[i].img = side_icon[i].imf->images;
	} else {
	    side_icon[i].img = NULL;
	}
	ps_initialize_imf(&side_icon[i]);
    }
    
    img_cooked = 1;
}

void 
ps_initialize_imf(psim)
PsImage *psim;
{
    /* checks & cleanup */
    if (!psim)
      return;
    if (!psim->imf)
      psim->img = NULL;

    if (!psim->img) {
	psim->hexmonodata = NULL;
	psim->hexmaskdata = NULL;
	return;
    }
    
    psim->hexmonodata =
      ps_hex_dump(psim->img->monodata, psim->img->w, psim->img->h);
    psim->hexmaskdata =
      ps_hex_dump(psim->img->maskdata, psim->img->w, psim->img->h);
}

char *
ps_hex_dump(data, w, h)
Obj *data;
int w, h;
{
    int i, numbytes;
    unsigned int u;
    char *hexdata, *c;
  
    numbytes = h * ((w + 7) / 8);
    interp_bytes(data, numbytes, buffer, 0);
    c = hexdata = xmalloc(2*numbytes+2);
    for (i = 0; i < numbytes; ++i) {
	u = buffer[i] & 0xff;
	sprintf(c, "%2.2x", u);
	c += 2;
    }
    *c = '\0';
    return hexdata;
}

/* Should use two symbolic constants instead of 8 ? */

char *
img_untile(tiled, w, h, istile, dither, scale, gray)
char *tiled;
int w, h, istile, dither, scale, gray;
{
    int dx, dy, x, y, xo, yo, xi, yi, tw, th;
    unsigned int u;
    char mask[8][8], str[12], *b;

    if (tiled == NULL)
      return zero;
    if (!istile)
      return tiled;

    if (strlen(tiled) != area_leng(8,8))
      return zero;
  
    /* untile image into hexagon */

    /* width and height of the four triangles we have to cut off
       a hex_w*hex_h rectangle to form a hexagon */
    tw = hex_w / 2;
    th = hex_h - hex_c;

    /* convert to binary */
    b = tiled;
    for (y = 0; y < 8; y++) {
	for (x = 0; x < 8; x++) {
	    if ((x & 0x3) == 0) {
		str[0] = *b++;
		str[1] = '\0';
		sscanf(str, "%x", &u);
	    }
	    mask[x][y] = (u & 0x8) >> 3;
	    u = u << 1;
	}
    }

    if (area_leng(scale * hex_w, scale * hex_h) > PSBUFSIZE) {
	notify(ps_side, "buffer size is %d, too small to load %dx%d image",
	       PSBUFSIZE, scale * hex_w, scale * hex_h);
	return zero;
    }

    /* Convert to hexadecimal. */
    b = buffer;
    for (yo = 0, y = 0; yo < hex_h; yo++) {
	dy = min(yo, hex_h - 1 - yo);
	for (yi = 0; yi < scale; yi++, y++) {
	    u = 0;
	    for (xo = 0, x = 0; xo < hex_w; xo++) {
		dx = min(xo, hex_w - 1 - xo);
		for (xi = 0; xi < scale; xi++, x++) {
		    u = u << 1;
		    /* is the center of the (xo,yo) cell inside the hexagon? */
		    if (th * dx + tw * dy >= th * tw - (th + tw) / 2) {
			u |= mask[x & 0x7][y & 0x7]
			  && xrandom(GRAY_SCALE) >= gray;
		    }
		    if ((x & 0x3) == 0x3 || x == scale * hex_w - 1) {
			sprintf(b++, "%1.1x", u);
			u = 0;
		    }
		}
	    }
	    /* does the hexadecimal string need padding? */
	    if (((scale * hex_w - 1) & 7) < 4) {
		*b++ = '0';
	    }
	}
    }

    *b = '\0';
    if (strlen(buffer) != area_leng(scale * hex_w, scale * hex_h)) {
	notify(ps_side, "length (%s) is %d, expecting %d",
	       buffer, strlen(buffer),
	       area_leng(scale * hex_w, scale * hex_h));
    }
    return buffer;
}

/* Convert to PostScript escape sequences:
   '\' to '\\', '(' to '\(', ')' to '\)'.  */

char *
add_esc_string(str)
char *str;
{
    char *ps, *pb;

    if (escbuf == NULL)
      escbuf = xmalloc(BUFSIZE);
    for (ps = str, pb = escbuf; *ps && pb < escbuf + BUFSIZE - 2; ps++, pb++) {
	if (*ps == '(' || *ps == ')' || *ps == '\\')
	  *pb++ = '\\';
	*pb = *ps;
    }
    *pb = '\0';
    return escbuf;
}


char *
pad_blanks(str, n)
char *str;
int n;
{
    char *pb;
    int i;
  
    if (padbuf == NULL)
      padbuf = xmalloc(BUFSIZE);
    pb = padbuf;
    while (*str && pb < padbuf + BUFSIZE - 2) {
	*pb++ = *str++;
	if (*str) {
	    for (i = 0; i < n; i++)
	      *pb++ = ' ';
	}
    }
    *pb = '\0';
    return padbuf;
}

/* set parameter defaults */
/* all length are in PostScript units (1/72 in)  */

void
init_ps_print(ipp)
PrintParameters *ipp;
{
    double in, cm;
    
    if (pp == NULL) {
	pp = (PrintParameters *) xmalloc(sizeof(PrintParameters));

	/* the following code must be executed once,
	   before calling the parameter selection front-end */
	in = 72.0;
	cm = in/2.54;

	/*  US, letter sheet  */
	pp->page_width =    8.5*in;
	pp->page_height =  11.0*in;
	pp->cm = 0;		/* use inches in front-end */

	/*  metric world, A4 sheet  */
	pp->page_width =   21.0*cm;
	pp->page_height =  29.7*cm;
	pp->cm = 1;		/* use centimeters in front-end */

	/*  title and coordinates are printed in the "margins":
	    make room in top_margin, left_margin and right_margin  */
	pp->top_margin =    3.0*cm;
	pp->bottom_margin = 2.0*cm;
	pp->left_margin =   2.0*cm;
	pp->right_margin =  2.0*cm;

	pp->cell_size        = 0.500*cm;
	pp->cell_grid_width  = 0.005*cm; /* 0 or negative to avoid hex borders */
	pp->border_width     = 0.050*cm;
	pp->connection_width = 0.025*cm;

	pp->features  =  1;	/* print geographical features */

	/*  names = 0:  don't print unit names;
	    names = 1:  filter unit names;
	    names = 2:  print all unit names.  */
	pp->names = 1;
	pp->cell_summary = 1;	/* print summary of units in cell */
	pp->corner_coord   = 1;	/* print coordinates of map corners */
	pp->terrain_gray   = 0.50; /* gray level for terrain;  0 = black */
	pp->enemy_gray     = 0.50; /* gray level for enemies;  0 = black */
	pp->terrain_dither = 0;	/* make terrain gray by dithering;
				   0 = use PostScript "setgray" operator */
	pp->terrain_double = 1;	/* make mesh finer when dithering */
    }
    
    if (ipp) {
	*ipp = *pp;
    }
}

/* initialization of variables */

void 
misc_init()
{
    if (pp->terrain_dither) {
	terr_gray = GRAY_SCALE*pp->terrain_gray + 0.5;
	terr_scale = (pp->terrain_double && terr_gray>0 &&
		      terr_gray<GRAY_SCALE) ? 2 : 1;
    } else {
	terr_gray = 0;
	terr_scale = 1;
    }
    ru = 1.0 / unit_w;
    rtw = 1.0 / hex_w;
    rth = 1.0 / hex_h;
    sqrt3 = 1.73205080756888;	/* sqrt(3.0);  hardwired to avoid need of -lm */
    asym = hex_c / (0.5 * sqrt3 * hex_w);
    
    x0 = area.xwrap ? 0 : (area.height+2)/4;
    
    /* from a hex center to neighbor hex centers */
    conn_x[0] =  hex_w/2;		conn_y[0] =  hex_c;
    conn_x[1] =  hex_w;		conn_y[1] =  0;
    conn_x[2] =  hex_w/2;		conn_y[2] = -hex_c;
    conn_x[3] = -hex_w/2;		conn_y[3] = -hex_c;
    conn_x[4] = -hex_w;		conn_y[4] =  0;
    conn_x[5] = -hex_w/2;		conn_y[5] =  hex_c;
    
    /* from a hex center to hex vertices (with wraparound) */
    hexc_x[0] =  0;		hexc_y[0] =  hex_h/2;
    hexc_x[1] =  hex_w/2;		hexc_y[1] =  hex_h/4;
    hexc_x[2] =  hex_w/2;		hexc_y[2] = -hex_h/4;
    hexc_x[3] =  0;		hexc_y[3] = -hex_h/2;
    hexc_x[4] = -hex_w/2;		hexc_y[4] = -hex_h/4;
    hexc_x[5] = -hex_w/2;		hexc_y[5] =  hex_h/4;
    hexc_x[6] =  0;		hexc_y[6] =  hex_h/2;
    
    cell_grid_width  = pp->cell_grid_width  * hex_w/pp->cell_size;
    border_width     = pp->border_width     * hex_w/pp->cell_size;
    connection_width = pp->connection_width * hex_w/pp->cell_size;
    
    /* rows per page and cells per row */
    height = (pp->page_height - pp->top_margin - pp->bottom_margin)
      * 2 / (sqrt3 * pp->cell_size);
    width  = (pp->page_width - pp->left_margin - pp->right_margin)
      / pp->cell_size;
}

/* write prolog and macro definitions to view file */
void
ps_initialize(fp)
FILE *fp;
{
    int i;
    Image *img;
    Side *s;
    
    /*  PostScript prolog  */
    fprintf(fp, "%%!PS-Adobe-2.0\n");
    fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n",
	    (int) pp->left_margin,
	    (int) pp->top_margin,
	    (int) (pp->page_width - pp->left_margin),
	    (int) (pp->page_height - pp->top_margin));
    fprintf(fp, "%%%%Creator: Xconq version %s\n", version_string());
    
#ifdef UNIX
    {
	char *userp, *hostp;

	userp = getenv("USER");
	hostp = getenv("HOST");
	if (userp != 0) {
	    if (hostp != 0) {
		fprintf(fp, "%%%%For: %s@%s\n", userp, hostp);
	    } else {
		fprintf(fp, "%%%%For: %s\n", userp);
	    }
	}
    }
#endif /* UNIX */
#ifdef MAC
    /* ??? */
#endif /* MAC */
    
    fprintf(fp, "%%%%DocumentFonts:\
 Helvetica Helvetica-Narrow Helvetica-Bold Times-Bold\n");
    fprintf(fp, "%%%%Pages: (atend)\n");
    fprintf(fp, "%%%%EndComments\n");
    
    /* PostScript definitions */
    
    /* new line */
    fprintf(fp, "/nl { 0 %d translate } bind def\n", -hex_c);
    /* backspace */
    fprintf(fp, "/bs { %d 0 translate } bind def\n", -hex_w);
    /* space */
    fprintf(fp, "/sp { %d 0 translate } bind def\n", hex_w);
    /* multiple spaces */
    fprintf(fp, "/msp { %d mul 0 translate } bind def\n", hex_w);
    /* half space */
    fprintf(fp, "/hs { %d 0 translate } bind def\n", hex_w / 2);
    /* show string right of current position */
    fprintf(fp, "/rshow { show } bind def\n");
    /* show string centered on current position */
    fprintf(fp, "/cshow { dup stringwidth pop -0.5 mul 0 rmoveto");
    fprintf(fp, " show } bind def\n");
    /* show string left of current position, correcting asymmetry */
    fprintf(fp, "/lshow { dup stringwidth pop neg 0 rmoveto");
    fprintf(fp, " show } bind def\n");
    /* draw hexagon (if requested) */
    if (cell_grid_width > 0) {
	fprintf(fp, "/hex { currentgray 0 setgray 0 %d moveto %d 0 lineto %d %d",
		hex_c-hex_h, hex_w/2, hex_w,hex_c-hex_h);
	fprintf(fp, " lineto %d %d lineto %d %d lineto 0 %d lineto 0 %d lineto",
		hex_w,-hex_c, hex_w/2,-hex_h, -hex_c, hex_c-hex_h);
	fprintf(fp, " stroke setgray } bind def\n");
    } else {
	fprintf(fp, "/hex { } bind def\n");
    }
    /* draw a line */
    fprintf(fp, "/li { moveto lineto stroke } bind def\n");
    
    /* print unit name below unit icon */
    fprintf(fp, "/n { %d %d moveto sbc } bind def\n", hex_w/2, 1-hex_h);
    
    /* print left-justified in upper section of hex  */
    fprintf(fp, "/nlu { 0 %d moveto sbl } bind def\n", -hex_h/2);
    /* print left-justified in lower section of hex  */
    fprintf(fp, "/nld { 0 %d moveto sbl } bind def\n", -(hex_h+hex_c)/2);
    /* print right-justified in upper section of hex  */
    fprintf(fp, "/nru { %d %d moveto sbr } bind def\n", hex_w, -hex_h/2);
    /* print right-justified in lower corner of hex  */
    fprintf(fp, "/nrd { %d %d moveto sbr } bind def\n",
	    hex_w, -(hex_h+hex_c)/2);
    
    /* print a centered string, blanking out a bounding rectangle */
    fprintf(fp, "/sbc { dup stringwidth gsave ");
    fprintf(fp, "0 -0.5 rmoveto pop dup 0.5 add 7 scale -0.5 0 rmoveto ");
    fprintf(fp,
	    "1 0 rlineto 0 1 rlineto -1 0 rlineto 0 -1 rlineto 1 setgray fill ");
    fprintf(fp, "grestore -0.5 mul 0.5 rmoveto show } bind def\n");
    /* print a left-aligned string, blanking out a bounding rectangle */
    fprintf(fp, "/sbl { dup stringwidth gsave ");
    fprintf(fp, "0 -0.5 rmoveto pop 0.5 add 7 scale ");
    fprintf(fp,
	    "1 0 rlineto 0 1 rlineto -1 0 rlineto 0 -1 rlineto 1 setgray fill ");
    fprintf(fp, "grestore 0 0.5 rmoveto show } bind def\n");
    /* print a right-aligned string, blanking out a bounding rectangle */
    fprintf(fp, "/sbr { dup stringwidth gsave ");
    fprintf(fp, "0 -0.5 rmoveto pop dup 0.5 add 7 scale -1 0 rmoveto ");
    fprintf(fp,
	    "1 0 rlineto 0 1 rlineto -1 0 rlineto 0 -1 rlineto 1 setgray fill ");
    fprintf(fp, "grestore neg 0.5 rmoveto show } bind def\n");
    
    /* draw a unit */
    for_all_unit_types(i) {
	if (!unit_icon[i].hexmonodata) {
	    fprintf(fp, "%% icon for unit type %s not found\n",
		    utypes[i].name);
	    fprintf(fp, "/u%X { } def\n", i);
	} else {
	    img = unit_icon[i].img;
	    fprintf(fp, "%% unit type %s\n", utypes[i].name);
	    /* center unit icon into hex */
	    fprintf(fp, "/u%X { gsave %d %d translate",
		    i, (hex_w-unit_w)/2, -(hex_h-unit_w)/2-3);
	    if (unit_icon[i].hexmaskdata) {
		fprintf(fp, " 1 setgray %d %d true [ %g 0 0 %g 0 0 ]",
			img->w, img->h, img->w*ru, -img->h*ru);
		fprintf(fp, " {<%s>} imagemask",
			unit_icon[i].hexmaskdata);
	    }
	    fprintf(fp, " 0 setgray %d %d true [ %g 0 0 %g 0 0 ]",
		    img->w, img->h, img->w*ru, -img->h*ru);
	    fprintf(fp, " {<%s>} imagemask grestore } def\n",
		    unit_icon[i].hexmonodata);
	}
    }
    
    /* draw a terrain call and a hex */
    for_all_terrain_types(i) {
	if (!terr_icon[i].hexmonodata) {
	    fprintf(fp, "%% icon for terrain type %s not found\n",
		    ttypes[i].name);
	    fprintf(fp, "/t%X { hex sp } def\n", i);
	} else {
	    int w, h;

	    img = terr_icon[i].img;
	    w = img->w;
	    h = img->h;
	    if (img->istile && w==8 && h==8) {
		w = terr_scale*hex_w;  h = terr_scale*hex_h;
	    }
	    fprintf(fp, "%% terrain type %s\n", ttypes[i].name);
	    fprintf(fp, "/t%X { %d %d true [ %g 0 0 %g 0 0 ]",
		    i, w, h, w*rtw, -h*rth);
	    fprintf(fp, " {<%s>} imagemask hex sp } def\n",
		    img_untile(terr_icon[i].hexmonodata, w, h, img->istile,
			       pp->terrain_dither, terr_scale, terr_gray));
	}
    }
    
    /* draw an emblem */
    i = 0;
    for_all_sides_plus_indep(s) {
	if (!side_icon[i].hexmonodata) {
	    fprintf(fp, "%% icon for side %s not found\n", side_name(s));
	    fprintf(fp, "/s%X { } def\n", side_number(s));
	} else {
	    img = side_icon[i].img;
	    fprintf(fp, "%% side %s\n", side_name(s));
	    /* place side icon into upper corner of hex */
	    fprintf(fp, "/s%X { gsave %g %g translate",
		    side_number(s), 0.5 * (hex_w - img->w), -4 / asym);
	    if (side_icon[i].hexmaskdata) {
		fprintf(fp, " 1 setgray %d %d true [ 1 0 0 -1 0 0 ]", img->w, img->h);
		fprintf(fp, " {<%s>} imagemask",
			side_icon[i].hexmaskdata);
	    }
	    fprintf(fp, " 0 setgray %d %d true [ 1 0 0 -1 0 0 ]", img->w, img->h);
	    fprintf(fp, " {<%s>} imagemask grestore } def\n",
		    side_icon[i].hexmonodata);
	}
	i++;
    }
    
    fprintf(fp, "%%%%EndProlog\n");
    fprintf(fp, "\n");
}

/*  initialize page  */

void 
page_init(fp, page, i, j, stime)
FILE *fp;
int page, i, j;
char *stime;
{
    int x, xx, y, y0, nx, ny;
    char *gametitle;
    
    fprintf(fp, "%%%%Page: %d %d\n", page, page);
    fprintf(fp, "%g %g translate ",
	    pp->left_margin, pp->page_height - pp->top_margin);
    fprintf(fp, "%g %g scale\n", pp->cell_size / hex_w, pp->cell_size / hex_w);
    
    /* print page title */
    gametitle = NULL;
    if (mainmodule != NULL) {
	if (mainmodule->title != NULL)
	  gametitle = mainmodule->title;
	else if (mainmodule->name != NULL)
	  gametitle = mainmodule->name;
    }
    
    /* "20" shouldn't be wired in... */
    fprintf(fp, "/Times-Bold findfont 16 scalefont setfont\n");
    
    fprintf(fp, "%g 20 moveto (%s) rshow\n", 0.05 * width * hex_w,
	    add_esc_string(stime));
    
    fprintf(fp, "%g 20 moveto (Xconq", 0.5 * width * hex_w);
    if (gametitle != NULL) {
	fprintf(fp, ":  %s", add_esc_string(gametitle));
    }
    fprintf(fp, "  \\(%s\\)) cshow\n",
	    add_esc_string(absolute_date_string(g_turn())));
    
    fprintf(fp, "%g 20 moveto (Page %d) lshow\n", 0.95 * width * hex_w, page);
    
    /*  print map corner coordinates? */
    if (pp->corner_coord) {
	fprintf(fp, "/Helvetica findfont 12 scalefont setfont\n");
	
	y0 = y = area.height - 1 - i * height;
	x = j * width - (y+1) / 2 + x0;
	x = wrapx(x);
	xx = nearest_valid_x(x, y) - x;
	x += xx;
	fprintf(fp, "%d %g moveto (%d,%d) lshow\n",
		(2 * xx - 1) * hex_w / 2, 0 / asym, x, y);
	
	nx = min(width, area.width-j*width);
	x = j * width + nx - 1 - (y + 1) / 2 + x0;
	x = wrapx(x);
	xx = nearest_valid_x(x, y) - x;
	x += xx;
	fprintf(fp, "%d %g moveto (%d,%d) rshow\n",
		(2 * (nx + xx) + 1) * hex_w / 2, 0 / asym, x, y);
	
	y = max(area.height - (i + 1) * height, 0);
	ny = y0 - y;
	x = j * width - (y + 1) / 2 + x0;
	x = wrapx(x);
	xx = nearest_valid_x(x, y) - x;
	x += xx;
	fprintf(fp, "%d %g moveto (%d,%d) lshow\n",
		(2 * xx - 1) * hex_w / 2, -(hex_c * (ny + 2) + 3) / asym,
		x, y);
	
	x = j * width + nx - 1 - (y + 1) / 2 + x0;
	x = wrapx(x);
	xx = nearest_valid_x(x, y) - x;
	x += xx;
	fprintf(fp, "%d %g moveto (%d,%d) rshow\n",
		(2 * (nx + xx) + 1) * hex_w / 2,
		-(hex_c * (ny + 2) +3 ) / asym, x, y);
    }
}

int
nearest_valid_x(x, y)
int x, y;
{
    int xlo, xhi;

    if (area.xwrap)
      return x;
    xlo = max(0, area.height / 2 - y);
    xhi = min(area.width-1, area.width + area.height / 2 - 1 - y);
    x = max(x, xlo);
    x = min(x, xhi);
    return x;
}

int
seen_terrain_at(x, y, side)
int x, y;
Side *side;
{
    if (in_area(x, y)
	&& (!side || g_see_all() || side->designer ||
	    terrain_view(side, x, y) != UNSEEN)) {
	return terrain_at(x, y);
    } else {
	return NONTTYPE;
    }
}

Unit *
seen_unit_at(x,y,side)
int x, y;
Side *side;
{
    if (!in_area(x, y))
      return NULL;
    if (!side || g_see_all() || side->designer || cover(side, x, y) > 0)
      return unit_at(x, y);
    return NULL;
}

int
seen_utype_at(x,y,side)
int x, y;
Side *side;
{
    Unit *unit;
    short view;

    if (!in_area(x, y))
      return NONUTYPE;
    unit = seen_unit_at(x, y, side);
    if (unit)
      return unit->type;

    view = unit_view(side, x, y);
    if (view != EMPTY)
      return vtype(view);

    return NONUTYPE;
}

int
sideno_of_seen_unit_at(x,y,side)
int x, y;
Side *side;
{
   Unit *unit;
   short view;

   if (!in_area(x, y))
     return indepside->id;
   unit = seen_unit_at(x,y,side);
   if (unit)
     return side_number(unit->side);
   view = unit_view(side, x, y);
   if (view != EMPTY)
     return vside(view);
   return indepside->id;
}

char *
name_of_seen_unit_at(x,y,side)
int x, y;
Side *side;
{
    Unit *unit;
    
    if (!pp->names || !in_area(x, y))
      return NULL;
    unit = seen_unit_at(x,y,side);
    if (unit) {
	if (pp->names > 1) {
	    name_or_number(unit, buffer);
	    if (buffer[0]) {
		return add_esc_string(buffer);
	    }
	} else {
	    if (unit->name && unit->name[0]) {
		return add_esc_string(unit->name);
	    } 
	}
    } 
    return NULL;
}

char *
summary_of_seen_units_at(x,y,side)
int x, y;
Side *side;
{
    Unit *mainunit, *other, *occ;
    int i, nums[MAXUTYPES];
    
    if (!in_area(x, y) || !pp->cell_summary)
      return NULL;
    mainunit = seen_unit_at(x, y, side);
    if (!mainunit || (!mainunit->nexthere && !mainunit->occupant))
      return NULL;
    
    for_all_unit_types(i) {
	nums[i] = 0;
    }
    for_all_stack(x, y, other) {
	nums[other->type]++;
	for_all_occupants(other, occ) {
	    nums[occ->type]++;
	}
    }
    nums[mainunit->type]--;
    
    strcpy(buffer, "(+");
    for_all_unit_types(i) {
	if (nums[i] != 0) {
	    tprintf(buffer, " %d %1s,", nums[i], utype_name_n(i, 1));
	}
    }
    i = strlen(buffer);
    buffer[i-1] = ')';
    
    return add_esc_string(buffer);
}

/* This routine could still use a cleanup */

void
dump_ps_view(side, ipp, filename)
Side *side;
PrintParameters *ipp;
char *filename;
{
    int x, y, xx, i, j, t, nx, utype, page, d, cx, cy, skip, nsp;
    int f, nf, tw, th, font, named_unit_max=100, xmin, ymin, xmin1;
    int l, l1, x1, y1, x2, y2, d1, mvalue, nvalue, pad;
    FILE *fp;
    char stime[42], *name, *summary, tmpbuf[BUFSIZE];
    char *name_layer, *name_position;
    time_t tnow;
    Legend *legs = NULL, *lp;
    double length, magnif;
    
    if (ipp) {
	*pp = *ipp;
    }
    
    ps_side = (side) ? side : sidelist->next;
    misc_init();
    
    /* does this work on the MAC too? (it's supposed to be ANSI) */
    time(&tnow);
    strftime(stime, 40, "%d %b %Y", localtime(&tnow));
    
    /* compute position of legends of geographical features */
    if (pp->features) {
	nf = num_features();
	if (nf > 0) {
	    legs = (Legend *) malloc((nf + 1) * sizeof(Legend));
	    if (legs)
	      place_feature_legends(legs, nf, ps_side, 2, 1);
	}
    }
    
    /* need to initialize and cook the image library? */
    if (img_cooked == 0) {
	init_ps_print(NULL);
	ps_cook_imf();
    }
    if (img_cooked!=1) {
	notify(ps_side, "image cooking failed, dump skipped");
	return;
    }
    
    /*  open view file  */
    fp = fopen(filename, "w");
    if (fp == NULL) {
	notify(ps_side, "Cannot open \"%s\", dump skipped", filename);
	return;
    }
    
    ps_initialize(fp);
    
    name_layer = (char *) malloc(((height+2) * (width+2) + 2) * sizeof(char));
    name_position = (char *) malloc((named_unit_max + 2) * sizeof(char));
    
    /* Loop over pages. */
    page = 0;
    for (i = 0; i < (area.height + height - 1) / height; i++) {
	for (j = 0; j < (area.width + width - 1) / width; j++) {
	    xmin = j * width;  ymin = max(area.height - (i+1) * height, 0);

	    /* look if the page can be skipped  */

	    skip = 1;
	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		nx = min(width, area.width - xmin);

		for (xx = xmin; xx < xmin + nx; xx++) {
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);

		    skip &= seen_terrain_at(x, y, side) == NONTTYPE &&
		      seen_utype_at(x, y, side) == NONUTYPE;
		}
	    }
	    if (skip)
	      continue;

	    page++;
	    page_init(fp, page, i, j, stime);

	    /* show terrain, asymmetric scale to make perfectly
	       regular hexagons  */

	    fprintf(fp, "%g setlinewidth gsave 1 %g scale %g setgray\n",
		    cell_grid_width, 1 / asym,
		    (pp->terrain_dither ? 0.0 : pp->terrain_gray));

	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		fprintf(fp, "gsave ");
		if ((y % 2) == 0)
		  fprintf(fp, "hs ");
		nx = min(width, area.width - xmin);
		nsp = 0;
		for (xx = xmin; xx < xmin + nx; xx++) {
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);

		    if ((t = seen_terrain_at(x, y, side)) != NONTTYPE) {
			if (nsp > 1) {
			    fprintf(fp, "%d msp ", nsp);
			} else if (nsp == 1) {
			    fprintf(fp, "sp ");
			}
			nsp = 0;
			fprintf(fp, "t%X ", t);
		    } else {
			nsp++;
		    }
		}
		fprintf(fp, "grestore nl\n");
	    }
	    fprintf(fp, "grestore\n");

	    /* draw borders.  (cx,cy) is the hex center */

	    fprintf(fp, "gsave %g setlinewidth 1 setlinecap\n", border_width);

	    cy = -hex_h/2 + hex_c;
	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		cy -= hex_c;

		cx = (1 + ((y%2)==0))*hex_w/2 - hex_w;
		nx = min(width, area.width-xmin);
		for (xx = xmin; xx < xmin + nx; xx++) {
		    cx += hex_w;
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);
		    if (!in_area(x, y))
		      continue;

		    for_all_terrain_types(t) {
			if (t_is_border(t) && aux_terrain_defined(t)) {
			    for_all_directions(d) {
				if (border_at(x, y, d, t) &&
				    (!side
				     || g_see_all()
				     || seen_border(side, x, y, d))) {
				    fprintf(fp, "%d %g %d %g li\n",
					    cx + hexc_x[d],
					    (cy + hexc_y[d]) / asym,
					    cx + hexc_x[d + 1],
					    (cy + hexc_y[d + 1]) / asym);
				}
			    }
			}
		    }
		}
	    }
	    fprintf(fp, "grestore\n");

	    /* draw connections.  (cx,cy) is the hex center */

	    fprintf(fp, "gsave %g setlinewidth 1 setlinecap\n",
		    connection_width);

	    cy = -hex_h/2 + hex_c;
	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		cy -= hex_c;
		cx = (1 + ((y % 2) == 0)) * hex_w / 2 - hex_w;
		nx = min(width, area.width - xmin);
		for (xx = xmin; xx < xmin + nx; xx++) {
		    cx += hex_w;
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);
		    if (!in_area(x, y))
		      continue;
		    for_all_terrain_types(t) {
			if (t_is_connection(t) && aux_terrain_defined(t)) {
			    for_all_directions(d) {
				if (connection_at(x, y, d, t) &&
				    (!side || g_see_all() ||
				     terrain_view(side, x, y) != UNSEEN)) {
				    fprintf(fp, "%d %g %d %g li\n",
					    cx,
					    cy / asym,
					    cx + conn_x[d],
					    (cy + conn_y[d]) / asym);
				}
			    }
			}
		    }
		}
	    }
	    fprintf(fp, "grestore\n");

	    /* print feature names.  (cx,cy) is the legend center */

	    if (pp->features && legs) {
		fprintf(fp, "gsave\n");
		th = -hex_c * (area.height - i * height - 1
			       - max(area.height - (i+1) * height,0))  - hex_h;
		tw = hex_w * min(width, area.width - xmin) + hex_w / 2;
		fprintf(fp, "newpath 0 0 moveto 0 %g lineto %d %g lineto ",
			th / asym, tw, th / asym);
		fprintf(fp, "%d 0 lineto 0 0 lineto closepath clip\n", tw);
		
		for (f = 1, lp = legs; f <= nf; f++, lp++) {
		    name = feature_desc(find_feature(f), buffer);
		    if (lp->dist < 0.0 || !name)
		      continue;
		    xx = wrap(lp->ox + (lp->oy + 1) / 2 - x0) - xmin;
		    if (xx >= nx) {
			xx -= area.width;
		    }
		    cx = (1 + ((lp->oy%2)==0))*hex_w/2 +
		      (2*lp->dx+lp->dy)*hex_w/4 + xx*hex_w;
		    cy = -hex_h/2 - hex_c*(area.height-1-i*height - lp->oy) +
		      lp->dy*hex_c/2;

		    length = hex_w*(lp->dist + 1);
		    magnif = (1000 * length) / ps_string_width(name, 2);

		    /* tweak these parameters: */
		    magnif *= 0.9;
		    if (magnif < 6) {
			magnif = 8;
			font = 0;
		    } else if (magnif < 8) {
			magnif = 8;
			font = 1;
		    } else if (magnif < 20) {
			font = 2;
		    } else {
			/* keep magnif near 20 */
			font = 2;
			pad = ps_string_width(name, 2)
			  * (magnif / 20.0 - 1.0)
			    / (strlen(name) * ps_string_width(" ", 2)) + 0.5;
			name = pad_blanks(name, pad);
			magnif =
			  (1000 * length) / ps_string_width(name, 2) * 0.9;
		    }

		    /* comment containing debugging info */
		    fprintf(fp, "%% f = %d, ox = %d, oy = %d, dx = %d, dy = %d, ",
			    f, lp->ox, lp->oy, lp->dx, lp->dy);
		    fprintf(fp, "angle = %g, dist = %g\n",
			    lp->angle, lp->dist);
		    fprintf(fp, "/%s findfont 1 scalefont setfont ",
			    ps_name[font]);
		    fprintf(fp, "%d %g moveto gsave %g rotate %g %g scale ",
			    cx, cy/asym, lp->angle, magnif, magnif);
		    fprintf(fp, "0 %g rmoveto (%s) cshow grestore\n",
			    -0.0005*ps_height, add_esc_string(name));
		}
		fprintf(fp, "grestore\n");
	    }

	    /* look for name/summary places */
	    if (name_layer && name_position) {
		for (l = 0; l < (height + 2) * (width + 2); l++) {
		    name_layer[l] = '\0';
		}

		l = 0;
		for (y = area.height - 1 - i * height; y >= ymin; y--) {
	
		    nx = min(width, area.width - xmin);
		    xmin1 = xmin - (y + 1) / 2 + x0;
		    for (xx = xmin; xx < xmin + nx; xx++) {
			x = xx - (y + 1) / 2 + x0;

			/* we don't need the string here, so can overwrite */
			name = name_of_seen_unit_at(wrapx(x), y, side);
			summary = summary_of_seen_units_at(wrapx(x), y, side);

			if (!name && !summary)
			  continue;
			l++;
			if (l > named_unit_max) {
			    named_unit_max *= 2;
			    name_position = (char *)
			      realloc(name_position,
				      (named_unit_max + 2) * sizeof(char));
			}
			name_position[l] = '\177';

			/* choose the free nearby hex with less named units around */
			nvalue = 99;
			for (l1 = 0; l1 < NUMDIRS; l1++) {
			    d = name_dir[l1];
			    point_in_dir(x, y, d, &x1, &y1);
			    if ((seen_unit_at(wrapx(x1), y1, side)
				 && on_page(x1 - xmin1, y1 - ymin))
				|| (name_at(x1 - xmin1, y1 - ymin) & N_BK))
			      continue;
			    mvalue = 0;
			    for_all_directions(d1) {
				point_in_dir(x1, y1, d1, &x2, &y2);
				if ((name_of_seen_unit_at(wrapx(x2), y2, side)
				     || summary_of_seen_units_at(wrapx(x2),
								 y2, side))
				    && on_page(x2 - xmin1, y2 - ymin))
				  mvalue++;
			    }
			    if (mvalue<nvalue) {
				name_position[l] = d;
				nvalue = mvalue;
			    }
			}
			
			d = name_position[l];
			if (d < NUMDIRS && d >= 0) {
			    point_in_dir(x, y, d, &x1, &y1);
			    name_at(x1 - xmin1, y1 - ymin) |= N_BK;
			    /* comment containing debugging info */
			    fprintf(fp, "%% found pos for %s in (%d,%d) at (%d,%d), %s\n",
				    name_of_seen_unit_at(wrapx(x),y,side), x,y,
				    x1, y1, dirnames[d]);
			}
		    }
		}
		
		/* print unit names.  (cx,cy) is the upper-left corner
		   of the rectangle containing the hex */

		fprintf(fp, "gsave\n");
		fprintf(fp, "/%s findfont 8 scalefont setfont\n", ps_name[1]);

		l = 0;
		cy = hex_c;
		for (y = area.height - 1 - i * height; y >= ymin; y--) {
		    cy -= hex_c;
		    
		    cx = ((y % 2) == 0) * hex_w / 2 - hex_w;
		    nx = min(width, area.width-xmin);
		    xmin1 = xmin - (y+1)/2 + x0;
		    for (xx = xmin; xx < xmin+nx; xx++) {
			cx += hex_w;
			x = xx - (y+1)/2 + x0;

			name = name_of_seen_unit_at(wrapx(x),y,side);
			/* free buffer */
			if (name) {
			    strcpy(tmpbuf,name);
			    name = tmpbuf;
			}
			summary = summary_of_seen_units_at(wrapx(x),y,side);

			if (!name && !summary)
			  continue;
			l++;

			if (name_position[l]=='\177')
			  continue;
			/* Good position found, otherwise delay until
			   second round. */
			d = name_position[l];
			if (d < 0 || d >= NUMDIRS) {
			    name_position[l] = '\177';
			    continue;
			}
			point_in_dir(x, y, d, &x1, &y1);
			/* comment containing debugging info */
			fprintf(fp, 
				"%% calling print_unit_legends(fp, \"%s\", \"%s\", \\x%2.2x, %s)\n",
				name ? name : "", summary ? summary : "",
				name_at(x1-xmin1,y1-ymin),
				dirnames[(int) name_position[l]]);
			if (!print_unit_legends(fp, name, summary,
						&(name_at(x1 - xmin1, y1 - ymin)),
						(int) name_position[l], cx, cy))
			  name_position[l] = '\127';
		    }
		}
	    }
	    fprintf(fp, "grestore\n");

	    /* print remaining unit names.  (cx,cy) is the upper-left corner
	       of the rectangle containing the hex */

	    fprintf(fp, "gsave\n");

	    l = 0;
	    cy = hex_c;
	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		cy -= hex_c;
		
		cx = ((y%2)==0)*hex_w/2 - hex_w;
		nx = min(width, area.width-xmin);
		for (xx = xmin; xx < xmin + nx; xx++) {
		    cx += hex_w;
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);
		    
		    name = name_of_seen_unit_at(x, y, side);
		    /* free buffer */
		    if (name) {
			strcpy(tmpbuf,name);
			name = tmpbuf;
		    }
		    summary = summary_of_seen_units_at(x,y,side);
		    if (!name)
		      name = summary;
		    if (!name)
		      continue;
		    l++;

		    if ((name_layer
			 && name_position
			 && name_position[l] == '\177')
			|| !name_layer
			|| !name_position) {
			/* choose font */
			if (8 * ps_string_width(name, 1) > 1000 * hex_w) {
			    font = 0;
			} else {
			    font = 1;
			}
			fprintf(fp, "/%s findfont 8 scalefont setfont\n",
				ps_name[font]);
			
			fprintf(fp, "gsave %d %g translate (%s) n grestore\n",
				cx, cy/asym, name);
		    }
		}
	    }
	    fprintf(fp, "grestore\n");

	    /* show units & side emblems.  (cx,cy) is the upper-left corner
	       of the rectangle containing the hex */

	    fprintf(fp, "gsave\n");

	    cy = hex_c;
	    for (y = area.height - 1 - i * height; y >= ymin; y--) {
		cy -= hex_c;
	
		cx = ((y % 2) == 0) * hex_w / 2 - hex_w;
		nx = min(width, area.width - xmin);
		for (xx = xmin; xx < xmin + nx; xx++) {
		    cx += hex_w;
		    x = xx - (y + 1) / 2 + x0;
		    x = wrapx(x);
		    if (!in_area(x, y))
		      continue;
	
		    if ((utype = seen_utype_at(x,y,side)) != NONUTYPE) {
			fprintf(fp, "gsave %d %g translate u%X s%X grestore\n",
				cx, cy / asym, utype,
				sideno_of_seen_unit_at(x, y, side));
		    }
		}
	    }
	    fprintf(fp, "grestore\n");

	    fprintf(fp, "showpage\n");
	}
    }
    /* End of loop over pages. */
    
    fprintf(fp, "%%%%Trailer\n");
    fprintf(fp, "%%%%Pages: %d\n", page);
    fprintf(fp, "%%%%EOF\n");
    
    fclose(fp);
    if (legs)
      free(legs);
    if (name_layer)
      free(name_layer);
    if (name_position)
      free(name_position);
}

/* Compute the width of a PostScript string, in thousands of the
   "nominal size". */

int 
ps_string_width(str, font)
char *str;
int font;
{
    int i = 0, esc = 0;
    
    if (!str)
      return 0;
    
    for (; *str; str++) {
	if (*str=='\\' && !esc) {
	    esc = 1;
	} else {
	    i += ps_width[font][(int) *str];
	    esc = 0;
	}
    }
    return i;
}

/* This routine can be used by the interface to place legends */

/* orient==0 :  E (horizontal) only;
   orient==1 :  E, SE, NE;
   orient==2 :  E, SE, NE, ESE, ENE, N; */

/* block==0  :  write over any unit;
   block==1  :  don't write over "city-like" units;
   block==2  :  don't write over visible units. */

void
place_feature_legends(leg, nf, side, orient, block)
Legend *leg;
int nf;
Side *side;
int orient, block;
{
  int x, y, x1, y1, dx, dy, f, i, i3, id, d, nd, d1, dc;
    double dist;
    static int ndt[] = { 1, 3, 6 },
    dt[] = { EAST, SOUTHEAST, NORTHEAST, NORTHEAST, SOUTHEAST, EAST },
    da[] = { 0, -60, 60, 90, -30, 30 };
  unsigned char *auxf_layer, dmask;
    
    if (!features_defined()) return;
    
    orient = min(orient, 2);
    nd = ndt[orient];
    
    for (f = 1; f <= nf; f++) {
	leg[f-1].ox = 0;
	leg[f-1].oy = 0;
	leg[f-1].dx = 0;
	leg[f-1].dy = 0;
	leg[f-1].angle = 0;
	leg[f-1].dist  = -1;
    }
    
  /* Speedup: in auxf_layer we keep this information:
     the cell is unseen or hosts a blocking unit (bit 7);
     the cell has already been reached from direction id (bit id)
     [this avoids repeating the same path over and over;
      note that directions 3,4,5 zig-zag with step 3,
      so this bit is set/checked only every 3 steps.] */

  auxf_layer = (unsigned char *)
      malloc(area.height*area.width*sizeof(unsigned char));

  if (!auxf_layer)  return;

    for_all_cells(x,y) {
    if (seen_terrain_at(x,y,side) == NONTTYPE ||
	 blocking_utype(seen_utype_at(x,y,side),block)) {
      aset(auxf_layer,x,y, '\x80');
    } else {
      aset(auxf_layer,x,y, '\0');
    }
  }

  for_all_cells(x,y) {
	f = raw_feature_at(x,y);
	if (f<1 || f>nf)
	  continue;

	for (id = 0; id < nd; id++) {
      dmask = '\x01' << id;
	    d = dt[id];
	    d1 = (id<3) ? d : left_dir(left_dir(d));
	    x1 = x;  y1 = y;
	    dx = dy = 0;
      i3 = i = 0;
	    dist = 0;
      while (raw_feature_at(x1,y1) == f &&
	     !(aref(auxf_layer,x1,y1) &
	       ((id<3 || !i3) ? ('\x80' | dmask) : '\x80'))) {
	if (dist > leg[f-1].dist && (id<3 || !i3)) {
		    leg[f-1].ox =  x;  leg[f-1].oy =  y;
		    leg[f-1].dx = dx;  leg[f-1].dy = dy;
		    leg[f-1].angle = da[id];
		    leg[f-1].dist  = dist;
		}
	if (id<3 || !i3) {
	  auxf_layer[area.width*y1 + x1] |= dmask;
	}
	dc = (i3 == 1) ? d1 : d;
		dx += dirx[dc];
		x1 = wrap(x1 + dirx[dc]);
		dy += diry[dc];
		y1 += diry[dc];
	dist += (id<3) ? 1.0 : (i3 ? 0.5*sqrt3 : 0.0);
		i++;
	i3 = i % 3;
	    }
	}
    }

  free(auxf_layer);
}

int
blocking_utype(u, block)
int u, block;
{
    if (u == NONUTYPE || block == 0)
      return 0;
    if (block > 1)
      return 1;
    /* block==1:  only visible see-always unmovable units */
    return ((u_already_seen(u)>99 || u_see_always(u)) && !mobile(u));
}

int 
num_features()
{
    int f = 0, x, y;

    if (!features_defined())
      return 0;

    for_all_cells(x,y) {
	f = max(f, raw_feature_at(x,y));
    }
    return f;
}

int
print_unit_legends(fp, name, summary, m, dir, cx, cy)
FILE *fp;
char *name, *summary, *m;
int dir, cx, cy;
{
    int left, wide_name, wide_summ, down;
    
    if (dir < 0 || dir >= NUMDIRS)
      return 0;
    
    cx += hex_w*dirx[dir] + hex_w/2*diry[dir];  cy += hex_c*diry[dir];
    if (dir == EAST) cx -= 4;
    if (dir == WEST) cx += 4;
    if (dir == NORTHEAST || dir == NORTHWEST) cy -= 1;
    if (dir == SOUTHEAST || dir == SOUTHWEST) cy += 4;
    
    left = dir == EAST || dir == NORTHEAST || dir == SOUTHEAST;
    wide_name = 8*ps_string_width(name, 1) > 1000*hex_w;
    wide_summ = 8*ps_string_width(summary, 1) > 1000*hex_w;
    
    if (name) {
	if (summary && !(*m & N_U)) {
	    down = 0;
	} else if (!(*m & N_U) && !(*m & N_D)) {
	    down = dir!=SOUTHEAST && dir!=SOUTHWEST;
	} else if (!(*m & N_D)) {
	    down = 1;
	} else if (!(*m & N_U)) {
	    down = 0;
	} else {
	    return 0;
	}
	if (!down && !(*m & N_D) && summary) {
	    if (dir == NORTHEAST || dir == NORTHWEST) {
		cy -= 3;
	    } else if (dir == EAST || dir == WEST) {
		cy -= 2;
	    }
	}

	fprintf(fp, "gsave %d %g translate (%s) n%c%c grestore\n",
		cx, cy/asym, name, left ? 'l' : 'r', down ? 'd' : 'u');
	*m |= down ? N_D : N_U;
    }
    
    if (summary) {
	if (name)  cy += 3;
	if (!(*m & N_U) && !(*m & N_D)) {
	    down = dir!=SOUTHEAST && dir!=SOUTHWEST;
	} else if (!(*m & N_D)) {
	    down = 1;
	} else if (!(*m & N_U)) {
	    down = 0;
	} else {
	    return 1;
	}

	fprintf(fp, "gsave %d %g translate (%s) n%c%c grestore\n",
		cx, cy/asym, summary, left ? 'l' : 'r', down ? 'd' : 'u');
	*m |= down ? N_D : N_U;
    }
    return 1;
}
