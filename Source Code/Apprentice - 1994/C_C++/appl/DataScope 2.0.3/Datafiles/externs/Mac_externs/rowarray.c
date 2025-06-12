#include "DScope.h"
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	rowarray	Fill the result array with
				copies of the y scale values,
				one for each row.  This will
				be used in calculations which
				are applied to the whole
				array, but depend on the
				values of the y scale.  In
				other words, at every point
				in the array, we have its
				corresponding column scale
				value.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
long addon(l,r,a)
	scope_array		*l,*r,*a;
{
	register float	*f,*fy;
	register int	i,j;

	fy = a->rows;
	f = a->vals;
	
	for (i=0; i< a->nrows; i++) {
		for (j=0; j< a->ncols; j++)
			*f++ = *fy;
		fy++;
	}
	return (0);	
}

