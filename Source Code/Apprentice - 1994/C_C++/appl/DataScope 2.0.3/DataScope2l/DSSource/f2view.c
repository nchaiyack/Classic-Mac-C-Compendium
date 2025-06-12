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
# include <StdIO.h>
# include <Math.h>
# include <Palette.h>

# include "fview.h"

#include	<Resources.h>
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	findbi 	find the FTEXT data structure associated with another 
				data structure of a different type.
	
 */
 
 struct Mwin * findbi (tw)
 	struct Mwin	*tw;
{
	struct Mwin *mw;
	extern struct Mwin	*Mlist;
	
	mw = Mlist;
	while (mw)
		if (mw->wintype == FBI && mw->dat == tw->dat)
			return (mw);
		else
			mw = mw->next;
			
	return NULL;
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	findimage 	find the FTEXT data structure associated with another 
				data structure of a different type.
	
 */
 
 struct Mwin * findimage (tw)
 	struct Mwin	*tw;
{
	struct Mwin *mw;
	extern struct Mwin	*Mlist;
	
	mw = Mlist;
	while (mw)
		if (mw->wintype == FIMG && mw->dat == tw->dat)
			return (mw);
		else
			mw = mw->next;
			
	return NULL;
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	finddata 	find the FTEXT data structure associated with another 
				data structure of a different type.
	
 */
 
 struct Mwin * finddata (tw)
 	struct Mwin	*tw;
{
	struct Mwin *mw;
	extern struct Mwin	*Mlist;
	
	mw = Mlist;
	while (mw)
		if (mw->wintype == FTEXT && mw->dat == tw->dat)
			return (mw);
		else
			mw = mw->next;
			
	return NULL;
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	interpit	create an interpolated image from the dataset.
					
				tw = pointer to the Mwin structure
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
interpit(tw)
	struct	Mwin		*tw;
{
	struct	fdatawin	*td;
	register int		i,change;
	struct	Mwin		*tempw, *keepw;
	register float		*f;
	void				ErrorAlert();
	
	if (!tw->dat)		return;
	keepw = tw;
	td = tw->dat;
	
	/* is there already an interpolation record. */
	
	if (td->interp) {
	
		/* did the interpolation size change. */
		
		tempw = findbi (tw);
		if (td->xsize != tempw->bw->xsize || td->ysize != tempw->bw->ysize)
			{
			if (td->interp == tw)
				keepw = tw = finddata (tw);
				
			loseinterp (td->interp);
			td = tw->dat;
			}
		else {
			tw = td->interp;
			setwtitle(tw->win,td->dvar);	/* could be a new window title */
			interpdraw(tw);
			SetCursor(&qd.arrow);
			return;
		}
	}

	if (td->xsize < 50)	{		/* minimum sizes for window to create */
		td->xsize = 50;
		td->exx = 50/td->xdim;
	}
	if (td->ysize < 50) {
		td->ysize = 50;
		td->exy = 50/td->ydim;
	}
	
	if (td->xsize & 1)			/* Odd width window.  For interpolated image, 
									we don't need odd width windows, who'll notice? */
		td->xsize++;
/*
*  warning about continuous scales.  Must be increasing or decreasing.
*/
	change = 0;											/* flag == OK */
	f = td->xvals;
	if (*td->xvals > *(td->xvals + td->xdim -1) ) {		/* decreasing */
		for (i=0; i < td->xdim-1; i++,f++)
			if (*f < *(f+1))
				change = 1;
	}
	else {												/* increasing */
		for (i=0; i < td->xdim-1; i++,f++)
			if (*f > *(f+1))
				change = 1;
	}
	f = td->yvals;
	if (*td->yvals > *(td->yvals + td->ydim -1) ) {		/* decreasing */
		for (i=0; i < td->ydim-1; i++,f++)
			if (*f < *(f+1))
				change = 1;
	}
	else {												/* increasing */
		for (i=0; i < td->ydim-1; i++,f++)
			if (*f > *(f+1))
				change = 1;
	}
	
	if (change)
		NoteAlert(1006,nil);							/* not OK, warn user */

/*
*  Are we going to have enough memory to do this?
*/
	if (checkmem(td->xsize*td->ysize+2000))
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }

	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
	if (!tempw)
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }
		
	tempw->dat = tw->dat;			/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tempw->bw = (struct fbiwin *)NewPtr(sizeof(struct fbiwin));
	
	tw = tempw;						/* move to this element of list */
	
	tw->dat->interp = tw;			/* back pointer for text references */
	tw->wintype = FBI;				/* indicate the image type flag */
	
	SetRect(&tw->bw->xr,-11,-11,-11,-11);
	tw->bw->exx = td->exx;
	tw->bw->exy = td->exy;
	tw->bw->xsize = td->xsize;
	tw->bw->ysize = td->ysize;
	
	if ( 0 > blankimg(tw, &tw->vdev, td->xsize, td->ysize, td->xsize, td->ysize))
	   {ErrorAlert(GetResource('STR ',1016));
		return;
	   }

	interpdraw(tw);
/*
*  window is ok, keep it around for updates
*/
	tw->next = keepw->next;			/* install in list */
	keepw->next = tw;

}

/***********************************************************************/
/* size2offset
*  for the bilinear interpolation, translate x and y coordinates from screen
*  coordinates to the offset into the scale array.
*  Takes a pointer to the x and y values and changes them to the new values.
*/
size2offset(tw,px,py)
	struct Mwin *tw;
	short int *px,*py;
	{
	struct fdatawin *td;
	register int i,lim,xsize,ysize;
	float *xv,*yv,pt,xrange,yrange;

	td = tw->dat;
	
	xsize = td->xsize;
	ysize = td->ysize;
	if (tw->wintype == FBI) {
		xsize = tw->bw->xsize;
		ysize = tw->bw->ysize;
	}
	
	xrange = *(td->xvals + td->xdim-1) - *td->xvals;
	yrange = *(td->yvals + td->ydim-1) - *td->yvals;
	
	yv = td->yvals;										/* beginning and end of yvals */
	lim = td->ydim-1;
	
	if (yrange > 0) {
		i = 0;
		pt = (*py * yrange) / ysize + *td->yvals;		/* distance along axis */
			
		while (*(yv+1) < pt && i < lim) {				/* move y pointer to find offset */
			yv++; i++;
		}
		*py = i;			
	} 
	else {												/* decrementing instead */
		/* yrange = -yrange; */
		
		i = 0;
		pt = (*py * yrange) / ysize + *td->yvals;		/* distance along axis */
			
		while (*(yv+1) > pt && i < lim) {				/* move y pointer to find offset */
			yv++; i++;
		}
		*py = i;			
	
	}

	xv = td->xvals;										/* beginning and end of xvals */
	lim = td->xdim-1;
	
	if (xrange > 0) {
		i = 0;
		pt = (*px * xrange) / xsize + *td->xvals;		/* distance along axis */
			
		while (*(xv+1) < pt && i < lim) {				/* move x pointer to find offset */
			xv++; i++;
		}
		*px = i;			
			
	} 
	else {												/* decrementing instead */
		/* xrange = -xrange; */
		
		i = 0;
		pt = (*px * xrange) / xsize + *td->xvals;		/* distance along axis */
			
		while (*(xv+1) > pt && i < lim) {				/* move x pointer to find offset */
			xv++; i++;
		}
		*px = i;			
			
	}
	
}

/***********************************************************************/
/* offset2size
*  for the bilinear interpolation, translate x and y coordinates from offsets
*  to the display coordinates.
*  Takes a pointer to the x and y values and changes them to the new values.
*/
offset2size(tw,px,py)
	struct Mwin *tw;
	short int *px,*py;
	{
	struct fdatawin *td;
	float pt,xrange,yrange;
	int xsize,ysize;
	
	td = tw->dat;
	
	xsize = td->xsize;
	ysize = td->ysize;
	if (tw->wintype == FBI) {
		xsize = tw->bw->xsize;
		ysize = tw->bw->ysize;
	}
	
	if (*px < 0 || *py < 0)						/* don't take any negative settings */
		return;
	
	xrange = *(td->xvals + td->xdim-1) - *td->xvals;
	yrange = *(td->yvals + td->ydim-1) - *td->yvals;
	
	/* if (xrange < 0) xrange = -xrange;
	if (yrange < 0) yrange = -yrange; */
	
	if (*px >= td->xdim)						/* beyond edge, return edge value */
		*px = xsize;
	else {
		pt = *(td->xvals + *px) - *td->xvals;	/* scaled distance from edge */
		
		*px = (pt * xsize) / xrange;
	}
	
	if (*py >= td->ydim)
		*py = ysize;
	else {
	
		pt = *(td->yvals + *py) - *td->yvals;	/* scaled distance from edge */
		
		*py = (pt * ysize) / yrange;
	}
	
}

/***********************************************************************/
/* interpdraw
*  Use a bilinear interpolation scheme to fill in the picture.
*  xsize,ysize = size of image to create.
*  xdim,ydim = size of float array.
*/
interpdraw(tw)
	struct Mwin *tw;
	{
	struct fdatawin *td;
	register int i,j,theval;
	float *f,*dxs,*dys,*xv,*yv,*lim,delx,dely,pt,xrange,yrange,range,zy,z;
	register float *z1,*z2,*z3,*z4;
	register unsigned char *p;
	char *xinc;
	int cmin,cmax,crange,*yoffs,spintime,copytime;


	td = tw->dat;
	
	p = tw->vdev.bp;										/* data space available */
	cmin = td->cmin;
	cmax = td->cmax;
	crange = td->cmax - td->cmin;
	range = td->valmax - td->valmin;
	xrange = *(td->xvals + td->xdim-1) - *td->xvals;
	yrange = *(td->yvals + td->ydim-1) - *td->yvals;
	delx = xrange / td->xsize;								/* x axis increment in image */
	dely = yrange / td->ysize;								/* y axis increment in image */
	
	dxs = (float *)NewPtr(sizeof(float)*td->xsize);			/* temp space for dx's */
	dys = (float *)NewPtr(sizeof(float)*td->ysize);			/* temp space for dy's */
	xinc = (char *)NewPtr(td->xsize);
	yoffs = (int *)NewPtr((td->ysize+1)*sizeof(int));
	yoffs[0] = 0;
	
	if (range < 0) range = -range;	 						/* max must be > min */
	
	f = dys;												/* beginning of dys to fill in */
	yv = td->yvals;											/* beginning and end of yvals */
	lim = td->yvals + td->ydim-2;
	
	if (yrange > 0) {
		for (i=0; i<td->ysize; i++) {						/* fill in dy's */
			pt = dely*(float)i + *td->yvals;				/* scaled position in new image */
			
			while (*(yv+1) < pt && yv < lim) {				/* move y pointer */
				yv++; yoffs[i]++;
			}
			*f++ = (*(yv+1) - pt)/(*(yv+1) - *yv);			/* calculate dy percentage and put in */
			
			yoffs[i+1] = yoffs[i];
		}
			
	} 
	else {													/* decrementing instead */
		yrange = -yrange;
		
		for (i=0; i<td->ysize; i++) {						/* fill in dy's */
			pt = dely*(float)i + *td->yvals;

			while (*(yv+1) > pt && yv < lim) {				/* move y pointer */
				yv++; yoffs[i]++;
			}
			*f++ = (*(yv+1) - pt)/(*(yv+1) - *yv);			/* calculate dy percentage and put in */
			
			yoffs[i+1] = yoffs[i];
		}
	
	}

	f = dxs;												/* beginning of dxs to fill in */
	xv = td->xvals;											/* beginning and end of xvals */
	lim = td->xvals + td->xdim-2;
	
	if (xrange > 0) {
		for (i=0; i<td->xsize; i++) {						/* fill in dx's */
			pt = delx*(float)i + *td->xvals;
			xinc[i] = 0;
			
			while (*(xv+1) < pt && xv < lim) {				/* move xv pointer */
				xv++;  xinc[i]++;
			}
			*f++ = (*(xv+1) - pt)/(*(xv+1) - *xv);			/* calculate dy percentage and put in */
		}
			
	} 
	else {													/* decrementing instead */
		xrange = -xrange;
		
		for (i=0; i<td->xsize; i++) {						/* fill in dx's */
			pt = delx*(float)i + *td->xvals;
			xinc[i] = 0;
	
			while (*(xv+1) > pt && xv < lim) {				/* move y pointer */
				xv++;  xinc[i]++;
			}
			*f++ = (*(xv+1) - pt)/(*(xv+1) - *xv);			/* calculate dy percentage and put in */
		}
	
	}

/*
*   Do the interpolation for each point in the target image.
*   We take advantage of the fact that we know the target is evenly spaced along both
*   axes.
*/
	spintime = copytime = 0;
	yv = dys;
	
	for (i= 0; i < td->ysize; i++,yv++) {
		
		z1 = td->vals + td->xdim*(yoffs[i]);
		z2 = z1 + 1;
		z3 = z1 + td->xdim;
		z4 = z3 + 1;
	
		xv = dxs;
		zy = *yv;
		
		for (j=0; j < td->xsize; j++,xv++) {					/* for each target point */
			
			z1 += xinc[j];				/* xinc == 0 when we don't need to shift */
			z2 += xinc[j];
			z3 += xinc[j];
			z4 += xinc[j];
			
			z = (*z1 - *z3 - *z2 + *z4)*(*xv)*zy +				/* weighted sum */
				(*z3 - *z4)*(*xv) + (*z2 - *z4)*zy + *z4;
				
			theval = cmin + crange*(z - td->valmin)/range;		/* scaled color value to put in */
			if (theval >= cmax || theval < cmin)
				*p++ = 0;
			else
				*p++ = (char)theval;
				
		}
		
		if (TickCount() > spintime) {				/* spin watch each second */
			spintime = TickCount() + 60;
			if (calcidle()) {
				DisposPtr((Ptr) dxs);
				DisposPtr((Ptr) dys);
				DisposPtr((Ptr) xinc);
				DisposPtr((Ptr) yoffs);
				return;
			}
			if (TickCount() > copytime) {			/* update image each 5-6 seconds */
				copytime = spintime + 200;
				copyimg(tw);
			}
		}
		
	}

	copyimg(tw);
	
	DisposPtr((Ptr) dxs);
	DisposPtr((Ptr) dys);
	DisposPtr((Ptr) xinc);
	DisposPtr((Ptr) yoffs);
	return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	polarit		create a polar-interpolated image from the dataset.
					
				tw = pointer to the Mwin structure
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
polarit(tw)
	struct Mwin		*tw;
{
	struct fdatawin	*td;
	double			sina1,sina2,
					cosa1,cosa2,
					invrange,rfactor;
	register int	j,theval,nrad;
	short 			xoff, yoff;
	int				i,rscaled,nextrscaled,
					nangles,rsize,cmin,cmax,crange;
	struct Mwin		*tempw,*keepw;
	register float	*f,*ival,*angptr;
	RGBColor		clor;
	PaletteHandle	pal;
	PolyHandle		sect;
	int				icode = 0;
	void			ErrorAlert();
	
	
	if (!tw->dat)
		return;
	keepw = tw;						/* need a copy for later insertion into the list */

/*
*  Are we going to have enough memory to do this?
*/
	td = tw->dat;

	if (td->polar) {
		tw = td->polar;
		setwtitle(tw->win,td->dvar);	/* could be a new window title */
		icode = 1;
		goto	genimage;
	}	

	i = (td->viewport.right - td->viewport.left)*td->exx;
	j = (td->viewport.bottom - td->viewport.top)*td->exx;

	if (checkmem(i*j+1000))
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }

/*
*  warning about continuous scales.  Must be increasing in radius and theta.
*/
	theval = 0;											/* flag == OK */
	f = td->xvals;
	if (*td->xvals > *(td->xvals + td->xdim -1) ) {		/* decreasing */
		theval = 1;
	}
	else {												/* increasing */
		for (i=0; i < td->xdim-1; i++,f++)
			if (*f > *(f+1))
				theval = 1;
	}
	f = td->yvals;
	if (*td->yvals > *(td->yvals + td->ydim -1) ) {		/* decreasing */
		theval = 1;
	}
	else {												/* increasing */
		for (i=0; i < td->ydim-1; i++,f++)
			if (*f > *(f+1))
				theval = 1;
	}
	
	if (theval) {
		StopAlert(1007,nil);		/* not OK, stop the user */
		return;
	}


/*
*  do it
*/
	tempw = (struct Mwin *)NewPtr(sizeof(struct Mwin));
	if (!tempw)
	   {ErrorAlert(GetResource('STR ',1001));
		return;
	   }
		
	tempw->dat = tw->dat;			/* point to same dataset */
	tempw->dat->refcount++;			/* increase refcount bookkeeping */
	tempw->pw = (struct fpolwin *)NewPtr(sizeof(struct fpolwin));
	
	tw = tempw;						/* move to this element of list */
	
	tw->dat->polar = tw;			/* back pointer for polar references */
	tw->wintype = FPOL;				/* indicate the image type flag */
	
	SetRect(&tw->pw->xr,-11,-11,-11,-11);
	tw->pw->exx = td->exx;
	tw->pw->exy = td->exx;
	
	if (50/td->exx > td->viewport.right - td->viewport.left)
		td->viewport.right = td->viewport.left + 50/td->exx;
	if (50/td->exx > td->viewport.bottom - td->viewport.top)
		td->viewport.bottom = td->viewport.top + 50/td->exx;
	
	td->xsize = (td->viewport.right - td->viewport.left)*td->exx;
	td->ysize = (td->viewport.bottom - td->viewport.top)*td->exx;
	tw->pw->viewport = td->viewport;
	
	if (0 > blankimg(tw, &tw->vdev, td->xsize, td->ysize,
		td->xsize, td->ysize))
	   {ErrorAlert(GetResource('STR ',1016));
		return;
	   }

	genimage:
	
	rsize = td->ydim/2;
	tw->pw->angleshift = td->angleshift;

	SetVDevice(&tw->vdev);

	PenMode(srcCopy);

	pal = GetPalette(tw->win);							/* color table */

/*
*  After this point, the window is a valid window, even if the image isn't created yet,
*  so we can insert it into the update list.
*/
	if (icode == 0)
	   {tw->next = keepw->next;				/* install in list */
		keepw->next = tw;
	   }
		
/*
*  Generate the polar image, going from the center out, from 0 to 2*pi.
*  Generate a four-sided filled polygon to simulate each sector.  Don't use arcs because they
*  would be too slow.
*
*  invrange:  inverse of range of data values to scale colors from.
*  rfactor:   scaling factor from data-space to pixel-space for radius values.
*  xoff:      offset to move circle's center into device space.
*  yoff:      offset to move circle's center ...
*  cosa1,cosa2:  cosine of angle previous and angle next to draw polygon between.
*  sina1,sina2:  sine ...
*  angptr:    pointer to current previous angle
*  nrad:      number of radius points to hit
*  nangles:   number of angles to cover
*  
*/

	cmin = td->cmin;
	cmax = td->cmax;
	crange = cmax - cmin;
	
	invrange = 1.0/ (td->valmax - td->valmin);
	rfactor = (float) rsize * td->exx /  (*(td->yvals + td->ydim -1) - *(td->yvals));
	
	if (invrange < 0) invrange = -invrange;						/* max must be > min */
	
	xoff =  - td->viewport.left*td->exx;
	yoff =  - td->viewport.top*td->exx;
	
	nrad = td->ydim - 1;
	nangles = td->xdim - 1;
		
	angptr = td->xvals;						/* first angle in dataset */
	sina2 = sin((double)*angptr + td->angleshift*3.14159/2.0);
	cosa2 = cos((double)*angptr++ + td->angleshift*3.14159/2.0);	/* prep for first angle */
	
/*
*  Start outer loop along the angles, inner loop goes out from center for radius values.
*/
	
	for (i=0; i < nangles; i++) {			/* traverse theta range, less one value */
	
		if (!(i&15)) {
			UnsetVDevice();
			copyimg(tw); 
			SetVDevice(&tw->vdev);
		}
			
		sina1 = sina2;
		cosa1 = cosa2;						/* shift angle values down 1 */
		sina2 = sin((double)*angptr + td->angleshift*3.14159/2.0);
		cosa2 = cos((double)*angptr++ + td->angleshift*3.14159/2.0);		/* setup values for next angle */
		
		ival = td->vals + i;				/* pointer to first data value in a ray (column) */
		
		f = td->yvals;						/* pointer to radius values */
		
		nextrscaled = (*f - *(td->yvals))*rfactor;		/* convert to pixel-space */


		UnsetVDevice();
		if (calcidle()) {					/* check user abort */
			copyimg(tw);
			return;
		}
		SetVDevice(&tw->vdev);


		for (j=0; j < nrad; j++) {			/* traverse r range, less one value */

			ival += td->xdim;				/* move to next row in dataset */
	
			rscaled = nextrscaled;
			nextrscaled = (*(++f) - *(td->yvals))*rfactor;		/* convert to pixel-space */
			
			theval = cmin + crange*(*ival - td->valmin)*invrange;  /* scaled color value  */
			if (theval >= cmax || theval < cmin)
				theval = 0;
				
			GetEntryColor( pal, theval, &clor);
			RGBForeColor(&clor);
			sect = OpenPoly ();
			MoveTo((short)(rscaled*cosa1+0.5)+xoff,(short)(rscaled*sina1+0.5)+yoff);	/* first point */
			LineTo((short)(nextrscaled*cosa1+0.5)+xoff,(short)(nextrscaled*sina1+0.5)+yoff);
			LineTo((short)(nextrscaled*cosa2+0.5)+xoff,(short)(nextrscaled*sina2+0.5)+yoff);
			LineTo((short)(rscaled*cosa2+0.5)+xoff,(short)(rscaled*sina2+0.5)+yoff);	/* fourth point */
			LineTo((short)(rscaled*cosa1+0.5)+xoff,(short)(rscaled*sina1+0.5)+yoff);	/* back to first point */
			
			ClosePoly();
			PaintPoly(sect);
			KillPoly(sect);
		}

	}	
	
	UnsetVDevice();
	SetCursor(&qd.arrow);
	copyimg(tw);
}

/*********************************************************************************/
/*  littlesect
*   Given the selection area in r,theta, draw the sector on the polar window.
*/
littlesect(tw,tsect,drawit)
	struct Mwin *tw;
	Rect *tsect;
	int drawit;
	{
	Rect tr;
	
	if (tw->wintype != FPOL)
		return;
		
	polarsect(tw,&tw->pw->xr);		/* get rid of old box */
	
	if (drawit) {
		tr = *tsect;

		tw->pw->xr = tr;			/* set in new values */
		polarsect(tw,&tr);			/* draw it */
	}

}

polarsect(tw,tsect)
	Rect *tsect;
	struct Mwin *tw;
	{
	struct fdatawin *td;
	double sina,cosa;
	int tpos,rscaled,nextrscaled,xbig,xoff,yoff;
	float *f,rrange;
	Rect tr;
	
	if (tw->wintype != FPOL)
		return;

	td = tw->dat;
	
	SetPort(tw->win);	
	PenMode(patXor);
	tr = *tsect;
	
	if (tsect->left < 0 || tsect->right < 0 ||
		tsect->top < 0 || tsect->bottom < 0)
		return;

	if (tsect->left)
		tsect->left--;
	if (tsect->top)
		tsect->top--;
		
	if (tsect->bottom >= td->ydim) 
		tsect->bottom = td->ydim - 1;
	if (tsect->right >= td->xdim)
		tsect->right = td->xdim - 1;
		
	rrange = *(td->yvals + td->ydim-1) - *(td->yvals);
	
	xoff = -tw->pw->viewport.left*tw->pw->exx;
	yoff = -tw->pw->viewport.top*tw->pw->exx;
		
	tpos = tsect->left;
	xbig = td->ydim*tw->pw->exx/2;
	
	f = td->yvals + tsect->top;
	rscaled = (int)(xbig*(*f - *(td->yvals))/rrange);

	f = td->yvals + tsect->bottom;
	nextrscaled = (int)(xbig*(*f - *(td->yvals))/rrange);
	
	cosa = cos((double) *(td->xvals + tpos) + tw->pw->angleshift*3.14159/2.0);
	sina = sin((double) *(td->xvals + tpos) + tw->pw->angleshift*3.14159/2.0);

	MoveTo((int)(rscaled*cosa)+xoff,(int)(rscaled*sina)+yoff);	/* first point */
	
	while (tpos <= tsect->right) {
		cosa = cos((double) *(td->xvals + tpos)  + tw->pw->angleshift*3.14159/2.0);
		sina = sin((double) *(td->xvals + tpos)  + tw->pw->angleshift*3.14159/2.0);
		LineTo((int)(nextrscaled*cosa)+xoff,(int)(nextrscaled*sina)+yoff);
		tpos++;
	}
	--tpos;
	
	while (tpos >= tsect->left) {
		cosa = cos((double) *(td->xvals + tpos)  + tw->pw->angleshift*3.14159/2.0);
		sina = sin((double) *(td->xvals + tpos)  + tw->pw->angleshift*3.14159/2.0);
		LineTo((int)(rscaled*cosa)+xoff,(int)(rscaled*sina)+yoff);
		--tpos;
	}
	
	*tsect = tr;
}

/*********************************************************************************/
/*  interprect
*
*  Draw a rectangle on the image which is according to the selection region
*  given in the parameter.
*/
interprect(tw,sel)
	struct Mwin *tw;
	Rect *sel;
{
	Rect tr;
	
	SetPort(tw->win);
	PenMode(patXor);
	
	tr = *sel; 

	tr.right++;
	tr.bottom++;

	offset2size(tw,&tr.left,&tr.top);		/* translate to screen coordinates by scales */
	offset2size(tw,&tr.right,&tr.bottom);
	
	if (tr.right > tr.left)
		tr.right--;							/* come in one from the edge */
	if (tr.bottom > tr.top)
		tr.bottom--;
	
	while (tr.right - tr.left < 6) {		/* minimum size in pixels for little box */
		tr.right++;
		tr.left--;
	}
	while (tr.bottom - tr.top < 6) {
		tr.top--;
		tr.bottom++;
	}
	
	FrameRect(&tr);

}

/*********************************************************************************/
/*  copyimg
*  Do the copybits operation which updates the image window on the screen.
*/
copyimg(tw)
	struct Mwin *tw;
	{
	Rect rfrom,rto;
	CGrafPtr hidep,savep;
	
	GetPort((GrafPtr *) &savep);
	SetPort(tw->win);
	
	hidep = &tw->vdev.vport;
	rfrom = tw->pref;
	
/*
 *  Copy the data from the off-screen Bitmap to the screen.
 */
 
/*	rfrom = (*(hidep->portPixMap))->bounds;		 boundary of data field */
	rto = tw->win->portRect;					/* boundary of drawing area */

	BackColor(whiteColor);
	ForeColor(blackColor);
			
	CopyBits((BitMap *) (*(hidep->portPixMap)), &tw->win->portBits,
			&rfrom, &rto, 0, tw->win->visRgn); 

/*
*  Draw the current selection.
*/
	switch (tw->wintype) {
	case FBI:
	case FIMG:
		littlebox(tw,&rto,0);
		break;
	case FPOL:
		littlesect(tw,&rto,0);
		break;
	}
	
	SetPort((GrafPtr) savep);
	
}

/************************************************************************************/
/* scrapimg
*  Copybits the image window into the clipboard.
*  
*/
scrapimg(tw)
	struct Mwin *tw;
	{
	Rect copysize,copyfrom;
	long len;
	int i;
	PicHandle picture;
	CGrafPtr hidep;
	Handle phandle;
	char *pcomment;
		
	hidep = &tw->vdev.vport;
	copyfrom = tw->pref;
			
	SetPort(tw->win);
	
	copysize = tw->win->portRect;			/* boundary of drawing area */

	ClipRect(&copysize);
	
	phandle = (Handle)NewHandle(6);
	HLock(phandle);
	pcomment = *phandle;
	pcomment[0] = 'N';
	pcomment[1] = 'L';
	pcomment[2] = 'a';
	pcomment[3] = 'o';
	pcomment[4] = 0;
	pcomment[5] = 0;
	HUnlock(phandle);
	
	picture= OpenPicture(&copysize);
	
	PicComment(100,6,phandle);				/* indicates NCSA image */

	BackColor(whiteColor);
	ForeColor(blackColor);
				
	CopyBits((BitMap *) (*(hidep->portPixMap)), &tw->win->portBits,
			&copyfrom, &copysize, srcCopy, NULL); 
			
	ClosePicture();
	
	DisposHandle(phandle);
	
/*
*  put the PICT into the scrap manager
*/
	len = GetHandleSize( (Handle) picture);
	HLock((Handle) picture);
	ZeroScrap();
	if (noErr != (i = (int)PutScrap( len, 'PICT', (Ptr) *picture))) {
		if (i == -108)
			nomem();					/* out of memory */
		ZeroScrap();					/* clear it, no good */
	}

	HUnlock((Handle) picture);
	KillPicture(picture);
	
	UnloadScrap();
	
}

/***************************************************************************************/
/*  mouseimg
*   Do mouse operations in the image window.
*   Make the visible text window reflect the mouse operation.
*/
mouseimg(tw,where,modifiers)
	struct Mwin *tw;
	int modifiers;
	Point *where;
	{
	Point tpt,newpt,nnpt;
	Rect tr;
	struct fdatawin *td;
	struct fimgwin *ti;

	if (tw->wintype == FBI) {
		mouseinterp(tw,where,modifiers);
		return;
	}

	SetPort(tw->win);							/* use this port for actions */
	PenMode(patXor);
	
	ti = tw->iw;
	td = tw->dat;								/* help for later de-referencing */

	FrameRect(&tw->iw->xr);						/* un-draw a little box */
/*
*   Find the floating point number for a spot on the image.
*   Set the selection region and origin for the text window and re-draw it.
*/
	GlobalToLocal(where);
/*
*   perform the drag operation in the local window to identify selection region.
*/
	nnpt = tpt = *where;						/* save anchor point */
	SetRect(&tr, tpt.h, tpt.v, tpt.h, tpt.v);	/* start with one point rect */
	
	while (StillDown()) {
		GetMouse(&newpt);
		if (nnpt != newpt) {					/* something changed */
			nnpt = newpt;
			FrameRect(&tr);						/* un-draw a little box */
			
			if (newpt.v <= tpt.v) {				/* above, move bottom to anchor, top to point */
				tr.bottom = tpt.v;
				tr.top = newpt.v;
			}
			else {								/* below, move top to anchor, bottom to point */
				tr.top = tpt.v;
				tr.bottom = newpt.v;
			}
			if (newpt.h <= tpt.h) {				/* left, move right to anchor, left to point */
				tr.right = tpt.h;
				tr.left = newpt.h;
			}
			else {
				tr.left = tpt.h;
				tr.right = newpt.h;
			}
			
			FrameRect(&tr);							/* draw a new box */
		}
	}
	
	tw->iw->xr = tr;	
/*
*  update the text window and synched windows.
*/
	tr.left /= ti->exx;
	tr.right /= ti->exx;
	tr.top /= ti->exy;
	tr.bottom /= ti->exy;
	
	synchfloat(tw,&tr,1);						/* set the other windows */
	
}

/***************************************************************************************/
/*  mouseinterp
*   Do mouse operations in the image window.
*   The rectangle taken by interprect is in dataset coordinates.  The mouse point is
*   in screen coordinates of course, so we call size2offset to translate.
*
*/
mouseinterp(tw,where,modifiers)
	struct Mwin *tw;
	int modifiers;
	Point *where;
	{
	Point tpt,newpt,nnpt;
	Rect tr;
	struct fdatawin *td;

	SetPort(tw->win);							/* use this port for actions */
	
	td = tw->dat;								/* help for later de-referencing */
	interprect(tw,&tw->bw->xr);					/* un-draw a little box */

/*
*   Find the floating point number for a spot on the image.
*   Set the selection region and origin for the text window and re-draw it.
*/
	GlobalToLocal(where);
	size2offset(tw,&where->h,&where->v);
/*
*   perform the drag operation in the local window to identify selection region.
*/
	nnpt = tpt = *where;							/* save anchor point */
	SetRect(&tr, tpt.h, tpt.v, tpt.h, tpt.v);		/* start with one point rect */
	interprect(tw,&tr);								/* draw a little box */
	
	while (StillDown()) {
		GetMouse(&newpt);
		size2offset(tw,&newpt.h,&newpt.v);
		
		if (nnpt != newpt) {					/* something changed */
			nnpt = newpt;
			interprect(tw,&tr);					/* un-draw a little box */
			
			if (newpt.v <= tpt.v) {				/* above, move bottom to anchor, top to point */
				tr.bottom = tpt.v;
				tr.top = newpt.v;
			}
			else {								/* below, move top to anchor, bottom to point */
				tr.top = tpt.v;
				tr.bottom = newpt.v;
			}
			if (newpt.h <= tpt.h) {				/* left, move right to anchor, left to point */
				tr.right = tpt.h;
				tr.left = newpt.h;
			}
			else {
				tr.left = tpt.h;
				tr.right = newpt.h;
			}
			
			interprect(tw,&tr);					/* draw a new box */
		}
	}
	
	tw->bw->xr = tr;
		
	synchfloat(tw,&tr,1);						/* set the other windows */
	
}

/***************************************************************************************/
/*  mousepol
*   Do mouse operations in the polar image window.
*/
mousepol(tw,where,modifiers)
	struct Mwin *tw;
	int modifiers;
	Point *where;
	{
	Point tpt,newpt,nnpt;
	Rect tr;
	struct fdatawin *td;
	struct fpolwin *tp;

	SetPort(tw->win);							/* use this port for actions */
	
	tp = tw->pw;
	td = tw->dat;								/* help for later de-referencing */
	polarsect(tw,&tw->pw->xr);					/* get rid of old box */

/*
*   Find the floating point number for a spot on the image.
*   Set the selection region and origin for the text window and re-draw it.
*/
	GlobalToLocal(where);
/*
*   perform the drag operation in the local window to identify selection region.
*/
	transpol(tw,where);
	
	nnpt = tpt = *where;						/* save anchor point */
	SetRect(&tr, tpt.h, tpt.v, tpt.h, tpt.v);	/* start with one point rect */
	polarsect(tw,&tr);							/* draw the first sect */
	
	while (StillDown()) {
		GetMouse(&newpt);
		transpol(tw,&newpt);					/* go to polar coords */
		
		if (nnpt != newpt) {					/* something changed */
			nnpt = newpt;
			polarsect(tw,&tr);					/* un-draw a little box */
			
			if (newpt.v <= tpt.v) {				/* above, move bottom to anchor, top to point */
				tr.bottom = tpt.v;
				tr.top = newpt.v;
			}
			else {								/* below, move top to anchor, bottom to point */
				tr.top = tpt.v;
				tr.bottom = newpt.v;
			}
			if (newpt.h <= tpt.h) {				/* left, move right to anchor, left to point */
				tr.right = tpt.h;
				tr.left = newpt.h;
			}
			else {
				tr.left = tpt.h;
				tr.right = newpt.h;
			}
			
			polarsect(tw,&tr);					/* draw a new box */
		}
	}
	
	tw->pw->xr = tr;	
/*
*  update the text window and synched windows.
*/
	synchfloat(tw,&tr,1);						/* set the other windows */
	
}

/***************************************************************************************/
/*  transpol
*   Translate a point from x,y to r,theta.
*/
transpol(tw,ppt)
	struct Mwin *tw;
	Point *ppt;
	{
	int i,j;
	double invtan,sqrt(),atan();
	register float *f;
	float angle,radius;
	struct fdatawin *td;
	
	td = tw->dat;
	
	ppt->h = (ppt->h + tw->pw->viewport.left*tw->pw->exx);	/* offset according to viewing region */
	ppt->v = (ppt->v + tw->pw->viewport.top*tw->pw->exx);

	i = ppt->h*ppt->h+ppt->v*ppt->v;		
	
/* 
*  calculate radius.
*  value in screen coordinates = sqrt(i)
*  value in selection region coordinates is related to value along y scale values
*  of the dataset.  So we divide by ydim/2 and multiply by the yscale range (max - min)
*  We are dealing with a circle, so assume ymin == 0.
*/
	radius = sqrt((double)i)*2.0/(td->ydim*tw->pw->exx) * (*(td->yvals + td->ydim-1));

	invtan = (double)ppt->v/(double)ppt->h;

	angle = atan(invtan);					/*  angle on image, not sign-determined */
	
	if (ppt->h < 0)	{						/*  push angle up depending on quad */
		angle += 3.14159;
	}
	else if (ppt->v < 0)					/* fourth quad creates -angle */
		angle += 2*3.14159;
		
/*
*  adjust by angleshift
*/
	angle -=  tw->pw->angleshift*3.14159/2.0;
	
	if (angle < 0)
		angle += 2*3.14159;

/*
*  find angle and radius value by searching x and y dim lists.
*/
	f = td->xvals;							/* angle values */
	
	for (i=0; i < td->xdim-1 && *f < angle ; i++,f++)
		;
	
	f = td->yvals;							/* radius values */
	
	for (j=0; j< td->ydim-1 && *f < radius ; j++,f++)
		;

	ppt->h = i;
	ppt->v = j;	
	
}


/***************************************************************************************/
/*  littlebox
*   Draw the little box in an image window.  XOR the old image off first.
*/
littlebox(tw,sr,drawit)
	struct Mwin *tw;
	Rect *sr;
	int drawit;
	{
	GrafPtr savep;
	Rect tr;
		
	GetPort(&savep);
	
	SetPort(tw->win);
	
/*
*  draw local box 
*/
	PenMode(patXor);

	if (tw->wintype == FIMG) {
		FrameRect(&tw->iw->xr);						/* un-draw a little box */
		if (drawit) {
/*
*  Outline the selected data point by incrementing right side.
*/
			tr = *sr;
			tr.right++;
			tr.bottom++;
/*
*  Expand with the window's expansion factors.
*/
			tr.top *= tw->iw->exy;
			tr.bottom *= tw->iw->exy;
			tr.left *= tw->iw->exx;
			tr.right *= tw->iw->exx;
			
			if (tr.right > tr.left)
				tr.right--;
			if (tr.bottom > tr.top)
				tr.bottom--;
			
/*
*  if the box to draw is too small, make it a little larger
*/
			while (tr.right - tr.left < 6) {
				tr.right++;
				tr.left--;
			}
			while (tr.bottom - tr.top < 6) {
				tr.top--;
				tr.bottom++;
			}
	
			tw->iw->xr = tr;
			FrameRect(&tr);								/* draw a box */
		}
	}
 	else if ( tw->wintype == FBI ) {
		interprect(tw,&tw->bw->xr);						/* un-draw a little box */
		if (drawit) {

			tr = *sr;
			tw->bw->xr = tr;
			interprect(tw,&tr);							/* draw a box */
		}
	
	
	}
	
	PenMode(srcCopy);
	
	SetPort(savep);
}
/***************************************************************************************/
/*  TrashWindow
*   Dump the window and its palette.
*/
TrashWindow (wnd)
	WindowPtr wnd;
{

	PaletteHandle pal;
	
	if (pal = GetPalette (wnd))
		DisposePalette (pal);

	DisposeWindow (wnd);
}

/***************************************************************************************/
/*  loseimage
*   Take an image window out of the list and get rid of all links.
*/
loseimage(tw)
	struct Mwin *tw;
	{

/*
*  Remove the backpointer stored in the data.
*/
	if (tw->dat->image == tw)		/* this should ALWAYS be true */
		tw->dat->image = NULL;
		
	losedat(tw->dat);				/* take away my reference to data */

	TrashVDevice(&tw->vdev);

	DisposPtr ((Ptr) tw->iw);

	rmwin(tw);						/* take window out of list */

	TrashWindow(tw->win);
	DisposPtr ((Ptr) tw);
				
}

/***************************************************************************************/
/*  loseinterp
*   Take an image window out of the list and get rid of all links.
*/
loseinterp(tw)
	struct Mwin *tw;
	{

/*
*  Remove the backpointer stored in the data.
*/
	if (tw->dat->interp == tw)		/* this should ALWAYS be true */
		tw->dat->interp = NULL;
		
	losedat(tw->dat);				/* take away my reference to data */
	
	TrashVDevice(&tw->vdev);

	DisposPtr ((Ptr) tw->bw);

	rmwin(tw);						/* take window out of list */
	
	TrashWindow(tw->win);
	DisposPtr((Ptr) tw);
				
}

/***************************************************************************************/
/*  losepolar
*   Take an image window out of the list and get rid of all links.
*/
losepolar(tw)
	struct Mwin *tw;
	{

/*
*  Remove the backpointer stored in the data.
*/
	if (tw->dat->polar == tw)		/* this should ALWAYS be true */
		tw->dat->polar = NULL;
		
	losedat(tw->dat);				/* take away my reference to data */
	
	TrashVDevice(&tw->vdev);

	DisposPtr((Ptr) tw->pw);

	rmwin(tw);						/* take window out of list */
	
	
	TrashWindow(tw->win);
	DisposPtr((Ptr) tw);
				
}

/***************************************************************************************/
/*  losenotes
*   Take a notes window out of the list and get rid of all links.
*/
losenotes(tw)
	struct Mwin *tw;
	{

/*
*  Remove the backpointer stored in the data.
*/
	if (tw->dat->notes == tw)		/* this should ALWAYS be true */
		tw->dat->notes = NULL;
		
/*
*  Keep the textedit data in case the dataset should retain notes
*/
	tw->dat->content = TEGetText(tw->nw->trec);			/* get the text handle */
	tw->dat->contentlen = (*tw->nw->trec)->teLength;	/* length of that text */
	(*tw->nw->trec)->hText = NULL;						/* get rid of old pointer */
		
	losedat(tw->dat);				/* take away my reference to data */
	
	TEDispose(tw->nw->trec);		/* get rid of textedit record */
	
	DisposPtr((Ptr) tw->nw);

	rmwin(tw);						/* take window out of list */
	
	
	DisposeWindow(tw->win);
	DisposPtr((Ptr) tw);
				
}

/***************************************************************************************/
/*  losedat
*   Check the refcount and get rid of the data memory.
*   The refcount makes sure that all references are gone before the data structure is
*   freed.  Backpointers should all be NULL by the time this routine is called for the
*   final deallocation.
*/

losedat(thedat)
	struct fdatawin *thedat;
	{
	
	if (!thedat)
		return;
		
	if (--thedat->refcount)				/* refcount must be zero to DisposPtr it */
		return;
		
	if (thedat->content)				/* if we have saved notes, send them */
		DisposHandle((Handle) thedat->content);
		
	DisposPtr((Ptr) thedat->vals);					/* DisposPtr all allocated memory */
	DisposPtr((Ptr) thedat->xvals);
	DisposPtr((Ptr) thedat->yvals);
	DisposPtr((Ptr) thedat);
	
}
	
	
/*******************************************************************************/
/*  loadpal
*   prompt and load a palette for the current image window.
*   Looks in HDF files only for first IP8 tag, ref==1.
*/
extern unsigned char *rgbsave;

loadpal(tw)
	struct Mwin *tw;
{	
	 SFReply reply;
	 Point wh;
	 int i;

	wh.h = wh.v = 50;
	sfgetfile(&wh, "Load from which HDF file?", nil, -1, nil, nil, &reply);
	if (reply.good) {
		p2cstr(reply.fName);
		setvol(NULL, reply.vRefNum);					/* set to this volume (dir) */
		if (0 <= dfpal(reply.fName,rgbsave))			/* load this file */
			setpal(tw,rgbsave);
		else {
			paramtext(reply.fName,"","","");
			i = StopAlert(1005, nil);
		}
	}
	

}

