/* */

/* */

#include"SAT.h"
#include "myPlatform.h"

extern FacePtr	platFace; /* Global from sMovPlatform.c */

void InitHMovPlatform()
{
}

pascal void SetupHMovPlatform(SpritePtr me)
{
	Rect			r;
	PolyHandle	pol;
	
	me->speed.h = -1 + Rand(2) * 2;
	/*me->kind = -2; */
	me->face = platFace; 
	SetRect(&(me->hotRect), 0, 3, 60, 20);
}

pascal void HandleHMovPlatform(SpritePtr me)
{
	me->position.h = me->position.h + me->speed.h;
	if(me->position.h < 40)
		me->speed.h = 1;
	if(me->position.h > (gSAT.offSizeH - 100))
		me->speed.h = -1;

	if(me->speed.h == 0){
		if(me->position.h > gSAT.offSizeH/2)
			me->speed.h = -1;
		else
			me->speed.h = 1;
	}
	me->layer = -me->position.v;
}

pascal void HitHMovPlatform(SpritePtr me, SpritePtr him)
{
	int 	mini, i, min;
	int	diff[5];
	
	if(him->task != HandlePlatform) {
		diff[1] = -me->hotRect2.top + (him->hotRect2.bottom);			/* TtoB */
		diff[2] = -him->hotRect2.top + (me->hotRect2.bottom);			/* BtoT */
		diff[3] = -me->hotRect2.left + (him->hotRect2.right);			/* LtoR */
		diff[4] = -him->hotRect2.left + (me->hotRect2.right);			/* RtoL */
		mini = 0;
		min = 10000;
		for(i = 1; i <= 4; i++)
			if(min > diff[i]){
					min = diff[i];
					mini = i;
			}
		switch(mini){
			case 1: 
					him->position.v = him->position.v - diff[1] + 1;
					him->position.h = him->position.h + me->speed.h; 
					him->kind = 10; 
					if(him->speed.v > 0)
						him->speed.v = 0;
					break;
			case 2: 
					him->position.v = him->position.v + diff[2] + 1;
					if(him->speed.v < 0)
						him->speed.v = -him->speed.v;
					break;
			case 3: 
					him->position.h = him->position.h - diff[3] - 1;
					him->kind = 10;
					if(him->speed.h > 0)
						him->speed.h = -him->speed.h;
					break;
			case 4: 
					him->position.h = him->position.h + diff[4] + 1;
					him->kind = 10; 
					if(him->speed.h < 0)
						him->speed.h = -him->speed.h;
					break;
		} /* switch */
	} /* if */
}
