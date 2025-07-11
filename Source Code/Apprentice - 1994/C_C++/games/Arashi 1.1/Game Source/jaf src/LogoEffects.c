/*
**  Logo effects.		//Jaf � copyright 1991
*/

#include "VA.h"
#include "STORM.h"
#include "flash.h"

#define MAXLOGOPOINTS 40 			/* changed for ARASHI (mz) */

extern int stormxtable[];
extern int stormytable[];
extern int stormlinetable[][2];
extern int logopoints;
extern int logolines;

extern	int				MainStage;
extern	int				SpecialEvent;
extern	int				MaxScale;
extern	VAColorInfo		*Col;
extern	Handle			ColorHandle;

static	long			LogoFade;
static	int				LogoCounter;

static	int 			xtable[MAXLOGOPOINTS];
static	int 			ytable[MAXLOGOPOINTS];


void RemoveLogoY()
{
	int		i,scale;
	int		done;
	
	scale = MaxScale;
	done = 1;
	for(i=0;i<logopoints;i++) {
		if (ytable[i]<scale*4) {	/* 50 */
			ytable[i]+=1+(scale*4-ytable[i])/4;		/* 7 */
			done = 0;
			}
		if (ytable[i]>2+scale*4) {	/* 54 */
			ytable[i]-=1+(ytable[i]-scale*4-2)/4;
			done = 0;
			}
	}
	
	DrawStormLogo(50,50);
	DrawStormLogo(51,52);

	if(LogoCounter++ ==0)
	{	VA.field=1;
		VA.offset=0;
		VA.color= 1;
	
		DrawStormLogoStatic(scale,50,50);
		DrawStormLogoStatic(scale,50,52);
	}

	if(done) 
		SpecialEvent = LOGOREMOVEX;	
}

void RemoveLogoX()
{
	int		i,x1,x2,y;
	int		done,scale;
	
	done = 0;
	VA.color=4;
	scale = MaxScale;
		
	x1=50+xtable[0];
	x2=50+41*scale-xtable[0];
	xtable[0]+=scale*1.3;	/* 17 */
	if(x1>=x2) 
		done = 1;
	y = 50+scale*4;
	VALine(x1,y,x2,y); /* 102, 104 */
	VALine(x1,y+2,x2,y+2);
	if(done) {
		SpecialEvent = NOEVENT;
		PlayA(Blow,0);
		VAExplosion(x1,y,3,4);
		VAExplosion(x1,y+2,3,2);	
	}
}

void	DrawStormLogo(xoff,yoff)
int		xoff,yoff;
{
	int		i;
	int		a,b;
	int		xtab[MAXLOGOPOINTS];
	int		ytab[MAXLOGOPOINTS];

	VA.color=4;
	for(i=0;i<logopoints;i++)
	{	xtab[i]=xtable[i]+xoff;
		ytab[i]=ytable[i]+yoff;	
	}
	for(i=0;i<logolines;i++)
	{	a=stormlinetable[i][0];
		b=stormlinetable[i][1];
		VALine(xtab[a],ytab[a],xtab[b],ytab[b]);
	}
}

void	DrawStormLogoStatic(scale,xoff,yoff)
int		scale,xoff,yoff;
{
	int		i,j;
	int		a,b;
	int		xtab[MAXLOGOPOINTS];
	int		ytab[MAXLOGOPOINTS];

	for(i=0;i<logopoints;i++) {
		xtab[i]=stormxtable[i]*scale+xoff;   
		ytab[i]=stormytable[i]*scale+yoff;	
	}
	for(i=0;i<logolines;i++) {
		a=stormlinetable[i][0];
		b=stormlinetable[i][1];
		VAStaticLine(xtab[a],ytab[a],xtab[b],ytab[b]);
	}
}

void Logo()
{
	int i;
	
	for(i=0;i<logopoints;i++) {
		xtable[i]=stormxtable[i];
		ytable[i]=stormytable[i];
	}
	(*Col)[7].red=0;
	(*Col)[7].blue=0;
	(*Col)[7].green=0;
	VASetColors(ColorHandle);
	SpecialEvent = LOGOSTART;
}

void StartLogo()
{
	int		scale;
	
	scale = MaxScale;
	VA.field=1;
	VA.offset=0;
	LogoFade = 1024;

	DrawStormLogoStatic(scale,50,50);
	DrawStormLogoStatic(scale,50,52);
	SpecialEvent = LOGOLIVING;
}

void DrawLogo()
{	
	int		i,scale;
		
	LogoFade += 900;	/* 1024 */
	if (LogoFade<65536) {
		(*Col)[7].red=0;
		(*Col)[7].blue=LogoFade;
		(*Col)[7].green=LogoFade;
		VASetColors(ColorHandle);
	}
	else {	
		scale = MaxScale;
		SpecialEvent = LOGOREMOVEY;
		
		for(i=0;i<logopoints;i++) {
			xtable[i]=stormxtable[i]*scale;  
			ytable[i]=stormytable[i]*scale;	
		}

		DrawStormLogo(50,50);
		DrawStormLogo(51,52);
		
		LogoCounter=0;
	}
}

