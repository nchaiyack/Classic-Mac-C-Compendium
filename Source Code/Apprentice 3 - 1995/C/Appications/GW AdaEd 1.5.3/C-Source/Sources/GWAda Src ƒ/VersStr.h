/*  VersStr.h */


/************************************************************************************/
/*   Copyright Joseph Laffey, 1993.													*/

/*	 You may use this code snippet in anyhting you'd like							*/

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
/*  as a paramater																	*/

/*  Assumes that the current resource file includes the appropriate 'vers' resource */
/************************************************************************************/


/************************************************************************************/

void	GetVersStr( Str255	theString );
	/* function prototype */

/************************************************************************************/

