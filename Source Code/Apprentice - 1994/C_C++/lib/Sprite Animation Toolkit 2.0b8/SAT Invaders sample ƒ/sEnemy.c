//� C translation from Pascal source file: sEnemy.p

//� ===============================================.
//� ================= Enemy sprite unit ================.
//� ===============================================.

//� Example file for Ingemars Sprite Animation Toolkit.
//� � Ingemar Ragnemalm 1992.
//� See doc files for legal terms for using this code.

//� sEnemy;

//� Sprite unit. A sprite unit should include the following routines:
//� Init-procedure, that Initializes private bitmaps.
//� Setup-procedure, that sets variables other than the standard ones set by MakeSprite.
//� Handle-procedure, to be called once per iteration until the sprite dies.
//� Hittask-procedure (optional), for advanced collission handling.

//� Enemy object for the SATInvaders sample game.

//� Prototypes, etc.

#include "SAT.h"
#include "InvadeSAT.h"
//	SAT, SoundConst, GameGlobals, sMissile;

extern void		InitMissile(void);
extern pascal void		SetupMissile(SpritePtr sp);
extern pascal void		HandleMissile(SpritePtr me);

Point globalSpeed;
Boolean turnFlag;
short last_H, downCount;

static FacePtr enemyFace[6];

void InitEnemy ()
{
	short ii;

	for (ii = 0; ii <  6; ii++)
		enemyFace[ii] = GetFace (128 + ii);
	globalSpeed.h = 5;
	globalSpeed.v = 0;
	turnFlag = false;
	last_H = 5;
	downCount = 0;
}

pascal void SetupEnemy (SpritePtr sp)
{
	sp->face = enemyFace[0];
	sp->mode = Rand (6); 	//� icon number.
	SetRect (&(sp->hotRect), 2, 4, 30, 32);
	//� Since enemies are only created at the beginning of each level, we re-Initialize this stuff here:
	globalSpeed.h = 5;
	globalSpeed.v = 0;
	turnFlag = false;
	last_H = 5;
	downCount = 0;
}

pascal void HandleEnemy (SpritePtr me)
{
	SpritePtr missile;

	if (me->kind != -3)
	{
		me->task = 0L;
		SATSoundPlay (dunkH, 1, false);
	}
	me->mode++;
	if (me->mode == 6)
		me->mode = 0;
	me->face = enemyFace[me->mode];

	me->position.h = me->position.h + globalSpeed.h;
	me->position.v = me->position.v + globalSpeed.v;

	if ((me->position.h < 0) || (me->position.h > gSAT.offSizeH - 32))
		turnFlag = true;
	if (me->position.v > gSAT.offSizeV)
		me->task = 0L;

	if (Rand (100) < 2)
		{
			missile = NewSprite (-1, me->position.h + 8, me->position.v + 20, &HandleMissile, &SetupMissile, 0L);
			SATSoundPlay (piuH, 1, false);
		}
}




