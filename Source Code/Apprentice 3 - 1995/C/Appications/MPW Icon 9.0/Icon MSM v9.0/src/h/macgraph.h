#ifdef MacGraph
/*
 * Constants
 */

#define kEmptyString    "\p"
#define kMoveToFront    (WindowPtr)-1L
#define kVisible        true
#define kInvisible      false
#define kHasGoAway      true
#define kNilRefCon      (long)nil
#define kPi             3.14159265358979323846264338327950288419716939937511

#define DMAXCOLORS           256
#define FS_SOLID             1
#define FS_STIPPLE           2
#define FS_OPAQUESTIPPLE     4


/*
 * Macros
 */

#define STDLOCALS(wb) \
                      wcp wc = (wb)->context;\
                      wsp ws = (wb)->window;\
                      WindowPtr stdwin = ws->theWindow
   
#define ICONFILENAME(wb) ((wb)->window->iconimage)
#define ICONLABEL(wb) ((wb)->window->iconlabel)
#define WINDOWLABEL(wb) ((wb)->window->windowlabel)

#define ANGLE(ang) (ang)
#define EXTENT(ang) (ang)
#define RECX(recs) ((recs).x)
#define RECY(recs) ((recs).y)
#define RECWIDTH(recs) ((recs).width)
#define RECHEIGHT(recs) ((recs).height)

#define FULLARC 360<<6
#define ARCWIDTH(arc) ((arc).width)
#define ARCHEIGHT(arc) ((arc).height)

#define RED(x) ((x).red)
#define GREEN(x) ((x).green)
#define BLUE(x) ((x).blue)

#define ASCENT(wb) (wb->context->font->fInfo.ascent)
#define DESCENT(wb) (wb->context->font->fInfo.descent)
#define LEADING(wb) (wb->context->font->fInfo.leading)
#define FWIDTH(wb) (wb->context->font->fInfo.widMax)
#define FHEIGHT(wb) ( (ASCENT(wb))+(DESCENT(wb))+(LEADING(wb)) )
#define MAXDESCENDER(wb) (wb->context->font->fInfo.descent)

#define DISPLAYWIDTH(wb) (screenBits.bounds.right-screenBits.bounds.left)
#define DISPLAYHEIGHT(wb) (screenBits.bounds.bottom-screenBits.bounds.top)

#define LINEWIDTH(wb) (wb->context->contextPtr->pnSize.h)
#define TEXTWIDTH(wb,s,n) ( TextWidth(s,(int)(s),n) )

#define SCREENDEPTH(wb) 8

#define COPYCONTEXT(c) BackPat((c)->bkPat);\
                       MoveTo((c)->pnLoc.h,(c)->pnLoc.v);\
                       PenSize((c)->pnSize.h,(c)->pnSize.v);\
                       PenMode((c)->pnMode);\
                       PenPat((c)->pnPat);\
                       TextFont((c)->txFont);\
                       TextFace((c)->txFace);\
                       TextMode((c)->txMode);\
                       TextSize((c)->txSize);\
                       SpaceExtra((c)->spExtra);\
                       ForeColor((c)->fgColor);\
                       BackColor((c)->bkColor);\
                       ColorBit((c)->colrBit)
                         
#define SETCONTEXTDEFAULT(c) memcpy(&((c)->bkPat),&white,sizeof(Pattern));\
                             memcpy(&((c)->fillPat),&black,sizeof(Pattern));\
                             (c)->pnLoc.h=0;(c)->pnLoc.v=0;\
                             (c)->pnSize.h=1;(c)->pnSize.v=1;\
                             (c)->pnMode=patCopy;\
                             memcpy(&((c)->pnPat),&black,sizeof(Pattern));\
                             (c)->pnVis=0;\
                             (c)->txFont=0;\
                             (c)->txFace=normal;\
                             (c)->txMode=srcOr;\
                             (c)->txSize=0;\
                             (c)->spExtra=0;\
                             (c)->fgColor=blackColor;\
                             (c)->bkColor=whiteColor;\
                             (c)->colrBit=0

/*
 * the bitmasks for the modifier keys
 */
 
#define ControlMask          (1 << 16)
#define Mod1Mask             (2 << 16)
#define ShiftMask            (4 << 16)
#define VirtKeyMask          (8 << 16)

/*
 * typedef & structs
 */
 
typedef struct
  {
  long x, y;
  long width, height;
  long angle1, angle2;
  } XArc;
  
typedef struct
  {
  long x,y;
  } XPoint;
  
typedef struct 
   {
   long x1, y1;
   long x2, y2;
   } XSegment;
   
typedef struct
   {
   long x, y;
   long width, height;
   } XRectangle;

/*
 * macros performing row/column to pixel y,x translations
 * computation is 1-based and depends on the current font's size.
 * exception: XTOCOL as defined is 0-based, because that's what its
 * clients seem to need.
 */
#define ROWTOY(wb,row) ((row-1) * LEADING(wb) + ASCENT(wb) + MARGIN)
#define COLTOX(wb,col) ((col-1) * FWIDTH(wb) + MARGIN)
#define YTOROW(wb,y)   (((y) - MARGIN) / LEADING(wb) + 1)
#define XTOCOL(wb,x)  (!FWIDTH(wb) ? ((x)-MARGIN) : (((x) - MARGIN) / FWIDTH(wb)))
#endif                  /* MacGraph */
