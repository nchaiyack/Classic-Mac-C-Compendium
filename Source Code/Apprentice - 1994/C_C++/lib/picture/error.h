//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		error.h
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	Defines class for reporting errors.
*/

# ifndef	error_h
# define	error_h

# include	"class.h"
# include	<stdio.h>

/******************************************************************
*   error class.  Intended use: define a global Error object
*	which is allocated in main().  There should be only one
*	error object at a time.  Error reports are written to the
*	file "error.fil".
******************************************************************/
class	Error:public Generic_Class
{
private:
	FILE			*error_file;

public:
	Error(void);
	virtual void	report(char*);
	virtual			~Error(void);
};

# endif