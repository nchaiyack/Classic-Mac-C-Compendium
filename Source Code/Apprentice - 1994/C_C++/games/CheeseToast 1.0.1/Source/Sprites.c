/****************************************************************************
 * Sprites.c
 *
 *		Sprite Maintenence, Logic, Animation
 *
 *		Most of Game Logic is here...
 *
 ****************************************************************************/
#include "CToast.h"
#include <stdlib.h>
#include <math.h>

#if __option(profile)					// 6/15 Optional profiling support
#include <Profile.h>
#endif


// Sprites Initialization Functions

void NewSaveMap(SpriteInstance *sp)
{
	short	n;
	if (gNbrSaveMaps == MaxSaveMaps - 1) {
		DebugStr("\pMax Save Maps");
		return;
	}
	n = gNbrSaveMaps;
	gNbrSaveMaps++;
	smTable[n].active = true;
	smTable[n].sp = sp;
	// sp->saveMapIdx = n;
	sp->saveMapPtr = &smTable[n];
}

void KillSaveMap(SaveMapPtr sm)
{
	sm->active = false;
	sm->sp = NULL;
	while (gNbrSaveMaps > 0 && !smTable[gNbrSaveMaps-1].active)
		--gNbrSaveMaps;
}

SpriteInstance *NewSprite(Boolean saveMapFlag)
{
	short			i;
	SpriteInstance *sp;
	if (gMaxSprite == MaxSprites - 1) {
		DebugStr("\pMax Sprites");
		return NULL;
	}
	sp = &sTable[gMaxSprite++];
	sp->active = true;
	if (saveMapFlag)
		NewSaveMap(sp);
	else
		sp->saveMapPtr = NULL;
	++gSpriteCnt;
	return sp;
}

void KillSprite(SpriteInstance *sp)
{
	sp->active = 0;
	if (sp->saveMapPtr) {
		KillSaveMap(sp->saveMapPtr);
		// sp->saveMapPtr = NULL;	// Don't kill - we still have to update sprite
	}
	--gSpriteCnt;
	while (gMaxSprite > 0 && !sTable[gMaxSprite-1].active && !sTable[gMaxSprite-1].update)
		--gMaxSprite;
}

void NewAsteroid(short type)
{
	register short	vecSpeed;
	register SpriteInstance *sp;
	if ((sp = NewSprite(true)) == NULL)
		return;
	if (MyRandom(50) == 0)
		type = ST_Jim;
	sp->type = type;
	sp->param1 = 0;
	sp->aniState = MyRandom(sDef[sp->type].nbrIcons);
	if (MyRandom(2) == 0) {
		sp->pos.h = 0;
		sp->pos.v = MyRandom(gPlayRect.bottom);
	}
	else {
		sp->pos.h = MyRandom(gPlayRect.right);
		sp->pos.v = 0;
	}
	sp->oldPos = sp->pos;
	// Pick any angle except 0,4,8,12 (which are straight)
	// and may cause asteroid to be invisible on sides
	sp->angle = MyRandom(MaxAngles-4);
	sp->angle += 1+(sp->angle/7);
	vecSpeed = MyRandom(3)+2+MyRandom(gGameLevel);
	sp->vector.lh = vecTable[sp->angle].lh * vecSpeed;
	sp->vector.lv = vecTable[sp->angle].lv * vecSpeed;
	if (labs(sp->vector.lh) < 0x00010000)
		sp->vector.lh = 0x00010000;
	if (labs(sp->vector.lv) < 0x00010000)
		sp->vector.lv = 0x00010000;
	sp->aniSpeed = 1 + MyRandom(2);
	sp->tickCtr = MyRandom(sp->aniSpeed);
	sp->width = 32;
	++gAsteroidCnt;
}

#if DEBUGGING
// This puts two special sprites in the lower left hand corner of the screen
// which are used to monitor sprite memory
//
void NewDebugDisplay(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(false)) == NULL)
		return;
	sp->type = ST_SpriteCnt;
	sp->param1 = 0;
	sp->pos.h = 40;
	sp->pos.v = 480+32;
	sp->oldPos = sp->pos;
	sp->width = 2;

	if ((sp = NewSprite(false)) == NULL)
		return;
	sp->type = ST_MaxSprite;
	sp->param1 = 0;
	sp->pos.h = 32;
	sp->pos.v = 480+32;
	sp->oldPos = sp->pos;
	sp->width = 2;
}
#endif

void NewSubAsteroid(short parent, short type, short vecOffset)
{
	register short vecSpeed;
	register SpriteInstance *sp,*par;
	if ((sp = NewSprite(true)) == NULL)
		return;
	par = &sTable[parent];
	sp->pos = par->pos;
	sp->oldPos = par->oldPos;
	sp->param1 = par->param1;
	sp->param2 = par->param2;
	sp->aniState = par->aniState;
	sp->lifeSpan = par->lifeSpan;
	sp->angle = par->angle;
	sp->vector = par->vector;

	sp->type = type;
	sp->angle += vecOffset;
	if (sp->angle < 0)
		sp->angle += MaxAngles;
	if (sp->angle >= MaxAngles)
		sp->angle -= MaxAngles;
	vecSpeed = MyRandom(3)+2+MyRandom(gGameLevel);
	sp->vector.lh = vecTable[sp->angle].lh * vecSpeed;
	sp->vector.lv = vecTable[sp->angle].lv * vecSpeed;
	sp->aniSpeed = 1 + MyRandom(2);
	sp->tickCtr = 0;
	sp->width = 32;
	++gAsteroidCnt;
}

void NewSaucer(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(true)) == NULL)
		return;
	sp->type = ST_Saucer;
	sp->param1 = 2+gGameLevel/2;			// Max Hits
	sp->param2 = 0;							// Hits Taken
	sp->aniState = MyRandom(sDef[sp->type].nbrIcons);
	if (MyRandom(2) == 0) {
		sp->pos.h = 0;
		sp->pos.v = MyRandom(gPlayRect.bottom);
	}
	else {
		sp->pos.h = MyRandom(gPlayRect.right);
		sp->pos.v = 0;
	}
	sp->oldPos = sp->pos;
	sp->angle = MyRandom(MaxAngles-4);
	sp->angle += 1+sp->angle/7;
	sp->vector.lh = vecTable[sp->angle].lh;
	sp->vector.lv = vecTable[sp->angle].lv;
	sp->aniSpeed = 2;
	sp->tickCtr = 0;
	sp->width = 32;
	PlaySound(S_BadGuyDebut, 3);
}

void NewBarbell(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(true)) == NULL)
		return;
	sp->type = ST_Barbell;
	sp->param1 = 3+gGameLevel/2;			// Max Hits
	sp->param2 = 0;							// Hits Taken
	sp->aniState = MyRandom(sDef[sp->type].nbrIcons);
	if (MyRandom(2) == 0) {
		sp->pos.h = 0;
		sp->pos.v = MyRandom(gPlayRect.bottom);
	}
	else {
		sp->pos.h = MyRandom(gPlayRect.right);
		sp->pos.v = 0;
	}
	sp->oldPos = sp->pos;
	sp->angle = MyRandom(MaxAngles-4);
	sp->angle += 1+sp->angle/7;
	sp->vector.lh = vecTable[sp->angle].lh;
	sp->vector.lv = vecTable[sp->angle].lv;
	sp->aniSpeed = 2;
	sp->tickCtr = 0;
	sp->width = 32;
	sp->lifeSpan = 200;
	PlaySound(S_BadGuyDebut, 3);
}

void NewCube(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(true)) == NULL)
		return;
	sp->type = ST_Cube;
	sp->param1 = 4+gGameLevel/2;			// Max Hits
	sp->param2 = 0;							// Hits Taken
	sp->aniState = MyRandom(sDef[sp->type].nbrIcons);
	if (MyRandom(2) == 0) {
		sp->pos.h = 0;
		sp->pos.v = MyRandom(gPlayRect.bottom);
	}
	else {
		sp->pos.h = MyRandom(gPlayRect.right);
		sp->pos.v = 0;
	}
	sp->oldPos = sp->pos;
	sp->angle = MyRandom(MaxAngles-4);
	sp->angle += 1+sp->angle/7;
	sp->vector.lh = vecTable[sp->angle].lh;
	sp->vector.lv = vecTable[sp->angle].lv;
	sp->aniSpeed = 2;
	sp->tickCtr = 0;
	sp->width = 32;
	sp->lifeSpan = 600;
	PlaySound(S_BadGuyDebut, 3);
}


void NewShipTimer(register SpriteInstance *sp)
{
	if (sp->tickCtr++ > 60) {
		sp->tickCtr = 0;
		sp->type = ST_Teapot;
		PlaySound(S_ShipDebut, 3);
		gShipMode |= SM_AutoShield;
		gShieldPower = MaxShieldPower;
	}
}

void NewShip(void)
{
	register SpriteInstance *sp;
	if (gRemainingShips == 0) {
		gGameState = GS_GameOver;
		return;
	}
	if ((sp = NewSprite(true)) == NULL)
		return;
	gShip = sp;
	sp->type = ST_TeapotNew;
	sp->param1 = 0;
	sp->pos = gCenterP;
	sp->oldPos = sp->pos;
	sp->angle = 0;
	sp->aniState = 0;
	sp->vector.lh = 0L;
	sp->vector.lv = 0L;
	sp->aniSpeed = 0;
	sp->tickCtr = 0;
	sp->width = 32;
	--gRemainingShips;
	gShipMode = 0;
}

void NewYummy(void)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(true)) == NULL)
		return;
	sp->type = ST_Yummies;
	sp->aniState = MyRandom(NbrYummies);
	sp->param1 = sp->aniState;
	sp->pos.h = IconWidth+MyRandom(gPlayRect.right-IconWidth);
	sp->pos.v = IconHeight+MyRandom(gPlayRect.bottom-IconWidth);
	sp->vector.lh = 0;
	sp->vector.lv = 0;
	sp->oldPos = sp->pos;
	sp->lifeSpan = 100;
	sp->tickCtr = 0;
	sp->width = 32;
	StandardSpriteDraw(sp);
	++gYummyCnt;
}

void NewQuake(void)
{
	sDef[ST_StatusDisplay].moveFunc = QuakeMove;
	sTable[0].param2 = 40;
}

void LaunchBullet(short type, short x, short y, long vx, long vy, short lifeSpan, long bTarget)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(false)) == NULL)
		return;
	sp->type = type;
	sp->param1 = bTarget;
	sp->pos.h = x;
	sp->pos.v = y;
	sp->oldPos = sp->pos;
	sp->vector.lh = vx;
	sp->vector.lv = vy;
	sp->tickCtr = 0;
	sp->lifeSpan = lifeSpan;
	sp->width = 2;
}

void LaunchSpark(short x, short y, long vx, long vy, short lifeSpan, short color)
{
	register SpriteInstance *sp;
	if ((sp = NewSprite(false)) == NULL)
		return;
	sp->type = ST_Spark;
	sp->param1 = color;
	sp->pos.h = x;
	sp->pos.v = y;
	sp->oldPos = sp->pos;
	sp->vector.lh = vx;
	sp->vector.lv = vy;
	sp->tickCtr = 0;
	sp->lifeSpan = lifeSpan;
	sp->width = 2;
	++gSparkCnt;
}

// Specialized Sprites - Kill Functions

void ConsumeYummy(register SpriteInstance *sp)
{
	switch (sp->param1) {
	case 0:		gScoreMultiply *= 2;		break;
	case 1:		gScoreMultiply *= 3;		break;
	case 2:		gScoreMultiply *= 5;		break;
	case 3:		gShipMode |= SM_Triple;		break;
	case 4:		gShipMode |= SM_Uzi;		break;
	}
	StandardSpriteErase(sp);
	KillSprite(sp);
	PlaySound(S_YummyConsume, 3);
	--gYummyCnt;
}

void ExplodeSprite(register SpriteInstance *sp, register short lifeSpan)
{
	register long	i;
	register short	r;
	register Ptr	colorPtr, maskPtr;
	colorPtr = sDef[sp->type].colorMaps + AniFrameIndex(sp->aniState);
	maskPtr = sDef[sp->type].maskMaps + AniFrameIndex(sp->aniState);
	r = 0;
	for (i = 0; i < 1024; i += 12) {
		if (!(*maskPtr)) {
			LaunchSpark(sp->pos.h+(i&31),sp->pos.v+(i>>5),
						 sp->vector.lh + (vecTable[r].lh << 1),
						 sp->vector.lv + (vecTable[r].lv << 1),
						 lifeSpan,
						 *colorPtr);
			++r;
			r &= 0x0F;
		}
		colorPtr += 12;
		maskPtr += 12;
	}
	KillSprite(sp);
	if (lifeSpan > 10)
		PlaySound(S_ShipExplodes, 3);
	else
		PlaySound(S_Explosion, 2);
}


// Sprite Move Functions

void StandardSpriteMove(register SpriteInstance *sp)
{
	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	// Update Animation Frame
	sp->tickCtr++;
	if (sp->tickCtr >= sp->aniSpeed) {
		sp->tickCtr = 0;
		sp->aniState++;
		if (sp->aniState >= sDef[sp->type].nbrIcons)
			sp->aniState = 0;
	}
}

#if DEBUGGING

void MaxSpriteMove(register SpriteInstance *sp)
{
	sp->pos.h = 32;
	sp->pos.v = 480+32-gMaxSprite;
}

void SpriteCntMove(register SpriteInstance *sp)
{
	sp->pos.h = 40;
	sp->pos.v = 480+32-gSpriteCnt;
}

#endif

void SaucerMove(register SpriteInstance *sp)
{
	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	// Update Animation Frame
	sp->tickCtr++;
	if (sp->tickCtr >= sp->aniSpeed) {
		sp->tickCtr = 0;
		sp->aniState++;
		if (sp->aniState >= sDef[sp->type].nbrIcons) {
			LongPoint	newVec;
			sp->aniState = 0;
			if (gShip && gShip->type == ST_Teapot || gShip->type == ST_TeapotT) {
				double scale;
				register short	slowness;
				newVec.lh = gShip->pos.h - sp->pos.h;
				newVec.lv = gShip->pos.v+12 - sp->pos.v;			
				scale = 1.0 / sqrt(	(double) newVec.lh*newVec.lh +
						newVec.lv*newVec.lv);
				newVec.lh *= scale * 65536.0;
				newVec.lv *= scale * 65536.0;
				newVec.lh <<= 3;
				newVec.lv <<= 3;
				slowness = 1 + MyRandom(2);
				sp->vector.lh = newVec.lh >> slowness;
				sp->vector.lv = newVec.lv >> slowness;
				LaunchBullet(ST_Photon,
							 sp->pos.h+16,sp->pos.v+4,
							 newVec.lh,newVec.lv,
							 25,
							 BF_BadBullet);

				PlaySound(S_EnemyFires, 2);
			}
		}
	}
}

void BarbellMove(register SpriteInstance *sp)
{
	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	if (--sp->lifeSpan == 0) {
		ExplodeSprite(sp,30);
		NewQuake();
		return;
	}

	// Update Animation Frame
	sp->tickCtr++;
	if (sp->tickCtr >= sp->aniSpeed) {
		sp->tickCtr = 0;
		if (++sp->aniState >= sDef[sp->type].nbrIcons)
			sp->aniState = 0;
		if ((sp->aniState & 1)) {
			LongPoint	bulletVec;
			if (gShip && gShip->type == ST_Teapot || gShip->type == ST_TeapotT) {
				double scale;

				bulletVec.lh = gShip->pos.h - sp->pos.h;
				bulletVec.lv = gShip->pos.v+12 - sp->pos.v;			
				scale = 1.0 / sqrt(	(double) bulletVec.lh*bulletVec.lh +
						bulletVec.lv*bulletVec.lv);
				bulletVec.lh *= scale * 65536.0;
				bulletVec.lv *= scale * 65536.0;
				bulletVec.lh <<= 2;
				bulletVec.lv <<= 2;

				sp->vector = bulletVec;
			}
		}
	}
}

void CubeMove(register SpriteInstance *sp)
{
	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	if (--sp->lifeSpan == 0) {
		ExplodeSprite(sp,30);
		NewQuake();
		return;
	}

	// Update Animation Frame
	sp->tickCtr++;
	if (sp->tickCtr >= sp->aniSpeed) {
		sp->tickCtr = 0;
		if (++sp->aniState >= sDef[sp->type].nbrIcons)
			sp->aniState = 0;
		if ((sp->aniState & 1)) {
			LongPoint	bulletVec;
			if (gShip && gShip->type == ST_Teapot || gShip->type == ST_TeapotT) {
				double scale;

				bulletVec.lh = gShip->pos.h - sp->pos.h;
				bulletVec.lv = gShip->pos.v+12 - sp->pos.v;			
				scale = 1.0 / sqrt(	(double) bulletVec.lh*bulletVec.lh +
						bulletVec.lv*bulletVec.lv);
				bulletVec.lh *= scale * 65536.0;
				bulletVec.lv *= scale * 65536.0;
				bulletVec.lh <<= 2;
				bulletVec.lv <<= 2;

				sp->vector = bulletVec;
			}
		}
		if (sp->aniState == 9) {
				LaunchBullet(ST_Photon,
							 sp->pos.h+16,sp->pos.v+16,
							 sp->vector.lh << 2,sp->vector.lv << 2,
							 25,
							 BF_BadBullet);

				PlaySound(S_EnemyFires, 2);
		}
	}
}


void ShipMove(register SpriteInstance *sp)
{
	register short j;
	register SpriteInstance *spj;
	register Point bPos,tPos;
	static Point firingPos[MaxAngles/2] = 
	   {4,16,	5,21,	7,25,	10,28,		
	    13,31,	18,30,	19,28,	23,23,
		26,16,	24,11,	20,4,	18,3,
		13,0,	10,4,	8,7,	6,12};
		
	// Check for Collision
	bPos = sp->pos;
	for (j = 0,spj=sTable; j < gMaxSprite; ++j,++spj) {
		tPos = spj->pos;
		if (spj->active &&						// Active
			bPos.h+32 > tPos.h &&				// In Range
			bPos.v+32 > tPos.v &&
			bPos.h < tPos.h+32 &&
			bPos.v < tPos.v+32 &&
			(BF_GoodBullet & (1L << spj->type)) > 0)		// Valid Target
		{
			register Ptr	mp2,mp1;
			register short	x,y;
			Rect			r;
			r.left = max(bPos.h,tPos.h);
			r.top = max(bPos.v,tPos.v);
			r.right = min(bPos.h+32,tPos.h+32);
			r.bottom = min(bPos.v+32,tPos.v+32);
			mp1 = sDef[sp->type].maskMaps + AniFrameIndex(sp->aniState);
			mp2 = sDef[spj->type].maskMaps + AniFrameIndex(spj->aniState);
			for (y = r.top; y < r.bottom; ++y) {
				for (x = r.left; x < r.right; ++x) {
					if (!mp2[(x - bPos.h)+AniRowIndex((y - bPos.v))] &&
						!mp2[(x - tPos.h)+AniRowIndex((y - tPos.v))])	
						goto Collision;
				}
			}
		}
	}

	if (sp->param1 & SF_Left) {
		sp->angle -= 2;
		if (sp->angle < 0)
			sp->angle = MaxAngles-2;
	}
	else if (sp->param1 & SF_Right) {
		sp->angle += 2;
		if (sp->angle >= MaxAngles)
			sp->angle = 0;
	}

	// Update Animation Frame
	sp->aniState = sp->angle >> 1;


	if (sp->param1 & SF_Thrust) {
		sp->vector.lh += vecTable[sp->angle].lh;
		sp->vector.lv += vecTable[sp->angle].lv;
		if (sp->type != ST_TeapotT) {
			sp->type = ST_TeapotT;
			PlaySound(S_Thrust, 1);
		}
	}
	else if (sp->param1 & SF_Shield) {
		if (sp->type != ST_TeapotS) {
			sp->type = ST_TeapotS;
			PlaySound(S_Shield, 1);
		}
	}
	else
		sp->type = ST_Teapot;
	

	if (sp->param1 & SF_Fire) {
		LaunchBullet(	ST_Bullet,
						bPos.h+firingPos[sp->aniState].h,
						bPos.v+firingPos[sp->aniState].v,
						sp->vector.lh + (vecTable[sp->angle].lh << 3),
						sp->vector.lv + (vecTable[sp->angle].lv << 3),
						25,
						BF_GoodBullet);
		if (gShipMode & SM_Triple) {
			LaunchBullet(	ST_Bullet,
							bPos.h+firingPos[sp->aniState].h,
							bPos.v+firingPos[sp->aniState].v,
							sp->vector.lh + (vecTable[(sp->angle+MaxAngles-1)%MaxAngles].lh << 3),
							sp->vector.lv + (vecTable[(sp->angle+MaxAngles-1)%MaxAngles].lv << 3),
							25,
							BF_GoodBullet);
			LaunchBullet(	ST_Bullet,
							bPos.h+firingPos[sp->aniState].h,
							bPos.v+firingPos[sp->aniState].v,
							sp->vector.lh + (vecTable[(sp->angle+MaxAngles+1)%MaxAngles].lh << 3),
							sp->vector.lv + (vecTable[(sp->angle+MaxAngles+1)%MaxAngles].lv << 3),
							25,
							BF_GoodBullet);
		}
		PlaySound(S_Firing, 1);
	}
	
	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	sp->tickCtr++;
	sp->param1 &= ~SF_Fire;
	return;
Collision:
	if (!(sp->param1 & SF_Shield) &&
		spj->type != ST_Yummies) {
		ExplodeSprite(sp,30);
		NewShip();
	}

	switch (spj->type) {
	case ST_Wheel:
		NewSubAsteroid(j,ST_WheelR,-2);
		NewSubAsteroid(j,ST_WheelG,0);
		NewSubAsteroid(j,ST_WheelB,2);
		--gAsteroidCnt;
		KillSprite(spj);
		PlaySound(S_CompoundFracture, 2);
		break;
	case ST_Aster:
		NewSubAsteroid(j,ST_Aster1,-2);
		NewSubAsteroid(j,ST_Aster2,2);
		--gAsteroidCnt;
		KillSprite(spj);
		PlaySound(S_CompoundFracture, 2);
		break;
	case ST_WheelR:
	case ST_WheelG:
	case ST_WheelB:
	case ST_Aster1:
	case ST_Aster2:
	case ST_Teacup:
	case ST_Jim:
		--gAsteroidCnt;
		ExplodeSprite(spj,10);
		break;
	case ST_Yummies:
		ConsumeYummy(spj);
		break;
	case ST_Saucer:
		ExplodeSprite(spj,10);
		break;
	case ST_Barbell:
		ExplodeSprite(spj,10);
		break;
	case ST_Cube:
		ExplodeSprite(spj,10);
		break;
	}
}

void BulletMove(register SpriteInstance *sp)
{
	register short j,n;
	register SpriteInstance *spj;
	register Point	bPos,tPos;

	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	// Update Animation Frame
	if (--sp->lifeSpan == 0) {
		KillSprite(sp);
		return;
	}
	// Check for Collision
	bPos = sp->pos;
	n = gMaxSprite;
	for (j = 0,spj=sTable; j < n; ++j,++spj) {
		tPos = spj->pos;
		if (spj->active &&							// Active
			// abs((bPos.h >> 5) - (tPos.h >> 5)) < 2 &&	// Range Check
			// abs((bPos.v >> 5) - (tPos.v >> 5)) < 2 &&	// Range Check
			bPos.h+2 > tPos.h &&				// In Range
			bPos.v+2 > tPos.v &&
			bPos.h < tPos.h+32 &&
			bPos.v < tPos.v+32 &&
			(sp->param1 & (1L << spj->type)) > 0)		// Valid Target
		{
			register Ptr	mp;
			Rect			r;
			register short y,x;
			r.left = max(sp->pos.h,tPos.h);
			r.top = max(sp->pos.v,tPos.v);
			r.right = min(sp->pos.h+2,tPos.h+32);
			r.bottom = min(sp->pos.v+2,tPos.v+32);
			mp = sDef[spj->type].maskMaps + AniFrameIndex(spj->aniState);
			for (y = r.top; y < r.bottom; ++y) {
				for (x = r.left; x < r.right; ++x) {
					if (!mp[(x - tPos.h)+AniRowIndex((y - tPos.v))])	
						goto Collision;
				}
			}
		}
	}
	return;

Collision:
	KillSprite(sp);		// Kill Bullet

	switch (spj->type) {
	case ST_Wheel:
		NewSubAsteroid(j,ST_WheelR,-2);
		NewSubAsteroid(j,ST_WheelG,0);
		NewSubAsteroid(j,ST_WheelB,2);
		--gAsteroidCnt;
		KillSprite(spj);
		AddScore(AsterPoints);
		PlaySound(S_CompoundFracture, 2);
		break;
	case ST_Aster:
		NewSubAsteroid(j,ST_Aster1,-2);
		NewSubAsteroid(j,ST_Aster2,2);
		--gAsteroidCnt;
		KillSprite(spj);
		AddScore(AsterPoints);
		PlaySound(S_CompoundFracture, 2);
		break;
	case ST_WheelR:
	case ST_WheelG:
	case ST_WheelB:
	case ST_Aster1:
	case ST_Aster2:
	case ST_Teacup:
	case ST_Jim:
		--gAsteroidCnt;
		ExplodeSprite(spj,10);
		AddScore(AsterPoints);
		break;
	case ST_Saucer:
		if (++spj->param2 >= spj->param1)	{		// Max Hits?
			ExplodeSprite(spj,20);
			AddScore(SaucerPoints);
		}
		break;
	case ST_Barbell:
		if (++spj->param2 >= spj->param1)	{		// Max Hits?
			ExplodeSprite(spj,20);
			AddScore(BarbellPoints);
		}
		break;
	case ST_Cube:
		if (++spj->param2 >= spj->param1)	{		// Max Hits?
			ExplodeSprite(spj,20);
			AddScore(CubePoints);
		}
		break;
	case ST_Teapot:
	case ST_TeapotT:
		ExplodeSprite(spj,30);
		NewShip();
		break;
	case ST_Yummies:
		StandardSpriteErase(spj);
		ExplodeSprite(spj,10);	// No Points for Exploded Yummies
		break;
	case ST_TeapotS:	// Nada - Shields are up
		PlaySound(S_DudBullet, 2);
		break;
	}
}

void YummyMove(register SpriteInstance *sp)
{
	if (--sp->lifeSpan == 0) {
		StandardSpriteErase(sp);
		KillSprite(sp);
		--gYummyCnt;
		return;
	}
}


void SparkMove(register SpriteInstance *sp)
{
	register short j;
	register SpriteInstance *spj;

	// Update position
	sp->pos.h += (sp->vector.lh >> 16);
	sp->pos.v += (sp->vector.lv >> 16);
	if (sp->pos.h < gPlayRect.left)
		sp->pos.h += gPlayWidth;
	if (sp->pos.v < gPlayRect.top)
		sp->pos.v += gPlayHeight;
	if (sp->pos.h > gPlayRect.right)
		sp->pos.h -= gPlayWidth;
	if (sp->pos.v > gPlayRect.bottom)
		sp->pos.v -= gPlayHeight;

	// Update Animation Frame
	if (--sp->lifeSpan == 0) {
		--gSparkCnt;
		KillSprite(sp);
		return;
	}
}

// This gets swapped into the status sprite's move function, which is
// normally null
// it causes a temporary "spacequake" jitter effect, possibly causing
// collisions with nearby objects
//
void QuakeMove(register SpriteInstance *sp)
{
	register short j,n,i;
	register SpriteInstance *spj;
	static char xOffset[] = {-2,3, 0,-4, 0,-1,2,2};
	static char yOffset[] = {-4,0,-1, 2,-2, 3,2,0};
	if (--sp->param2 == 0) {
		sDef[ST_StatusDisplay].moveFunc = NullFunc;
		return;
	}
	n = gMaxSprite;
	i = sp->param2 & 7;
	for (j = 0,spj=sTable; j < n; ++j,++spj) {
		spj->pos.h += xOffset[i];
		spj->pos.v += yOffset[i];
		++i;
		i &= 7;
	}
}

void NullFunc(register SpriteInstance *sp)
{
}


