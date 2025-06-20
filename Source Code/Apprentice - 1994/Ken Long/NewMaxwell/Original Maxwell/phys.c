/*
 * do collisions and movement
 */

#include "ball.h"

extern wide, tall;			/* the size of the box we are in */
extern GateState;			/* != 0 iff gate is open */

#define TWO(X) ((X)+(X))
#define THREE(X) (TWO(X)+(X))

/*
 * bbump() deals with a possible collision between two specific balls.
 * The first ball will not be to the right of the second ball.  Returns
 * one if the second ball is far enough to the right so that no balls
 * farther right could collide with the first ball, else returns zero
 */
bbump( pA, pB )
	register ball *pA, *pB;
{
	register long k;
	long tAvx, tAvy;
	register long tBx, tBy;
	long tBvx, tBvy;
	long WIDE, TALL;
	
	WIDE = wide<<2;			/* scale from window to physical co-ords */
	TALL = tall<<2;
	
	/*
	 * Deal with the barrier and the gate
	 */
	if ( TWO(pA->x) <= WIDE && TWO(pB->x) >= WIDE )
		/* gate closed means no collision */
		if ( ! GateState )
			return 0;
		else
		/* if either ball is below gate, no collision */
		if ( THREE(pA->y) < TALL || THREE(pB->y) < TALL )
			return 0;
		else
		/* if either ball is above gate, no collision */
		if ( THREE(pA->y) > TWO(TALL) || THREE(pB->y) > TWO(TALL) )
			return 0;

	/* shift to A's co-ordinate system */
	tBx = pB->x - pA->x;
	tBy = pB->y - pA->y;
	tBvx = pB->vx - pA->vx;	tAvx = 0;
	tBvy = pB->vy - pA->vy;	tAvy = 0;
	
	/*
	 * see if the balls are close enough to have collided
	 */
	if ( tBx > TWO(CRAD) )
		return 1;
	if ( tBx * tBx + tBy * tBy > (CRAD*CRAD<<2) )
		return 0;
	
	k = tBx * tBvx + tBy * tBvy;
	
	/*
	 * make sure they are going towards each other
	 */	
	if ( k > -1 )
		return 0;
		
	k = ( tBy * tBvx - tBx * tBvy ) / k;
	
	tAvx = ( tBvx - k * tBvy ) / ( 1 + k*k );
	tAvy = ( k * tBvx + tBvy ) / ( 1 + k*k );
	
	tBvx =  k * tAvy;
	tBvy = -k * tAvx;
	
	pB->vx = pA->vx + tBvx;
	pB->vy = pA->vy + tBvy;
	pA->vx += tAvx;
	pA->vy += tAvy;
	
	return 0;
}

/*
 * Because the calculations above use longs instead of floats, we have a
 * lot of round off error.  This seems to manifest itself by causing the
 * balls to slow down over time.  We use the walls to correct this.
 *
 * If "we" is greater than zero, we are attempting to add energy to the
 * system.  We do this by looking for slow balls bouncing off the right
 * wall.  When we find such a ball, we give it a swift kick towards the
 * left.
 *
 * If "we" is less than zero, then we are trying to remove energt from
 * the system for some reason.  In this case, the outer walls become
 * slightly sticky, with the ball slowing down by abs(we) perpendicular
 * to the wall.
 *
 * This stuff is done in wbump().
 *
 */

static int we = 0;			/* wall energy factor */

walls( i ) {
	we = i;
}

/*
 * wump() checks for collisions between a ball walls or the gate
 */
wbump( pA )
	register ball *pA;
{
	register long WIDE, TALL;
	
	WIDE = wide<<2; TALL = tall<<2;
	
	if ( (pA->x <= CRAD && pA->vx < 0)
	  || (pA->x >= WIDE-CRAD && pA->vx > 0) ) {
		pA->vx = -pA->vx;
		if ( we > 0 ) {
			if ( pA->vx < 0 && pA->pict == 0 && pA->x >= WIDE-CRAD)
				pA->vx -= 30;				/* swift kick */
		} else {
			if ( pA->vx > 0 )
				pA->vx += we;
			else
				pA->vx -= we;
		}
	}
	if ( (pA->y <= CRAD && pA->vy < 0)
	  || (pA->y >= TALL-CRAD && pA->vy > 0) ) {
		pA->vy = -pA->vy;
		if ( we < 0 ) {
			if ( pA->vy > 0 )
				pA->vy += we;
			else
				pA->vy -= we;
		}

	}
	
	/*
	 * if the ball is on the same level as the gate, and the gate is open,
	 * there is nothing for the ball to hit on the barrier
	 */
	if ( TALL < THREE(pA->y) && THREE(pA->y) < TWO(TALL) && GateState )
		return;
		
	WIDE >>= 1;			/* location of the barrier */

	/*
	 * see if the ball hits the barrier
	 */
	if ( pA->x <= WIDE && pA->vx > 0 ) {
		if ( pA->x + CRAD >= WIDE || pA->x + pA->vx > WIDE )
			pA->vx = -pA->vx;
	} else
	if ( pA->x >= WIDE &&  pA->vx < 0 )
		if ( pA->x - CRAD <= WIDE || pA->x + pA->vx < WIDE )
			pA->vx = -pA->vx;
}

mball( pA )
	ball *pA;
{
	register long vx, vy;
	pA->x += vx = pA->vx;
	pA->y += vy = pA->vy;

	/*
	 * check for stalled balls, and offer them a chance to get going again
	 */
	if ( vx == 0 && vy == 0 ) {
		if ( Random() > 0 )
			pA->vx = Random() > 0 ? 1 : -1;
		if ( Random() > 0 )
			pA->vy = Random() > 0 ? 1 : -1;
	}
}
