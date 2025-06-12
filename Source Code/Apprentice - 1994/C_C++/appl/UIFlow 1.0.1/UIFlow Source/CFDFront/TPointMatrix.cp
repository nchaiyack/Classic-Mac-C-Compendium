#pragma segment Matrix
// **********************************************************************
//	TPointMatrix Class
//		Contains a list of grid rows (list of TGridRow).
//		Each row contains a list of points (list of TPoint).
//		This class is responsible for managing the point information in the geometry.
//		including creating new points, draggin, obstacles etc.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
Boolean TPointMatrix::IPointMatrix(TDocument * tDocument, TView * tGeom)	
	{
	TCornorPoint 	*	nCPt1, * nCPt2, * nCPt3, * nCPt4, * nCPt5;		// new cornor point
	TRow				*	tRow;																	// new row of points
	
	fDocument	= tDocument;																// store the document pointer
	fGeomView	= tGeom;																		// store the geomview pointer
	inherited::IList();																				// create the list

//	Create the points
	nCPt1 = new TCornorPoint;															// allocate space for point object
		FailNIL(nCPt1);																			// LAM
	nCPt1->IPoint(0, 0, Wall,true,((TGeomView *) fGeomView)->fMagnify);// initialize Cornor Point

	nCPt2 = new TCornorPoint;															// allocate space for point object
		FailNIL(nCPt2);																			// LAM
	nCPt2->IPoint(0, 0, Wall,true,((TGeomView *) fGeomView)->fMagnify);	// initialize Cornor Point

	nCPt3 = new TCornorPoint;															// allocate space for point object
		FailNIL(nCPt3);																			// LAM
	nCPt3->IPoint(0, 0, Wall,true,((TGeomView *) fGeomView)->fMagnify);		// initialize Cornor Point

	nCPt4 = new TCornorPoint;															// allocate space for point object
		FailNIL(nCPt4);																			// LAM
	nCPt4->IPoint(0, 0, Wall,true,((TGeomView *) fGeomView)->fMagnify);	// initialize Cornor Point

	nCPt5 = new TCornorPoint;															// allocate space for point object
		FailNIL(nCPt5);																			// LAM
	nCPt5->IPoint(0, 0, Wall,true,((TGeomView *) fGeomView)->fMagnify);	// initialize Cornor Point

	tRow = new TRow;																		// allocate space for the new row
		FailNIL(tRow);																			// LAM
	tRow->IRow(nCPt1,nCPt2);															// initialize the row
	this->InsertFirst((TObject *) tRow);											// insert the row
	
	tRow = new TRow;																		// allocate space for the new row
		FailNIL(tRow);																			// LAM
	tRow->IRow(nCPt3,nCPt4);															// initialize the row
	this->InsertLast((TObject *) tRow);												// insert the row
	
	aColumn = new TList;
	aColumn->IList();
	
	fTop			= new TBoundry;
		FailNIL(fTop);
	fBottom	= new TBoundry;
		FailNIL(fBottom);
	fLeft			= new TBoundry;
		FailNIL(fLeft);
	fRight		= new TBoundry;
		FailNIL(fRight);
		
	fTop->IRow(nCPt1,nCPt2,cTop);
	fBottom->IRow(nCPt3,nCPt4,cBottom);
	fLeft->IRow(nCPt1,nCPt3,cLeft);
	fRight->IRow(nCPt2,nCPt5,cRight);
		
	fBoundry	= cTop;
	fShown		= true;
	fFineGrid	= false;
	fRow		= fColumn = 1;
	return true;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetCurrentBoundry
//		returns the boundry object for the current boundry
// --------------------------------------------------------------------------------------------------
void TPointMatrix::InitCorners(void)	
	{
	Point	ul, ur, ll, lr;
	TCornorPoint 	*	nCPt1;																// new cornor point

//	Find the cornor points of the initial geometry.  This geometry is a rectangle.
//	It's size is based on the size entered by the used in the dimensions dialog.
	ul.v	=	cGeomViewBorder;
	ul.h 	= 	cGeomViewBorder;
	ur.v	= 	cGeomViewBorder;
	ll.h 	= 	cGeomViewBorder;
	ur.h	=	((TGeomView *) fGeomView)->GetHorizViewSize() + cGeomViewBorder;
	ll.v	=	((TGeomView *) fGeomView)->GetVertViewSize() + cGeomViewBorder;
	lr.v	=	((TGeomView *) fGeomView)->GetVertViewSize() + cGeomViewBorder;
	lr.h	=	((TGeomView *) fGeomView)->GetHorizViewSize() + cGeomViewBorder;

//	Create the points
	nCPt1 = (TCornorPoint *) fTop->First();										// get point
		FailNIL(nCPt1);																			// LAM
	nCPt1->SetStart(ul.v, ul.h);															// initialize Cornor Point

	nCPt1 = (TCornorPoint *) fTop->Last();										// get point
		FailNIL(nCPt1);																			// LAM
	nCPt1->SetStart(ur.v, ur.h);														// initialize Cornor Point

	nCPt1 = (TCornorPoint *) fBottom->First();									// get point
		FailNIL(nCPt1);																			// LAM
	nCPt1->SetStart(ll.v, ll.h);															// initialize Cornor Point

	nCPt1 = (TCornorPoint *) fBottom->Last();									// get point
		FailNIL(nCPt1);																			// LAM
	nCPt1->SetStart(lr.v, lr.h);															// initialize Cornor Point

	nCPt1 = (TCornorPoint *) fRight->Last();										// get point
		FailNIL(nCPt1);																			// LAM
	nCPt1->SetStart(lr.v, lr.h);															// initialize Cornor Point
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	Free
//		returns the boundry object for the current boundry
// --------------------------------------------------------------------------------------------------
pascal void TPointMatrix::Free(void)	
	{
	short 	i,ii;
	TRow 	* tRow;
	TPoint	* tPt;
	for (ii = (short) this->fSize; ii > 0; ii--)
		{
		tRow = (TRow *) this->At(ii);
		for (i = (short) tRow->fSize; i > 0; i--)
			{
			tPt = (TPoint *) tRow->At(i);
			if (tPt->fObsUpLeft != NULL)
				FreeIfObject(tPt->fObsUpLeft);
			if (tPt->fBaffAbove != NULL)
				FreeIfObject(tPt->fBaffAbove);
			if (tPt->fBaffLeft != NULL)
				FreeIfObject(tPt->fBaffLeft);
			
			tRow->Delete(tPt);
			FreeIfObject(tPt);
			}
		this->Delete(tRow);
		FreeIfObject(tRow);
		}
	FreeIfObject(aColumn);
	inherited::Free(); 
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetCurrentBoundry
//		returns the boundry object for the current boundry
// --------------------------------------------------------------------------------------------------
TRow * TPointMatrix::GetCurrentBoundry(void)	
	{
	switch (fBoundry)
		{
		case cTop:
			return fTop;
		case cBottom:
			return fBottom;
		case cLeft:
			return fLeft;
		case cRight:
			return fRight;
		}
	return NULL;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetOpposite
//		returns the boundry object for the current boundry
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::GetOpposite(TBoundry * side, short row, short column)	
	{
	short		s, i;
	TBoundry	* ss;
	
	s = side->GetSide();
	switch (s)
		{
		case cTop:
			i	= column;
			ss = fBottom;
			break;
		case cBottom:
			i	= column;
			ss = fTop;
			break;
		case cLeft:
			i	= row;
			ss = fRight;
			break;
		case cRight:
			i	= row;
			ss = fLeft;
			break;
		}
	return (TPoint *) ss->At(i);
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetTColumn .... 
//		returns the number of columns in the grid
// --------------------------------------------------------------------------------------------------
short TPointMatrix::GetTColumn(void)
	{
	TRow * tRow;
	tRow = (TRow *) this->First();
	return ((short) tRow->fSize);
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetNumBaffle .... 
//		returns the number of baffles in the grid
// --------------------------------------------------------------------------------------------------
short TPointMatrix::GetNumBaffle(Boolean vert)
	{
	TRow 	* tRow;
	short 	i, total;
	
	total = 0;
	for (i = 1; i <= (short) fSize; i++)
		{
		tRow = (TRow *) this->At(i);
		total += tRow->GetNumBaffle(vert);
		}
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetNumBaffle .... 
//		returns the number of baffles in the grid
// --------------------------------------------------------------------------------------------------
short TPointMatrix::GetNumObstacle(void)
	{
	TRow 	* tRow;
	short 	i, total;
	
	total = 0;
	for (i = 1; i <= (short) fSize; i++)
		{
		tRow = (TRow *) this->At(i);
		total += tRow->GetNumObstacle();
		}
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetCurrent .... 
//		returns the current point described by fRow & fColumn
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::GetCurrent(void)
	{
	TRow * tRow;
	if (fRow <= fSize)
		tRow = (TRow *) this->At(fRow);
	if (fColumn <= (short) tRow->fSize)
		return ((TPoint *) tRow->At(fColumn));
	return NULL;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SetCurrent .... 
//		sets fRow & fColumn to the point passed in
// --------------------------------------------------------------------------------------------------
short TPointMatrix::SetCurrent(TPoint * tPt)
	{
	short x,y;
	TRow	* tRow;
	TPoint	* t;
	
	t = (TPoint *) fRight->Last();														// check lower right corner
	if (t == tPt)
		{
		fColumn	= (short) fBottom->fSize;
		fRow		= (short) fRight->fSize;
		return 0;
		}
		
	for (y=1; y<= (short) fSize; y++)
		{
		tRow = (TRow *) this->At(y);
		if ((x = (short) tRow->GetSameItemNo((TObject *) tPt)) !=  0)
			{
			fRow = y;
			fColumn = x;
			if (fRow == 1)
				fBoundry = cTop;
			else if (fRow == (short) this->fSize)
				fBoundry = cBottom;
			else if (fColumn == 1)
				fBoundry = cLeft;
			else if (fColumn == (short) ((TRow *) this->At(fRow))->fSize)
				fBoundry = cRight;
			return 0;
			}
		}
	return 1;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SetCornors .... 
//		places the surrounding points in the PointInfo structure
// --------------------------------------------------------------------------------------------------
short TPointMatrix::SetCornors(TPoint * tPt, PointInfo * info)
	{
	TRow *	tRow;
	if (this->SetCurrent(tPt) == 1)														// set fRow/fColumn to this point
		return 1;
	
	info->left = info->right = info->above = info->below = info->oPt = NULL;
	info->aboveRow = info->belowRow = NULL;
	
	info->row = fRow;																		// current row
	info->column = fColumn;																// current column
	info->gridOnly = !fShown;																// assign opposite of fShown
	info->lrc = NULL;
	
	if (tPt->IsBoundryPt())																	// only load boundry
		{
		TBoundry * side;
		side = (TBoundry *) this->GetCurrentBoundry();
		info->boundry = fBoundry;
		if (tPt->IsCornor())																	// point is on a cornor
			{
			if (fRow == 1 && fColumn == 1)											// upper left cornor
				{
				info->left		= (TPoint *) fLeft->At(2);
				info->right	= (TPoint *) fTop->At(2);
				info->dLeft	= info->dRight = true;
				return 0;
				}
			if (fRow == 1 && fColumn == (short) fTop->fSize)				// upper right cornor
				{
				info->left		= (TPoint *) fTop->At((fTop->fSize) - 1);
				info->right	= (TPoint *) fRight->At(2);
				info->dLeft	= false;
				info->dRight	= true;
				return 0;
				}
			if (fRow == (short) fSize && fColumn == (short) fBottom->fSize) // lower right cornor
				{
				info->left		= (TPoint *) fRight->At((fRight->fSize) - 1);
				info->right	= (TPoint *) fBottom->At((fBottom->fSize)-1);
				info->lrc		= (TPoint *) fRight->Last();
				info->dLeft	= false;
				info->dRight	= false;
				return 0;
				}
			info->left		= (TPoint *) fBottom->At(2);							// lower left cornor
			info->right	= (TPoint *) fLeft->At((fLeft->fSize)-1);
			info->dLeft	= true;
			info->dRight	= false;
			return 0;
			}
			
		tRow = (TRow *) this->At(fRow);												// get current row
		if (fBoundry == cLeft || fBoundry == cRight)
			{
			info->left		= (TPoint *) side->At(fRow - 1);						// get boundry points on
			info->right	= (TPoint *)  side->At(fRow + 1);						//   either side of current
			if (fShown)																			// is grid on?
				{																						// get grid points
				if (fBoundry == cLeft)														// left boundry
					info->above = (TPoint *) tRow->At(2);
				else																					// right boundry
					info->above = (TPoint *) tRow->At((tRow->fSize) - 1);
				}
			return 0;
			}
		else																							// top & bottom boundry
			{
			info->left		=	(TPoint *) side->At(fColumn - 1);					// get boundry points on
			info->right	=	(TPoint *) side->At(fColumn + 1);				// either side of current
			if (fShown)																			// is grid on?
				{
				if (fBoundry == cTop)														// get grid points
					info->above	=	(TPoint *) ((TRow *) this->At(2))->At(fColumn); // top boundry
				else																					// bottom boundry
					info->above	=	(TPoint *) ((TRow *) this->At((short) fSize - 1))->At(fColumn);
				}
			return 0;
			}
		}

	if (!fShown)																					// no grid 
		return 1;

//	grid is on so load grid points too.		
	tRow = (TRow *) this->At(fRow);													// get current row
	info->left = (TPoint *) tRow->At(fColumn - 1);								// point to the left of this
	info->right = (TPoint *) tRow->At(fColumn + 1);							// point to the left	
	info->aboveRow = (TRow *) this->At(fRow - 1);							// row above current
	info->above = (TPoint *) (info->aboveRow)->At(fColumn);			// point above current
	info->belowRow = (TRow *) this->At(fRow + 1);							// get next row
	info->below = (TPoint *) (info->belowRow)->At(fColumn);			// point below current
	return 0;
	}
	
// *********************************************************************************
//	Drawing Methods
// *********************************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	Draw
//		Draws the points, segment lines & grid lines
// --------------------------------------------------------------------------------------------------
void TPointMatrix::DoDraw(void)	
	{
	PointInfo info;
	
	info.gridOnly = false;
	info.noGrid	= fShown;
	info.magnify	= ((TGeomView *) fGeomView)->fMagnify;
	info.lrc = NULL;
	HLock((Handle) this);
	if (fShown)
		{
		this->Each(DrawAllLines,&info);
		this->Each(DrawAllPoints,&info);
		}
	else
		{
		info.boundry = cTop;																							// top boundry
		this->TBBoundryDo(DrawAllLines,&info);
		this->TBBoundryDo(DrawAllPoints,&info);
		info.boundry = cBottom;																					// bottom boundry
		this->TBBoundryDo(DrawAllLines,&info);
		this->TBBoundryDo(DrawAllPoints,&info);

		info.boundry = cRight;																						// right boundry
		this->RLBoundryDo(DrawLine,&info);
		this->RLBoundryDo(DrawPoint,&info);
		info.boundry = cLeft;																						// left boundry
		this->RLBoundryDo(DrawLine,&info);
		this->RLBoundryDo(DrawPoint,&info);
		}
	HUnlock((Handle) this);

	if (fFineGrid)
		this->ShowFineGrid();
	if (((TGeomView *) fGeomView)->fSPoint != NULL)
		this->DoHighlight();
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	Draw
//		Draws the points & grid lines
// --------------------------------------------------------------------------------------------------
void TPointMatrix::DoDrawGrid(void)	
	{		
	PointInfo info;
	
	fGeomView->Focus();
	info.gridOnly = true;
	info.noGrid	=	fShown;
	info.magnify	= ((TGeomView *) fGeomView)->fMagnify;
	HLock((Handle) this);
	this->Each(DrawAllLines,&info);
	this->Each(DrawAllPoints,&info);
	HUnlock((Handle) this);
	
	fShown = !fShown;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	DrawSegment .... 
//		draws each section in this segment
// --------------------------------------------------------------------------------------------------
void  TPointMatrix::DoDrawSegment(void)
	{
	short 		index;																		// counter
	TRow 	*	side;																			// the side the boundry is on
	Point		pt;
	TPoint	*	tPt1, * tPt2;															// the point

	side 	= ((TGeomView *) fGeomView)->fSSide;
	this->SetCurrent (((TGeomView *) fGeomView)->fSPoint);
	
	if (side->GetSide() == cTop  || side->GetSide() == cBottom)
		index = fColumn;
	else
		index = fRow;
		
	while (index > 1)																		// do until previous segment
		{
		tPt1 = (TPoint *) side->At(index);
		tPt2 = (TPoint *) side->At(index-1);
		
		pt = tPt2->fTrans;
		tPt1->DrawSection(pt);
		tPt1->Draw();

		if (tPt2->IsSegment())
			break;
		index--;
		}

	tPt2->Draw();
	return;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	Highlight .... 
//		highlights the point or the segment
// -------------------------------------------------------------------------------------------- -----
void  TPointMatrix::DoHighlight(void)
	{
	if (((TGeomView *) fGeomView)->fSPoint ==NULL)
		return;
		
	fGeomView->Focus();
	if (!((TGeomView *) fGeomView)->fSegment)
		{
		Point t;
		t = ((TGeomView *) fGeomView)->fSPoint->fStart;
		((TGeomView *) fGeomView)->fSPoint->Highlight();				// highlight only the point
		return;
		}

	short 		index;																		// counter
	TRow 	*	side;																			// the side the boundry is on
	Point		pt;
	TPoint	*	tPt1, * tPt2;															// the point

	side 	= ((TGeomView *) fGeomView)->fSSide;
	this->SetCurrent (((TGeomView *) fGeomView)->fSPoint);

	if (side->GetSide() == cTop  || side->GetSide() == cBottom)
		index = fColumn;
	else
		index = fRow;
		
	while (index > 1)																		// do until previous segment
		{
		tPt1 = (TPoint *) side->At(index);
		tPt2 = (TPoint *) side->At(index-1);
		
		tPt1->Highlight();
		pt = tPt2->fTrans;
		tPt1->HighlightSection(pt);

		if (!tPt2->IsSelected())
			break;
		index--;
		}

	tPt2->Highlight();
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ShowFineGrid .... 
//		Displays the fine grid
// -------------------------------------------------------------------------------------------- -----
void  TPointMatrix::ShowFineGrid(void)
	{
//
//	draw the fine grid lines.
//	draw all the fine rows from bottom right corner to top left corner
//	then draw all the fine columns from bottom right corner to top left corner
//				Point / Row Order: 
//	Row 2			4	¥				3	¥
//			
//
//	Row 1			2	¥				1	¥
//
	lineSlope	line1, line2;
	short 		gridLines;
	short		xx, yy, i, temp;
	TRow		* tR1, * tR2;
	TPoint		* tPt;
	Point		pt1[16], pt2[16];
	extended	interval1, interval2, dist1, dist2;
	
	temp = (short) ((TCFDFrontDocument *) fDocument)->GetFineGrid();
	gridLines = (short) pow(2,(temp-1));
	if (gridLines == 1)
		return;
		
	fGeomView->Focus();
	PenMode(srcXor);
	RGBForeColor(&FineGridColor);														// forground color.
	PenPixPat(FineGridPat);
	PenSize (gGridsize_H, gGridsize_V);													// set pensize

//	horizontal fine grid lines
	tR2 = (TRow *) this->Last();															// get the bottom row
	for (yy = (short) this->fSize-1; yy > 0; yy--)
		{
		tR1 = tR2;																					// assign tr1 to previous tr2
		tR2 = (TRow *) this->At(yy);														// get row above tr1
		
		line1.first		= ((TPoint *) tR1->Last())->fTrans;
		line1.second	= ((TPoint *) tR2->Last())->fTrans;
		HLock((Handle) this);
		SlopeIntercept(&line1);
		dist1 = CalculateDistance(line1.first, line1.second);						// get length of cell segments
		interval1 = dist1 / gridLines;
		
		this->CalculateFinePt(&line1, gridLines, interval1, pt1);
		HUnlock((Handle) this);
		
		for (xx = (short) tR1->fSize-1; xx > 0; xx--)								// do all points in the row
			{
			tPt				= (TPoint *) tR1->At(xx+1);
			line2.first		= ((TPoint *) tR1->At(xx))->fTrans;
			line2.second	= ((TPoint *) tR2->At(xx))->fTrans;
			HLock((Handle) this);
			SlopeIntercept(&line2);
			dist2 = CalculateDistance(line2.first, line2.second);
			interval2 = dist2 / gridLines;														// length of each sub-cell
			
			this->CalculateFinePt(&line2, gridLines, interval2, pt2);
			HUnlock((Handle) this);
			for (i = 0; i < gridLines-1; i++)
				{
				if (tPt->fObsUpLeft == NULL)
					{
					MoveTo(pt1[i].h+1,pt1[i].v+1);												// move to provided point
					LineTo (pt2[i].h+1, pt2[i].v+1);												// draw to this point
					}
				pt1[i].h = pt2[i].h; 
				pt1[i].v = pt2[i].v;  
				}
			}
		}

//	vertical fine grid lines
	for (xx = (short) tR1->fSize-1; xx > 0; xx--)									// do all points in the row
		{
		tR2 = (TRow *) this->Last();														// get the bottom row
		tR1 = tR2;																					// assign tr1 to previous tr2
		
		line1.first		= ((TPoint *) tR1->At(xx+1))->fTrans;
		line1.second	= ((TPoint *) tR1->At(xx))->fTrans;
		HLock((Handle) this);
		SlopeIntercept(&line1);
		dist1 = CalculateDistance(line1.first, line1.second);						// get length of cell segments
		interval1 = dist1 / gridLines;
		
		this->CalculateFinePt(&line1, gridLines, interval1, pt1);
		HUnlock((Handle) this);
		
		for (yy = (short) this->fSize-1; yy > 0; yy--)
			{
			tR1 = tR2;																				// assign tr1 to previous tr2
			tR2 = (TRow *) this->At(yy);													// get row above tr1
			tPt				= (TPoint *)  tR1->At(xx+1);
			line2.first		= ((TPoint *) tR2->At(xx+1))->fTrans;
			line2.second	= ((TPoint *) tR2->At(xx))->fTrans;
			HLock((Handle) this);
			SlopeIntercept(&line2);
			dist2 = CalculateDistance(line2.first, line2.second);
			interval2 = dist2 / gridLines;													// length of each sub-cell
			
			this->CalculateFinePt(&line2, gridLines, interval2, pt2);
			HUnlock((Handle) this);
			for (i = 0; i < gridLines-1; i++)
				{
				if (tPt->fObsUpLeft == NULL)
					{
					MoveTo(pt1[i].h+1,pt1[i].v+1);										// move to provided point
					LineTo (pt2[i].h+1, pt2[i].v+1);												// draw to this point
					}
				pt1[i].h = pt2[i].h;
				pt1[i].v = pt2[i].v;  
				}
			}
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	CalculateFinePt .... 
//		Calculates the location of the fine grid  points
// -------------------------------------------------------------------------------------------- -----
void  TPointMatrix::CalculateFinePt(lineSlope * line, short grid, extended dist, Point * p)
	{
	short		z, temp;
	extended	t1, t2, t3, x, y;
	extended  	total;
	
	x = line->first.h;
	y = line->first.v;
	temp  = 0;
	for (z = 0; z < grid-1; z++)																// one for each subdivision - up to 15
		{
		if (line->vertical)																			// is segment vertical?
			{
			if (line->first.v < line->second.v)
				y += dist;
			else
				y -= dist;

 			p[z].h = (short) x;
			p[z].v = (short) y;
			}
		else if (line->horizontal)																// is segment horizontal?
			{
			if (line->first.h < line->second.h)
				x += dist;
			else
				x -= dist;

			p[z].h = (short) x;
			p[z].v = (short) y;
			}
		else
			{
			t1 = dist / (sqrt((line->m * line->m) + 1));
			if (temp == 0 || temp == 1)
				{
				x += t1;
				y = (line->m * x) + line->b;
				
				p[z].h = (short) x;
				p[z].v = (short) y;
				
				if (temp == 0)
					{
					t2 = CalculateDistance(line->first,p[z]);
					t3 = CalculateDistance(line->second,p[z]);
					total = CalculateDistance(line->first, line->second);
					
					if (t2 + t3 - 1 > total)													// between the end points?
						{
						x = line->first.h;
						y = line->first.v;
						temp = 2;
						}
					else
						temp = 1;
					}
				}
											
			if (temp == 2)
				{
				x -= t1;
				y = (line->m * x) + line->b;

				p[z].h = (short) x;
				p[z].v = (short) y;
				}
			}
		}
	}		

// *********************************************************************************
//	Boundry Methods
// *********************************************************************************	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	RLBoundryDo
//		Do the supplied method for left or right boundry
// --------------------------------------------------------------------------------------------------
void TPointMatrix::RLBoundryDo(pascal void (*DoToItem)(TObject *, void *), void *tinfo)
	{
	TBoundry 	*	side;																				// the boundry
	TPoint		*	tPt;																				// the point
	PointInfo	*	info;																				// information structure
	short		index;																				// counter
	
	info 			= (PointInfo *) tinfo;
	
	if (fSize > 0)																						// any entries?
		{
		tPt						= NULL;
		info->belowRow	= NULL;
		info->below			= NULL;															// no points below on either boundry
		info->aboveRow	= NULL;
		info->above			= NULL;
		info->oPt				= NULL;
		info->lrc				= NULL;
			
		if (info->boundry == cRight)
			side = fRight;
		else
			{
			side = fLeft;
			info->column			= 1;
			}
		info->left = (TPoint *) side->Last();
			
		for (index = (short) side->fSize; index > 0; index--)
			{
			info->right 	= tPt;
			tPt				= info->left;
			if (index > 1)
				info->left = (TPoint *) side->At(index-1);
			else
				info->left = NULL;
			info->row = index;

			DoToItem((TObject *) tPt,info);
			}
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	For Each point in the list Do the DoToItem Function
// --------------------------------------------------------------------------------------------------
void TPointMatrix::TBBoundryDo(pascal void (*DoToItem)(TObject *, void *), void *tInfo)
	{
	TRow		*	tRow;																		// the row
	PointInfo	*	info;																			// information structure
	
	info = (PointInfo *) tInfo;																// cast the structure
	info->lrc				= NULL;
	if (fSize > 0)																					// anything in the list?
		{
		if (info->boundry == cBottom)													// bottom row
			{
			tRow					= fBottom;												// last row
			info->aboveRow	= NULL;
			info->belowRow	= NULL;
			info->row				= (short) fSize;
			}
		else																							// top ro÷
			{
			tRow					= fTop;														// last row
			info->belowRow	= (TRow *) this->At(2);							// get 2nd row
			info->aboveRow	= NULL;
			info->row				= 1;
			}
		
		DoToItem((TObject *)tRow,info);												// perform the function
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindBoundry
// --------------------------------------------------------------------------------------------------
short TPointMatrix::FindBoundry(TPoint * tPt)
	{
	short	rIndex;
	
	rIndex = (short) fTop->GetSameItemNo((TObject *) tPt);					// search this row for mouse
	if (rIndex != 0)	
		{
		fColumn	= rIndex;
		fRow		= 1;
		return cTop;																					// found point
		}
		
	rIndex = (short) fBottom->GetSameItemNo((TObject *) tPt);				// search this row for mouse
	if (rIndex != 0)
		{
		fColumn 	= rIndex;
		fRow		= (short) this->Last();
		return cBottom;																			// found point
		}
		
	rIndex = (short) fLeft->GetSameItemNo((TObject *) tPt);					// search this row for mouse
	if (rIndex != 0)
		{
		fRow		=	rIndex;
		fColumn	=	1;
		return cLeft;																				// found point
		}

	rIndex = (short) fRight->GetSameItemNo((TObject *) tPt);				// search this row for mouse
	if (rIndex != 0)
		{
		fRow		=	rIndex;
		fColumn	=	(short) ((TRow *) this->At(fRow))->fSize;
		return cRight;																				// found point
		}
	
	return NULL;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindBoundryPoint
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::FindBoundryPoint(Point * tMouse)
	{
	short 	index;
	
	index	=	fTop->SearchPoint(tMouse);												// search this row for mouse
	if (index > 0)
		{
		fBoundry		= cTop;
		fColumn		= index;
		fRow			= 1;
		return (TPoint *) fTop->At(index);												// found point
		}
				
	index	=	fBottom->SearchPoint(tMouse);										// search this row for mouse
	if (index > 0)
		{
		fBoundry		= cBottom;
		fColumn		= index;
		fRow			= (short) this->fSize;
		return  (TPoint *) fBottom->At(index);
		}

	index	=	fLeft->SearchPoint(tMouse);												// search this row for mouse
	if (index > 0)
		{
		fBoundry		= cLeft;
		fColumn		= 1;
		fRow			= index;
		return  (TPoint *) fLeft->At(index);
		}

	index	=	fRight->SearchPoint(tMouse);											// search this row for mouse
	if (index > 0)
		{
		fBoundry		= cRight;
		fColumn		= (short) ((TRow *) this->At(index))->fSize;
		fRow			= index;
		return  (TPoint *) fRight->At(index);
		}
	
	return NULL;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindBoundryPoint
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::FindPoint(Point * tMouse)
	{
	short x, index;
	TRow	*	tRow;
	TPoint	*	tPt;
	
	tPt = this->FindBoundryPoint(tMouse);												// boundries first
	if (tPt != NULL)																					// if found point return it
		return tPt;
		
	if (!fShown)																						// no grid
		return NULL;																					// return not found
		
	fBoundry = cNotOnBoundry;																// set current boundry
	for (x = 2; x <= (short) (fSize - 1); x++)											// search grid rows
		{
		tRow = (TRow *) this->At(x);														// get the row
		index = tRow->SearchPoint(tMouse);											// search it
		if (index > 0)																				// if found return the point
			{
			fRow = x;
			fColumn = index;
			return (TPoint *) ((TRow *) this->At(x))->At(index);
			}
		}
	return NULL;																						// not a point
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindSegment
//		Determines which segment a particular boundry section is contained in.
// --------------------------------------------------------------------------------------------------
TSegPoint * TPointMatrix::FindSegment(TPoint * tPt, TRow * tside)
	{
	short 		rIndex;
	short 		side, index;
	short 		e;
				
//	this->SetCurrent(tPt);
	e = (short) tside->fSize;
	side = tside->GetSide();
	if (side == cTop)
		{
		if (fColumn == 1)
			index = 2;
		else
			index = fColumn;
		}
	else if (side == cBottom)
		index = fColumn;
	else
		index = fRow;
		
	for (rIndex =  index; rIndex <= e; rIndex++)
		{
		tPt = (TPoint *) tside->At(rIndex);
		if (tPt->IsSegment())
			return (TSegPoint *) tPt;
		}		
	return NULL;
	}
		
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindSection .... 
//		determine which boundry section a segment or grid point is in
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::FindSection(Point * tMouse)
	{	
	if ((flocation = fTop->SearchSection(tMouse)) > 0)								// found the section?
		{
		fRow			= 1;																				// set the current row
		fColumn		= flocation;																	// set the current column
		fBoundry		= cTop;
		fOBoundry	= cBottom;
		return ((TPoint *) fTop->At(flocation));
		}
	
	if ((flocation = fBottom->SearchSection(tMouse)) > 0)							// found section?
		{
		fRow 			= (short) fSize;															// set current row
		fColumn		= flocation;																	// set current column
		fBoundry		= cBottom;
		fOBoundry	= cTop;
		return ((TPoint *) fBottom->At(flocation));
		}

	if ((flocation = fLeft->SearchSection(tMouse)) > 0)								// test left boundry	
		{
		fRow			= flocation;																	// set current row
		fColumn		= 1;																				// set current column
		fBoundry		= cLeft;
		fOBoundry	= cRight;
		return ((TPoint *) fLeft->At(flocation));
		}

	if ((flocation = fRight->SearchSection(tMouse)) > 0) 								// test right boundry
		{
		fRow			= flocation;																	// set current row
		fColumn		= (short) ((TRow *) this->At(fRow))->fSize;				// set current column
		fBoundry		= cRight;
		fOBoundry	= cLeft;
		return ((TPoint *) fRight->At(flocation));
		}
	return NULL;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindCell .... 
//		determine which cell the user has selected
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::FindCell(Point * tMouse, PointInfo * info)
	{	
// ....................................................................................................................................................................
//	This method impliments a searching algorithm to determine which grid cell was selected by the user
//	It performs a logrithmic binary search.
//	First examines the whole ... if the point is in the reqion
//	divides the whole into four pieces & locates the quadrant the point is in
//	divides that into four quadrants ... etc until only a single grid cell remains.
// ....................................................................................................................................................................
	char msg[80];
	
	if (!fShown)																								// have a grid?
		{
		SysBeep (3);
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(5,NULL);
		tWarning->ShowWarning();
		return NULL;
		}
	
	short	tRow, tCol;																					// total rows & columns
	short	sR, eR, sC, eC;																				// start & end row / column
	short	saveS, saveE;																				// save indexes
	Boolean notFound = true;																			// found the point?
	RgnHandle		rgn;																						// draw region for "cell"
	Point			mPt;																						// mouse point
	info->oPt = info->above = info->left = info->right = info->below = NULL;
	
	tRow	= eR = (short) this->fSize;															// init number rows
	tCol		= eC = (short) fTop->fSize;															// init number columns
	sR		= sC = 1;																						// begin at 1,1
	saveS	= saveE = 0;																					// init save variables
	mPt.v	= tMouse->v;
	mPt.h	= tMouse->h;
	
	rgn = NewRgn();																						// create the region handle
	while (notFound)																						// loop till find the point
		{
		this->DefineSearchRegion(info,rgn,sR,eR,sC,eC);									// draw the region
		if (PtInRgn (mPt,rgn))																			// in this region?
			{																										// yes it is!
			if (sR + 1 == eR && sC + 1 == eC)													// have only 1 cell
				{
				DisposeRgn(rgn);																			// free the region
				fRow		= sR;																			// set current row / column
				fColumn	= sC;
				sprintf(msg,"Selected Cell R %d C %d",sR,sC);
				((TCFDFrontDocument *) fDocument)->fInfoView->InfoString(msg);
				return (TPoint *) ((TRow *) this->At(sR))->At(sC);
				}
			tRow	= eR;																					// reset bottom
			tCol		= eC;
			eR		= sR + ((eR - sR) / 2);															// reset search quadrant
			eC		= sC + ((eC - sC) / 2);
		if (sR < 1 || sR > tRow || eR < 1 || eR > tRow)
			msg[0] = 0;
		if (sC < 1 || sC > tCol || eC < 1|| eC > tCol)
			msg[0] = 0;
			
			}
		else																										// not in this quadrant
			{
			if (eC < tCol)																						// left side quadrants
				{
				saveS	= sC;																				// store current start column
				saveE	= eC;																				// store current end column
				sC		= eC;																				// reset to right quadrant
				eC		= tCol;																				// reset to right quadrant
		if (sR < 1 || sR > tRow || eR < 1 || eR > tRow)
			msg[0] = 0;
		if (sC < 1 || sC > tCol || eC < 1|| eC > tCol)
			msg[0] = 0;
			
				}
			else if (eR < tRow)																			// upper quadrants
				{
				sC	= saveS;																				// reset start column to left side
				eC	= saveE;																				// reset end column to left side
				sR	= eR;																					// reset start row to bottom quadrants
				eR	= tRow;																				// reset end row to bottom quadrants
		if (sR < 1 || sR > tRow || eR < 1 || eR > tRow)
			msg[0] = 0;
		if (sC < 1 || sC > tCol || eC < 1|| eC > tCol)
			msg[0] = 0;
			
				}
			else
				{
				DisposeRgn(rgn);																			// free the region
				SysBeep (3);
				TWarning * tWarning;
				TWindow	* aWindow;
		
				aWindow = NewTemplateWindow(kWarnMe, fDocument);
				tWarning = (TWarning *) aWindow->FindSubView('WARN');
				tWarning->IWarning(4,NULL);
				tWarning->ShowWarning();
				return NULL;
				}
			}
		}
	return NULL;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	DefineSearchRegion .... 
//		Draws the region to be searched in the FindCell method
// --------------------------------------------------------------------------------------------------
void  TPointMatrix::DefineSearchRegion(PointInfo * info, RgnHandle rgn, short sR,short eR,short sC,short eC)
	{
	Point		ul, tPt;
	short		cR, cC;																						// current row / column
	
	cR = sR;																									// row to start on
	SetEmptyRgn(rgn);																					// clear the region
	OpenRgn();																								// open drawing area
	info->oPt		= (TPoint *) ((TRow *) this->At(sR))->At(sC);						// get starting point
	info->above	= (TPoint *) ((TRow *) this->At(sR))->At(eC);
	info->left		= (TPoint *) ((TRow *) this->At(eR))->At(sC);
	info->below	= (TPoint *) ((TRow *) this->At(eR))->At(eC);
	ul = info->oPt->fStart;																				// get coordinates
	MoveTo(ul.h, ul.v);																					// position pen
	
//	draw across top of the region (left to right)
	for (cC = sC; cC <= eC; cC++)																	// do all columns
		{
		tPt = ((TPoint *) ((TRow *) this->At(cR))->At(cC))->fStart;
		LineTo (tPt.h, tPt.v);
		}
	cC = eC;

//	draw down right side of the region (top to bottom)
	for (cR = sR; cR <= eR; cR++)																	// do all rows
		{
		tPt = ((TPoint *) ((TRow *) this->At(cR))->At(cC))->fStart;
		LineTo (tPt.h, tPt.v);
		}
	cR = eR;

//	draw across bottom of the region (right to left)
	for (cC = eC; cC >= sC; cC--)																	// do all columns
		{
		tPt = ((TPoint *) ((TRow *) this->At(cR))->At(cC))->fStart;
		LineTo (tPt.h, tPt.v);
		}
	cC = sC;

//	draw up left side of the region (bottom to top)
	for (cR = eR; cR >= sR; cR--)																	// do all rows
		{
		tPt = ((TPoint *) ((TRow *) this->At(cR))->At(cC))->fStart;
		LineTo (tPt.h, tPt.v);
		}
	
	CloseRgn (rgn);
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FindLine .... 
//		determine which line the user has selected
// --------------------------------------------------------------------------------------------------
short TPointMatrix::FindLine (Point * tMouse, PointInfo * info)
	{	
// ....................................................................................................................................................................
//	This method impliments a searching algorithm to determine which grid line was selected by the user
// ....................................................................................................................................................................
//	search the horizontal lines first

	short	i, tmp;
	TRow	* tRow;
	char		msg[80];
	
	info->above = info->below = info->left = info->right = NULL;
	
	for (i = 1; i <= (short) this->fSize; i++)
		{
		tRow = (TRow *) this->At(i);
		if ((tmp = tRow->SearchSection(tMouse)) > 0)
			{
			fRow		= i;																					// reset currents
			fColumn	= tmp-1;
			info->left		= (TPoint *) tRow->At(fColumn);
			info->right	= (TPoint *) tRow->At(tmp);
			sprintf(msg,"Selected Horizontal Grid Line: R [%d] C1 [%d] C2 [%d]",i,tmp-1,tmp);
			((TCFDFrontDocument *) fDocument)->fInfoView->InfoString(msg);
			return 2;
			}
		}

	for (i = 1; i <= (short) fTop->fSize; i++)
		{
		if ((tmp = this->SearchColumnSection (i,tMouse,info)) > 0)
			{
			fRow		= tmp - 1;
			fColumn	= i;
			sprintf(msg,"Selected Vertical Grid Line: R1 [%d] R2 [%d] C [%d]",tmp-1,tmp, i);
			((TCFDFrontDocument *) fDocument)->fInfoView->InfoString(msg);
			return 1;
			}
		}
	return 0;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SelectSegment .... 
//		changes the wall type for each section in this segment
// --------------------------------------------------------------------------------------------------
void  TPointMatrix::SelectSegment(TPoint * tPt, TRow * tSide, Boolean on)
	{
	short side;																								// the boundry
	short index;																								// counter
	TPoint * t;
	
	side = tSide->GetSide();																			// gets the boundry
	this->SetCurrent(tPt);																				// sets tpt as current
	
	if (side == cTop || side == cBottom)
		index = fColumn;
	else
		index = fRow;
		
	t = (TPoint *) tSide->At(index);																// get the point
	t->SetSelection(on);
	index--;
	
	while (index > 0)																						// do until begin of list
		{
		t = (TPoint *) tSide->At(index);															// get the point
		if (t->IsSegment())																				// is the point a segment
			return;																								// yes : exit loop
		t->SetSelection(on);																				// turn on
		index--;																								// next point
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SetSegment .... 
//		changes the wall type for each section in this segment
// --------------------------------------------------------------------------------------------------
void  TPointMatrix::SetSegment(TPoint * tPt, TRow * tSide, short type)
	{
	short side;																								// the boundry
	short index;																								// counter
	TPoint * t;
	
	side = tSide->GetSide();																			// gets the boundry
	this->SetCurrent(tPt);																				// sets tpt as current
	
	if (side == cTop || side == cBottom)
		index = fColumn;
	else
		index = fRow;
		
	if (side == cRight && index == fSize)
		t = (TPoint *) tSide->Last();
	else
		t = (TPoint *) tSide->At(index);															// get the point
	t->SetSectionType(type);																			// turn on
	index--;																									// next point

	while (index > 0)																						// do until begin of list
		{
		t = (TPoint *) tSide->At(index);															// get the point
		if (t->IsSegment())																				// is the point a segment
			return;																								// yes : exit loop
		t->SetSectionType(type);																		// turn on
		index--;																								// next point
		}
	return;
	}
	
// *********************************************************************************
//	Add Segment Methods
// *********************************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	AddAt .... 
//		adds the segment... if it requires a new row this method creates it
// --------------------------------------------------------------------------------------------------
Boolean TPointMatrix::AddAt(TPoint * pt1, TPoint * pt2, short row, short col)
	{
	TRow 	* tRow;																					// pointer to new row
	TRow 	* side;

	side = (TRow *) this->GetCurrentBoundry();
	
	if (fBoundry == cLeft || fBoundry == cRight)											// left or right side create a new row
		{
		tRow = new TRow;																			// allocate space for new row
		if (tRow == NULL)																				// no more space
			return false;																					// return error
		if (fBoundry == cLeft)																		// left side
			{
			fLeft->InsertBefore(row,(TObject *) pt1);
			fRight->InsertBefore(row,(TObject *) pt2);
			tRow->IRow(pt1,pt2);																	// initialize row
			}
		else
			{
			fLeft->InsertBefore(row,(TObject *) pt2);
			fRight->InsertBefore(row,(TObject *) pt1);
			tRow->IRow(pt2,pt1);																	// initialize row
			}
		this->InsertBefore(row, (TObject *) tRow);										// add row to the matrix
		
		this->CreateGridRow(row);
			
		return true;
		}
		
	if (fBoundry == cTop)
		{
		fTop->InsertBefore(col, (TObject *) pt1);
		fBottom->InsertBefore(col, (TObject *) pt2);
		tRow = (TRow *) this->First();
		tRow->InsertBefore(col, (TObject *) pt1);										// add new column to row
		tRow = (TRow *) this->Last();															// get bottom boundry
		}
	else
		{
		fTop->InsertBefore(col, (TObject *) pt2);
		fBottom->InsertBefore(col, (TObject *) pt1);
		tRow = (TRow *) this->Last();
		tRow->InsertBefore(col, (TObject *) pt1);										// add new column to row
		tRow = (TRow *) this->First();															// get top boundry
		}
	tRow->InsertBefore(col, (TObject *) pt2);											// add opposite point
	
	this->CreateGridColumn(col);	
	return true;		
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetOpposite .... 
//		determine the location of the new point on the opposite boundry
// --------------------------------------------------------------------------------------------------
TGPoint * TPointMatrix::GetOpposite(Point * tMouse, PointInfo * tInfo, PointInfo * oInfo)
	{
	lineSlope 	tSlope;
	TGPoint	* tPt;
	TPoint	 	* pt1, * ot1, * ot2;
	Point		tPt1, tPt2;
	Point		oPt1, oPt2;
	Point		nPt;
	extended 	dist1, dist2, odist;
	extended 	ratio, dist2Pt;
	
//	get the points on the opposite boundry	
	switch (fOBoundry)
		{
		case cTop:
			tPt2 = ((TPoint *) fBottom->At(fColumn))->fStart;
			pt1 = (TPoint *) fBottom->At(fColumn-1);
			ot1 = (TPoint *) fTop->At(fColumn-1);
			ot2 = (TPoint *) fTop->At(fColumn);
			break;
		case cBottom:			
			tPt2 = ((TPoint *) fTop->At(fColumn))->fStart;
			pt1 = (TPoint *) fTop->At(fColumn-1);
			ot1 = (TPoint *) fBottom->At(fColumn-1);
			ot2 = (TPoint *) fBottom->At(fColumn);
			break;
		case cLeft:
			tPt2 = ((TPoint *) fRight->At(fRow))->fStart;
			pt1 = (TPoint *) fRight->At(fRow-1);
			ot1 = (TPoint *) fLeft->At(fRow-1);
			ot2 = (TPoint *) fLeft->At(fRow);
			break;
		case cRight:
			tPt2 = ((TPoint *) fLeft->At(fRow))->fStart;
			pt1 = (TPoint *) fLeft->At(fRow-1);
			ot1 = (TPoint *) fRight->At(fRow-1);
			ot2 = (TPoint *) fRight->At(fRow);
			break;
		}
	
	tInfo->left = pt1;
	oInfo->left = ot1;
	oInfo->right = ot2;
	
	tPt1 = pt1->fStart;
	oPt1 = ot1->fStart;
	oPt2 = ot2->fStart;
	oInfo->boundry = fOBoundry;

	dist1 = CalculateDistance(* tMouse, tPt1);									// distance between mouse & left
	dist2 = CalculateDistance(tPt1, tPt2);											// distance between endpoints
	if (dist2 == 0)
		return NULL;
	
	ratio = dist1/dist2;																		// get the ratio of the distances

	odist = CalculateDistance(oPt1, oPt2);											// distance between opp endpoints
	dist2Pt = ratio * odist;																	// distance to the point

	tSlope.first		= oPt1;																	// set points to get slope
	tSlope.second	= oPt2;
	HLock((Handle) this);
	SlopeIntercept(&tSlope);																// calculate slope
			
	nPt		= this->CalculateLocation(&tSlope,odist,dist2Pt);				// point location
	HUnlock((Handle) this);

	tPt = new TGPoint;
	if (tPt == NULL)
		return NULL;
	tPt->IPoint(nPt.v, nPt.h,(ot2->GetSectionType()),((TGeomView *) fGeomView)->fMagnify);		// create the point
	return tPt;
	}
	
// *********************************************************************************
//	Grid / Boundry Methods
//		these methods work on the entire mesh ... this includes the boundry
// *********************************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SearchColumn .... 
//		try to match the location of the mouse point to an object point in this column only works with grid on
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::SearchColumnPoint(short index, Point * tMouse)
	{
	short rIndex;
	TPoint * tPt;
	
	for (rIndex = 1; rIndex <= fSize; rIndex++)
		{
		tPt = (TPoint *)((TRow *) this->At(rIndex))->At(index);
		if (tPt->OnPoint(tMouse))
			{
			fRow = rIndex;
			fColumn = index;
			return tPt;
			}
		}
	return NULL;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SearchColumn .... 
//		try to match the location of the mouse point to a line between point objects
// --------------------------------------------------------------------------------------------------
short TPointMatrix::SearchColumnSection(short index, Point * tMouse, PointInfo * info)
	{
	short rIndex;
	TPoint * tPt1, * tPt2;
	Point	pt1, pt2;
	
	tPt1	= (TPoint *) ((TRow *) this->First())->At(index);						// first point in column
	pt1	= tPt1->fStart;																			// actual location
	for (rIndex = 2; rIndex <= fSize; rIndex++)
		{
		tPt2	= (TPoint *)((TRow *) this->At(rIndex))->At(index);
		pt2	= tPt2->fStart;
		info->above = tPt1;
		info->below = tPt2;
		if (((TRow *) this->At(rIndex))->PointOnLine(pt1,pt2,tMouse))
			{
			fRow = rIndex;
			fColumn = index;
			return rIndex;
			}

		tPt1	= tPt2;
		pt1	= pt2;
		}
	return 0;
	}
	
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	EachRowDo
//		Do the supplied method for each row
// --------------------------------------------------------------------------------------------------
pascal void TPointMatrix::Each(pascal void (*DoToItem)(TObject *, void *), void*DoToItem_StaticLink)
	{
	TRow 		* tRow;
	PointInfo	* info;
	short 		index;
 	
	info = (PointInfo *) DoToItem_StaticLink;											// cast info to type PointInfo

	if (fSize > 0)																						// anything in the list?
		{
		tRow					= NULL;															// initialize the row
		info->aboveRow	= (TRow *) this->At(fSize);								// get bottom row
		info->boundry		= cBottom;														// initialize the boundry type
		info->lrc				= (TPoint *) fRight->Last();
		for (index = (short) fSize; index > 0; index--)								// do all rows
			{
			info->belowRow	= tRow;														// assign the row below this one
			tRow					= info->aboveRow;										// assign current row
			if (index > 1)																			// on top row?
				info->aboveRow = (TRow *) this->At(index-1);					// get new above row
			else
				{
				info->aboveRow	= NULL;													// on top
				info->boundry		= cTop;														// assign new boundry
				}
			info->row					= index;													// assign current row
			DoToItem(tRow, info);																// perform the function
			info->boundry = cNotOnBoundry;												// reassign the boundry
			}
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	OneRowDo
//		Do the supplied method for each column
// --------------------------------------------------------------------------------------------------
void TPointMatrix::OneRowDo(pascal void (*DoToItem)(TObject *, void *), void *tinfo)
	{
	TRow * tRow;
	PointInfo * info;
	
	info 			= (PointInfo *) tinfo;
	info->lrc	= NULL;
	
	if (info->row <= 0 || info->row > (short) fSize)
		return;
	
	info->noGrid		=	fShown;
	info->boundry	= cNotOnBoundry;
	if (info->row > 1)
		info->aboveRow = (TRow *) this->At(info->row-1);
	else
		{
		info->aboveRow	= NULL;
		info->boundry		= cTop;
		}
	
	if (info->row < (short) fSize)
		info->belowRow = (TRow *) this->At(info->row+1);
	else
		{
		info->belowRow	= NULL;
		info->boundry		= cBottom;
		info->lrc	= (TPoint *) fRight->Last();
		}
	tRow = (TRow *) this->At(info->row);
	DoToItem((TObject *) tRow, info);	
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	EachColumnDo
//		Do the supplied method for each column
// --------------------------------------------------------------------------------------------------
void TPointMatrix::OneColumnDo(pascal void (*DoToItem)(TObject *, void *), void *tinfo)
	{
	TPoint		*	tPt;																				// the point
	TRow		* 	tRow;																			// the row
	PointInfo	*	info;																				// information structure
	short		index;																				// counter
	
	info 			= (PointInfo *) tinfo;
	
	if (fSize > 0)																						// any entries?
		{
		tRow			= NULL;																	// initialize the row
		tPt				= NULL;																	// initialize the point
		info->right	= NULL;																	// initialize the right point
		info->aboveRow	= (TRow *) this->Last();									// get last row
		info->boundry		= cBottom;
		info->lrc				= (TPoint *) fRight->Last();
		info->above			= (TPoint *) info->aboveRow->At(info->column);	// get point above this
			
		if (info->column > 1)																		// on left boundry?
			info->oPt = (TPoint *) info->aboveRow->At(info->column-1);	// get obstacle point
		else
			info->oPt = NULL;																		// no point there
			
		for (index = (short) fSize; index > 0; index--)								// do all rows in this column
			{
			info->belowRow	=	tRow;														// assign row below
			info->below			=	tPt;															// assign point below
			tRow					=	info->aboveRow;										// assign current row
			tPt						=	info->above;												// assign current point
			info->left				=	info->oPt;													// assign point to left
			if (index > 1)																			// on top boundry?
				{
				info->aboveRow 	= (TRow *) this->At(index-1);					// get new row above
				info->above 			= (TPoint *) (info->aboveRow)->At(info->column); // get new above point
				if (info->column > 1)																// on left boundry?
					info->oPt = (TPoint *) (info->aboveRow)->At(info->column-1); // get new obstacle point
				}
			else
				{
				info->boundry		= cTop;
				info->aboveRow	= NULL;
				info->above			= NULL;
				info->oPt				= NULL;													// no row ; no points
				}
			if (info->column < (tRow->fSize))												// on right boundry?
				info->right = (TPoint *) tRow->At(info->column+1);				// get point to right
				
			info->row = index;																	// assign the column
			
			DoToItem((TObject *) tPt,info);
			info->lrc = NULL;
			info->boundry = cNotOnBoundry;
			}
		}
	}

// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	CalculateLocation
//		creates a row of grid points
// --------------------------------------------------------------------------------------------------
Point TPointMatrix::CalculateLocation(lineSlope * line, extended d3, extended dist)
	{
	extended		x, y;
	extended		d1, d2, d4;
	Point			nPt;

	if (line->vertical)																			// vertical line
		{
		x = (extended) line->first.h;														// set x coordinate
		if (line->first.v < line->second.v)
			y = (extended) line->first.v + dist;										// set y coordinate
		else
			y = (extended) line->first.v - dist;										// set y coordinate
		}
	else if (line->horizontal)																// horizontal line
		{
		y = (extended) line->first.v;														// set y coordinate
		if (line->first.h < line->second.h)													
			x = (extended) line->first.h + dist;										// set x coordinate
		else
			x = (extended) line->first.h - dist;										// set x coordinate
		}
	else
		{
		d4 = dist / (sqrt((line->m * line->m) + 1));								// dist to point 
		x = line->first.h + d4;																// set x coordinate
		y = (line->m * x) + line->b;														// set y coordinate
		nPt.h = (short) x;
		nPt.v = (short) y;
			
		d2 = CalculateDistance(line->first,nPt);
		d1 = CalculateDistance(line->second,nPt);
		if ((d1+d2 - .5) > d3)																// wrong direction
			{
			x = line->first.h - d4;															// set x coordinate
			y = (line->m * x) + line->b;													// set y coordinate
			}
		}
	nPt.h = (short) x;
	nPt.v = (short) y;
	return nPt;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	CreateGridRow
//		creates a row of grid points
// --------------------------------------------------------------------------------------------------
TRow * TPointMatrix::CreateGridRow(short row)
	{
	short i;
	lineSlope		line;																				// line segment information
	TGridPoint	*	nPoint;																		// internal grid point
	TRow			*	tRow;
	extended		ratioLeft, ratioRight, dist;
	extended		ld1, rd1, tl1, tr1, td, d2, d3;
	Point			l1, l2, l3, r1, r2, r3, nPt;
	
//	get points locations on the left & right boundry
	tRow = (TRow *) this->At(row);
	l1 = ((TPoint *) ((TRow *) this->At(row-1))->First())->fStart;
	l2 = ((TPoint *) ((TRow *) this->At(row+1))->First())->fStart;
	l3 = ((TPoint *) tRow->First())->fStart;
	r1 = ((TPoint *) ((TRow *) this->At(row-1))->Last())->fStart;
	r2 = ((TPoint *) ((TRow *) this->At(row+1))->Last())->fStart;
	r3 = ((TPoint *) tRow->Last())->fStart;
	
	ld1	= CalculateDistance(l1,l3);														// distance from pt above to this row
	tl1	= CalculateDistance(l1,l2);														// distance from pt  above to pt below
	rd1	= CalculateDistance(r1,r3);														// distance from pt above to this row
	tr1	= CalculateDistance(r1,r2);														// distance from pt  above to pt below
	td		= CalculateDistance(l1,r1);														// distance for wieght factor
	
	ratioLeft	= ld1 / tl1;																		// ratio of dist to point left boundry
	ratioRight	= rd1 / tr1;																		// ratio of dist to point right boundry
	for (i = 2; i < (short) fTop->fSize; i++)
		{
		line.first		= ((TPoint *) ((TRow *) this->At(row-1))->At(i))->fStart;
		line.second	= ((TPoint *) ((TRow *) this->At(row+1))->At(i))->fStart;
		HLock((Handle) this);
		SlopeIntercept(&line);																	// get slope of line to dissect

		d3	= 	CalculateDistance(line.first,line.second);							// calculate distance between points
		d2	=	CalculateDistance(l1,line.first);										// distance for wiõght factor
		dist	=	(((d2/td) * ratioLeft) + ((1 - d2/td) * ratioRight)) * d3;	// distance down line 
		
		nPt		= CalculateLocation(&line,d3,dist);									// point location
		HUnlock((Handle) this);
			
		nPoint = tRow->AddPointAt(i,nPt,((TGeomView *) fGeomView)->fMagnify);	// add point to row
		}		
	return tRow;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	CreateGridColumn
//		creates a column of grid points
// --------------------------------------------------------------------------------------------------
TRow * TPointMatrix::CreateGridColumn(short column)
	{
	short i;
	lineSlope	line;
	TGridPoint	*	nPoint;																		// internal grid point
	TRow			*	tRow;
	extended		ratioTop, ratioBottom, dist;
	extended		td1, bd1, tt1, tb1, td, d2, d3;
	Point			t1, t2, t3, b1, b2, b3, nPt;

//	get point locations on the top & bottom boundry
	t1		=	((TPoint *) ((TRow *) this->First())->At(column-1))->fStart;
	t2		=	((TPoint *) ((TRow *) this->First())->At(column+1))->fStart;
	t3		=	((TPoint *) ((TRow *) this->First())->At(column))->fStart;
	b1	=	((TPoint *) ((TRow *) this->Last())->At(column-1))->fStart;
	b2	=	((TPoint *) ((TRow *) this->Last())->At(column+1))->fStart;
	b3	=	((TPoint *) ((TRow *) this->Last())->At(column))->fStart;
	
	td1	=	CalculateDistance(t1,t3);														// distance of top segment
	tt1	=	CalculateDistance(t1,t2);														// total distance of top segment
	bd1	=	CalculateDistance(b1,b3);														// distance of bottom segment
	tb1	=	CalculateDistance(b1,b2);														// total distance of bottom segment
	td		=	CalculateDistance(t1,b1);														// weight factor
	
	ratioTop		=	td1 / tt1;																	// ratio of dist to point : top boundry
	ratioBottom	=	bd1 / tb1;																// ratio of dist to point : bottom boundry

	for (i = 2; i < (short) fSize; i++)
		{
		tRow = (TRow *) this->At(i);														// this row
		line.first 		= 	((TPoint *) tRow->At(column-1))->fStart;			// end points for line to bisect		
		line.second	=	((TPoint *) tRow->At(column))->fStart;
		HLock((Handle) this);
		SlopeIntercept(&line);																	// get slope of line
		
		d3 	= CalculateDistance(line.first,line.second);							// length of bisect line
		d2 	= CalculateDistance(t1,line.first);											// weight factor
		dist 	= (((d2/td) * ratioTop) + ((1 - d2/td) * ratioBottom)) * d3;// distance along bisect line
		
		nPt	= CalculateLocation(&line,d3,dist);										// get point location
		HUnlock((Handle) this);
		nPoint = tRow->AddPointAt(column,nPt,((TGeomView *) fGeomView)->fMagnify);// add the point
		}
	return tRow;
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	GetColumnLength
//		Finds the total cumulative length of a column
// --------------------------------------------------------------------------------------------------
short TPointMatrix::GetColumnLength(short column)
	{
	extended total;
	Point t1, t2;
	short i;
	
	total = 0;
	t1 = ((TPoint *) ((TRow *) this->First())->At(column))->fStart;
	for (i = 2; i <= (short) fSize; i++)
		{
		t2 = ((TPoint *) ((TRow *) this->At(i))->At(column))->fStart;
		total += CalculateDistance(t1,t2);
		t1 = t2;
		}
	
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	MovePoints
//		moves a row or column of points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::MovePoints(Point temp, extended xlength)
	{
	TBoundry *	side;
	
	side = (TBoundry*) ((TGeomView *) fGeomView)->fSSide;
	if (side->GetSide() == cRight || side->GetSide() == cLeft)
		this->MoveRow(temp,xlength);
	else
		this->MoveColumn(temp,xlength);
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	MoveRow
//		moves a row or column of points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::MoveRow(Point temp, extended rowLength)
	{
	TRow		* tRow;
	TPoint		* tr;
	Point		tPt, oPt;
	extended	ratio, l, aRatio;
	short		changeX, changeY;
	extended	x, y;
	short		column;
	
	tRow	= (TRow *) this->At(fRow);
	
	l = 0.0;
	
	PenMode(srcXor);
	if (fBoundry == cRight)
		oPt = ((TPoint *) tRow->Last())->fStart;
	else
		oPt 	= ((TPoint *) tRow->First())->fStart;
	changeX = temp.h - oPt.h;
	changeY = temp.v - oPt.v;
		
	oPt 	= ((TPoint *) tRow->First())->fStart;
	for (column = 2; column < (short) tRow->fSize; column++)
		{
		tr	= (TPoint *) tRow->At(column);
		tPt = tr->fStart;
			
		l += CalculateDistance(oPt,tPt);												// distance from boundry
		ratio = l / rowLength;																// ratio for this point

		if (fBoundry == cLeft)
			aRatio = 1 - ratio;
		else
			aRatio = ratio;
		
		x = tPt.h +  (aRatio*changeX)+.5;												// store x
		y = tPt.v +  (aRatio*changeY)+.5;												// store y
			
		if (fShown)
			tr->DrawHold();

		tr->SetHold((short) y, (short) x);

		if (fShown)
			tr->DrawHold();
		oPt = tPt;
		}
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	MoveColumn
//		moves a row or column of points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::MoveColumn(Point temp, extended colLength)
	{
	TPoint		*	tr;
	Point		tPt, oPt;
	extended	ratio, l, aRatio;
	short		changeX, changeY;
	extended	x, y;
	short		row;
	
	l = 0.0;
	
	PenMode(srcXor);
	if (fBoundry == cBottom)
		oPt = ((TPoint *) ((TRow *) this->Last())->At(fColumn))->fStart;
	else
		oPt 	= ((TPoint *) ((TRow *) this->First())->At(fColumn))->fStart;
	changeX = temp.h - oPt.h;
	changeY = temp.v - oPt.v;
		
	oPt 	= ((TPoint *) ((TRow *) this->First())->At(fColumn))->fStart;
	for (row = 2; row < (short) fSize; row++)
		{
		tr	= (TPoint *) ((TRow *) this->At(row))->At(fColumn);
		tPt = tr->fStart;
			
		l += CalculateDistance(oPt,tPt);												// distance from boundry
		ratio = l / colLength;																// ratio for this point

		if (fBoundry == cTop)
			aRatio = 1 - ratio;
		else
			aRatio = ratio;
		
		x = tPt.h +  (aRatio*changeX)+.5;												// store x
		y = tPt.v +  (aRatio*changeY)+.5;												// store y
			
		if (fShown)
			tr->DrawHold();
		tr->SetHold((short) y, (short) x);

		if (fShown)
			tr->DrawHold();	
		oPt = tPt;
		}
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	RememberColumn
//		All points in that column remember yourself
// --------------------------------------------------------------------------------------------------
void TPointMatrix::RememberColumn(void)
	{
	TPoint  * t1;
	short i;
	
	for (i = 1; i <= (short) fSize; i++)
		{
		t1 = (TPoint *) ((TRow *) this->At(i))->At(fColumn);
		t1->RememberPoint();
		}
	
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FixHold
//		moves a row or column of points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::FixHold(void)
	{
	TBoundry *	side;
	
	side = (TBoundry*) ((TGeomView *) fGeomView)->fSSide;
	if (side->GetSide() == cRight || side->GetSide() == cLeft)
		{
		TRow * tRow;
		tRow = (TRow *) this->At(fRow);
		tRow->FixHold();
		}
	else
		this->FixHoldColumn(fColumn);

	return;		
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FixHoldColumn
//		sets fStart = fHold (Done Dragging)
// --------------------------------------------------------------------------------------------------
void TPointMatrix::FixHoldColumn(short c)
	{
	short i;
	TPoint * tPt;
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) ((TRow *) this->At(i))->At(c);
		tPt->FixHold();
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	RetrieveAndFix2Hold
//		Retrieves the saved coordinates and places them in hold & fixes to fStart
// --------------------------------------------------------------------------------------------------
void TPointMatrix::RetrieveAndFix2Hold(void)
	{
	TBoundry *	side;
	
	side = (TBoundry*) ((TGeomView *) fGeomView)->fSSide;
	if (side->GetSide() == cRight || side->GetSide() == cLeft)
		{
		TRow * tRow;
		tRow = (TRow *) this->At(fRow);
		tRow->RetrieveAndFixRow();
		}
	else
		this->RetrieveAndFixColumn(fColumn);

	return;		
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	RetrieveAndFixColumn
//		Retrieves the saved coordinates and places them in hold & fixes to fStart
// --------------------------------------------------------------------------------------------------
void TPointMatrix::RetrieveAndFixColumn(short c)
	{
	short	i;
	TPoint	* tPt;
	Point	temp;
	
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) ((TRow *) this->At(i))->At(c);
		temp = tPt->RetrievePoint();
		tPt->RememberPoint();
		tPt->SetStart(temp.v,temp.h);
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	DrawHold
//		Draws the lines & points for the hold (called by TDragCommand)
// --------------------------------------------------------------------------------------------------
void TPointMatrix::DrawHold(Boolean dPoint)
	{
	short		side;
	TRow		* tRow;
	TPoint		*	above, * below, * left, * right, * tPt;
	short		i;
	
	fGeomView->Focus();
	side = ((TBoundry*) ((TGeomView *) fGeomView)->fSSide)->GetSide();
	if (side == cRight || side == cLeft)
		{
//	Draw left boundry	
		tRow	= (TRow *) this->At(fRow);
		above 	= (TPoint *) ((TRow *) this->At(fRow-1))->First();
		below	= (TPoint *) ((TRow *) this->At(fRow+1))->First();
		tPt		= (TPoint *) tRow->First();

		if (tPt->fObsUpRight != NULL)
			((TObstacle *) ((TPoint * ) tPt)->fObsUpRight)->Draw();
		if (tPt->fObsLwRight != NULL)
			((TObstacle *) ((TPoint * ) tPt)->fObsLwRight)->Draw();

		if (tPt->fBaffAbove != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffAbove)->Draw();
		if (tPt->fBaffBelow != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffBelow)->Draw();

		if (side == cLeft)
			{
			tPt->DrawHoldLine(tPt->GetSectionType(),above->fTrans);
			below->DrawHoldLine(below->GetSectionType(),tPt->fTrans);
			}

		tPt->Draw();
		if (dPoint)
			{
			above->Draw();
			below->Draw();
			}
			
//	Draw Internal Points
		for (i = 2; i < (short) tRow->fSize; i++)
			{
			left = tPt;
			above 	= (TPoint *) ((TRow *) this->At(fRow-1))->At(i);
			below	= (TPoint *) ((TRow *) this->At(fRow+1))->At(i);
			tPt		= (TPoint *) tRow->At(i);
			if (tPt->fObsUpRight != NULL)
				((TObstacle *) ((TPoint * ) tPt)->fObsUpRight)->Draw();
			if (tPt->fObsLwRight != NULL)
				((TObstacle *) ((TPoint * ) tPt)->fObsLwRight)->Draw();

			if (tPt->fBaffAbove != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffAbove)->Draw();
			if (tPt->fBaffBelow != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffBelow)->Draw();
			if (tPt->fBaffLeft != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffLeft)->Draw();

			tPt->DrawHoldLine(Grid,above->fTrans);
			tPt->DrawHoldLine(Grid,below->fTrans);
			tPt->DrawHoldLine(Grid,left->fTrans);
			if (dPoint)
				{
				tPt->Draw();
				above->Draw();
				below->Draw();
				}
			}
			
//	Draw Right Boundry
		left 		=	tPt;
		tPt		= (TPoint *) tRow->Last();
		above 	= (TPoint *) ((TRow *) this->At(fRow-1))->Last();
		below	= (TPoint *) ((TRow *) this->At(fRow+1))->Last();
		tPt->DrawHoldLine(Grid,left->fTrans);
		
		if (tPt->fBaffAbove != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffAbove)->Draw();
		if (tPt->fBaffBelow != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffBelow)->Draw();
		if (tPt->fBaffLeft != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffLeft)->Draw();

		if (side == cRight)
			{
			tPt->DrawHoldLine(tPt->GetSectionType(),above->fTrans);
			if (fRow == this->fSize-1)
				below = (TPoint *) fRight->Last();

			below->DrawHoldLine(below->GetSectionType(),tPt->fTrans);
			}
		tPt->Draw();
		if (dPoint)
			{
			above->Draw();
			below->Draw();
			}
		}
	else
		{
//	Draw Top Boundry
		tRow 	= 	(TRow *) this->First();
		left		=	(TPoint *) tRow->At(fColumn-1);
		right		=	(TPoint *) tRow->At(fColumn+1);
		tPt		=	(TPoint *) tRow->At(fColumn);
		if (tPt->fObsLwLeft != NULL)
			((TObstacle *) ((TPoint * ) tPt)->fObsLwLeft)->Draw();
		if (tPt->fObsLwRight != NULL)
			((TObstacle *) ((TPoint * ) tPt)->fObsLwRight)->Draw();

		if (tPt->fBaffLeft != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffLeft)->Draw();
		if (tPt->fBaffRight != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffRight)->Draw();

		if (side == cTop)
			{
			tPt->DrawHoldLine(tPt->GetSectionType(),left->fTrans);
			right->DrawHoldLine(right->GetSectionType(),tPt->fTrans);
			}

		tPt->Draw();
		if (dPoint)
			{
			left->Draw();
			right->Draw();
			}
			
//	Draw Internal Points
		for (i = 2; i < (short) fSize; i++)
			{
			tRow	=	(TRow *) this->At(i);
			above	= tPt;
			left 		= (TPoint *) tRow->At(fColumn-1);
			right		= (TPoint *) tRow->At(fColumn+1);
			tPt		= (TPoint *) tRow->At(fColumn);
			if (tPt->fObsLwLeft != NULL)
				((TObstacle *) ((TPoint * ) tPt)->fObsLwLeft)->Draw();
			if (tPt->fObsLwRight != NULL)
				((TObstacle *) ((TPoint * ) tPt)->fObsLwRight)->Draw();

			if (tPt->fBaffLeft != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffLeft)->Draw();
			if (tPt->fBaffRight != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffRight)->Draw();
			if (tPt->fBaffAbove != NULL)
				((TBaffle *) ((TPoint * ) tPt)->fBaffAbove)->Draw();

			tPt->DrawHoldLine(Grid,above->fTrans);
			tPt->DrawHoldLine(Grid,left->fTrans);
			tPt->DrawHoldLine(Grid,right->fTrans);
			if (dPoint)
				{
				left->Draw();
				right->Draw();
				tPt->Draw();
				}
			}
			
//	Draw Bottom Boundry
		above	=	tPt;
		tRow	=	(TRow *) this->Last();
		left 		= 	(TPoint *) tRow->At(fColumn-1);
		right		= 	(TPoint *) tRow->At(fColumn+1);
		tPt		= 	(TPoint *) tRow->At(fColumn);
		tPt->DrawHoldLine(Grid,above->fTrans);

		if (tPt->fBaffLeft != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffLeft)->Draw();
		if (tPt->fBaffRight != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffRight)->Draw();
		if (tPt->fBaffAbove != NULL)
			((TBaffle *) ((TPoint * ) tPt)->fBaffAbove)->Draw();

		if (side == cBottom)
			{
			tPt->DrawHoldLine(tPt->GetSectionType(),left->fTrans);
			right->DrawHoldLine(right->GetSectionType(),tPt->fTrans);
			}
		tPt->Draw();
		if (dPoint)
			{
			left->Draw();
			right->Draw();
			}
		}
	}
	
// *********************************************************************************
//	Delete Methods
//		these methods delete rows / columns from the mesh
// *********************************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ObsBaffInLine .... 
//		delete the current row / column
// --------------------------------------------------------------------------------------------------
short TPointMatrix::ObsBaffInLine(TBoundry * side)
	{
	TPoint 	*	tPt;
	short 		x;
	
	fBoundry = side->GetSide();
	if (fBoundry == cTop || fBoundry == cBottom)
		{
		for (x = 1; x <= (short) this->fSize; x++)
			{
			tPt = (TPoint *) ((TRow *) this->At(x))->At(fColumn);
			if (tPt->fObsUpLeft != NULL || tPt->fObsLwLeft != NULL)
				return 1;
		if (tPt->fBaffLeft != NULL)
				return 2;
			}
		return 0;
		}
	TRow 	*	tRow;
	tRow = (TRow *) this->At(fRow);
	for (x = 1; x <= (short) tRow->fSize; x++)
		{
		tPt = (TPoint *) tRow->At(x);
		if (tPt->fObsUpLeft != NULL || tPt->fObsUpRight != NULL)
			return 1;
		if (tPt->fBaffAbove != NULL)
			return 2;
		}
	return 0;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	DeleteCurrent .... 
//		delete the current row / column
// --------------------------------------------------------------------------------------------------
TList * TPointMatrix::DeleteCurrent(TRow * side)
	{
	PointInfo	info;
	TPoint		* tPt, * t1, * t2;
	TRow		* tRow;
	Point		last, temp;
	short		y;
	
	fGeomView->Focus();
	PenMode(srcXor);
	fBoundry = side->GetSide();
	if (fBoundry == cTop || fBoundry == cBottom)
		{
		info.above = NULL;
		aColumn->DeleteAll();
		
		tRow = (TRow *) this->First();
		tPt	= (TPoint *) fTop->At(fColumn);															// get the points
		t1		= (TPoint *) fTop->At(fColumn-1);
		t2		= (TPoint *) fTop->At(fColumn+1);
		last.h = tPt->fTrans.h;
		last.v = tPt->fTrans.v;
		
		tPt->DrawSection(t1->fTrans);																	// erase the segments
		t2->DrawSection(tPt->fTrans);
		t1->DrawSection(t2->fTrans);																		// redraw the segments
		tPt->Draw();																								// erase the points
		 
		fTop->Delete(tPt);																						// delete from top list
		tRow->Delete(tPt);																						// delete from top row
		aColumn->InsertLast(tPt);																			// put in hold list
		if (fBoundry == cTop)
			{
			temp = tPt->RetrievePoint();
			tPt->SetStart(temp.v,temp.h);
			}
	
		for (y = 2; y <= (short) this->fSize; y++)														// all points in the column
			{
			tRow	=	(TRow *) this->At(y);
			tPt	= (TPoint *) tRow->At(fColumn);														// get the points
			t1		= (TPoint *) tRow->At(fColumn-1);
			t2		= (TPoint *) tRow->At(fColumn+1);
			tPt->Draw();																							// erase the points
			 
			if (y == (short) this->fSize)																		// bottom boundry
				{
				tPt->DrawSection(t1->fTrans);																// erase the segments
				t2->DrawSection(tPt->fTrans);
				if (fShown)
					tPt->DrawGrid(last);																		// erase the grid

				t1->DrawSection(t2->fTrans);																// redraw the segments
				fBottom->Delete(tPt);																			// delete from bottom list
				if (fBoundry == cBottom)
					{
					temp = tPt->RetrievePoint();
					tPt->SetStart(temp.v,temp.h);
					}
				}
			else if (fShown)
				{
				tPt->DrawGrid(t1->fTrans);																	// erase the grid
				t2->DrawGrid(tPt->fTrans);
				tPt->DrawGrid(last);																			// erase the grid
				t1->DrawGrid(t2->fTrans);																	// redraw the grid
				}
			
			last.h = tPt->fTrans.h;
			last.v = tPt->fTrans.v;
			tRow->Delete(tPt);																					// delete the point from list
			aColumn->InsertLast(tPt);																		// put in hold list
			}
		return aColumn;
		}

	short size;

	info.above = NULL;
	tRow	=	(TRow *) this->At(fRow);															// the row
	tPt		= (TPoint *) tRow->Last();																// get the points
	t1			= (TPoint *) ((TRow *) this->At(fRow-1))->Last();
	t2			= (TPoint *) ((TRow *) this->At(fRow+1))->Last();
	last.h 	= tPt->fTrans.h;
	last.v 	= tPt->fTrans.v;

	tPt->Draw();																								// erase the points
	tPt->DrawSection(t1->fTrans);																		// erase the segment
	t2->DrawSection(tPt->fTrans);
	t1->DrawSection(t2->fTrans);																		// redraw the segment

	fRight->Delete(tPt);
	if (fBoundry == cRight)
		{
		temp = tPt->RetrievePoint();
		tPt->SetStart(temp.v,temp.h);
		}
		
	size = (short) tRow->fSize;
	for (y = size-1; y > 0 ; y--)																			// all points in the row
		{
		tPt	= (TPoint *) tRow->At(y);																// get the points
		t1		= (TPoint *) ((TRow *) this->At(fRow-1))->At(y);
		t2		= (TPoint *) ((TRow *) this->At(fRow+1))->At(y);
		tPt->Draw();																							// erase the points
		
		if (y == 1)																								// on left or right boundry 
			{
			tPt->DrawSection(t1->fTrans);																// erase the segment
			t2->DrawSection(tPt->fTrans);
			if (fShown)
				tPt->DrawGrid(last);																		// erase the grid

			t1->DrawSection(t2->fTrans);																// redraw the segment
			fLeft->Delete(tPt);
			if (fBoundry == cLeft)
				{
				temp = tPt->RetrievePoint();
				tPt->SetStart(temp.v,temp.h);
				}
			}
		else if (fShown)
			{
			tPt->DrawGrid(t1->fTrans);																	// erase the grid
			t2->DrawGrid(tPt->fTrans);
			tPt->DrawGrid(last);																			// erase the grid
			t1->DrawGrid(t2->fTrans);																	// redraw the grid
			}
		last.h = tPt->fTrans.h;
		last.v = tPt->fTrans.v;
		}
			
	this->Delete(tRow);																						// delete row from matrix
	return (TList *) tRow;
	}
			
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	AddLine .... 
//		delete the current row / column
// --------------------------------------------------------------------------------------------------
TPoint * TPointMatrix::AddLine(TList * tList,TRow * side, short row, short column)
	{
	PointInfo	info, bInfo;
	TPoint		* tPt, * t1, * t2;
	TRow 		* tRow;
	Point		last;
	short		y;
	
	fGeomView->Focus();
	PenMode(srcXor);
	fBoundry	= side->GetSide();
	fRow 		= row;
	fColumn	= column;
	if (fBoundry == cTop || fBoundry == cBottom)
		{
		tRow = (TRow *) this->First();
		tPt	= (TPoint *) tList->First();		
		t1		= (TPoint *) fTop->At(fColumn-1);
		t2		= (TPoint *) fTop->At(fColumn);
		last.h = tPt->fTrans.h;
		last.v = tPt->fTrans.v;
		
		t2->DrawSection(t1->fTrans);																		// erase section
		t2->DrawSection(tPt->fTrans);																		// redraw section
		tPt->DrawSection(t1->fTrans);																		// redraw section
		tPt->Draw();																								// redraw point
					
		fTop->InsertBefore(fColumn,tPt);																	// Add to top list
		tRow->InsertBefore(fColumn,tPt);																// Add to top row

		for (y = 2; y <= (short) this->fSize; y++)														// all points in the column
			{
			tRow	=	(TRow *) this->At(y);
			tPt	= (TPoint *) tList->At(y);																// get the points
			t1		= (TPoint *) tRow->At(fColumn-1);
			t2		= (TPoint *) tRow->At(fColumn);
			
			if (y == (short) this->fSize)																		// bottom boundry
				{
				t2->DrawSection(t1->fTrans);																// erase section
				fBottom->InsertBefore(fColumn,tPt);													// Add to top list
				t2->DrawSection(tPt->fTrans);																// redraw section
				if (fShown)
					tPt->DrawGrid(last);																		// erase the grid	
				tPt->DrawSection(t1->fTrans);																// redraw section
				}
			else if (fShown)
				{
				t2->DrawGrid(t1->fTrans);																	// erase section
				tPt->DrawGrid(t1->fTrans);																	// erase the grid
				tPt->DrawGrid(last);																			// erase the grid	
				t2->DrawGrid(tPt->fTrans);				
				}
			
			tRow->InsertBefore(fColumn,tPt);															// Add to top row
			tPt->Draw();															// redraw point
			last.h = tPt->fTrans.h;
			last.v = tPt->fTrans.v;
			}
		if (fBoundry == cTop)
			return (TPoint *) tList->First();
		else
			return (TPoint *) tList->Last();
		}

	short size;
	
	info.above 	=	NULL;
	bInfo.above	=	NULL;
	tPt	= (TPoint *) tList->Last();																	// get the points
	t1		= (TPoint *) ((TRow *) this->At(fRow-1))->Last();
	t2		= (TPoint *) ((TRow *) this->At(fRow))->Last();
	last.h = tPt->fTrans.h;
	last.v = tPt->fTrans.v;

	t1->DrawSection(t2->fTrans);																		// erase section
	tPt->DrawSection(t1->fTrans);																		// redraw the segment
	t2->DrawSection(tPt->fTrans);
	tPt->Draw();																								// redraw the point

	fRight->InsertBefore(fRow,tPt);
		
	size = (short) tList->fSize;
	for (y = size-1; y > 0 ; y--)																			// all points in the row
		{
		tPt	= (TPoint *) tList->At(y);																// get the points
		t1		= (TPoint *) ((TRow *) this->At(fRow-1))->At(y);
		t2		= (TPoint *) ((TRow *) this->At(fRow))->At(y);

		if (y == 1)																								// on left or right boundry 
			{
			t1->DrawSection(t2->fTrans);																// erase the segment
			tPt->DrawSection(t1->fTrans);																// erase the segment
			t2->DrawSection(tPt->fTrans);
			if (fShown)
				tPt->DrawGrid(last);																		// erase the grid
			fLeft->InsertBefore(fRow,tPt);
			}
		else if (fShown)
			{
			t1->DrawGrid(t2->fTrans);																	// erase the grid
			tPt->DrawGrid(t1->fTrans);																	// redraw the grid
			tPt->DrawGrid(last);																			// erase the grid
			t2->DrawGrid(tPt->fTrans);
			}

		tPt->Draw();																							// redraw the point
		last.h = tPt->fTrans.h;
		last.v = tPt->fTrans.v;
		}
			
	this->InsertBefore(fRow,tList);																	// add row to matrix
	if (fBoundry == cLeft)
		return (TPoint *) tList->First();
	else
		return (TPoint *) tList->Last();
	}			
	
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	WriteGrid
//		creates a column of grid points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::WriteGrid(TOutput * output, short aRefNum, short export)
	{
	short	xx, yy, i;
	TPoint	* tPt;
	char 	string[80];
	realPt	num;
	
	yy = 1;
	for (xx =1; xx <= (short) fTop->fSize; xx++)
		{		
		for (yy = (short) fSize; yy > 0; yy--)
			{
			tPt = (TPoint *) ((TRow *) this->At(yy))->At(xx);				// get point
			
			if (export == 1)
				{
				i = sprintf(string,"%d ",tPt->fStart.h);
				i += sprintf(string+i,"%d\n",tPt->fStart.v);
				}
			else
				{
				HLock((Handle) this);
				((TCFDFrontDocument *) fDocument)->ViewToReal(tPt->fStart,&num);
				HUnlock((Handle) this);
				i = sprintf(string,"%f ",num.x);											// x coordinate
				i += sprintf(string+i,"%f",num.y);										// y coordinate
				if (export == 2)
					sprintf(string+i,"/\n");
				else
					sprintf(string+i,"/");
				}
			output->Write(aRefNum,string);
			}
		}
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	WriteGrid
//		creates a column of grid points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::WriteBaffles(Boolean vert, TOutput * output, short aRefNum)
	{
	short	xx, yy, i;
	TPoint	* tPt;
	char 	string[80], sEnd[4];

	((TCFDFrontDocument *) fDocument)->AddTerminator(sEnd);
	
	for (xx =1; xx <= (short) fTop->fSize; xx++)
		{		
		for (yy = (short) fSize; yy > 0; yy--)
			{
			tPt = (TPoint *) ((TRow *) this->At(yy))->At(xx);					// get point
			
			if (vert && tPt->fBaffAbove != NULL)
				{
				i = sprintf(string,"%d ",xx-1);											// x coordinate
				i += sprintf(string+i,"%d ",(fSize - yy + 1));						// y coordinate
				i += sprintf(string+i,"%d",(fSize - yy + 1));						// y coordinate
				sprintf(string+i,"%s",sEnd);												// model
				output->Write(aRefNum,string);
				}
			else if (!vert && tPt->fBaffLeft != NULL)
				{
				i = sprintf(string,"%d ",(fSize - yy));									// y coordinate
				i += sprintf(string+i,"%d ",xx-1);										// x coordinate
				i += sprintf(string+i,"%d",xx-1);										// x coordinate
				sprintf(string+i,"%s",sEnd);												// model
				output->Write(aRefNum,string);
				}
			}
		}
	}

// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	WriteObstacles
//		writes obstacles
// --------------------------------------------------------------------------------------------------
void TPointMatrix::WriteObstacles(TOutput * output, short aRefNum)
	{
	short	xx, yy, i;
	TPoint	* tPt;
	char 	string[80], sEnd[4];

	((TCFDFrontDocument *) fDocument)->AddTerminator(sEnd);
		
	for (xx =1; xx <= (short) fTop->fSize; xx++)
		{		
		for (yy = (short) fSize; yy > 0; yy--)
			{
			tPt = (TPoint *) ((TRow *) this->At(yy))->At(xx);					// get point
			
			if (tPt->fObsLwLeft != NULL)
				{
				i = sprintf(string,"%d ",xx-1);											// x coordinate
				i += sprintf(string+i,"%d ",xx-1);										// x coordinate
				i += sprintf(string+i,"%d ",(fSize - yy));								// y coordinate
				i += sprintf(string+i,"%d",(fSize - yy));								// y coordinate
				sprintf(string+i,"%s",sEnd);												// model
				output->Write(aRefNum,string);
				}
			}
		}
	}

// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ReadSegment
//		reads the segment information from the file
// --------------------------------------------------------------------------------------------------
Boolean TPointMatrix::ReadSegment(short side, short aRefNum, char * buff, short total)
	{
	char 			s[80];
	short			num, x, y, type, tEnd, tCell;
	TRow			* tRow;
	TGPoint		* gPt;
	TSegPoint		* sPt;
	WallRecord	data;
	
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// ns		-	number of segments
	num = atoi(s);
	
	switch (side)
		{
		case cTop:
			tRow = fTop;
			break;
		case cBottom:
			tRow = fBottom;
			break;
		case cLeft:
			tRow = fLeft;
			break;
		case cRight:
			tRow = fRight;
			break;
		}
			
	x = y = 1;
	tEnd = 0;
	while (x <= total)
		{
		if (x > tEnd && y <= num)
			{
			((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// kb		-	boundry type
			type = atoi(s);

			((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jf		-	starting grid cell
			tCell = atoi(s);
			((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jl			-	ending grid cell
			tEnd = atoi(s);
		
			HLock((Handle) this);
			this->ReadData(&data, aRefNum, buff);															// jl			-	ending grid cell
			HUnlock((Handle) this);
			
			if (side == cLeft || side == cRight)
				{
				if (x == 1)
					{
					sPt = (TSegPoint *) tRow->Last();														// get the point
					sPt->SetData(data);																				// store the data
					sPt->SetSectionType(type);
					}
				else
					{
					sPt = new TSegPoint;
					if (sPt == NULL)
						return false;
					sPt->IPoint(0,0,type,((TGeomView *) fGeomView)->fMagnify);
					sPt->SetData(data);
					tRow->InsertBefore(2, (TObject *) sPt);
					}
				while (x < tEnd)
					{
					gPt = new TGPoint;
					if (gPt == NULL)
						return false;
					gPt->IPoint(0,0,type,((TGeomView *) fGeomView)->fMagnify);
					tRow->InsertBefore(2, (TObject *) gPt);
					x++;
					}				
				}
			else
				{
				while (x < tEnd)
					{
					gPt = new TGPoint;
					if (gPt == NULL)
						return false;
					gPt->IPoint(0,0,type,((TGeomView *) fGeomView)->fMagnify);
					tRow->InsertBefore(((short) tRow->fSize), (TObject *) gPt);
					x++;
					}
				if (x == total-1 && (side == cTop || side == cBottom))
					{
					sPt = (TSegPoint *) tRow->Last();															// get the point
					sPt->SetData(data);																					// store the data
					sPt->SetSectionType(type);
					}
				else
					{
					sPt = new TSegPoint;
					if (sPt == NULL)
						return false;
					sPt->IPoint(0,0,type,((TGeomView *) fGeomView)->fMagnify);
					sPt->SetData(data);
					tRow->InsertBefore(((short) tRow->fSize), (TObject *) sPt);
					}
				}
			y++;
			}
		x++;
		}
	}
				
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ReadData
//		reads the segment information from the file
// --------------------------------------------------------------------------------------------------
void TPointMatrix::ReadData(WallRecord * data, short aRefNum, char * buff)
	{
	char 			s[80];

	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// u		-	u velocity
	data->u = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// v		-	v velocity
	data->v = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// w	-	w velocity
	data->w = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// visc	-	viscosity
	data->visc = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// t		-	temperature
	data->temp = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// rh	-	density
	data->density = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// f		-	mixture fraction
	data->mixfrac = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// g		-	concentration fraction
	data->concfrac = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// tk	-	kinetic energy
	data->kenergy = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// td	-	dissipation
	data->dissip = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// fu	-	fuel fraction
	data->fuelfrac = atof(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// co2	-	always 0 - not stored
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// h2o	-	always 0 - not stored
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// o2	-	always 0 - not stored
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// wm	-	always 0 - not stored
	}
		
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	FillGrid
//		creates the interior grid points
//		used after segment data points have been read from a file
// --------------------------------------------------------------------------------------------------
Boolean TPointMatrix::FillGrid(short row, short column)
	{
	short tCol, tRow;
	
	tCol		= (short) fTop->fSize;
	tRow	= (short) fLeft->fSize;
		
	if (tRow != row || tCol != column)
		{
		SysBeep (3);
		TWarning * tWarning;
		TWindow	* aWindow;
		
		aWindow = NewTemplateWindow(kWarnMe, fDocument);
		tWarning = (TWarning *) aWindow->FindSubView('WARN');
		tWarning->IWarning(6,NULL);
		tWarning->ShowWarning();
		Failure(0,0);
		}
	
	short x, y;
	TPoint		* tPt1, * tPt2;
	TGridPoint 	* gPt;
	TRow			* gRow;
	
	gRow = (TRow *) this->First();																		// create top
	for (y = tCol-1; y > 1; y--)
		{
		tPt1 = (TPoint *) fTop->At(y);																		// get point
		gRow->InsertBefore(2, (TObject *) tPt1);													// insert point
		}
	gRow = (TRow *) this->Last();																			// create bottom
	for (y = tCol-1; y > 1; y--)
		{
		tPt1 = (TPoint *) fBottom->At(y);																// get point
		gRow->InsertBefore(2,(TObject *) tPt1);														// insert point
		}
		
	for (y = tRow-1; y > 1; y--)
		{
		tPt1 = (TPoint *) fLeft->At(y);
		tPt2 = (TPoint *) fRight->At(y);
		gRow = new TRow;
			FailNIL(gRow);																							// LAM
		gRow->IRow(tPt1,tPt2);																				// initialize the row
		this->InsertBefore(2, (TObject *) gRow);														// add the row
		for (x = 2; x < tCol; x++)
			{
			gPt = new TGridPoint;
				FailNIL(gPt);																							// LAM
			gPt->IPoint(0,0,Grid,((TGeomView *) fGeomView)->fMagnify);				// initialize the point
			gRow->InsertBefore(2, (TObject *) gPt);													// add the point
			}
		}
	return true;
	}
						
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ReadObstructions
//		reads the obstacle & baffle information
// --------------------------------------------------------------------------------------------------
void TPointMatrix::ReadObstructions(short aRefNum, char * buff)
	{
	PointInfo	info;
	TBaffle		* tBaffle;
	TObstacle	* tObstacle;
	char 		s[80];
	short		nxbaf, nybaf, nobs, i;
	short		xstart, xend, ystart, yend;
	
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// nxbaf	-	# vertical baffles
	nxbaf = atoi(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// nybaf	-	# horizontal baffles
	nybaf = atoi(s);
	((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);					// nobs	-	# obstacles
	nobs  = atoi(s);
	
	info.left = NULL;
	info.right = NULL;																								// null non-used points
	for (i = 1; i <= nxbaf; i++)																				// vertical baffles
		{
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// ifc		-	xstart cell
		xstart = atoi(s) + 1;
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jfc		-	ystart cell
		ystart = (short) this->fSize - atoi(s);
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jlc		- 	yend cell
		yend = (short) this->fSize - atoi(s) + 1;
		
		info.above = (TPoint *) ((TRow *) this->At(ystart))->At(xstart);				// get baffle points
		info.below = (TPoint *) ((TRow *) this->At(yend))->At(xstart);
		
		tBaffle = new TBaffle;
			FailNIL (tBaffle);																						// LAM
		HLock((Handle) this);
		tBaffle->IBaffle(&info,1);																				// initialize vertical baffle
		HUnlock((Handle) this);
		}
		
	info.above = info.below = NULL;																			// null non-used points
	for (i = 1; i <= nybaf; i++)																				// horizontal baffles
		{
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// ifc		-	xstart cell
		ystart	= (short) this->fSize - atoi(s);
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jfc		-	ystart cell
		xstart	= atoi(s);
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jlc		- 	yend cell
		xend		= atoi(s) + 1;
		
		info.left	= (TPoint *) ((TRow *) this->At(ystart))->At(xstart);					// get baffle points
		info.right	= (TPoint *) ((TRow *) this->At(ystart))->At(xend);
		
		tBaffle = new TBaffle;
			FailNIL (tBaffle);																						// LAM
		HLock((Handle) this);
		tBaffle->IBaffle(&info,2);																				// initialize vertical baffle
		HUnlock((Handle) this);
		}
		
	info.right = NULL;																								// null non-used points
	for (i = 1; i <= nobs; i++)																					// obstacles
		{
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// ifc		-	xstart cell
		xstart	= atoi(s);
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jlc		- 	yend cell
		xend		= atoi(s) + 1;
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jfc		-	ystart cell
		ystart	= (short) this->fSize - atoi(s);
		((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);				// jlc		- 	yend cell
		yend		= (short) this->fSize - atoi(s) + 1;
		
		info.oPt	= (TPoint *) ((TRow *) this->At(ystart))->At(xstart);					// get obstacle points
		info.above	= (TPoint *) ((TRow *) this->At(ystart))->At(xend);
		info.left	= (TPoint *) ((TRow *) this->At(yend))->At(xstart);	
		info.below	= (TPoint *) ((TRow *) this->At(yend))->At(xend);
		
		tObstacle = new TObstacle;
			FailNIL (tObstacle);																						// LAM
		HLock((Handle) this);
		tObstacle->IObstacle(&info);																				// initialize vertical baffle
		HUnlock((Handle) this);
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	ReadGrid
//		read a column of grid points
// --------------------------------------------------------------------------------------------------
void TPointMatrix::ReadGrid(short aRefNum, char * buff)
	{
	short	xx, yy, x, y;
	TPoint	* tPt, * tPt1;
	char 	s[80];
	
short t;
	
	for (xx =1; xx <= (short) fTop->fSize; xx++)
		{		
		for (yy = (short) fSize; yy > 0; yy--)
			{
			if (yy == 1)
				t  = 1;
			((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);			// x coordinate
			x = atoi(s);
			((TCFDFrontDocument *) fDocument)->ReadLine(aRefNum, buff, s);			// y coordinate
			y = atoi(s);
			
			tPt = (TPoint *) ((TRow *) this->At(yy))->At(xx);									// get point
			tPt->SetStart(y,x);
			}
		}
	tPt  = (TPoint *) fBottom->Last();
	tPt1 = (TPoint *) fRight->Last();
	tPt1->SetStart(tPt->fStart.v,tPt->fStart.h);
	}

