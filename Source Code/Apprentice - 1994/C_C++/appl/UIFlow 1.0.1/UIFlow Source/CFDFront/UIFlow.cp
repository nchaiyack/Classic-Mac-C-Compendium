#pragma segment geometry
#include "Globals.h"
#include "NewStuff.h"
// #include "DTMClass.h"
#include "CFDFront.h"																				// all class headers

#include "CFunctions.c"
#include "TWarnDialog.cp"																		// error dialog

// #include "TFilterDialog.cp"
#include "TModelessDialog.cp"																	// modeless dialog base class
#include "TModalDialog.cp"																		// modal dialogs
#include "TDataView.cp"																			// segment data view class
#include "TInformationView.cp"																// info view at bottom of geom view

#include "TOptPrint.cp"																			// print options dialog
#include "TOptFlow.cp"																				// flow parameters dialog
#include "TOptSolution.cp"																		// solutions parameters dialog
#include "TOptRelax.cp"																			// relaxation parameters

#include "CFDFrontDocument.cp"
#include "TFortranView.cp"																// info view at bottom of geom view
#include "TPctsView.cp"
#include "TTrash.cp"

extern pascal void INITFORTRAN();
extern pascal void EXITFORTRAN();

// **********************************************************************
//	TGeomView Methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	init this from our own resource.
// --------------------------------------------------------------------------------------------------
pascal void TGeomView::IRes (TDocument * itsDocument, TView * itsSuperView, Ptr * itsParams)
	{
	TView::IRes(itsDocument, itsSuperView, itsParams);
	fDocument	= itsDocument;
	fCDocument	= (TCFDFrontDocument *) itsDocument;
	fDragging	=	true;
	fSPoint		=	NULL;																				// store the point
	fSSide		= 	NULL;
	fSegment	=	false;
	fHorizSize =	0;
	fVertSize	=	0;
	fMagnify	=	0;
	fMCount	=	0;
	return;
	}

// --------------------------------------------------------------------------------------------------
//	get the size of the geometry subview.
// --------------------------------------------------------------------------------------------------
void TGeomView::SetGeomViewSize (void)
	{
	if ( fCDocument->fWidth > fCDocument->fHeight)														// calculate the size of the view
		{
		fHorizSize = cMaxSize + 5;
		fCDocument->fUnitFraction = fCDocument->fWidth / cMaxSize;
		fVertSize = (short)((fCDocument->fHeight / fCDocument->fWidth) * (float)cMaxSize) + 5;
		}
	else	
		{
		fVertSize = cMaxSize +5;
		fCDocument->fUnitFraction = fCDocument->fHeight / cMaxSize;
		fHorizSize = (short) ((fCDocument->fWidth / fCDocument->fHeight) * (float)cMaxSize) + 5;
		}
	
	this->AdjustSize();																											// now resize the view.
	}

// --------------------------------------------------------------------------------------------------
//	get the size of the geometry subview.
// --------------------------------------------------------------------------------------------------
void TGeomView::MagnifyView (float mag)
	{
	VPoint 	newView;
	Rect 	oRect;
	
	oRect.top 			= 0;
	oRect.left			= 0;
	oRect.right		= (short) fSize.h;
	oRect.bottom	= (short) fSize.v;
	HLock((Handle) this);
	this->InvalidRect(&oRect);
	HUnlock((Handle) this);
	
	if (mag == 0)
		{
		newView.h = (cMaxSize + (2 * cGeomViewBorder)) ;
		newView.v = (cMaxSize + (2 * cGeomViewBorder));
		}
	else
		{
		newView.h = (cMaxSize + (2 * cGeomViewBorder)) * mag;
		newView.v = (cMaxSize + (2 * cGeomViewBorder)) * mag;
		}
	Resize(newView.h,newView.v,false);																			// now resize the view.
	}

// --------------------------------------------------------------------------------------------------
//	Draws the rectangle
// --------------------------------------------------------------------------------------------------
pascal void TGeomView::Draw(Rect * /*tRect*/)
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
	
	this->Adorn(&tRect,pen,adnLineRight);
	this->Adorn(&tRect,pen,adnLineBottom);
	HUnlock((Handle) this);
	fCDocument->fPointMatrix->DoDraw();
	}

// --------------------------------------------------------------------------------------------------
// 	calculate a minimum size
//--------------------------------------------------------------------------------------------------
pascal void TGeomView::CalcMinSize(VPoint * minSize)
	{	 
	minSize->h = fHorizSize + cGeomViewBorder*2;
	minSize->v = fVertSize + cGeomViewBorder*2;
	}

// --------------------------------------------------------------------------------------------------
//	Do the mouse command.
// --------------------------------------------------------------------------------------------------
pascal struct TCommand * TGeomView::DoMouseCommand(Point * theMouse, EventInfo *  eInfo,
							Point * /*hysteresis*/)
	{
	char strng[255];																										// temp string
	int n = 0;
	Point mMouse;
	
	mMouse = AntiTransform(* theMouse, fMagnify);
// what mouse action mode is available?
//
//	Get Mouse Action returns the palette number... telling the program which function is
//	currently being executed.
//
	fCDocument->fInfoView->ClrInfo();
	switch (fCDocument->GetMouseAction())
		{
		TPoint * tPt;																							// the point
		TBoundry * side;

		case cDragTool:																						// palette option #0
			HLock((Handle) this);
			if ((tPt = (TPoint *) (fCDocument->fPointMatrix)->FindPoint(theMouse)) == NULL) // on a point?
				{
				HUnlock((Handle) this);
				return gNoChanges;																			// LAM -- give message
				}
				
			HUnlock((Handle) this);
			this->Focus();
			fCDocument->fPointMatrix->DoHighlight();
			side = (TBoundry *) fCDocument->fPointMatrix->GetCurrentBoundry();
			this->UnSelect();																					// unselect the current stuff
			this->Select(tPt,false,side);																	// select new point

			fCDocument->fPointMatrix->DoHighlight();
			TDragCommand * nDrag;
			nDrag = new TDragCommand;																// allocate space
			if (nDrag == NULL)
				return gNoChanges;																			// LAM -- give message
			HLock((Handle) this);
			if (nDrag->IDragCommand(this,tPt,theMouse) == 1)								// init the dragger
				{
				HUnlock((Handle) this);
				return gNoChanges;																			// LAM -- give message
				}
			HUnlock((Handle) this);
			return nDrag;
//
// 	Add a Segment
//
		case cSegmentTool:																					// palette option #1
//
//	A BOUNDRY operation.....
//	the point does not already exist so create it.
//
			HLock((Handle) this);
			if ((tPt = (TPoint *) (fCDocument->fPointMatrix)->FindBoundryPoint (theMouse)) != NULL)
				{
				HUnlock((Handle) this);
				if (!tPt->IsCornor() && tPt->IsSegment())										// make sure is a segment
					{						
					TDeletePointCommand * deletePoint;												// create delete command object
					deletePoint = new TDeletePointCommand;
					if (deletePoint == NULL) {}															// LAM - error processing
					if (!deletePoint->IDeletePointCommand(this,tPt)) 							// init the delete object
						return gNoChanges;
					return deletePoint;
					}
				return gNoChanges;
				}
		
			HUnlock((Handle) this);
			TNewSegmentCommand * nSegment;														// declare command
			nSegment = new TNewSegmentCommand; 												// create a command object to handle this one.
				FailNIL (nSegment);																			// creation sucessful?
			nSegment->INewSegmentCommand (this, fCDocument);							// init the command object
			return nSegment;																					// return the command object to MacApp
//
// 	Add a Section
//
		case cGridTool:																							// palette option #2
//
//	A BOUNDRY operation.....
//	the point does not already exist so create it.
//
			HLock((Handle) this);
			if ((tPt = (TPoint *) (fCDocument->fPointMatrix)->FindBoundryPoint (theMouse)) != NULL)
				{
				HUnlock((Handle) this);
				if (!tPt->IsCornor() && tPt->IsBoundryPt() && !tPt->IsSegment())	// make sure on the boundry
					{						
					TDeleteGridCommand * deletePoint;												// create delete command object
					deletePoint = new TDeleteGridCommand;
					if (deletePoint == NULL) {}															// LAM - error processing
					if (!deletePoint->IDeleteGridCommand(this,tPt))						 	// init the delete object
						return gNoChanges;
					return deletePoint;
					}
				return gNoChanges;
				}
				
			HUnlock((Handle) this);
			TNewSectionCommand * nSection;														// declare command
			nSection = new TNewSectionCommand; 													// create a command object to handle this one.
				FailNIL (nSection);																// creation sucessful?
			nSection->INewSectionCommand (this, fCDocument);									// init the command object
			return nSection;																	// return the command object to MacApp

//
//		Baffle Operations
//
		case cBaffle:
			{
			short 		tDirection;
			PointInfo 	info;
			Boolean		t1, t2;
			
			HLock((Handle) this);
			tDirection = fCDocument->fPointMatrix->FindLine(&mMouse,&info);						// get associated segment
			HUnlock((Handle) this);
			if (tDirection == 0)
				break;
				
			t1 = TRUE;
			t2 = TRUE;
			if (info.above == NULL)
				t1 = FALSE;
			else if (info.above->fBaffBelow == NULL)
				t1 = FALSE;
			if (info.left == NULL)
				t2 = FALSE;
			else if (info.left->fBaffRight == NULL)
				t2 = FALSE;
			if ((tDirection == 1 && t1) || (tDirection == 2 && t2))
				{
				if (info.above->fBaffBelow == NULL)
					break;
				TDeleteBaffleCommand * dCommand;
				dCommand = new TDeleteBaffleCommand;
					FailNIL (dCommand);
				HLock((Handle) this);
				if (!(dCommand->IDeleteBaffleCommand (this, fCDocument, &info, tDirection)))	// initialize delete obstacle
					{
					HUnlock((Handle) this);
					return gNoChanges;
					}
				HUnlock((Handle) this);
				return dCommand;
				}
				
			TBaffleCommand * bCommand;
			bCommand = new TBaffleCommand;
				FailNIL (bCommand);																// LAM
			HLock((Handle) this);
			bCommand->IBaffleCommand (this, fCDocument, &info,tDirection);						// initialize command
			HUnlock((Handle) this);
			return bCommand;
			}
			break;
//
//		Obstacle Operations
//
		case cObstacle:
			{
			TPoint 						*	tPt;
			PointInfo 					info;
			
			HLock((Handle) this);
			tPt = fCDocument->fPointMatrix->FindCell(&mMouse,&info);							// get associated segment
			HUnlock((Handle) this);
			if (tPt == NULL)
				break;

			if (tPt->GetObsQuad(4) != NULL)														// already an obstacle
				{
				TDeleteObstacleCommand * dCommand;
				dCommand = new TDeleteObstacleCommand;
					FailNIL (dCommand);
				HLock((Handle) this);
				if (!(dCommand->IDeleteObstacleCommand (this, fCDocument, &info)))				// initialize delete obstacle
					{
					HUnlock((Handle) this);
					return gNoChanges;
					}
				HUnlock((Handle) this);
				return dCommand;
				}
		
			TObstacleCommand 	* oCommand;
			oCommand = new TObstacleCommand;
				FailNIL (oCommand);																			// LAM
			HLock((Handle) this);
			oCommand->IObstacleCommand(this, fCDocument,&info);
			HUnlock((Handle) this);
			return oCommand;
			}
//
//	View segment data
//
		case cData:																								// segment data
			{
			TSegPoint	* sPt;
			
			HLock((Handle) this);
			tPt = fCDocument->fPointMatrix->FindSection(&mMouse);						// returns boundry point is on
			HUnlock((Handle) this);
			if (tPt == NULL)
				{
				if (fSPoint != NULL && fSPoint->IsBoundryPt())
					fCDocument->fPointMatrix->DoHighlight();
				this->UnSelect();
				this->Select(fCDocument->fInterior,false,NULL);
				fCDocument->fDataView->ShowData(fCDocument->fInterior);
				break;
				}
			side = (TBoundry *) fCDocument->fPointMatrix->GetCurrentBoundry();
																														// also sets fRow & fCol to current section
			sPt = fCDocument->fPointMatrix->FindSegment(tPt,side);						// get associated segment
			
			if (sPt == NULL)																					// not on a boundry
				return gNoChanges;																			// return nothing

			if (fSPoint != NULL && !fSPoint->IsInterior())
				fCDocument->fPointMatrix->DoHighlight();
			this->UnSelect();																					// unselect the current stuff
		
			this->Select(sPt,true,side);																	// select new point
			fCDocument->fPointMatrix->DoHighlight();											// highlight the selection
			fCDocument->fDataView->ShowData(sPt);												// show the data
			break;
			}
//
//	Magnify
//
		case cMagnify:																							// segment data
			{
			if (!eInfo->theCmdKey)
				{
				if (fMCount == 7.5)
					{
					SysBeep (3);
					return gNoChanges;
					}
				fMCount += 1.5;
				}
			else
				{
				if (fMCount == -7.5)
					{
					SysBeep (3);
					return gNoChanges;
					}
				fMCount -= 1.5;
				}
			if (fMCount > 0)
				fMagnify = fMCount;
			else if (fMCount < 0)
				fMagnify = 1 / (-1 * fMCount);
			else
				fMagnify = 0;
			
			PointInfo 	info;
			TPoint 		* tPt;

			info.gridOnly 	= false;
			info.magnify		= fMagnify;
			info.lrc				= NULL;
			
			HLock((Handle) this);
			fCDocument->fPointMatrix->Each(TransformGrid,&info);
			tPt = (TPoint *) (fCDocument->fPointMatrix->fRight)->Last();
			tPt->TransformPoint(fMagnify);
			HUnlock((Handle) this);
			
			this->MagnifyView(fMagnify);
			this->ForceRedraw();
			break;
			}
		} 
	return gNoChanges;																						// no command.
	}

// --------------------------------------------------------------------------------------------------
// 	unselected the segment
//--------------------------------------------------------------------------------------------------
void TGeomView::UnSelect(void)
	{
	if (fSPoint != NULL)																						// highlighted object is point
		{
		if (fSegment)
			fCDocument->fPointMatrix->SelectSegment(fSPoint,fSSide,false);
		else
			fSPoint->SetSelection(false);

		fSPoint	=	NULL;																						// store the point
		fSSide	= 	NULL;
		fSegment = false;
		}
	}
	
// --------------------------------------------------------------------------------------------------
// 	selected the segment
//--------------------------------------------------------------------------------------------------
void TGeomView::Select(TPoint * tPt, Boolean seg, TRow * side)
	{
	fSPoint		=	tPt;																							// store the point
	fSSide		=	side;
	fSegment	=	seg;																							// store the boundry

	if (seg)
		fCDocument->fPointMatrix->SelectSegment(tPt,side,true);
	else
		tPt->SetSelection(true);
	return;
	}
	
// --------------------------------------------------------------------------------------------------
// 	change cursor?
//--------------------------------------------------------------------------------------------------
pascal Boolean TGeomView::DoSetCursor(Point localPoint, RgnHandle /*cursorRgn*/)
	{
	switch (fCDocument->GetMouseAction())
		{
		case cDragTool :
		case cGridTool :
		case cSegmentTool:
			HLock((Handle) this);
			if (fCDocument->fPointMatrix->FindPoint(&localPoint) == NULL)
				{
				HUnlock((Handle) this);
				InitCursor();
				return false;
				}
				
			HUnlock((Handle) this);
			SetCursor(*PointCursor);
			return true;
	
		case cMagnify:
			{
			KeyMap 	theKeys;
			Boolean		t;
			GetKeys(theKeys);
	
			t = BitTst(theKeys,(long) 48);
			if (t)
				SetCursor(*MagCursorDn);
			else
				SetCursor(*MagCursorUp);
			return true;
			}
		default:
			InitCursor();
			return false;
		} 
	return false;
	}
	
// --------------------------------------------------------------------------------------------------
//	get the size of the geometry subview.
// --------------------------------------------------------------------------------------------------
Point TGeomView::DragCurrent (Point newPoint)
	{
	Point savePt;																							// temp storage
	
	savePt.v = 0;
	savePt.h = 0;
	this->Focus();
	if (fSPoint != NULL)																					// if have a selected point
		{
		savePt = fSPoint->GetStart();
		TDragCommand * nDrag;
		nDrag = new TDragCommand;																// allocate space
			FailNIL(nDrag);
			
		HLock((Handle) this);
		if (nDrag->IDragCommand(this,fSPoint,&newPoint) == 1)						// init the dragger
			{
			HUnlock ((Handle) this);
			return savePt;																					// LAM -- give message
			}
		
		fCDocument->fPointMatrix->DoHighlight();											// unhighlight point
		nDrag->MovePoint();																			// move the point
		HUnlock ((Handle) this);
		}
	return savePt;
	}
	
// --------------------------------------------------------------------------------------------------
//	The application initializer.
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::ICFDFrontApplication(OSType itsMainFileType)
	{
	int i;
	
	IApplication(itsMainFileType);
	
	fLaunchWithNewDocument = FALSE;
// So my view will be substituted when MacApp¨ creates the "default view"
	RegisterStdType("\pTGeomView", 'DFLT');
	
	if (gDeadStripSuppression)															// So the linker doesn't dead strip class info 
		{
		TWarning * warnMe = new TWarning;
	  	TInformationView *infoView = new TInformationView;
	  	TFortranView *fortView = new TFortranView;
		TGeomView *aGeomView = new TGeomView;
		TOpPict * aOpPict = new TOpPict;
		TPctsView * aPctsView = new TPctsView;
		TDimension * aDimension = new TDimension;
		TDataView * aDataView = new TDataView;
		TOptSolution * aSolution = new TOptSolution;
		TOptPrint * anOptPrint = new TOptPrint;
		TOptFlow * anOptFlow = new TOptFlow;
		TOptSolution * anOptSolution = new TOptSolution;
		TBoundryRadius * aRadius = new TBoundryRadius;
		TPremix * aMix = new TPremix;
		TDiffusion * aDiffusion = new TDiffusion;
		TTurbulence * aTurbulence = new TTurbulence;
		TRelax * aRelaxation = new TRelax;
		TPressure * aPressure = new TPressure;
		TWarnDelete * dWarn = new TWarnDelete;
		TCTrash * trash = new TCTrash;
		TAppDialog * app = new TAppDialog;
		TImageDialog * img = new TImageDialog;
		}
	  
	for (i=0; i < NumSegmentTypes; i++)
		GetIndPattern (SegPatterns[i], 0, pat[i]);
		
	fReadOld = false;
	}

// --------------------------------------------------------------------------------------------------
//	Watch for magnification icon
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::OpenOld(CmdNumber itsOpenCmd, AppFile *anAppFile)
	{
	fReadOld = true;
	inherited::OpenOld(itsOpenCmd,anAppFile);
	fReadOld = false;
	}

// --------------------------------------------------------------------------------------------------
//	Watch for magnification icon
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::Idle(IdlePhase phase)
	{
	TWindow 		* 	tWindow;
	TGeomView	*	tGeom;
	tWindow 	= (TWindow *) this->GetActiveWindow();
	if (tWindow == NULL)
		{
		inherited::Idle(phase);
		return;
		}
	tGeom		= (TGeomView *) tWindow->FindSubView('geom');
	if (tGeom == NULL)
		{
		inherited::Idle(phase);
		return;
		}
	
	if (((TCFDFrontDocument *) (tGeom->fCDocument))->GetMouseAction() == cMagnify)
		{
		KeyMap 	theKeys;
		Boolean		t;
	
		GetKeys(theKeys);
		t = BitTst(theKeys,(long) 48);
		if (t)
			SetCursor(*MagCursorDn);
		else
			SetCursor(*MagCursorUp);
		}

	inherited::Idle(phase);
	}
	
// --------------------------------------------------------------------------------------------------
//	Enable and disable menu items for this kind of application
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::DoSetupMenus (void)
	{
	// enable new
	inherited::DoSetupMenus();
	for (short menuIndex = cFirst; menuIndex <= cLast; menuIndex++)
		Enable(menuIndex,true);
	Enable(cPrintOne,false);
	Enable(cPageSetup,false);
	Enable(cPrint,false);
	Enable(cSelectAll,false);
	Enable(cShowClipboard,false);
	}

// --------------------------------------------------------------------------------------------------
//	Show About UIFlow Tool
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::DoShowAboutApp(void)
	{
	Str255	tmp;
	StringHandle strHandle;
	DialogPtr theSplash;
	
	// get the splash screen
	tmp[0] = NULL;
	strHandle = StringHandle (GetResource ('SPFW',0));
	if (strHandle == NULL)
		exit(0);
	ParamText (*strHandle, tmp, tmp, tmp);
	
	theSplash = GetNewCenteredDialog (1000, NULL, (WindowPtr) -1);
	
	if (theSplash)
		{
		SetPort (theSplash);
		DrawDialog (theSplash);
		}
	else
		SysBeep (3);
		
// ditch the splash screen.
	EventRecord theEvent;
	Boolean 		loop = TRUE;
	while (loop)
		{
		GetOSEvent(keyDownMask+mDownMask,&theEvent);
		if (theEvent.what == keyDown || theEvent.what == mouseDown)
			loop = FALSE;
		}

	if (theSplash)
		DisposDialog (theSplash);
	}
		
// --------------------------------------------------------------------------------------------------
//	Enable and disable menu items for this kind of application
// --------------------------------------------------------------------------------------------------
pascal void TCFDFrontApplication::SetUndoText(Boolean cmdDone, CmdNumber aCmdNumber)
	{
	StringPtr pString;
	char			string[80];
	short		i;
	
	if (cmdDone)
		i = sprintf(string,"Undo ");
	else
		i = sprintf(string,"Redo ");
		
	switch (aCmdNumber)
		{
		case cMakeBaffle:
			sprintf(string+i,"Create Baffle");
			break;
		case cMakeObstacle:
			sprintf(string+i,"Create Obstacle");
			break;
		case cDragPointCommand:
			sprintf(string+i,"Drag Point");
			break;
		case cNewSegCommand:
			sprintf(string+i,"Create Segment");
			break;
		case cNewSecCommand:
			sprintf(string+i,"Create Section");
			break;
		case cDeleteSegCommand:
			sprintf(string+i,"Delete Segment");
			break;
		case cDeleteSecCommand:
			sprintf(string+i,"Delete Section");
			break;
		case cDeleteBaffle:
			sprintf(string+i,"Delete Baffle");
			break;
		case cDeleteObstacle:
			sprintf(string+i,"Delete Obstacle");
			break;
		default:
			inherited::SetUndoText(cmdDone,aCmdNumber);
			return;
		}
	pString = c2pstr(string);
	SetCmdName(cUndo,pString);
	gUndoState	=	cmdDone;
	gUndoCmd		=	aCmdNumber;
	return;
	}

// -------------- main program -----------------------------------
// GLOBALS

#pragma segment Main

void main ()
	{
	Str255	tmp;
	StringHandle strHandle;
	DialogPtr theSplash;

	InitToolBox();																						// init the mac tool box
	PullApplicationToFront();																		// bring the application to front
	
// does the mac have the hardware / software to run this program
	if (ValidateConfiguration (&gConfiguration))
		{
// get the version number for the program.
		tmp[0] = NULL;
		strHandle = StringHandle (GetResource ('SPFW',0));
		if (strHandle == NULL)
			exit(0);
		ParamText (*strHandle, tmp, tmp, tmp);
		
// get the splash screen
		theSplash = GetNewCenteredDialog (1000, NULL, (WindowPtr) -1);

		if (theSplash)
			{
			SetPort (theSplash);
			DrawDialog (theSplash);
			}
		else
			SysBeep (3); 
			
// init the MacApp stuff. 
		
		INITFORTRAN();
		InitUMacApp (8);																					// mac app mem management. 
		InitUTEView ();																						// init the text edit stuff. 
		InitUGridView ();																					// init the grid list manager. 	
		InitUDialog ();																						// init the dialog manager. 
		InitColorPattern();																				// initialize the color pix pattern
		
		PointCursor = GetCursor(128);
		HLock((Handle) PointCursor);
		MagCursorUp = GetCursor(131);
		HLock((Handle) MagCursorUp);
		MagCursorDn = GetCursor(132);
		HLock((Handle) MagCursorDn);

//--------------------------------------------------------------------------------------------------------		
// Allocate a new Application object, and check for errors
//--------------------------------------------------------------------------------------------------------				
		gUnloadAllSegs = false;
		gCFDFrontApplication = new TCFDFrontApplication;
		FailNIL(gCFDFrontApplication);
		
// Initialize the application object
		gCFDFrontApplication->ICFDFrontApplication(kFileType);

// ditch the splash screen.
		if (theSplash)
			DisposDialog (theSplash);
			
// Run the application. When it's done, exit.
		gCFDFrontApplication->Run();
		EXITFORTRAN();
		}		
}
		
		