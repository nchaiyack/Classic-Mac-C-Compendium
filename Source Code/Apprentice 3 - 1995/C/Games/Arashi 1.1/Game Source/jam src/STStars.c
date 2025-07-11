/*/
     Project Arashi: STStars.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:45
     Created: Sunday, January 6, 1991, 7:20

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

#define  ARRIVALSPEED		8
#define  CENTERSTARTRATIO	96

typedef	struct
{	
	int		index[2];
	int		direction[2];
	int		ratio;
	char	color;
	char	state;
	int		*notify;
}	CenterStar;

enum	{	available,	used,	unavailable	};

CenterStar	*AllStars;
int			TotalStars;
int			FirstUnused;

void	AllocCStars()
{
	TotalStars=MAXCENTERSTARS;
	AllStars=(CenterStar *)NewPtr(sizeof(CenterStar)*(MAXCENTERSTARS+1));
	FirstUnused=0;
}
void	InitCStars()
{
	register	int		i,d;
	
	FirstUnused=0;
	for(i=0;i<TotalStars;i++)
	{	for(d=0;d<2;d++)
		{	AllStars[i].index[d]=VAPosRandom() % ww.starsegs;
			AllStars[i].direction[d]=((VARandom() & 1)<<1) -1;
		}
		AllStars[i].ratio=CENTERSTARTRATIO;
		AllStars[i].color=i & 7;
		AllStars[i].state=available;
	}
}
void	UpdateCStars()
{
	register	int			i,d;
	register	int			x1,x0,y1,y0;
	register	CenterStar	*csp;
				CenterStar	temp;
				int			availcount;
	
	availcount=ThisLevel.starCount;

	csp=AllStars;
	VA.color=5;
	for(i=0;i<TotalStars;i++)
	{	x1=	ww.starx[csp->index[1]][1];
		x0=	ww.starx[csp->index[0]][0]-x1;
		y1=	ww.stary[csp->index[1]][1];
		y0=	ww.stary[csp->index[0]][0]-y1;

		if(csp->state==used)
		{	csp->ratio+=ARRIVALSPEED;
			if(csp->ratio>=256)
			{	csp->ratio=CENTERSTARTRATIO;
				csp->state=available;
				csp->color=0;
				temp=*csp;
				
				for(d=0;d<2;d++)
				{	csp->index[d]=VAPosRandom() % ww.starsegs;
					csp->direction[d]=((VARandom() & 1)<<1) -1;
				}
				*csp=AllStars[--FirstUnused];
				AllStars[FirstUnused]=temp;
				*(temp.notify)= -1-temp.index[0];
			}
		}
		else
		{	if(availcount-- > 0)
			{	csp->color++;
				if(csp->color>6)	csp->color=0;
			}
			else
			{	csp->state=unavailable;
			}
		}
		
		if(csp->state!=unavailable)	
		{	VA.color=csp->color;
			VASafePixel(	x1+(long)x0*csp->ratio/256,
							y1+(long)y0*csp->ratio/256);
		}
		else
		{	csp->state=available;
		}
	
		for(d=0;d<2;d++)
		{	csp->index[d]+=csp->direction[d];
			if(ww.wraps)
			{	if(csp->index[d]<0)
					csp->index[d]+=ww.starsegs;
				else
				if(csp->index[d]>=ww.starsegs)
					csp->index[d]-=ww.starsegs;
			}
			else
			{	if(csp->index[d]<=0)
				{	csp->index[d]=0;
					csp->direction[d]=-csp->direction[d];
				}
				if(csp->index[d]>=ww.starsegs)
				{	csp->index[d]=ww.starsegs;
					csp->direction[d]=-csp->direction[d];
				}
			}
		}
		csp++;
	}
}

void	StarApproach(notify,color)
int		*notify;
int		color;
{	
	if(FirstUnused>=TotalStars)
	{	*notify=-(((unsigned int)VARandom()) % (ww.numsegs-1) + 1);
	}
	else
	{	AllStars[FirstUnused].state=used;
		AllStars[FirstUnused].notify=notify;
		AllStars[FirstUnused].color=color;
		FirstUnused++;
		*notify=0;
	}
}