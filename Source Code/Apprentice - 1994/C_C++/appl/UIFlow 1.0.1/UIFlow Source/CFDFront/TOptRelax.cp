#pragma segment MenuOption
//*********************************************************************************
//	TRelax - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//	fRelax	[0]	=	u
//				[1]	=	v
//				[2]	=	pressure
//				[3]	=	w
//				[4]	=	kinetic energy & dissipation
//				[5]	=	density
//				[6]	=	h, f, fu, & g
//------------------------------------------------------------------------------------------
//	TOptFlow DoChoice Method... Implementation
//------------------------------------------------------------------------------------------
pascal void TRelax::DoChoice(TView *origView, short itsChoice)
	{
	TWindow		* aWindow;
	short 			index;
	
	aWindow = this->GetWindow();																// window containing this object
	
	switch (itsChoice)
		{
		case mVScrollBarHit :																		// change a scroll bar?
			float itsValue;																				// the coord of the scroller
			for (index = 1; index < 8; index++)												// cycle through all scrollers
				{
				this->GetControlName(index,"SC");											// get scroller name
				if (origView == (TScrollBar *) aWindow->FindSubView(tbox.boxID)) // get the scroll bar object
					{
					itsValue = (float) ((TScrollBar *) origView)->fLongVal;			// scroller value
					itsValue =  1 - (itsValue / 10);
					this->DisplayValue(index,itsValue);
					((TCFDFrontDocument *) fDocument)->SetRelaxOpts(index,itsValue); // store the data
					break;
					}
				}
			break;
		}
	inherited::DoChoice(origView,itsChoice);
	}

//------------------------------------------------------------------------------------------
//	Dismiss the Dialog
//------------------------------------------------------------------------------------------
pascal void TRelax::DismissDialog(ResType dismisser)
	{
	((TCFDFrontDocument *) fDocument)->SetDialogOn(false,cRelaxDialog);
	inherited::DismissDialog(dismisser);
	}

//------------------------------------------------------------------------------------------
//	Decode the print options long int
//------------------------------------------------------------------------------------------
void TRelax::MarkScroller (float * scroller)
	{
	TWindow 		*	aWindow;
	TScrollBar 	*	bar;
	short index;
	long	 value;
	
	aWindow = this->GetWindow();													// get reference to the window

	for (index = 1; index < 8; index++)											// do all scrollers
		{
		this->GetControlName(index,"SC");										// puts scroller name in tbox	
		bar = (TScrollBar *) aWindow->FindSubView(tbox.boxID);		// get pointer to the box
		value = (long) (scroller[index-1] * 10);
		value = 10 - value;
		bar->SetLongVal(value,true);												// set new value & redraw
		this->DisplayValue(index,scroller[index-1]);						// set the static display boxes
		}
	return;
	}

//------------------------------------------------------------------------------------------
//	Display the value associated to the scroll bar
//------------------------------------------------------------------------------------------
void TRelax::DisplayValue (short index, float value)
	{
	TWindow		* aWindow;
	TStaticText	* theText;
	StringPtr		pText;
	char 			strng[10];
	
	aWindow = this->GetWindow();													// window containing this object
	this->GetControlName(index,"SB");											// get associated text field name
	theText = (TStaticText *) aWindow->FindSubView(tbox.boxID); // get  associated text name
	sprintf(strng,"%2.1f",value);													// create a string
	pText = c2pstr(strng);																// convert to pascal
	theText->SetText(pText,kRedraw);											// display the value
	
	}