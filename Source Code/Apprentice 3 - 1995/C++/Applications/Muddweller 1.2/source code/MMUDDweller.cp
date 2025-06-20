/* MMUDDweller - Main program                                                 */

#define  __MMUDDWELLER__

//  {$MC68020-}			// The main program must be universal code
//  {$MC68881-}

		// � Toolbox
#if qDebug
#include <stdio.h>
#include <iostream.h>
#endif

#ifndef __CMINTF__
#include "CMIntf.h"
#endif

#ifndef __CRMINTF__
#include "CRMIntf.h"
#endif

#ifndef __CTBUTILS__
#include "CTBUtils.h"
#endif

#ifndef __OSUTILS__
#include "OSUtils.h"
#endif

		// � MacApp
#ifndef __UMacApp__
#include <UMacApp.h>
#endif

#ifndef __UDialog__
#include <UDialog.h>
#endif

#ifndef __UPrinting__
#include <UPrinting.h>
#endif

#ifndef __UTEView__
#include <UTEView.h>
#endif

		// � Implementation use
#ifndef __COMMANDVIEW__
#include "CommandView.h"
#endif

#ifndef __DOCDIALOGS__
#include "DocDialogs.h"
#endif

#ifndef __GLOBALS__
#include "Globals.h"
#endif

#ifndef __LOGVIEW__
#include "LogView.h"
#endif

#ifndef __LOGWINDOW__
#include "LogWindow.h"
#endif

#ifndef __MACROS__
#include "Macros.h"
#endif

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif

#ifndef __UMUDDWELLER__
#include "UMUDDweller.h"
#endif

#ifndef __NOTRACE__
#include "NoTrace.h"
#endif

#ifndef __USizerView__
#include "USizerView.h"
#endif


//------------------------------------------------------------------------------

const unsigned char kTCPName[] = "\p.ipp";

//------------------------------------------------------------------------------

#pragma segment AInit

pascal Boolean ValidateComm ()
{
	OSErr err;
	Boolean wasPermanent;
	
	gCTBToolsFound = TRUE;
	gCTBAvailable = TrapExists (_CommToolboxDispatch);
	if (gCTBAvailable) {
		wasPermanent = PermAllocation (TRUE);
		err = InitCTBUtilities ();
		if (err == noErr) err = InitCRM ();
		if (err == noErr) err = InitCM ();
		gCTBAvailable = (err == noErr);
		gCTBToolsFound = (err != cmNoTools);
		wasPermanent = PermAllocation (wasPermanent);
	}
	gTCPAvailable = OpenDriver ((Str255) kTCPName, &gTCPRef) == noErr;
	return gCTBAvailable || gTCPAvailable;
}

//------------------------------------------------------------------------------

#pragma segment Main

void main ()
{
	TMUDDwellerApp *gMUDDwellerApp;

	InitToolBox ();
	if (!ValidateConfiguration (&gConfiguration)) {
		StdAlert (phUnsupportedConfiguration);
	} else {
		InitUMacApp (8);
#if qDebug
		cout.sync_with_stdio ();
#endif
		if (!ValidateComm ()) {
			if (!gCTBToolsFound)
				StdAlert (phNoToolsID);
			else
				StdAlert (phNoCommID);
		} else {
			InitNoTrace ();
			InitUDialog ();
			InitUPrinting ();
			InitUTEView ();
			InitUSizerView();
			InitCommandView ();
			InitLogView ();
			InitLogWindow ();
			InitDocDialogs ();
			InitMacros ();
			InitMUDDoc ();
			gMUDDwellerApp = new TMUDDwellerApp;
			FailNIL (gMUDDwellerApp);
			gMUDDwellerApp->IMUDDwellerApp ();
			gMUDDwellerApp->Run ();
		}
	}
}

//------------------------------------------------------------------------------
