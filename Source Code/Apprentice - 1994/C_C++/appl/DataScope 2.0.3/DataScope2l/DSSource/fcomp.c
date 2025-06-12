#include	<resources.h>
#include	"macfview.h"

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	file fcomp.c
			Equation processing code for DataScope.
			Uses lex and yacc generated parsing system to recognize and
			then compute expressions which may contain arrays.
			
	yfor.lex
		- the lex input which contains the lexical specification for
		what is accepted in the notebook window.
		
	yfor.y
		- the yacc input which uses yfor.lex and parses the simple
		expressions which can be used in the notebook window.
		
	On the Unix side:
		lex yfor.lex
		mv lex.yy.c fparse.l.c
		yacc yfor.y
		mv y.tab.c fparse.y.c
		
		There are some changes to these files required - diff them to
		find out what changes to make.
		
	On the Mac, when you compile fparse.y.c it includes fcomp.c and
	fn_fcomp.c in the compile to produce fparse.y.c.o
	
	The parser builds a parse tree which traverseit() follows to 
	execute the equation.  The memory allocation for arrays and constants
	as sub-elements in the parse tree is VERY tricky.  Study it carefully
	if you are going to modify it.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

char	*expstr;
int		explen;

node	*mkone(oper,lf,rt)
		int		oper;
		node	*lf,*rt;
{
		node	*n;

		if (NULL == (n = (node *)NewPtr(sizeof(node))))	return(NULL);

		n->left		= lf;
		n->right	= rt;
		n->token	= oper;
		n->var		= NULL;

		return(n);
}

#ifdef standalone
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	main	main program
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
main()
{
		int		i;
		char	s[255];

		printf("Enter the expression: ");
		gets(s);
		expstr = s;
		printf("yy = %d\n",i=yyparse());

		traverse(nlist);
		traverseit(nlist);

		printf("\ndone\n");
}
#endif

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	execit
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
execit()
{
		int r;
	
		if (yyparse())	return(-1);
		else
		     {r = traverseit(nlist);
			  unparse(nlist);				/* free the used memory */
			  if (r)	return(1);
			  else		return(0);
			 }
}
	
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	yyerror		dialog to indicate parsing error
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
yyerror(es)		char *es;
{
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unparse		Free the space used for the linked list of nodes created
				during the parsing phase.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
unparse(n)		node *n;
{
		if (!n)	return;
		unparse(n->left);
		unparse(n->right);
		if (n->var)	DisposPtr(n->var);
		DisposPtr((Ptr) n);
}
	

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Code to traverse a parse tree and calculate results from array
	operand computation.
	
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
				
struct workert
		{float	cval;	/* constant value when we are carrying a constant */
		
		 int	dimx,
				dimy;	/* x and y dimensions of the array */
	
		 char	talloc,	/* flag byte, is the data array temporarily alloced */
				stype,	/* structure type flag for this structure */
				*id;	/* pointer to a string which is an identifier
						   for this structure */
		
		 struct	fdatawin 
				*dat;	/* if appropriate, data from a current window */
		};

typedef struct workert WORKING;

/* 
*  the valid values for the stype field.
*/

#define SBLANK  1	/* nothing in structure */
#define SERR    2	/* structure hit an error, no fields are valid except stype */
#define SCONST  3	/* cval field is valid as a float constant */
#define SSTR	4	/* *id field is valid as a string pointer (debugging only) */
#define SARRAY  5	/* dp,xs and ys fields are valid for array */
#define SSTOP   6   /* user level abort found */


struct	fdatawin *clonedata();

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	traverseit	Traverse the parse tree, calculating the variable results.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
traverseit(n)
		node *n;
{
		WORKING	a;
		void	fn_mxmn();
	
		a.talloc	= 0;
		a.stype		= SBLANK;

		if (!n)							return(0);
		else if (n->token != TEQUAL)	return(-1);
		else if (!n->left)				return(-1);
		else							calc(n->left, &a);

		/*
		****************************************************
		arrays:  if borrowing space from a real array, clone
				 it; recalc the max and min of the array;
				 install new variable name and then create
				 the window for it.
		****************************************************
		*/
		if (a.stype == SARRAY)
		   {if (!a.dat)		return(-1);
			if (!a.talloc)	a.dat = clonedata(a.dat);

				fn_mxmn(a.dat->vals,(int)(a.dat->xdim * a.dat->ydim),
							&a.dat->valmax,&a.dat->valmin);
							
			strncpy(a.dat->dvar,n->var,20);
			ctextwin(a.dat);				/* make it a real window */
		   }
		else if (a.stype == SERR)	return(-1);
		else if (a.stype == SCONST)	ans_const(a.cval);
		else;
		
		return(0);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_mxmn		Find and get the max and min of a float array
	
				p = pointer to the first element of the array
				n = number of elements in the array
				a = pointer to the max value
				b = pointer to the min value
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_mxmn(p,n,a,b)
		register float	*p;
		float			*a,*b;
		int				n;
{
		register float	max,min;
		register float	*q;

		q = p + n - 1;
		max = *q;
		min = *q;
		while (p < q)
			  {if	   (*p > max)	max = *p++;
			   else if (*p < min)	min = *p++;
			   else					p++;
			  }
		*a = max;
		*b = min;

		return;
}



#include	"fn_fcomp.c"

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	useor		When doing a calculation, determine which pointers
					need to be used, malloc whatever temporary space is
					needed.  If we are already dealing with temporary
					space, just re-use it, it would be destroyed anyway.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
useormalloc(d,a)
		WORKING		*d,*a;
{
		void		ErrorAlert();
	
		a->dimx		= d->dimx;
		a->dimy		= d->dimy;
		a->stype	= SARRAY;
	
		if (d->talloc)
		   {a->dat		= d->dat;
			a->talloc	= 1;
			d->talloc	= 0;
	   	   }
		else
			 {a->dat = clonedata(d->dat);
			  if (!a->dat)
	   			 {a->stype = SERR;
				  ErrorAlert(GetResource('STR ',1001));
				  return(-1);
	   			 }
			  a->talloc = 1;
			 }
		return(0);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	mallocornot		If the calculation requires that the target array
					be different from the source arrays, this makes 
					sure that the answer array is its own temporary
					storage.  Must be called AFTER useormalloc to be
					accurate.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
cloneornot(d,a)
		WORKING		*d,*a;
{
		void		ErrorAlert();
		
	if (a->stype != SARRAY || d->stype != SARRAY)
		return(-1);

	if (a->dat == d->dat) {		/* target and source are the same */
	
		a->dimx		= d->dimx;
		a->dimy		= d->dimy;
		a->stype	= SARRAY;
	
		a->dat = clonedata(d->dat);		/* clone it */
		if (!a->dat)
	   		{a->stype = SERR;
			 ErrorAlert(GetResource('STR ',1001));
				 return(-1);
	   		}
			
		 a->talloc = 1;			/* we know to DisposPtr both of them eventually */
		 d->talloc = 1;
			 
	}

	return(0);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fcompget	Get a character from the expression and pass it to yylex
				for recognition.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
fcompget(a)
		int		a;
{
	
		if (explen <= 0) return(0);
		--explen;
		return((int)tolower((int)*expstr++));
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fcompout	Kill all output from lex/yacc.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
fcompout(a)
		int		a;
{
}


/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	unknown.  Inform the user and look for an external function to
	handle the request..
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#include "DScope.h"
#include "resources.h"

void 
unknown(s,d,e,a)
	char		*s;
	WORKING		*d,*e,*a;
{
	Handle		rsrcH;
	ProcPtr		pp;
	scope_array	lft,rgt,answer;
	int			ret;
	void		ErrorAlert();
	
/*
*  set up the lft,rgt,answer arrays for the call to the external fn.
*/
	lft.kind = DS_CONSTANT;
	lft.cval = 0.0;
	if (d) {
		lft.cval = d->cval;
		if (d->stype == SARRAY) {
			lft.kind = DS_ARRAY;
			lft.nrows = d->dimy;
			lft.ncols = d->dimx;
			lft.vals = d->dat->vals;
			lft.rows = d->dat->yvals;
			lft.cols = d->dat->xvals;
		}
	}

	rgt.kind = DS_CONSTANT;
	rgt.cval = 0.0;
	if (e) {
		rgt.cval = e->cval;
		if (e->stype == SARRAY) {
			rgt.kind = DS_ARRAY;
			rgt.nrows = e->dimy;
			rgt.ncols = e->dimx;
			rgt.vals = e->dat->vals;
			rgt.rows = e->dat->yvals;
			rgt.cols = e->dat->xvals;
		}
	}


	answer.kind = DS_ARRAY;
	answer.vals = a->dat->vals;
	answer.rows = a->dat->yvals;
	answer.cols = a->dat->xvals;
	answer.nrows = a->dimy;
	answer.ncols = a->dimx;
	
	ret = 0;

	while (!ret) {
	
		if (rsrcH = getnamedresource('DSfn', s)) {
			HLock(rsrcH);
			pp = (ProcPtr) *rsrcH;
			(*pp)(&lft,&rgt,&answer);
			HUnlock(rsrcH);
			ReleaseResource(rsrcH);
			ret = -1;
	
			
		}
		else {
			ret = findfunction(s);			/* open the correct resource file */

			if (ret < 0) {
				a->stype = SERR;			/* error return */
				ErrorAlert(GetResource('STR ',1002));
				return;
			}
			
/*
*  Try the network solution if the user chooses.
*/
			if (ret > 0) {
				netfunction(s,&lft,&rgt,&answer);
				ret = -1;
			}
			
		}
			
	}
	
/*
*  fill in the values for the return structure from the external fn.
*  If it was an array, then the data is shared, so the external
*  function already changed all of the values we care about.
*/
	if (answer.kind < 0) {
		a->stype = SERR;
		ErrorAlert(GetResource('STR ',1003));
	}
	else if (answer.kind == DS_CONSTANT) {
		a->cval = answer.cval;
		a->stype = SCONST;
	}
	
	return;							/* found and executed function */

	
}