#include "SAT.h"

//¥ C translation from Pascal source file: sApple.p

//¥  Apple sprite for SATcollision .

//¥ sApple;

//¥ Prototypes, etc.

#include "Collision.h"
	
Handle theSound;
FacePtr appleFace;

void InitApple()
{
	theSound = SATGetSound(128);
	appleFace = SATGetFace(132);
}

pascal void SetupApple(SpritePtr me)
{
	me->speed.h = 1 + SATRand(3);
	me->kind = -1; //¥ Enemy kind.
	me->face = appleFace;
	SetRect(&me->hotRect, 0, 0, 32, 32);
	me->task = HandleApple;
}

pascal void HandleApple(SpritePtr me)
{
	if ( me->kind != -1 ) //¥ Something hit us!.
	{
		SATSoundPlay(theSound, 1, false);
		me->task = 0L; //¥ Go away.
	}
//¥ Move.
	me->position.h = me->position.h + me->speed.h;
	if ( me->position.h > gSAT.offSizeH - 16 )
		me->speed.h = -1 - SATRand(3);
	if ( me->position.h < -16 )
		me->speed.h = 1 + SATRand(3);
}

