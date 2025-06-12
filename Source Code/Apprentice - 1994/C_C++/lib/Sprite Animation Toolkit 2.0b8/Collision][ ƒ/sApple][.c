/* Apple sprite for SATcollision][ */
/* No, it has nothing to do with old Apple computers :-) */

/* unit sApple; */

#include "SAT.h"
#include "Collision][.h"

	static Handle	nammSound, bliaehSound;
	static FacePtr	goodFace, badFace;
	static FacePtr	coreDump;


	void InitApple()
	{
		int i;
		
		nammSound = SATGetNamedSound("\Pnamm");
		bliaehSound = SATGetNamedSound("\Pbliaeh");
		goodFace = GetFace(132);
		badFace = GetFace(133);
		coreDump = GetFace(135);
	}

	pascal void SetupApple (SpritePtr me)
	{
		me->speed.h = 1 + Rand(3);
		me->kind = -2; /*Enemy kind*/
		me->face = goodFace;
		SetRect(&me->hotRect, 0, 0, 32, 32);
	}

/*We use kind -2 for fresh apples and kind -3 for bad apples. We avoid -1, since it doesn't count for the "anymonsters" flag.*/
/*Note that the "kind" field is not modified my SAT since we are not using KindCollision!*/

	pascal void HandleApple (SpritePtr me)
	{
		switch (me->kind) {
			case -2: 
				me->face = goodFace;
				break;
			case -3: 
				me->face = badFace;
				break;
		}; /*switch*/

		me->position.h = me->position.h + me->speed.h;
		if (me->position.h > gSAT.offSizeH - 16)
			{
				me->speed.h = -1 - Rand(3);
				if (Rand(2) == 0)
					me->kind = -3;
				else
					me->kind = -2;
			};
		if (me->position.h < -16)
			{
				me->speed.h = 1 + Rand(3);
				if (Rand(2) == 0)
					me->kind = -3;
				else
					me->kind = -2;
			};
	}

	pascal void HitApple(SpritePtr me, SpritePtr him)
	{
		if (him->task == &HandleApple)
			me->kind = -3; /*Colliding apples corrupt each other!*/
		else
/*If "him" is not an apple, then it must be the player!*/
			if (him->mode >= 0) /*if the player feels bad, don't eat*/
				switch (me->kind) {
					case -2:
							SATSoundPlay(nammSound, 1, false);
							me->task = nil;
							him->mode = 25;
							SATPlotFace(coreDump, nil, nil, me->position, true);
							break;
					case -3: /*Bad apple, make player feel bad.*/
							him->mode = -20;
							SATSoundPlay(bliaehSound, 2, false);
							break;
				};
	}
