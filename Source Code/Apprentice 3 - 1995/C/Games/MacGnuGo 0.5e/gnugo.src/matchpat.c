#include "comment.header"

#define EMPTY 0
#define MAXPC 16
#define abs(x) ((x) < 0 ? -(x) : (x))
#define line(x) (abs(x - 9))

extern unsigned char p[19][19];
extern int currentStone, opposingStone, MAXX, MAXY;
extern int lib;
extern void countlib(int,int,int);
extern int fiot(int,int,int);		/* filling in safe territory? */
extern int fieldval(int,int);

int selpat, selv;

/* match pattern and get next move */
int matchpat(int m, int n, int *i, int *j, int *val)
{
  struct patval {
    int x;	/* pattern x, y coordinate */
    int y;
    int att;	/* pattern attribute */
      /* att = 0 - empty, 1 - your piece, 2 - my piece, 3 - my next move */
      /* 4 - empty on edge, 5 - your piece on edge, 6 - my piece on edge */
      /* 7 - not my piece, 8 - not your piece */
  };

  struct pattern {
    struct patval patn[MAXPC];		/* pattern */
    int patlen;		/* number of pieces in pattern */
    int trfno;		/* number of transformations to try, 4 or 8 */
    int patwt;		/* pattern value */
  };
  
#include "patterns.c"	/* patterns in a seperate file */
  
  static int trf [8][2][2] = {		/* transformation matrice */
    {{ 1,  0}, { 0,  1}},  /* no transformation */
    {{ 1,  0}, { 0, -1}},  /* invert */
    {{ 0,  1}, {-1,  0}},  /* rotate 90 */
    {{ 0, -1}, {-1,  0}},  /* rotate 90 and invert */
    {{-1,  0}, { 0,  1}},  /* flip left */
    {{-1,  0}, { 0, -1}},  /* flip left and invert */
    {{ 0,  1}, { 1,  0}},  /* rotate 90 and flip left */
    {{ 0, -1}, { 1,  0}}   /* rotate 90, flip left and invert */
  };

  int k, my, nx, l, r, cont;
  int ti, tj, tval;
  int fv;
  
  *i = -1;   *j = -1;   *val = -1;		/* initialize */
  ti = tj = 0;  fv = 0;
  for (r = 0; r < PATNO; r++)
    if (pat[r].patwt + 10  > *val) {			/* try each pattern */
    /* try each orientation transformation */
      for (l = 0; l < pat[r].trfno; l++) {
	k = 0;					/* k = pattern point index */
	cont = 1;				/* set pattern ok flag */
	while ((k != pat[r].patlen) && cont) {	/* match each point */
	  /* transform pattern to a real coordinate */
	  nx = n + trf[l][0][0] * pat[r].patn[k].x
	      + trf[l][0][1] * pat[r].patn[k].y;
	  my = m + trf[l][1][0] * pat[r].patn[k].x
	      + trf[l][1][1] * pat[r].patn[k].y;
	    
	  if ((my < 0) || ( my > MAXY - 1) || (nx < 0) || (nx > MAXX - 1)) {
		cont = 0;			 /* it's outside the board */
		break;
	  }
	  switch (pat[r].patn[k].att) {
	    case 0 : if (p[my][nx] == EMPTY)		/* open */
	               break;
	             else {
		       cont = 0;	/* clear ok flag if not EMPTY */
	             }
		     break;
	    case 1 : if (p[my][nx] == opposingStone) 	/* your piece */
	               break;
	             else {
		       cont = 0;
	             }
		     break;
	    case 2 : if (p[my][nx] == currentStone) 	/* my piece */
	               break;
	             else {
		       cont = 0;
	             }
		     break;
	    case 3 : if (p[my][nx] == EMPTY) {		/* move here */
		       lib = 0; 			/* open for move */
		       countlib(my, nx, currentStone);	/* check liberty */
		       if (lib > 1 && !fiot(my,nx,3)) {
		         ti = my;			/* move o.k. */
		         tj = nx;
			 fv = fieldval(my,nx);
		       } else {
		         cont = 0;		/* suicide not allowed */
		       }
	             } else {
		       cont = 0;
	             }
		     break;
	    case 4 : if ((p[my][nx] == EMPTY)		/* open on edge */
			 && ((my == 0) || (my == MAXY - 1) ||
			     (nx == 0) || (nx == MAXX - 1)))
	               break;
	             else {
		       cont = 0;
	             }
		     break;
	    case 5 : if ((p[my][nx] == opposingStone) /* your piece on edge */
			 && ((my == 0) || (my == MAXY - 1) ||
			     (nx == 0) || (nx == MAXX - 1)))
	               break;
	             else {
		       cont = 0;
	             }
		     break;
	    case 6 : if ((p[my][nx] == currentStone)	/* my piece on edge */
			 && ((my == 0) || (my == MAXY - 1) ||
			     (nx == 0) || (nx == MAXX - 1)))
	               break;
	             else {
		       cont = 0;
	             }
		     break;
	    case 7 : if (p[my][nx] == opposingStone || p[my][nx] == EMPTY)
	               break;				/* not my piece */
	             else {
		       cont = 0;
	             }
		     break;
	    case 8 : if (p[my][nx] == currentStone || p[my][nx] == EMPTY)
	               break;				/* not your piece */
	             else {
		       cont = 0;
	             }
		     break;
	  }	/* done with switch on type */
	  ++k;
	}	/* next point in pattern */

	if (cont) {			/* it matches pattern */
	    tval = pat[r].patwt - fv;
	    if ((r >= 8) && (r <= 13)) { /* patterns for expand region */
		if (line(ti) > 7)	/* penalty on line 1, 2 */
		  tval--;
		else
		  if ((line(ti) == 6) || (line(ti) == 7))
		    tval++;		/* reward on line 3, 4 */
		
		if (line(tj) > 7) 	/* penalty on line 1, 2 */
		  tval--;
		else
		  if ((line(tj) == 6) || (line(tj) == 7))
		    tval++;		/* reward on line 3, 4 */
	    }
	    if (tval > *val) {
		*val = tval;
		*i = ti;
		*j = tj;
		selpat = 1000 + r; selv = fv;
	    }
	}	/* found match */
      }		/* each orientation */
    }		/* each pattern */

  if (*val > 0) return 1;	/* pattern matched */
  else          return 0;	/* all matches failed */
}
/* end matchpat */
