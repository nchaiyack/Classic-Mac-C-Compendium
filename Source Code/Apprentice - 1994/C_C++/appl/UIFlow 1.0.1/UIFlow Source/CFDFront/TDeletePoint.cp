#pragma segment CObj
// **********************************************************************
//	TDeletePointCommand - Methods
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Initializing the dragger command .....
// --------------------------------------------------------------------------------------------------
Boolean TDeletePointCommand::IDeletePointCommand(TGeomView * itsGeometry, TPoint * tPt)
	{
	short		temp;																							// how to center the trash cans
	Point		prev, next;																					// actual locations
	short		maxSize;

	fGeom			=	itsGeometry;																		// save the geometry
	fCDocument	=	(TCFDFrontDocument *)fGeom->fCDocument;						// get the document
	fPoint			=	tPt;
	fSide			=	(TBoundry *) ((TPointMatrix *) fCDocument->fPointMatrix)->GetCurrentBoundry();	// which boundry are we in?
	HLock((Handle) this);
	(TPointMatrix *) (fCDocument->fPointMatrix)->SetCornors(fPoint, &fInfo);
	HUnlock((Handle) this);
	fPoint->RememberPoint();
	fRow			= ((TPointMatrix *) fCDocument->fPointMatrix)->GetRow();
	fColumn		= ((TPointMatrix *) fCDocument->fPointMatrix)->GetColumn();
	
	if ((temp = ((TPointMatrix *) fCDocument->fPointMatrix)->ObsBaffInLine(fSide)) != 0)
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
		
	fPrev	= fSide->GetAdjacentSegment(true, fPoint);										// get previous segment
	fNext	= fSide->GetAdjacentSegment(false, fPoint);									// get previous segment
	prev		= fPrev->fTrans;																				// get location of next point
	next		= fNext->fTrans;																				// get location of last point
		
	if (fInfo.boundry == cTop || fInfo.boundry == cBottom)
		{
		lCent = 1;																									// for centering the trash can
		nCent = 3;
		temp = next.h - prev.h;
		if (temp < 10)
			{
			prev.h 	-= abs(temp);
			next.h += abs(temp);
			}
		}
	else																												// right or left boundry
		{
		lCent = 2;
		nCent = 0;
		temp = next.v - prev.v;
		if (temp < 10)
			{
			prev.v 	-= abs(temp);
			next.v += abs(temp);
			}
		}
	
	if (fGeom->fMagnify == 0)
		maxSize	= (short) (cGeomViewBorder+cMaxSize);
	else
		maxSize	= (short) ((cGeomViewBorder+cMaxSize) * fGeom->fMagnify);
	trashL		= new TCTrash;																		// left trash can closed
	trashLO	= new TCTrash;																		// left trash can open
	trashL->ICTrash(prev,131,26,16,maxSize,lCent);
	trashLO->ICTrash(prev,133,26,16,maxSize,lCent);
	
	trashR = new TCTrash;																				// right trash can closed
	trashRO = new TCTrash;																			// right trash can open
	trashR->ICTrash(next,131,26,16,maxSize,nCent);
	trashRO->ICTrash(next,133,26,16,maxSize,nCent);
	
	fGeom->SetDragging(false);																		// init dragging to false
	ICommand (cDeleteSegCommand,fCDocument,fGeom,fGeom->GetScroller(true));
	fConstrainsMouse = true;																			// constrain the mouse
	rightOn = leftOn = false;
	saveDeleted = NULL;
	return true;
	}

// --------------------------------------------------------------------------------------------------
//	Display the Lines
// --------------------------------------------------------------------------------------------------
void TDeletePointCommand::DisplayIt(Point trans, Point temp)
	{
	PenMode(srcXor);																					// pen mode to xor
	
//	Erase Boundry points and segment lines		
	fPoint->Draw();																					// erase point
	if (fInfo.above != NULL)																			// is there a grid?
		fInfo.above->DrawGrid(fPoint->fTrans);											// erase the grid lines
		
	fPoint->DrawSection(fInfo.left->fTrans);												// erase the left segment
	fInfo.right->DrawSection(fPoint->fTrans);											// erase the right segment

	if (trashLO->InTheCan(trans))																// over left can
		{
		leftOn = true;																					// set left open
		trashLO->DrawCan();																		// draw open can
		if (rightOn)																						// was right open?
			{	
			trashR->DrawCan();																		// close right
			rightOn = false;																			// set right closed
			}
		}
	else if (trashRO->InTheCan(trans))														// over right can?
		{
		rightOn = true;																					// set right open
		trashRO->DrawCan();																		// draw right open
		if (leftOn)																							// was left open?
			{
			trashL->DrawCan();																		// close left
			leftOn = false;																				// set left closed
			}
		}
	else if (leftOn)																						// not in either can but left open
		{
		leftOn = false;																					// set left closed
		trashL->DrawCan();																			// draw left closed
		}
	else if (rightOn)																					// right open?
		{
		rightOn = false;																				// set right closed
		trashR->DrawCan();																			// draw right closed
		}

	fPoint->SetStart(temp.v,temp.h);															// set fStart Point to new point

//	Redraw boundry points and segment lines
	fPoint->DrawSection(fInfo.left->fTrans);												// draw the left segment
	fInfo.right->DrawSection(fPoint->fTrans);											// draw the right segment

	if (fInfo.above != NULL)																			// is there a grid?
		fInfo.above->DrawGrid(fPoint->fTrans);											// draw the grid lines

	fPoint->Draw();																					// draw point
	}

// --------------------------------------------------------------------------------------------------
//	Do the command
// --------------------------------------------------------------------------------------------------
pascal void TDeletePointCommand::DoIt(void)
	{
	Point 	temp;
	Rect 	can;
	WallRecord nData;
	 
	if (trashLO->InTheCan(fPoint->fTrans))													// over left can
		{
		delRightSeg	= 1;																				// deleting left segment
		saveData		= ((TSegPoint *) fPoint)->GetData();							// get data for this segment
		saveType		= fPoint->GetSectionType();											// keep the section type
		if (saveType != fNext->GetSectionType())											// are segment types same?
			{																									// no reset : redraw
			fGeom->Select(fPoint,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fPoint,fSide,fNext->GetSectionType());
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);	
		saveDeleted = ((TPointMatrix *) fCDocument->fPointMatrix)->DeleteCurrent(fSide);
		fPoint = NULL;
		}
	else if (trashRO->InTheCan(fPoint->fTrans))
		{
		delRightSeg	= 2;
		saveData		= ((TSegPoint *) fNext)->GetData();								// get data for this segment
		saveType		= fNext->GetSectionType();
		nData 			= ((TSegPoint *) fPoint)->GetData();							// get data for this segment
		((TSegPoint *) fNext)->SetData(nData);											// change data on next segment

		if (fPoint->GetSectionType() != saveType)
			{
			fGeom->Select(fNext,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fNext,fSide,fPoint->GetSectionType());
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);	
		saveDeleted = ((TPointMatrix *) fCDocument->fPointMatrix)->DeleteCurrent(fSide);
		fPoint = NULL;
		}
	else
		{
		temp = fPoint->RetrievePoint();
		this->DisplayIt(fPoint->fTrans,temp);
		fGeom->Select(fPoint,false,fSide);
		((TPointMatrix *) fCDocument->fPointMatrix)->DoHighlight();
		delRightSeg	= 0;
		saveDeleted	= NULL;
		}		
	
	can = trashL->GetRect();
	HLock((Handle) this);
	fGeom->InvalidRect(&can);
	can = trashR->GetRect();
	fGeom->InvalidRect(&can);
	HUnlock((Handle) this);
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();				// show number of segments
	}
		
// --------------------------------------------------------------------------------------------------
//	UnDo the command
//		recreate the point & add it back in
// --------------------------------------------------------------------------------------------------
pascal void TDeletePointCommand::UndoIt(void)
	{
	if (delRightSeg == 0)
		return;
			
//	Reset the data & wall types back to the original

	fGeom->Focus();
	fPoint = ((TPointMatrix *) fCDocument->fPointMatrix)->AddLine(saveDeleted,fSide,fRow,fColumn);	
	if (delRightSeg == 2)
		{
		((TSegPoint *) fNext)->SetData(saveData);
		if (saveType != fNext->GetSectionType())
			{
			fGeom->Select(fNext,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fNext,fSide,saveType);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		}
	else
		{
		if (saveType != fNext->GetSectionType())
			{
			fGeom->Select(fPoint,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fPoint,fSide,saveType);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		}		

	((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);						// point to the point
	((TInformationView *) fCDocument->fInfoView)->ShowDimension();						// show number of segments
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string return;
	}
	
// --------------------------------------------------------------------------------------------------
//	UnDo the command
// --------------------------------------------------------------------------------------------------
pascal void TDeletePointCommand::RedoIt(void)
	{
	WallRecord nData;
	 
	fGeom->Focus();
	if (delRightSeg == 1)																				// over left can
		{
		if (saveType != fNext->GetSectionType())											// are segment types same?
			{																									// no reset : redraw
			fGeom->Select(fPoint,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fPoint,fSide,fNext->GetSectionType());
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);	
		saveDeleted = ((TPointMatrix *) fCDocument->fPointMatrix)->DeleteCurrent(fSide);
		fPoint = NULL;
		}
	else if (delRightSeg == 2)
		{
		nData 			= ((TSegPoint *) fPoint)->GetData();							// get data for this segment
		((TSegPoint *) fNext)->SetData(nData);											// change data on next segment

		if (fPoint->GetSectionType() != saveType)
			{
			fGeom->Select(fNext,true,fSide);
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			((TPointMatrix *) fCDocument->fPointMatrix)->SetSegment(fNext,fSide,fPoint->GetSectionType());
			((TPointMatrix *) fCDocument->fPointMatrix)->DoDrawSegment();
			fGeom->UnSelect();
			}
		((TPointMatrix *) fCDocument->fPointMatrix)->SetCurrent(fPoint);	
		saveDeleted = ((TPointMatrix *) fCDocument->fPointMatrix)->DeleteCurrent(fSide);
		fPoint = NULL;
		}
	}
	
	
// --------------------------------------------------------------------------------------------------
//	Track the Mouse
// --------------------------------------------------------------------------------------------------
pascal TCommand * TDeletePointCommand::TrackMouse(TrackPhase thePhase, VPoint * /*anchorPoint*/,
	VPoint * /*previousPoint*/, VPoint * /*nextPoint*/, Boolean /*mouseDidMove*/)
	{
	if (!fGeom->IsDragging())																		// starting the delete procedure?
		{																										// show trash cans
		fGeom->lastCmd = 1;
		fGeom->SetDragging(true);																// turn dragging on
		trashL->DrawCan();																			// draw the trash cans
		trashR->DrawCan();																			// draw the trash cans
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
pascal void TDeletePointCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * nextPoint, 
	Boolean /*turnItOn*/, Boolean mouseDidMove)
	{
	Point trans, thisPoint;
	
	fGeom->Focus();
	if (mouseDidMove)																				// got movement
		{
		trans = fGeom->ViewToQDPt(nextPoint);
		thisPoint = AntiTransform(trans, fGeom->fMagnify);
		this->DisplayIt(trans,thisPoint);
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Free the object
// --------------------------------------------------------------------------------------------------
pascal void TDeletePointCommand::Free(void)
	{
	delete trashL;
	delete trashR;
	inherited::Free();
	}

// --------------------------------------------------------------------------------------------------
//	Commit the command
// --------------------------------------------------------------------------------------------------
pascal void TDeletePointCommand::Commit(void)
	{
	short x, side;
	TPoint * t;
	
	if (saveDeleted != NULL)
		{
		side = fSide->GetSide();
		for (x = (short) saveDeleted->fSize; x > 0; x--)
			{
			t = (TPoint *) saveDeleted->At(x);
			saveDeleted->Delete(t);
			FreeIfObject(t);
			}

		if (side == cLeft || side == cRight)														// if on right or left delete the list
			FreeIfObject(saveDeleted);
		}
	}
