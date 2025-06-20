typedef struct particle {
	long x,y;				/* location */
	long vx, vy;			/* velocity */
	int	pict;				/* which picture this ball is */
} ball;

#define	GRAD	6			/* radius for drawing (quickdraw units) */
#define	CRAD	28			/* radius for collision (box units) */
#define	SDIM	(2*GRAD)	/* diameter on screen */

#define	SLOW	225			/* max v squared for slow ball */
