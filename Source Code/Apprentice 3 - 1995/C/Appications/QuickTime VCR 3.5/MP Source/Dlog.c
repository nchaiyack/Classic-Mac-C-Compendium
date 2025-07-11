/*---------------------------------------------------------------------
	Program: Dlog.c
	By: Geoffrey Slinker
	Date: 11:59:55 AM  5/2/92
	Purpose: Contains methods for Dlog class, see Dlog.h
---------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "Dlog.h"
#include "Alert_Class.h"

extern Alert_Class *myAlert;
/**********************************************************************/
Dlog::Dlog(void)	// constructor code
{	
	myDlog = NULL;
	rsrcNumber = 0;
	itemHit = 0;
	itemType = 0;
	dialogDone = 0;
}
/*....................................................................*/
Dlog::~Dlog(void)	// destructor code
{	
	DialogPtr myDlog;
	int rnum;
	
	/*-------------------------
	Shadow for purging problems
	-------------------------*/
	myDlog = this->myDlog;
	rnum = this->rsrcNumber;
	if ( myDlog != NULL  ) {
		FreeDialog(rsrcNumber);
		DisposDialog(myDlog);
	} /* end if */
	this->myDlog = NULL;
}
/*....................................................................*/
void Dlog::loadResource( int number)	// load resource from resource file
{
	DialogPtr myDlog;
	
	/*-------------------------
	Shadow for purging problems
	-------------------------*/
	myDlog = this->myDlog;
	rsrcNumber = number;
	myDlog = GetNewDialog(number,0L,(WindowPtr)-1L);
	
	if ( myDlog == 0L  ) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to allocate new dialog",
							"\pDlog::loadResource",NULL);
	} /* end if */
	
	CouldDialog(rsrcNumber);	// could be purged so lock it down
	
	/*-------------------------
	Copy back from shadow
	-------------------------*/
	this->myDlog = myDlog;

}
/*....................................................................*/
/* This routine simply shows the dialog box and when you click in it
	it close, used mostly for testing position and look, not used
	in the final application code
*/
void Dlog::showDialog()
{
	EventRecord smallEvent;
	WindowPtr whichWindow;
	GrafPtr oldPort;
	int done = 0;
	DialogPtr myDlog;
	
	/*-------------------------
	Shadow for purging problems
	-------------------------*/
	myDlog = this->myDlog;
	
	SetItemValues();

	GetPort(&oldPort);
	SetPort(myDlog);
	SelectWindow(myDlog);
	DrawDialog(myDlog);
	

	while ( !done  ) {
		SystemTask();
		
		GetNextEvent((mDownMask + mUpMask),&smallEvent);
		
		switch ( smallEvent.what  ) {
			case mouseDown:
				FindWindow(smallEvent.where,&whichWindow);
				if ( whichWindow == myDlog  ) {
					done = 1;
				}
				else {
					SysBeep(7);
				} /* end if then else */
				break;
			
			default: break;
		} /* end switch */
	} /* end while */
	
	HideWindow(myDlog);
	
	SetPort(oldPort);
}
/*....................................................................*/
void Dlog::HandleDialog(EventRecord *aEvent)
{
	DialogPtr myDlog;
	int itemHit;
	int dialogDone;
	GrafPtr aPort;
	Point thePoint;
	int thePart;

	
	/*-------------------------
	Shadow for handle protection
	-------------------------*/
	myDlog = this->myDlog;
	itemHit = this->itemHit;
	//dialogDone = this->dialogDone;
	
	
	// this was an experiment in handling dialog events in their
	// class instead of the main event loop.
	switch ( (*aEvent).what  ) {
		
		case updateEvt:
			GetPort(&aPort);
			SetPort((WindowPtr)(*aEvent).message);

			BeginUpdate((WindowPtr)(*aEvent).message);
				RedrawDialog();
			EndUpdate((WindowPtr)(*aEvent).message);
			SetPort(aPort);
			break;
			
		case nullEvent:
			SystemTask();
			break;
				
		default: break;
	} /* end switch */
	
	thePoint = (*aEvent).where;
	GlobalToLocal(&thePoint);

	thePart = FindControl(thePoint,myDlog,&itemHandle);			
	if (DialogSelect(aEvent,&myDlog,&itemHit) ) {
		HandleDialogItem(itemHit,thePoint,thePart);
	}
}
/*....................................................................*/
/* Should be overloaded or something! */
int Dlog::HandleDialogItem(int itemHit, Point thePoint, int thePart)
{
	DialogPtr myDlog;
	Handle itemHandle;
	Rect itemRect;
	int itemType;
	
	myDlog = this->myDlog;
	itemHandle = this->itemHandle;
	itemRect = this->itemRect;
	
	switch ( itemHit  ) {
		
		default: break;
	} /* end switch */
}
/*....................................................................*/
void Dlog::SetItemValues()
{
	/*-------------------------
	Sets item values.	
	-------------------------*/
}
/*....................................................................*/
void Dlog::PutUpDialog()
{
	DialogPtr myDlog;
	
	myDlog = (*this).myDlog;
	
	ShowWindow(myDlog);	
	SetPort(myDlog);
	SelectWindow(myDlog);
	DamageDialog();
}
/*....................................................................*/
void Dlog::RedrawDialog()
{
	DialogPtr myDlog;
	GrafPtr oldPort;
	
	GetPort(&oldPort);
	
	myDlog = (*this).myDlog;
	
	SetPort(myDlog);
	DrawDialog(myDlog);
	SetPort(oldPort);
}
/*....................................................................*/
void Dlog::DamageDialog()
{
	Rect theRect;
	DialogPtr myDlog;
	
	myDlog = (*this).myDlog;
	
	theRect = (*myDlog).portRect;
	InvalRect(&theRect);
}
/**********************************************************************/
