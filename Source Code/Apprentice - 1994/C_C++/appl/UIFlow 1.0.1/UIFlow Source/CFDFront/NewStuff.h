// *********************************************************************************
//	Header File for Class definitions
// *********************************************************************************
// **********************************************************************
//	TOutput class.
// **********************************************************************
class TOutput : public TObject
	{
	protected:
		TDocument * fDocument;
		Boolean	fGrid;																								// save the grid in this file?
	public:
		void IOutput (TDocument *,short);
		virtual void Write(short, char *);
		Boolean GetGrid (void)										{return fGrid;}						// return fGrid value
	};
	
// **********************************************************************
//	TOutput class.
// **********************************************************************
class TFOutput : public TOutput
	{
	public:
		void Write(short, char *);
	};
	
// *********************************************************************************
//	TPoint Class
// *********************************************************************************
class TPoint : public TObject
	{
// ----------------------------------------------------------------------------------------------------
//	protected variables & methods
// ----------------------------------------------------------------------------------------------------
	protected:
		Point		savePoint;																					// save the location of this object
		Boolean 	fIsSelected;																					// is the point selected
		Boolean		fIsObstacle;																					// is point the upper left cornor of an obstacle
		Rect			fFillRect, fillRect,fHoldRect;															// display rectangle
		float			fMag;
		short		fType;																							// segment type
		short		fCell;
// ----------------------------------------------------------------------------------------------------
//	public variables & methods
// ----------------------------------------------------------------------------------------------------
	public:
		TObject	*	fObsUpLeft;																				// the obstacle pointer
		TObject	*	fObsUpRight;																				// the obstacle pointer
		TObject	*	fObsLwLeft;																				// the obstacle pointer
		TObject	*	fObsLwRight;																			// the obstacle pointer
		TObject	*	fBaffAbove;																				// the obstacle pointer
		TObject	*	fBaffBelow;																				// the obstacle pointer
		TObject	*	fBaffLeft;																					// the obstacle pointer
		TObject	*	fBaffRight;																				// the obstacle pointer
		Point		fStart;																							// object location
		Point		fTrans, fHTrans;
		Point		fHold;
		
		void 	IPoint (short, short,short,float);															// initialization routine
		Boolean OnPoint (Point *);																			// is mouse on this point?
		void 	DrawSection (Point);																			// draws the segment
		void 	DrawGrid(Point);																					// draws the line as grid
		void 	HighlightSection (Point);
		void 	SetStart(short, short);
		void 	SetHold(short, short);
		void	FixHold(void);
		void 	DrawHoldLine(short,Point);
		void  DrawTemp(Point, Point);
		void 	TransformPoint(float);

		Point	GetStart (void)							{return fStart;}								// returns fStart
		Boolean	IsSelected (void)						{return fIsSelected;}							// returns fIsSelected
		void		SetSelection (Boolean value)		{fIsSelected = value;}						// stores fIsSelected
		void		RememberPoint (void)				{savePoint = fStart;}						// stores fStart in savePoint
		Point	RetrievePoint (void)					{return savePoint;}							// returns savePoint
		void		SetSectionType (short sType)	{fType = sType;}								// store the segments type
		short	GetSectionType (void)				{return fType;}									// return the segments type
		void		SetCell (short cell)					{fCell = cell;}									// store the starting cell
		short	GetCell (void)							{return fCell;}									// return the starting cell
		void		SetObstacle(Boolean,short,TObject *);												// establish the obstacle
		TObject* GetObsQuad(short);
		void		SetBaffle(Boolean,short,TObject *);													// establish the baffle
		TObject * GetBaffQuad(short);
			
// ----------------------------------------------------------------------------------------------------
//	Virtuals .... Override these!
// ----------------------------------------------------------------------------------------------------
		virtual void Highlight(void);																			// highlight the point
		virtual void Draw(void);																				// draw the point
		virtual void DrawHold(void);																			// temp holding for drag
		virtual Boolean IsCornor (void);																	// point a cornor point?
		virtual Boolean IsSegment (void);																	// returns false.
		virtual Boolean IsBoundryPt (void);																// returns true.
		virtual Boolean IsInterior (void);																	// returns true.
	};

// *********************************************************************************
//	TGPoint Class 					=	Grid Points
//		draws differently from TPoint
//		these are the grid markers found on the boundries
// *********************************************************************************
class TGPoint : public TPoint
	{
// ----------------------------------------------------------------------------------------------------
//	public variables & methods  :  OVERRIDING FROM TPOINT
// ----------------------------------------------------------------------------------------------------
	public:
//	Overriding TPoint Methods	.......................................................			
		void Highlight(void);																						// highlight the point
		void Draw(void);																							// draw the point
		void DrawHold(void);
void Drawd (void);
	};
	
// *********************************************************************************
//	TGridPoint Class 				=	Grid Points
//		draws differently from TPoint 
//		these are the actual INTERAL grid points
// *********************************************************************************
class TGridPoint : public TGPoint
	{
// ----------------------------------------------------------------------------------------------------
//	public variables & methods  :  OVERRIDING FROM TPOINT
// ----------------------------------------------------------------------------------------------------
	public:
//	Overriding TPoint Methods	.......................................................			
		Boolean IsBoundryPt(void)						{return false;}								// returns false
		Boolean IsInterior (void)							{return true;}								// point IS interior
	};
	
// *********************************************************************************
//	TSegPoint Class				=	Segment Points
//		contains the data for the segments
//		has methods for accessing the data & drawing
// *********************************************************************************
class TSegPoint : public TPoint
	{
// ----------------------------------------------------------------------------------------------------
//	protected variables & methods 
// ----------------------------------------------------------------------------------------------------
	protected:
		WallRecord	fInitData;																					// data for this segment
		short			fIsProfile[cNumProfiles];
		float				fProfile[cNumProfiles][2][cMaxProfVals];
		
// ----------------------------------------------------------------------------------------------------
//	public variables & methods 
// ----------------------------------------------------------------------------------------------------
	public:
		void SetData (WallRecord newData);																// store the segment data
		WallRecord GetData (void)								{return fInitData;}					// return the segment data

//	Overriding TPoint Methods	.......................................................	
		void IPoint(short p1V, short p1H, short type,float);										// initialization routine
		void Highlight(void);																						// highlight the point
		void Draw(void);																							// draw the point
		void DrawHold(void);
		Boolean IsSegment (void);																				// returns true
	};
	
// *********************************************************************************
//	TCornorPoint Class			=	Cornor points of the original rectangle
//		draws differently from TPoint
// *********************************************************************************
class TCornorPoint : public TSegPoint
	{
		Boolean fSeg;
// ----------------------------------------------------------------------------------------------------
//	public variables & methods  :  OVERRIDING FROM TPOINT
// ----------------------------------------------------------------------------------------------------
	public:
//	Overriding TPoint Methods	.......................................................			
		void IPoint(short p1V, short p1H, short type, Boolean seg,float);
		void Highlight(void);																						// highlight the point
		void Draw(void);																							// draw the point
		Boolean IsCornor (void);																				// point IS cornor
		Boolean IsSegment (void);																				// returns seg;
	};
	

// *********************************************************************************
//	TInteriorPoint Class			=	Interior point to hold interior data
//		draws differently from TPoint
// *********************************************************************************
class TInteriorPoint : public TSegPoint
	{
// ----------------------------------------------------------------------------------------------------
//	public variables & methods  :  OVERRIDING FROM TPOINT
// ----------------------------------------------------------------------------------------------------
	public:
//	Overriding TPoint Methods	.......................................................			
		void Highlight(void);																						// highlight the point
		void Draw(void);																							// draw the point
		Boolean IsInterior (void)					{return true;}										// point IS interior
	};
	
// *********************************************************************************
//	TRow Class						=	List of Pointers to Segment Points
// *********************************************************************************
class TRow : public TList
	{
	protected:
		Point	snap;
// ----------------------------------------------------------------------------------------------------
//	public variables & methods 
// ----------------------------------------------------------------------------------------------------
	public:
		void 		IRow (TPoint *, TPoint *);															// init the row
		pascal void Each (pascal void (*DoToItem)(TObject *, void *), void*DoToItem_StaticLink);
		short 	SearchPoint (Point *);																	// search for a point
		short 	SearchSection (Point *);																// searchs line segs for point
		Boolean PointOnLine (Point, Point, Point  *);												// is point on this line?
		void 		SnaptoSection (Point *);
	
		TGridPoint *	AddPointAt (short, Point,float);											// add a point to row
		extended		GetLength (TPoint *);															// get length of line
		extended		GetDifferance (short, ArrayIndex, Boolean);							// get x/y difference
		short			GetNumberSegments (void);													// how many segments
		TPoint *		GetAdjacentSegment (Boolean, TPoint *);								// return surrounding segments
		short			GetNumBaffle(Boolean);														// return number baffles
		short			GetNumObstacle(void);															// return number baffles
		void 				WriteSegmentData (WallRecord *, TOutput *, short, char *);	// write out the wall data
		void 				WBack (TOutput *, short, char *,long);
		void 				WForward (TOutput *, short, char *,long);
		void 				FixHold(void);
		void 				RememberRow (void);
		void 				RetrieveAndFixRow(void);

		virtual short GetSide(void)			{return cNotOnBoundry;}						// return this boundry
	};
	
// *********************************************************************************
//	TSegmentList Class			=	List of Pointers to Segment Points
// *********************************************************************************
class TBoundry : public TRow
	{
	protected:
		short 	fSide;																						// the boundry
// ----------------------------------------------------------------------------------------------------
//	public variables & methods 
// ----------------------------------------------------------------------------------------------------
	public:
		void IRow(TPoint * p1, TPoint * p2, short side);									// init the row : override
		short GetSide(void)							{return fSide;}								// return this boundry
	};
	
// *********************************************************************************
//	Structure for adding and dragging points
// *********************************************************************************
typedef struct PointInfo
	{
	TRow				*	aboveRow;																		// row above this row
	TRow				*	belowRow;																		// row below this row
	TPoint				*	above;																				// point above
	TPoint				*	below;																				// point below
	TPoint				*	left;																					// point to the left
	TPoint				*	right;																				// point to the right
	TPoint				*	oPt;																					// point up & left
	TPoint				* lrc;																					// lower right corner
	Boolean				dLeft;																					// drag draw direction
	Boolean				dRight;																					// drag draw direction
	Boolean				gridOnly;																				// draw only the grid?
	Boolean				noGrid;																					// draw only boundries.
	short 				boundry;																				// boundy its on
	short				row;																						// row point is in
	short				column;																				// column point is in
	float					magnify;																				// magnification factor
	} PointInfo;

// *********************************************************************************
//	TBaffle Class			=	the Baffles
// *********************************************************************************
class TBaffle : public TObject
	{
	protected:
		TPoint	*	fPt1;																						// cornors of obstacle
		TPoint	*	fPt2;
		short	fDirection;

	public:
		void IBaffle(PointInfo *, short);
		void BaffleRelease(void);
		void BaffleSet(void);

//	Overriding MacApp Methods	.......................................................			
		void Draw(void);																						// draws the baffle
		pascal void Free(void);
	};
	
// *********************************************************************************
//	TObstacle Class			=	the Obstacles
// *********************************************************************************
class TObstacle : public TObject
	{
	protected:
		TPoint	*	fUpLeft;																					// cornors of obstacle
		TPoint	*	fUpRight;
		TPoint	*	fLwLeft;
		TPoint	*	fLwRight;
		
	public:
		void IObstacle(PointInfo *);																		// initialize obstacle list
		void SetCorners(PointInfo *);

//	Overriding MacApp Methods	.......................................................			
		void Draw (void);																						// draws the obstacle
		void ReleaseObstacle(void);
		void SetObstacle(void);
		pascal void Free(void);
	};
	
// *********************************************************************************
//	TPointMatrix Class			=	List of Rows of Points
//		This is the OverSeer point list
// *********************************************************************************
class TPointMatrix : public TList
	{
// ----------------------------------------------------------------------------------------------------
//	protected variables & methods 
// ----------------------------------------------------------------------------------------------------
	protected:
		TView			* 	fGeomView;
		TDocument	*	fDocument;
		TList			*	aColumn;																				// a column
		Boolean			fShown;																					// is grid on screen
		Boolean			fFineGrid;
		short			fRow, fColumn;																			// current row & column
		short			flocation;																					// index of the point
		short			fOBoundry;																				// opposite boundry
// ----------------------------------------------------------------------------------------------------
//	public variables & methods 
// ----------------------------------------------------------------------------------------------------
	public:
		short			fBoundry;																					// current boundry
		TBoundry		*	fTop;
		TBoundry		* 	fBottom;
		TBoundry		* 	fLeft;
		TBoundry		* 	fRight;
		
		Boolean 	IPointMatrix(TDocument *, TView *);
		void 			InitCorners(void);
		pascal void Free(void);
		TPoint	* 	GetCurrent(void);																			// current row / column
		short  		SetCurrent(TPoint * );																	// set new current row/column
		TRow	* 	GetCurrentBoundry(void);																// return current boundry
		TPoint	* 	GetOpposite(TBoundry *, short, short);										// point on opposite boundry
		short		GetRow(void)						{return fRow;}									// returns current row
		short		GetColumn(void)					{return fColumn;}								// returns current row
		short		GetTColumn(void);																		// how many columns?
		short		GetNumBaffle(Boolean);																// return number baffles
		short		GetNumObstacle(void);																	// return number baffles
		short		SetCornors(TPoint * , PointInfo *);												// set surrounding points
		Boolean		IsShown(void)						{return fShown;}								// is grid displayed?
		void			SetFineGrid(Boolean value)	{fFineGrid = value;}							// fine grid on/off
		Boolean		GetFineGrid(void)					{return fFineGrid;}							// fine grid on/off
		
//	Drawing methods	.......................................................	
		void DoDraw(void);																						// draw all
		void DoDrawSegment(void);																			// draw a segment
		void DoDrawGrid(void);																				// draw the grid only
		void DoHighlight(void);																					// highlight segment or point
		void DrawHold(Boolean);
		void ShowFineGrid(void);																				// draw the fine grid
		void CalculateFinePt(lineSlope *, short, extended, Point *);

//	boundry methods	.......................................................	
		TPoint * 		FindBoundryPoint(Point * );														// find a point on boundry
		short 			FindBoundry(TPoint * );															// which boundry is point on
		TPoint * 		FindSection(Point * );																// find line section point is on
		TSegPoint *	FindSegment(TPoint * ,TRow * );												// find line segment point is on
		TPoint * 		FindCell(Point *, PointInfo *);
		void  			DefineSearchRegion(PointInfo *, RgnHandle, short,short,short,short);
		short			FindLine (Point *, PointInfo *);

		void TBBoundryDo(pascal void (*DoToItem)(TObject *, void *), void *);		// do to top / bottom boundry
		void RLBoundryDo(pascal void (*DoToItem)(TObject *, void *), void *);		// do to left/ right boundry

		void  SelectSegment(TPoint * , TRow * , Boolean );										// select all sections in a segment
		void  SetSegment(TPoint * , TRow * , short );												// change segment type

//	grid & boundry methods ... work when grid is present on entire mesh
		TPoint * 	FindPoint(Point * );																		// find any pt (boundry / grid)
		short 		SearchColumnSection(short , Point *, PointInfo *);						// search a column for section
		TPoint *	SearchColumnPoint(short , Point * );											// search a column for point

		pascal void Each(pascal void (*DoToItem)(TObject *, void *), void * );		// do to each point
		void			OneRowDo(pascal void (*DoToItem)(TObject *, void *), void *);	// do to a row
		void 			OneColumnDo(pascal void (*DoToItem)(TObject *, void *), void * ); // do to a column

//	Add segment methods	.......................................................	
		Boolean			AddAt(TPoint * , TPoint * , short , short );								// add a new point
		TGPoint 	*	GetOpposite(Point * , PointInfo * , PointInfo * );						// calculate opposite point

//	Grid creation / manipulation methods	.......................................................			
		Point			CalculateLocation(lineSlope *, extended, extended);
		TRow	*		CreateGridRow(short);
		TRow	* 		CreateGridColumn(short);
		void				RememberColumn(void);
		void 				FixHold(void);
		void				FixHoldColumn(short);
		void 				RetrieveAndFix2Hold(void);
		void 				RetrieveAndFixColumn(short);
		void 				MovePoints(Point,extended);
		void 				MoveRow(Point,extended);
		void 				MoveColumn(Point,extended);
		
//	Grid deletion  methods	.......................................................			
		short			ObsBaffInLine(TBoundry *);
		TList	* 		DeleteCurrent(TRow *);
		TPoint	*		AddLine(TList *,TRow *,short,short);

//	File read / write methods	.......................................................			
		void 				WriteGrid(TOutput *, short, short);
		void 				WriteBaffles(Boolean, TOutput *, short);
		void 				WriteObstacles(TOutput *, short);
		short 			GetColumnLength(short);
		void 				ReadGrid(short, char *);
		Boolean			ReadSegment(short, short, char *, short);
		void 				ReadData(WallRecord *, short, char *);
		Boolean			FillGrid(short, short);
		void 				ReadObstructions(short, char *);
	};

