/* This file takes care of the floating scores above Fuseballs 					*/
/* It basically implememts another set of 7 segment #'s, like VADrawNumber 		*/
/* but it is compatible with using on the pplaying field, VADrawNumber will 	*/
/* overdraw the playing field and leave holes when erased. (mz)					*/

/* December 22,1992 � Mike Zimmerman */


#include "VA.h"
#include "Storm.h"
#include <Math.h>

#define STARTAGE 		40
#define	MAXFLSCORES		20

extern	Player	Hero;

typedef struct
{
	int		AGE;
	int		score;
	int		x1;
	int		y1;
	int		scale;
}	FloatingScore;

int	FSCount;
int FSStart;
FloatingScore	*FLScores;


void	DrawScore(x,y,score,scale)
int	x;
int	y;
int	score;
int	scale;
{
	int		numlen;
	
	VA.color=BG1;						
	VA.segmscale=scale;
	VADrawNumber(score,x,y);
}

void UpdateFloatingScores()
{
	int	i,j;

	for(i = FSStart;i < ((FSCount+FSStart)%MAXFLSCORES) ; i++)
	{
		j=i%MAXFLSCORES;
		FLScores[j].AGE -= 1;
		if (FLScores[j].AGE == 0 || Hero.state == HeroFlying) 
		/* Score expired, so remove it */
		{
			VA.color=-1;						
			VA.segmscale=FLScores[j].scale;
			VADrawNumber(FLScores[j].score,FLScores[j].x1,FLScores[j].y1);
			FLScores[j].AGE = 0;
			FSCount--;
			FSStart++;
			VA.color=0;
			if (FSCount == 0)
				RedrawField();
		}
	}
}

void	AddFLScore(x,y,fscore)  /* mz */
int	x;
int	y;
int	fscore;
{
	FLScores[(FSCount+FSStart)%MAXFLSCORES].scale = (int)(Getfontscale());
	DrawScore(x,y,fscore,FLScores[(FSCount+FSStart)%MAXFLSCORES].scale);
	FLScores[(FSCount+FSStart)%MAXFLSCORES].AGE=STARTAGE;
	FLScores[(FSCount+FSStart)%MAXFLSCORES].score=fscore;
	FLScores[(FSCount+FSStart)%MAXFLSCORES].x1=x;
	FLScores[(FSCount+FSStart)%MAXFLSCORES].y1=y;
	FLScores[(FSCount+FSStart)%MAXFLSCORES].scale = (int)(Getfontscale());
	FSCount++;
}

void	AllocFloatingScores()
{
	FLScores=(FloatingScore *)NewPtr(sizeof(FloatingScore) * MAXFLSCORES);
	FSCount=0;
}

void	InitFloatingScores()
{
	register	int		i;
	
	for(i=0;i<MAXFLSCORES;i++)
	{	FLScores[i].AGE=0;
	}
	FSCount=0;
	FSStart=0;
}
