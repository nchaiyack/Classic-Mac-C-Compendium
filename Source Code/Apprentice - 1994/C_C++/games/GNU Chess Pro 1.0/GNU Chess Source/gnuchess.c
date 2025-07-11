/*
  C source for GNU CHESS

  Modified : 20-3-91 Airy ANDRE.

  Copyright (C) 1986, 1987, 1988, 1989, 1990 Free Software Foundation, Inc.
  Copyright (c) 1988, 1989, 1990  John Stanback
  Copyright (c) 1991 Airy Andr�

  This file is part of CHESS.

  CHESS is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY.  No author or distributor accepts responsibility to anyone for
  the consequences of using it or for whether it serves any particular
  purpose or works at all, unless he says so in writing.  Refer to the CHESS
  General Public License for full details.

  Everyone is granted permission to copy, modify and redistribute CHESS, but
  only under the conditions described in the CHESS General Public License. A
  copy of this license is supposed to have been given to you along with
  CHESS so you can know your rights and responsibilities.  It should be in a
  file named COPYING.  Among other things, the copyright notice and this
  notice must be preserved on all copies.
*/

/* This file is the original "gnuchess.c" modified to allow a Mac interface
   and a few new options.
   It is for Mac version ONLY
   --------------------------
*/

#include <stdio.h>


#include <ctype.h>

/*
  ttblsz must be a power of 2.
  Setting ttblsz 0 removes the transposition tables.
*/
#include <time.h>
#define ttblsz 65536
#define huge

extern int abs ();
extern int atoi ();

extern void *memset(void *, int, size_t);

#include "gnuchess.h"

#define bpawn 7
#define valueP 100
#define valueN 350
#define valueB 355
#define valueR 550
#define valueQ 1100
#define valueK 1200
#define ctlP 0x4000
#define ctlN 0x2800
#define ctlB 0x1800
#define ctlR 0x0400
#define ctlQ 0x0200
#define ctlK 0x0100
#define ctlBQ 0x1200
#define ctlRQ 0x0600
#define ctlNN 0x2000


#if ttblsz
#define truescore 0x0001
#define lowerbound 0x0002
#define upperbound 0x0004
#define kingcastle 0x0008
#define queencastle 0x0010

struct hashval
{
  unsigned long key,bd;
};

struct hashentry
{
  unsigned long hashbd;
  unsigned short mv;
  unsigned char flags, depth;	/* char saves some space */
  short score;
#ifdef HASHTEST
  unsigned char bd[32];
#endif /* HASHTEST */
};

#ifdef HASHFILE
/*
  persistent transposition table.
  The size must be a power of 2. If you change the size,
  be sure to run gnuchess -t before anything else.
*/
#define frehash 6
#define filesz 131072
struct fileentry
{
  unsigned char bd[32];
  unsigned char f, t, flags, depth, sh, sl;
};
/*
  In a networked enviroment gnuchess might be compiled on different
  hosts with different random number generators, that is not acceptable
  if they are going to share the same transposition table.
*/
unsigned long int next = 1;

unsigned int rand()
{
  next *= 1103515245;
  next += 12345;
  return ((unsigned int) (next >> 16) & 0xFFFF);
}

void srand (seed)
     unsigned int seed;
{
  next = seed;
}

#endif /* HASHFILE */

static unsigned long hashkey, hashbd;
static struct hashval *hashcode[2][7];
static struct hashentry huge *ttable[2];
#endif /* ttblsz */

int hashfile;
struct leaf *Tree, *root;
short TrPnt[maxdepth];
short PieceList[2][16], PawnCnt[2][8];
short castld[2], Mvboard[64];
short c1, c2, *atk1, *atk2, *PC1, *PC2, atak[2][64];
short mate, post, opponent, computer, Sdepth, Awindow, Bwindow, dither;
long ResponseTime, ExtraTime, Level[2], et, et0, ft;
unsigned long time0;
long NodeCnt, ETnodes, EvalNodes, HashCnt, FHashCnt, HashCol;
short quit, reverse, bothsides, hashflag, InChk, player, force, easy, beep;
short timeout, xwndw, rehash;
struct GameRec *GameList;
short GameCnt, Game50, epsquare, lpost, rcptr, contempt;
short MaxSearchDepth;
struct BookEntry *Book;
struct TimeControlRec TimeControl;
short TCflag[2], TCmoves[2], TCminutes[2], OperatorTime[2];

short donotplay;

#ifdef ttblsz
static unsigned long real_ttblsz = ttblsz;
#endif /* ttblsz */

const short otherside[3] =
{1, 0, 2};
unsigned short hint, PrVar[maxdepth];

static short Pindex[64], svalue[64];
static short PieceCnt[2];
static short mtl[2], pmtl[2], emtl[2], hung[2];
static short EnemyKing;
static short wking, bking, FROMsquare, TOsquare, Zscore, zwndw, slk;
static short INCscore;
static short HasPawn[2], HasKnight[2], HasBishop[2], HasRook[2], HasQueen[2];
static short ChkFlag[maxdepth], CptrFlag[maxdepth], PawnThreat[maxdepth];
static short Pscore[maxdepth], Tscore[maxdepth];
static const short qrook[3] =
{0, 56, 0};
static const short krook[3] =
{7, 63, 0};
static const short kingP[3] =
{4, 60, 0};
static const short rank7[3] =
{6, 1, 0};
static const short sweep[7] =
{false, false, false, true, true, true, false};
static unsigned short *killr0, *killr1, *killr2;
static unsigned short *killr3;
static unsigned short PV, Swag0, Swag1, Swag2, Swag3, Swag4;
static unsigned char *history;

static short *Mwpawn, *Mbpawn, *Mknight[2], *Mbishop[2];
static short *Mking[2], *Kfield[2];
static const short value[7] =
{0, valueP, valueN, valueB, valueR, valueQ, valueK};
static const short control[7] =
{0, ctlP, ctlN, ctlB, ctlR, ctlQ, ctlK};
static const short PassedPawn0[8] =
{0, 60, 80, 120, 200, 360, 600, 800};
static const short PassedPawn1[8] =
{0, 30, 40, 60, 100, 180, 300, 800};
static const short PassedPawn2[8] =
{0, 15, 25, 35, 50, 90, 140, 800};
static const short PassedPawn3[8] =
{0, 5, 10, 15, 20, 30, 140, 800};
static const short ISOLANI[8] =
{-12, -16, -20, -24, -24, -20, -16, -12};
static const short BACKWARD[16] =
{-6, -10, -15, -21, -28, -28, -28, -28,
 -28, -28, -28, -28, -28, -28, -28, -28};
static const short BMBLTY[14] =
{-2, 0, 2, 4, 6, 8, 10, 12, 13, 14, 15, 16, 16, 16};
static const short RMBLTY[15] =
{0, 2, 4, 6, 8, 10, 11, 12, 13, 14, 14, 14, 14, 14, 14};
static const short KTHRT[36] =
{0, -8, -20, -36, -52, -68, -80, -80, -80, -80, -80, -80,
 -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80,
 -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80, -80};
static short KNIGHTPOST, KNIGHTSTRONG, BISHOPSTRONG, KATAK, KBNKsq;
static short PEDRNK2B, PWEAKH, PADVNCM, PADVNCI, PAWNSHIELD, PDOUBLED, PBLOK;
static short RHOPN, RHOPNX, KHOPN, KHOPNX, KSFTY;
static short ATAKD, HUNGP, HUNGX, KCASTLD, KMOVD, XRAY, PINVAL;
static short stage, stage2, Zwmtl, Zbmtl, Developed[2], PawnStorm;
static short PawnBonus, BishopBonus, RookBonus;
static const short KingOpening[64] =
{0, 0, -4, -10, -10, -4, 0, 0,
 -4, -4, -8, -12, -12, -8, -4, -4,
 -12, -16, -20, -20, -20, -20, -16, -12,
 -16, -20, -24, -24, -24, -24, -20, -16,
 -16, -20, -24, -24, -24, -24, -20, -16,
 -12, -16, -20, -20, -20, -20, -16, -12,
 -4, -4, -8, -12, -12, -8, -4, -4,
 0, 0, -4, -10, -10, -4, 0, 0};
static const short KingEnding[64] =
{0, 6, 12, 18, 18, 12, 6, 0,
 6, 12, 18, 24, 24, 18, 12, 6,
 12, 18, 24, 30, 30, 24, 18, 12,
 18, 24, 30, 36, 36, 30, 24, 18,
 18, 24, 30, 36, 36, 30, 24, 18,
 12, 18, 24, 30, 30, 24, 18, 12,
 6, 12, 18, 24, 24, 18, 12, 6,
 0, 6, 12, 18, 18, 12, 6, 0};
static const short DyingKing[64] =
{0, 8, 16, 24, 24, 16, 8, 0,
 8, 32, 40, 48, 48, 40, 32, 8,
 16, 40, 56, 64, 64, 56, 40, 16,
 24, 48, 64, 72, 72, 64, 48, 24,
 24, 48, 64, 72, 72, 64, 48, 24,
 16, 40, 56, 64, 64, 56, 40, 16,
 8, 32, 40, 48, 48, 40, 32, 8,
 0, 8, 16, 24, 24, 16, 8, 0};
static const short KBNK[64] =
{99, 90, 80, 70, 60, 50, 40, 40,
 90, 80, 60, 50, 40, 30, 20, 40,
 80, 60, 40, 30, 20, 10, 30, 50,
 70, 50, 30, 10, 0, 20, 40, 60,
 60, 40, 20, 0, 10, 30, 50, 70,
 50, 30, 10, 20, 30, 40, 60, 80,
 40, 20, 30, 40, 50, 60, 80, 90,
 40, 40, 50, 60, 70, 80, 90, 99};
static const short pknight[64] =
{0, 4, 8, 10, 10, 8, 4, 0,
 4, 8, 16, 20, 20, 16, 8, 4,
 8, 16, 24, 28, 28, 24, 16, 8,
 10, 20, 28, 32, 32, 28, 20, 10,
 10, 20, 28, 32, 32, 28, 20, 10,
 8, 16, 24, 28, 28, 24, 16, 8,
 4, 8, 16, 20, 20, 16, 8, 4,
 0, 4, 8, 10, 10, 8, 4, 0};
static const short pbishop[64] =
{14, 14, 14, 14, 14, 14, 14, 14,
 14, 22, 18, 18, 18, 18, 22, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 18, 22, 22, 22, 22, 18, 14,
 14, 22, 18, 18, 18, 18, 22, 14,
 14, 14, 14, 14, 14, 14, 14, 14};
static const short PawnAdvance[64] =
{0, 0, 0, 0, 0, 0, 0, 0,
 4, 4, 4, 0, 0, 4, 4, 4,
 6, 8, 2, 10, 10, 2, 8, 6,
 6, 8, 12, 16, 16, 12, 8, 6,
 8, 12, 16, 24, 24, 16, 12, 8,
 12, 16, 24, 32, 32, 24, 16, 12,
 12, 16, 24, 32, 32, 24, 16, 12,
 0, 0, 0, 0, 0, 0, 0, 0};


/* .... MOVE GENERATION VARIABLES AND INITIALIZATIONS .... */


#define taxicab(a,b) taxidata[a][b]
short *distdata[64], *taxidata[64];

inline void
Initialize_dist (void)
{
  register short a, b, d, di;

  for (a = 0; a < 64; a++)
    for (b = 0; b < 64; b++)
      {
	d = abs (column (a) - column (b));
	di = abs (row (a) - row (b));
	taxidata[a][b] = d + di;
	distdata[a][b] = (d > di ? d : di);
      }
}

const short Stboard[64] =
{rook, knight, bishop, queen, king, bishop, knight, rook,
 pawn, pawn, pawn, pawn, pawn, pawn, pawn, pawn,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 pawn, pawn, pawn, pawn, pawn, pawn, pawn, pawn,
 rook, knight, bishop, queen, king, bishop, knight, rook};
const short Stcolor[64] =
{white, white, white, white, white, white, white, white,
 white, white, white, white, white, white, white, white,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 black, black, black, black, black, black, black, black,
 black, black, black, black, black, black, black, black};
short board[64], color[64];
static unsigned char *nextpos[8][64];
static unsigned char *nextdir[8][64];
/*
  ptype is used to separate white and black pawns, like this;
  ptyp = ptype[side][piece]
  piece can be used directly in nextpos/nextdir when generating moves
  for pieces that are not black pawns.
*/
static const short ptype[2][8] =
{
  no_piece, pawn, knight, bishop, rook, queen, king, no_piece,
  no_piece, bpawn, knight, bishop, rook, queen, king, no_piece};
static const short direc[8][8] =
{
  0, 0, 0, 0, 0, 0, 0, 0,
  10, 9, 11, 0, 0, 0, 0, 0,
  8, -8, 12, -12, 19, -19, 21, -21,
  9, 11, -9, -11, 0, 0, 0, 0,
  1, 10, -1, -10, 0, 0, 0, 0,
  1, 10, -1, -10, 9, 11, -9, -11,
  1, 10, -1, -10, 9, 11, -9, -11,
  -10, -9, -11, 0, 0, 0, 0, 0};
static const short max_steps[8] =
{0, 2, 1, 7, 7, 7, 1, 2};
static const short nunmap[120] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, -1,
  -1, 8, 9, 10, 11, 12, 13, 14, 15, -1,
  -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
  -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
  -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
  -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
  -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
  -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


#ifdef HASHFILE

/* 
   Check if hashfile exists. If it doesn't, ask if a new one have to be
   created
*/

int CheckHashFile(void)
{
  if (FSOpen (HASHFILE, 0, &hashfile) == fnfErr)
  {
  		if (!AreYouSure(_NEWTABLE_STR)) return 0;
  		Create(HASHFILE, 0, 'GCHS', 'GCht');
 		if (FSOpen (HASHFILE, 0, &hashfile) != noErr)
  		{
  			ShowError(_CREATE_STR);
  			hashfile = 0;
  			return 0;
  		}
		if (hashfile != 0)
		{
			long i, j;
			struct fileentry n;
			long count;
			
		    for(j = 0; j < 32; j++)
		      n.bd[j] = 0;
		    n.f = n.t = 0;
		    n.flags = 0;
		    n.depth = 0;
		    n.sh = n.sl = 0;
		    for (j = 0; j < filesz; j++) {
		      long count = sizeof(struct fileentry);
		      if (FSWrite(hashfile, &count, &n) != noErr) {
		      	ShowError(_WRITE_STR);
		      	FSClose(hashfile);
		      	FSDelete(HASHFILE, 0);
		      	hashfile = 0;
		      	return 0;
		      }
		    }
		}
	}
	return 1;
}
#endif

static void alloc_tables()
{
	int i,j;
	
#if ttblsz
	void *reserved;
	
	for (i=0; i<2; i++)
		for (j=0; j<7; j++) {
			hashcode[i][j] = (struct hashval*)NewPtr(64*sizeof(struct hashval));
			if (hashcode[i][j] == NULL)
				ExitToShell();
	}
	
	reserved = NewPtr(200000);
	
	if (reserved == NULL) ExitToShell();
	while (real_ttblsz) {
		ttable[0] = (struct hashentry*)NewPtr(real_ttblsz*sizeof(struct hashentry));
		if (ttable[0] == NULL) {
			real_ttblsz >>= 1;
			continue;
		}
		ttable[1] = (struct hashentry*)NewPtr(real_ttblsz*sizeof(struct hashentry));
		if (ttable[1] == NULL) {
			DisposPtr(ttable[0]);
			real_ttblsz >>= 1;
			continue;
		}
		break;
	}
	DisposPtr(reserved);
	if (real_ttblsz == 0) ExitToShell();
	
#endif /* ttblsz */

	killr0 = (unsigned short *)NewPtr(maxdepth*sizeof(unsigned short ));
	killr1 = (unsigned short *)NewPtr(maxdepth*sizeof(unsigned short ));
	killr2 = (unsigned short *)NewPtr(maxdepth*sizeof(unsigned short ));
	killr3 = (unsigned short *)NewPtr(maxdepth*sizeof(unsigned short ));
	history = (unsigned char*)NewPtr(8192 * sizeof(unsigned char));

	Mwpawn = (short *)NewPtr(64*sizeof(short ));
	Mbpawn = (short *)NewPtr(64*sizeof(short ));
	Mknight[0] = (short *)NewPtr(64*sizeof(short ));
	Mknight[1] = (short *)NewPtr(64*sizeof(short ));
	Mking[0] = (short *)NewPtr(64*sizeof(short ));
	Mking[1] = (short *)NewPtr(64*sizeof(short ));
	Mbishop[0] = (short *)NewPtr(64*sizeof(short ));
	Mbishop[1] = (short *)NewPtr(64*sizeof(short ));
	Kfield[0] = (short *)NewPtr(64*sizeof(short ));
	Kfield[1] = (short *)NewPtr(64*sizeof(short ));
	
	GameList = (struct GameRec*)NewPtr(500 * sizeof(struct GameRec));
	for (i=0; i<64; i++) {
		distdata[i] = (short*)NewPtr(64*sizeof(short));
		taxidata[i] = (short*)NewPtr(64*sizeof(short));
	}
	for (i=0; i<8; i++)
		for (j=0; j<64; j++) {
			nextpos[i][j] = (unsigned char *)NewPtr(64L);
			nextdir[i][j] = (unsigned char *)NewPtr(64L);
		}
		
	root = Tree = (struct leaf *)NewPtr(2000*sizeof(struct leaf));

}

void
Initialize_moves ()

/*
  This procedure pre-calculates all moves for every piece from every square.
  This data is stored in nextpos/nextdir and used later in the move generation
  routines.
*/

{
  short ptyp, po, p0, d, di, s, delta;
  unsigned char *ppos, *pdir;
  short dest[8][8];
  short steps[8];
  short sorted[8];

  for (ptyp = 0; ptyp < 8; ptyp++)
    for (po = 0; po < 64; po++)
      for (p0 = 0; p0 < 64; p0++)
	{
	  nextpos[ptyp][po][p0] = po;
	  nextdir[ptyp][po][p0] = po;
	}
  for (ptyp = 1; ptyp < 8; ptyp++)
    for (po = 21; po < 99; po++)
      if (nunmap[po] >= 0)
	{
	  ppos = nextpos[ptyp][nunmap[po]];
	  pdir = nextdir[ptyp][nunmap[po]];
	  /* dest is a function of direction and steps */
	  for (d = 0; d < 8; d++)
	    {
	      dest[d][0] = nunmap[po];
	      delta = direc[ptyp][d];
	      if (delta != 0)
		{
		  p0 = po;
		  for (s = 0; s < max_steps[ptyp]; s++)
		    {
		      p0 = p0 + delta;
		      /*
			break if (off board) or
			(pawns only move two steps from home square)
		      */
		      if (nunmap[p0] < 0 || (ptyp == pawn || ptyp == bpawn)
			  && s > 0 && (d > 0 || Stboard[nunmap[po]] != pawn))
			break;
		      else
			dest[d][s] = nunmap[p0];
		    }
		}
	      else
		s = 0;

	      /*
	        sort dest in number of steps order
	        currently no sort is done due to compability with
	        the move generation order in old gnu chess
	      */
	      steps[d] = s;
	      for (di = d; s > 0 && di > 0; di--)
		if (steps[sorted[di - 1]] == 0)	/* should be: < s */
		  sorted[di] = sorted[di - 1];
		else
		  break;
	      sorted[di] = d;
	    }

	  /*
	    update nextpos/nextdir,
	    pawns have two threads (capture and no capture)
	  */
	  p0 = nunmap[po];
	  if (ptyp == pawn || ptyp == bpawn)
	    {
	      for (s = 0; s < steps[0]; s++)
		{
		  ppos[p0] = dest[0][s];
		  p0 = dest[0][s];
		}
	      p0 = nunmap[po];
	      for (d = 1; d < 3; d++)
		{
		  pdir[p0] = dest[d][0];
		  p0 = dest[d][0];
		}
	    }
	  else
	    {
	      pdir[p0] = dest[sorted[0]][0];
	      for (d = 0; d < 8; d++)
		for (s = 0; s < steps[sorted[d]]; s++)
		  {
		    ppos[p0] = dest[sorted[d]][s];
		    p0 = dest[sorted[d]][s];
		    if (d < 7)
		      pdir[p0] = dest[sorted[d + 1]][0];
		    /* else is already initialized */
		  }
	    }
	}
}

/* Dispose memory allocated for the Opening Book */
void DisposBook()
{
    struct BookEntry *p;
    
	while (Book != NULL) {
    	p = Book;
    	Book = Book->next;
    	free(p->mv);
    	free(p);
    }
}

void
NewGame ()
/*
  Reset the board and other variables to start a new game.
*/
{
  short l, c, p;

  stage = stage2 = -1;		/* the game is not yet started */
  mate = quit = false;
  PawnStorm = false;
  beep = rcptr = true; 
  
  NodeCnt = et0 = epsquare = lpost = 0;
  dither = 0;
  Awindow = 90;
  Bwindow = 90;
  xwndw = 90;
  MaxSearchDepth = 29;
  contempt = 0;
  GameCnt = -1;
  Game50 = 0;
  Zwmtl = Zbmtl = 0;
  Developed[white] = Developed[black] = false;
  castld[white] = castld[black] = false;
  PawnThreat[0] = CptrFlag[0] = false;
  Pscore[0] = 12000;
  Tscore[0] = 12000;

  for (l = 0; l < 2000; l++)
    Tree[l].f = Tree[l].t = 0;
#if ttblsz
  rehash = 6;
  ZeroTTable ();
  srand ((unsigned int) 1);
  for (c = white; c <= black; c++)
    for (p = pawn; p <= king; p++)
      for (l = 0; l < 64; l++)
	{
	  hashcode[c][p][l].key = (((unsigned long) rand ()));
	  hashcode[c][p][l].key += (((unsigned long) rand ()) << 16);
	  hashcode[c][p][l].bd = (((unsigned long) rand ()));
	  hashcode[c][p][l].bd += (((unsigned long) rand ()) << 16);
	  if (sizeof(long) > 4)
	    {
	      hashcode[c][p][l].key += (((unsigned long) rand ()) << 32);
	      hashcode[c][p][l].key += (((unsigned long) rand ()) << 48);
	      hashcode[c][p][l].bd += (((unsigned long) rand ()) << 32);
	      hashcode[c][p][l].bd += (((unsigned long) rand ()) << 48);
	    }
	}
#endif /* ttblsz */
  for (l = 0; l < 64; l++)
    {
      board[l] = Stboard[l];
      color[l] = Stcolor[l];
      Mvboard[l] = 0;
    }
  if (TCflag[white]) SetTimeControl (white);
  if (TCflag[black]) SetTimeControl (black);

  InitializeStats ();
  DisposBook();
  GetOpenings ();
  time0 = Ticks;
  ElapsedTime (1);
}


/* ............    MOVE GENERATION & SEARCH ROUTINES    .............. */

inline void
pick (p1, p2)
     short int p1;
     register short int p2;

/*
  Find the best move in the tree between indexes p1 and p2. Swap the best
  move into the p1 element.
*/

{
  register short p, s;
  register short p0, s0;
  struct leaf temp;

  s0 = Tree[p1].score;
  p0 = p1;
  for (p = p1 + 1; p <= p2; p++)
    if ((s = Tree[p].score) > s0)
      {
	s0 = s;
	p0 = p;
      }
  if (p0 != p1)
    {
      temp = Tree[p1];
      Tree[p1] = Tree[p0];
      Tree[p0] = temp;
    }
}

void
SelectMove (side, iop)
     short int side;
     short int iop;
/*
  Select a move by calling function search() at progressively deeper ply
  until time is up or a mate or draw is reached. An alpha-beta window of -90
  to +90 points is set around the score returned from the previous
  iteration. If Sdepth != 0 then the program has correctly predicted the
  opponents move and the search will start at a depth of Sdepth+1 rather
  than a depth of 1.
*/
{
  static alpha, beta, score, tempb, tempc, tempsf, tempst, xside, rpt;
  register short i;
  
  timeout = false;
  xside = otherside[side];
  if (iop != 2)
    player = side;
  if (TCflag[side])
    {
      if ((TimeControl.moves[side] + 3) != 0)
	ResponseTime = (TimeControl.clock[side]) /
	  (TimeControl.moves[side] + 3) -
	  OperatorTime[side];
      else
	ResponseTime = 0;
      ResponseTime += (ResponseTime * TimeControl.moves[side]) / (2 * TCmoves[side] + 1);
    }
  else
    ResponseTime = Level[side];
  if (iop == 2)
    ResponseTime = 99999;
  if (Sdepth > 0 && root->score > Zscore - zwndw)
    ResponseTime -= ft;
  else if (ResponseTime < 1)
    ResponseTime = 1;
  ExtraTime = 0;
  ExaminePosition ();
  ScorePosition (side, &score);
  /* Pscore[0] = -score; */

  if (Sdepth == 0)
    {
#if ttblsz
      /* ZeroTTable (); */
#endif /* ttblsz */
      SearchStartStuff (side);
#ifdef NOMEMSET
      for (i = 0; i < 8192; i++)
	history[i] = 0;
#else
      memset ((char *) history, 0, sizeof (history));
#endif /* NOMEMSET */
      FROMsquare = TOsquare = -1;
      PV = 0;
      if (iop != 2)
		hint = 0;
      for (i = 0; i < maxdepth; i++)
		PrVar[i] = killr0[i] = killr1[i] = killr2[i] = killr3[i] = 0;
      alpha = score - 90;
      beta = score + 90;
      rpt = 0;
      TrPnt[1] = 0;
      root = &Tree[0];
      MoveList (side, 1);
      for (i = TrPnt[1]; i < TrPnt[2]; i++)
		pick (i, TrPnt[2] - 1);
      if (Book != NULL)
			OpeningBook ();
      if (Book != NULL)
			timeout = true;
      NodeCnt = ETnodes = EvalNodes = HashCnt = FHashCnt = HashCol = 0;
      Zscore = 0;
      zwndw = 20;
    }
	while (!timeout && Sdepth < MaxSearchDepth)
	{
      Sdepth++;
      score = search (side, 1, Sdepth, alpha, beta, PrVar, &rpt);
      for (i = 1; i <= Sdepth; i++)
		killr0[i] = PrVar[i];
      if (score < alpha)
	{
	  ExtraTime = 10 * ResponseTime;
	  /* ZeroTTable (); */
	  score = search (side, 1, Sdepth, -9000, score, PrVar, &rpt);
	}
      if (score > beta && !(root->flags & exact))
	{
	  ExtraTime = 0;
	  /* ZeroTTable (); */
	  score = search (side, 1, Sdepth, score, 9000, PrVar, &rpt);
	}
      score = root->score;
      if (!timeout)
	for (i = TrPnt[1] + 1; i < TrPnt[2]; i++)
	  pick (i, TrPnt[2] - 1);
      ShowResults (score, PrVar, '.');
      for (i = 1; i <= Sdepth; i++)
		killr0[i] = PrVar[i];
	if (score > Zscore - zwndw && score > Tree[1].score + 250)
		ExtraTime = 0;
      else if (score > Zscore - 3 * zwndw)
	ExtraTime = ResponseTime;
      else
	ExtraTime = 3 * ResponseTime;
      if (root->flags & exact)
	timeout = true;
      if (Tree[1].score < -9000)
	timeout = true;
      if (4 * et > 2 * ResponseTime + ExtraTime)
	timeout = true;
      if (!timeout)
		{
		  Tscore[0] = score;
	
		  Zscore = Zscore?(Zscore + score) / 2:score;
		}
      zwndw = 20 + abs (Zscore / 12);
      beta = score + Bwindow;
      if (Zscore < score)
	alpha = Zscore - Awindow - zwndw;
      else
	alpha = score - Awindow - zwndw;
    }

  if (donotplay) return;

  score = root->score;
  if (rpt >= 2 || score < -12000)
    root->flags |= draw;
  if (iop == 2)
    return;

  ElapsedTime (1);
  if (Book == NULL)
    hint = PrVar[2];

  if (score > -9999 && rpt <= 2)
    {
      MakeMove (side, root, &tempb, &tempc, &tempsf, &tempst);
      algbr (root->f, root->t, (short) root->flags);
    }
  else
    algbr (0, 0, 0);
    
  if (score == -9999 || score == 9998)
    mate = true;
  if (mate)
    hint = 0;
  if (root->flags & cstlmask)
    Game50 = GameCnt;
  else if (board[root->t] == pawn || (root->flags & capture))
    Game50 = GameCnt;
  GameList[GameCnt].score = score;
  GameList[GameCnt].nodes = NodeCnt;
  GameList[GameCnt].time = (short) et;
  GameList[GameCnt].depth = Sdepth;
  if (TCflag[side])
    {
      TimeControl.clock[side] -= (et + OperatorTime[side]);
      if (--TimeControl.moves[side] == 0)
					SetTimeControl (side);
    }
  if ((root->flags & draw) && bothsides)
    mate = true;
  if (GameCnt > 498)
    mate = true; /* out of move store, you loose */
  player = xside;
  Sdepth = 0;

  OutputMove ();
  return;
}

void
OpeningBook ()
/*
  Go thru each of the opening lines of play and check for a match with the
  current game listing. If a match occurs, generate a random number. If this
  number is the largest generated so far then the next move in this line
  becomes the current "candidate". After all lines are checked, the
  candidate move is put at the top of the Tree[] array and will be played by
  the program. Note that the program does not handle book transpositions.
*/
{
  short j, pnt;
  unsigned short m, *mp;
  unsigned r, r0;
  struct BookEntry *p;

  srand ((unsigned int) time0);
  r0 = m = 0;
  p = Book;
  while (p != NULL)
    {
      mp = p->mv;
      for (j = 0; j <= GameCnt; j++)
	if (GameList[j].gmove != *(mp++))
	  break;
      if (j > GameCnt)
	if ((r = rand ()) > r0)
	  {
	    r0 = r;
	    m = *mp;
	    hint = *(++mp);
	  }
      p = p->next;
    }

  for (pnt = TrPnt[1]; pnt < TrPnt[2]; pnt++)
    if (((Tree[pnt].f << 8) | Tree[pnt].t) == m)
      Tree[pnt].score = 0;
  pick (TrPnt[1], TrPnt[2] - 1);
  if (Tree[TrPnt[1]].score < 0)
    DisposBook();
}


inline void
repetition (cnt)
     short int *cnt;

/*
  Check for draw by threefold repetition.
*/

{
  register short i, c, f, t;
  short b[64];
  unsigned short m;

  *cnt = c = 0;
  if (GameCnt > Game50 + 3)
    {
#ifdef NOMEMSET
      for (i = 0; i < 64; b[i++] = 0) ;
#else
      memset ((char *) b, 0, sizeof (b));
#endif /* NOMEMSET */
      for (i = GameCnt; i > Game50; i--)
	{
	  m = GameList[i].gmove;
	  f = m >> 8;
	  t = m & 0xFF;
	  if (++b[f] == 0)
	    c--;
	  else
	    c++;
	  if (--b[t] == 0)
	    c--;
	  else
	    c++;
	  if (c == 0)
	    (*cnt)++;
	}
    }
}

int
search (side, ply, depth, alpha, beta, bstline, rpt)
     short int side;
     short int ply;
     short int depth;
     short int alpha;
     short int beta;
     short unsigned int *bstline;
     short int *rpt;

/*
  Perform an alpha-beta search to determine the score for the current board
  position. If depth <= 0 only capturing moves, pawn promotions and
  responses to check are generated and searched, otherwise all moves are
  processed. The search depth is modified for check evasions, certain
  re-captures and threats. Extensions may continue for up to 11 ply beyond
  the nominal search depth.
*/

#define UpdateSearchStatus \
{\
   if (post) ShowCurrentMove(pnt,node->f,node->t);\
     if (pnt > TrPnt[1])\
       {\
	  d = best-Zscore; e = best-node->score;\
	    if (best < alpha) ExtraTime = 10*ResponseTime;\
	    else if (d > -zwndw && e > 4*zwndw) ExtraTime = -ResponseTime/3;\
	    else if (d > -zwndw) ExtraTime = 0;\
	    else if (d > -3*zwndw) ExtraTime = ResponseTime;\
	    else if (d > -9*zwndw) ExtraTime = 3*ResponseTime;\
	    else ExtraTime = 5*ResponseTime;\
	    }\
	    }
#define prune (cf && score+node->score < alpha)
#define ReCapture (rcptr && score > alpha && score < beta &&\
		   ply > 2 && CptrFlag[ply-1] && CptrFlag[ply-2])
			/* && depth == Sdepth-ply+1 */
#define Parry (hung[side] > 1 && ply == Sdepth+1)
#define MateThreat (ply < Sdepth+4 && ply > 4 &&\
		    ChkFlag[ply-2] && ChkFlag[ply-4] &&\
		    ChkFlag[ply-2] != ChkFlag[ply-4])

{
  register short j, pnt;
  short best, tempb, tempc, tempsf, tempst;
  short xside, pbst, d, e, cf, score, rcnt;
  unsigned short mv, nxtline[maxdepth];
  struct leaf *node, tmp;

  NodeCnt++;
  xside = otherside[side];

  /* could this be removed ? */
  if (depth < 0)
    depth = 0;

  if (ply <= Sdepth + 3)
    repetition (rpt);
  else
    *rpt = 0;
  /* Detect repetitions a bit earlier. SMC. 12/89 */
  if (*rpt == 1 && ply > 1)
    return (0);
  /* if (*rpt >= 2) return(0); */

  score = evaluate (side, xside, ply, alpha, beta);
  if (score > 9000)
    {
      bstline[ply] = 0;
      return (score);
    }
  if (depth > 0)
    {
      /* Allow opponent a chance to check again */
      if (InChk) {
		if (depth < 2) depth = 2;
	}
      else if (PawnThreat[ply - 1] || ReCapture)
	++depth;
    }
  else
    {
      if (score >= alpha &&
	  (InChk || PawnThreat[ply - 1] || Parry))
	depth = 1;
      else if (score <= beta && MateThreat)
	depth = 1;
    }

#if ttblsz
  if (depth > 0 && hashflag && ply > 1)
    {
      if (ProbeTTable (side, depth, &alpha, &beta, &score) == false)
#ifdef HASHFILE	
	if ((depth > 4) && (GameCnt < 12) && hashfile)
	  ProbeFTable (side, depth, &alpha, &beta, &score);
#else
      /* do nothing */;
#endif /* HASHFILE */      
      bstline[ply] = PV;
      bstline[ply + 1] = 0;
      if (beta == -20000)
	return (score);
      if (alpha > beta)
	return (alpha);
    }
#endif /* ttblsz */
  d = (Sdepth == 1)?7:11;
  if (ply > Sdepth + d || (depth < 1 && score > beta))
    /* score >= beta ?? */
    return (score);

  if (ply > 1)
    if (depth > 0)
      MoveList (side, ply);
    else
      CaptureList (side, xside, ply);

  if (TrPnt[ply] == TrPnt[ply + 1])
    return (score);

  cf = (depth < 1 && ply > Sdepth + 1 && !ChkFlag[ply - 2] && !slk);

  if (depth > 0)
    best = -12000;
  else
    best = score;
  if (best > alpha)
    alpha = best;

  for (pnt = pbst = TrPnt[ply];
       pnt < TrPnt[ply + 1] && best <= beta;	/* best < beta ?? */
       pnt++)
    {
      if (ply > 1)
	pick (pnt, TrPnt[ply + 1] - 1);
      node = &Tree[pnt];
      mv = (node->f << 8) | node->t;
      nxtline[ply + 1] = 0;

      if (prune)
	break;
      if (ply == 1)
	UpdateSearchStatus;

      if (!(node->flags & exact))
	{
	  MakeMove (side, node, &tempb, &tempc, &tempsf, &tempst);
	  CptrFlag[ply] = (node->flags & capture);
	  PawnThreat[ply] = (node->flags & pwnthrt);
	  Tscore[ply] = node->score;
	  PV = node->reply;
	  node->score = -search (xside, ply + 1, depth - 1, -beta, -alpha,
				 nxtline, &rcnt);
	  if (abs (node->score) > 9000)
	    node->flags |= exact;
	  else if (rcnt == 1)
	    node->score /= 2;
	  if (rcnt >= 2 || GameCnt - Game50 > 99 ||
	      (node->score == 9999 - ply && !ChkFlag[ply]))
	    {
	      node->flags |= draw;
	      node->flags |= exact;
	      if (side == computer)
		node->score = contempt;
	      else
		node->score = -contempt;
	    }
	  node->reply = nxtline[ply + 1];
	  UnmakeMove (side, node, &tempb, &tempc, &tempsf, &tempst);
	}
      if (node->score > best && !timeout)
	{
	  if (depth > 0)
	    if (node->score > alpha && !(node->flags & exact))
	      node->score += depth;
	  best = node->score;
	  pbst = pnt;
	  if (best > alpha)
	    alpha = best;
	  for (j = ply + 1; nxtline[j] > 0; j++)
	    bstline[j] = nxtline[j];
	  bstline[j] = 0;
	  bstline[ply] = mv;
	  if (ply == 1)
	    {
	      if (best > root->score)
		{
		  tmp = Tree[pnt];
		  for (j = pnt - 1; j >= 0; j--)
		    Tree[j + 1] = Tree[j];
		  Tree[0] = tmp;
		  pbst = 0;
		}
	      if (Sdepth > 4)
		if (best > beta)
		  ShowResults (best, bstline, '+');
		else if (best < alpha)
		  ShowResults (best, bstline, '-');
		else
		  ShowResults (best, bstline, '&');
	    }
	}
      if (NodeCnt > ETnodes)
	ElapsedTime (0);
      if (timeout)
	return (-Tscore[ply - 1]);
    }

  node = &Tree[pbst];
  mv = (node->f << 8) | node->t;
#if ttblsz
  if (hashflag && ply <= Sdepth && *rpt == 0 && best == alpha)
    {
      PutInTTable (side, best, depth, alpha, beta, mv);
#ifdef HASHFILE      
      if ((depth > 4) && (GameCnt < 12) && hashfile)
	PutInFTable (side, best, depth, alpha, beta, node->f, node->t);
#endif /* HASHFILE */      
    }
#endif /* ttblsz */
  if (depth > 0)
    {
      j = (node->f << 6) | node->t;
      if (side == black)
	j |= 0x1000;
      if (history[j] < 150)
	history[j] += 2 * depth;
      if (node->t != (GameList[GameCnt].gmove & 0xFF))
	if (best <= beta)
	  killr3[ply] = mv;
	else if (mv != killr1[ply])
	  {
	    killr2[ply] = killr1[ply];
	    killr1[ply] = mv;
	  }
      if (best > 9000)
	killr0[ply] = mv;
      else
	killr0[ply] = 0;
    }
  return (best);
}

#if ttblsz
#define CB(i) ((color[2 * (i)] ? 0x80 : 0)\
	       | (board[2 * (i)] << 4)\
	       | (color[2 * (i) + 1] ? 0x8 : 0)\
	       | (board[2 * (i) + 1]))

int
ProbeTTable (side, depth, alpha, beta, score)
     short int side;
     short int depth;
     short int *alpha;
     short int *beta;
     short int *score;

/*
  Look for the current board position in the transposition table.
*/

{
  register struct hashentry *ptbl;
  register unsigned short i;

  ptbl = &ttable[side][hashkey & (real_ttblsz - 1)];

  /* rehash max rehash times */
  for (i = 1; ptbl->hashbd != hashbd && i <= rehash; i++)
    ptbl = &ttable[side][(hashkey + i) & (real_ttblsz - 1)];
  if (ptbl->depth >= depth && ptbl->hashbd == hashbd)
    {
      HashCnt++;
#ifdef HASHTEST
      for (i = 0; i < 32; i++)
	{
	  if (ptbl->bd[i] != CB(i))
	    {
	      HashCol++;
	      break;
	    }
	}
#endif /* HASHTEST */

      PV = ptbl->mv;
      if (ptbl->flags & truescore)
	{
	  *score = ptbl->score;
	  *beta = -20000;
	}
#if 0 /* commented out, why? */
      else if (ptbl->flags & upperbound)
	{
	  if (ptbl->score < *beta) *beta = ptbl->score+1;
	}
#endif
      else if (ptbl->flags & lowerbound)
	{
	  if (ptbl->score > *alpha)
	    *alpha = ptbl->score - 1;
	}
      return(true);
    }
  return(false);
}

void
PutInTTable (side, score, depth, alpha, beta, mv)
     short int side;
     short int score;
     short int depth;
     short int alpha;
     short int beta;
     short unsigned int mv;

/*
  Store the current board position in the transposition table.
*/

{
  register struct hashentry *ptbl;
  register unsigned short i;

  ptbl = &ttable[side][hashkey & (real_ttblsz - 1)];

  /* rehash max rehash times */
  for (i = 1; depth < ptbl->depth && ptbl->hashbd != hashbd && i <= rehash; i++)
    ptbl = &ttable[side][(hashkey + i) & (real_ttblsz - 1)];
  if (depth > ptbl->depth || ptbl->hashbd != hashbd)
    {
      ptbl->hashbd = hashbd;
      ptbl->depth = depth;
      ptbl->score = score;
      ptbl->mv = mv;
      ptbl->flags = 0;
      if (score < alpha)
	ptbl->flags |= upperbound;
      else if (score > beta)
	ptbl->flags |= lowerbound;
      else
	ptbl->flags |= truescore;
#ifdef HASHTEST
      for (i = 0; i < 32; i++)
	{
	  ptbl->bd[i] = CB(i);
	}
#endif /* HASHTEST */
    }
}

void
ZeroTTable ()
{
  register unsigned long side, i;

  if (hashflag)
    for (side = 0; side < 2; side++)
      for (i = 0; i < real_ttblsz; i++)
		ttable[side][i].depth = 0;
}

#ifdef HASHFILE
int
ProbeFTable(side, depth, alpha, beta, score)
     short int side;
     short int depth;
     short int *alpha;
     short int *beta;
     short int *score;

/*
  Look for the current board position in the persistent transposition table.
*/

{
  register unsigned short i, j;
  register unsigned long hashix;
  short s;
  struct fileentry new, t;

  if (side == white)
    hashix = hashkey & 0xFFFFFFFE & (filesz - 1);
  else
    hashix = hashkey | 1 & (filesz - 1);

  for (i = 0; i < 32; i++)
    new.bd[i] = CB(i);
  new.flags = 0;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[qrook[side]] == 0))
    new.flags |= queencastle;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[krook[side]] == 0))
    new.flags |= kingcastle;

  for (i = 0; i < frehash; i++)
    {
      long count;
      
      SetFPos(hashfile,
        fsFromStart,
	    sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1))
	    );
	  count = sizeof(struct fileentry);
      FSRead(hashfile, &count, &t);
      for (j = 0; j < 32; j++)
	if (t.bd[j] != new.bd[j])
	  break;
      if ((t.depth >= depth) && (j >= 32)
	  && (new.flags == (t.flags & (kingcastle | queencastle))))
	{
	  FHashCnt++;
	  PV = (t.f << 8) | t.t;
	  s = (t.sh << 8) | t.sl;
	  if (t.flags & truescore)
	    {
	      *score = s;
	      *beta = -20000;
	    }
	  else if (t.flags & lowerbound)
	    {
	      if (s > *alpha)
		*alpha = s - 1;
	    }
	  return(true);
	}
    }
  return(false);
}

void
PutInFTable (side, score, depth, alpha, beta, f, t)
     short int side;
     short int score;
     short int depth;
     short int alpha;
     short int beta;
     short unsigned int f;
     short unsigned int t;

/*
  Store the current board position in the persistent transposition table.
*/

{
  register unsigned short i;
  register unsigned long hashix;
  struct fileentry new, tmp;

  if (side == white)
    hashix = hashkey & 0xFFFFFFFE & (filesz - 1);
  else
    hashix = hashkey | 1 & (filesz - 1);

  for (i = 0; i < 32; i++)
    new.bd[i] = CB(i);
  new.f = f;
  new.t = t;
  new.flags = 0;
  if (score < alpha)
    new.flags |= upperbound;
  else if (score > beta)
    new.flags |= lowerbound;
  else
    new.flags |= truescore;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[qrook[side]] == 0))
    new.flags |= queencastle;
  if ((Mvboard[kingP[side]] == 0) && (Mvboard[krook[side]] == 0))
    new.flags |= kingcastle;
  new.depth = depth;
  new.sh = score >> 8;
  new.sl = score & 0xFF;

  for (i = 0; i < frehash; i++)
    {
      long count;
      
      SetFPos(hashfile,
      	fsFromStart,
	    sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1))
	    );
	  count = sizeof(struct fileentry);
      FSRead(hashfile, &count, &tmp);
       if (tmp.depth <= depth)
	{
      SetFPos(hashfile,
      	fsFromStart,
	    sizeof(struct fileentry) * ((hashix + 2 * i) & (filesz - 1))
	    );
	  count = sizeof(struct fileentry);
      FSWrite(hashfile, &count, &new);
	  break;
	}

    }
}
#endif /* HASHFILE */
#endif /* ttblsz */

#define Link(from,to,flag,s) \
{\
   node->f = from; node->t = to;\
   node->reply = 0;\
   node->flags = flag;\
   node->score = s;\
   ++node;\
   ++TrPnt[ply+1];\
}

inline void
LinkMove (ply, f, t, flag, xside)
     short int ply;
     short int f;
     short int t;
     short int flag;
     short int xside;

/*
  Add a move to the tree.  Assign a bonus to order the moves
  as follows:
  1. Principle variation
  2. Capture of last moved piece
  3. Other captures (major pieces first)
  4. Killer moves
  5. "history" killers
*/

{
  register short s, z;
  register unsigned short mv;
  register struct leaf *node;

  node = &Tree[TrPnt[ply + 1]];
  mv = (f << 8) | t;
  s = 0;
  if (mv == Swag0)
    s = 2000;
  else if (mv == Swag1)
    s = 60;
  else if (mv == Swag2)
    s = 50;
  else if (mv == Swag3)
    s = 40;
  else if (mv == Swag4)
    s = 30;
  z = (f << 6) | t;
  if (xside == white)
    z |= 0x1000;
  s += history[z];
  if (color[t] != neutral)
    {
      flag |= capture;
      if (t == TOsquare)
	s += 500;
      s += value[board[t]] - board[f];
    }
  if (board[f] == pawn)
    if (row (t) == 0 || row (t) == 7)
      {
	flag |= promote;
	s += 800;
	Link (f, t, flag | queen, s - 20000);
	s -= 200;
	Link (f, t, flag | knight, s - 20000);
	s -= 50;
	Link (f, t, flag | rook, s - 20000);
	flag |= bishop;
	s -= 50;
      }
    else if (row (t) == 1 || row (t) == 6)
      {
	flag |= pwnthrt;
	s += 600;
      }
    else if (t == epsquare)
      flag |= epmask;
  Link (f, t, flag, s - 20000);
}


inline void
GenMoves (ply, sq, side, xside)
     short int ply;
     short int sq;
     short int side;
     short int xside;

/*
  Generate moves for a piece. The moves are taken from the precalulated
  array nextpos/nextdir. If the board is free, next move is choosen from
  nextpos else from nextdir.
*/

{
  register short u, piece;
  register unsigned char *ppos, *pdir;

  piece = board[sq];
  ppos = nextpos[ptype[side][piece]][sq];
  pdir = nextdir[ptype[side][piece]][sq];
  if (piece == pawn)
    {
      u = ppos[sq];	/* follow no captures thread */
      if (color[u] == neutral)
	{
	  LinkMove (ply, sq, u, 0, xside);
	  u = ppos[u];
	  if (color[u] == neutral)
	    LinkMove (ply, sq, u, 0, xside);
	}
      u = pdir[sq];	/* follow captures thread */
      if (color[u] == xside || u == epsquare)
	LinkMove (ply, sq, u, capture, xside);
      u = pdir[u];
      if (color[u] == xside || u == epsquare)
	LinkMove (ply, sq, u, capture, xside);
    }
  else
    {
      u = ppos[sq];
      do
	{
	  if (color[u] == neutral)
	    {
	      LinkMove (ply, sq, u, 0, xside);
	      u = ppos[u];
	    }
	  else
	    {
	      if (color[u] == xside)
		LinkMove (ply, sq, u, capture, xside);
	      u = pdir[u];
	    }
      } while (u != sq);
    }
}

void
MoveList (side, ply)
     short int side;
     short int ply;

/*
  Fill the array Tree[] with all available moves for side to play. Array
  TrPnt[ply] contains the index into Tree[] of the first move at a ply.
*/

{
  register short i, xside, f;

  xside = otherside[side];
  TrPnt[ply + 1] = TrPnt[ply];
  if (PV == 0)
    Swag0 = killr0[ply];
  else
    Swag0 = PV;
  Swag1 = killr1[ply];
  Swag2 = killr2[ply];
  Swag3 = killr3[ply];
  Swag4 = 0;
  if (ply > 2)
    Swag4 = killr1[ply - 2];
  for (i = PieceCnt[side]; i >= 0; i--)
    GenMoves (ply, PieceList[side][i], side, xside);
  if (!castld[side])
    {
      f = PieceList[side][0];
      if (castle (side, f, f + 2, 0))
	{
	  LinkMove (ply, f, f + 2, cstlmask, xside);
	}
      if (castle (side, f, f - 2, 0))
	{
	  LinkMove (ply, f, f - 2, cstlmask, xside);
	}
    }
}

void
CaptureList (side, xside, ply)
     short int side;
     short int xside;
     short int ply;

/*
  Fill the array Tree[] with all available cature and promote moves for
  side to play. Array TrPnt[ply] contains the index into Tree[]
  of the first move at a ply.
*/

{
  register short u, sq;
  register unsigned char *ppos, *pdir;
  short i, piece, *PL, r7;
  struct leaf *node;

  TrPnt[ply + 1] = TrPnt[ply];
  node = &Tree[TrPnt[ply]];
  r7 = rank7[side];
  PL = PieceList[side];
  for (i = 0; i <= PieceCnt[side]; i++)
    {
      sq = PL[i];
      piece = board[sq];
      if (sweep[piece])
	{
	  ppos = nextpos[piece][sq];
	  pdir = nextdir[piece][sq];
	  u = ppos[sq];
	  do
	    {
	      if (color[u] == neutral)
		u = ppos[u];
	      else
		{
		  if (color[u] == xside)
		    Link (sq, u, capture,
			  value[board[u]] + svalue[board[u]] - piece);
		  u = pdir[u];
		}
	  } while (u != sq);
	}
      else
	{
	  pdir = nextdir[ptype[side][piece]][sq];
	  if (piece == pawn && row (sq) == r7)
	    {
	      u = pdir[sq];
	      if (color[u] == xside)
		Link (sq, u, capture | promote | queen, valueQ);
	      u = pdir[u];
	      if (color[u] == xside)
		Link (sq, u, capture | promote | queen, valueQ);
	      ppos = nextpos[ptype[side][piece]][sq];
	      u = ppos[sq]; /* also generate non capture promote */
	      if (color[u] == neutral)
		Link (sq, u, promote | queen, valueQ);
	    }
	  else
	    {
	      u = pdir[sq];
	      do
		{
		  if (color[u] == xside)
		    Link (sq, u, capture,
			  value[board[u]] + svalue[board[u]] - piece);
		  u = pdir[u];
	      } while (u != sq);
	    }
	}
    }
}


int
castle (side, kf, kt, iop)
     short int side;
     short int kf;
     short int kt;
     short int iop;

/* Make or Unmake a castling move. */

{
  register short rf, rt, t0, xside;

  xside = otherside[side];
  if (kt > kf)
    {
      rf = kf + 3;
      rt = kt - 1;
    }
  else
    {
      rf = kf - 4;
      rt = kt + 1;
    }
  if (iop == 0)
    {
      if (kf != kingP[side] ||
	  board[kf] != king ||
	  board[rf] != rook ||
	  Mvboard[kf] != 0 ||
	  Mvboard[rf] != 0 ||
	  color[kt] != neutral ||
	  color[rt] != neutral ||
	  color[kt - 1] != neutral ||
	  SqAtakd (kf, xside) ||
	  SqAtakd (kt, xside) ||
	  SqAtakd (rt, xside))
	return (false);

    }
  else
    {
      if (iop == 1)
	{
	  castld[side] = true;
	  Mvboard[kf]++;
	  Mvboard[rf]++;
	}
      else
	{
	  castld[side] = false;
	  Mvboard[kf]--;
	  Mvboard[rf]--;
	  t0 = kt;
	  kt = kf;
	  kf = t0;
	  t0 = rt;
	  rt = rf;
	  rf = t0;
	}
      board[kt] = king;
      color[kt] = side;
      Pindex[kt] = 0;
      board[kf] = no_piece;
      color[kf] = neutral;
      board[rt] = rook;
      color[rt] = side;
      Pindex[rt] = Pindex[rf];
      board[rf] = no_piece;
      color[rf] = neutral;
      PieceList[side][Pindex[kt]] = kt;
      PieceList[side][Pindex[rt]] = rt;
#if ttblsz
      if (hashflag)
	{
	  UpdateHashbd (side, king, kf, kt);
	  UpdateHashbd (side, rook, rf, rt);
	}
#endif /* ttblsz */
    }
  return (true);
}


inline void
EnPassant (xside, f, t, iop)
     short int xside;
     short int f;
     short int t;
     short int iop;

/*
  Make or unmake an en passant move.
*/

{
  register short l;

  if (t > f)
    l = t - 8;
  else
    l = t + 8;
  if (iop == 1)
    {
      board[l] = no_piece;
      color[l] = neutral;
    }
  else
    {
      board[l] = pawn;
      color[l] = xside;
    }
  InitializeStats ();
}


inline void
UpdatePieceList (side, sq, iop)
     short int side;
     short int sq;
     short int iop;

/*
  Update the PieceList and Pindex arrays when a piece is captured or when a
  capture is unmade.
*/

{
  register short i;
  if (iop == 1)
    {
      PieceCnt[side]--;
      for (i = Pindex[sq]; i <= PieceCnt[side]; i++)
	{
	  PieceList[side][i] = PieceList[side][i + 1];
	  Pindex[PieceList[side][i]] = i;
	}
    }
  else
    {
      PieceCnt[side]++;
      PieceList[side][PieceCnt[side]] = sq;
      Pindex[sq] = PieceCnt[side];
    }
}

void
MakeMove (side, node, tempb, tempc, tempsf, tempst)
     short int side;
     struct leaf *node;
     short int *tempb;
     short int *tempc;
     short int *tempsf;
     short int *tempst;

/*
  Update Arrays board[], color[], and Pindex[] to reflect the new board
  position obtained after making the move pointed to by node. Also update
  miscellaneous stuff that changes when a move is made.
*/

{
  register short f, t, xside, ct, cf;

  xside = otherside[side];
  f = node->f;
  t = node->t;
  epsquare = -1;
  FROMsquare = f;
  TOsquare = t;
  INCscore = 0;
  GameList[++GameCnt].gmove = (f << 8) | t;

  if (node->flags & cstlmask)
    {
      GameList[GameCnt].piece = no_piece;
      GameList[GameCnt].color = side;
      (void) castle (side, f, t, 1);
    }
  else
    {
      *tempc = color[t];
      *tempb = board[t];
      *tempsf = svalue[f];
      *tempst = svalue[t];
      GameList[GameCnt].piece = *tempb;
      GameList[GameCnt].color = *tempc;
      if (*tempc != neutral)
	{
	  UpdatePieceList (*tempc, t, 1);
	  if (*tempb == pawn)
	    --PawnCnt[*tempc][column (t)];
	  if (board[f] == pawn)
	    {
	      --PawnCnt[side][column (f)];
	      ++PawnCnt[side][column (t)];
	      cf = column (f);
	      ct = column (t);
	      if (PawnCnt[side][ct] > 1 + PawnCnt[side][cf])
		INCscore -= 15;
	      else if (PawnCnt[side][ct] < 1 + PawnCnt[side][cf])
		INCscore += 15;
	      else if (ct == 0 || ct == 7 || PawnCnt[side][ct + ct - cf] == 0)
		INCscore -= 15;
	    }
	  mtl[xside] -= value[*tempb];
	  if (*tempb == pawn)
	    pmtl[xside] -= valueP;
#if ttblsz
	  if (hashflag)
	    UpdateHashbd (xside, *tempb, -1, t);
#endif /* ttblsz */
	  INCscore += *tempst;
	  Mvboard[t]++;
	}
      color[t] = color[f];
      board[t] = board[f];
      svalue[t] = svalue[f];
      Pindex[t] = Pindex[f];
      PieceList[side][Pindex[t]] = t;
      color[f] = neutral;
      board[f] = no_piece;
      if (board[t] == pawn)
	if (t - f == 16)
	  epsquare = f + 8;
	else if (f - t == 16)
	  epsquare = f - 8;
      if (node->flags & promote)
	{
	  board[t] = node->flags & pmask;
	  if (board[t] == queen)
	    HasQueen[side]++;
	  else if (board[t] == rook)
	    HasRook[side]++;
	  else if (board[t] == bishop)
	    HasBishop[side]++;
	  else if (board[t] == knight)
	    HasKnight[side]++;
	  --PawnCnt[side][column (t)];
	  mtl[side] += value[board[t]] - valueP;
	  pmtl[side] -= valueP;
#if ttblsz
	  if (hashflag)
	    {
	      UpdateHashbd (side, pawn, f, -1);
	      UpdateHashbd (side, board[t], f, -1);
	    }
#endif /* ttblsz */
	  INCscore -= *tempsf;
	}
      if (node->flags & epmask)
	EnPassant (xside, f, t, 1);
      else
#if ttblsz
      if (hashflag)
	UpdateHashbd (side, board[t], f, t);
#endif /* ttblsz */
      Mvboard[f]++;
    }
}

void
UnmakeMove (side, node, tempb, tempc, tempsf, tempst)
     short int side;
     struct leaf *node;
     short int *tempb;
     short int *tempc;
     short int *tempsf;
     short int *tempst;

/*
  Take back a move.
*/

{
  register short f, t, xside;

  xside = otherside[side];
  f = node->f;
  t = node->t;
  epsquare = -1;
  GameCnt--;
  if (node->flags & cstlmask)
    (void) castle (side, f, t, 2);
  else
    {
      color[f] = color[t];
      board[f] = board[t];
      svalue[f] = *tempsf;
      Pindex[f] = Pindex[t];
      PieceList[side][Pindex[f]] = f;
      color[t] = *tempc;
      board[t] = *tempb;
      svalue[t] = *tempst;
      if (node->flags & promote)
	{
	  board[f] = pawn;
	  ++PawnCnt[side][column (t)];
	  mtl[side] += valueP - value[node->flags & pmask];
	  pmtl[side] += valueP;
#if ttblsz
	  if (hashflag)
	    {
	      UpdateHashbd (side, (short) node->flags & pmask, -1, t);
	      UpdateHashbd (side, pawn, -1, t);
	    }
#endif /* ttblsz */
	}
      if (*tempc != neutral)
	{
	  UpdatePieceList (*tempc, t, 2);
	  if (*tempb == pawn)
	    ++PawnCnt[*tempc][column (t)];
	  if (board[f] == pawn)
	    {
	      --PawnCnt[side][column (t)];
	      ++PawnCnt[side][column (f)];
	    }
	  mtl[xside] += value[*tempb];
	  if (*tempb == pawn)
	    pmtl[xside] += valueP;
#if ttblsz
	  if (hashflag)
	    UpdateHashbd (xside, *tempb, -1, t);
#endif /* ttblsz */
	  Mvboard[t]--;
	}
      if (node->flags & epmask)
	EnPassant (xside, f, t, 2);
      else
#if ttblsz
      if (hashflag)
	UpdateHashbd (side, board[f], f, t);
#endif /* ttblsz */
      Mvboard[f]--;
    }
}


#if ttblsz
void
UpdateHashbd (side, piece, f, t)
     short int side;
     short int piece;
     short int f;
     short int t;

/*
  hashbd contains a 32 bit "signature" of the board position. hashkey
  contains a 16 bit code used to address the hash table. When a move is
  made, XOR'ing the hashcode of moved piece on the from and to squares with
  the hashbd and hashkey values keeps things current.
*/

{
  if (f >= 0)
    {
      hashbd ^= hashcode[side][piece][f].bd;
      hashkey ^= hashcode[side][piece][f].key;
    }
  if (t >= 0)
    {
      hashbd ^= hashcode[side][piece][t].bd;
      hashkey ^= hashcode[side][piece][t].key;
    }
}

#endif /* ttblsz */

void
InitializeStats ()
/*
  Scan thru the board seeing what's on each square. If a piece is found,
  update the variables PieceCnt, PawnCnt, Pindex and PieceList. Also
  determine the material for each side and set the hashkey and hashbd
  variables to represent the current board position. Array
  PieceList[side][indx] contains the location of all the pieces of either
  side. Array Pindex[sq] contains the indx into PieceList for a given
  square.
*/
{
  register short i, sq;
  epsquare = -1;
  for (i = 0; i < 8; i++)
    PawnCnt[white][i] = PawnCnt[black][i] = 0;
  mtl[white] = mtl[black] = pmtl[white] = pmtl[black] = 0;
  PieceCnt[white] = PieceCnt[black] = 0;
#if ttblsz
  hashbd = hashkey = 0;
#endif /* ttblsz */
  for (sq = 0; sq < 64; sq++)
    if (color[sq] != neutral)
      {
	mtl[color[sq]] += value[board[sq]];
	if (board[sq] == pawn)
	  {
	    pmtl[color[sq]] += valueP;
	    ++PawnCnt[color[sq]][column (sq)];
	  }
	if (board[sq] == king)
	  Pindex[sq] = 0;
	else
	  Pindex[sq] = ++PieceCnt[color[sq]];
	PieceList[color[sq]][Pindex[sq]] = sq;
#if ttblsz
	hashbd ^= hashcode[color[sq]][board[sq]][sq].bd;
	hashkey ^= hashcode[color[sq]][board[sq]][sq].key;
#endif /* ttblsz */
      }
}


int
SqAtakd (sq, side)
     short int sq;
     short int side;

/*
  See if any piece with color 'side' ataks sq.  First check pawns then Queen,
  Bishop, Rook and King and last Knight.
*/

{
  register short u;
  register unsigned char *ppos, *pdir;
  short xside;

  xside = otherside[side];
  pdir = nextdir[ptype[xside][pawn]][sq];
  u = pdir[sq];		/* follow captures thread */
  if (u != sq)
    {
      if (board[u] == pawn && color[u] == side)
	return (true);
      u = pdir[u];
      if (u != sq && board[u] == pawn && color[u] == side)
	return (true);
    }
  /* king capture */
  if (distance (sq, PieceList[side][0]) == 1)
    return (true);
  /* try a queen bishop capture */
  ppos = nextpos[bishop][sq];
  pdir = nextdir[bishop][sq];
  u = ppos[sq];
  do
    {
      if (color[u] == neutral)
	u = ppos[u];
      else
	{
	  if (color[u] == side &&
	      (board[u] == queen || board[u] == bishop))
	    return (true);
	  u = pdir[u];
	}
  } while (u != sq);
  /* try a queen rook capture */
  ppos = nextpos[rook][sq];
  pdir = nextdir[rook][sq];
  u = ppos[sq];
  do
    {
      if (color[u] == neutral)
	u = ppos[u];
      else
	{
	  if (color[u] == side &&
	      (board[u] == queen || board[u] == rook))
	    return (true);
	  u = pdir[u];
	}
  } while (u != sq);
  /* try a knight capture */
  ppos = nextpos[knight][sq];
  pdir = nextdir[knight][sq];
  u = ppos[sq];
  do
    {
      if (color[u] == side && board[u] == knight)
	return (true);
      u = pdir[u];
  } while (u != sq);
  return (false);
}

inline void
ataks (side, a)
     short int side;
     short int *a;

/*
  Fill array atak[][] with info about ataks to a square.  Bits 8-15 are set
  if the piece (king..pawn) ataks the square.  Bits 0-7 contain a count of
  total ataks to the square.
*/

{
  register short u, c, sq;
  register unsigned char *ppos, *pdir;
  short i, piece, *PL;

#ifdef NOMEMSET
  for (u = 64; u; a[--u] = 0) ;
#else
  memset ((char *) a, 0, 64 * sizeof (a[0]));
#endif /* NOMEMSET */
  PL = PieceList[side];
  for (i = PieceCnt[side]; i >= 0; i--)
    {
      sq = PL[i];
      piece = board[sq];
      c = control[piece];
      if (sweep[piece])
	{
	  ppos = nextpos[piece][sq];
	  pdir = nextdir[piece][sq];
	  u = ppos[sq];
	  do
	    {
	      a[u] = ++a[u] | c;
	      u = (color[u] == neutral) ? ppos[u] : pdir[u];
	  } while (u != sq);
	}
      else
	{
	  pdir = nextdir[ptype[side][piece]][sq];
	  u = pdir[sq];	/* follow captures thread for pawns */
	  do
	    {
	      a[u] = ++a[u] | c;
	      u = pdir[u];
	  } while (u != sq);
	}
    }
}

/* ............    POSITIONAL EVALUATION ROUTINES    ............ */

int
evaluate (side, xside, ply, alpha, beta)
     short int side;
     short int xside;
     short int ply;
     short int alpha;
     short int beta;
/*
  Compute an estimate of the score by adding the positional score from the
  previous ply to the material difference. If this score falls inside a
  window which is 180 points wider than the alpha-beta window (or within a
  50 point window during quiescence search) call ScorePosition() to
  determine a score, otherwise return the estimated score. If one side has
  only a king and the other either has no pawns or no pieces then the
  function ScoreLoneKing() is called.
*/

{
  register short evflag;
  short s;

  s = -Pscore[ply - 1] + mtl[side] - mtl[xside] - INCscore;
  hung[white] = hung[black] = 0;

  if (slk = ((mtl[white] == valueK && (pmtl[black] == 0 || emtl[black] == 0)) ||
	 (mtl[black] == valueK && (pmtl[white] == 0 || emtl[white] == 0))))
    evflag = false;
  else
    evflag =
		(ply == 1 || ply < Sdepth ||
		((ply == Sdepth + 1 || ply == Sdepth + 2) &&
		(s > alpha - xwndw && s < beta + xwndw)) ||
		(ply > Sdepth + 2 && s >= alpha - 25 && s <= beta + 25));

  if (evflag)
    {
      EvalNodes++;
      ataks (side, atak[side]);
      if (atak[side][PieceList[xside][0]] > 0)
			return (10001 - ply);
      ataks (xside, atak[xside]);
      InChk = (atak[xside][PieceList[side][0]] > 0);
      ScorePosition (side, &s);
    }
  else
    {
      if (SqAtakd (PieceList[xside][0], side))
			return (10001 - ply);
      InChk = SqAtakd (PieceList[side][0], xside);
      if (slk)
			ScoreLoneKing (side, &s);
    }

  Pscore[ply] = s - mtl[side] + mtl[xside];
  if (InChk)
    ChkFlag[ply - 1] = Pindex[TOsquare];
  else
    ChkFlag[ply - 1] = 0;
  return (s);
}

void
ScorePosition (side, score)
     short int side;
     short int *score;
/*
  Perform normal static evaluation of board position. A score is generated
  for each piece and these are summed to get a score for each side.
*/

{
  register short sq, s, i, xside;
  short pscore[3];

  wking = PieceList[white][0];
  bking = PieceList[black][0];
  UpdateWeights ();
  xside = otherside[side];
  pscore[white] = pscore[black] = 0;

  for (c1 = white; c1 <= black; c1++)
    {
      c2 = otherside[c1];
      if (c1 == white)
			EnemyKing = bking;
      else
			EnemyKing = wking;
      atk1 = atak[c1];
      atk2 = atak[c2];
      PC1 = PawnCnt[c1];
      PC2 = PawnCnt[c2];
      for (i = 0; i <= PieceCnt[c1]; i++)
	{
	  sq = PieceList[c1][i];
	  s = SqValue (sq, side);
	  pscore[c1] += s;
	  svalue[sq] = s;
	}
    }
  if (hung[side] > 1)
    pscore[side] += HUNGX;
  if (hung[xside] > 1)
    pscore[xside] += HUNGX;

  *score = mtl[side] - mtl[xside] + pscore[side] - pscore[xside] + 10;
  if (dither)
    *score += rand () % dither;

  if (*score > 0 && pmtl[side] == 0)
    if (emtl[side] < valueR)
      *score = 0;
    else if (*score < valueR)
      *score /= 2;
  if (*score < 0 && pmtl[xside] == 0)
    if (emtl[xside] < valueR)
      *score = 0;
    else if (-*score < valueR)
      *score /= 2;

  if (mtl[xside] == valueK && emtl[side] > valueB)
    *score += 200;
  if (mtl[side] == valueK && emtl[xside] > valueB)
    *score -= 200;
}

void
ScoreLoneKing (side, score)
     short int side;
     short int *score;

/*
  Static evaluation when loser has only a king and winner has no pawns or no
  pieces.
*/

{
  register short winner, loser, king1, king2, s, i;

  UpdateWeights ();
  if (mtl[white] > mtl[black])
    winner = white;
  else
    winner = black;
  loser = otherside[winner];
  king1 = PieceList[winner][0];
  king2 = PieceList[loser][0];

  s = 0;

  if (pmtl[winner] > 0)
    for (i = 1; i <= PieceCnt[winner]; i++)
      s += ScoreKPK (side, winner, loser, king1, king2, PieceList[winner][i]);

  else if (emtl[winner] == valueB + valueN)
    s = ScoreKBNK (winner, king1, king2);

  else if (emtl[winner] > valueB)
    s = 500 + emtl[winner] - DyingKing[king2] - 2 * distance (king1, king2);

  if (side == winner)
    *score = s;
  else
    *score = -s;
}


int
ScoreKPK (side, winner, loser, king1, king2, sq)
     short int side;
     short int winner;
     short int loser;
     short int king1;
     short int king2;
     short int sq;

/*
  Score King and Pawns versus King endings.
*/

{
  register short s, r;

  if (PieceCnt[winner] == 1)
    s = 50;
  else
    s = 120;
  if (winner == white)
    {
      if (side == loser)
	r = row (sq) - 1;
      else
	r = row (sq);
      if (row (king2) >= r && distance (sq, king2) < 8 - r)
	s += 10 * row (sq);
      else
	s = 500 + 50 * row (sq);
      if (row (sq) < 6)
	sq += 16;
      else
	sq += 8;
    }
  else
    {
      if (side == loser)
	r = row (sq) + 1;
      else
	r = row (sq);
      if (row (king2) <= r && distance (sq, king2) < r + 1)
	s += 10 * (7 - row (sq));
      else
	s = 500 + 50 * (7 - row (sq));
      if (row (sq) > 1)
	sq -= 16;
      else
	sq -= 8;
    }
  s += 8 * (taxicab (king2, sq) - taxicab (king1, sq));
  return (s);
}


int
ScoreKBNK (winner, king1, king2)
     short int winner;
     short int king1;
     short int king2;


/*
  Score King+Bishop+Knight versus King endings.
  This doesn't work all that well but it's better than nothing.
*/

{
  register short s;

  s = emtl[winner] - 300;
  if (KBNKsq == 0)
    s += KBNK[king2];
  else
    s += KBNK[locn (row (king2), 7 - column (king2))];
  s -= taxicab (king1, king2);
  s -= distance (PieceList[winner][1], king2);
  s -= distance (PieceList[winner][2], king2);
  return (s);
}


inline void
BRscan (sq, s, mob)
     short int sq;
     short int *s;
     short int *mob;

/*
  Find Bishop and Rook mobility, XRAY attacks, and pins. Increment the
  hung[] array if a pin is found.
*/
{
  register short u, piece, pin;
  register unsigned char *ppos, *pdir;
  short *Kf;

  Kf = Kfield[c1];
  *mob = 0;
  piece = board[sq];
  ppos = nextpos[piece][sq];
  pdir = nextdir[piece][sq];
  u = ppos[sq];
  pin = -1;			/* start new direction */
  do
    {
      *s += Kf[u];
      if (color[u] == neutral)
	{
	  (*mob)++;
	  if (ppos[u] == pdir[u])
	    pin = -1;		/* oops new direction */
	  u = ppos[u];
	}
      else if (pin < 0)
	{
	  if (board[u] == pawn || board[u] == king)
	    u = pdir[u];
	  else
	    {
	      if (ppos[u] != pdir[u])
		pin = u;	/* not on the edge and on to find a pin */
	      u = ppos[u];
	    }
	}
      else
	{
	  if (color[u] == c2 && (board[u] > piece || atk2[u] == 0))
	    {
	      if (color[pin] == c2)
		{
		  *s += PINVAL;
		  if (atk2[pin] == 0 ||
		      atk1[pin] > control[board[pin]] + 1)
		    ++hung[c2];
		}
	      else
		*s += XRAY;
	    }
	  pin = -1;		/* new direction */
	  u = pdir[u];
	}
  } while (u != sq);
}

int
SqValue (sq, side)
     short int sq;
     short int side;

/*
  Calculate the positional value for the piece on 'sq'.
*/

{
  register short j, fyle, rank;
  short s, piece, a1, a2, in_square, r, mob, e, c;

  piece = board[sq];
  a1 = (atk1[sq] & 0x4FFF);
  a2 = (atk2[sq] & 0x4FFF);
  rank = row (sq);
  fyle = column (sq);
  s = 0;
  if (piece == pawn && c1 == white)
    {
      s = Mwpawn[sq];
      if (sq == 11 || sq == 12)
	if (color[sq + 8] != neutral)
	  s += PEDRNK2B;
      if ((fyle == 0 || PC1[fyle - 1] == 0) &&
	  (fyle == 7 || PC1[fyle + 1] == 0))
	s += ISOLANI[fyle];
      else if (PC1[fyle] > 1)
	s += PDOUBLED;
      if (a1 < ctlP && atk1[sq + 8] < ctlP)
	{
	  s += BACKWARD[a2 & 0xFF];
	  if (PC2[fyle] == 0)
	    s += PWEAKH;
	  if (color[sq + 8] != neutral)
	    s += PBLOK;
	}
      if (PC2[fyle] == 0)
	{
	  if (side == black)
	    r = rank - 1;
	  else
	    r = rank;
	  in_square = (row (bking) >= r && distance (sq, bking) < 8 - r);
	  if (a2 == 0 || side == white)
	    e = 0;
	  else
	    e = 1;
	  for (j = sq + 8; j < 64; j += 8)
	    if (atk2[j] >= ctlP)
	      {
		e = 2;
		break;
	      }
	    else if (atk2[j] > 0 || color[j] != neutral)
	      e = 1;
	  if (e == 2)
	    s += (stage * PassedPawn3[rank]) / 10;
	  else if (in_square || e == 1)
	    s += (stage * PassedPawn2[rank]) / 10;
	  else if (emtl[black] > 0)
	    s += (stage * PassedPawn1[rank]) / 10;
	  else
	    s += PassedPawn0[rank];
	}
    }
  else if (piece == pawn && c1 == black)
    {
      s = Mbpawn[sq];
      if (sq == 51 || sq == 52)
	if (color[sq - 8] != neutral)
	  s += PEDRNK2B;
      if ((fyle == 0 || PC1[fyle - 1] == 0) &&
	  (fyle == 7 || PC1[fyle + 1] == 0))
	s += ISOLANI[fyle];
      else if (PC1[fyle] > 1)
	s += PDOUBLED;
      if (a1 < ctlP && atk1[sq - 8] < ctlP)
	{
	  s += BACKWARD[a2 & 0xFF];
	  if (PC2[fyle] == 0)
	    s += PWEAKH;
	  if (color[sq - 8] != neutral)
	    s += PBLOK;
	}
      if (PC2[fyle] == 0)
	{
	  if (side == white)
	    r = rank + 1;
	  else
	    r = rank;
	  in_square = (row (wking) <= r && distance (sq, wking) < r + 1);
	  if (a2 == 0 || side == black)
	    e = 0;
	  else
	    e = 1;
	  for (j = sq - 8; j >= 0; j -= 8)
	    if (atk2[j] >= ctlP)
	      {
		e = 2;
		break;
	      }
	    else if (atk2[j] > 0 || color[j] != neutral)
	      e = 1;
	  if (e == 2)
	    s += (stage * PassedPawn3[7 - rank]) / 10;
	  else if (in_square || e == 1)
	    s += (stage * PassedPawn2[7 - rank]) / 10;
	  else if (emtl[white] > 0)
	    s += (stage * PassedPawn1[7 - rank]) / 10;
	  else
	    s += PassedPawn0[7 - rank];
	}
    }
  else if (piece == knight)
    {
      s = Mknight[c1][sq];
    }
  else if (piece == bishop)
    {
      s = Mbishop[c1][sq];
      BRscan (sq, &s, &mob);
      s += BMBLTY[mob];
    }
  else if (piece == rook)
    {
      s += RookBonus;
      BRscan (sq, &s, &mob);
      s += RMBLTY[mob];
      if (PC1[fyle] == 0)
	s += RHOPN;
      if (PC2[fyle] == 0)
	s += RHOPNX;
      if (rank == rank7[c1] && pmtl[c2] > 100)
	s += 10;
      if (stage > 2)
	s += 14 - taxicab (sq, EnemyKing);
    }
  else if (piece == queen)
    {
      if (stage > 2)
	s += 14 - taxicab (sq, EnemyKing);
      if (distance (sq, EnemyKing) < 3)
	s += 12;
    }
  else if (piece == king)
    {
      s = Mking[c1][sq];
      if (KSFTY > 0)
	if (Developed[c2] || stage > 0)
	  KingScan (sq, &s);
      if (castld[c1])
	s += KCASTLD;
      else if (Mvboard[kingP[c1]])
	s += KMOVD;

      if (PC1[fyle] == 0)
	s += KHOPN;
      if (PC2[fyle] == 0)
	s += KHOPNX;
      if (fyle == 1 || fyle == 2 || fyle == 3 || fyle == 7)
	{
	  if (PC1[fyle - 1] == 0)
	    s += KHOPN;
	  if (PC2[fyle - 1] == 0)
	    s += KHOPNX;
	}
      if (fyle == 4 || fyle == 5 || fyle == 6 || fyle == 0)
	{
	  if (PC1[fyle + 1] == 0)
	    s += KHOPN;
	  if (PC2[fyle + 1] == 0)
	    s += KHOPNX;
	}
      if (fyle == 2)
	{
	  if (PC1[0] == 0)
	    s += KHOPN;
	  if (PC2[0] == 0)
	    s += KHOPNX;
	}
      if (fyle == 5)
	{
	  if (PC1[7] == 0)
	    s += KHOPN;
	  if (PC2[7] == 0)
	    s += KHOPNX;
	}
    }
  if (a2 > 0)
    {
      c = (control[piece] & 0x4FFF);
      if (a1 == 0 || a2 > c + 1)
	{
	  s += HUNGP;
	  ++hung[c1];
	  if (piece != king && trapped (sq, piece))
	    ++hung[c1];
	}
      else if (piece != pawn || a2 > a1)
	if (a2 >= c || a1 < ctlP)
	  s += ATAKD;
    }
  return (s);
}

void
KingScan (sq, s)
     short int sq;
     short int *s;

/*
  Assign penalties if king can be threatened by checks, if squares
  near the king are controlled by the enemy (especially the queen),
  or if there are no pawns near the king.
  The following must be true:
  board[sq] == king
  c1 == color[sq]
  c2 == otherside[c1]
*/

#define ScoreThreat \
if (color[u] != c2)\
  if (atk1[u] == 0 || (atk2[u] & 0xFF) > 1) ++cnt;\
  else *s -= 3

{
  register short u;
  register unsigned char *ppos, *pdir;
  register short cnt, ok;

  cnt = 0;
  if (HasBishop[c2] || HasQueen[c2])
    {
      ppos = nextpos[bishop][sq];
      pdir = nextdir[bishop][sq];
      u = ppos[sq];
      do
	{
	  if (atk2[u] & ctlBQ)
	    ScoreThreat;
	  u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  if (HasRook[c2] || HasQueen[c2])
    {
      ppos = nextpos[rook][sq];
      pdir = nextdir[rook][sq];
      u = ppos[sq];
      do
	{
	  if (atk2[u] & ctlRQ)
	    ScoreThreat;
	  u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  if (HasKnight[c2])
    {
      pdir = nextdir[knight][sq];
      u = pdir[sq];
      do
	{
	  if (atk2[u] & ctlNN)
	    ScoreThreat;
	  u = pdir[u];
      } while (u != sq);
    }
  *s += (KSFTY * KTHRT[cnt]) / 16;

  cnt = 0;
  ok = false;
  pdir = nextpos[king][sq];
  u = pdir[sq];
  do
    {
      if (board[u] == pawn)
	ok = true;
      if (atk2[u] > atk1[u])
	{
	  ++cnt;
	  if (atk2[u] & ctlQ)
	    if (atk2[u] > ctlQ + 1 && atk1[u] < ctlQ)
	      *s -= 4 * KSFTY;
	}
      u = pdir[u];
  } while (u != sq);
  if (!ok)
    *s -= KSFTY;
  if (cnt > 1)
    *s -= KSFTY;
}


int
trapped (sq, piece)
     short int sq;
     short int piece;

/*
  See if the attacked piece has unattacked squares to move to.
  The following must be true:
  piece == board[sq]
  c1 == color[sq]
  c2 == otherside[c1]
*/

{
  register short u;
  register unsigned char *ppos, *pdir;

  ppos = nextpos[ptype[c1][piece]][sq];
  pdir = nextdir[ptype[c1][piece]][sq];
  if (piece == pawn)
    {
      u = ppos[sq];	/* follow no captures thread */
      if (color[u] == neutral)
	{
	  if (atk1[u] >= atk2[u])
	    return (false);
	  if (atk2[u] < ctlP)
	    {
	      u = ppos[u];
	      if (color[u] == neutral && atk1[u] >= atk2[u])
		return (false);
	    }
	}
      u = pdir[sq];	/* follow captures thread */
      if (color[u] == c2)
	return (false);
      u = pdir[u];
      if (color[u] == c2)
	return (false);
    }
  else
    {
      u = ppos[sq];
      do
	{
	  if (color[u] != c1)
	    if (atk2[u] == 0 || board[u] >= piece)
	      return (false);
	  u = (color[u] == neutral) ? ppos[u] : pdir[u];
      } while (u != sq);
    }
  return (true);
}


inline void
BlendBoard (a, b, c)
     const short int *a;
     const short int *b;
     short int *c;
{
  register int sq;

  for (sq = 0; sq < 64; sq++)
    c[sq] = (a[sq] * (10 - stage) + b[sq] * stage) / 10;
}


inline void
CopyBoard (a, b)
     const short int *a;
     short int *b;
{
  register int sq;

  for (sq = 0; sq < 64; sq++)
    b[sq] = a[sq];
}

void
ExaminePosition ()

/*
  This is done one time before the search is started. Set up arrays
  Mwpawn, Mbpawn, Mknight, Mbishop, Mking which are used in the
  SqValue() function to determine the positional value of each piece.
*/

{
  register short i, sq;
  short wpadv, bpadv, wstrong, bstrong, z, side, pp, j, k, val, Pd, fyle, rank;

  wking = PieceList[white][0];
  bking = PieceList[black][0];
  ataks (white, atak[white]);
  ataks (black, atak[black]);
  Zwmtl = Zbmtl = 0;
  UpdateWeights ();
  HasPawn[white] = HasPawn[black] = 0;
  HasKnight[white] = HasKnight[black] = 0;
  HasBishop[white] = HasBishop[black] = 0;
  HasRook[white] = HasRook[black] = 0;
  HasQueen[white] = HasQueen[black] = 0;
  for (side = white; side <= black; side++)
    for (i = 0; i <= PieceCnt[side]; i++)
      switch (board[PieceList[side][i]])
	{
	case pawn:
	  ++HasPawn[side];
	  break;
	case knight:
	  ++HasKnight[side];
	  break;
	case bishop:
	  ++HasBishop[side];
	  break;
	case rook:
	  ++HasRook[side];
	  break;
	case queen:
	  ++HasQueen[side];
	  break;
	}
  if (!Developed[white])
    Developed[white] = (board[1] != knight && board[2] != bishop &&
			board[5] != bishop && board[6] != knight);
  if (!Developed[black])
    Developed[black] = (board[57] != knight && board[58] != bishop &&
			board[61] != bishop && board[62] != knight);
  if (!PawnStorm && stage < 5)
    PawnStorm = ((column (wking) < 3 && column (bking) > 4) ||
		 (column (wking) > 4 && column (bking) < 3));

  CopyBoard (pknight, Mknight[white]);
  CopyBoard (pknight, Mknight[black]);
  CopyBoard (pbishop, Mbishop[white]);
  CopyBoard (pbishop, Mbishop[black]);
  BlendBoard (KingOpening, KingEnding, Mking[white]);
  BlendBoard (KingOpening, KingEnding, Mking[black]);

  for (sq = 0; sq < 64; sq++)
    {
      fyle = column (sq);
      rank = row (sq);
      wstrong = bstrong = true;
      for (i = sq; i < 64; i += 8)
	if (atak[black][i] >= ctlP)
	  {
	    wstrong = false;
	    break;
	  }
      for (i = sq; i >= 0; i -= 8)
	if (atak[white][i] >= ctlP)
	  {
	    bstrong = false;
	    break;
	  }
      wpadv = bpadv = PADVNCM;
      if ((fyle == 0 || PawnCnt[white][fyle - 1] == 0) &&
	  (fyle == 7 || PawnCnt[white][fyle + 1] == 0))
	wpadv = PADVNCI;
      if ((fyle == 0 || PawnCnt[black][fyle - 1] == 0) &&
	  (fyle == 7 || PawnCnt[black][fyle + 1] == 0))
	bpadv = PADVNCI;
      Mwpawn[sq] = (wpadv * PawnAdvance[sq]) / 10;
      Mbpawn[sq] = (bpadv * PawnAdvance[63 - sq]) / 10;
      Mwpawn[sq] += PawnBonus;
      Mbpawn[sq] += PawnBonus;
      if (Mvboard[kingP[white]])
	{
	  if ((fyle < 3 || fyle > 4) && distance (sq, wking) < 3)
	    Mwpawn[sq] += PAWNSHIELD;
	}
      else if (rank < 3 && (fyle < 2 || fyle > 5))
	Mwpawn[sq] += PAWNSHIELD / 2;
      if (Mvboard[kingP[black]])
	{
	  if ((fyle < 3 || fyle > 4) && distance (sq, bking) < 3)
	    Mbpawn[sq] += PAWNSHIELD;
	}
      else if (rank > 4 && (fyle < 2 || fyle > 5))
	Mbpawn[sq] += PAWNSHIELD / 2;
      if (PawnStorm)
	{
	  if ((column (wking) < 4 && fyle > 4) ||
	      (column (wking) > 3 && fyle < 3))
	    Mwpawn[sq] += 3 * rank - 21;
	  if ((column (bking) < 4 && fyle > 4) ||
	      (column (bking) > 3 && fyle < 3))
	    Mbpawn[sq] -= 3 * rank;
	}
      Mknight[white][sq] += 5 - distance (sq, bking);
      Mknight[white][sq] += 5 - distance (sq, wking);
      Mknight[black][sq] += 5 - distance (sq, wking);
      Mknight[black][sq] += 5 - distance (sq, bking);
      Mbishop[white][sq] += BishopBonus;
      Mbishop[black][sq] += BishopBonus;
      for (i = 0; i <= PieceCnt[black]; i++)
	if (distance (sq, PieceList[black][i]) < 3)
	  Mknight[white][sq] += KNIGHTPOST;
      for (i = 0; i <= PieceCnt[white]; i++)
	if (distance (sq, PieceList[white][i]) < 3)
	  Mknight[black][sq] += KNIGHTPOST;
      if (wstrong)
	Mknight[white][sq] += KNIGHTSTRONG;
      if (bstrong)
	Mknight[black][sq] += KNIGHTSTRONG;
      if (wstrong)
	Mbishop[white][sq] += BISHOPSTRONG;
      if (bstrong)
	Mbishop[black][sq] += BISHOPSTRONG;

      if (HasBishop[white] == 2)
	Mbishop[white][sq] += 8;
      if (HasBishop[black] == 2)
	Mbishop[black][sq] += 8;
      if (HasKnight[white] == 2)
	Mknight[white][sq] += 5;
      if (HasKnight[black] == 2)
	Mknight[black][sq] += 5;

      if (board[sq] == bishop)
	if (rank % 2 == fyle % 2)
	  KBNKsq = 0;
	else
	  KBNKsq = 7;

      Kfield[white][sq] = Kfield[black][sq] = 0;
      if (distance (sq, wking) == 1)
	Kfield[black][sq] = KATAK;
      if (distance (sq, bking) == 1)
	Kfield[white][sq] = KATAK;

      Pd = 0;
      for (k = 0; k <= PieceCnt[white]; k++)
	{
	  i = PieceList[white][k];
	  if (board[i] == pawn)
	    {
	      pp = true;
	      if (row (i) == 6)
		z = i + 8;
	      else
		z = i + 16;
	      for (j = i + 8; j < 64; j += 8)
		if (atak[black][j] > ctlP || board[j] == pawn)
		  {
		    pp = false;
		    break;
		  }
	      if (pp)
		Pd += 5 * taxicab (sq, z);
	      else
		Pd += taxicab (sq, z);
	    }
	}
      for (k = 0; k <= PieceCnt[black]; k++)
	{
	  i = PieceList[black][k];
	  if (board[i] == pawn)
	    {
	      pp = true;
	      if (row (i) == 1)
		z = i - 8;
	      else
		z = i - 16;
	      for (j = i - 8; j >= 0; j -= 8)
		if (atak[white][j] > ctlP || board[j] == pawn)
		  {
		    pp = false;
		    break;
		  }
	      if (pp)
		Pd += 5 * taxicab (sq, z);
	      else
		Pd += taxicab (sq, z);
	    }
	}
      if (Pd != 0)
	{
	  val = (Pd * stage2) / 10;
	  Mking[white][sq] -= val;
	  Mking[black][sq] -= val;
	}
    }
}

void
UpdateWeights ()

/*
  If material balance has changed, determine the values for the positional
  evaluation terms.
*/

{
  register short tmtl, s1;

  if (mtl[white] != Zwmtl || mtl[black] != Zbmtl)
    {
      Zwmtl = mtl[white];
      Zbmtl = mtl[black];
      emtl[white] = Zwmtl - pmtl[white] - valueK;
      emtl[black] = Zbmtl - pmtl[black] - valueK;
      tmtl = emtl[white] + emtl[black];
      s1 = (tmtl > 6600) ? 0 : ((tmtl < 1400) ? 10 : (6600 - tmtl) / 520);
      if (s1 != stage)
	{
	  stage = s1;
	  stage2 = (tmtl > 3600) ? 0 : ((tmtl < 1400) ? 10 : (3600 - tmtl) / 220);
	  PEDRNK2B = -15;	/* centre pawn on 2nd rank & blocked */
	  PBLOK = -4;		/* blocked backward pawn */
	  PDOUBLED = -14;	/* doubled pawn */
	  PWEAKH = -4;		/* weak pawn on half open file */
	  PAWNSHIELD = 10 - stage;	/* pawn near friendly king */
	  PADVNCM = 10;		/* advanced pawn multiplier */
	  PADVNCI = 7;		/* muliplier for isolated pawn */
	  PawnBonus = stage;

	  KNIGHTPOST = (stage + 2) / 3;	/* knight near enemy pieces */
	  KNIGHTSTRONG = (stage + 6) / 2;	/* occupies pawn hole */

	  BISHOPSTRONG = (stage + 6) / 2;	/* occupies pawn hole */
	  BishopBonus = 2 * stage;

	  RHOPN = 10;		/* rook on half open file */
	  RHOPNX = 4;
	  RookBonus = 6 * stage;

	  XRAY = 8;		/* Xray attack on piece */
	  PINVAL = 10;		/* Pin */

	  KHOPN = (3 * stage - 30) / 2;	/* king on half open file */
	  KHOPNX = KHOPN / 2;
	  KCASTLD = 10 - stage;
	  KMOVD = -40 / (stage + 1);	/* king moved before castling */
	  KATAK = (10 - stage) / 2;	/* B,R attacks near enemy king */
	  if (stage < 8)
	    KSFTY = 16 - 2 * stage;
	  else
	    KSFTY = 0;

	  ATAKD = -6;		/* defender > attacker */
	  HUNGP = -8;		/* each hung piece */
	  HUNGX = -12;		/* extra for >1 hung piece */
	}
    }
}


int
main ()
{
  int ahead = 0;
  int hash = 0;
  char *xwin = 0;
  int l;

  SetUpThing();
  SetupMenus();
  alloc_tables();

  Level[black] = 1;
  Level[white] = 1;
  TCflag[black] = true;
  TCflag[white] = true;
  OperatorTime[white] = 0;
  OperatorTime[black] = 0;
  Book = NULL;
  for (l = 0; l < 64; l++)
    {
      board[l] = Stboard[l];
      color[l] = Stcolor[l];
      Mvboard[l] = 0;
    }
  Initialize ();
  Initialize_dist ();
  Initialize_moves ();

  TCmoves[white] = 60;
  TCmoves[black] = 60;
  TCminutes[white] = 5;
  TCminutes[black] = 5;
  SetTimeControl (white);
  SetTimeControl (black);

  if (ahead)
    seteasy ();
  if (hash)
    hashflag = 1;
  if (xwin)
    xwndw = atoi (xwin);

#if ttblsz
#ifdef HASHFILE
  hashfile = 0;
#endif /* HASHFILE */
#endif  /* ttblsz */

  loop();
  
#if ttblsz
#ifdef HASHFILE
  if (hashfile) FSClose(hashfile);
#endif /* HASHFILE */
#endif /* ttblsz */

  ExitChess ();
  return (0);
}
