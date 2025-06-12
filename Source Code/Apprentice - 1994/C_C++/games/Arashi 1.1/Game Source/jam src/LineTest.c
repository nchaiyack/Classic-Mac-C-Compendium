/*/
     Project Arashi: LineTest.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:44
     Created: Sunday, February 5, 1989, 18:05

     Copyright � 1989-1992, Juri Munkki
/*/

#include "Line256.h"
extern	Handle	Specs[];

void	LineTest()
{
	int		i;
	long	j;
	long	ticker;
	int		xr[1001];
	int		yr[1001];

	for(i=0;i<1001;i++)
	{	xr[i]=(Random()&16383+16384) % 640;
		yr[i]=(Random()&16383+16384) % 480;
	}
	
	SetEntries(252,2,*Specs[0]);

	offset=7;
	field=1;
	VAColor=0;

	for(i=0;i<479;i+=4)
	{	Bresenheim(0,0,i,479);
		Bresenheim(0,470,i,0);
		Bresenheim(0,0,640,i);
		Bresenheim(0,479,640,i);
	}

	SetEntries(252,2,*Specs[1]);
	offset=6;
	field=1;
	ticker=Ticks;
	for(j=0;j<1;j++)
	for(i=0;i<1000;i++)
	{	Bresenheim(xr[i],yr[i],xr[i+1],yr[i+1]);
	}

}