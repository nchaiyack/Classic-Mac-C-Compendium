/*********
** ak_ppc.c
**
** contains code for ppc functions of the program
** and code for launching other apps.
***********/

#ifndef THINK_C
 /* include the <standard mac headers> */
#include <AppleEvents.h>
#include <Desk.h>
#include <Dialogs.h>
#include <Events.h>
#include <Fonts.h>
#include <GestaltEqu.h>
#include <Menus.h>
#include <OSEvents.h>
#include <QuickDraw.h>
#include <THINK.h>
#include <ToolUtils.h>
#include <Types.h>
#include <Windows.h>
#else
 #include "ak_headers"
#endif
#include <BDC.h>
#include <pascal.h>
#include <StandardFile.h>
#include <string.h>
#include "ak_ppc.h"

void DoAlert(Str255, Str255, Str255, Str255); /* in appkiller.c */

void MyPPCInit(void)
{
	OSErr err;
	err = PPCInit();
} /* MyPPCInit() */


void PostWithPPCBrowser(void)		/* p. 5-25, Inside Macintosh-6 */
{
	EventRecord	myHLEvent;
	OSErr	myErr;
	PortInfoRec	myPortInfo;
	TargetID	myTarget;
	short numTries=3;        /* try resending event max 3 times */
	
	myErr = PPCBrowser("\pSelect an Application to Stop", "\pApplication", FALSE,
		&myTarget.location, &myPortInfo, NULL, "\p");
	if (myErr == noErr) {

		/* copy portname into myTarget.name */

		BlockMove( &myPortInfo.name, &myTarget.name, 
				sizeof(myPortInfo.name));
		myHLEvent.what = kHighLevelEvent;
		myHLEvent.message = (long)kCoreEventClass;
		*(long*)(&myHLEvent.where) = (long)kAEQuitApplication;
		
		do {
			myErr = PostHighLevelEvent(&myHLEvent, (long)&myTarget, 0, NULL,0,
							receiverIDisTargetID);
		} while ( (myErr != noErr) && (--numTries > 0));
	} /* if */
} /* PostWithPPCBrowser() */


void DoLaunch(void)
{
	// PLAN OF ATTACK:
	//
	// 1) pop up standard file open box; select any file of type APPL
	// 2) SubLaunch the application, while keeping ourselves alive.
	// 3) Cleanup(?) and return.

	StandardFileReply theSFR;
	SFTypeList	theTypes;
	OSErr	err;

	theTypes[0] = 'APPL';    /* to launch any application */
	theTypes[1] = 'FNDR';    /* to launch the finder again */
	
	StandardGetFile(NULL, 2, theTypes, &theSFR);		// 1)
	
	if ( theSFR.sfGood == TRUE) { 
		LaunchParamBlockRec	launchParams;
		
		launchParams.launchBlockID = extendedBlock;
		launchParams.launchEPBLength = extendedBlockLen;
		launchParams.launchFileFlags = 0;
		launchParams.launchControlFlags = launchContinue // keep us alive
										  + launchNoFileFlags;
		launchParams.launchAppSpec = &(theSFR.sfFile);
		launchParams.launchAppParameters = NULL;
		
		err = LaunchApplication(&launchParams);			// 2)
		
		if (err != noErr) DoAlert("\pSorry, an error occured and",
			"\pSystem 7 choked.", "\p", "\p");
	} /* if user selected file... */
	
	return;												// 3)
} /* DoLaunch() */




