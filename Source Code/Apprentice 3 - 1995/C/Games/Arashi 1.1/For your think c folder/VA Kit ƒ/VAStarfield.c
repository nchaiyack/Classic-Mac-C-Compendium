/*/
     Project Arashi: VAStarfield.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Wednesday, February 6, 1991, 4:53

     Copyright � 1991-1992, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"

#define	NEWNUMBER	2
#define	MAXSTARS	75L
#define	STARSPEED	5

typedef struct
{	
	int		x,y,z;
} MegaStar;

MegaStar	*MegaStars;
int			StarsUsed;
Point		StarTwist;
Point		NewStarTwist;
Point		OldStarTwist;
int			Twister;

void	FeedStars()
{
	register	int			i;
	register	MegaStar	*starp;
	register	Point		area,doublearea;

	if(StarsUsed<MAXSTARS)
	{
		area.h=VA.frame.right>>1;
		area.v=VA.frame.bottom>>1;
	
		doublearea.h=VA.frame.right;
		doublearea.v=VA.frame.bottom;
	
		starp = MegaStars+StarsUsed;
		if(MAXSTARS-StarsUsed < NEWNUMBER)
		{	i=MAXSTARS-StarsUsed;
		}
		else
		{	i=NEWNUMBER;
		}
		StarsUsed += i;
		
		while(i--)
		{	starp->x = ((unsigned int)FastRandom()) % doublearea.h - area.h + StarTwist.h;
			starp->y = ((unsigned int)FastRandom()) % doublearea.v - area.v + StarTwist.v;
			starp->z = DEPTH+STARTDEPTH;
			starp++;
		}
	}
}

int		TwistTweeker[]={ 1,1,1,2,2,2,3,3,4,5,6,7,8,10,12,16,
						 20,22,24,25,26,27,28,29,29,30,30,30,31,31,31,32};

void	RideStars()
{
	register	int			i;
	register	MegaStar	*starp;
	register	int			x,y;
				Point		midpoint;
				Point		area,doublearea;
	
	if(StarTwist.h != NewStarTwist.h || StarTwist.v != NewStarTwist.v)
	{	int		TwistFactor;
		Twister++;
		TwistFactor=TwistTweeker[Twister];

		StarTwist.h = OldStarTwist.h + (((NewStarTwist.h - OldStarTwist.h) * Twister)>>5);
		StarTwist.v = OldStarTwist.v + (((NewStarTwist.v - OldStarTwist.v) * Twister)>>5);
	}
	else
	{	OldStarTwist = StarTwist;
		Twister = 0;
	}
	
	midpoint.h=(VA.frame.right>>1)+StarTwist.h;
	midpoint.v=(VA.frame.bottom>>1)+StarTwist.v;

	VA.color=0;
	starp=MegaStars;
	
	if(VA.Late)
	{	for(i=0;i<StarsUsed;i++)
		{	if(starp->z<=STARSPEED)
			{	*starp=MegaStars[--StarsUsed];
				starp--;
				i--;
			}
			else
			{	starp->z -= STARSPEED;
			}
			starp++;
		}
	}
	else
	{	for(i=0;i<StarsUsed;i++)
		{	x= (((starp->x-StarTwist.h) * STARTDEPTH) / starp->z) + midpoint.h;
			y= (((starp->y-StarTwist.v) * STARTDEPTH) / starp->z) + midpoint.v;
			
			if(x<=0 || x>=VA.frame.right || y<=0 || y>=VA.frame.bottom || starp->z<=STARSPEED)
			{	*starp=MegaStars[--StarsUsed];
				starp--;
				i--;
			}
			else
			{	
				if(starp->z<STARTDEPTH*2)
						VA.color=0;
				else	VA.color=(starp->z-STARTDEPTH)*7/(DEPTH-1);

				VASafeSpot(x,y);
				starp->z -= STARSPEED;
			}
			starp++;
		}
	}
}

void	CloseStars()
{
	StarsUsed=0;
	DisposPtr(MegaStars);
	MegaStars=0;
}
void	OpenStars(Colors)
Handle	Colors;
{
	register	ColorSpec	*colp;
	register	int			i;

	HandToHand(&Colors);

	StarTwist.h=0;
	StarTwist.v=0;
	colp = (ColorSpec *)*Colors;
	HLock(Colors);
	for(i=0;i<7;i++)
	{	colp[i].rgb.red		=colp[8].rgb.red   * (7L-i) / 7;
		colp[i].rgb.green	=colp[8].rgb.green * (7L-i) / 7;
		colp[i].rgb.blue	=colp[8].rgb.blue  * (7L-i) / 7;
	}
	HUnlock(Colors);
	VASetColors(Colors);
	DisposHandle(Colors);
	
	StarsUsed=0;
	MegaStars=(MegaStar *)NewPtr(MAXSTARS * sizeof(MegaStar));
}

#ifdef TEST_STARS
void	TestStars()
{
	Point	Mouser;

	OpenStars(GetResource('CLOT',1001));
	VAEraseBuffer();
	do
	{	FeedStars();
		RideStars();
		VAStep();
		GetMouse(&Mouser);
		VALine(0,0,Mouser.h,Mouser.v);
		if(Twister==0)
		{
			NewStarTwist.h=(Mouser.h-VA.frame.right/2);
			NewStarTwist.v=(Mouser.v-VA.frame.bottom/2);
		}	
		
	}	while(!Button());

	while(StarsUsed)
	{	RideStars();
		VAStep();
	}
	VASetColors(GetResource('CLOT',1000));
	CloseStars();
}
#endif