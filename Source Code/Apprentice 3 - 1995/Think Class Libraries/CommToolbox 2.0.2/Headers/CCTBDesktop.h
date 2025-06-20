/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CCTBDesktop.h
	
	CommToolbox compatible Desktop.

	SUPERCLASS = CDesktop / CFWDesktop.
	
	Copyright � 1992 Romain Vignes. All rights reserved.

같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#ifndef _H_CCTBDesktop
#define _H_CCTBDesktop

#include <CDesktop.h>				/* Interface for its superclass */

/* Forward declarations */

class CBureaucrat;

/* Class definition */

class CCTBDesktop : public CDesktop	{

public:
	TCL_DECLARE_CLASS

	CCTBDesktop(CBureaucrat *aSupervisor);
	
	virtual void	DispatchClick(EventRecord *macEvent);
};

#endif

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */