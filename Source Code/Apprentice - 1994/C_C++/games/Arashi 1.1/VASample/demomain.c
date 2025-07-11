/*/
     Project Arashi/STORM: demomain.c
     Major release: 9/9/92

     Last modification: Wednesday, September 9, 1992, 21:30
     Created: Saturday, September 29, 1990, 15:45

     Copyright � 1990-1992, Juri Munkki
/*/
#include "VA.h"
#define	N	20

void	main()
{
	EventRecord		Event;
	int				i;
	Point			vector[N];
	Point			speed[N];

	DoInits();
	randSeed=TickCount();
	
	VAInit(GetMainDevice());
	VA.FrameSpeed=2;

	for(i=0;i<N;i++)
	{	vector[i].h=((unsigned int)Random()) % VA.frame.right;
		vector[i].v=((unsigned int)Random()) % VA.frame.bottom;
		speed[i].h=1;
		speed[i].v=1;
	}	
	
	do
	{	VA.color=255-128;
		VA.segmscale=20;
		VADrawNumber(TickCount(),200,VA.segmscale+VA.segmscale+5);
	
		VA.color=3;
		for(i=0;i<N;i++)
		{	vector[i].h+=speed[i].h;
			if(vector[i].h<VA.frame.left || vector[i].h>=VA.frame.right)
			{	vector[i].h-=speed[i].h;
				speed[i].h=-speed[i].h;
				VAExplosion(vector[i].h,vector[i].v,3,2);
			}
			vector[i].v+=speed[i].v;
			if(vector[i].v<VA.frame.top || vector[i].v>=VA.frame.bottom)
			{	vector[i].v-=speed[i].v;
				speed[i].v=-speed[i].v;
				VAExplosion(vector[i].h,vector[i].v,3,2);
			}
			if((Random() & 255)==0)
			{	VAExplosion(vector[i].h,vector[i].v,2,3);
				speed[i].h+=speed[0].h;
				speed[i].v+=speed[0].v;
			}
			
			VASafeSpot(vector[i].h,vector[i].v);
		}
		VA.color=0;
		VAMoveTo(vector[N-1].h,vector[N-1].v);
		for(i=0;i<N;i++)
		{	VALineTo(vector[i].h,vector[i].v);
		}

		VAStep();
	} while(!Button());

	VAClose();
}