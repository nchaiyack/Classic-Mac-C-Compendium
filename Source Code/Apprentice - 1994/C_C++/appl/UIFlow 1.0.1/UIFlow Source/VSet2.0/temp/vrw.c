/****************************************************************************e
* 
*			  NCSA HDF Vset release 2.1
*					May 1991
*
* NCSA HDF Vset release 2.1 source code and documentation are in the public
* domain.  Specifically, we give to the public domain all rights for future
* licensing of the source code, all resale rights, and all publishing rights.
* 
* We ask, but do not require, that the following message be included in all
* derived works:
* 
* Portions developed at the National Center for Supercomputing Applications at
* the University of Illinois at Urbana-Champaign.
* 
* THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
* SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
* WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
* 
***************************************************************************** 
* Likkai Ng NCSA May 1991
*
* vrw.c
* Part of the HDF VSet interface.
*
************************************************************************/

#include <stdio.h>
#include "vg.h"
/*
USEFUL MACRO. Print an error message, then return  -1.
*/

/* ------------------------------------------------------------------------ */

/*
the series of #defines below establishes the conversion routines
to be used for a particular machine. For all machines, the 
following 6 conversion routines are defined:

	FLOATtoIEEE 	- machine float to IEEE float 
	FLOATfromIEEE 	- machine float from IEEE float	
	INTtoIEEE     	- machine integer to IEEE integer 
	INTfromIEEE   	- machine integer from IEEE integer 
	CHARtoIEEE    	- machine character to IEEE character 
	CHARfromIEEE   - machine character from IEEE character 
	LONGtoIEEE    	- machine long to IEEE long 
	LONGfromIEEE  	- machine long from IEEE long 
*/

#ifdef CTSS 
#define FLOATtoIEEE   	crayFtoIEEE
#define FLOATfromIEEE 	crayFfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif CTSS 

#ifdef UNICOS
#define FLOATtoIEEE   	crayFtoIEEE
#define FLOATfromIEEE 	crayFfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif UNICOS

#ifdef VMS
#define FLOATtoIEEE   	vaxFtoIEEE
#define FLOATfromIEEE 	vaxFfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif VMS

#ifdef PC
#define FLOATtoIEEE   	floattoIEEE
#define FLOATfromIEEE 	floatfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif PC

#ifdef SUN
#define FLOATtoIEEE   	floattoIEEE
#define FLOATfromIEEE 	floatfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif SUN

#ifdef ALLIANT
#define FLOATtoIEEE   	floattoIEEE
#define FLOATfromIEEE 	floatfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif ALLIANT

#ifdef IRIS4
#define FLOATtoIEEE   	floattoIEEE
#define FLOATfromIEEE 	floatfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif IRIS4

#ifdef MAC
#define FLOATtoIEEE   	floattoIEEE
#define FLOATfromIEEE 	floatfromIEEE
#define INTtoIEEE     	inttoIEEE
#define INTfromIEEE   	intfromIEEE
#define CHARtoIEEE     	bytetoIEEE
#define CHARfromIEEE   	bytefromIEEE
#define LONGtoIEEE     	longtoIEEE
#define LONGfromIEEE   	longfromIEEE
#endif MAC

/* ------------------------------------------------------------------------ */
/*
  The above 6 defined function entry-points are then stuffed
  into 2 PRIVATE arrays of conversion pointers ie, (*convtoIEEE[])() and
  (*convfromIEEE[])().

  Any conversion function in these arrays may be invoked by
  indexing into the private arrays with one of the following:
  LOCAL_NOTYPE,
  LOCAL_CHARTYPE, 
  LOCAL_INTTYPE,
  LOCAL_FLOATTYPE 
  LOCAL_LONGTYPE,

 */

extern int

DIRECTtoIEEE(), DIRECTfromIEEE(),
CHARtoIEEE(),   CHARfromIEEE(), 
INTtoIEEE(),    INTfromIEEE(), 
LONGtoIEEE(),   LONGfromIEEE(), 
FLOATtoIEEE(),  FLOATfromIEEE(),
nolongfn;

PRIVATE int (*convtoIEEE[])()  = { 
	DIRECTtoIEEE,  CHARtoIEEE,  INTtoIEEE,  FLOATtoIEEE, LONGtoIEEE };

PRIVATE int (*convfromIEEE[])()= { 
	DIRECTfromIEEE,  CHARfromIEEE,  INTfromIEEE,  FLOATfromIEEE, LONGfromIEEE };

/* ------------------------------------------------------------------------ */
/* 
	VSseek

	Seeks to an element boundary within a vdata
	Vdata must be attached with "r" access.
	Specify eltpos = 0 for 1st element, 1 for 2nd element etc.
	RETURNS -1 on error
	RETURNS position of element seeked to (0 or a +ve integer)
        (eg  returns 5 if seek to the 6th element, etc)
*/

PUBLIC int VSseek (vs, eltpos)         /*@-@*/

VDATA *vs;
int32	   eltpos;
{
	int32 offset, n;

	if (vs==NULL)          RTNEG("VSseek: err vs null");
	if (vs->access !='r')  RTNEG("VSseek: err only read-only allowed");
	if (eltpos < 0)        RTNEG("VSseek: err neg value");

	if (eltpos >= vs->nvertices ) RTNEG("VSseek: err over seek");

	vs->vpos = eltpos;

	/* do the actual access and seek.  is this redundant?  */
	if(DFaccess(vs->f,VSDATATAG,vs->oref,"r") == -1){
		sprintf(sjs,"@VSseek: access <%d/%d> gives D err=%d\n",
		    VSDATATAG, vs->oref,DFerror);
		zj;
	}

	offset  = vs->vpos * vs->wlist.ivsize;
	n = DFseek(vs->f,offset);
	if (n != offset ) {
		sprintf(sjs,"@VSseek: seek to %d rets %d\n", n, offset);
		zj;
		return(n/vs->wlist.ivsize);
	}
	else
		return(eltpos); /* ok */

} /* VSseek */

/* ------------------------------------------------------------------------ */

/*
	VSread
	Reads a specified number of elements' worth of data from a vdata.
	Data will be returned to you interlaced in the way you specified.
	RETURNS -1 if error
	RETURNS the number of elements read (0 or a +ve integer).
*/

PUBLIC int VSread (vs, buf, nelt, interlace)    /*@-@*/

VDATA    *vs;
int32	   nelt;
int		interlace;
unsigned char	*buf;
{
	register int 				isize,esize,hsize;
	register unsigned char  *b1,*b2;
	int 				i,j, n, nv, offset, type;
	VWRITELIST 		*w;
	VREADLIST  		*r;
	unsigned char  *tbuf;
	int 				uvsize; 			/* size of "element" as NEEDED by user */

	if(vs == NULL)					RTNEG("r: vs null");
	if(vs->access != 'r')	   RTNEG("r: access is r");
	if(vs->nvertices == 0)		RTNEG("r: vs has 0 vertices");
	if (-1 == vexistvs(vs->f,vs->oref) )	RTNEG("w: vs not found");

	if (interlace != FULL_INTERLACE  && 
	    interlace != NO_INTERLACE )
		RTNEG("bad interlace specified");

	w = &(vs->wlist);
	r = &(vs->rlist);
	hsize = vs->wlist.ivsize; 		/* size as stored in HDF */

	/* alloc space (tbuf) for reading in the raw data from vdata */
	tbuf = (unsigned char*) DFIgetspace ( nelt * hsize );
	if (tbuf==NULL) RTNEG("r: space");


	/* ================ start reading ============================== */
	/* ================ start reading ============================== */
	/* ================ start reading ============================== */

	/*
					  Procedure: 
					  first access the vdata,
		           then seek to the place last read from.
		           read the next elements from that place.
		           update element pointer in vdata.
   */

	if ( DFaccess (vs->f,VSDATATAG,vs->oref,"r") == -1){
		sprintf(sjs,"@VR: access <%d/%d> gives D err=%d\n",
		    VSDATATAG, vs->oref,DFerror);
		zj;
	}

	j = vs->vpos * vs->wlist.ivsize;

	n = DFseek(vs->f,j);
	if (vjv)   { 
		sprintf(sjs,"VSREAD: seek to v#%d ie pos %d\n",vs->vpos,j);
		zj; 
	}
	if (n!=j) { 
		sprintf(sjs,"@VR: seek to %d rets %d\n", n, j);
		zj; 
	}

	nv = ( (nelt > vs->nvertices) ? vs->nvertices : nelt );

	n = DFread(vs->f,tbuf,nv * hsize);
	if (vjv) { 
		sprintf(sjs,"#vsread: DFREAD read %d bytes (%dx%d) from vs\n", 
		    n,nv,hsize);
		zj;
	}

	if ( nv != nelt ) {
		sprintf(sjs,"@vsread:NOTE:nv=%d nelt=%d!!\n",nv,nelt);
		zj;
		vs->vpos += (n/vs->wlist.ivsize);
	}
	else 
		vs->vpos +=nelt;

	/* ================ done reading =============================== */
	/* ================ done reading =============================== */
	/* ================ done reading =============================== */



	/* 
		Now, convert and repack field(s) from tbuf into buf.    

		This section of the code deals with interlacing. In all cases
		the items for each of the fields are converted and shuffled 
		around from the internal buffer "tbuf" to the user's buffer 
		"buf".  When all is done, the space in "tbuf" is freed.

		There are 5 cases :
		(A) user=NO_INTERLACE   & vdata=FULL_INTERLACE) 
  		(B) user=NO_INTERLACE   & vdata=NO_INTERLACE) 
  		(C) user=FULL_INTERLACE & vdata=FULL_INTERLACE) 
		(D) user=FULL_INTERLACE & vadat=NO_INTERLACE) 
		(E) SPECIAL CASE when field has order>1. 

		Cases (A)-(D) handles multiple fields of order 1.
		Case (E) handles one field of order>1. Interlace is
		irrelevant in this case. The case where there are 
		multiple fields of order>1 is prevented from
		existing by VSsetfields explicitly checking for this.
				
	*/

	/* --------------------------------------------------------------------- */
	/* CASE  (E): Special Case For one field with order >1 only */

	if (w->n == 1 && w->order[0] > 1) {
		b1 = buf;
		b2 = tbuf;
		esize = w->esize[0];
		isize = w->isize[0];
		type = w->type[0];

		if (vjv) {
			sprintf(sjs,"@VSREAD: SPECIAL order=%d esiz=%d isiz=%d nelt=%d vsize %d\n", w->order[0], esize, isize,
			    nelt , hsize); 
			zj;
			sprintf(sjs,"@VSREAD: type is %d\n", type);
			zj;
		}

		(*convfromIEEE[type]) (b2, b1,
		    isize/w->order[0], 
		    esize/w->order[0],
		    w->order[0] * nelt);

	} /* case (e) */

	/* --------------------------------------------------------------------- */
	/* CASE  (A):  user=none, vdata=full */

	else if (interlace==NO_INTERLACE && vs->interlace==FULL_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#R:  CASE  (A):  iu=none, iv=full \n"); 
			zj;
		}

		b1 = buf;
		for (j=0;j<r->n;j++) {
			i     = r->item[j];
			b2    = tbuf + w->off[i];
			type  = w->type[i];
			esize = w->esize[i];

			(*convfromIEEE[type]) (b2, b1,
			    hsize,
			    esize,
			    nelt);

			b1 += (nelt * esize);
		}

	} /* case (a) */

	/* --------------------------------------------------------------------- */
	/* CASE  (B):  user=none, vdata=none */
	else if (interlace==NO_INTERLACE && vs->interlace==NO_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#R:  CASE  (B):  iu=none, iv=none\n"); 
			zj;
		}
		b1 = buf;
		for (j=0;j<r->n;j++) {
			i     = r->item[j];
			b2    = tbuf + w->off[i] * nelt;
			type  = w->type[i];
			esize = w->esize[i];
			isize = w->isize[i];

			(*convfromIEEE[type]) (b2, b1,
			    isize,
			    esize,
			    nelt);

			b1 += (nelt * esize);
		}
	} /* case (b) */

	/* --------------------------------------------------------------------- */
	/* CASE  (C):  iu=full, iv=full */
	else if (interlace==FULL_INTERLACE && vs->interlace==FULL_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#R:  CASE  (C):  iu=full, iv=full\n"); 
			zj;
		}

		for (uvsize=0, j=0;j<r->n;j++)
			uvsize += w->esize[r->item[j]];

		for (offset=0,j=0;j<r->n;j++) {
			i     = r->item[j];
			b1    = buf + offset;
			b2    = tbuf + w->off[i];
			type  = w->type[r->item[j]];
			esize = w->esize[i];
			isize = w->isize[i];

			(*convfromIEEE[type]) (b2, b1,
			    hsize,
			    uvsize,
			    nelt);
			offset += esize;
		}

	} /* case (c) */


	/* --------------------------------------------------------------------- */
	/* CASE  (D):  user=full, vdata=none */
	else if(interlace==FULL_INTERLACE && vs->interlace==NO_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#R:  CASE  (D):  iu=full, iv=none\n"); 
			zj;
		}

		for (uvsize=0, j=0;j<r->n;j++)
			uvsize += w->esize[r->item[j]];

		for (offset=0,j=0;j<r->n;j++) {
			i     = r->item[j];
			b1    = buf + offset;
			b2    = tbuf + w->off[i] * nelt;
			type  = w->type[i];
			isize = w->isize[i];

			(*convfromIEEE[type]) (b2, b1,
			    isize,
			    uvsize,
			    nelt);
			offset +=isize;
		}
	} /* case (d) */

	DFIfreespace(tbuf);
	return(n/hsize);

} /* VSread */

/* ------------------------------------------------------------------ */
/*
	VSwrite
	Writes a specified number of elements' worth of data to a vdata.
	You must specify how your data in your buffer is interlaced.
	
	RETURNS -1 if error
	RETURNS the number of elements written (0 or a +ve integer).
*/



PUBLIC int VSwrite (vs, buf, nelt, interlace)   /*@-@*/

VDATA 	*vs;
int32		nelt;
int		interlace;
unsigned char *buf;
{
	register int 				isize,esize,hsize;
	register unsigned char 	*b1,*b2;
	unsigned char 				*tbuf;
	int 			j,type, n,nv,offset;
	VWRITELIST	*w;
	int 			uvsize;			/* size of "element" as needed by user */

	if (nelt <= 0) 			RTNEG("w: 0 vertices");
	if (vs == NULL) 			RTNEG("w: vs null");
	if (vs->access != 'w') 	RTNEG("w: access is w");

	if ( -1 == vexistvs(vs->f,vs->oref) )	RTNEG("w: vs not found");
	if (vs->wlist.ivsize == 0)	RTNEG("w: vsize 0. fields not set for write!");

	if (vs->nvertices != 0) { 
		sprintf(sjs,"@VSwrite: append\n"); 
		zj; 
	}

	if (interlace != NO_INTERLACE && 
	    interlace != FULL_INTERLACE )
		RTNEG("w: bad interlace");

	w = (VWRITELIST*) &vs->wlist;
	hsize = w->ivsize; 		/* as stored in HDF file */

	tbuf = (unsigned char*) DFIgetspace( nelt * hsize);
	if (tbuf == NULL)  RTNEG("W: space ");


	/* 
		First, convert and repack field(s) from tbuf into buf.    

		This section of the code deals with interlacing. In all cases
		the items for each of the fields are converted and shuffled 
		around from the user's buffer "buf" to the internal's buffer 
		"tbuf".  The data in "tbuf" is then written out to the vdata.
		When all is done, the space in "tbuf" is freed.

		There are 5 cases :
		(A) user=NO_INTERLACE   & vdata=FULL_INTERLACE) 
      (B) user=NO_INTERLACE   & vdata=NO_INTERLACE) 
      (C) user=FULL_INTERLACE & vdata=FULL_INTERLACE) 
      (D) user=FULL_INTERLACE & vadat=NO_INTERLACE) 
		(E) SPECIAL CASE when field has order>1. 

		Cases (A)-(D) handles multiple fields of order 1.
		Case (E) handles one field of order>1. Interlace is
		irrelevant in this case. The case where there are 
		multiple fields of order>1 is prevented from
		existing by VSsetfields explicitly checking for this.

/* --------------------------------------------------------------------- */
	/* CASE  (E): Special Case For one field with order >1 only */

	if (w->n == 1 && w->order[0] > 1) {
		b1    = buf;
		b2    = tbuf;
		esize = w->esize[0];
		isize = w->isize[0];
		type  = w->type[0];

		if (vjv) {
			sprintf(sjs,"@VSWRITE: SPECIAL order=%d esiz=%d isiz=%d nelt=%d vsiz=%d\n", w->order[0], esize, isize,
			    nelt, hsize); 
			zj;
			sprintf(sjs,"@VSWRITE: type = %d\n", type);
			zj;
		}

		(*convtoIEEE[type]) (b1, b2,
		    esize/w->order[0], 
		    isize/w->order[0],
		    w->order[0] * nelt);
	} /* case (e) */


	/* --------------------------------------------------------------------- */
	/* CASE  (A):  user=none, vdata=full */
	else if (interlace==NO_INTERLACE && vs->interlace==FULL_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#w:  CASE  (A):  iu=none, iv=full \n"); 
			zj;
		}

		b1 = buf;
		for (j=0;j<w->n;j++) {
			b2    = tbuf + w->off[j];
			type  = w->type[j];
			esize = w->esize[j];

			(*convtoIEEE[type]) (b1, b2,
			    esize,
			    hsize, nelt);

			b1 += (nelt * esize);
		}

	} /* case (a) */

	/* --------------------------------------------------------------------- */
	/* CASE  (B):  user=none, vdata=none */
	else if (interlace==NO_INTERLACE && vs->interlace==NO_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#w:  CASE  (B):  iu=none, iv=none\n"); 
			zj;
		}

		b1 = buf;
		for (j=0;j<w->n;j++) {
			b2    = tbuf + w->off[j] * nelt;
			type  = w->type[j];
			esize = w->esize[j];
			isize = w->isize[j];

			(*convtoIEEE[type]) (b1, b2,
			    esize,
			    isize,
			    nelt);

			b1 += (nelt * esize);
		}

	} /* case (b) */

	/* --------------------------------------------------------------------- */
	/* CASE  (C):  user=full, vdata=full */
	else if (interlace==FULL_INTERLACE && vs->interlace==FULL_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#w:  CASE  (C):  iu=full, iv=full\n"); 
			zj;
		}

		for (uvsize=0, j=0;j<w->n;j++)
			uvsize += w->esize[j];

		for (offset=0,j=0;j<w->n;j++) {
			b1    = buf + offset;
			b2    = tbuf + w->off[j];
			type  = w->type[j];
			esize = w->esize[j];
			isize = w->isize[j];

			(*convtoIEEE[type]) (b1, b2,
			    uvsize,
			    hsize,
			    nelt);
			offset += esize;
		}


	} /* case (c) */

	/* --------------------------------------------------------------------- */
	/* CASE  (D):  user=full, vdata=none */
	else if (interlace==FULL_INTERLACE && vs->interlace==NO_INTERLACE) {
		if (vjv) {
			sprintf(sjs,"#w:  CASE  (D):  iu=full, iv=none\n"); 
			zj;
		}

		for (uvsize=0, j=0;j<w->n;j++)
			uvsize += w->esize[j];

		for (offset=0,j=0;j<w->n;j++) {
			b1    = buf  + offset;
			b2    = tbuf + w->off[j] * nelt;
			type  = w->type[j];
			isize = w->isize[j];
			esize = w->esize[j];
			(*convtoIEEE[type]) (b1, b2,
			    uvsize,
			    isize,
			    nelt);
			offset +=esize;
		}
	} /* case (d) */



	/* ================ start writing ============================== */
	/* ================ start writing ============================== */
	/* ================ start writing ============================== */


	n = DFaccess (vs->f,VSDATATAG,vs->oref,"a");
	if (n == -1){
		sprintf(sjs,"@VSwrite: access err, stat=%d DFerr=%d\n",n,DFerror);
		zj;
		DFIfreespace(tbuf);
		return(FAIL);
	}

	n  = DFwrite (vs->f,tbuf,nelt * hsize);
	nv = n/hsize;

	if (vjv) {
		sprintf(sjs,"#vswrite: written %d bytes (%d x %d) to vs [%s]\n",
		    n,nv,hsize,vs->vsname);
		zj;
	}

	/* ================ done writing =============================== */
	/* ================ done writing =============================== */
	/* ================ done writing =============================== */

	if ( nv != nelt) {
		sprintf(sjs,"@vswrite: NOTE: nv=%d nvertices=%d!!\n",nv,nelt);
		zj;
	}

	DFIfreespace(tbuf);
	vs->nvertices += nv;

	vs->marked = 1;

	return(nv);

} /* VSwrite */

/* ------------------------------------------------------------------ */
