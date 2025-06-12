#pragma segment MenuOption
//*********************************************************************************
//	TOptSolution - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//	fSolution	[0]	=	number of grid levels
//					[1]	=	number of sweeps on momentum equations
//					[2]	=	number of sweeps on pressure corrections
//					[3]	=	number of sweeps on scalar equations
//	fAccuracy		=	solution accuracy
//	fIteration			=	maximum iterations
//------------------------------------------------------------------------------------------
//	TOptFlow DoChoice Method... Implementation
//------------------------------------------------------------------------------------------
pascal void TOptSolution::DoChoice(TView *origView, short itsChoice)
	{
	TWindow		* aWindow;
	short 			index;
	
	aWindow = this->GetWindow();																// window containing this object
	
	switch (itsChoice)
		{
		case mHScrollBarHit :																		// change a scroll bar?
			long itsValue;																				// the coord of the scroller
			for (index = 1; index < 5; index++)												// cycle through all scrollers
				{
				this->GetControlName(index,"SR");											// get scroller name
				if (origView == (TScrollBar *) aWindow->FindSubView(tbox.boxID)) // get the scroll bar object
					{
					itsValue = ((TScrollBar *) origView)->fLongVal;					// scroller value
					this->DisplayValue(index,itsValue);
					((TCFDFrontDocument *) fDocument)->SetSolutionOpts(true,index,itsValue,0.0,0); // store the data
					break;
					}
				}
			break;
		}
	inherited::DoChoice(origView,itsChoice);
	this->SaveEdit();
	}

//------------------------------------------------------------------------------------------
//	Dismiss the Dialog
//------------------------------------------------------------------------------------------
pascal TCommand * TOptSolution::DoKeyCommand(short ch, short aKeyCode, EventInfo *info)
	{
	inherited::DoKeyCommand(ch,aKeyCode,info);
	this->SaveEdit();
	return gNoChanges;
	}
	
//------------------------------------------------------------------------------------------
//	Store the Accuracy field to the document
//------------------------------------------------------------------------------------------
void TOptSolution::SaveEdit(void)
	{
	TWindow		* aWindow;
	TEditText		* eText;
	char 			* accString;
	float 			accuracy;
	short			iteration;
	Str255 		atemp;
	
	aWindow = this->GetWindow();																// window containing this object
	eText = (TEditText *) aWindow->FindSubView('ED01');
	eText->GetText(atemp);
	accString = p2cstr((StringPtr) atemp);
	accuracy = atof(accString);
	
	eText = (TEditText *) aWindow->FindSubView('ED02');
	eText->GetText(atemp);
	accString = p2cstr((StringPtr) atemp);
	iteration = atoi(accString);
			
	((TCFDFrontDocument *) fDocument)->SetSolutionOpts(false,0,0L,accuracy,iteration);
	}
	
//------------------------------------------------------------------------------------------
//	Override the bring to front method
//------------------------------------------------------------------------------------------
void TOptSolution::BringToFront(void)
	{
	inherited::BringToFront();
	this->SaveEdit();
	}

//------------------------------------------------------------------------------------------
//	Dismiss the Dialog
//------------------------------------------------------------------------------------------
pascal void TOptSolution::DismissDialog(ResType dismisser)
	{
	this->SaveEdit();
	((TCFDFrontDocument *) fDocument)->SetDialogOn(false,cSolutionDialog);
	inherited::DismissDialog(dismisser);
	}

//------------------------------------------------------------------------------------------
//	Decode the print options long int
//------------------------------------------------------------------------------------------
void TOptSolution::MarkScroller (long * scroller, float accuracy, short iteration)
	{
	TWindow 		*	aWindow;
	TScrollBar 	*	bar;
	TEditText		*	eText;
	StringPtr 			pAccuracy;
	char				 	sAccuracy[15];
	short index;
	
	aWindow = this->GetWindow();													// get reference to the window

	for (index = 1; index < 5; index++)											// do all scrollers
		{
		this->GetControlName(index,"SR");										// puts scroller name in tbox	
		bar = (TScrollBar *) aWindow->FindSubView(tbox.boxID);		// get pointer to the box
		bar->SetLongVal(scroller[index-1],true);								// set new value & redraw
		this->DisplayValue(index,scroller[index-1]);						// set the static display boxes
		}
	sprintf(sAccuracy,"%f",accuracy);											// convert accuracy
	pAccuracy = c2pstr(sAccuracy);
	eText = (TEditText *) aWindow->FindSubView('ED01');				// get accuracy window
	eText->SetText(pAccuracy,true);											// display accuracy data

	sprintf(sAccuracy,"%d",iteration);											// convert accuracy
	pAccuracy = c2pstr(sAccuracy);
	eText = (TEditText *) aWindow->FindSubView('ED02');				// get accuracy window
	eText->SetText(pAccuracy,true);											// display accuracy data
	}

//------------------------------------------------------------------------------------------
//	Display the value associated to the scroll bar
//------------------------------------------------------------------------------------------
void TOptSolution::DisplayValue (short index, long value)
	{
	TWindow		* aWindow;
	TStaticText	* theText;
	StringPtr		pText;
	char 			strng[10];
	
	aWindow = this->GetWindow();													// window containing this object
	this->GetControlName(index,"SC");											// get associated text field name
	theText = (TStaticText *) aWindow->FindSubView(tbox.boxID); // get  associated text name
	sprintf(strng,"%ld",value);														// create a string
	pText = c2pstr(strng);																// convert to pascal
	theText->SetText(pText,kRedraw);											// display the value
	}