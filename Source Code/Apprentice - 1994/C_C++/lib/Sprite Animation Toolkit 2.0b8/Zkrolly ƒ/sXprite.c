
//� C translation from Pascal source file: sXprite.p

//� sXprite;

//� Prototypes, etc.

#include "SAT.h"

	
FacePtr Xface;

void		InitXprite(void);
pascal void		SetupXprite(SpritePtr me);
pascal void		HandleXprite(SpritePtr me);


	enum {
		xpeed = 2
	};

void InitXprite()
{
	Xface = GetFace(130);
}

pascal void SetupXprite(SpritePtr me)
{
	me->mode = 0;
	me->speed.h = xpeed;
	me->face = Xface;
}

pascal void HandleXprite(SpritePtr me)
{	
	//� if mode < 1 then Face := nil;{test av faceless.

	me->position.h = me->position.h + me->speed.h;
	if (me->position.h > gSAT.offSizeH ) //� 400.
	{
		me->speed.h = -xpeed;
	}
	if (me->position.h < -20 ) //� 50.
	{
		me->speed.h = xpeed;
	}
}

