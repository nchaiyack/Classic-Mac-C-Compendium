#include "DScope.h"
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 colarray	Fill the result array with copies
	 			of the x scale values, one for
				each row.  This will be used in
				calculations which are applied
				to the whole array, but depend
				on the values of the x scale.
				In other words, at every point
				in the array, we have its
				corresponding column scale value.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
long addon(l,r,a)
	scope_array		*l,*r,*a;
{
	register float	*f,*fx;
	register int	i,j;

	f = a->vals;
	
	for (i=0; i< a->nrows; i++) {
		fx = a->cols;
		for (j=0; j< a->ncols; j++)
			*f++ = *fx++;
	}
	return (0);
}

