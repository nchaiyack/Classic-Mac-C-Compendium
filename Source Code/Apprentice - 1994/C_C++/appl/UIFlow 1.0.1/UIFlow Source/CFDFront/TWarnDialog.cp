#pragma segment warning
//**********************************************************************
//	The TWarning class methods.
//**********************************************************************
//--------------------------------------------------------------------------------------------------
// 	Initialize the Dialog
//--------------------------------------------------------------------------------------------------
void TWarning::IWarning(short which,char * parm)
	{
	msg = new char[256];
	switch(which)
		{
		case 0 :
			sprintf(msg,"The requested point is beyond the bounds of this view.\nMaximum views point is 400 x 400\n%s",parm);
			fIconNumber = 136;
			break;
		case 1 :
			sprintf(msg,"The %s boundries MUST have an Equal Number of Segment Points",parm);
			fIconNumber = 129;
			break;
		case 2 :
			sprintf(msg,"The %s boundries MUST have an Equal Number of Segment Points",parm);
			fIconNumber = 130;
			break;
		case 3 :
			sprintf(msg,"Select a Segment to Edit");
			fIconNumber = 0;
			break;
		case 4:
			sprintf(msg,"...No Cell Selected...\nClick the Mouse Inside a Cell to Select.");
			fIconNumber = 137;
			break;
		case 5:
			sprintf(msg,"...No Obstacle Created...\nShow Grid and Click Inside a Cell.");
			fIconNumber = 137;
			break;
		case 6:
			sprintf(msg,"...Can Not Read File...");
			fIconNumber = NULL;
			break;
		case 7:
			sprintf(msg,"...Can Not Delete a Grid Point With an Opposing Segment Point...\nDelete the Segment Point on Opposite Boundry.");
			fIconNumber = NULL;
			break;
		case 8:
			sprintf(msg,"Can Not Add a Grid Line Through a(n) %s\nPlease Delete the Connecting %s(s) Before Adding the Line.",parm,parm);
			if (strcmp(parm,"Obstacle") == 0)
				fIconNumber = 138;
			else
				fIconNumber = 139;
			break;
		case 9:
			sprintf(msg,"Can Not Delete a Grid Line Through a(n) %s\nPlease Delete the Connecting %s(s) Before Deleting the Line.",parm,parm);
			if (strcmp(parm,"Obstacle") == 0)
				fIconNumber = 138;
			else
				fIconNumber = 139;
			break;
		}
	}
	
// --------------------------------------------------------------------------------------------------
// 	Show the Dialog
// --------------------------------------------------------------------------------------------------
void TWarning::ShowWarning(void)
	{	
	Rect				iconRect;
	TWindow 		* aWindow;
	TStaticText	* warnUser;
	TIcon 			* theIcon;
	CIconHandle	cIcon;
	IDType 			dismisser;

	aWindow	= this->GetWindow();												// get the window for this object
	warnUser	= (TStaticText *) aWindow->FindSubView('outb');		// static text field
	theIcon     = (TIcon *) aWindow->FindSubView('micn');

	this->fDefaultItem = 'OKOK';														// set defaults
	this->fCancelItem = 'CNCL';

	iconRect.top			= 8;
	iconRect.left			= 8;
	iconRect.bottom	= 40;
	iconRect.right		= 40;
	
	cIcon = GetCIcon(fIconNumber);													// get the icon
	if (cIcon != NULL)
		{
		HLock((Handle) cIcon);
		theIcon->fRsrcID = fIconNumber;
		theIcon->SetIcon((Handle) cIcon,true);
		}
			
	StringPtr dText = c2pstr(msg);	
	warnUser->SetText(dText,false);
		
	dismisser = this->PoseModally();
	if (cIcon != NULL)
		HUnlock((Handle) cIcon);
		
	aWindow->Close();
	}

pascal void TWarning::Free(void)
	{
	delete msg;
	inherited::Free();
	}