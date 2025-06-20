//� C translation from Pascal source file: sPlayer.p

//� ===============================================.
//� ================= Player sprite unit ================.
//� ===============================================.

//� Example file for Ingemars Sprite Animation Toolkit.
//� � Ingemar Ragnemalm 1992.
//� See doc files for legal terms for using this code.

//� This file is the first of several sprite units, units that holds the full.
//� description of the objects to be animated.

//� sPlayer;

//� Sprite unit. A sprite unit should include the following routines:
//� Init-procedure, that Initializes private bitmaps.
//� Setup-procedure, that sets variables other than the standard ones set by MakeSprite.
//� Handle-procedure, to be called once per iteration until the sprite dies.
//� Hittask-procedure (optional), for advanced collission handling.

//� This is the sprite unit for the player object, in this case a butterfly.

//� Prototypes, etc.
#include "SAT.h"
#include "InvadeSAT.h"

extern void		InitShot(void);
extern pascal void		SetupShot(SpritePtr sp);
extern pascal void		HandleShot(SpritePtr me);

//� SoundConst, sShot;


enum {
	playerSpeed = 16
};

static FacePtr playerFace;
//static long posh, posv;

void InitPlayer ()
{
	playerFace = GetFace (134);
}

pascal void SetupPlayer (SpritePtr player)
{
	player->face = playerFace;
	SetRect (&player->hotRect, 1, 7, 31, 32);
}

pascal void HandlePlayer (SpritePtr me)
{
	Point pt;
	SpritePtr shot;
//	Handle toffH, dunkH, piuH, kraschH;

	if (me->kind != 2)
	{
		SATSoundPlay (kraschH, 10, true);
		stillRunning = false; 	//� Tell MoveIt that the game is over, to quit the game loop.
		//� Real games make an explosion first.
	}

	SetPort (gSAT.wind);
	GetMouse (&pt);

	me->speed.h = pt.h - me->position.h;

	if (me->speed.h < -playerSpeed)
		me->position.h = me->position.h - playerSpeed;
	else if (me->speed.h > playerSpeed)
		me->position.h = me->position.h + playerSpeed;
	else
		me->position.h = me->position.h + me->speed.h;

	if (me->position.h > gSAT.offSizeH - 32) 	//� xsize.
	{
		me->position.h = gSAT.offSizeH - 32; 	//� ev. xsize - 10?.
	}
	if (me->position.h < 0)
	{
		me->position.h = 0;
	}

	//� Create shots.
	if (me->mode > 0)
		me->mode--;
	if (me->mode == 0)
		if (Button ())
		{
			shot = NewSprite (1, me->position.h + 12, me->position.v, &HandleShot, &SetupShot, 0L);
			me->mode = 10;
			SATSoundPlay (toffH, 1, false);
		}
}




