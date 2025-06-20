/*/
     Project Arashi: Explosion.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:40
     Created: Thursday, February 9, 1989, 22:35

     Copyright � 1989-1992, Juri Munkki
/*/

/*
**	Explosion generator
*/

#undef	_ERRORCHECK_
#define	_NOERRORCHECK_
#include <Math.h>
#include "VA.h"

#define	NUMFRAGMENTS 100

double	vx[NUMFRAGMENTS];
double	vy[NUMFRAGMENTS];
double	fx[NUMFRAGMENTS];
double	fy[NUMFRAGMENTS];

void	CreateNewExplosions()
{
	Handle	Explosion;
	int		*expl;
	double	ang;
	long	size;
	int		i,j,lvl;
	int		fragments;
	
	for(lvl=0;lvl<4;lvl++)
	{	Explosion=GetResource('GNRL',1000+lvl);
		HUnlock(Explosion);
		SetHandleSize(Explosion,65536L);
		HLock(Explosion);
		expl=*(int **)Explosion;
		
		fragments=(lvl+1)*10;
		for(i=0;i<fragments;i++)
		{	do
			{	ang=Random()/100.0;
				vx[i]=cos(ang)*(lvl+2);
				vy[i]=sin(ang)*(lvl+2);
				ang=Random()/100.0;
				vx[i]*=sin(ang);
				vy[i]*=sin(ang);
				fx[i]=3*vx[i];
				fy[i]=3*vy[i];
			} while(fabs(vx[i]*vx[i]+vy[i]*vy[i])<0.2);
		}
		for(j=fragments;j>(fragments/4);j--)
		{	*expl++=j;
			for(i=0;i<j;i++)
			{	VA.color=5;
				fx[i]+=vx[i];
				fy[i]+=vy[i];
				VASafePixel((int)fx[i]+320,(int)fy[i]+240);
				*expl++=fx[i];
				*expl++=fy[i];
				vy[i]*=0.97;
				vx[i]*=0.97;
			}
			while(Ticks % 15);
			VADoFrame();
		}
		*expl++=0;
		size=(Ptr)expl-*Explosion;
		HUnlock(Explosion);
		SetHandleSize(Explosion,size);
		ChangedResource(Explosion);
		WriteResource(Explosion);
		ReleaseResource(Explosion);
	}
}