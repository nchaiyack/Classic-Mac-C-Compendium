/*/
     Project Arashi: VAHeart.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:41
     Created: Friday, February 3, 1989, 21:18

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VAInternal.h"
#include "VA.h"

void	VAEraseBuffer()
{
	int		mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);

	if(VA.frame.right & 15)
asm	{
	move.l	VA.base,A0
	move.w	VA.frame.bottom,D0
	subq.w	#1,D0
	moveq.l	#-1,D2
@vertigo
	move.w	VA.frame.right,D1
	subq.w	#1,D1
	move.l	A0,A1
	add.w	VA.row,A0
@horigo
	move.b	D2,(A1)+
	dbra	D1,@horigo
	dbra	D0,@vertigo
	}
	else
asm	{
	move.l	VA.base,A0
	move.w	VA.frame.bottom,D0
	subq.w	#1,D0
	moveq.l	#-1,D2
@bvertigo
	move.w	VA.frame.right,D1
	asr.w	#4,D1
	subq.w	#1,D1
	move.l	A0,A1
	add.w	VA.row,A0
@bhorigo
	move.l	D2,(A1)+
	move.l	D2,(A1)+
	move.l	D2,(A1)+
	move.l	D2,(A1)+
	dbra	D1,@bhorigo
	dbra	D0,@bvertigo
	}
	SwapMMUMode(&mode);

	VA.ticker=QuickTicks+VA.FrameSpeed;	

#ifdef	COPYRIGHT_MESSAGE
	{	char	*copyr="\PSTORM Test Bench 0.9,Copyright 1991 Project STORM Team";

#ifdef	REALTEXT
		SetPort(VA.window);
		MoveTo(20,VA.frame.bottom-12);
		TextSize(9);
		TextMode(srcCopy);
		RGBForeColor(VAColorToQD(BG2));
		RGBBackColor(VAColorToQD(BGC));
		PmForeColor(BG2);
		DrawString(copyr);
#else
		VA.color=BG2;
		VAMoveTo(20,VA.frame.bottom-10);
		VA.segmscale=2;
		VADrawText(copyr+1,0,*copyr);
#endif
	}	
#endif
}

void	VABresenham(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
asm	{
	movem.l	D3-D7,-(sp)
	
	move.l	VA.base,A0
	
	move.w	x1,D0		;	X1 to D0
	move.w	x2,D1		;	X2 to D1
	move.w	y1,D2		;	Y1 to D2
	move.w	y2,D3		;	Y2 to D3

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
	ext.l	D4			;	Sign extend D4
	move.l	VA.offset,D0;	D0 is offset from byte boundary.
	
	move.w	VA.color,D1	;	Copy colorvalue to D1
	move.w	VA.field,D5	;	Field width to D5
	cmp.w	D6,D7		;	is DeltaY>=DeltaX?
	bge		@vertigo	;	Yes, jump

	move.w	D6,D2		;	ErrorAcc=DeltaX
	asr.w	#1,D2		;	ErrorAcc=DeltaX/2
	move.w	D2,D3		;	DeltaX/2 is loop counter
@xplot
	subq.w	#8,D3		;	Subtract 8 from loopcount
	bmi		@xshort		;	Should we fall out?

	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext1		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext1
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext2		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext2
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext3		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext3
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext4		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext4
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext5		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext5
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext6		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext6
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext7		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext7
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl		@xplot		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext8
	bra		@xplot
@xshort
	addq.w	#8,D3
@xplot2
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	addq.l	#1,A0		;	Advance to next x pixel
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	subq.l	#1,A1
	sub.w	D7,D2		;	ErrorAcc-=DeltaY
	bpl.s	@xnext		;	No acc overflow
	add.l	D4,A0		;	Move RowBase to next line
	sub.l	D4,A1
	add.w	D6,D2		;	Acc overflow. ErrorAcc+=DeltaX
@xnext
	dbra	D3,@xplot2	;	Loop...
	bra		@exit
	
@vertigo
	move.w	D7,D2		;	ErrorAcc=DeltaY
	asr.w	#1,D2		;	ErrorAcc=DeltaY/2
	move.w	D2,D3		;	DeltaY/2 is loop counter
@yplot
	subq.w	#8,D3
	bmi		@yshort

	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext1		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext1
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext2		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext2
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext3		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext3
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext4		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext4
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext5		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext5
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext6		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext6
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext7		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext7
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl		@yplot		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1

	bra		@yplot
@dot
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	bra.s	@exit
@yshort
	addq.l	#8,D3
@yplot2
	bfins	D1,(A0){D0:D5}	;	Draw pixel
	add.l	D4,A0		;	Move RowBase to next line
	bfins	D1,(A1){D0:D5}	;	Draw pixel
	sub.l	D4,A1		;	Move RowBase to next line
	sub.w	D6,D2		;	ErrorAcc-=DeltaX
	bpl.s	@ynext		;	No acc overflow
	add.w	D7,D2		;	Acc overflow. ErrorAcc+=DeltaY
	addq.l	#1,A0		;	Advance to next x pixel
	subq.l	#1,A1
@ynext
	dbra	D3,@yplot2	;	Loop...

@exit
	movem.l	(sp)+,D3-D7
	}
}

void	VAStaticLine(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
	int		mode;
	
	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);
	VABresenham(x1,y1,x2,y2);
	SwapMMUMode(&mode);
}

void	VAStaticLineTo(x,y)
int		x,y;
{
	int		mode;
	
	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);
	VABresenham(VA.CurrentX,VA.CurrentY,x,y);
	VAMoveTo(x,y);
	SwapMMUMode(&mode);
}
