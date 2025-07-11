/* Platform sprite, experimental faceless sprite */

#include "SAT.h"
#include "myPlatform.h"

void InitPlatform()
{
/* nada*/
}

pascal void SetupPlatform(SpritePtr me)
{
	Rect			r;
	PolyHandle	pol;
	
	me->face = nil; /* = faceless! */
	SetRect(&(me->hotRect), 0, 0, 100, 16);
	r = me->hotRect;
	OffsetRect(&r, me->position.h, me->position.v);
	SetPort(gSAT.backScreen); /* or SATSetPortBackScreen if in a hurry*/
	FillRect(&r, dkGray);

	pol = OpenPoly();
	MoveTo(r.left, r.top);
	LineTo(r.left + 5, r.top - 5);
	LineTo(r.right + 5, r.top - 5);
	LineTo(r.right, r.top);
	LineTo(r.left, r.top);
	LineTo(r.right, r.top);

	LineTo(r.right, r.bottom);
	LineTo(r.right + 5, r.bottom - 5);
	LineTo(r.right + 5, r.top - 5);
	LineTo(r.right, r.top);

	ClosePoly();
	ErasePoly(pol);
	FramePoly(pol);
	KillPoly(pol);

	r.top = r.top - 5;
	r.right = r.right + 5;
	SATBackChanged(&r); /* Tell SAT to draw it when appropriate */

	me->layer = -me->position.v;
}

pascal void HandlePlatform(SpritePtr me)
{
	/*me->face = nil;*/ 
}

pascal void HitPlatform(SpritePtr me, SpritePtr him)
{
	int	mini, i, min;
	int	diff[5];
	
	if (him->task != HandlePlatform){
		diff[1] = -me->hotRect2.top + (him->hotRect2.bottom);		/* TtoB */
		diff[2] = -him->hotRect2.top + (me->hotRect2.bottom); 		/* BtoT */
		diff[3] = -me->hotRect2.left + (him->hotRect2.right);		/* LtoR */
		diff[4] = -him->hotRect2.left + (me->hotRect2.right);		/* RtoL */
		mini = 0;
		min = 10000;
		for(i = 1; i <= 4 ; i++){
			if(min > diff[i]){
				min = diff[i];
				mini = i;
			} /* if */
		}
		switch(mini){
			case 1: /*floor*/
					him->position.v = him->position.v - diff[1] + 1; 
					him->kind = 10; /* Signal to him, as if we used KindCollision */
					if(him->speed.v > 0)
						him->speed.v = 0;
					break;
			case 2: /* ceiling */
					him->position.v = him->position.v + diff[2] + 1;
/*We don't signal here. A hit in the ceiling should just send him back down again.*/
					if(him->speed.v < 0)
						him->speed.v = -him->speed.v;
					break;
			case 3: /*left*/
					him->position.h = him->position.h - diff[3] - 1;
					him->kind = 10;  /* Signal to him, as if we used KindCollision */
					if(him->speed.h > 0)
						him->speed.h = -him->speed.h;
					break;
			case 4: /*right*/
					him->position.h = him->position.h + diff[4] + 1;
					him->kind = 10;  /* Signal to him, as if we used KindCollision */
					if(him->speed.h < 0)
						him->speed.h = -him->speed.h;
					break;
		} /* switch */
	}
}
