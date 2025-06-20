/*
  Mac interface for GNU Chess

  Revision: 10 Feb 1991

  Copyright (C) 1986, 1987, 1988 Free Software Foundation, Inc.
  Copyright (c) 1991  Airy ANDRE

	expanded game save, list, and restore features
	optional auto-updating of positional information

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

#include <stdio.h>
#include "math.h"

#include "DragMgr.h"
#include "gnuchess.h"
#include "macintf.h"
#include "rsrc.h"


char mvstr[5][6];
long evrate;

short PositionFlag = 0;
short coords = 1;
short stars = 0;
short rv = 1;
short shade = 0;
short preview = 0;
short idoit = 0;
short drawn = 0;
short undo = 0;
short anim = 1;
short showvalue = 0;
short newgame = 0;
short getgame = 0;
short wne;

char buff[100];

Boolean	Finished;
CursHandle	ClockCursor;
CursHandle	ArrowCursor;
CursHandle	CrossCursor;
Rect	DragArea;
Rect	GrowArea;
MenuHandle	Menu;
WindowPtr	WindBoard, WindList, WindThink;
int	width,height;

short background = 0;

Point	thePoint;
Rect	MinSize;
	
Rect	nameRec, ValueRec, ValueFRec,chessRectOff;

Rect	MsgRec, MsgFRec;
Rect	ThinkRec[2], ThinkFRec[2];
Rect 	ScoreRec, ScoreFRec;

int 	theScore;
Str255	Msg;

Str255	ThinkMove[2][30];
int		maxThink;

DragHandle	myDragStuff;
Point	pt;

Pattern blackPat, whitePat;

Rect	CaseRec, CaseFRec;
int		MouseX, MouseY;

int 	ref;
long	nbre;

short saveBoard[64], saveColor[64];

int towho;

ListHandle List;
Rect theBar;


void SetUpThing(void)
{	
	int typ,err,i,j;
	Handle Hitem;
	Rect r;

	FlushEvents(everyEvent,0);

	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MaxApplZone();

	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);

	ClockCursor=GetCursor(watchCursor);
	ArrowCursor=GetCursor(1000);
	HNoPurge((Handle)ClockCursor);
	HNoPurge((Handle)ArrowCursor);
	SetCursor(*ClockCursor);
	
	StuffHex(blackPat, "\pFFFFFFFFFFFFFFFF");
	StuffHex(whitePat, "\p0000000000000000");

#define WNETrapNum 0x60
#define UnImplTrapNum 0x9F

	wne = NGetTrapAddress(WNETrapNum, ToolTrap) ==
					NGetTrapAddress(UnImplTrapNum, ToolTrap);
}

void SetupMenus(void)
{
	MenuHandle MenuTopic ;
	int index;

	MenuTopic=GetMenu(AppleMenu);
	AddResMenu(MenuTopic,'DRVR');
	InsertMenu(MenuTopic,0);

	for (index=FileMenu; index<=OptionsMenu; index++)
			InsertMenu(GetMenu(index),0);

	InsertMenu(GetMenu(BlackMenu),-1);
	InsertMenu(GetMenu(WhiteMenu),-1);

	DrawMenuBar();
}


void
ShowPMessage (char *s)
{
  BlockMove(s, Msg, (long)*s + 1);
  UpdateMsg(NULL, 3);
}

void
ShowCMessage (char *s)
{
  *Msg = strlen(s);
  strcpy(Msg+1, s);
  UpdateMsg(NULL, 3);
}


int RemoveMove(int GameCnt)
{
  Cell theCell;

  theCell.v = GameCnt/2;

  if (!(GameCnt&1)) {
  	LDelRow(1, theCell.v, List);
  } else {
  	theCell.h = 1;
  	LClrCell(theCell, List);
  }
  
  LScroll(0, 10000, List);
}

int ShowError(int errno)
{
	ParamText(*GetString(errno), "", "", "");
	return CautionAlert(_ERROR_ALRT, NULL);
}

int AreYouSure(num)
{
	ParamText(*GetString(num), "", "", "");
	return CautionAlert(_CONFIRM_ALRT, NULL) == 2;
}

char choosePromo()
{
	int item;
    DialogPtr theDlog;
    
	theDlog = GetNewDialog(_PROMO_DLOG,(DialogPeek)0L,(WindowPtr)-1L);
	
	SetPort(theDlog);
	
	ModalDialog(NULL, &item);
	
	DisposDialog(theDlog);
	
	switch (item) {
		case 1: return 'Q';
		case 2: return 'N';
		case 3: return 'R';
		case 4: return 'B';
	}
}

int chooseTime(int *moves, int *minutes)
{
	int item;
    DialogPtr theDlog;
    Str255 str;
    long res;
    Handle hdl;
    Rect theRect;
    int type;
    
	theDlog = GetNewDialog(_TIME_DLOG,(DialogPeek)0L,(WindowPtr)-1L);
	
	GetDItem(theDlog, 6, &type, &hdl, &theRect);
	NumToString(*moves, str);
	SetIText(hdl, str);
	GetDItem(theDlog, 5, &type, &hdl, &theRect);
	NumToString(*minutes, str);
	SetIText(hdl, str);
	
	SetPort(theDlog);
	
	
	do {
		ModalDialog(NULL, &item);
	} while ((item != 1) && (item != 2));
	
	if (item == 1) {
		GetDItem(theDlog, 6, &type, &hdl, &theRect);
		GetIText(hdl, str);
		StringToNum(str, &res);
		*moves = res;
		GetDItem(theDlog, 5, &type, &hdl, &theRect);
		GetIText(hdl, str);
		StringToNum(str, &res);
		*minutes = res;
	}
	
	DisposDialog(theDlog);
	
	return (item==1);
}

int AddMove(int GameCnt, char *mvstr)
{
  Cell theCell;
  char mv[10];
 
  theCell.v = GameCnt/2;

  if (!(GameCnt&1)) {
    int num = theCell.v + 1;
    
  	theCell.h = 0;
  	LAddRow(1, theCell.v, List);
  	mv[0] = (num / 100)?(num/100 + '0'):' ';
  	mv[1] = ((num % 100) / 10)?((num % 100)/10 + '0'):' ';
  	mv[2] = num%10 + '0';
  	mv[3] = '.';
  	strcpy(mv + 4, mvstr);
  } else {
  	theCell.h = 1;
  	strcpy(mv, mvstr);
  }
  
  LSetCell((Ptr)mv, strlen(mv), theCell, List);
  LDraw(theCell, List);
  LScroll(0, 10000, List);
}


void
Initialize ()
{
 int	i,j;
 Rect	r;
 Handle Hitem;
 Cell Csize;
 Rect bound;
 
	SetRect(&DragArea,
			screenBits.bounds.left+4,
			screenBits.bounds.top+24,
			screenBits.bounds.right-4,
			screenBits.bounds.bottom-4);
	SetRect(&GrowArea,
			80,
			screenBits.bounds.top+60,
			screenBits.bounds.right,
			screenBits.bounds.bottom);
	
	WindBoard = GetNewDialog(770,(DialogPeek)0L,(WindowPtr)-1L);	
	SetPort(WindBoard);
	TextFont(geneva);
	TextSize(9);
	GetDItem(WindBoard, 1, &i, &Hitem, &chessRectOff);
	SetDItem(WindBoard, 1, i, (Handle)UpdateGraphic, &chessRectOff);
	width = 1+(chessRectOff.right-chessRectOff.left)/8;
	height = width;

	GetDItem(WindBoard, 2, &i, &Hitem, &CaseFRec);
	SetDItem(WindBoard, 2, i, (Handle)UpdateCase, &CaseFRec);
	CaseRec = CaseFRec;
	InsetRect(&CaseRec, 1, 1);
		
	
	WindThink = GetNewDialog(_THINK_DLOG,(DialogPeek)0L,(WindowPtr)-1L);
	SetPort(WindThink);
	TextFont(monaco);
	TextSize(9);
	
	GetDItem(WindThink, 1, &i, &Hitem, &nameRec);
	SetDItem(WindThink, 1, i,  (Handle)UpdateChronos, &nameRec);
	
	GetDItem(WindThink, 5, &i, &Hitem, &ValueFRec);
	SetDItem(WindThink, 5, i, (Handle)UpdateValue, &ValueFRec);
	ValueRec = ValueFRec;
	InsetRect(&ValueRec, 1, 1);
	
	GetDItem(WindThink, 4, &i, &Hitem, &MsgFRec);
	SetDItem(WindThink, 4, i, (Handle)UpdateMsg, &MsgFRec);
	MsgRec = MsgFRec;
	InsetRect(&MsgRec, 1, 1);
			
	GetDItem(WindThink, 2, &i, &Hitem, &ThinkFRec[0]);
	SetDItem(WindThink, 2, i, (Handle)UpdateThink, &ThinkFRec[0]);
	ThinkRec[0] = ThinkFRec[0];
	InsetRect(&ThinkRec[0], 1, 1);
			
	GetDItem(WindThink, 3, &i, &Hitem, &ThinkFRec[1]);
	SetDItem(WindThink, 3, i, (Handle)UpdateThink, &ThinkFRec[1]);
	ThinkRec[1] = ThinkFRec[1];
	InsetRect(&ThinkRec[1], 1, 1);
	
	maxThink = (ThinkRec[1].bottom - ThinkRec[1].top - 5)/12 - 1;
	
	WindList = GetNewDialog(768, (DialogPeek)0L, (WindowPtr)-1L);
	SetPort(WindList);
	TextSize(9);
	TextFont(monaco);
	GetDItem(WindList, 1, &i, &Hitem, &r);
	SetDItem(WindList, 1, i, (Handle)UpdateListe, &r);
	r.right -= 15;
	Csize.v=12;Csize.h=(r.right-r.left)/2;
	bound.top = 0;
	bound.left = 0;
	bound.right = 2;
	bound.bottom = 0;
	List=LNew(&r,&bound,Csize,0,WindList,TRUE,FALSE,FALSE,TRUE);

	theBar = r;
	theBar.left = r.right;
	theBar.right += 15;
	
   	(**List).selFlags=lOnlyOne;
					
	LAutoScroll(List);

	ShowWindow(WindList);
	
	SetPort(WindBoard);
	MouseX = MouseY = 0;
		
	UpdateChronos(NULL, 2);
	UpdateValue(NULL, 5);
	UpdateMsg(NULL, 3);
	UpdateCase(NULL, 4);
	
	SetPort(WindBoard);
	InitDrag(0L, &chessRectOff);
	UpdateDisplay(0,0,1,0,1,color,board);
	
	InitCursor();
}

void
ExitChess ()
{
  CloseDrag(0);
}


void
algbr (f, t, flag)
     short int f;
     short int t;
     short int flag;
/*
   Generate move strings in different formats, a hook has been provided for
   underpromotion
*/

{
  int m3p;

  if (f != t)
    {
      /* algebraic notation */
      mvstr[0][0] = cxx[column (f)];
      mvstr[0][1] = rxx[row (f)];
      mvstr[0][2] = cxx[column (t)];
      mvstr[0][3] = rxx[row (t)];
      mvstr[0][4] = '\0';
      mvstr[3][0] = '\0';
      if ((mvstr[1][0] = qxx[board[f]]) == 'P')
	{
	  if (mvstr[0][0] == mvstr[0][2])	/* pawn did not eat */
	    {
	      mvstr[2][0] = mvstr[1][0] = mvstr[0][2];	/* to column */
	      mvstr[2][1] = mvstr[1][1] = mvstr[0][3];	/* to row */
	      m3p = 2;
	    }
	  else
	    /* pawn ate */
	    {
	      mvstr[2][0] = mvstr[1][0] = mvstr[0][0];	/* from column */
	      mvstr[2][1] = mvstr[1][1] = mvstr[0][2];	/* to column */
	      mvstr[2][2] = mvstr[0][3];
	      m3p = 3;		/* to row */
	    }
	  mvstr[2][m3p] = mvstr[1][2] = '\0';
	  if (flag & promote)
	    {
	      mvstr[0][4] = mvstr[1][2] = mvstr[2][m3p] = qxx[flag & pmask];
	      mvstr[1][3] = mvstr[2][m3p + 1] = mvstr[0][5] = '\0';
	    }
	}
      else
	/* not a pawn */
	{
	  mvstr[2][0] = mvstr[1][0];
	  mvstr[2][2] = mvstr[1][1] = mvstr[0][2];	/* to column */
	  mvstr[2][3] = mvstr[1][2] = mvstr[0][3];	/* to row */
	  mvstr[2][4] = mvstr[1][3] = '\0';
	  mvstr[2][1] = mvstr[0][1];
	  strcpy (mvstr[3], mvstr[2]);
	  mvstr[3][1] = mvstr[0][0];
	  if (flag & cstlmask)
	    {
	      if (t > f)
		{
		  strcpy (mvstr[1], "o-o");
		  strcpy (mvstr[2], "O-O");
		}
	      else
		{
		  strcpy (mvstr[1], "o-o-o");
		  strcpy (mvstr[2], "O-O-O");
		}
	    }
	}
    }
  else
    mvstr[0][0] = mvstr[1][0] = mvstr[2][0] = mvstr[3][0] = '\0';
}

void
seteasy()
{
  easy = !easy;
}


int
VerifyMove (s, iop, mv)
     char *s;
     short int iop;
     short unsigned int *mv;
/*
   Compare the string 's' to the list of legal moves available for the
   opponent. If a match is found, make the move on the board.
*/
{
  static short pnt, tempb, tempc, tempsf, tempst, cnt;
  static struct leaf xnode;
  struct leaf *node;

  *mv = 0;
  if (iop == 2)
    {
      UnmakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
      return (false);
    }
  cnt = 0;
  MoveList (opponent, 2);
  pnt = TrPnt[2];
  while (pnt < TrPnt[3])
    {
      node = &Tree[pnt++];
      algbr (node->f, node->t, (short) node->flags);
      if (strcmp (s, mvstr[0]) == 0 || strcmp (s, mvstr[1]) == 0 ||
	  	strcmp (s, mvstr[2]) == 0 || strcmp (s, mvstr[3]) == 0)
		{
		  cnt++;
		  xnode = *node;
		}
    }
  if (cnt == 1)
    {
      MakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst, 0);
      if (SqAtakd (PieceList[opponent][0], computer))
		{
		  UnmakeMove (opponent, &xnode, &tempb, &tempc, &tempsf, &tempst);
		  return (false);
		}
      else
		{
		  if (iop == 1)
		    return (true);
		  if (xnode.flags & cstlmask)
		    Game50 = GameCnt;
		  else if (board[xnode.t] == pawn || (xnode.flags & capture))
		    Game50 = GameCnt;
		  GameList[GameCnt].depth = GameList[GameCnt].score = 0;
		  GameList[GameCnt].nodes = 0;
		  GameList[GameCnt].time = (short) et;
		  GameList[GameCnt].flags = xnode.flags;
		  TimeControl.clock[opponent] -= et;
		  --TimeControl.moves[opponent];
		  
		  ElapsedTime(1);

		  *mv = (xnode.f << 8) + xnode.t;
		  algbr (xnode.f, xnode.t, xnode.flags);

		  if (xnode.flags & epmask)
		    UpdateDisplay (0, 0, 1, 0, 1, color, board);
		  else
		    UpdateDisplay(xnode.f, xnode.t, 0, (short) xnode.flags & cstlmask, 0,
		    			   color, board);
		  return (true);
		}
    }
  return (false);
}

void
ShowResults (score, bstline, ch)
     short int score;
     short unsigned int *bstline;
     char ch;
{
  short d, e, ply;
  theScore = score;
  if (showvalue) UpdateValue(0L,5);
  if (post)
    {
      GrafPtr SavePort;
      int h,v;
      
 	  GetPort(&SavePort);
	  SetPort(WindThink);
	  TextMode(srcCopy);

	  d = 0;
	  
	  h = ThinkRec[computer].left+2;
	  v = ThinkRec[computer].top+15;
	  
      for (ply = 1; bstline[ply] > 0; ply++)
	{
	  algbr (bstline[ply] >> 8, bstline[ply] & 0xFF, false);
	  strcpy(ThinkMove[computer][d]+1, mvstr[0]);
	  ThinkMove[computer][d][0] = strlen(mvstr[0]);
	  MoveTo(h,v);
	  DrawString(ThinkMove[computer][d]);
	  if (++d>maxThink) break;
	  v += 12;
	}
      lpost = d;
      while (d <= maxThink)
	{
	  MoveTo(h,v);
	  ThinkMove[computer][d++][0] = 0;
	  v += 12;
	  DrawString("\p       ");
	}
	SetPort(SavePort);
    }
}


void
SearchStartStuff (side)
     short int side;
{
  short i;
  
  SetPort(WindThink);
  ThinkMove[side][0][0] = 0;
  EraseRect(&ThinkRec[side]);
}

void
OutputMove ()
{
  int i;
 
  ShowCMessage(""); 
  if (SqAtakd (PieceList[!towho][0], towho))
  {
    ShowPMessage(*(char **)GetString(_CHECK_STR));
  }
  
  AddMove(GameCnt, mvstr[(root->flags & cstlmask) != 0]);
  
  for (i=0; i<64; i++) {
  	saveBoard[i] = board[i];
  	saveColor[i] = color[i];
  }
  
  if (root->flags & epmask)
    UpdateDisplay (0, 0, 1, 0, 1, color, board);
  else
    UpdateDisplay (root->f, root->t, 0, root->flags & cstlmask, 1, color, board);

  if (root->flags & draw) {
    ShowPMessage(*(char **)GetString(_DRAWN_STR));
    drawn = true;
  }
  else if (root->score == -9999) {
    ShowPMessage(*(char **)GetString(_MATE_STR));
  }
  else if (root->score == 9998) {
    ShowPMessage(*(char **)GetString(_MATE_STR));
  }
  else if (root->score < -9000) {
    ShowPMessage(*(char **)GetString(_SOON_STR));
  }
  else if (root->score > 9000) {
    ShowPMessage(*(char **)GetString(_SOON_STR));
  }
}

void
ElapsedTime(iop)
     short int iop;
/*
  Determine the time that has passed since the search was started. If
  the elapsed time exceeds the target (ResponseTime+ExtraTime) then set
  timeout to true which will terminate the search.
*/
{
  static unsigned long LastClick = 0;
  
  et = (Ticks - time0)/60;
  if (et < 0) {
  	et = 0;
  }
  if (((Ticks-LastClick)>10) && (towho == computer || bothsides || preview)) {
  	if (mainLoop()) timeout = true;
  	LastClick = Ticks;
  }

  ETnodes += 50;
  if (et > et0 || iop == 1)
    {
      if (et > ResponseTime + ExtraTime && Sdepth > 1)
			timeout = true;
      et0 = et;
      if (iop == 1)
	  {
			time0 = Ticks;
			et0 = 0;
	  }
      if (et > 0)
			/* evrate is Nodes / cputime */
			evrate = NodeCnt / (et + ft);
      else	evrate = 0;
      
      ETnodes = NodeCnt + 50;
	  UpdateClocks ();
    }
    
}

void
UpdateClocks ()
{	
	static unsigned long LastDraw = 0;
	static int LastColor = black;
	
	if (LastColor!=towho || Ticks-LastDraw>45) {
			UpdateChronos(NULL, 0);
			LastDraw = Ticks; LastColor = towho;
	}
}


void
SetTimeControl (color)
{
  if (TCflag[color])
    {
      TimeControl.moves[color] = TCmoves[color];
      TimeControl.clock[color] = 60 * (long) TCminutes[color];
    }
  else
    {
      TimeControl.moves[color] = 0;
      TimeControl.clock[color] = 0;
      Level[color] = 60 * (long) TCminutes[color];
    }
  et = 0;
  ElapsedTime (1);
}


void
Undo ()
/*
  Undo the most recent half-move.
  */
{
  short f, t, i;
  
  RemoveMove(GameCnt);
  f = GameList[GameCnt].gmove >> 8;
  t = GameList[GameCnt].gmove & 0xFF;
  if (board[t] == king && distance (t, f) > 1)
    {
    	castle (GameList[GameCnt].color, f, t, 2);
	  	DrawPieces (PieceToNum(color[t], board[t]), t);
	  	DrawPieces (PieceToNum(color[f], board[f]), f);	
	  	if (distance(t,f) == 2) {
	  		DrawPieces (PieceToNum(color[t+1], board[t+1]), t+1);
	  		DrawPieces (PieceToNum(color[t-1], board[t-1]), t-1);
	  	} else {
	  		DrawPieces (PieceToNum(color[t+1], board[t+1]), t+1);
	  		DrawPieces (PieceToNum(color[t-2], board[t-2]), t-2);
	  	}	
    }
  else
    {
      board[f] = board[t];
      color[f] = color[t];
      board[t] = GameList[GameCnt].piece;
      color[t] = GameList[GameCnt].color;
	  DrawPieces(PieceToNum(color[t], board[t]), t);
	  DrawPieces(PieceToNum(color[f], board[f]), f);
    }
  if (TCflag[color[f]])
    ++TimeControl.moves[color[f]];
  GameCnt--;
  drawn = mate = false;
  Sdepth = 0;
  Mvboard[f]--;
  for (i=0; i<64; i++) {
  	saveColor[i] = color[i];
  	saveBoard[i] = board[i];
  }
  InitializeStats ();
}

void
ShowCurrentMove (pnt, f, t)
     short int pnt;
     short int f;
     short int t;
{
}


void
GiveHint ()
{
  char s[40];
  int f, t;
  
  if (!hint) return;
  
  f = hint >> 8;
  t = hint & 0xFF;
  
  algbr ((short) f, (short) t, false);
  ShowCMessage (mvstr[0]);
  AnimePieces(0,PieceToNum(saveColor[f], saveBoard[f]),f, t, 1);
  AnimePieces(PieceToNum(saveColor[t], saveBoard[t]),
      		  PieceToNum(saveColor[f], saveBoard[f]),
      			  t, f, 1);
}


void
InputCommand ()
/*
  Process the users command. If easy mode is OFF (the computer is
  thinking on opponents time) and the program is out of book, then make
  the 'hint' move on the board and call SelectMove() to find a response.
  The user terminates the search by entering ^C (quit siqnal) before
  entering a command. If the opponent does not make the hint move, then
  set Sdepth to zero.
  */
{
  short ok, i, tmp;
  long cnt, rate, t1, t2;
  unsigned short mv;
  char s[80];

  ok = quit = false;

  player = opponent = towho;
  computer = !towho;
  
  ft = 0;
  if (hint > 0 && !easy && Book == NULL)
    {
      preview = 1; idoit = 0;
      time0 = Ticks;
      algbr (hint >> 8, hint & 0xFF, false);
      strcpy (s, mvstr[0]);
      tmp = epsquare;
      if (VerifyMove (s, 1, &mv))
	  {
			SelectMove (computer, 2);
			VerifyMove (mvstr[0], 2, &mv);
			if (Sdepth > 0)
				Sdepth--;
	  }
      ft = (Ticks - time0)/60;
      epsquare = tmp;
      preview = 0;
    }

  ok = idoit;
  while (!(ok || quit))
  {
  	  ElapsedTime(0);
      ok = mainLoop();
  }
  ElapsedTime(1);
  ShowCMessage("");
}

static void mouseToXY(Point pt, int *OuX, int *OuY)
{
	*OuX = (pt.h/(width-1))+1;
	*OuX = scrntoxy(*OuX);
	*OuY= 8 - (pt.v/(height-1));
	*OuY = scrntoxy(*OuY);
}


static void followMouse(int *x1, int *y1, Point *Dum)
{
	do {
		GetMouse(Dum);
		PicInRect(&chessRectOff, myDragStuff, Dum);
		mouseToXY(*Dum,x1,y1);
		if ((MouseX!= *x1) || (MouseY != *y1))
		{
			MouseX = *x1; MouseY = *y1;
			UpdateCase(0L,4);
		}
		else
		{
			MouseX = *x1; MouseY = *y1;
		}
		DragItTo(myDragStuff,*Dum,1, FALSE);
     	ElapsedTime (0);
	} while (StillDown());
}

static void playTheMove(int sq, int x1, int y1, Point Dum)
{
	Point theEnd;
	Rect r;
	
	theEnd = CenterRect(Rectangle(scrntoxy(x1),scrntoxy(9-y1), &r));
	DrawPieces(no_piece, sq);
	DragFromTo(myDragStuff, Dum, theEnd);
	DisposeDraggable(myDragStuff);
}



int MouseInContent(Point MouseLoc, int *eatit)			
{
	int x,y,x1,y1,i,j,ok;
	
	ok = 0;
	if (FrontWindow()==WindList) {
		SetPort(WindList);
		GlobalToLocal(&MouseLoc);
		if (PtInRect(MouseLoc, &theBar)) LClick(MouseLoc, 0, List);
		LocalToGlobal(&MouseLoc);
	} else
	if ((FrontWindow()==WindBoard) && (((opponent == towho) && (!bothsides))
									|| force) )
	{
		SetPort(WindBoard);
		GlobalToLocal(&MouseLoc);
		mouseToXY(MouseLoc,&x,&y);
		if (y>=1 && y<=8)
 		{
 			Rect r;
 			int Num;
 			
	 		int sq = (y-1)*8+x-1;
	 		int sq1;
	 		
	 		MouseX=x;MouseY=y; UpdateCase(0L,4);
			SetRect(&r,0,0,8*(width-1),8*(height-1));
			if ((saveBoard[sq]!=no_piece)&&(saveColor[sq]==towho))
			{
				Point Dum;
				PicHandle h1, h2;
				char s[6];
				unsigned int mv;
				
				if (preview) {
					*eatit = false;
					return 1;
				}

		 		Num=PieceToNum(saveColor[sq],saveBoard[sq]);
				DrawPieces(no_piece ,sq);
				
				Dum=MouseLoc;
				h1=GetPicture(Num);
				HNoPurge((Handle)(h1));
				h2=GetPicture(Num-1+Num%4);
				HNoPurge((Handle)(h1));
		
				NewDraggable(h1,h2,0L,0L, &myDragStuff);
		
				HPurge((Handle)(h1));
				HPurge((Handle)(h2));
		
			 	ShadowStuff.visible = 1;
			 	ShadowStuff.dx = 4;
			 	ShadowStuff.dy = 4;
		
				followMouse(&x1, &y1, &Dum);

				sq1 = (y1-1)*8+x1-1;
				if ((saveBoard[sq]==king) && (x1-x == 2)) strcpy(s,"o-o");
				else 
				if ((saveBoard[sq]==king) && (x-x1 == 3)) strcpy(s,"o-o-o");
				else {
					s[0] = x+'a'-1;
					s[1] = y+'0';
					s[2] = x1+'a'-1;
					s[3] = y1+'0';
					if (saveBoard[sq] == pawn && (y1 == 8 || y1 == 1)) {
							s[4] = choosePromo();
							s[5] = 0;
					} else  s[4] = 0;
				}
				
				if (ok = VerifyMove(s, 1, &mv)) {
      				if (mv != hint)
					{
					  Sdepth = 0;
					  ft = 0;
					}
				    VerifyMove (s, 2, &mv);
					playTheMove(sq, x1, y1, Dum);
				    VerifyMove (s, 0, &mv);
					DrawPieces(PieceToNum(color[sq1], board[sq1]), sq1);
					AddMove(GameCnt, s);
				} else /* Not valid... Go back home */
					{
						Rect r;
						Point theEnd;
						theEnd = CenterRect(Rectangle(scrntoxy(x),scrntoxy(9-y), &r));
						DragFromTo(myDragStuff,Dum,theEnd);
						DisposeDraggable(myDragStuff);
					}
		} else while (StillDown());
		LocalToGlobal(&MouseLoc);
	}
}
	return ok;
}

int ProcessMenu_in(long CodeWord)
{
	MenuHandle MenuTopic;
	int Menu_No;
	int Item_No;
	Str255 NameHolder;
	int wind,i,Err;
	Rect nulrec;
	Handle ItemHdl;
	
	int ok = 0;
	if (CodeWord)
		{
		int change = false;
		Menu_No=HiWord(CodeWord);
		Item_No=LoWord(CodeWord);

		switch (Menu_No) {

			case AppleMenu:
				if (Item_No==1) {
					Alert(_ABOUT_ALRT, NULL);
				}
				else {
					GetItem(GetMHandle(AppleMenu),Item_No,NameHolder);
					OpenDeskAcc(NameHolder);
				}
				break;
			case FileMenu:
				switch (Item_No) {
				case 1:
					if (!AreYouSure(_END_STR)) break;
					newgame = 1;
					donotplay = 1;
					ok = true;
					break;
				case 2:
					if (!AreYouSure(_END_STR)) break;
					getgame = 1;
					donotplay = 1;
					ok = true;
					break;
				case 3:
					SaveGame();
					break;
				case 5:
					ListGame();
					break;
				case 7:
					if (!AreYouSure(_END_STR)) break;
					donotplay = 1;
					quit = true;
					ok = true;
					break;
				}
				break;

			case EditMenu:
				if (!SystemEdit(Item_No-1))
					switch (Item_No) {
						case 1:
							if (GameCnt >= 0) {
								undo = 1;
								donotplay = 1;
								Sdepth = 0;
								ok = 1;
							}
							break;
				}
				break;
			case PlayerMenu:
				switch (Item_No) {
					case 1:
						donotplay = true;
						bothsides = false;
						computer = black;
						opponent = white;
						ok = true;
						force = false;
						Sdepth = 0;
						break;
					case 2:
						donotplay = true;
						bothsides = false;
						computer = white;
						opponent = black;
						ok = true;
						force = false;
						Sdepth = 0;
						break;
					case 3:
						donotplay = true;
						bothsides = true;
						computer = towho;
						opponent = !towho;
						Sdepth = 0;
						ok = true;
						force = false;
						break;
					case 4:
						force = true;
						bothsides = false;
						donotplay = true;
						computer = !towho;
						opponent = towho;
						Sdepth = 0;
						ok = true;
						break;
				}
				if (Item_No>0 && Item_No<5)
			  		for (i=1; i<=4; i++)
			  			CheckItem(GetMHandle(Menu_No), i, i==Item_No);
				break;
			case WhiteMenu:
			case BlackMenu:
			  switch (Item_No)
			    {
			      
			    case 1:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 60;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 5;
			      change = true;
			      break;
			    case 2:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 60;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 15;
			      change = true;
			      break;
			    case 3:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 60;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 30;
			      change = true;
			      break;
			    case 4:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 40;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 30;
			      change = true;
			      break;
			    case 5:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 40;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 60;
			      change = true;
			      break;
			    case 6:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 40;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 120;
			      change = true;
			      break;
			    case 7:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 40;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 240;
			      change = true;
			      break;
			    case 8:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 1;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 15;
			      change = true;
			      break;
			    case 9:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 1;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 60;
			      change = true;
			      break;
			    case 10:
			      TCmoves[Menu_No==WhiteMenu?white:black] = 1;
			      TCminutes[Menu_No==WhiteMenu?white:black] = 600;
			      change = true;
			      break;
			    case 12:
			      change = chooseTime((int *) (&TCmoves[Menu_No==WhiteMenu?white:black])
			      				,(int *) (&TCminutes[Menu_No==WhiteMenu?white:black]));
			      break;
			    }
			
			  if (change) {
				  for (i=1; i<=12; i++)
				  	CheckItem(GetMHandle(Menu_No), i, i==Item_No);
				  SetTimeControl (Menu_No==WhiteMenu?white:black);
			  }
			  break;
			case OptionsMenu:
				switch (Item_No) {
					case 1:
						easy = !easy;
						CheckItem(GetMHandle(Menu_No), Item_No, easy);
						break;
					case 2:
						reverse = !reverse;
						UpdateDisplay(0, 0, 1, 0, 1, saveColor, saveBoard);
						CheckItem(GetMHandle(Menu_No), Item_No, reverse);
						break;
					case 3:
						anim = !anim;
						CheckItem(GetMHandle(Menu_No), Item_No, anim);
						break;
					case 4:
						dither = dither?0:6;
						CheckItem(GetMHandle(Menu_No), Item_No, dither!=0);
						break;
					case 5:
						post = !post;
						CheckItem(GetMHandle(Menu_No), Item_No, post);
						break;
					case 6:
						showvalue = !showvalue;
						CheckItem(GetMHandle(Menu_No), Item_No, showvalue);
						break;
#ifdef HASHFILE
					case 7:
						hashflag = !hashflag;
						if (hashflag) hashflag = CheckHashFile();
						CheckItem(GetMHandle(Menu_No), Item_No, hashflag);
						break;
#endif
					case 8:
						break;
					case 9:
						GiveHint();
						break;
					case 10:
						timeout = true;
						ok = true;
						break;
				}
		}

		HiliteMenu(0);
	}
	return ok;
}

int DealwthMouseDowns(EventRecord *Event, int *eatit)
{
	int Location;
	WindowPtr WindowPointedTo;
	Point MouseLoc,theEnd;
	int WindoLoc;
	long Hw,NS;
	Rect r,rDum;
	int x,y,x1,y1,i,j;
	RgnHandle rg;
	Point Dum;
	Str255 C;
	Boolean Bool;
	int Item,ItemType,Num;
	Handle ItemHdl;
	PicHandle h1,h2;

	*eatit = true;
	MouseLoc = Event->where;
	WindoLoc = FindWindow(MouseLoc,&WindowPointedTo);
	switch (WindoLoc) {
		case inMenuBar:
			return ProcessMenu_in((long)MenuSelect(MouseLoc));
			break;
		case inSysWindow:
			SystemClick(Event,WindowPointedTo);
			break;
		case inContent:
			if (WindowPointedTo != FrontWindow()) 
				SelectWindow(WindowPointedTo);
			else return MouseInContent(MouseLoc, eatit);
			break;
		case inGrow:
			if (WindowPointedTo != FrontWindow())
				SelectWindow(WindowPointedTo);
			else
				{
				Hw = GrowWindow(WindowPointedTo, MouseLoc, &GrowArea);
				SizeWindow(WindowPointedTo,Hw & 0xFFFF, Hw>>16, 1);
				r = WindowPointedTo->portRect;
				ClipRect(&r);
				EraseRect(&r);
				InvalRect(&r);
				}
				break;
		case inDrag:
				DragWindow(WindowPointedTo,MouseLoc,&DragArea);
				break;

		case inGoAway:
				if (TrackGoAway(WindowPointedTo,MouseLoc))
					HideWindow(WindowPointedTo);
				break;
		case inZoomIn:
		case inZoomOut:
				if  (WindowPointedTo != FrontWindow())
					SelectWindow(WindowPointedTo);
				else
				{
				ZoomWindow(WindowPointedTo,WindoLoc,1);
				r =WindowPointedTo->portRect;
				Hw = r.bottom - r.top;
				NS = Hw>>16 + r.right - r.left;
				if (NS)
				{
				SizeWindow(WindowPointedTo,NS & 0xFFFF,NS>>16,1);
				r = WindowPointedTo->portRect;
				ClipRect(&r);
				EraseRect(&r);
				InvalRect(&r);
				}
				}
				break;
	}		
	return 0;
}


void DealWthNull(EventRecord *Event)
{
	WindowPtr WindowPointedTo;
	Point MouseLoc;
	int WindoLoc;
	int x,y;
	Boolean Update;

	WindoLoc=FindWindow(MouseLoc=Event->where,&WindowPointedTo);
	
	if ((WindoLoc==inContent) && (WindowPointedTo==WindBoard))
	{
		SetPort(WindBoard);
		GlobalToLocal(&MouseLoc);
		mouseToXY(MouseLoc,&x,&y);
		if (y>0 && y<9 && x>0 && x<9)
		{
			Update=(MouseX!=x) || (MouseY!=y);
			MouseX=x;
			MouseY=y;
			if (Update) {
				UpdateCase(0L,4);
			}
		}
	}
}

int mainLoop()
{
	EventRecord evt;
	int ok, eatit, res, item;
	DialogPtr dlog;
	char theChar;
	
	res = 0;
	
	if (wne)
		ok = WaitNextEvent(everyEvent, &evt, 0L, NULL);
	else {
		SystemTask();
		ok = GetNextEvent(everyEvent, &evt);
	}
	if (!ok) {
		if (force || (towho == opponent)) DealWthNull(&evt);
		return 0;
	}

    eatit = preview;
    
	switch (evt.what) {
		case mouseDown:
			res = DealwthMouseDowns(&evt, &eatit);
			break;
		case keyDown:
		case autoKey: 
			theChar = evt.message & charCodeMask;
			if ((evt.modifiers & cmdKey) != 0) 
			res = ProcessMenu_in( MenuKey( theChar ));
			break;
		case activateEvt: 
			break;
		case updateEvt:
			break;
		case 15 : /* Multifinder Event */
			if ((evt.message & 0xFF000000) != 0xFA000000) {
						background = !(evt.message & 1);
			}
			break;
		default:
			break;
	}

	if (IsDialogEvent(&evt)) {
		DialogSelect(&evt, &dlog, &item);
	}

	if (!eatit && preview) {
		static EvQEl globEvt;
		/* We re-post the event. */
		globEvt.qType = evType;
		globEvt.evtQWhat = evt.what;
		globEvt.evtQMessage = evt.message;
		globEvt.evtQWhen = evt.when;
		globEvt.evtQWhere.h = evt.where.h;
		globEvt.evtQWhere.v = evt.where.v;
		globEvt.evtQModifiers = evt.modifiers;
		Enqueue((QElemPtr)&globEvt, (QHdrPtr)GetEvQHdr());
	}
	return res;
}


loop()
{
  int i;
  
  SetCursor(*ClockCursor);
  towho = white;
  opponent = white;
  computer = black;
  bothsides = false;
  force = reverse = post = easy = false;
  hashflag = false;
  drawn = 0;
  NewGame();
  for (i=0; i<64; i++) {
  	saveBoard[i] = board[i];
  	saveColor[i] = color[i];
  }
  theScore = 0;
  UpdateValue(NULL, 0);
  *Msg = 0;
  UpdateMsg(NULL, 3);
  
  UpdateDisplay(0,0,1,0,1,color,board);
  undo = getgame = newgame = preview = 0;
  while (!(quit))
    {
      player = towho;
	  if (force)
      {
      	opponent = player;
      	computer = otherside[player];
      }
	  if (bothsides)
      {
      	computer = player;
      	opponent = otherside[player];
      }
     
      UpdateMenus();
      
	  if (undo) {
	  		Undo();
	  		undo = 0;
	  		towho = !towho;
	  	}
	  	if (getgame) {
			SetCursor(*ClockCursor);
			Sdepth = 0;
			GetGame();
	  		getgame = 0;
	  	}
	  	if (newgame) {
			SetCursor(*ClockCursor);
	  		LDelRow(0, 0, List);
			NewGame();
			for (i=0; i<64; i++) {
			  	saveColor[i] = color[i];
			  	saveBoard[i] = board[i];
			}
			UpdateDisplay(0,0,1,0,1,color,board);
			towho = white;
			if (force) {
				computer = !towho;
				opponent = towho;
			}
			if (bothsides) {
				computer = towho;
				opponent = !towho;
			}
			Sdepth = 0;
	  		newgame = 0;
	  		drawn = 0;
	  	}
	  	
	  if (mate || drawn)
	  	mainLoop();
	  else {
		if (towho == white)
	  			SetCursor(*ArrowCursor);
	  	else	SetCursor(&arrow);
	  	

	  	for (i=0; i<64; i++) {
		  	saveBoard[i] = board[i];
		  	saveColor[i] = color[i];
		}

	  	if (((towho == opponent) && !bothsides) || force)
	  			InputCommand();
	  	else	SelectMove (towho, 1);
	  	
	  	if (!donotplay) {
	  		towho = !towho;
		} else donotplay = 0;
	  }
    }
}

