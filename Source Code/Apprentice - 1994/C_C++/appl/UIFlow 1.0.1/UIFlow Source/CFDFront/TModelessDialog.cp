#pragma segment MenuOption
//*********************************************************************************
//	TModelessDialog - Methods
//		inherits all methods and vars from TDialogView
//		provides a base class for the modeless dialog windows in this program
//*********************************************************************************
//------------------------------------------------------------------------------------------
//	TOptFlow BringToFront Method... Implementation
//		Brings the dialog to the front.
//------------------------------------------------------------------------------------------
void TMDialog::BringToFront(void)
	{
	TWindow * aWindow;
	aWindow = this->GetWindow();
	aWindow->Select();
	}
	
//------------------------------------------------------------------------------------------
//	Get the name of this check box
//------------------------------------------------------------------------------------------
void TMDialog::GetControlName (short index, char * strng)
	{
	sprintf(tbox.boxCH,strng);														// stuff letters
	if (index < 10)																			// is index < 10
		sprintf (tbox.boxCH+2,"0%d",index);									// yes: add 0 before index
	else
		sprintf(tbox.boxCH+2,"%d",index);										// no: stuff index
	tbox.boxCH[4] = 0;																	// null terminate
	return;
	}

