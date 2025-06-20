/* This routine sums the scores fro the LEVL rsrc included in the ARS.rsrc file */
/* Simply put, there is only a probability of how many points a player might score */
/* How many Spikers, and Plasma shots will be scored by the player is perhaps */
/* the largest variable.  Also, the player does not receive points for enemies */
/* onEdge at Flythru.  Further at higher levels, the player may tend to one spot */
/* and will not hit more Spikers even if more are appearing.  This is all only */
/* a guess, legitimized by scores from sample games. */

/* This routine will also store the new lvStBonus scores in the LEVEL resource which */
/* can then be pasted into a new ARASHI file. */

/* December 16, 1992  Mike Zimmerman */

#include "STORM.h"
#include <stdio.h>

#define probOfHittingSpkr 0.40
#define	predNumber	20

main()
{
	LevelInfo	**LInfo;
	int			i;
	long		int levelbonus=0;
	long		int totalbonus=0;
	FILE		*fp;
	int			predNumSpkrs=0;
	long		int	oldBonus=0;
	
	/* fp=fopen("scores","w"); */
	
	for(i=1;i <= 96 ;i++)
	{	LInfo=(void *)GetResource('LEVL',(127+i));
	HNoPurge(LInfo);
	/* fprintf(fp,"LEVEL %d \n",i); */
#ifdef BONUSGEN
	predNumSpkrs=0;
	if ((*LInfo)->spProb)
		predNumSpkrs = (int)((i * .45));
	if (predNumSpkrs > 12)
		predNumSpkrs = 12 + (int)(i * .08);
	/* fprintf(fp,"spProb was %d \n",(*LInfo)->spProb);
	fprintf(fp,"Predicted number of Spikers %d \n",predNumSpkrs); */
	levelbonus =  (long)(*LInfo)->flPoints*(*LInfo)->flCount + 
			(long)(*LInfo)->puPoints*(*LInfo)->puCount + 
			(long)(*LInfo)->fuBullseye*(*LInfo)->fuCount + 
			(long)(300 + ((*LInfo)->tk[0].points) )*(*LInfo)->tk[0].count + 
			(long)(1000 + ((*LInfo)->tk[1].points) )*(*LInfo)->tk[1].count + 
			(long)(400 + ((*LInfo)->tk[2].points) )*(*LInfo)->tk[2].count +
			(long)(*LInfo)->lvBonus +
			(long)( (*LInfo)->spPoints * predNumSpkrs) +
			(long)( (*LInfo)->spPlPoints * predNumSpkrs * (*LInfo)->spPlasma * .35);
	
	/* fprintf(fp,"%d Flippers at %d points each = %ld \n",(*LInfo)->flCount,
		(*LInfo)->flPoints,	(long)(*LInfo)->flPoints*(*LInfo)->flCount);
	fprintf(fp,"%d Pulsars at %d points each = %ld \n",(*LInfo)->puCount,
		(*LInfo)->puPoints,(long)(*LInfo)->puPoints*(*LInfo)->puCount);
	fprintf(fp,"%d Fireballs at %d points each = %ld \n",(*LInfo)->fuCount,
		(*LInfo)->fuBullseye,(long)(*LInfo)->fuBullseye*(*LInfo)->fuCount);
	fprintf(fp,"%d fltankers at %d points each = %ld \n",(*LInfo)->tk[0].count,
		(*LInfo)->tk[0].points,(long)(300+(*LInfo)->tk[0].points)*(*LInfo)->tk[0].count);
	fprintf(fp,"%d putankers at %d points each = %ld \n",(*LInfo)->tk[1].count,
		(*LInfo)->tk[1].points,(long)(400+(*LInfo)->tk[1].points)*(*LInfo)->tk[1].count);
	fprintf(fp,"%d futankers at %d points each = %ld \n",(*LInfo)->tk[2].count,
		(*LInfo)->tk[2].points,(long)(400+(*LInfo)->tk[2].points)*(*LInfo)->tk[2].count);
	fprintf(fp,"Predicted point for Spikers = %ld \n",
		(long)( (*LInfo)->spPoints * predNumSpkrs));
	fprintf(fp,"Predicted points for Plasma = %ld \n",
		(long)( (*LInfo)->spPlPoints * predNumSpkrs * (*LInfo)->spPlasma * .35));
	fprintf(fp,"So the total for level %d is %ld \n",i,levelbonus); */
	
	(*LInfo)->lvStBonus = oldBonus;
#endif
	(*LInfo)->shPower = 5;
	ChangedResource(LInfo);
	WriteResource(LInfo);
	HPurge(LInfo);
#ifdef BONUSGEN	
	ReleaseResource(LInfo);
	totalbonus += levelbonus;
	oldBonus = (long)(1.10*totalbonus);
	oldBonus = oldBonus/10;
	oldBonus = oldBonus*10;

	/* fprintf(fp,"Total going into level %d is %ld \n", i+1, totalbonus); */
	/*	fprintf(fp,"So the starting bonus for level %d is %ld \n",(i+1),(long)(totalbonus*.95));
	fprintf(fp,"=========================== \n");
#endif
	}
#ifdef BONUSGEN
	fclose(fp); */
#endif
	}
}

