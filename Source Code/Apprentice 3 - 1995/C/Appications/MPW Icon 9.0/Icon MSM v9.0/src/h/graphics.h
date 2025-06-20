/*
 * graphics.h - macros and types used in Icon's graphics interface.
 */
#ifdef MacGraph
#include "::h:MacGraph.h"
#endif					/* MacGraph */

#ifdef XWindows
#include "::h:xwin.h"
#endif					/* XWindows */

#ifdef PresentationManager
#include "::h:pmwin.h"
#endif					/* PresentationManager */

#ifdef MSWindows
#include "::h:mswin.h"
#endif					/* MSWindows */

#ifndef MAXXOBJS
#define MAXXOBJS 256
#endif					/* MAXXOBJS */

#ifndef DMAXCOLORS
#define DMAXCOLORS 256
#endif					/* DMAXCOLORS */

#ifndef MAXCOLORNAME
#define MAXCOLORNAME 40
#endif					/* MAXCOLORNAME */

#ifndef MAXFONTWORD
#define MAXFONTWORD 40
#endif					/* MAXFONTWORD */

#define DEFAULTFONTSIZE 14

#define FONTATT_SPACING		0x01000000
#define FONTFLAG_MONO		0x00000001
#define FONTFLAG_PROPORTIONAL	0x00000002

#define FONTATT_SERIF		0x02000000
#define FONTFLAG_SANS		0x00000004
#define FONTFLAG_SERIF		0x00000008

#define FONTATT_SLANT		0x04000000
#define FONTFLAG_ROMAN		0x00000010
#define FONTFLAG_ITALIC		0x00000020
#define FONTFLAG_OBLIQUE	0x00000040

#define FONTATT_WEIGHT		0x08000000
#define FONTFLAG_LIGHT		0x00000100
#define FONTFLAG_MEDIUM		0x00000200
#define FONTFLAG_DEMI		0x00000400
#define FONTFLAG_BOLD		0x00000800

#define FONTATT_WIDTH		0x10000000
#define FONTFLAG_CONDENSED	0x00001000
#define FONTFLAG_NARROW		0x00002000
#define FONTFLAG_NORMAL		0x00004000
#define FONTFLAG_WIDE		0x00008000
#define FONTFLAG_EXTENDED	0x00010000

/*
 * EVENT HANDLING
 *
 * Each window keeps an associated queue of events waiting to be
 * processed.  The queue consists of <eventcode,x,y> triples,
 * where eventcodes are strings for normal keyboard events, and
 * integers for mouse and special keystroke events.
 *
 * The main queue is an icon list.  In addition, there is a queue of
 * old keystrokes maintained for cooked mode operations, maintained
 * in a little circular array of chars.
 */
#define EQ_MOD_CONTROL (1<<16)
#define EQ_MOD_META    (1<<17)
#define EQ_MOD_SHIFT   (1<<18)

#define EVQUESUB(w,i) *evquesub(w,i)
#define EQUEUELEN 256
#define MARGIN 0


/*
 * mode bits for the Icon window context (as opposed to X context)
 */

#ifdef XWindows
#define ISZOMBIE(w)     ((w)->window->bits & 1)
#endif					/* XWindows */
#ifdef PresentationManager
#define ISINITIAL(w)    ((w)->window->bits & 1)
#define ISINITIALW(w)   ((w)->bits & 1)
#endif					/* PresentationManager */
#define ISCURSORON(w)   ((w)->window->bits & 2)
#ifdef PresentationManager
#define ISCURSORONW(ws) ((ws->bits) & 2)
#endif					/* PresentationManager */
#define ISMAPPED(w)	((w)->window->bits & 4)
#define ISREVERSE(w)    ((w)->context->bits & 8)
#define ISXORREVERSE(w)	((w)->context->bits & 16)
#define ISXORREVERSEW(w) ((w)->bits & 16)
#define ISEXPOSED(w)    ((w)->window->bits & 256)
#define ISCEOLON(w)     ((w)->window->bits & 512)
#define ISECHOON(w)     ((w)->window->bits & 1024)
#ifdef PresentationManager
#define ISMINPEND(w)    ((w)->window->bits & 2048)
#define ISMINPENDW(w)   ((w)->bits & 2048)
#endif					/* PresentationManager */

#ifdef XWindows
#define SETZOMBIE(w)    ((w)->window->bits |= 1)
#endif					/* XWindows */
#ifdef PresentationManager
#define SETINITIAL(w)   ((w)->window->bits |= 1)
#endif					/* PresentationManager */
#define SETCURSORON(w)  ((w)->window->bits |= 2)
/* 4 is available */
#define SETMAPPED(w)   ((w)->window->bits |= 4)
#define SETREVERSE(w)   ((w)->context->bits |= 8)
#define SETXORREVERSE(w) ((w)->context->bits |= 16)
#define SETEXPOSED(w)   ((w)->window->bits |= 256)
#define SETCEOLON(w)    ((w)->window->bits |= 512)
#define SETECHOON(w)    ((w)->window->bits |= 1024)
#ifdef PresentationManager
#define SETMINPEND(w)   ((w)->window->bits |= 2048)
#endif					/* PresentationManager */

#ifdef XWindows
#define CLRZOMBIE(w)    ((w)->window->bits &= ~1)
#endif					/* XWindows */
#ifdef PresentationManager
#define CLRINITIAL(w)   ((w)->window->bits &= ~1)
#define CLRINITIALW(w)  ((w)->bits &= ~1)
#endif					/* PresentationManager */
#define CLRCURSORON(w)  ((w)->window->bits &= ~2)
#define CLRMAPPED(w)    ((w)->window->bits &= ~4)
#define CLRREVERSE(w)   ((w)->context->bits &= ~8)
#define CLRXORREVERSE(w) ((w)->context->bits &= ~16)
#define CLREXPOSED(w)   ((w)->window->bits &= ~256)
#define CLRCEOLON(w)    ((w)->window->bits &= ~512)
#define CLRECHOON(w)    ((w)->window->bits &= ~1024)
#ifdef PresentationManager
#define CLRMINPEND(w)   ((w)->window->bits &= ~2048)
#define CLRMINPENDW(w)  ((w)->bits &= ~2048)
#endif					/* PresentationManager */



/*
 * Window Resources
 * Icon "Resources" are a layer on top of the window system resources,
 * provided in order to facilitate resource sharing and minimize the
 * number of calls to the window system.  Resources are reference counted.
 * These data structures are simple sets of pointers
 * into internal window system structures.
 */



/*
 * Fonts are allocated within displays.
 */
typedef struct _wfont {
  int		refcount;
  struct _wfont *previous, *next;
#ifdef MacGraph                 /* MacGraph */
  short     fontNum;
  Style     fontStyle;
  int       fontSize;
  FontInfo  fInfo;              /* I-173 */
#endif                          /* MacGraph */
#ifdef XWindows
  char	      *	name;			/* name for XAttrib and fontsearch */
  int		height;			/* font height */
  int		leading;		/* inter-line leading */
  XFontStruct *	fsp;			/* X font pointer */
#endif					/* XWindows */
#ifdef PresentationManager
/* XXX replace this HUGE structure with single fields later - when we know
   conclusively which ones we need */
  FONTMETRICS	metrics;		/* more than you ever wanted to know */
#endif					/* PresentationManager */
#ifdef MSWindows
  TEXTMETRIC	metrics;
#endif					/* MSWindows */
} wfont, *wfp;


/*
 * These structures and definitions are used for colors and images.
 */
typedef struct {
   int red, green, blue;		/* color components, linear 0 - 65535 */
   } LinearColor;

struct palentry {			/* entry for one palette member */
   LinearColor clr;			/* RGB value of color */
   char valid;				/* nonzero if entry is valid */
   char used;				/* nonzero if char is used */	
   };

struct imgdata {			/* image loaded from a file */
   int width, height;			/* image dimensions */
   struct palentry *paltbl;		/* pointer to palette table */
   unsigned char *data;			/* pointer to image data */
   };

#define TCH1 '~'			/* usual transparent character */
#define TCH2 0377			/* alternate transparent character */
#define PCH1 ' '			/* punctuation character */
#define PCH2 ','			/* punctuation character */


#ifdef XWindows
/*
 * Displays are maintained in a global list in rwinrsc.r.
 */
typedef struct _wdisplay {
  int		refcount;
  char		name[MAXDISPLAYNAME];
  Display *	display;
  GC		icongc;
  Colormap	cmap;
  double	gamma;
  int		screen;
  int		numFonts;
  wfp		fonts;
  int           numColors;		/* allocated color info */
  struct wcolor	colors[DMAXCOLORS];
  Cursor	cursors[NUMCURSORSYMS];
  struct _wdisplay *previous, *next;
} *wdp;
#endif					/* XWindows */

#ifdef PresentationManager
/*
 * Presentation space local id's are used to identify fonts, bitmaps
 * and markers.  Since we have 2 presentation spaces for each window,
 * and contexts can be associated with different windows through bindings,
 * the local identifier map must be identical throughout all ps (since the
 * context can identify a font as ID 2 on one space and that must be valid
 * on each space it is bound to).  This will be handled by a global array
 * of lclIdentifier.
 */
#define MAXLOCALS               255
#define IS_FONT                 1
#define IS_PATTERN              2
#define IS_MARKER               4               /* unused for now */

typedef struct _lclIdentifier {
  SHORT idtype;         /* type of the id, either font or pattern */
  SHORT refcount;       /* reference count, when < 1, deleted */
  union {
     wfont font;    /* font info */
     HBITMAP   hpat;    /* pattern bitmap handle */
     } u;
  struct _lclIdentifier *next,          /* dbl linked list */
                        *previous;
  } lclIdentifier;

#endif					/* PresentationManager */


/*
 * "Context" comprises the graphics context, and the font (i.e. text context).
 * Foreground and background colors (pointers into the display color table)
 * are stored here to reduce the number of window system queries.
 * Contexts are allocated out of a global array in rwinrsrc.c.
 */
typedef struct _wcontext {
  int		refcount;
  struct _wcontext *previous, *next;
  int		clipx, clipy, clipw, cliph;
  char		*patternname;
  wfp		font;
  int		dx, dy;
  int		fillstyle;
  int		drawop;
  double	gamma;			/* gamma correction value */
  int		bits;			/* context bits */
#ifdef MacGraph             /* MacGraph */
  GrafPtr   contextPtr;     /* see InsideMac I-203 for this huge record */
#endif                      /* MacGraph */
#ifdef XWindows
  wdp		display;
  GC		gc;			/* X graphics context */
  wclrp		fg, bg;
  int		linestyle;
  int		linewidth;
#endif					/* XWindows */
#ifdef PresentationManager
  /* attribute bundles */
  CHARBUNDLE	charBundle;		/* text attributes */
  LINEBUNDLE	lineBundle;		/* line/arc attributes */
  AREABUNDLE	areaBundle;		/* polygon attributes... */
  IMAGEBUNDLE	imageBundle;		/* attributes use in blit of mono bms */
  LONG 		fntLeading;		/* external leading for font - user */
  SHORT		currPattern;		/* id of current pattern */
  LONG		numDeps;		/* number of window dependants */
  LONG		maxDeps;		/* maximum number of deps in current table */
  struct _wstate **depWindows;           /* array of window dependants */
#endif					/* PresentationManager */
#ifdef MSWindows
  HPEN		pen;
  HBRUSH	brush;
  SysColor	fg, bg;
  char		*fgname, *bgname;
  int		linestyle, linewidth, leading;
  TEXTMETRIC	metrics;
#endif					/* MSWindows*/
} wcontext, *wcp;

/*
 * "Window state" includes the actual X window and references to a large
 * number of resources allocated on a per-window basis.  Windows are
 * allocated out of a global array in rwinrsrc.c.  Windows remember the
 * first WMAXCOLORS colors they allocate, and deallocate them on clearscreen.
 */
typedef struct _wstate {
  int		refcount;		/* reference count */
  struct _wstate *previous, *next;
  int		pixheight;		/* backing pixmap height, in pixels */
  int		pixwidth;		/* pixmap width, in pixels */
  char		*windowlabel;		/* window label */
  char		*iconimage;		/* icon pixmap file name */
  char		*iconlabel;		/* icon label */
  struct imgdata initimage;		/* initial image data */
  struct imgdata initicon;		/* initial icon image data */
  int		y, x;			/* current cursor location, in pixels*/
  int		pointery,pointerx;	/* current mouse location, in pixels */
  int		posy, posx;		/* desired upper lefthand corner */
  unsigned int	height;			/* window height, in pixels */
  unsigned int	width;			/* window width, in pixels */
  int		bits;			/* window bits */
  int		theCursor;		/* index into cursor table */
  char		eventQueue[EQUEUELEN];  /* queue of cooked-mode keystrokes */
  int		eQfront, eQback;
  struct descrip filep, listp;		/* icon values for this window */
#ifdef MacGraph         /* MacGraph */
  WindowPtr theWindow;      /* pointer to the window */
  PicHandle windowPic;      /* handle to backing pixmap */
#endif                  /* MacGraph */
#ifdef XWindows
  wdp		display;
  Window	win;			/* X window */
  Pixmap	pix;			/* current screen state */
  Pixmap	initialPix;		/* an initial image to display */
  Window        iconwin;		/* icon window */
  Pixmap	iconpix;		/* icon pixmap */
  int		normalx, normaly;	/* pos to remember when maximized */
  int		normalw, normalh;	/* size to remember when maximized */
  wclrp		winbg;			/* window background color */
  int           numColors;		/* allocated color info */
  short		*theColors;		/* indices into display color table */
  int           numiColors;		/* allocated color info for the icon */
  short		*iconColors;		/* indices into display color table */
  int		iconic;			/* window state; icon, window or root*/
  int		iconx, icony;           /* location of icon */
  unsigned int	iconw, iconh;		/* width and height of icon */
  long		wmhintflags;		/* window manager hints */
#endif					/* XWindows */
#ifdef PresentationManager
  HWND		hwnd;			/* handle to the window (client) */
  HWND		hwndFrame;		/* handle to the frame window */
  HMTX		mutex;			/* window access mutex sem */
  HDC		hdcWin;			/* handle to window device context */
  HPS		hpsWin;			/* pres space for window */
  HPS		hpsBitmap;		/* pres space for the backing bitmap */
  HBITMAP	hBitmap;		/* handle to the backing bitmap */
  HDC		hdcBitmap;		/* handle to the bit, memory DC */
  wcontext	*charContext;		/* context currently loaded in PS's */
  wcontext	*lineContext;		
  wcontext 	*areaContext;
  wcontext	*imageContext;
  wcontext	*clipContext;
  LONG 		winbg;			/* window background color */
  HBITMAP	hInitialBitmap;		/* the initial image to display */
  HPOINTER	hPointer;		/* handle to window's current pointer*/
  CURSORINFO	cursInfo;		/* cursor information stored on lose focus */
  LONG		numDeps;		/* number of context dependants */
  LONG		maxDeps;
  wcontext      **depContexts;          /* array of context dependants */
  /* XXX I don't like this next line, but it will do for now - until I figure
     out something better.  Following the charContext pointer to find the
     descender value is not enough as it could be NULL */
  SHORT         lastDescender;          /* the font descender value from last wc */
  HRGN		hClipWindow;		/* clipping regions */
  HRGN		hClipBitmap;
  BYTE		winState;               /* window state: icon, window, maximized */
  HBITMAP       hIconBitmap;            /* bitmap to display when iconized */
#endif					/* PresentationManager */
#ifdef MSWindows
  HWND		win;			/* client window */
  HBITMAP	pix;			/* backing bitmap */
  HWND		iconwin;		/* client window */
  HBITMAP	iconpix;		/* backing bitmap */
  HBITMAP	initialPix;		/* backing bitmap */
  DWORD		last_time;
#endif					/* MSWindows */
} wstate, *wsp;

/*
 * Icon window file variables are actually pointers to "bindings"
 * of a window and a context.  They are allocated out of a global
 * array in rwinrsrc.c.  There is one binding per Icon window value.
 */
typedef struct _wbinding {
  int refcount;
  struct _wbinding *previous, *next;
  wcp context;
  wsp window;
} wbinding, *wbp;



typedef struct {
  char *s;
  int i;
} stringint, *siptr;

/*
 * Gamma Correction value to compensate for nonlinear monitor color response
 */
#ifndef GammaCorrection
#define GammaCorrection 2.5
#endif					/* GammaCorrection */

/*
 * Attributes
 */

#define A_ASCENT	1
#define A_BG		2
#define A_CANVAS	3
#define A_CEOL		4
#define A_CLIPH		5
#define A_CLIPW		6
#define A_CLIPX		7
#define A_CLIPY		8
#define A_COL		9
#define A_COLUMNS	10
#define A_CURSOR	11
#define A_DEPTH		12
#define A_DESCENT	13
#define A_DISPLAY	14
#define A_DISPLAYHEIGHT	15
#define A_DISPLAYWIDTH	16
#define A_DRAWOP	17
#define A_DX		18
#define A_DY		19
#define A_ECHO		20
#define A_FG		21
#define A_FHEIGHT	22
#define A_FILLSTYLE	23
#define A_FONT		24
#define A_FWIDTH	25
#define A_GAMMA		26
#define A_GEOMETRY	27
#define A_HEIGHT	28
#define A_ICONIC	29
#define A_ICONIMAGE     30
#define A_ICONLABEL	31
#define A_ICONPOS	32
#define A_IMAGE		33
#define A_LABEL		56
#define A_LEADING	34
#define A_LINES		35
#define A_LINESTYLE	36
#define A_LINEWIDTH	37
#define A_PATTERN	38
#define A_POINTERCOL	39
#define A_POINTERROW	40
#define A_POINTERX	41
#define A_POINTERY	42
#define A_POINTER	43
#define A_POS		44
#define A_POSX		45
#define A_POSY		46
#define A_REVERSE	47
#define A_ROW		48
#define A_ROWS		49
#define A_SIZE		50
#define A_VISUAL	51
#define A_WIDTH		52
#define A_WINDOWLABEL   53
#define A_X		54
#define A_Y		55

#define NUMATTRIBS	56

#define XICONSLEEP	20 /* milliseconds */
