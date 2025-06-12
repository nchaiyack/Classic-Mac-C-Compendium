/* -*-C-*- fatal.h */
/*-->fatal*/
/**********************************************************************/
/******************************* fatal ********************************/
/**********************************************************************/
#include "dvihead.h"
#include "commands.h"
#include "gendefs.h"
#include "gblprocs.h"
#include "egblvars.h"
#include "m72.h"
#include "sillymalloc.h"


#define	NIL		0L
#if OS_THINKC
void	Show_error( char *msg );
#endif

void
fatal(msg)				/* issue a fatal error message */
char *msg;				/* message string */
{
	/* 
		For safety, let's clear some memory.  We'll be quitting anyway.
	*/
	sillyfree();
	printf("FATAL--");
	printf("%s\n", msg);
	printf( "Current TeX page counters: [%s]\n",tctos());
    Show_error( msg );
    alldone();
}

/**********************************************************************/
/****************************** warning *******************************/
/**********************************************************************/

void
warning(msg)				/* issue a warning */
char *msg;				/* message string  */
{
    (void)printf("\n%s\n", msg );
	(void)printf("Current TeX page counters: [%s]\n",tctos());
}
