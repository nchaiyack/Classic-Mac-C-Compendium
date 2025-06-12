//¥ C translation from Pascal source file: sShot.p

//¥ ===============================================.
//¥ ================= Shot sprite unit ================.
//¥ ===============================================.

//¥ Example file for Ingemars Sprite Animation Toolkit.
//¥ © Ingemar Ragnemalm 1992.
//¥ See doc files for legal terms for using this code.

//¥ sShot;

//¥ Sprite unit. A sprite unit should include the following routines:
//¥ Init-procedure, that Initializes private bitmaps.
//¥ Setup-procedure, that sets variables other than the standard ones set by MakeSprite.
//¥ Handle-procedure, to be called once per iteration until the sprite dies.
//¥ Hittask-procedure (optional), for advanced collission handling.

//¥ Shot object for the SATInvaders sample game.

//¥ Prototypes, etc.

#include "SAT.h"
//#include "GameGlobals.h"
//SoundConst, GameGlobals;

enum {
	shotSpeed = 15
};

static FacePtr shotFace;

void InitShot ()
{
	shotFace = GetFace (135);
}

pascal void SetupShot (SpritePtr sp)
{
	sp->face = shotFace;
	SetRect (&sp->hotRect, 0, 0, 8, 12);
}

pascal void HandleShot (SpritePtr me)
{
	if (me->kind != 1)
	{
		me->task = 0L;
		//¥ No sound here - we assume that the bad guys (sEnemy and sMissile) do that.
	}

	me->position.v = me->position.v - shotSpeed;
	if (me->position.v < 0)
		me->task = 0L;
}

