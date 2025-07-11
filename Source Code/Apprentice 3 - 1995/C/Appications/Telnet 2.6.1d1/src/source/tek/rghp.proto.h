
/* rghp.c */
int RGHPoutfunc(int (*f )());
int RGHPnewwin(void);
int RGHPclrscr(int w);
int RGHPclose(int w);
int RGHPpoint(int w, int x, int y);
int RGHPdrawline(int w, int x0, int y0, int x1, int y1);
int RGHPpagedone(int w);
int RGHPdataline(int w, int data, int count);
int RGHPpencolor(int w, int color);
int RGHPcharmode(int w, int rotation, int size);
int RGHPshowcur(void);
int RGHPlockcur(void);
int RGHPhidecur(void);
int RGHPbell(int w);
int RGHPuncover(int w);
char *RGHPdevname(void);
int RGHPinit(void);
int RGHPinfo(int w, int a, int b, int c, int d, int v);
int RGHPgmode(void);
int RGHPtmode(void);
