#pragma once
/*****
 *
 *	DebugUtil.h
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#define kForceAssert	false

/* if you are debugging your code, the first line below should be uncommented.
	If you are shipping your application, the second line should be uncommented.
	You should have only one of the two lines uncommented when you compile */

#if 1
/* #if 0 */

	void	my_assert			( Boolean, StringPtr );
#else
	#define my_assert(a,b)	
#endif

/***  EOF  ***/
