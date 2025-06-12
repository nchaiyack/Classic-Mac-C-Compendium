/* Player sprite for SATcollision */

/*unit sMrEgghead;*/

#include "SAT.h"
#include "Collision.h"

	FacePtr	mrEggheadFaces[4];


	void InitMrEgghead()
	{
		int i;
		
		for(i=0;i<=3;i++)
			mrEggheadFaces[i] = GetFace(128 + i);
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

		me->mode++;
		me->face = mrEggheadFaces[me->mode % 4];
	}
