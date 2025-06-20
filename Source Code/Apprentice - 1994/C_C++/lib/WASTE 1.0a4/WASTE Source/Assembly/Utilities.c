/*
 *	Utilities.c
 *
 *	WASTE PROJECT
 *	General purpose utility
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 *
 */

pascal Boolean _WEBlockCmp(void *block1, void *block2, long blockSize);
pascal void _WEBlockClr(void *block, long blockSize);
pascal void _WEForgetHandle(Handle *h);
pascal Boolean _WESetHandleLock(Handle h, Boolean lock);
pascal void _WEReorder(long *a, long *b);

pascal Boolean _WEBlockCmp(void *block1, void *block2, long blockSize)
{
	asm {
		moveq #0, d0			; assume function result is FALSE
		movea.l block1, a0		; 
		movea.l block2, a1		; 
		move.l blockSize, d1	; 
		bra.s @1				; 
		
@loop	cmpm.b (a0)+, (a1)+		; 
		bne.s @exit				; drop out from loop if 2 bytes differ
@1		subq.l #1, d1			; 
		bge.s @loop				; 
		
		moveq #1, d0			; set function result to TRUE
@exit
	}
}

pascal void _WEBlockClr(void *block, long blockSize)
{
	asm {
		movea.l block, a0		; 
		move.l blockSize, d0	; 
		bra.s @1				; 
		
@loop	clr.b (a0)+				; 
@1		subq.l #1, d0			; 
		bge.s @loop				; 
	}
}

pascal void _WEForgetHandle(Handle *h)
{
	asm {
		movea.l h, a0			; get ptr to VAR parameter
		move.l (a0), d0			; d0 = h
		beq.s @1				; if h = NIL then exit
		clr.l (a0)				; set VAR parameter to NIL
		movea.l d0, a0			; 
		_DisposHandle			; get rid of handle
@1	}
}

pascal Boolean _WESetHandleLock(Handle h, Boolean lock)
{
	asm {
		movea.l h, a0			; a0 = h
		_HGetState				; get state byte
		btst #7, d0				; test MSB (lock bit)
		sne.b d1				; 
		neg.b d1				; d1 = current lock
		move.b lock, d0			; d0 = required lock
		cmp.b d0, d1			; same lock?
		beq.s @exit				; if yes, do nothing
		tst.b d0				; 
		beq.s @1				; 
		_HLock					; 
		bra.s @exit				; 
@1		_HUnlock				; 
@exit	move.b d1, d0			; 
	}
}

pascal void _WEReorder(long *a, long *b)
{
	asm {
		movea.l a, a0
		movea.l b, a1
		move.l (a0), d0
		move.l (a1), d1
		cmp.l d0, d1
		bge.s @exit
		move.l d1, (a0)
		move.l d0, (a1)
@exit	
	}
}