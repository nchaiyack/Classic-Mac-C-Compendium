/*
	StdControlPanel
	©1995 Chris K. Thomas.  All Rights Reserved.
	
	Safe Control Panel class.
*/

// * includes
#include <UException.h>
#include "StdControlPanel.h"
#include <a4stuff.h>
//#include <UDrawingState.h>

// * locals
static StdControlPanel *gOurPanel = NULL;

// * implementation
pascal unsigned long main (short message, short item, short numItems, 
					short cPrivateVal, const EventRecord *theEvent, 
					 unsigned long cdevStorageValue,  DialogPtr 
						cPDialog)
{
	long out = 554;
	
	EnterCodeResource();
	
	SetPort(cPDialog);
//	InvertRect(&cPDialog->portRect);
	
	Try_
	{
		if(gOurPanel)
		{
			gOurPanel->SetLastEvent((EventRecord *)theEvent);
			switch(message)
			{
				case updateDev:
					gOurPanel->Update();
					break;
					
				case hitDev:
					gOurPanel->Click();
					break;
					
				case nulDev:
					gOurPanel->Idle();
					break;
					
				case activDev:
					gOurPanel->Activate();
					break;
					
				case deactivDev:
					gOurPanel->Deactivate();
					break;
					
				case keyEvtDev:
					gOurPanel->KeyDown();
					break;
					
				case undoDev:
					gOurPanel->Undo();
					break;
					
				case cutDev:
					gOurPanel->Cut();
					break;
					
				case copyDev:
					gOurPanel->Copy();
					break;
					
				case pasteDev:
					gOurPanel->Paste();
					break;
					
				case clearDev:
					gOurPanel->Clear();
					break;
					
				case closeDev:
					if(gOurPanel)
						delete gOurPanel;
					
					gOurPanel = NULL;
					break;
			}
		}
		else
		{
			switch(message)
			{
				case initDev:
					gOurPanel = CreateControlPanel(cPDialog);	//user-provided function
					ThrowIfNULL_(gOurPanel);
					break;
			}
		}
		
		
	}
	Catch_(inErr)
	{
		SysBeep(32);
		out = cdevGenErr;
	}
	EndCatch_
	
	ExitCodeResource();
	
	return out;
}

StdControlPanel::StdControlPanel(DialogPtr inDialog)
{
	mOurDialog = inDialog;
}

StdControlPanel::~StdControlPanel()
{

}