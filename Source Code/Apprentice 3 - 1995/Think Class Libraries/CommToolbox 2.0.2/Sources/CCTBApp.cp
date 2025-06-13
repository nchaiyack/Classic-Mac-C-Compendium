/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CCTBApp.c
	
	CommToolbox compatible application class.
	
	SUPERCLASS = CApplication.
	
	Original copyright © 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright © 1994-95 Ithran Einhorn. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#include <CommResources.h>					/* Apple includes */

#include <CError.h>							/* TCL includes */
#include <Global.h>
#include <OSChecks.h>

#ifdef __USETHREADS__
	#include "CThread.h"					/* Thread Manager */
#endif

#include "CConnection.h"					/* Other includes */
#include "CCTBApp.h"
#include "CCTBDesktop.h"
#include "CCTBSwitchboard.h"
#include "CFileTransfer.h"
#include "CTermPane.h"

/* Constants & Macros */

#define ERR_STR_ID			2000	/* Error messages ID */
#define CTB_STR_INDEX		1		/* CTB message */

#define MIN_CTB_VERS		0x100	/* Minimum CommToolbox version */

#define CTB_TRAP			0x8B	/* CommToolbox trap */

/* Variables globales */

extern tSystem	gSystem;
extern CDesktop	*gDesktop;
extern CError	*gError;

TCL_DEFINE_CLASS_M1(CCTBApp,  CApplication);

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * CCTBApp
 *
 * Initialize the CTBApp object
 *
 * extraMasters:		Number of extra blocks
 * aRainyDayFund:		Bytes of memory to reserve
 * aCriticalBalance:	Bytes of memory to reserve for a critical operation	
 * aToolboxBalance:		Bytes to save for the toolbox
 *
 */
 
CCTBApp::CCTBApp(short extraMasters, Size aRainyDayFund,
				 Size aCriticalBalance, Size aToolboxBalance)
					  
		: CApplication(extraMasters,aRainyDayFund,
					   aCriticalBalance,aToolboxBalance)	/* Initialize superclass */
{
	CCTBAppX();
}

/***
 * ICTBApp
 *
 *	alternate construction scheme.
 *
 ***/

void CCTBApp::ICTBApp(short extraMasters, Size aRainyDayFund,
				 Size aCriticalBalance, Size aToolboxBalance)

{
	CApplication::IApplication(extraMasters,aRainyDayFund,
					   aCriticalBalance,aToolboxBalance);

	CCTBAppX();
}

/***
 * CCTBAppX
 *
 *	common construction stuff.
 *
 ***/

void CCTBApp::CCTBAppX(void)

{
#ifdef __USETHREADS__
	threadManagerPresent = CThread::cIsPresent();
#endif

	CApplication::cMaxSleepTime = 0L;
	
	if (!managersPresent())		/* CommToolbox installed ? */
	{
		SysBeep(3);
		gError->PostAlert(ERR_STR_ID,CTB_STR_INDEX);
		this->Exit();
		ExitToShell();
	}
	
	InitCRM();				/* CTB managers initialization */
	InitCTBUtilities();
	CConnection::cInitManager();
	CTermPane::cInitManager();
	CFileTransfer::cInitManager();
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * MakeDesktop
 *
 * CommToolbox desktop initialization
 *
 */

void CCTBApp::MakeDesktop(void)
{
	gDesktop = (CCTBDesktop *)new CCTBDesktop(this);	
}	

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * MakeSwitchboard
 *
 * CommToolbox switchboard initialization
 *
 */

void CCTBApp::MakeSwitchboard(void)
{
	(itsSwitchboard = (CCTBSwitchboard *)new CCTBSwitchboard)->InitAppleEvents();
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * Process1Event
 *
 * Handle the next Event
 *
 */		
		
void CCTBApp::Process1Event(void)
{
	#ifdef __USETHREADS__
	if (threadManagerPresent)
		ThreadBeginCritical();
	#endif

	#ifdef __USETHREADS__
	if (! threadManagerPresent)
	{
	#endif
	
		CConnection::cConnIdle();		/* Idle loop for connection objects */

		CTermPane::cTermIdle();			/* Idle loop for terminal objects */
	
		CFileTransfer::cFTransIdle();	/* Idle loop for transfer objects */
		
	#ifdef __USETHREADS__
	}
	#endif
	
	inherited::Process1Event();		/* Send the message to its superclass */
	
	#ifdef __USETHREADS__
	if (threadManagerPresent)
	{
		ThreadEndCritical();
		YieldToAnyThread();
	}
	#endif
}	

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * GetCTBVersions
 *
 * Return the CTB managers versions
 *
 * CTBVers:		CommToolbox version
 * CMVers:		Connection Manager version
 * TMVers:		Terminal Manager version
 * FTVers:		File Transfer Manager version
 * CRMVers:		Communications Resources Manager version
 *
 */

void CCTBApp::GetCTBVersions(short *CTBVers,short *CMVers,short *TMVers,
								short *FTVers,short *CRMVers)
{
	*CTBVers = CTBGetCTBVersion();
	*CRMVers = CRMGetCRMVersion();
	
	*CMVers = CConnection::cGetCMVersion();
	*TMVers = CTermPane::cGetTMVersion();
	*FTVers = CFileTransfer::cGetFTVersion();
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * managersPresent
 *
 * Are the CTB managers installed ?
 *
 * Return TRUE if all managers are present
 *
 */

Boolean CCTBApp::managersPresent(void)
{
	if (TrapAvailable(CTB_TRAP))
		return TRUE;
	else
		return FALSE;
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
