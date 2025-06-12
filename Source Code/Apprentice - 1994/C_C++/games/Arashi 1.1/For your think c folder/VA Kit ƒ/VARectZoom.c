/*/
     Project Arashi: VARectZoom.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Tuesday, February 19, 1991, 21:58

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

#define	ZOOMSTEPS	32
#define	MAPPINGS	5

int		RectMappings[][4]={ 
					{	1,2,3,0	},
					{	3,0,1,2	},
					{	2,3,0,1	},
					{	2,1,0,3	},
					{	0,3,2,1	}};

Point	FromPoints[4];
Point	MidPoints[4];
Point	ToPoints[4];
int		*CurrentMapping;
long	Phaze;

void	DrawZoom()
{
	register	int		i,sizfactor;
	register	Point	*Mid,*To;
	
	if(Phaze>0)
	{	Phaze--;
		Mid=MidPoints;
		To=ToPoints;
		for(i=0;i<4;i++)
		{	Mid->h = To->h + (Phaze * (FromPoints[CurrentMapping[i]].h - To->h)) / ZOOMSTEPS;
			Mid->v = To->v + (Phaze * (FromPoints[CurrentMapping[i]].v - To->v)) / ZOOMSTEPS;
			Mid++;
			To++;
		}
		
	}

#define	JITTERTIME	6
	if(Phaze>JITTERTIME && Phaze<ZOOMSTEPS-JITTERTIME)
	{	VAMoveTo(MidPoints[3].h,MidPoints[3].v);
		Mid=MidPoints;
		for(i=0;i<4;i++)
		{	VASafeLineTo(Mid->h,Mid->v);
			Mid++;
		}
	}
	else
	{	if(Phaze>=ZOOMSTEPS-JITTERTIME)
			sizfactor=ZOOMSTEPS-Phaze;
		else
			sizfactor=Phaze;
		sizfactor+=6;
		Mid=MidPoints;
		for(i=0;i<4;i++)
		{	VAFractalLine(Mid->h,Mid->v,MidPoints[(i+1)&3].h,MidPoints[(i+1)&3].v,1<<sizfactor,5);
			Mid++;
		}
	}
	if(Phaze==0)
	{	PlayA(PhazerOut,0);
		PlayB(PhazerOut,0);
	}
}
void	NewZoomRect(r)
register	Rect	*r;
{
				Point	swapper;
	register	Point	*dest;
	register	int		i;
	
	for(i=0;i<4;i++)
	{	FromPoints[i]=ToPoints[i];
	}

	ToPoints[0].h=r->left;
	ToPoints[1].h=r->left;
	ToPoints[2].h=r->right;
	ToPoints[3].h=r->right;

	ToPoints[0].v=r->top;
	ToPoints[1].v=r->bottom;
	ToPoints[2].v=r->bottom;
	ToPoints[3].v=r->top;
	
	CurrentMapping=RectMappings[FastPosRandom() % MAPPINGS];
	Phaze=ZOOMSTEPS+1;
}

void	InitZoomRect(r)
register	Rect	*r;
{
	register	int		i;

	FromPoints[0].h=r->left;
	FromPoints[1].h=r->left;
	FromPoints[2].h=r->right;
	FromPoints[3].h=r->right;

	FromPoints[0].v=r->top;
	FromPoints[1].v=r->bottom;
	FromPoints[2].v=r->bottom;
	FromPoints[3].v=r->top;
	
	for(i=0;i<4;i++)
	{	ToPoints[i]=FromPoints[i];
		MidPoints[i]=FromPoints[i];
	}
	
	Phaze=0;
}

#ifdef TEST_ZOOMRECT
void	TestZoomRect()
{
	Rect	r;
	int		oldspeed;
	
	VAInitFractalLines();

	oldspeed=VA.FrameSpeed;
	VA.FrameSpeed=3;
	SetRect(&r,10,10,300,200);
	InitZoomRect(&r);
	
	VA.color=5;

	while(!Button())
	{
		r.right=(FastPosRandom() % VA.frame.right);
		r.bottom=(FastPosRandom() % VA.frame.bottom);
		r.left=(FastPosRandom() % (VA.frame.right-r.right));
		r.top=(FastPosRandom() % (VA.frame.bottom-r.bottom));

		r.right+=r.left;
		r.bottom+=r.top;
		NewZoomRect(&r);
		
		while(Phaze)
		{
			DrawZoom();
			VAStep();
		}
		DrawZoom();
		VAStep();
	}

	r=VA.frame;
	NewZoomRect(&r);
	while(Phaze)
	{	DrawZoom();
		VAStep();
	}
	VA.FrameSpeed=oldspeed;
	
	PlayA(Blast,100);
	PlayB(Blast,100);
	VACloseFractalLines();
}
#endif