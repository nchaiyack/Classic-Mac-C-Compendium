#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oops.h>

#include "about_Dlog.h"
#include "Alert_Class.h"

extern Alert_Class *myAlert;

/**********************************************************************/
void about_Dlog::HandleDialog(EventRecord *aEvent)
{
	GrafPtr oldPort;
	DialogPtr myDlog;
	int itemHit;
	int dialogDone;
	Handle itemHandle;
	char stuff[80];
	
	/*-------------------------
	Shadow for handle protection
	-------------------------*/
	myDlog = this->myDlog;
	itemHit = this->itemHit;
	dialogDone = this->dialogDone;
	
	GetPort(&oldPort);
	
	if ( myDlog == NULL ) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pmyDlog is NULL",
							"\pDlog::HandleDlog",NULL);
	} /* end if */
	
	SetItemValues();
	BringToFront(myDlog);
	ShowWindow(myDlog);
	SelectWindow(myDlog);

	// If Item #1 happened to be a button then we
	// asume it is the OKAY button and draw the
	// Default rect around it. However, my about box
	// does not use a button to close.
	GetDItem(myDlog,1,&itemType,&itemHandle,&itemRect);
	
	if ( itemType == (btnCtrl+ctrlItem)  ) {
		
		SetPort(myDlog);
		PenSize(3,3);
		InsetRect(&itemRect, -4, -4);
		FrameRoundRect(&itemRect,16,16);
		PenNormal();
	} /* end if */
	
	dialogDone = 0;
	
	// There is a user Item that is #1, when clicked in will
	// cause the dialog to be done and close.
	while ( dialogDone == 0  ) {
		ModalDialog(NULL,&itemHit);
		this->itemHit = itemHit;
		if (itemHit == 1) {
			dialogDone = 1;
		}
	} /* end while */
	
	HideWindow(myDlog);
	SetPort(oldPort);
	BringToFront(oldPort);
	SelectWindow(oldPort);
}
/**********************************************************************/
