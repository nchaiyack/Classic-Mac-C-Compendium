// This may look like C code, but it is really -*- C++ -*-
//************************************************************************
//
//			A standard environment
//			  I am accustomed to

//#pragma once
#ifndef _myenv_h

#define _myenv_h
#pragma interface

				/* Strings of symbols			*/
				/* They may be used as a delimiting lines*/
extern const char _Minuses [];
extern const char _Asteriscs [];
extern const char _Equals [];

				/* Print an error message at stderr and	*/
				/* abort				*/
volatile void _error(
	const char * message,		/* Message to be printed	*/
	...                             /* Additional args to printf	*/
	   );

				/* Print a message at stderr 		*/
void message(
	const char * text,		/* Message to be printed	*/
	...                             /* Additional args to printf	*/
	   );


//------------------------------------------------------------------------
//			Verify the assertion

#if 0
  					/* Print a message and abort*/
extern volatile void _error( const char * message,... ); 
#endif

#define assert(ex) \
        (void)((ex) ? 1 : \
              (_error("Failed assertion " #ex " at line %d of `%s'.\n", \
               __LINE__, __FILE__), 0))
#define assertval(ex) assert(ex)

#define assure(expr,message)				\
	if	(expr) ;				\
	else _error("%s\n at line %d of '%s'.",message,__LINE__, __FILE__);


#endif
