/*

rghp.c by Aaron Contorer for NCSA
Copyright 1987, board of trustees, University of Illinois

Routines for HP-GL plotter output.  Only 1 window output at a time.

*/

#ifdef MPW
#pragma segment TEKHP
#endif

#include <stdio.h>

#include "rghp.proto.h"

#include "rghp.proto.h"
static signore(char *s);
static HPbegin(void);
static signore(char *s);
static HPbegin(void);

static char *HPname = "Hewlett-Packard HP-GL plotter";
static char busy; /* is device already in use */
static int winbot,winleft,wintall,winwide;
	/* position and size of window into virtual space */
static int (*outfunc)(char *);
	/* the function to call with pointer to strings */
static char HPtext[100];
	/* the string containing the HP-GL output text */
static int HPpenx,HPpeny;
static int HPblank;
static int HPcolor;

static signore(char *s)
/* Ignore the string pointer passed here. */
{}


RGHPoutfunc(int (*f )())
/*
	Specify the function that is to be called with pointers to all
	the HP-GL strings.
*/
{
	outfunc = f;
}


static HPbegin(void)
/* set up environment for whole new printout */
{
	(*outfunc)("IN;SP1;SC-50,4370,-100,4120;PU0,0;");
	HPpenx = HPpeny = 0;
}

int RGHPnewwin(void)
{
	if (busy) return(-1);

	HPtext[0] = '\0';
	HPpenx = HPpeny = 0;
	HPblank = TRUE;
	HPcolor = 100;
	return(0);
}

RGHPclrscr(int w){
	RGHPpagedone(w);
}

RGHPclose(int w) {
	RGHPclrscr(w);
	busy=FALSE;
}

RGHPpoint(int w, int x, int y) {
	(*outfunc)("PD;PU;");
} 

RGHPdrawline(int w, int x0, int y0, int x1, int y1)
{
	if (HPblank) {
		HPbegin();
		HPblank = FALSE;
	}

	if (x0 != HPpenx || y0 != HPpeny) {
		/* only move pen if not already there */
		sprintf(HPtext,"PU%d,%d;",x0, y0);
		(*outfunc)(HPtext);
	}
	sprintf(HPtext,"PD%d,%d;",x1, y1);
	(*outfunc)(HPtext);
	HPpenx=x1;
	HPpeny=y1;
}

RGHPpagedone(int w) {
	(*outfunc)("PG;");
	HPblank = TRUE;
}

RGHPdataline(int w, int data, int count) {}

RGHPpencolor(int w, int color) {
	color &= 7;
	if (color) {
		sprintf(HPtext,"SP%d;",color);
		(*outfunc)(HPtext);
	}
}

RGHPcharmode(int w, int rotation, int size) {}
RGHPshowcur(void) {}
RGHPlockcur(void) {}
RGHPhidecur(void) {}
RGHPbell(int w) {}
RGHPuncover(int w) {}

char *RGHPdevname(void) {
	return(HPname);
}

RGHPinit(void) {
	busy=FALSE;
	outfunc=signore;
}

RGHPinfo(int w, int a, int b, int c, int d, int v) {}

RGHPgmode(void) {}
RGHPtmode(void) {}
