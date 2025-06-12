/* Program INUSE.C for the Megamax C compiler */
/* (C) 1986 William Woody, commercial rights reserved */

/* If you think this program is cute, please send me computer mail and tell */
/* me so!  I personally think it's a cute hack, myself, but what do I know. */

/* Though I don't believe this program has any commercial value, (or ANY */
/* monitary value whatsoever), I have reserved commercial rights anyways.*/
/* Silly me...  But if you want to give it to your friends, please leave */
/* my name on the darn thing; I need to feel appreciated.  */

#include <stdio.h>
#include <qdvars.h>		/* What they don't tell you is that you need to */
						/* include this for the SRAND macro to work... */

extern int random();
extern char *malloc();

#define MAXX 480
#define MAXY 300
#define MAXM 400

struct foo {
	double a,b;			/* This is a point which gets to fly around */
	struct foo *next;
};

main()
{
	int numb,n,x,done = 0;
	struct foo *top,*ptr,*ptr2;
	double mx,my,nx,ny;
	struct {
		int top,left,bottom,right;
	} r;
	long seed;

	_autowin("Inuse");	/* Megamax C instruction to create a default window */
	setrect(&r,0,0,512,342);
	
	eraserect(&r);
	moveto(10,10);
	textfont(0);
	textsize(12);
	printf("Inuse program ©1985 by William Woody, commercial rights reserved\n");
	printf("To halt the program, press the mouse button.\n");

	getdatetime(&seed);
	srand(seed);			/* Seed the random number generator */

	while (!done) {
		numb = (random() & 3) + 2;  /* Number of points generated */
		x = 0;
		top = ptr = (struct foo *)malloc(sizeof(struct foo));
		while (x++ != numb)
			ptr = (ptr->next = (struct foo *)malloc(sizeof(struct foo)));
		ptr = (ptr->next = top);
		mx = 0;
		my = 0;
		nx = 9999999.0;		/* Large numbers for finding minimum value */
		ny = 9999999.0;
		do {
			ptr->a = (double)random();
			if (ptr->a > mx) mx = ptr->a;
			if (ptr->a < nx) nx = ptr->a;
			ptr->b = (double)random();
			if (ptr->b > my) my = ptr->b;
			if (ptr->b < ny) ny = ptr->b;
			ptr = ptr->next;
		} while (ptr != top);		/* Make up the points and create a */
		do {						/* circularly linked list of them */
			ptr->a = (ptr->a - nx) * MAXX / (mx - nx);
			ptr->b = (ptr->b - ny) * MAXY / (my - ny);
			ptr = ptr->next;
		} while (ptr != top);
		n = 0;
		moveto((int)ptr->a,(int)ptr->b);
		while (n++ != MAXM * numb) {	/* Now, move the points around */
			ptr->a = (ptr->a * 40 + ptr->next->a) / 41;
			ptr->b = (ptr->b * 40 + ptr->next->b) / 41;
			lineto((int)ptr->a,(int)ptr->b);
			ptr = ptr->next;
			if (button()) {		/* Halt program when button is pressed */
				done = 1;
				break;
			}
		}
		ptr = top;				/* Clean up the circular linked list */
		do {
			ptr = (ptr2 = ptr)->next;
			free(ptr2);
		} while (ptr != top);

		eraserect(&r);
	}
}