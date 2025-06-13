#pragma once
/*****
 *
 *	ErrorUtil.h
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

/***  CONSTANT DECLARATIONS  ***/

#define krErrSystemDefault		300
#define krStartupErrorStrs		301

#define ksErrSystemDefault		"\pAn error was encountered in a system call."


/***  FUNCTION PROTOTYPES  ***/

	void	ErrorSystem		( OSErr );
	void	ErrorStartup	( short );


/* EOF */