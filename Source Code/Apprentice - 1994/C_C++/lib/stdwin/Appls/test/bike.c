#include "stdwin.h"
#include "tilist.h"

#define NFRONT	3
#define NBACK	6

#define VSTEP 50
#define HSCALE 470

#define TIPWIDTH 40

int front[NFRONT] = {28, 38, 48};
int back[NBACK] = {28, 24, 20, 17, 15, 13};

double verzet[NFRONT][NBACK];
double lowest, highest;

WINDOW *win;

TILIST *til;

TEXTITEM *fronttips[NFRONT];
TEXTITEM *backtips[NBACK];

calculate()
{
	int f, b;
	double v;
	
	lowest = 1e10;
	highest = 0.0;
	for (f = 0; f < NFRONT; ++f) {
		for (b = 0; b < NBACK; ++b) {
			v = (double)front[f] / (double)back[b];
			verzet[f][b] = v;
			if (v < lowest)
				lowest = v;
			if (v > highest)
				highest = v;
		}
	}
}

setuptil()
{
	int f, b;
	int h, v;
	
	til = tilcreate(win);
	
	h = 0;
	v = 2;
	for (b = 0; b < NBACK; ++b) {
		backtips[b] = tiladd(til, h, v, h+TIPWIDTH-2, 0, 1);
		h += TIPWIDTH;
	}
	
	h = 0;
	v = 0;
	for (f = 0; f < NFRONT; ++f) {
		v += VSTEP;
		fronttips[f] = tiladd(til, h, v, h+TIPWIDTH-2, 0, 1);
	}
	
	settilvalues();
}

settilvalues()
{
	int f, b;
	char buf[100];
	
	for (f = 0; f < NFRONT; ++f) {
		sprintf(buf, "%d", front[f]);
		tilsettext(fronttips[f], buf);
	}
	for (b = 0; b < NBACK; ++b) {
		sprintf(buf, "%d", back[b]);
		tilsettext(backtips[b], buf);
	}
}

gettilvalues()
{
	int f, b;
	char *text;
	
	for (f = 0; f < NFRONT; ++f) {
		text = tilgettext(fronttips[f]);
		sscanf(text, "%d", &front[f]);
	}
	for (b = 0; b < NBACK; ++b) {
		text = tilgettext(backtips[b]);
		sscanf(text, "%d", &back[b]);
	}
	settilvalues();
	calculate();
	wchange(win, 0, 0, 30000, 30000);
}

void
drawproc(win, left, top, right, bottom)
	WINDOW *win;
	int left, top, right, bottom;
{
	int f, b;
	int h, v;
	int lasth;
	
	tildraw(til, left, top, right, bottom);
	
	v = VSTEP;
	for (f = 0; f < NFRONT; ++f) {
		for (b = 0; b < NBACK; ++b) {
			h = (verzet[f][b] - lowest) *
				 (HSCALE - 2*TIPWIDTH) / (highest - lowest);
			h += 2*TIPWIDTH;
			wcprintf(50, h, v-2-wlineheight(), "%d", back[b]);
			wdrawbox(h-2, v-2, h+2, v+2);
			wcprintf(50, h, v+2, "%.2f", verzet[f][b]);
			if (b > 0) {
				wpaint(lasth+2, v-1, h-2, v+1);
			}
			lasth = h;
		}
		v += VSTEP;
	}
}

main(argc, argv)
	int argc;
	char **argv;
{
	winitargs(&argc, &argv);
	calculate();
	wsetdefwinsize(HSCALE + 4*wcharwidth('0'), (NFRONT+1) * VSTEP);
	win = wopen("Bike", drawproc);
	wsetdocsize(win, HSCALE + 4*wcharwidth('0'), (NFRONT+1) * VSTEP);
	setuptil();
	for (;;) {
		EVENT e;
		wgetevent(&e);
		if (tilevent(til, &e))
			continue;
		switch (e.type) {
		case WE_COMMAND:
			switch (e.u.command) {
			case WC_CLOSE:
				quit();
				/*NOTREACHED*/
			case WC_CANCEL:
				settilvalues();
				break;
			case WC_RETURN:
				gettilvalues();
				break;
			}
			break;
		case WE_CLOSE:
			quit();
			/*NOTREACHED*/
		}
	}
}

quit()
{
	wclose(win);
	wdone();
	exit(0);
	/*NOTREACHED*/
}
