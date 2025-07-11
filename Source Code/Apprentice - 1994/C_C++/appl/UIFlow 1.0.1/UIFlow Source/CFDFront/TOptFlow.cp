#pragma segment MenuOption
//*********************************************************************************
//	TOptFlow - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//------------------------------------------------------------------------------------------
//	TOptFlow DoChoice Method... Implementation
//------------------------------------------------------------------------------------------
pascal void TOptFlow::DoChoice(TView *origView, short itsChoice)
	{
	TWindow	* aWindow;
	TRadio		* aRadio;
	short		index;
	long 			temp, t, t1;
	
	aWindow = this->GetWindow();												// get the window
	temp = ((TCFDFrontDocument *) fDocument)->GetFlowOpts();
	if (itsChoice == mRadioHit)													// a radio button was selected
		{
		for (index = 1; index < 15; index++)									// look at all buttons
			{
			this->GetControlName(index,"RD");								// get button name
			t 	= pow(2,(index-1));													// get bit value
			t1	= temp & t;																// is it on?
			
			aRadio = (TRadio *) aWindow->FindSubView(tbox.boxID);
			if (origView == aRadio) 												// get radio button
				{
				if (((TRadio *) origView)->IsOn())								// is this button on?
					this->DoLayerDialog((TRadio *) origView,index);	// display additional dialog
				}
			if (t1 == t && !aRadio->IsOn())										// was it on before
				temp -= t;																// no : turn on
			else if (t1 == 0 && aRadio->IsOn())
				temp += t;																// yes : turn off
			}
		((TCFDFrontDocument *) fDocument)->SetFlowOpts(temp);	// store radio configuration
		}
	
	inherited::DoChoice(origView,itsChoice);
	}
	
//------------------------------------------------------------------------------------------
//	Dismiss the Dialog
//------------------------------------------------------------------------------------------
pascal void TOptFlow::DismissDialog(ResType dismisser)
	{
	((TCFDFrontDocument *) fDocument)->SetDialogOn(false,cFlowDialog);
	inherited::DismissDialog(dismisser);
	}

//------------------------------------------------------------------------------------------
//	TOptFlow DoLayerDialog
//------------------------------------------------------------------------------------------
void TOptFlow::DoLayerDialog(TRadio * /*theButton*/, short index)
	{
	TWindow * dWindow;																	// the window
	
	switch (index)
		{
		case 2 :
			TPressure * thePressure;
			
			dWindow = NewTemplateWindow(kOptPressure,fDocument);	// create the dialog box
			if (dWindow == NULL)
				return;
			thePressure = (TPressure *) (dWindow->FindSubView('pres')); // get radius class
			thePressure->GetParams();
			break;
		
		case 3 :
			TPremix * theMix;
			
			dWindow = NewTemplateWindow(kOptPremix,fDocument);		// create the dialog box
			if (dWindow == NULL)
				return;
			theMix = (TPremix *) (dWindow->FindSubView('prem')); 		// get radius class
			theMix->GetParams();
			break;
		
		case 4 :
			TDiffusion * theDiffusion;
			
			dWindow = NewTemplateWindow(kOptDiffusion,fDocument);		// create the dialog box
			if (dWindow == NULL)
				return;
			theDiffusion = (TDiffusion *) (dWindow->FindSubView('diff')); 	// get radius class
			theDiffusion->GetParams();
			break;
		
		case 6 :
			TTurbulence * theTurbulence;
			
			dWindow = NewTemplateWindow(kOptTurbulence,fDocument);	// create the dialog box
			if (dWindow == NULL)
				return;
			theTurbulence = (TTurbulence *) (dWindow->FindSubView('turb')); // get radius class
			theTurbulence->GetParams();
			break;
		
		case 8 :
			TBoundryRadius * theRadius;
			
			dWindow = NewTemplateWindow(kOptRadius,fDocument);		// create the dialog box
			if (dWindow == NULL)
				return;
			theRadius = (TBoundryRadius *) (dWindow->FindSubView('RADI')); // get radius class
			theRadius->GetParams();
			break;
		
		}
	return;
	}
			
//------------------------------------------------------------------------------------------
//	Mark the options correctly
//------------------------------------------------------------------------------------------
void TOptFlow::MarkRadioButton (Boolean * radioButton)
	{
	TWindow 	* aWindow;
	TRadio	 	* button;
	short index;
	
	aWindow = this->GetWindow();													// get reference to the window

	for (index = 1; index <= 14; index++)										// do all check boxes
		{
		this->GetControlName(index,"RD");										// puts checkbox name in tbox	
		button = (TRadio *) aWindow->FindSubView(tbox.boxID);		// get pointer to the box
			
		if (radioButton[index-1])														// is this box on?
			button->SetState(true,false);											// yes mark it
		else
			button->SetState(false,false);
		}
	}
