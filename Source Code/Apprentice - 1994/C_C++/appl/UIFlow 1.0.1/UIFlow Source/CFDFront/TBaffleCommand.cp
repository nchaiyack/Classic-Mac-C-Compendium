#pragma segment Matrix
// **********************************************************************
//	TBaffleCommand - Methods
// **********************************************************************
// -------------------------------------------------------------------------------------------------
//	TBaffleCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Initialize the TBaffleCommand.
// -------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::IBaffleCommand (TGeomView * itsGeometry, TCFDFrontDocument * itsDocument, 
	PointInfo * info, short tDirection)
	{
	fGeom		 	= itsGeometry;																		// save geometry
	fCDocument 	= itsDocument;																		// save document
	fDirection		= tDirection;
	fInfo.above	= info->above;
	fInfo.right		= info->right;
	fInfo.below	= info->below;
	fInfo.left		= info->left;
	ICommand (cMakeBaffle,fCDocument, fGeom, fGeom->GetScroller(true));
	fConstrainsMouse = true;
	}

// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::DoIt (void)
	{
	fBaffle = new TBaffle;
	if (fBaffle == NULL)
		{
		SysBeep (3);
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fCDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(6,"Baffle");
		tWarning->ShowWarning();
		return;
		}
		
	HLock((Handle) this);
	fBaffle->IBaffle(&fInfo,fDirection);														// initialize Baffle
	HUnlock((Handle) this);
	fGeom->Focus();
	fBaffle->Draw();																					// draw the Baffle
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::UndoIt (void)
	{
	fGeom->Focus();
	fBaffle->BaffleRelease();
	fBaffle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::RedoIt (void)
	{
	fGeom->Focus();
	fBaffle->BaffleSet();
	fBaffle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Commit BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * /*nextPoint*/, 
	Boolean /*turnItOn*/, Boolean /*mouseDidMove*/)
	{
	fGeom->lastCmd = 1;
	return;																									// do nothing
	}

// --------------------------------------------------------------------------------------------------
// 	Commit BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::Commit (void)
	{
	fBaffle = NULL;																						// don't want to free the object
	}

// --------------------------------------------------------------------------------------------------
// 	Free BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TBaffleCommand::Free (void)
	{
	if (!fCmdDone)
		FreeIfObject(fBaffle);
	inherited::Free();
	}

//**********************************************************************
//	TDeleteBaffleCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	TDeleteBaffleCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Initialize the TDeleteBaffleCommand.
// -------------------------------------------------------------------------------------------------
Boolean TDeleteBaffleCommand::IDeleteBaffleCommand (TGeomView * itsGeometry, TCFDFrontDocument * itsDocument, 
	PointInfo * info, short tDirection)
	{
	fGeom		 	= itsGeometry;																		// save geometry
	fCDocument 	= itsDocument;																		// save document
	fInfo.right		= info->right;
	fInfo.above	= info->above;
	fInfo.left		= info->left;
	fInfo.below	= info->below;
	fDirection		= tDirection;
	
	if (fDirection == 1)
		fBaffle = (TBaffle *) fInfo.above->fBaffBelow;
	else
		fBaffle = (TBaffle *) fInfo.left->fBaffRight;
	SysBeep (3);

	Boolean			dismiss;
	TWindow		* aWindow;
	TWarnDelete	* dWarn;
	char				message[80];
	 
	sprintf(message,"Delete Baffle?");
	aWindow = NewTemplateWindow(kWarnDelete, fCDocument);
	if (aWindow == NULL)
		return false;
	dWarn = (TWarnDelete *) aWindow->FindSubView('delt');
	dismiss = dWarn->ShowDelete(message);
	if (!dismiss)
		return false;
		
	ICommand (cDeleteBaffle,fCDocument, fGeom, fGeom->GetScroller(true));
	fConstrainsMouse = true;
	return true;
	}
// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::DoIt (void)
	{
	fGeom->Focus();
	fBaffle->Draw();																				// draw the Baffle
	fBaffle->BaffleRelease();
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::UndoIt (void)
	{
	fGeom->Focus();
	fBaffle->Draw();
	fBaffle->BaffleSet();
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::RedoIt (void)
	{
	fGeom->Focus();
	fBaffle->Draw();
	fBaffle->BaffleRelease();
	}

// --------------------------------------------------------------------------------------------------
// 	Commit BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * /*nextPoint*/, 
	Boolean /*turnItOn*/, Boolean /*mouseDidMove*/)
	{
	fGeom->lastCmd = 1;
	return;																									// do nothing
	}

// --------------------------------------------------------------------------------------------------
// 	Commit BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::Commit (void)
	{
	FreeIfObject(fBaffle);
	}

// --------------------------------------------------------------------------------------------------
// 	Free BaffleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteBaffleCommand::Free (void)
	{
	inherited::Free();
	}

