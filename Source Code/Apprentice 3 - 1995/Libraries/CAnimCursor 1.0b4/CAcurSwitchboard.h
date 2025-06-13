/*
 * CAcurSwitchboard.h
 * Version 1.0b3, 13 May 1992
 *
 */



/********************************/

#pragma once

/********************************/

#include <CSwitchboard.h>

/********************************/



class CAcurSwitchboard : public CSwitchboard {
	
public:
	
	void			IAcurSwitchboard(void);
	
	void			ProcessEvent(void); // override
	void			DispatchEvent(EventRecord *macEvent); // override
} ;
