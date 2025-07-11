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

void
GetOpenings ()

/*
  Read in the Opening Book file and parse the algebraic notation for a
  move into an unsigned integer format indicating the from and to
  square. Create a linked list of opening lines of play, with
  entry->next pointing to the next line and entry->move pointing to a
  chunk of memory containing the moves. More Opening lines of up to 256
  half moves may be added to gnuchess.book.
  */

{
  FILE *fd;
  int c, i, j, side;
  struct BookEntry *entry;
  unsigned short mv, *mp, tmp[100];

  if (((fd = fopen (BOOK, "r")) != NULL) ||
      ((fd = fopen ("gnuchess.book", "r")) != NULL))
    {
      Book = NULL;
      i = 0;
      side = white;
      while ((c = parse (fd, &mv, side)) >= 0)
	if (c == 1)
	  {
	    tmp[++i] = mv;
	    side = otherside[side];
	  }
	else if (c == 0 && i > 0)
	  {
	    entry = (struct BookEntry *) malloc (sizeof (struct BookEntry));
	    mp = (unsigned short *) malloc ((i + 1) * sizeof (unsigned short));
	    if (entry!=NULL && mp!=NULL) {
		    entry->mv = mp;
		    entry->next = Book;
		    Book = entry;
		    for (j = 1; j <= i; j++)
		      *(mp++) = tmp[j];
		    *mp = 0;
		    i = 0;
		    side = white;
		} else {
			if (entry != NULL) free(entry);
			if (mp != NULL) free(mp);
		}
	  }
      fclose (fd);
    }
}


int
parse (fd, mv, side)
     FILE *fd;
     short unsigned int *mv;
     short int side;
{
  int c, i, r1, r2, c1, c2;
  char s[100];
  while ((c = getc (fd)) == ' ') ;
  i = 0;
  s[0] = c;
  while (c != ' ' && c != '\n' && c != EOF)
    s[++i] = c = getc (fd);
  s[++i] = '\0';
  if (c == EOF)
    return (-1);
  if (s[0] == '!' || s[0] == ';' || i < 3)
    {
      while (c != '\n' && c != EOF)
	c = getc (fd);
      return (0);
    }
  if (s[4] == 'o')
    if (side == black)
      *mv = 0x3C3A;
    else
      *mv = 0x0402;
  else if (s[0] == 'o')
    if (side == black)
      *mv = 0x3C3E;
    else
      *mv = 0x0406;
  else
    {
      c1 = s[0] - 'a';
      r1 = s[1] - '1';
      c2 = s[2] - 'a';
      r2 = s[3] - '1';
      *mv = (locn(r1, c1) << 8) + locn(r2, c2);
    }
  return (1);
}

void
GetGame ()
{
  int i;
  long count;
  SFTypeList list;
  Point where;
  SFReply reply;
  int ref;
  FileHeader header;
  
  where.h = 80; where.v = 90;
  list[0] = 'GCsg';
  SFGetFile(where, "\p", 0L, 1, list, 0L, &reply);
  if (!reply.good) return;
  
  LDelRow(0,0,List);
 
  if ( FSOpen( reply.fName, reply.vRefNum, &ref ) == noErr) {
		SetCursor(*ClockCursor);
		count = sizeof(header);
		FSRead( ref, &count, &header);
		if (header.version == CUR_VERSION && header.signature == SIGNATURE) {
			count = sizeof(drawn);
			FSRead( ref, &count, &drawn);
			count = sizeof(mate);
			FSRead( ref, &count, &mate);
			count = sizeof(towho);
			FSRead( ref, &count, &towho);
			count = sizeof(computer);
			FSRead( ref, &count, &computer);
			count = sizeof(opponent);
			FSRead( ref, &count, &opponent);
			count = sizeof(castld);
			FSRead( ref, &count, castld);
			count = sizeof(TCflag);
			FSRead( ref, &count, TCflag);
			count = sizeof(OperatorTime);
			FSRead( ref, &count, OperatorTime);
			count = sizeof(TimeControl);
			FSRead( ref, &count, &TimeControl);
			count = sizeof(GameCnt);
			FSRead( ref, &count, &GameCnt);
			count = sizeof(color);
			FSRead( ref, &count, color);
			count = sizeof(board);
			FSRead( ref, &count, board);
			count = sizeof(Mvboard);
			FSRead( ref, &count, Mvboard);
			count = 500 * sizeof(struct GameRec);
			FSRead( ref, &count, GameList);
			FSClose( ref );
			InitCursor();
		} else {
			FSClose( ref );
			InitCursor();
			return;
		}
  }
  
  preview = 0;
  InitializeStats ();
  Sdepth = 0;
  for (i=0; i<64; i++) {
  	saveColor[i] = color[i];
  	saveBoard[i] = board[i];
  }
  for (i=0; i<=GameCnt; i++) {
  	algbr(GameList[i].gmove >> 8,GameList[i].gmove & 0xFF,GameList[i].flags);
  	AddMove(i, mvstr[(GameList[i].flags & cstlmask) != 0]);
  }
  UpdateDisplay (0, 0, 1, 0, 1, color, board);
}

void SaveGame()
{
	Point where;
	SFReply reply;
	long count;
	int outRef;
	FileHeader header;
	Str255 fileName;
	
	where.h = 100; where.v = 50;
	
	header.version = CUR_VERSION;
	header.signature = SIGNATURE;
	GetWTitle(WindBoard, fileName);
	SFPutFile(where, "", fileName, 0L, &reply);
	if (!reply.good) return;
	FSDelete(reply.fName, reply.vRefNum);
	Create( reply.fName, reply.vRefNum, 'GCHS', 'GCsg' );
	if ( FSOpen( reply.fName, reply.vRefNum, &outRef ) == noErr) {
		SetCursor(*ClockCursor);
		count = sizeof(header);
		FSWrite( outRef, &count, &header);
		count = sizeof(drawn);
		FSWrite( outRef, &count, &drawn);
		count = sizeof(mate);
		FSWrite( outRef, &count, &mate);
		count = sizeof(towho);
		FSWrite( outRef, &count, &towho);
		count = sizeof(computer);
		FSWrite( outRef, &count, &computer);
		count = sizeof(opponent);
		FSWrite( outRef, &count, &opponent);
		count = sizeof(castld);
		FSWrite( outRef, &count, castld);
		count = sizeof(TCflag);
		FSWrite( outRef, &count, TCflag);
		count = sizeof(OperatorTime);
		FSWrite( outRef, &count, OperatorTime);
		count = sizeof(TimeControl);
		FSWrite( outRef, &count, &TimeControl);
		count = sizeof(GameCnt);
		FSWrite( outRef, &count, &GameCnt);
		count = sizeof(color);
		FSWrite( outRef, &count, color);
		count = sizeof(board);
		FSWrite( outRef, &count, board);
		count = sizeof(Mvboard);
		FSWrite( outRef, &count, Mvboard);
		count = 500 * sizeof(struct GameRec);
		FSWrite( outRef, &count, GameList);		
		FSClose( outRef );
		InitCursor();
	}
}

int FSWriteStr(int ref, char * str)
{
	long count;
	
	count = *str;
	return FSWrite( ref, &count, str); 
}

void
ListGame ()
{
	Point where;
	int i, outRef;
	SFReply reply;
	Str255 fileName;
	char bufout[100];
	
 	where.h = 100; where.v = 50;
	
	GetWTitle(WindList, fileName);
	SFPutFile(where, "", fileName, 0L, &reply);
	if (!reply.good) return;
	FSDelete(reply.fName, reply.vRefNum);
	Create( reply.fName, reply.vRefNum, 'GCHS', 'TEXT' );
	if ( FSOpen( reply.fName, reply.vRefNum, &outRef ) == noErr) {
		SetCursor(*ClockCursor);

		FSWriteStr( outRef, "\p\r");
  		FSWriteStr( outRef, *(char **)GetString(_LISTHEAD_STR));
  		FSWriteStr( outRef, "\p         ");
  		FSWriteStr( outRef, *(char **)GetString(_LISTHEAD_STR));
		FSWriteStr( outRef, "\p\r");
  		for (i = 0; i <= GameCnt; i++)
  	  	{
  	  		int f,t;
  	  		
	  	    f = GameList[i].gmove >> 8;
	  	    t = (GameList[i].gmove & 0xFF);
	  	    algbr (f, t, GameList[i].flags);
	  	    if ((i % 2) == 0)
				FSWriteStr( outRef, "\p\r");
	 	     else
				FSWriteStr ( outRef, "\p         ");
	   	   	sprintf (bufout+1, "%5s  %5d     %2d %7ld %5d", mvstr[0],
		 	      GameList[i].score, GameList[i].depth,
		 	      GameList[i].nodes, GameList[i].time);
		 	*bufout = strlen(bufout+1);
		 	FSWriteStr( outRef, bufout);
    	}
 	    FSWriteStr ( outRef, "\p\r\r");
		FSClose( outRef );
		InitCursor();
	}
}
