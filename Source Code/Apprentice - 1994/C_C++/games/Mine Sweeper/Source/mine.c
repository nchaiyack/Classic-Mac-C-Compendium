/*	mine.c
 *
 *		How to handle the window for mining
 */

#include <stdlib.h>
#include "event.h"
#include "mines.h"

#define		HEIGHT				20			/* Border height	*/

/****************************************************************/
/*																*/
/*	Support Routines											*/
/*																*/
/****************************************************************/

/*	DrawSICN
 *
 *		Draw this small icon
 */

static void DrawSICN(Rect rr, short i)
{
	BitMap bmap;
	Handle h;
	GrafPtr foo;
	Rect s;
	
	h = GetResource('SICN',128);
	if (h == NULL) return;
	HLock(h);
	bmap.baseAddr = *h;
	s.left = 0;
	s.right = 16;
	s.top = i * 16;
	s.bottom = s.top + 16;
	bmap.bounds = s;
	bmap.bounds.top = 0;
	bmap.rowBytes = 2;
	
	GetPort(&foo);
	CopyBits(&bmap,&(foo->portBits),&s,&rr,0,NULL);
	
	HUnlock(h);
}


/*	GenBombs
 *
 *		This generates the bomb locations
 */

void GenBombs(void)
{
	short i,x,y;
	short a,b;
	
	for (x = 0; x < 40; x++) for (y = 0; y < 40; y++) {
		BPosition[x][y] = 0;
		BStatus[x][y] = 0;
	}
	for (i = 0; i < Bombs; i++) {
		do {
			x = Random() % SizeX;  if (x < 0) x += SizeX;
			y = Random() % SizeY;  if (y < 0) y += SizeY;
		} while (BPosition[x][y]);
		BPosition[x][y] = -1;
	}
	for (x = 0; x < 40; x++) for (y = 0; y < 40; y++) {
		if (BPosition[x][y] == -1) continue;
		i = 0;
		for (a = -1; a <= 1; a++) {
			if (a + x < 0) continue;
			if (a + x >= SizeX) continue;
			for (b = -1; b <= 1; b++) {
				if (b + y < 0) continue;
				if (b + y >= SizeY) continue;
				if (BPosition[a+x][b+y] == -1) i++;
			}
		}
		BPosition[x][y] = i;
	}
}

/*	FindLocation
 *
 *		This computes the location
 */

Rect FindLocation(short x, short y)
{
	Rect r;
	
	r.left = x * 16 - 1;
	r.top = y * 16 + HEIGHT;
	r.right = r.left + 16;
	r.bottom = r.top + 16;
	return r;
}


/*	FillFailure
 *
 *		This fills in the failure states when this click fails
 */

static void FillFailure(void)
{
	short a,b;
	
	for (a = 0; a < SizeX; a++) for (b = 0; b < SizeY; b++) {
		if (BPosition[a][b] != -1) {
			if (BStatus[a][b] == 1) BStatus[a][b] = 4;
		} else {
			if (BStatus[a][b] == 0) BStatus[a][b] = 1;
			else if (BStatus[a][b] == 2) BStatus[a][b] = 1;
		}
		DrawSICN(FindLocation(a,b),BStatus[a][b]);
	}
}



/*	ClearPoint
 *
 *		Given a point, this attempts to handle the click in this point
 */

static void ClearPoint(short i,short j, short flag)
{
	if (i < 0) return;
	if (j < 0) return;
	if (i >= SizeX) return;
	if (j >= SizeY) return;
	
	if (BState) return;						/* Ack!  Don't do anything		*/
	
	if (BStatus[i][j] == 0) {				/* Status is cleared			*/
		if (BPosition[i][j] == -1) {		/* Position has bomb; die		*/
			BState = 1;
			BStatus[i][j] = 5;
			DrawSICN(FindLocation(i,j),BStatus[i][j]);
			FillFailure();
		} else if (BPosition[i][j] > 0) {	/* Position is not friendly		*/
			BStatus[i][j] = BPosition[i][j] + 5;
			DrawSICN(FindLocation(i,j),BStatus[i][j]);
			
			if (Cruse) {
				short a,b;					/* See if total marked are okay */
				short count;
				short n;
				
				/*
				 *	Cruse control:  if I'm clicking on this cell, and the
				 *	number of marked items match, then continue.
				 */
				
				count = 0;
				for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
					if (a + i < 0) continue;
					if (b + j < 0) continue;
					if (a + i >= SizeX) continue;
					if (b + j >= SizeY) continue;
					n = BStatus[a+i][b+j];
					
					if (n == 1) count++;
				}
				
				if (count == BPosition[i][j]) {	/* Count match */
					for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
						if (a + i < 0) continue;
						if (b + j < 0) continue;
						if (a + i >= SizeX) continue;
						if (b + j >= SizeY) continue;
						if (BStatus[a+i][b+j] == 1) continue;
						
						ClearPoint(a+i,b+j,flag+1);
					}
				}
			}
		} else {							/* Position is friendly.		*/
			short a,b;
			BStatus[i][j] = 3;
			DrawSICN(FindLocation(i,j),BStatus[i][j]);
			for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
				ClearPoint(i+a,b+j,flag+1);
			}
		}
	} else if ((BStatus[i][j] >= 6) && (!flag)) {		/* Status has value		*/
		short num = 0;
		short a,b;
		short count = 0;
		for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
			if ((a+i<0) || (b+j<0)) continue;
			if ((a+i>=SizeX) || (b+j>=SizeY)) continue;
			if (BStatus[a+i][b+j] == 1) num++, count++;
			if (BStatus[a+i][b+j] == 2) count++;
			if (BStatus[a+i][b+j] == 0) count++;
		}
		if (num < BPosition[i][j]) {
			/*
			 *	Cruse Control #2:  If I click on a cell which has a match between
			 *	the number of unclicked objects and the number in the cell, then
			 *	this marks those objects
			 */
			
			if (Cruse && (count == BPosition[i][j])) {
				for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
					if ((a+i<0) || (b+j<0)) continue;
					if ((a+i>=SizeX) || (b+j>=SizeY)) continue;
					if ((BStatus[a+i][b+j] == 2) || (BStatus[a+i][b+j] == 0)) {
						BStatus[a+i][b+j] = 1;
						DrawSICN(FindLocation(a+i,b+j),BStatus[a+i][b+j]);
					}
				}
			}
		} else {
			for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
				ClearPoint(a+i,b+j,flag+1);
			}
		}
	}
}

/*	ScanUniverse
 *
 *		This scans through my entire universe, calling ClearPoint over and over again
 *	if any entry point with a count has the same number of uncovered or marked points
 *	as the point provided.
 */

static void ScanUniverse(void)
{
	short a,b;
	short i,j;
	short n,count;
	short cover;
	short nbomb;
	
	if (!Cruse) return;						/* Cruse control is not on */
	if (BState) return;						/* The state routine fails */
	
	for (a = 0; a < SizeX; a++) for (b = 0; b < SizeY; b++) {
		if (BStatus[a][b] < 6) continue;
		count = 0;
		cover = 0;
		nbomb = 0;
		for (i = -1; i <= 1; i++) for (j = -1; j <= 1; j++) {
			if (a + i < 0) continue;
			if (b + j < 0) continue;
			if (a + i >= SizeX) continue;
			if (b + j >= SizeY) continue;
			n = BStatus[a+i][b+j];
			if ((n == 0) || (n == 1) || (n == 2)) count++;
			if (n == 0) cover++;
			if (n == 1) nbomb++;
		}
		
		if ((BPosition[a][b] == count) && (cover > 0)) {	/* Click if bomb is here */
			ClearPoint(a,b,0);
			a = 0;
			b = -1;							/* Force restart */
		}
		
		if ((BPosition[a][b] == nbomb) && (cover > 0)) {	/* Click if nbombs same */
			ClearPoint(a,b,0);
			a = 0;
			b = -1;							/* Force restart */
		}
	}
}

			
/*	TestWin
 *
 *		Scan to see if I won
 */

void TestWin()
{
	short a,b;
	short i1;
	Handle i2;
	Rect i3;
	
	for (a = 0; a < SizeX; a++) for (b = 0; b < SizeY; b++) {
		if (BStatus[a][b] == 0) return;
		if (BStatus[a][b] == 2) return;			/* Fail */
		if (BStatus[a][b] == 1) {
			if (BPosition[a][b] == -1) continue;
			return;
		}
		if (BStatus[a][b] == 3) continue;
		if (BStatus[a][b] == 4) return;
		if (BStatus[a][b] == 5) return;
	}
	
	/*
	 *	If I made it here, there are no clears, no questions, and every
	 *	mark is correct, then I won
	 */
	
	SysBeep(5);
	BState = 1;
	BCount = 0;
	
	if ((SizeFlag != 2) || (BombFlag != 1)) return;
	
	for (a = 0; a < 4; a++) {
		if (CurTime < Scores[a].time) break;
	}
	if (a == 4) return;
	for (b = 3; b > a; b--) {
		Scores[b] = Scores[b-1];
	}
	Scores[a].time = CurTime;
	GetUserName(Scores[a].name);
}

/****************************************************************************/
/*																			*/
/*	Window management routines												*/
/*																			*/
/****************************************************************************/

/*	NewMines
 *
 *		This opens the mines window (if state is cleared)
 */

void NewMines(void)
{
	Rect r;
	short x = XLoc, y = YLoc;
	
	if (MineWindow != NULL) {
		x = MineWindow->portRect.left - MineWindow->portBits.bounds.left;
		y = MineWindow->portRect.top - MineWindow->portBits.bounds.top;
	}
	r.left = x;
	r.right = r.left - 1 + 16 * SizeX;
	r.top = y;
	r.bottom = r.top + HEIGHT + 16 * SizeY;
	
	if (MineWindow == NULL) {
		MineWindow = NewWindow(NULL,&r,"\pMines",1,4,(WindowPtr)-1L,1,0L);
		SetPort(MineWindow);
	} else {
		SetPort(MineWindow);
		EraseRect(&(MineWindow->portRect));
		SizeWindow(MineWindow,r.right-r.left,r.bottom-r.top,0);
		InvalRect(&(MineWindow->portRect));
	}
	((WindowPeek)MineWindow)->windowKind = WK_GAME;
	
	Bombs = (SizeX * SizeY) / BRatio;	/* Bomb count ratio */
	BState = 0;							/* Start game over again */
	BFirst = 1;
	GenBombs();							/* And generate some bombs */
	LogTime = TickCount();				/* Store current time */
	CurTime = -1;
}

/*	DrawInteger
 *
 *		This draws the integer provided on the display at the location specified
 */

void DrawInteger(short val, short px, short py)
{
	Rect r;
	short x;
	short y;
	char buffer[32];
	char *a;
	
	ForeColor(redColor);
	r.top = py; r.bottom = r.top + 16;
	r.left = px + 32; r.right = r.left + 16;
	
	NumToString(val,(unsigned char *)buffer);
	y = buffer[0];
	if (y <= 3) {
		for (x = 0; x < y; x++) buffer[x] = buffer[x+1];
		buffer[x] = '\0';
	} else {
		buffer[0] = '-';
		buffer[1] = '-';
		buffer[2] = '-';
		buffer[3] = '\0';
	}
	for (a = buffer; *a != '\0'; a++);
	y = 3;
	while (--a >= buffer) {
		if (*a == '-') DrawSICN(r,16);
		else DrawSICN(r,17+(*a-'0'));
		OffsetRect(&r,-16,0);
	}
	while (y >= 0) {
		EraseRect(&r);
		OffsetRect(&r,-16,0);
		y--;
	}
	ForeColor(blackColor);
}

/*	DoPeriodic
 *
 *		This handle periodic events.  The only thing that happens periodically
 *	is the timer
 */

void DoPeriodic(void)
{
	long time;
	
	if (BState) return;
	
	time = (TickCount() - LogTime)/60;
	if (time == CurTime) return;
	SetPort(MineWindow);
	DrawInteger((short)(CurTime = time),(MineWindow->portRect.right)-50,2);
}

/*	CountUpdate
 *
 *		Draw the score
 */

void CountUpdate(void)
{
	short i,j;
	
	if (!BState) {
		BCount = 0;
		for (i = 0; i < SizeX; i++) for (j = 0; j < SizeY; j++) {
			if (BPosition[i][j] == -1) BCount++;
		}
		for (i = 0; i < SizeX; i++) for (j = 0; j < SizeY; j++) {
			if (BStatus[i][j] == 1) BCount--;
		}
	}
	DrawInteger((long)BCount,2,2);
	DrawInteger((short)CurTime,(MineWindow->portRect.right)-50,2);
}


/*	UpdateMines
 *
 *		This updates the mines display
 */

void UpdateMines(WindowPtr w)
{
	short i,j;
	short minx,maxx,miny,maxy;
	Rect r;
	
	CountUpdate();
	
	/*
	 *	Draw the contents display
	 */
	
	r = (**(w->visRgn)).rgnBBox;
	r.left = (r.left+1)/16;
	r.right = (r.right/16)+1;
	r.top = (r.top-HEIGHT+1)/16;
	r.bottom = (r.bottom-HEIGHT+2)/16+1;
	InsetRect(&r,-1,-1);				/* Paranoia--written at 12:45am */
	if (r.left < 0) r.left = 0;
	if (r.right > SizeX) r.right = SizeX;
	if (r.top < 0) r.top = 0;
	if (r.bottom > SizeY) r.bottom = SizeY;
	for (i = r.left; i < r.right; i++) for (j = r.top; j < r.bottom; j++) {
		DrawSICN(FindLocation(i,j),BStatus[i][j]);
	}
}

/*	ClickMines
 *
 *		What to do when the mouse goes down
 */

void ClickMines(WindowPtr w, Point pt, short modifiers)
{
	Rect r,rr,rpos;
	short i,j;
	short tr[3][3];
	short a,b;
	short state;
	
	SetPort(w);
	if (BState) return;						/* No cigar						*/
	
	/*
	 *	Find where I've clicked, and change the state of the display
	 */
	
	i = (pt.h + 1)/16;
	j = (pt.v - HEIGHT)/16;
	if (j < 0) return;						/* No click */
	if (i < 0) i = 0;
	if (i >= SizeX) i = SizeX;
	if (j >= SizeY) j = SizeY;
	rpos = FindLocation(i,j);
	
	/*
	 *	Handle shift key down
	 */
	
	if (modifiers & shiftKey) {
		if (BStatus[i][j] == 0) BStatus[i][j] = 1;
		else if (BStatus[i][j] == 1) BStatus[i][j] = 2;
		else if (BStatus[i][j] == 2) BStatus[i][j] = 0;
		DrawSICN(rpos,BStatus[i][j]);
		CountUpdate();		
		TestWin();
		
		return;
	}
	
	/*
	 *	Handle until mouse down or up
	 */
	
	if (BStatus[i][j] == 0) {
		for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
			if ((i+a<0) || (a+i>=SizeX)) continue;
			if ((j+b<0) || (b+j>=SizeY)) continue;
			tr[a+1][b+1] = BStatus[a+i][b+j];
			tr[1][1] = 3;							/* Empty */
		}
	} else if (BStatus[i][j] >= 6) {
		for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
			if ((i+a<0) || (a+i>=SizeX)) continue;
			if ((j+b<0) || (b+j>=SizeY)) continue;
			tr[a+1][b+1] = BStatus[a+i][b+j];
			if (BStatus[a+i][b+j] == 0) tr[a+1][b+1] = 3;
			else if (BStatus[a+i][b+j] == 1) tr[a+1][b+1] = 5;
		}
	} else return;									/* Not accepted */

	state = 0;
	while (Button()) {
		GetMouse(&pt);
		if (PtInRect(pt,&rpos)) {
			if (!state) {
				state = 1;
				for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
					if ((i+a<0) || (a+i>=SizeX)) continue;
					if ((j+b<0) || (b+j>=SizeY)) continue;
					DrawSICN(FindLocation(a+i,b+j),tr[a+1][b+1]);
				}
			}
		} else {
			if (state) {
				state = 0;
				for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
					if ((i+a<0) || (a+i>=SizeX)) continue;
					if ((j+b<0) || (b+j>=SizeY)) continue;
					DrawSICN(FindLocation(a+i,b+j),BStatus[a+i][b+j]);
				}
			}
		}
	}

	/*
	 *	Now, put things back
	 */
	
	if (state) {
		for (a = -1; a <= 1; a++) for (b = -1; b <= 1; b++) {
			if ((i+a<0) || (a+i>=SizeX)) continue;
			if ((j+b<0) || (b+j>=SizeY)) continue;
			DrawSICN(FindLocation(a+i,b+j),BStatus[a+i][b+j]);
		}
	} else return;								/* Don't do it! */
	
	/*
	 *	Figure out what has happened
	 */
	
	if ((BFirst) && (BPosition[i][j] == -1)) {
		BStatus[i][j] = 1;
		DrawSICN(FindLocation(i,j),1);
	} else {
		ClearPoint(i,j,0);
		ScanUniverse();							/* Try to clear as much as I can */
	}
	
	BFirst = 0;
	CountUpdate();	
	TestWin();
}


