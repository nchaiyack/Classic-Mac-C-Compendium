/* Hamurabi.c (Macintosh version of the BASIC Game, Hamurabi)
*/

/*Includes */
#ifndef _MacTypes_
#include "MacTypes.h"
#endif
#include "Stdio.h"
#include "stdlib.h"
#include "Hamurabi.h"

#define AO 0 /* if set, use artificial opponent instead of player */

/* Global Variables */
int Year, /* position in the 10 year cycle */
	People, /* number of people in country */
	StoredGrain, /* Grain in storage */
	Impeached = 0, /* was player impeached for starving his people? */ 
	TotalDeath = 0, /* total death from all causes */
	AvgStarve = 0, /* avg number of people who starved per year */
	Land;/* land owned */
int quit = False,
	starved, /* number of people starved */
	newbies, /* number of new people */
	yield, /* bushels of grain per acre */
	lostgrain, /* bushels eaten by rats */
	landcost, /* cost of land */
	peoplefed, /* people fed */
	farmers, /* people needed to tend land */
	landtraded, /* amount of land traded */
	harvest, /* bushels harvested */
	feedgrain, /* amount of grain fed to the people */
	plantacre,
	plantgrain;/* amount of grain used for seed */

/* Support Routines */
void InitMac(void)
{
/*	InitGraf(&thePort);
	InitFonts();
	InitWindows();*/
	/*InitDialogs(-1);*/
/*	OpenPort(&MyPort);
	SetPort(&MyPort);

	InitCursor();
*/
}
void InitGame(void)
{
	asm
	{
		move.l	Ticks,-126(A5)
	}
	People = INITPEOPLE;
	Land = INITLAND;
	StoredGrain = INITSTORED;
	starved = INITSTARVED;
	newbies = INITNEWBIES;
	yield = INITYIELD;
	lostgrain = INITRATS;

}/* Initialize */
/* ============= */
int CalcStarved(int grain)
/* Calculate and return the number of people who will starve 
	given the number of people (a global) and the amount of grain.*/
{	
	int starved;
	
	if (starved == People - grain/GRAINperMAN>0)
		return starved;
	else
		return 0;
}
/* ============= */
int CalcHarvest(int seed, int yield, int farmers)
/* Calculate and return the amount of food havested given the 
	amount of land, the amount of seed and the number of people */
{
	int landseeded,landfarmed,Growing;

	landseeded = seed*ACREperGRAIN;
	landfarmed = farmers*ACREperFARMER;
/* now we need to find the smallest of land, landseeded, and landfarmed */
	Growing = min(min(landseeded,landfarmed),Land);
/* now we simply return the amount of grain */
	return yield * Growing;
}/* CalcHarvest */
/* ============= */
int CalcLostGrain(void)
/* determine amount of spoilage from all causes */
{
	int rnd;
	
	rnd = RandRng(RNGLOST);
	if ((rnd == 2) || (rnd == 4)) /* following original Ham*/
		return StoredGrain/rnd;
	else
		return 0;
}/* CalcLostGrain */
/* ============= */
int CalcLandPrice(void)
/* this routine calculates the price of land */
{
	return RandRng(RNGLANDPRICE)+ADDLANDPRICE;
}/* CalcLandPrice */
/* ============= */
int CalcYield(void)
/* this routine calculates the amount of grain that you get for one acre of land
 */
{
	return RandRng(RNGYIELD)+ADDYIELD;
}/* CalcYield */
/* ============= */
int CalcNewbies(void)
/* this routine calcs the amount of population growth */
{
long temp,temp1,temp2,Rand;

	Rand = RandRng(RNGNEWBIE)  ;
	temp1 = (long) Land * LANDATTRACT;
	temp2 = (long) StoredGrain * GRAINATTRACT;
	temp = ((temp1+temp2)*Rand)/People/100 +1;
	return temp;
}/* CalcNewbie */
/* ============= */
void RandomEvent(void)
/* this routine determines what, if any random events happen this 
	turn and adjusts the global values appropriately */
{
		lostgrain = CalcLostGrain();
		StoredGrain -= lostgrain;
		if (RandRng(100)<=PLAGUECHANCE)
		{
			printf("\nA horrible plague struck!! Half the people died!\n");
			People  /= 2;
		}
}
/* ============= */
int GetInt(char * prompt)
{
	char s[132];
	int result,done,status;
	done = False;
	while (!done) 
	{
		printf("%s ",prompt);
		gets(s);
		printf("\n");
		status = sscanf(s,"%d",&result);
		if ((status != 1) || (result < 0))
				printf("Please enter a valid positive integer.\n");
		else
			done = True;
	}/* while */
	return result;
}
/* ============= */

int GetFeedGrain(void)
/* Asks the user how much grain she wants to feed her people, 
	verifies that this is a legal value and returns with it. */
{
	int grain,done;
	
#if AO
grain = People *GRAINperMAN;
if (grain> StoredGrain) 
	grain = StoredGrain /2;
printf("Feeding people %i bushels (%i per man).\n",grain,grain/People);
#else

	done = False;
	while (!done)
	{
		printf("\nYou have %i people and %i bushels of grain.\n",
						People,StoredGrain);


		grain = GetInt("How many bushels do you wish to feed your people? ");
		printf("\n");
		if (grain>StoredGrain) 
			printf("You don't have enough grain to do that!");
		else
			done=True;
	}/* while */
#endif
	return grain;
}
/* ============= */
int GetPlantAcres(void)
/* Asks the user how many acres she wants to plant, 
	verifies that it's a valid number and returns with it. */
{
	int plantland,done;
	
/* The pertinent equations for harvest are:
	L = S * ACREperGRAIN (acres planted related to bushels of seed)
	L = P * ACREperFARMER; (acres planted related to population)

Where L is land planted, P is the population and S is the bushels of seed.
*/
#if AO
plantland = Land;
if (plantland > People * ACREperFARMER) 
	plantland = People * ACREperFARMER;
if (plantland > StoredGrain * ACREperGRAIN) 
	plantland = StoredGrain * ACREperGRAIN;
done=True;
printf("Planting %i acres.\n",plantland);
#else
	done = False;
	while (!done)
	{
		printf("\nYou have %i acres, %i bushels in storage and %i farmers.\n ",
						Land, StoredGrain,People);
		plantland =GetInt("How much land would you like to plant?");
		printf("\n");
		if (plantland > Land) /* not enough land */
			printf("You don't have enough land!\n");
		else if (plantland / ACREperGRAIN > StoredGrain) /* not enough grain */
			printf("That would take %i bushels and you don't have that much.\n",
							plantland/ACREperGRAIN);
		else if (plantland / ACREperFARMER  > People) /* not enough farmers */
			printf("That would take %i farmers and you don't have that many.\n",
						plantland/ACREperFARMER);
		else
			done = True; /* whew! a legal number! */
	}/* while */
#endif
	return plantland;
} /* GetPlantAcres */
/* ============= */
int GetTradeLand(int price)
/* Asks the user how much land she wants to trade, verifies that 
	this is a legal value and returns with it. A negaive return value 
	indicates the amount of land  sold. */
{
	int tradeland,done,result,surplus;
#if AO
surplus = StoredGrain - ((GRAINperMAN + ACREperFARMER/ACREperGRAIN)*People);
tradeland = surplus/price;
if (tradeland>=0) 
	printf("Bought %i acres.\n",tradeland);
else
	printf("Sold %i acres.\n",-tradeland);
#else
	done = False;
	while (!done)
	{
		printf("\nLand is selling for %i bushels per acre and you have %i ",
					price,StoredGrain);
		printf("bushels in storage.\n");
		tradeland = GetInt("How many acres do you wish to buy? ");


		printf("\n");
		if (tradeland == 0)
		{
				tradeland = GetInt("How many acres do you wish to sell? ");
				if (tradeland > Land)
					printf("You don't have that much land!\n");
				else
				{
					done = True;
					tradeland = -tradeland;
				}
		}/* not buying land */
		else if (tradeland > StoredGrain/price) 
			printf("You don't have enough grain for that!\n");
		else 
			done = True;

	}/* while */
#endif
	return tradeland;
} /* GetTradeLand() */
/* ============= */
void Report(void)
/* this routine tells the player her status at the beginning of 
	the turn.  This information includes:
Year, starved, newbies, People, Land, yield, StoredGrain, lostgrain */
{
	printf("\nHamurabi:  I beg to report to you, \n");
	printf("in year %i, %i people starved, %i came to the city,\n",
			Year,starved,newbies);
	printf("the population is now %i.\n",People);
	printf("The city now owns %i acres.\n",Land);
	printf("You harvested %i bushels per acre.\n",yield);
	printf("Rats ate %i bushels.\n",lostgrain);
	printf("You now have %i bushels in store.\n",StoredGrain);
}
/* ============= */
void Impeach(void)
/* */
{
	printf("\n\n");
	printf("You starved %i people in one year!!!\n",starved);
	printf("Due to this extreme mismanagement you have not only\n");
	printf("been impeached and thrown out of office but you have");
	printf("also been declared national FINK!!!");
	quit = True;
}
/* ============= */
int RandRng(int Range)
{
	#define MaxRandom 0X7FFF
	long Rand,Temp;
	int Return;
	Rand = (long)Random() & MaxRandom;
	Return = (int)((Rand*Range)/MaxRandom)+1; /* convert it to the range */
	if (Return>Range) Return = Range;
	return Return;
}	
/* ============= */
main(void)
{
int i,rnd,PerCap,TotScore;
long count [101];
long tot;
	/*InitMac();*/
	InitGame();

	printf("THE GAME OF HAMURABI \n");
	printf("Creative Computing   Morristown, New Jersey\n");
	printf("from Basic Computer Games by David Ahl\n");
	printf("Converted to 'C' by Dave Menconi\n");
	printf("\nTry your hand at running ancient Sumaria for a 10 year\n");
	printf("term.  Maybe you will get lucky and live in history forever.\n");
	printf("Then again, maybe not.\n\n");
	for (Year = 1; Year <= 10 && !quit; Year++)
	{
		Report();

		landcost = CalcLandPrice();
		landtraded = GetTradeLand(landcost);
		Land += landtraded;
		StoredGrain -= landtraded*landcost;

		feedgrain = GetFeedGrain();
		StoredGrain -= feedgrain;

		plantacre = GetPlantAcres();
		plantgrain = plantacre/ACREperGRAIN;
		StoredGrain -= plantgrain;
		
		starved = CalcStarved(feedgrain);
		if (starved>People*STARVELIMIT/100)
		{
			Impeach();
			break;
		}
		TotalDeath += starved;
		/* avg # people who starve per year as a percentage */
		AvgStarve = ((Year - 1) * AvgStarve + starved * 100/People)/Year;
		
		yield = CalcYield();
		harvest = CalcHarvest(plantgrain,yield,People);
		StoredGrain += harvest;
		RandomEvent();
		
		newbies = CalcNewbies();
		People = People - starved + newbies;
	}/* for */
	
	PerCap = Land/People;

	printf("\n\nIn your 10 year term of office %i percent\n",AvgStarve);
	printf("of the population starved per year on the averag. A total\n");
	printf("of %i people starved during your tenure.\n",TotalDeath);
	printf("You started with %i acres per person and ended with\n",
					INITLAND/INITPEOPLE);
	printf("%i acres per person.\n",PerCap);
	printf("\n\n");
	TotScore = (PerCap-AvgStarve);
	printf("Overall I would give you a %i.\n\n",TotScore);
	if (TotScore<-20)
		printf("You are a despicable despot! You killed off your people right and left!\n");
	else if (TotScore<0)
		printf("You did a poor job.  A change of career is in order!\n");
	else if (TotScore<10)
		printf("Not a bad score, altogether! Apply for president...\n");
	else if (TotScore>10)
	{
		printf("This is incredible! Your mother should be very proud!\n");
		printf("I recommend a career in hotel/motel management!\n");
	}
}/* main */
