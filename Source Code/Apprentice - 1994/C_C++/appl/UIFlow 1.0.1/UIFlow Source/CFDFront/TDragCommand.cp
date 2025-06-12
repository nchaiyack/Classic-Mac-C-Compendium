#pragma segment Boundry
// **********************************************************************
//	TDragCommand - Methods
// **********************************************************************
//		drag directions (in info structure: dLeft & dRight
//			true is toward the selected point (point we are dragging
//			false is away from the point being dragged
// --------------------------------------------------------------------------------------------------
//	Initializing the dragger command .....
// --------------------------------------------------------------------------------------------------
short TDragCommand::IDragCommand(TGeomView * geom, TPoint * pt, Point * theMouse)
	{
	fGeom				= geom;																		// save the geometry
	fCDocument		= (TCFDFrontDocument *)fGeom->fCDocument;				// get the document
	fMatrix			= fCDocument->fPointMatrix;										// overseer matrix
	fPoint				= pt;																				// save this segment
	fMouse.h			= theMouse->h;																// save the mouse location
	fMouse.v			= theMouse->v;
		
	lastTrack = fPoint->GetStart();															// save location of this segment
	fPoint->RememberPoint();																	// remember yourselft
	HLock((Handle) this);
	if (fMatrix->SetCornors(fPoint,&dInfo) == 1)										// get the surrounding points
		{
		HUnlock((Handle) this);
		return 1;
		}
	HUnlock((Handle) this);
	if (dInfo.lrc != NULL)
		dInfo.lrc->RememberPoint();
		
	if (fGeom->fSSide != NULL)
		fSide = fGeom->fSSide->GetSide();
	else
		fSide = cNotOnBoundry;
	fGeom->SetDragging(false);																	// init dragging to false
	ICommand (cDragPointCommand,fCDocument,fGeom,fGeom->GetScroller(true));
	fConstrainsMouse = true;																		// constrain the mouse
	return 0;
	}

// --------------------------------------------------------------------------------------------------
//	Display the Lines
// --------------------------------------------------------------------------------------------------
void TDragCommand::DisplayIt(Point trans, Point temp)
	{
	fGeom->Focus();																					// focus on geometry
	PenMode(srcXor);																					// xor drawing
	
	if (!fPoint->IsBoundryPt())																	// a grid point
		{
		fPoint->Draw();																				// erase point
		fPoint->DrawGrid(dInfo.above->fTrans);											// erase old lines
		fPoint->DrawGrid(dInfo.right->fTrans);
		fPoint->DrawGrid(dInfo.below->fTrans);
		fPoint->DrawGrid(dInfo.left->fTrans);
		
		fPoint->SetStart(temp.v,temp.h);														// set the point to the new point
		
		fPoint->DrawGrid(dInfo.above->fTrans);											// draw new lines
		fPoint->DrawGrid(dInfo.right->fTrans);
		fPoint->DrawGrid(dInfo.below->fTrans);
		fPoint->DrawGrid(dInfo.left->fTrans);
		fPoint->Draw();																				// redraw the point	
		}
	else if (fPoint->IsCornor())
		{
		fPoint->Draw();																				// erase point
		if (dInfo.lrc != NULL)
			dInfo.lrc->DrawSection(dInfo.left->fTrans);
		else if (dInfo.dLeft)
			dInfo.left->DrawSection(fPoint->fTrans);
		else
			fPoint->DrawSection(dInfo.left->fTrans);										// redraw boundry line
		if (dInfo.dRight)
			dInfo.right->DrawSection(fPoint->fTrans);									// erase boundry line
		else
			fPoint->DrawSection(dInfo.right->fTrans);									// redraw boundry line
		
		if (dInfo.above != NULL)																		// is there a grid line?
			fPoint->DrawGrid(dInfo.above->fTrans);										// erase it
		
		fPoint->SetStart(temp.v,temp.h);														// set the point to the new point
		if (dInfo.lrc != NULL)
			dInfo.lrc->SetStart(temp.v,temp.h);

		if (dInfo.lrc != NULL)
			dInfo.lrc->DrawSection(dInfo.left->fTrans);
		else if (dInfo.dLeft)
			dInfo.left->DrawSection(fPoint->fTrans);
		else
			fPoint->DrawSection(dInfo.left->fTrans);										// redraw boundry line
		if (dInfo.dRight)
			dInfo.right->DrawSection(fPoint->fTrans);									// erase boundry line
		else
			fPoint->DrawSection(dInfo.right->fTrans);									// redraw boundry line

		if (dInfo.above != NULL)																		// is there a grid line?
			fPoint->DrawGrid(dInfo.above->fTrans);										// redraw it
		fPoint->Draw();																				// redraw the point	
		}
	else		
		{
		fPoint->DrawHold();
		fPoint->DrawTemp(trans,dInfo.left->fTrans);									// redraw boundry line
		dInfo.right->DrawTemp(trans,dInfo.right->fTrans);							// erase boundry line

		fPoint->SetHold(temp.v,temp.h);
		fMatrix->MovePoints(temp,fLength);												// move selected row / column
		fPoint->DrawHold();
		}
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Display the Obstacles
// --------------------------------------------------------------------------------------------------
void TDragCommand::DisplayObstacles(void)
	{
	if (fPoint->fObsUpLeft != NULL)
		((TObstacle *) ((TPoint * ) fPoint)->fObsUpLeft)->Draw();
	if (fPoint->fObsUpRight != NULL)
		((TObstacle *) ((TPoint * ) fPoint)->fObsUpRight)->Draw();
	if (fPoint->fObsLwLeft != NULL)
		((TObstacle *) ((TPoint * ) fPoint)->fObsLwLeft)->Draw();
	if (fPoint->fObsLwRight != NULL)
		((TObstacle *) ((TPoint * ) fPoint)->fObsLwRight)->Draw();

	if (fPoint->fBaffAbove != NULL)
		((TBaffle *) ((TPoint * ) fPoint)->fBaffAbove)->Draw();
	if (fPoint->fBaffBelow != NULL)
		((TBaffle *) ((TPoint * ) fPoint)->fBaffBelow)->Draw();
	if (fPoint->fBaffLeft != NULL)
		((TBaffle *) ((TPoint * ) fPoint)->fBaffLeft)->Draw();
	if (fPoint->fBaffRight != NULL)
		((TBaffle *) ((TPoint * ) fPoint)->fBaffRight)->Draw();
	}

// --------------------------------------------------------------------------------------------------
//	Do the command
// --------------------------------------------------------------------------------------------------
pascal void TDragCommand::DoIt(void)
	{
	if (fPoint->IsBoundryPt() && !fPoint->IsCornor())
		{
		fPoint->SetStart(fPt.v,fPt.h);										// set the point to the new point
		fMatrix->FixHold();
		if (fMatrix->IsShown())
			fMatrix->DrawHold(false);
		else
			{
			fPoint->DrawSection(dInfo.left->fTrans);						// redraw boundry line
			dInfo.right->DrawSection(fPoint->fTrans);						// erase boundry line
			}
		fMatrix->DoHighlight();												// rehighlight the point
		((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
		return;
		}
		
	this->DisplayObstacles();
	fMatrix->DoHighlight();													// rehighlight the point
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
	fGeom->Focus();
	}
	
// --------------------------------------------------------------------------------------------------
//	UnDo the command
// --------------------------------------------------------------------------------------------------
pascal void TDragCommand::UndoIt(void)
	{
	Point temp,trans;
	
	if (!fPoint->IsBoundryPt() || fPoint->IsCornor())
		{
		fMatrix->DoHighlight();												// rehighlight the point
		this->DisplayObstacles();
		temp		= fPoint->RetrievePoint();
		trans	= fPoint->fTrans;
		fPoint->RememberPoint();
		if (dInfo.lrc != NULL)
			dInfo.lrc->RememberPoint();
		this->DisplayIt(trans,temp);
		lastTrack = temp;

		this->DisplayObstacles();
		fMatrix->DoHighlight();																			// rehighlight the point
		((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
		return;
		}
	
	fMatrix->DoHighlight();																				// rehighlight the point
	if (fMatrix->IsShown())
		fMatrix->DrawHold(true);	
	else
		{
		fPoint->DrawSection(dInfo.left->fTrans);													// redraw boundry line
		dInfo.right->DrawSection(fPoint->fTrans);												// erase boundry line
		fPoint->Draw();
		}

	fMatrix->RetrieveAndFix2Hold();																	// gets old pt back into hold & fixes it
	if (fMatrix->IsShown())
		fMatrix->DrawHold(true);	
	else
		{
		fPoint->DrawSection(dInfo.left->fTrans);													// redraw boundry line
		dInfo.right->DrawSection(fPoint->fTrans);												// erase boundry line
		fPoint->Draw();
		}

	fMatrix->DoHighlight();																				// rehighlight the point
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
	
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	UnDo the command
// --------------------------------------------------------------------------------------------------
pascal void TDragCommand::RedoIt(void)
	{
	Point temp,trans;

	if (!fPoint->IsBoundryPt() || fPoint->IsCornor())
		{
		fMatrix->DoHighlight();																		// rehighlight the point
		this->DisplayObstacles();
		temp		= fPoint->RetrievePoint();
		trans	= fPoint->fTrans;
		fPoint->RememberPoint();
		if (dInfo.lrc != NULL)
			dInfo.lrc->RememberPoint();
		this->DisplayIt(trans,temp);
		lastTrack = temp;
	
		this->DisplayObstacles();
		fMatrix->DoHighlight();																		// rehighlight the point
		((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
		return;
		}

	fMatrix->DoHighlight();																				// rehighlight the point
	if (fMatrix->IsShown())
		fMatrix->DrawHold(true);	
	else
		{
		fPoint->DrawSection(dInfo.left->fTrans);													// redraw boundry line
		dInfo.right->DrawSection(fPoint->fTrans);												// erase boundry line
		fPoint->Draw();
		}

	fMatrix->RetrieveAndFix2Hold();																	// gets old pt back into hold & fixes it
	if (fMatrix->IsShown())
		fMatrix->DrawHold(true);	
	else
		{
		fPoint->DrawSection(dInfo.left->fTrans);													// redraw boundry line
		dInfo.right->DrawSection(fPoint->fTrans);												// erase boundry line
		fPoint->Draw();
		}

	fMatrix->DoHighlight();																				// rehighlight the point
	((TInformationView *) fCDocument->fInfoView)->StatusString(fPoint->fStart);	// display the status string
	
	return;
	}
	
	
// --------------------------------------------------------------------------------------------------
//	Track the Mouse
// --------------------------------------------------------------------------------------------------
pascal TCommand * TDragCommand::TrackMouse(TrackPhase thePhase, VPoint * /*anchorPoint*/,
	VPoint * /*previousPoint*/, VPoint * nextPoint, Boolean mouseDidMove)
	{
	PointInfo 	sInfo;
	
	fGeom->Focus();
		
	if (thePhase == trackRelease)																	// mouse button released?
		{
		if (fGeom->IsDragging())																		// done dragging reselect & highlight point
			{
			fGeom->SetDragging(false);																// set dragging off
			if (fMatrix->IsShown())
				{
				fPoint->DrawHold();
				fGeom->Focus();
				sInfo.gridOnly = false;																	// assign opposite of fShown
				if (!fPoint->IsCornor())
					{
					if (fSide == cRight || fSide == cLeft)
						{
						HLock((Handle) this);
						sInfo.row = (fMatrix->GetRow()-1);
						fMatrix->OneRowDo(DrawAllPoints,&sInfo);
						sInfo.row = (fMatrix->GetRow()+1);
						fMatrix->OneRowDo(DrawAllPoints,&sInfo);
						HUnlock((Handle) this);
						}
					else if (fSide == cTop || fSide == cBottom)
						{
						HLock((Handle) this);
						sInfo.column = (fMatrix->GetColumn()-1);
						fMatrix->OneColumnDo(DrawPoint,&sInfo);
						sInfo.column = (fMatrix->GetColumn()+1);
						fMatrix->OneColumnDo(DrawPoint,&sInfo);
						HUnlock((Handle) this);
						}
					}
				}					
			}
		else																										// wasn't dragging in the first place
			return gNoChanges;																			// do nothing
		}
	else if (!fGeom->IsDragging())																	// starting to drag point unselect & unhighlight
		{
		fGeom->lastCmd = 1;
		fGeom->SetDragging(true);																	// set dragging on
		fMatrix->DoHighlight();																		// unhighlight point
		fPoint->RememberPoint();																	// remember starting location
		if (!fPoint->IsCornor() && fPoint->IsBoundryPt())
			{
			if (fSide == cRight || fSide  == cLeft)
				{
				TRow * tRow;
				short 	 r;
				r = fMatrix->GetRow();
				tRow = (TRow *) fMatrix->At(r);
				fLength = tRow->GetLength(NULL);												// the cumulative length of the row
				tRow->RememberRow();																// all points in row remember self
				}
			else if (fSide == cTop || fSide == cBottom)
				{
				short c;
				c = fMatrix->GetColumn();
				fLength = fMatrix->GetColumnLength(c);
				fMatrix->RememberColumn();														// all points in column remember self
				}
			if (fMatrix->IsShown())
				fMatrix->DrawHold(true);
			else
				{
				fPoint->DrawSection(dInfo.left->fTrans);										// redraw boundry line
				dInfo.right->DrawSection(fPoint->fTrans);									// erase boundry line
				fPoint->Draw();
				}
			}
		else
			this->DisplayObstacles();
		}
		
	if (mouseDidMove)																					// did the mouse move?
		{
		Point tmp, tmp1; 
		tmp1 = VPtToPt (nextPoint);																	// convert nextpoint to view point
		tmp = AntiTransform(tmp1,fGeom->fMagnify);
		
		((TInformationView *) fCDocument->fInfoView)->StatusString(tmp);	// display the status string
		}
	return this;																								// return this command to MacApp
	}

// -------------------------------------------------------------------------------------------------
//	tracking feedback. limit the mouse motion to the geometry not including the border.
// -------------------------------------------------------------------------------------------------
pascal void TDragCommand::TrackConstrain (VPoint * /*ancorPoint*/, 
			VPoint * /*previousPoint*/, VPoint * nextPoint)	
	{
	if (nextPoint->v <= 5)																				// limit the verticle dimension
		nextPoint->v = 5;
	else
		if (nextPoint->v >= fGeom->fSize.v - 5)
			nextPoint->v = fGeom->fSize.v - 5;

	if (nextPoint->h <= 5)																				// limit the horizontal dimension
		nextPoint->h = 5;
	else
		if (nextPoint->h >= fGeom->fSize.h - 5)
			nextPoint->h = fGeom->fSize.h - 5;
}

// --------------------------------------------------------------------------------------------------
//	Give user feedback on the mouse drag
// --------------------------------------------------------------------------------------------------
pascal void TDragCommand::TrackFeedback(VPoint * /*anchorPoint*/,VPoint * nextPoint, 
	Boolean /*turnItOn*/, Boolean mouseDidMove)
	{
//
//	lastTrack is thisPoint the last time through this method.  It is initialized to the fStart point
//	of the selected segment in IDragPointCommand Method.
//
	fGeom->Focus();																						// focus on geomview
	if (mouseDidMove)																					// did the mouse move
		{
		Point tmp, thisPoint;
		
		tmp = fGeom->ViewToQDPt(nextPoint);													// get quick draw coordinates
		thisPoint = AntiTransform(tmp,fGeom->fMagnify);
		fPt = thisPoint;
		this->DisplayIt(tmp,thisPoint);																// display the lines.		
		lastTrack = thisPoint;																			// reset tracking point
		}
	return;																										// return
	}
	
// --------------------------------------------------------------------------------------------------
//	Move the Point (called from TInformationView::DoChoice)
// --------------------------------------------------------------------------------------------------
 void TDragCommand::MovePoint(void)
	{
	Point tmp;
	PointInfo 	sInfo;
	tmp = fPoint->fTrans;
	
	fGeom->Focus();																					// focus on geometry
	fPoint->RememberPoint();																	// remember starting location
	if (!fPoint->IsCornor() && fPoint->IsBoundryPt())
		{
		if (fSide == cRight || fSide  == cLeft)
			{
			TRow * tRow;
			short 	 r;
			r = fMatrix->GetRow();
			tRow = (TRow *) fMatrix->At(r);
			fLength = tRow->GetLength(NULL);												// the cumulative length of the row
			tRow->RememberRow();																// all points in row remember self
			}
		else if (fSide == cTop || fSide == cBottom)
			{
			short c;
			c = fMatrix->GetColumn();
			fLength = fMatrix->GetColumnLength(c);
			fMatrix->RememberColumn();														// all points in column remember self
			}
		if (fMatrix->IsShown())
			fMatrix->DrawHold(true);
		else
			{
			fPoint->DrawSection(dInfo.left->fTrans);										// redraw boundry line
			dInfo.right->DrawSection(fPoint->fTrans);									// erase boundry line
			fPoint->Draw();
			}
		fPoint->SetHold(fMouse.v,fMouse.h);
		fMatrix->MovePoints(fMouse,fLength);
		}
	else
		{
		Point tmp, thisPoint;
		VPoint t;
		t.h = fMouse.h;
		t.v = fMouse.v;
		tmp = fGeom->ViewToQDPt(&t);
		thisPoint = AntiTransform(tmp,fGeom->fMagnify);
		this->DisplayIt(tmp,thisPoint);
		this->DisplayObstacles();
		}
	
	this->DoIt();
	fGeom->Focus();

	if (fMatrix->IsShown())
		{
		if (!fPoint->IsCornor())
			{
			sInfo.gridOnly = false;																		// assign opposite of fShown

			if (fSide == cRight || fSide == cLeft)
				{
				HLock((Handle) this);
				sInfo.row = (fMatrix->GetRow()-1);
				fMatrix->OneRowDo(DrawAllPoints,&sInfo);
				sInfo.row = (fMatrix->GetRow()+1);
				fMatrix->OneRowDo(DrawAllPoints,&sInfo);
				HUnlock((Handle) this);
				}
			else if (fSide == cTop || fSide == cBottom)
				{
				HLock((Handle) this);
				sInfo.column = (fMatrix->GetColumn()-1);
				fMatrix->OneColumnDo(DrawPoint,&sInfo);
				sInfo.column = (fMatrix->GetColumn()+1);
				fMatrix->OneColumnDo(DrawPoint,&sInfo);
				HUnlock((Handle) this);
				}
			} 
		}				
	return;																										// return;
	}
	
	