
//� C translation from Pascal source file: sZprite.p

//� sZprite;

//� Prototypes, etc.

#include "SAT.h"

void InitZprite();
pascal void SetupZprite(SpritePtr me);
pascal void HandleZprite(SpritePtr me);


enum {
	zpeedH = 1,
	zpeedV = 1
};

Handle theSound;
FacePtr Zfaces[2];

void InitZprite()
{
	short i;
	
	theSound = SATGetSound(128);
	for ( i = 0; i <= 1; i++)
		Zfaces[i] = GetFace(128 + i);
}

pascal void SetupZprite(SpritePtr me)
{
		me->speed.h =   zpeedH;
		me->speed.v = - zpeedV;
		me->face = Zfaces[1];
}

pascal void HandleZprite(SpritePtr me)
{
	//� if mode < 1 then Face := nil;{test av faceless.

	me->position.h = me->position.h + me->speed.h;
	me->position.v = me->position.v + me->speed.v;
	if ( me->position.h > gSAT.offSizeH ) //� 400.
	{
		me->speed.h = -zpeedH;
		SATSoundPlay(theSound, 1, false);
		me->face = Zfaces[0];
	}
	if ( me->position.h < -20 ) //� 50.
	{
		me->speed.h = zpeedH;
		SATSoundPlay(theSound, 1, true);
		me->face = Zfaces[1];
	}
	if ( me->position.v > gSAT.offSizeV ) //� 400.
	{
		me->speed.v = -zpeedV;
		SATSoundPlay(theSound, 1, false);
	}
	if ( me->position.v < -20 ) //� 50.
	{
		me->speed.v = zpeedV;
		SATSoundPlay(theSound, 1, true);
	}
}

