#pragma once
/*****
 *
 *	Events.h
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

/***  FUNCTION PROTOTYPES  ***/

	void	doHighLevelEvent	( EventRecord * );
	void	doMouseDown			( EventRecord * );
	void	doMouseUp			( EventRecord * );
	void	doKeyDown			( EventRecord * );
	void	doAutoKey			( EventRecord * );
	void	doKeyUp				( EventRecord * );
	void	doActivateEvent		( EventRecord * );
	void	doUpdateEvent		( EventRecord * );
	void	doOsEvt				( const EventRecord * );
	void	doDiskEvt			( EventRecord * );
	void	doIdle				( EventRecord * );


/***** EOF *****/
