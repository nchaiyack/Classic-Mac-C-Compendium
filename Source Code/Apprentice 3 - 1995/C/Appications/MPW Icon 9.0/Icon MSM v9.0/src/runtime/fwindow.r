/*
 * File: fwindow.r - Icon graphics interface
 *
 * Contents: Active, Bg, Color, CopyArea, Couple,
 *  DrawArc, DrawCircle, DrawCurve, DrawImage, DrawLine,
 *  DrawSegment, DrawPoint, DrawPolygon, DrawString,
 *  DrawRectangle, EraseArea, Event, Fg, FillArc, FillCircle,
 *  FillRectangle, FillPolygon, Font, FreeColor, GotoRC, GotoXY,
 *  NewColor, Pattern, PaletteChars, PaletteColor, PaletteKey,
 *  Pending, QueryPointer, ReadImage, TextWidth, Uncouple,
 *  WAttrib, WDefault, WFlush, WSync, WriteImage
 */

#ifdef Graphics

/*
 * Global variables.
 *  A poll counter for use in interp.c,
 *  the binding for the console window - FILE * for simplicity,
 *  &col, &row, &x, &y, &interval, timestamp, and modifier keys.
 */
int pollctr;
FILE *ConsoleBinding = NULL;

#ifndef MultiThread
struct descrip amperX = {D_Integer};
struct descrip amperY = {D_Integer};
struct descrip amperCol = {D_Integer};
struct descrip amperRow = {D_Integer};
struct descrip amperInterval = {D_Integer};
struct descrip lastEventWin = {D_Null};
uword prevtimestamp;
uword xmod_control, xmod_shift, xmod_meta;
#endif					/* MultiThread */


"Active() - produce the next active window"

function{0,1} Active()
   abstract {
      return file
      }
   body {
      wsp ws;
      if (!wstates || !(ws = getactivewindow())) fail;
      return ws->filep;
      }
end


"Alert(w,volume) - Alert the user"

function{1} Alert(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0;
      C_integer volume;
      OptWindow(w);

      if (argc == warg) volume = 0;
      else if (!def:C_integer(argv[warg], 0, volume))
        runerr(101, argv[warg]);
      walert(w, volume);
      ReturnWindow;
      }
end

"Bg(w,s) - background color"

function{0,1} Bg(argv[argc])
   abstract {
      return string
      }
   body {
      wbp w;
      char sbuf1[MaxCvtLen];
      int len;
      tended char *tmp;
      int warg = 0;
      OptWindow(w);

      /*
       * If there is a (non-window) argument we are setting by
       * either a mutable color (negative int) or a string name.
       */
      if (argc - warg > 0) {
	 if (is:integer(argv[warg])) {    /* mutable color or packed RGB */
	    if (!isetbg(w, IntVal(argv[warg]))) fail;
	    }
	 else {
	    if (!cnv:C_string(argv[warg], tmp))
	       runerr(103,argv[warg]);
	    if(!setbg(w, tmp)) fail;
	    }
         }

      /*
       * In any event, this function returns the current background color.
       */
      getbg(w, sbuf1);
      len = strlen(sbuf1);
      Protect(tmp = alcstr(sbuf1, len), runerr(0));
      return string(len, tmp);
      }
end


"Clip(w, x, y, w, h) - set context clip rectangle"

function{1} Clip(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0, r;
      C_integer x, y, width, height;
      wcp wc;
      OptWindow(w);

      wc = w->context;

      if (argc == 0) {
         wc->clipx = wc->clipy = 0;
         wc->clipw = wc->cliph = -1;
         unsetclip(w);
         }
      else {
         r = rectargs(w, argc, argv, warg, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);
         wc->clipx = x;
         wc->clipy = y;
         wc->clipw = width;
         wc->cliph = height;
         setclip(w);
         }

      ReturnWindow;
      }
end


"Clone(w, attribs...) - create a new context bound to w's canvas"

function{1} Clone(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w, w2;
      int warg = 0, n;
      tended struct descrip sbuf, sbuf2;
      char answer[128];
      OptWindow(w);

      Protect(w2 = alc_wbinding(), runerr(0));
      w2->window = w->window;
      w2->window->refcount++;

      if (argc>warg && is:file(argv[warg])) {
	 if ((BlkLoc(argv[warg])->file.status & Fs_Window) == 0)
	    runerr(140,argv[warg]);
	 if ((BlkLoc(argv[warg])->file.status & (Fs_Read|Fs_Write)) == 0)
	    runerr(142,argv[warg]);
	 Protect(w2->context =
		 clone_context((wbp)BlkLoc(argv[warg])->file.fd), runerr(0));
	 warg++;
	 }
      else {
	 Protect(w2->context = clone_context(w), runerr(0));
	 }

      for (n = warg; n < argc; n++) {
	 if (!is:null(argv[n])) {
	    if (!cnv:tmp_string(argv[n], sbuf))
	       runerr(109, argv[n]);
	    switch (wattrib(w2, StrLoc(argv[n]), StrLen(argv[n]), &sbuf2, answer)) {
	    case Failed: fail;
	    case Error: runerr(0, argv[n]);
	       }
	    }
	 }

      Protect(BlkLoc(result) =
	       (union block *)alcfile((FILE *)w2, Fs_Window|Fs_Read|Fs_Write,
				      &emptystr),runerr(0));
      result.dword = D_File;
      return result;
      }
end



"Color(argv[]) - return or set color map entries"

function{0,1} Color(argv[argc])
   abstract {
      return file ++ string
      }
   body {
      wbp w;
      int warg = 0;
      int i, len;
      C_integer n;
      char *colorname, *srcname;
      tended char *tmp;

      OptWindow(w);
      if (argc - warg == 0) runerr(101);

      if (argc - warg == 1) {			/* if this is a query */
         CnvCInteger(argv[warg], n)
         if ((colorname = get_mutable_name(w, n)) == NULL)
            fail;
         len = strlen(colorname);
         Protect(tmp = alcstr(colorname, len), runerr(0));
         return string(len, colorname);
         }

      CheckArgMultiple(2);

      for (i = warg; i < argc; i += 2) {
         CnvCInteger(argv[i], n)
         if ((colorname = get_mutable_name(w, n)) == NULL)
            fail;

         if (is:integer(argv[i+1])) {		/* copy another mutable  */
            if (IntVal(argv[i+1]) >= 0)
               runerr(205, argv[i+1]);		/* must be negative */
            if ((srcname = get_mutable_name(w, IntVal(argv[i+1]))) == NULL)
               fail;
            if (set_mutable(w, n, srcname) == Failed) fail;
            strcpy(colorname, srcname);
            }
   
         else {					/* specified by name */
            tended char *tmp;
            if (!cnv:C_string(argv[i+1],tmp))
               runerr(103,argv[i+1]);
   
            if (set_mutable(w, n, tmp) == Failed) fail;
            strcpy(colorname, tmp);
            }
         }

      ReturnWindow;
      }
end


"ColorValue(w,s) - produce RGB components from string color name"

function{0,1} ColorValue(argv[argc])
   abstract {
      return string
      }
   body {
      wbp w;
      int warg = 0, len, r, g, b;
      tended char *s;
      char tmp[24];

      if (is:file(argv[0]) && (BlkLoc(argv[0])->file.status & Fs_Window)) {
         w = (wbp)BlkLoc(argv[0])->file.fd;		/* explicit window */	
         warg = 1;
         }
      else if (is:file(kywd_xwin[XKey_Window]) &&
            (BlkLoc(kywd_xwin[XKey_Window])->file.status & Fs_Window))
         w = (wbp)BlkLoc(kywd_xwin[XKey_Window])->file.fd;	/* &window */
      else
         w = NULL;			/* no window (but proceed anyway) */

      if (!(warg < argc))
         runerr(103);
      if (!cnv:C_string(argv[warg], s))
         runerr(103,argv[warg]);
      if (parsecolor(w, s, &r, &g, &b) == Succeeded) {
         sprintf(tmp,"%d,%d,%d", r, g, b);
	 len = strlen(tmp);
	 Protect(s = alcstr(tmp,len), runerr(306));
	 return string(len, s);
         }
      fail;
      }
end


"CopyArea(w,w2,x,y,width,height,x2,y2) - copy area"

function{0,1} CopyArea(argv[argc]) /* w,w2,x,y,width,height,x2,y2 */
   abstract {
      return file
      }
   body {
      int warg = 0, n, r;
      C_integer x, y, width, height, x2, y2, width2, height2;
      wbp w, w2;
      OptWindow(w);

      /*
       * 2nd window defaults to value of first window
       */
      if (argc>warg && is:file(argv[warg])) {
	 if ((BlkLoc(argv[warg])->file.status & Fs_Window) == 0)
	    runerr(140,argv[warg]);
	 if ((BlkLoc(argv[warg])->file.status & (Fs_Read|Fs_Write)) == 0)
	    runerr(142,argv[warg]);
	 w2 = (wbp)BlkLoc(argv[warg])->file.fd;
	 warg++;
	 }
      else {
	 w2 = w;
	 }

      /*
       * x1, y1, width, and height follow standard conventions.
       */
      r = rectargs(w, argc, argv, warg, &x, &y, &width, &height);
      if (r >= 0)
         runerr(101, argv[r]);

      /*
       * get x2 and y2, ignoring width and height.
       */
      n = argc;
      if (n > warg + 6)
         n = warg + 6;
      r = rectargs(w2, n, argv, warg + 4, &x2, &y2, &width2, &height2);
      if (r >= 0)
         runerr(101, argv[r]);

      if (copyArea(w, w2, x, y, width, height, x2, y2) == Failed)
         fail;
      ReturnWindow;
      }
end

/*
 * Bind the canvas associated with w to the context
 *  associated with w2.  If w2 is omitted, create a new context.
 *  Produces a new window variable.
 */
"Couple(w,w2) - couple canvas to context"

function{0,1} Couple(w,w2)
   abstract {
      return file
      }
   body {
      tended struct descrip sbuf, sbuf2;
      tended struct b_list *hp;
      tended struct b_lelem *bp;
      wbp wb, wb_new;
      wsp ws;

      /*
       * make the new binding
       */
      Protect(wb_new = alc_wbinding(), runerr(0));

      /*
       * if w is a file, then we bind to an existing window
       */
      if (is:file(w) && (BlkLoc(w)->file.status & Fs_Window)) {
	 wb = (wbp)(BlkLoc(w)->file.fd);
	 wb_new->window = ws = wb->window;
	 if (is:file(w2) && (BlkLoc(w2)->file.status & Fs_Window)) {
	    /*
	     * Bind an existing window to an existing context,
	     * and up the context's reference count.
	     */
	    if (rebind(wb_new, (wbp)(BlkLoc(w2)->file.fd)) == Failed) fail;
	    wb_new->context->refcount++;
	    }
	 else 
	    runerr(140, w2);

	 /* bump up refcount to ws */
	 ws->refcount++;
	 }
      else
	 runerr(140, w);

      Protect(BlkLoc(result) =
	 (union block *)alcfile((FILE *)wb_new,	Fs_Window|Fs_Read|Fs_Write,
				&emptystr),runerr(0));
      result.dword = D_File;
      return result;
      }
end

/*
 * DrawArc(w, x1, y1, width1, height1, angle11, angle21,...)
 */
"DrawArc(argv[]){1} - draw arc"

function{1} DrawArc(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, r, warg = 0;
      XArc arcs[MAXXOBJS];
      C_integer x, y, width, height;
      double a1, a2;
      SHORT angle1, angle2;

      OptWindow(w);
      j = 0;
      for (i = warg; i < argc || i == warg; i += 6) {
         if (j == MAXXOBJS) {
            drawarcs(w, arcs, MAXXOBJS);
            j = 0;
            }
         r = rectargs(w, argc, argv, i, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);

         arcs[j].x = x;
         arcs[j].y = y;
         if (width > 0) width--;
         if (height > 0) height--;
         ARCWIDTH(arcs[j]) = width;
         ARCHEIGHT(arcs[j]) = height;

         if (i + 4 >= argc || is:null(argv[i + 4]))
            angle1 = 0;
         else {
            if (!cnv:C_double(argv[i + 4], a1))
               runerr(102, argv[i + 4]);
            if (a1 >= 0.0)
               a1 = fmod(a1, 2 * Pi);
            else
               a1 = -fmod(-a1, 2 * Pi);
            angle1 = -a1 * 180 / Pi * 64;
            }
         if (i + 5 >= argc || is:null(argv[i + 5]))
            angle2 = FULLARC;
         else {
            if (!cnv:C_double(argv[i + 5], a2))
               runerr(102, argv[i + 5]);
            if (fabs(a2) > 3 * Pi)
               runerr(101, argv[i + 5]);
            angle2 = -a2 * 180 / Pi * 64;
            }
         if (abs(angle2) >= (360<<6))
            arcs[j].angle2 = FULLARC;
         else {
            if (angle2 < 0) {
               angle1 += angle2;
               angle2 = abs(angle2);
               }
            arcs[j].angle2 = EXTENT(angle2);
            }
         if (angle1 < 0)
            angle1 = (360<<6) - (abs(angle1) % (360<<6));
         else
            angle1 %= (360 << 6);
         arcs[j].angle1 = ANGLE(angle1);

         j++;
         }

      drawarcs(w, arcs, j);
      ReturnWindow;
      }
end

/*
 * DrawCircle(w, x1, y1, r1, angle11, angle21, ...)
 */
"DrawCircle(argv[]){1} - draw circle"

function{1} DrawCircle(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0, r;

      OptWindow(w);
      r = docircles(w, argc - warg, argv + warg, 0);
      if (r < 0)
         ReturnWindow;
      else if (r >= argc - warg)
         runerr(146);
      else 
         runerr(102, argv[warg + r]);
      }
end

/*
 * DrawCurve(w,x1,y1,...xN,yN)
 *  Draw a smooth curve through the given points.
 */
"DrawCurve(argv[]){1} - draw curve"

function{1} DrawCurve(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, n, closed = 0, warg = 0;
      C_integer dx, dy, x0, y0, xN, yN;
      XPoint *points;

      OptWindow(w);
      CheckArgMultiple(2);

      dx = w->context->dx;
      dy = w->context->dy;

      Protect(points = (XPoint *)malloc(sizeof(XPoint) * (n+2)), runerr(305));

      if (n > 1) {
	 CnvCInteger(argv[warg], x0)
	 CnvCInteger(argv[warg + 1], y0)
	 CnvCInteger(argv[argc - 2], xN)
	 CnvCInteger(argv[argc - 1], yN)
         if ((x0 == xN) && (y0 == yN)) {
            closed = 1;               /* duplicate the next to last point */
	    CnvCShort(argv[argc-4], points[0].x);
	    CnvCShort(argv[argc-3], points[0].y);
	    points[0].x += w->context->dx;
	    points[0].y += w->context->dy;
            }
         else {                       /* duplicate the first point */
	    CnvCShort(argv[warg], points[0].x);
	    CnvCShort(argv[warg + 1], points[0].y);
	    points[0].x += w->context->dx;
	    points[0].y += w->context->dy;
            }
         for (i = 1; i <= n; i++) {
	    int base = warg + (i-1) * 2;
            CnvCShort(argv[base], points[i].x);
            CnvCShort(argv[base + 1], points[i].y);
	    points[i].x += dx;
	    points[i].y += dy;
            }
         if (closed) {                /* duplicate the second point */
            points[i] = points[2];
            }
         else {                       /* duplicate the last point */
            points[i] = points[i-1];
            }
	 if (n < 3) {
	    drawlines(w, points+1, n);
	    }
	 else {
	    drawCurve(w, points, n+2);
	    }
         }
      free(points);
      ReturnWindow;
      }
end


"DrawImage(w,x,y,s) - draw bitmapped figure"

function{0,1} DrawImage(argv[argc])
   abstract {
      return null++integer
      }
   body {
      wbp w;
      int warg = 0;
      int c, i, width, height, row, p;
      C_integer x, y;
      word nchars;
      unsigned char *s, *t, *z;
      struct descrip d;
      struct palentry *e;
      OptWindow(w);

      /*
       * X or y can be defaulted but s is required.
       * Validate x/y first so that the error message makes more sense.
       */
      if (argc - warg >= 1 && !def:C_integer(argv[warg], 0, x))
         runerr(101, argv[warg]);
      if (argc - warg >= 2 && !def:C_integer(argv[warg + 1], 0, y))
         runerr(101, argv[warg + 1]);
      if (argc - warg < 3)
         runerr(103);			/* missing s */
      if (!cnv:tmp_string(argv[warg+2], d))
         runerr(103, argv[warg + 2]);

      x += w->context->dx;
      y += w->context->dy;
      /*
       * Extract the Width and skip the following comma.
       */
      s = (unsigned char *)StrLoc(d);
      z = s + StrLen(d);		/* end+1 of string */
      width = 0;
      while (s < z - 1 && isdigit(*s))
         width = 10 * width + *s++ - '0';
      if (width == 0 || *s++ != ',')
         fail;

      /*
       * Check for a bilevel format.
       */
      if ((c = *s) == '#' || c == '~') {
         s++;
         nchars = 0;
         for (t = s; t < z; t++)
            if (isxdigit(*t))
               nchars++;			/* count hex digits */
            else if (*t != PCH1 && *t != PCH2)
               fail;				/* illegal punctuation */
         if (nchars == 0)
            fail;
         row = (width + 3) / 4;			/* digits per row */
         if (nchars % row != 0)
            fail;
         height = nchars / row;
         if (blimage(w, x, y, width, height, c, s, (word)(z - s)) == Error)
            runerr(305);
         else
            return nulldesc;
         }

      /*
       * Extract the palette name and skip its comma.
       */
      c = *s++;
      if (s >= z - 2 || !isdigit(*s))
         fail;
      p = 0;
      while (s < z - 1 && isdigit(*s))
         p = 10 * p + *s++ - '0';
      if (*s++ != ',')
         fail;
      if (c == 'g' && p >= 2 && p <= 256)
         p = -p;
      else if (c != 'c' || p < 1 || p > 6)
         fail;

      /*
       * Scan the image to see which colors are needed.
       */
      e = palsetup(p); 
      if (e == NULL)
         runerr(305);
      for (i = 0; i < 256; i++)
         e[i].used = 0;
      nchars = 0;
      for (t = s; t < z; t++) {
         c = *t; 
         e[c].used = 1;
         if (e[c].valid || c == TCH1 || c == TCH2)
            nchars++;			/* valid color, or transparent */
         else if (c != PCH1 && c != PCH2)
            fail;
         }
      if (nchars == 0)
         fail;					/* empty image */
      if (nchars % width != 0)
         fail;					/* not rectangular */

      /*
       * Call platform-dependent code to draw the image.
       */
      height = nchars / width;
      i = strimage(w, x, y, width, height, e, s, (word)(z - s), 0);
      if (i == 0)
         return nulldesc;
      else if (i < 0)
         runerr(305);
      else
         return C_integer i;
      }
end

/*
 * DrawLine(w,x1,y1,...xN,yN)
 */
"DrawLine(argv[]){1} - draw line"

function{1} DrawLine(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, n, warg = 0;
      XPoint points[MAXXOBJS];
      int dx, dy;

      OptWindow(w);
      CheckArgMultiple(2);

      dx = w->context->dx;
      dy = w->context->dy;
      for(i=0, j=0;i<n;i++, j++) {
	 int base = warg + i * 2;
         if (j==MAXXOBJS) {
	    drawlines(w, points, MAXXOBJS);
	    points[0] = points[MAXXOBJS-1];
   	    j = 1;
            }
         CnvCShort(argv[base], points[j].x);
         CnvCShort(argv[base + 1], points[j].y);
	 points[j].x += dx;
	 points[j].y += dy;
         }
      drawlines(w, points, j);
      ReturnWindow;
      }
end

/*
 * DrawPoint(w, x1, y1, ...xN, yN)
 */
"DrawPoint(argv[]){1} - draw point"

function{1} DrawPoint(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, n, warg = 0;
      XPoint points[MAXXOBJS];
      int dx, dy;

      OptWindow(w);
      CheckArgMultiple(2);

      dx = w->context->dx;
      dy = w->context->dy;
      for(i=0, j=0; i < n; i++, j++) {
	 int base = warg + i * 2;
         if (j == MAXXOBJS) {
	    drawpoints(w, points, MAXXOBJS);
            j = 0;
            }
         CnvCShort(argv[base], points[j].x);
         CnvCShort(argv[base + 1], points[j].y);
	 points[j].x += dx;
	 points[j].y += dy;
       }
      drawpoints(w, points, j);
      ReturnWindow;
      }
end

/*
 * DrawPolygon(w,x1,y1,...xN,yN)
 */
"DrawPolygon(argv[]){1} - draw polygon"

function{1} DrawPolygon(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, n, base, dx, dy, warg = 0;
      XPoint points[MAXXOBJS];

      OptWindow(w);
      CheckArgMultiple(2);

      dx = w->context->dx;
      dy = w->context->dy;

      /*
       * To make a closed polygon, start with the *last* point.
       */
      CnvCShort(argv[argc - 2], points[0].x);
      CnvCShort(argv[argc - 1], points[0].y);
      points[0].x += dx;
      points[0].y += dy;

      /*
       * Now add all points from beginning to end, including last point again.
       */
      for(i = 0, j = 1; i < n; i++, j++) {
         base = warg + i * 2;
         if (j == MAXXOBJS) {
            drawlines(w, points, MAXXOBJS);
            points[0] = points[MAXXOBJS-1];
            j = 1;
            }
         CnvCShort(argv[base], points[j].x);
         CnvCShort(argv[base + 1], points[j].y);
         points[j].x += dx;
         points[j].y += dy;
         }
      drawlines(w, points, j);
      ReturnWindow;
      }
end

/*
 * DrawRectangle(w, x1, y1, width1, height1, ..., xN, yN, widthN,heightN)
 */
"DrawRectangle(argv[]){1} - draw rectangle"

function{1} DrawRectangle(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, r, warg = 0;
      XRectangle recs[MAXXOBJS];
      C_integer x, y, width, height;

      OptWindow(w);
      j = 0;

      for (i = warg; i < argc || i == warg; i += 4) {
         r = rectargs(w, argc, argv, i, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);
         if (j == MAXXOBJS) {
            drawrectangles(w,recs,MAXXOBJS);
            j = 0;
            }
         RECX(recs[j]) = x;
         RECY(recs[j]) = y;
         RECWIDTH(recs[j]) = width;
         RECHEIGHT(recs[j]) = height;
         j++;
         }

      drawrectangles(w, recs, j);
      ReturnWindow;
      }
end

/*
 * DrawSegment(x11,y11,x12,y12,...,xN1,yN1,xN2,yN2)
 */
"DrawSegment(argv[]){1} - draw line segment"

function{1} DrawSegment(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, n, warg = 0, dx, dy;
      XSegment segs[MAXXOBJS];

      OptWindow(w);
      CheckArgMultiple(4);

      dx = w->context->dx;
      dy = w->context->dy;
      for(i=0, j=0; i < n; i++, j++) {
	 int base = warg + i * 4;
         if (j == MAXXOBJS) {
	    drawsegments(w, segs, MAXXOBJS);
            j = 0;
            }
         CnvCShort(argv[base], segs[j].x1);
         CnvCShort(argv[base + 1], segs[j].y1);
         CnvCShort(argv[base + 2], segs[j].x2);
         CnvCShort(argv[base + 3], segs[j].y2);
	 segs[j].x1 += dx;
	 segs[j].x2 += dx;
	 segs[j].y1 += dy;
	 segs[j].y2 += dy;
         }
      drawsegments(w, segs, j);
      ReturnWindow;
      }
end

/*
 * DrawString(w, x1, y1, s1, ..., xN, yN, sN)
 */
"DrawString(argv[]){1} - draw text"

function{1} DrawString(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, n, len, warg = 0;
      char *s;
      int dx, dy;

      OptWindow(w);
      CheckArgMultiple(3);

      for(i=0; i < n; i++) {
         C_integer x, y;
	 int j, base = warg + i * 3;
         CnvCInteger(argv[base], x);
         CnvCInteger(argv[base + 1], y);
	 x += w->context->dx;
	 y += w->context->dy;
         CnvTmpString(argv[base + 2], argv[base + 2]);
	 s = StrLoc(argv[base + 2]);
	 len = StrLen(argv[base + 2]);

	 for(j=0; j < len; j++) {
	    if (s[j] == '\n') {
	       drawstring(w, x, y, s, j);
	       s += j+1;
	       len -= j+1;
	       j = -1;
	       y += LEADING(w);
	       }
	    }
	 drawstring(w, x, y, s, len);
         }
      ReturnWindow;
      }
end


"EraseArea(w,x,y,width,height) - clear an area of the window"

function{1} EraseArea(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0, i, r;
      C_integer x, y, width, height;
      OptWindow(w);

      for (i = warg; i < argc || i == warg; i += 4) {
         r = rectargs(w, argc, argv, i, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);
         eraseArea(w, x, y, width, height);
         }

      ReturnWindow;
      }
end


"Event(w) - return an event from a window"

function{1} Event(argv[argc])
   abstract {
      return string ++ integer
      }
   body {
      wbp w;
      C_integer i;
      tended struct descrip d;
      int warg = 0;
      OptWindow(w);

      d = nulldesc;
      i = wgetevent(w,&d);
      if (i == 0) {
         if (is:file(kywd_xwin[XKey_Window]) &&
               w == (wbp)BlkLoc(kywd_xwin[XKey_Window])->file.fd)
	    lastEventWin = kywd_xwin[XKey_Window];
	 else
	    lastEventWin = argv[warg-1];
	 return d;
	 }
      else if (i == -1)
	 runerr(141);
      else
	 runerr(143);
      }
end


"Fg(w,s) - foreground color"

function{0,1} Fg(argv[argc])
   abstract {
      return string
      }
   body {
      wbp w;
      char sbuf1[MaxCvtLen];
      int len;
      tended char *tmp;
      int warg = 0;
      OptWindow(w);

      /*
       * If there is a (non-window) argument we are setting by
       *  either a mutable color (negative int) or a string name.
       */
      if (argc - warg > 0) {
	 if (is:integer(argv[warg])) {	/* mutable color or packed RGB */
	    if (!isetfg(w, IntVal(argv[warg]))) fail;
	    }
	 else {
	    if (!cnv:C_string(argv[warg], tmp))
	       runerr(103,argv[warg]);
	    if(!setfg(w, tmp)) fail;
	    }
         }

      /*
       * In any case, this function returns the current foreground color.
       */
      getfg(w, sbuf1);
      len = strlen(sbuf1);
      Protect(tmp = alcstr(sbuf1, len), runerr(0));
      return string(len, tmp);
      }
end

/*
 * FillArc(w, x1, y1, width1, height1, angle11, angle21,...)
 */
"FillArc(argv[]){1} - fill arc"

function{1} FillArc(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, r, warg = 0;
      XArc arcs[MAXXOBJS];
      C_integer x, y, width, height;
      double a1, a2;
      SHORT angle1, angle2;

      OptWindow(w);
      j = 0;
      for (i = warg; i < argc || i == warg; i += 6) {
         if (j == MAXXOBJS) {
            fillarcs(w, arcs, MAXXOBJS);
            j = 0;
            }
         r = rectargs(w, argc, argv, i, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);

         arcs[j].x = x;
         arcs[j].y = y;
         ARCWIDTH(arcs[j]) = width;
         ARCHEIGHT(arcs[j]) = height;

         if (i + 4 >= argc || is:null(argv[i + 4]))
            angle1 = 0;
         else {
            if (!cnv:C_double(argv[i + 4], a1))
               runerr(102, argv[i + 4]);
            if (a1 >= 0.0)
               a1 = fmod(a1, 2 * Pi);
            else
               a1 = -fmod(-a1, 2 * Pi);
            angle1 = -a1 * 180 / Pi * 64;
            }
         if (i + 5 >= argc || is:null(argv[i + 5]))
            angle2 = FULLARC;
         else {
            if (!cnv:C_double(argv[i + 5], a2))
               runerr(102, argv[i + 5]);
            if (fabs(a2) > 3 * Pi)
               runerr(101, argv[i + 5]);
            angle2 = -a2 * 180 / Pi * 64;
            }
         if (abs(angle2) >= (360<<6))
            arcs[j].angle2 = FULLARC;
         else {
            if (angle2 < 0) {
               angle1 += angle2;
               angle2 = abs(angle2);
               }
            arcs[j].angle2 = EXTENT(angle2);
            }
         if (angle1 < 0)
            angle1 = (360<<6) - (abs(angle1) % (360<<6));
         else
           angle1 %= (360 << 6);
         arcs[j].angle1 = ANGLE(angle1);

         j++;
         }

      fillarcs(w, arcs, j);
      ReturnWindow;
      }
end

/*
 * FillCircle(w, x1, y1, r1, angle11, angle21, ...)
 */
"FillCircle(argv[]){1} - draw filled circle"

function{1} FillCircle(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0, r;

      OptWindow(w);
      r = docircles(w, argc - warg, argv + warg, 1);
      if (r < 0)
         ReturnWindow;
      else if (r >= argc - warg)
         runerr(146);
      else 
         runerr(102, argv[warg + r]);
      }
end

/*
 * FillPolygon(w, x1, y1, ...xN, yN)
 */
"FillPolygon(argv[]){1} - fill polygon"

function{1} FillPolygon(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, n, warg = 0;
      XPoint *points;
      int dx, dy;

      OptWindow(w);

      CheckArgMultiple(2)

      /*
       * Allocate space for all the points in a contiguous array,
       * because a FillPolygon must be performed in a single call.
       */
      n = argc>>1;
      Protect(points = (XPoint *)malloc(sizeof(XPoint) * n), runerr(305));
      dx = w->context->dx;
      dy = w->context->dy;
      for(i=0; i < n; i++) {
	 int base = warg + i * 2;
         CnvCShort(argv[base], points[i].x);
         CnvCShort(argv[base + 1], points[i].y);
	 points[i].x += dx;
         points[i].y += dy;
         }
      fillpolygon(w, points, n);
      free(points);
      ReturnWindow;
      }
end

/*
 * FillRectangle(w, x1, y1, width1, height1,...,xN, yN, widthN, heightN)
 */
"FillRectangle(argv[]){1} - draw filled rectangle"

function{1} FillRectangle(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int i, j, r, warg = 0;
      XRectangle recs[MAXXOBJS];
      C_integer x, y, width, height;

      OptWindow(w);
      j = 0;

      for (i = warg; i < argc || i == warg; i += 4) {
         r = rectargs(w, argc, argv, i, &x, &y, &width, &height);
         if (r >= 0)
            runerr(101, argv[r]);
         if (j == MAXXOBJS) {
            fillrectangles(w,recs,MAXXOBJS);
            j = 0;
            }
         RECX(recs[j]) = x;
         RECY(recs[j]) = y;
         RECWIDTH(recs[j]) = width;
         RECHEIGHT(recs[j]) = height;
         j++;
         }

      fillrectangles(w, recs, j);
      ReturnWindow;
      }
end



"Font(w,s) - get/set font"

function{0,1} Font(argv[argc])
   abstract {
      return string
      }
   body {
      tended char *tmp;
      int len;
      wbp w;
      int warg = 0;
      char buf[MaxCvtLen];
      OptWindow(w);

      if (warg < argc) {
         if (!cnv:C_string(argv[warg],tmp))
            runerr(103,argv[warg]);
         if (setfont(w,&tmp) == Failed) fail;
         }
      getfntnam(w, buf);
      len = strlen(buf);
      Protect(tmp = alcstr(buf, len), runerr(0));
      return string(len,tmp);
      }
end


"FreeColor(argv[]) - free colors"

function{1} FreeColor(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0;
      int i;
      C_integer n;
      tended char *s;

      OptWindow(w);
      if (argc - warg == 0) runerr(103);

      for (i = warg; i < argc; i++) {
         if (is:integer(argv[i])) {
            CnvCInteger(argv[i], n)
            if (n < 0)
               free_mutable(w, n);
            }
         else {
            if (!cnv:C_string(argv[i], s))
               runerr(103,argv[i]);
            freecolor(w, s);
            }
         }

      ReturnWindow;
      }

end


"GotoRC(w,r,c) - move cursor to a particular text row and column"

function{1} GotoRC(argv[argc])
   abstract {
      return file
      }
   body {
      C_integer r, c;
      wbp w;
      int warg = 0;
      OptWindow(w);

      if (argc - warg < 1)
	 r = 1;
      else
	 CnvCInteger(argv[warg], r)
      if (argc - warg < 2)
	 c = 1;
      else
	 CnvCInteger(argv[warg + 1], c)

      /*
       * turn the cursor off
       */
      HideCursor(w->window);

      w->window->y = ROWTOY(w, r);
      w->window->x = COLTOX(w, c);
      w->window->x += w->context->dx;
      w->window->y += w->context->dy;

      /*
       * turn it back on at new location
       */
      UpdateCursorPos(w->window, w->context);
      ShowCursor(w->window);

      ReturnWindow;
      }
end


"GotoXY(w,x,y) - move cursor to a particular pixel location"

function{1} GotoXY(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      C_integer x, y;
      int warg = 0;
      OptWindow(w);

      if (argc - warg < 1)
	 x = 0;
      else
	 CnvCInteger(argv[warg], x)
      if (argc - warg < 2)
	 y = 0;
      else
	 CnvCInteger(argv[warg + 1], y)

      x += w->context->dx;
      y += w->context->dy;

      HideCursor(w->window);

      w->window->x = x;
      w->window->y = y;

      UpdateCursorPos(w->window, w->context);
      ShowCursor(w->window);

      ReturnWindow;
      }
end


"Lower(w) - lower w to the bottom of the window stack"

function{1} Lower(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0;
      OptWindow(w);
      lowerWindow(w);
      ReturnWindow;
      }
end


"NewColor(w,s) - allocate an entry in the color map"

function{0,1} NewColor(argv[argc])
   abstract {
      return integer
      }
   body {
      wbp w;
      int rv;
      int warg = 0;
      OptWindow(w);

      if (mutable_color(w, argv+warg, argc-warg, &rv) == Failed) fail;
      return C_integer rv;
      }
end



"PaletteChars(w,p) - return the characters forming keys to palette p"

function{0,1} PaletteChars(argv[argc])
   abstract {
      return string
      }
   body {
      int n, warg;
      extern char c1list[], c2list[], c3list[], c4list[];

      if (is:file(argv[0]) && (BlkLoc(argv[0])->file.status & Fs_Window))
         warg = 1;
      else
         warg = 0;		/* window not required */
      if (argc - warg < 1)
         n = 1;
      else
         n = palnum(&argv[warg]);
      switch (n) {
         case -1:  runerr(103, argv[warg]);		/* not a string */
         case  0:  fail;				/* unrecognized */
         case  1:  return string(90, c1list);			/* c1 */
         case  2:  return string(9, c2list);			/* c2 */
         case  3:  return string(31, c3list);			/* c3 */
         case  4:  return string(73, c4list);			/* c4 */
         case  5:  return string(141, (char *)allchars);	/* c5 */
         case  6:  return string(241, (char *)allchars);	/* c6 */
         default:					/* gn */
            if (n >= -64)
               return string(-n, c4list);
            else
               return string(-n, (char *)allchars);
         }
      }
end


"PaletteColor(w,p,s) - return color of key s in palette p"

function{0,1} PaletteColor(argv[argc])
   abstract {
      return string
      }
   body {
      int p, warg, len;
      char tmp[24], *s;
      struct palentry *e;
      tended struct descrip d;

      if (is:file(argv[0]) && (BlkLoc(argv[0])->file.status & Fs_Window))
         warg = 1;
      else
         warg = 0;			/* window not required */
      if (argc - warg < 2)
         runerr(103);
      p = palnum(&argv[warg]);
      if (p == -1)
         runerr(103, argv[warg]);
      if (p == 0)
         fail;
      if (!cnv:tmp_string(argv[warg + 1], d))
         runerr(103, argv[warg + 1]);
      if (StrLen(d) != 1)
         runerr(205, d);
      e = palsetup(p); 
      if (e == NULL)
         runerr(305);
      e += *StrLoc(d) & 0xFF;
      if (!e->valid)
         fail;
      sprintf(tmp, "%d,%d,%d", e->clr.red, e->clr.green, e->clr.blue);
      len = strlen(tmp);
      Protect(s = alcstr(tmp, len), runerr(306));
      return string(len, s);
      }
end


"PaletteKey(w,p,s) - return key of closest color to s in palette p"

function{0,1} PaletteKey(argv[argc])
   abstract {
      return string
      }
   body {
      wbp w;
      int warg = 0, p;
      C_integer n;
      tended char *s;
      int r, g, b;

      if (is:file(argv[0]) && (BlkLoc(argv[0])->file.status & Fs_Window)) {
         w = (wbp)BlkLoc(argv[0])->file.fd;		/* explicit window */	
         warg = 1;
         }
      else if (is:file(kywd_xwin[XKey_Window]) &&
            (BlkLoc(kywd_xwin[XKey_Window])->file.status & Fs_Window))
         w = (wbp)BlkLoc(kywd_xwin[XKey_Window])->file.fd;	/* &window */
      else
         w = NULL;			/* no window (but proceed anyway) */

      if (argc - warg < 2)
         runerr(103);
      p = palnum(&argv[warg]);
      if (p == -1)
         runerr(103, argv[warg]);
      if (p == 0)
         fail;

      if (cnv:C_integer(argv[warg + 1], n)) {
         if (w == NULL || (s = get_mutable_name(w, n)) == NULL)
            fail;
         }
      else if (!cnv:C_string(argv[warg + 1], s))
         runerr(103, argv[warg + 1]);

      if (parsecolor(w, s, &r, &g, &b) == Succeeded)
         return string(1, rgbkey(p, r / 65535.0, g / 65535.0, b / 65535.0));
      else
         fail;
      }
end


"Pattern(w,s) - sets the context fill pattern by string name"

function{1} Pattern(argv[argc])
   abstract {
      return file
      }
   body {
      int warg = 0;
      wbp w;
      OptWindow(w);

      if (argc - warg == 0)
         runerr(103, nulldesc);

      if (! cnv:string(argv[warg], argv[warg]))
         runerr(103, nulldesc);

      switch (SetPattern(w, StrLoc(argv[warg]), StrLen(argv[warg]))) {
         case Error:
            runerr(0, argv[warg]);
         case Failed:
            fail;
         }

      ReturnWindow;
      }
end


"Pending(w,x[]) - produce a list of events pending on window"

function{0,1} Pending(argv[argc])
   abstract {
      return list
      }
   body {
      wbp w;
      int warg = 0;
      wsp ws;
      int i;
      OptWindow(w);

      ws = w->window;
      wsync(w);

      /*
       * put additional arguments to Pending on the pending list in
       * guaranteed consecutive order.
       */
      for (i = warg; i < argc; i++) {
         c_put(&(ws->listp), &argv[i]);
         }

      /*
       * retrieve any events that might be relevant before returning the
       * pending queue.
       */
      switch (pollevent()) {
         case -1: runerr(141);
         case 0: fail;
	 }
      return ws->listp;
      }
end



"Pixel(w,x,y,width,height) - produce the contents of some pixels"

function{3} Pixel(argv[argc])
   abstract {
      return integer ++ string
      }
   body {
      C_integer x, y, width, height;
      int x2, y2, width2, height2;
      wbp w;
      int warg = 0, slen, r;
      tended struct descrip lastval;
      char strout[50];
      OptWindow(w);

      r = rectargs(w, argc, argv, warg, &x, &y, &width, &height);
      if (r >= 0)
         runerr(101, argv[r]);

      {
      int i, j;
      long rv;
      wsp ws = w->window;

#ifndef max
#define max(x,y) (((x)<(y))?(y):(x))
#define min(x,y) (((x)>(y))?(y):(x))
#endif

      x2 = max(x,0);
      y2 = max(y,0);
      width2 = min(width, (int)ws->width - x2);
      height2 = min(height, (int)ws->height - y2);

      if (getpixel_init(w, x2, y2, width2, height2) == Failed) fail;

      lastval = emptystr;

      for (j=y; j < y + height; j++) {
         for (i=x; i < x + width; i++) {
            getpixel(w, i, j, &rv, strout);
            slen = strlen(strout);
            if (rv >= 0) {
               if (slen != StrLen(lastval) ||
                     strncmp(strout, StrLoc(lastval), slen)) {
                  Protect((StrLoc(lastval) = alcstr(strout, slen)), runerr(0));
                  StrLen(lastval) = slen;
                  }
               suspend lastval;
               }
            else {
               suspend C_integer rv;
               }
            }
         }
      fail;
      }
      }
end


"QueryPointer(w) - produce mouse position"

function{0,2} QueryPointer(w)

   declare {
      XPoint xp;
      }
   abstract {
      return integer
      }
   body {
      pollevent();
      if (is:null(w)) {
	 query_rootpointer(&xp);
	 }
      else {
	 if (!is:file(w) || !(BlkLoc(w)->file.status & Fs_Window))
	    runerr(140, w);
	 query_pointer((wbp)BlkLoc(w)->file.fd, &xp);
	 }
      suspend C_integer xp.x;
      suspend C_integer xp.y;
      fail;
      }
end


"Raise(w) - raise w to the top of the window stack"

function{1} Raise(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0;
      OptWindow(w);
      raiseWindow(w);
      ReturnWindow;
      }
end


"ReadImage(w, s, x, y, p) - load image file"

function{0,1} ReadImage(argv[argc])
   abstract {
      return integer
      }
   body {
      wbp w;
      char filename[MaxFileName + 1];
      tended char *tmp;
      int status, warg = 0;
      C_integer x, y;
      int p, r;
      struct imgdata imd;
      OptWindow(w);

      if (argc - warg == 0)
	 runerr(103,nulldesc);
      if (!cnv:C_string(argv[warg], tmp))
	 runerr(103,argv[warg]);

      /*
       * x and y must be integers; they default to 0.
       */
      if (argc - warg < 2) x = 0;
      else if (!def:C_integer(argv[warg+1], 0, x))
         runerr(101, argv[warg+1]);
      if (argc - warg < 3) y = 0;
      else if (!def:C_integer(argv[warg+2], 0, y))
         runerr(101, argv[warg+2]);

      /*
       * p is an optional palette name.
       */
      if (argc - warg < 4 || is:null(argv[warg+3])) p = 0;
      else {
         p = palnum(&argv[warg+3]);
         if (p == -1)
            runerr(103, argv[warg+3]);
         if (p == 0)
            fail;
         }

      x += w->context->dx;
      y += w->context->dy;
      mystrncpy(filename, tmp, MaxFileName);   /* copy to loc that won't move*/

      /*
       * First try to read as a GIF file.
       * If that doesn't work, try platform-dependent image reading code.
       */
      r = readGIF(filename, p, &imd);
      if (r == Succeeded) {
         status = strimage(w, x, y, imd.width, imd.height, imd.paltbl,
			   imd.data, (word)imd.width * (word)imd.height, 0);
         if (status < 0)
            r = Error;
         free((pointer)imd.paltbl);
         free((pointer)imd.data);
         }
      else if (r == Failed)
         r = readimage(w, filename, x, y, &status);
      if (r == Error)
         runerr(305);
      if (r == Failed)
         fail;
      if (status == 0)
         return nulldesc;
      else
         return C_integer (word)status;
      }
end



"WSync(w) - synchronize with server"

function{1} WSync(w)
   abstract {
      return file++null
      }
   body {
      wbp _w_;

      if (is:null(w)) {
	 _w_ = NULL;
	 }
      else if (!is:file(w)) runerr(140,w);
      else {
         if (!(BlkLoc(w)->file.status & Fs_Window))
            runerr(140,w);
         _w_ = (wbp)BlkLoc(w)->file.fd;
	 }

      wsync(_w_);
      pollevent();
      return w;
      }
end


"TextWidth(w,s) - compute text pixel width"

function{1} TextWidth(argv[argc])
   abstract {
      return integer
      }
   body {
      wbp w;
      int warg=0;
      C_integer i;
      OptWindow(w);

      if (warg == argc) runerr(103,nulldesc);
      else if (!cnv:tmp_string(argv[warg],argv[warg]))
	 runerr(103,argv[warg]);
       
      i = TEXTWIDTH(w, StrLoc(argv[warg]), StrLen(argv[warg]));
      return C_integer i;
      }
end


"Uncouple(w) - uncouple window"

function{1} Uncouple(w)
   abstract {
      return file
      }
   body {
      wbp _w_;
      if (!is:file(w)) runerr(140,w);
      if ((BlkLoc(w)->file.status & Fs_Window) == 0) runerr(140,w);
      if ((BlkLoc(w)->file.status & (Fs_Read|Fs_Write)) == 0) runerr(142,w);
      _w_ = (wbp)BlkLoc(w)->file.fd;
      BlkLoc(w)->file.status = Fs_Window; /* no longer open for read/write */
      free_binding(_w_);
      return w;
      }
end

"WAttrib(argv[]) - read/write window attributes"

function{*} WAttrib(argv[argc])
   abstract {
      return file++string
      }
   body {
      wbp w, wsave;
      word n;
      tended struct descrip sbuf, sbuf2;
      char answer[128];
      int  pass, config = 0;
      int warg = 0;
      OptWindow(w);

      wsave = w;
      /*
       * Loop through the arguments.
       */
      for (pass = 1; pass <= 2; pass++) {
	 w = wsave;
	 if (config && pass == 2) {
	    if (do_config(w, config) == Failed) fail;
	    }
         for (n = warg; n < argc; n++) {
            if (is:file(argv[n])) {/* Current argument is a file */
               /*
                * Switch the current file to the file named by the
                *  current argument providing it is a file.  argv[n]
                *  is made to be a empty string to avoid a special case.
                */
               if (!(BlkLoc(argv[n])->file.status & Fs_Window))
                  runerr(140,argv[n]);
               w = (wbp)BlkLoc(argv[n])->file.fd;
	       if (config && pass == 2) {
		  if (do_config(w, config) == Failed) fail;
		  }
               }
            else {	/* Current argument should be a string */
	       /*
		* In pass 1, a null argument is an error; failed attribute
		*  assignments are turned into null descriptors for pass 2
		*  and are ignored.
		*/
               if (is:null(argv[n])) {
		  if (pass == 2)
		     continue;
		  else runerr(109, argv[n]);
		  }
               /*
                * Convert the argument to a string
                */
               if (!cnv:tmp_string(argv[n], sbuf)) 
                  runerr(109, argv[n]);
               /*
                * Read/write the attribute
                */
               if ((pass == 1) &&
		  strnchr(StrLoc(sbuf), '=', StrLen(sbuf))) {
                  /*
                   * pass 1: perform attribute assignments
                   */
                  switch (wattrib(w, StrLoc(sbuf), StrLen(sbuf),
				  &sbuf2, answer)) {
		     case Failed:
		        /*
			 * Mark the attribute so we don't produce a result
			 */
		        argv[n] = nulldesc;
		        continue;
		     case Error: runerr(0, argv[n]);
		     }
		  if (StrLen(sbuf) > 4) {
		     if (!strncmp(StrLoc(sbuf), "pos=", 4)) config |= 1;
		     if (StrLen(sbuf) > 5) {
			if (!strncmp(StrLoc(sbuf), "posx=", 5)) config |= 1;
			if (!strncmp(StrLoc(sbuf), "posy=", 5)) config |= 1;
			if (!strncmp(StrLoc(sbuf), "rows=", 5)) config |= 2;
			if (!strncmp(StrLoc(sbuf), "size=", 5)) config |= 2;
			if (StrLen(sbuf) > 6) {
			   if (!strncmp(StrLoc(sbuf), "width=", 6))
			      config |= 2;
			   if (!strncmp(StrLoc(sbuf), "lines=", 6))
			      config |= 2;
			   if (StrLen(sbuf) > 7) {
			      if (!strncmp(StrLoc(sbuf), "height=", 7))
				 config |= 2;
			      if (StrLen(sbuf) > 8) {
				 if (!strncmp(StrLoc(sbuf), "columns=", 8))
				    config |= 2;
				 if (StrLen(sbuf) > 9) {
				    if (!strncmp(StrLoc(sbuf), "geometry=", 9))
				       config |= 3;
				    }
				 }
			      }
			   }
			}
		     }
                  }
	       /*
		* pass 2: perform attribute queries, suspend result(s)
		*/
               else if (pass==2) {
		  char *stmp;
		  /*
		   * Turn assignments into queries.
		   */
		  if (stmp = strnchr(StrLoc(sbuf),'=',StrLen(sbuf)))
		     StrLen(sbuf) = stmp - StrLoc(sbuf);

		  switch (wattrib(w, StrLoc(sbuf), StrLen(sbuf),
				  &sbuf2, answer)) {
		  case Failed: continue;
		  case Error:  runerr(0, argv[n]);
		     }
		  if (is:string(sbuf2))
		     Protect(StrLoc(sbuf2) = alcstr(StrLoc(sbuf2),StrLen(sbuf2)), runerr(0));
                  suspend sbuf2;
                  }
               }
            }
	 }
      fail;
      }
end


"WDefault(w,program,option) - get a default value from the environment"

function{0,1} WDefault(argv[argc])
   abstract {
      return string
      }
   body {
      wbp w;
      int warg = 0;
      long l;
      tended char *prog, *opt;
      char sbuf1[MaxCvtLen];
      OptWindow(w);

      if (argc-warg < 2)
         runerr(103);
      if (!cnv:C_string(argv[warg],prog))
         runerr(103,argv[warg]);
      if (!cnv:C_string(argv[warg+1],opt))
         runerr(103,argv[warg+1]);

      if (getdefault(w, prog, opt, sbuf1) == Failed) fail;
      l = strlen(sbuf1);
      Protect(prog = alcstr(sbuf1,l),runerr(0));
      return string(l,prog);
      }
end


"WFlush(w) - flush all output to window w"

function{1} WFlush(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int warg = 0;
      OptWindow(w);
      wflush(w);
      ReturnWindow;
      }
end


"WriteImage(w,filename,x,y,width,height) - write an image to a file"

function{0,1} WriteImage(argv[argc])
   abstract {
      return file
      }
   body {
      wbp w;
      int r;
      C_integer x, y, width, height, warg = 0;
      tended char *s;
      OptWindow(w);

      if (argc - warg == 0)
         runerr(103, nulldesc);
      else if (!cnv:C_string(argv[warg], s))
         runerr(103, argv[warg]);

      r = rectargs(w, argc, argv, warg + 1, &x, &y, &width, &height);
      if (r >= 0)
         runerr(101, argv[r]);

      /*
       * fail if image is zero-sized or is not contained within window
       */
      if ((x < 0) || (y < 0) || (width + x > w->window->pixwidth) ||
          (height + y > w->window->pixheight) || !width || !height) fail;

      /*
       * try platform-dependent code first; it will reject the call
       * if the file name s does not specify a platform-dependent format.
       */
      r = dumpimage(w, s, x, y, width, height);
      if (r == NoCvt)
         r = writeGIF(w, s, x, y, width, height);
      if (r != Succeeded)
         fail;
      ReturnWindow;
      }
end

#else					/* Graphics */
static char x;			/* avoid empty module */
#endif					/* Graphics */


