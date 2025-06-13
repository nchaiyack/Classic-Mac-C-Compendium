/*/
     Project Arashi: NewTitleEffect.c
     Major release: Version 1.1, 7/22/92

     Last modification: Friday, July 23, 1993, 0:25
     Created: Sunday, March 14, 1993, 22:00

     Copyright � 1993, Juri Munkki
/*/

#include "VA.h"
#include "ThingBlocks.h"
#include "NewTitleEffect.h"

void	VAFractalLine(int,int,int,int,int,int);

typedef struct
{
	int		state;

	long	x,y;
	int		sx,sy;
	int		timer;
}	LiveLineSegment;

enum	/*	States	*/
{	startState=0, sparkState, solidState, lastState	};

ThingBlock	PolyThing;

void	ScalePolyThingToRect(ThingBlock	theThing, Rect *newBounds)
{
	Rect				CurrentBounds;
	int					counter;
	ThingBlockHeader	*thing;
	LiveLineSegment		*lp;
	char				state;
	int					x,y;
	long				o1x,o1y,s1x,s1y;
	long				o2x,o2y,s2x,s2y;
	
	state = HGetState(theThing);
	HLock(theThing);
	thing = *theThing;
	
	counter = thing->numItems;
	lp = (void *)thing->theStuff;
	SetRect(&CurrentBounds,32767,32767,-32768,-32768);
	
	while(counter--)
	{	x = lp->x;
		y = lp->y;
		if(x > CurrentBounds.right)		CurrentBounds.right = x;
		if(x < CurrentBounds.left)		CurrentBounds.left = x;
		if(y > CurrentBounds.bottom)	CurrentBounds.bottom = y;
		if(y < CurrentBounds.top)		CurrentBounds.top = y;
		lp++;
	}
	
	o1x = (CurrentBounds.left+CurrentBounds.right)/2;
	o1y = (CurrentBounds.top+CurrentBounds.bottom)/2;
	s1x = CurrentBounds.right - CurrentBounds.left;
	s1y = CurrentBounds.bottom - CurrentBounds.top;
	
	o2x = (newBounds->left+newBounds->right)/2;
	o2y = (newBounds->top+newBounds->bottom)/2;
	s2x = newBounds->right - newBounds->left;
	s2y = newBounds->bottom - newBounds->top;
	
	/*	Maintain aspect ratio.	*/
	if(s1x * s2y < s1y * s2x)
	{	s2x = s2y;
		s1x = s1y;
	}

	counter = thing->numItems;
	lp = (void *)thing->theStuff;
	while(counter--)
	{	lp->x = o2x + ((lp->x - o1x) * s2x) / s1x;
		lp->y = o2y + ((lp->y - o1y) * s2x) / s1x;
		lp->sx = lp->sx * s2x / s1x;
		lp->sy = lp->sy * s2x / s1x;
		lp++;
	}

	HSetState(theThing, state);
}

pascal	void	MyStdPoly(grafVerb,thePoly)
int			grafVerb;
PolyHandle	thePoly;
{
	long			polysize;
	Point			*p;
	LiveLineSegment	newsegment;
	char			state;
	
	StdPoly(grafVerb,thePoly);

	state = HGetState(thePoly);
	HLock(thePoly);
	polysize = (GetHandleSize(thePoly)-14)>>2;
	p = (*thePoly)->polyPoints;
	
	while(polysize-- > 0)
	{	newsegment.x = p->h;
		newsegment.y = p->v;
		p++;
		newsegment.sx = p->h - newsegment.x;
		newsegment.sy = p->v - newsegment.y;
		newsegment.state = startState;
		AddThing(PolyThing,&newsegment);
	}

	HSetState(thePoly,state);
}

void	ConvertToPolys(resId)
int		resId;
{
	long		someBits[10];
	BitMap		someMap;
	PicHandle	thePicture;
	GrafPtr		tempPort;
	GrafPtr		saved;
	CQDProcs	myQDProcs;


	PolyThing = NewThingBlock(sizeof(LiveLineSegment));

	GetPort(&saved);
	
	tempPort = (GrafPtr) NewPtr(sizeof(GrafPort));
	OpenPort(tempPort);
	SetPort(tempPort);

	someMap.baseAddr = (Ptr)someBits;
	SetRect(&someMap.bounds,0,0,16,5);
	someMap.rowBytes = 4;
	SetPortBits(&someMap);

	SetStdCProcs(&myQDProcs);

	myQDProcs.polyProc = (Ptr)MyStdPoly;
	tempPort->grafProcs = (void *)&myQDProcs;

	thePicture = GetPicture(resId);
	HLock(thePicture);
	DrawPicture(thePicture,&((*thePicture)->picFrame));
	HUnlock(thePicture);
	ReleaseResource(thePicture);

	SetPort(saved);

	tempPort->grafProcs = 0;
	ClosePort(tempPort);
	DisposPtr(tempPort);

	if(0)
	{	Rect	testr = { 10, 10, 120, 400};
		ThingBlockHeader	*thing;
		LiveLineSegment		*lp;
		int					counter;
	
		ScalePolyThingToRect(PolyThing,&testr);
		EraseRect(&testr);
		FrameRect(&testr);
		HLock(PolyThing);
		thing = *PolyThing;
		
		counter = thing->numItems;
		lp = (void *)thing->theStuff;

		while(counter--)
		{	MoveTo(lp->x,lp->y);
			Line(lp->sx,lp->sy);
			lp++;
		}
		
		while(!Button());
	}
}

void	titlemain(ResPictNum, fadeDirection)
int		ResPictNum;
int		fadeDirection;
{
	EventRecord			Event;
	int					i;
	Rect				titleRect;
	ThingBlockHeader	*thing;
	LiveLineSegment		*lp;
	int					counter;
	int					sweep,dsweep;
	int					doneOnce = 0;
		
	ConvertToPolys(ResPictNum);
	randSeed=TickCount();
	
	VA.FrameSpeed=3; 
	
	titleRect = VA.frame;
	InsetRect(&titleRect,titleRect.right >> 3, titleRect.bottom >> 3);
	ScalePolyThingToRect(PolyThing,&titleRect);
	HLock(PolyThing);
	thing = *PolyThing;

	sweep = 0;
	if (fadeDirection == HORIZONTAL)
		dsweep = 1 + (titleRect.right >> 6);
	else
	{	
		dsweep = 1 + (titleRect.bottom >>7);
		sweep += 2*dsweep;
	}

	do
	{	Boolean	needsDraw;
	
		needsDraw = FALSE;

		counter = thing->numItems;
		lp = (void *)thing->theStuff;
		
		sweep += dsweep;
		
		while(counter--)
		{	switch(lp->state)
			{	case startState:
					if (fadeDirection == HORIZONTAL)
					{
						if(sweep > lp->x+(lp->sx >> 1))
						{	lp->state = sparkState;
							lp->timer = 6;
						}
					}
					else
						if(sweep > lp->y+(lp->sy >> 1))
						{	lp->state = sparkState;
							lp->timer = 6;
						}
					break;
				case sparkState:
					needsDraw = TRUE;
					VA.color = 1;
					VAFractalLine(lp->x,lp->y,
								  lp->x+lp->sx,lp->y+lp->sy,
								  1<<(8-(lp->timer >> 1)),3);
					if(lp->timer-- <= 0)
					{	lp->state = solidState;
						lp->timer = 100;
					}
					break;
				case solidState:
					needsDraw = TRUE;
					VA.color = 0;
					VALine(lp->x,lp->y,lp->x+lp->sx,lp->y+lp->sy);
					lp->timer--;
					if(lp->timer < 0)
					{	lp->state = startState;
						sweep = 0;
					}
					break;
			}
			lp++;
		}
		
		if (sweep==0)
		{	
			doneOnce=1;
		}
		VA.segmscale=5;
		VA.color=BG1;
		
		if(needsDraw)
			VAStep();
		else
			VACatchUp();
	 } while( (!Button()) && (!(doneOnce)) && (!(GamePeekEvent())) ); 
}