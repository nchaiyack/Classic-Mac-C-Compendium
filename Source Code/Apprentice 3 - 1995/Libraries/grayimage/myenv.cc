// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *			Service C++ functions 
 *	     that support the standard environment for me
 */

#pragma implementation

#include "myenv.h"
#include <builtin.h>
#include <stdarg.h>

/*
 *-----------------------------------------------------------------------
 *		Some global constant pertaining to input/output
 */

const char _Minuses [] = "\
-------------------------------------------------------------------------------";

const char _Asteriscs [] = "\
*******************************************************************************";

const char _Equals [] = "\
===============================================================================";


/*
 *------------------------------------------------------------------------
 *	        Print an error message at stderr and abort
 * Synopsis
 *	volatile void _error(const char * message,... );
 *	Message may contain format control sequences %x. Items to print 
 *	with the control sequences are to be passed as additional arguments to
 *	the function call.
 */

volatile void _error(const char * message,...)
{
  va_list args;
  va_start(args,message);		/* Init 'args' to the beginning of */
					/* the variable length list of args*/
  fprintf(stderr,"\n_error:\n"); 	
  vfprintf(stderr,message,args);
  fputs("\n",stderr);
  abort();
}


/*
 *------------------------------------------------------------------------
 *	       		 Print a message at stderr
 * Synopsis
 *	void message(const char * text,... );
 *	Message may contain format control sequences %x. Items to print 
 *	with the control sequences are to be passed as additional arguments to
 *	the function call.
 */

void message(const char * text,...)
{
  va_list args;
  va_start(args,text);		/* Init 'args' to the beginning of */
					/* the variable length list of args*/
  vfprintf(stderr,text,args);
}

