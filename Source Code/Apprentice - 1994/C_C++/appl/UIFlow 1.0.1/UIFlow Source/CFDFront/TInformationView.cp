#pragma segment UIFlow
//*********************************************************************************
//	Methods for TInformationView Class
//*********************************************************************************
// --------------------------------------------------------------------------------------------------
//	Initialize the TControl Objects
// --------------------------------------------------------------------------------------------------
void TInformationView::IInformationView(TDocument * doc)
	{
	fDocument = doc;
	XCoord = (TEditText *) this->FindSubView('XCor');								// find XCor Box	
	YCoord = (TEditText *) this->FindSubView('YCor');								// find YCor Box
	}	
	
pascal void TInformationView::IRes(TDocument *itsDocument, TView *itsSuperView, Ptr *itsParams)
	{
	inherited::IRes(itsDocument,itsSuperView,itsParams);
	}
	
// --------------------------------------------------------------------------------------------------
//	Draw the TControl Objects
// --------------------------------------------------------------------------------------------------
pascal void TInformationView::Draw(Rect * /*thisRect*/)								// Draws the view seen in the window. 
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
	
	this->Adorn(&tRect,pen,adnLineTop);
	this->Adorn(&tRect,pen,adnLineRight);
	HUnlock((Handle) this);
	this->Focus();
	return;
	} 
	
// --------------------------------------------------------------------------------------------------
//	If the SuperView is changed want to move TinformationView
// --------------------------------------------------------------------------------------------------
pascal void TInformationView::SuperViewChangedSize(VPoint *delta, Boolean /*invalidate*/)
	{
	VPoint newLoc;
	
	newLoc.h = fLocation.h;
	newLoc.v = fLocation.v + delta->v;
	this->Locate(newLoc.h,newLoc.v,true);
	newLoc.h = fSize.h + delta->h;
	newLoc.v = fSize.v;
	this->Resize(newLoc.h,newLoc.v,true);
	return;	
	}

// --------------------------------------------------------------------------------------------------
//	Menu Command for Undoing & Redoing the edits
// --------------------------------------------------------------------------------------------------
pascal struct TCommand * TInformationView::DoMenuCommand(CmdNumber aCommand)
	{
	TGeomView * geomView;																		// the geometry
	geomView = (TGeomView *)(fSuperView->FindSubView ('geom'));			// get pointer to GeomView

	if (aCommand == 101 && geomView->lastCmd == 2) 							// Undo / Redo menu & lastCmd was infoview
		{
		this->StatusString(savePt);																// display the points last coords
		this->DoChoice(this,mButtonHit);														// do DoChoice to move / change it.
		return gNoChanges;
		}
	return inherited::DoMenuCommand(aCommand);									// process other menu commands.
	}
	
// --------------------------------------------------------------------------------------------------
//	Process the users action
// --------------------------------------------------------------------------------------------------
pascal void TInformationView::DoChoice(TView * origView, short itsChoice)
	{
	Str255 XPoint, YPoint;																				// pascal string
	StringPtr xText, yText;																			// pascal string storage
	char * XP, * YP;																						// pointer to pascal c format
	short saveCmd;																						// temp storage
	Point newPoint;																					// the newPoint
	TGeomView * geomView;																			// the geometry
	
	geomView = (TGeomView *)(fSuperView->FindSubView ('geom'));				// get pointer to GeomView
	inherited::DoChoice(origView,itsChoice);													// do inherited dochoice first
	
	if (itsChoice == mButtonHit)																		// pressed the set button?
		{
		struct realPt info;																				// define real coord structure
		
		saveCmd = geomView->lastCmd;															// save the last command processed
		geomView->lastCmd = 2;																		// set last command to set button
		XCoord->GetText(XPoint);																	// get the coordinates entered by the user.	
		YCoord->GetText(YPoint);
		
		XP = p2cstr((StringPtr) XPoint);															// 1st convert pascal string to c string.
		YP = p2cstr((StringPtr) YPoint);
		info.x = atof(XP);																					// convert string to float
		info.y = atof(YP);
		
		HLock((Handle) this);
		newPoint = geomView->fCDocument->RealToView(&info);							// convert real coords to view coords
//		newPoint = transform(temp,geomView->fMagnify);
		HUnlock((Handle) this);
		if (newPoint.v == NULL)
			{
			geomView->lastCmd = saveCmd;														// reset lastCmd
			xText = c2pstr(XP);
			yText = c2pstr(YP);
			XCoord->SetText(xText,true);															// reset x value
			YCoord->SetText(yText,true);															// reset y value
			return;																								// return to caller
			}
		savePt = geomView->DragCurrent(newPoint);										// drag the current point & save old fStart
		}
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Set the status string.
// --------------------------------------------------------------------------------------------------
void TInformationView::StatusString (Point thePoint)
	{
	char Xstrng[20], Ystrng[20];																// c storage string
	TGeomView * geomView;																		// the geometry
	StringPtr XStatus, YStatus;																	// pascal storage string
	struct realPt info;																				// real number structure
	Point temp;
	
	geomView = (TGeomView *)(fSuperView->FindSubView ('geom'));			// get pointer to GeomView
	HLock((Handle) this);
	temp = transform(thePoint,geomView->fMagnify);
	geomView->fCDocument->ViewToReal(thePoint, &info);						// convert point
	HUnlock((Handle) this);
	
	sprintf (Xstrng,"%.4E",info.x);															// create a c string containing x
	XStatus = c2pstr (Xstrng);																	// create a pascal type string
	sprintf (Ystrng,"%.4E",info.y);															// create a c string containing y
	YStatus = c2pstr (Ystrng);	

	XCoord->SetText (XStatus, true);															// display text
	YCoord->SetText (YStatus, true);
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Set the information string
// --------------------------------------------------------------------------------------------------
void TInformationView::InfoString (char * theString)
	{
	StringPtr pText;																					// pascal storage string
	
	pText = c2pstr(theString);																	// convert pascal to c
	fInfoBox = (TStaticText *) this->FindSubView('gInf');							// find the window
	fInfoBox->SetText(pText,kRedraw);														// draw the text
	}

// --------------------------------------------------------------------------------------------------
//	Clear the information string
// --------------------------------------------------------------------------------------------------
void TInformationView::ClrInfo (void)
	{
	char strg[2];
	StringPtr pText;																					// pascal storage string
	
	strg[0] = 0;																							// create empty string
	pText = c2pstr(strg);																			// convert pascal to c
	fInfoBox = (TStaticText *) this->FindSubView('gInf');							// find the window
	fInfoBox->SetText(pText,kRedraw);														// draw the text
	}

// --------------------------------------------------------------------------------------------------
//	Display the number of boundry points defined in the geometry
// --------------------------------------------------------------------------------------------------
void TInformationView::ShowDimension (void)
	{
	char			x[155];																				// c string
	StringPtr	pText;																					// pascal string
	short 		fTop, fLeft;																			// number of points on the boundry
	
	fTop = fLeft = 2;																					// init number of points
	fLeft	= ((TCFDFrontDocument *) fDocument)->fPointMatrix->GetTColumn();
	fTop	= (short) ((TCFDFrontDocument *) fDocument)->fPointMatrix->fSize;
		
	sprintf(x,"Points: R %d  C %d",fTop,fLeft);											// create the message
	pText = c2pstr(x);																				// convert message to pascal
	fLocBox = (TStaticText *) this->FindSubView('LOCT');							// find LOCT box
	fLocBox->SetText(pText,kRedraw);														// display message
	}
