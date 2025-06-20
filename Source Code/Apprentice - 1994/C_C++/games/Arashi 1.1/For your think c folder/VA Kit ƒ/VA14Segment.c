/*/
     Project Arashi: VA14Segment.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:40
     Created: Friday, March 24, 1989, 15:07

     Copyright � 1989-1992, Juri Munkki
/*/

/*
**	We needed some text effects for project STORM, so the
**	easiest way to create a quick and dirty font was to
**	simulate the same type of LED display that is used
**	by most pinballs today. Only uppercase characters
**	are defined. The text routines should be pretty fast
**	though, so you can create some interesting animations
**	with them. Double buffering is not currently supported
**	and all 8 pixel bits are written. A masking version
**	of these routines should be easy enough to write.
*/

#include "VA.h"
#include "VAInternal.h"

#define	N(a)	(1<<a)

int		Characters[]=		/*	Each bit represents a segment:	*/
		{	0,								/* space*/
			N(7)+N(11),
			+N(6)+N(8),
			-1,
			1+2+8+16+N(5)+N(9)+N(7)+N(11),	/*	$	*/
			+N(12)+N(8)+16+2,
			0,
			+N(7),
			+N(8)+N(10),					/*	(	*/
			N(6)+N(12),						/*	)	*/
			+N(5)+N(6)+N(7)+N(8)+N(9)+N(10)+N(11)+N(12),
											/*	*	*/
			N(5)+N(7)+N(9)+N(11),			/*	+	*/
			N(12),							/*	,	*/
			N(5)+N(9),						/*	-	*/
			N(11),							/*	.	*/
			N(12)+N(8),						/*	/	*/
			1+2+4+8+16+N(8)+N(12)+N(13),	/*	0	*/
			N(7)+N(11),						/*	1	*/
			1+4+8+N(9)+N(12),				/*	2	*/
			1+2+4+8+N(9),					/*	3	*/
			2+4+16+N(5)+N(9),				/*	4	*/
			8+64+N(9)+2+1,					/*	5	*/
			1+2+8+16+32+N(9)+N(13),			/*	6	*/
			N(12)+N(8)+8,					/*	7	*/
			1+2+4+8+16+32+N(9)+N(13),		/*	8	*/
			1+2+4+8+16+N(5)+N(9),			/*	9	*/
			0,								/*	:	*/
			0,								/*	;	*/
			1+N(12),						/*	<	*/
			1+32+N(9),						/*	=	*/
			1+N(10),						/*	>	*/
			4+8+16+N(9)+N(11),				/*	?	*/
			1+4+8+16+N(13)+N(9)+N(7),		/*	@	*/
			2+4+8+16+32+N(9)+N(13),			/*	A	*/
			1+2+4+8+N(7)+N(9)+N(11),		/*	B	*/
			1+8+16+N(13),					/*	C	*/
			1+2+4+8+N(7)+N(11),				/*	D	*/
			1+8+16+N(5)+N(9)+N(13),			/*	E	*/
			8+16+N(5)+N(9)+N(13),			/*	F	*/
			1+2+8+16+N(9)+N(13),			/*	G	*/
			2+4+16+N(5)+N(9)+N(13),			/*	H	*/
			1+8+N(7)+N(11),					/*	I	*/
			1+2+4+N(13),					/*	J	*/
			16+N(13)+N(5)+N(8)+N(10),		/*	K	*/
			1+16+N(13),						/*	L	*/
			2+4+16+N(13)+N(6)+N(8),			/*	M	*/
			2+4+16+N(13)+N(6)+N(10),		/*	N	*/
			1+2+4+8+16+N(13),				/*	O	*/
			4+8+16+N(13)+N(5)+N(9),			/*	P	*/
			1+2+4+8+16+N(13)+N(10),			/*	Q	*/
			4+8+16+N(13)+N(5)+N(9)+N(10),	/*	R	*/
			1+2+8+16+N(5)+N(9),				/*	S	*/
			8+N(11)+N(7),					/*	T	*/
			1+2+4+16+N(13),					/*	U	*/
			16+N(13)+N(12)+N(8),			/*	V	*/
			2+4+16+N(13)+N(12)+N(10),		/*	W	*/
			N(12)+N(8)+N(6)+N(10),			/*	X	*/
			N(6)+N(8)+N(11),				/*	Y	*/
			1+8+N(8)+N(12),					/*	Z	*/
			1+8+16+N(13),					/*	[	*/
			N(6)+N(10),						/*	\	*/
			1+2+4+8,						/*	]	*/
			N(12)+N(10),					/*	^	*/
			1,								/*	_	*/
			8+N(6)+N(8)+N(12)+N(10),		/*	�	*/
			0
		};

void	VADraw14Segments(x,y,code)
int		x,y,code;
{
	asm	{	
			move.l	D3,-(SP)
			move.l	A2,-(SP)
			move.w	code,D3
			move.l	VA.base,A0
			move.w	y,D0
			move.w	VA.row,D1
			mulu.w	D1,D0
			add.l	D0,A0
			add.w	x,A0
			
			move.l	A0,A2
			addq.l	#1,A0			;	Move right
			
			moveq.l	#-1,D0			;	Erase, if no segment
			move.w	VA.segmscale,D2	;	Load segment scale
			add.w	D2,D2
			ror.w	#1,D3			;	Shift code bit out
			bcc.s	@seg0			;	Visible segment?
			move.w	VA.color,D0		;	Yes.
	@seg0
			move.b	D0,(A0)+		;	Draw, move right
			dbra	D2,@seg0		;	Until done
			
			sub.w	D1,A0			;	Move up

			moveq.l	#-1,D0			;	Erase, if no segment
			move.w	VA.segmscale,D2	;	As above...
			add.w	D2,D2
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
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@seg2
			move.w	VA.color,D0
	@seg2
			move.b	D0,(A0)
			sub.w	D1,A0
			dbra	D2,@seg2
			
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@seg3
			move.w	VA.color,D0
	@seg3
			move.b	D0,-(A0)
			dbra	D2,@seg3
			
			subq.l	#1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@seg4
			move.w	VA.color,D0
	@seg4
			add.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@seg4

			addq.l	#1,A0
			add.w	D1,A0
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@seg5
			move.w	VA.color,D0
	@seg5
			move.b	D0,(A0)+
			dbra	D2,@seg5
			
			move.l	A0,A1			;	Center. Store in A1

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@seg6
			move.w	VA.color,D0
	@seg6
			sub.w	D1,A0
			move.b	D0,-(A0)
			sub.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@seg6
			
			move.l	A1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@seg7
			move.w	VA.color,D0
	@seg7
			sub.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@seg7
			
			move.l	A1,A0

			addq.l	#1,A0
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@seg8
			move.w	VA.color,D0
	@seg8
			sub.w	D1,A0
			move.b	D0,(A0)
			sub.w	D1,A0
			move.b	D0,(A0)+
			dbra	D2,@seg8
	
			move.l	A1,A0

			addq.l	#1,A0
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@seg9
			move.w	VA.color,D0
	@seg9
			move.b	D0,(A0)+
			dbra	D2,@seg9
			
			move.l	A1,A0

			addq.l	#1,A0
			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@segA
			move.w	VA.color,D0
	@segA
			add.w	D1,A0
			move.b	D0,(A0)
			add.w	D1,A0
			move.b	D0,(A0)+
			dbra	D2,@segA

			move.l	A1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@segB
			move.w	VA.color,D0
	@segB
			add.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@segB
			
			move.l	A1,A0

			moveq.l	#-1,D0
			move.w	VA.segmscale,D2
			subq.l	#1,D2
			ror.w	#1,D3
			bcc.s	@segC
			move.w	VA.color,D0
	@segC
			add.w	D1,A0
			move.b	D0,-(A0)
			add.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@segC

			move.l	A2,A0
			moveq.l	#-1,D0			;	Erase, if no segment
			move.w	VA.segmscale,D2	;	As above...
			add.w	D2,D2
			ror.w	#1,D3
			bcc.s	@segD
			move.w	VA.color,D0
	@segD
			sub.w	D1,A0
			move.b	D0,(A0)
			dbra	D2,@segD

			move.l	(sp)+,A2
			move.l	(sp)+,D3
		}
}

/*
**	VA14Seg draws a shape with the 14 segment system.
*/
void	VA14Seg(num,x,y)
int		num;
int		x,y;
{
	register	int		dig,neg;
				int		mode;

	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);

	VADraw14Segments(x,y,Characters[num]);
	SwapMMUMode(&mode);
}

/*
**	VADrawText is similar to the QuickDraw DrawText,
**	but you use VAMoveTo to specify the location.
*/
void	VADrawText(p,a,b)
register	char	*p;
register	long	a,b;
{
	register	unsigned	char	thechar;
	register				int		step;
							int		mode;

	p = (char *) StripAddress((void *) p);
	mode=QD32COMPATIBLE;
	SwapMMUMode(&mode);

	p+=a;
	b+=a;
	if(VA.CurrentX>=0
		&& VA.CurrentY>=(VA.segmscale*4+4)
		&& VA.CurrentY<VA.frame.bottom)
	{	step= VA.segmscale*3+3;
		while(a++<b)
		{	if(VA.CurrentX<VA.frame.right-step)
			{	thechar= *p++ - 32;
				if(thechar>=64) thechar-=32;
				if(thechar>64) thechar=64;
				VADraw14Segments(VA.CurrentX,VA.CurrentY,Characters[thechar]);
				VA.CurrentX+=step;
			}
			else
			{	a=b;
			}
		}
	}
	SwapMMUMode(&mode);
}