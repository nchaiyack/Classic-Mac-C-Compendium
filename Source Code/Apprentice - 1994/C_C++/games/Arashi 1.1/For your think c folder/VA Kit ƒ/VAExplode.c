/*/
     Project Arashi: VAExplode.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:41
     Created: Saturday, February 11, 1989, 0:53

     Copyright � 1989-1992, Juri Munkki
/*/

#include "VA.h"
#include "VAInternal.h"

#define	NUMEXPLOSIONS  30
#define	RESEXPLOSIONS	4

int			numexploding;
int			**exploder;
int			**exploffset;
int			*explx;
int			*exply;
int			*explcolor;

int		*Boom[RESEXPLOSIONS];
int		*Bang[RESEXPLOSIONS];

Ptr		MaxBaseAddr,MinBaseAddr;

void	VALoadExplosions()
{
				int		n;
	register	int		m,i;
	register	long	delta;
	register	int		*p;
	register	long	*q;
				Handle	Kaboom;
	
	MinBaseAddr=VA.base+VA.frame.top*(long)VA.row;
	MaxBaseAddr=VA.base+VA.frame.bottom*(long)VA.row;
	
	for(n=0;n<RESEXPLOSIONS;n++)
	{	Kaboom=GetResource('GNRL',n+1000);
		HLock(Kaboom);
		Bang[n]=(int *)NewPtr(GetHandleSize(Kaboom));
		Boom[n]=(int *)*Kaboom;

		p=Boom[n];
		q=(long *)Bang[n];

		do
		{	m=*p++;
			q=(long *)(((Ptr) q)+2);
			for(i=0;i<m;i++)
			{	delta = *p++;						/*	X coord	*/
				delta+= *p++ * (long)VA.row;		/*	Y coord	*/
				*q++=delta;
			}
		} while(m!=0);
	}
	exploder	=(void *) NewPtr(sizeof(int *)	* NUMEXPLOSIONS);
	exploffset	=(void *) NewPtr(sizeof(int *)	* NUMEXPLOSIONS);
	explx		=(void *) NewPtr(sizeof(int)	* NUMEXPLOSIONS);
	exply		=(void *) NewPtr(sizeof(int)	* NUMEXPLOSIONS);
	explcolor	=(void *) NewPtr(sizeof(int)	* NUMEXPLOSIONS);
	numexploding=0;

}

void	VAExplosion(x,y,size,color)
int		x,y,size,color;
{
	register	int		n;
	
	if(numexploding<NUMEXPLOSIONS)
	{	n=numexploding++;
		exploder[n]		=Boom[size];
		exploffset[n]	=Bang[size];
		explx[n]		=x;
		exply[n]		=y;
		explcolor[n]	=color;
	}
}

void	VAInsertExplosions()
{
	register	int		n,m,x,y,color;
	register	long	*pixp;
	register	int		*coordp,*offs;
				int		*colp;
				int		*numpix;

	numpix=VA.numpix+VA.curbuffer;

	for(n=0;n<numexploding;n++)
	{	x=explx[n];
		y=exply[n];
		color=explcolor[n];
		coordp=exploder[n];
		offs=exploffset[n];
		m=*coordp++;

		if(*numpix+m < MAXSAVEDPIX)
		{	colp=VA.pixcolors[VA.curbuffer]+*numpix;
			pixp=VA.pix[VA.curbuffer]+*numpix;
		asm	{
			addq.l	#2,offs			;	Move to next offset
			move.w	VA.row,D0		;	Calculate base address
			mulu.w	y,D0
			move.l	D0,A0
			add.w	x,A0
			add.l	VA.base,A0

			subq.l	#1,m
		@loop
			move.w	(coordp)+,D0	;	X offset
			addq.l	#2,coordp		;	Move past Y offset
			add.w	x,D0			;	add center x
			cmp.w	VA.frame.left,D0	;	too much left?
			blt.s	@nopixel
			cmp.w	VA.frame.right,D0	;	too much right?
			bge.s	@nopixel

			move.l	A0,A1			;	Copy explosion base
			add.l	(offs),A1		;	read offset
			cmp.l	MinBaseAddr,A1	;	above screen?
			bcs.s	@nopixel		;	yes
			cmp.l	MaxBaseAddr,A1	;	under screen?
			bcc.s	@nopixel		;	yes

			move.l	A1,(pixp)+		;	pixel is ok.
			move.l	colp,A1			;	queue color
			move.w	color,(A1)
			addq.l	#2,colp			;	next color
		@nopixel
			addq.l	#4,offs			;	next offset
			dbra	m,@loop
			}
			*numpix+=	pixp-(VA.pix[VA.curbuffer]+*numpix);
		}
		else
		{	offs+= 1+2*m;
			coordp+=2*m;
		}

		exploder[n]=coordp;
		exploffset[n]=offs;
		if(*coordp==0)
		{	/*	Remove from queue	*/
			numexploding--;
			explx[n]		=explx[numexploding];
			exply[n]		=exply[numexploding];
			exploder[n]		=exploder[numexploding];
			exploffset[n]	=exploffset[numexploding];
			explcolor[n]	=explcolor[numexploding];
			n--;
		}
	}
}