#define FILE_NUM 10
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * routines to handle the cursor
 ************************************************************************/
#pragma load EUDORA_LOAD
#define MAX_CURSOR 8
short CStack[MAX_CURSOR+1];
int CSTop = 0;
/************************************************************************
 * CycleBalls - spin the beach ball
 ************************************************************************/
void CycleBalls(void)
{
	static int currentBall= -1;
	static long ticks;
	
	if (TickCount()>ticks)
	{
		ticks = TickCount();
		currentBall = (currentBall+1) % 4;
		if (CStack[CSTop]<BALL_CURS || CStack[CSTop]>BALL_CURS+3)
			PushCursor(BALL_CURS+currentBall);
		else
			SetTopCursor(BALL_CURS+currentBall);
		SFWTC = True;
		GiveTime();
	}
}

/************************************************************************
 * CyclePendulum - swing the pendulum
 ************************************************************************/
void CyclePendulum(void)
{
	static int currentPend= -1;
	static int currentDir= 1;
	
	if (currentDir > 0)
	{
		currentPend++;
		if (CStack[CSTop]<PENDULUM_CURS ||
								 CStack[CSTop]>PENDULUM_CURS+3)
			PushCursor(PENDULUM_CURS+currentPend);
		else
			SetTopCursor(PENDULUM_CURS+currentPend);
		if (currentPend == 3)
		{
			currentPend++;
			currentDir = -1;
		}
	}
	else
	{
		currentPend--;
		if (CStack[CSTop]<PENDULUM_CURS ||
				CStack[CSTop]>PENDULUM_CURS+3)
			PushCursor(PENDULUM_CURS+currentPend);
		else
			SetTopCursor(PENDULUM_CURS+currentPend);

		if (currentPend == 0)
		{
			currentPend--;
			currentDir = 1;
		}
	}
	SFWTC = True;
}

/************************************************************************
 * SetCursorByLocation - make the cursor fit the window
 ************************************************************************/
void SetCursorByLocation(void)
{
	MyWindowPtr win;
	Point mouse;
	Rect r;
	SAVE_PORT;
	
#ifdef DEBUG
	if (BUG1) SysBeep(10L);
#endif
	GetPort(&oldPort);
	win = FrontWindow();
	if (win) SetPort(win);
	GetMouse(&mouse);

	if (!MouseRgn) MouseRgn = NewRgn();
	SetRectRgn(MouseRgn,-INFINITY/2,-INFINITY/2,INFINITY/2,INFINITY/2);
	if (!win)
		SetMyCursor(0);
	else if (!CursorInRect(mouse,((GrafPtr)win)->portRect,MouseRgn))
		SetMyCursor(arrowCursor);
	else if (IsMyWindow(win))
	{
		SetPort(win);
		r = win->contR;
		r.top = 0;
		
		if (win->cursor)
		  (*win->cursor)(mouse);
		else if (win->ste)
			STECursor(win->ste);
		else
		{
			if (CursorInRect(mouse,win->contR,MouseRgn))
			{
				switch(((WindowPeek)win)->windowKind)
				{
					case COMP_WIN:
						if (mouse.v<win->topMargin)
						{
							SetMyCursor(arrowCursor);
							r.bottom = win->contR.top;
							break;
						}
						else
							r.top = win->contR.top;
							/* fall through */
					default:
						SetMyCursor(iBeamCursor);
						break;
				}
				RectRgn(MouseRgn,&r);
			}
			else
				SetMyCursor(arrowCursor);
		}
	}
	GlobalizeRgn(MouseRgn);
	REST_PORT;
	SFWTC = False;
}

/************************************************************************
 * SetMyCursor - clear the cursor stack, and set the cursor
 ************************************************************************/
void SetMyCursor(int cursor)
{
	CSTop = 1;
	SetTopCursor(cursor);
}

/************************************************************************
 * PushCursor - push a cursor on the stack
 ************************************************************************/
void PushCursor(int cursor)
{
	if (CSTop<MAX_CURSOR) CSTop++;
	SetTopCursor(cursor);
	SFWTC = True;
}

/************************************************************************
 * PopCursor - restore to the last cursor on the stack
 ************************************************************************/
void PopCursor(void)
{
	if (CSTop) CSTop--;
	SetTopCursor(CStack[CSTop]);
}

/**********************************************************************
 * change cursors
 **********************************************************************/
void SetTopCursor(cursor)
int cursor;
{
	CStack[CSTop] = cursor;
	if (cursor == arrowCursor)
		SetCursor(&qd.arrow);
	else
	{
		Handle curse = GetCursor(cursor);
		short state = HGetState(curse);
		SetCursor(LDRef(curse));
		HSetState(curse,state);
	}
}

