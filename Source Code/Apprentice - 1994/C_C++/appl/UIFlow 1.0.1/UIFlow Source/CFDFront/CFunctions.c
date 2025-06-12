// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void DrawAllPoints (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TRow		*	item;
	
	item = (TRow *) tItem;
	info	=	(PointInfo *) tinfo;
	
	item->Each(DrawPoint,info);
	}
	
// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void DrawPoint (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TPoint		*	item;
	
	item = (TPoint *) tItem;
	info = (PointInfo *) tinfo;
	
	if (info->gridOnly && item->IsBoundryPt())
		return;

	if (item->IsCornor() && (info->boundry == cLeft || info->boundry == cRight))
		return;
	PenMode(srcXor);
	item->Draw();
	return;
	}

// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void DrawAllLines (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TRow		*	item;
	
	item = (TRow *) tItem;
	info	=	(PointInfo *) tinfo;
	
	item->Each(DrawLine,info);
	}
	
// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void DrawLine (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TPoint		*	item;
	
	item = (TPoint *) tItem;
	info	=	(PointInfo *) tinfo;
	if (info->gridOnly)
		{
		if (info->above != NULL && (info->boundry != cLeft && info->boundry != cRight) && !item->IsCornor())
			item->DrawGrid(info->above->fTrans);
		if (info->left != NULL && (info->boundry != cTop && info->boundry != cBottom))
			item->DrawGrid(info->left->fTrans);
		if (item->fObsUpLeft != NULL)
			((TObstacle *) ((TPoint * ) item)->fObsUpLeft)->Draw();
		if (item->fBaffAbove != NULL)
			((TBaffle *) ((TPoint * ) item)->fBaffAbove)->Draw();
		if (item->fBaffLeft != NULL)
			((TBaffle *) ((TPoint * ) item)->fBaffLeft)->Draw();
		return;
		}
	
	if (info->above != NULL)
		{
		if (item->IsCornor() && info->lrc != NULL)
			info->lrc->DrawSection(info->above->fTrans);
		else if (item->IsCornor())
			item->DrawSection(info->above->fTrans);
		else if (info->above->IsBoundryPt() && item->IsBoundryPt() && (info->boundry==cLeft||info->boundry==cRight))
			item->DrawSection(info->above->fTrans);
		else
			item->DrawGrid(info->above->fTrans);
		}
	if (info->left != NULL)
		{
		if (item->IsCornor())
			item->DrawSection(info->left->fTrans);
		else if (!info->noGrid && (info->boundry==cTop||info->boundry==cBottom))
			item->DrawSection(info->left->fTrans);
		else if (!info->noGrid && (info->boundry==cLeft||info->boundry==cRight))
			item->DrawGrid(info->left->fTrans);
		else if (info->left->IsBoundryPt() && item->IsBoundryPt() && (info->boundry==cTop||info->boundry==cBottom))
			item->DrawSection(info->left->fTrans);
		else
			item->DrawGrid(info->left->fTrans);
		}
	if (item->fObsUpLeft != NULL)
		((TObstacle *) ((TPoint * ) item)->fObsUpLeft)->Draw();
	if (item->fBaffAbove != NULL)
		((TBaffle *) ((TPoint * ) item)->fBaffAbove)->Draw();
	if (item->fBaffLeft != NULL)
		((TBaffle *) ((TPoint * ) item)->fBaffLeft)->Draw();
	return;
	}
	
//--------------------------------------------------------------------------------------------------------		
// 	determine the slope & intercept for the given points.
//	return false if line is vertical
// --------------------------------------------------------------------------------------------------------		
void SlopeIntercept(slopeStruct * tSlope)
	{																												// determine the slope for these points
	extended tx, ty;
	
	tSlope->vertical = false;																			// not vertical
	tSlope->horizontal = false;																		// not horizontal
	tSlope->m = 0;
	tSlope->b  = 0;
	tx = (extended) tSlope->second.h - (extended) tSlope->first.h;					// get Æx
	ty = (extended) tSlope->second.v - (extended) tSlope->first.v;					// get Æy
	
	if (tx != 0 && ty != 0)
		{																											// sloped line
		tSlope->m = ty / tx;																				// calculate slope
		tSlope->b = (extended) tSlope->first.v - (tSlope->m * (extended) tSlope->first.h);	// calculate y intercept
		return;
		}
	if (!tx)																										// vertical line
		{
		tSlope->vertical = true;
		tSlope->b = tSlope->first.v;
		}
	else																											// horizontal line
		{
		tSlope->horizontal = true;
		tSlope->b = 0;
		}	
	tSlope->m = 0;
	return;
	}

//--------------------------------------------------------------------------------------------------------		
// 	determine the distance between the given points.
//	return false if line is vertical
// --------------------------------------------------------------------------------------------------------		
extended CalculateDistance(Point first, Point second)	 								// determine the slope for these points
	{
	extended tx, ty;
	extended dist;
	
	tx = (extended) second.h - (extended) first.h;											// get Æx
	ty = (extended) second.v - (extended) first.v;											// get Æy
	dist = sqrt((tx * tx) + (ty * ty));
	return dist;
	}
	
// --------------------------------------------------------------------------------------------------------		
// 	make the pixel pattern
// --------------------------------------------------------------------------------------------------------		
void 	InitColorPattern(void)
	{																											// initialize the color pix pattern	
	long colorIndex;
	RGBColor patColor;																				// declaration 
	for (int x=0; x< NumSegmentTypes; x++)	{
		colorIndex = Color2Index(&DrawColors[x]);										// convert SegmentColor to an Index
		Index2Color(colorIndex, &patColor);													// convert the index to RGBColor
		SegmentPat[x] = NewPixPat();															// create the pattern data structure
		MakeRGBPat(SegmentPat[x], &patColor);											// create the pattern
		}

// make pixel pattern for the segment point

	colorIndex = Color2Index(&SegmentPoint);
	Index2Color(colorIndex, &patColor);														// convert the index to RGBColor
	SegmentPointPat = NewPixPat();															// create the pattern data structure
	MakeRGBPat(SegmentPointPat, &patColor);											// create the pattern

// make pixel pattern for the segment point

	colorIndex = Color2Index(&StartPoint);
	Index2Color(colorIndex, &patColor);														// convert the index to RGBColor
	StartPointPat = NewPixPat();																// create the pattern data structure
	MakeRGBPat(StartPointPat, &patColor);												// create the pattern

//	make pixel pattern for the frame
	colorIndex = Color2Index(&BlackColor);
	Index2Color(colorIndex, &patColor);
	PointFramePat = NewPixPat();
	MakeRGBPat(PointFramePat, &patColor);
	
//	make pixel pattern for the frame
	colorIndex = Color2Index(&GridPoint);
	Index2Color(colorIndex, &patColor);
	GridPointPat = NewPixPat();
	MakeRGBPat(GridPointPat, &patColor);
	
//	make pixel pattern for the frame
	colorIndex = Color2Index(&GridLineColor);
	Index2Color(colorIndex, &patColor);
	GridLinePat = NewPixPat();
	MakeRGBPat(GridLinePat, &patColor);
	
//	make pixel pattern for the frame
	colorIndex = Color2Index(&FineGridColor);
	Index2Color(colorIndex, &patColor);
	FineGridPat = NewPixPat();
	MakeRGBPat(FineGridPat, &patColor);
	
//	make pixel pattern for the obstacle
	colorIndex = Color2Index(&ObstacleColor);
	Index2Color(colorIndex, &patColor);
	ObstaclePat = NewPixPat();
	MakeRGBPat(ObstaclePat, &patColor);
	
//	make pixel pattern for the obstacle
	colorIndex = Color2Index(&BaffleColor);
	Index2Color(colorIndex, &patColor);
	BafflePat = NewPixPat();
	MakeRGBPat(BafflePat, &patColor);
	
	return;
	}

// --------------------------------------------------------------------------------------------------------		
// 	Transform the point for magnification
// --------------------------------------------------------------------------------------------------------		
Point transform(Point t, float mag)
	{
	Point newP;
	
	if (mag == 0)
		return t;
		
	newP.h	= (short) (t.h * mag);
	newP.v	= (short) (t.v * mag);
	return newP;
	}
	
// --------------------------------------------------------------------------------------------------------		
// 	Transform the point back
// --------------------------------------------------------------------------------------------------------		
Point AntiTransform(Point t, float mag)
	{
	Point newP;
	
	if (mag == 0)
		return t;
		
	newP.h	= (short) (t.h / mag);
	newP.v	= (short) (t.v / mag);
	return newP;
	}
	
// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void TransformGrid (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TRow		*	item;
	
	item = (TRow *) tItem;
	info	=	(PointInfo *) tinfo;
	
	item->Each(TransformPoint,info);
	}
	
// --------------------------------------------------------------------------------------------------
// 	Draws a boundry section
// --------------------------------------------------------------------------------------------------
pascal void TransformPoint (TObject * tItem, void * tinfo)
	{
	PointInfo	*	info;
	TPoint		*	item;
	
	item = (TPoint *) tItem;
	info = (PointInfo *) tinfo;
	item->TransformPoint(info->magnify);	
	return;
	}

