/*/
     Project Arashi: STORM.h
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, February 14, 1993, 10:54
     Created: Sunday, February 12, 1989, 23:17

     Copyright � 1989-1993, Juri Munkki
/*/

#ifdef	_MAKEGRID_
#define	GLOBAL
#else
#define	GLOBAL		extern
#endif

#include "STStruct.h"

#define	MAXSHOTS	9

#define MAXSEGS 	32
#define DEPTH		120
#define STARTDEPTH	20
#define	ZOOMDEPTH	-19

#define	STARDIVISION	4
#define	MAXCENTERSTARS	10L

typedef	union
{
	long	f;
	int		i;
}	IFixed;


typedef struct
{
	int		lane;
	int		level;

}	Shot,*ShotPtr;

typedef	struct
{
	int		state;		/*	State that the player is in.		*/
	int		timer;		/*	State timer.						*/
	int		endtimer;	/*	Time after complete/before fly thru	*/

	/*	Position information:									*/
	int		rawpos;		/*	"raw" player position"				*/
	int		lane;		/*	Current lane of player.				*/
	int		oldlane;	/*	Old position of player.				*/
	int		shifter;	/*	Visual twist of player on lane.		*/
	int		flydepth;	/*	Position during flythru.			*/
	int		dropdepth;	/*	Position when dropping dead.		*/

	/*	Fighting information:									*/
	int		pendingshot;/*	Is there a buffered mouse press?	*/
	int		couldshoot;	/*	Was there a shot fired?				*/
	int		superzapping;/*	1->superzap, 2->single enemy zap.	*/
	int		changedlane;/*	Was there a lane change?			*/
	int		lanestat[MAXSEGS];	/*	Flags for enemy presence.	*/
	int		segmstat[MAXSEGS+1];/*	Flags for enemy presence.	*/
	int		lanemissing[MAXSEGS];/*	Flags for field display.	*/
	
	/*	Game information:										*/
	int		lives;		/*	How many lives left?				*/
	long	score;		/*	Amount of points player has.		*/
	int		drawscore;	/*	True, if score needs to be drawn.	*/
	long	freeLives;	/*  Score needed for next free life		(mz) 	*/
	unsigned	int		Flags; 	/* set of flags checked by bit position.*/
								/* see heroflags.h for details (mz)		*/
}	Player;

enum	{	HeroPlaying, HeroFlying, HeroDropping, HeroCracking,
			HeroDisintegrating, HeroPostMortem, HeroDead, HeroQuit	};

typedef	struct
{	int		numsegs;		/*	Number of segments on this grid.	*/
	int		wraps;			/*	Does this grid wrap around?			*/
	int		*unitlen;		/*	Pointer to table of segment widths.	*/
	int		*x[MAXSEGS];	/*	Tables of segment edge x coords.	*/
	int		*y[MAXSEGS];	/*	Tables of segment edge y coords.	*/
	int		*xc[MAXSEGS];	/*	Tables of segment center x coords.	*/
	int		*yc[MAXSEGS];	/*	Tables of segment center y coords.	*/
	int		starsegs;		/*	Number of divisions for stars.		*/
	int		starx[STARDIVISION*MAXSEGS][2];	/*	center star x stuff	*/
	int		stary[STARDIVISION*MAXSEGS][2];	/*	center star y stuff	*/
}	GridWorld;

typedef	struct
{	int		numsegs;
	int		wraps;
	int		xoff;
	int		yoff;
	int		unitlen;
	int		x[MAXSEGS];
	int		y[MAXSEGS];
}	worldstruct;

typedef struct
{
	int		numsegs;
	char	wraps;
	char	seglen;
	char	xoff;
	char	yoff;
	int		filler;
	int		ang[MAXSEGS];
}	shape;

/*	Masks for different types of monsters for stat array.			*/
enum
{	FlipMask=1,
	PlasMask=2,
	FuseMask=4,
	PulsMask=8,
	SpikeMask=16
};

/*	Enums for sounds. Note that sound files are in alphabetical order!	*/
enum	{	Blast,		/*	Title lightning		*/
			Blow,		/*	Exploding fuseball	*/
			Bonk,		/*	Player movement		*/
			Dziung,		/*	Spiker explode		*/
			PhazerIn,	/*	Player shots		*/
			PhazerOut,	/*	*/
			FallYell,	/*	Player falling.		*/
			FallZap,	/*	Player capture		*/
			FlyThru,	/*	Level warp			*/
			Phazer2,	/*	Flipping pulsar		*/
			Splitter,	/*	Exploding pulsar	*/
			Springy,	/*	*/
			Swish,		/*	Player hit			*/
			Swoosh,		/*	Exploding flipper	*/
			Whiz,		/*	Flying plasma		*/
			Zroom,		/*	Splitting tanker	*/
			ZZFreeLife,	/* 	Chime for free life	(mz) */
			ZZSuperZap	/* for superzap (mz) */
			
		};

enum	{	NormalPlay, RecordPlay, PlaybackPlay	};

GLOBAL	worldstruct	gridOutline;
GLOBAL	GridWorld	ww;
GLOBAL	int			NextSeg[MAXSEGS+1];
GLOBAL	int			PrevSeg[MAXSEGS+1];
GLOBAL	char		*LaneSel[MAXSEGS+1];

extern	int		VAIsLate;
void	StarApproach(int *notify,int color);
void	IncreaseScore(long);

