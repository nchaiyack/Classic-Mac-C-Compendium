/*/
     Project Arashi: VAEraseLines.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:41
     Created: Thursday, February 9, 1989, 20:14

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "VAInternal.h"

int		mask;

void	VAEraseBufferedLines(count,lines)
int		count;
Rect	*lines;
{
	mask=(7<<5)>>VA.offset;
	
asm	{
	movem.l	D3-D7/A2,-(sp)

	move.l	lines,A2
	sub.w	#1,count
	bmi		@exitreally
@bigloop
	move.w	(A2)+,D2	;	Y1 to D2
	move.w	(A2)+,D0	;	X1 to D0
	move.w	(A2)+,D3	;	Y2 to D3
	move.w	(A2)+,D1	;	X2 to D0

	cmp.w	D1,D0		;	Is X1<X2
	blt.s	@right		;	Yes
	exg.l	D0,D1		;	Swap X1 and X2
	exg.l	D2,D3		;	Swap Y1 and Y2
@right

	move.w	D1,D6
	sub.w	D0,D6		;	DeltaX to D6

	move.w	D3,D7
	sub.w	D2,D7		;	DeltaY to D7
	bpl.s	@delta2
	neg.w	D7			;	Abs DeltaY
@delta2
	move.w	VA.row,D4	;	VA.row into D4
	move.l	VA.quickrow,A1		;	Pointer to base address table
	move.l	(0,A1,D2.w*4),A0	;	Get pointer to row base
	add.w	D0,A0		;	Add X1 to base address
	
	move.l	(0,A1,D3.w*4),A1
	add.w	D1,A1

	cmp.w	D3,D2		;	is Y1<Y2
	blt.s	@down		;	Yes, direction is down
	neg.w	D4			;	No, direction is up
@down	
	move.w	mask,D0

	cmp.w	D6,D7		;	is DeltaY>=DeltaX?
	bge		@vertigo	;	Yes, jump

	move.w	D6,D2		;	ErrorAcc=DeltaX
	asr.w	#1,D2		;	ErrorAcc=DeltaX/2
	addq.l	#1,A1
	move.w	D2,D3		;	DeltaX/2 is loop counter
@xplot
	subq.w	#8,D3		;	Subtract 8 from loopcount
	bmi.s	@xshort		;	Should we fall out?

	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext1		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext1
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext2		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext2
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext3		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext3
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext4		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext4
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext5		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext5
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext6		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext6
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext7		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext7
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xplot		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext8
	bra.s	@xplot
@xshort
	addq.w	#8,D3
@xplot2
	or.b	D0,(A0)+	;	Write out pattern
	or.b	D0,-(A1)
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext		;	No acc overflow
	add.w	D4,A0		;	Move RowBase to next line
	sub.w	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext
	dbra	D3,@xplot2	;	Loop...

	or.b	D0,(A0)
	bra		@exit
	
@vertigo
	move.w	D7,D2		;	ErrorAcc=DeltaY
	asr.w	#1,D2		;	ErrorAcc=DeltaY/2
	move.w	D2,D3		;	DeltaY is loop counter
@yplot
	subq.w	#8,D3
	bmi		@yshort

	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext1		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext1
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext2		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext2
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext3		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext3
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext4		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext4
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext5		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext5
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext6		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext6
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext7		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext7
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl		@yplot		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1

	bra		@yplot
@dot
	bra.s	@exit
@yshort
	addq.l	#8,D3
@yplot2
	or.b	D0,(A0)		;	Write out pattern
	add.w	D4,A0		;	Move RowBase to next line
	or.b	D0,(A1)		;	Write out pattern
	sub.w	D4,A1
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext
	dbra	D3,@yplot2	;	Loop...

@exit
	sub.w	#1,count
	bpl		@bigloop
@exitreally
	movem.l	(sp)+,D3-D7/A2
	}
}