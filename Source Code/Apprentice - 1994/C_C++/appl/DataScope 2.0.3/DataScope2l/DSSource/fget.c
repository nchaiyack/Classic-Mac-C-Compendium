#include "macfview.h"
#include "df.h"

/****************************************************************************/

dfpal(filename,rgb)
	char *filename,*rgb;
	{
	DF *fdf;
	int ret;
	DFdesc dfd;
	
	if (NULL == (fdf = DFopen(filename,DFACC_READ,16))) {
		return(-1);
	}

	DFsetfind(fdf, DFTAG_IP8, DFTAG_WILDCARD);			/* the first palette in the file */
	if (DFfind(fdf, &dfd))
		return(-1);

	if (0 > DFaccess( fdf, DFTAG_IP8, dfd.ref, "r"))	/* open section of file */
		return(-1);
		
	if (0 > (ret = DFread( fdf, rgb, 768)))
		return(-1);

	DFclose(fdf);
	
	return(0);
}



extern struct {
	int
		sref,					/* return flag for sdg */
		bref,					/* ref #'s for each of these */
		iref,
		pref,
		bxx,bxy,				/* expansion factors for block image */
		ixx,ixy,				/* interpolated image factors */
		pxx,					/* expansion for polar image */
		ashift,
		dleft,
		dtop,					/* viewport for polar image */
		dright,
		dbottom,
		dxsize,					/* sizes for interp image windows */
		dysize;
		
} dsinfo;

/****************************************************************************/
/*  getdf
*   Given a filename, we are responsible for reading in all of the relevant
*   data from that file.  This may include, floating point data, images, and
*   notes which are stored with the dataset.
*
*   For each type of data in the file, call the allocation routines, read the
*   data and then activate the data by storing it into the window list.
*/

extern unsigned char *rgbsave;

getdf(filename)
	char *filename;
	{
	int prank,sizes[2],xdim,ydim,i,filestat;
	
	filestat = -1;
	DFSDrestart();
	
	
/*
*  open DF file to read in special record
*/
	while (0 <= (i = DFSDgetdims(filename, &prank, sizes, 2)) && prank == 2) {
	
		if (filestat <= 0) {
			/* dialog prompt */
				filestat = 0;		/* ok */
				/* filestat = 1;  read them all */
		}
		
		readdf(filename,sizes);

	}
	
	if (filestat < 0) {					/* no files were read */
		extern int DFerror;
		paramtext(filename,"","","");
		NoteAlert(1001, nil);
		return(-1);
	}
	
	return(0);
}

readdf(filename,sizes)
	char *filename;
	int sizes[];
	{
	char s[255],*p,*q,*qbase;
	DFdesc dfd,dfd2;
	DF *fdf;
	struct fdatawin *td,*newdatawin();
	int xdim,ydim,i,j,sref,ispal;
	float *f;
	
	ydim = sizes[0];
	xdim = sizes[1];

/*
*   create the dataset, read it in and install it into the window 
*   structure for DataScope.
*/
	td = newdatawin(filename,xdim,ydim);				/* allocates all mem */
	if (NULL == td)
		return(-1);
		
	strncpy(td->fname,filename,sizeof(td->fname));		/* keep file name */
	td->valmax = td->valmin = 0.0;
		
	DFSDgetdata(filename, 2, sizes, td->vals);
	
	DFSDgetmaxmin(&td->valmax, &td->valmin);

	if (0 > DFSDgetdimscale(1, ydim, td->yvals)) {		/* load scale values */
		f = td->yvals;
		for (i=0; i<ydim ; i++)
			*f++ = (float)i;							/* defaults if they are not there */
	}

	if (0 > DFSDgetdimscale(2, xdim, td->xvals)) {		/* load scale values */
		f = td->xvals;
		for (i=0; i<xdim ; i++)
			*f++ = (float)i;							/* defaults if they are not there */
	}
	
	DFSDsetlengths(sizeof(td->dvar), 255, sizeof(td->fmt), 255);	
	DFSDgetdatastrs(td->dvar, s, td->fmt, s);	

	DFSDsetlengths(sizeof(td->xvar), 255, sizeof(td->labfmt), 255);	
	DFSDgetdimstrs(1, td->yvar, s, td->labfmt);
	DFSDgetdimstrs(2, td->xvar, s, td->labfmt);

/*
*  check for what we were expecting from the labels.  Accepts C or Fortran formats.
*/
	if (td->fmt[0] != '%')
		ftn2c(td->fmt,td->fmt,"%11.4e");
	if (td->labfmt[0] != '%')
		ftn2c(td->labfmt,td->labfmt,"%5.1f");
		
	sref = DFSDlastref();

/*
*   Now that we have all of the data, install it into the window list.
*/
	ctextwin(td);

/*
*  If max and min are wrong, re-calculate.
*/
	if (td->valmax <= td->valmin) {
		maxmin( td->vals, xdim, ydim,
				&td->valmax, &td->valmin, 0,0, ydim, xdim);
	}

/*
*   Get the information from the DataScope text record.
*   This will fill the dsinfo record with the ref #s of the records which
*   we are concerned with and will provide the expansion factors which should
*   be kept for the images.
*/
	if (NULL == (fdf = DFopen(filename,DFACC_READ,16))) {
		return(-1);
	}
	
	if (0 <= DFaccess( fdf, 8000, sref, "r")) { 	/* open annotation */
		i = DFread(fdf, s, 250);
		Sconf(s, i);
	}

	DFclose(fdf);
	
	DFR8restart();
/*
*  Read in any image windows which are stored.  Display them, etc.
*/
	if (dsinfo.bref > 0) {
		td->exx = dsinfo.bxx;				/* set the expansion factors */
		td->exy = dsinfo.bxy;
		td->xsize = td->exx*td->xdim;		/* set sizes */
		td->ysize = td->exy*td->ydim;
		
		if (0 <= DFR8getdims(filename, &i, &j, &ispal)) {
			emptyimage(td->text,FIMG);		/* create an empty image window */
			DFR8getimage(filename, td->image->vdev.bp, i, j, rgbsave);
			if (ispal)
				setpal(td->image,rgbsave);
		}
	}

	if (dsinfo.iref > 0) {					/* interpolated image */
		td->exx = dsinfo.ixx;				/* set the expansion factors */
		td->exy = dsinfo.ixy;
		td->xsize = td->exx*td->xdim;		/* set sizes */
		td->ysize = td->exy*td->ydim;
		if (dsinfo.dysize > 0)	{			/* is the image size in the file? */
			td->ysize = dsinfo.dysize;
			td->exy = td->ysize/td->ydim;
		}
		if (dsinfo.dxsize > 0) {
			td->xsize = dsinfo.dxsize;
			td->exx = td->xsize/td->xdim;
		}

		if (0 <= DFR8getdims(filename, &i, &j, &ispal)) {
			emptyimage(td->text,FBI);		/* create an empty image window */
			DFR8getimage(filename, td->interp->vdev.bp, i, j, rgbsave);
			if (ispal)
				setpal(td->interp,rgbsave);
		}
	}

	if (dsinfo.pref > 0) {			/* interpolated image */
		td->exy = td->exx = dsinfo.pxx;		/* set the expansion factors */
		if (dsinfo.dright > dsinfo.dleft) {	/* we have a viewport */
			td->viewport.left = dsinfo.dleft;
			td->viewport.top = dsinfo.dtop;
			td->viewport.right = dsinfo.dright;
			td->viewport.bottom = dsinfo.dbottom;
			td->angleshift = dsinfo.ashift;
		}
		else {
			SetRect(&td->viewport, -j/2, -j/2, j/2, j/2);
			td->angleshift = 0;
		}
		if (0 <= DFR8getdims(filename, &i, &j, &ispal)) {
			emptyimage(td->text,FPOL);		/* create an empty image window */
			DFR8getimage(filename, td->polar->vdev.bp, 
				i, j, rgbsave);
			if (ispal)
				setpal(td->polar,rgbsave);
				
		}
	}

/*
*  If there are notes, read it from the file and create the notes window.
*/

	if (NULL == (fdf = DFopen(filename,DFACC_READ,16))) {
		return(-1);
	}
		
	DFsetfind(fdf, DFTAG_DIA, sref);				/* annotation for the SDG we just read */
	if (DFfind(fdf, &dfd))
		{
		DFclose(fdf);
		return(-1);
		}

	if (0 <= DFaccess( fdf, DFTAG_DIA, dfd.ref, "r")) { 	/* open annotation */

		DFread( fdf, &dfd2.tag, 4);							/* read tag/ref from DIA */
/*
*  for the length of the annotation, read it in and change EOLs
*/
		td->contentlen = dfd.length-4;
		td->content = (CharsHandle)NewHandle(td->contentlen);
		
		HLock((Handle) td->content);
		
		DFread( fdf, *td->content, td->contentlen);
		/*
		*  convert EOLs from CRLF to CR
		*/
		q = p = qbase = (char *) *td->content;
		i = td->contentlen;
		while (--i > 0 && *p) {
			*q++ = *p++;
			if (*p == 10) {							/* skip LFs */
				p++; --i;
			}
		}
		
		td->contentlen = q - qbase;					/* new length */
		
		HUnlock((Handle) td->content);
		
		SetHandleSize( (Handle) td->content,td->contentlen);	/* set new length */
		
		/*
		*  Use the just-created text window as a jumping off point and
		*  create a visible notes window to display the notes we just read.
		*/
		if (td->text)
			newnotes(td->text);
		
	}
	
	DFclose(fdf);
	return(0);
}

#ifdef DEBUGxx
/***************************************************************************/
/* specialdebug
*  create a dataset from the image portion of a window.
*/
specialdebug(tw)
	struct Mwin *tw;
	{
	unsigned char *p;
	int wid,i,j;
	float *f;
	struct fdatawin *td;
	
	p = tw->vdev.bp;
	
	/* wid = (*(gp->portPixMap))->rowBytes & 0x7fff;	 resetting high flag bit */
	wid = 100;
	if (wid < 0 || wid > 1000)
		wid = 100;
		
	td = newdatawin("test",wid,10);
	
	f = td->xvals;
	for (i=0; i<td->xdim; i++)
		*f++ = (float)i;
		
	f = td->yvals;
	for (i=0; i<td->ydim; i++)
		*f++ = (float)i;
		
	f = td->vals;
	for (i=0; i<td->xdim*td->ydim; i++)
		*f++ = (float)*p++;
		
	ctextwin(td);
	
}
#endif

/***************************************************************************/
/*  gettext
*  Read in a text file which has the correct format.  Install it into the
*  window list as a text array.
*/
gettext(filename)
	char *filename;
	{
	FILE *fp;
	int i,j,xdim,ydim;
	float *f;
	struct fdatawin *td;
	
	if (NULL == (fp = fopen(filename,"r"))) 
		return(-1);

	if (2 != fscanf(fp,"%d %d",&ydim,&xdim)
		|| xdim < 2 || ydim < 2) {
		return(-1);
	}
	
/*
*   create the dataset, read it in and install it into the window 
*   structure for DataScope.
*/
	if (NULL == (td = newdatawin(filename,xdim,ydim)))
		return(-1);
	
	strncpy(td->fname,filename,sizeof(td->fname));		/* keep file name */
	
	if (calcidle()) {
		losedat(td);
		return(-1);
	}
/*
*  look for max/min
*/
	if (2 != fscanf(fp,"%e %e",&td->valmax, &td->valmin)) {
		losedat(td);
		return(-1);
	}

/*
*  the x and y scale arrays
*/
	f = td->yvals;
	for (i=0 ; i<ydim; i++)
		if (1 != fscanf(fp,"%e",f++)) {
			losedat(td);
			return(-1);
		}
	*f = *(f-1);

	f = td->xvals;
	for (i=0 ; i<xdim; i++)
		if (1 != fscanf(fp,"%e",f++)) {
			losedat(td);
			return(-1);
		}
	*f = *(f-1);
	
	if (calcidle()) {
		losedat(td);
		return(-1);
	}
		
/*
*  the data array 
*/
	f = td->vals;

	for (i=0; i< ydim; i++) {
		if (calcidle()) {
			losedat(td);
			return(-1);
		}
		for (j=0; j<xdim; j++)
			if (1 != fscanf(fp,"%e",f++)) {
				losedat(td);
				return(-1);
			}
	}


	fclose(fp);
	
/*
*  If we get this far, then we assume things are ok and we start 'er up.
*/
	ctextwin(td);
	
/*
*  If max and min are wrong, re-calculate.
*/
	if (td->valmax <= td->valmin) {
		maxmin( td->vals, xdim, ydim,
				&td->valmax, &td->valmin, 0,0, ydim, xdim);
	}

	return(0);
	
}

/***************************************************************************/
/* setapal
*  Take a window pointer and save its color palette into a 768 byte look-up
*  table for association with a saved image.
*/
setapal(gp)
	WindowPtr gp;
	{
	register int i;
	unsigned char *p,rgb[770];
	RGBColor clor;
	PaletteHandle pal;
/*
*  prep the data structure to be saved.
*  If there is a color map, make arrangements to save it too.
*/
		p = NULL;
		pal = GetPalette(gp);
		p = rgb;
		for (i=0; i<255; i++) {
			GetEntryColor(pal, i, &clor);
			*p++ = clor.red >> 8;
			*p++ = clor.green >> 8;
			*p++ = clor.blue >> 8;
		}
		
	DFR8setpalette(rgb);
}


/***************************************************************************/
/*
*  Save the floating point numbers as an SDG and each image window as a
*  RIG image with appropriate palette.
*/

savedf(filename,td)
	unsigned char *filename;
	struct fdatawin *td;
	{
	int dims[2],sref,trefs,i,buflen;
	DF *fdf;
	DFdesc dfd;
	char *sp,*spbase,*nt,s[200];
	
	dsinfo.sref = -1;
	dsinfo.bref = -1;
	dsinfo.iref = -1;
	dsinfo.pref = -1;

	dims[0] = td->ydim;
	dims[1] = td->xdim;
	
	strncpy(td->fname,filename,sizeof(td->fname));		/* keep file name as we save it */
	td->needsave = 0;									/* don't need save anymore */
	
	DFSDsetdims(2, dims);
	
/*
*  Use SDG to store the floating point data.
*/
	DFSDsetdatastrs(td->dvar, "", td->fmt, "cartesian");
	DFSDsetdimstrs(1, td->yvar, "", td->labfmt);
	DFSDsetdimstrs(2, td->xvar, "", td->labfmt);
	
	DFSDsetdimscale(1, dims[0], td->yvals);
	DFSDsetdimscale(2, dims[1], td->xvals);
	
	DFSDsetmaxmin(td->valmax, td->valmin);
			
	DFSDputdata(filename, 2, dims, td->vals);
	
	dsinfo.sref = sref = DFSDlastref();			/* ref # for SDG we just put out */
	dfd.tag	= DFTAG_SDG;
	dfd.ref = sref;								/* save info for DIA storage */
/*
*  If there are associated images, save them, too. 
*/
	
	DFR8restart();
	if (td->image) {
		setapal(td->image->win);
		DFR8addimage(filename, td->image->vdev.bp, 
		td->image->vdev.bounds.right - td->image->vdev.bounds.left, 
		td->image->vdev.bounds.bottom - td->image->vdev.bounds.top, 0);
		dsinfo.bref = DFR8lastref();
		dsinfo.bxx = td->image->iw->exx;
		dsinfo.bxy = td->image->iw->exy;
	}
		
	if (td->interp) {
		setapal(td->interp->win);
		DFR8addimage(filename, td->interp->vdev.bp, 
			(long) td->interp->bw->xsize, 
			(long) td->interp->bw->ysize, (long) 0);
		dsinfo.iref = DFR8lastref();
		dsinfo.dxsize = td->interp->bw->xsize;
		dsinfo.dysize = td->interp->bw->ysize;
	}

	if (td->polar) {
		setapal(td->polar->win);
		DFR8addimage(filename, td->polar->vdev.bp, 
		td->polar->vdev.bounds.right - td->polar->vdev.bounds.left, 
		td->polar->vdev.bounds.bottom - td->polar->vdev.bounds.top, 0);
		dsinfo.pref = DFR8lastref();
		dsinfo.pxx = td->polar->pw->exx;
		dsinfo.dleft = td->polar->pw->viewport.left;
		dsinfo.dtop = td->polar->pw->viewport.top;
		dsinfo.dright = td->polar->pw->viewport.right;
		dsinfo.dbottom = td->polar->pw->viewport.bottom;
		dsinfo.ashift = td->polar->pw->angleshift;
	}
	
		
/*
*  save notebook too, if we can
*/
	if (td->notes) {
		td->content = TEGetText(td->notes->nw->trec);			/* get the text handle */
		td->contentlen = (*td->notes->nw->trec)->teLength;		/* length of that text */
			
		if (NULL == (fdf = DFopen(filename,DFACC_WRITE,16))) {
			return(-1);
		}
	
		DFaccess( fdf, DFTAG_DIA, sref, "w");		/* open annotation */
	
		DFwrite( fdf, &dfd.tag, 4);					/* write tag/ref into DIA */
		
		spbase = sp = (char *) NewPtr(2010);
		HLock(( Handle) td->content);
		nt = (char *) *td->content;					/* pointer to notebook */
		buflen = td->contentlen;
		i = 0;										/* make sure all init'ed, no stupidity */
		while (buflen > 0) {
			while (buflen > 0 && i < 2000) {
				*sp++ = *nt;
				if (*nt++ == 13) {					/* augment EOLs with LFs */
					*sp++ = 10;
					i++;
				}
				--buflen;
				i++;
			}
			DFwrite( fdf, spbase, i);
			sp = spbase;
			i=0;
		}
			
		DFwrite( fdf, &i, 4);						/* write out zeros */
				
		DisposPtr(spbase);								/* give back temporary mem */
		HUnlock((Handle) td->content);
	
		DFclose(fdf);

	}

/*
*  Write out custom record.
*  sprintf it into a string and then write it into the HDF file.
*/
	if (NULL == (fdf = DFopen(filename,DFACC_WRITE,16))) {
		return(-1);
	}
	sprintf(s,"# DataScope record \n block=%d,%d,%d\n vimage=%d,%d,%d\n polar=%d,%d\n pview=%d,%d,%d,%d,%d\n",
		dsinfo.bref,dsinfo.bxx,dsinfo.bxy,
		dsinfo.iref,dsinfo.dxsize,dsinfo.dysize,
		dsinfo.pref,dsinfo.pxx,
		dsinfo.ashift,dsinfo.dleft,dsinfo.dtop,dsinfo.dright,dsinfo.dbottom);

	DFputelement(fdf, 8000, dsinfo.sref, s, strlen(s));
	
	DFclose(fdf);
	
	return(0);
}

/***************************************************************************/
/*  maxmin
*   Calculate the maximum and minimum values for a region in the array.
*
*/
maxmin( arr, xd, yd, mx, mn, ptop, pleft, pbottom, pright )
	float *arr, *mx, *mn;
	int xd,yd,ptop,pleft,pbottom,pright;
	{
	register int i;
	int skip,line;
	register float *f,tmax,tmin;

	skip = xd - (pright - pleft);		/*  remainder of each row */
	
	f = arr + ptop*xd + pleft;			/* offset of start */
	tmax = tmin = *f;					/* set to first value */

	for (line=ptop; line < pbottom; line++) {
		for (i=pleft; i<pright; i++) {
			if (*f > tmax)				/*  new max ? */
				tmax = *f;
			else if (*f < tmin)			/*  new min ? */
				tmin = *f;
			f++;
		}
		f += skip;
	}

	*mx = tmax;
	*mn = tmin;
}

