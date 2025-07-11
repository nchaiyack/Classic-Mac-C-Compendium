/*
	LControlPanel
	
	PowerPlant-based control panel.
	�1995 Chris K. Thomas.  All Rights Reserved.
*/

// * includes
#include <LGrafPortView.h>
#include "LControlPanel.h"
#include <UDrawingState.h>
//#include <UEnvironment.h>

#define DebugMe(x) //DebugStr(x)

// * implementation
LControlPanel::LControlPanel(DialogPtr inCPDialog)
:StdControlPanel(inCPDialog)
{
	UQDGlobals::SetQDGlobals((QDGlobals *) (LMGetCurrentA5() - (sizeof(QDGlobals) - 4)));
	
	Int32		qdVersion = gestaltOriginalQD;
	Gestalt(gestaltQuickdrawVersion, &qdVersion);
//	UEnvironment::SetFeature(env_SupportsColor,
//							 (qdVersion > gestaltOriginalQD))
	
	// uncomment the above if you need to link with UEnvironment-
	// most PP classes don't appear to use it!
	
	mOverView = new LGrafPortView(inCPDialog);
	ThrowIfNULL_(mOverView);
}

LControlPanel::~LControlPanel()
{
	if(mOverView)
		delete mOverView;
}

void LControlPanel::Update()
{
	DebugMe("\pUpdate");
	mOverView->Draw(NULL);
}

void LControlPanel::Click()
{
	DebugMe("\pClick");
	mOverView->ClickInContent(mLastEvent);
}

void LControlPanel::Activate()
{
	DebugMe("\pActivate");
	mOverView->Activate();
}

void LControlPanel::Deactivate()
{
	DebugMe("\pDeactivate");
	mOverView->Deactivate();
}

void LControlPanel::KeyDown()
{
	DebugMe("\pKeyDown");
	mOverView->DoKeyPress(mLastEvent);
}

void LControlPanel::Idle()
{
	DebugMe("\pIdle");
	mOverView->DoIdle(mLastEvent);
}
