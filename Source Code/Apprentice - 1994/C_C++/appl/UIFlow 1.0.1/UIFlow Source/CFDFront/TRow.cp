#pragma segment Matrix
// **********************************************************************
//	TBoundry Class
//		Contains a list of pointers to TBoundry points stored in TGridMatrix
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TBoundry	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
void TBoundry::IRow(TPoint * p1, TPoint * p2, short side)
	{
	inherited::IRow(p1,p2);																// initialize the list
	
	fSide = side;
	return;
	}
	
// **********************************************************************
//	TRow Class
//		Contains a list of pointers to TPoint
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	TRow	:	Initialize the Class
// --------------------------------------------------------------------------------------------------
void TRow::IRow(TPoint * p1, TPoint * p2)
	{
	inherited::IList();																			// initialize the list
	
	InsertFirst ((TObject *) p1);														// store pointer to segment in list
	InsertLast ((TObject *) p2);															// store pointer to segment in list
	return;
	}
		
// --------------------------------------------------------------------------------------------------
//	TRow	:	For Each point in the list Do the DoToItem Function
// --------------------------------------------------------------------------------------------------
pascal void TRow::Each(pascal void (*DoToItem)(TObject *, void *), void *DoToItem_StaticLink)
	{
	TPoint		*	tPt;																			// the point to process
	PointInfo	*	info;																			// information structure
	short		index;
 	
	info = (PointInfo *) DoToItem_StaticLink;										// cast the structure
	if (fSize > 0)																					// anything in the list?
		{
		tPt					= NULL;															// initialize the current point
		info->below 		= NULL;															// initialize the point below
		info->left			= (TPoint *) this->At(fSize);							// get last point in this row
		if (info->aboveRow != NULL)														// on top row?
			info->oPt 		= (TPoint *) info->aboveRow->At(fSize);			// get last point in the row above
		else
			info->oPt		= NULL;															// no point
		
		if (info->boundry == cNotOnBoundry)
			info->boundry = cRight;
		
		for (index = (short) fSize; index > 0; index--)							// all points in the row
			{
			info->right	= tPt;																// assign point to right
			tPt 				= info->left;														// assign current point	
			info->above	= info->oPt;														// assign point above
			if (index > 1)																		// not on left boundry
				{
				info->left = (TPoint *) this->At(index-1);							// get new left point
				if (info->aboveRow != NULL)												// on top row?
					info->oPt = (TPoint *) info->aboveRow->At(index-1);	// get new obstacle point
				}
			else
				{
				info->left = info->oPt = NULL;											// no point
				if (info->boundry == cNotOnBoundry)
					info->boundry = cLeft;
				}
				
			if (info->belowRow != NULL)													// on bottom row?
				info->below = (TPoint *) info->belowRow->At(index);		// get point below
			info->column = index;
						
			DoToItem((TObject *)tPt,info);												// perform the function
			if (info->boundry != cTop && info->boundry != cBottom)
				info->boundry = cNotOnBoundry;
			info->lrc = NULL;
			}
		}
	return;
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	SearchRow .... 
//		try to match the location of the mouse point to an object point in this row
// -------------------------------------------------------------------------------- -----------------
short TRow::SearchPoint(Point * tMouse)
	{
	short rIndex;
	TPoint * tPt;
	
	for (rIndex = 1; rIndex <= fSize; rIndex++)
		{
		tPt = (TPoint *) this->At(rIndex);
		if (tPt->OnPoint(tMouse))
			return rIndex;
		}
	return 0;
	}
	
// --------------------------------------------------------------------------------------------------
//	TPointMatrix	:	SearchRow .... 
//		try to match the location of the mouse point to a line between point objects
// --------------------------------------------------------------------------------------------------
short TRow::SearchSection(Point * tMouse)
	{
	short cIndex;
	TPoint * tPt1, * tPt2;
	Point	pt1, pt2;
	
	tPt1	= (TPoint *) this->First();															// first point in column
	pt1	= tPt1->fStart;																			// actual location
	for (cIndex = 2; cIndex <= fSize; cIndex++)
		{
		tPt2	= (TPoint *) this->At(cIndex);
		pt2	= tPt2->fStart;
		if (this->PointOnLine(pt1,pt2,tMouse))
			return cIndex;

		tPt1	= tPt2;
		pt1	= pt2;
		}
	return 0;
	}
	
// --------------------------------------------------------------------------------------------------
//	Is the point, theMouse contained by the line segment terminated by last and next.
//	theMouse is changed so that it falls exactly on the line.
// --------------------------------------------------------------------------------------------------
Boolean TRow::PointOnLine (Point tpt1, Point tpt2, Point  * tMouse)
	{
	lineSlope 	tSlope;																			// slope structure
	Point		cpt1, cpt2;																	// center of the points
	short 		testPoint;
	extended	mMousev;
	extended	mMouseh;
	
	cpt1.v	=	(short) (tpt1.v + .50 * (gPensize_V+1));
	cpt1.h	=	(short) (tpt1.h + .50 * (gPensize_H+1));
	cpt2.v	=	(short) (tpt2.v + .50 * (gPensize_V+1));
	cpt2.h	=	(short) (tpt2.h + .50 * (gPensize_H+1));

	mMouseh = tMouse->h;
	mMousev = tMouse->v;

	tSlope.first		=	cpt1;																// initialize the slope structure
	tSlope.second	=	cpt2;
	HLock((Handle) this);
	SlopeIntercept(&tSlope);																// get the slope of this line
	HUnlock((Handle) this);
	
//	handle the case where the denominator of the the slope equation would be zero.
//	a zero denominator would indicate a HORIZONTAL line.

	if (tSlope.horizontal)																		// line is horizontal
		{
		testPoint = cpt2.v - tMouse->v;												// determine v distance from pt to line

		snap.v =  tpt1.v;																		// later want to snap to the line
		snap.h =  tMouse->h;

		if (testPoint < cMousePlay && testPoint > -cMousePlay)			// within Play limits?
			if (cpt1.h > cpt2.h)	 															// it is on the ray, is it on the line?
				if (tMouse->h <= cpt1.h && tMouse->h >= cpt2.h)				// is it between next & last horiz points?
					return true;																	// inform caller.
				else;
			else																						// last.h <= next.h
				if (tMouse->h >= cpt1.h && tMouse->h <= cpt2.h)				// on line?
					return true;
				else;
		else;
		}	
	else	if (tSlope.vertical)																// Line is NOT HORIZONTAL
		{
		testPoint = cpt1.h - tMouse->h;
			
		snap.v =  tMouse->v;																// later want to snap to the line
		snap.h = tpt1.h;
			
		if (testPoint < cMousePlay && testPoint > -cMousePlay) 			// if testPoint = 0 point is on line
																										// within play limits?
				if (cpt1.v > cpt2.v) 															// it is on the ray, is it on the line?
					if (tMouse->v <= cpt1.v && tMouse->v >= cpt2.v) 		// is on line
						return true;																// inform caller.
					else;
				else																					// last.v <= next.v
					if (tMouse->v >= cpt1.v && tMouse->v <= cpt2.v) 		// on line?
						return true;
					else;
		else;
		}
	else																								// line NOT horizontal or vertical
		{
//
//	To calculate the closest point on the boundry line to the mouse click find the intersection point on the boundry line
//	and the perpendicular which passes through the mouse click point.
//
// 	struct tSlope contains the slope & intercept of the original boundry line.  
//		This is represented by the intercept formula : y = yInterceptA - slopeA * x
// 	Line B is perpendicular to the boundry line.  It contains  the mouse point.  It's slope is the inverse of the slope of line A.
//	The intercept formula for line B is : y = bB - mB * x
//	Since the two lines intersect, the system of equations can be solved to find a single X,Y that satisfies both equations.
//		tSlope.m * x + tSlope.b = mB * x + bB    .... (Then solve for x).
//
		extended	mB;																					// slope of line B
		extended 	bB;																					// y intercept point of line B
		extended	x,y;																					// X,Y coordinate on boundry line.
		extended	testDistance;																	// distance between X,Y & mouse click point
		extended my, mx;
			
// compute the slope for the boundry line			
		mB = -1/tSlope.m;																			// calculate slope B
		bB  = (extended) tMouse->v - (mB * (extended) tMouse->h);				// calculate y Intercept of line B
			
		x = (bB - tSlope.b) / (tSlope.m - mB);												// calculate X coordinate on boundry line
		y = (tSlope.m * x) + tSlope.b; 															// calculate Y coordinate on boundry line
			
		mx = x - (extended) tMouse->h;
		my = y - (extended) tMouse->v;
		testDistance = sqrt((mx*mx) + (my*my));										// calculate distance between X,Y & mMouse.
	
		if (testDistance <= cMousePlay && testDistance >= -cMousePlay) 	// inside mouse play limits?
			{
//
//	We now know that the point is within the slope of the line.  Now determine whether the point is within
//	the limits of the line defined by points: last & next.  Determine this by calculating the distance between
//	last & next then last & mMouse.  If the mMouse distance is less then the point is within the limits of the line
//
//	calculate the distance between last & mMouse
//
			extended LineSize;
			extended testDistance2;
				
			snap.v = (short) (y - .50 *(gPensize_V+1));									// later want to snap to the line
			snap.h = (short) (x - .50 *(gPensize_H+1));
				
			testDistance = CalculateDistance(cpt1,*tMouse);							// calculate distance mouse to left point
			testDistance2 = CalculateDistance(cpt2,*tMouse);						// calculate distance mouse to right point
			if (testDistance < testDistance2)
				testDistance = testDistance2;

//	calculate the distance between last & next
			LineSize = CalculateDistance(cpt1,cpt2);										// total length of section
				
			if (testDistance <= LineSize)
				return true;																				// yes return true
			}
		}
	return false;																								// not in the section
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	SnaptoSection .... 
//		snap the point to the current line...  
//		Point Snap is set in PointOnLine method
// --------------------------------------------------------------------------------------------------
void TRow::SnaptoSection (Point * theMouse)
	{
//
//	odds are the user will not click exactly on the line.
// 	it is therefore necessary to snap the point to the line.  This prevents the line
//	from jumping all over the place.
//	The snap point is set in PointOnLine.
//
	theMouse->v = snap.v;
	theMouse->h = snap.h;
	return;
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	GetLength .... 
//		determines the total length of the row   
// --------------------------------------------------------------------------------------------------
extended TRow::GetLength (TPoint * t)
	{
	extended total;
	Point 	t1, t2;
	short x, y;																							// counter
	
	if (t == NULL)
		y = 2;
	else if ((y = (short) this->GetSameItemNo((TObject *) t)) ==  0)
		y = 2;
		
	total = 0L;
	t1 = ((TPoint *) this->At(1))->fStart;
	for (x = y; x <= (short) fSize; x++)
		{
		t2 = ((TPoint *) this->At(x))->fStart;
		total += CalculateDistance(t1,t2);
		t1 = t2;
		}
		
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	GetLength .... 
//		determines the total length of the row   
// --------------------------------------------------------------------------------------------------
extended TRow::GetDifferance (short start, ArrayIndex index, Boolean xCalc)
	{
	extended totalh, totalv;
	Point 	t1, t2;
	short 	x;																						
			
	if (index == 0)
		return 0;
		
	if (start == 0)
		start = 1;
	if (index == 0)
		index = (short) fSize;
		
	totalv = totalh = 0L;
	t1 = ((TPoint *) this->At(start))->fStart;
	for (x = start+1; x <= index; x++)
		{
		t2 = ((TPoint *) this->At(x))->fStart;
		totalh += (abs(t2.h  - t1.h));
		totalv += (abs(t2.v  - t1.v));
		t1 = t2;
		}
		
	if (xCalc)
		return totalh;
	return totalv;
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	FixHold .... 
//		Adds the point into the list at the specified index 
// --------------------------------------------------------------------------------------------------
void TRow::FixHold (void)
	{
	TPoint * tPt;
	short	i;
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) this->At(i);
		tPt->FixHold();
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	RememberRow .... 
//		All Points in the row remember yourself
// --------------------------------------------------------------------------------------------------
void TRow::RememberRow (void)
	{
	TPoint * tPt;
	short	i;
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) this->At(i);
		tPt->RememberPoint();
		}
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	RetrieveAndFixRow
//		Retrieves the saved coordinates and places them in hold & fixes to fStart
// --------------------------------------------------------------------------------------------------
void TRow::RetrieveAndFixRow(void)
	{
	short	i;
	TPoint	* tPt;
	Point	temp;
	
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) this->At(i);
		temp = tPt->RetrievePoint();
		tPt->RememberPoint();
		tPt->SetStart(temp.v,temp.h);
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	AddPointAt .... 
//		Adds the point into the list at the specified index 
// --------------------------------------------------------------------------------------------------
TGridPoint * TRow::AddPointAt (short index, Point tPoint, float mag)
	{
	TGridPoint * nPoint;

	if (fSize > 0)
		{
		nPoint = new TGridPoint;																// allocate memory
		if (nPoint == NULL)																		// not enough memory
			return NULL;
		nPoint->IPoint(tPoint.v, tPoint.h,	Grid, mag);								// initialize point
		this->InsertBefore(index, (TObject *) nPoint);								// register point in list
		return nPoint;
		}
	return NULL;
	}
		
// --------------------------------------------------------------------------------------------------
//	TRow	:	GetNumberSegments .... 
//		Adds the point into the list at the specified index 
// --------------------------------------------------------------------------------------------------
short TRow::GetNumberSegments (void)
	{
	short index, num;
	TPoint * tPt;
	
	num = 1;
	for (index = 2; index < (short) fSize; index++)
		{
		tPt = (TPoint *) this->At(index);
		if (tPt->IsSegment())
			num++;
		}
	return  num;
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	GetSegments .... 
//		Finds the segment adjacent to supplied point
//		True 	= Previous Segment
//		False	= Next Segment
// --------------------------------------------------------------------------------------------------
TPoint * TRow::GetAdjacentSegment (Boolean direction, TPoint * tPt)
	{
	short	index, y;
	TPoint	* sPt;
	
	y = (short) this->GetSameItemNo(tPt);													// find original point
	if (direction)
		{
		for (index = y-1; index > 0; index--)												// search for previous segment
			{
			sPt = (TPoint *) this->At(index);													// get point
			if (sPt->IsSegment())																	// is segment?
				return sPt;																				// got it
			}
		return  (TPoint *) this->First();														// return the corner
		}
	
	for (index = y+1; index < (short) this->fSize; index++)								// search for next segment
		{
		sPt = (TPoint *) this->At(index);														// get point
		if (sPt->IsSegment())																		// is segment?
			return sPt;																					// got it
		}
	return  (TPoint *) this->Last();															// return corner
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	GetNumBaffle .... 
//		returns the number of baffles in the grid
// --------------------------------------------------------------------------------------------------
short TRow::GetNumBaffle(Boolean vert)
	{
	TPoint 	* tPt;
	short 	i, total;
	
	total = 0;
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) this->At(i);
		if (vert && tPt->fBaffAbove != NULL)
			total++;
		else if (!vert && tPt->fBaffLeft != NULL)
			total++;
		}
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	GetNumObstacle .... 
//		returns the number of baffles in the grid
// --------------------------------------------------------------------------------------------------
short TRow::GetNumObstacle(void)
	{
	TPoint 	* tPt;
	short 	i, total;
	
	total = 0;
	for (i = 1; i <= (short) fSize; i++)
		{
		tPt = (TPoint *) this->At(i);
		if (tPt->fObsLwRight != NULL)
			total++;
		}
	return total;
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	WriteSegment .... 
//		Writes the information for each segment on the boundry
// --------------------------------------------------------------------------------------------------
void TRow::WForward (TOutput * output, short aRefNum, char * export,long fine)
	{
	char string[80];
	short index, i, fst;
	TPoint * tPt;
	WallRecord data;
	long temp;
	
	string[0] = 0;																						// null the string
	fst 		= 1;
	i 			= 0;
	data.u	= 0.0;
	for (index = 2; index <= (short) fSize; index++)					// do entire boundry
		{
		tPt = (TPoint *) this->At(index);												// get point
		if (tPt->IsSegment())																		// is a new segment
			{
			data		= ((TSegPoint *) tPt)->GetData();							// get point data
			i = sprintf(string,"%d ",tPt->GetSectionType());			// boundry type
	
			if (strcmp(export,"\n") == 0)
				{
				i += sprintf(string+i,"%d ",fst);										// jf	-	first cell
				i += sprintf(string+i,"%d",index-1);								// jl	-	last cell
				}
			else
				{
				temp = (pow(2,fine)) * (fst - 1) + 1;
				i += sprintf(string+i,"%ld ",temp);
				temp = (pow(2,fine)) * (index-1);
				i += sprintf(string+i,"%ld",temp);
				}
			sprintf(string+i,export);
			output->Write(aRefNum,string);												// write the string
			
			HLock((Handle) this);
			this->WriteSegmentData(&data,output,aRefNum,export);
			HUnlock((Handle) this);
			
			fst		= index;																				// start cell
			}
		}
	}

// --------------------------------------------------------------------------------------------------
//	TRow	:	WriteSegment .... 
//		Writes the information for each segment on the boundry
// --------------------------------------------------------------------------------------------------
void TRow::WBack (TOutput * output, short aRefNum, char * export,long fine)
	{
	char string[80];
	short index, i, fst,lst;
	long temp;
	TPoint * tPt;
	WallRecord data;
	
	string[0] = 0;																					// null the string
	fst 			= 1;
	i 				= 0;
	data.u		= 0.0;
	tPt = (TPoint *) this->Last();													// get point
	data		= ((TSegPoint *) tPt)->GetData();								// get point data
	i = sprintf(string,"%d ",tPt->GetSectionType());				// boundry type
	
	for (index = (short) fSize-1; index > 0; index--)				// do entire boundry
		{
		tPt = (TPoint *) this->At(index);											// get point
		if (tPt->IsSegment())																	// is a new segment
			{
			lst = (short) fSize - index;
			if (strcmp(export,"\n") == 0)
				{
				i += sprintf(string+i,"%d ",fst);									// jf		-	first cell
				i += sprintf(string+i,"%d",lst);									// jl		-	last cell
				}
			else
				{
				temp = (pow(2,fine)) * (fst - 1) + 1;
				i += sprintf(string+i,"%ld ",temp);
				temp = (pow(2,fine)) * lst;
				i += sprintf(string+i,"%ld",temp);
				}
			sprintf(string+i,export);
			output->Write(aRefNum,string);											// write the string
				
			HLock((Handle) this);
			this->WriteSegmentData(&data,output,aRefNum,export);
			HUnlock((Handle) this);
				
			data = ((TSegPoint *) tPt)->GetData();							// get point data
			i = sprintf(string,"%d ",tPt->GetSectionType());		// boundry type

			fst = (short) fSize - index + 1;
			}
		}
	}
	
// --------------------------------------------------------------------------------------------------
//	TRow	:	WriteSegment .... 
//		Writes the information for each segment on the boundry
// --------------------------------------------------------------------------------------------------
void TRow::WriteSegmentData (WallRecord * data, TOutput * output, short aRefNum, char * export)
	{
	char string[80];
	short i;
	
	i = sprintf(string,"%f ",data->u);											// ub		-	u velocity
	i += sprintf(string+i,"%f ",data->v);										// vb		-	v velocity
	i += sprintf(string+i,"%f ",data->w);										// wb		-	w velocity
	i += sprintf(string+i,"%f ",data->visc);								// vscty	-	viscosity
	i += sprintf(string+i,"%f",data->temp);									// tx		-	temperature
	sprintf(string+i,export);
	output->Write(aRefNum,string);													// write the string
				
	i = sprintf(string,"%f ",data->density);								// rh		-	density
	i += sprintf(string+i,"%f ",data->mixfrac);							// fx		-	mixture fraction
	i += sprintf(string+i,"%f ",data->concfrac);						// gx		-	concentration fraction
	i += sprintf(string+i,"%f ",data->kenergy);							// tx		-	kinetic energy
	i += sprintf(string+i,"%f",data->dissip);								// td		-	dissipation
	sprintf(string+i,export);
	output->Write(aRefNum,string);													// write the string
				
	i = sprintf(string,"%f ",data->fuelfrac);								// fu		-	fuel fraction
	i += sprintf(string+i,"0 ");														// co2		-	(always 0)
	i += sprintf(string+i,"0 ");														// h2o		-	(always 0)
	i += sprintf(string+i,"0 ");														// o2		- 	(always 0)
	i += sprintf(string+i,"0");															// wm		-	(always 0)
	sprintf(string+i,export);
	output->Write(aRefNum,string);													// write the string
	}
