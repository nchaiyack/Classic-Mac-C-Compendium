
#include "DScope.h"


/*
*  hifilter
*  Given the array (first parameter), and some bounds, either a constant or an
*  array, return in the answer the higher of the two numbers.  This produces a
*  hi-pass filter -- only the larger values get through.
*
*/
long exhifilter(l,r,a)
	scope_array *l,*r,*a;
{
	register float *f,*sf,*sf2,hval;
	register int i,j;
	int constyes=0;

	if (!l || !r) {
		a->kind = DS_ERROR;
		return(0);
	}
	
	f = a->vals;
	
	if (l->kind == DS_CONSTANT) {		/* is the left a constant? */
		hval = l->cval;
		constyes = 1;
		sf = r->vals;
	}

	if (r->kind == DS_CONSTANT) {		/* pass under a constant */
		hval = r->cval;
		if (constyes) {
			a->kind = DS_CONSTANT;
			if (l->cval < r->cval)		/* simple max function */
				a->cval = r->cval;
			else
				a->cval = l->cval;
			return;
		}
		constyes = 1;
		sf = l->vals;
	}
	
	if (constyes) {
		
		for (i=0; i< a->nrows; i++) 
			for (j=0; j< a->ncols; j++) {
				if (*sf < hval) {
					*f++ = hval;
					sf++;
				}
				else
					*f++ = *sf++;
			}
	}

	else {								/* compare two arrays */

		sf = l->vals;
		sf2	= r->vals;
		
		for (i=0; i< a->nrows; i++) {
			for (j=0; j< a->ncols; j++)
				if (*sf < *sf2) {
					*f++ = *sf2++;
					sf++;
				}
				else {
					*f++ = *sf++;
					sf2++;
				}
		}
	}		
}

/******************************************************/
/*

*/
long smooth(l,r,a)
    scope_array *l,*r,*a;
{
    register float *f,*sf,*sf2,hval;
    register int i,j;
    int valfill,times=0;

    if (!l || !r) {
        a->kind = DS_ERROR;
        return(0);
    }

    if (l->kind != DS_ARRAY
		|| r->kind != DS_CONSTANT) {   
        a->kind = DS_ERROR;
        return(0);
    }

	hval = r->cval;
	f = a->vals;
	sf = l->vals;

	for (i=0; i< a->nrows; i++) 
		for (j=0; j< a->ncols; j++)
			*f++ = *sf++;			/* copy left to answer */


	do {
		times++;
		valfill = 0;			/* default off, see if one got filled */

		f = sf = a->vals;
		
		for (i=1; i< a->nrows-1; i++,f+=2) 
			for (j=1; j< a->ncols-1; j++,f++) {
				if (*f < hval + 1e-8 && *f > hval - 1e-8) {
					valfill = 1;
					*f = (*(f-1) + *(f+1) + *(f-a->ncols) + *(f+a->ncols))/4.0;
				}
			}


	} while (valfill && times < 50);
	

}


#include "math.h"

scos(l,r,a)
	scope_array *l,*r,*a;
	{
	register int i,j,lim;
	register float f,*sf;

    if (!l || !r) {
        a->kind = DS_ERROR;
        return(0);
    }

    if (l->kind != DS_ARRAY) {
        a->kind = DS_ERROR;
        return(0);
    }


	sf = a->vals;
	for (i=0; i<a->nrows; i++)
		for (j=0; j<a->ncols; j++) {
			f = sqrt((double)i*i + j*j)/a->ncols;

			*sf++ = sin(20.0*f);
		}
}


nill(l,r,a)
	scope_array *l,*r,*a;
	{
	char *p,*q;
		int i;

	p = (char *)a->vals;
	q = (char *)l->vals;

	for (i=0; i<4*a->nrows*a->ncols; i++)
		*p++ = *q++;


}
