/*/
     Project Arashi: VAClip.c
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, November 2, 1992, 16:52
     Created: Friday, March 20, 1987, 17:24

     Copyright � 1987-1992, Juri Munkki
/*/

#include "VAInternal.h"
#include "VA.h"

/*
**	This is a total rewrite of the old clipping
**	routine. The old routine passed some invisible
**	lines as visible and caused nasty crashes in
**	some applications. The old version was safe to
**	use with STORM, but failed when used with a
**	more demanding application.
**
**	The rectangle pointed to by rp is clipping to
**	the VA.frame boundaries. The routine returns
**	TRUE, if the resulting line is visible.
*/
int		VAClip(
	Rect	*rp)
{
asm	{	movem.l	D3-D7,-(sp)
		move.l	rp,A0
		
		move.w	Rect.left(A0),D0
		move.w	Rect.right(A0),D1
		move.w	Rect.top(A0),D2
		move.w	Rect.bottom(A0),D3
		
		move.w	VA.frame.left,D4
		cmp.w	D4,D0
		bge		@leftx1ok
		cmp.w	D4,D1
		blt		@reject
@leftx1ok
		move.w	VA.frame.right,D5
		cmp.w	D5,D0
		blt		@rightx1ok
		cmp.w	D5,D1
		bge		@reject
@rightx1ok
		move.w	VA.frame.top,D6
		cmp.w	D6,D2
		bge		@topy1ok
		cmp.w	D6,D3
		blt		@reject
@topy1ok
		move.w	VA.frame.bottom,D7
		cmp.w	D7,D2
		blt		@bottomy1ok
		cmp.w	D7,D3
		bge		@reject
@bottomy1ok
		
		cmp.w	D4,D0
		bge.s	@x1leftdone
		move.w	D4,Rect.left(A0)
		sub.w	D0,D4				; D4 = leftclip - x1
		sub.w	D1,D0				; D0 = x1 - x2
		sub.w	D2,D3				; D3 = y2 - y1
		muls.w	D4,D3				; D3 = dy * t
		divs.w	D0,D3				; D3 = (dy * t)/-dx
		sub.w	D3,D2				; D2 = y1 - D3
		move.w	D2,Rect.top(A0)
		move.w	VA.frame.left,D4

		move.w	Rect.left(A0),D0
		move.w	Rect.right(A0),D1
		move.w	Rect.bottom(A0),D3
@x1leftdone
		cmp.w	D4,D1
		bge.s	@x2leftdone
		move.w	D4,Rect.right(A0)
		sub.w	D1,D4				; Same algorithm as in above
		sub.w	D0,D1
		sub.w	D3,D2
		muls.w	D4,D2
		divs.w	D1,D2
		sub.w	D2,D3
		move.w	D3,Rect.bottom(A0)
		
		move.w	Rect.left(A0),D0
		move.w	Rect.right(A0),D1
		move.w	Rect.top(A0),D2
@x2leftdone
		cmp.w	D5,D0
		blt.s	@x1rightdone
		move.w	D5,D4
		subq.w	#1,D4
		move.w	D4,Rect.left(A0)
		sub.w	D0,D4
		sub.w	D1,D0
		sub.w	D2,D3
		muls.w	D4,D3
		divs.w	D0,D3
		sub.w	D3,D2
		move.w	D2,Rect.top(A0)

		move.w	Rect.left(A0),D0
		move.w	Rect.right(A0),D1
		move.w	Rect.bottom(A0),D3
@x1rightdone
		cmp.w	D5,D1
		blt.s	@x2rightdone
		subq.w	#1,D5
		move.w	D5,Rect.right(A0)
		sub.w	D1,D5
		sub.w	D0,D1
		sub.w	D3,D2
		muls.w	D5,D2
		divs.w	D1,D2
		sub.w	D2,D3
		move.w	D3,Rect.bottom(A0)

		move.w	Rect.left(A0),D0
		move.w	Rect.right(A0),D1
		move.w	Rect.top(A0),D2
@x2rightdone
		cmp.w	D6,D2
		bge		@topstillok
		cmp.w	D6,D3
		blt		@reject
@topstillok
		cmp.w	D7,D2
		blt		@bottomstillok
		cmp.w	D7,D3
		bge		@reject
@bottomstillok

		cmp.w	D6,D2
		bge.s	@y1topdone
		move.w	D6,Rect.top(A0)
		move.w	D6,D4
		sub.w	D2,D4
		sub.w	D3,D2
		sub.w	D0,D1
		muls.w	D4,D1
		divs.w	D2,D1
		sub.w	D1,D0
		move.w	D0,Rect.left(A0)

		move.w	Rect.right(A0),D1
		move.w	Rect.top(A0),D2
		move.w	Rect.bottom(A0),D3
@y1topdone
		cmp.w	D6,D3
		bge.s	@y2topdone
		move.w	D6,Rect.bottom(A0)
		move.w	D6,D4
		sub.w	D3,D4
		sub.w	D2,D3
		sub.w	D1,D0
		muls.w	D4,D0
		divs.w	D3,D0
		sub.w	D0,D1
		move.w	D1,Rect.right(A0)
		
		move.w	Rect.left(A0),D0
		move.w	Rect.top(A0),D2
		move.w	Rect.bottom(A0),D3
@y2topdone
		cmp.w	D7,D2
		blt.s	@y1bottomdone
		move.w	D7,D4
		subq.w	#1,D4
		move.w	D4,Rect.top(A0)
		sub.w	D2,D4
		sub.w	D3,D2
		sub.w	D0,D1
		muls.w	D4,D1
		divs.w	D2,D1
		sub.w	D1,D0
		move.w	D0,Rect.left(A0)

		move.w	Rect.right(A0),D1
		move.w	Rect.top(A0),D2
		move.w	Rect.bottom(A0),D3

@y1bottomdone
		cmp.w	D7,D3
		blt.s	@y2bottomdone
		subq.w	#1,D7
		move.w	D7,Rect.bottom(A0)
		sub.w	D3,D7
		sub.w	D2,D3
		sub.w	D1,D0
		muls.w	D7,D0
		divs.w	D3,D0
		sub.w	D0,D1
		move.w	D1,Rect.right(A0)
		move.w	Rect.left(A0),D0
@y2bottomdone
		move.w	VA.frame.left,D4
		cmp.w	D4,D0
		bge		@leftstillok
		cmp.w	D4,D1
		blt		@reject
@leftstillok
		move.w	VA.frame.right,D5
		cmp.w	D5,D1
		blt		@rightstillok
		cmp.w	D5,D1
		bge		@reject
@rightstillok

		moveq.l	#1,D0
		bra.s	@alldone
		
@reject
		moveq.l	#0,D0
@alldone
		movem.l	(sp)+,D3-D7
	}
}