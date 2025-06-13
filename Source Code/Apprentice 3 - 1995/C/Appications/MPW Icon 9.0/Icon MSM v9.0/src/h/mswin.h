/*
 * mswin.h - macros and types used in the MS Windows graphics interface.
 */

#define DRAWOP_AND			R2_MASKPEN
#define DRAWOP_ANDINVERTED		R2_MASKNOTPEN
#define DRAWOP_ANDREVERSE		R2_NOTMASKPEN
#define DRAWOP_CLEAR			R2_BLACK
#define DRAWOP_COPY			R2_COPYPEN
#define DRAWOP_COPYINVERTED		R2_NOTCOPYPEN
#define DRAWOP_EQUIV			R2_NOTXORPEN
#define DRAWOP_INVERT			R2_NOT
#define DRAWOP_NAND			R2_MASKNOTPEN
#define DRAWOP_NOOP			R2_NOP
#define DRAWOP_NOR			R2_MERGENOTPEN
#define DRAWOP_OR			R2_MERGEPEN
#define DRAWOP_ORINVERTED		R2_MERGEPENNOT
#define DRAWOP_ORREVERSE		R2_NOTMERGEPEN
#define DRAWOP_REVERSE			R2_USER1
#define DRAWOP_SET			R2_WHITE
#define DRAWOP_XOR			R2_XORPEN

#define TEXTWIDTH(w,s,n) textWidth(w, s, n)
#define SCREENDEPTH(w) 0
#define ASCENT(w)  (w->context->metrics.tmAscent)
#define DESCENT(w) (w->context->metrics.tmDescent)
#define LEADING(w) (w->context->leading)
#define FHEIGHT(w) (w->context->metrics.tmHeight)
#define FWIDTH(w)  (w->context->metrics.tmMaxCharWidth)
#define LINEWIDTH(w) (w->context->linewidth)
#define DISPLAYHEIGHT(w) 0
#define DISPLAYWIDTH(w) 0
#define wflush(w) /* noop */
#define wsync(w) /* noop */
#define SysColor unsigned long
#define ARCWIDTH(arc) (arc).width
#define ARCHEIGHT(arc) (arc).height
/*
 * These get fixed up in the window-system-specific code
 */
#define RECX(rec) (rec).left
#define RECY(rec) (rec).top
#define RECWIDTH(rec) (rec).right
#define RECHEIGHT(rec) (rec).bottom
/*
 *
 */
#define ANGLE(ang) 0
#define EXTENT(ang) 0
#define FULLARC 2 * PI
#define ISICONIC(w) 0
#define ISFULLSCREEN(w) 0
#define ISROOTWIN(w) (0) 0
#define ISNORMALWINDOW(w) 0
#define ICONFILENAME(w) ""
#define ICONLABEL(w) ""
#define WINDOWLABEL(w) ""

#define MAXLABEL	128

#define MAXDESCENDER(w) 0

/*
 * gemeotry bitmasks
 */
#define GEOM_WIDTH           1
#define GEOM_HEIGHT          2
#define GEOM_POSX            4
#define GEOM_POSY            8
/*
 * fill styles
 */
#define FS_SOLID             1
#define FS_STIPPLE           2
#define FS_OPAQUESTIPPLE     4
/*
 * the special ROP code for mode reverse
 */
#define R2_USER1            (R2_LAST << 1)
/*
 * window states
 */
#define WS_NORMAL            0
#define WS_MIN               1
#define WS_MAX               2

/*
 * something I think should be #defined
 */
#define EOS                  '\0'

/* size of the working buffer, used for dialog messages and such */
#define PMSTRBUFSIZE         2048
/*
 * the bitmasks for the modifier keys
 */
#define ControlMask          (1 << 16)
#define Mod1Mask             (2 << 16)
#define ShiftMask            (4 << 16)
#define VirtKeyMask          (8 << 16)

/* some macros for PresentationManager */

#define MAKERGB(r,g,b) RGB(r,g,b)
#define RGB16TO8(x) if ((x) > 0xff) (x) = (((x) >> 8) & 0xff)
#define HideCursor(ws)
#define ShowCursor(ws)
#define FNTWIDTH(size) ((size) & 0xFFFF)
#define FNTHEIGHT(size) ((size) >> 16)
#define MAKEFNTSIZE(height, width) (((height) << 16) | (width))
#define WaitForEvent(msgnum, msgstruc) ObtainEvents(NULL, WAIT_EVT, msgnum, msgstruc)

/*
 * "get" means remove them from the Icon list and put them on the ghost que
 */
#define EVQUEGET(ws,d) { \
  int i;\
  if (!c_get((struct b_list *)BlkLoc((ws)->listp),&d)) fatalerr(0,NULL); \
  if (Qual(d)) {\
      ws->eventQueue[(ws)->eQfront++] = *StrLoc(d); \
      if ((ws)->eQfront >= EQUEUELEN) (ws)->eQfront = 0; \
      (ws)->eQback = (ws)->eQfront; \
      } \
  }
#define EVQUEEMPTY(ws) (BlkLoc((ws)->listp)->list.size == 0)

/*
 * we make the segment structure look like this so that we can
 * cast it to POINTL structures that can be passed to GpiPolyLineDisjoint
 */
typedef struct {
   LONG x1, y1;
   LONG x2, y2;
   } XSegment;

typedef POINT XPoint;
typedef RECT XRectangle;

typedef struct {
  LONG x, y;
  LONG width, height;
  LONG angle1, angle2;
  } XArc;

/*
 * Macros to ease coding in which every window system call must be done twice.
 */
#define RENDER1(func,v1) {\
   if (ws->hpsWin) func(ws->hpsWin, v1); \
   func(ws->hpsBitmap, v1);}
#define RENDER2(func,v1,v2) {\
   if (ws->hpsWin) func(ws->hpsWin, v1, v2); \
   func(ws->hpsBitmap, v1, v2);}
#define RENDER3(func,v1,v2,v3) {\
   if (ws->hpsWin) func(ws->hpsWin, v1, v2, v3); \
   func(ws->hpsBitmap, v1, v2, v3);}
#define RENDER4(func,v1,v2,v3,v4) {\
   if (ws->hpsWin) func(ws->hpsWin, v1, v2, v3, v4); \
   func(ws->hpsBitmap, v1, v2, v3, v4);}
#define RENDER5(func,v1,v2,v3,v4,v5) {\
   if (ws->hpsWin) func(ws->hpsWin, v1, v2, v3, v4, v5); \
   func(ws->hpsBitmap, v1, v2, v3, v4, v5);}

/*
 * macros performing row/column to pixel y,x translations
 * computation is 1-based and depends on the current font's size.
 * exception: XTOCOL as defined is 0-based, because that's what its
 * clients seem to need.
 */
#define ROWTOY(wb, row)  ((row - 1) * (LEADING(wb) + FHEIGHT(wb)) + \
                          MARGIN + ASCENT(wb))
#define COLTOX(wb, col)  ((col - 1) * FWIDTH(wb) + MARGIN)
#define YTOROW(wb, y)    (((y) - MARGIN) /  (LEADING(wb) + FHEIGHT(wb)) + 1)
#define XTOCOL(wb, x)    (((x) - MARGIN) / FWIDTH(wb))
/*
 * system size values
 */
#define BORDERWIDTH      (WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER))
#define BORDERHEIGHT     (WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER))
#define TITLEHEIGHT      (WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR))

#define STDLOCALS(w) \
   wcp wc = (w)->context;\
   wsp ws = (w)->window;\
   HWND stdwin = ws->win;\
   HBITMAP stdpix = ws->pix;\
   HDC stddc = CreateWinDC(w);\
   HDC pixdc = CreatePixDC(w, stddc);

#define FREE_STDLOCALS if (stdwin) ReleaseDC(stdwin, stddc); DeleteDC(pixdc);
