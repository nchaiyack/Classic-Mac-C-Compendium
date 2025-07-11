#pragma segment Matrix
//**********************************************************************
//	TNewSegmentCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	TObstacleCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Initialize the TObstacleCommand.
// -------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::IObstacleCommand (TGeomView * itsGeometry, TCFDFrontDocument * itsDocument, PointInfo * info)
	{
	fGeom		 	= itsGeometry;																		// save geometry
	fCDocument 	= itsDocument;																		// save document
	fInfo.oPt		= info->oPt;
	fInfo.above	= info->above;
	fInfo.left		= info->left;
	fInfo.below	= info->below;
	ICommand (cMakeObstacle,fCDocument, fGeom, fGeom->GetScroller(true));
	fConstrainsMouse = true;
	}

// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::DoIt (void)
	{
	fObstacle = new TObstacle;
	if (fObstacle == NULL)
		{
		SysBeep (3);
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fCDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(6,"Obstacle");
		tWarning->ShowWarning();
		return;
		}
		
	HLock((Handle) this);
	fObstacle->IObstacle(&fInfo);																	// initialize obstacle
	HUnlock((Handle) this);
	fGeom->Focus();
	fObstacle->Draw();																					// draw the obstacle
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::UndoIt (void)
	{
	fGeom->Focus();
	fObstacle->ReleaseObstacle();
	fObstacle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::RedoIt (void)
	{
	fGeom->Focus();
	fObstacle->SetObstacle();
	fObstacle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Commit ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * /*nextPoint*/, 
	Boolean /*turnItOn*/, Boolean /*mouseDidMove*/)
	{
	fGeom->lastCmd = 1;
	return;																									// do nothing
	}

// --------------------------------------------------------------------------------------------------
// 	Commit ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::Commit (void)
	{
	fObstacle = NULL;																					// don't want to free the object
	}

// --------------------------------------------------------------------------------------------------
// 	Free ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TObstacleCommand::Free (void)
	{
	if (!fCmdDone)
		FreeIfObject(fObstacle);
	inherited::Free();
	}

//**********************************************************************
//	TDeleteObstacleCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	TDeleteObstacleCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Initialize the TDeleteObstacleCommand.
// -------------------------------------------------------------------------------------------------
Boolean TDeleteObstacleCommand::IDeleteObstacleCommand (TGeomView * itsGeometry, TCFDFrontDocument * itsDocument, PointInfo * info)
	{
	fGeom		 	= itsGeometry;																		// save geometry
	fCDocument 	= itsDocument;																		// save document
	fInfo.oPt		= info->oPt;
	fInfo.above	= info->above;
	fInfo.left		= info->left;
	fInfo.below	= info->below;

	fObstacle = (TObstacle *) fInfo.oPt->fObsLwRight;
	SysBeep (3);

	Boolean			dismiss;
	TWindow		* aWindow;
	TWarnDelete	* dWarn;
	char				message[80];
	
	sprintf(message,"Delete Obstacle?");
	aWindow = NewTemplateWindow(kWarnDelete, fCDocument);
	if (aWindow == NULL)
		return false;
	dWarn = (TWarnDelete *) aWindow->FindSubView('delt');
	dismiss = dWarn->ShowDelete(message);
	if (!dismiss)
		return false;
		
	ICommand (cMakeObstacle,fCDocument, fGeom, fGeom->GetScroller(true));
	fConstrainsMouse = true;
	return true;
	}
// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::DoIt (void)
	{
	StringPtr pString;
	char			string[80];
	
	sprintf(string,"Undo Delete Obstacle");
	pString = c2pstr(string);
	SetCmdName(cUndo,pString);

	fGeom->Focus();
	fObstacle->ReleaseObstacle();
	fObstacle->Draw();																	// draw the obstacle
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::UndoIt (void)
	{
	StringPtr pString;
	char			string[80];
	
	sprintf(string,"Redo Delete Obstacle");
	pString = c2pstr(string);
	SetCmdName(cUndo,pString);

	fGeom->Focus();
	fObstacle->SetObstacle();
	fObstacle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::RedoIt (void)
	{
	StringPtr pString;
	char			string[80];
	
	sprintf(string,"Undo Delete Obstacle");
	pString = c2pstr(string);
	SetCmdName(cUndo,pString);

	fGeom->Focus();
	fObstacle->ReleaseObstacle();
	fObstacle->Draw();
	}

// --------------------------------------------------------------------------------------------------
// 	Commit ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * /*nextPoint*/, 
	Boolean /*turnItOn*/, Boolean /*mouseDidMove*/)
	{
	fGeom->lastCmd = 1;
	return;																									// do nothing
	}

// --------------------------------------------------------------------------------------------------
// 	Commit ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::Commit (void)
	{
	FreeIfObject(fObstacle);
	}

// --------------------------------------------------------------------------------------------------
// 	Free ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteObstacleCommand::Free (void)
	{
	inherited::Free();
	}

