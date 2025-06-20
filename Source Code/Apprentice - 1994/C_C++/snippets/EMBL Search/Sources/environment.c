/*
*********************************************************************
*	
*	environment.c
*	Checks operating system environment
*		
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <Traps.h>
#include <GestaltEqu.h>
#include <Folders.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "environment.h"
#include "util.h"

static Boolean IsGestaltAvail(void);
static short GetSystemVersion(void);
static Boolean IsTempMemAvail(void);
static Boolean IsColorQDAvail(void);
static Boolean IsBalloonHelpAvail(void);
static Boolean IsAppleEventsAvail(void);
static Boolean IsFindFolderAvail(void);
static Boolean IsAliasMgrAvail(void);
static Boolean IsFSSpecAvail(void);
static short FindSysFolder(void);
static short FindTempFolder(void);
static short FindPrefFolder(void);
static Boolean TrapAvailable(short theTrap);
static TrapType GetTrapType(short theTrap);
static short NumToolboxTraps(void);
static long GetGestaltResult(OSType gestaltSelector);


/*
******************************** Global variables *****************
*/

short		gSysVRefNum;
short		gTempVRefNum;
short		gPrefVRefNum;
short		gSysVersion;

Boolean	gHasGestalt;
Boolean	gHasAppleEvents;
Boolean	gHasTempMemory;
Boolean	gHasFindFolder;
Boolean	gHasBalloonHelp;
Boolean	gHasAliasMgr;
Boolean	gHasFSSpec;
Boolean	gHasColorQD;

Str255	gAppName;
short		gAppResRef;							/* Application rsrc file ref num */

/**************************************
*	Check system version, availability of certain functions, etc.
*/

void CheckEnvironment()
{
	Handle	apParam;
	
	/* Can we use Gestalt? */
	gHasGestalt=IsGestaltAvail();

	/* No support for System versions less than 6.0.x */
	if ( (gSysVersion=GetSystemVersion()) < 0x0605 )
		FatalErrorMsg(ERR_VERSION);

	/* Get application name */
	GetAppParms(gAppName, &gAppResRef, &apParam);

	/* Check for availability of some specific functions */
	gHasTempMemory		= IsTempMemAvail();
	gHasBalloonHelp	= IsBalloonHelpAvail();
	gHasFindFolder		= IsFindFolderAvail();
	gHasAppleEvents	= IsAppleEventsAvail();
	gHasAliasMgr		= IsAliasMgrAvail();
	gHasFSSpec			= IsFSSpecAvail();
	gHasColorQD			= IsColorQDAvail();
	
	/* Get system folder vRefNum */
	gSysVRefNum = FindSysFolder(); /* error check ... */
	
	/* Find temporary items folder
		If FindFolder is available we use/create the "Temporary Items" folder,
		otherwise we simply use the System folder (we don't look for a
		subfolder in this case, because names are language-dependent
	*/
	if( (gTempVRefNum = FindTempFolder()) == 0)
		gTempVRefNum = gSysVRefNum;
		
	/* Find preferences folder
		If FindFolder is available we use/create the "Preferences" folder,
		otherwise we simply use the System folder (we dont look for a
		subfolder in this case, because names are language-dependent
	*/
	if( (gPrefVRefNum = FindPrefFolder()) == 0)
		gPrefVRefNum = gSysVRefNum;
}

/**************************************
*	Check for availability of Gestalt
*	Return value:	TRUE, if Gestalt available
*						FALSE, if not
*/

static Boolean IsGestaltAvail()
{
#define _Gestalt 0xA1Ad

	return(TrapAvailable(_Gestalt));
}

/**************************************
*	Get System version
*	Return value:	System version, or 0 if not available
*/

static short GetSystemVersion()
{
	SysEnvRec theWorld;

	if(gHasGestalt)
		return( (short)GetGestaltResult (gestaltSystemVersion) );
	else
		return( (SysEnvirons(curSysEnvVers,&theWorld)) ? theWorld.systemVersion : 0);
}

/**************************************
*	Check for availability of temporary memory calls
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsTempMemAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltOSAttr);
	if(result && (result & (1 << gestaltTempMemSupport)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Check for availability of color QuickDraw
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsColorQDAvail()
{
	long result;
	SysEnvRec theWorld;
	
	if(gHasGestalt) {
		result = GetGestaltResult(gestaltQuickdrawVersion);
		if(result && result >= 0x100)
			return(TRUE);
		else return(FALSE);
	}
	else
		return( (SysEnvirons(curSysEnvVers,&theWorld)) ? theWorld.hasColorQD : FALSE);
}

/**************************************
*	Check for Help Manager
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsBalloonHelpAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltHelpMgrAttr);
	if(result && (result & (1 << gestaltHelpMgrPresent)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Check for AppleEvents
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsAppleEventsAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltAppleEventsAttr);
	if(result && (result & (1 << gestaltAppleEventsPresent)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Check for FindFolder
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsFindFolderAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltFindFolderAttr);
	if(result && (result & (1 << gestaltFindFolderPresent)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Check for Alias Manager
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsAliasMgrAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltAliasMgrAttr);
	if(result && (result & (1 << gestaltAliasMgrPresent)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Check for System 7 File Manager calls
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean IsFSSpecAvail()
{
	long result;
	
	result=GetGestaltResult(gestaltFSAttr);
	if(result && (result & (1 << gestaltHasFSSpecCalls)))
		return(TRUE);
	else return(FALSE);

}

/**************************************
*	Obtain working directory refnum for system folder
*	Return value:	working directory refnum or 0, if not available
*/

static short FindSysFolder()
{
	SysEnvRec	envRec;
	
	if (!SysEnvirons (curSysEnvVers, &envRec))
		return(envRec.sysVRefNum);
	else return(0);
}

/**************************************
*	Obtain working directory refnum for "Temporary Items" folder.
*	Return value:	working directory refnum or 0, if not available
*/

static short FindTempFolder()
{
	short foundVRefNum;
	long foundDirID;
	OSErr err;
	short	wdRefNum;
		
	err=noErr;
	
	/* find folder and create it if necessary */
	if(gHasFindFolder)
		err=FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder, 
				&foundVRefNum, &foundDirID);
				
	if(!gHasFindFolder || err != noErr)
		return(0);
	else	/* convert directory ID to working directory refnum */
		err=OpenWD(foundVRefNum,foundDirID,kApplSignature,&wdRefNum);

	return( (err != noErr) ? 0 : wdRefNum);
}


/**************************************
*	Get working directory refnum for "Preferences" folder.
*	Return value:	working directory refnum or 0, if not available
*/

static short FindPrefFolder()
{
	short foundVRefNum;
	long foundDirID;
	OSErr err;
	short wdRefNum;
	
	err=noErr;
	
	/* find folder and create it if necessary */
	if(gHasFindFolder)
	err=FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
				&foundVRefNum, &foundDirID);
				
	if(!gHasFindFolder || err != noErr)
		return(0);
	else	/* convert directory ID to working directory refnum */
		err = OpenWD(foundVRefNum,foundDirID,kApplSignature,&wdRefNum);

	return( (err != noErr) ? 0 : wdRefNum);
}

/**************************************
*	Check availability of trap
*	Return value:	TRUE, if available
*						FALSE, if not
*/

static Boolean TrapAvailable(short theTrap)
{
	TrapType tType;
	
	tType=GetTrapType(theTrap);
	if(tType == ToolTrap) {
		theTrap &= 0x07FF;
		if(theTrap >= NumToolboxTraps())
			theTrap = _Unimplemented;
	}
	return( 	NGetTrapAddress(theTrap,tType) !=
				NGetTrapAddress(_Unimplemented,ToolTrap) );
}

/**************************************
*	Obtain trap type
*	Return value:	trap type
*/

static TrapType GetTrapType(short theTrap)
{
#define TrapMask 0x0800

	return( (theTrap & TrapMask > 0) ? ToolTrap : OSTrap);
}

/**************************************
*	Check size of trap table
*	Return value:	size of trap table
*/
		
static short NumToolboxTraps()
{
	return( (NGetTrapAddress(_InitGraf, ToolTrap) ==
				NGetTrapAddress(0xAA6E,ToolTrap) ) ? 0x200 : 0x400);
}

/**************************************
*	Call Gestalt to get system information.
*	Return value:	value returned by Gestalt, or 0 if Gestalt is not available
*/

static long	GetGestaltResult(OSType gestaltSelector)
{
	long	gestaltResult;
	
	if(!gHasGestalt)
		return(0L);
		
	if (Gestalt(gestaltSelector, &gestaltResult) == noErr)
		return(gestaltResult);
	else
		return(0);
}

