/*
** GetPutEnv.h
**
** Client interfaces to Mac environment variable server.
** This file must be included in any source files that call UNIX
** getenv() or putenv().  Remember to include the GPenv.lib
** library to your project...
**
** 5/12/93 - created
** 5/31/93 - changed around, cleaned up
*/

#ifndef GETPUTENV_H
#define GETPUTENV_H


#define putenv(x)	Putenv(x)		/* to override the default versions */
#define getenv(x)	Getenv(x)

int   Putenv( const char*);	/* Prototypes for the new calls. */
char *Getenv( const char*);	/* Both have same calling semantics of originals */


#endif