#include "DScope.h"
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	genconst	Given an array and a constant, generate
				an array where the output dimensions are
				the same as the input dimensions and 
				where each element of the array has the
				value of the constant
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
long addon(l,r,a)
	scope_array			*l,*r,*a;
{
	register float		*p,*q;

	if (!l || !r)
	   {a->kind = DS_ERROR;
		return(0);
	   }
	   
	if (l->kind == DS_CONSTANT && r->kind == DS_CONSTANT)
	   {a->kind = DS_ERROR;
		return(0);
	   }
	   
	if (l->kind == DS_ARRAY && r->kind == DS_ARRAY)
	   {a->kind = DS_ERROR;
		return(0);
	   }
	   
	p = a->vals;
	q = p + (long)(a->ncols * a->nrows - 1);
	   
	if (l->kind == DS_CONSTANT)
	   {while (p < q)	*p++ = l->cval;
		*p = l->cval;
	    return (0);
	   }
	
	while (p < q)	*p++ = r->cval;
	*p = r->cval;
	return (0);
}

