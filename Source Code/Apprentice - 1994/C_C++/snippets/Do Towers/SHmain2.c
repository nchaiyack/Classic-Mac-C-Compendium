#include "stdio.h"

/*#define RANGECK*/

int runLength(int);
long make12(int);
void dumpMoves(void);
void stackOne(int,int,int,int);
int countEmptyTowers(void);
Ptr newPtr(int);
int doStates(void);
int destChk(int,int);
int doAces(void);
void BuildMyMenu(void);
void doCards(void);

/* SHmain.c
	 This program parses the "indexor" TOWR resource left by the Seahaven Towers
	program and attempts to produce a solution.  The current state of the Towers
	game must currently be written to the TOWR resource by quitting the game at
	whatever state you desire the solution to start from.  This program will run 
	fairly quietly in the background, calling getnextevent every 50 moves or so.
	When in the forground, the menu can be used to display certain info about the
	progress, by typing "M". [all letters need to be upper case][The menu bar is normally blank]
	 In addition, the game will snapshot its current move list into the log file 
	[SH.log11] upon receipt of an "L", followed by continuous logging
	until the next "L".  It is strongly advised that L's be entered in pairs to avoid
	giant log files.  This program will terminate upon receipt of a "Q", or by
	successful solution [which will be recorded in the log file] or by exhausting the
	possible move tree. In addition, the program will beep once on failure and beep 4 times
	if successful solution is found. This is to allow the program to run without cluttering
	up the display.  I normally ignore the log file and just use Do.Towers to see if the game
	is solvable.
	  
	 A note about memory.  Do.Towers is set to a 600k partition as released.  It will use whatever
	you give it.  The memory is used to store "wrong" paths to speed solution time. Given
	enough memory, Do.Towers will solve most games in a few seconds, and use only a few hundred K.
	the 600K partition will solve 95% of the games without running out of memory.  If it runs
	out, the process will slow way down because we no longer will know all "wrong paths".
	Once in a while, I run into a game that needs 1000k or even 2000k to produce an answer
	in a reasonable time.
	
	 A note about the status "M" command.
	Typing "M" will paint a menu bar with the following info:
	X:L..H, MaxChain:A, Held:B, skpMoves:C, Heap=D
	...where
	  X= total number of moves so far
	  L= Lowest move depth since the last M command
	  H= Highest move depth since the last M command
	  A= Longest search path to find "wrong pathways"
	  B= Number of "wrong pathways" held in storage
	  C= Number of skipped moves due to being on a doomed path
	  D= Heap space to store more "wrong pathways"
	  
	The last one "D" is the most interesting.  When it gets below 12 the program will no longer
	maintain every doomed path, and will slow way down.  In this mode it could take weeks to
	solve very tough games, so you might as well give up with "Q", add more memory and try again.
	
	 A note about shareware.
	Be SURE you send Art Cabral his $20.00 for his very fine game.  Do.Towers is FREE.  However,
	if you would like the source code, send me $25.00 and a disk and I will send it to you.
	Do.Towers is a Lightspeed C 3.0 program.
	
	Chuck Pliske
	Seawell Microsystems
	Seattle, WA 98116
	(206) 938-5420

	see BuildMyMenu() for the menu display data.
		
	The format of the "TOWR" resource file follows:
	
	 (0,2)   (0,2)     (0,1)(1,1)(2,1)(3,1)     (0,2)   (0,2)
	
	
	(0,3) (1,3) (2,3) (3,3) (4,3) (5,3) (6,3) (7,3) (8,3) (9,3)	
	(0,4) (1,4) (2,4) (3,4) (4,4) (5,4) (6,4) (7,4) (8,4) (9,4)	
	(0,5) (1,5) (2,5) (3,5) (4,5) (5,5) (6,5) (7,5) (8,5) (9,5)	
	(0,6) (1,6) (2,6) (3,6) (4,6) (5,6) (6,6) (7,6) (8,6) (9,6)	
	(0,7) (1,7) (2,7) (3,7) (4,7) (5,7) (6,7) (7,7) (8,7) (9,7)	
	...etc
	
	starting at byte 0x40 in the resource, the above map of short
	int pairs is produced for the four suits in order, i.e. Spades,
	Hearts, Diamonds, Clubs, 13 int pairs per suit.  Thus a completed
	game has 52 sets of short ints:(0,2), and an AceSpade as the uppermost
	card of leftmost lower stack has the value (0,3) as bytes 40,42 in 
	the TOWR resource.
	
	for our internal format, we will use a struct of 17 byte pairs plus an index int
	(18 is the deepest theoretical stack = 5 original cards plus 12 if bottom card is
	a king)
	
	
	Revision History:
		source file saved as shMark.c to indicate the "standard" program which is believed
		to work, but is slow.  Modifications to that file to improve speed are discussed below.
		
		3/25/89: added state detector to save board state after every move.  One line of code
		at label toff: detects a move state matching an earlier state and assumes that this
		will be another blind alley and backs up a move.  State maintenance is done with two
		lines of code at end of the "stackOne" procedure. A chain of states is maintained for
		each move level.  The length of each chain is constrained by a constant [search on
		"ChainLen"].  Good performance in reasonable size [<300K] is acheived with chain
		lengths of 50-100.  Longer chains result in slow moves due to # of comparisons.
		
		5/31/89: changed state maintenance to keep states in an array indexed by number
		of aces covered [0..51] and to search all states in the appropriate element of the array.
		because before the states were saved by move level some duplicate states were not found.
		Now move level is ignored, and performance is much better, although it can still take a
		few hours on a macII to produce tough solutions.  The best all-around performance
		seems to be with large chains, now. Num_moves has been shortened to improve multifinder
		performance.
		
		The major problem affecting solution speed is still the "double run" problem, e.g.
		2H,9S,8S,7S,6S,5S,4S on one pile, 2S,9H,8H,7H,6H,5H,4H on another pile.  Configurations
		like this can't possibly be solved, yet I have not figured out how to detect these
		in the general case quickly.
		
		This version of the program has been saved as doTowers.2 and logs to sh.log2.
	1990 changes:
		A minor bug has been discovered in the unmove routine.  There was a variable,
		ÒmdestÓ that was initialized to -1 and stuffed with the dest during a multiple
		move.  Problem was the initialization was inside the multi loop, so it was
		moved out to test.  This version logs to sh.log11.
	
	1991 changes:
		For the think C 5.0 upgrade, several changes were made (as usual with Think)
		1: in doStates, a missing return statement caused bad results in 5.0
		2: in SetState, a dual use of "tmp" depended upon left to right evaluation.
		   since this apparently doesn't happen in 5.0, a new var was introduced "tmp2"
		3: in make12, the var K was occasionally 0, resulting in invalid card access.
		(the net effect of these fixes was to lengthen the runtime about 10%, with
		 probably fewer missed deadEnds, and hopefully no invalid deadEnds)
		
	Known problems: If there is a choice of kings to move to empty pile, only the first
	choice will be used.
*/

#define CHAINLIMIT 32000 /* Max length of chain at any move level; 100 is reasonable */
#define SHLOG "sh.log12" /* name of log file */
#define NUM_MOVES ((MaxChain<50)?200:(MaxChain<100)?100:(MaxChain<150)?50:20);
				 /* make this many moves before calling getnextEvent */ /*heh heh*/

typedef struct {
	Byte rank,suit; 
	int toTowCnt; /* for tracking when moved to tower */
	} card;
	
typedef struct pile {
	int	cnt;
	card	cards[17];
	}pile;

typedef struct move {
	Byte source,dest,type,multi;
	card	oneCard;
	long	movesct;
	} move;
	
struct moveList {
	int 	moveCnt;
	move	moves[1000];
	}moveList ={0};
	
typedef struct state{
	long	st1,st2,st3,st4; /* 8 bits per row */
	Ptr		next;	/* and the chain link */
	} state;

void setState(register state *);
int EQstate(register state *,register state *);
card moveOne(int,int,int);

int unmoveCnt;
state mySt; /* here an attempt to detect "stuck" configs */
int deadEnd =0; /* notes dead end */
long SScnt = 0l; /* counts saved states */
int ChainLen; /* notes length of one chain */
int MaxChain = 0; /*notes max chain length */
long skipped = 0l; /* master count of skipped moves */

#define spades 0
#define hearts 1
#define diamonds 2
#define clubs 3

#define pile_to_tower	0
#define tower_to_pile	1
#define pile_to_pile	2
#define tower_to_ace	3
#define ace_to_tower	4
#define pile_to_ace		5
#define ace_to_pile		6

char	msg[40] = "I can solve this";

pile	piles[10],aces[4],towers[4];

int	unMoveMany = 0; /* flag for backing up to last pile-to-tower move */

long	movesct = 0;
int	quit = 0;
int	log = 0;
int	newLog =0;

state	*stp[4096]/*,*curStp moved into proc */; /* try this one */
int	cksum;
Ptr		myHeap; /* locally allocate my heap */
long	myHeapSize;

long make12(int i)
	/*int i;*/
{
	register int j,k;
	register card *l;
	j = runLength(i);

	k = piles[i].cnt - j; /* is 0 if 0 cds or 1 single or just a run */
	j = (12 * k) + j;     /* is 0 if 0 cds or 1 for singles or rl for just runs */
	l = &piles[i].cards[k?(k-1):0]; /* if k is 0 just use top cd of run */
	i = j<<6|l->rank<<2|l->suit;
	cksum ^= i;
	return i;
}

/* attempt to detect a "stuck" configuration */
void setState(register state *stateP)
/*register state	*stateP;*/
{
	register long tmp,tmp2;
	
	cksum = 0; /* use eor of all 12bit values to address 4k array of chains */
	
	/* pack 10 12 bit quantities into 3 and 3/4 longwords */
	stateP->st1 = make12(0)<<20 | make12(1)<<8 | (((tmp = make12(2))>>4)&0xFF);
	stateP->st2 = (tmp & 0xF)<<28 | make12(3)<<16 | make12(4)<<4
					| (((tmp2 = make12(5))>>8)&0xF);
	stateP->st3 = (tmp2 & 0xFF)<<24 | make12(6)<<12 | make12(7);
	stateP->st4 = make12(8)<<20 | make12(9)<<8; /* whew */
}

int EQstate(s1,s2)
register state *s1,*s2;
{	
	if(s1->st1 != s2->st1) return 0;
	if(s1->st2 != s2->st2) return 0;
	if(s1->st3 != s2->st3) return 0;
	if(s1->st4 != s2->st4) return 0;
	return 1;
}

card moveOne(int class,int source,int dest)
/*int	class,source,dest;*/
{
	card	tmp;
	movesct++;
	switch (class) {
		case	pile_to_tower:
#ifdef RANGECK
			if(source>9 || dest>3) {
				SysBeep(1);
				return;
			}
#endif
			/* moveList.moves[moveList.moveCnt++].oneCard = */
			return towers[dest].cards[(towers[dest].cnt)++] = 
				piles[source].cards[--(piles[source].cnt)];
					
		case	tower_to_pile:
#ifdef RANGECK
			if(source>3 || dest>9) {
				SysBeep(1);
				return;
			}
#endif
			return piles[dest].cards[(piles[dest].cnt)++] = 
				towers[source].cards[--(towers[source].cnt)];
		
		case	pile_to_pile:
#ifdef RANGECK
			if(source>9 || dest>9) {
				SysBeep(1);
				return;
			}
#endif
			return piles[dest].cards[(piles[dest].cnt)++] = 
				piles[source].cards[--(piles[source].cnt)];

		case	pile_to_ace:
#ifdef RANGECK
			if(source>9 || dest>3) {
				SysBeep(1);
				return;
			}
#endif
			return aces[dest].cards[(aces[dest].cnt)++] = 
				piles[source].cards[--(piles[source].cnt)];

		case	tower_to_ace:
#ifdef RANGECK
			if(source>3 || dest>3) {
				SysBeep(1);
				return;
			}
#endif
			return aces[dest].cards[(aces[dest].cnt)++] = 
				towers[source].cards[--(towers[source].cnt)];

		case	ace_to_pile:
#ifdef RANGECK
			if(source>3 || dest>9) {
				SysBeep(1);
				return;
			}
#endif
			return piles[dest].cards[(piles[dest].cnt)++] =
				aces[source].cards[--(aces[source].cnt)];
		
		case	ace_to_tower:
#ifdef RANGECK
			if(source>3 || dest>3) {
				SysBeep(1);
				return;
			}
#endif
			return towers[dest].cards[(towers[dest].cnt)++] =
				aces[source].cards[--(aces[source].cnt)];
				
		default:	SysBeep(60);
		
		}
}

void dumpMoves()
{
	int i,mflag=0;
	for (i=0;i<moveList.moveCnt;i++) 
	  /* multiple moves are characterized by their pile_to_pile portion */
	  /* the first move has multi flag=0, then =1 for all the rest.  for*/
	  /* now we just print out all the moves */
		switch (moveList.moves[i].type) {
			case pile_to_pile:
				printf("\nmove %d to %d [%ld]",moveList.moves[i].source+1,moveList.moves[i].dest+1,moveList.moves[i].movesct);
				break;
			case pile_to_tower:
				printf("\nmove %d to tower[%d] [%ld]",moveList.moves[i].source+1,moveList.moves[i].dest+1,moveList.moves[i].movesct);
				break;
			case tower_to_pile:
				printf("\nmove tower[%d] to %d [%ld]",moveList.moves[i].source+1,moveList.moves[i].dest+1,moveList.moves[i].movesct);
				break;
		}
}

int usource = -1, udest = -1,utype = -1, umulti = 0; /* global unmove flags */
int mLow = 1000, mHigh = 0; /* keeps track of move range for menu display */

void stackOne(int class,int source,int dest,int multi)
/*int	class,source,dest,multi; */
{
				int	cnt= moveList.moveCnt;
	register	move	*m = &moveList.moves[cnt];
	
	/* keep track of low and hi since last menu report */
	if(cnt<mLow) mLow = cnt;
	if(cnt>mHigh)mHigh= cnt;
#ifdef RANGECK
	if((source>9) || (dest>9)) {
		SysBeep(60);
		return;
	}
#endif
	if(log) {
		if(!newLog) {
			newLog = 1;
			dumpMoves();
		}
		printf("\nmove %d to %d [%d] (%d)",source+1,dest+1,class,moveList.moveCnt+1);
	} else if(newLog) {
		newLog = 0;
		/*fclose(stdout);*/
		freopen(SHLOG,"a",stdout);
	}
	
	m->source = source;
	m->dest = dest;
	m->type = class;
	m->multi = multi;
	m->oneCard = moveOne(class,source,dest);
	m->movesct = movesct;
		
	moveList.moveCnt++;
	

#ifdef RANGECK
	if(moveList.moveCnt>1000) {
		SysBeep(60);
		moveList.moveCnt--;
	}
#endif
	usource = udest = utype = -1; /* reset unmove status */
}

int runLength(int source)
{
	int cur = piles[source].cnt-1;
	int len = 1;
	if(cur<0) return 0;
	while (cur>0) {
		if (( piles[source].cards[cur].suit == piles[source].cards[cur-1].suit)
		  &&( piles[source].cards[cur].rank+1 == piles[source].cards[cur-1].rank )) len++;
		else goto exit;
		cur--;
	}
exit:
	return len;
}

int countEmptyTowers()
{
	int emp = 0,index = 0;
	while (index<4) {
		if(towers[index].cnt == 0) emp++;
		index++;
	}
	return emp;
}

/* newPtr replaces NewPtr and is a local heap allocater */
Ptr newPtr(int size)
	/*int	size;*/
{
	Ptr		tmpHeap;
	if(myHeapSize>=size) {
		tmpHeap = myHeap;
		myHeap +=size;
		myHeapSize -=size;
		return tmpHeap;
	}
	else return 0l;
}
/* doStates will generate a current state snapshot, try to find it in the
	history data, and save it if not found, and return the found status */
	
int doStates()
{
	register state *curStp;
	/* try to find the current state at the same level */
	setState(&mySt);
	deadEnd = 0;
	curStp = stp[cksum];
	ChainLen = 0;
	while (curStp) {
		if (EQstate(curStp,&mySt)) { deadEnd = 1; curStp = 0; }
		else { curStp = (state *)curStp->next; ChainLen++; }
	}
	if(ChainLen>MaxChain) MaxChain = ChainLen; /* for info only */
#if 0 /* bug */
	if(log) {
		printf("\nfound %d match at cksum= %X for state %lX,%lX,%lX,%lX",deadEnd,cksum,mySt.st1,mySt.st2,mySt.st3,mySt.st4);
		}
#endif	
	/* below constant limits length of chain at each move level.  */
	if(!deadEnd&&(ChainLen<CHAINLIMIT)) { /* no match found, add my state to list for this move */
		if(curStp = (state *)newPtr(sizeof(state))) {
			*curStp = mySt;
			curStp->next = (Ptr)stp[cksum]; /* link it in */
			stp[cksum] = curStp; 
			SScnt++;
#if 0 /* bug */
			if(log) printf(" installed # %ld",SScnt);
#endif
		}
		/* else out of memory */
	}
	return deadEnd; /* fix 1/1/92, somehow this worked before 5.0, probably should return deadEnd */
	/* end of states */
}

/* destChk will return true if not trapping a little one at top of long run */
int destChk(int dest,int Lsource)
{
	int dlen = runLength(dest);
	if(dlen == 0) return 1;
	if((dlen + Lsource) <= 5) return 1; /* ok to move these */
	/* otherwise tst for blocking card */
	else {
		card	*cur	= &(piles[dest].cards[piles[dest].cnt - dlen]);
		int	suit	= cur->suit;
		int 	rank	= cur->rank;
		while( cur != &(piles[dest].cards[0])) {
			cur--;
			if((cur ->suit == suit) && ( cur->rank <rank)) return 0; /* can't move here */
		}
		return 1;
	}
}

int doAces()
{
	int	ace,pilen,tryAgain =1;
	card	top;
	register pile *p;
	while (tryAgain) {
		tryAgain = 0;
		/* first stack any ace candidates from the 10 piles */
		for (pilen = 0; pilen < 10; pilen++) if(piles[pilen].cnt) {
			top = piles[pilen].cards[piles[pilen].cnt-1];
			p = &aces[top.suit];
			if(((p->cnt == 0) && (top.rank == 0)) ||
			   ((p->cnt != 0) &&
				(p->cards[p->cnt-1].rank == top.rank-1))) {
				stackOne(pile_to_ace,pilen,top.suit,0);
				tryAgain = 1;
			}
		}
		
		for (pilen = 0; pilen < 4; pilen++) 
			/* and then from the 4 towers */
			if (towers[pilen].cnt) {
				top = towers[pilen].cards[0];
				p = &aces[top.suit];
				if(((p->cnt == 0) && (top.rank == 0)) ||
				   ((p->cnt != 0) &&
					(p->cards[p->cnt-1].rank == top.rank-1))) {
						stackOne(tower_to_ace,pilen,top.suit,0);
						tryAgain = 1;
					}
			}
	}
	if(doStates()) return 0; else /* don't bother with the count if seen this one before*/
	return ((aces[spades].cnt == 13) & (aces[clubs].cnt == 13) &
			(aces[diamonds].cnt == 13) & (aces[hearts].cnt == 13));
}

char	myStr[80];
//Str255 myStr;
MenuHandle	MM = 0;

void BuildMyMenu()
{
	if(MM) DisposeMenu(MM);
	myStr[0] = sprintf(&myStr[1],
		"%ld:%d..%d, MaxChain:%d, Held:%ld, skpMoves:%ld, Heap=%ld",
		movesct,mLow,mHigh,MaxChain,SScnt,skipped,myHeapSize);
	MM = NewMenu(1,myStr);
	InsertMenu(MM,0);
}
	
void doCards()
{
	int source,dest,runLen,empties,flag;
	int msource,mdest,p2pRank;	/* track the prime card of a multi-move pile-to-pile */
	long movesFlag = 0;
	card sc,dc;
	EventRecord testEvent;
	while ((!doAces()) && (!quit)) {
		if((movesct - movesFlag) > 0 ){
			movesFlag += NUM_MOVES;
			GetNextEvent(everyEvent,&testEvent);
			if(testEvent.what == keyDown) {
				if((testEvent.message & charCodeMask) == 'L') log = (log==0)?1:0;
				else if((testEvent.message & charCodeMask) == 'Q') {
					quit=1;
					msg[0] = 'C';
					msg[1] = 'a';
					msg[2] = 'n';
					msg[3] = '\'';
					msg[4] = 't';
				}
				else if((testEvent.message & charCodeMask) == 'M') {
					ClearMenuBar();
					BuildMyMenu();
					DrawMenuBar();
					mLow = 1000;
					mHigh = 0;
				}
				else SysBeep(1);
			}
		}
		
		/* now try to detect a "stuck" config by comparing the state to the prev. state
		   before we make any more moves... */
		   
toff:	if(deadEnd == 1) { /* display for debug purposes */
					skipped++;
		/*			ClearMenuBar();
					BuildMyMenu();
					DrawMenuBar(); */
			goto mLoopI; /* skip this move... */
		}
		   
		/* the first thing to try is to move cards off the towers until can't do that */
		/* then move from pile to pile, attempting to dump towers each move */
		/* then after no more moves, move a card to tower and try again */
		/* when no card-tower moves can be made, unmake the last move and continue */
/* toff: */
		for (source=0; source<4; source++) {
			flag = 0;
			/* move any cards off the towers */
			/* careful, here, we must not duplicate (inverse) the last move... */
			/* first we qualify this entire mess by an available tower card...*/
			/* also must never move tower card back where it came from... */
			if(towers[source].cnt > 0)
			  for(dest=0; dest<10; dest++) {
				/* also test for duplicate of last _un_move NOTE line below commented out!
				if((source == usource) && (dest == udest) && (utype == tower_to_pile))
					goto test3; /* can't move from this pile now cause was last unmove */
				sc = towers[source].cards[0];

				if (piles[dest].cnt == 0) {
					/* do the king case */
					dc.rank = 13; dc.suit = sc.suit; 
				}
				else /* use the card that's there for compare */
					dc = piles[dest].cards[piles[dest].cnt-1];
				
#if 1 /* if we just moved this card up, don't move it down again... */
				/* need to change this to "skip" ace moves */
				if ((moveList.moveCnt == 0) 
				|| (moveList.moves[moveList.moveCnt-1].type != pile_to_tower)
				|| (moveList.moves[moveList.moveCnt-1].dest != source) )
#endif
				/* the above enables the if below if not inversing the last move */
				if ((sc.rank+1 == dc.rank) && (sc.suit == dc.suit) && (destChk(dest,1))) {
					stackOne(tower_to_pile,source,dest,0);
					flag = 1; /* note the hit */
					doStates(); /* sets global 'deadend' */
					goto toff;
				}
test3:;		}
			
nxtP3:;	}
		if(flag) goto next;

p2p:	for (source=0; source<10; source++) if(piles[source].cnt) {
			/* move cards from pile to pile if they fit */
			/* here, too, don't dup the last unmove */
			/* 12/20/88 add logic to move multiple cards from pile to pile */
			/* note unmoves must duplicate this */
			runLen = runLength(source);	/*.|cbp881220 */
			empties= countEmptyTowers();	/*.|cbp881220 */
			sc = piles[source].cards[piles[source].cnt-runLen];	/*.|cbp881220 */
			/* note a move can only be made if runLen=1 or empties >= runlen-1 */
			if(empties >= (runLen-1)) /* qualify the mess below on run length */ /*.|cbp881220 */
			/* don't move any king piles */
			if((piles[source].cnt != runLen) || (piles[source].cards[0].rank != 12))
			for (dest=0; dest<10; dest++) 
			  if ((dest != source) && (piles[source].cnt > 0)) {

				if (piles[dest].cnt == 0) {
					/* do the king case */
					dc.rank = 13; dc.suit = sc.suit; 
					/* if 1 source card don't move it..... done by above rank !=12 if...
					if(piles[source].cnt == 1) dc.suit++; /* force non-compare */
				}
				else /* use the card that's there for compare */
					dc = piles[dest].cards[piles[dest].cnt-1];

				/* here, too, don't dup the last unmove */
				/* maybe not required as move is not symmetrical ***********/
				if((source == usource) && (dest == udest) && (utype == pile_to_pile));
				else 
				/* the above enables the if below if not inversing the last move */

				/*if (destChk(dest,runLen)) /* don't move if result is too long dest run */
				if ((sc.rank+1 == dc.rank) && (sc.suit == dc.suit) && (destChk(dest,runLen))) {
					/* note we may have to make multiple moves here */
					if(runLen == 1) stackOne(pile_to_pile,source,dest,0);
					else {
						/* move multiple; we know we have enuf empties */
						int towerP =0;
						int flag =0;	/* multi move tracker */
						int i;
						int twrs[4]; /* keep track of towers moved to */
						
						/* first move runlen-1 cards to towers */
						i = runLen-2;
						while (i>=0) {
							while (towers[towerP].cnt>0) towerP++; /* find empty tower*/
							twrs[i] = towerP; /* track it */
							stackOne(pile_to_tower,source,towerP,flag); /* 1 means multiple */
							if(flag==0) flag = pile_to_pile; /* the first multi is zero to stop unmove*/
							i--;
						}
						/* now move the card from pile to pile */
						stackOne(pile_to_pile,source,dest,1);
						
						/* now unstack the tower cards onto the dest pile */
						i = 0;
						while (i<runLen-1) {
							stackOne(tower_to_pile,twrs[i],dest,1);
							i++;
						}
					}
					goto next;
				}
test1:;		}
test1a:;}
/* test1a was pointed to "if" below but 1/2/89 changed to just go next major card */
			/* now move cards to any empty towers */
			/* note we must be careful here not to duplicate the last unmove */
			/* and also careful not to duplicate (inverse) the last move */
			
		empties= countEmptyTowers();
		if(empties) for(source = 0; source<10; source++) {

			if((source == usource) /* && (dest == udest) */ && (utype == tower_to_pile))
				goto nxtP; /* can't move from this pile now cause was last unmove */
			if(piles[source].cnt > 0){
				/* a candidate can only be moved if (its run length is */
				/* less than pile length _or_ top card ne King) _and_ */
				/* "length" empty spaces exist on towers */
				/* also, there is no point in moving less than "run length" */
				/* to a tower because the runlen-1 case will have been covered */
				/* by the pile-to-pile code above. */
				int runLen = runLength(source);
								
				if ((moveList.moveCnt == 0) 
				|| (moveList.moves[moveList.moveCnt-1].type != tower_to_pile)
				|| (moveList.moves[moveList.moveCnt-1].dest != source) )
				/* the above enables the if below if not inversing the last move */
				if(((runLen<piles[source].cnt) || (piles[source].cards[0].rank != 12))
				  &&(empties >= runLen) ) {
					/* note we may have to make multiple moves here */
					if(runLen == 1) {
						int towerP =0;
						while (towers[towerP].cnt>0) towerP++; /* find empty tower*/
/* test 				piles[source].cards[piles[source].cnt].toTowCnt = moveList.moveCnt;  /* note this move..*/
						stackOne(pile_to_tower,source,towerP,0);
					}
					else {
						/* move multiple; we know we have enuf empties */
						int i;
						int flag = 0;
						int towerP =0;
						/* move runlen cards to towers */
						i = runLen-1;
						while (i>=0) {
							while (towers[towerP].cnt>0) towerP++; /* find empty tower*/
/* test 					piles[source].cards[piles[source].cnt].toTowCnt = moveList.moveCnt;  /* note this move..*/
							stackOne(pile_to_tower,source,towerP,flag); /* 1 means multiple */
							if(flag == 0) flag = 10; /* use 10 cause pile_to_tower=0 */
							i--;
						}
					}
					goto next;
				}
				else goto nxtP; /* can't move from this pile now */
			}
test2:;
nxtP:;	}
		
		
		/* here we have tried all moves and not made any, time to take back a move */
		/* undo a move, calculate where we were and resume there... */
		/* 12/20/88 add logic to undo a multi card pile to pile move */
				
mLoopI:	mdest = -1; /* reset the multiple flag going in */
mLoop:	if(log) printf("\nUndo");
aceLoop:if(moveList.moveCnt) {
			register move *m = &moveList.moves[--moveList.moveCnt];
			usource = source = m->source;
			udest   = dest   = m->dest;
			/* mdest	=			-1;	/* reset the multiple flag */
			utype	= m->type;
			umulti  = m->multi;
			
			switch	(utype) {
		  
				case	pile_to_tower:
					/* move from tower back to pile...*/
					moveOne(tower_to_pile,udest,usource);
					if(umulti) goto mLoop;	/*.|cbp881220 */
					if(!(mdest==-1)) {
						/* returning from multiple move... */
						source = msource;
						dest = mdest;
						goto mp2p;
					}
					empties= countEmptyTowers();	/*.|cbp881227 */
					goto test2; /* enter middle of loop */
					
				case	pile_to_pile:
					p2pRank = m->oneCard.rank;	/* save for king test */
					moveOne(pile_to_pile,udest,usource);
					if(umulti) {
						mdest = dest;
						msource = source;
						goto mLoop;	/*.|cbp881220 */
					}
mp2p:				runLen = runLength(source);	/*.|cbp881221 */
					empties= countEmptyTowers();	/*.|cbp881221 */
					sc = piles[source].cards[piles[source].cnt-runLen];	/*.|cbp881221 */
					if((empties >= (runLen-1)) /* qualify the mess below on run length */ /*.|cbp881221 */
					  && /*((piles[source].cnt != runLen) || (piles[source].cards[0].rank != 12)) ) */
					  	   (sc.rank != 12)  ) /* I think this will replace above */
						goto test1;
					else goto test1a; /* skip this card to card set */
					
				case	pile_to_ace:
					/* ace moves occurred in "doAces".  We unmove by */
					/* looping until a non-ace move is undone */
					moveOne(ace_to_pile,udest,usource);			
					goto aceLoop;
					
				case	tower_to_ace:
					/* ace moves occurred in "doAces".  We unmove by */
					/* looping until a non-ace move is undone */
					moveOne(ace_to_tower,udest,usource);			
					goto aceLoop;
					
				case	tower_to_pile:
					moveOne(pile_to_tower,udest,usource);
					if(umulti) goto mLoop;	/*.|cbp881220 */
					flag = 0; /* set up for next tower_to_pile test */
					goto nxtP3;
				
				default:
					SysBeep(60);
					SysBeep(60);
			}
		}
		/* if we get here, there are no more moves to be undone. */
		/* since we only undo when no more moves to be made, we have */
		/* failed in our mission... and have to admit it */
		msg[0] = 'C';
		msg[1] = 'a';
		msg[2] = 'n';
		msg[3] = '\'';
		msg[4] = 't';
		goto exit;
			
next:;	/* next is just before ace check to continue a sort of "outer loop " */
	}
exit:;
}

main()
{
	int 	i,j,k,offset,int1,int2,hisResFile;
	FILE *myFile;
	Handle	towr;
	/*Ptr	myPtr;*/

	MaxApplZone();

	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	
	/* fetch his layout [in system folder] */
	hisResFile = OpenResFile("\pindexor");
	towr = GetResource('TOWR',128);
	
	/* reset the stacks */
	for (i=0;i<10;i++) {
		if(i<4) {aces[i].cnt=0; towers[i].cnt=0;}
		piles[i].cnt=0;
	}
	/* and the state detector */
	for (j=0;j<4096;j++) stp[j] = 0;
	
	/* and the towMove flags */
	/*towMovect[0]=towMovect[1]=towMovect[2]=towMovect[3] = 9999;*/
	
	/* now load my piles */
	
	for(k=spades,offset=0x40;k<=clubs;k++,offset+=52){
		i=0; /* here is first int */
		j=0;
		while (i<52) {/* do the cards */
			int1 = *(int *)((*towr)+i+offset);
			i+=2;
			int2 = *(int *)((*towr)+i+offset);
			i+=2;
			
			if(int2>2) {
				piles[int1].cards[int2-3].rank=j;
				piles[int1].cards[int2-3].suit=k;
				piles[int1].cnt++;
			}
			else if(int2==2) { /* on an ace; put in order of rank */
				aces[k].cards[j].rank=j;
				aces[k].cards[j].suit=k;
				aces[k].cnt++;
			}
			else if(int2==1) { /* on the center towrs */
				towers[int1].cards[0].rank=j;
				towers[int1].cards[0].suit=k;
				towers[int1].cnt=1;
			}
			j++;
		}
	}
	CloseResFile(hisResFile);
	if(1) myFile = freopen(SHLOG,"w",stdout);

	myHeapSize = ApplZone->zcbFree-20000;
	myHeap = NewPtr(myHeapSize);
		
	doCards(); 
	if(msg[0] == 'I') { 
		SysBeep(1);
		SysBeep(1);
		SysBeep(1);
		SysBeep(1);
	}
	else SysBeep(1); /* notify of winning or losing condition */
	printf(msg);
	dumpMoves();
	printf("\nTotal attempts = %ld",movesct);
	printf("\nTotal moves = %d",moveList.moveCnt);
	printf("\nTotal stored states = %ld",SScnt);
	printf("\nTotal skipped moves = %ld",skipped);
	printf("\nLongest chain = %d",MaxChain);
	printf("\nHeap space left = %ld",myHeapSize);
	
	if(1)fclose(stdout);
}