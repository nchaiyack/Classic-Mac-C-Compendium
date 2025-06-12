/*
** GetPutEnvP.h
**
** Private header file for the Getenv()/Putenv() functions.
*/

#ifndef GETPUTENVP_H
#define GETPUTENVP_H


enum {

/** Error types **/

errESNotFound = 0x00000fa0,     /* decimal 4000; hex for easier debugging */

/** Event classes, types and appl signature **/

kESSignature	=	'EnvS',
kESEventClass	=	'EnvS',
kESGetEnvMsg	=	'GENV',
kESPutEnvMsg	=	'PENV',

/** EnvironServer keywords **/
keyESLen	=	'EnvL',		/* length of data parameter */
keyESString	=	'Envs',		/* String parameter's data bytes */
keyESFlag	=	'EnvF',		/* Success/fail flags for server responses */

/* ---------- */
nothing = 0
};


#endif

