/*/
     Project Arashi: VA7Segment.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:40
     Created: Friday, March 24, 1989, 15:07

     Copyright � 1989-1992, Juri Munkki
/*/

/*
**	These routines are a precursor to the 14 segment
**	display routines. I left them here, because it is
**	faster to draw the numbers with just 8 segments.
**
**	Your digital watch might have the same kind of
**	numbers that these routines produce.
**
**	The 14 segment routines are better documented and
**	somewhat similar to the following.
*/

#include "VA.h"
#include "VAInternal.h"


char	numbers[]={	1+2+4+8+16+32,		/*	0	*/
					  2+4,				/*	1	*/
					1  +4+8   +32+64,	/*	2	*/
					1+2+4+8      +64,	/*	3	*/
					  2+4  +16   +64,	/*	4	*/
					1+2  +8+16   +64,	/*	5	*/
					1+2  +8+16+32+64,	/*	6	*/
					  2+4+8,			/*	7	*/
					1+2+4+8+16+32+64,	/*	8	*/
					1+2+4+8+16   +64,	/*	9	*/
					0};
void	VADrawSegments(x,y,code)
int		x,y,code;
{
	asm	{	
			move.l	D3,-(SP)
			move.w	code,D3
			move.l	VA.base,A0
			move.w	y,D0
			move.w	VA.row,D1
			mulu.w	D1,D0
			add.l	D0,A0
			add.w	x,A0
			addq.l	#1,A0
			
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg0
			move.w	VA.color,D0
	@seg0	
			move.b	D0,(A0)+
			dbra	D2,@seg0
			
			sub.w	D1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg1
			move.w	VA.color,D0
	@seg1
			move.b	D0,(A0)
			sub.w	D1,A0
			dbra	D2,@seg1
			
			sub.w	D1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg2
			move.w	VA.color,D0
	@seg2
			move.b	D0,(A0)
			sub.w	D1,A0
			dbra	D2,@seg2
			
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg3
			move.w	VA.color,D0
	@seg3
			move.b	D0,-(A0)
			dbra	D2,@seg3
			
			subq.l	#1,A0
			add.w	D1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg4
			move.w	VA.color,D0
	@seg4
			move.b	D0,(A0)
			add.w	D1,A0
			dbra	D2,@seg4
			
			move.l	A0,A1
			add.w	D1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg5
			move.w	VA.color,D0
	@seg5
			move.b	D0,(A0)
			add.w	D1,A0
			dbra	D2,@seg5
			
			addq.l	#1,A1
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			ror.w	#1,D3
			bcc.s	@seg6
			move.w	VA.color,D0
	@seg6
			move.b	D0,(A1)+
			dbra	D2,@seg6
			move.l	(sp)+,D3
		}
}

void	VADrawNumber(num,x,y)
register	long	num;
register	int		x,y;
{
	register	int		dig,neg;
				int		mode;

	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);

	if(num<0)
	{	num=-num;
		neg=-1;
	}
	else
	{	neg=0;
	}
	do
	{	dig= num % 10;
		num/=10;
		VADrawSegments(x,y,numbers[dig]);
		x-= VA.segmscale + 7;
	}	while(num!=0 && x>0);
	if(x>0 && neg)
		VADrawSegments(x,y,64);

	SwapMMUMode(&mode);
}

void	VADrawPadNumber(num,x,y,width)
register	long	num;
register	int		x,y,width;
{
	register	int		dig,neg;
				int		mode;

	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);

	if(num<0)
	{	num=-num;
		neg=-1;
	}
	else
	{	neg=0;
	}
	do
	{	dig= num % 10;
		num/=10;
		VADrawSegments(x,y,numbers[dig]);
		x-= VA.segmscale + 7;
	}	while(num!=0 && x>0 && --width);
	
	if(x>0 && neg && --width>0)
	{	VADrawSegments(x,y,64);
		x-= VA.segmscale + 7;
	}
	while(--width>0 && x>0)
	{	VADrawSegments(x,y,0);
		x-= VA.segmscale + 7;
	}
	SwapMMUMode(&mode);
}