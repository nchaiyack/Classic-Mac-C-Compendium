/*
  Header file for GNU CHESS

  Copyright (C) 1986, 1987, 1988, 1989, 1990 Free Software Foundation, Inc.

  This file is part of CHESS.

  CHESS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.  No author or distributor
  accepts responsibility to anyone for the consequences of using it
  or for whether it serves any particular purpose or works at all,
  unless he says so in writing.  Refer to the CHESS General Public
  License for full details.

  Everyone is granted permission to copy, modify and redistribute
  CHESS, but only under the conditions described in the
  CHESS General Public License.   A copy of this license is
  supposed to have been given to you along with CHESS so you
  can know your rights and responsibilities.  It should be in a
  file named COPYING.  Among other things, the copyright notice
  and this notice must be preserved on all copies.
*/

#define MAC
/*
#define HASHFILE "\pGNU Table"
*/

#ifdef MAC
#include "rsrc.h"
#define malloc(x) NewPtr(x)
#define free(x) DisposPtr(x)
#define time(x) Ticks
#ifndef HASHFILE
#define rand() (unsigned)Random()
#define srand(x) qd.randSeed = x
#endif
#endif

#ifndef __STDC__
#define const
#endif

#ifndef __GNUC__
#define inline
#endif

#ifdef MSDOS
#endif

/* The following is just a hint, it`s not yet implemented */
#ifdef ENUM
enum colors { white, black, neutral };
enum pieces { no_piece, pawn, knight, bishop, rook, queen, king };
enum squares { a1, a2, a3, a4, a5, a6, a7, a8,
		 b1, b2, b3, b4, b5, b6, b7, b8,
		 c1, c2, c3, c4, c5, c6, c7, c8,
		 d1, d2, d3, d4, d5, d6, d7, d8,
		 e1, e2, e3, e4, e5, e6, e7, e8,
		 f1, f2, f3, f4, f5, f6, f7, f8,
		 g1, g2, g3, g4, g5, g6, g7, g8,
		 h1, h2 ,h3, h4, h5, h6, h7, h8 };
#else
#define white 0
#define black 1 
#define neutral 2
#define no_piece 0
#define pawn 1
#define knight 2
#define bishop 3
#define rook 4
#define queen 5
#define king 6
#endif /* ENUM */
#define pmask 0x0007
#define promote 0x0008
#define cstlmask 0x0010
#define epmask 0x0020
#define exact 0x0040
#define pwnthrt 0x0080
#define check 0x0100
#define capture 0x0200
#define draw 0x0400
#define maxdepth 30
#define true 1
#define false 0
/* #define absv(x) ((x) < 0 ? -(x) : (x)) */

struct leaf
  {
    short f,t,score,reply;
    unsigned short flags;
  };
struct GameRec
  {
    unsigned short gmove, flags;
    short score,depth,time,piece,color;
    long nodes;
  };
struct TimeControlRec
  {
    short moves[2];
    long clock[2];
  };
struct BookEntry
  {
    struct BookEntry *next;
    unsigned short *mv;
  };

extern struct leaf *Tree,*root;
extern short TrPnt[maxdepth],board[64],color[64];
extern short PieceList[2][16],PawnCnt[2][8];
extern short atak[2][64],*atk1,*atk2;
extern short castld[2],Mvboard[64];
extern short c1,c2,*PC1,*PC2;
extern short mate,post,opponent,computer,Sdepth,Awindow,Bwindow,dither;
extern long ResponseTime,ExtraTime,Level[2],et,et0,ft;
extern unsigned long time0;
extern long NodeCnt,ETnodes,EvalNodes,HashCnt,HashCol;
extern short quit,reverse,bothsides,hashflag,InChk,player,force,easy,beep;
extern short timeout,xwndw;
extern struct GameRec *GameList;
extern short GameCnt,Game50,epsquare,lpost,rcptr,contempt;
extern short MaxSearchDepth;
extern struct BookEntry *Book;
extern struct TimeControlRec TimeControl;
extern short TCflag[2],TCmoves[2],TCminutes[2],OperatorTime[2];
extern const short otherside[3];
extern const short Stboard[64];
extern const short Stcolor[64];
extern unsigned short hint,PrVar[maxdepth];

extern short donotplay;

#ifndef HZ
#define HZ 60
#endif

#define distance(a,b) distdata[a][b]
#define row(a) ((a) >> 3)
#define column(a) ((a) & 7)
#define locn(a,b) (((a) << 3) | b)
extern short *distdata[64];

/* gnuchess.c external functions */
extern void Initialize_moves ();
extern void NewGame ();
extern void SelectMove ();
extern void OpeningBook ();
extern void repetition ();
extern int search ();
#if ttblsz
extern int ProbeTTable ();
extern void PutInTTable ();
extern void ZeroTTable ();
#ifdef HASHFILE
extern int ProbeFTable ();
extern void PutInFTable ();
#endif /* HASHFILE */
extern void UpdateHashbd ();
#endif /* ttblsz */
extern void MoveList ();
extern void CaptureList ();
extern int castle ();
extern void MakeMove ();
extern void UnmakeMove ();
extern void InitializeStats ();
extern int SqAtakd ();
extern int evaluate ();
extern void ScorePosition ();
extern void ScoreLoneKing ();
extern int ScoreKPK ();
extern int ScoreKBNK ();
extern int SqValue ();
extern void KingScan ();
extern int trapped ();
extern void ExaminePosition ();
extern void UpdateWeights ();

/* *dsp.c external functions */
extern void Initialize ();
extern void seteasy ();
extern void InputCommand ();
extern void ExitChess ();
extern void ClrScreen ();
extern void SetTimeControl ();
extern void SelectLevel ();
extern void ElapsedTime ();
extern void GetOpenings ();
extern void ShowSidetomove ();
extern void SearchStartStuff ();
extern void ShowDepth ();
extern void ShowResults ();
extern void algbr ();
extern void OutputMove ();
extern void ShowCurrentMove ();
extern void ListGame ();
extern void ShowMessage ();
extern void ClrScreen ();
extern void gotoXY ();
extern void ClrEoln ();
extern void DrawPiece ();
extern void UpdateClocks ();
extern int parse ();
void InitUPPs(void);
