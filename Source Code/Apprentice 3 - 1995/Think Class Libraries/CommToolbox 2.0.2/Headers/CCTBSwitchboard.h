/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CCTBSwitchboard.h
	
	CommToolbox compatible switchboard.
	
	SUPERCLASS = CSwitchboard.
	
	Copyright © 1992-93 Romain Vignes. All rights reserved.
	modified Ithran Einhorn 1994
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#ifndef _H_CCTBSwitchboard
#define _H_CCTBSwitchboard

#include <CSwitchboard.h>				/* Interface for its superclass */

/* Class definition */

class CCTBSwitchboard : public CSwitchboard	{

public:
	CCTBSwitchboard();
	
	void	DispatchEvent(EventRecord* macEvent);
};

#endif

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
