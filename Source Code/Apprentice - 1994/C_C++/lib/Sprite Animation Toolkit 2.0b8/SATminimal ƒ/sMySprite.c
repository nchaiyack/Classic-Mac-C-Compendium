#include "SAT.h"

	Handle	theSound;
	FacePtr	faces[6];

/*	procedure InitMySprite;
	procedure SetupMySprite (me: SpritePtr);
	procedure HandleMySprite (me: SpritePtr);*/


void InitMySprite()
{
int i;

	theSound = SATGetSound(128);
	for (i=0; i<=5; i++)
		faces[i] = GetFace(128+i);
}

/* Important! Callback routines (Setup, Handle, Hit) must be declared "pascal"! */

pascal void SetupMySprite (me)
SpritePtr me;
{
	me->mode = 0;
	me->speed.h = 2;
}

pascal void HandleMySprite (me)
SpritePtr me;
{
/* Choose face */
	me->mode = (me->mode + 1) % 6;
	me->face = faces[me->mode];

/* Move */
	me->position.h = me->position.h + me->speed.h;
	if (me->position.h > gSAT.offSizeH - 16)
		{
			me->speed.h = -2;
			SATSoundPlay(theSound, 1, false);
		};
	if (me->position.h < -16)
		{
			me->speed.h = 2;
			SATSoundPlay(theSound, 1, false);
		};
}
