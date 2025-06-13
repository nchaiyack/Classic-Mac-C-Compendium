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


UserItemUPP	UpdateGraphicUPP;
UserItemUPP	UpdateCaseUPP;
UserItemUPP	UpdateChronosUPP;
UserItemUPP	UpdateValueUPP;
UserItemUPP	UpdateMsgUPP;
UserItemUPP	UpdateThinkUPP;
UserItemUPP	UpdateListeUPP;


void InitUPPs(void) 
{
	UpdateGraphicUPP = NewUserItemProc(UpdateGraphic);
	UpdateCaseUPP = NewUserItemProc(UpdateCase);
	UpdateChronosUPP = NewUserItemProc(UpdateChronos);
	UpdateValueUPP = NewUserItemProc(UpdateValue);
	UpdateMsgUPP = NewUserItemProc(UpdateMsg);
	UpdateThinkUPP = NewUserItemProc(UpdateThink);
	UpdateListeUPP = NewUserItemProc(UpdateListe);
}


/* Redraw the Window */
pascal void UpdateGraphic(WindowPtr theWindow, int ItemNo)
{
 	GrafPtr SavePort;

	if (ItemNo==1) {
		GetPort(&SavePort);
		SetPort(WindBoard);
		CopyBits(&OffScreenBits,&WindBoard->portBits,&chessRectOff,&chessRectOff,
						srcCopy,0L);
		SetPort(SavePort);
	}
}


/* Redraw the clocks */
pascal void UpdateChronos(WindowPtr theWindow, int ItemNo)
{

	GrafPtr SavePort;
	int i,j;
 	Rect r;
	char time[9];

  	short h, m, s;
  
  
	GetPort(&SavePort);
	SetPort(WindThink);

	i = (nameRec.right-nameRec.left)/2;

	if (ItemNo) {
		SetRect(&r,nameRec.left,nameRec.top,nameRec.left+i,nameRec.bottom);
		FillRect(&nameRec,(ConstPatternParam)&whitePat);
		SetRect(&r,nameRec.left+i,nameRec.top,nameRec.right,nameRec.bottom);
		FillRect(&r,(ConstPatternParam)&blackPat);
	}
	
    if (TCflag[white])
    {
      if (towho == white) {
      		h = (TimeControl.clock[white] - et) / 3600;
      		m = ((TimeControl.clock[white] - et) / 60) % 60;
      		s = (TimeControl.clock[white] - et) % 60;
      } else {
      		h = TimeControl.clock[white] / 3600;
      		m = (TimeControl.clock[white] / 60) % 60;
      		s = TimeControl.clock[white] % 60;
      }
    } else
  if (towho == white) {
	  		h = et / 3600;
	  		m = (et / 60) % 60;
	  		s = et % 60;
  } else {
      		h = TimeControl.clock[white] / 3600;
      		m = (TimeControl.clock[white] / 60) % 60;
      		s = TimeControl.clock[white] % 60;
  }

  if (h < 0)
    h = 0;
  if (m < 0)
    m = 0;
  if (s < 0)
    s = 0;

	time[0] = 8;
	time[1] = '0'+h/10;
	time[2] = '0'+h%10;
	time[3] = ':';
	time[4] = '0'+m/10;
	time[5] = '0'+m%10;
	time[6] = ':';
	time[7] = '0'+s/10;
	time[8] = '0'+s%10;
	TextMode(srcCopy);
	MoveTo((short)(nameRec.left+i-5-StringWidth((ConstStr255Param)time)),(short)(nameRec.top+15));
	DrawString((ConstStr255Param)time);
	
  if (TCflag[black])
    {
      if (towho == black) {
      		h = (TimeControl.clock[black] - et) / 3600;
      		m = ((TimeControl.clock[black] - et) / 60) % 60;
      		s = (TimeControl.clock[black] - et) % 60;
      } else {
      		h = TimeControl.clock[black] / 3600;
      		m = (TimeControl.clock[black] / 60) % 60;
      		s = TimeControl.clock[black] % 60;
      }
    } else
  if (towho == black) {
   		h = et / 3600;
  		m = (et / 60) % 60;
  		s = et % 60;
  } else {
      		h = TimeControl.clock[black] / 3600;
      		m = (TimeControl.clock[black] / 60) % 60;
      		s = TimeControl.clock[black] % 60;
  }
  
  if (h < 0)
    h = 0;
  if (m < 0)
    m = 0;
  if (s < 0)
    s = 0;

	time[0] = 8;
	time[1] = '0'+h/10;
	time[2] = '0'+h%10;
	time[3] = ':';
	time[4] = '0'+m/10;
	time[5] = '0'+m%10;
	time[6] = ':';
	time[7] = '0'+s/10;
	time[8] = '0'+s%10;
	TextMode(notSrcCopy);
	MoveTo((short)(nameRec.right-5-StringWidth((ConstStr255Param)time)),(short)(nameRec.top+15));
	DrawString((ConstStr255Param)time);

 	SetPort(SavePort); 		
}

pascal void UpdateMsg(WindowPtr theWindow, int ItemNo)
{
 	GrafPtr SavePort;
 	int val;
 	int sq;
 	Str255 str;
 	
 	GetPort(&SavePort);
	SetPort(WindThink);
	FrameRect(&MsgFRec);
	
	MoveTo(MsgRec.left+1,MsgRec.top+16);
	TextMode(srcCopy);
	MoveTo(MsgRec.right-5-StringWidth(Msg),MsgRec.top+15);
	FillRect(&MsgRec,(ConstPatternParam)&whitePat);
	DrawString(Msg);

	SetPort(SavePort);
}

pascal void UpdateThink(WindowPtr theWindow, int ItemNo)
{
 	GrafPtr SavePort;
 	int num = ItemNo - 2;
 	int i,h,v;
 	Str255 *tm = ThinkMove[num];
 	
 	GetPort(&SavePort);
	SetPort(WindThink);
	FrameRect(&ThinkFRec[num]);
	
	h = ThinkRec[num].left+2;
	v = ThinkRec[num].top+15;
	TextMode(srcCopy);
	for (i = 0; i <= maxThink; i++)
	if (tm[i][0])
	{
		MoveTo(h, v);
		DrawString(tm[i]);
		v += 12;
	} else break;

	SetPort(SavePort);
}

pascal void UpdateValue(WindowPtr theWindow, int ItemNo)
{
 	GrafPtr SavePort;
 	Str255 str;
 	
 	GetPort(&SavePort);
	SetPort(WindThink);
	FrameRect(&ValueFRec);
	if (showvalue) {
		TextMode(srcCopy);
		NumToString(theScore, str);
		FillRect(&ValueRec,(ConstPatternParam)&whitePat);
		MoveTo(ValueRec.right-5-StringWidth(str),ValueRec.top+15);
		DrawString(str);
	} else FillRect(&ValueRec,(ConstPatternParam)&whitePat);
	SetPort(WindThink);
}

pascal void UpdateCase(WindowPtr theWindow, int ItemNo)
{
 	GrafPtr SavePort;
 	
	GetPort(&SavePort);
	SetPort(WindBoard);
	if (MouseX)
	{
		TextMode(srcCopy);
		MoveTo(CaseRec.left+5,CaseRec.top+15);
		FillRect(&CaseRec,(ConstPatternParam)&whitePat);
		DrawChar(" abcdefgh"[MouseX]);
		DrawChar(" 12345678"[MouseY]);
	} else FillRect(&CaseRec,(ConstPatternParam)&whitePat);
	SetPort(SavePort);
}

pascal void UpdateListe(WindowPtr theWindow, int ItemNo)
{
	LUpdate(WindList->visRgn, List);
}

void UpdateMenus()
{
	int i;
	int moves, minutes;
	int Item_No;
	
	if (GameCnt >= 0)
		EnableItem(GetMHandle(EditMenu), 1);
	else
		DisableItem(GetMHandle(EditMenu), 1);
	
	if ((computer != player) && hint)
		EnableItem(GetMHandle(OptionsMenu), 9);
	else
		DisableItem(GetMHandle(OptionsMenu), 9);
	
	if (computer == player)
		EnableItem(GetMHandle(OptionsMenu), 10);
	else
		DisableItem(GetMHandle(OptionsMenu), 10);
	
	moves = TCmoves[white];
	minutes = TCminutes[white];
	if ((moves == 60) && (minutes == 5)) Item_No = 1;
	else
	if ((moves == 60) && (minutes == 15)) Item_No = 2;
	else
	if ((moves == 60) && (minutes == 30)) Item_No = 3;
	else
	if ((moves == 40) && (minutes == 30)) Item_No = 4;
	else
	if ((moves == 40) && (minutes == 60)) Item_No = 5;
	else
	if ((moves == 40) && (minutes == 120)) Item_No = 6;
	else
	if ((moves == 40) && (minutes == 240)) Item_No = 7;
	else
	if ((moves == 1) && (minutes == 15)) Item_No = 8;
	else
	if ((moves == 1) && (minutes == 60)) Item_No = 9;
	else
	if ((moves == 1) && (minutes == 600)) Item_No = 10;
	else Item_No = 12;

	for (i=1; i<=12; i++)
		CheckItem(GetMHandle(WhiteMenu), i, i==Item_No);

	moves = TCmoves[black];
	minutes = TCminutes[black];
	if ((moves == 60) && (minutes == 5)) Item_No = 1;
	else
	if ((moves == 60) && (minutes == 15)) Item_No = 2;
	else
	if ((moves == 60) && (minutes == 30)) Item_No = 3;
	else
	if ((moves == 40) && (minutes == 30)) Item_No = 4;
	else
	if ((moves == 40) && (minutes == 60)) Item_No = 5;
	else
	if ((moves == 40) && (minutes == 120)) Item_No = 6;
	else
	if ((moves == 40) && (minutes == 240)) Item_No = 7;
	else
	if ((moves == 1) && (minutes == 15)) Item_No = 8;
	else
	if ((moves == 1) && (minutes == 60)) Item_No = 9;
	else
	if ((moves == 1) && (minutes == 600)) Item_No = 10;
	else Item_No = 12;
	
	for (i=1; i<=12; i++)
		CheckItem(GetMHandle(BlackMenu), i, i==Item_No);

	CheckItem(GetMHandle(PlayerMenu), 1, !bothsides && (opponent == white)
										 && (computer == black) && !force);
	CheckItem(GetMHandle(PlayerMenu), 2, !bothsides && (opponent == black)
										 && (computer == white) && !force);
	CheckItem(GetMHandle(PlayerMenu), 3, bothsides && !force);
	CheckItem(GetMHandle(PlayerMenu), 4, !bothsides && force);
	
	CheckItem(GetMHandle(OptionsMenu), 1, easy);
	CheckItem(GetMHandle(OptionsMenu), 2, reverse);
	CheckItem(GetMHandle(OptionsMenu), 3, anim);
	CheckItem(GetMHandle(OptionsMenu), 4, dither!=0);
	CheckItem(GetMHandle(OptionsMenu), 5, post);
	CheckItem(GetMHandle(OptionsMenu), 6, showvalue);
	CheckItem(GetMHandle(OptionsMenu), 7, hashflag);

}

