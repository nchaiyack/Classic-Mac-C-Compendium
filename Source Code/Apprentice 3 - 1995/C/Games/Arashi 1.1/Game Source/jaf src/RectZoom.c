/*/
     Project Arashi: RectZoom.c
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, March 23, 1993, 20:43
     Created: Sunday, March 14, 1993, 22:01

     Copyright � 1993, Juri Munkki
/*/

/*
** //Jaf � copyright 1991
*/

#include "VA.h"
#include "STORM.h"

#define	ZOOMSTEPS	20
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
extern  long	Phaze;
extern  int		SpecialEvent;

void	DrawZoom()
{
	int		i,sizfactor;
	Point	*Mid,*To;
	
	VA.color = 2;
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
		sizfactor+=7;
		Mid=MidPoints;
		for(i=0;i<4;i++)
		{	VAFractalLine(Mid->h,Mid->v,MidPoints[(i+1)&3].h,MidPoints[(i+1)&3].v,1<<sizfactor,5);
			Mid++;
		}
	}
}
void	NewZoomRect(r)
Rect	*r;
{
	Point	swapper,*dest;
	int		i;
	
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
Rect	*r;
{
	int		i;

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

/* Jaf */
void	DrawZoomer()
{
	extern Rect	Zoom;	
	int MaxScale;
	extern int MainStage;
	
	MaxScale=Getfontscale();
	
	if(!Phaze) {
		InitZoomRect(&Zoom);
		if(MainStage >= 13){	
		 	Zoom.right=(VA.frame.right-VA.frame.right/12);
			Zoom.bottom=(VA.frame.bottom-VA.frame.bottom/4);
			Zoom.left=(VA.frame.left+VA.frame.right/12);
			Zoom.top=(VA.frame.top+VA.frame.bottom/12);
		}
		else {
			Zoom.left=((unsigned int)Random())%(VA.frame.right-22*(MaxScale*3+3));
			Zoom.right=Zoom.left+22*(MaxScale*3+3);
			Zoom.top=((unsigned int)Random())%(VA.frame.bottom-90*MaxScale);
			Zoom.bottom=Zoom.top+80*MaxScale;
		}
		NewZoomRect(&Zoom);
	}
	DrawZoom();
	if(!Phaze) {
		PlayB(PhazerOut,0);		
		SpecialEvent = 0;
	}
}
