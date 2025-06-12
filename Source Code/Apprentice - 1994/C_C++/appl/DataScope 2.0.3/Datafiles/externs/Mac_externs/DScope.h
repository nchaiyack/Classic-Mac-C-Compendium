/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	Data structure for external functions.
	
	The field "kind" determines whether the array contains a valid
	cval or a valid set of ncols,nrows,rows,cols,vals.
	
	External functions are called as:
	your_fn(lft,rgt,answer)
	scope_array *lft,			 left parameter 
				*rgt,			 right parameter 
				*answer;		 place to put the answer 
				
	Answer will always contain pre-allocated space for an array of
	resulting values, including the rows and cols arrays.  You may
	change any value (and should) in the rows,cols and vals arrays.
	Do not change any values in the lft or rgt storage.
	
	If your routine returns only a constant, set kind == DS_CONSTANT
	and put the answer in cval.
	
	DON'T allocate anything you don't free yourself.
	FREE everything you allocate.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#define DS_ERROR    -1
#define DS_CONSTANT  0
#define DS_ARRAY     1

typedef struct 
		{float	
				cval,	/* constant value when we are carrying a constant */
				*rows,	/* row labels, one for each column   */
				*cols,	/* col labels, one for each row   */
				*vals;	/* data values in the array, if there is an array 
							size = ncols*nrows                        */
		
		 int	ncols,
				nrows;	/* dimensions of the array */
	
		 char	
				kind;	/* DS_ERROR, DS_CONSTANT, DS_ARRAY */
		} 
		
		scope_array;

