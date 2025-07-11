/*/
     Project Arashi: STStruct.h
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, March 14, 1993, 16:28
     Created: Sunday, March 3, 1991, 7:22

     Copyright � 1991-1993, Juri Munkki
/*/

typedef struct
{
	int				points;
	int				count;
	unsigned int	prob;
	Fixed			speed;
	int				color;
}	AnyTanker;

enum	{	FlipTank, FuseTank, PulsTank, TANKTYPES	};

typedef	struct
{
	int				lvNumber;
	int				lvField;
	int				lvColor;
	long			lvBonus;
	int				lvNext;
	long			lvStBonus;
	int				lvType;

	int				plMaxMove;
	int				plSuperZaps;

	int				shSpeed;
	int				shPower;
	int				shColor[5];

	int				flPoints;
	int				flCount;
	unsigned int	flProb;
	Fixed			flSpeed;
	int				flRot;
	int				flColor;

	int				fuPoints;
	int				fuBullseye;
	int				fuCount;
	unsigned int	fuProb;
	int				fuWarpP;
	int				fuPlayerPlus;
	int				fuColor[5];

	int				puPoints;
	int				puCount;
	unsigned int	puProb;
	Fixed			puSpeed;
	int				puRot;
	int				puTime;
	int				puColor;
	int				puPulsDepth;

	AnyTanker		tk[3];
	
	int				spPoints;
	unsigned int	spProb;
	Fixed			spSpeed;
	int				spStart;
	int				spTop;
	int				spPlasma;
	Fixed			spPlSpeed;
	int				spPlPoints;
	int				spSpikePoints;
	int				spColor[3];	



	int		boredomCount;	/*	If activeCount<boredomCount:				*/
	long	boredProb;		/*		probIncrease=boredProb					*/
	int		endTimer;		/*	When timer==0, fly to next level.			*/
	int		ShowSpikesMsg;
	int		Doomsday;
	
	/*	The following variables are not directly from the resource data:	*/
	int		activeCount;	/*	Total number of aliens that are active.		*/
	int		totalCount;		/*	Total number of aliens left on that level.	*/
	int		starCount;		/*	Number of baby-stars needed on this level.	*/
	int		edgeCount;		/*	Fly thru when edgeCount == totalCount.		*/
	long	probIncrease;
}	LevelInfo;

GLOBAL	LevelInfo	ThisLevel;
