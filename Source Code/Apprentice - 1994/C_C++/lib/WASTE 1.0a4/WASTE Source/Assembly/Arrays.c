/*
 *	Arrays.c
 *
 *	WASTE PROJECT
 *	Utilities for handling handle-based dynamic arrays
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 *
 */

pascal OSErr _WEInsertSlot(Handle h, void *element, long insertAt, long slotSize);
pascal OSErr _WERemoveSlot(Handle h, long removeAt, long slotSize);

#define	_BlockMoveData	dc.w 0xA22E

pascal OSErr _WEInsertSlot(Handle h, void *element, long insertAt, long slotSize)
{
	asm {
			movem.l d3-d5, -(sp)			; 
			move.l slotSize, d3				; d3 = slotSize
			movea.l h, a0					; 
			_GetHandleSize					; 
			move.l d0, d4					; d4 = handle size
			add.l d3, d0					; 
			_SetHandleSize					; add one "slot"
			bne.s @exit					; 
			subq.w #8, sp					; reserve space for 64-bit result
			move.l insertAt, -(sp)			; 
			move.l d3, -(sp)				; 
			pea 8(sp)						; 
			_LongMul						; 
			addq.w #4, sp					; throw away high long
			move.l (sp)+, d5				; offset = insertAt * slotSize
			
			moveq #-50, d0				; check for a paramErr (-50)
			tst.l d5						; offset < 0 ?
			slt d1						; 
			cmp.l d4, d5					; offset > handle size ?
			sgt d2						; 
			or.b d1, d2					; 
			bne.s @exit					; 
			
			movea.l h, a0					; 
			movea.l (a0), a0				; 
			adda.l d5, a0					; a0 = *h + offset
			movea.l a0, a1					; 
			adda.l d3, a1					; a1 = a0 + slotSize
			move.l d4, d0					; 
			sub.l d5, d0					; d0 = hSize - offset
			_BlockMoveData				; make room for new element
			
			movea.l a0, a1					; 
			movea.l element, a0				; 
			move.l d3, d0					; 
			_BlockMoveData				; insert new element
			
@exit		movem.l (sp)+, d3-d5
	}
}

pascal OSErr _WERemoveSlot(Handle h, long removeAt, long slotSize)
{
	asm {
			movem.l d3-d5, -(sp)			; 
			move.l slotSize, d3				; d3 = slotSize
			movea.l h, a0					; 
			_GetHandleSize					; 
			move.l d0, d4					; 
			sub.l d3, d4					; d4 = hSize - slotSize
			subq.w #8, sp					; reserve space for 64-bit result
			move.l removeAt, -(sp)			; 
			move.l d3, -(sp)				; 
			pea 8(sp)						; 
			_LongMul						; 
			addq.w #4, sp					; throw away high long
			move.l (sp)+, d5				; offset = removeAt * slotSize
			
			moveq #-50, d0				; check for a paramErr (-50)
			tst.l d5						; offset < 0 ?
			slt d1						; 
			cmp.l d4, d5					; offset > (hSize - slotSize) ?
			sgt d2						; 
			or.b d1, d2					; 
			bne.s @exit					; 
			
			movea.l h, a1					; 
			movea.l (a1), a1				; 
			adda.l d5, a1					; 
			movea.l a1, a0					; 
			adda.l d3, a0					; 
			move.l d4, d0					; 
			sub.l d5, d0					; 
			_BlockMoveData				; compact the array
			
			movea.l h, a0					; 
			move.l d4, d0					; 
			_SetHandleSize					; remove unused space
			
@exit		movem.l (sp)+, d3-d5			; 
	}
}