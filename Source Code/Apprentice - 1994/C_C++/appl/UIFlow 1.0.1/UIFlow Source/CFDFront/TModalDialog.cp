#pragma segment MenuOption
//*********************************************************************************
//	Modal Dialog Boxes used by the menus
//	these are created by the modeless dialogs
//*********************************************************************************
//------------------------------------------------------------------------------------------
//	TBoundryRadius - GetParams
//------------------------------------------------------------------------------------------
void TBoundryRadius::GetParams(void)
	{
	float				radius;
	TWindow 	* aWindow;																			// window for this dialog
	TEditText	* theRadius;																			// radius of the inner boundry
	IDType			dismisser;
	Str255			pRadius;
	StringPtr		pText;
	char 		*	itsRadius, iRadius[20];
	
	aWindow = this->GetWindow();																// get the window
	theRadius = (TEditText *) aWindow->FindSubView('RD01');					// get the text window

	this->fDefaultItem = 'OKOK';																	// set the default items
	this->fCancelItem = 'CNCL';
	radius = ((TCFDFrontDocument *) fDocument)->GetRadiusOpts();		 	// store the data
	sprintf(iRadius,"%f",radius);																// convert to string
	pText = c2pstr(iRadius);																		// convert to pascal
	theRadius->SetText(pText,true);															// display the value
	
	dismisser = this->PoseModally();															// pose the question
	
	if (dismisser == kOK)																			// ok button
		{
		theRadius->GetText(pRadius);															// get the value
		itsRadius = p2cstr((StringPtr) pRadius);											// convert to a c string
		radius = atof(itsRadius);																	// convert to a number
		((TCFDFrontDocument *) fDocument)->SetRadiusOpts(radius); 			// store the data
		}
	
	aWindow->Close();																				// close the window
	return;
	}
	
//------------------------------------------------------------------------------------------
//	TPremix - GetParams
//------------------------------------------------------------------------------------------
void TPremix::GetParams(void)
	{
	float				breakup, p5, p8, p9;
	short			cmp, index;
	extended		p;
	TWindow 	* aWindow;																			// window for this dialog
	TEditText	*	b, * pr, * pr5, * pr8, *pr9;
	TRadio		* c[2];
	IDType			dismisser;
	Str255			pPran;
	StringPtr		pText;
	char 		*	itsPran, iPran[20];
	
	aWindow = this->GetWindow();																// get the window
	HLock((Handle) this);
	((TCFDFrontDocument *) fDocument)->GetPremixOpts(&breakup,&cmp,&p,&p8,&p9,&p5);// store the data
	HUnlock((Handle) this);

	this->fDefaultItem = 'OKOK';																	// set the default items
	this->fCancelItem = 'CNCL';

	b = (TEditText *) aWindow->FindSubView('PR01');								// get the edit box
	pr = (TEditText *) aWindow->FindSubView('PR05');								// get the edit box
	pr5 = (TEditText *) aWindow->FindSubView('PR04');							// get the edit box
	pr8 = (TEditText *) aWindow->FindSubView('PR02');							// get the edit box
	pr9 = (TEditText *) aWindow->FindSubView('PR03');							// get the edit box
	c[0] = (TRadio *) aWindow->FindSubView('icmp');									// get methane button
	c[1] = (TRadio *) aWindow->FindSubView('cmpr');								// get town gas button
	
	sprintf(iPran,"%f",breakup);																// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	b->SetText(pText,true);																		// display the value
	
	sprintf(iPran,"%f",p8);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr8->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%f",p9);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr9->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%f",p5);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr5->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%e",p);																			// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr->SetText(pText,true);																		// display the value
	
	c[0]->SetState(false,false);																	// turn all off
	c[1]->SetState(false,false);																	// turn all off
	c[cmp]->SetState(true,true);																// turn correct one on

	dismisser = this->PoseModally();															// pose the question
		
	if (dismisser == kOK)																			// ok button
		{
		b->GetText(pPran);																			// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		breakup = atof(itsPran);																	// convert to float
			
		pr8->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p8 = atof(itsPran);																			// convert to float
			
		pr9->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p9 = atof(itsPran);																			// convert to float
			
		pr5->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p5 = atof(itsPran);																			// convert to float
			
		pr->GetText(pPran);																			// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p = atof(itsPran);																				// convert to float
			
		for (index = 0; index < 2; index++)
			{
			if (c[index]->IsOn())
				cmp = index;
			}
			
		((TCFDFrontDocument *) fDocument)->SetPremixOpts(breakup,cmp,p,p8,p9,p5);// store the data
		}
	
	aWindow->Close();																				// close the window
	return;
	}
	
//------------------------------------------------------------------------------------------
//	TDiffusion - GetParams
//------------------------------------------------------------------------------------------
void TDiffusion::GetParams(void)
	{
	float				p8, p10,tA,tF;
	extended		p;
	short			fuel, cmp, index;
	TWindow 	* aWindow;																			// window for this dialog
	TEditText	* pr8, *pr10,  * pr, * tempA, * tempF;
	TRadio		*	f[3], * c[2];
	IDType			dismisser;
	Str255			pPran;
	StringPtr		pText;
	char 		*	itsPran, iPran[20];
	
	aWindow = this->GetWindow();																// get the window
	HLock((Handle) this);
	((TCFDFrontDocument *) fDocument)->GetDiffusionOpts(&fuel,&cmp,&p,&p8,&p10,&tA,&tF);// store the data
	HUnlock((Handle) this);

	this->fDefaultItem = 'OKOK';																	// set the default items
	this->fCancelItem = 'CNCL';

//	get reference to each data object in the dialog box
	f[0] = (TRadio *) aWindow->FindSubView('prop');									// get propane button
	f[1] = (TRadio *) aWindow->FindSubView('meth');									// get methane button
	f[2] = (TRadio *) aWindow->FindSubView('town');									// get town gas button
	c[0] = (TRadio *) aWindow->FindSubView('icmp');									// get methane button
	c[1] = (TRadio *) aWindow->FindSubView('cmpr');								// get town gas button
	pr8 = (TEditText *) aWindow->FindSubView('PR01');							// get the edit box
	pr10 = (TEditText *) aWindow->FindSubView('PR02');							// get the edit box
	tempA = (TEditText *) aWindow->FindSubView('PR04');						// get the edit box
	tempF = (TEditText *) aWindow->FindSubView('PR03');						// get the edit box
	pr = (TEditText *) aWindow->FindSubView('PR05');								// get the edit box
	
//	set the initial values for the data 
	sprintf(iPran,"%f",p8);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr8->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%f",p10);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr10->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%.2f",tA);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	tempA->SetText(pText,true);																// display the value
	
	sprintf(iPran,"%.2f",tF);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	tempF->SetText(pText,true);																// display the value
	
	sprintf(iPran,"%e",p);																			// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr->SetText(pText,true);																		// display the value
	
	f[0]->SetState(false,false);																	// turn all off
	f[1]->SetState(false,false);																	// turn all off
	f[2]->SetState(false,false);																	// turn all off
	f[fuel]->SetState(true,true);																// turn correct one on

	c[0]->SetState(false,false);																	// turn all off
	c[1]->SetState(false,false);																	// turn all off
	c[cmp]->SetState(true,true);																// turn correct one on

//	get the new values	
	dismisser = this->PoseModally();															// pose the question
		
	if (dismisser == kOK)																			// ok button
		{
//	store the new values to the document
		pr8->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p8 = atof(itsPran);																			// convert to float
			
		pr10->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p10 = atof(itsPran);																			// convert to float
			
		tempF->GetText(pPran);																	// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		tF = atof(itsPran);																			// convert to float
			
		tempA->GetText(pPran);																	// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		tA = atof(itsPran);																			// convert to float
			
		pr->GetText(pPran);																			// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p = atof(itsPran);																				// convert to float
			
		for (index = 0; index < 3; index++)
			{
			if (f[index]->IsOn())																		// is it on?
				fuel = index;
			if (index < 2)
				if (c[index]->IsOn())
					cmp = index;
			}
		((TCFDFrontDocument *) fDocument)->SetDiffusionOpts(fuel,cmp,p,p8,p10,tA,tF);// store the data
		}
	
	aWindow->Close();																				// close the window
	return;
	}
	
//------------------------------------------------------------------------------------------
//	TTurbulence - GetParams
//------------------------------------------------------------------------------------------
void TTurbulence::GetParams(void)
	{
	float				p6, p7;
	TWindow 	* aWindow;																			// window for this dialog
	TEditText	* pr6, *pr7;
	IDType			dismisser;
	Str255			pPran;
	StringPtr		pText;
	char 		*	itsPran, iPran[20];
	
	aWindow = this->GetWindow();																// get the window
	HLock((Handle) this);
	((TCFDFrontDocument *) fDocument)->GetTurbulenceOpts(&p6,&p7);	// store the data
	HUnlock((Handle) this);

	this->fDefaultItem = 'OKOK';																	// set the default items
	this->fCancelItem = 'CNCL';

//	get reference to each data object in the dialog box
	pr6 = (TEditText *) aWindow->FindSubView('PR01');							// get the edit box
	pr7 = (TEditText *) aWindow->FindSubView('PR02');							// get the edit box
	
//	set the initial values for the data 
	sprintf(iPran,"%f",p6);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr6->SetText(pText,true);																	// display the value
	
	sprintf(iPran,"%f",p7);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr7->SetText(pText,true);																	// display the value
	
//	get the new values	
	dismisser = this->PoseModally();															// pose the question
		
	if (dismisser == kOK)																			// ok button
		{
//	store the new values to the document
		pr6->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p6 = atof(itsPran);																			// convert to float
			
		pr7->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p7 = atof(itsPran);																			// convert to float

		((TCFDFrontDocument *) fDocument)->SetTurbulenceOpts(p6,p7);		// store the data
		}
	
	aWindow->Close();																				// close the window
	return;
	}
	
//------------------------------------------------------------------------------------------
//	TTurbulence - GetParams
//------------------------------------------------------------------------------------------
void TPressure::GetParams(void)
	{
	float				p5;
	extended		p;
	TWindow 	* aWindow;																			// window for this dialog
	TEditText	* pr, *pr5;
	IDType			dismisser;
	Str255			pPran;
	StringPtr		pText;
	char 		*	itsPran, iPran[20];
	
	aWindow = this->GetWindow();																// get the window
	HLock((Handle) this);
	((TCFDFrontDocument *) fDocument)->GetPressureOpts(&p,&p5);		// store the data
	HUnlock((Handle) this);

	this->fDefaultItem = 'OKOK';																	// set the default items
	this->fCancelItem = 'CNCL';

//	get reference to each data object in the dialog box
	pr = (TEditText *) aWindow->FindSubView('PR01');								// get the edit box
	pr5 = (TEditText *) aWindow->FindSubView('PR02');							// get the edit box
	
//	set the initial values for the data 
	sprintf(iPran,"%E",p);																			// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr->SetText(pText,true);																		// display the value
	
	sprintf(iPran,"%f",p5);																		// convert to string
	pText = c2pstr(iPran);																			// convert to pascal
	pr5->SetText(pText,true);																	// display the value
	
//	get the new values	
	dismisser = this->PoseModally();															// pose the question
		
	if (dismisser == kOK)																			// ok button
		{
//	store the new values to the document	
		pr->GetText(pPran);																			// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p = atof(itsPran);																				// convert to float
			
		pr5->GetText(pPran);																		// get the text
		itsPran = p2cstr((StringPtr) pPran);												// convert to c
		p5 = atof(itsPran);																			// convert to float

		((TCFDFrontDocument *) fDocument)->SetPressureOpts(p,p5);		// store the data
		}
	
	aWindow->Close();																				// close the window
	return;
	}
	
// -----------------------------------------------------------------------------------------
//	get the dimensions of the geometry.
//	throw up the initial MODAL get dimensions dialog box.
// -----------------------------------------------------------------------------------------
Boolean TDimension::GetDimensions (void)
	{
	TWindow * aWindow;																	// debug window
	TEditText * Dimension;
	IDType 		dismisser;

	aWindow = this->GetWindow();														// create the window
	
	// select the x dimension.
	Dimension = (TEditText *) (aWindow->FindSubView ('xdim'));		// find xdim
	Dimension->SetSelection (1, 32000, true);									// set the selection
	
	// get the dimsDialog
	this->fDefaultItem = 'OKOK';															// set the default items
	this->fCancelItem = 'CNCL';
	
	dismisser = this->PoseModally();										// is a modal dialog
	
	if (dismisser == kOK)																	// clicked ok
		{
		char sizeStr[256];
		float width, height;
		
		Dimension->GetText ((StringPtr) sizeStr);								// get the x dimension
		p2cstr ((StringPtr) sizeStr);													// convert to c
		width = atof(sizeStr);
		
		Dimension = (TEditText*) (aWindow->FindSubView ('ydim'));	// get the y sizeStr
		Dimension->GetText ((StringPtr) sizeStr);								// get the text
		p2cstr ((StringPtr) sizeStr);													// convert to c
		height = atof(sizeStr);
		((TCFDFrontDocument *) fDocument)->SetDimensions(width,height); // store info in document
		
		aWindow->Close();																	// close the window
		return true;																				// return true
		}
	else
		{
		aWindow->Close();																	// close the window
		return false;																				// return false
		}
	}
		
// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
Boolean TImageDialog::GetImage (char * string)
	{
	TWindow 	* aWindow;								
	TEditText * app;
	IDType 		dismisser;
	char			test[30];
	StringPtr		pText;

	aWindow = this->GetWindow();																	// create the window

	// select the x dimension.
	app = (TEditText *) (aWindow->FindSubView ('name'));					// find appl

	if (strlen(string) == 0)
		sprintf(test,"Result.Image");																// convert to string
	else
		strcpy(test,string);
	pText = c2pstr(test);																					// convert to pascal
	app->SetText (pText, TRUE);																		// set the selection
	
	// get the dimsDialog
	this->fDefaultItem 	= 'OKOK';																	// set the default items
	this->fCancelItem 	= 'CNCL';
	
	dismisser = this->PoseModally();															// is a modal dialog
	
	if (dismisser == kOK)																					// clicked ok
		{
		char text[256];
		
		app->GetText ((StringPtr) text);														// get the application
		p2cstr ((StringPtr) text);																	// convert to c
		
		((TCFDFrontDocument *) fDocument)->SetImageName(text); 			// store info in document
		
		aWindow->Close();																						// close the window
		return true;																								// return true
		}
	else
		{
		aWindow->Close();																						// close the window
		return false;																								// return false
		}
	}
		
// -----------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------
Boolean TAppDialog::GetApp (char * string)
	{
	TWindow 	* aWindow;								
	TEditText * app;
	IDType 		dismisser;
	char			test[30];
	StringPtr		pText;

	aWindow = this->GetWindow();																	// create the window

	// select the x dimension.
	app = (TEditText *) (aWindow->FindSubView ('appl'));					// find appl

	if (strlen(string) == 0)
		sprintf(test,"tryme");																			// convert to string
	else
		strcpy(test,string);
	pText = c2pstr(test);																					// convert to pascal
	app->SetText (pText, TRUE);																		// set the selection
	
	// get the dimsDialog
	this->fDefaultItem 	= 'OKOK';																	// set the default items
	this->fCancelItem 	= 'CNCL';
	
	dismisser = this->PoseModally();															// is a modal dialog
	
	if (dismisser == kOK)																					// clicked ok
		{
		char text[256];
		
		app->GetText ((StringPtr) text);														// get the application
		p2cstr ((StringPtr) text);																	// convert to c
		
		((TCFDFrontDocument *) fDocument)->SetApp(text); 						// store info in document
		
		aWindow->Close();																						// close the window
		return true;																								// return true
		}
	else
		{
		aWindow->Close();																						// close the window
		return false;																								// return false
		}
	}
		
// -----------------------------------------------------------------------------------------
//	TWarnDelete
//	Warn the user they are about to delete something
// -----------------------------------------------------------------------------------------
Boolean TWarnDelete::ShowDelete (char * message)
	{
	TWindow * aWindow;																	// debug window
	TEditText * tText;
	IDType 		dismisser;
	StringPtr		pText;

	aWindow = this->GetWindow();														// create the window

	tText = (TEditText *) (aWindow->FindSubView ('dmsg'));				// find text box
	pText = c2pstr(message);																// convert to pascal
	tText->SetText(pText,true);														// display the value
	
	// get the dimsDialog
	this->fDefaultItem = 'OKOK';															// set the default items
	this->fCancelItem = 'CNCL';
	
	dismisser = this->PoseModally();													// is a modal dialog
	
	if (dismisser == kOK)																	// clicked ok
		{		
		aWindow->Close();																	// close the window
		return true;																				// return true
		}
	else
		{
		aWindow->Close();																	// close the window
		return false;																				// return false
		}
	}
		
