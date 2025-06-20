/*
 * File: rwindow.r
 *  non window-system-specific window support routines
 */

#ifdef Graphics

hidden	int	colorphrase	Params((char *buf, int *r, int *g, int *b));
hidden	double	rgbval		Params((double n1, double n2, double hue));

hidden	int	setpos          Params((wbp w, char *s));
hidden	int	sicmp		Params((siptr sip1, siptr sip2));

#ifdef PresentationManager
extern LONG ScreenBitsPerPel;
#endif					/* PresentationManager */

/*
 * A non-existent library function (exported to fsys.c)
 */
char *strnchr(s,ci,len)
char *s;
int ci;
long len;
   {
   register long i;
   char c = (char)ci;
   for(i=0;i<len;i++,s++) if (*s == c) return s;
   return NULL;
   }

/*
 * subscript the already-processed-events "queue" to index i.
 * used in "cooked mode" I/O to determine, e.g. how far to backspace.
 */
char *evquesub(w,i)
wbp w;
int i;
   {
   wsp ws = w->window;
   int j = ws->eQback+i;

   if (i < 0) {
      if (j < 0) j+= EQUEUELEN;
      else if (j > EQUEUELEN) j -= EQUEUELEN;
      return &(ws->eventQueue[j]);
      }
   else {
      /* "this isn't getting called in the forwards direction!\n" */
      return NULL;
      }
   }


/*
 * get event from window, assigning to &x, &y, and &interval
 *
 * returns 0 for success, -1 if window died, -2 for malformed queue
 */
int wgetevent(w,res)
wbp w;
dptr res;
   {
   struct descrip xdesc, ydesc;
   uword i;

   if (wgetq(w,res) == -1)
      return -1;					/* window died */
   if (BlkLoc(w->window->listp)->list.size < 2)
      return -2;					/* malformed queue */

   wgetq(w,&xdesc);
   wgetq(w,&ydesc);

   if (xdesc.dword != D_Integer || ydesc.dword != D_Integer)
      return -2;			/* bad values on queue */

   IntVal(amperX) = IntVal(xdesc) & 0xFFFF;		/* &x */
   if (IntVal(amperX) >= 0x8000)
      IntVal(amperX) -= 0x10000;
   IntVal(amperY) = IntVal(ydesc) & 0xFFFF;		/* &y */
   if (IntVal(amperY) >= 0x8000)
      IntVal(amperY) -= 0x10000;
   IntVal(amperX) -= w->context->dx;
   IntVal(amperY) -= w->context->dy;
   MakeInt(1 + XTOCOL(w,IntVal(amperX)), &(amperCol));	/* &col */
   MakeInt(YTOROW(w,IntVal(amperY)) , &(amperRow));	/* &row */

   xmod_control = IntVal(xdesc) & EQ_MOD_CONTROL;	/* &control */
   xmod_meta = IntVal(xdesc) & EQ_MOD_META;		/* &meta */
   xmod_shift = IntVal(xdesc) & EQ_MOD_SHIFT;		/* &shift */

   i = (((uword) IntVal(ydesc)) >> 16) & 0xFFF;		/* mantissa */
   i <<= 4 * ((((uword) IntVal(ydesc)) >> 28) & 0x7);	/* scale it */
   IntVal(amperInterval) = i;				/* &interval */
   return 0;
   }

/*
 * get event from window (drop mouse events), no echo
 *
 * returns 1 for success, -1 if window died, -2 for malformed queue
 */
int wgetchne(w,res)
wbp w;
dptr res;
   {
   int i;

   while (1) {
      i = wgetevent(w,res);
      if (i != 0)
	 return i;
      if (is:string(*res))
         return 1;
      }
   }

/*
 * get event from window (drop mouse events), with echo
 *
 * returns 1 for success, -1 if window died, -2 for malformed queue
 */
int wgetche(w,res)
wbp w;
dptr res;
   {
   int i;
   i = wgetchne(w,res);
   if (i != 1)
      return i;
   i = *StrLoc(*res);
   if ((0 <= i) && (i <= 127) && (ISECHOON(w))) {
#ifndef PresentationManager
      wputc(i, w);
      if (i == '\r') wputc((int)'\n', w); /* CR -> CR/LF */
#else					/* PresentationManager */
     wputc(((i == '\r') ? '\n' : i), w);
#endif					/* PresentationManager */
      }
   return 1;
   }

/*
 * go to row,col within window w.  updates state variables only.
 * A cursor, if any, is drawn by event-reading code.
 */
novalue wgoto(w,row,col)
wbp w;
int row, col;
   {
   w->window->x = COLTOX(w, col);
   w->window->y = ROWTOY(w, row);
   }


/*
 * strncpy with guaranteed NUL termination
 */
novalue mystrncpy(dest,src,n)
char *dest, *src;
int n;
   {
   strncpy(dest,src,n);
   dest[n] = '\0';
   }

/*
 * wlongread(s,elsize,nelem,f) -- read string from window for reads(w)
 *
 * returns length(>=0) for success, -1 if window died, -2 for malformed queue
 */
int wlongread(s, elsize, nelem, f)
char *s;
int elsize, nelem;
FILE *f;
   {
   int c;
   tended char *ts = s;
   struct descrip foo;
   long l = 0, bytes = elsize * nelem;

   while (l < bytes) {
     c = wgetche((wbp)f, &foo);
     if (c < 0)
	return c;
     c = *StrLoc(foo);
     switch(c) {
       case '\177':
       case '\010':
         if (l > 0) { ts--; l--; }
         break;
       default:
         *ts++ = c; l++;
         break;
       } /* End of switch - type of character */
     } /* End of while - go until full */
   return l;
   } /* End of wlongread */

/*
 * wgetstrg(s,maxlen,f) -- get string from window for read(w) or !w
 *
 * returns length(>=0) for success, -1 if window died, -2 for malformed queue
 */
int wgetstrg(s, maxlen, f)
char *s;
long  maxlen;
FILE *f;
   {
   int c;
   tended char *ts = s;
   long l = 0;
   struct descrip foo;

   while (l < maxlen) {
      c = wgetche((wbp)f,&foo);
      if (c < 0)
	 return c;
      c = *StrLoc(foo);
      switch(c) {
        case '\177':
        case '\010':
          if (l > 0) { ts--; l--; }
          break;
        case '\r':
        case '\n':
          return l;
        default:
          *ts++ = c; l++;
          break;
        } /* End of switch - type of character */
      } /* End of while - not full */
   return l;
   }


/*
 * Assignment side-effects for &x,&y,&row,&col
 */
int xyrowcol(dx)
dptr dx;
{
   if (VarLoc(*dx) == &amperX) { /* update &col too */
      wbp w;
      if (!is:file(lastEventWin))
	 return -1;
      w = (wbp)BlkLoc(lastEventWin)->file.fd;
      MakeInt(1 + XTOCOL(w, IntVal(amperX)), &amperCol);
      }
   else if (VarLoc(*dx) == &amperY) { /* update &row too */
      wbp w;
      if (!is:file(lastEventWin))
	 return -1;
      w = (wbp)BlkLoc(lastEventWin)->file.fd;
      MakeInt(YTOROW(w, IntVal(amperY)), &amperRow);
      }
   else if (VarLoc(*dx) == &amperCol) { /* update &x too */
      wbp w;
      if (!is:file(lastEventWin))
	 return -1;
      w = (wbp)BlkLoc(lastEventWin)->file.fd;
      MakeInt(COLTOX(w, IntVal(amperCol)), &amperX);
      }
   else if (VarLoc(*dx) == &amperRow) { /* update &y too */
      wbp w;
      if (!is:file(lastEventWin))
	 return -1;
      w = (wbp)BlkLoc(lastEventWin)->file.fd;
      MakeInt(ROWTOY(w, IntVal(amperRow)), &amperY);
      }
   return 0;
   }


/*
 * Enqueue an event, encoding time interval and key state with x and y values.
 */
novalue qevent(q,e,x,y,t,f)
dptr q;		/* a window's event queue (Icon list value) */
dptr e;		/* event code (descriptor pointer) */
int x, y;	/* x and y values */
uword t;	/* ms clock value */
int f;		/* modifier key flags */
   {
   struct descrip d;
   uword ivl, mod;
   int expo;

   mod = 0;				/* set modifier key bits */
   if (f & ControlMask) mod |= EQ_MOD_CONTROL;
   if (f & Mod1Mask)    mod |= EQ_MOD_META;
   if (f & ShiftMask)   mod |= EQ_MOD_SHIFT;

   if (t != ~(uword)0) {		/* if clock value supplied */
      if (prevtimestamp == 0)		/* if first time */
	 prevtimestamp = t;
      if (t < prevtimestamp)		/* if clock went backwards */
	 t = prevtimestamp;
      ivl = t - prevtimestamp;		/* calc interval in milliseconds */
      prevtimestamp = t;		/* save new clock value */
      expo = 0;
      while (ivl >= 0x1000) {		/* if too big */
	 ivl >>= 4;			/* reduce significance */
	 expo += 0x1000;		/* bump exponent */
	 }
      ivl += expo;			/* combine exponent with mantissa */
      }
   else
      ivl = 0;				/* report 0 if interval unknown */

   c_put(q, e);
   d.dword = D_Integer;
   IntVal(d) = mod | (x & 0xFFFF);
   c_put(q, &d);
   IntVal(d) = (ivl << 16) | (y & 0xFFFF);
   c_put(q, &d);
   }

/*
 * setpos() - set (move) canvas position on the screen
 */
static int setpos(w,s)
wbp w;
char *s;
   {
   char *s2, tmp[32];
   int posx, posy;
   STDLOCALS(w);

   s2 = s;
   while (isspace(*s2)) s2++;
   if (!isdigit(*s2) && (*s2 != '-')) return Error;
   posx = atol(s2);
   if (*s2 == '-') s2++;
   while (isdigit(*s2)) s2++;
   if (*s2++ != ',') return Error;
   posy = atol(s2);
   if (posx < 0) {
      if (posy < 0) sprintf(tmp,"%d%d",posx,posy);
      else sprintf(tmp,"%d+%d",posx,posy);
      }
   else {
      if (posy < 0) sprintf(tmp,"+%d%d",posx,posy);
      else sprintf(tmp,"+%d+%d",posx,posy);
      }
   return setgeometry(w,tmp);
   }

/*
 * setsize() - set canvas size
 */
int setsize(w,s)
wbp w;
char *s;
   {
   char *s2, tmp[32];
   int width, height;
   STDLOCALS(w);

   s2 = s;
   while (isspace(*s2)) s2++;
   if (!isdigit(*s2) && (*s2 != '-')) return Error;
   width = atol(s2);
   if (*s2 == '-') s2++;
   while (isdigit(*s2)) s2++;
   if (*s2++ != ',') return Error;
   height = atol(s2);
   sprintf(tmp,"%dx%d",width,height);
   return setgeometry(w,tmp);
   }



/*
 * put a string out to a window using the current attributes
 */
novalue wputstr(w,s,len)
wbp w;
char *s;
int len;
   {
  char *s2 = s;
  wstate *ws = w->window;
  /* turn off the cursor */
  HideCursor(ws);

  while (len > 0) {
    /* find a chunk of printable text */
    while (isprint(*s2) && len > 0) { s2++; len--; }
    /* if a chunk was parsed, write it out */
    if (s2 != s)
       xdis(w, s, s2 - s);
    /* put the 'unprintable' character, if didn't just hit the end */
    if (len-- > 0)
       wputc(*s2++, w);
    s = s2;
    }

  /* show the cursor again */
  UpdateCursorPos(ws, w->context);
  ShowCursor(ws);
  return;
} /* End of wputstr */


/*
 * Structures and tables used for color parsing.
 *  Tables must be kept lexically sorted.
 */

typedef struct {	/* color name entry */
   char name[8];	/* basic color name */
   char ish[12];	/* -ish form */
   short hue;		/* hue, in degrees */
   char lgt;		/* lightness, as percentage */
   char sat;		/* saturation, as percentage */
} colrname;

typedef struct {	/* arbitrary lookup entry */
   char word[10];	/* word */
   char val;		/* value, as percentage */
} colrmod;

static colrname colortable[] = {		/* known colors */
   /* color       ish-form     hue  lgt  sat */
   { "black",    "blackish",     0,   0,   0 },
   { "blue",     "bluish",     240,  50, 100 },
   { "brown",    "brownish",    30,  25, 100 },
   { "cyan",     "cyanish",    180,  50, 100 },
   { "gray",     "grayish",      0,  50,   0 },
   { "green",    "greenish",   120,  50, 100 },
   { "grey",     "greyish",      0,  50,   0 },
   { "magenta",  "magentaish", 300,  50, 100 },
   { "orange",   "orangish",    15,  50, 100 },
   { "pink",     "pinkish",    345,  75, 100 },
   { "purple",   "purplish",   270,  50, 100 },
   { "red",      "reddish",      0,  50, 100 },
   { "violet",   "violetish",  270,  75, 100 },
   { "white",    "whitish",      0, 100,   0 },
   { "yellow",   "yellowish",   60,  50, 100 },
   };

static colrmod lighttable[] = {			/* lightness modifiers */
   { "dark",       0 },
   { "deep",       0 },		/* = very dark (see code) */
   { "light",    100 },
   { "medium",    50 },
   { "pale",     100 },		/* = very light (see code) */
   };

static colrmod sattable[] = {			/* saturation levels */
   { "moderate",  50 },
   { "strong",    75 },
   { "vivid",    100 },
   { "weak",      25 },
   };

/*
 *  parsecolor(w, s, &r, &g, &b) - parse a color specification
 *
 *  parsecolor interprets a color specification and produces r/g/b values
 *  scaled linearly from 0 to 65535.  parsecolor returns Succeeded or Error.
 *
 *  An Icon color specification can be any of the forms
 *
 *     #rgb			(hexadecimal digits)
 *     #rrggbb
 *     #rrrgggbbb
 *     #rrrrggggbbbb
 *     nnnnn,nnnnn,nnnnn	(integers 0 - 65535)
 *     <Icon color phrase>
 *     <native color spec>
 */

int parsecolor(w, buf, r, g, b)
wbp w;
char *buf;
int *r, *g, *b;
   {
   int len, mul;
   char *fmt, c, cbuffer[MAXCOLORNAME];

   *r = *g = *b = 0;

   /* trim leading spaces */
   while (isspace(*buf))
      buf++;

   /* try interpreting as three comma-separated integers */
   if (sscanf(buf, "%d,%d,%d%c", r, g, b, &c) == 3) {
      if (*r>=0 && *r<=65535 && *g>=0 && *g<=65535 && *b>=0 && *b<=65535)
         return Succeeded;
      else
         return Error;
      }

   /* try interpreting as a hexadecimal value */
   if (*buf == '#') {
      buf++;
      for (len = 0; isalnum(buf[len]); len++);
      switch (len) {
         case  3:  fmt = "%1x%1x%1x%c";  mul = 0x1111;  break;
         case  6:  fmt = "%2x%2x%2x%c";  mul = 0x0101;  break;
         case  9:  fmt = "%3x%3x%3x%c";  mul = 0x0010;  break;
         case 12:  fmt = "%4x%4x%4x%c";  mul = 0x0001;  break;
         default:  return Error;
      }
      if (sscanf(buf, fmt, r, g, b, &c) != 3)
         return Error;
      *r *= mul;
      *g *= mul;
      *b *= mul;
      return Succeeded;
      }

   /* try interpreting as a color phrase or as a native color spec */
   if (colorphrase(buf, r, g, b) || nativecolor(w, buf, r, g, b))
      return Succeeded;
   else
      return Failed;
   }

/*
 *  colorphrase(s, &r, &g, &b) -- parse Icon color phrase.
 *
 *  An Icon color phrase matches the pattern
 *
 *                               weak
 *                  pale         moderate
 *                  light        strong
 *          [[very] medium ]   [ vivid    ]   [color[ish]]   color
 *                  dark 
 *                  deep
 *
 *  where "color" is any of:
 *
 *          black gray grey white pink violet brown
 *          red orange yellow green cyan blue purple magenta
 *
 *  A single space or hyphen separates each word from its neighbor.  The
 *  default lightness is "medium", and the default saturation is "vivid".
 *
 *  "pale" means "very light"; "deep" means "very dark".
 *
 *  This naming scheme is based loosely on
 *	A New Color-Naming System for Graphics Languages
 *	Toby Berk, Lee Brownston, and Arie Kaufman
 *	IEEE Computer Graphics & Applications, May 1982
 */

static int colorphrase(buf, r, g, b)
char *buf; 
int *r, *g, *b;
   {
   int len, very;
   char *p, *ebuf, cbuffer[MAXCOLORNAME];
   float lgt, sat, blend, bl2, m1, m2;
   float h1, l1, s1, h2, l2, s2, r2, g2, b2;

   lgt = -1.0;				/* default no lightness mod */
   sat =  1.0;				/* default vivid saturation */
   len = strlen(buf);
   while (isspace(buf[len-1]))
      len--;				/* trim trailing spaces */

   /* copy spec, replacing spaces and hyphens with NULs */
   if (len >= sizeof(cbuffer))
      return 0;				/* if too long for valid Icon spec */
   strncpy(cbuffer, buf, len);
   ebuf = cbuffer + len;
   for (buf = cbuffer; buf < ebuf; buf++)
      if (*buf == ' ' || *buf == '-')
         *buf = '\0';
   *ebuf = '\0';
   buf = cbuffer;

   /* check for "very" */
   if (strcmp(buf, "very") == 0) {
      very = 1;
      buf += strlen(buf) + 1;
      if (buf >= ebuf)
         return 0;
      }
   else
      very = 0;

   /* check for lightness adjective */
   p = qsearch(buf, (char *)lighttable,
      ElemCount(lighttable), ElemSize(lighttable), strcmp);
   if (p) {
      lgt = ((colrmod *)p) -> val / 100.0;
      /* check for other effects */
      if (strcmp(buf, "pale") == 0)
         very = 1;			/* pale = very light */
      else if (strcmp(buf, "deep") == 0)
         very = 1;			/* deep = very dark */
      /* skip past word */
      buf += strlen(buf) + 1;
      if (buf >= ebuf)
         return 0;
      }
   else if (very)
      return 0;

   /* check for saturation adjective */
   p = qsearch(buf, (char *)sattable,
      ElemCount(sattable), ElemSize(sattable), strcmp);
   if (p) {
      sat = ((colrmod *)p) -> val / 100.0;
      buf += strlen(buf) + 1;
      if (buf >= ebuf)
         return 0;
      }

   if (buf + strlen(buf) >= ebuf)
      blend = 0.0;			/* only one word left */
   else {
      /* we have two (or more) name words; get the first */
      if (p = qsearch(buf, colortable[0].name,
            ElemCount(colortable), ElemSize(colortable), strcmp)) {
         blend = 0.5;
         }
      else if (p = qsearch(buf, colortable[0].ish,
            ElemCount(colortable), ElemSize(colortable), strcmp)) {
         p -= sizeof(colortable[0].name);
         blend = 0.25;
         }
      else
         return 0;

      h1 = ((colrname *)p) -> hue;
      l1 = ((colrname *)p) -> lgt / 100.0;
      s1 = ((colrname *)p) -> sat / 100.0;
      buf += strlen(buf) + 1;
      }

   /* process second (or only) name word */
   p = qsearch(buf, colortable[0].name,
      ElemCount(colortable), ElemSize(colortable), strcmp);
   if (!p || buf + strlen(buf) < ebuf)
      return 0;
   h2 = ((colrname *)p) -> hue;
   l2 = ((colrname *)p) -> lgt / 100.0;
   s2 = ((colrname *)p) -> sat / 100.0;

   /* at this point we know we have a valid spec */

   /* interpolate hls specs */
   if (blend > 0) {
      bl2 = 1.0 - blend;
   
      if (s1 == 0.0)
         ; /* use h2 unchanged */
      else if (s2 == 0.0)
         h2 = h1;
      else if (h2 - h1 > 180)
         h2 = blend * h1 + bl2 * (h2 - 360);
      else if (h1 - h2 > 180)
         h2 = blend * (h1 - 360) + bl2 * h2;
      else
         h2 = blend * h1 + bl2 * h2;
      if (h2 < 0)
         h2 += 360;
   
      l2 = blend * l1 + bl2 * l2;
      s2 = blend * s1 + bl2 * s2;
      }

   /* apply saturation and lightness modifiers */
   if (lgt >= 0.0) {
      if (very)
         l2 = (2 * lgt + l2) / 3.0;
      else
         l2 = (lgt + 2 * l2) / 3.0;
      }
   s2 *= sat;

   /* convert h2,l2,s2 to r2,g2,b2 */
   /* from Foley & Van Dam, 1st edition, p. 619 */
   /* beware of dangerous typos in 2nd edition */
   if (s2 == 0)
      r2 = g2 = b2 = l2;
   else {
      if (l2 < 0.5)
         m2 = l2 * (1 + s2);
      else
         m2 = l2 + s2 - l2 * s2;
      m1 = 2 * l2 - m2;
      r2 = rgbval(m1, m2, h2 + 120);
      g2 = rgbval(m1, m2, h2);
      b2 = rgbval(m1, m2, h2 - 120);
      }

   /* scale and convert the calculated result */
   *r = 65535 * r2;
   *g = 65535 * g2;
   *b = 65535 * b2;

   return 1;
   }

/*
 * rgbval(n1, n2, hue) - helper function for HLS to RGB conversion
 */
static double rgbval(n1, n2, hue)
double n1, n2, hue;
   {
   if (hue > 360)
      hue -= 360;
   else if (hue < 0)
      hue += 360;

   if (hue < 60)
      return n1 + (n2 - n1) * hue / 60.0;
   else if (hue < 180)
      return n2;
   else if (hue < 240)
      return n1 + (n2 - n1) * (240 - hue) / 60.0;
   else
      return n1;
   }

/*
 *  Functions and data for reading and writing GIF images
 */

#define GifSeparator	0x2C	/* (',') beginning of image */
#define GifTerminator	0x3B	/* (';') end of image */
#define GifExtension	0x21	/* ('!') extension block */
#define GifEmpty	-1	/* internal flag indicating no prefix */

#define GifTableSize	4096	/* maximum number of entries in table */
#define GifBlockSize	255	/* size of output block */

typedef struct lzwnode {	/* structure of LZW encoding tree node */
   unsigned short tcode;		/* token code */
   unsigned short child;	/* first child node */
   unsigned short sibling;	/* next sibling */
   } lzwnode;

hidden	int	gfread		Params((char *fn, int p));
hidden	int	gfheader	Params((FILE *f));
hidden	int	gfskip		Params((FILE *f));
hidden	int	gfimhdr		Params((FILE *f));
hidden	int	gfmap		Params((FILE *f, int p));
hidden	int	gfsetup		Params((noargs));
hidden	int	gfrdata		Params((FILE *f));
hidden	int	gfrcode		Params((FILE *f));
hidden	novalue	gfinsert	Params((int prev, int c));
hidden	int	gffirst		Params((int c));
hidden	novalue	gfgen		Params((int c));
hidden	novalue	gfput		Params((int b));

hidden	int	gfwrite		Params((wbp w, char *filename,
				   int x, int y, int width, int height));
hidden	novalue	gfpack		Params((unsigned char *data, long len, 
				   struct palentry *paltbl));
hidden	novalue	gfmktree	Params((lzwnode *tree));
hidden	novalue	gfout		Params((int tcode));
hidden	novalue	gfdump		Params((noargs));

static FILE *gf_f;			/* input file */

static int gf_gcmap, gf_lcmap;		/* global color map? local color map? */
static int gf_nbits;			/* number of bits per pixel */
static int gf_ilace;			/* interlace flag */
static int gf_width, gf_height;		/* image size */

static short *gf_prefix, *gf_suffix;	/* prefix and suffix tables */
static int gf_free;			/* next free position */

static struct palentry *gf_paltbl;	/* palette table */
static unsigned char *gf_string;	/* image string */
static unsigned char *gf_nxt, *gf_lim;	/* store pointer and its limit */
static int gf_row, gf_step;		/* current row and step size */

static int gf_cdsize;			/* code size */
static int gf_clear, gf_eoi;		/* values of CLEAR and EOI codes */
static int gf_lzwbits, gf_lzwmask;	/* current bits per code */

static unsigned char *gf_obuf;		/* output buffer */
static unsigned long gf_curr;		/* current partial byte(s) */
static int gf_valid;			/* number of valid bits */
static int gf_rem;			/* remaining bytes in this block */

/*
 * readGIF(filename, p, imd) - read GIF file into image data structure
 *
 * p is a palette number to which the GIF colors are to be coerced;
 * p=0 uses the colors exactly as given in the GIF file.
 */
int readGIF(filename, p, imd)
char *filename;
int p;
struct imgdata *imd;
   {
   int r;

   r = gfread(filename, p);			/* read image */

   if (gf_prefix) free((pointer)gf_prefix);	/* deallocate temp memory */
   if (gf_suffix) free((pointer)gf_suffix);
   if (gf_f) fclose(gf_f);

   if (r != Succeeded) {			/* if no success, free mem */
      if (gf_paltbl) free((pointer) gf_paltbl);
      if (gf_string) free((pointer) gf_string);
      return r;					/* return Failed or Error */
      }

   imd->width = gf_width;			/* set return variables */
   imd->height = gf_height;
   imd->paltbl = gf_paltbl;
   imd->data = gf_string;
   return Succeeded;				/* return success */
   }

/*
 * gfread(filename, p) - read GIF file, setting gf_ globals
 */
static int gfread(filename, p)
char *filename;
int p;
   {
   int i;

   gf_f = NULL;
   gf_prefix = NULL;
   gf_suffix = NULL;
   gf_string = NULL;

   if (!(gf_paltbl = (struct palentry *)malloc(256 * sizeof(struct palentry))))
      return Failed;

   if ((gf_f = fopen(filename, "r")) == NULL)
      return Failed;

   for (i = 0; i < 256; i++)		/* init palette table */
      gf_paltbl[i].valid = gf_paltbl[i].used = 0;

   if (!gfheader(gf_f))			/* read file header */
      return Failed;
   if (gf_gcmap)			/* read global color map, if any */
      if (!gfmap(gf_f, p))
         return Failed;
   if (!gfskip(gf_f))			/* skip to start of image */
      return Failed;
   if (!gfimhdr(gf_f))			/* read image header */
      return Failed;
   if (gf_lcmap)			/* read local color map, if any */
      if (!gfmap(gf_f, p))
         return Failed;
   if (!gfsetup())			/* prepare to read image */
      return Error;
   if (!gfrdata(gf_f))			/* read image data */
      return Failed;

   return Succeeded;
   }

/*
 * gfheader(f) - read GIF file header; return nonzero if successful
 */
static int gfheader(f)
FILE *f;
   {
   unsigned char hdr[13];		/* size of a GIF header */
   int b;

   if (fread((char *)hdr, sizeof(char), sizeof(hdr), f) != sizeof(hdr))
      return 0;				/* header short or missing */
   if (strncmp((char *)hdr, "GIF", 3) != 0 ||
         !isdigit(hdr[3]) || !isdigit(hdr[4]))
      return 0;				/* not GIFnn */

   b = hdr[10];				/* flag byte */
   gf_gcmap = b & 0x80;			/* global color map flag */
   gf_nbits = (b & 7) + 1;		/* number of bits per pixel */
   return 1;
   }

/*
 * gfskip(f) - skip intermediate blocks and locate image
 */
static int gfskip(f)
FILE *f;
   {
   int c, n;

   while ((c = getc(f)) != GifSeparator) { /* look for start-of-image flag */
      if (c == EOF)
         return 0;
      if (c == GifExtension) {		/* if extension block is present */
         getc(f);				/* skip label */
         while ((n = getc(f)) != 0) {		/* read blks until empty one */
            if (n == EOF)
               return 0;
            while (n--)				/* skip block contents */
               getc(f);
            }
         }
      }
   return 1;
   }

/*
 * gfimhdr(f) - read image header
 */
static int gfimhdr(f)
FILE *f;
   {
   unsigned char hdr[9];		/* size of image hdr excl separator */
   int b;

   if (fread((char *)hdr, sizeof(char), sizeof(hdr), f) != sizeof(hdr))
      return 0;				/* header short or missing */
   gf_width = hdr[4] + 256 * hdr[5];
   gf_height = hdr[6] + 256 * hdr[7];
   b = hdr[8];				/* flag byte */
   gf_lcmap = b & 0x80;			/* local color map flag */
   gf_ilace = b & 0x40;			/* interlace flag */
   if (gf_lcmap)
      gf_nbits = (b & 7) + 1;		/* if local map, reset nbits also */
   return 1;
   }

/*
 * gfmap(f, p) - read GIF color map into paltbl under control of palette p
 */
static int gfmap(f, p)
FILE *f;
int p;
   {
   int ncolors, i, r, g, b, c;
   struct palentry *stdpal = 0;

   if (p)
      stdpal = palsetup(p);

   ncolors = 1 << gf_nbits;

   for (i = 0; i < ncolors; i++) {
      r = getc(f);
      g = getc(f);
      b = getc(f);
      if (r == EOF || g == EOF || b == EOF)
         return 0;
      if (p) {
         c = *(unsigned char *)(rgbkey(p, r / 255.0, g / 255.0, b / 255.0));
         gf_paltbl[i].clr = stdpal[c].clr;
         }
      else {
         gf_paltbl[i].clr.red   = 257 * r;	/* 257 * 255 -> 65535 */
         gf_paltbl[i].clr.green = 257 * g;
         gf_paltbl[i].clr.blue  = 257 * b;
         }
      gf_paltbl[i].valid = 1;
      }

   return 1;
   }

/*
 * gfsetup() - prepare to read GIF data
 */
static int gfsetup()
   {
   int i;
   word len;

   len = (word)gf_width * (word)gf_height;
   gf_string = (unsigned char *)malloc((msize)len);
   gf_prefix = (short *)malloc(GifTableSize * sizeof(short));
   gf_suffix = (short *)malloc(GifTableSize * sizeof(short));
   if (!gf_string || !gf_prefix || !gf_suffix)
      return 0;
   for (i = 0; i < GifTableSize; i++) {
      gf_prefix[i] = GifEmpty;
      gf_suffix[i] = i;
      }

   gf_row = 0;				/* current row is 0 */
   gf_nxt = gf_string;			/* set store pointer */

   if (gf_ilace) {			/* if interlaced */
      gf_step = 8;			/* step rows by 8 */
      gf_lim = gf_string + gf_width;	/* stop at end of one row */
      }
   else {
      gf_step = 1;			/* step by one */
      gf_lim = gf_string + len;		/* do whole image at once */
      }

   return 1;
   }

/*
 * gfrdata(f) - read GIF data
 */
static int gfrdata(f)
FILE *f;
   {
   int curr, prev, c;

   if ((gf_cdsize = getc(f)) == EOF)
      return 0;
   gf_clear = 1 << gf_cdsize;
   gf_eoi = gf_clear + 1;
   gf_free = gf_eoi + 1;

   gf_lzwbits = gf_cdsize + 1;
   gf_lzwmask = (1 << gf_lzwbits) - 1;

   gf_curr = 0;
   gf_valid = 0;
   gf_rem = 0;

   prev = curr = gfrcode(f);
   while (curr != gf_eoi) {
      if (curr == gf_clear) {		/* if reset code */
         gf_lzwbits = gf_cdsize + 1;
         gf_lzwmask = (1 << gf_lzwbits) - 1;
         gf_free = gf_eoi + 1;
         prev = curr = gfrcode(f);
         gfgen(curr);
         }
      else if (curr < gf_free) {	/* if code is in table */
         gfgen(curr);
         gfinsert(prev, gffirst(curr));
         prev = curr;
         }
      else if (curr == gf_free) {	/* not yet in table */
         c = gffirst(prev);
         gfgen(prev);
         gfput(c);
         gfinsert(prev, c);
         prev = curr;
         }
      else {				/* illegal code */
         if (gf_nxt == gf_lim)
            return 1;			/* assume just extra stuff after end */
         else
            return 0;			/* more badly confused */
         }
      curr = gfrcode(f);
      }

   return 1;
   }

/*
 * gfrcode(f) - read next LZW code
 */
static int gfrcode(f)
FILE *f;
   {
   int c, r;

   while (gf_valid < gf_lzwbits) {
      if (--gf_rem <= 0) {
         if ((gf_rem = getc(f)) == EOF)
            return gf_eoi;
         }
      if ((c = getc(f)) == EOF)
         return gf_eoi;
      gf_curr |= ((c & 0xFF) << gf_valid);
      gf_valid += 8;
      }
   r = gf_curr & gf_lzwmask;
   gf_curr >>= gf_lzwbits;
   gf_valid -= gf_lzwbits;
   return r;
   }

/*
 * gfinsert(prev, c) - insert into table
 */
static novalue gfinsert(prev, c)
int prev, c;
   {

   if (gf_free >= GifTableSize)		/* sanity check */
      return;

   gf_prefix[gf_free] = prev;
   gf_suffix[gf_free] = c;

   /* increase code size if code bits are exhausted, up to max of 12 bits */
   if (++gf_free > gf_lzwmask && gf_lzwbits < 12) {
      gf_lzwmask = gf_lzwmask * 2 + 1;
      gf_lzwbits++;
      }

   }

/*
 * gffirst(c) - return the first pixel in a map structure
 */
static int gffirst(c)
int c;
   {
   int d;

   if (c >= gf_free)
      return 0;				/* not in table (error) */
   while ((d = gf_prefix[c]) != GifEmpty)
      c = d;
   return gf_suffix[c];
   }

/*
 * gfgen(c) - generate and output prefix
 */
static novalue gfgen(c)
   {
   int d;

   if ((d = gf_prefix[c]) != GifEmpty)
      gfgen(d);
   gfput(gf_suffix[c]);
   }

/*
 * gfput(b) - add a byte to the output string
 */
static novalue gfput(b)
int b;
   {
   if (gf_nxt >= gf_lim) {		/* if current row is full */
      if ((gf_row += gf_step) >= gf_height) {
         if (gf_step == 8) {
            if ((gf_row % 8) == 0)
               gf_row = 4;
            else {
               gf_row = 2;
               gf_step = 4;
               }
            }
         else if (gf_step == 4) {
            gf_row = 1;
            gf_step = 2;
            }
         else {
            /* shouldn't happen */
            return;
            }
         }
      gf_nxt = gf_string + ((word)gf_row * (word)gf_width);
      gf_lim = gf_nxt + gf_width;
      }

   *gf_nxt++ = b;			/* store byte */
   gf_paltbl[b].used = 1;		/* mark color entry as used */
   }

/*
 * writeGIF(w, filename, x, y, width, height) - write GIF image
 *
 * Returns Succeeded, Failed, or Error.
 * We assume that the area specified is within the window.
 */
int writeGIF(w, filename, x, y, width, height)
wbp w;
char *filename;
int x, y, width, height;
   {
   int r;

   r = gfwrite(w, filename, x, y, width, height);
   if (gf_f) fclose(gf_f);
   if (gf_string) free((pointer)gf_string);
   return r;
   }

/*
 * gfwrite(w, filename, x, y, width, height) - write GIF file
 *
 * We write GIF87a format (not 89a) for maximum acceptability and because
 * we don't need any of the extensions of GIF89.
 */

static int gfwrite(w, filename, x, y, width, height)
wbp w;
char *filename;
int x, y, width, height;
   {
   int i, c, cur;
   long len;
   LinearColor *cp;
   unsigned char *p, *q;
   struct palentry paltbl[DMAXCOLORS];
   unsigned char obuf[GifBlockSize];
   lzwnode tree[GifTableSize + 1];

   len = (long)width * (long)height;	/* total length of data */

   if (!(gf_f = fopen(filename, "wb")))
      return Failed;
   if (!(gf_string = (unsigned char*)malloc((msize)len)))
      return Error;

   for (i = 0; i < DMAXCOLORS; i++)
      paltbl[i].used = paltbl[i].valid = 0;
   if (!getimstr(w, x, y, width, height, paltbl, gf_string))
      return Error;

   gfpack(gf_string, len, paltbl);	/* pack color table, set color params */

   gf_clear = 1 << gf_cdsize;		/* set encoding variables */
   gf_eoi = gf_clear + 1;
   gf_free = gf_eoi + 1;
   gf_lzwbits = gf_cdsize + 1;

   /* 
    * Write the header, global color table, and image descriptor.
    */

   fprintf(gf_f, "GIF87a%c%c%c%c%c%c%c", width, width >> 8, height, height >> 8,
      0x80 | ((gf_nbits - 1) << 4) | (gf_nbits - 1), 0, 0);


   for (i = 0; i < (1 << gf_nbits); i++) {	/* output color table */
      if (i < DMAXCOLORS && paltbl[i].valid) {
         cp = &paltbl[i].clr;
         putc(cp->red >> 8, gf_f);
         putc(cp->green >> 8, gf_f);
         putc(cp->blue >> 8, gf_f);
         }
      else {
         putc(0, gf_f);
         putc(0, gf_f);
         putc(0, gf_f);
         }
      }

   fprintf(gf_f, "%c%c%c%c%c%c%c%c%c%c%c", GifSeparator, 0, 0, 0, 0,
      width, width >> 8, height, height >> 8, gf_nbits - 1, gf_cdsize);

   /*
    * Encode and write the image.
    */
   gf_obuf = obuf;			/* initialize output state */
   gf_curr = 0;
   gf_valid = 0;
   gf_rem = GifBlockSize;

   gfmktree(tree);			/* initialize encoding tree */

   gfout(gf_clear);			/* start with CLEAR code */

   p = gf_string;
   q = p + len;
   cur = *p++;				/* first pixel is special */
   while (p < q) {
      c = *p++;				/* get code */
      for (i = tree[cur].child; i != 0; i = tree[i].sibling)
         if (tree[i].tcode == c)	/* find as suffix of previous string */
            break;
      if (i != 0) {			/* if found in encoding tree */
         cur = i;			/* note where */
         continue;			/* and accumulate more */
         }
      gfout(cur);			/* new combination -- output prefix */
      tree[gf_free].tcode = c;		/* make node for new combination */
      tree[gf_free].child = 0;
      tree[gf_free].sibling = tree[cur].child;
      tree[cur].child = gf_free;
      cur = c;				/* restart string from single pixel */
      ++gf_free;			/* grow tree to account for new node */
      if (gf_free > (1 << gf_lzwbits)) {
         if (gf_free > GifTableSize) {
            gfout(gf_clear);		/* table is full; reset to empty */
            gf_lzwbits = gf_cdsize + 1;
            gfmktree(tree);
            }
         else
            gf_lzwbits++;		/* time to make output one bit wider */
         }
      }

   /*
    * Finish up.
    */
   gfout(cur);				/* flush accumulated prefix */
   gfout(gf_eoi);			/* send EOI code */
   gf_lzwbits = 7;
   gfout(0);				/* force out last partial byte */
   gfdump();				/* dump final block */
   putc(0, gf_f);			/* terminate image (block of size 0) */
   putc(GifTerminator, gf_f);		/* terminate file */

   fflush(gf_f);
   if (ferror(gf_f))
      return Failed;
   else
      return Succeeded;			/* caller will close file */
   }

/*
 * gfpack() - pack palette table to eliminate gaps
 *
 * Sets gf_nbits and gf_cdsize based on the number of colors.
 */
static novalue gfpack(data, len, paltbl)
unsigned char *data;
long len;
struct palentry *paltbl;
   {
   int i, ncolors, lastcolor;
   unsigned char *p, *q, cmap[DMAXCOLORS];

   ncolors = 0;
   lastcolor = 0;
   for (i = 0; i < DMAXCOLORS; i++)
      if (paltbl[i].used) {
         lastcolor = i;
         cmap[i] = ncolors;		/* mapping to output color */
         if (i != ncolors) {
            paltbl[ncolors] = paltbl[i];		/* shift down */
            paltbl[i].used = paltbl[i].valid = 0;	/* invalidate old */
            }
         ncolors++;
         }

   if (ncolors < lastcolor + 1) {	/* if entries were moved to fill gaps */
      p = data;
      q = p + len;
      while (p < q) {
         *p = cmap[*p];			/* adjust color values in data string */
         p++;
         }
      }

   gf_nbits = 1;
   while ((1 << gf_nbits) < ncolors)
      gf_nbits++;
   if (gf_nbits < 2)
      gf_cdsize = 2;
   else
      gf_cdsize = gf_nbits;
   }

/*
 * gfmktree() - initialize or reinitialize encoding tree
 */

hidden novalue gfmktree(tree)
lzwnode *tree;
   {
   int i;

   for (i = 0; i < gf_clear; i++) {	/* for each basic entry */
      tree[i].tcode = i;			/* code is pixel value */
      tree[i].child = 0;		/* no suffixes yet */
      tree[i].sibling = i + 1;		/* next code is sibling */
      }
   tree[gf_clear - 1].sibling = 0;	/* last entry has no sibling */
   gf_free = gf_eoi + 1;		/* reset next free entry */
   }

/*
 * gfout(code) - output one LZW token
 */
hidden novalue gfout(tcode)
int tcode;
   {
   gf_curr |= tcode << gf_valid;		/* add to current word */
   gf_valid += gf_lzwbits;		/* count the bits */
   while (gf_valid >= 8) {		/* while we have a byte to output */
      gf_obuf[GifBlockSize - gf_rem] = gf_curr;	/* put in buffer */
      gf_curr >>= 8;				/* remove from word */
      gf_valid -= 8;
      if (--gf_rem == 0)			/* flush buffer when full */
         gfdump();
      }
   }

/*
 * gfdump() - dump output buffer
 */
hidden novalue gfdump()
   {
   int n;

   n = GifBlockSize - gf_rem;
   putc(n, gf_f);			/* write block size */
   fwrite((pointer)gf_obuf, 1, n, gf_f); /*write block */
   gf_rem = GifBlockSize;		/* reset buffer to empty */
   }

/*
 * Static data for XDrawImage and XPalette functions
 */

/*
 * c<n>list - the characters of the palettes that are not contiguous ASCII
 */
char c1list[] = "0123456789?!nNAa#@oOBb$%pPCc&|\
qQDd,.rREe;:sSFf+-tTGg*/uUHh`'vVIi<>wWJj()xXKk[]yYLl{}zZMm^=";
char c2list[] = "kbgcrmywx";
char c3list[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZabcd";
char c4list[] =
   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz{}$%&*+-/?@";

/*
 * cgrays -- lists of grayscales contained within color palettes
 */
static char *cgrays[] = { "0123456", "kxw", "@abMcdZ", "0$%&L*+-g/?@}", 
"\0}~\177\200\37\201\202\203\204>\205\206\207\210]\211\212\213\214|",
"\0\330\331\332\333\334+\335\336\337\340\341V\342\343\344\345\346\201\
\347\350\351\352\353\254\354\355\356\357\360\327" };

/*
 * c1cube - a precomputed mapping from a color cube to chars in c1 palette
 *
 * This is 10x10x10 cube (A Thousand Points of Light).
 */
#define C1Side 10  			/* length of one side of C1 cube */
static char c1cube[] = "\
00wwwWWWJJ00vvvWWWJJsttvvVVVVJsttuuVVVVIsttuuVVVIISSTTTUUUII\
SSTTTUUUUISSTTTUUUUHFFTTGGUUHHFFFGGGGHHH00xxxWWWJJ!11vvWWWJJ\
r11vvVVVjjrrtuuVVVjjrrtuuVVVIISSTTTUUUIISSTTTUUUiiSSTTTUUUii\
FFffGGggHHFFffGGggHHnzxxxXXXXJ!11xxXXXjjp1122)Vjjjrr222)Vjjj\
rr222>>>jjRR--//>>iiRRRT//'iiiRRff//ggiiRffffggghhFffffggghh\
nzzyyXXXXKoozyyXXXjjpp222)Xjjjqq222))jjjqq222>>>jjRR--//>>ii\
RRR-//''iiRRff//'giiRffffggghhEffffggghhnzzyyXXXKKoozyyXXXKK\
???22]]]jjqq222]]]jjqq2233>>jjRR::33>>iiRR:::/''iiRR:::/''ii\
EEfffggghhEEfffggghhNNZZZYYYKKOOZZZYYYKK???@=]]]kkPP@@=]]]kk\
PP%%%3]]kkQQ||3344((QQ:::444((QQ:::444<<EEeee++**<EEeee++**`\
NNZZZYYYYKOOZZZYYYkkOOOZ==}kkkPPP@==}}kkPP%%%=}}kkQQ|||444((\
QQ...444((QQe..444<<Qeeee++**<Eeeee++**`NNZZZYYYYLOOZZZYYYkk\
OOOa==mkkkPPaa==}kkkPP%%%=}888QQ|||44888QQc..444[[QQcc99455<\
Qeeee;;55<Deeee;;;*`AAZZMMYYLLAAaaMMmmLLBBaaammmllBBaaammmll\
CCbbb77788CCbbb77^[[Qcccc##^[[Qccc99$55[DDdd9&&556DDddd;;;66\
AAAMMMMLLLAAaaMMmmLLBBaaammmllBBaaammmllCCbbb777llCCbbb77^^{\
Ccccc##^^{Dccc99$$^{DDdd9&&&66DDddd,,,66";

/*
 * c1rgb - RGB values for c1 palette entries
 *
 * Entry order corresponds to c1list (above).
 * Each entry gives r,g,b in linear range 0 to 48.
 */
static unsigned char c1rgb[] = {
   0, 0, 0,		/*  0             black		*/
   8, 8, 8,		/*  1   very dark gray		*/
   16, 16, 16,		/*  2        dark gray		*/
   24, 24, 24,		/*  3             gray		*/
   32, 32, 32,		/*  4       light gray		*/
   40, 40, 40,		/*  5  very light gray		*/
   48, 48, 48,		/*  6             white		*/
   48, 24, 30,		/*  7             pink		*/
   36, 24, 48,		/*  8             violet	*/
   48, 36, 24,		/*  9  very light brown		*/
   24, 12, 0,		/*  ?             brown		*/
   8, 4, 0,		/*  !   very dark brown		*/
   16, 0, 0,		/*  n   very dark red		*/
   32, 0, 0,		/*  N        dark red		*/
   48, 0, 0,		/*  A             red		*/
   48, 16, 16,		/*  a       light red		*/
   48, 32, 32,		/*  #  very light red		*/
   30, 18, 18,		/*  @        weak red		*/
   16, 4, 0,		/*  o   very dark orange	*/
   32, 8, 0,		/*  O        dark orange	*/
   48, 12, 0,		/*  B             orange	*/
   48, 24, 16,		/*  b       light orange	*/
   48, 36, 32,		/*  $  very light orange	*/
   30, 21, 18,		/*  %        weak orange	*/
   16, 8, 0,		/*  p   very dark red-yellow	*/
   32, 16, 0,		/*  P        dark red-yellow	*/
   48, 24, 0,		/*  C             red-yellow	*/
   48, 32, 16,		/*  c       light red-yellow	*/
   48, 40, 32,		/*  &  very light red-yellow	*/
   30, 24, 18,		/*  |        weak red-yellow	*/
   16, 16, 0,		/*  q   very dark yellow	*/
   32, 32, 0,		/*  Q        dark yellow	*/
   48, 48, 0,		/*  D             yellow	*/
   48, 48, 16,		/*  d       light yellow	*/
   48, 48, 32,		/*  ,  very light yellow	*/
   30, 30, 18,		/*  .        weak yellow	*/
   8, 16, 0,		/*  r   very dark yellow-green	*/
   16, 32, 0,		/*  R        dark yellow-green	*/
   24, 48, 0,		/*  E             yellow-green	*/
   32, 48, 16,		/*  e       light yellow-green	*/
   40, 48, 32,		/*  ;  very light yellow-green	*/
   24, 30, 18,		/*  :        weak yellow-green	*/
   0, 16, 0,		/*  s   very dark green		*/
   0, 32, 0,		/*  S        dark green		*/
   0, 48, 0,		/*  F             green		*/
   16, 48, 16,		/*  f       light green		*/
   32, 48, 32,		/*  +  very light green		*/
   18, 30, 18,		/*  -        weak green		*/
   0, 16, 8,		/*  t   very dark cyan-green	*/
   0, 32, 16,		/*  T        dark cyan-green	*/
   0, 48, 24,		/*  G             cyan-green	*/
   16, 48, 32,		/*  g       light cyan-green	*/
   32, 48, 40,		/*  *  very light cyan-green	*/
   18, 30, 24,		/*  /        weak cyan-green	*/
   0, 16, 16,		/*  u   very dark cyan		*/
   0, 32, 32,		/*  U        dark cyan		*/
   0, 48, 48,		/*  H             cyan		*/
   16, 48, 48,		/*  h       light cyan		*/
   32, 48, 48,		/*  `  very light cyan		*/
   18, 30, 30,		/*  '        weak cyan		*/
   0, 8, 16,		/*  v   very dark blue-cyan	*/
   0, 16, 32,		/*  V        dark blue-cyan	*/
   0, 24, 48,		/*  I             blue-cyan	*/
   16, 32, 48,		/*  i       light blue-cyan	*/
   32, 40, 48,		/*  <  very light blue-cyan	*/
   18, 24, 30,		/*  >        weak blue-cyan	*/
   0, 0, 16,		/*  w   very dark blue		*/
   0, 0, 32,		/*  W        dark blue		*/
   0, 0, 48,		/*  J             blue		*/
   16, 16, 48,		/*  j       light blue		*/
   32, 32, 48,		/*  (  very light blue		*/
   18, 18, 30,		/*  )        weak blue		*/
   8, 0, 16,		/*  x   very dark purple	*/
   16, 0, 32,		/*  X        dark purple	*/
   24, 0, 48,		/*  K             purple	*/
   32, 16, 48,		/*  k       light purple	*/
   40, 32, 48,		/*  [  very light purple	*/
   24, 18, 30,		/*  ]        weak purple	*/
   16, 0, 16,		/*  y   very dark magenta	*/
   32, 0, 32,		/*  Y        dark magenta	*/
   48, 0, 48,		/*  L             magenta	*/
   48, 16, 48,		/*  l       light magenta	*/
   48, 32, 48,		/*  {  very light magenta	*/
   30, 18, 30,		/*  }        weak magenta	*/
   16, 0, 8,		/*  z   very dark magenta-red	*/
   32, 0, 16,		/*  Z        dark magenta-red	*/
   48, 0, 24,		/*  M             magenta-red	*/
   48, 16, 32,		/*  m       light magenta-red	*/
   48, 32, 40,		/*  ^  very light magenta-red	*/
   30, 18, 24,		/*  =        weak magenta-red	*/
   };

/*
 * palnum(d) - return palette number, or 0 if unrecognized.
 *
 *    returns +1 ... +6 for "c1" through "c6"
 *    returns +1 for &null
 *    returns -2 ... -256 for "g2" through "g256"
 *    returns 0 for unrecognized palette name
 *    returns -1 for non-string argument
 */
int palnum(d)
dptr d;
   {
   tended char *s;
   char c, x;
   int n;

   if (is:null(*d))
      return 1;
   if (!cnv:C_string(*d, s))
      return -1;
   if (sscanf(s, "%c%d%c", &c, &n, &x) != 2)
      return 0;
   if (c == 'c' && n >= 1 && n <= 6)
      return n;
   if (c == 'g' && n >= 2 && n <= 256)
      return -n;
   return 0;
   }

/*
 * palsetup(p) - set up palette for specified palette.
 */
struct palentry *palsetup(p)
int p;
   {
   int r, g, b, i, n, c;
   unsigned int rr, gg, bb;
   unsigned char *s, *t;
   double m;
   struct palentry *e;

   static struct palentry *palette;	/* current palette */
   static int palnumber;		/* current palette number */

   if (palnumber == p)
      return palette;
   if (palette == NULL) {
      palette = (struct palentry *)malloc(256 * sizeof(struct palentry));
      if (palette == NULL)
         return NULL;
      }
   palnumber = p;

   for (i = 0; i < 256; i++)
      palette[i].valid = 0;
   
   if (p < 0) {				/* grayscale palette */
      n = -p;
      if (n <= 64)
         s = (unsigned char *)c4list;
      else
         s = allchars;
      m = 1.0 / (n - 1);

      for (i = 0; i < n; i++) {
         e = &palette[*s++];
         gg = 65535 * m * i;
         e->clr.red = e->clr.green = e->clr.blue = gg;
         e->valid = 1;
         }
      return palette;
      }

   if (p == 1) {			/* special c1 palette */
      s = (unsigned char *)c1list;
      t = c1rgb;
      while (c = *s++) {
         e = &palette[c];
         e->clr.red   = 65535 * (((int)*t++) / 48.0);
         e->clr.green = 65535 * (((int)*t++) / 48.0);
         e->clr.blue  = 65535 * (((int)*t++) / 48.0);
         e->valid = 1;
         }
      return palette;
      }

   switch (p) {				/* color cube plus extra grays */
      case  2:  s = (unsigned char *)c2list;	break;	/* c2 */
      case  3:  s = (unsigned char *)c3list;	break;	/* c3 */
      case  4:  s = (unsigned char *)c4list;	break;	/* c4 */
      case  5:  s = allchars;			break;	/* c5 */
      case  6:  s = allchars; 			break;	/* c6 */
      }
   m = 1.0 / (p - 1);
   for (r = 0; r < p; r++) {
      rr = 65535 * m * r;
      for (g = 0; g < p; g++) {
         gg = 65535 * m * g;
         for (b = 0; b < p; b++) {
            bb = 65535 * m * b;
            e = &palette[*s++];
            e->clr.red = rr;
            e->clr.green = gg;
            e->clr.blue = bb;
            e->valid = 1;
            }
         }
      }
   m = 1.0 / (p * (p - 1));
   for (g = 0; g < p * (p - 1); g++)
      if (g % p != 0) {
         gg = 65535 * m * g;
         e = &palette[*s++];
         e->clr.red = e->clr.green = e->clr.blue = gg;
         e->valid = 1;
         }
   return palette;
   }

/*
 * rgbkey(p,r,g,b) - return pointer to key of closest color in palette number p.
 *
 * In color cubes, finds "extra" grays only if r == g == b.
 */
char *rgbkey(p, r, g, b)
int p;
double r, g, b;
   {
   int n, i;
   double m;
   char *s;

   if (p > 0) { 			/* color */
      if (r == g && g == b) {
         if (p == 1)
            m = 6;
         else
            m = p * (p - 1);
         return cgrays[p - 1] + (int)(0.501 + m * g);
         }
      else {
         if (p == 1)
            n = C1Side;
         else
            n = p;
         m = n - 1;
         i = (int)(0.501 + m * r);
         i = n * i + (int)(0.501 + m * g);
         i = n * i + (int)(0.501 + m * b);
         switch(p) {
            case  1:  return c1cube + i;		/* c1 */
            case  2:  return c2list + i;		/* c2 */
            case  3:  return c3list + i;		/* c3 */
            case  4:  return c4list + i;		/* c4 */
            case  5:  return (char *)allchars + i;	/* c5 */
            case  6:  return (char *)allchars + i;	/* c6 */
            }
         }
      }
   else {				/* grayscale */
      if (p < -64)
         s = (char *)allchars;
      else
         s = c4list;
      return s + (int)(0.5 + (0.299 * r + 0.587 * g + 0.114 * b) * (-p - 1));
      }

   }

/*
 * mapping from recognized style attributes to flag values
 */
stringint fontwords[] = {
   { 0,			17 },		/* number of entries */
   { "bold",		FONTATT_WEIGHT	| FONTFLAG_BOLD },
   { "condensed",	FONTATT_WIDTH	| FONTFLAG_CONDENSED },
   { "demi",		FONTATT_WEIGHT	| FONTFLAG_DEMI },
   { "demibold",	FONTATT_WEIGHT	| FONTFLAG_DEMI | FONTFLAG_BOLD },
   { "extended",	FONTATT_WIDTH	| FONTFLAG_EXTENDED },
   { "italic",		FONTATT_SLANT	| FONTFLAG_ITALIC },
   { "light",		FONTATT_WEIGHT	| FONTFLAG_LIGHT },
   { "medium",		FONTATT_WEIGHT	| FONTFLAG_MEDIUM },
   { "mono",		FONTATT_SPACING	| FONTFLAG_MONO },
   { "narrow",		FONTATT_WIDTH	| FONTFLAG_NARROW },
   { "normal",		FONTATT_WIDTH	| FONTFLAG_NORMAL },
   { "oblique",		FONTATT_SLANT	| FONTFLAG_OBLIQUE },
   { "proportional",	FONTATT_SPACING	| FONTFLAG_PROPORTIONAL },
   { "roman",		FONTATT_SLANT	| FONTFLAG_ROMAN },
   { "sans",		FONTATT_SERIF	| FONTFLAG_SANS },
   { "serif",		FONTATT_SERIF	| FONTFLAG_SERIF },
   { "wide",		FONTATT_WIDTH	| FONTFLAG_WIDE },
};

/*
 * parsefont - extract font family name, style attributes, and size
 *
 * these are window system independent values, so they require
 *  further translation into window system dependent values.
 *
 * returns 1 on an OK font name
 * returns 0 on a "malformed" font (might be a window-system fontname)
 */
int parsefont(s, family, style, size)
char *s;
char family[MAXFONTWORD+1];
int *style;
int *size;
   {
   char c, *a, attr[MAXFONTWORD+1];
   int tmp;

   /*
    * set up the defaults
    */
   *family = '\0';
   *style = 0;
   *size = -1;

   /*
    * now, scan through the raw and break out pieces
    */
   for (;;) {

      /*
       * find start of next comma-separated attribute word
       */
      while (isspace(*s) || *s == ',')	/* trim leading spaces & empty words */
         s++;
      if (*s == '\0')			/* stop at end of string */
         break;

      /*
       * copy word, converting to lower case to implement case insensitivity
       */
      for (a = attr; (c = *s) != '\0' && c != ','; s++) {
         if (isupper(c))
            c = tolower(c);
         *a++ = c;
         if (a - attr >= MAXFONTWORD)
            return 0;			/* too long */
         }

      /*
       * trim trailing spaces and terminate word
       */
      while (isspace(a[-1]))
         a--;
      *a = '\0';

      /*
       * interpret word as family name, size, or style characteristic
       */
      if (*family == '\0')
         strcpy(family, attr);		/* first word is the family name */

      else if (sscanf(attr, "%d%c", &tmp, &c) == 1 && tmp > 0) {
         if (*size != -1 && *size != tmp)
            return 0;			/* if conflicting sizes given */
         *size = tmp;			/* integer value is a size */
         }
         
      else {				/* otherwise it's a style attribute */
         tmp = si_s2i(fontwords, attr);	/* look up in table */
         if (tmp != -1) {		/* if recognized */
            if ((tmp & *style) != 0 && (tmp & *style) != tmp)
               return 0;		/* conflicting attribute */
            *style |= tmp;
            }
         }
      }

   /* got to end of string; it's OK if it had at least a font family */
   return (*family != '\0');
   }

/*
 * getpattern() - return the stipple pattern if there is one
 */
int getpattern(w, answer)
wbp w;
char *answer;
   {
   if (w->context->patternname == NULL) return Failed;
   sprintf(answer, "%s", w->context->patternname);
   return Succeeded;
   }

/*
 * parsepattern() - parse an encoded numeric stipple pattern
 */
int parsepattern(s, len, width, nbits, bits)
char *s;
int len;
int *width, *nbits;
C_integer *bits;
   {
   C_integer v;
   int i, j, hexdigits_per_row, maxbits = *nbits;

   /*
    * Get the width
    */
   if (sscanf(s, "%d,", width) != 1) return Error;
   if (*width < 1) return Failed;

   /*
    * skip over width
    */
   while ((len > 0) && isdigit(*s)) {
      len--; s++;
      }
   if ((len <= 1) || (*s != ',')) return Error;
   len--; s++;					/* skip over ',' */
   
   if (*s == '#') {
      /*
       * get remaining bits as hex constant
       */
      s++; len--;
      if (len == 0) return Error;
      hexdigits_per_row = *width / 4;
      if (*width % 4) hexdigits_per_row++;
      *nbits = len / hexdigits_per_row;
      if (len % hexdigits_per_row) *nbits++;
      if (*nbits > maxbits) return Failed;
      for (i = 0; i < *nbits; i++) {
         v = 0;
	 for (j = 0; j < hexdigits_per_row; j++, len--, s++) {
	    if (len == 0) break;
	    v <<= 4;
	    if (isdigit(*s)) v += *s - '0';
	    else switch (*s) {
	    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	       v += *s - 'a' + 10; break;
	    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	       v += *s - 'A' + 10; break;
	    default: return Error;
	       }
	    }
	 *bits++ = v;
	 }
      }
   else {
      if (*width > 32) return Failed;
      /*
       * get remaining bits as comma-separated decimals
       */
      v = 0;
      *nbits = 0;
      while (len > 0) {
	 while ((len > 0) && isdigit(*s)) {
	    v = v * 10 + *s - '0';
	    len--; s++;
	    }
	 (*nbits)++;
	 if (*nbits > maxbits) return Failed;
	 *bits++ = v;
	 v = 0;

	 if (len > 0)
	    if (*s == ',') { len--; s++; }
	    else {
	       ReturnErrNum(205, Error);
	       }
	 }
      }
   return Succeeded;
   }

/*
 * parsegeometry - parse a string of the form: intxint[+-]int[+-]int
 * Returns:
 *  0 on bad value, 1 if size is set, 2 if position is set, 3 if both are set
 */
int parsegeometry(buf, x, y, width, height)
char *buf;
SHORT *x, *y, *width, *height;
   {
   int retval = 0;
   if (isdigit(*buf)) {
      retval++;
      *width = atoi(buf);
      while (isdigit(*++buf));
      if (*buf++ != 'x') return 0;
      if ((*height = atoi(buf)) <= 0) return 0;
      while (isdigit(*++buf));
      }

   if (*buf == '+' || *buf == '-') {
      retval += 2;
      *x = atoi(buf);
      buf++; /* skip over +/- */
      while (isdigit(*buf)) buf++;

      if (*buf != '+' && *buf != '-') return 0;
      *y = atoi(buf);
      buf++; /* skip over +/- */
      while (isdigit(*buf)) buf++;
      if (*buf) return 0;
      }
   return retval;
   }

#define ATOBOOL(s) (!strcmp(s,"on") || !strcmp(s,"ON") || !strcmp(s,"On"))

#begdef AttemptAttr(operation)
   switch (operation) {
      case Failed: return Failed;
      case Error: ReturnErrNum(145, Error);
      }
#enddef					/* AttemptAttr */
/*
 * Attribute manipulation
 *
 * wattrib() - get/set a single attribute in a window, return the result attr
 *  string.
 */
int wattrib(w, s, len, answer, abuf)
wbp w;
char *s;
long len;
dptr answer;
char * abuf;
   {
   char val[128], *valptr;
   struct descrip d;
   char *mid, c;
   int r, a;
   C_integer tmp;
   long lenattr, lenval;
   double gamma;
   SHORT new_height, new_width;
   STDLOCALS(w);

   valptr = val;
   /*
    * catch up on any events pending - mainly to update pointerx, pointery
    */
   if (pollevent() == -1)
      fatalerr(141,NULL);

   if (mid = strnchr(s, '=', len)) {
      /*
       * set an attribute
       */
      lenattr = mid - s;
      lenval  = len - lenattr - 1;
      mid++;

      mystrncpy(abuf, s, lenattr);
      mystrncpy(val, mid, lenval);
      StrLen(d) = strlen(val);
      StrLoc(d) = val;

      switch (a = si_s2i(attribs, abuf)) {
      case A_LINES: case A_ROWS: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 if ((new_height = tmp) < 1)
	    return Failed;
	 new_height = ROWTOY(w, new_height);
	 new_height += MAXDESCENDER(w);
	 if (setheight(w, new_height) == Failed) return Failed;
	 break;
         }
      case A_COLUMNS: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 if ((new_width = tmp) < 1)
	    return Failed;
	 new_width = COLTOX(w, new_width + 1);
	 if (setwidth(w, new_width) == Failed) return Failed;
	 break;
         }
      case A_HEIGHT: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
         if ((new_height = tmp) < 1) return Failed;
	 if (setheight(w, new_height) == Failed) return Failed;
	 break;
         }
      case A_WIDTH: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
         if ((new_width = tmp) < 1) return Failed;
	 if (setwidth(w, new_width) == Failed) return Failed;
	 break;
         }
      case A_SIZE: {
	 AttemptAttr(setsize(w, val));
	 break;
	 }
      case A_GEOMETRY: {
	 AttemptAttr(setgeometry(w, val));
	 break;
         }
      case A_ROW: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->y = ROWTOY(w, tmp) + wc->dy;
	 break;
	 }
      case A_COL: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->x = COLTOX(w, tmp) + wc->dx;
	 break;
	 }
      case A_CANVAS: { 
	 AttemptAttr(setcanvas(w,val));
	 break; 
	 }
      case A_ICONIC: { 
	 AttemptAttr(seticonicstate(w,val));
	 break; 
	 }
      case A_ICONIMAGE: {
	 /* does not use AttemptAttr because different errors are possible */
	 switch (seticonimage(w, &d)) {
	 case Failed: return Failed;
	 case Error: return Error;
	    }
         break;
	 }
      case A_ICONLABEL: { 
	 AttemptAttr(seticonlabel(w, val));
	 break; 
	 }
      case A_ICONPOS: {
	 AttemptAttr(seticonpos(w,val));
	 break;
	 }
      case A_LABEL:
      case A_WINDOWLABEL: {
	 AttemptAttr(setwindowlabel(w, val));
	 break;
         }
      case A_CURSOR: {
	 int on_off;
	 if (strcmp(val, "on") & strcmp(val, "off"))
	    ReturnErrNum(145, Error);
	 on_off = ATOBOOL(val);
         setcursor(w, on_off);
	 break;
         }
      case A_FONT: {
	 AttemptAttr(setfont(w, &valptr));
	 break;
         }
      case A_PATTERN: {
	 AttemptAttr(SetPattern(w, val, strlen(val)));
         break;
	 }
      case A_POS: {
	 AttemptAttr(setpos(w, val));
	 break;
	 }
      case A_POSX: {
	 char tmp[20];
	 sprintf(tmp,"%s,%d",val,ws->posy);
	 AttemptAttr(setpos(w, tmp));
	 break;
	 }
      case A_POSY: {
	 char tmp[20];
	 sprintf(tmp,"%d,%s",ws->posx,val);
	 AttemptAttr(setpos(w, tmp));
         break;
	 }
      case A_FG: {
	 if (cnv:C_integer(d, tmp) && tmp < 0) {
	    if (!isetfg(w, tmp)) return Failed;
	    }
	 else {
	    if (!setfg(w, val)) return Failed;
	    }
	 break;
         }
      case A_BG: {
	 if (cnv:C_integer(d, tmp) && tmp < 0) {
	    if (!isetbg(w, tmp)) return Failed;
	    }
	 else {
	    if (!setbg(w, val)) return Failed;
	    }
	 break;
         }
      case A_GAMMA: {
         if (sscanf(val, "%lf%c", &gamma, &c) != 1 || gamma <= 0.0)
            ReturnErrNum(205, Error);
         if (!setgamma(w, gamma))
            return Failed;
         break;
         }
      case A_FILLSTYLE: {
	 AttemptAttr(setfillstyle(w, val));
	 break;
	 }
      case A_LINESTYLE: {
	 AttemptAttr(setlinestyle(w, val));
	 break;
	 }
      case A_LINEWIDTH: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 if (setlinewidth(w, tmp) == Error) ReturnErrNum(205, Error);
	 break;
	 }
      case A_POINTER: {
	 AttemptAttr(setpointer(w, val));
	 break;
         }
      case A_DRAWOP: {
	 AttemptAttr(setdrawop(w, val));
	 break;
         }
      case A_DISPLAY: {
	 AttemptAttr(setdisplay(w,val));
	 break;
         }
      case A_X: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->x = tmp + wc->dx;
         UpdateCursorPos(ws, wc);	/* tell system where to blink it */
	 break;
	 }
      case A_Y: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->y = tmp + wc->dy;
         UpdateCursorPos(ws, wc);	/* tell system where to blink it */
	 break;
	 }
      case A_DX: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 wc->dx = tmp;
         UpdateCursorPos(ws, wc);	/* tell system where to blink it */
	 break;
	 }
      case A_DY: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 wc->dy = tmp;
         UpdateCursorPos(ws, wc);	/* tell system where to blink it */
	 break;
	 }
      case A_LEADING: { 
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 setleading(w, tmp);
         break;
         }
      case A_IMAGE: {
         /* first try GIF; then try platform-dependent format */
         r = readGIF(val, 0, &ws->initimage);
         if (r == Succeeded) {
            setwidth(w, ws->initimage.width);
            setheight(w, ws->initimage.height);
            }
         else
            r = setimage(w, val);
	 AttemptAttr(r);
         break;
         }
      case A_ECHO: {
	 if (strcmp(val, "on") & strcmp(val, "off"))
	    ReturnErrNum(145, Error);
	 if (ATOBOOL(val)) SETECHOON(w);
	 else CLRECHOON(w);
	 break;
         }

      case A_CLIPX:
      case A_CLIPY:
      case A_CLIPW:
      case A_CLIPH: {
         if (!*val) {			/* setting null to disable */
            wc->clipx = wc->clipy = 0;
            wc->clipw = wc->cliph = -1;
            unsetclip(w);
            }
         else {
            if (!cnv:C_integer(d, tmp))
               ReturnErrNum(205, Error);
            if (wc->clipw < 0) {		/* if disabled, reenable */
               wc->clipx = wc->clipy = 0;	/* init with full window */
               wc->clipw = ws->width;
               wc->cliph = ws->height;
               }
            switch(a) {				/* now set the given value */
               case A_CLIPX:  wc->clipx = tmp;  break;
               case A_CLIPY:  wc->clipy = tmp;  break;
               case A_CLIPW:  wc->clipw = tmp;  break;
               case A_CLIPH:  wc->cliph = tmp;  break;
               }
            setclip(w);
            }
         break;
         }

      case A_REVERSE: {
	 if (strcmp(val, "on") & strcmp(val, "off"))
	    ReturnErrNum(145, Error);
	 if ((!ATOBOOL(val) && ISREVERSE(w)) ||
	     (ATOBOOL(val) && !ISREVERSE(w))) {
	    toggle_fgbg(w);
	    ISREVERSE(w) ? CLRREVERSE(w) : SETREVERSE(w);
	    }
	 break;
         }
      case A_POINTERX: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->pointerx = tmp;
	 warpPointer(w, ws->pointerx, ws->pointery);
	 break;
	 }
      case A_POINTERY: {
	 if (!cnv:C_integer(d, tmp))
	    ReturnErrNum(205, Error);
	 ws->pointery = tmp;
	 warpPointer(w, ws->pointerx, ws->pointery);
	 break;
	 }
      default:
	 ReturnErrNum(145, Error);
	 }
      mystrncpy(abuf, s, len);
      }
   else {
      /*
       * get an attribute
       */
      mystrncpy(abuf, s, len);
      switch (a=si_s2i(attribs, abuf)) {
      case A_VISUAL:
	 if (getvisual(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_DEPTH:
	 MakeInt(SCREENDEPTH(w), answer);
	 break;
      case A_DISPLAY:
	 if (getdisplay(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ASCENT:
	 MakeInt(ASCENT(w), answer);
	 break;
      case A_DESCENT:
	 MakeInt(DESCENT(w), answer);
	 break;
      case A_FHEIGHT:
	 MakeInt(FHEIGHT(w), answer);
	 break;
      case A_FWIDTH:
	 MakeInt(FWIDTH(w), answer);
	 break;
      case A_ROW:
	 MakeInt(YTOROW(w, w->window->y-1), answer);
	 break;
      case A_COL:
	 MakeInt(1 + XTOCOL(w, w->window->x), answer);
	 break;
      case A_POINTERROW: {
	 XPoint xp;
	 query_pointer(w, &xp);
	 MakeInt(YTOROW(w, xp.y - 1), answer);
	 break;
	 }
      case A_POINTERCOL: {
	 XPoint xp;
	 query_pointer(w, &xp);
	 MakeInt(1 + XTOCOL(w, xp.x), answer);
	 break;
	 }
      case A_LINES:
      case A_ROWS:
	 MakeInt(YTOROW(w,ws->height - DESCENT(w)), answer);
	 break;
      case A_COLUMNS:
	 MakeInt(XTOCOL(w,ws->width), answer);
	 break;
      case A_POS: case A_POSX: case A_POSY:
	 if (getpos(w) == Failed) return Failed;
	 switch (a) {
	 case A_POS:
	    sprintf(abuf, "%d,%d", ws->posx, ws->posy);
	    MakeStr(abuf, strlen(abuf), answer);
	    break;
	 case A_POSX:
	    MakeInt(ws->posx, answer);
	    break;
	 case A_POSY:
	    MakeInt(ws->posy, answer);
	    break;
	    }
	 break;
      case A_FG:
	 getfg(w, abuf);
	 MakeStr(abuf, strlen(abuf), answer);
	 break; 
      case A_BG:
	 getbg(w, abuf);
	 MakeStr(abuf, strlen(abuf), answer);
         break; 
      case A_GAMMA:
         Protect(BlkLoc(*answer) = (union block *)alcreal(wc->gamma),
            return Error);
         answer->dword = D_Real;
         break;
      case A_FILLSTYLE:
         sprintf(abuf, "%s",
            (wc->fillstyle == FS_SOLID) ? "solid" :
            (wc->fillstyle == FS_STIPPLE) ? "masked" : "textured");
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_LINESTYLE:
	 if (getlinestyle(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_LINEWIDTH:
	 MakeInt(LINEWIDTH(w), answer);
	 break;
      case A_HEIGHT: { MakeInt(ws->height, answer); break; }
      case A_WIDTH: { MakeInt(ws->width, answer); break; }
       case A_SIZE:
	 sprintf(abuf, "%d,%d", ws->width, ws->height);
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_DISPLAYHEIGHT:
	 MakeInt(DISPLAYHEIGHT(w), answer);
	 break;
      case A_DISPLAYWIDTH:
	 MakeInt(DISPLAYWIDTH(w), answer);
	 break;
      case A_CURSOR:
	 sprintf(abuf,"%s",(ISCURSORON(w)?"on":"off"));
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ECHO:
	 sprintf(abuf,"%s",(ISECHOON(w)?"on":"off"));
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_REVERSE:
	 sprintf(abuf,"%s",(ISREVERSE(w)?"on":"off"));
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_FONT:
	 getfntnam(w, abuf);
	 MakeStr(abuf, strlen(abuf), answer);
	 break; 
      case A_X:  MakeInt(ws->x, answer); break;
      case A_Y:  MakeInt(ws->y, answer); break;
      case A_DX: MakeInt(wc->dx, answer); break;
      case A_DY: MakeInt(wc->dy, answer); break;
      case A_LEADING: MakeInt(LEADING(w), answer); break;
      case A_POINTERX: {
	 XPoint xp;
	 query_pointer(w, &xp);
	 MakeInt(xp.x, answer);
	 break;
	 }
      case A_POINTERY: {
	 XPoint xp;
	 query_pointer(w, &xp);
	 MakeInt(xp.y, answer);
	 break;
	 }
      case A_POINTER:
	 if (getpointername(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_DRAWOP:
	 if (getdrawop(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_GEOMETRY:
	 if (getpos(w) == Failed) return Failed;
         if (stdwin)
           sprintf(abuf, "%dx%d+%d+%d",
		   ws->width, ws->height, ws->posx, ws->posy);
         else
           sprintf(abuf, "%dx%d", ws->pixwidth, ws->pixheight);
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_CANVAS:
	 if (getcanvas(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ICONIC:
	 geticonic(w, abuf);
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ICONIMAGE:
	 if (ICONFILENAME(w) != NULL)
	    sprintf(abuf, "%s", ICONFILENAME(w));
	 else return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ICONLABEL:
	 if (ICONLABEL(w) != NULL)
	    sprintf(abuf, "%s", ICONLABEL(w));
	 else return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_LABEL:
      case A_WINDOWLABEL:
	 if (WINDOWLABEL(w) != NULL)
	    sprintf(abuf,"%s", WINDOWLABEL(w));
	 else return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
      case A_ICONPOS: {
	 switch (geticonpos(w,abuf)) {
	 case Failed: return Failed;
	 case Error: return Error;
	    }
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
	 }
      case A_PATTERN: {
	 if (getpattern(w, abuf) == Failed) return Failed;
	 MakeStr(abuf, strlen(abuf), answer);
	 break;
	 }
      case A_CLIPX:
         if (wc->clipw >= 0) {
            MakeInt(wc->clipx, answer);
            }
         else
            *answer = nulldesc;
         break;
      case A_CLIPY:
         if (wc->clipw >= 0) {
            MakeInt(wc->clipy, answer);
            }
         else
            *answer = nulldesc;
            break;
      case A_CLIPW:
         if (wc->clipw >= 0) {
            MakeInt(wc->clipw, answer);
            }
         else
            *answer = nulldesc;
            break;
      case A_CLIPH:
         if (wc->clipw >= 0) {
            MakeInt(wc->cliph, answer);
            }
         else
            *answer = nulldesc;
            break;
      default:
	 ReturnErrNum(145, Error);
	 }
   }
   wflush(w);
   return Succeeded;
   }

/*
 * rectargs -- interpret rectangle arguments uniformly
 *
 *  Given an arglist and the index of the next x value, rectargs sets
 *  x/y/width/height to explicit or defaulted values.  These values are
 *  in canonical form:  Width and height are nonnegative and x and y
 *  have been corrected by dx and dy.
 *
 *  Returns index of bad argument, if any, or -1 for success.
 */
int rectargs(w, argc, argv, i, px, py, pw, ph)
wbp w;
int argc;
dptr argv;
int i;
C_integer *px, *py, *pw, *ph;
   {
   int defw, defh;
   wcp wc = w->context;
   wsp ws = w->window;

   /*
    * Get x and y, defaulting to zero.
    */
   if (i >= argc)
      *px = 0;
   else if (!def:C_integer(argv[i], 0, *px))
      return i;

   if (++i >= argc)
      *py = 0;
   else if (!def:C_integer(argv[i], 0, *py))
      return i;

   *px += wc->dx;
   *py += wc->dy;

   /*
    * Get w and h, defaulting to extend to the edge
    */
   defw = ws->width - *px;
   defh = ws->height - *py;

   if (++i >= argc)
      *pw = defw;
   else if (!def:C_integer(argv[i], defw, *pw))
      return i;

   if (++i >= argc)
      *ph = defh;
   else if (!def:C_integer(argv[i], defh, *ph))
      return i;

   /*
    * Correct negative w/h values.
    */
   if (*pw < 0)
      *px -= (*pw = -*pw);
   if (*ph < 0)
      *py -= (*ph = -*ph);

   return -1;
   }

/*
 * docircles -- draw or file circles.
 *
 *  Helper for DrawCircle and FillCircle.
 *  Returns index of bad argument, or -1 for success.
 */
int docircles(w, argc, argv, fill)
wbp w;
int argc;
dptr argv;
int fill;
   {
   XArc arc;
   int i, dx, dy;
   double x, y, r, theta, alpha;
   STDLOCALS(w);

   dx = w->context->dx;
   dy = w->context->dy;

   for (i = 0; i < argc; i += 5) {	/* for each set of five args */

      /*
       * Collect arguments.
       */
      if (i + 2 >= argc)
         return i + 2;			/* missing y or r */
      if (!cnv:C_double(argv[i], x))
         return i;
      if (!cnv:C_double(argv[i + 1], y))
         return i + 1;
      if (!cnv:C_double(argv[i + 2], r))
         return i + 2;
      if (i + 3 >= argc)
         theta = 0.0;
      else if (!def:C_double(argv[i + 3], 0.0, theta))
         return i + 3;
      if (i + 4 >= argc)
         alpha = 2 * Pi;
      else if (!def:C_double(argv[i + 4], 2 * Pi, alpha))
         return i + 4;

      /*
       * Put in canonical form: r >= 0, -2*pi <= theta < 0, alpha >= 0.
       */
      if (r < 0) {			/* ensure positive radius */
         r = -r;
         theta += Pi;
         }
      if (alpha < 0) {			/* ensure positive extent */
         theta += alpha;
         alpha = -alpha;
         }

      theta = fmod(theta, 2 * Pi);
      if (theta > 0)			/* normalize initial angle */
         theta -= 2 * Pi;

      /*
       * Build the Arc descriptor.
       */
      arc.x = x + dx - r;
      arc.y = y + dy - r;
      ARCWIDTH(arc) = 2 * r;
      ARCHEIGHT(arc) = 2 * r;

      arc.angle1 = ANGLE((int)(-theta * 180 / Pi * 64));
      if (alpha >= 2 * Pi)
         arc.angle2 = FULLARC;
      else
         arc.angle2 = EXTENT((int)(-alpha  * 180 / Pi * 64));

      /*
       * Draw or fill the arc.
       */
      if (fill) {			/* {} required due to form of macros */
         fillarcs(w, &arc, 1);
         }
      else {
         drawarcs(w, &arc, 1);
         }
      }
   return -1;
   }


/*
 * genCurve - draw a smooth curve through a set of points.  Algorithm from
 *  Barry, Phillip J., and Goldman, Ronald N. (1988).
 *  A Recursive Evaluation Algorithm for a class of Catmull-Rom Splines.
 *  Computer Graphics 22(4), 199-204.
 */
novalue genCurve(w, p, n, helper)
wbp w;
XPoint *p;
int n;
void (*helper)();
   {
   int    i, j, steps;
   float  ax, ay, bx, by, stepsize, stepsize2, stepsize3;
   float  x, dx, d2x, d3x, y, dy, d2y, d3y;
   XPoint thepoints[MAXXOBJS*5];

   for (i = 3; i < n; i++) {
      /*
       * build the coefficients ax, ay, bx and by, using:
       *                             _              _   _    _
       *   i                 i    1 | -1   3  -3   1 | | Pi-3 |
       *  Q (t) = T * M   * G   = - |  2  -5   4  -1 | | Pi-2 |
       *               CR    Bs   2 | -1   0   1   0 | | Pi-1 |
       *                            |_ 0   2   0   0_| |_Pi  _|
       */

      ax = p[i].x - 3 * p[i-1].x + 3 * p[i-2].x - p[i-3].x;
      ay = p[i].y - 3 * p[i-1].y + 3 * p[i-2].y - p[i-3].y;
      bx = 2 * p[i-3].x - 5 * p[i-2].x + 4 * p[i-1].x - p[i].x;
      by = 2 * p[i-3].y - 5 * p[i-2].y + 4 * p[i-1].y - p[i].y;

      /*
       * calculate the forward differences for the function using
       * intervals of size 0.1
       */
#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif
#ifndef max
#define max(x,y) ((x>y)?x:y)
#endif

#if VMS
      {
      int tmp1 = abs(p[i-1].x - p[i-2].x);
      int tmp2 = abs(p[i-1].y - p[i-2].y);
      steps = max(tmp1, tmp2) + 10;
      }
#else						/* VMS */
      steps = max(abs(p[i-1].x - p[i-2].x), abs(p[i-1].y - p[i-2].y)) + 10;
#endif						/* VMS */
      stepsize = 1.0/steps;
      stepsize2 = stepsize * stepsize;
      stepsize3 = stepsize * stepsize2;

      x = thepoints[0].x = p[i-2].x;
      y = thepoints[0].y = p[i-2].y;
      dx = (stepsize3*0.5)*ax + (stepsize2*0.5)*bx + (stepsize*0.5)*(p[i-1].x-p[i-3].x);
      dy = (stepsize3*0.5)*ay + (stepsize2*0.5)*by + (stepsize*0.5)*(p[i-1].y-p[i-3].y);
      d2x = (stepsize3*3) * ax + stepsize2 * bx;
      d2y = (stepsize3*3) * ay + stepsize2 * by;
      d3x = (stepsize3*3) * ax;
      d3y = (stepsize3*3) * ay;

      /* calculate the points for drawing the curve */

      for (j = 0; j < steps; j++) {
	 x = x + dx;
	 y = y + dy;
	 dx = dx + d2x;
	 dy = dy + d2y;
	 d2x = d2x + d3x;
	 d2y = d2y + d3y;
         thepoints[j+1].x = (int)x;
         thepoints[j+1].y = (int)y;
         }
      helper(w,thepoints,steps+1);
      }
   }

static void curveHelper(w,thepoints,n)
wbp w;
XPoint thepoints[];
int n;
   {
   /*
    * Could use drawpoints(w, thepoints, n)
    *  but that ignores the linewidth and linestyle attributes...
    * Might make linestyle work a little better by "compressing" straight
    *  sections produced by genCurve into single drawline points.
    */
   drawlines(w, thepoints, n);
   }

/*
 * draw a smooth curve through the array of points
 */
novalue drawCurve(w, p, n)
wbp w;
XPoint *p;
int n;
   {
   genCurve(w, p, n, curveHelper);
   }

/*
 * the next section consists of code to deal with string-integer
 * (stringint) symbols.  See graphics.h.
 */

/*
 * string-integer comparison, for qsearch()
 */
static int sicmp(sip1,sip2)
siptr sip1, sip2;
{
  return strcmp(sip1->s, sip2->s);
}

/*
 * string-integer lookup function: given a string, return its integer
 */
int si_s2i(sip,s)
siptr sip;
char *s;
{
  stringint key;
  siptr p;
  key.s = s;

  p = (siptr)qsearch((char *)&key,(char *)(sip+1),sip[0].i,sizeof(key),sicmp);
  if (p) return p->i;
  return -1;
}

/*
 * string-integer inverse function: given an integer, return its string
 */
char *si_i2s(sip,i)
siptr sip;
int i;
{
  register siptr sip2 = sip+1;
  for(;sip2<=sip+sip[0].i;sip2++) if (sip2->i == i) return sip2->s;
  return NULL;
}


/*
 * And now, the stringint data.
 * Convention: the 0'th element of a stringint array contains the
 * NULL string, and an integer count of the # of elements in the array.
 */

stringint attribs[] = {
   { 0,			NUMATTRIBS},
   {"ascent",		A_ASCENT},
   {"bg",		A_BG},
   {"canvas",		A_CANVAS},
   {"ceol",		A_CEOL},
   {"cliph",		A_CLIPH},
   {"clipw",		A_CLIPW},
   {"clipx",		A_CLIPX},
   {"clipy",		A_CLIPY},
   {"col",		A_COL},
   {"columns",		A_COLUMNS},
   {"cursor",		A_CURSOR},
   {"depth",		A_DEPTH},
   {"descent",		A_DESCENT},
   {"display",		A_DISPLAY},
   {"displayheight",	A_DISPLAYHEIGHT},
   {"displaywidth",	A_DISPLAYWIDTH},
   {"drawop",		A_DRAWOP},
   {"dx",		A_DX},
   {"dy",		A_DY},
   {"echo",		A_ECHO},
   {"fg",		A_FG},
   {"fheight",		A_FHEIGHT},
   {"fillstyle",	A_FILLSTYLE},
   {"font",		A_FONT},
   {"fwidth",		A_FWIDTH},
   {"gamma",		A_GAMMA},
   {"geometry",		A_GEOMETRY},
   {"height",		A_HEIGHT},
   {"iconic",		A_ICONIC},
   {"iconimage",	A_ICONIMAGE},
   {"iconlabel",	A_ICONLABEL},
   {"iconpos",		A_ICONPOS},
   {"image",		A_IMAGE},
   {"label",		A_LABEL},
   {"leading",		A_LEADING},
   {"lines",		A_LINES},
   {"linestyle",	A_LINESTYLE},
   {"linewidth",	A_LINEWIDTH},
   {"pattern",		A_PATTERN},
   {"pointer",		A_POINTER},
   {"pointercol",	A_POINTERCOL},
   {"pointerrow",	A_POINTERROW},
   {"pointerx",		A_POINTERX},
   {"pointery",		A_POINTERY},
   {"pos",		A_POS},
   {"posx",		A_POSX},
   {"posy",		A_POSY},
   {"reverse",		A_REVERSE},
   {"row",		A_ROW},
   {"rows",		A_ROWS},
   {"size",		A_SIZE},
   {"visual",		A_VISUAL},
   {"width",		A_WIDTH},
   {"windowlabel",	A_WINDOWLABEL},
   {"x",		A_X},
   {"y",		A_Y},
};


/*
 * There are more, X-specific stringint arrays in ../common/xwindow.c
 */

#else					/* Graphics */
static char junk;		/* avoid empty module */
#endif					/* Graphics */
