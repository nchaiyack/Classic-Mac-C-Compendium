#ifndef __Assembler__
#include	<Assembler.h>
#endif
/*
; CDIconKiller © 1994, 1995 Quinn & Peter N Lewis, Fabrizio Oddone
; Version 1.0 written in 4 hours from 02:00 to 05:30 18/June/94

bug fixes, CD driver string in resource (suggested by Quinn),
asm tweaks etc. (version 1.1 thru 1.3) by Fabrizio Oddone

; Patches GetCatInfo, tests for Custom Icon attribute on
; CD-ROM files/folders, and clears it.

; Detailed description:

; Init time:
;   load the resource containing the driver strings
;   Patch GetCatInfo

; GetCatInfo Patch:
;   Test for GetCatInfo selector on HFSDispatch, do nothing otherwise
;   Call thru
;   Return immediately on error or async call
;   Return immediately unless Custom Icon bit set
;   Walk the volume queue looking for the vrefnum of the GetCatInfo
;   If we don't find it, we leave
;   If the command key is down, we leave
;   We determine whether the driver refnum is known (yes/no custom icons)
;   If "no", then clear the custom icon bit
;   If "yes", then leave
;   If the driver refnum is NOT known, we scan the driver queue and update our lists

	include	'sysequ.a'
	include	'fsequ.a'
	include	'traps.a'

; procedure ShowIcon (id: integer; delta_x: integer);
	import	ShowIcon
*/
//main	proc	export
extern pascal void ShowIcon(short iconID, short pixels);

#define	patchSize	(526)	// because of CodeWarrior asm limitations
#define	HFSDispatchTrapNum	0x60

asm void main(void)
{
	subq.w	#4,sp
	move.l	#'STR#',-(SP)
	move.w	#128,-(sp)
	_Get1Resource
	move.l	(sp)+,d0
	beq.s	bad
	lea	stringH,a0
	move.l	d0,(a0)	// remember string handle
	move.l	d0,-(sp)
	_DetachResource

	moveq	#HFSDispatchTrapNum,d0	// patch HFSDispatch
	_GetOSTrapAddress
	lea	oldHFSDispatch,a1
	move.l	a0,(a1)

//	move.l	#(EndHFSDispatch-OurHFSDispatch),d0
	move.l	#patchSize,d0
	_NewPtrSys
	bne.s	baddisposepref
	
	movea.l	a0,a1
	lea	OurHFSDispatch,a0
	move.l	#patchSize,d0
//	move.l	#(EndHFSDispatch-OurHFSDispatch),d0
	_BlockMove

	moveq	#HFSDispatchTrapNum,d0
	movea.l	a1,a0
	_SetOSTrapAddress	

	MOVE.L	#0xffff0080,-(SP)
	JSR	ShowIcon
bad:
	rts

baddisposepref:
	movea.l	stringH,a0
	_DisposHandle
	rts


OurHFSDispatch:
	cmp.w	#9,d0	// Test for GetCatInfo selector
	bne.s	dontdoit
	pea	continued
dontdoit:
	move.l	oldHFSDispatch,-(sp)	// Call thru
	rts

continued:
	tst.w	struct (DirInfo.ioResult)(a0)
	bne.s	fin	// give up on error or async
	
	btst	#2,struct (DirInfo.ioDrUsrWds.frFlags)(a0)
	beq.s	fin	// give up unless custom icon
	
	movem.l	a1/d1,-(sp)
	
	MOVE.L	A0,-(SP)
	LEA	-sizeof(EventRecord)(SP),SP
	MOVEA.L	SP,A0
	MOVEQ	#0,D0
	_OSEventAvail
	BTST	#0,struct (EventRecord.modifiers)(A0)	//cmdKey
	LEA	sizeof(EventRecord)(SP),SP
	MOVEA.L	(SP)+,A0
	bne.s	leave

	movea.l	0x358,a1	// first volume control block LMGetVCBQHdr()+2
	move.w	struct (DirInfo.ioVRefNum)(a0),d1
lop:
	cmp.w	struct (VCB.vcbVRefNum)(a1),d1
	bne.s	cont

	move.w	struct (VCB.vcbDRefNum)(a1),d1
//	_Debugger
	bsr	finddriverdontwantcustomicons
	bne.s	clearcustombit
	bsr	finddriverwantcustomicons
	bne.s	leave
	bsr.s	findDriverbyName
	bra.s	leave
clearcustombit:
	bclr	#2,struct (DirInfo.ioDrUsrWds.frFlags)(a0)	// clear custom icon
	bra	leave
cont:
	movea.l	(a1),a1	// next volume control block
	move.l	a1,d0
	bne.s	lop

leave:
	movem.l	(sp)+,a1/d1
	moveq	#0,d0
	
fin:
	tst.w	d0
notstfin:
	rts

finddriverdontwantcustomicons:
	move.l	a0,-(sp)
	lea	lastskipthesedrivers,a0
	move.w	d1,(a0)
	lea	skipthesedrivers,a0
loopfinddontwant:
	cmp.w	(A0)+,D1
	bne.s	loopfinddontwant
	pea	lastskipthesedrivers+2
	cmpa.l	(sp)+,A0
	movea.l	(sp)+,a0
	rts

finddriverwantcustomicons:
	move.l	a0,-(sp)
	lea	lastotherdrivers,a0
	move.w	D1,(a0)
	lea	otherdrivers,a0
loopfindwant:
	cmp.w	(A0)+,D1
	bne.s	loopfindwant
	pea	lastotherdrivers+2
	cmpa.l	(sp)+,A0
	movea.l	(sp)+,a0
	rts

findDriverbyName:
	movem.l	a0-a4/a6/d1/d3-d5,-(sp)
	moveq	#0,d5
	lea	nextfreedontwant,a3
	tst.l	(a3)
	bne.s	alreadyinitednfdw
	st	d5
	pea	skipthesedrivers
	move.l	(sp)+,(a3)
alreadyinitednfdw:
	movea.l	(a3),a0
	pea	lastskipthesedrivers
	cmpa.l	(sp)+,a0
	beq.s	plainexit
	lea	nextfreewant,a4
	tst.l	(a4)
	bne.s	alreadyinitednfw
	pea	otherdrivers
	move.l	(sp)+,(a4)
alreadyinitednfw:
	movea.l	(a4),a0
	pea	lastotherdrivers
	cmpa.l	(sp)+,a0
	beq.s	plainexit


	movea.l	0x11C,a2	// LMGetUTableBase()
	move.w	0x1d2,d3	// LMGetUnitTableEntryCount()
loop:
	move.l	(a2)+,d0
	beq.s	endloop
	movea.l	d0,a1

// a1 = curDCtlHndl

	movea.l	(a1),a1
	move.l	/*struct (DCtlEntry.dCtlDriver)*/(a1),d0
	beq.s	endloop
	movea.l	d0,a0
	btst	#6,(struct (DCtlEntry.dCtlFlags)+1)(a1)	// test for RAMDriver
	beq.s	notram
	move.l	(a0),d0
	beq.s	endloop
	movea.l	d0,a0
notram:
	move.w	d3,d4
	sub.w	0x1d2,d4	// calc driver refnum -(unit num+1) LMGetUnitTableEntryCount()
	subq.w	#1,d4
	tst.b	d5
	bne.s	firsttime
	move.w	d4,d1
	bsr	finddriverdontwantcustomicons
	bne.s	endloop
	bsr	finddriverwantcustomicons
	bne.s	endloop
firsttime:
	lea	struct (DRVRHeader.drvrName)(a0),a0
	moveq	#0,d2
	move.b	(a0)+,d2
	swap	d2
	bsr	cmpallstrings
	bne.s	endloopsavedriver
	movea.l	a3,a1
	bra.s	commonpart
endloopsavedriver:
	movea.l	a4,a1
commonpart:
	movea.l	(a1),a0
	move.w	d4,(a0)+	// remember driver refnum
	move.l	a0,(a1)
endloop:
	subq.w	#1,d3
	bne	loop
plainexit:
	movem.l	(sp)+,a0-a4/a6/d1/d3-d5
	rts

// input: a0 string, d2 (already in hi word) length
// output: sets the flags
// registers are saved&restored by the calling routine
cmpallstrings:
	movea.l	stringH,a6
	movea.l	(a6),a6
	move.w	(a6)+,d1
	andi	#0xB,CCR
	bra.s	thedbcc
cmploop:
	move.l	d2,d0
	move.b	(a6)+,d0
	movea.l	a6,a1
	adda.w	d0,a6
	_CmpString
thedbcc:
	dbeq	d1,cmploop
cmpexit:
	rts

skipthesedrivers:
	ds.w	15
lastskipthesedrivers:
	dc.w	-1
otherdrivers:
	ds.w	63
lastotherdrivers:
	dc.w	-1
nextfreedontwant:
	dc.l	0
nextfreewant:
	dc.l	0
stringH:
	dc.l	0
oldHFSDispatch:
	dc.l	0xffffffff

EndHFSDispatch:

	

//	string	pascal

}
//	endp
	
//	end
	
/*
asm -wb "{active}"
(evaluate "{active}" =~ "/(?*:)¨1(?*)/" )> dev:null
(evaluate "{active}" =~ "/(?*)¨2.a/" )> dev:null
link -rt INIT=128 -ra =ressysheap,reslocked -t INIT -c CDik -o "{¨2}" "{active}.o" "{¨1}ShowIcon.o"
setfile -a B "{¨2}"



dumpobj "{¨1}SHOWICON.o"
*/
