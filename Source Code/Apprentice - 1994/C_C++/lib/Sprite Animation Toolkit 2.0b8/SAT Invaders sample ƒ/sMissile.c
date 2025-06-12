//¥ C translation from Pascal source file: sMissile.p

//¥ ===============================================.
//¥ ================= Missile sprite unit ================.
//¥ ===============================================.

//¥ Example file for Ingemars Sprite Animation Toolkit.
//¥ © Ingemar Ragnemalm 1992.
//¥ See doc files for legal terms for using this code.

//¥ sMissile;

//¥ Sprite unit. A sprite unit should include the following routines:
//¥ Init-procedure, that Initializes private bitmaps.
//¥ Setup-procedure, that sets variables other than the standard ones set by MakeSprite.
//¥ Handle-procedure, to be called once per iteration until the sprite dies.
//¥ Hittask-procedure (optional), for advanced collission handling.

//¥ Missile object for the SATInvaders sample game.

//¥ Prototypes, etc.

#include "SAT.h"
#include "InvadeSAT.h"
//¥ SoundConst, GameGlobals;


short missileCount; 	//¥ Exported since it has to be reset by SetUpLevel.

void InitMissile ();
pascal void SetupMissile (SpritePtr sp);
pascal void HandleMissile (SpritePtr me);


enum {
	missileSpeed = 10,
	maxMissiles = 5
};

static FacePtr missileFace;

void InitMissile ()
{
	missileFace = GetFace (136);
}

pascal void SetupMissile (SpritePtr sp)
{
	sp->face = missileFace;
	SetRect (&sp->hotRect, 3, 0, 13, 17);

	if (missileCount >= maxMissiles)
		sp->task = 0L; 	//¥ If too many, remove us immediately.
	else
		missileCount = missileCount++;
}

pascal void HandleMissile (SpritePtr me)
{
	if (me->kind != -1)
		{
			me->task = 0L;
			missileCount--;
			SATSoundPlay (dunkH, 1, false);
		}

	me->position.v = me->position.v + missileSpeed;

	if (me->position.v > gSAT.offSizeV)
		{
			me->task = 0L;
			missileCount--;
		}
}




