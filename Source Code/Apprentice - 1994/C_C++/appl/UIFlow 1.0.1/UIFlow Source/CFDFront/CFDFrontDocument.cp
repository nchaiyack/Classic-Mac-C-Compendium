#pragma segment UIFlow
extern pascal int UIFLOW();
extern "C" int ConvertVSet2Raster(char *);
extern "C" int DFR8restart();
extern "C" int DFR8getdims(char*,long*,long*,Boolean*);
extern "C" int DFR8getimage(char*,char*,long,long,char*);
extern "C" int DFR8lastref();
extern "C" int DFANgetlabel(char*,int,int,char*,long);
#include "::HDF3.1r4:Src:df.h"


EventRecord * gEvent;
short gStop;
//**********************************************************************
//	The TCFDFrontDocument class methods.	
//**********************************************************************
//	Prandlt Numbers	[0]	=	u
//								[1]	=	v
//								[2]	=	pressure
//								[3]	=	w
//								[4]	=	sigma h
//								[5]	=	kinetic energy
//								[6]	=	dissipation rate
//								[7]	=	sigma f
//								[8]	=	sigma fu
//								[9]	=	sigma g
//								[10]	=	density
// -----------------------------------------------------------------------------------------
//	Make the view for the document. This will create the geometry window
// -----------------------------------------------------------------------------------------
Boolean TCFDFrontDocument::ICFDFrontDocument (Boolean /*forPrinting*/)
{
	IDocument(kFileType, 																	// This document's file type.
			  kSignature, 																		// This document's creator.	
			  kUsesDataFork, 																	// This document does use the data fork	
			  ~kUsesRsrcFork,																	// Ébut doesn't use the resource fork.	
			  ~kDataOpen,																			// We don't want the data fork kept open
			  ~kRsrcOpen);																		// Énor the resource fork.	

	fRemoteApplication[0] = 0;
	fImageName[0]					=	0;
	fDataOpen 	= FALSE;
	fGridOperation = false;
	fPrintOpts	= 0L;																				// set all print options to off
	fDialogOn		= 0L;	
	fFlowOpts		= 5457L;																		// set the correct radio buttons
	fSolution[0]	= fSolution[1] = fSolution[2] = fSolution[3] = fSolution[4] = 1L;
	fRelax[0]		= fRelax[1] = fRelax[3] = fRelax[4] = fRelax[5] = fRelax[6] = .5;
	fRelax[2] 		= .8;
	fPran[0]		= fPran[1] = fPran[2] = fPran[3] = fPran[5] = fPran[10] = 1.0;
	fPran[4]		= .7;
	fPran[6]		= 1.3;
	fPran[7]		= fPran[8] = fPran[9] = 0.9;
	fBreakup		= 3.0;
	fFuel 			= 0;
	fTempFuel	= fTempAir	= 300.00;
	fSolutionAccuracy	 	= 0.0005;
	fIteration					= 100;
	fBoundryRadius			= 0;
	fPressure						= 1.0000E+05;													// pressure
	fCompress						= 0;																	// incompressible
	fWidth							= 0;
	fHeight							= 0;
	fUnitFraction				= 0;
	fInterior						= NULL;
	gFortranView				= NULL;

	fMakeView						= false;
	fExport							= 0;
	gEvent							= NULL;

	TWindow * aWindow;
	aWindow = NewTemplateWindow(kCFDFrontWindowId, (TDocument *)this);	// create geomview
	FailNIL (aWindow);																				// enough memory?
		
	fGeom = (TGeomView *)(aWindow->FindSubView ('geom'));			// find geom subview
	fPointMatrix	= new TPointMatrix;													// create the matrix list
	FailNIL(fPointMatrix);																		// error?
	fPointMatrix->IPointMatrix(this,fGeom);										// initialize the list

	fInterior = new TInteriorPoint;														// create a pt which represents interior
	fInterior->IPoint(0,0,Grid,0);														// initialize new point
		
	return true;
	}

// -----------------------------------------------------------------------------------------
//	Make the view for the document. This will create the geometry window
// -----------------------------------------------------------------------------------------
TCFDFrontDocument::TCFDFrontDocument ()	
	{
	fMouseAction = 0;
	}

// -----------------------------------------------------------------------------------------
//	Free all things allocated with New
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::Free(void)
	{
//	FreeIfObject(fPointMatrix);
//	FreeIfObject(fInterior);
	inherited::Free();
	}
	
// --------------------------------------------------------------------------------------------------
//	this routine will make a new geometry document
// --------------------------------------------------------------------------------------------------
pascal struct TDocument * TCFDFrontApplication::DoMakeDocument (CmdNumber /*itsCmdNumber*/)
	{
	TCFDFrontDocument	*anCFDFrontDocument;
	
	anCFDFrontDocument = new TCFDFrontDocument;	
	anCFDFrontDocument->ICFDFrontDocument(false);											// Énor the resource fork.	
			
	FailNIL(anCFDFrontDocument);	
	return (struct TDocument *) anCFDFrontDocument;
	}

// -----------------------------------------------------------------------------------------
//	Make the view for the document. This will create the geometry window
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::DoMakeViews (Boolean /*forPrinting*/)
	{
	TWindow		*aWindow;
	Point 			miSize, maSize;
		
//	if (fMakeView)
//		return;
		
	if (!gCFDFrontApplication->fReadOld)
		this->GetDims();																				// get the dimensions of the geometry
	
	aWindow = (TWindow *) fGeom->GetWindow();
	fGeom->SetGeomViewSize ();																// set the geometry size
	maSize.h = aWindow->fResizeLimits.right;									// restrict the size user can
	maSize.v = aWindow->fResizeLimits.bottom;									// shrink the window to.
	miSize.h  = 300;
	miSize.v = 300;
	aWindow->SetResizeLimits(miSize,maSize);
	if (!gCFDFrontApplication->fReadOld)
		fPointMatrix->InitCorners();

	fDataView = (TDataView *)(aWindow->FindSubView('DATA'));	// init the data view object
	fDataView->IDataView();																		// init the data view
	
	fInfoView = (TInformationView *)(aWindow->FindSubView ('info'));	// create TInformationView
	fInfoView->IInformationView(this);
	gInfoView = fInfoView;
	
	fMakeView = true;
	}
	
// -----------------------------------------------------------------------------------------
//	set the mouse action mode.
// -----------------------------------------------------------------------------------------
void TCFDFrontDocument::SetMouseAction (IDType op)	
	{
	// fMouseAction is the zero based index that indicates what
	//	mouse action mode we are in.
	fMouseAction = (short) ((long) op  - (long) 'Ops1');
	}

// -----------------------------------------------------------------------------------------
//	Enable and disable menu items for this kind of document
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::DoSetupMenus (void)
	{
//	 Enable the new menu item
	inherited::DoSetupMenus();
	for (short menuIndex = cFirst; menuIndex <= cLast; menuIndex++)
		Enable(menuIndex,true);
	}

// -----------------------------------------------------------------------------------------
// 	Convert view coordinate to real coordinates
// -----------------------------------------------------------------------------------------
void TCFDFrontDocument::ViewToReal (Point thePoint, struct realPt * realNum)
	{
	realNum->x = (float) (thePoint.h - cGeomViewBorder);			// convert the x component
	realNum->x *= fUnitFraction;
	realNum->y = (float) (cMaxSize+cGeomViewBorder - thePoint.v);	// convert the y component
	realNum->y *= fUnitFraction;
	return;
	}

// -----------------------------------------------------------------------------------------
// 	Convert real coordinate to view coordinates
// -----------------------------------------------------------------------------------------
Point TCFDFrontDocument::RealToView (struct realPt * realNum)
	{
	Point newPoint;
	char XP[20], YP[20];
	
	sprintf(XP,"%.4f",realNum->x);														// store values for warning message
	sprintf(YP,"%.4f",realNum->y);
	realNum->x /= fUnitFraction;															// convert the x component
	realNum->x += (float) cGeomViewBorder;
	realNum->y /= (fUnitFraction * -1);												// convert the y component
	realNum->y += (float) (cMaxSize+cGeomViewBorder);

	newPoint.h = (short) realNum->x;													// store x in newPoint
	newPoint.v = (short) realNum->y;													// store y in newPoint
		
	if (newPoint.h > cMaxSize || newPoint.v > cMaxSize)				// within boundries?
		{
		TWindow	* aWindow;																			// warning window
		TWarning	* theWarning;																	// warning object
		char warning[40];																				// no : give warning message
		sprintf(warning,"%s = %d  :   %s = %d",XP,newPoint.h,YP,newPoint.v); // create warning message
		aWindow = NewTemplateWindow(kWarnMe,this);							// create window
		theWarning = (TWarning *) aWindow->FindSubView('WARN');	// get pointer to object
		theWarning->IWarning(0,warning);												// initialize the warning
		theWarning->ShowWarning();															// display the warning
		newPoint.v = NULL;																			// set NULL for return
		}
	return newPoint;																					// return the point
	}
		
// -----------------------------------------------------------------------------------------
//	Process the Menu Commands
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::Close(void)
	{
	fPass			= 1;
	fExport			= 0;
	fWriteGrid	= true;
	inherited::Close();
	}
	
// -----------------------------------------------------------------------------------------
//	Process the Menu Commands
// -----------------------------------------------------------------------------------------
pascal TCommand* TCFDFrontDocument::DoMenuCommand(CmdNumber aCmdNumber)
	{
	switch (aCmdNumber)
		{
		case cSave:
		case cSaveAs:
		case cSaveCopy:
			{
			fOutput = new TFOutput;
			if (fOutput == NULL)																// LAM - error message
				break;
			
			fOutput->IOutput(this,true);
			fPass			= 1;
			fExport			= 1;
			fWriteGrid	= true;
			return inherited::DoMenuCommand(aCmdNumber);
			}
		case cExport:
			{
			char		sTitle[255], *s;													// temp storage file name
			char		tTitle[225];
			char		gTitle[225];
			char 	msg[80];
			short	sRefNum, bRef, bRsrc;												// temp storage volume reference

			StringPtr volume, name, gname;
			short refNum;
			OSErr	error;
			
			fOutput = new TFOutput;
			if (fOutput == NULL)															// LAM - error message
				break;
			
			fOutput->IOutput(this,false);

			sprintf(msg,"Creating UIFlow.In & UIFlow.Grid"); 	// give message
			fInfoView->InfoString(msg);
			
			bRef 	= fDataRefnum;
			bRsrc	= fRsrcRefnum;
			sprintf(msg,"Export Operation Complete");		 			// give message
//			strcpy(sTitle,(char *) fTitle);										// store the file name
			s = p2cstr(*fTitle);
			strcpy(sTitle,s);
			     
			sprintf(tTitle,"UIFlow.In");
			name = c2pstr(tTitle);
			sprintf(gTitle,"UIFlow.Grid");
			gname = c2pstr(gTitle);

			fFileType = 'TEXT';
			this->SetTitle(name);															// store the name	
			volume		= c2pstr("test");
			sRefNum	= fVolRefNum;															// store the volume
			HLock((Handle) this);
			GetVol(volume,&refNum);														// get volumne information
			HUnlock((Handle) this);
			fVolRefNum = refNum;															// store volume

			error 			= FSDelete(name,refNum);							// delete old input deck
			fPass				= 1;
			fWriteGrid	= false;															// write on second pass
			fExport			= 2;
			this->Save(aCmdNumber,false,true);								// save file

			this->SetTitle(gname);														// store the name

			error = FSDelete(gname,refNum);										// delete old input deck
			fChangeCount++;																		// flag to save
			fPass			= 2;
			fWriteGrid	= true;																// write on second pass
			this->Save(aCmdNumber,false,true);								// save file
			
			fFileType		=	kFileType;
			name 				= c2pstr(sTitle);
			this->SetTitle(name);															// store the name	
			fPass				= 1;
			fExport			= 0;
			fWriteGrid	= false;
			
			CloseFile(fDataRefnum, fRsrcRefnum);
			fDataRefnum = bRef;
			fRsrcRefnum	= bRsrc;
			
			FreeIfObject(fOutput);
			fInfoView->InfoString(msg);
			break;
			}
		case cGridOn:																				// create grid option
			{
			char			string[80];
			StringPtr	pString;
				
			if (!fPointMatrix->IsShown())
				sprintf(string,"Hide Grid");
			else
				sprintf(string,"Show Grid");				

			pString = c2pstr(string);
			SetCmdName(cGridOn,pString);
			if (fGeom->fSPoint != NULL)
				{
				if (fGeom->fSPoint->IsInterior())
					{
					fPointMatrix->DoHighlight();
					fGeom->UnSelect();
					}
				}
			fPointMatrix->DoDrawGrid();
			break;
			}			
		case cOptPrint :
			this->MakeOptPrint();
			break;
		case cOptRelax :
			this->MakeOptRelax();
			break;
		case cOptSolution :
			this->MakeOptSolution();
			break;
		case cOptFlow :
			this->MakeOptFlow();
			break;
		
		case cRApp :
			this->GetRemoteApp();
			break;

		case cRemote :
			{
			}
			
		case cLocal:
			{
			char		sTitle[255], *s;															// temp storage file name
			char		tTitle[225];
			char		gTitle[225];
			char		vTitle[225];
			char		image[225];
			char 	msg[80], msg1[80], msg2[80], msg3[80], msg4[80];
			short	sRefNum, bRef, bRsrc;												// temp storage volume reference
			StringPtr volume, name, gname, vname;
			short refNum;
			OSErr	error;
			
			this->GetImageName();
			fOutput = new TFOutput;
			if (fOutput == NULL)															// LAM - error message
				break;
			
			fOutput->IOutput(this,false);

			sprintf(msg,"Creating UIFlow.In & UIFlow.Grid"); 	// give message
			fInfoView->InfoString(msg);
			
			bRef 	= fDataRefnum;
			bRsrc	= fRsrcRefnum;

			sprintf(msg,"Export Operation Complete");		 			// give message
			sprintf(msg1,"Converting Output to Raster Image");// give message
			sprintf(msg2,"Raster Image is Available in File %s",fImageName);// give message
			sprintf(msg3,"VSet File is Corrupted");						// give message
			sprintf(msg4,"Unable to Create Raster");					// give message
			s = p2cstr(*fTitle);
			strcpy(sTitle,s);
			sprintf(tTitle,"UIFlow.In");
			name = c2pstr(tTitle);
			sprintf(gTitle,"UIFlow.Grid");
			gname = c2pstr(gTitle);
			sprintf(vTitle,"vset.out");
			vname = c2pstr(vTitle);

			fFileType = 'TEXT';
			this->SetTitle(name);															// store the name	
			volume		= c2pstr("test");
			sRefNum	= fVolRefNum;															// store the volume
			HLock((Handle) this);
			GetVol(volume,&refNum);														// get volumne information
			HUnlock((Handle) this);
			fVolRefNum = refNum;															// store volume

			error = FSDelete(vname,refNum);										// delete the old vset
			error = FSDelete(name,refNum);										// delete old input deck
			fPass			= 1;
			fWriteGrid	= false;															// write on second pass
			fExport			= 2;
			this->Save(aCmdNumber,false,true);								// save file

			this->SetTitle(gname);														// store the name

			error = FSDelete(gname,refNum);										// delete old input deck
			fChangeCount++;																		// flag to save
			fPass			= 2;
			fWriteGrid	= true;																// write on second pass
			this->Save(aCmdNumber,false,true);								// save file
			
			fFileType		=	kFileType;
			name 				= c2pstr(sTitle);
			this->SetTitle(name);															// store the name	
			fPass				= 1;
			fExport			= 0;
			fWriteGrid	= false;
			
			CloseFile(fDataRefnum, fRsrcRefnum);
			fDataRefnum = bRef;
			fRsrcRefnum	= bRsrc;
			
			FreeIfObject(fOutput);
			fInfoView->InfoString(msg);

			if (gFortranView == NULL)
				this->MakeFortranWindow();
			else
				{
				TWindow * aWindow;	
				char  string[20];
				StringPtr pString;
				
				aWindow = gFortranView->GetWindow();
				aWindow->Select();
				string[0] = 0;																		// convert the data to string
				pString = c2pstr(string);													// convert to pascal type
				gFortranView->SetText(pString);
				}

			Boolean  	ispal;
			long			i, j;
			int			 	ref;
			char			label[79];
			char 			* data;
//			TDataTransport * dTm;																// dtm connection

			gEvent = new EventRecord;
			gStop  = 0;
  		HLock((Handle) gFortranView);
				UIFLOW();
  		HUnlock((Handle) gFortranView);
			delete gEvent;
			if (gStop == 0)
				{
				fInfoView->InfoString(msg1);
				switch (ConvertVSet2Raster(fImageName))
					{
					case -1:
						fInfoView->InfoString(msg4);
						FailNIL (0);
						break;
					case -2:
						fInfoView->InfoString(msg3);
						FailNIL (0);
						break;
					} 
						
/*				data[0] = 0;
	  		HLock((Handle) this);
				dTm = new TDataTransport;
				FailNIL(data);
				
				DFR8restart();  																	// start from the begining of the file
			 	while (0 <= DFR8getdims(fImageName, &i, &j, &ispal))	
				 	{
					data = new char[i*j+10];
					FailNIL(data);
					DFR8getimage(fImageName, data, i, j, NULL);
					if (ispal)
						{}																						// send palette - is none
					ref = DFR8lastref ();
					DFANgetlabel (fImageName, (int) DFTAG_RIG, ref, label,80);
					dTm->Send2DByteData(data,(int)i,(int)j,label);
					}
	  		HUnlock((Handle) this); */
				}
			else
				fInfoView->InfoString(msg2);
			
			break;
			}
			
		case cQuit:
			return inherited::DoMenuCommand(aCmdNumber);
					
		default :
			return inherited::DoMenuCommand(aCmdNumber);				// all others
		}			
	return gNoChanges;
	}

pascal void TCFDFrontDocument::Save(CmdNumber itsCmdNumber, Boolean askForFilename, Boolean makingCopy)
	{
	inherited::Save(itsCmdNumber,askForFilename,makingCopy);
	if (fExport == 1)
		{
		FreeIfObject(fOutput);																// free the space
		fOutput = NULL;																				// null the var
		}
	}
	
pascal void TCFDFrontDocument::ReadFromFile(AppFile *anAppFile, Boolean forPrinting)
	{
//	this->DoMakeViews(false);															// make view
	inherited::ReadFromFile(anAppFile,forPrinting);
	}
	
// -----------------------------------------------------------------------------------------
//	Read the file
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::DoRead(short aRefNum, Boolean /*rsrcExists*/, Boolean /*forPrinting*/)
	{
	char 	* buff, s[80];
	Boolean	flowOpts[15], printOpts[13];
	short	i, t1;
	short	gRow, gColumn;																			// number rows & columns
	long		t2;
	WallRecord data;
		
	flowOpts[0] = flowOpts[1] = flowOpts[2] = flowOpts[3] = flowOpts[4] = flowOpts[5] = false;
	flowOpts[6] = flowOpts[7] = flowOpts[8] = flowOpts[9] = flowOpts[10] = flowOpts[11]  = false;
	flowOpts[12] = flowOpts[13] = flowOpts[14] = false;
	printOpts[0] = printOpts[1] = printOpts[2] = printOpts[3] = printOpts[4] = printOpts[5] = printOpts[6] = false;
	printOpts[7] = printOpts[8] = printOpts[9] = printOpts[10] = printOpts[11] = printOpts[12] = false;

//	FailOSErr(GetEOF(aRefNum, &numChars));									// Read in the text 

	buff = new char[514];
	FailMemError();
	buff[0]	= 0;

//	line 1 		....................................................................
	this->ReadLine(aRefNum, buff, s);													// fWidth
	fWidth = atof(s);
	this->ReadLine(aRefNum, buff, s);													// fHeight
	fHeight = atof(s);
	this->ReadLine(aRefNum, buff, s);													// fUnitFraction
	fUnitFraction = atof(s);

	this->ReadLine(aRefNum, buff, s);													// klam	-	laminar flow
	t1 = atoi(s);
	if (t1 == 0)																							// set laminar
		flowOpts[5] = true;
	else																											// set turbulent
		flowOpts[4] = true;
		
	this->ReadLine(aRefNum, buff, s);													// kcomp	-	compressible flow
	t1 = atoi(s);
	fCompress = t1;
		
	this->ReadLine(aRefNum, buff, s);													// kswrl	-	swirl / nonswirl
	t1 = atoi(s);
	if (t1 == 0)
		flowOpts[8] = true;																			// nonswirling
	else
		flowOpts[9] = true;																			// swirling
	
	this->ReadLine(aRefNum, buff, s);													// kpgrid	-	UIFlow Grid / external grid
	t1 = atoi(s);
	if (t1 == 0)
		flowOpts[11] = true;
	else
		flowOpts[10] = true;
		
	this->ReadLine(aRefNum, buff, s);													// model	-	model number
	t1 = atoi(s);
	flowOpts[t1] = true;																			// turn on model
	
//	line 2 		....................................................................
	this->ReadLine(aRefNum, buff, s);														// kfuel	-	fuel type
	t1 = atoi(s);
	fFuel = t1;
	
	this->ReadLine(aRefNum, buff, s);														// knorth	-	always 0 - not stored
	this->ReadLine(aRefNum, buff, s);														// kplax	-	planar[1] / axisymmetric [0]
	t1 = atoi(s);
	if (t1 == 0)
		flowOpts[7] = true;
	else
		flowOpts[6] = true;
		
	this->ReadLine(aRefNum, buff, s);														// kadjst	-	Blocked adjustment
	t1 = atoi(s);
	if (t1 == 0)
		flowOpts[13] = true;
	else
		flowOpts[12] = true;
	
//	line 3 		....................................................................
	this->ReadLine(aRefNum, buff, s);														// ngrid	-	number of fine grid cells
	t2 = atol(s);
	fSolution[0] = t2;
	t2--;
	
	this->ReadLine(aRefNum, buff, s);														// ncelx	-	number of columns
	t1 = atoi(s);
	gColumn = (short) (t1 / (pow(2,t2))) + 1;
	
	this->ReadLine(aRefNum, buff, s);														// ncely 	-	number of rows
	t1 = atoi(s);
	gRow = (short) (t1 / (pow(2,t2))) + 1;
	
	
//	line 4 +		....................................................................
	fPointMatrix->ReadSegment(cLeft, aRefNum, buff, gRow);
	fPointMatrix->ReadSegment(cRight, aRefNum, buff, gRow);
	fPointMatrix->ReadSegment(cBottom, aRefNum, buff, gColumn);
	fPointMatrix->ReadSegment(cTop, aRefNum, buff, gColumn);
	fPointMatrix->FillGrid(gRow, gColumn);
	
//	Interior Data	....................................................................
	this->ReadLine(aRefNum, buff, s);															// u			-	u velocity
	data.u = atof(s);
	this->ReadLine(aRefNum, buff, s);															// v			-	v velocity
	data.v = atof(s);
	this->ReadLine(aRefNum, buff, s);															// w		-	w velocity
	data.w = atof(s);
	this->ReadLine(aRefNum, buff, s);															// rh		-	density
	data.density = atof(s);
	this->ReadLine(aRefNum, buff, s);															// t			-	temperature
	data.temp = atof(s);
	this->ReadLine(aRefNum, buff, s);															// tk		-	kinetic energy
	data.kenergy = atof(s);
	this->ReadLine(aRefNum, buff, s);															// td		-	dissipation
	data.dissip = atof(s);
	this->ReadLine(aRefNum, buff, s);															// f			-	mixture fraction
	data.mixfrac = atof(s);
	this->ReadLine(aRefNum, buff, s);															// g			-	concentration fraction
	data.concfrac = atof(s);
	this->ReadLine(aRefNum, buff, s);															// fu		-	fuel fraction
	data.fuelfrac = atof(s);
	
	this->ReadLine(aRefNum, buff, s);															// tfuel	- 	temperature fuel
	fTempFuel = atof(s);
	this->ReadLine(aRefNum, buff, s);															// tair		-	temperature air
	fTempAir = atof(s);
	
//	Prandtl Numbers	....................................................................
	for (i = 0; i < 11; i++)																			// read prandtl numbers
		{
		this->ReadLine(aRefNum, buff, s);														// get the value
		fPran[i] = atof(s);																			// convert number
		}
	for (i = 0; i < 11; i++)																			// extra line prandtl numbers
		this->ReadLine(aRefNum, buff, s);
		
//	Relaxation Numbers	....................................................................
	this->ReadLine(aRefNum, buff, s);															// relx1	-	u velocity
	fRelax[0] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx2	-	v velocity
	fRelax[1] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx3	-	pressure
	fRelax[2] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx4	-	w velocity
	fRelax[3] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx5	-	h = scalar
	fRelax[6] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx6	-	kinetic energy
	fRelax[4] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// relx7	-	dissipation = kinetic energy
	this->ReadLine(aRefNum, buff, s);															// relx8	-	f = scalar
	this->ReadLine(aRefNum, buff, s);															// relx9	-	fu = scalar
	this->ReadLine(aRefNum, buff, s);															// relx10	-	g = scalar
	this->ReadLine(aRefNum, buff, s);															// relx11	-	density
	fRelax[5] = atof(s);
	
//	Sweep Numbers	....................................................................
	this->ReadLine(aRefNum, buff, s);															// nswp1	-	momentum
	fSolution[1] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// nswp2	-	momentum
	this->ReadLine(aRefNum, buff, s);															// nswp3	-	pressure corrections
	fSolution[2] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// nswp4	-	momentum
	this->ReadLine(aRefNum, buff, s);															// nswp5	-	scalar
	fSolution[3] = atof(s);
	this->ReadLine(aRefNum, buff, s);															// nswp6	-	scalar
	this->ReadLine(aRefNum, buff, s);															// nswp7	-	scalar
	this->ReadLine(aRefNum, buff, s);															// nswp8	-	scalar
	this->ReadLine(aRefNum, buff, s);															// nswp9	-	scalar
	this->ReadLine(aRefNum, buff, s);															// nswp10-	scalar
	this->ReadLine(aRefNum, buff, s);															// nswp11-	scalar

//	Print Variables	....................................................................
	for (i = 0; i < 12; i++)
		{
		this->ReadLine(aRefNum, buff, s);														// iprint	-	print variables
		t1 = atoi(s);
		if (t1 == 1)
			printOpts[i] = true;
		}
	HLock((Handle) this);
	this->EncodeParam(14,flowOpts,&fFlowOpts);										// encode the booleans
	this->EncodeParam(12,printOpts,&fPrintOpts);	
	HUnlock((Handle) this);

//	Miscellaneous Variables	....................................................................
	this->ReadLine(aRefNum, buff, s);															// refp		-	system pressure
	fPressure = atol(s);
	this->ReadLine(aRefNum, buff, s);															// vscty	-	internal viscosity
	data.visc 	= atof(s);
	fInterior->SetData(data);																		// store the data
	this->ReadLine(aRefNum, buff, s);															// maxi	-	maximum iterations
	fIteration = atoi(s);
	this->ReadLine(aRefNum, buff, s);															// tolr		-	solution accuracy
	fSolutionAccuracy = atof(s);
	this->ReadLine(aRefNum, buff, s);															// rn		-	boundry radius
	fBoundryRadius = atof(s);
	
//	Obstacles & Baffles		....................................................................
	fPointMatrix->ReadObstructions(aRefNum, buff);

//	Grid Point Locations		....................................................................
	fPointMatrix->ReadGrid(aRefNum, buff);
	delete buff;
	return;
	}

// -----------------------------------------------------------------------------------------
//	Read the file
// -----------------------------------------------------------------------------------------
Boolean TCFDFrontDocument::ReadLine(short aRefNum, char * buff, char * string)
	{
	short 	l, t, i, ii;
	long 		pos, eof;
	short	mode;
	
	t = strlen(buff);
	mode = fsFromMark;
	if (t == 0)
		{
		long numChars;
		numChars = 512L;
		
		HLock((Handle) this);
		GetEOF(aRefNum, &eof);
		GetFPos(aRefNum, &pos);
		HUnlock((Handle) this);
		
		if (pos+512 > eof)
			numChars = eof - pos;

		HLock((Handle) this);
		FailOSErr(FSRead(aRefNum, &numChars, buff));
		HUnlock((Handle) this);
		
		if (numChars == 512 )
			{
			i = 0;
			ii = (short) numChars;
			while (buff[ii] != ' ')
				{
				i++;
				ii--;
				}
			buff[ii] = 0;
			SetFPos(aRefNum,mode,(long) (-1 * (i-1)));
			}
		t = strlen(buff);
		}
	
	sscanf (buff, "%s",string);
	l = strlen(string);
	string[l+1] = 0;
	for (i = l+1;  i <= t; i++)
		buff[i-l-1] = buff[i];
	buff[i-l-1] = 0;
	
	return true;
	}
		
// -----------------------------------------------------------------------------------------
//	Add End
// -----------------------------------------------------------------------------------------
void TCFDFrontDocument::AddTerminator(char * string)
	{
	switch(fExport)
		{
		case 1:
			strcpy(string,"\n");
			return;
		case 2:
			strcpy(string,"/\n");
			return;
		case 3:
			strcpy(string,"/");
			return;
		}
	}
		
// -----------------------------------------------------------------------------------------
//	Write the file
// -----------------------------------------------------------------------------------------
pascal void TCFDFrontDocument::DoWrite(short aRefNum, Boolean /*makingCopy*/)
	{
	char 	* string, sEnd[4];
	short	i, x, n;
	long		temp;
	Boolean flowOpt[15], printOpt[13];
	
	string = new char[80];
	
	HLock((Handle) this);
	this->AddTerminator(sEnd);
	HUnlock((Handle) this);
	
	if (fPass == 1)
		{
		if (fExport == 1)
			{
			i = sprintf(string,"%f ",fWidth);
			i += sprintf(string+i,"%f ",fHeight);
			i += sprintf(string+i,"%f\n",fUnitFraction);
			fOutput->Write(aRefNum,string);
			}
			
//	write the global information first
//	line 1	................................	
		this->DecodeParam(14,flowOpt,fFlowOpts);								// decode the flow parameters
		i = sprintf(string,"%d ",flowOpt[4]);										// klam	- laminar flow
		if (flowOpt[1])
			i += sprintf(string+i,"1 ");													// kcomp	- compressible
		else
			i += sprintf(string+i,"0 ");													// kcomp	- incompressible
		i += sprintf(string+i,"%d ",flowOpt[9]);									// kswrl	- nonswirl [0] / swirl [1]
		if (flowOpt[10])
			i += sprintf(string+i,"1 ");													// kpgrid	- created by UIFlow
		else
			i += sprintf(string+i,"0 ");													// kpgrid	- not created by UIFlow
		for (x = 0; x <= 3; x++)																// model	- which model?
			{
			if (flowOpt[x])
				i += sprintf(string+i,"%d",x);											// model
			}
		sprintf(string+i,"%s",sEnd);														// model
		fOutput->Write(aRefNum,string);

//	line 2	................................	
		i = sprintf(string,"%d ",fFuel);												// kfuel	- fuel type
		i += sprintf(string+i,"0 ");														// knorth	- (always = 0)
		i += sprintf(string+i,"%d ",flowOpt[6]);								// kplax	- planar [1] / axisymmetric [0]
		if (flowOpt[12])
			i += sprintf(string+i,"1 ");													// kadjst	- Block adjustment
		else
			i += sprintf(string+i,"0 ");													// kadjst	- No Block adjustment

		sprintf(string+i,"%s",sEnd);														// model
		fOutput->Write(aRefNum,string);

//	line 3	................................	
		n = (short) fSolution[0];
		i = sprintf(string,"%d ",n);														// ngrid	- number of fine grids
		n--;
		x = fPointMatrix->GetTColumn() - 1;											// get number of columns
		temp = (pow(2,n)) * x;																	// number of fine grid cells - x direction
		i += sprintf(string+i,"%ld ",temp);											// ncelx	- number fine grid cells - x direction
		x = (short) fPointMatrix->GetSize() - 1;								// get number of rows
		temp = (pow(2,n)) * x;																	// number of fine grid cells - y direction
		i += sprintf(string+i,"%ld",temp);											// ncely	- number fine grid cells - y direction

		sprintf(string+i,"%s",sEnd);														// model
		fOutput->Write(aRefNum,string);

//	Segment Data	................................	
		i = sprintf(string,"%d",(fPointMatrix->fLeft->GetNumberSegments()));
		sprintf(string+i,"%s",sEnd);														// model
		fOutput->Write(aRefNum,string);
		fPointMatrix->fLeft->WBack(fOutput, aRefNum, sEnd,(fSolution[0]-1));	// write segment data backward

		i = sprintf(string,"%d",(fPointMatrix->fRight->GetNumberSegments()));
		sprintf(string+i,"%s",sEnd);														// add line terminator
		fOutput->Write(aRefNum,string);
		fPointMatrix->fRight->WBack(fOutput, aRefNum, sEnd,(fSolution[0]-1));	// write segment data backward

		i = sprintf(string,"%d",(fPointMatrix->fBottom->GetNumberSegments()));
		sprintf(string+i,"%s",sEnd);														// add line terminator
		fOutput->Write(aRefNum,string);
		fPointMatrix->fBottom->WForward(fOutput, aRefNum, sEnd,(fSolution[0]-1));	// write segment data forward

		i = sprintf(string,"%d",(fPointMatrix->fTop->GetNumberSegments()));
		sprintf(string+i,"%s",sEnd);														// add line terminator
		fOutput->Write(aRefNum,string);
		fPointMatrix->fTop->WForward(fOutput, aRefNum, sEnd,(fSolution[0]-1));		// write segment data forward
	
//	Interior Data	................................	
		WallRecord data;
		data = fInterior->GetData();														// get data for interior
		i = sprintf(string,"%f ",data.u);												// u			-	u velocity
		i += sprintf(string+i,"%f ",data.v);										// v			-	v velocity
		i += sprintf(string+i,"%f ",data.w);										// w		-	w velocity
		i += sprintf(string+i,"%f",data.density);								// rh		-	density
		sprintf(string+i,"%s",sEnd);														// add line terminator
		fOutput->Write(aRefNum,string);													// write the string
				
		i = sprintf(string,"%f ",data.temp);											// t			-	temperature
		i += sprintf(string+i,"%f ",data.kenergy);								// tk		-	kinetic energy
		i += sprintf(string+i,"%f ",data.dissip);									// td		-	dissipation
		i += sprintf(string+i,"%f ",data.mixfrac);								// f			-	mixture fraction
		i += sprintf(string+i,"%f ",data.concfrac);								// g			-	concentration fraction
		i += sprintf(string+i,"%f",data.fuelfrac);									// fu		-	fuel fraction
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
				
		i = sprintf(string,"%f ",fTempFuel);											// tfuel	-	temperature of fuel
		i += sprintf(string+i,"%f",fTempAir);										// tair		-	temperature of air
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string

//	Prandtl Numbers	................................	
		i = 0;
		for (x = 0; x< 10; x++)
			i += sprintf(string+i,"%f ",fPran[x]);									// prl		-	prandlt numbers
		i += sprintf(string+i,"%f",fPran[10]);
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);
		fOutput->Write(aRefNum,string);												// prt		-	prandlt numbers
	
//	Relaxation Numbers	................................	
		i = sprintf(string,"%f ",fRelax[0]);											// relx	1	- 	u velocity
		i += sprintf(string+i,"%f ",fRelax[1]);										// relx2	-	v velocity
		i += sprintf(string+i,"%f ",fRelax[2]);										// relx	3	-	pressure
		i += sprintf(string+i,"%f ",fRelax[3]);										// relx4	-	w velocity
		i += sprintf(string+i,"%f ",fRelax[6]);										// relx5	-	h = scalar
		i += sprintf(string+i,"%f ",fRelax[4]);										// relx6	-	kinetic energy
		i += sprintf(string+i,"%f ",fRelax[4]);										// relx7	-	dissipation = kenergy
		i += sprintf(string+i,"%f ",fRelax[6]);										// relx8	-	f = scalar
		i += sprintf(string+i,"%f ",fRelax[6]);										// relx9	-	fu = scalar
		i += sprintf(string+i,"%f ",fRelax[6]);										// relx10	-	g = scalar
		i += sprintf(string+i,"%f",fRelax[5]);										// relx11	-	density
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
	
//	Sweep Numbers	................................	
		i = sprintf(string,"%ld ",fSolution[1]);										// nswp1	- 	momentum
		i += sprintf(string+i,"%ld ",fSolution[1]);								// nswp2	- 	momentum
		i += sprintf(string+i,"%ld ",fSolution[2]);								// nswp3	- 	pressure corrections
		i += sprintf(string+i,"%ld ",fSolution[1]);								// nswp4	- 	momentum
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp5	- 	scalar
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp6	- 	scalar
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp7	- 	scalar
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp8	- 	scalar
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp9	- 	scalar
		i += sprintf(string+i,"%ld ",fSolution[3]);								// nswp10- 	scalar
		i += sprintf(string+i,"%ld",fSolution[3]);									// nswp11- 	scalar
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
	
//	Print Variables	................................	
		this->DecodeParam(12,printOpt,fPrintOpts);							// decode the print parameters
		i = 0;
		for (x = 0; x < 11; x++)
			i += sprintf(string+i,"%d ",printOpt[x]);								// iprint	-	print variables
		i += sprintf(string+i,"%d",printOpt[11]);
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
	
//	miscellaneous	................................	
		i = sprintf(string,"%f ",fPressure);											// refp		-	system pressure
		i += sprintf(string+i,"%f",data.visc);										// vscty	-	internal viscosity
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
	
		i = sprintf(string,"%d ",fIteration);											// maxi	-	max iterations
		i += sprintf(string+i,"%f",fSolutionAccuracy);							// tolr		-	solution accuracy
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
	
		i = sprintf(string,"%.4f",fBoundryRadius);								// rn		-	boundry radius
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string

//	Obstacles & Baffles	................................	
		i = sprintf(string,"%d ",fPointMatrix->GetNumBaffle(true));	// number of vertical baffles
		i += sprintf(string+i,"%d ",fPointMatrix->GetNumBaffle(false)); // number of horizontal baffles
		i += sprintf(string+i,"%d",fPointMatrix->GetNumObstacle());	// number of obstacles
		sprintf(string+i,"%s",sEnd);													// add line terminator
		fOutput->Write(aRefNum,string);												// write the string
		fPointMatrix->WriteBaffles(true, fOutput, aRefNum);				// write segment data forward
		fPointMatrix->WriteBaffles(false, fOutput, aRefNum);				// write segment data forward
		fPointMatrix->WriteObstacles(fOutput, aRefNum);					// write segment data forward
		
		}

	if (fExport == 3)
		fOutput->Write(0,"GRIDS NEXT");												// write to DTM
		
	if (fWriteGrid)
		fPointMatrix->WriteGrid(fOutput, aRefNum, fExport);				// write segment data
		
	if (fExport == 3)
		fOutput->Write(0,"DONE");														// write to DTM
		
	delete string;
	}

// ****************************************************************
//	Modeless Dialog Window Methods
// ****************************************************************
// -----------------------------------------------------------------------------------------
//	Find the segment that contains theMouse.
// -----------------------------------------------------------------------------------------
void TCFDFrontDocument::SetDialogOn (Boolean theState, short dbox)
	{
	long t, t1;																					// temporary vars

	t = pow(2, (dbox-1));																// value of a specific bit
	t1 = fDialogOn & t;																	// = 0 if not on = t if on
	if (theState && t1 == 0)														// set to true (window is open)
		fDialogOn += t;
	else if (!theState && t1 == t)											// set to false (window is closed)
		fDialogOn -= t;
		
	return;
	}

// -----------------------------------------------------------------------------------------
//	Find the segment that contains theMouse.
// -----------------------------------------------------------------------------------------
Boolean TCFDFrontDocument::IsDialogOn (short dbox)
	{
	long t, t1;																					// temporary vars

	t = pow(2,(dbox-1));																// bit value of this dialog box
	t1 = fDialogOn & t;																	// = 0 if off ; = t if on
	if (t1 == t)																				// is bit on?
		return true;																			// yes
	return false;																				// no
	}

// ------------------------------------------------------------------------------------------
//	Decode the print options long int
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::DecodeParam(short index, Boolean * param, long decodeVal)
	{
	long t,temp;
	
	temp = decodeVal;																				// get data for Flow
	while (index >= 0)																				// check all bits
		{
		t = pow(2,index);																			// t = (2)^index power
		if ((temp - t) >= 0)																		// is this param on?
			{
			param[index] = true;																// yes: set true
			temp -= t;																				// reset temp for next index
			}
		else			
			param[index] = false;																// no: set false
		index--;																						// next param
		}
	return;
	}
		
// ------------------------------------------------------------------------------------------
//	Decode the print options long int
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::EncodeParam(short index, Boolean * param, long * decodeVal)
	{
	short 	t;
	long 		temp;
	
	*decodeVal = 0L;
	for (t = 0; t < index; t++)
		{
		if (param[t])
			{
			temp = pow(2,t);
			*decodeVal += temp;
			}
		}
	return;
	}
		
// ****************************************************************
//	Print Parameters Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the print options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::MakeFortranWindow(void)
	{	
	TWindow 			* aWindow;
	TScroller			*	sView;
	TFortranView 	*	fortView;
		
	aWindow = NewTemplateWindow(1019, ((TDocument *) this));		// create window
	aWindow->Open();																						// open window
	fortView = (TFortranView *) (aWindow->FindSubView('fort'));	// get reference to window
	sView = (TScroller *) (aWindow->FindSubView('VW01'));	// get reference to window

	aWindow->Select();
	fortView->AutoScrolling (TRUE);
	sView->fScrollBars[0]->Activate (TRUE);
	sView->fScrollBars[1]->Activate (TRUE);
	aWindow->Update();
	gFortranView = fortView;	
	return;	
	}
	
// ****************************************************************
//	Print Parameters Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the print options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::MakeOptPrint(void)
	{	
	TWindow * aWindow;
	Boolean checkBox[13];
	
	if (this->IsDialogOn(cPrintDialog))													// if already open 
		{
		optPrint->BringToFront();																	// bring window to the front
		return;
		}
		
	aWindow = NewTemplateWindow(kOptPrint, ((TDocument *) this));	// create window
	aWindow->Open();																						// open window
	optPrint = (TOptPrint *) (aWindow->FindSubView('DP03'));		// get reference to window
	
	this->SetDialogOn(true,cPrintDialog);												// turn dialog on
	this->DecodeParam(12,checkBox,fPrintOpts);									// figure which check boxes are on
	optPrint->MarkCheckBox(checkBox);														// mark the boxes
	return;	
	}
	
// ****************************************************************
//	Flow Parameters Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Flow options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::MakeOptFlow(void)
	{	
	TWindow * aWindow;
	Boolean radioButton[15];
	
	if (this->IsDialogOn(cFlowDialog))														// if already open 
		{
		optFlow->BringToFront();																// bring window to the front
		return;
		}
		
	aWindow = NewTemplateWindow(kOptFlow, ((TDocument *) this));	// create window
	aWindow->Open();																				// open window
	optFlow = (TOptFlow *) (aWindow->FindSubView('FLOW'));				// get reference to window
	
	this->SetDialogOn(true,cFlowDialog);												// turn dialog on
	this->DecodeParam(14,radioButton,fFlowOpts);								// figure which check boxes are on
	optFlow->MarkRadioButton(radioButton);											// mark the boxes
	return;	
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetPremixOpts(float b, short c, extended p, float p8, float p9, float p5)
	{	
	fBreakup = b;
	fCompress = c;
	fPressure = p;
	fPran[7] = p8;
	fPran[8] = p9;
	fPran[4] = p5;
	return;
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetPremixOpts(float * b, short * c, extended * p, float * p8, float * p9, float * p5)
	{	
	*b 	= fBreakup;
	*c	= fCompress;
	*p	= fPressure;
	*p8 	= fPran[7];
	*p9 	= fPran[8];
	*p5 	= fPran[4];
	return;
	}

// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetDiffusionOpts(short fuel, short cmp, extended p, float p8, float p10, float tA, float tF)
	{	
	fFuel				= fuel;
	fCompress	= cmp;
	fTempAir		= tA;
	fTempFuel		= tF;
	fPressure 	= p;
	fPran[7]		= p8;
	fPran[9]		= p10;
	return;
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetDiffusionOpts(short * fuel, short * cmp, extended * p, float * p8, float * p10, float * tA, float * tF)
	{	
	*fuel	= fFuel;
	*cmp	= fCompress;
	*tA		= fTempAir;
	*tF		= fTempFuel;
	*p		= fPressure;
	*p8		= fPran[7];
	*p10	= fPran[9];
	return;
	}

// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetTurbulenceOpts(float p6, float p7)
	{	
	fPran[5]	= p6;
	fPran[6]	= p7;
	return;
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetTurbulenceOpts(float * p6, float * p7)
	{	
	*p6		= fPran[5];
	*p7		= fPran[6];
	return;
	}

// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetPressureOpts(extended p, float p5)
	{	
	fPressure	= p;
	fPran[4] = p5;
	return;
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Premix options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetPressureOpts(extended * p, float * p5)
	{	
	*p = fPressure;
	*p5 = fPran[4];
	return;
	}

// ****************************************************************
//	Solution Parameters Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Flow options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::MakeOptSolution(void)
	{	
	TWindow * aWindow;
	if (this->IsDialogOn(cSolutionDialog))													// if already open 
		{
		optSol->BringToFront();																				// bring window to the front
		return;	
		}
		
	aWindow = NewTemplateWindow(kOptSolution, ((TDocument *) this));	// create window
	aWindow->Open();																								// open window
	optSol = (TOptSolution *) (aWindow->FindSubView('DP06'));				// get reference to window
	
	this->SetDialogOn(true,cSolutionDialog);												// turn dialog on
	optSol->MarkScroller(fSolution,fSolutionAccuracy,fIteration);
	return;	
	}
	
// ------------------------------------------------------------------------------------------
//	Initialize the Flow options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetSolutionOpts(Boolean scroller, short index, long value, float value1,short value2)
	{
	if (scroller)
		{
		if (index-1 == 0 && fPointMatrix->GetFineGrid())
			fPointMatrix->ShowFineGrid();
		fSolution[index-1] = value;																	// levels set with scrollers
		
		if (index-1 == 0 && fPointMatrix->GetFineGrid())
			fPointMatrix->ShowFineGrid();
		}
	else
		{
		fSolutionAccuracy = value1;																	// solution accuracy
		fIteration = value2;
		}
	}
	
// ****************************************************************
//	Solution Parameters Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Flow options menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::MakeOptRelax(void)
	{	
	TWindow * aWindow;
	if (this->IsDialogOn(cRelaxDialog))														// if already open 
		{
		optRelax->BringToFront();																		// bring window to the front
		return;
		}
		
	aWindow = NewTemplateWindow(kOptRelax, ((TDocument *) this));	// create window
	aWindow->Open();																				// open window
	optRelax = (TRelax *) (aWindow->FindSubView('relx'));					// get reference to window
	
	this->SetDialogOn(true,cRelaxDialog);												// turn dialog on
	optRelax->MarkScroller(fRelax);
	return;	
	}
	
// ****************************************************************
//	Dimensions Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Dimensions menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetRemoteApp(void)
	{	
	TWindow			* aWindow;
	TAppDialog	* theApp;
	
	aWindow = NewTemplateWindow(1018, ((TDocument *) this)); 					// create window
	theApp = (TAppDialog *) (aWindow->FindSubView('parm'));						// get reference to window
	HLock((Handle) this);
		theApp->GetApp(fRemoteApplication);
	HUnlock((Handle) this);
	return;	
	}
	
// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetApp(char * app)
	{
	if (strlen(app) > 511)
		app[511] = 0;
	
	strcpy(fRemoteApplication,app);
	return;
	}

// ****************************************************************
//	Dimensions Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Dimensions menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetImageName(void)
	{	
	TWindow				* aWindow;
	TImageDialog	* theApp;
	
	aWindow = NewTemplateWindow(1020, ((TDocument *) this)); 					// create window
	theApp = (TImageDialog *) (aWindow->FindSubView('imge'));						// get reference to window
	HLock((Handle) this);
		theApp->GetImage(fImageName);
	HUnlock((Handle) this);
	return;	
	}
	
// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetImageName(char * app)
	{
	if (strlen(app) > 511)
		app[511] = 0;
	
	strcpy(fImageName,app);
	return;
	}

// ****************************************************************
//	Dimensions Dialog
// ****************************************************************
// ------------------------------------------------------------------------------------------
//	Initialize the Dimensions menu dialog
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::GetDims(void)
	{	
	TWindow		* aWindow;
	TDimension	* theDim;
	
	aWindow = NewTemplateWindow(kOptDimension, ((TDocument *) this)); // create window
	theDim = (TDimension *) (aWindow->FindSubView('dims'));					// get reference to window
	if (!theDim->GetDimensions())
		{
		TQuitCommand * q;
		q = new TQuitCommand;
		FailNIL(q);
		q->IQuitCommand(cQuit);
		gCFDFrontApplication->PostCommand(q); 
		}
	return;	
	}
	
// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
// ------------------------------------------------------------------------------------------
void TCFDFrontDocument::SetDimensions(float width, float height)
	{
	fWidth = width;
	fHeight = height;
	return;
	}

// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
//	mangled name: ShowL__FPc
// ------------------------------------------------------------------------------------------
void test(void)
	{
	gFortranView->PutLine("This is a test");
	}

// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
//	mangled name: ShowL__FPc
// ------------------------------------------------------------------------------------------
void ShowL(char * msg)
	{
	short len;
	char  * msg1;
	
	len = strlen(msg);
	msg1 = new char[len+2];
	strcpy(msg1,msg);
	msg1[len] = '\n';
	msg1[len+1] = 0;
	gFortranView->PutLine(msg1);
	}

// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
//	mangled name: StopKey__Fv
// ------------------------------------------------------------------------------------------
short StopKey(void)
	{
	long	key;
	char	msg[80];

	if (GetOSEvent(keyDownMask,gEvent))
		{
		key = BitAnd(gEvent->message,charCodeMask);
		sprintf(msg,"Got Key %c\n",key);
		gFortranView->PutLine(msg);
		gStop = 1;
		if (key == 46 && gEvent->modifiers & cmdKey)
			return 1;
		}
	return 0;
	}

// ------------------------------------------------------------------------------------------
//	Set the dimensions for the geometry
//	mangled name: StopKey__Fv
// ------------------------------------------------------------------------------------------
void SetStop(void)
	{
	gStop = 1;
	}
