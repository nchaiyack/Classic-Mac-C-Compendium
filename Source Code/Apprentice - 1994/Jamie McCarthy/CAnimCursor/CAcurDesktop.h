/*
 * CAcurDesktop.h
 * Version 1.0b3, 13 May 1992
 *
 */



/********************************/

#pragma once

/********************************/

#include <CDesktop.h>

/********************************/



class CAcurDesktop : public CDesktop {
	
public:
	
	void			IAcurDesktop(CBureaucrat *aSupervisor);
	
	void			DispatchCursor(Point where, RgnHandle mouseRgn); // override
	
} ;
