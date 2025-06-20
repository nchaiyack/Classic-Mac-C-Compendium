/*/
     Project Arashi: GridTest.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, March 14, 1993, 23:16
     Created: Thursday, February 9, 1989, 22:32

     Copyright � 1989-1993, Juri Munkki
/*/

#include <Math.h>
#include "VA.h"
#define	_MAKEGRID_
#include "STORM.h"
#include "heroflags.h"

#define PI 3.1415926535

#define	FIELDADVANCE	8
#define	FIELDSTART		(2*DEPTH)

extern	Player	Hero;

int		xcen;
int		ycen;

int		FieldDistance;

#define	DOSTARRIDE

extern	int		StarsUsed;
extern	Point	NewStarTwist;

void	FieldRide(
	worldstruct	*world,
	short		fieldAdvance)
{
	int		i;
	int		x[MAXSEGS],y[MAXSEGS];
	int		xb[MAXSEGS],yb[MAXSEGS];
	
	if(!VA.Late)
	{	VA.color=0;
		if(FieldDistance<STARTDEPTH*2)
				VA.color=0;
		else	VA.color=(FieldDistance-STARTDEPTH)*7/(FIELDSTART-STARTDEPTH-1);
		
		if(VA.color > 6) VA.color = 6;

		for(i=0;i<=ww.numsegs;i++)
		{	x[i]=((world->x[i]-world->xoff)*STARTDEPTH)
					/(long)(STARTDEPTH+FieldDistance)+xcen+world->xoff;
			y[i]=((world->y[i]-world->yoff)*STARTDEPTH)
					/(long)(STARTDEPTH+FieldDistance)+ycen+world->yoff;
			xb[i]=((world->x[i]-world->xoff)*STARTDEPTH)
					/(long)(STARTDEPTH+DEPTH+FieldDistance)+xcen+world->xoff;
			yb[i]=((world->y[i]-world->yoff)*STARTDEPTH)
					/(long)(STARTDEPTH+DEPTH+FieldDistance)+ycen+world->yoff;
			
			VAMoveTo(x[i],y[i]);
			VASafeLineTo(xb[i],yb[i]);
		}
		VAMoveTo(x[0],y[0]);
		for(i=1;i<=ww.numsegs;i++)
		{	VASafeLineTo(x[i],y[i]);
		}
	
		VAMoveTo(xb[0],yb[0]);
		for(i=1;i<=ww.numsegs;i++)
		{	VASafeLineTo(xb[i],yb[i]);
		}
	}
	
	FieldDistance += fieldAdvance;
	if(FieldDistance<0)	FieldDistance=0;
}
void	StarStep()
{
#ifdef	DOSTARRIDE
#ifdef BACKGROUND_FRIENDLY
		GameEvent();
#endif
		FeedStars();
		RideStars();
		VAStep();
#endif
}
void	StarsOut()
{
#ifdef DOSTARRIDE
	if(StarsUsed)		
	{	RideStars();
	}
#endif
}

void DoZapRecharge(onoff)
int onoff;
{
	/* This next section will add the message 			*/
	/* "Superzapper recharge" to the screen (mz)		*/
	
	unsigned char	*text=(void *)"\pSuperzapper Recharge";
	int				x,y;
	
	if(Hero.Flags & ShowSuperZapMsgMask)
	{
		VA.segmscale = (int)(Getfontscale() * 1.5);
		if(VA.segmscale <=1) 
			VA.segmscale=2;
		x = (int)( (VA.frame.right - 20*(VA.segmscale*3 + 3))/2 );
		y = (int)(VA.frame.bottom - 2);
		VAMoveTo(x,y);
		VA.color=BG1;
		if (!onoff)					/* Erase option */
			VA.color = -1;		
		VADrawText((char *)text,1,20);
	}
}

void	MakeSpots(world)
worldstruct	*world;
{
	register	int		d,i,j;

	ww.numsegs=world->numsegs;
	ww.wraps=world->wraps;

	for(d=ZOOMDEPTH;d<=DEPTH;d++)
	{	for(i=0;i<=ww.numsegs;i++)
		{	ww.x[i][d]=((world->x[i]-world->xoff)*(long)STARTDEPTH)
							/(STARTDEPTH+d)+xcen+world->xoff;
			ww.y[i][d]=((world->y[i]-world->yoff)*(long)STARTDEPTH)
							/(STARTDEPTH+d)+ycen+world->yoff;
		}
		ww.unitlen[d]=(world->unitlen * (long)STARTDEPTH) / (STARTDEPTH+d);
		if(!(d & 31))
			StarStep();
	}


	for(d=ZOOMDEPTH+1;d<DEPTH+1;d++)
	{	for(i=0;i<ww.numsegs;i++)
		{	ww.xc[i][d]=(ww.x[i][d]+ww.x[i+1][d])>>1;
			ww.yc[i][d]=(ww.y[i][d]+ww.y[i+1][d])>>1;
		}
		if(!(d & 15))
		{	StarsOut();
			if(StarsUsed)
			{	VAStep();
#ifdef BACKGROUND_FRIENDLY
				GameEvent();
#endif
			}
		}
	}

	FieldDistance=FIELDSTART; 

	/*	Precalculate paths for spinning stars at center of playfield:	*/
	ww.starsegs=ww.numsegs*STARDIVISION;
	for(d=0;d<2;d++)
	{	for(i=0;i<=ww.numsegs;i++)
		{	ww.starx[i*STARDIVISION][d]=((world->x[i]-world->xoff)*(long)STARTDEPTH)
							/(STARTDEPTH+DEPTH+(DEPTH*2)*d)+xcen+world->xoff;
			ww.stary[i*STARDIVISION][d]=((world->y[i]-world->yoff)*(long)STARTDEPTH)
							/(STARTDEPTH+DEPTH+(DEPTH*2)*d)+ycen+world->yoff;
		}
		for(i=0;i<ww.starsegs;i+=STARDIVISION)
		{	for(j=1;j<STARDIVISION;j++)
			{	ww.starx[i+j][d]=(ww.starx[i+STARDIVISION][d]-ww.starx[i][d])*j/STARDIVISION+
								  ww.starx[i][d];
				ww.stary[i+j][d]=(ww.stary[i+STARDIVISION][d]-ww.stary[i][d])*j/STARDIVISION+
								  ww.stary[i][d];
			}
			if(FieldDistance)
			{	FieldRide(world,-FIELDADVANCE);
				StarsOut();
#ifdef BACKGROUND_FRIENDLY
				GameEvent();
#endif
				VAStep();
			}
		}
	}

	if(FieldDistance)
	{	while(FieldDistance)
		{	FieldRide(world,-FIELDADVANCE);
#ifdef BACKGROUND_FRIENDLY
			GameEvent();
#endif
			VAStep();
		}
		
		VA.Late=0;
	}
	FieldRide(world,-FIELDADVANCE);
	VAStep();

	{	int		mode;
		
		mode=QD32COMPATIBLE;
		SwapMMUMode(&mode);
	
		VA.field=1;		VA.offset=1;		VA.color=0;
	
		for(i=0;i<ww.numsegs;i++)
		{	VABresenham(ww.x[i][0],ww.y[i][0],ww.x[i][DEPTH],ww.y[i][DEPTH]);
		}
		if(ww.wraps==0)
			VABresenham(ww.x[i][0],ww.y[i][0],ww.x[i][DEPTH],ww.y[i][DEPTH]);
		
		for(d=0;d<=DEPTH;d+=DEPTH)
		{	for(i=0;i<ww.numsegs;i++)
			{	VABresenham(ww.x[i][d],ww.y[i][d],ww.x[i+1][d],ww.y[i+1][d]);
			}
		}
		VA.field=3;
		SwapMMUMode(&mode);
	}
	VAStep();
	VASetColors(GetResource('CLOT',ThisLevel.lvColor));
	VACatchUp();

}

void	CreateFit()
{

	register	double	x,y,sx,sy;
	register	int		i,j,k;
	register	shape	*space;
				double	xmin,xmax,ymin,ymax,a;
	register	shape	**spacehand;

	double		AngularConstant= PI * 2.0 / ANGLES;


	if (ThisLevel.lvColor != 5)			/* Use alternate grey for invis levels (mz) */
		OpenStars(GetResource('CLOT',ThisLevel.lvColor));
	else
		OpenStars(GetResource('CLOT',ThisLevel.lvColor + 128));

	VA.segmscale=(VA.frame.bottom>>6);
	if(VA.segmscale<1) VA.segmscale=1;
	k=VA.color;
	VA.color=BG2;
	if (ThisLevel.lvColor == 5)		/* check for invis BG2 on ivis waves (mz) */
		VA.color=BG1;
	VADrawNumber(ThisLevel.lvNumber,VA.frame.right/2-VA.segmscale-3,VA.segmscale*2+4);
	VA.color=k;
		
	DoZapRecharge(1);				/* (mz) */

	spacehand=(void *)GetResource('SPCE',ThisLevel.lvField);
	HLock(spacehand);
	space=*spacehand;
	
	gridOutline.numsegs=space->numsegs;
	if(gridOutline.numsegs>=MAXSEGS-1) DebugStr("\PToo many shape segments error");
	xmin=xmax=ymin=ymax=x=y=0.0;
	a=0.0;
	
	for(i=0;i<space->numsegs;i++)
	{	double	TempAngle;
	
		a+=space->ang[i];
		TempAngle=a*AngularConstant;
		x+= cos(TempAngle);
		y-= sin(TempAngle);
		if(x<xmin)	xmin=x;
		else
		if(x>xmax)	xmax=x;
		if(y<ymin)	ymin=y;
		else
		if(y>ymax)	ymax=y;
		
		StarStep();
	}

#ifdef TWIST_CONTROL
	{	/*	Change margins to account for twist.	*/
	
		double	xshift,yshift,normalmargin;
		double	twist,midpoint;

		xshift = space->xoff/6.0;
		yshift = space->yoff/6.0;
		
		midpoint = (xmax+xmin)/2;
	
		twist = (xmax - xshift) * (long)STARTDEPTH / (DEPTH+STARTDEPTH) + xshift;
		if(twist > xmax)		xmax = twist;
	
		twist = (xmin - xshift) * (long)STARTDEPTH / (DEPTH+STARTDEPTH) + xshift;
		if(twist < xmin)		xmin = twist;
	
		midpoint = (ymax+ymin)/2;

		twist = (ymax - yshift) * (long)STARTDEPTH / (DEPTH+STARTDEPTH) + yshift;
		if(twist > ymax)		ymax = twist;
	
		twist = (ymin - yshift) * (long)STARTDEPTH / (DEPTH+STARTDEPTH) + yshift;
		if(twist < ymin)		ymin = twist;
	
		normalmargin = 1.2;
		xmin -= normalmargin;
		ymin -= normalmargin;
		xmax += normalmargin;
		ymax += normalmargin;
	}
#else
	{	double normalmargin;

		normalmargin = 1.2;
		xmin -= normalmargin;
		ymin -= normalmargin;
		xmax += normalmargin;
		ymax += normalmargin;
	}
#endif
	x = -(xmin+xmax)/2;
	y = -(ymin+ymax)/2;
	
	xcen=(VA.frame.right+VA.frame.left)/2;
	ycen=(VA.frame.bottom+VA.frame.top)/2;
	
	sx=(VA.frame.right-VA.frame.left)/(xmax-xmin);
	sy=(VA.frame.bottom-VA.frame.top)/(ymax-ymin);
	if(sy<sx)	sx=sy;

#ifdef TWIST_CONTROL
#define	TWIST_DIVIDE	8
#else
#define	TWIST_DIVIDE	32
#endif
	gridOutline.xoff=space->xoff*sx/TWIST_DIVIDE;
	gridOutline.yoff=space->yoff*sx/TWIST_DIVIDE;

	NewStarTwist.h=gridOutline.xoff;
	NewStarTwist.v=gridOutline.yoff;
	
	gridOutline.unitlen= sx;
	x= sx * x + 0.5;
	y= sx * y + 0.5;
	a=0.0;
	for(i=0;i<space->numsegs;i++)
	{	double	TempAngle;
	
		a+=space->ang[i];
		TempAngle=a*AngularConstant;

		gridOutline.x[i]=x;
		gridOutline.y[i]=y;
		
		x+= cos(TempAngle)*sx;
		y-= sin(TempAngle)*sx;
		
		StarStep();
	}
		
	gridOutline.x[i]=x;
	gridOutline.y[i]=y;
	gridOutline.wraps=space->wraps;
	if(space->wraps)
	{	gridOutline.x[i]=gridOutline.x[0];
		gridOutline.y[i]=gridOutline.y[0];
	}

	j=0;
	for(i=0;i<space->numsegs;i++)
	{	j+=space->ang[i];
		NextSeg[i]=(j+4*ANGLES) % ANGLES;
	}
	NextSeg[i]=NextSeg[0];

	for(i=0;i<=space->numsegs;i++)
	{	PrevSeg[i]=(ANGLES/2+NextSeg[(i-1+space->numsegs) % space->numsegs]) % ANGLES;
	}
	
	for(i=0;i<=space->numsegs;i++)
	{	k=(NextSeg[i]+PrevSeg[i])/2;
		if(NextSeg[i]<k) k+=ANGLES/2;

		for(j=k;j<k+ANGLES/2;j++)
			LaneSel[i][j % ANGLES]=i;
		for(j=k+ANGLES/2;j<k+ANGLES;j++)
			LaneSel[i][j % ANGLES]=i-1;
			
		if(!(i & 7)) StarStep();
	}
	
	
	for(j=0;j<ANGLES;j++)
		if(LaneSel[0][j]==-1)
			LaneSel[0][j]=space->numsegs-1;

	StarStep();

	DoZapRecharge(0);				/* (mz) */
	
	for(j=k;j<k+ANGLES/2;j++)
		LaneSel[space->numsegs][j % ANGLES]=0;

	StarStep();

	if(space->wraps==0)
 	{	PrevSeg[0]=-999;
 		NextSeg[space->numsegs]=-999;
 		for(j=0;j<ANGLES;j++)
 		{	LaneSel[0][j]=0;
 			LaneSel[space->numsegs][j]=space->numsegs-1;
 		}
 	}

	StarStep();
	HUnlock(spacehand);
	MakeSpots(&gridOutline);

	ReleaseResource(spacehand);
	CloseStars();
}

void	AllocGrids()
{
	register	int		*ip;
	register	char	*cp;
	register	int		i;
	
	ip=(int *)NewPtr(			(DEPTH-ZOOMDEPTH+1L)*sizeof(int)+
						MAXSEGS*(DEPTH-ZOOMDEPTH+1L)*sizeof(int)*4+
								 (MAXSEGS+1L)*ANGLES*sizeof(char));
	for(i=0;i<MAXSEGS;i++)
	{	ww.x[i]=ip-ZOOMDEPTH;		ip+=DEPTH-ZOOMDEPTH+1;
		ww.y[i]=ip-ZOOMDEPTH;		ip+=DEPTH-ZOOMDEPTH+1;
		ww.xc[i]=ip-ZOOMDEPTH;		ip+=DEPTH-ZOOMDEPTH+1;
		ww.yc[i]=ip-ZOOMDEPTH;		ip+=DEPTH-ZOOMDEPTH+1;
	}
	ww.unitlen=ip-ZOOMDEPTH;
	ip+=DEPTH-ZOOMDEPTH+1;

	cp=(char *)ip;
	for(i=0;i<=MAXSEGS;i++)
	{	LaneSel[i]=cp;	cp+=ANGLES;
	}
}