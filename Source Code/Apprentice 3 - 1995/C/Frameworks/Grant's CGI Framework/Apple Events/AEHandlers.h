#pragma once
/*****
 *
 *	AEHandlers.h
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

/***  FUNCTION PROTOTYPES  ***/

pascal	OSErr	doAEOpenApp			( AppleEvent *, AppleEvent *, long );
pascal	OSErr	doAEOpenDoc			( AppleEvent *, AppleEvent *, long );
pascal	OSErr	doAEPrintDoc		( AppleEvent *, AppleEvent *, long );
pascal	OSErr	doAEQuitApp			( AppleEvent *, AppleEvent *, long );
pascal	OSErr	doAECGISearchDoc	( AppleEvent *, AppleEvent *, long );

		OSErr	myInstallAEHandlers	( void );


/***** EOF *****/
