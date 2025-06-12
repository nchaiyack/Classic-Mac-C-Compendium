#pragma segment UIFlow
// **********************************************************************
//	TPctsView & TOpPict - Methods
// 	TPctsView & TOpPict together provide the palette functionality.
// 	The superview contains the entire palette, and the components of the palette also
// 	have their own class and functionality
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Do the mouse command.
// --------------------------------------------------------------------------------------------------
pascal struct TCommand * TOpPict::DoMouseCommand(Point * /*theMouse*/, EventInfo * /*info*/,
							Point * /*hysteresis*/)
	{
	IDType dataPic	= (IDType) 'Ops6';															// the data icon
	IDType fineGrid	= (IDType) 'Ops7';															// fine grid icon
	short oldIcon;
	
	oldIcon = ((TCFDFrontDocument *) fDocument)->GetMouseAction();
	((TPctsView *) fSuperView)->TurnAllOff();
	((TCFDFrontDocument *) fDocument)->SetMouseAction ((IDType) fIdentifier);
	HiliteState (true, true);																			// hilite the picture.
	
	if ((IDType) fIdentifier == dataPic)
		{
		if (! ((TCFDFrontDocument *) fDocument)->fDataView->IsShown())
			((TCFDFrontDocument *) fDocument)->fDataView->ShowIt();
		}
	else if (oldIcon == 5)
		if (((TCFDFrontDocument *) fDocument)->fDataView->IsShown())
			((TCFDFrontDocument *) fDocument)->fDataView->HideIt();

	if ((IDType) fIdentifier == fineGrid && oldIcon != 6)
		{
		((TCFDFrontDocument *) fDocument)->fPointMatrix->SetFineGrid(true);
		((TCFDFrontDocument *) fDocument)->fPointMatrix->ShowFineGrid();
		}
	else if ((IDType) fIdentifier != fineGrid && oldIcon == 6)
		{
		((TCFDFrontDocument *) fDocument)->fPointMatrix->SetFineGrid(false);
		((TCFDFrontDocument *) fDocument)->fPointMatrix->ShowFineGrid();
		}
	return gNoChanges;																					// no command.
	}
	
// --------------------------------------------------------------------------------------------------
//	init this from our own resource.
// --------------------------------------------------------------------------------------------------
pascal void TOpPict::IRes (TDocument * itsDocument, TView * itsSuperView, Ptr * itsParams)
	{
	TPicture::IRes(itsDocument, itsSuperView, itsParams);
	fDocument	= itsDocument;
	fGeom			=	(TGeomView *) itsSuperView;
	}
	
// --------------------------------------------------------------------------------------------------
//		TPctsView stuff
//	init this from our own resource.
// --------------------------------------------------------------------------------------------------
pascal void TPctsView::IRes (TDocument * itsDocument, TView * itsSuperView, Ptr * itsParams)
	{
	TView::IRes(itsDocument, itsSuperView, itsParams);
	fDocument	= itsDocument;
	}

// --------------------------------------------------------------------------------------------------
//	Turn off the picture for the currently active mouse action mode. 
// --------------------------------------------------------------------------------------------------
void TPctsView::TurnAllOff (void)
	{
	TOpPict * activeOpPict;
	IDType itsIdentifier = (IDType) 'Ops1';
	
	// get the identifier for the currently active mouse action picture
	itsIdentifier += ((TCFDFrontDocument*)fDocument)->GetMouseAction();
	activeOpPict = (TOpPict*) FindSubView(itsIdentifier);
	activeOpPict->HiliteState(false, true);													// unhilite it
	}

// --------------------------------------------------------------------------------------------------
//	Draw the View 
// --------------------------------------------------------------------------------------------------
pascal void TPctsView::Draw(Rect * /*thisRect*/)											// Draws the view seen in the window. 
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
	this->Adorn(&tRect,pen,adnLineRight);
	HUnlock((Handle) this);
	this->Focus();
	return;
	} 

// --------------------------------------------------------------------------------------------------
//	If the SuperView is changed want to move TinformationView
// --------------------------------------------------------------------------------------------------
pascal void TPctsView::SuperViewChangedSize(VPoint *delta, Boolean /*invalidate*/)
	{
	VPoint newLoc;
	
	newLoc.h = fLocation.h;
	newLoc.v = fLocation.v;
	this->Locate(newLoc.h,newLoc.v,true);
	newLoc.h = fSize.h;
	newLoc.v = fSize.v + delta->v;
	this->Resize(newLoc.h,newLoc.v,true);
	return;	
	}

