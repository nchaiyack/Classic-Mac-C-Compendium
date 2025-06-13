/*/
     Project Arashi: VAFractalLine.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, March 24, 1993, 19:06
     Created: Tuesday, January 15, 1991, 23:33

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"

static	char	*JitterP;
static	int		JitterStep;
static	int		JitterPoint;
static	int		SizzleLevel;
static	int		SizzleFactor;
static	int		OpenCounter=0;

void	VACloseFractalLines()
{
	if(!--OpenCounter)
		DisposPtr(JitterP);
}
void	VAInitFractalLines()
{
	register	int		i;
	
	if(!OpenCounter++)
	{	JitterP=NewPtr(sizeof(char)*256);
	
		for(i=0;i<256;i++)
		{	JitterP[i]=(((FastRandom()>>4) & 15)
							* Sins[((unsigned int)FastRandom()) % ANGLES])
							/ 256;
		}
	}
}

void	SizzlingLine(x1,y1,x2,y2)
int		x1,y1,x2,y2;
{
	register	int		midx,midy;
	register	int		dx,dy;
	register	long	sizzler;

	dx=(x2-x1);
	dy=(y2-y1);
	sizzler=JitterP[*(unsigned char *)&JitterPoint];
	JitterPoint+=JitterStep;

	midx=x1+(dx>>1)+((dy*sizzler)/SizzleFactor);
	midy=y1+(dy>>1)-((dx*sizzler)/SizzleFactor);

	if(--SizzleLevel>0)
	{	SizzlingLine(x1,y1,midx,midy);
		SizzlingLine(midx,midy,x2,y2);
	}
	else
	{	VASafeLineTo(midx,midy);
		VASafeLineTo(x2,y2);
	}
	SizzleLevel++;
}
void	VAFractalLine(x1,y1,x2,y2,factor,level)
int		x1,y1,x2,y2,factor,level;
{
	SizzleFactor=factor;
	JitterStep=FastRandom();
	SizzleLevel=level;

	VAMoveTo(x1,y1);	
	SizzlingLine(x1,y1,x2,y2);
}

void	VAFractalLineTo(x,y,factor,level)
int		x,y,factor,level;
{
	VAFractalLine(VA.CurrentX,VA.CurrentY,x,y,factor,level);
	VAMoveTo(x,y);
}