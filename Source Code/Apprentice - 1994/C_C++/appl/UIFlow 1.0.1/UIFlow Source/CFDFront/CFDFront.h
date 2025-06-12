// *********************************************************************************
//	TInformationView Header
// *********************************************************************************
class TInformationView : public TDialogView
	{
	private:
		TDocument		*	fDocument;
		TEditText			*	XCoord, * YCoord;
		TStaticText		*	fInfoBox, * fLocBox;
//		TPointData		* fPoint;
//		TBoundryList	* fBoundry;
//		TGPoint		* fgPoint;
		float 				saveX, saveY;
		Point 				savePt;
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		void IInformationView(TDocument * doc);												// initialize the information view
		pascal void IRes(TDocument *, TView *, Ptr *);
		pascal void Draw(Rect * thisRect);														// Draws the view seen in the window. 
		pascal void SuperViewChangedSize(VPoint *delta, Boolean invalidate);	// superview moved; move tinfoview
		pascal void  DoChoice(TView *origView, short itsChoice);						// do key choice
		void StatusString (Point thePoint);														// display the point coordinates in real.
		void InfoString(char * theString);															// display a message
		void ClrInfo (void);																				// clear message window
		pascal struct TCommand * DoMenuCommand(CmdNumber aCommand);	// do a menu choice
		void ShowDimension (void);																	// show # rows & # columns in grid
	};

TInformationView	*gInfoView;
// *********************************************************************************
//	TMDialog - Header
//		Provides a base class for the modeless dialog windows in this program
// *********************************************************************************
class TMDialog : public TDialogView
	{
	protected :
		union xbox
			{
			IDType boxID;
			char boxCH[5];
			};
		union xbox tbox;

	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		virtual void BringToFront(void);
		virtual void GetControlName (short index,char * strng);
	};
	
// *********************************************************************************
//	TOptPremix - Header
// *********************************************************************************
class TPremix : public TDialogView
	{
	public:
		void GetParams(void);																			// get the data
	};
	
// *********************************************************************************
//	TDiffusion - Header
// *********************************************************************************
class TDiffusion : public TDialogView
	{
	public:
		void GetParams(void);																			// get the data
	};
	
// *********************************************************************************
//	TTurbulence - Header
// *********************************************************************************
class TTurbulence : public TDialogView
	{
	public:
		void GetParams(void);																			// get the data
	};
	
// *********************************************************************************
//	TPressure - Header
// *********************************************************************************
class TPressure : public TDialogView
	{
	public:
		void GetParams(void);																			// get the data
	};
	
// *********************************************************************************
//	TOptPrint - Header
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
// *********************************************************************************
class TOptPrint : public TMDialog
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void IRes(TDocument *itsDocument, TView *itsSuperView, Ptr *itsParams);
		pascal void DoChoice(TView *origView, short itsChoice);
		pascal void DismissDialog(ResType dismisser);
		void DecodeBits(long printO);
		void MarkCheckBox (Boolean * checkBox);
	};
	
// *********************************************************************************
//	TOptFlow - Header
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
// *********************************************************************************
class TOptFlow : public TMDialog
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void DoChoice(TView *origView, short itsChoice);
		pascal void DismissDialog(ResType dismisser);
		void DoLayerDialog(TRadio * theButton, short index);
		void MarkRadioButton (Boolean * radioButton);
	};
	
// *********************************************************************************
//	TOptSolution - Header
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
// *********************************************************************************
class TOptSolution : public TMDialog
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void DoChoice(TView *, short );
		pascal void DismissDialog(ResType);
		void MarkScroller (long *, float, short);
		void DisplayValue (short, long);
		pascal TCommand * DoKeyCommand(short ch, short aKeyCode, EventInfo *info);
		void SaveEdit(void);
		void BringToFront(void);
	};

// *********************************************************************************
//	TOptSolution - Header
//		inherits all methods and vars from TDialogView
//		overrides method : DoChoice
// *********************************************************************************
class TRelax : public TMDialog
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void DoChoice(TView *origView, short itsChoice);
		pascal void DismissDialog(ResType dismisser);
		void MarkScroller (float * scroller);
		void DisplayValue (short index, float value);
	};

class TMyTypingCommand : public TTETypingCommand
	{
	public :
		pascal void AddCharacter(short aChar);
	};
	
class TMyEView : public TTEView
	{
	public :
		pascal struct TTETypingCommand * DoMakeTypingCommand(short ch);
	};

class TMyDialogTEView : public TDialogTEView
	{
	public:
		pascal void IDialogTEView(TDocument *itsDocument, TView *itsSuperView, VPoint *
				itsLocation, VPoint *itsSize, SizeDeterminer itsHDeterminer, SizeDeterminer itsVDeterminer, 
				Rect *itsInset, TextStyle *itsTextStyle, short itsJustification, Boolean itsStyleType, Boolean
				itsAutoWrap);
		pascal struct TTETypingCommand * DoMakeTypingCommand(short ch);
		pascal void InstallSelection(Boolean wasActive, Boolean beActive);
		void StoreData(void);
	};

// *********************************************************************************
//	TDataView - Header
//		inherits all methods and vars from TDialogView
// *********************************************************************************
class TDataView : public TMDialog
	{
	private:
		TPoint 				* fPoint;
		TPointMatrix	* matrix;
		TView				* fGeom;																	// the geometry
		TScroller			* fScroll;																	// the scroll bar
		TRadio				* tWall;																	// wall radio button
		TRadio				* tIflw;																	// inflow radio button
		TRadio				* tSymm;																	// symmetry radio button
		TRadio				* tOflw;																	// outflow radio button
		TRadio				* tIntr;																	// Interior radio button
		union wRecord
			{
			WallRecord theData;
			float iData[12];
			};
		union wRecord wRec;
	public:
		pascal void IDataView (void);															// init from resource
		pascal void Draw(Rect * thisRect);													// Draws the view seen in the window. 
		pascal void SuperViewChangedSize(VPoint *delta, Boolean invalidate);
		void ShowData(TSegPoint * tPt);
		pascal void DoChoice(TView *origView, short itsChoice);
		pascal struct TDialogTEView * MakeTEView(void);
		void StoreData(void);
		void ShowIt(void);
		void HideIt(void);
	};
	
// *********************************************************************************
// 	the geometry document class.
// *********************************************************************************
class TCFDFrontDocument : public TDocument 
	{
	private:
		friend class TGeomView;
		TGeomView	*	fGeom;
		Str255			fFileName;
		char				fRemoteApplication[512];
		char				fImageName[512];
		short			fMouseAction;
		short			fPass;
		Boolean 		fMakeView;
		Boolean			fWriteGrid;	
		
// the fields of the document used to fill in info.
// ----------------------------------------------------------------------------------------------------
//	Document data
// ----------------------------------------------------------------------------------------------------
		float				fWidth;																			// the width of the geometry
		float				fHeight;																		// the height of the geometry
		long				fDialogOn;																		// which dialogs are open?
		TOptPrint		*	optPrint;																	// print dialog - options menu
		TOptFlow		*	optFlow;																	// flow dialog - options menu
		TOptSolution * optSol;																		// solution dialog - options menu
		TRelax			*	optRelax;																	// relaxation dialog - options menu
		
		extended		fPressure;																	// system pressure
		long				fPrintOpts;																	// print options data
		long				fSolution[5];																	// solutions data values
		long				fFlowOpts;																	// flow options data
		float				fRelax[8];																	// solutions data values
		float				fSolutionAccuracy;
		float				fPran[11];																	// prandlt numbers
		float				fBreakup;																		// eddy breakup coefficient
		float 			fBoundryRadius;															// inner boundry radius
		float				fTempFuel;																	// temperature of fuel
		float				fTempAir;																	// temperature of air
		short			fCompress;																	// compressiblity
		short			fFuel;																			// type of fuel
		short			fIteration;																	// maximum iterations
		
	public:
		float			fUnitFraction;																	// relates unit of measure to fWidth & fHeight
		short		fExport;
		Boolean		fGridOperation;																	// last operation was on a grid
// ----------------------------------------------------------------------------------------------------
//  	the boundries & object pointers
// ----------------------------------------------------------------------------------------------------
		TPointMatrix		*	fPointMatrix;													// matrix overseer
		TInformationView	*	fInfoView;														// the information view at the top
		TDataView			*	fDataView;														// segment data view
		TSegPoint				*	fInterior;															// point representing all interior data
		TOutput				*	fOutput;															// output to disk object
		
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		TCFDFrontDocument (void);																// initialize the extra fields
		pascal void Free(void);
		pascal void DoMakeViews (Boolean forPrinting);								// make the views
		void SetMouseAction (IDType op);														// set the mouse action mode to that for IDType
		short GetMouseAction (void)					{return fMouseAction;}			// get the mouse action mode
		pascal void DoSetupMenus (void);														// set up the menu bar
		pascal void Close(void);																	// close document
		pascal TCommand* DoMenuCommand(CmdNumber aCmdNumber);		// menu command processor
		Boolean ICFDFrontDocument(Boolean forPrinting);								// set up the point fields.
		void SetViewSize (short * xsize, short * ysize);								// Get the size of the geometry view.
		void ViewToReal(Point thePoint, realPt * realNum);							// convert the point to the real coords.
		Point RealToView (struct realPt * realNum);										// convert the real coords to view point
		pascal void DoWrite(short, Boolean);
		pascal void DoRead(short, Boolean, Boolean);
		Boolean ReadLine(short, char *, char *);
		void AddTerminator(char *);
		pascal void Save(CmdNumber, Boolean, Boolean);
		pascal void ReadFromFile(AppFile *, Boolean);
		long GetFineGrid(void)							{return fSolution[0];}			// return number of fine grids
// ----------------------------------------------------------------------------------------------------
//	methods for modeless dialog initialization & communication between doc & view
// ----------------------------------------------------------------------------------------------------
		void DecodeParam(short, Boolean *, long);										// decode long parameter variable
		void EncodeParam(short, Boolean *, long *);									// encode long parameter booleans
		void SetDialogOn (Boolean, short);													// set the bit for the dialog box
		Boolean IsDialogOn (short);																// return the status of the dialog box
		void MakeOptPrint(void);																	// create a print parameters dialog
		void SetPrintOpts (long newData)			{fPrintOpts = newData;}		// set the print options data variable
		long GetPrintOpts (void)						{return fPrintOpts;}				// get the print options data variable
		void MakeOptFlow(void);																	// create a print parameters dialog
		long GetFlowOpts(void)							{return fFlowOpts;}				// get the flow options param
		void SetFlowOpts(long newData)			{fFlowOpts = newData;}		// store the flow options param
		float GetRadiusOpts(void)						{return fBoundryRadius;}		// get the value of the inner radius
		void SetRadiusOpts(float value)				{fBoundryRadius = value;}		// store inner boundry radius
		void MakeOptSolution(void);																// make the solutions dialog
		void SetSolutionOpts(Boolean, short, long, float,short);	 				// set the solution parameter data
		void GetRemoteApp(void);
		void SetApp(char *);
		void GetDims (void);																			// Get the dimensions of the geometry
		void SetDimensions(float, float);
		void GetPremixOpts(float *, short *, extended *, float *, float *, float *);	// return the prandlt number
		void SetPremixOpts(float,short,extended,float,float,float);				// set the premix values 
		void GetDiffusionOpts(short *, short *, extended *, float *, float *, float *, float *);	// get diffusion values
		void SetDiffusionOpts(short, short, extended, float, float, float, float);// set diffusion values
		void SetTurbulenceOpts(float, float);
		void GetTurbulenceOpts(float *, float *);
		void MakeOptRelax(void);																	// make relaxation dialog
		void SetRelaxOpts(short index, float value)
																	{fRelax[index-1] = value;}	// set relaxation value
		void SetPressureOpts(extended, float);												// pressure options model 1
		void GetPressureOpts(extended *, float *);										// pressure options model 1
		void MakeFortranWindow(void);
		void GetImageName(void);
		void SetImageName(char * app);
		};

// **********************************************************************
//	the geometry view class.
// **********************************************************************
class TGeomView : public TView
	{
	private:
		Boolean 	fDragging;																		// are we dragging a point?
		float			fMCount;
		
	public:
		TCFDFrontDocument	*	fCDocument;													// the document
		TPoint 		* fSPoint;																		// currently selected point
		Boolean		fSegment;																			// highlight segment?
		TRow 		*	fSSide;																			// current boundry
		short		lastCmd;																			// drag or tinfo command
		float			fMagnify;																			// magnification factor
		short		fHorizSize;																		// horizontal size of geometry
		short 		fVertSize;																		// vertical size of geometry
		
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		void				SetGeomViewSize (void);												// Set the size of the geometry view from the geometry dims.
		pascal void	Draw(Rect *);																// Draws the view seen in the window. 
		pascal void	IRes (TDocument *, TView *,  Ptr *);							// init from resource
		pascal struct TCommand * DoMouseCommand(Point * , EventInfo *,	// handle mouse downs in this view
			 Point * );
		void 				MagnifyView (float);
		pascal void	CalcMinSize(VPoint *);													// for sizing the geometry view
		void 				UnSelect(void);
		void 				Select(TPoint * , Boolean , TRow * );
		pascal Boolean DoSetCursor(Point , RgnHandle );
		Point 			DragCurrent (Point);														// drag the current point (w/ set button)
		
//	Inlines
		Boolean	IsDragging(void)									{return fDragging;}
		void		SetDragging(Boolean setit)					{fDragging  = setit;}
		short	GetHorizViewSize(void)						{return fHorizSize;}
		short	GetVertViewSize (void)							{return fVertSize;}
	};

// **********************************************************************
//	the TFortranView view class.
// **********************************************************************
class TFortranView : public TTEView
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void IRes (TDocument *, TView *,  Ptr *);							// init from resource
		pascal void Close(void);
		void PutLine (char * message);
	};

TFortranView * gFortranView;

// **********************************************************************
//	the application class.
// **********************************************************************
class TCFDFrontApplication : public TApplication
	{
	public:
		Boolean fReadOld;
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal struct TDocument *DoMakeDocument(CmdNumber itsCmdNumber);	// make a new document
		pascal void ICFDFrontApplication(OSType itsMainFileType);						// initialize the application
		pascal void DoSetupMenus (void);																// set up the menus for this application
		pascal void SetUndoText(Boolean, CmdNumber);
		pascal void DoShowAboutApp(void);
		pascal void Idle(IdlePhase);
		pascal void OpenOld(CmdNumber, AppFile *);
	};

TCFDFrontApplication *gCFDFrontApplication;													// The application object 

// **********************************************************************
//	TNewSegmentCommand - Header
// 	create a new segment on a boundry.
// **********************************************************************
class TNewSegmentCommand : public TCommand
	{
	protected:
		TGeomView				*	fGeomView;															// geomView object
		TCFDFrontDocument	*	fCDocument;															// document object
		TPointMatrix			*	fPointMatrix;														// matrix overseer
		TList						*	saveDeleted;															// list of deleted points
		TPoint						*	fPoint;																	// the saved point
		TBoundry					*	fSide;
		Boolean							done;
		Point							fLocation;																// location to put point
		short							fRow, fColumn;														// current row/column for point
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		void INewSegmentCommand (TGeomView *,TCFDFrontDocument *);			// init the command object
		pascal void TrackFeedback (VPoint *, VPoint *,Boolean, Boolean)		{}; 	// tracking feedback. Done inline.
		pascal void TrackConstrain (VPoint *, VPoint *, VPoint *);						// constrain mouse.
		pascal TCommand * TrackMouse (TrackPhase, 											// tracking mouse.
					VPoint *, VPoint *, VPoint *, Boolean);
		virtual pascal void DoIt (void);																	// do the command
		pascal void UndoIt (void);																			// do the command
		pascal void RedoIt (void);																			// do the command
		void Draw (TPoint *, TPoint *, short, PointInfo *, PointInfo *);
		pascal void Free (void);																			// free the object

		virtual TPoint * CreatePoint (TPoint * tPt, Point * mMouse);
	};

// **********************************************************************
//	TNewSectionCommand - Header
// 	create a new segment on a boundry.
// **********************************************************************
class TNewSectionCommand : public TNewSegmentCommand
	{
	public:	
		void INewSectionCommand (TGeomView * itsGeometry,						// init the command object
			 TCFDFrontDocument * tDoc);
		TPoint * CreatePoint (TPoint * tPt, Point * mMouse);
	};
	
// *********************************************************************************
//	TDragCommand Header
// *********************************************************************************
class TDragCommand : public TCommand
	{
	protected:
		TCFDFrontDocument	*	fCDocument;														// the document
		TGeomView				*	fGeom;																// the geometry
		TPoint		  				* fPoint;																// selected point
		TPointMatrix			*	fMatrix;															// the overseer matrix
	
		PointInfo					dInfo;																	// information structure
		Point			 			fMouse;																// where is the mouse
		Point						lastTrack;															// where mouse was last tracked
		Point						fPt;	
		extended					fLength;																// length of row / column
		short						fSide;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
//	Overriding MacApp methods.........................................................................................................................
		pascal void UndoIt(void);																		// undo the command
		pascal void RedoIt(void);																		// redo the command
		pascal void DoIt(void);																			// do the command
		pascal TCommand * TrackMouse(TrackPhase, VPoint *, VPoint *, 		// track the mouse
			  VPoint * , Boolean );
		pascal void TrackFeedback(VPoint * ,VPoint *, Boolean, Boolean);			// give user tracking feedback
		pascal void TrackConstrain (VPoint * ancorPoint, 										// constrain mouse.
			VPoint * previousPoint, VPoint * nextPoint);

//	New Methods...............................................................................................................................................
		short IDragCommand (TGeomView *,TPoint *, Point * );						// initialize the dragger command
		void DisplayIt(Point,Point);																	// display the new point
		void DisplayObstacles(void);
		void MovePoint(void);																			// move a point directly
	};
	
// **********************************************************************
//	TObstacleCommand - Header
// 	create an obstacle in the selected grid cell.
// **********************************************************************
class TObstacleCommand : public TCommand
	{
	protected:
		TGeomView				*	fGeom;
		TCFDFrontDocument 	* 	fCDocument;
		PointInfo						fInfo;
		TObstacle					*	fObstacle;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
//	Overriding MacApp methods.........................................................................................................................
		pascal void IObstacleCommand (TGeomView *,TCFDFrontDocument *, PointInfo *); 	// init the create obstacle object
		pascal void DoIt (void);																			// do the command
		pascal void UndoIt (void);																		// do the command
		pascal void RedoIt (void);																		// do the command
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean, Boolean);
		pascal void Commit (void);
		pascal void Free (void);
	};

// **********************************************************************
//	TDeleteObstacleCommand - Header
// 	delete an obstacle in the selected grid cell.
// **********************************************************************
class TDeleteObstacleCommand : public TCommand
	{
	protected:
		TGeomView				*	fGeom;
		TCFDFrontDocument 	* 	fCDocument;
		PointInfo						fInfo;
		TObstacle					*	fObstacle;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
//	Overriding MacApp methods.........................................................................................................................
		Boolean IDeleteObstacleCommand (TGeomView *,TCFDFrontDocument *, PointInfo *); 	// init the create obstacle object
		pascal void DoIt (void);																			// do the command
		pascal void UndoIt (void);																		// do the command
		pascal void RedoIt (void);																		// do the command
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean, Boolean);
		pascal void Commit (void);
		pascal void Free (void);
	};

// **********************************************************************
//	TBaffleCommand - Header
// 	create an obstacle in the selected grid cell.
// **********************************************************************
class TBaffleCommand : public TCommand
	{
	protected:
		TGeomView				*	fGeom;
		TCFDFrontDocument 	* 	fCDocument;
		PointInfo						fInfo;
		TBaffle						*	fBaffle;
		short							fDirection;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
//	Overriding MacApp methods.........................................................................................................................
		pascal void IBaffleCommand (TGeomView *,TCFDFrontDocument *, PointInfo *,short); 	// init the create obstacle object
		pascal void DoIt (void);																			// do the command
		pascal void UndoIt (void);																		// do the command
		pascal void RedoIt (void);																		// do the command
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean, Boolean);
		pascal void Commit (void);
		pascal void Free (void);
	};

// **********************************************************************
//	TDeleteBaffleCommand - Header
// 	delete an Baffle in the selected grid cell.
// **********************************************************************
class TDeleteBaffleCommand : public TCommand
	{
	protected:
		TGeomView				*	fGeom;
		TCFDFrontDocument 	* 	fCDocument;
		PointInfo						fInfo;
		TBaffle						*	fBaffle;
		short							fDirection;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
//	Overriding MacApp methods.........................................................................................................................
		Boolean IDeleteBaffleCommand (TGeomView *,TCFDFrontDocument *, PointInfo *,short); 	// init the create obstacle object
		pascal void DoIt (void);																			// do the command
		pascal void UndoIt (void);																		// do the command
		pascal void RedoIt (void);																		// do the command
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean, Boolean);
		pascal void Commit (void);
		pascal void Free (void);
	};

// **********************************************************************
//	TTrash - Header
// **********************************************************************
class TCTrash : public TObject
	{
	private :
		short 			fRsrsId;
		Rect				fCTrash;
		CIconHandle	theIcon;
	public:
		void 			ICTrash(Point, short,short,short,short,short);
		void 			CalcLocation(Point *,short,short,short,short);
		Boolean		InTheCan(Point);
		Rect 		GetRect(void);
		pascal void DrawCan(void);
		pascal void Free(void);
	};
 	
// *********************************************************************************
//	TDeletePointCommand Header
// *********************************************************************************
class TDeletePointCommand : public TCommand
	{
	protected:
		TList						*	saveDeleted;														// list of deleted points
		WallRecord 				saveData;																// data values
		short						saveType;															// wall type
		short						delRightSeg;															// which segment was deleted
		short						fRow, fColumn;														// index of deleted line
		
		TGeomView				*	fGeom;																// the geometry
		TCFDFrontDocument	*	fCDocument;														// the document
		TBoundry 					*	fSide;																// which side are we on?
		TPoint						*	fPoint;																// selected segment
		TPoint						*	fPrev, * fNext;												// next / prev segment
		TCTrash 					* 	trashL;																// left trash can
		TCTrash 					* 	trashR;																// right trash can
		TCTrash 					* 	trashLO;															// open trash can
		TCTrash 					* 	trashRO;															// open trash can

 		PointInfo 					fInfo;																	// surrounding points
 		short 						nCent, lCent;														// how to center trash cans
		Boolean 					rightOn, leftOn;
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		Boolean IDeletePointCommand (TGeomView *, TPoint *);							// initialize the dragger command
		pascal void UndoIt(void);																			// undo the command
		pascal void RedoIt(void);																			// redo the command
		pascal void DoIt(void);																				// do the command
		pascal TCommand * TrackMouse(TrackPhase, VPoint *, VPoint *, VPoint *, Boolean);	// track the mouse
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean,Boolean); 				// give user tracking feedback
		void DisplayIt(Point,Point);																		// display the new point
		pascal void Commit(void);
		pascal void Free(void);																				// free command object
	};
	
// *********************************************************************************
//	TDeleteGridCommand Header
// *********************************************************************************
class TDeleteGridCommand : public TCommand
	{
	protected:
		TList						*	saveDeleted;															// list of deleted points
		short						fRow, fColumn;															// index of deleted line
 		PointInfo 					fInfo;																		// surrounding points
		
		TGeomView				*	fGeom;																	// the geometry
		TCFDFrontDocument	*	fCDocument;															// the document
		TBoundry 					*	fSide;																	// which side are we on?
		TPoint						*	fPoint;																	// selected segment
		
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		Boolean IDeleteGridCommand (TGeomView *, TPoint *);							// initialize the dragger command
		pascal void UndoIt(void);																			// undo the command
		pascal void RedoIt(void);																			// redo the command
		pascal void DoIt(void);																				// do the command
		pascal TCommand * TrackMouse(TrackPhase, VPoint *, VPoint *, VPoint *, Boolean);	// track the mouse
		pascal void TrackFeedback(VPoint *,VPoint *, Boolean,Boolean); 				// give user tracking feedback
		pascal void Commit(void);
	};
	
// **********************************************************************
//	TPctsView - Header
// 	TPctsView & TOpPict together provide the palette functionality.
// 	The superview contains the entire palette, and the components of the palette also
// 	have their own class and functionality
// **********************************************************************
class TPctsView : public TView
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		inline void TurnAllOff (void);																// Draws the view seen in the window. 
		pascal void IRes (TDocument * itsDocument, TView * itsSuperView, 	// we need the fDocument initialized to something real.
			Ptr * itsParams);	
		pascal void Draw(Rect * thisRect);														// Draws the view seen in the window. 
		pascal void SuperViewChangedSize(VPoint *delta, Boolean /*invalidate*/);
	};

// **********************************************************************
//	TOpPict - Header
// 	TPctsView & TOpPict together provide the palette functionality.
// 	The superview contains the entire palette, and the components of the palette also
// 	have their own class and functionality
// **********************************************************************
class TOpPict : public TPicture 
	{
	private:
		TGeomView * fGeom;
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal struct TCommand * DoMouseCommand(Point * theMouse,			// handle mouse clicks in the palette area
			EventInfo * info,Point * hysteresis);
		pascal void IRes (TDocument * itsDocument, TView * itsSuperView, 	// we need the fDocument initialized to something real.
			Ptr * itsParams);
};

// *********************************************************************************
//	TBoundryRadius - Header
//		inherits all methods and vars from TDialogView
// *********************************************************************************
class TBoundryRadius : public TDialogView
	{
	public:
		void GetParams(void);
	};
	
// *********************************************************************************
//	TDimension - Header
//		inherits all methods and vars from TDialogView
// *********************************************************************************
class TDimension : public TDialogView
	{
	public:
		Boolean GetDimensions (void);
	};
	
// *********************************************************************************
//	TAppDialog - Header
//		inherits all methods and vars from TDialogView
// *********************************************************************************
class TAppDialog : public TDialogView
	{
	public:
		Boolean GetApp (char *);
	};
	
// *********************************************************************************
//	TAppDialog - Header
//		inherits all methods and vars from TDialogView
// *********************************************************************************
class TImageDialog : public TDialogView
	{
	public:
		Boolean GetImage (char *);
	};
	
// *********************************************************************************
//	TWarnDelete - Header
//	Warn the user they are about to delete something
// *********************************************************************************
class TWarnDelete : public TDialogView
	{
	public:
		Boolean ShowDelete (char *);
	};
	
// *********************************************************************************
//	TWarnDialog - Header
// *********************************************************************************
class TWarning : public TDialogView
	{
	short fIconNumber;
	char * msg;
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		void ShowWarning(void);
		void IWarning(short which, char * parm);
		pascal void Free(void);
	};

// **********************************************************************
// 	Function declarations
// **********************************************************************
pascal void DrawAllPoints (TObject * tItem, void * tinfo);
pascal void DrawPoint (TObject * tItem, void * tinfo);
pascal void DrawAllLines (TObject * tItem, void * tinfo);
pascal void DrawLine (TObject * tItem, void * tinfo);

void SlopeIntercept(slopeStruct * tSlope);
extended CalculateDistance(Point first, Point second);
void 	InitColorPattern(void);
Point transform(Point, float);
Point AntiTransform(Point, float);
pascal void TransformGrid (TObject *, void *);
pascal void TransformPoint (TObject *, void *);
extern "C" int RunSpv (char *);
short StopKey(void);
void ShowL(char *);
void test(void);
void SetStop(void);
