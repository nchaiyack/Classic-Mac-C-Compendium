/************************************************************************************/
/*   Copyright Joseph Laffey, 1993.													*/

/*	 You may use this code snippet in anything you'd like							*/

/*   It would be cool if you could give me a little credit...  						*/
/************************************************************************************/


/************************************************************************************/
/*   Of course, USE AT YOUR OWN RISK; I'm not responsible for anything, etc., etc.	*/
/************************************************************************************/

/*
Totally rewritten by Fabrizio Oddone, who is as responsible as the original author
*/

/************************************************************************************/
/*  Gets the version number of a program from its 'vers' resource #1 as listed in 	*/
/*  the "short" version field.  The version is returned in a Pascal string passed 	*/
/*  as a parameter.																	*/
/*  Assumes that the current resource file includes the appropriate 'vers' resource */
/************************************************************************************/

/* Assumes inclusion of MacHeaders */
#include	"VersStr.h"

void	GetVersStr( Str255	theString )
{
VersRecHndl	myvers;		/* holds the 'vers' resource */

myvers = (VersRecHndl)Get1Resource('vers', 1);

if (myvers) {
	(void) PLstrncpy(theString, (*myvers)->shortVersion, 15);
	ReleaseResource((Handle)myvers);
	}
else
	StrLength(theString) = 0;
}
