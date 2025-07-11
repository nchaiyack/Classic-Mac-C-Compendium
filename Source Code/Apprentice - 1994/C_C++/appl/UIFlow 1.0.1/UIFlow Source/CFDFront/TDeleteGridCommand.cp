#pragma segment Matrix
//**********************************************************************
//	TDeleteGridCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	TDeleteGridCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Initialize the TDeleteGridCommand.
// -------------------------------------------------------------------------------------------------
Boolean TDeleteGridCommand::IDeleteGridCommand (TGeomView * itsGeometry, TPoint * tPt)
	{
	TPoint	*	oPt;
	short		temp;
	
	fGeom		 	= itsGeometry;																				// save geometry
	fCDocument 	= (TCFDFrontDocument *) fGeom->fCDocument;							// save document
	fPoint			= tPt;
	fSide 			= (TBoundry *) ((TPointMatrix *) fCDocument->fPointMatrix)->GetCurrentBoundry();
	
	HLock((Handle) this);
	(TPointMatrix *) (fCDocument->fPointMatrix)->SetCornors(fPoint, &fInfo);		// find the surrounding points
	HUnlock((Handle) this);
	fPoint->RememberPoint();
	fRow			= ((TPointMatrix *) fCDocument->fPointMatrix)->GetRow();		// get the current row
	fColumn		= ((TPointMatrix *) fCDocument->fPointMatrix)->GetColumn();	// get the current column
	oPt				= ((TPointMatrix *) fCDocument->fPointMatrix)->GetOpposite(fSide, fRow, fColumn);	// get point on opposite boundry	
	
	SysBeep (3);
	if (oPt->IsSegment())
		{
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fCDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(7,NULL);
		tWarning->ShowWarning();
		return false;
		}
	else if ((temp = ((TPointMatrix *) fCDocument->fPointMatrix)->ObsBaffInLine(fSide)) != 0)
		{
		char	msg[80];
		
		if (temp == 1)
			sprintf(msg,"Obstacle");
		else
			sprintf(msg,"Baffle");
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fCDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(9,msg);
		tWarning->ShowWarning();
		return false;
		}
		
	Boolean			dismiss;
	TWindow		* aWindow;
	TWarnDelete	* dWarn;
	char				message[80];
	
	sprintf(message,"Delete Grid Line?");
	aWindow = NewTemplateWindow(kWarnDelete, fCDocument);
	if (aWindow == NULL)
		return false;
	dWarn = (TWarnDelete *) aWindow->FindSubView('delt');
	dismiss = dWarn->ShowDelete(message);
	if (!dismiss)
		return false;
		
	ICommand (cDeleteSecCommand,fCDocument, fGeom, fGeom->GetScroller(true));
	fConstrainsMouse = true;
	return true;
	}
// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteGridCommand::DoIt (void)
	{
	fGeom->Focus();
	((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);
	saveDeleted = ((TPointMatrix *) fCDocument->fPointMatrix)->DeleteCurrent(fSide);
	fPoint = NULL;
	
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteGridCommand::UndoIt (void)
	{
	fGeom->Focus();
	fPoint = ((TPointMatrix *) fCDocument->fPointMatrix)->AddLine(saveDeleted,fSide,fRow,fColumn);	
	((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TDeleteGridCommand::RedoIt (void)
	{
	this->DoIt();
	}

// --------------------------------------------------------------------------------------------------
// 	TrackMouse :	Do Nothing
// --------------------------------------------------------------------------------------------------
pascal TCommand * TDeleteGridCommand::TrackMouse (TrackPhase thePhase, VPoint * /*anchorPoint*/,
	VPoint * /*previousPoint*/, VPoint * /*nextPoint*/, Boolean /*mouseDidMove*/)
	{
	if (!fGeom->IsDragging())																		// starting the delete procedure?
		{																										// show trash cans
		fGeom->lastCmd = 1;
		fGeom->SetDragging(true);																// turn dragging on
		if (fGeom->fSPoint != NULL)																// is there a selected point?
			{
			((TPointMatrix *) fCDocument->fPointMatrix)->DoHighlight();		// unhighlight
			fGeom->UnSelect();																		// turn off selection
			}
		}
	if (thePhase == trackRelease)
		fGeom->SetDragging(false);
	return this;
	}
	
// --------------------------------------------------------------------------------------------------
//	Give user feedback on the mouse drag
// --------------------------------------------------------------------------------------------------
pascal void TDeleteGridCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * /*nextPoint*/, 
	Boolean /*turnItOn*/, Boolean /*mouseDidMove*/)
	{
	return;
	}
	
// --------------------------------------------------------------------------------------------------
// 	Commit ObstacleCommand
// --------------------------------------------------------------------------------------------------
pascal void TDeleteGridCommand::Commit (void)
	{
	short x, side;
	TPoint * t;
	
	side = fSide->GetSide();
	for (x = (short) saveDeleted->fSize; x > 0; x--)
		{
		t = (TPoint *) saveDeleted->At(x);
		saveDeleted->Delete(t);
		FreeIfObject(t);
		}

	if (side == cLeft || side == cRight)																// if on right or left delete the list
		FreeIfObject(saveDeleted);
	}

