/*/
     Project Arashi: VALines.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Thursday, February 9, 1989, 15:32

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "VAInternal.h"

void	VADrawBufferedLines(count,lines,colors)
int		count;
Rect	*lines;
int		*colors;
{
asm	{
	movem.l	D3-D7/A2-A3,-(sp)

	move.l	lines,A3
	move.l	colors,A2
	sub.w	#1,count
	bmi		@exitreally
@bigloop

	move.w	(A3)+,D2	;	Y1 to D2
	move.w	(A3)+,D0	;	X1 to D0
	move.w	(A3)+,D3	;	Y2 to D3
	move.w	(A3)+,D1	;	X2 to D1

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
	
	move.w	(A2)+,D1	;	Copy colorvalue to D1
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
	sub.w	#1,count
	bpl		@bigloop
@exitreally
	movem.l	(sp)+,D3-D7/A2-A3
	}
}


void	VALineTo(x,y)
int		x,y;
{
	register	Rect	*p;

	if(VA.numlines[VA.curbuffer]<MAXSAVED)
	{	VA.linecolors[VA.curbuffer][VA.numlines[VA.curbuffer]]=VA.color;
		p=VA.lines[VA.curbuffer]+VA.numlines[VA.curbuffer]++;
		p->top=VA.CurrentY;
		p->left=VA.CurrentX;
		p->bottom=y;
		p->right=x;
	}
	VA.CurrentX=x;
	VA.CurrentY=y;
}
void	VASafeLineTo(x,y)
int		x,y;
{
	register	Rect	*p;
	register	int		flag;
	
asm	{
		moveq.l	#0,flag

		move.w	x,D0
		cmp.w	VA.frame.left,D0
		blt.s	@outside
		cmp.w	VA.frame.right,D0
		bge.s	@outside

		move.w	y,D0
		cmp.w	VA.frame.top,D0
		blt.s	@outside
		cmp.w	VA.frame.bottom,D0
		bge.s	@outside
		
		move.w	VA.CurrentX,D0
		cmp.w	VA.frame.left,D0
		blt.s	@outside
		cmp.w	VA.frame.right,D0
		bge.s	@outside

		move.w	VA.CurrentY,D0
		cmp.w	VA.frame.top,D0
		blt.s	@outside
		cmp.w	VA.frame.bottom,D0
		bge.s	@outside

		moveq.l	#-1,flag
@outside
	}
	if(flag)
	if(VA.numlines[VA.curbuffer]<MAXSAVED)
	{	VA.linecolors[VA.curbuffer][VA.numlines[VA.curbuffer]]=VA.color;
		p=VA.lines[VA.curbuffer]+VA.numlines[VA.curbuffer]++;
		p->top=VA.CurrentY;
		p->left=VA.CurrentX;
		p->bottom=y;
		p->right=x;
	}
	VA.CurrentX=x;
	VA.CurrentY=y;
}
void	VALine(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
	register	Rect	*p;
	
	if(VA.numlines[VA.curbuffer]<MAXSAVED)
	{	VA.linecolors[VA.curbuffer][VA.numlines[VA.curbuffer]]=VA.color;
		p=VA.lines[VA.curbuffer]+VA.numlines[VA.curbuffer]++;
		p->top=y1;
		p->left=x1;
		p->bottom=y2;
		p->right=x2;
	}
}
