/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CCTBApp.h
	
	CommToolbox compatible application class.
	
	SUPERCLASS = CApplication.
	
	Original copyright © 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright © 1994-95 Ithran Einhorn. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
 
#ifndef _H_CCTBApp
#define _H_CCTBApp

#include <CApplication.h>			/* Interface for its superclass */

/* Class definition */

class CCTBApp : public CApplication {

protected:
	TCL_DECLARE_CLASS

	virtual Boolean		managersPresent(void);
	Boolean				threadManagerPresent;

public:

	CCTBApp() {};
	CCTBApp(short extraMasters, Size aRainyDayFund,
			Size aCriticalBalance, Size aToolboxBalance);
			
	void 					ICTBApp(short extraMasters, Size aRainyDayFund,
				 					Size aCriticalBalance, Size aToolboxBalance);
				 					
	void 					CCTBAppX(void);
	
	virtual void			MakeDesktop(void);
	virtual void			MakeSwitchboard(void);
	virtual void			Process1Event(void);
	virtual void			GetCTBVersions(short *CTBVers,short *CMVers,
								short *TMVers, short *FTVers,short *CRMVers);
};

#endif

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
