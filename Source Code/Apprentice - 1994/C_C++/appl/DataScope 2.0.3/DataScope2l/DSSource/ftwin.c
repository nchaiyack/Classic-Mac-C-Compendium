#define __ALLNU__
# include <types.h> 				/* Nearly always required */
# include <quickdraw.h> 			/* To access the qd globals */
# include <toolutils.h> 			/* CursHandle and iBeamCursor */
# include <fonts.h> 				/* Only for InitFonts() trap */
# include <events.h>				/* GetNextEvent(), ... */
# include <windows.h>				/* GetNewWindow(), ... */
# include <controls.h>
# include <files.h>
# include <fcntl.h>
# include <packages.h>
# include <dialogs.h>				/* InitDialogs() and GetNewDialog() */
# include <menus.h> 				/* EnableItem(), DisableItem() */
# include <desk.h>					/* SystemTask(), SystemClick() */
# include <textedit.h>				/* TENew() */
# include <scrap.h> 				/* ZeroScrap() */
# include <segload.h>				/* UnloadSeg() */
# include <StdIO.h>
# include <StdLib.h>
# include <Math.h>
# include <Palette.h>

#include "fview.h"

extern
struct Mwin *mw,*Mlist;		/* Mwin list and temporary pointer */
extern
Point startwin;

extern char *rgbsave;
extern
RgnHandle Urgn;				/* storage for update region */

/*
*  IDs for scroll bar controls
*/
# define vbarID	300
# define hbarID 301

# define windowID		128
# define imgwinID	  129

/*
 *	This routine handles autorepeat scrolling.
 */
 
pascal void DoAutoScroll (ControlHandle, short);
pascal void DoAutoScroll (theCtl, part)
	ControlHandle		theCtl;
	short				part;
{
	struct Mwin *tw;
	struct fdatawin *td;
	struct ftextwin *tc;
	int r, oldl,oldt;
	
	/* get the data structure. */
	tw = findm(FrontWindow());
	td = tw->dat;					/* help for later de-referencing */
	tc = tw->cw;					/* text window data */
	oldl = tc->offl;
	oldt = tc->offt;
	
	/* which scroll bar is it? */
	if (theCtl == tc->hbar)
		{		/* horiz scroll bar. */
		switch (part)
			{
			case inUpButton:
				tc->offl--;  break;
			case inDownButton:
				tc->offl++;  break;
			case inPageUp:
				tc->offl -= tc->ccount.h-1;  break;
			case inPageDown:
				tc->offl += tc->ccount.h-1;  break;
			case inThumb:
				r = GetCtlValue(theCtl);
				tc->offl = r*td->xdim/1000;			/* offset from left */
				break;
			}
		if (tc->offl < 0) tc->offl = 0;
		if (tc->offl >= td->xdim) tc->offl = td->xdim-1;
		}
	else 
		{
		switch (part) 
			{
			case inUpButton:
				tc->offt--;  break;
			case inDownButton:
				tc->offt++;  break;
			case inPageUp:
				tc->offt -= tc->ccount.v-1;  break;
			case inPageDown:
				tc->offt += tc->ccount.v-1;  break;
			case inThumb:
				r = GetCtlValue(theCtl);
				tc->offt = r*td->ydim/1000;			/* offset from left */
				break;
			}
		if (tc->offt < 0) tc->offt = 0;
		if (tc->offt >= td->ydim) tc->offt = td->ydim-1;
		}
/*
*  Call the scroll routine which scrolls and then invalidates the remaining area of
*  the window, so that update will fill in missing areas.
*  Also call routine which invalidates scroll bars, so they will re-draw.
*/
	if (tc->offl != oldl || tc->offt != oldt)
		{
		drawfloat(tw, tc->offl - oldl, tc->offt - oldt); 
		controlfloat (tw);
		}
}


/************************************************************************************/
/* mousefloat
*  Process the mouse click in a floating point window.
*
*/
mousefloat(tw,where,modifiers)
	struct Mwin *tw;
	int modifiers;
	Point *where;
	{
	Point tpt;
	Rect tr;
	ControlHandle aControl;
	int r,i,j,oldl,oldt;
	struct fdatawin *td;
	struct ftextwin *tc;
	
	td = tw->dat;					/* help for later de-referencing */
	tc = tw->cw;					/* text window data */
		
	SetPort(tw->win);
	oldl = tc->offl;
	oldt = tc->offt;
	
/*
*  look for control access (scroll bars)
*/
	GlobalToLocal(where);							/* translate to local */
	i = findcontrol( where, tw->win, &aControl);	/* see if it is a control */
	
	if (i) 
		{										/* do control actions */
		if (i != inThumb)
			trackcontrol( aControl, where, (ProcPtr) DoAutoScroll);
		else
			{
			if (i = trackcontrol( aControl, where, NULL)) 
				{
				if (i == inThumb)
					if (aControl == tc->hbar)
						{
						r = GetCtlValue(aControl);
						tc->offl = r*td->xdim/1000;			/* offset from left */
						if (tc->offl < 0) tc->offl = 0;
						if (tc->offl >= td->xdim) tc->offl = td->xdim-1;
						}
					else 
						{
						r = GetCtlValue(aControl);
						tc->offt = r*td->ydim/1000;			/* offset from left */
						if (tc->offt < 0) tc->offt = 0;
						if (tc->offt >= td->ydim) tc->offt = td->ydim-1;
						}
/*
*  Call the scroll routine which scrolls and then invalidates the remaining area of
*  the window, so that update will fill in missing areas.
*  Also call routine which invalidates scroll bars, so they will re-draw.
*/
				if (tc->offl != oldl || tc->offt != oldt) 
					{
					drawfloat(tw, tc->offl - oldl, tc->offt - oldt); 
					inscrfloat(tw);		/* invalidate bars, so they will re-draw */
					}
				}
			}
		}
	
/*
*  Not a click in a scrollbar, do the selection.
*/
	else {
		tr = tw->win->portRect;
		tr.top = 0;
		tr.left = 0;
		tr.bottom -= 15;
		tr.right -= 15;
		if (!ptinrect(where, &tr))				/* make sure not in grow box */
			return;
		
		pttocell(tw,where);						/* find cell #s */
		if (!(modifiers & shiftKey) ||
			tc->sel.top < 0) {					/* zero selection to start */
			for (i= tc->sel.top; i<=tc->sel.bottom; i++)		/* remove visible old selection */
				for (j=tc->sel.left; j<= tc->sel.right; j++)
					invertfloat(tw,j,i);
			tc->sel.top = tc->sel.bottom = where->v;
			tc->sel.left = tc->sel.right = where->h;
			tc->selanchor = *where;				/* keep the anchor point */
			invertfloat(tw,where->h,where->v);	/* make it reverse */

		}
/*
*  Special behavior with shift-select.  If the mouse is outside of the region, move
*  the anchor to the far point in the selection.
*/
		else {
			if (where->v <= tc->sel.top)				/* point above selection */
				tc->selanchor.v = tc->sel.bottom;		/* set anchor to bottom of selection */
			else if (where->v >= tc->sel.bottom)		/* point below selection */
				tc->selanchor.v = tc->sel.top;			/* set anchor to top of selection */
			if (where->h <= tc->sel.left)				/* similar for left and right situation */
				tc->selanchor.h = tc->sel.right;
			else if (where->h >= tc->sel.right)
				tc->selanchor.h = tc->sel.left;
		}

/*
*  While mouse is pressed, extend the selection.  If the shift key was pressed to
*  start the selection, then we are automatically extending the selection.
*/
		tpt = tc->selanchor;				/* make a copy of anchor point */
		
		while (StillDown()) {
			GetMouse(where);
			if (!ptinrect(where,&tr)) {			/* Is outside of window, scroll */
				if (where->h > tr.right && tc->offl < td->xdim - 1) {
					where->h = tr.right;
					tc->offl++;
					drawfloat(tw, 1, 0);
				}
				if (where->h < tr.left && tc->offl > 0) {
					where->h = tr.left;
					tc->offl--;
					drawfloat(tw, -1,0);
				}
				if (where->v < tr.top && tc->offt > 0) {
					where->v = tr.top;
					tc->offt--;
					drawfloat(tw, 0, -1);
				}
				if (where->v > tr.bottom && tc->offt < td->ydim - 1) {
					where->v = tr.bottom;
					tc->offt++;
					drawfloat(tw, 0, 1);
				}
					
			}	/* fall through to selection extension */
			{
				pttocell(tw,where);				/* make relative to data set */
				
			/* 
			*  determine the direction to extend the selection
			*/
				if (where->v <= tpt.v) {		/* move bottom to anchor, top to point */
					while (tc->sel.bottom > tpt.v)
						invrow(tw,tc->sel.bottom--);	/* make it visible */
					while (tc->sel.top > where->v)
						invrow(tw,--tc->sel.top);
					while (tc->sel.top < where->v)
						invrow(tw,tc->sel.top++);
				}
				else {							/* move top to anchor, bottom to point */
					while (tc->sel.top < tpt.v)
						invrow(tw,tc->sel.top++);
					while (tc->sel.bottom < where->v)
						invrow(tw,++tc->sel.bottom);
					while (tc->sel.bottom > where->v)
						invrow(tw,tc->sel.bottom--);
				}
				
				if (where->h <= tpt.h) {		/* move right to anchor, left to point */
					while (tc->sel.right > tpt.h)
						invcol(tw,tc->sel.right--);		/* make it visible */
					while (tc->sel.left > where->h)
						invcol(tw,--tc->sel.left);
					while (tc->sel.left < where->h)
						invcol(tw,tc->sel.left++);
				}
				else {							/* move left to anchor, right to point */
					while (tc->sel.left < tpt.h)
						invcol(tw,tc->sel.left++);
					while (tc->sel.right < where->h)
						invcol(tw,++tc->sel.right);
					while (tc->sel.right > where->h)
						invcol(tw,tc->sel.right--);
				}
				
			}
		}
/*
*  After selection is made, 
*  synch other windows also, including all image windows.
*  But, since I am already updated, don't re-draw me.
*/
		synchfloat(tw,&tc->sel,0);

	}

	
	return(1);
}


/************************************************************************************/
/*  invrow,invcol
*  invert a row or column of cells during selection.
*/
invrow(tw,rown)
	struct Mwin *tw;
	int rown;
	{
	int i;
	
	for (i=tw->cw->sel.left ; i <= tw->cw->sel.right; i++)
		invertfloat(tw, i, rown);
}

invcol(tw,coln)
	struct Mwin *tw;
	int coln;
	{
	int i;
	
	for (i=tw->cw->sel.top ; i <= tw->cw->sel.bottom; i++)
		invertfloat(tw, coln, i);
}

/************************************************************************************/
/*  pttocell
*   Convert a window-local point to the cell location that the point refers to.
*   Accounts for scroll bar offsets, too.
*/
pttocell(tw,thept)
	Point *thept;
	struct Mwin *tw;
	{
	struct ftextwin *tc;
	
	tc = tw->cw;
	
	thept->v = thept->v/tc->csize.v + tc->offt;
	thept->h = thept->h/tc->csize.h + tc->offl;
	
	if (thept->v >= tw->dat->ydim)
		thept->v = tw->dat->ydim - 1;
	if (thept->h >= tw->dat->xdim)
		thept->h = tw->dat->xdim - 1;
	if (thept->v < 0)
		thept->v = 0;
	if (thept->h < 0)
		thept->h = 0;
	
}

#define CLORLOWER	1
#define CLORHIGHER	239

/*******************************************************************************/
/*  newdatawin
*  Allocate and create a blank datawin record.
*  Fill in blank and default values.
*/
struct fdatawin
*newdatawin(varname,xdim,ydim)
	char *varname;
	int xdim,ydim;
	{
	struct fdatawin *td;
	register int i,j;
	
/*
*  do a memory estimate for safety, then malloc the needed memory.
*/
	j = (xdim*ydim+xdim+ydim)*sizeof(float) + 5000;
	if (checkmem(j)) 
		return(NULL);
	
	if (NULL == (td = (struct fdatawin *)NewPtr(sizeof(struct fdatawin))))
		return(NULL);

	if (NULL == (td->xvals = (float *)NewPtr((xdim+1)*sizeof(float)))
		|| NULL == (td->yvals = (float *)NewPtr((ydim+1)*sizeof(float)))
		|| NULL == (td->vals = (float *)NewPtr((xdim*ydim)*sizeof(float)))) {
		return(NULL);
	}
/*
*  fill in default fields that may get overridden by later loading of data.
*/
	td->xdim = xdim;
	td->ydim = ydim;
	
	strcpy(td->fmt,"%11.4e");			/* set up attribute values */
	strcpy(td->labfmt,"%5.0f");			/* format for column labels */
	td->cmin = 1;
	td->cmax = CLORHIGHER;						/* default color range */
	
	for (i=strlen(varname)-1; i > 0 && varname[i] == ' '; i--)	/* trim trailing spaces */
			varname[i] = 0;
	strncpy(td->dvar,varname,sizeof(td->dvar)-1);
	td->dvar[sizeof(td->dvar)-1] = 0;	/* trim it to required length */
	for (i=0; i< strlen(td->dvar); i++)
		if (td->dvar[i] < 48 || td->dvar[i] > 122)
			td->dvar[i] = '_';			/* translate periods and spaces to _ */

	strcpy(td->xvar,"X axis");
	strcpy(td->yvar,"Y axis");
	i = 300/xdim;
	j = 300/ydim;						/* goodly size for a default image */
	if (i < j)
		j = i;							/* j is default expansion factor */
	if (!j)
		j = 1;							/* if dataset is > 300, make it expansion = 1 */
	td->exx = td->exy = j;
	td->xsize = j*xdim;
	td->ysize = j*ydim;
	
	SetRect(&td->viewport,-ydim/2,-ydim/2,ydim/2,ydim/2);
	
	td->valmax = td->valmin = 0.0;
	td->angleshift = 0;
	
	td->refcount = 1;					/* currently only one reference to this datawin */
	td->needsave = 0;					/* by default, needs no saving */
	td->text = NULL;	
	td->image = NULL;					/* set all redirects to NULL */
	td->interp = NULL;
	td->polar = NULL;
	td->notes = NULL;
	td->content = NULL;

	return(td);

}

/*******************************************************************************/
/*  clonedata
*   Take an fdatawin and clones it.  Reset all of the fields
*   which are tied into the record's current position after copying it.
*
*/
struct fdatawin
*clonedata(tdold)
	struct fdatawin *tdold;
	{
	struct fdatawin *td;
	int xdim,ydim,len;
	float *dat,*xs,*ys;
	
/*
*  Clone the data fields from the datawin structure
*/
	xdim = tdold->xdim;
	ydim = tdold->ydim;
	len = xdim*ydim*sizeof(float);
	
	if (NULL == (dat = (float *)NewPtr(len)))
		return(NULL);		
	memcpy(dat,tdold->vals,len);
	
	if (NULL == (xs = (float *)NewPtr(xdim*sizeof(float))))
		return(NULL);
	memcpy(xs,tdold->xvals,xdim*sizeof(float));

	if (NULL == (ys = (float *)NewPtr(ydim*sizeof(float))))
		return(NULL);
	memcpy(ys,tdold->yvals,ydim*sizeof(float));
	
	if (NULL == (td = (struct fdatawin *)NewPtr(sizeof(struct fdatawin))))
		return(NULL);
		
	*td = *tdold;				/*  copy the datawin structure, every field */
	
	td->refcount = 1;			/* this datawin is brand new */
	td->text = NULL;			/* not installed in an Mwin yet */
	td->image = NULL;			/* set other redirects to NULL */
	td->interp = NULL;
	td->polar = NULL;
	td->notes = NULL;
	td->content = NULL;			/* don't copy notebook from old win */
	td->fname[0] = 0;
	td->needsave = 1;			/* this hasn't been saved */
	
	td->vals = dat;				/* install the new data copies in the struct */
	td->xvals = xs;
	td->yvals = ys;
	
	return(td);
}

/*******************************************************************************/
/*  extractdata
*   Take an fdatawin and clone a portion of it.  
*   Reset all of the fields
*   which are tied into the record's current position after copying it.
*
*/
struct fdatawin
*extractdata(tdold,whatr)
	struct fdatawin *tdold;
	Rect *whatr;
	{
	struct fdatawin *td;
	int xdim,ydim,len,i;
	float *dat,*xs,*ys;
/*
*  Clone the data fields from the datawin structure
*/
	if (whatr->right > tdold->xdim)				/* safety - out of range */
		whatr->right = tdold->xdim;
	if (whatr->bottom > tdold->ydim)
		whatr->bottom = tdold->ydim;
		
	xdim = whatr->right - whatr->left + 1;
	ydim = whatr->bottom - whatr->top + 1;
	len = xdim*sizeof(float);
	
	if (xdim < 2 || ydim < 2)
		return(NULL);
		
	if (checkmem((1+xdim)*(1+ydim)*sizeof(float)+1000)) 
		return(NULL);

	if (NULL == (dat = (float *)NewPtr(len*ydim)))
		return(NULL);
	for (i=0; i<ydim; i++) 				/* copy each line */
		memcpy(dat+i*xdim,tdold->vals+(i+whatr->top)*tdold->xdim+whatr->left,len);
	
	if (NULL == (xs = (float *)NewPtr(xdim*sizeof(float))))
		return(NULL);
	memcpy(xs,tdold->xvals+whatr->left,xdim*sizeof(float));

	if (NULL == (ys = (float *)NewPtr(ydim*sizeof(float))))
		return(NULL);
	memcpy(ys,tdold->yvals+whatr->top,ydim*sizeof(float));
	
	if (NULL == (td = (struct fdatawin *)NewPtr(sizeof(struct fdatawin))))
		return(NULL);
		
	*td = *tdold;				/*  copy the datawin structure, every field */
	td->xdim = xdim;
	td->ydim = ydim;			/*  The size doesn't carry */
	td->xsize = xdim*td->exx;	/*  Re-do the image size by the expansion factors */
	td->ysize = ydim*td->exy;
	SetRect(&td->viewport,-ydim/2,-ydim/2,ydim/2,ydim/2);
	
	td->refcount = 1;			/* this datawin is brand new */
	td->text = NULL;			/* not installed in an Mwin yet */
	td->image = NULL;			/* set other redirects to NULL */
	td->interp = NULL;
	td->polar = NULL;
	td->notes = NULL;
	td->content = NULL;			/* don't copy notebook from old win */
	td->fname[0] = 0;
	td->needsave = 1;			/* need to save extract */
	
	td->vals = dat;				/* install the new data copies in the struct */
	td->xvals = xs;
	td->yvals = ys;
	
	return(td);
}

/*******************************************************************************/
/*  findvar
*   Find the Mwin which is associated with a text record which is associated
*   with data which matches a particular variable name.
*
*   If we don't have it, ask the user what the correct name was.
*/
struct Mwin 
*findvar(s)
	char *s;
	{
	struct Mwin *m;
	char snew[255];
	int again;
	
	again = 1;
	strcpy(snew,s);
	
	while (again) {
		m = Mlist;
		
		while (m) {
			if (m->wintype == FTEXT && !ncstrcmp(m->dat->dvar,snew))
				return(m);
			m = m->next;
		}

/*
*  didn't have it, ask for correction.
*/
		again = askvar(snew);
	}
	
	return(NULL);	
}

/*******************************************************************************/
/*  ctextwin 
*   Given a filled in fdatawin record, create and display the text window which
*   displays that data.  Insert it into the list with all capabilities.  
*/
ctextwin(td)
	struct fdatawin *td;
	{
	struct Mwin *mw;
	
	if (!td)
		return(-1);

	if (NULL == (mw = (struct Mwin *)NewPtr(sizeof(struct Mwin))))
		return(-1);
		
	mw->wintype = FTEXT;

	if (NULL == (mw->cw = (struct ftextwin *)NewPtr(sizeof(struct ftextwin))))
		return(-1);
		
	mw->dat = td;
	td->text = mw;
		
	mw->cw->synchon = 0;
	mw->cw->csize.h = 80;
	mw->cw->csize.v = 15;
	mw->cw->offt = mw->cw->offl = 0;
	SetRect(&mw->cw->sel,-1,-1,-1,-1);
	
	mw->win = GetNewWindow(windowID, NULL, (WindowPtr) -1);
	setwtitle(mw->win, mw->dat->dvar);
	MoveWindow(mw->win,startwin.h,startwin.v,false);
	newstartwin();
	SetPort(mw->win);
/*
*  Load the controls (scroll bars) for the window.
*/
	mw->cw->vbar = GetNewControl(vbarID, mw->win);
	mw->cw->hbar = GetNewControl(hbarID, mw->win);
	
/*
*  install the window in the list.
*/
	mw->next = Mlist;
	Mlist = mw;
	
/*
*  resize to fit.
*/
	checkfloat(mw);
	ShowWindow(mw->win);						/* make it visible */

}

/************************************************************************************/
/*  checkfloat
*   Resets the size of the float window and positions scroll bars so that the window
*   is a multiple of the cell size.
*
*   Calculates and installs the count fields "csize".
*/
checkfloat(tw)
	struct Mwin *tw;
	{
	int i,cx,cy;
	struct fdatawin *td;
	struct ftextwin *tc;
	Rect tr;
	char s[100];
	
	td = tw->dat;						/* help for later de-referencing */
	tc = tw->cw;						/* text window data */
	
	SetPort(tw->win);					/* Set to the window's grafport */
	TextFont(2);
	TextSize(10);
	
	tr = tw->win->portRect;				/* Visible size of window */
	tr.right -= 15;						/* Account for scroll bars */
	tr.bottom -= 15;

/*
*  Calculate the correct csize by testing the sample string -10.000
*/
	sprintf(s,td->fmt,(float)-100.0);
	tc->csize.h = stringwidth(s)+8;
	sprintf(s,td->labfmt,(float)-100.0);		/* try label format, too */
	i = stringwidth(s)+8;
	if (i > tc->csize.h)
		tc->csize.h = i;					/* use max of the values */
		
	sprintf(s,td->fmt,td->valmax);
	if (tc->csize.h < (i= stringwidth(s)+8))	/* is maximum value larger in text form? */
		tc->csize.h = i;
	sprintf(s,td->fmt,td->valmin);
	if (tc->csize.h < (i= stringwidth(s)+8))	/* is minimum value larger in text form? */
		tc->csize.h = i;
	
	SetOrigin(-(tc->csize.h+1), -(tc->csize.v+1));	/* allow for row at top and side of window */
	tr.right -= tc->csize.h + 1;			/* for independent vars */
	tr.bottom -= tc->csize.v + 1;
		
	cx = (tr.right - tr.left)/tc->csize.h;	/* how many cells will fit? */
	cy = (tr.bottom - tr.top)/tc->csize.v;
	if (cx < 1)
		cx = 1;								/* minimum one cell visible */
	if (cy < 1)
		cy = 1;
	
	tc->ccount.h = cx;				/* set window counters to how many the window will hold */
	tc->ccount.v = cy;

		SizeWindow( tw->win, (cx+1)*tc->csize.h + 15 + 1,				/* window outline */
								(cy+1)*tc->csize.v + 15 + 1, true);
		tr = tw->win->portRect;				/* Visible size of window */
		tr.right -= 15;						/* Account for scroll bars */
		tr.bottom -= 15;
		
		MoveControl(tc->vbar, tr.right, tr.top-1);			/* scroll bars */
		MoveControl(tc->hbar, tr.left-1, tr.bottom);
		SizeControl(tc->vbar, 16, tr.bottom-tr.top+2);
		SizeControl(tc->hbar, tr.right-tr.left+2, 16);
	
	tc->forcedraw = 0;
}

/************************************************************************************/
/*  drawfloat
*  Draws the given window structure from scratch.
*  Leaves the grafport set to the window requested.
*  dh,dv is a special mod - if they are non-zero, then we know that the window can
*  be scrolled to make the new display.
*/
drawfloat(tw,dh,dv)
	struct Mwin *tw;
	int dh,dv;
	{
	int i,j,
	si,sj,							/* start limit for the loop */
	cx,cy,							/* limits of loop to draw in main window */
	skipcnt,
	widex,widey,					/* width of one item window */
	a,b;
	register float *f;
	Rect tr,etr;
	char s[100];
	struct fdatawin *td;
	struct ftextwin *tc;

	td = tw->dat;					/* help for later de-referencing */
	tc = tw->cw;					/* text window data */
	
	SetPort(tw->win);				/* Set to the window's grafport */
	
	tr = tw->win->portRect;
	tr.bottom -= 15;
	tr.right -= 15;
	
	cx = tc->ccount.h;				/* set window counters to how many the window will hold */
	cy = tc->ccount.v;
	a = b = 0;

	if (tc->offt + cy > td->ydim) {	/* portion off of bottom of array */
		cy = td->ydim - tc->offt;
		a = 1;
	}
	if (tc->offl + cx > td->xdim) {	/* portion off of the right of array */
		cx = td->xdim - tc->offl;
		b = 1;
	}
		
	widex = tc->csize.h;			/* set cell sizes where we can get them easy */
	widey = tc->csize.v;
/*
*  draw dividers for row/col labels
*/
	PenMode(srcCopy);
	MoveTo(-widex-1,-1);
	LineTo(tr.right,-1);
	MoveTo(-1,-widey-1);
	LineTo(-1,tr.bottom);
/*
*   Label the rows and columns with the xs and ys stored in the data structure
*/
	if (!dv) {
		SetRect(&etr, 0, -widey-1, tr.right, -2 );
		EraseRect(&etr);
		f = td->xvals + tc->offl;
		for (i=0; i<cx; i++) {
			sprintf(s,td->labfmt,*f++);
			MoveTo(i*widex+2,-4);
			drawstring(s);				/* draw it */
		}
	}
	
	if (!dh) {
		SetRect(&etr, -widex-1, 0, -2, tr.bottom );
		EraseRect(&etr);
		f = td->yvals + tc->offt;
		for (i=0; i<cy; i++) {
			sprintf(s,td->labfmt,*f++);
			MoveTo(-widex+2,(i+1)*widey-4);
			drawstring(s);				/* draw it */
		}
	}

/*
*  Draw in the data inside the window.  First check to see if we can scroll to save work.
*/
	si = sj = 0;
	
	if (dh > cx || -dh > cx || dv > cy || -dv > cy)
		dh = dv = 0;
	
	if (dh || dv) {						/* we can scroll */

	    SetRect(&etr, 0, 0, widex*tc->ccount.h, widey*tc->ccount.v );
		
		if (dh < 0) 
			cx = sj - dh;
		if (dh > 0) 
			sj = cx - dh;				/* change drawing region */
		if (dv < 0) 
			cy = si - dv;
		if (dv > 0) 
			si = cy - dv;
		
	    ScrollRect(&etr, -dh*widex, -dv*widey, Urgn);
		inscrfloat(tw);								/* invalidate bars, so they will re-draw */
		
		if ((dv > 0 && a) || (dh > 0 && b))			/* check for condition where drawing is not needed */
			return;

	}
	else {		
		tr = tw->win->portRect;
		tr.top = 0;
		tr.left = 0;
		tr.bottom -= 15;
		tr.right -= 15;
		EraseRect(&tr);								/* Clear stuff in window */
	}


	PenMode(srcCopy);
	f = td->vals + (tc->offt + si)*td->xdim + 
		tc->offl + sj;								/* start of data segment */

	skipcnt = td->xdim - cx + sj;					/* how many to skip each time */
	
	for (i=si; i<cy; i++) {
		for (j=sj; j<cx; j++) {
			sprintf(s,td->fmt,*f++);				/* get text representation of float # */
/*
*  position the text inside the cell.  Four pixels up from the bottom of the cell.
*  Two pixels over from the side of the cell.
*/
			MoveTo(j*widex+2,(i+1)*widey-4);
			drawstring(s);
			a = tc->offt + i;
			b = tc->offl + j;
			if (a >= tc->sel.top && a <= tc->sel.bottom &&
				b >= tc->sel.left && b <= tc->sel.right) 
				invertfloat(tw,b,a);
			
		}
		
		f += skipcnt;
	}
	
}

/************************************************************************************/
/*  invertfloat
*   Take a cell in the visible array and invert it.  The cell is in data coordinates.
*/
invertfloat(tw,x,y)
	struct Mwin *tw;
	int x,y;
	{
	Rect tr;
	int i,j,widex,widey;
	struct fdatawin *td;
	struct ftextwin *tc;
	
	td = tw->dat;					/* help for later de-referencing */
	tc = tw->cw;					/* text window data */
	
	if (x < tc->offl || y < tc->offt ||
		x >= tc->offl + tc->ccount.h ||
		y >= tc->offt + tc->ccount.v ||
		x >= td->xdim || y >= td->ydim)
		return;
		
	j = x - tc->offl;				/* put into screen coordinates */
	i = y - tc->offt;
	
	widex = tc->csize.h;			/* stored size of each cell */
	widey = tc->csize.v;
	
	tr.top = i*widey;
	tr.bottom = (i+1)*widey;
	tr.left = j*widex;
	tr.right = (j+1)*widex;
	InvertRect(&tr);

}

/************************************************************************************/
/* controlfloat
*  Draw the controls according to the current window active region setting.
*/
controlfloat(tw)
	struct Mwin *tw;
	{

	if (tw->dat->ydim > 1) {
		SetCtlValue(tw->cw->vbar,1000*tw->cw->offt/(tw->dat->ydim-1));
		SetCtlValue(tw->cw->hbar,1000*tw->cw->offl/(tw->dat->xdim-1));
	}
	
	DrawControls(tw->win);
	DrawGrowIcon(tw->win);

}

/************************************************************************************/
/* copyfloat
*  Copy the selected data into the Scrap Manager for pasting into other programs.
*/
copyfloat(tw)
	struct Mwin *tw;
	{
	int len,datsize,i,j,cx,cy,skipcnt;
	char *data,*p;
	register float *f;
	char s[100];
	struct fdatawin *td;
	struct ftextwin *tc;
	
	td = tw->dat;					/* help for later de-referencing */
	tc = tw->cw;					/* text window data */

	sprintf(s,td->fmt,(float)0.0);
	len = strlen(s);

	cx = tc->sel.right-tc->sel.left+1;	
	cy = tc->sel.bottom-tc->sel.top+1;
	
	datsize = (len+1)*cy*cx;			/* total data area size */
	if (NULL == (p = data = (char *)NewPtr(datsize+10)))
		return(-1);
	
	f = td->vals + tc->sel.top*td->xdim + tc->sel.left;	/* start of data segment */
	
	skipcnt = td->xdim - cx;			/* how many to skip each time */
	
	for (i=0; i<cy; i++) {
		for (j=0; j<cx; j++) {
			sprintf(p,td->fmt,*f++);	/* get text representation of float # */
			p += len;
			*p++ = '\t';				/* tab delimiters */
		}
		*(p-1) = '\n';					/* newline at end of line */
		f += skipcnt;
	}
	
/*
*  Scrap calls.
*/
	ZeroScrap();
	if (noErr != (i = (int)PutScrap(datsize,'TEXT',data))) {		/* put it there */
		if (i == -108)
			nomem();					/* out of memory */
		ZeroScrap();					/* clear it, no good */
	}
	
	DisposPtr(data);							/* don't need to keep memory now */
}

/************************************************************************************/
/*  inscrfloat
*  	Invalidate the current scroll bars so that they will be re-drawn.
*/
inscrfloat(tw)
	struct Mwin *tw;
	{
	Rect tr;
/*
*  Force a re-draw of scrollbars and grow icon.  
*
*/
	tr = (*tw->cw->vbar)->contrlRect;	/* vertical scroll bar rect */
	tr.bottom += 20;					/* include grow icon */
	InvalRect(&tr);						/* invalidate this region */
	
	tr = (*tw->cw->hbar)->contrlRect;	/* same for horizontal */
	InvalRect(&tr);

}


/************************************************************************************/
/*  growfloat
*   In a floating point window, redraw it according to a grow operation.
*/
growfloat(tw, where)
	Point *where;
	struct Mwin *tw;
	{
	int newsize;
			
	SetPort(tw->win);
	
	newsize = growwindow( tw->win, where, &qd.screenBits.bounds);
	
	if (!newsize)
		return(1);
		
	SizeWindow( tw->win, LoWord(newsize), HiWord(newsize), true);

	inscrfloat(tw);						/* invalidate current scroll bar regions */
/*
*  Now that the old scroll bars have been invalidated, check the window size and
*  control positions.  Cause them to move to make the window a multiple of the
*  cell size.  Will invalidate all regions which need to be re-drawn.
*/
	checkfloat(tw);						
	
	return(1);
}

/***************************************************************************************/
/*  setfloat
*  Sets the current selection rectangle in the given text window.
*  Invalidates the window so that it will re-draw.
*/
setfloat(tw,sr)
	struct Mwin *tw;
	Rect *sr;
	{
	struct ftextwin *tc;
	
	if (tw->wintype != FTEXT)
		return;
		
	if (sr->right >= tw->dat->xdim)				/* safety check of bounds */
		sr->right = tw->dat->xdim-1;
	if (sr->bottom >= tw->dat->ydim)
		sr->bottom = tw->dat->ydim-1;

	tc = tw->cw;
	tc->sel = *sr;								/* set selection rectangle */
	
	tc->offl = sr->left - ((tc->ccount.h - (sr->right-sr->left))>>1);		/* center it */
	if (tc->offl < 0)
		tc->offl = 0;
	tc->offt = sr->top - ((tc->ccount.v - (sr->bottom-sr->top))>>1);
	if (tc->offt < 0)
		tc->offt = 0;

	SetPort(tw->win);						/* refer to the text window */
	InvalRect(&tw->win->portRect);

	
}

/***************************************************************************************/
/*  loseit
*   Get rid of all windows which are associated with a particular dataset.
*/
loseit(tw)
	struct Mwin *tw;
{
	struct fdatawin *td;
	struct Mwin *mw;
	
	if (!tw || !(td = tw->dat))
		return;
	
	mw = Mlist;
	while (mw) {
		if (mw->dat == td) {			/* if it has anything to do with same data */
		
			switch (mw->wintype) {
			case FTEXT:
				if (checksave(mw))		/* save file first? */
					return;				/* didn't want to go away */
				losefloat(mw);
				break;
			case FIMG:
				loseimage(mw);
				break;
			case FBI:
				loseinterp(mw);
				break;
			case FPOL:
				losepolar(mw);
				break;
			case FNOTES:
				losenotes(mw);
				break;
			}
		}
		
		mw = mw->next;
	}
				
}

/***************************************************************************************/
/*  losefloat
*   Take a text window out of the list and get rid of all links.
*/
losefloat(tw)
	struct Mwin *tw;
{
	
/*
*   Remove the back pointer from the dataset
*/
	if (tw->dat->text == tw)		/* this should ALWAYS be true */
		tw->dat->text = NULL;

/*
*  losedat only takes away this parition's reference to the data.  If this is
*  the last reference, then the memory will be released.
*/
	losedat(tw->dat);

	DisposeControl(tw->cw->vbar);	/* scroll bars */
	DisposeControl(tw->cw->hbar);
	DisposPtr((Ptr) tw->cw);

	rmwin(tw);						/* take window out of list */
	
	DisposeWindow(tw->win);
	DisposPtr((Ptr) tw);
				
}

/*******************************************************************************/
/*  openfloat
*  Open a new data file, use the Standard File and then call getdf to establish
*  the new window(s).  getdf will load all of the correct contents into the right
*  places.
*/
openfloat(tw)
	struct Mwin *tw;
	{
	 SFReply reply;
	 Point wh;
	
	wh.h = wh.v = 50;
	/* tlst[0] = mytype; */
	sfgetfile(&wh, "Open HDF data file", nil, -1, nil,
			nil, &reply);
	if (reply.good) {
		p2cstr(reply.fName);
		setvol(NULL, reply.vRefNum);			/* set to this volume (dir) */
		getdf(reply.fName);						/* load this file */

	}

}

/*******************************************************************************/
/*  loadtext
*  Open a new data file, use the Standard File and then call gettext to establish
*  the new window.
*/
loadtext(tw)
	struct Mwin *tw;
	{
	 SFReply reply;
	 SFTypeList tlst;
	 Point wh;
	 int ret;
	
	wh.h = wh.v = 50;
	tlst[0] = 'TEXT';
	sfgetfile(&wh, "Open Text data file", nil, 1, tlst,
			nil, &reply);
	if (reply.good) {
		p2cstr(reply.fName);
		setvol(NULL, reply.vRefNum);			/* set to this volume (dir) */
		ret = gettext(reply.fName);				/* load this file */
		
		if (ret < 0) {
			paramtext(reply.fName,"","","");
			StopAlert(1004, nil);
			return(-1);
		}
	
	}

}

/*******************************************************************************/
/*  doatt
*   put up the attributes dialog box and process all of the options.
*/
# define attrDLOG  129

# define afname 4
# define aok	1
# define acancel 3
# define adepend	5
# define axax		6
# define ayax		7
# define afmt		8
# define amax		9
# define amin		10
# define acmin		11
# define acmax		12
# define adh		13
# define alfmt      15
# define adv		2
# define aoutl      16
# define acalc		17


extern pascal void OutlineItem();
	
doatt(tw)
	struct Mwin *tw;
{
	GrafPtr		savePort;
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	short		itemHit;
	char s[257];
	struct fdatawin *td;
	float tf;
			
	td = tw->dat;
	
	GetPort(&savePort);
	theDialog = GetNewDialog(attrDLOG, nil, (WindowPtr) -1);
	SetPort(theDialog);

	UItemAssign( theDialog, aoutl, OutlineItem);
/*
*  set previous values into the dialog
*/
	GetDItem(theDialog, afname, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, td->fname);
	GetDItem(theDialog, adepend, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, td->dvar);
	GetDItem(theDialog, axax, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, td->xvar);
	GetDItem(theDialog, ayax, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, td->yvar);
	GetDItem(theDialog, amax, &itemType, &itemHdl, &itemRect);
	sprintf(s,td->fmt,td->valmax);
	setitext(itemHdl, s);
	GetDItem(theDialog, amin, &itemType, &itemHdl, &itemRect);
	sprintf(s,td->fmt,td->valmin);
	setitext(itemHdl, s);
	GetDItem(theDialog, afmt, &itemType, &itemHdl, &itemRect);
	c2ftn(td->fmt,s,"E11.4");
	setitext(itemHdl, s);
	GetDItem(theDialog, alfmt, &itemType, &itemHdl, &itemRect);
	c2ftn(td->labfmt,s,"F5.1");
	setitext(itemHdl, s);
	GetDItem(theDialog, acmin, &itemType, &itemHdl, &itemRect);
	sprintf(s,"%d",td->cmin);
	setitext(itemHdl, s);
	GetDItem(theDialog, acmax, &itemType, &itemHdl, &itemRect);
	sprintf(s,"%d",td->cmax);
	setitext(itemHdl, s);
	GetDItem(theDialog, adh, &itemType, &itemHdl, &itemRect);
	sprintf(s,"%d",td->ydim);
	setitext(itemHdl, s);
	GetDItem(theDialog, adv, &itemType, &itemHdl, &itemRect);
	sprintf(s,"%d",td->xdim);
	setitext(itemHdl, s);
	
	SelIText(theDialog, adepend, 0,32767);

	do {
		ModalDialog(nil, &itemHit);
		
		switch (itemHit) {
		case acancel:
			SetPort(savePort);
			DisposDialog(theDialog);
			return;
		case acalc:
			maxmin( td->vals, td->xdim, td->ydim, &td->valmax, &td->valmin,
				0,0,td->ydim, td->xdim);
			GetDItem(theDialog, amax, &itemType, &itemHdl, &itemRect);
			sprintf(s,td->fmt,td->valmax);
			setitext(itemHdl, s);
			GetDItem(theDialog, amin, &itemType, &itemHdl, &itemRect);
			sprintf(s,td->fmt,td->valmin);
			setitext(itemHdl, s);
			break;
		default:
			break;
		}
	} while (itemHit != aok);
	

	GetDItem(theDialog, afmt, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	ftn2c(s,td->fmt,"%11.4e");
	GetDItem(theDialog, alfmt, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	ftn2c(s,td->labfmt,"%5.1f");
	
	GetDItem(theDialog, acmin, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	td->cmin = atoi(s);
	if (td->cmin < CLORLOWER) td->cmin = CLORLOWER;
	if (td->cmin > CLORHIGHER)	td->cmin = CLORHIGHER;
	GetDItem(theDialog, acmax, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	td->cmax = atoi(s);
	if (td->cmax < CLORLOWER) td->cmax = CLORLOWER;
	if (td->cmax < td->cmin)
		td->cmax = td->cmin;
	if (td->cmax > CLORHIGHER)	td->cmax = CLORHIGHER;
	
	GetDItem(theDialog, amax, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	tf = atof(s);					/* set new max value */
	if (tf > -1e30 && tf < 1e30)
		td->valmax = tf;
	GetDItem(theDialog, amin, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	tf = atof(s);					/* set new min value */
	if (tf > -1e30 && tf < 1e30)
		td->valmin = tf;
	
	GetDItem(theDialog, adepend, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
/*
*  if the variable name has changed, we must change all of the corresponding
*  window titles and the stored variable name.
*/
	if (strcmp(td->dvar,s)) {
		if (td->text)
			setwtitle(td->text->win,s);			/* set window to this title */
		if (td->polar)
			setwtitle(td->polar->win,s);		/* set window to this title */
		if (td->image)
			setwtitle(td->image->win,s);		/* set window to this title */
		if (td->interp)
			setwtitle(td->interp->win,s);		/* set window to this title */
		if (td->notes)
			setwtitle(td->notes->win,s);		/* set window to this title */
		strncpy(td->dvar,s,19);
	}
	GetDItem(theDialog, axax, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	strncpy(td->xvar,s,19);
	GetDItem(theDialog, ayax, &itemType, &itemHdl, &itemRect);
	getitext(itemHdl, s);
	strncpy(td->yvar,s,19);
	
	if (td->text) {
		checkfloat(td->text);		/* reset float window with new parms */
		SetPort(td->text->win);
		InvalRect(&td->text->win->portRect);	/* window will re-draw in entirety */
		SetPort(savePort);
	}

	DisposDialog(theDialog);

	return;
}

/************************************************************************************/
/*  c2ftn, ftn2c
*   Translate from one number format string to another.
*   Fill in the program default when the string is invalid.
*/
c2ftn(cs,fs,def)
	char *cs,*fs,*def;
	{
	int wide,prec;
	char fc;

	if (3 != sscanf(cs,"%%%d.%d%c",&wide,&prec,&fc)) {
		strcpy(fs,def);
		return;
	}
	
	sprintf(fs,"%c%d.%d",toupper(fc),wide,prec);
	
}

ftn2c(fs,cs,def)
	char *fs,*cs,*def;
	{
	int wide,prec;
	char fc;

	if (tolower(*fs) == 'i') {
		if (1 != sscanf(&fs[1],"%d",&wide)) {
			strcpy(cs,def);
			return;
		}
		fc = 'f';
		prec = 0;
	}
	else {
		if (3 != sscanf(fs,"%c%d.%d",&fc,&wide,&prec)) {
			strcpy(cs,def);
			return;
		}
	}
	
	sprintf(cs,"%%%d.%d%c",wide,prec,tolower(fc));

}

/************************************************************************************/
struct Mwin *targ;

pascal void mybits(srcBits, srcRect,dstRect, mode, maskRgn)
	PixMapPtr srcBits;
	Rect *srcRect,*dstRect;
	short mode;
	RgnHandle maskRgn;
{
	register int i,j,trans,lim;
	char *p;

	trans = 1;
	if ((*srcBits->pmTable)->ctFlags & 0x8000)
		trans = 0;
	lim = (*srcBits->pmTable)->ctSize;			/* number of entries in color table */
	if (lim > 255)
		lim = 255;
	p = rgbsave;
	for (i=0; i<lim; i++) {						/* convert color table to our storage format */
		j = i;
		if (trans) {
			j = (*srcBits->pmTable)->ctTable[i].value;
			if (j > 255 || j < 0)
				j = 0;
		}
		*p++ = (*srcBits->pmTable)->ctTable[j].rgb.red >> 8;
		*p++ = (*srcBits->pmTable)->ctTable[j].rgb.green >> 8;
		*p++ = (*srcBits->pmTable)->ctTable[j].rgb.blue >> 8;
	}
	setpal(targ,rgbsave);						/* install color table for use */

	StdBits((BitMap *) srcBits, 							/* then call it */
			srcRect, dstRect, mode, maskRgn); 

}

/************************************************************************************/
/*  dopaste
*  create a new dataset if the paste data is usable.
*/
dopaste(tw)
	struct Mwin *tw;
	{
	struct Mwin *tempw;
	register int i,j;
	int wide,tall,lim;
	register float *f;
	register unsigned char *p;
	unsigned char *pstart;
	struct fdatawin *td;
	struct fimgwin *ti;
	PicHandle pich;
	int picsize;
	Rect pframe;
	CQDProcs myprocs;
	
	LoadScrap();

	if (clutpaste(tw))			/* check for color table pasting */
		return(0);
/*
*  Check for PICT waiting to be pasted.  We will be creating two windows to hold the
*  data from this PICT.
*/
	pich = (PicHandle)NewHandle(0);
	picsize = GetScrap( (Handle) pich, 'PICT', &lim);
	if (picsize <= 0)
		return(textpaste());
		
	pframe = (*pich)->picFrame;				/* get dimensions */
	wide = pframe.right - pframe.left;
	tall = pframe.bottom - pframe.top;
	pframe.top = 0;
	pframe.left = 0;
	pframe.right = wide;
	pframe.bottom = tall;
	
/*
*  Is the image too small in either direction to give it its own window?
*  Are we going to have enough memory to do this?
*  5 bytes = 4 bytes for each data point in floating point + 1 byte for image, each point.
*/

	if (wide < 30 ||
		tall < 30 ||
		checkmem((wide+1)*(tall+1)*5+1000)) {
		DisposHandle((Handle) pich);
		return(-1);
	}

	if (NULL == (td = newdatawin("Untitled",wide,tall))) {
		DisposHandle((Handle) pich);
		return(-1);
	}
	
	strcpy(td->fname,"Untitled");
	strcpy(td->fmt,"%5.0f");
	td->needsave = 1;

	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
		
	tempw->dat = td;				/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tempw->iw = (struct fimgwin *)NewPtr(sizeof(struct fimgwin));
	
	tw = tempw;						/* move to this element of list */
	
	td->image = tw;					/* back pointer for text references */
	tw->wintype = FIMG;				/* indicate the image type flag */
	
	ti = tw->iw;					/* image window stuff */

	SetRect(&ti->xr,-11,-11,-11,-11);				/* flag that box isn't set yet */
	ti->exx = 1;
	ti->exy = 1;

	tw->pref = pframe;				/* handles odd width */

	if ( 0 > blankimg(tw,&tw->vdev, wide, tall, wide, tall))
		return;						/* set up window stuff */

/*
*  Replace the standard copybits call with my own.  During DrawPicture, mine will be called.
*/
	SetStdCProcs(&myprocs);
	tw->vdev.vport.grafProcs = (CQDProcsPtr) &myprocs;
	myprocs.bitsProc = (Ptr)mybits;
	targ = tw;
/*
*  Install the image which we got from the clipboard.
*/
 	SetVDevice(&tw->vdev);
	
	DrawPicture(pich, &pframe);
	
	UnsetVDevice(&tw->vdev);

/*
*  Different from makeimage, this time we convert from image back to the floating
*  point instead of the other way around.
*/
	f = td->vals;							/* floating pt data */
	p = pstart = tw->vdev.bp;				/* data space available */

	for (i=0; i<tall; i++) {
		for (j=0; j<wide; j++)
			*f++ = *p++;					/* compiler converts datatypes for us */
		if (wide & 1)
			p++;
	}

	f = td->yvals;
	for (i=0; i<tall ; i++)
		*f++ = (float)i;					/* defaults if they are not there */

	f = td->xvals;
	for (i=0; i<wide ; i++)
		*f++ = (float)i;					/* defaults if they are not there */


/*
*  If we get this far, then we assume things are ok and we start 'er up.
*/
	maxmin( td->vals, wide, tall,
			&td->valmax, &td->valmin, 0,0, tall, wide);

	ctextwin(td);
	
	td->cmin = td->valmin;			/* assign color range from image as pasted */
	td->cmax = td->valmax;

	tw->next = Mlist;				/* install in list */
	Mlist = tw;
	
	DisposHandle((Handle) pich);				/* don't need pic memory anymore */
	return(0);

}

/************************************************************************************/
/*  clutpaste
*  create a new dataset if the paste data is usable.
*/
clutpaste(tw)
	struct Mwin *tw;
{
	register int i,j;
	int lim;
	register unsigned char *p;
	CTabHandle cth;
	int csize;
	
/*
*  Check for CLUT waiting to be pasted. 
*/
	cth = (CTabHandle)NewHandle(0);
	csize = GetScrap( (Handle) cth, 'clut', &lim);
	if (csize <= 0)
		return(0);
		
	HLock( (Handle) cth);
	lim = (*cth)->ctSize;						/* number of entries in color table */
	if (lim > 255)
		lim = 255;

	p = rgbsave;
	for (i=0; i<lim; i++) {						/* convert color table to our storage format */
		j = i;
		j = (*cth)->ctTable[i].value;
		if (j > 255 || j < 0)
			j = 0;
		*p++ = (*cth)->ctTable[j].rgb.red >> 8;
		*p++ = (*cth)->ctTable[j].rgb.green >> 8;
		*p++ = (*cth)->ctTable[j].rgb.blue >> 8;
	}
	
	HUnlock( (Handle) cth);
	DisposHandle((Handle) cth);
	
	setpal(tw,rgbsave);							/* install color table for use */
	
	return(1);
}

/************************************************************************************/
/*  textpaste
*  create a new dataset if the paste data is usable.
*/
textpaste()
	{
	struct Mwin *tempw,*tw;
	register int i,j;
	int wide,tall,lim;
	register float *f;
	register unsigned char *p;
	unsigned char *pstart;
	struct fdatawin *td;
	Handle txth;
	int tsize;
	
/*
*  Check for TEXT waiting to be pasted.  We will be creating two windows to hold the
*  data from this PICT.
*/
	txth = (Handle)NewHandle(0);
	tsize = GetScrap( txth, 'TEXT', &lim);
	if (tsize <= 0)
		return(-1);
	
	HLock(txth);
	pstart = p = *txth;								/* start of text record */
	tall = 0;
	wide = 0;
	j = 0;
	
	
/*
*  Count the number per line and the number of lines.
*/
	
	for (i=0; i<tsize; ) {
		while (i < tsize && (*p == ' ' || *p == 9)) {  /* skip white space */
			p++; i++;
		}
		j++;
		while (i < tsize && (*p > ' '))	{	/* skip the number */
			p++; i++;
		}
		
		if (*p == 13) {						/* if at EOL, keep track */
			/* printf("i: %d, lim: %d, tall: %d\n",i,tsize,tall); */
			if (j > 1)						/* don't count lines with one or fewer numbers */
				tall++;
			if (j > wide)
				wide = j;
			j = 0;
			p++;							/* get past EOL */
			i++;
		}
	}
	
	if (j > 1)								/* if it doesn't end in EOL, count last line */
		tall++;
	
/*
*  Are we going to have enough memory to do this?
*  4 bytes for each data point in floating point.
*/
	if ( wide < 2 || tall < 2 || checkmem((wide+1)*(tall+1)*4+1000) ) {
		HUnlock(txth);
		DisposHandle(txth);
		return(-1);
	}

	if (NULL == (td = newdatawin("Untitled",wide,tall))) {
		HUnlock(txth);
		DisposHandle(txth);
		return(-1);
	}
	
	strcpy(td->fname,"Untitled");
	td->needsave = 1;

	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
		
	tempw->dat = td;				/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	
	tw = tempw;						/* move to this element of list */

/*
*  the data array, read from the data space.
*/
	f = td->vals;
	p = pstart;

	for (i=0; i< tall; i++) {
		if (calcidle()) {
			losedat(td);
			DisposPtr((Ptr) tw);
			HUnlock(txth);
			DisposHandle(txth);				/* don't need text memory anymore */
			return(-1);
		}
		
		for (j=0; j<wide; j++) {

			while (*p == ' ' || *p == 9)	/* skip white space, but not EOL */
				p++;

			if (*p == 13) {					/* don't bother sscanf if we are at EOL, leave 0.0 */
				if (j)
					*f++ = 0.0;				/* missing number in set */
				else {
					i--;
					break;					/* blank line, break out of loop, don't count it */
				}
			}
			else
				{
				*f = atof ((const char *) p);
				f++;
				}
				/*sscanf(p,"%e",f++);			/* get the number */
			
			while (*p > ' ')				/* skip the number, but not white space or EOL */
				p++;
		}
		
		p++;
	}

	HUnlock(txth);
	DisposHandle(txth);				/* don't need pic memory anymore */

	f = td->yvals;
	for (i=0; i<tall ; i++)
		*f++ = (float)i;					/* defaults for the scales */

	f = td->xvals;
	for (i=0; i<wide ; i++)
		*f++ = (float)i;					/* defaults for the scales */


/*
*  If we get this far, then we assume things are ok and we start 'er up.
*/
	maxmin( td->vals, wide, tall,
			&td->valmax, &td->valmin, 0,0, tall, wide);

	ctextwin(td);

	return(0);
}
