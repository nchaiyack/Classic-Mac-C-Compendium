/* Copyright (c) 1992, 1993  Stanley T. Shebs. */
/* This program may be used, copied, modified, and redistributed freely */
/* for noncommercial purposes, so long as this notice remains intact. */

/* Definitions for the Mac interface. */

#undef USE_CMDLINE
#undef USE_CONSOLE

#ifdef THINK_C
#include <MacHeaders>
#endif /* THINK_C */

#ifndef THINK_C /* assume MPW */
#include <Values.h>
#include <Types.h>
#include <Resources.h>
#include <QuickDraw.h>
#include <Fonts.h>
#include <Events.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Desk.h>
#include <ToolUtils.h>
#include <Memory.h>
#include <SegLoad.h>
#include <Files.h>
#include <Folders.h>
#include <OSUtils.h>
#include <OSEvents.h>
#include <DiskInit.h>
#include <Packages.h>
#include <Traps.h>
#include <Lists.h>
#include <PPCToolbox.h>
#include <AppleEvents.h>
#include <StandardFile.h>
#include <Sound.h>
#endif

#ifndef THINK_C
#define QD(whatever) (qd.##whatever)
#else
#define QD(whatever) (whatever)
#endif

#define top_left(rect) (*(Point *) &(rect.top))
#define bottom_right(rect) (*(Point *) &(rect.bottom))

#include "macdefs.h"
#include "macimf.h"

/* How many magnifications we're supporting. */

#define NUMPOWERS 8

enum grays {
	blackgray,
	darkgray,
	mediumgray,
	lightgray,
	whitegray,
	fullcolor
};

/* The types of available designer tools. */

enum tooltype {
	notool,
	terraintool,
	unittool,
	peopletool,
	featuretool,
	brushsizetool,

	materialtool,
	elevationtool,
	temperaturetool,
	cloudstool,
	windstool
};

/* The user interface substructure.  This is only allocated for sides with
   attached displays. */

typedef struct a_ui {
	short followaction;	   /* move to where a change has occured */
	int itertime;
	int active;
} UI;

/* Each side can open up any number and shape of maps. */

#define MAXSELECTIONS 500

typedef struct a_map {
	short sx, sy;
	long  totsw, totsh;
	short pxw, pxh;		    /* size of window in pixels */
	short vx, vy;			/* lower left of viewport in hex coords */
	short vw, vh;			/* Boundaries of viewport in hex coords */
	short power;			/* index to this map's magnification */
	short mag;				/* magnification of each hex (a power of 2) */
	short vcx, vcy;		    /* center of the view */
	short hw, hh;			/* pixel dims of a hex cell */
	short hch;				/* height of hex cell between centers */
	short uw, uh;			/* pixel dims of unit subcell */
	short toph;
	WindowPtr window;
	ControlHandle hscrollbar;
	ControlHandle vscrollbar;
	/* How to draw the map. */
	short drawterrain;		/* Display solid color terrain on the map? */
	short drawgrid;		    /* Draw outlines around hexes? */
	short drawhexpats;		/* Draw terrain patterns? */
	short drawothermaps;	/* Draw outlines of other maps? */
	short drawunits;		/* Draw units on the map? */
	short drawnames;		/* Draw unit names/numbers on the map? */
	short oldesttoshow;	    /* the relative time of the oldest data */
	short agetofade;		/* point at which to gray out the display */
	short newesttoshow;	    /* the relative time of the newest data */
	short fadeterrain;		/* fade world features as well as units? */
	short lowestaltitude;   /* the lowest altitude of units to display */
	short highestaltitude;	/* the highest altitude of units to display */
	short drawborders;
	short drawconnections;
	short drawpeople;		/* Draw people sides on the map? */
	short drawelevations;	/* Draw elevations on the map? */
	short drawmaterials[MAXMTYPES];
	short nummaterialstodraw;
	short drawlighting;		/* Draw day/night on the map? */
	short drawtemperature;	/* Draw temperatures on the map? */
	short drawwinds;
	short drawclouds;
	short drawstorms;
	short drawplans;
	short drawai;
	short autoselect;
	Unit *curunit;
	short moveonclick;
	int numselections;
	int maxselections;
	Unit **selections;
	Rect contentrect;
	RgnHandle cellrgn;
	int cellrgnx, cellrgny;
	int maxdepth;           /* Largest # of bits/pixel of screens this map uses */
	struct a_map *next;		/* Link to the next map. */
} Map;

/* Each side can open up any number of lists of units. */

#define MAXINLIST 500

typedef struct a_list {
#if (MAXSIDES < 32)
	long sides;                      /* mask of sides to display the units of */
#else
	short sides[MAXSIDES];
#endif
	int mainsortmi;
	int listglimpsed;              /* list glimpsed units also? */
	enum sortkeys sortkeys[MAXSORTKEYS];  /* attributes to sort list elements on */
	UnitVector *contents;
	int numunits;
	int firstvisible;
	int lastvisible;
	int firstvisfield;
	int lastvisfield;
	int largeicons;
	int shouldreorg;
	WindowPtr window;
	ControlHandle hscrollbar;
	ControlHandle vscrollbar;
	struct a_list *next;
} List;

/* A closer look at a unit. */

typedef struct a_unit_closeup {
	struct a_unit *unit;
	WindowPtr window;
	struct a_unit_closeup *next;
} UnitCloseup;

/* A closer look at a side. */

typedef struct a_side_closeup {
	struct a_side *side;
	DialogPtr dialog;
	struct a_side_closeup *next;
} SideCloseup;

/* Iteration over all of a side's maps. */

#define for_all_maps(m)  \
  for ((m) = maplist; (m) != NULL; (m) = (m)->next)

/* Iteration over all of a side's lists. */

#define for_all_lists(l)  \
  for ((l) = listlist; (l) != NULL; (l) = (l)->next)

/* Iteration over all of a side's side closeups. */

#define for_all_side_closeups(c)  \
  for ((c) = sidecloseuplist; (c) != NULL; (c) = (c)->next)

#define for_all_unit_closeups(c)  \
  for ((c) = unitcloseuplist; (c) != NULL; (c) = (c)->next)

/* Other useful macros. */

#define window_width(w) ((w)->portRect.right - (w)->portRect.left)
#define window_height(w) ((w)->portRect.bottom - (w)->portRect.top)

#define clip_to_limits(lo,x,hi) (max((lo), min((x), (hi))))

#define hexagon_adjust(m) (area.xwrap ? 0 : ((area.height / 2) * (m)->hw) / 2)

#define partial_views() (!(g_see_all() || dside->designer || weseeall))
 
#define all_terrain_visible()  \
  (seeall || terrainseen || dside->designer || weseeall)

#define terrain_visible(xw, y)  \
  (seeallterrain || terrain_view(dside, xw, y) != UNSEEN)

#define units_visible(x, y)  \
  (seeall || dside->designer || weseeall || cover(dside, wrap(x), y) >= 1)

#define borders_visible(x, y, d)  \
  (seeall || dside->designer || weseeall || seen_border(dside, x, y, d))

#define bords_to_draw(m) (numbordtypes > 0 && bwid[(m)->power] > 0)

#define conns_to_draw(m) (numconntypes > 0 && cwid[(m)->power] > 0)

#define draw_any_materials(m) (0)

#define any_borders_at(x, y, b) (aux_terrain_at(x, y, b) != 0)

#define any_connections_at(x, y, c) (aux_terrain_at(x, y, c) != 0)

enum {
	dontdraw,
	useblocks,
	usepictures,
	usepolygons
};

#define p2c(pstr,cbuf)  \
  strncpy(cbuf, ((char *) (pstr) + 1), pstr[0]);  cbuf[pstr[0]] = '\0';

extern Side *dside;

extern int mags[], hws[], hhs[], hcs[], uws[], uhs[];
extern int dolin[];
extern int bsx[NUMPOWERS][7], bsy[NUMPOWERS][7], lsx[NUMPOWERS][6], lsy[NUMPOWERS][6];

Map *map_from_window();
List *list_from_window();
SideCloseup *side_closeup_from_window();

#define pascalify(STR) \
  sprintf(tmpbuf, " %s", STR);  \
  tmpbuf[0] = strlen(STR);

extern int playsounds;

extern int nummaps;

extern ImageFamily *uimages;
extern ImageFamily *timages;
extern ImageFamily *eimages;

extern ImageColor **tcolors;

extern WindowPtr helpwin;
extern struct a_helpnode *curhelpnode;
extern char *helpstring;
extern int helpscreentop;
extern TEHandle helptext;

extern WindowPtr instructionswin;

extern WindowPtr gamewin;
extern ControlHandle gamevscrollbar;

extern WindowPtr constructionwin;
extern ListHandle constructionunitlist;
extern ListHandle constructiontypelist;

extern WindowPtr designwin;
extern enum tooltype tooltype;
extern short curutype;
extern short curttype;
extern short cursidenumber;
extern short curfid;
extern Feature *curfeature;

extern WindowPtr historywin;

extern MenuHandle sidemenu;
extern MenuHandle utypemenu;
extern MenuHandle mtypemenu;
extern MenuHandle ttypemenu;

extern int hasColorQD;

extern int minscreendepth;
extern int maxscreendepth;

extern int sbarwid;

extern int bwid[];
extern int bwid2[];
extern int cwid[];

extern WindowPtr *winmenuwins;

extern int defaultdrawgrid;		   /* Display grid on the map? */
extern int defaultdrawnames;		   /* Display unit names/numbers on the map? */

extern int modaltool;

extern int defaultmoveonclick;
extern int defaultautoselect;

extern PolyHandle polygons[];
extern int lastpolyx[], lastpolyy[];
extern PolyHandle gridpolygons[];
extern int lastgridpolyx[], lastgridpolyy[];

extern RgnHandle cellrgns[];
extern int lastcellrgnx[], lastcellrgny[];
extern RgnHandle gridcellrgns[];
extern int lastgridcellrgnx[], lastgridcellrgny[];

extern struct a_map *maplist;	   /* chain of maps that we're using */
extern struct a_list *listlist;	 /* chain of lists */
extern struct a_unit_closeup *unitcloseuplist;	 /* chain of unit closeups */
extern struct a_side_closeup *sidecloseuplist;	 /* chain of side closeups */

extern CursHandle paintcursors[];

extern CursHandle cellpaintor;
extern CursHandle bordpaintor;
extern CursHandle connpaintor;
extern CursHandle unitpaintor;
extern CursHandle peoplepaintor;
extern CursHandle featurepaintor;
extern CursHandle materialpaintor;
extern CursHandle elevpaintor;
extern CursHandle cloudpaintor;
extern CursHandle temppaintor;

extern CursHandle movecursors[];
extern CursHandle nomovecursor;
extern CursHandle allmovecursor;
extern CursHandle grayarrowcursor;
extern CursHandle opencrosscursor;

extern char *curdatestr;

extern int mayseeall;

extern int weseeall;

extern RGBColor graycolor, blackcolor;

extern enum grays gridgray;
extern enum grays unseengray;
extern enum grays bggray;

extern RGBColor gridcolor;
extern RGBColor unseencolor;
extern RGBColor bgcolor;
extern RGBColor blackcolor;

extern int conwid, tophgt;

/* Function prototypes. */

char *savefile_name();
char *newsfile_name();

char *get_string_from_item(Handle itemhandle);

Map *create_map(int power);

UnitCloseup *unit_closeup_from_window();
UnitCloseup *find_unit_closeup(Unit *unit);

void draw_unit_image(WindowPtr win, int sx, int sy, int sw, int sh, int u, int e, int mod);
void draw_side_emblem(WindowPtr win, int ex, int ey, int ew, int eh, int e);

void xform(Map *map, int x, int y, int *sxp, int *syp);
void xform_unit(Map *map, Unit *unit, int *sxp, int *syp, int *swp, int *shp);
void xform_unit_self(Map *map, Unit *unit, int *sxp, int *syp, int *swp, int *shp);
void xform_occupant(Map *map, Unit *transport, Unit *unit, int sx, int sy, int sw, int sh, int *sxp, int *syp, int *swp, int *shp);
int nearest_cell(Map *map, int sx, int sy, int *xp, int *yp);
int nearest_boundary(Map *map, int sx, int sy, int *xp, int *yp, int *dirp);
int nearest_unit(Map *map, int sx, int sy, Unit **unitp);
