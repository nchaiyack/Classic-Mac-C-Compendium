#pragma segment CObj
//**********************************************************************
//	TNewSectionCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	Initialize the NewSegmentCommand.
// -------------------------------------------------------------------------------------------------
void TNewSectionCommand::INewSectionCommand (TGeomView * itsGeometry, TCFDFrontDocument * tDoc)
	{
	fGeomView	= itsGeometry;																				// store the geometry
	fCDocument	= tDoc;																							// get the document
	fPointMatrix	=	fCDocument->fPointMatrix;														// store pointer to matrix object

	fConstrainsMouse = true;																					// constrain the mouse
	fGeomView->lastCmd = 0;																					// this is a newSegment
	saveDeleted	= NULL;
	fPoint			= NULL;
	inherited::ICommand (cNewSecCommand,															// init the command
					fCDocument, fGeomView, fGeomView->GetScroller(true));
	}

// --------------------------------------------------------------------------------------------------
// 	Create the point
// --------------------------------------------------------------------------------------------------
TPoint * TNewSectionCommand::CreatePoint (TPoint * tPt, Point * mMouse)
	{
	TGPoint * sPt;
	short		type;
	
	sPt = new TGPoint;																								// allocate the memory
	if (sPt == NULL)																									// could not add a point
		return NULL;
	
	type = tPt->GetSectionType();
	fSide->SnaptoSection(mMouse);																			// snap the point to line
	sPt->IPoint(mMouse->v,mMouse->h,type,fGeomView->fMagnify);							// initialize the point
	return (TPoint *) sPt;
	}
	
//**********************************************************************
//	TNewSegmentCommand - Methods
//**********************************************************************
// -------------------------------------------------------------------------------------------------
//	NewSegmentCommand Methods.
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
//	Initialize the NewSegmentCommand.
// -------------------------------------------------------------------------------------------------
void TNewSegmentCommand::INewSegmentCommand (TGeomView * itsGeometry, TCFDFrontDocument * tDoc)
	{
	fGeomView	= itsGeometry;																				// store the geometry
	fCDocument	= tDoc;																							// get the document
	fPointMatrix	=	fCDocument->fPointMatrix;														// store pointer to matrix object

	fConstrainsMouse = true;																					// constrain the mouse
	fGeomView->lastCmd = 0;																					// this is a newSegment
	saveDeleted	= NULL;
	fPoint			= NULL;
	inherited::ICommand (cNewSegCommand,															// init the command
					fCDocument, fGeomView, fGeomView->GetScroller(true));
	}

// -------------------------------------------------------------------------------------------------
//	tracking feedback. limit the mouse motion to the geometry not including the border.
// -------------------------------------------------------------------------------------------------
pascal void TNewSegmentCommand::TrackConstrain (VPoint * /*ancorPoint*/, 
			VPoint * /*previousPoint*/, VPoint * nextPoint)	
	{
	if (nextPoint->v <= cGeomViewBorder)															// limit the verticle dimension
		nextPoint->v = cGeomViewBorder + 1;
	else
		if (nextPoint->v >= fGeomView->fSize.v - cGeomViewBorder)
			nextPoint->v = fGeomView->fSize.v - cGeomViewBorder;

	if (nextPoint->h <= cGeomViewBorder)															// limit the horizontal dimension
		nextPoint->h = cGeomViewBorder + 1;
	else
		if (nextPoint->h >= fGeomView->fSize.h - cGeomViewBorder)
			nextPoint->h = fGeomView->fSize.h - cGeomViewBorder;
}

// -------------------------------------------------------------------------------------------------
//	tracking feedback.
// -------------------------------------------------------------------------------------------------
pascal TCommand * TNewSegmentCommand::TrackMouse (TrackPhase aTrackPhase, 
					VPoint * /*ancorPoint*/, VPoint * /*previousPoint*/, VPoint * nextPoint, 
					Boolean mouseDidMove)
	{
	Point tmp;
	tmp = VPtToPt(nextPoint);																				// save nextPoint
	
	if (mouseDidMove)
		{		
		Point tmp1;
		tmp1 = AntiTransform(tmp,fGeomView->fMagnify);										// convert nextPoint to viewpoint																
		((TInformationView *) fCDocument->fInfoView)->StatusString(tmp1);			// display the status string

		if (aTrackPhase == trackRelease)																	// if mouse is released
			fLocation = AntiTransform(tmp,fGeomView->fMagnify);
		}
	return (TCommand *) this;
	}

// --------------------------------------------------------------------------------------------------
//	Do it.
// --------------------------------------------------------------------------------------------------
pascal void TNewSegmentCommand::DoIt (void)	
	{
	PointInfo	sInfo, oInfo;																							// draw structures
	Point 		mMouse;
	TPoint 		* tPt;																									// current point
	TGPoint	*	oPt;																									// point on opposite boundry
	short		theSide;
	short		temp;
	
	done			=	false;
	mMouse 	= 	fLocation;
	HLock((Handle) this);
	if ((tPt = (TPoint *) fPointMatrix->FindSection(&mMouse)) == NULL)					// on a boundry?
		{
		HUnlock((Handle) this);
		return;
		}
	HUnlock((Handle) this);
		
	fSide = (TBoundry *) fPointMatrix->GetCurrentBoundry();									// what boundry?
	theSide = fSide->GetSide();																					// which side

	if ((temp = fPointMatrix->ObsBaffInLine(fSide)) != 0)
		{
		char	msg[80];
		
		if (temp == 1)
			sprintf(msg,"Obstacle");
		else
			sprintf(msg,"Baffle");
		SysBeep (3);
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fCDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(8,msg);
		tWarning->ShowWarning();
		return;
		}
		
	HLock((Handle) this);
	fPoint = this->CreatePoint(tPt,&mMouse);	
	HUnlock((Handle) this);
	if (fPoint == NULL)
		return;
		
	fPoint->RememberPoint();	
	HLock((Handle) this);
	if ((oPt = fPointMatrix->GetOpposite(&mMouse,&sInfo,&oInfo)) == NULL)				// create opposite point
		{
		HUnlock((Handle) this);
		delete fPoint;
		return;
		}
	
	HUnlock((Handle) this);
	done = true;
	fGeomView->Focus();
	TPoint * save1, * save2;
	save1 = sInfo.left;
	save2 = oInfo.left;
	
	fRow 				=	sInfo.row			= fPointMatrix->GetRow();								// set the row
	fColumn			=	sInfo.column		= fPointMatrix->GetColumn();							// set the column
	sInfo.gridOnly	= oInfo.gridOnly 	= false;
	sInfo.above		= oInfo.above 		= NULL;
	sInfo.below		= sInfo.below 		= NULL;
	sInfo.magnify	= oInfo.magnify		= fGeomView->fMagnify;
	sInfo.right		= tPt;
	
	HLock((Handle) this);
	this->Draw(sInfo.right, oInfo.right, theSide, &sInfo, &oInfo);
	
	if (!fPointMatrix->AddAt(fPoint,oPt,sInfo.row,sInfo.column))								// add it
		{
		delete fPoint;																									// can't add it
		delete oPt;
		this->Draw(sInfo.right,oInfo.right,theSide, &sInfo, &oInfo);
		return;
		}
	
	this->Draw(fPoint,oPt,theSide, &sInfo, &oInfo);
	
	if (fPointMatrix->IsShown() && (theSide == cLeft || theSide == cRight))				// do a grid line?
		fPointMatrix->OneRowDo(DrawAllPoints,&sInfo);
	else if (fPointMatrix->IsShown())
		fPointMatrix->OneColumnDo(DrawPoint,&sInfo);
	else
		{
		DrawPoint(fPoint,&sInfo);
		DrawPoint(oPt,&oInfo);
		}
	
	HUnlock((Handle) this);

	sInfo.row += 1;
	sInfo.column += 1;
	sInfo.left = fPoint;
	oInfo.left = oPt;
	this->Draw(sInfo.right,oInfo.right,theSide, &sInfo, &oInfo);
	
	sInfo.left = save1;
	oInfo.left = save2;
	
	if (fGeomView->fSPoint != NULL)
		fPointMatrix->DoHighlight();																				// unhighlight selection
		
//	select the new point
	fGeomView->UnSelect();																						// unselect old point
	fGeomView->Select(fPoint,false,fSide);																// select new point
	fPointMatrix->DoHighlight();																					// highlight point
		
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();						// show number of segments
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string return;
	}

// --------------------------------------------------------------------------------------------------
// 	Create the point
// --------------------------------------------------------------------------------------------------
TPoint * TNewSegmentCommand::CreatePoint (TPoint * tPt, Point * mMouse)
	{
	TSegPoint *  sPt, * sPt1;
	WallRecord 	i;
	short 			type;
	
	sPt = new TSegPoint;																							// allocate the memory
	if (sPt == NULL)																									// could not add a point
		return NULL;
	
	type = tPt->GetSectionType();																				// get the type
	fSide->SnaptoSection(mMouse);																			// snap the point to line
	sPt->IPoint(mMouse->v,mMouse->h,type,fGeomView->fMagnify);							// initialize the point
	
	sPt1 = fPointMatrix->FindSegment(tPt,fSide);														// find the old segment
	i = sPt1->GetData();																								// get old data
	sPt->SetData(i);																									// set the new the same
	
	return (TPoint *) sPt;
	}
	
// --------------------------------------------------------------------------------------------------
// 	Draw the new segment/section/grid line
// --------------------------------------------------------------------------------------------------
void TNewSegmentCommand::Draw (TPoint * sPt, TPoint * oPt, short theSide, PointInfo * sInfo, PointInfo * oInfo)
	{
	HLock((Handle) this);
	if (fPointMatrix->IsShown() && (theSide == cLeft || theSide == cRight))				// do a grid line?
		fPointMatrix->OneRowDo(DrawAllLines,sInfo);
	else if (fPointMatrix->IsShown())
		fPointMatrix->OneColumnDo(DrawLine,sInfo);
	else
		{
		sInfo->noGrid = fPointMatrix->IsShown();
		oInfo->noGrid = sInfo->noGrid;
		DrawLine(sPt,sInfo);
		DrawLine(oPt,oInfo);
		}
	HUnlock((Handle) this);
	}
	
// --------------------------------------------------------------------------------------------------
// 	Undo it.
// --------------------------------------------------------------------------------------------------
pascal void TNewSegmentCommand::UndoIt (void)
	{
	fGeomView->Focus();
	if (!done)
		return;
		
	if (fGeomView->fSPoint != NULL)
		fPointMatrix->DoHighlight();																				// unhighlight selection
		
	fGeomView->UnSelect();																						// unselect old point

	fPointMatrix->SetCurrent(fPoint);
	saveDeleted = fPointMatrix->DeleteCurrent(fSide);
	fPoint = NULL;
	
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();						// show number of segments
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Redo it.
// --------------------------------------------------------------------------------------------------
pascal void TNewSegmentCommand::RedoIt (void)
	{
	fGeomView->Focus();
	if (!done)
		return;
	
	fPoint = fPointMatrix->AddLine(saveDeleted,fSide,fRow,fColumn);						// add back in
	fPointMatrix->SetCurrent(fPoint);
	
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();						// show number of segments
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string return;
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Free NewSegment
// --------------------------------------------------------------------------------------------------
pascal void TNewSegmentCommand::Free (void)
	{
	if (!fCmdDone)																											// undo the add
		{																															// so free all the new objects
		short x, side;
		TPoint * t;
		
		side = fSide->GetSide();
		for (x = (short) saveDeleted->fSize; x > 0; x--)
			{
			t = (TPoint *) saveDeleted->At(x);
			saveDeleted->Delete(t);
			FreeIfObject(t);
			}
		if (side == cLeft || side == cRight)
			FreeIfObject(saveDeleted);
		}
	inherited::Free();
	}