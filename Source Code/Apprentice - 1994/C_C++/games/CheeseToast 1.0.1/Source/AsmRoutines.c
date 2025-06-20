/************************************************************************************
 * AsmRoutines.c
 *
 * CheeseToast by Jim Bumgardner
 *
 * These are the assembly language routines used to speed sprite rendering
 *
 ************************************************************************************/

#include "CToast.h"

extern SpriteDef sDef[];
extern SpriteInstance *sTable;
extern long		gScreenRowBytes,gVideoRowBytes;
extern Ptr		gScreenMem,gVideoMem;
extern Rect		gPlayRect,gOffscreenRect;

void StandardSpriteDraw(register SpriteInstance *sp)
{
	register Ptr	spritePtr,scrnPtr;
	Ptr				maskPtr;
	register short	j;
	register long	rowBytesOffset;
	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	rowBytesOffset = gScreenRowBytes - IconWidth;

	// Copy area underneath sprite to temporary save area (part of sprite structure)
	// so sprite can be erased.
	// Pixels are copied 4 at a time, loop is unrolled for speed
	//
	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	spritePtr = sp->saveMapPtr->saveMap;
	asm {
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr)+, (spritePtr)+
		move.l	(scrnPtr), (spritePtr)
	}


	// Render Sprite, 4 pixels at a time
	// Mask is used to allow screen to show through transparent areas
	//
	spritePtr = sDef[sp->type].colorMaps + AniFrameIndex(sp->aniState);
	maskPtr = sDef[sp->type].maskMaps + AniFrameIndex(sp->aniState);
	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	asm {
		move.l	maskPtr, a0
	}
	for (j = 0; j < IconWidth; ++j) {
		asm {
			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			move.l	(spritePtr)+, d0
			move.l	(a0)+,d1
			and.l	d1,(scrnPtr)
			or.l	d0,(scrnPtr)+

			add.l	rowBytesOffset,scrnPtr
		}
	}
}

// Erase the sprite.
// When sprite was rendered, the area underneath it was saved.
// This code restores restores those pixels

void StandardSpriteErase(register SpriteInstance *sp)
{
	register Ptr	scrnPtr,spritePtr;
	register long 	rowBytesOffset;

	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	spritePtr = sp->saveMapPtr->saveMap;
	rowBytesOffset = gScreenRowBytes - IconWidth;

	asm {
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr

		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr)+, (scrnPtr)+
		move.l	(spritePtr), (scrnPtr)
	}
}


// Draw a Bullet
//

void BulletDraw(register SpriteInstance *sp)
{
	register Ptr	spritePtr,scrnPtr;
	register long	rowBytesOffset;
	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	rowBytesOffset = gScreenRowBytes - BulletWidth;

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	spritePtr = sp->savePixel;

	// Save area underneath bullet, so bullet can be erased
	//
	asm {
		move.w	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.w	(scrnPtr), (spritePtr)
	}

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);

	// Draw Bullet, 2 pixels at a time (pixels are cleared to 0 = white)
	asm {
		clr.w	(scrnPtr)+
		add.l	rowBytesOffset,scrnPtr
		clr.w	(scrnPtr)
	}
}

// Draw a Yellow Bullet
//

void PhotonDraw(register SpriteInstance *sp)
{
	register Ptr	spritePtr,scrnPtr;
	register long	rowBytesOffset;
	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	rowBytesOffset = gScreenRowBytes - BulletWidth;

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	spritePtr = sp->savePixel;

	// Save area underneath bullet, so bullet can be erased
	//
	asm {
		move.w	(scrnPtr)+, (spritePtr)+
		add.l	rowBytesOffset,scrnPtr
		move.w	(scrnPtr), (spritePtr)
	}

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);

	// Draw Photons 2 pixels at a time (05 = Yellow)
	asm {
		move.w	#0x0505,(scrnPtr)+
		add.l	rowBytesOffset,scrnPtr
		move.w	#0x0505,(scrnPtr)
	}
}

// Erase a bullet, by restoring the area that was saved
//
void BulletErase(register SpriteInstance *sp)
{
	register Ptr	scrnPtr,spritePtr;
	register long	rowBytesOffset;

	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	spritePtr = sp->savePixel;
	rowBytesOffset = gScreenRowBytes - BulletWidth;

	asm {
		move.w	(spritePtr)+, (scrnPtr)+
		add.l	rowBytesOffset,scrnPtr
		move.w	(spritePtr), (scrnPtr)
	}
}

// Draw a spark (single pixel)
//
void SparkDraw(register SpriteInstance *sp)
{
	register Ptr	scrnPtr;
	register char	color;
	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	color = sp->param1;
	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	sp->savePixel[0] = *scrnPtr;	// Save original color, so spark can be erased
	*scrnPtr = color;				// Draw spark
}

// Erase a spark
//
void SparkErase(register SpriteInstance *sp)
{
	register Ptr	scrnPtr;
	register Point	loc;

	sp->update = true;

	loc = sp->pos;

	scrnPtr = gScreenMem + (loc.v*gScreenRowBytes + loc.h);
	*scrnPtr = sp->savePixel[0];	// Restore original screen color
}

// Copy entire offscreen area to onscreen window
//
void MyCopyBits(void)
{
	register Ptr	scrnPtr, vidPtr;
	register long	vidRowBytesOffset,scrnRowBytesOffset;
	register short	y,x;
	char			mmode;

	if (g12InchMode) {
		// If using a 12 inch monitor (512 x 384), use special version of
		// this routine
		MyCopyBits12Inch();
		return;
	}
	// otherwise, assume 640 x 480

	scrnPtr = gScreenMem;
	scrnPtr += IconWidth + (gScreenRowBytes << 5);
	scrnRowBytesOffset = gScreenRowBytes - 640;

	vidPtr = gVideoMem + (gVidOffset.v * gVideoRowBytes) + gVidOffset.h;
	vidRowBytesOffset = gVideoRowBytes - 640;

	// When drawing to onscreen video, we must temporarily switch to
	// 32bit memory, since video address will generally be > 8 megs
	//
	mmode = true32b;
	SwapMMUMode(&mmode);
	y = 480;
	while (y--) {
		asm {
			// 160 Occurences 160 x 4 = 640
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			
			add.l	vidRowBytesOffset, vidPtr
			add.l	scrnRowBytesOffset, scrnPtr
		}
	}
	SwapMMUMode(&mmode);
}

void MyCopyBits12Inch(void)
{
	register Ptr	scrnPtr, vidPtr;
	register long	vidRowBytesOffset,scrnRowBytesOffset;
	register short	y,x;
	char			mmode;

	scrnPtr = gScreenMem;
	scrnPtr += IconWidth + (gScreenRowBytes << 5);
	scrnRowBytesOffset = gScreenRowBytes - 512;

	vidPtr = gVideoMem + (gVidOffset.v * gVideoRowBytes) + gVidOffset.h;
	vidRowBytesOffset = gVideoRowBytes - 512;

	// When drawing to onscreen video, we must temporarily switch to
	// 32bit memory, since video address will generally be > 8 megs
	//
	mmode = true32b;
	SwapMMUMode(&mmode);
	y = 384;
	while (y--) {
		asm {
			// 128 Occurences
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			move.l	(scrnPtr)+,(vidPtr)+
			
			add.l	vidRowBytesOffset, vidPtr
			add.l	scrnRowBytesOffset, scrnPtr
		}
	}
	SwapMMUMode(&mmode);
}

// This routine is used to copy a rectangle (typically a sprite)
// from offscreen pixelmap to onscreen video
//
void MyCopyRect(register Rect *r)
{
	register Ptr	scrnPtr, vidPtr;
	register long	vidRowBytesOffset,scrnRowBytesOffset;
	register short	y,x;
	char			mmode;

	scrnPtr = gScreenMem;
	scrnPtr += r->left + (gScreenRowBytes *r->top);
	scrnRowBytesOffset = gScreenRowBytes - (r->right - r->left);

	vidPtr = gVideoMem + (gVidOffset.v * gVideoRowBytes) + gVidOffset.h;
	vidPtr += (r->left-32) + ((r->top-32)*gVideoRowBytes);
	vidRowBytesOffset = gVideoRowBytes - (r->right - r->left);

	// When drawing to onscreen video, we must temporarily switch to
	// 32bit memory, since video address will generally be > 8 megs
	//
	mmode = true32b;
	SwapMMUMode(&mmode);
	y = r->bottom - r->top;
	while (y--) {
		x = r->right - r->left;
		while (x--) {
			asm {
				move.b	(scrnPtr)+,(vidPtr)+
			}
		}
		scrnPtr += scrnRowBytesOffset;
		vidPtr += vidRowBytesOffset;
	}
	SwapMMUMode(&mmode);
}