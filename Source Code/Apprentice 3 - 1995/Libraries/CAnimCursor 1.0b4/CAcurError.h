/*
 * CAcurError.h
 * Version 1.0b3, 18 May 1992
 *
 */



/********************************/

#pragma once

/********************************/

#include <CError.h>

/********************************/



class CAcurError : public CError {
	
public:
	
	void			SevereMacError(OSErr macErr);
	Boolean		CheckOSError(OSErr macErr);
	void			PostAlert(short STRid, short index);
	
} ;
