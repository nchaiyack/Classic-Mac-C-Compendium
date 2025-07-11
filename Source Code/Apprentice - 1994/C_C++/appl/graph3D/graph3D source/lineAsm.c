/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#ifdef	_LSC3_
#	include <ColorToolbox.h>
#endif

extern	Boolean			colorQD;
extern	WindowPtr		graphWind;
extern	Boolean		grphOnScrn;

static	int	top, left, scrnRowBytes;
static	Point	*grphPnLoc;

void	Aline(...);


CheckGrphWind()
{
	int	bottom, right;
	Boolean		ScrnHasColor();
	Rect	*bBox;

	if (colorQD)
	{
		top = -(*(((CGrafPtr)graphWind)->portPixMap))->bounds.top;
		left = -(*(((CGrafPtr)graphWind)->portPixMap))->bounds.left;
		grphPnLoc = &(((CGrafPtr)graphWind)->pnLoc);
	}
	else
	{
		top = -graphWind->portBits.bounds.top;
		left = -graphWind->portBits.bounds.left;
		grphPnLoc = &(graphWind->pnLoc);
	}
	bBox = &((**(((WindowPeek)graphWind)->contRgn)).rgnBBox);
	bottom = bBox->bottom;
	right = bBox->right;

	grphOnScrn = ((bottom < screenBits.bounds.bottom)
		&& (right < screenBits.bounds.right))
		&& (left > 0);

	scrnRowBytes = screenBits.rowBytes;
}

pascal void
MLineTo(end)
Point	end;
{
	Point	start;
return;
	start = *grphPnLoc;
	*grphPnLoc = end;
	end.h += left;
	end.v += top;
	start.h += left;
	start.v += top;
if ((end.h > 1024) || (start.h > 1024))
{
MoveTo(start.h, start.v);
LineTo(end.h, end.v);
}
else
	Aline(start.h, start.v, end.h, end.v);
}

static
void
Aline()
{
asm{
	link	a6,#0

	MOVEM.L D0-D7/A0-A1,-(A7)
	MOVE.W	8(a6),d4
	CLR.L	d5
	MOVE.W	10(a6),d5
	MOVE.W	12(a6),d6
	MOVE.W	14(a6),d7
	CMP.W   D6,D4
	BLS.S   @lah_1
	EXG     D6,D4
	EXG     D7,D5
lah_1:	MOVE    D6,D2
	SUB     D4,D2
	MOVE    D7,D3
	SUB     D5,D3
	MOVE    scrnRowBytes,D6
	CMP.W   D7,D5
	BLS.S   @lah_2
	NEG     D6
	NEG     D3
lah_2:	MULS    scrnRowBytes,D5
	MOVE.L  0x824,A0
	ADDA.L  D5,A0
	MOVE    D4,D0
	LSR     #3,D0
	ANDI    #0x7E,D0
	ADDA.W  D0,A0
	LSL     #1,D4
	ANDI    #0x1E,D4
	LEA     @masks,a1
	ADD	d4,a1
	MOVE	(a1),d0
	CMP.W   D3,D2
	BHI.S   @lah_6
	MOVE    D3,D1
	MOVE    D3,D7
lah_3:	OR      D0,(A0)
	SUB     D2,D1
	BLS.S   @lah_5
lah_4:	ADDA.W  D6,A0
	DBF     D7,@lah_3
	MOVEM.L (A7)+,D0-D7/A0
	UNLK    A6
	RTS     
lah_5:	ADD     D3,D1
	ROR     #1,D0
	BCC     @lah_4
	ADDQ.L  #2,A0
	BRA     @lah_4
lah_6:	MOVE    D2,D1
	MOVE    D2,D7
lah_7:	OR      D0,(A0)
	SUB     D3,D1
	BLS.S   @lah_10
lah_8:	ROR     #1,D0
	BCC.S   @lah_9
	ADDQ.L  #2,A0
lah_9:	DBF     D7,@lah_7
	MOVEM.L (A7)+,D0-D7/A0
	UNLK    A6
	RTS     
lah_10:	ADD     D2,D1
	ADDA.W  D6,A0
	BRA     @lah_8

masks:	DC.W    0x8000,0x4000,0x2000,0x1000,0x800,0x400,0x200,0x100
	DC.W    0x80,0x40,0x20,0x10,8,4,2,1
}
}