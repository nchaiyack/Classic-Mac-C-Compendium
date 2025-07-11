#pragma segment MenuOption
//*********************************************************************************
//	TOptPrint - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//	Print Variable Order
//		u velocity
//		v velocity
//		pressure
//		w velocity
//		enthalpy (sigma h)
//		kinetic energy
//		dissipation
//		mixture fraction (sigma f)
//		fuel fraction (sigma fu)
//		concentration fluctuation (sigma g)
//		density
//		chemistry variables
//
pascal void TOptPrint::IRes(TDocument *itsDocument, TView *itsSuperView, Ptr *itsParams)
	{
	inherited::IRes(itsDocument,itsSuperView,itsParams);
	}
	
//------------------------------------------------------------------------------------------
//	TOptPrint DoChoice Method... Implementation
//------------------------------------------------------------------------------------------
pascal void TOptPrint::DoChoice(TView *origView, short itsChoice)
	{
	long printO,t, t1;
	short index;
	TWindow * aWindow;

	aWindow = this->GetWindow();	
	printO = ((TCFDFrontDocument *) fDocument)->GetPrintOpts();
	switch (itsChoice)
		{
		case mCheckBoxHit :
			for (index = 1; index <= 12; index++)
				{
				this->GetControlName(index,"PC");											// put name in tbox				
				if (origView == (TCheckBox *) aWindow->FindSubView(tbox.boxID))
					{
					t = pow(2,(index - 1));
					t1 = printO & t;
					if (t1 == t && !(((TCheckBox *) origView)->IsOn()))
						printO -= t;
					else if (t1 == 0 && (((TCheckBox *) origView)->IsOn()))
						printO += t;						
					}
				}
			((TCFDFrontDocument *) fDocument)->SetPrintOpts(printO);
			break;
			}
	inherited::DoChoice(origView,itsChoice);
	}
	
//------------------------------------------------------------------------------------------
//	Dismiss the Dialog
//------------------------------------------------------------------------------------------
pascal void TOptPrint::DismissDialog(ResType dismisser)
	{
	((TCFDFrontDocument *) fDocument)->SetDialogOn(false,cPrintDialog);
	inherited::DismissDialog(dismisser);
	}

//------------------------------------------------------------------------------------------
//	Decode the print options long int
//------------------------------------------------------------------------------------------
void TOptPrint::MarkCheckBox (Boolean * checkBox)
	{
	TWindow * aWindow;
	TCheckBox * box;
	short index;
	
	aWindow = this->GetWindow();													// get reference to the window

	for (index = 1; index <= 12; index++)										// do all check boxes
		{
		this->GetControlName(index,"PC");										// puts checkbox name in tbox	
		box = (TCheckBox *) aWindow->FindSubView(tbox.boxID);	// get pointer to the box
			
		if (checkBox[index-1])															// is this box on?
			box->SetState(true,false);												// yes mark it
		}
	}
