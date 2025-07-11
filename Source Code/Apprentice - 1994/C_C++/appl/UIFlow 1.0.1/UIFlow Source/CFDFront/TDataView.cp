#pragma segment MenuOption
//*********************************************************************************
//	TMyDialogTEView - Methods
//*********************************************************************************
// ------------------------------------------------------------------------------------------
//	DialogTEView initializer
// ------------------------------------------------------------------------------------------
pascal void TMyDialogTEView::IDialogTEView(TDocument *itsDocument, TView *itsSuperView, VPoint *
		itsLocation, VPoint *itsSize, SizeDeterminer itsHDeterminer, SizeDeterminer itsVDeterminer, 
		Rect *itsInset, TextStyle *itsTextStyle, short itsJustification, Boolean itsStyleType, Boolean
	    itsAutoWrap)
		{
		inherited::IDialogTEView(itsDocument, itsSuperView,itsLocation, itsSize, itsHDeterminer,itsVDeterminer, 
	  		itsInset,itsTextStyle,itsJustification,itsStyleType, itsAutoWrap);
	   }

// ------------------------------------------------------------------------------------------
//	select the edit text box for cursor insertion
// ------------------------------------------------------------------------------------------
pascal void TMyDialogTEView::InstallSelection(Boolean wasActive, Boolean beActive)
	{
//	if (((TDataView *) fEditText->fSuperView)->fPoint == NULL && beActive)
//		{
//		TWarning * tWarning;
//		TWindow	* aWindow;
		
//		aWindow = NewTemplateWindow(kWarnMe,(TDocument*) fDocument);
//		tWarning = (TWarning *) aWindow->FindSubView('WARN');
//		tWarning->IWarning(3,NULL);
//		tWarning->ShowWarning();
//		((TDataView *)fEditText->fSuperView)->ForceRedraw();
//		return;
//		}
	inherited::InstallSelection(wasActive,beActive);
	}
	
// ------------------------------------------------------------------------------------------
//	Make the typing command
// ------------------------------------------------------------------------------------------
pascal struct TTETypingCommand * TMyDialogTEView::DoMakeTypingCommand(short ch)
	{
	TMyTypingCommand * aTypingCommand;

	aTypingCommand = new TMyTypingCommand;
	if (aTypingCommand == NULL)
		return (TTETypingCommand *) gNoChanges;
	aTypingCommand->ITETypingCommand(this, ch);
	return (TTETypingCommand *) aTypingCommand;
	}

// ------------------------------------------------------------------------------------------
//	Store the Data
// ------------------------------------------------------------------------------------------
void TMyDialogTEView::StoreData(void)
	{
	((TDataView *) fEditText->fSuperView)->StoreData();
	return;
	}

//*********************************************************************************
//	TMyTypingCommand - Methods
//*********************************************************************************
// ------------------------------------------------------------------------------------------
//	Add the character into the field
// ------------------------------------------------------------------------------------------
pascal void TMyTypingCommand::AddCharacter(short aChar)
	{
	char test;
	
	test = aChar;
	if ((aChar >= '0' && aChar <= '9') || aChar == '.' || aChar == '-')
		test = chReturn;
		
	switch (test)
		{
		case chFwdDelete:
		case chBackspace:
		case chTab:
		case chReturn:
		case chEnd:
		case chDown:
		case chEscape:
		case chHome:
		case chLeft:
		case chRight:
		case chUp:
			inherited::AddCharacter(aChar);
			((TMyDialogTEView *) fTEView)->StoreData();
			break;
		default:
			SysBeep (3);
			return;
		}
	}

//*********************************************************************************
//	TDataView - Methods
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
//*********************************************************************************
//	init this from our own resource.
// --------------------------------------------------------------------------------------------------
pascal void TDataView::IDataView (void)
	{
	TWindow 		* aWindow;

	aWindow = this->GetWindow();																	// get the window reference
	fGeom = (TGeomView *) aWindow->FindSubView('geom');							// get GeomView reference
	fScroll = (TScroller *) aWindow->FindSubView('VW02');							// get scroll bar reference
	tWall	= (TRadio *) aWindow->FindSubView('WALL');
	tIflw		= (TRadio *) aWindow->FindSubView('IFLW');
	tSymm	= (TRadio *) aWindow->FindSubView('SYMM');
	tOflw	= (TRadio *) aWindow->FindSubView('OFLW');
	tIntr		= (TRadio *) aWindow->FindSubView('INTR');
	return;
	}
// --------------------------------------------------------------------------------------------------
//	Draw the View 
// --------------------------------------------------------------------------------------------------
pascal void TDataView::Draw(Rect * /*thisRect*/)											// Draws the view seen in the window. 
	{	
	Point pen;
	Rect  tRect;
	pen.v = 1; 	pen.h = 1;
	tRect.top = 0;
	tRect.left = 0;
	tRect.right = (short) fSize.h;
	tRect.bottom = (short) fSize.v;

	HLock((Handle) this);
	RGBForeColor(&GridLineColor);																	// set color
	PenPixPat(GridLinePat);
	
	this->Adorn(&tRect,pen,adnLineBottom);
	this->Adorn(&tRect,pen,adnLineLeft);
	HUnlock((Handle) this);
	return;
	} 

// --------------------------------------------------------------------------------------------------
//	If the SuperView is changed want to move TinformationView
// --------------------------------------------------------------------------------------------------
pascal void TDataView::SuperViewChangedSize(VPoint *delta, Boolean /*invalidate*/)
	{
	VPoint newLoc, newLoc1;
	
	newLoc.h = fLocation.h  + delta->h;
	newLoc.v = fLocation.v;
	newLoc1.h = fSize.h;
	newLoc1.v = fSize.v + delta->v;
	if (fShown)
		{
		this->Locate(newLoc.h,newLoc.v,true);
		this->Resize(newLoc1.h,newLoc1.v,true);
		}
	else
		{
		this->Locate(newLoc.h,newLoc.v,false);
		this->Resize(newLoc1.h,newLoc1.v,false);
		}
	return;	
	}

// --------------------------------------------------------------------------------------------------
//	Display the data view
// --------------------------------------------------------------------------------------------------
void TDataView::ShowIt(void)
	{
	VCoordinate 	width, height;
	TPoint			* tPt;
	TWindow 		* aWindow;

	aWindow = this->GetWindow();																	// get the window reference	
	matrix = ((TGeomView *) fGeom)->fCDocument->fPointMatrix;				// get matrix reference
	
	height	= fScroll->fSize.v;																		// calculate new size
	width	= fScroll->fSize.h - 116;
	fScroll->Resize(width,height,true);															// change & redraw
	fGeom->Focus();																						// focus on geometry
	this->Show(true,true);																				// bring view to front
	fGeom->Update();

	if (((TGeomView *) fGeom)->fSegment)													// segment is already selected
		{
		this->ShowData((TSegPoint *)((TGeomView *) fGeom)->fSPoint);			// just show the data
		return;
		}
	else if (((TGeomView *) fGeom)->fSPoint != NULL)									// have a point no segment
		{
		TSegPoint * sPt;
		TRow		*	side;
		
//		if (((TGeomView *) fGeom)->fSPoint->IsInterior())
//			this->ShowData((TSegPoint *) ((TGeomView *) fGeom)->fSPoint);	// is interior point
//		else
		if (!((TGeomView *) fGeom)->fSPoint->IsInterior())
			{
			tPt = ((TGeomView *) fGeom)->fSPoint;											// save the old selected point
			side = ((TGeomView *) fGeom)->fSSide;
			matrix->DoHighlight(); 																		// unhighlight old point
			((TGeomView *) fGeom)->UnSelect();												// unselect everything
		
			sPt = matrix->FindSegment(tPt,side);												// get the segment
			((TGeomView *) fGeom)->Select(sPt,true,side);								// select new segment
			matrix->DoHighlight(); 																		// highlight new segment
	
			this->Focus();
			this->ShowData(sPt);																		// display the data
			return;
			}
		else
			{
			matrix->DoHighlight();
			((TGeomView *) fGeom)->UnSelect();
			}
		}


	tPt = ((TGeomView *) fGeom)->fCDocument->fInterior;
	((TGeomView *) fGeom)->Select(tPt,false,NULL);
	this->ShowData((TSegPoint *) tPt);
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Hide the data view
// --------------------------------------------------------------------------------------------------
void TDataView::HideIt(void)
	{
	VCoordinate 	width, height;
	
	this->DeselectCurrentEditText();																	// dispose current TEView

	height	= fScroll->fSize.v;																			// calculate new size
	width	= fScroll->fSize.h + 116;
	fScroll->Resize(width,height,true);																// resize & redraw
	this->Show(false,true);																				// hide it
	fGeom->Focus();																							// focus on geometry
	
	if (fPoint->IsInterior())																					// interior point?
		((TGeomView *) fGeom)->UnSelect();														// unselect it
		
	else if (((TGeomView *) fGeom)->fSSide != NULL)											// have a side?
		{
		TPoint	*	tPt;
		TRow	*	side;
		
		tPt = ((TGeomView *) fGeom)->fSPoint;													// save the point
		side = ((TGeomView *) fGeom)->fSSide;
		matrix->DoHighlight(); 																				// highlight new segment
		((TGeomView *) fGeom)->UnSelect();														// unselect the segment
		((TGeomView *) fGeom)->Select(tPt,false,side);										// reselect the point
		matrix->DoHighlight(); 																				// highlight new segment
		}
	}
		
// --------------------------------------------------------------------------------------------------
//	Show the point data
// --------------------------------------------------------------------------------------------------
void TDataView::ShowData(TSegPoint * tPt)
	{
	TWindow	* 	aWindow;
	TEditText	*	iText;
	TCluster	* tCluster;
	TRadio		* tRadio;
	IDType		rCurrent;
	short index;
	char  string[20];
	StringPtr pString;
	
	fPoint = tPt;
	wRec.theData = tPt->GetData();													// the data

	aWindow = this->GetWindow();														// get the window pointer
	for (index=1; index< 12; index++)												// display each piece of data
		{
		this->GetControlName(index,"CK");											// get scroller name
		iText = (TEditText *) aWindow->FindSubView(tbox.boxID);		// find the data box
		
		sprintf(string,"%e",wRec.iData[index-1]);								// convert the data to string
		pString = c2pstr(string);															// convert to pascal type
		iText->SetText(pString,true);
		}
				
	tCluster 	= (TCluster *) aWindow->FindSubView('WTYP');			// get the cluster view
	rCurrent	= tCluster->ReportCurrent();										// get current selected button

	switch (fPoint->GetSectionType())
		{
		case Wall:
			tWall->Toggle(true);
			break;
		case InFlow:
			tIflw->Toggle(true);
			break;
		case Symm:
			tSymm->Toggle(true);
			break;
		case OutFlow:
			tOflw->Toggle(true);
			break;
		case Grid:
			tIntr->Toggle(true);			
			tWall->DimState(true,true);													// dim other controls
			tIflw->DimState(true,true);
			tSymm->DimState(true,true);
			tOflw->DimState(true,true);
			break;
		}

	tRadio = (TRadio *) aWindow->FindSubView(rCurrent);					// turn off old value
	tRadio->Toggle(true);

	if (tWall->IsDimmed() && !fPoint->IsInterior())							// controls dimmed & boundry point
		{
		tWall->DimState(false,true);
		tIflw->DimState(false,true);
		tSymm->DimState(false,true);
		tOflw->DimState(false,true);
		}		
	return;
	}
	
// ------------------------------------------------------------------------------------------
//	DoChoice Method - get the user response in this window
// ------------------------------------------------------------------------------------------
pascal void TDataView::DoChoice(TView *origView, short itsChoice)
	{
	TRow			* side;
	
	if (itsChoice == mRadioHit)
		{
		if (origView == tIntr)																// interior point?
			{
			TSegPoint * t;																		// get interior point
			t = (TSegPoint *) ((TCFDFrontDocument *) (((TGeomView *) fGeom)->fCDocument))->fInterior;
			this->ShowData(t);																// display the data
			((TGeomView *) fGeom)->Focus();										// focus on geometry
			matrix->DoHighlight();															// unhighlight old
			((TGeomView *) fGeom)->UnSelect();									// unselect old
			((TGeomView *) fGeom)->Select(t,false,NULL);					// select interior point
			}
		else
			{
			if (fPoint->IsInterior())															// point is interior
				SysBeep (3);
			else
				{			
				side = ((TGeomView *) fGeom)->fSSide;
				((TGeomView *) fGeom)->Focus();
				PenMode(srcXor);
				matrix->DoDrawSegment();												// erase endpoints

				if (origView == tWall)
					matrix->SetSegment(fPoint, side, Wall);
				else if (origView == tIflw)
					matrix->SetSegment(fPoint, side, InFlow);
				else if (origView == tOflw)
					matrix->SetSegment(fPoint, side, OutFlow);
				else if (origView == tSymm)
					matrix->SetSegment(fPoint, side, Symm);
				matrix->DoDrawSegment();													// erase endpoints
				}
			}
		}
	this->Focus();
	inherited::DoChoice(origView,itsChoice);
	}

// ------------------------------------------------------------------------------------------
//	Make a dialog view used for textedit fields
// ------------------------------------------------------------------------------------------
pascal struct TDialogTEView * TDataView::MakeTEView(void)
	{
	TMyDialogTEView * aDialogTEView;
	
	aDialogTEView = new TMyDialogTEView;
	if (aDialogTEView == NULL)
		return (TDialogTEView *) gNoChanges;
	HLock((Handle) this);
	aDialogTEView->IDialogTEView(NULL, NULL, &gZeroVPt, &gZeroVPt, sizeRelSuperView, sizeVariable,
								&gZeroRect, &gSystemStyle, teJustSystem, kWithoutStyle, false);
	HUnlock((Handle) this);

	aDialogTEView->fMinAhead = 1;															// Don't _jump_ the view ahead 
																												// when autoscrolling for
																												// scrollselectionintoview
	return (TDialogTEView *) aDialogTEView;
	}
	
// ------------------------------------------------------------------------------------------
//	Set the text for this point
// ------------------------------------------------------------------------------------------
void TDataView::StoreData(void)
	{
	short			index;
	Str255			pString;
	char 			* string;
	TWindow		*	aWindow;
	TEditText		*	iText;
	
	if (fPoint == NULL)																				// no point to store data for	
		return;

	aWindow = this->GetWindow();																// get the window pointer
	for (index=1;index<12;index++)															// display each piece of data
		{
		this->GetControlName(index,"CK");													// get scroller name
		iText = (TEditText *) aWindow->FindSubView(tbox.boxID);				// find the data box
		if (iText == fCurrentEditText)															// same box?
			{
			float num;
			
			iText->GetText(pString);
			string = p2cstr((StringPtr) pString);
			num = atof(string);
			wRec.iData[index-1] = num;
			break;
			}
		}
	((TSegPoint *) fPoint)->SetData(wRec.theData);
	}