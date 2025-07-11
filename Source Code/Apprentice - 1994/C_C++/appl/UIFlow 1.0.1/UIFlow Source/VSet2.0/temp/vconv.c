/*****************************************************************************
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
*******************************************************************************
* Likkai Ng NCSA May 91
*
* vconv.c
* Part of the HDF Vset interface.

  conversion routines
  -------------------

  Routines here convert an array of numbers to and from IEEE format.
  Numbers do not have to be contiguous within array: specify the
  step as the distance (in bytes) between consecutive numbers.

Description of arguments:

	machfp - pointer to buffer for machine data
	ieeefp - pointer to data for IEEE data.
	mstep  - distance betwn consecutive data in machine buffer.
	istep  - distance betwn consecutive data in IEEE buffer.
	n	    - no of numbers to convert)

These routines always return 1.

************************************************************************/


#include "vg.h"

/* ============================================================== */
/* ================== S T U B for unimplemented conversions ===== */
/* ============================================================== */
nolongfn()							/*@@*/
{
  fprintf(stderr,"This long conversion fn not implemented\n");
}


/* ============================================================== */
/* ================== U N T Y P E D ============================= */
/* ============================================================== */
/* 
	the data is stored unconverted, ie stored in local machine 
	represention. such files are NOT portable, and are of use
	only to applications on the same machine.  Of course, sun-like
	machines may be able to share such files among themselves if
	they are lucky. No guarantees.  

   there are only 2 routines: DIRECTtoIEEE and DIRECTfromIEEE 
*/

int DIRECTtoIEEE (machfp, ieeefp, mstep, istep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int ret;

	ret = istep;
	if(vjv) {
		sprintf(sjs,"#in DIRECTtoIEEE: which invokes bytetoIEEE\n");
		zj;
	}
	ret = bytetoIEEE(machfp, ieeefp, mstep, mstep, n);

	return(ret);

} /* DIRECTtoIEEE */


int DIRECTfromIEEE (ieeefp, machfp, istep, mstep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int ret;

	if(vjv) {
		sprintf(sjs,"#in DIRECTfromIEEE: which invokes bytefromIEEE\n");
		zj;
	}
	ret = bytefromIEEE(ieeefp, machfp, istep, mstep, n);

	return(ret);

} /* DIRECTfromIEEE */


/* ============================================================== */
/* ==================== B Y T E S =============================== */
/* ============================================================== */

/* there are only 2 routines: bytetoIEEE and bytefromIEEE  */

int bytetoIEEE (machfp, ieeefp, mstep, istep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int j,t;
	register unsigned char *mfp, *ifp, *i, *m;

	if(vjv) {
		sprintf(sjs,"# in bytetoIEEE\n");
		zj;
	}

	mfp = machfp;
	ifp = ieeefp;
	for (j=0;j<n;j++) {
		for (i=ifp, m=mfp, t=0;t<4;t++) *i++ = *m++;
		mfp += mstep;
		ifp += istep;
	}

	return(1);

} /* bytetoIEEE */

int bytefromIEEE (ieeefp, machfp, istep, mstep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int t,j;
	register unsigned char *mfp, *ifp, *i, *m;

	if(vjv) {
		sprintf(sjs,"# in bytefromIEEE\n");
		zj;
	}

	mfp = machfp;
	ifp = ieeefp;
	for (j=0;j<n;j++) {
		for (i=ifp, m=mfp, t=0;t<4;t++) *m++ = *i++;
		mfp += mstep;
		ifp += istep;
	}

	return(1);

} /* bytefromIEEE */


/* ============================================================== */
/* ================= I N T E G E R S ============================ */
/* ============================================================== */


/* src are on this machine , dst are IEEE */

int inttoIEEE (src,dst,sdelta,ddelta,n)			/*@@*/

unsigned char *src, *dst;
int      n, sdelta, ddelta;
{
	int temp,i,j,sdel,ddel;
	register unsigned char *s, *ss, *d, *dd;
	register unsigned char *t;

	sdel = sdelta;
	ddel = ddelta;
	ss = src;
	dd = dst;

	if(vjv){
		sprintf(sjs,"#in inttoIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);
		zj;
	}

	for (i=0;i<n;i++) {
		t = (unsigned char*) &temp;
		s = ss;
		for(j=0;j<sizeof(temp);j++) *t++ = *s++;

		d = dd;
		INT16WRITE(d,temp);

		ss += sdel;
		dd += ddel;
	}

	return(1);

} /* inttoIEEE */

/* -------------------------------------------------------------- */
/* src are IEEE , dst are on this machine */

int intfromIEEE (src,dst,sdelta,ddelta,n)			/*@@*/

unsigned char *src,*dst;
int     n,sdelta,ddelta;
{
	register unsigned char *s, *ss, *d, *dd;
	register unsigned char *t;
	int                    temp,i,j,sdel,ddel;

	sdel = sdelta;
	ddel = ddelta;
	ss = src;
	dd = dst;
	if(vjv){
		sprintf(sjs,"#in intfromIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);
		zj;
	}

	for(i=0;i<n;i++) {
		s = ss;
		INT16READ(s,temp);

		t = (unsigned char*) &temp;
		d = dd;
		for (j=0;j<sizeof(temp);j++) *d++ = *t++;

		ss += sdel;
		dd += ddel;
	}

	return(1);

} /* intfromIEEE */


/* ============================================================== */
/* ================= L O N G S ================================== */
/* ============================================================== */

/* src are on this machine , dst are IEEE */

int longtoIEEE (src,dst,sdelta,ddelta,n)			/*@@*/

unsigned char *src, *dst;
int      n, sdelta, ddelta;
{
	int temp,i,j,sdel,ddel;
	register unsigned char *s, *ss, *d, *dd;
	register unsigned char *t;

	sdel = sdelta;
	ddel = ddelta;
	ss = src;
	dd = dst;

	if(vjv){
		sprintf(sjs,"#in longtoIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);
		zj;
	}
	printf("#in longtoIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);

	for (i=0;i<n;i++) {
		t = (unsigned char*) &temp;
		s = ss;
		for(j=0;j<sizeof(temp);j++) *t++ = *s++;

		d = dd;
		INT32WRITE(d,temp);

		ss += sdel;
		dd += ddel;
	}

	return(1);

} /* longtoIEEE */

/* -------------------------------------------------------------- */
/* src are IEEE , dst are on this machine */

int longfromIEEE (src,dst,sdelta,ddelta,n)			/*@@*/

unsigned char *src,*dst;
int     n,sdelta,ddelta;
{
	register unsigned char *s, *ss, *d, *dd;
	register unsigned char *t;
	int                    temp,i,j,sdel,ddel;

	sdel = sdelta;
	ddel = ddelta;
	ss = src;
	dd = dst;
	if(vjv){
		sprintf(sjs,"#in intfromIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);
		zj;
	}
	printf("#in longfromIEEE sdel=%d, ddel=%d, n=%d\n", sdel,ddel,n);

	for(i=0;i<n;i++) {
		s = ss;
		INT32READ(s,temp);

		t = (unsigned char*) &temp;
		d = dd;
		for (j=0;j<sizeof(temp);j++) *d++ = *t++;

		ss += sdel;
		dd += ddel;
	}

	return(1);

} /* longfromIEEE */

/* ============================================================== */
/* ==================== F L O A T S ============================= */
/* ============================================================== */

/* 
	floats are very machine specific. 
	There will always be  a set of 2 routines (XXXtoIEEE and XXXfromIEEE)
	for each machine that is special.
   So far, 2 sets are written specifically for the Cray and Vax.
	A third set is written for all other supported machines. These 
	machines use IEEE format (MAC, Sun, etc).
	One set, written for IEEE machines, is used by all other machines 
	(assumed to be IEEE) 
	
*/

#ifdef CRAY

/* ------------------ C R A Y ------------------------------------------ */

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


/*
convert from Cray2 floating point format to IEEE format
*/

int crayFtoIEEE (crayfp, ieeefp, cstep, istep, n)		/*@@*/

unsigned char *crayfp, *ieeefp;
int cstep, istep, n;
{
	long tmp, C2I_diff;
	long craylong;
	register int i,j,c;

	if(vjv) {
		sprintf(sjs,"#in crayFtoIEEE cstep=%d istep=%d n=%d\n",cstep,istep,n);
		zj;
	}

	c=0; 
	i=0;
	for (j=0;j<n;j++) {
		movebytes ((unsigned char*) &crayfp[c],(unsigned char*) &craylong,sizeof(long));
		/* ============================== */
		if (craylong == 0)  {
			tmp = 0;
		}
		else {
			tmp = (C_EMASK & craylong);
			if (tmp < MINEXP || tmp > MAXEXP)  return(-1);
			C2I_diff = (IEEE_BIAS - CRAY_BIAS - 1) << 48;
			tmp = (( tmp + C2I_diff ) << 7)
			    | ( (craylong  & C_FMASK) << 8 )
			    | ( (craylong  & C_SMASK));
		}
		/* ============================== */
		movebytes(unsigned &tmp, unsigned &ieeefp[i], 4);
		c += cstep; 
		i += istep;
	}
	return(0);

} /* crayFtoIEEE */

/* -------------------------------------------------------------- */

/*
Conversion from IEEE floating point format to Cray format 
*/

int crayFfromIEEE (ieeefp, crayfp, istep, cstep, n)		/*@@*/

unsigned char *crayfp, *ieeefp;
int cstep, istep, n;
{

	register int c,i,j;
	long tmp;
	long I2C_diff;
	long craylong;

	if(vjv) {
		sprintf(sjs,"#in crayFfromIEEE cstep=%d, istep=%d, n=%d\n", 
		    cstep,istep,n);
		zj;
	}

	c=0; 
	i=0;
	for (j=0;j<n;j++) {
		tmp = 0;
		movebytes (unsigned char*) &ieeefp[i],((unsigned char *) &tmp)+4, 4); /* right 4 bytes of tmp */

		/* ================================== */
		craylong  = (tmp & I_EMASK);
		if (craylong != 0) {
			I2C_diff   = (CRAY_BIAS - IEEE_BIAS + 1) << 23;
			craylong  += I2C_diff;
			craylong   = (craylong<< 25)
			    | ( (tmp & I_FMASK) << 24)
			    | ( (tmp & I_SMASK) << 32)
			    | C_IMPLICIT;
		}
		/* ================================== */

		movebytes ((unsigned char*) &craylong,(unsigned char*) &crayfp[c],sizeof(long));
		c += cstep;
		i += istep;
	}
	return (0);
} /* crayFfromIEEE */
#endif


#ifdef VMS
/* ------------------ V A X --------------------------------------------- */

/*
Conversion from VAx format IEEE floating point format.
*/

int vaxFtoIEEE (vaxfp, ieeefp, vstep, istep, n)		/*@@*/

unsigned char *vaxfp, *ieeefp;
int vstep, istep, n;
{
	{ sprintf(sjs,"@%c vaxFtoIEEE unimplemeted yet\n",7);
	zj; 
}

} /* vaxFtoIEEE */

/*
Conversion from IEEE floating point format to Vax format.
*/

int
vaxFfromIEEE (ieeefp, vaxfp, istep, vstep, n)		/*@@*/

unsigned char *vaxfp, *ieeefp;
int vstep, istep, n;
{
	{ sprintf(sjs,"@%c vaxFfromIEEE unimplemeted yet\n",7);
	zj; 
}

} /* vaxFfromIEEE */
#endif


/* -------------------------------------------------------------- */
/* ----------- A L L   O T H E R    M A C H I N E S ------------- */
/* -------------------------------------------------------------- */

int floattoIEEE (machfp, ieeefp, mstep, istep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int ret;
	if(vjv)  {
		sprintf(sjs,"#in floattoIEEE: which invokes bytetoIEEE\n");
		zj;
	}
	ret = bytetoIEEE(machfp, ieeefp, mstep, istep, n);

	return(ret);

} /* floattoIEEE */


int floatfromIEEE (ieeefp, machfp, istep, mstep, n)		/*@@*/

unsigned char *machfp, *ieeefp;
int mstep, istep, n;
{
	int ret;
	if(vjv)  {
		sprintf(sjs,"#in floatfromIEEE: which invokes bytefromIEEE\n");
		zj;
	}
	ret = bytefromIEEE(ieeefp, machfp, istep, mstep, n);
	return(ret);
} /* floatfromIEEE */

/* -------------------------------------------------------------- */

/* move n bytes of from ss to dd */
int movebytes (ss,dd,n)		/*@@*/

unsigned char *ss, *dd;
int n;
{
	register unsigned char *d,*s;
	register int i;
	for (d=dd,s=ss,i=0;i<n;i++) *d++ = *s++;

	return(1);

} /* movebytes */
/* -------------------------------------------------------------- */



/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/*                                                                    */ 
/* routines for converting from vsets in v1.0 to v2.x                 */
/*                                                                    */ 
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */
/* $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ */

#define INT16SIZE 2
#define INT32SIZE 4

/* ------------------------------------------------------------------ */

/*
*  this routine checks that the given OPENED file is compatible with
*	version 2.0 or later of the HDF Vset library .
*  returns 1 if file already compatible with r2.
*          0 if not compatible.
*          -1 if error.
*/

int vicheckcompat(f) DF * f; {

	DFdesc	desc;
	int 		foundold, foundnew;

	foundold = 0;
	foundnew = 0;

	/* locate any OLD vgs */
		DFsetfind(f, OLD_VGDESCTAG, DFTAG_WILDCARD);
		if ( DFfind(f, &desc)  == 0) foundold++; 
				
	/* locate any OLD vdatas */
		DFsetfind(f, OLD_VSDESCTAG, DFTAG_WILDCARD);
		if ( DFfind(f, &desc)  == 0) foundold++; 
				
	/* locate any NEW vgs */
		DFsetfind(f, NEW_VGDESCTAG, DFTAG_WILDCARD);
		if ( DFfind(f, &desc)  == 0) foundnew++; 
				
	/* locate any NEW vdatas */
		DFsetfind(f, NEW_VSDESCTAG, DFTAG_WILDCARD);
		if ( DFfind(f, &desc)  == 0) foundnew++; 

	if ( foundold == 0 ) /* has no old vset elements */
		return (1); /* just assume compatible */

	if ( foundnew > 0 ) 
		return (1); /* file is already compatible */
	else
		return (0); /* file is not compatible */
} /* vicheckcompat */

/* ------------------------------------------------------------------ */
/*
* This routine will modify a given OPENED file so that is becomes compatible
* with version 2.0 or later of the HDF Vset library.
* Note that the file is assumed to be not compatible to begin with.
* This routine will not check to see if the file is already compatible,
* but it is harmless to run an already-compatible file through again.
*
* However, be aware that each time, the file gets larger.
* Also, file must be opened with DFACC_ALL access.
*
* returns  1 if successful. if error, returns 0
*/


int vimakecompat(f)  DF *f; {

	VGROUP 	tempvgroup;
	VDATA		tempvdata;

	VGROUP	* vg = &tempvgroup;
	VDATA		*vs = &tempvdata;
	unsigned char buf[5000];
	int 		i, dum, bsize;
	DFdesc	desc;

	/* =============================================  */
	/* --- read all vgs and convert each --- */

	dum = 0;
	DFsetfind (f, OLD_VGDESCTAG, DFTAG_WILDCARD);
	while ( DFfind(f, &desc) == 0) {
		if ( -1 == (bsize = DFgetelement (f, OLD_VGDESCTAG, desc.ref, buf)) ) {
 			fprintf(stderr,"getvgroup_old. getelement err. DFerror=%d\n",DFerror);
			return -1;
			}

		oldunpackvg (vg, buf, &bsize);
		/* add new items */
			vg->vgclass[0] = '\0';
			vg->extag = 0;
			vg->exref = 0;
			vg->version = 2; /* version 2 */
			vg->more = 0;
		/* inside each vgroup, change the old tags to new */
			for(i=0;i<vg->nvelt;i++)
				if (vg->tag[i] == OLD_VGDESCTAG) vg->tag[i] = NEW_VGDESCTAG;
				else if (vg->tag[i] == OLD_VSDESCTAG) vg->tag[i] = NEW_VSDESCTAG;
				else { fprintf( stderr,"vimakecompat: unknown tag %d] in vgroup!\n",
								vg->tag[i]);
						}
		vpackvg (vg, buf, &bsize);

		if ( -1 == DFputelement (f, VGDESCTAG, desc.ref, buf, bsize)) {
      	fprintf(stderr,"put vgroup desc error. DFerror=%d\n",DFerror);
			return -1;
			}	

		dum++;
		}

	/* =============================================  */
	/* --- read all vdata descs  and convert each --- */
	/* --- then dup a tag for each vdata data elt --- */

	dum = 0;
	DFsetfind (f, OLD_VSDESCTAG, DFTAG_WILDCARD);
	while ( DFfind(f, &desc) == 0) {
		if ( -1 == (bsize = DFgetelement (f, OLD_VSDESCTAG, desc.ref, buf)) ) {
 			fprintf(stderr,"getvdata_old. getelement err. DFerror=%d\n",DFerror);
			return -1;
			}

		oldunpackvs (vs, buf, &bsize);
		/* add new items */
			vs->vsclass[0] = '\0';
			vs->extag = 0;
			vs->exref = 0;
			vs->version = 2; /* version 2 */
			vs->more = 0;
		vpackvs (vs, buf, &bsize);

		if ( -1 == DFputelement (f, VSDESCTAG, desc.ref, buf, bsize)) {
      	fprintf(stderr,"put vdata desc error. DFerror=%d\n",DFerror);
			return -1;
			}	

		/* duplicate a tag to point to vdata vdata */

			if ( DFdup(f, NEW_VSDATATAG, desc.ref, OLD_VSDATATAG, desc.ref) < 0) {
				fprintf(stderr,"DFdup - cannot duplicate. DFerror=%d\n",DFerror);
				return -1;
				}

		dum++;
		}

	return(1);

} /* vimakecompat */


/* ================================================================== */
/*
*  this routine checks that the given file is compatible with
*	version 2.0 or later of the HDF Vset library .
*
*  All it does is to open the file, call vicheckcompat to do all the
*  checking, and then closes it.
*	See comments for vicheckcompat().

*  returns 1 if file already compatible with r2.
*          0 if not compatible.
*          -1 if error.
*/

int vcheckcompat(fs) char * fs; {

	DF			* f;
	int 		stat;

   if (NULL==(f=DFopen(fs,DFACC_READ,0))) {
		fprintf(stderr,"vcheckcompat: cannot open %s\n",fs);
		return(-1);
		}	 	
	stat = vicheckcompat(f); 
	DFclose (f);	

	return (stat);
} /* vcheckcompat */

/* ================================================================== */
/*
* This routine will modify a given file so that is becomes compatible
* with version 2.0 or later of the HDF Vset library.
*
* All this routine does is to open the file, call vimakecompat to
* do all the conversion, and then to close the file.
* See comments for vimakecompat().
*
* returns  1 if successful. if error, returns 0
*/

int vmakecompat(fs) char * fs; {
	DF * f;
	int stat;

   if (NULL==(f=DFopen(fs,DFACC_ALL,0))) {
		fprintf(stderr,"vmakecompat: cannot open %s\n",fs);
		return(0);
		}	 	
	stat = vimakecompat(f);
	DFclose(f);

} /* vmakecompat */

/* ==================================================================== */

oldunpackvg (vg,buf,size)          /*@@*/

VGROUP*    	  	vg;		/* vgroup to be loaded with file data */
unsigned char 	buf[]; 	/* must contain a VGDESCTAG data object from file */

int*          	size;  	/* ignored, but included to look like packvg() */

{

	unsigned char	*b, *bb;
	int 				i;

	i = *size; /* dummy, so that compiler thinks it is used  */

	bb = &buf[0];

	/* retrieve nvelt */
	b = bb;
	INT16READ(b,vg->nvelt);
	bb+=INT16SIZE;

	/* retrieve the tags */
	for (i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16READ(b,vg->tag[i]);
		bb +=INT16SIZE;
	}

	/* retrieve the refs */
	for (i=0;i<vg->nvelt;i++) {
		b= bb;
		INT16READ(b,vg->ref[i]);
		bb +=INT16SIZE;
	}

	/* retrieve vgname */
	strcpy(vg->vgname,bb);
	bb += ( strlen(vg->vgname)+1 );

	if (vjv) {
		sprintf(sjs,"unpackvg: vgname is [%s]\n",vg->vgname);
		zj;
	}

} /* unpackvg */

/* ================================================================= */


oldunpackvs (vs,buf,size)       /*@@*/

VDATA *vs;
int   *size;			/* UNUSED, but retained for compatibility with packvs */
unsigned char buf[];
{
	unsigned char *b, *bb;
	int            i;

	i = *size; /* dum */

	bb = &buf[0];

	b = bb;
	INT16READ(b,vs->interlace);
	bb += INT16SIZE;

	b = bb;
	INT32READ(b,vs->nvertices);
	bb += INT32SIZE;

	b = bb;
	INT16READ(b,vs->wlist.ivsize);
	bb += INT16SIZE;

	b = bb;
	INT16READ(b,vs->wlist.n);
	bb += INT16SIZE;

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the type */
		b = bb;
		INT16READ(b,vs->wlist.type[i]);
		bb += INT16SIZE;
	}

	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the isize */
		b = bb;
		INT16READ(b,vs->wlist.isize[i]);
		bb += INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the off */
		b = bb;
		INT16READ(b,vs->wlist.off[i]);
		bb += INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++)  { /* retrieve the order */
		b = bb;
		INT16READ(b,vs->wlist.order[i]);
		bb += INT16SIZE;
	}
	for (i=0;i<vs->wlist.n;i++) {
		strcpy(vs->wlist.name[i], bb);
		bb += ( strlen(vs->wlist.name[i]) + 1 );
	}

	strcpy(vs->vsname,bb);
	bb += ( strlen(vs->vsname) + 1);

	/* **EXTRA**  fill in the machine-dependent size fields */
	for (i=0;i<vs->wlist.n;i++) {
		vs->wlist.esize[i] = vs->wlist.order[i] * SIZEOF(vs->wlist.type[i]);
	}

} /* unpackvs */

/* ------------------------------------------------------------------ */
