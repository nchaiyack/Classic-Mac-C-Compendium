/*
  Mac interface for GNU Chess

  Revision: 10 Feb 1991

  Copyright (C) 1986, 1987, 1988 Free Software Foundation, Inc.
  Copyright (c) 1988, 1989, 1990  John Stanback
  Copyright (c) 1991  Airy ANDRE


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
  
/* This file is based on standard GNU Chess interface */

#include <stdio.h>
#include "math.h"

#include "DragMgr.h"
#include "gnuchess.h"
#include "macintf.h"
#include "rsrc.h"

void OffDrawCase(Rect *r, Pattern color)
/* Paint the rectancle "r" with color "color" on the OffScreen Bitmap */
{
	Rect rBis;
	
	UpdateOffScreen();

	FrameRect(r);

	rBis = *r;
	InsetRect(&rBis,1,1);

	FillRect(&rBis,color);

	SetPort(WindBoard);
}


void UpdateDisplay (short f, short t, short flag, short iscastle, 
					short drawall, short *color, short *board)
{
  short i,j;
  
  GrafPtr SavePort;
  
  GetPort(&SavePort);
  SetPort(WindBoard);
  if (flag)
    {
      for (i = 0; i < 64; i++)
		  DrawPieces(PieceToNum(color[i], board[i]), i);
    }
  else
  {
      if (drawall)
      	AnimePieces(PieceToNum(color[f], board[f]),
      				PieceToNum(color[t], board[t]),
      			  	f, t, anim);
      if (saveBoard[f] == pawn && (abs(t-f) == 7 || abs(t-f) == 9) &&
      	  saveBoard[t] == no_piece) {
      		/* en passant */
      		if (saveColor[f] == white)
      			DrawPieces(no_piece, t - 8);
      		else
      			DrawPieces(no_piece, t + 8);
      }	
      if (iscastle) /* Castle */
	  if (t > f)
	  {
      AnimePieces(PieceToNum(color[f + 3], board[f + 3]),
      			  PieceToNum(color[t - 1], board[t - 1]),
      			  f + 3, t - 1, anim);
	  }
	else
	  {
      AnimePieces(PieceToNum(color[f - 4], board[f - 4]),
      			  PieceToNum(color[t + 1], board[t + 1]),
      			  f - 4, t + 1, anim);
	  }
    }

  SetPort(SavePort);
}





Point CenterRect(Rect *r) 
{
	Point pt;
	
	pt.h = (r->left+r->right)/2;
	pt.v = (r->top+r->bottom)/2;
	return pt;
}


Rect * Rectangle(int xr, int yr, Rect *r)
{
	SetRect( r, 1-xr+(xr-1)*width, 1-yr+(yr-1)*height, 1-xr+xr*width,
				1-yr+yr*height);
	return r;
}

int PieceToNum(int color, int type)
{
 int Num = 0;
 
 switch (type)
 {
 	case pawn:
 		Num = _PICT_PAWN_;
 		break;
 	case rook:
 		Num = _PICT_ROOK_;
 		break;
 	case knight:
 		Num = _PICT_KNIGHT_;
 		break;
 	case bishop:
 		Num = _PICT_BISHOP_;
 		break;
 	case queen:
 		Num = _PICT_QUEEN_;
 		break;
 	case king:
 		Num = _PICT_KING_;
 		break;
 }
 
 if (Num && color == black) Num += 2;
 
 return Num;
}

void PicInRect(Rect *rec, DragHandle DragStuff, Point *Pt)
{
	Rect r1,r2;

	r1=(**DragStuff).ShadowBits.bounds;
	r2=(**DragStuff).PictureBits.bounds;
	OffsetRect(&r1,-r1.right,-r1.bottom);
	OffsetRect(&r2,-r2.right,-r2.bottom);
	OffsetRect(&r1,Pt->h+(r1.right-r1.left)/2,Pt->v+(r1.bottom-r1.top)/2);
	OffsetRect(&r2,Pt->h+(r2.right-r2.left)/2,Pt->v+(r2.bottom-r2.top)/2);
	if (ShadowStuff.visible)
			OffsetRect(&r1,ShadowStuff.dx,ShadowStuff.dy);
	UnionRect(&r1,&r2,&r1);
	InsetRect(&r1,-1,-1);
	if (r1.top<rec->top)  Pt->v+=rec->top-r1.top;
	if (r1.bottom>rec->bottom)  Pt->v+=rec->bottom-r1.bottom;
	if (r1.left<rec->left)  Pt->h+=rec->left-r1.left;
	if (r1.right>rec->right)  Pt->h+=rec->right-r1.right;
}


void DrawPieces(int Num, int sq)
{
 Rect r;
 Boolean White;
 PicHandle h1,h2;
 int h,v;
 GrafPtr oldPort;
 long oldTicks;
 
 h = sq % 8 + 1;
 v = sq / 8 + 1;
 
 White = (h+v)&1;
 v = 9-v;
 
 h = scrntoxy(h);
 v = scrntoxy(v);
 
 Rectangle(h,v,&r);

  if (Num)
  {
	 pt = CenterRect(&r);

	 h1 = GetPicture(Num);
	 HNoPurge((Handle)h1);
	 h2 = GetPicture(Num-1+Num%4);
	 HNoPurge((Handle)h2);

	 NewDraggable(h1,h2,NULL,NULL,&myDragStuff);

 	 ShadowStuff.CopyMode = srcOr;
 	 ShadowStuff.visible = FALSE;
  }

  OffDrawCase(&r,White?ltGray:gray);
  
  if (Num) {
	 DragItTo(myDragStuff,pt,TRUE, TRUE);
	 DisposeDraggable(myDragStuff);

	 HPurge((Handle)h1);
	 HPurge((Handle)h2);
	}

  CopyBits(&OffScreenBits,&WindBoard->portBits,&r,&r,srcCopy,0L);
}

void DragFromTo(DragHandle Drag, Point PtFrom, Point PtTo)
{
	long DeltaX,DeltaY,i;
	double iMax;
	Point Pt;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	SetPort(WindBoard);

	DeltaX = -PtFrom.h+PtTo.h;
	DeltaY = -PtFrom.v+PtTo.v;
	iMax = sqrt(DeltaX*DeltaX+DeltaY*DeltaY);

 	ShadowStuff.visible=1;
 	ShadowStuff.dx=4;
 	ShadowStuff.dy=4;

	for (i=4; i<=iMax+4; i++)
		if (!(i % 5)) {
			Pt.h = PtFrom.h+(i*DeltaX)/iMax;
			Pt.v = PtFrom.v+(i*DeltaY)/iMax;
			PicInRect(&chessRectOff,myDragStuff,&Pt);
			DragItTo(Drag,Pt,1, FALSE);
		}
 	ShadowStuff.CopyMode = srcOr;
 	ShadowStuff.visible = 0;
 	ShadowStuff.dx = 0;
 	ShadowStuff.dy = 0;
	DragItTo(Drag,PtTo,1, TRUE);

	SetPort(oldPort);
}

void AnimePieces(int before,int after ,int d, int a, int anim)
{
	int i,iMax,DeltaX,DeltaY;
 	long Count;
	PicHandle h1,h2;
 	Point p1,p2;
	Rect r;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
 	SetPort(WindBoard);
 	
 	SetPort(WindBoard);
 	DrawPieces(before, d);

 	if (anim) 
 	{
		 p1 = CenterRect(Rectangle(scrntoxy(d%8+1),scrntoxy(8-d/8), &r));
		 p2 = CenterRect(Rectangle(scrntoxy(a%8+1),scrntoxy(8-a/8), &r));
	
		 h1 = GetPicture(after);
		 HNoPurge((Handle)h1);
		 h2 = GetPicture(after-1+after % 4);
		 HNoPurge((Handle)h2);
		
		NewDraggable(h1, h2, 0L, 0L, &myDragStuff);
	
		HPurge((Handle)(h1));
		HPurge((Handle)(h2));
	
		DragFromTo(myDragStuff,p1,p2);
	
		DisposeDraggable(myDragStuff);
	}

 	DrawPieces(after,a);
	SetPort(oldPort);
}

