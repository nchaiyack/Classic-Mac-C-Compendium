/*/
     Project Arashi: VAPixProcs.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Thursday, February 9, 1989, 21:21

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "VAInternal.h"

void	VAPixel(x,y)
register	int		x,y;
{
	register	long	*p;

	if(VA.numpix[VA.curbuffer]<MAXSAVEDPIX)
	{	VA.pixcolors[VA.curbuffer][VA.numpix[VA.curbuffer]]=VA.color;
		p=VA.pix[VA.curbuffer]+VA.numpix[VA.curbuffer]++;
		
	asm	{
		move.l	VA.quickrow,A1		;	Pointer to base address table
		move.l	(0,A1,y.w*4),A0		;	Get pointer to row base
		add.w	x,A0				;	Add x to base address
		move.l	A0,(p)
		}
	}
}
void	VASafePixel(x,y)
register	int	x,y;
{
	register	long	*p;

	if(VA.numpix[VA.curbuffer]<MAXSAVEDPIX)
	{	if(x<VA.frame.left)	return;
		if(x>=VA.frame.right)	return;
		if(y<VA.frame.top)	return;
		if(y>=VA.frame.bottom)return;
		{	VA.pixcolors[VA.curbuffer][VA.numpix[VA.curbuffer]]=VA.color;
			p=VA.pix[VA.curbuffer]+VA.numpix[VA.curbuffer]++;
			
		asm	{
			move.l	VA.quickrow,A1		;	Pointer to base address table
			move.l	(0,A1,y.w*4),A0		;	Get pointer to row base
			add.w	x,A0				;	Add x to base address
			move.l	A0,(p)
			}
		}
	}
}

void	VAPlotPixels(count,pixels,colors)
int		count;
long	*pixels;
int		*colors;
{
asm	{
		movem.l	D5/A2,-(sp)
		move.w	count,D2
		subq.w	#1,D2
		bmi.s	@nopix

		move.l	pixels,A2
		move.l	colors,A1
		move.l	VA.offset,D0
		move.w	VA.field,D5	;	Field width to D5
@loop
		move.l	(A2)+,A0	;	Read next address
		move.w	(A1)+,D1	;	Read next VA.color
		dc.l	0xEFD01825	;	BFINS	BitValue,(RowBase){OffSet:4}
		dbra	D2,@loop
		
@nopix
		movem.l	(sp)+,D5/A2
	}
}
void	VAErasePixels(count,pixels)
int		count;
long	*pixels;
{
asm	{
		move.l	D5,-(sp)
		move.w	count,D2
		subq.w	#1,D2
		bmi.s	@nopix

		move.l	pixels,A1
		move.l	VA.offset,D0
		move.w	VA.field,D1	;	Field width to D5
@loop
		move.l	(A1)+,A0	;	Read next address
		dc.l	0xEED01821	;	BFINS	BitValue,(RowBase){OffSet:4}
		dbra	D2,@loop
		
@nopix
		move.l	(sp)+,D5
	}
}

void	VASpot(x,y)
register	int	x,y;
{
	register	long	*p;

	y--;
	if(VA.numspots[VA.curbuffer]<MAXSAVEDPIX)
	{	VA.spotcolors[VA.curbuffer][VA.numspots[VA.curbuffer]]=VA.color;
		p=VA.spots[VA.curbuffer]+VA.numspots[VA.curbuffer]++;
		
	asm	{
		move.l	VA.quickrow,A1		;	Pointer to base address table
		move.l	(0,A1,y.w*4),A0		;	Get pointer to row base
		add.w	x,A0				;	Add x to base address
		move.l	A0,(p)
		}
	}
}
void	VASafeSpot(x,y)
register	int	x,y;
{
	register	long	*p;

	y--;
	if(VA.numspots[VA.curbuffer]<MAXSAVEDPIX)
	{	if(x<=VA.frame.left)		return;
		if(x>=VA.frame.right-2)	return;
		if(y<=VA.frame.top)		return;
		if(y>=VA.frame.bottom-2)	return;
		{	VA.spotcolors[VA.curbuffer][VA.numspots[VA.curbuffer]]=VA.color;
			p=VA.spots[VA.curbuffer]+VA.numspots[VA.curbuffer]++;
			
		asm	{
			move.l	VA.quickrow,A1		;	Pointer to base address table
			move.l	(0,A1,y.w*4),A0		;	Get pointer to row base
			add.w	x,A0				;	Add x to base address
			move.l	A0,(p)
			}
		}
	}
}

void	VAPlotSpots(count,pixels,colors)
int		count;
long 	*pixels;
int		*colors;
{
asm	{
		movem.l	D3-D5/A2,-(sp)

		move.w	count,D2
		subq.w	#1,D2
		bmi.s	@nopix

		move.w	VA.row,D4
		subq.w	#1,D4
		moveq.l	#8,D5
		move.l	pixels,A2
		move.l	colors,A1
		move.l	VA.offset,D0
		move.w	VA.field,D3	;	Field width to D5

@loop
		move.w	(A1)+,D1	;	Read next VA.color
		move.l	(A2)+,A0	;	Read next address
		dc.l	0xEFD01823	;	BFINS	VA.color,(RowBase){OffSet:field}
		add.w	D4,A0
		dc.l	0xEFD01823	;	BFINS	VA.color,(RowBase){OffSet:field}
		add.l	D5,D0
		dc.l	0xEFD01823	;	BFINS	VA.color,(RowBase){OffSet:field}
		add.l	D5,D0
		dc.l	0xEFD01823	;	BFINS	VA.color,(RowBase){OffSet:field}
		add.w	D4,A0
		dc.l	0xEFD01823	;	BFINS	VA.color,(RowBase){OffSet:field}
		sub.l	D5,D0
		sub.l	D5,D0
		dbra	D2,@loop
		
@nopix
		movem.l	(sp)+,D3-D5/A2
	}
}
void	VAEraseSpots(count,pixels)
int		count;
long	*pixels;
{
asm	{
		movem.l	D3-D5,-(sp)
		move.w	VA.row,D4
		subq.w	#1,D4
		moveq.l	#8,D5
		move.w	count,D2
		move.l	pixels,A1
		move.l	VA.offset,D0
		move.w	VA.field,D3	;	Field width to D5
		subq.w	#1,D2
		bmi.s	@nopix
@loop
		move.l	(A1)+,A0	;	Read next address
		dc.l	0xEED00823	;	BFTST	(RowBase){OffSet:field}
		add.w	D4,A0
		dc.l	0xEED00823	;	BFTST	(RowBase){OffSet:field}
		add.l	D5,D0
		dc.l	0xEED00823	;	BFTST	(RowBase){OffSet:field}
		add.l	D5,D0
		dc.l	0xEED00823	;	BFTST	(RowBase){OffSet:field}
		add.w	D4,A0
		dc.l	0xEED00823	;	BFTST	(RowBase){OffSet:field}
		sub.l	D5,D0
		sub.l	D5,D0
		dbra	D2,@loop
		
@nopix
		movem.l	(sp)+,D3-D5
	}
}