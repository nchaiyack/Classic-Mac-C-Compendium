/*	mines.h
 *
 *		Various globals
 */

#ifndef __MINES__
#define __MINES__

#define NUMSCORE				6		/* Number score slots	*/

/****************************************************************/
/*																*/
/*							Globals								*/
/*																*/
/****************************************************************/

extern short	SizeX;					/* size bitmaps			*/
extern short	SizeY;
extern short	SizeFlag;				/* Easy mode			*/
extern short	Bombs;					/* # of bombs			*/
extern short	BombFlag;				/* Easy mode (again)	*/
extern short	BRatio;					/* Bomb ratio			*/

extern short	BStatus[40][40];		/* Bomb status			*/
extern short	BPosition[40][40];		/* Bomb info flags		*/
extern short	BState;					/* Game state			*/
extern short	BFirst;					/* First click?			*/
extern short	BCount;					/* User bomb count		*/

extern short	XLoc,YLoc;				/* X, Y window loc.		*/
extern long		LogTime;				/* What time is it?		*/
extern long		CurTime;				/* What time is it now?	*/

extern short	Cruse;					/* Cruse Control		*/

extern WindowPtr MineWindow;			/* Mine window display	*/

/*	ScoreRec
 *
 *		This is the structure of the high score record
 */

typedef struct ScoreRec {
	char		name[32];				/* Name buffer			*/
	long		time;					/* Time					*/
} ScoreRec;

/*	PrefFiles
 *
 *		These are how preferences are stored
 */

typedef struct PrefFiles {
	short		SizeX;					/* Size of X			*/
	short		SizeY;					/* Size of Y			*/
	short		SizeFlag;				/* Size Flag			*/
	short		BRatio;					/* Bomb ratio			*/
	short		BombFlag;				/* Bomb flag			*/
	
	short		XWindLoc;				/* X loc of window		*/
	short		YWindLoc;				/* Y loc of window		*/
	
	ScoreRec	Scores[NUMSCORE];		/* High scores			*/
	
	short		Cruse;					/* Cruse Control		*/
} PrefFiles;

extern ScoreRec	Scores[NUMSCORE];		/* High scores			*/

/****************************************************************/
/*																*/
/*							 Code								*/
/*																*/
/****************************************************************/


#endif
