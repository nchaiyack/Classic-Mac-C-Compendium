/**************************************************************************/
/*  NCSA DataScope
*   An experiment with real numbers.
*   by Tim Krauskopf
*   
*   National Center for Supercomputing Applications
*   University of Illinois at Urbana-Champaign
*   605 E. Springfield Ave.
*   Champaign, IL  61820
*
*   email:  		softdev@ncsa.uiuc.edu
*   bug reports:	bugs@ncsa.uiuc.edu
*   server:         ftp.ncsa.uiuc.edu  (128.174.20.50)
*
*   NCSA DataScope is in the public domain.  See the manual for a complete
*   permissions statement.  We ask that the following message be included
*   in all derivative works:
*   Portions developed at the National Center for Supercomputing Applications
*   University of Illinois at Urbana-Champaign.
*
*   version                      comments
*   -----------                -------------
*   1.0 TKK  December 1988
*   1.1 TKK  May 1989       -- new polar, computations, interpolation
*   1.2 TKK  January 1990   -- networking additions
*/
/*
*  DS_serve
*
*  This server sits on a unix machine and waits 
*  for a DataScope client to send it work to do.
*
*  It is run from rexecd, so it reads and writes to/from stdin
*  and stdout.
*
*  Calls are made to C or FORTRAN subroutines to execute user-declared
*  functions.  See dsfn.h and documentation for declaration of functions.
*
*
*  September, 1989
*  Tim Krauskopf
*/


#include <stdio.h>


#ifdef UNICOS
#define INTOFF 4		/* distance from beginning of int to 32 bit half */
#define FLOFF 4			/* offset for floating point length diff */
#define FLEN 8			/* floating point number length, for readability */

#endif


#ifdef SUN
#define INTOFF 0
#define FLOFF 0
#define FLEN 4

#endif SUN


/********************************************************************/

main(argc,argv)
	int argc;
	char *argv[];
	{
	int i,cd,ret;
	char *malloc();
	char dowhat[5];

	cd = 0;
/*
*  Send the first byte to indicate that we are ready to receive.
*/
	fullmsg("\0");

/*
*  Check the first four characters to make sure they aren't coming from some
*  other client who doesn't know what is going on.
*/
	if (4 == read(cd,dowhat,4)) {

		if (!strncmp("DSfn",dowhat,4))
		{			
			DSfn(cd);					/* DataScope fn */
		}
		else
			fullmsg("DS_serve: Unsupported function for this server");
	}

	fullmsg("DS_serve: Cannot read startup code.");
}


#include "DScope.h"
#include "dsfn.h"
#define DS_ERROR -1
#define DS_CONSTANT 0
#define DS_ARRAY 1

DSfn(cd)
	int cd;
{
	scope_array lft,rgt,answer;

	int i,j,cnt,atemp;
	char *malloc(),fn[256];
	float *f,*fr,*fc;


	if (0 > readtonul(cd,fn))			/* get name of function */
		return(-1);

/*
*  Read parameter information, first left, then right.
*  The 13 bytes are required, then an optional array, depending upon
*  the setting of the "kind" subfield.
*/
	read(cd,&lft.kind,1);
	lft.cval = 0.0;
	read(cd,(char *)&lft.cval,4);
	flconvert(&lft.cval,1);
	lft.nrows = 0;
	read(cd,INTOFF + (char *)&lft.nrows,4);
	lft.ncols = 0;
	read(cd,INTOFF + (char *)&lft.ncols,4);

	if (lft.kind == DS_ARRAY) {		/* read array in */
		lft.rows = (float *)malloc(FLEN*lft.nrows);
		lft.cols = (float *)malloc(FLEN*lft.ncols);
		lft.vals = (float *)malloc(FLEN*lft.ncols*lft.nrows);

		if (0 > fullread(cd,(char *)lft.rows,4*lft.nrows))
			return(-1);
		flconvert(lft.rows,lft.nrows);

		if (0 > fullread(cd,(char *)lft.cols,4*lft.ncols))
			return(-1);
		flconvert(lft.cols,lft.ncols);

		if (0 > fullread(cd,(char *)lft.vals,4*lft.nrows*lft.ncols))
			return(-1);
		flconvert(lft.vals,lft.ncols*lft.nrows);

	}

	read(cd,&rgt.kind,1);
	rgt.cval = 0.0;
	read(cd,(char *)&rgt.cval,4);
	flconvert(&rgt.cval,1);
	rgt.nrows = 0;
	read(cd,INTOFF + (char *)&rgt.nrows,4);
	rgt.ncols = 0;
	read(cd,INTOFF + (char *)&rgt.ncols,4);

	if (rgt.kind == DS_ARRAY) {		/* read array in */
		rgt.rows = (float *)malloc(FLEN*rgt.nrows);
		rgt.cols = (float *)malloc(FLEN*rgt.ncols);
		rgt.vals = (float *)malloc(FLEN*rgt.ncols*rgt.nrows);

		if (0 > fullread(cd,(char *)rgt.rows,4*rgt.nrows))
			return(-1);
		flconvert(rgt.rows,rgt.nrows);

		if (0 > fullread(cd,(char *)rgt.cols,4*rgt.ncols))
			return(-1);
		flconvert(rgt.cols,rgt.ncols);

		if (0 > fullread(cd,(char *)rgt.vals,4*rgt.nrows*rgt.ncols))
			return(-1);
		flconvert(rgt.vals,rgt.nrows*rgt.ncols);

	}

	answer.kind = DS_CONSTANT;
	if (lft.kind == DS_ARRAY) {
		answer.kind = DS_ARRAY;
		answer.nrows = lft.nrows;
		answer.ncols = lft.ncols;
		fr = lft.rows; 
		fc = lft.cols;
	}
	else if (rgt.kind == DS_ARRAY) {
		answer.kind = DS_ARRAY;
		answer.nrows = rgt.nrows;
		answer.ncols = rgt.ncols;
		fr = rgt.rows; 
		fc = rgt.cols;
	}
	atemp = 0;
	if (answer.kind == DS_ARRAY) {
		atemp = 1;						/* we malloced for answer */
		answer.rows = (float *)malloc(FLEN*answer.nrows);
		answer.cols = (float *)malloc(FLEN*answer.ncols);
		answer.vals = (float *)malloc(FLEN*answer.ncols*answer.nrows);

		f = answer.rows; 				/* copy row and col labels */
		for (i=0; i<answer.nrows; i++)
			*f++ = *fr++;
		f = answer.cols;
		for (i=0; i<answer.ncols; i++)
			*f++ = *fc++;
	}

/*
*  call processing routine
*/

	doDSfn(fn,&lft,&rgt,&answer);
/*
* write answer back 
*/
	cd = 1;
	write(cd,"DSfn",4);
	write(cd,&answer.kind,1);
	flbackconvert(&answer.cval,1);
	write(cd,(char *)&answer.cval,4);

	write(cd,INTOFF + (char *)&answer.nrows,4);
	write(cd,INTOFF + (char *)&answer.ncols,4);

	if (answer.kind == DS_ARRAY) {

		flbackconvert(answer.rows,answer.nrows);
		if (0 > fullwrite(cd,(char *)answer.rows,4*answer.nrows))
			return(-1);

		flbackconvert(answer.cols,answer.ncols);
		if (0 > fullwrite(cd,(char *)answer.cols,4*answer.ncols))
			return(-1);

		flbackconvert(answer.vals,answer.nrows*answer.ncols);
		if (0 > fullwrite(cd,(char *)answer.vals,4*answer.nrows*answer.ncols))
			return(-1);
	}

	if (lft.kind == DS_ARRAY) {
		free(lft.rows);
		free(lft.cols);
		free(lft.vals);
	}
	if (rgt.kind == DS_ARRAY) {
		free(rgt.rows);
		free(rgt.cols);
		free(rgt.vals);
	}
	if (atemp) {
		free(answer.rows);
		free(answer.cols);
		free(answer.vals);
	}

	return(0);
}

/**********************************************************************/
/*  doDSfn
*   perform the function we are here to do.
*/
doDSfn(fn,lft,rgt,answer)
	char *fn;
	scope_array *lft,*rgt,*answer;
{
	int i,lim;
	float *fl,*fr;
	char msg[256];

/*
*  First, check the C routines
*/
	i = 0;
	while ( dsc[i].namestring[0] ) {

	/*
	*  If we find a match, make the function call and then return.
	*/
		if (!strcmp(dsc[i].namestring,fn)) {
			(*dsc[i].fncall)(lft,rgt,answer);
			return;
		}

		i++;
	}

/*
*  Now, the FORTRAN routines
*/
	i = 0;
	while ( dsf[i].namestring[0] ) {
	
	/*
	*  If we find a match, make the function call and then return.
	*  For FORTRAN, use the call:
	*     subr(vals,rows,cols,nrows,ncols,maxr,maxc,p)
	*     INTEGER maxr,maxc,p                    // array limits, uses extra space sometimes
	*     REAL vals(maxc,maxr,0:p)               // the array values for answer, parms
	*     REAL rows(maxr,0:p),cols(maxc,0:p)     // the scale values for answer, parms
	*     REAL nrows(0:p),ncols(0:p)             // the number of rows,cols, use 1 for constants
	*
	*/
	
		if (!strcmp(dsf[i].namestring,fn)) {
		
			register int a,b;
			float *vals,*rows,*cols;
			register float *f,*ft;
			int nrows[3],ncols[3],maxr,maxc,p;			/* p will always be 2, even if one unused */

/*
*  set up master arrays, first find largest row and column girth for multiple array
*/
			maxr = 0;
			maxc = 0;
			p = 2;

			if (lft->kind == DS_CONSTANT) {
				if (maxr < 1) maxr = 1;
				if (maxc < 1) maxc = 1;
				nrows[1] = 1;
				ncols[1] = 1;
			} else {
				if (maxr < lft->nrows) maxr = lft->nrows;
				if (maxc < lft->ncols) maxc = lft->ncols;	
				nrows[1] = lft->nrows;
				ncols[1] = lft->ncols;
			}
			if (rgt->kind == DS_CONSTANT) {
				if (maxr < 1) maxr = 1;
				if (maxc < 1) maxc = 1;
				nrows[2] = 1;
				ncols[2] = 1;
			} else {
				if (maxr < rgt->nrows) maxr = rgt->nrows;
				if (maxc < rgt->ncols) maxc = rgt->ncols;	
				nrows[2] = rgt->nrows;
				ncols[2] = rgt->ncols;
			}
			if (answer->kind == DS_CONSTANT) {
				if (maxr < 1) maxr = 1;
				if (maxc < 1) maxc = 1;
				nrows[0] = 1;
				ncols[0] = 1;
			} else {
				if (maxr < answer->nrows) maxr = answer->nrows;
				if (maxc < answer->ncols) maxc = answer->ncols;	
				nrows[0] = answer->nrows;
				ncols[0] = answer->ncols;
			}
			
			if ((NULL == (vals = (float *)malloc(3*maxr*maxc*sizeof(float)))) ||
				(NULL == (rows = (float *)malloc(3*maxr*sizeof(float)))) ||
				(NULL == (cols = (float *)malloc(3*maxc*sizeof(float))))) {
				answer->kind = DS_ERROR;
				return;
			}
/*
*  fill in those master arrays, first the data values
*/			
			f = lft->vals;
			if (lft->kind == DS_CONSTANT) f = &lft->cval;
			ft = vals + maxr*maxc;					/* skip to left parm array */
			for (a=0; a < nrows[1]; a++) {
				for (b=0; b < ncols[1]; b++)
					*ft++ = *f++;

				ft += maxc - ncols[1];				/* skip space */
			}
				
			f = rgt->vals;
			if (rgt->kind == DS_CONSTANT) f = &rgt->cval;
			ft = vals + 2*maxr*maxc;					/* skip to right parm array */
			for (a=0; a < nrows[2]; a++) {
				for (b=0; b < ncols[2]; b++)
					*ft++ = *f++;

				ft += maxc - ncols[2];				/* skip space */
			}

/*
*  Now the scale values for each of the rows and columns
*/
			if (answer->kind == DS_ARRAY) {			
				f = answer->rows;
				ft = rows;
				for (a=0; a < nrows[0]; a++)
					*ft++ = *f++;
				f = answer->cols;
				ft = cols;
				for (a=0; a < ncols[0]; a++)
					*ft++ = *f++;
			}
			if (lft->kind == DS_ARRAY) {			
				f = lft->rows;
				ft = rows + maxr;
				for (a=0; a < nrows[1]; a++)
					*ft++ = *f++;
				f = lft->cols;
				ft = cols + maxc;
				for (a=0; a < ncols[1]; a++)
					*ft++ = *f++;
			}
			if (rgt->kind == DS_ARRAY) {			
				f = rgt->rows;
				ft = rows + 2*maxr;
				for (a=0; a < nrows[2]; a++)
					*ft++ = *f++;
				f = rgt->cols;
				ft = cols + 2*maxc;
				for (a=0; a < ncols[2]; a++)
					*ft++ = *f++;
			}
			
/*
*  now finally make the call
*/
			
			(*dsf[i].fncall)(vals,rows,cols,nrows,ncols,
				&maxr,&maxc,&p);
				
/*
*  put the answer where it belongs and free the temp memory
*/
			f = answer->vals;
			if (nrows[0] <= 1 && ncols[0] <= 1) {
				answer->kind = DS_CONSTANT;
				f = &answer->cval;
			}
			ft = vals;
			for (a=0; a < nrows[0]; a++) {			/* data values themselves */
				for (b=0; b < ncols[0]; b++)
					*f++ = *ft++;

				ft += maxc - ncols[0];				/* skip space */
			}

			if (answer->kind == DS_ARRAY) {			/* row and column scales */	
				f = answer->rows;
				ft = rows;
				for (a=0; a < nrows[0]; a++)
					*f++ = *ft++;
				f = answer->cols;
				ft = cols;
				for (a=0; a < ncols[0]; a++)
					*f++ = *ft++;
			}


			free(vals);
			free(rows);
			free(cols);
			
			return;
		}

		i++;
	}

/*
*  We did not find the function name in either the FORTRAN or C lists.
*/
	sprintf(msg,"DS_serve: Function not found \n> %s",fn);
	fullmsg(msg);
	answer->kind = DS_ERROR;
	return;

}

/**********************************************************************/
/*  fullread
*   read a full segment from the network.
*   returns 0 for successful read, -1 for error 
*/
fullread(skt,whereread,toread)
	int skt,toread;
	char *whereread;
{
	int cnt;

	while (toread > 0) {			/* count of remaining bytes to read */

		cnt = read(skt,whereread,toread);		/* read a chunk */
		if (cnt < 0)				/* connection broken */
			return(-1);

		toread -= cnt;				/* adjust counters for what was read */
		whereread += cnt;
	}

	return(0);
}


/**********************************************************************/
/*  fullmsg
*  Send a message to the Macintosh, probably an error message.
*  Mac messages go to stdout.
*  Includes the terminating zero in the string that it sends.
*/
fullmsg(s)
	char *s;
{

	return(fullwrite(1,s,strlen(s)+1));

}

/**********************************************************************/
/*  fullwrite
*   write a full segment to the network.
*   returns 0 for successful write, -1 for error
*/
fullwrite(skt,wherewrite,towrite)
    int skt,towrite;
    char *wherewrite;
{
    int cnt;

    while (towrite > 0) {            /* count of remaining bytes to read */

        cnt = write(skt,wherewrite,towrite);       /* write a chunk */
        if (cnt < 0)                /* connection broken */
            return(-1);

        towrite -= cnt;              /* adjust counters for what was write */
        wherewrite += cnt;
    }

    return(0);
}


/**********************************************************************/
/* readtonul
*  read from the stream until reaching a NUL
*/
readtonul(skt,p)
	int skt;
	char *p;
	{
	char cc;
	int ret;

	do {
		if (0 >= (ret = read(skt,&cc,1)))
			return(-1);
		*p++ = cc;
	} while (ret && cc);

	return(0);
}

/**********************************************************************/


#ifdef SUN
flconvert()
	{

}

flbackconvert()
	{

}
#endif


#ifdef UNICOS


/*
*  Convert floats from 4 bytes IEEE-32 to/from Cray-64 8 bytes.
*  Also responsible for unpacking and packing the 4 byte numbers.
*
*  These routines are not responsible for space allocation whatsoever.
*  They are assured that the space given is 8-bytes per float for as
*  many floats are given.  When extra space is generated in backconvert,
*  it lets the calling routine still take care of it.
*/


#define MINEXP    0x3f81000000000000  /* min valid Cray masked exponent */
#define MAXEXP    0x407e000000000000  /* max valid Cray masked exponent */

#define C_FMASK   0x00007fffff000000  /* Cray fraction mask (1st 23 bits)*/
#define C_EMASK   0x7fff000000000000  /* Cray exponent mask */
#define C_SMASK   0x8000000000000000  /* Cray sign mask */
#define C_IMPLICIT 0x0000800000000000 /* Cray implicit bit */

#define I_FMASK   0x007fffff          /* IEEE fraction mask */
#define I_EMASK   0x7f800000          /* IEEE exponent mask */
#define I_SMASK   0x80000000          /* IEEE sign mask     */

#define IEEE_BIAS 0177
#define CRAY_BIAS 040000

static long C2I_diff;
static long I2C_diff;



flbackconvert(farr,nf)
	char *farr;
	int nf;
	{
	int i;
	long tmp,newnum;
	char *to,*p;

	to = farr;			/* counts 4 byte IEEE numbers */

	for (i=0; i< nf; i++) {
		bcopy(farr, &newnum, 8);
		farr += 8;

		if (!newnum)
			tmp = 0;
		else {
			 
			tmp = (C_EMASK & newnum);
			if (tmp < MINEXP) {
				newnum = 1e-30;			/* should be -INF */
				tmp = (C_EMASK & newnum);
			}
			else if (tmp > MAXEXP) {
				newnum = 1e30;			/* should be +INF */
				tmp = (C_EMASK & newnum);
			}

			C2I_diff = (IEEE_BIAS - CRAY_BIAS - 1) << 48;
			tmp = (( tmp + C2I_diff ) << 7)
				| ( (newnum & C_FMASK) << 8 )
				| ( (newnum & C_SMASK));

		}

		bcopy(&tmp,to,4);
		to += 4;

	}

}


 
/* Conversion from IEEE floating point format to Cray format */
 
flconvert(farr,nf)
	char *farr;
	int nf;
	{
	int i;
	long tmp,targ;
	char *from,*to;

	from = farr + 4*(nf-1);		/* end of IEEE array, work backwards */
	to = farr + 8*(nf-1);		/* end of Cray array, work backwards */

	for (i=0; i<nf; i++) {		/* for each float */
		tmp = 0;
		bcopy(from, FLOFF+(char *)&tmp, 4); 
		from -= 4;

		if (!(targ = (tmp & I_EMASK))) {
			targ = 0;
		}
		else {
			I2C_diff = (CRAY_BIAS - IEEE_BIAS + 1) << 23;
			targ += I2C_diff;
			targ = (targ<< 25)  | ( (tmp & I_FMASK) << 24)
			   | ( (tmp & I_SMASK) << 32) | C_IMPLICIT;

		}
		bcopy(&targ, to, 8);
		to -= 8;				/* room for next one */

	}

}



#endif
