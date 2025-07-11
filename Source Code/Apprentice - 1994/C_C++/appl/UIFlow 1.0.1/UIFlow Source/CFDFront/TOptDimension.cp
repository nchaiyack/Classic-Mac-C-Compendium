#pragma segment MenuOption
//*********************************************************************************
//	TOptDimension - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//------------------------------------------------------------------------------------------
//	TOptDimension IOptDimension Method... Implementation
//		initialize the dialog window......
//------------------------------------------------------------------------------------------
pascal void TOptDimension::IOptDimension(const short dialogID)
	{
	if (qdbug == 1)
		{
		char x[255];
		sprintf(x,"Intializing TOptDimension");
		StringPtr dText = c2pstr(x);
	
		TEditText *buggy = (TEditText *)dbWindow->FindSubView('dbug');
		buggy->SetText(dText,kRedraw);
		}
		
	aWindow = NewTemplateWindow(dialogID, NULL);
	aWindow->Open();
	}

//------------------------------------------------------------------------------------------
//	TOptDimension DoChoice Method... Implementation
//------------------------------------------------------------------------------------------
pascal void TOptDimension::DoChoice(TView */*origView*/, short /*itsChoice*/)
	{
	if (qdbug == 1)
		{
		char x[255];
	
		sprintf(x,"TOptDimension::DoChoice");
		StringPtr dText = c2pstr(x);
		TEditText *buggy = (TEditText *)dbWindow->FindSubView('dbug');
		buggy->SetText(dText,kRedraw);
		}
		
//	inherited::DoChoice(origView,itsChoice);
	}
	
