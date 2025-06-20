#include "DScope.h"
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	greater	Given 2 arrays or 2 constants or a constant
			and an array, the output is the greater value;
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
long addon(l,r,a)
	scope_array			*l,*r,*a;
{
	register float		*p,*q,*s,*t;

	if (!l || !r)
	   {a->kind = DS_ERROR;
		return(0);
	   }
	   
	if (l->kind == DS_CONSTANT && r->kind == DS_CONSTANT)
	   {a->kind = DS_CONSTANT;
	    if (l->cval > r->cval)	a->cval = l->cval;
		else					a->cval = r->cval;
		return (0);
	   }
	   
	if (l->kind == DS_CONSTANT)
	   {p = r->vals;
	    q = p + (long)(r->ncols * r->nrows - 1);
		t = a->vals;
		while (p < q)
			  {if (*p > l->cval)	*t++ = *p++;
			   else
			   		{*t++ = l->cval;
					 p++;
					}
			  }
		if (*p > l->cval)	*t = *p;
		else				*t = l->cval;
	    return (0);
	   }
	
	if (r->kind == DS_CONSTANT)
	   {p = l->vals;
	    q = p + (long)(l->ncols * l->nrows - 1);
		t = a->vals;
		while (p < q)
			  {if (*p > r->cval)	*t++ = *p++;
			   else
			   		{*t++ = r->cval;
					 p++;
					}
			  }
		if (*p > r->cval)	*t = *p;
		else				*t = r->cval;
	    return (0);
	   }
	
	if (l->nrows != r->nrows || l->ncols != r->ncols)
	   {a->kind = DS_ERROR;
	    return (0);
	   }
	p = l->vals;
	q = p + (long)(l->ncols * l->nrows - 1);
	s = r->vals;
	t = a->vals;
	while (p < q)
		  {if (*p > *s)		*t++ = *p;
		   else				*t++ = *s;
		   p++;
		   s++;
		  }
	if (*p > *s)	*t = *p;
	else			*t = *s;
	return (0);
}

