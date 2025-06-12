#pragma segment Boundry
// **********************************************************************
//	TPoint Class 
//		Base class for all point classes.
// **********************************************************************
// **********************************************************************
//	The TPoint class methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Initialize the points data.
// --------------------------------------------------------------------------------------------------
void TPoint::IPoint(short p1V, short p1H, short type, float mag)
	{
	fType 			= type;																								// set the wall type
	fIsSelected 	= false;
	fHold.v 		= fHold.h = 0;
	fObsUpLeft	= fObsUpRight	= fObsLwLeft	= fObsLwRight	= NULL;
	fBaffAbove 	= fBaffRight		= fBaffBelow	= fBaffLeft		= NULL;
	fHoldRect.top = fHoldRect.bottom = fHoldRect.right = fHoldRect.left = 0;
	savePoint.v	= savePoint.h = 0;
	fMag				= mag;
	this->SetStart(p1V,p1H);
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Return the fStart Point for the segment
// --------------------------------------------------------------------------------------------------
void TPoint::SetStart(short newV, short newH)
	{
	fStart.v = newV;																							// set the location to a new place
	fStart.h = newH;

	fTrans				= transform(fStart,fMag);
	fFillRect.top		= fTrans.v-1;																		// create the point area
	fFillRect.right	= fTrans.h + gPensize_H+1;
	fFillRect.bottom= fTrans.v + gPensize_V+1;
	fFillRect.left		= fTrans.h-1;

	fillRect.top		= fFillRect.top - 4;																// create point area								
	fillRect.right		= fFillRect.right + 4;
	fillRect.bottom	= fFillRect.bottom + 4;
	fillRect.left		= fFillRect.left - 4;
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Return the fStart Point for the segment
// --------------------------------------------------------------------------------------------------
void TPoint::SetHold(short y, short x)
	{
	fHold.v = y;
	fHold.h = x;
	
	fHTrans				= transform(fHold,fMag);
	fHoldRect.top		= fHTrans.v-1;																// create the point area
	fHoldRect.right		= fHTrans.h + gPensize_H+1;
	fHoldRect.bottom	= fHTrans.v + gPensize_V+1;
	fHoldRect.left		= fHTrans.h-1;
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Return the fStart Point for the segment
// --------------------------------------------------------------------------------------------------
void TPoint::TransformPoint(float mag)
	{
	fMag						= mag;
	
	fTrans					= transform(fStart,fMag);
	fHTrans				= transform(fHold,fMag);
	
	fHoldRect.top		= fHTrans.v-1;																// create the point area
	fHoldRect.right		= fHTrans.h + gPensize_H+1;
	fHoldRect.bottom	= fHTrans.v + gPensize_V+1;
	fHoldRect.left		= fHTrans.h-1;

	fFillRect.top			= fTrans.v-1;																// create the point area
	fFillRect.right		= fTrans.h + gPensize_H+1;
	fFillRect.bottom	= fTrans.v + gPensize_V+1;
	fFillRect.left			= fTrans.h-1;

	fillRect.top			= fFillRect.top - 4;														// create point area								
	fillRect.right			= fFillRect.right + 4;
	fillRect.bottom		= fFillRect.bottom + 4;
	fillRect.left			= fFillRect.left - 4;
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Return the fStart Point for the segment
// --------------------------------------------------------------------------------------------------
void TPoint::FixHold(void)
	{
	if (fHold.v == 0 && fHold.h == 0)
		return;
		
	fStart.v 	= fHold.v;
	fStart.h 	= fHold.h;
	fTrans.v	= fHTrans.v;
	fTrans.h	= fHTrans.h;
	
	fFillRect.top			= fHoldRect.top;
	fFillRect.right		= fHoldRect.right;
	fFillRect.bottom	= fHoldRect.bottom;
	fFillRect.left			= fHoldRect.left;

	fillRect.top			= fFillRect.top - 4;															// create point area								
	fillRect.right			= fFillRect.right + 4;
	fillRect.bottom		= fFillRect.bottom + 4;
	fillRect.left			= fFillRect.left - 4;
	fHold.v					= fHold.h = 0;
	}
	
// --------------------------------------------------------------------------------------------------
//	is the mouse on this point?
// --------------------------------------------------------------------------------------------------
Boolean TPoint::OnPoint (Point * tMouse)
	{
	Point tM;
	tM.v = tMouse->v;  tM.h = tMouse->h;															// store mouse in correct form
	HLock((Handle) this);
	if (PtInRect(tM,&fFillRect))																			// is mouse inside rect
		{
		HUnlock((Handle) this);
		return true;																								// return on point
		}
	HUnlock((Handle) this);
	return false;																									// return not on point
	}
	
// --------------------------------------------------------------------------------------------------
//	increment / decrement the obstacle counter
//	each point can be attached to up to 4 cells
// --------------------------------------------------------------------------------------------------
void TPoint::SetObstacle (Boolean obs, short iquad, TObject * tObs)
	{
	TObject * t;
	
	if (obs)
		t = tObs;
	else
		t = NULL;
		
	switch (iquad)
		{
		case 1:
			fObsUpLeft = t;
			return;
		case 2:
			fObsUpRight = t;
			return;
		case 3:
			fObsLwLeft = t;
			return;
		case 4:
			fObsLwRight = t;
			return;
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	increment / decrement the obstacle counter
//	each point can be attached to up to 4 cells
// --------------------------------------------------------------------------------------------------
TObject * TPoint::GetObsQuad (short iquad)
	{
	switch (iquad)
		{
		case 1:
			return fObsUpLeft;
		case 2:																									// upper right quad
			return fObsUpRight;
		case 3:																									// lower left quad
			return fObsLwLeft;
		case 4:																									// lower right quad
			return fObsLwRight;
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	increment / decrement the obstacle counter
//	each point can be attached to up to 4 cells
// --------------------------------------------------------------------------------------------------
void TPoint::SetBaffle (Boolean baff, short iquad, TObject * tBaff)
	{
	TObject * t;
	
	if (baff)
		t = tBaff;
	else
		t = NULL;
		
	switch (iquad)
		{
		case 1:
			fBaffAbove = t;
			return;
		case 2:
			fBaffRight = t;
			return;
		case 3:
			fBaffBelow = t;
			return;
		case 4:
			fBaffLeft = t;
			return;
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	increment / decrement the obstacle counter
//	each point can be attached to up to 4 cells
// --------------------------------------------------------------------------------------------------
TObject * TPoint::GetBaffQuad (short iquad)
	{
	switch (iquad)
		{
		case 1:
			return fBaffAbove;
		case 2:																									// upper right quad
			return fBaffRight;
		case 3:																									// lower left quad
			return fBaffBelow;
		case 4:																									// lower right quad
			return fBaffLeft;
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TPoint::IsCornor (void)
	{
	return false;
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TPoint::IsSegment (void)
	{
	return false;
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TPoint::IsBoundryPt (void)
	{
	return true;
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TPoint::IsInterior (void)
	{
	return false;
	}
	
// --------------------------------------------------------------------------------------------------
//	Draws the section connecting to the point ... can be boundry or grid
// --------------------------------------------------------------------------------------------------
void TPoint::DrawHoldLine(short type,Point pt)
	{	
	if (type == Grid)
		this->DrawGrid(pt);
	else
		this->DrawSection(pt);	
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draws the section connecting to the point ... can be boundry or grid
// --------------------------------------------------------------------------------------------------
void TPoint::DrawSection(Point pt)
	{
	PenMode(srcXor);
	HLock((Handle) this);
	RGBForeColor(&DrawColors[fType]);															// forground color.
	HUnlock((Handle) this);
	PenPixPat(SegmentPat[fType]);

	PenSize (gPensize_H, gPensize_V);																// set pensize
	MoveTo(pt.h,pt.v);																						// move to provided point
	LineTo (fTrans.h, fTrans.v);																			// draw to this point
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draws the section connecting to the point ... can be boundry or grid
// --------------------------------------------------------------------------------------------------
void TPoint::DrawTemp(Point pt, Point pt1)
	{
	PenMode(srcXor);
	HLock((Handle) this);
	RGBForeColor(&DrawColors[fType]);															// forground color.
	HUnlock((Handle) this);
	PenPixPat(SegmentPat[fType]);

	PenSize (gPensize_H, gPensize_V);																// set pensize
	MoveTo(pt.h,pt.v);																						// move to provided point
	LineTo (pt1.h, pt1.v);																					// draw to this point
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draws the section connecting to the point ... can be boundry or grid
// --------------------------------------------------------------------------------------------------
void TPoint::DrawGrid(Point pt)
	{
	PenMode(srcXor);
	HLock((Handle) this);
	RGBForeColor(&GridLineColor);																	// set color
	HUnlock((Handle) this);
	PenPixPat(GridLinePat);

	PenSize (gGridsize_H, gGridsize_V);																// set pensize
	MoveTo(pt.h,pt.v);																						// move to provided point
	LineTo (fTrans.h, fTrans.v);																			// draw to this point
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draw the Segment.
// --------------------------------------------------------------------------------------------------
void TPoint::HighlightSection (Point pt)
	{
	HLock((Handle) this);
	RGBForeColor(&BlackColor);																			// set frame color
	HUnlock((Handle) this);
	PenPixPat(PointFramePat);
	PenMode(srcXor);
	PenSize (1, 1);																								// set pensize
	if (fTrans.h == pt.h)
		{
		MoveTo(pt.h-4,pt.v+gPensize_V+4);															// move to provided point
		LineTo (fTrans.h-4, fTrans.v-4);																// draw to this point
		MoveTo (pt.h+gPensize_H+4,pt.v+gPensize_V+4);
		LineTo (fTrans.h+gPensize_H+4,fTrans.v-4);
		}
	else if (fTrans.v == pt.v)
		{
		MoveTo(pt.h+gPensize_H+4,pt.v-4);															// move to provided point
		LineTo (fTrans.h-4, fTrans.v-4);																// draw to this point
		MoveTo (pt.h+gPensize_H+4,pt.v+gPensize_V+4);
		LineTo (fTrans.h+gPensize_H,fTrans.v+gPensize_V+4);
		}
	else
		{
		if ((pt.h < fTrans.h && pt.v < fTrans.v) || (pt.h > fTrans.h && pt.v > fTrans.v))
			{
			MoveTo (pt.h+gPensize_H+4,pt.v-4);
			LineTo (fTrans.h+gPensize_H+4,fTrans.v-4);
			MoveTo (pt.h-4,pt.v+gPensize_V+4);
			LineTo (fTrans.h-4,fTrans.v+gPensize_V+4);
			}
		else
			{
			MoveTo(pt.h-4,pt.v-4);																		// move to provided point
			LineTo (fTrans.h-4, fTrans.v-4);															// draw to this point
			MoveTo(pt.h+gPensize_H+4,pt.v+gPensize_V+4);									// move to provided point
			LineTo (fTrans.h+gPensize_H+4, fTrans.v+gPensize_V+4);					// draw to this point
			}
		}
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draw Method is Null : MUST be OVERRIDEN
// --------------------------------------------------------------------------------------------------
void TPoint::Draw(void)
	{}

// --------------------------------------------------------------------------------------------------
//	Draw Method is Null : MUST be OVERRIDEN
// --------------------------------------------------------------------------------------------------
void TPoint::DrawHold(void)
	{}

// --------------------------------------------------------------------------------------------------
//	Highlight Method is Null : MUST be OVERRIDEN
// --------------------------------------------------------------------------------------------------
void TPoint::Highlight(void)
	{}
	
// **********************************************************************
//	TGPoint Class 
//		Grid Point Class  :  Inherits everything from TPoint except Drawing functions
// **********************************************************************
// **********************************************************************
//	The TGPoint class methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Highlight this point
// --------------------------------------------------------------------------------------------------
void TGPoint::Highlight (void)
	{
	PenMode(srcXor);																					// draw xor
	PenSize (2, 2);																						// set frame width

	HLock((Handle) this);
	RGBForeColor(&BlackColor);																	// set color
	PenPixPat(PointFramePat);
	FrameOval(&fillRect);																			// draw frame
	HUnlock((Handle) this);
	return;
	}

// -----------------------------------------------------------------------------------------
//	Draw the GridPoint
// -----------------------------------------------------------------------------------------
void TGPoint::Draw (void)
	{
	PenMode(srcXor);																					// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&GridPoint);																	// set point color
	PenPixPat(GridPointPat);
	PaintOval(&fFillRect);																			// draw point
	HUnlock((Handle) this);
	return;	
	}

// -----------------------------------------------------------------------------------------
//	Draw the GridPoint
// -----------------------------------------------------------------------------------------
void TGPoint::Drawd (void)
	{
	PenMode(srcXor);																					// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&ObstacleColor);															// set point color
	PenPixPat(ObstaclePat);
	PaintOval(&fFillRect);																			// draw point		
	HUnlock((Handle) this);
	return;	
	}

// -----------------------------------------------------------------------------------------
//	Draw the GridPoint
// -----------------------------------------------------------------------------------------
void TGPoint::DrawHold (void)
	{
	if (fHold.v == 0 && fHold.h == 0)
		return;

	PenMode(srcXor);																					// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&GridPoint);																	// set point color
	PenPixPat(GridPointPat);
	PaintOval(&fHoldRect);																			// draw point	
	HUnlock((Handle) this);
	return;	
	}

// **********************************************************************
//	TCornorPoint Class 
//		Cornor Point Class  :  Inherits everything from TPoint except Drawing functions
// **********************************************************************
// **********************************************************************
//	The TCornorPoint class methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Initialize the points data.
// --------------------------------------------------------------------------------------------------
void TCornorPoint::IPoint(short p1V, short p1H, short type,Boolean seg,float mag)
	{
	fSeg = seg;
	inherited::IPoint(p1V,p1H,type,mag);
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	Highlight this point
// --------------------------------------------------------------------------------------------------
void TCornorPoint::Highlight (void)
	{
	PenMode(srcXor);																							// draw xor
	PenSize (2, 2);																								// set frame width

	HLock((Handle) this);
	RGBForeColor(&BlackColor);																			// set frame color
	PenPixPat(PointFramePat);
	FrameRect(&fillRect);																					// draw frame
	HUnlock((Handle) this);
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draw the Point indicator.
// --------------------------------------------------------------------------------------------------
void TCornorPoint::Draw (void)
	{
	PenMode(srcXor);																							// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&StartPoint);
	PenPixPat(StartPointPat);
	PaintRect(&fFillRect);																					// paint the point
	HUnlock((Handle) this);
	return;	
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TCornorPoint::IsCornor (void)
	{
	return true;
	}
	
// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TCornorPoint::IsSegment (void)
	{
	return fSeg;
	}
	
// **********************************************************************
//	TSegPoint Class 
//		Based on TPoint
//		In addition contains the data for the segment
// **********************************************************************
// **********************************************************************
//	The TSegPoint class methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Initialize the points data.
// --------------------------------------------------------------------------------------------------
void TSegPoint::IPoint(short p1V, short p1H, short type,float mag)
	{
	inherited::IPoint(p1V, p1H,type,mag);
	
	fInitData.u 			= 0.0;
	fInitData.v 			= 0.0;
	fInitData.w 			= 0.0;
	fInitData.kenergy 	= 0.0;
	fInitData.dissip 	= 0.0;
	fInitData.density 	= 0.0;
	fInitData.mixfrac 	= 0.0;
	fInitData.concfrac	= 0.0;
	fInitData.fuelfrac 	= 0.0;
	fInitData.temp 		= 0.0;
	fInitData.visc		= 0.0;
	return;
	}


// --------------------------------------------------------------------------------------------------
//	get the segments type.
// --------------------------------------------------------------------------------------------------
void TSegPoint::SetData (WallRecord newData)
	{
	fInitData.u 			= 	newData.u;
	fInitData.v			=	newData.v;
	fInitData.w			=	newData.w;
	fInitData.kenergy	=	newData.kenergy;
	fInitData.dissip		=	newData.dissip;
	fInitData.density	=	newData.density;
	fInitData.mixfrac	=	newData.mixfrac;
	fInitData.concfrac	=	newData.concfrac;
	fInitData.fuelfrac	= 	newData.fuelfrac;
	fInitData.temp		=	newData.temp;
	fInitData.visc		=	newData.visc;
	return;
	}

// --------------------------------------------------------------------------------------------------
//	Draw the Point indicator.
// --------------------------------------------------------------------------------------------------
void TSegPoint::Draw (void)
	{
	PenMode(srcXor);																							// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&SegmentPoint);
	PenPixPat(SegmentPointPat);
	PaintRect(&fFillRect);
	HUnlock((Handle) this);
	return;	
	}

// -----------------------------------------------------------------------------------------
//	Draw the GridPoint
// -----------------------------------------------------------------------------------------
void TSegPoint::DrawHold (void)
	{
	if (fHold.v == 0 && fHold.h == 0)
		return;

	PenMode(srcXor);																					// draw xor
	PenSize(1,1);

	HLock((Handle) this);
	RGBForeColor(&SegmentPoint);															// set point color
	PenPixPat(SegmentPointPat);
	PaintRect(&fHoldRect);																			// draw point		
	HUnlock((Handle) this);
	return;	
	}

// --------------------------------------------------------------------------------------------------
//	Highlight this point
// --------------------------------------------------------------------------------------------------
void TSegPoint::Highlight (void)
	{
	PenMode(srcXor);																							// draw xor
	PenSize (2, 2);																								// set frame width

	HLock((Handle) this);
	RGBForeColor(&BlackColor);																			// set frame color
	PenPixPat(PointFramePat);
	FrameRect(&fillRect);																					// draw frame
	HUnlock((Handle) this);
	return;
	}

// --------------------------------------------------------------------------------------------------
//	is this point a cornor?
// --------------------------------------------------------------------------------------------------
Boolean TSegPoint::IsSegment (void)
	{
	return true;
	}

// **********************************************************************
//	TInteriorPoint Class 
//		Based on TSegPoint
//		In addition contains the data for the interior of the geometry
// **********************************************************************
// **********************************************************************
//	The TInteriorPoint class methods.
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	Draw Method is Null : MUST be OVERRIDEN
// --------------------------------------------------------------------------------------------------
void TInteriorPoint::Draw(void)
	{}

// --------------------------------------------------------------------------------------------------
//	Highlight Method is Null : MUST be OVERRIDEN
// --------------------------------------------------------------------------------------------------
void TInteriorPoint::Highlight(void)
	{}
	
	
