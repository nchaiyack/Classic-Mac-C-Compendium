
/* rsmac.c */
void	RSunload(void);
void RSinitall(short max);
short RSTextSelected(short w);
void RSsetConst(short w);
short RSsetwind(short w);
void RSbell(short w);
void RScursblink(short w);
void RScursblinkon(short w);
void RScursblinkoff(short w);
void RScursoff(short w);
void RScurson(short w, short x, short y);
void RSTextFont(short myfnum, short myfsiz, short myface);
void RSsetattr(short a);
void DoHiliteMode(void);
void RSinvText(short w, Point curr, Point last, RectPtr constrain);
void RSdraw(short w, short x, short y, short a, short len, char *ptr);
void RSdelcols(short w, short n);
void RSdelchars(short w, short x, short y, short n);
void RSdellines(short w, short t, short b, short n, short scrolled);
void RSerase(short w, short x1, short y1, short x2, short y2);
void RSinslines(short w, short t, short b, short n, short scrolled);
void RSinscols(short w, short n);
void RSinsstring(short w, short x, short y, short a, short len, char *ptr);
void RSsendstring(short w, char *ptr, short len);
void RSmargininfo(short w, short total, short current);
void RSbufinfo(short w, short total, short current, short bottom);
GrafPtr RSgetwindow(short w);
void RSattach(short w, GrafPtr wind);
void RSdetach(short w);
short RSfindvwind(GrafPtr wind);
short RSfindscroll(ControlHandle control, short *n);
void RSregnconv(RgnHandle regn, short *x1, short *y1, short *x2, short *y2, short fh, short fw);
short RSupdate(GrafPtr wind);
short RSupdprint(WindowPeek wind);
Point getlocalmouse(GrafPtr wind);
Point normalize(Point in_long, short w);
void HandleDoubleClick(short w);
void	RSsortAnchors(short w);
void RSselect(short w, Point pt, EventRecord theEvent);
void RSactivate(short w);
void RSdeactivate(short w);
char **RSGetTextSel(short w, short table);
void RSsetsize(short w, short v, short h);
void RSdrawsep(short w, short y1, short draw);
void RSfontmetrics(void);
void RSchangefont(short w, short fnum, long fsiz, short resizwind);
short RSgetfont(short w, short *pfnum, short *pfsiz);
short RSnewwindow(RectPtr wDims, short scrollback, short width, short lines, StringPtr name, short wrapon, short fnum, short fsiz, short showit, short goaway, short forcesave);
void RSkillwindow(short w);
void RSzoom(GrafPtr window, short code, short shifted);
void RSsize(GrafPtr window, long *where, long modifiers);
short RSclick(GrafPtr window, EventRecord theEvent);
pascal void ScrollProc(ControlHandle control, short part);
void RShide(short w);
void RSshow(short w);
void RScprompt(short w);
short RSsetcolor(short w, short n, unsigned short r, unsigned short g, unsigned short b);
Boolean NewRSsetcolor(short w, short n, RGBColor Color);
void RSgetcolor(short w, short n, unsigned short *r, unsigned short *g, unsigned short *b);
short RSmouseintext(short w, Point myPoint);
void RSskip(short w, Boolean on);
short RSokmem(short amount);
void RScalcwsize(short w, short width);
void RSbackground(short w, short value);
void RScheckmaxwind(Rect *origRect, short origW, short origH, short *endW, short *endH);
short ClickInContent(Point where, short w);
