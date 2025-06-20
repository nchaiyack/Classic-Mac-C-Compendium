#include "ep.const.h"
#include "ep.extern.h"
#include "DialogMgr.h"
#include "Math.h"
#include "EventMgr.h"

#define pi100 0.03141592654	/* pi / 100 */
#define fudge 10000
/* real fudge so we can shift to divide rather than use expensive ldiv */
#define fudge2 20078

 
/*double		coslat, coslon, sinlon, x, y, z;*/
double			coslat,alat, alon,coslon,sinlon,x,y,z,a_real;
long		ix, iy, iz;
double		alatd, alond, height; 
double		cos1, cos2, sin1, sin2, xpos, xmax, scaler;
long		icos1, icos2, isin1, isin2, ixmax, ixpos, iscaler;
int			lat, lon, alt;
int			ihalf_xsize,ihalf_ysize;
char		s[255];
extern		long	oldBackground,oldFore;

eraseTerra ()
{
	SetPort(eWindow);

	FillRect(&eWindow->portRect,white);
	FrameOval(&eWindow->portRect);
}

DrawEarth ()
{
register	int					j,a,b,c;
register	long				la,lb,lc;
			int					temp,i;
register	unsigned char		**earthData;
register	unsigned char		*ptr;
EventRecord		theevent;

	SetPort(cWindow);
	HiliteWindow(cWindow,FALSE);

	SetPort(eWindow);
	BringToFront(eWindow);
	HiliteWindow(eWindow,TRUE);

	SetCursor(*watchCursorHand);

	xsize = ixsize = eWindow->portRect.right  - eWindow->portRect.left - 2;
	ysize = iysize = eWindow->portRect.bottom - eWindow->portRect.top  - 2;
	half_xsize = ihalf_xsize = xsize / 2.0;
	half_ysize = ihalf_ysize = ysize / 2.0;

	ClipRect(&eWindow->portRect);

	if (ePict != ebmPict)
		HPurge(ePict);
	HPurge(ebmPict);

	BackColor(backgroundC);
	EraseRect(&eWindow->portRect);

	if (npictButton==1) {
		ePict = OpenPicture(&eWindow->portRect);
		ShowPen();
	}

	ForeColor(earthbackgroundC);
	PaintOval(&eWindow->portRect);

	ForeColor(earthoutlineC);
	FrameOval(&eWindow->portRect);

	alatd = GetCtlValue(latSB);
	alond = GetCtlValue(lonSB);
	height= GetCtlValue(altSB)*(long)altScale;
	if (GetCtlValue(mileCheck) == 0)
		height = height * 1.61;

	alat = alatd * conv;
	alon = alond * conv;
	
	cos1 = cos(alat);
	cos2 = cos(alon);
	sin1 = sin(alat);
	sin2 = sin(alon);
	
	if (GetCtlValue(southCheck) == 1) sin1 = -sin1;
	if (GetCtlValue(westCheck)  == 1) sin2 = -sin2;

	xpos = (height + diam) / diam;
	xmax = 1.0 / xpos;
	scaler = sqrt (1.0 - (xmax * xmax));

	icos1 = (long)(cos1 * fudge);
	icos2 = (long)(cos2 * fudge);
	isin1 = (long)(sin1 * fudge);
	isin2 = (long)(sin2 * fudge);
	ixmax = (long)(xmax * fudge);
	ixpos = (long)(xpos * fudge);
	iscaler = (long)(scaler * fudge);
 
	MoveTo((int)xsize,(int)half_ysize);
	
	draw = TRUE;
	
  if (nlnlBox) 
		for (j = 1; j < 11; j++) {
 
			ForeColor(latC);

			alat   = (double)((j - 6) * 15) * conv;
			coslat = cos(alat) * fudge;
			iz = (long)(sin(alat) * fudge);
			draw = FALSE;
			over = FALSE;
			for (i = 0; i < 204; i += 2) {
 
				alon = (double)(i) * pi100;
				ix = (long)(coslat * cos(alon));
				iy = (long)(coslat * sin(alon));
 
				xxx_int(ix, iy, iz);
 
				draw = TRUE;
 
			}
			if (GetNextEvent(everyEvent,&theevent) != 0)
				if (theevent.what == mouseDown) goto abort_drawing;
		}

  if (nlnlBox)
		for (j = 0; j < 24; j++) {
 
			ForeColor(latC);

			alon   = (double)((j * 15)) * conv;
			coslon = cos(alon) * fudge;
			sinlon = sin(alon) * fudge;
			draw = FALSE;
			over = FALSE;
			for (i = 0; i < 104; i += 2) {
 
				alat = ((double)(i) / 100.0 - 0.5) * pi;
				coslat = cos(alat);
				ix = (long)(coslat * coslon);
				iy = (long)(coslat * sinlon);
				iz = (long)(sin(alat) * fudge);
 
				xxx_int(ix, iy, iz);
 
				draw = TRUE;
 
			}
			if (GetNextEvent(everyEvent,&theevent) != 0)
				if (theevent.what == mouseDown) goto abort_drawing;
		}

 	earthData = (unsigned char **)GetResource('eDAT',100);
 	HLock(earthData);
 	ptr = *earthData;
	j = 0;
	
	ForeColor(landC);

	if (nfpButton) 
		while (j</*9262*/27786) {
			a = ptr[j++];
			b = ptr[j++];
			c = ptr[j++];
			draw = (a+b+c) != 0;
			if (!draw) {
				a = ptr[j++];
				b = ptr[j++];
				c = ptr[j++];
			}
			
			x = (a / 127.5) - 1.0;
			y = (b / 127.5) - 1.0;
			z = (c / 127.5) - 1.0;
			
			xxx(x,y,z);
							
			if (GetNextEvent(everyEvent,&theevent) != 0)
				if (theevent.what == mouseDown) goto abort_and_close;
		}

	if (nintButton) 
		while (j</*9262*/27786) {
			a = ptr[j++];
			b = ptr[j++];
			c = ptr[j++];
			draw = a || b || c;
			if (!draw) {
				a = ptr[j++];
				b = ptr[j++];
				c = ptr[j++];
			}
			
			la = (((long)a * fudge2) >> 8) - fudge;
			lb = (((long)b * fudge2) >> 8) - fudge;
			lc = (((long)c * fudge2) >> 8) - fudge;
			
			xxx_int(la,lb,lc);
			
			if (GetNextEvent(everyEvent,&theevent) != 0)
				if (theevent.what == mouseDown) goto abort_and_close;
		}

abort_and_close:


abort_drawing:	/******** !ARG! **********/

 	HUnlock(earthData);

	if (npictButton==1) {
		HidePen();
		ClosePicture();
	}
		
	ebmPict = OpenPicture(&eWindow->portRect);

	BackColor(backgroundC);
	ForeColor(backgroundC == blackColor ? whiteColor : blackColor);

	if (FALSE /*hasColorQD*/) {
		/*copy the pixMap, when I learn how (no MAC II around here!)*/
	} else {
		CopyBits(&eWindow->portBits,&eWindow->portBits,
		 &eWindow->portRect, &eWindow->portRect, srcCopy, 0L);
	}

	ClosePicture();

	if (nbitmapButton==1)
		ePict = ebmPict;

	InitCursor();

	oldBackground = backgroundC;
	oldFore = backgroundC == blackColor ? whiteColor : blackColor;
	myDrawGrowIcon();
}

xxx(xabs,yabs,zabs)	/* plot a line <-- Richard's Name! */
double	xabs,yabs,zabs;
{
register int		move;
register int		x,y;
static	int over = FALSE;
double	factor,xtmp,xrel,yrel,zrel;

	move = ( ! draw) || over;
	draw = ! move;
	xtmp = xabs * cos2 + yabs * sin2;
	xrel = xtmp * cos1 + zabs * sin1;
	over = (xrel < xmax);
/*	if (nhiddenlinesBox == 0) over = FALSE;	*/

	if (over) return;

	yrel = yabs * cos2 - xabs * sin2;
	zrel = zabs * cos1 - xtmp * sin1;
	
	factor = (-xpos) / (xrel - xpos);
	
	x = ((( yrel * factor * scaler) + 1.0) * half_xsize) + 1 ;
	y = (((-zrel * factor * scaler) + 1.0) * half_ysize) + 1 ;
	
	if (move) MoveTo (x,y);
	if (draw) LineTo (x,y);
}

xxx_int(xabs, yabs, zabs)	/* plot a line or move current position */
long	xabs, yabs, zabs;
{
register	int		x, y;
register	long	ixtmp, ixrel, ifactor;
long		 iyrel, izrel;

	/*
	 * draw line if we want to draw (as opposed to move)
	 * and the previous point was drawn too.
	 */
	draw = draw && !over;

/*	ixtmp = ((xabs * icos2) + (yabs * isin2)) / fudge;
	ixrel = ((ixtmp * icos1) + (zabs * isin1)) / fudge;
asm for SPEED! */
asm {
		move.l	xabs,d0		/* 		xabs * icos2		*/
		move.l	icos2,d2
		muls.w	d2,d0
		move.l	yabs,d1		/*		yabs * isin2		*/
		move.l	isin2,d2
		muls.w	d2,d1
		add.l	d1,d0		/*		add these			*/
		divs.w	#fudge,d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0,ixtmp

	/*	move.l	xabs,d0				not required		*/
		move.l	icos1,d2	/* 		ixtmp * icos1		*/
		muls.w	d2,d0
		move.l	zabs,d1		/*		zabs * isin1		*/
		move.l	isin1,d2
		muls.w	d2,d1
		add.l	d1,d0		/*		add these			*/
		divs.w	#fudge,d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0,ixrel
	}

	over = (ixrel < ixmax);
 
	if (over) return;
 
/*	iyrel = ((yabs * icos2) - (xabs * isin2)) / fudge;
	izrel = ((zabs * icos1) - (ixtmp * isin1)) / fudge;
asm for SPEED! */
asm {
		move.l	yabs,d0		/* 		yabs * icos2		*/
		move.l	icos2,d2
		muls.w	d2,d0
		move.l	xabs,d1		/*		xabs * isin2		*/
		move.l	isin2,d2
		muls.w	d2,d1
		sub.l	d1,d0		/*		add these			*/
		divs.w	#fudge,d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0,iyrel

		move.l	zabs,d0		/* 		zabs * icos1		*/
		move.l	icos1,d2	
		muls.w	d2,d0
		move.l	ixtmp,d1	/*		ixtmp * isin1		*/
		move.l	isin1,d2
		muls.w	d2,d1
		sub.l	d1,d0		/*		add these			*/
		divs.w	#fudge,d0	/*		/ fudge				*/
		ext.l	d0
		move.l	d0,izrel
	}

	ifactor = ixpos * iscaler / (ixpos - ixrel);

	x = (((( iyrel * ifactor) / fudge) + fudge) * ihalf_xsize) / fudge + 1;
  	y = ((((-izrel * ifactor) / fudge) + fudge) * ihalf_ysize) / fudge + 1;

	if (draw)	LineTo (x, y);
	else		MoveTo (x, y);
}

DoEarthMenu (item)
int		item;
{
	switch (item) {
	case 1:
		DrawEarth();
		break;
	case 2:
		setOptions();
 		break;
	case 3:
		setColors();
		break;
	}
}
