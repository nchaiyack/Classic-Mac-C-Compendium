/* Player sprite for the Platform test */

#include "SAT.h"
#include <stdlib.h>
#include "myPlatform.h"

#ifndef abs
#define abs(x)	(x>0?x:-x)
#endif

FacePtr	rightFaces[4], leftFaces[4];
FacePtr	fallFace[2] ;

void InitPlayerSprite()
{
	int i;
	
	for(i=0;i<=3;i++){
		rightFaces[i] = GetFace(128 + i);
		leftFaces[i] = GetFace(132 + i);
	}
	for(i=0;i<=1;i++)
		fallFace[i] = GetFace(136 + i);
}

pascal void HandlePlayerSprite(SpritePtr me)
{
	EventRecord	theEvent;
	char			temp;
	
		if(me->kind == 10) {
			me->speed.h /= 2;
		}

		if(GetOSEvent(keyDownMask + autoKeyMask, &theEvent))
			if(me->kind != 1){
			temp = (char)BitAnd(theEvent.message, charCodeMask);
			switch(temp){
				case ',': 	me->speed.h = -5;
						break;
				case '.': 	me->speed.h = 5;
						break;
				case ' ': 	me->speed.v = -10;
						break;
			}
		} /* if */

		me->position.h += me->speed.h;
		me->position.v += me->speed.v;
		
		if(me->kind < 3)  /*acceleration when falling*/
			me->speed.v += 1;
		if(me->speed.v > 30)
			me->speed.v = 30;

		if(me->kind > 1)
			me->kind -= 1;

/*Make sure we are always visible!*/
		if (me->position.v < 0){
			me->position.v = 0;
			me->speed.v = 0;
		}
		if(me->position.h < 0){
			me->position.h = 0;
			me->speed.h = abs(me->speed.h);
		}
		if(me->position.v > gSAT.offSizeV - 32) {
			me->position.v = gSAT.offSizeV - 32;
			me->speed.v = (-abs(me->speed.v) * 2)/3;
			me->kind = 7; 
		}
		if(me->position.h > gSAT.offSizeH - 32){
			me->position.h = gSAT.offSizeH - 32;
			me->speed.h = -abs(me->speed.h);
		} 

		me->mode = abs(me->mode + 1);

		if(me->kind > 1){
			if(me->speed.h > 0)
				me->face = rightFaces[(me->mode/3) & 3]; 
			if(me->speed.h < 0)
				me->face = leftFaces[(me->mode/3) & 3]; 
		}
		else
		 	me->face = fallFace[me->mode & 1]; 
			
			
		me->layer = -1*(me->position.v);
}

pascal void SetupPlayerSprite(SpritePtr me)
{
		me->mode = 0;
		me->speed.h = 1;
		me->kind = 1;  								/* friend kind */
		SetRect(&(me->hotRect), 8, 0, 24, 32);
		me->face = fallFace[0];
	
}

pascal void HitPlayerSprite(SpritePtr me, SpritePtr him)
{

/* Hit something! We can take whatever action we need here, but in this case,
     we let the other sprites decide. We could have omitted this function altogether and passed nil.*/
		if(him->kind == 1){
		}
		else if(him->kind == 2 )
		{
		}
}