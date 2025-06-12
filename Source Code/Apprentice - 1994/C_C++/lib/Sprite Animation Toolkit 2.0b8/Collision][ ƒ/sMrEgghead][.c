/* Player sprite for SATcollision][ */

/*unit sMrEgghead;*/

#include "SAT.h"
#include "Collision][.h"

	FacePtr	mrEggheadFaces[4];
	FacePtr	yuckFace;


	void InitMrEgghead()
	{
		int i;
		
		for(i=0;i<=3;i++)
			mrEggheadFaces[i] = GetFace(128 + i);
		yuckFace = GetFace(134);
	}

	pascal void SetupMrEgghead (SpritePtr me)
	{
		me->mode = 0;
		me->speed.h = 1;
		me->kind = 1; /*Friend kind*/
		SetRect(&me->hotRect, 0, 0, 32, 32);
	}

	pascal void HandleMrEgghead (SpritePtr me)
	{
		GetMouse(&me->position);

/*This time, let's make sure Mr Egghead is always visible!*/
		if (me->position.v < 0)
			me->position.v = 0;
		if (me->position.h < 0)
			me->position.h = 0;
		if (me->position.v > gSAT.offSizeV - 32)
			me->position.v = gSAT.offSizeV - 32;
		if (me->position.h > gSAT.offSizeH - 32)
			me->position.h = gSAT.offSizeH - 32;


		if (me->mode < 0) /*we have taken a bite in a bad apple recently*/
			{
				me->face = yuckFace;
				me->mode = me->mode + 1;
			}
		else if (me->mode == 0) /*nothing special recently*/
			me->face = mrEggheadFaces[2];
		else
			{ /*ate an apple recently - chew!*/
				me->mode = me->mode - 1;
				me->face = mrEggheadFaces[me->mode % 4];
			};
	}

	pascal void HitMrEgghead (SpritePtr me, SpritePtr him)
	{
/* Hit something! We can take whatever action we need here, but in this case,*/
/* we let sApple decide. (We could have omitted this function altogether */
/* and passed nil as hittask to NewSprite.)*/
	}
