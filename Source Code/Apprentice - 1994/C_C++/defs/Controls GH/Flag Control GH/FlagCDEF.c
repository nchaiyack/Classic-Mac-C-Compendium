/************* Flag.c v 1.0
*** an arrow CDEF for use when the app
*** has a window that toggles between 
*** minimum and extended views 
By Glenn R. Howes 
internet: howes@bert.chem.wisc.edu
© 1993 by Glenn R. Howes 
all rights reserved
***********/
#include <GestaltEqu.h>
#include "utilities.h"
pascal long main(short variation, ControlHandle me,
				short msg, long param)
{
	long result = 0L;

	// msg's described in Inside Mac Vol 1 around pg 358
	switch (msg)
	{
		case testCntl: // test mouse to see if in control
			result = TestMe(me, HiWord(param), LoWord(param));
		break;
		case calcCRgns: // 24-bit mode method to calc drag region
			result = CalcStripRegion(me, (RgnHandle)param);
		break;
		case initCntl: // should be first message, set up private data
			InitMe(me);
		break;
		case dispCntl: // get rid of allocated memory
			if ((*me)->contrlData)
				DisposeHandle((*me)->contrlData);
		break;
		case posCntl: // ignore, don't have thumb
		break;
		case thumbCntl: // ignore, don't have thumb
		break;
		case dragCntl: // ignore, don't have thumb
		break;
		case autoTrack: // not tested, should not be called
			result = TrackMe(me, LoWord(param));
		break;
		case calcCntlRgn: // 32-bit method of calc gray region
		case calcThumbRgn: // don't have thumb
			CalcRegion(me, (RgnHandle)param);
			result = 1L; 
		// see tech note Control Manager Q&A's for why result = 1
		break;
		case drawCntl: // draw control, param indicates which part
			DrawMe(me, LoWord(param));
		break;
	}
	
	return (result);
}
long TrackMe(ControlHandle me, short part) // shouldn't  be called
{
	Point		mouseLoc, oldPoint = {32767,32767};
	Boolean		oldInRect = FALSE;
	Boolean		inRect;
	Point		vertices[3];
	PenState	oldPenState;
	GetPenState(&oldPenState);
	PenNormal();
	CalcVertices(vertices, &(*me)->contrlRect, 
			GetCtlValue(me));
	while(StillDown())
	{
		GetMouse(&mouseLoc);
		if (oldPoint.v != mouseLoc.v 
			|| oldPoint.h != mouseLoc.h) // mouse has moved
		{
			inRect = PtInRect(mouseLoc, &(*me)->contrlRect);
			if (inRect && !oldInRect)
			{
				EraseRect(&(*me)->contrlRect);
				DrawLines(vertices);
				
			}
			else if (!inRect && oldInRect)
			{
				DrawVertices(me, vertices);
			}
			oldPoint.v = mouseLoc.v;
			oldPoint.h = mouseLoc.h;
		}
	}
	DrawVertices(me, vertices);
	SetPenState(&oldPenState);
}

// under old  24-bit mode we will use this routine to calc the region
// this control occupies. If the control manager wants just the
// region of the thumb, the 31th bit of theRegion will be set
long CalcStripRegion(ControlHandle me, RgnHandle theRegion)
{
	if (Using32Bit()) // WHY IS THIS GETTING CALLED
	{
		return (CalcRegion(me, theRegion));
	}
	else
	{
		theRegion = (RgnHandle)(0x7FFFFFFFL & (long)theRegion); 
		// stripping out high bit only see tech note "joy of 32 bit clean"
		return (CalcRegion(me, theRegion));
	}
}
long CalcRegion(ControlHandle me, RgnHandle theRegion)
{
	RgnHandle	myRegion;
	Point		vertices[3];

	myRegion = NewRgn();
	if (myRegion)
	{
		OpenRgn();
		CalcVertices(vertices, &(*me)->contrlRect, 
			GetCtlValue(me));
		DrawLines(vertices);
		CloseRgn(myRegion);
		InsetRgn(myRegion, -1, -1); // making region slightly larger than
		// actual drawing to take into account width of lines
		UnionRgn(theRegion, myRegion, theRegion);
		DisposeRgn(myRegion);
	}
	return ((long)theRegion);
}
/******* TestMe ******
***** a mousedown has occurred or a track is underway, so I'm 
*** being asked if a point is within my boundaries ********/
long TestMe(ControlHandle me, short v, short h)
{
	Point			testPoint;
	long			result = 0L;
	
	testPoint.h = h; testPoint.v = v;
	if(PtInRect(testPoint, &(*me)->contrlRect))
	{
		result = inCheckBox; // It seems a reasonable part to use
	}
	return (result);
}
/********** InitMe *********
**** I've just be allocated and I'm going to allocate some private
**** data and initialize a few variables, including auxillary color
**** information *********/
void InitMe(ControlHandle me)
{
	PrivateHandle	privDataH;
	AuxCtlHandle		acHndl;
	CCTabHandle		tableH;
	short			tabLen;
	RGBColor		*tempColor;
	privDataH = (PrivateHandle)NewHandleClear(sizeof(Private));
	if (privDataH)
	{
		if ((*privDataH)->useColorQD = TestForColor())
		{// we are using color, find the highlight color
			tempColor = &(*privDataH)->frameColor;
			tempColor->red = tempColor->blue = tempColor->green = 0; // black
			tempColor = &(*privDataH)->fillColor; 
			tempColor->green = tempColor->red = 39321; // baby blue
			tempColor->blue = 65535;
			GetAuxCtl(me, &acHndl);
			if (acHndl)
			{
				tableH = (*acHndl)->acCTable;
				tabLen = (*tableH)->ctSize;
				while(tabLen >= 0)
				{
					if ((*tableH)->ctTable[tabLen].value == cFrameColor)
					{
						BlockMove(&(*tableH)->ctTable[tabLen].rgb, 
							&(*privDataH)->frameColor, sizeof(RGBColor));
					}
					tabLen--;
				}
			}
		}
		(*privDataH)->devicesAvailable = TrapAvailable(0xAA29);
		(*privDataH)->oldValue = GetCtlValue(me);
	}
	(*me)->contrlData = (Handle)privDataH;
}

void DrawMe(ControlHandle me, short parm)
{
	Point			vertices[3]; // a triangle
	short			myValue, oldValue;
	PrivateHandle	privData;
	unsigned long	ticks;
	
	myValue = GetCtlValue(me);
	privData = (PrivateHandle)(*me)->contrlData;
	if (privData)
	{
		oldValue = (*privData)->oldValue;
		if (oldValue != myValue) // the value is changed, animate the transition
		{
			CalcVertices(vertices, &(*me)->contrlRect, 
					2); // calculate diagonal arrow
			EraseRect(&(*me)->contrlRect);
			DrawVertices(me, vertices);
			(*privData)->oldValue = myValue;
			ticks = TickCount()+5;
			while (ticks > TickCount()) 
			// await 5/60ths of a second so its visibly turning
			{
				// do nothing
			}
		}
	}
	// calc either an up or down arrow, erase the old arrow, and then draw it
	CalcVertices(vertices, &(*me)->contrlRect, 
			myValue); 
	EraseRect(&(*me)->contrlRect);
	DrawVertices(me, vertices);
}
/********* DrawLines ********
******* given a set of 3 points, draw lines connecting all three ****/
void DrawLines(Point *vertices)
{
	MoveTo(vertices[0].h, vertices[0].v);
	LineTo( vertices[1].h, vertices[1].v);
	LineTo(vertices[2].h, vertices[2].v);
	LineTo(vertices[0].h, vertices[0].v);
}
/******** DrawVertices ********
***** given 3 pre-calculated points, make a polygon,
**** fill it if > 4 bit color is available and then
**** frame it *******/
void DrawVertices(ControlHandle me, Point *vertices)
{
	PrivateHandle	privData;
	Boolean			useColorQD = FALSE;
	PolyHandle		theFlag;
	PenState		oldState;
	Pattern		grayPattern;
	privData = (PrivateHandle)(*me)->contrlData;
	
	useColorQD = UseColorQD(me, &(*me)->contrlRect); // my rectangle is > 4 bit color

	theFlag = OpenPoly();
	if (theFlag)
	{
		DrawLines(vertices);
		ClosePoly();
		GetPenState(&oldState);
		PenNormal();
		if ((*me)->contrlHilite == 255) // disabled
		{
			if (useColorQD)
			{
				RGBColor	oldColor, newColor; // semi-lite gray
				newColor.red = newColor.blue = newColor.green = 43690;
				GetForeColor(&oldColor);
				RGBForeColor(&newColor);
				FramePoly(theFlag);
				RGBForeColor(&oldColor);
			}
			else
			{
				// first get a pattern that will simulate a dotted frame
				if (GetCtlValue(me)) // if we are pointing down use up and down line pat
					GetIndPattern(grayPattern, 0, 6);
				else
					GetIndPattern(grayPattern, 0, 25); // else side to side pat
				PenPat(&grayPattern);
				FramePoly(theFlag);
			}
		}
		else // don't use gray out
		{
			if ((*me)->contrlHilite)
			{
				PaintPoly(theFlag); // paint it black
				FramePoly(theFlag);
			}
			else if (useColorQD) // paint it blue
			{
				RGBColor	oldColor;
				GetForeColor(&oldColor);
				RGBForeColor(&(*privData)->fillColor);
				// for some reason the constant patterns (ltGray, dkGray, black...) are
				// not available
				GetIndPattern(grayPattern, 0, 4);// medium grey
				PenPat(&grayPattern);
				PaintPoly(theFlag);
				GetIndPattern(grayPattern, 0, 1);// standard black for frame
				PenPat(&grayPattern);
				RGBForeColor(&(*privData)->frameColor);
				FramePoly(theFlag);
				RGBForeColor(&oldColor);
				
			}
			else // just use a simple unfilled triangle
			{
				FramePoly(theFlag);
			}
			
		}
		SetPenState(&oldState);
		KillPoly(theFlag);
	}
}
/**************** CalcVertcices ***************
**** given a rectangle to put it in, calc the vertices of
**** a triangle which fill it in 3 possible ways
*** 2: diagonal
*** 1: pointing down
*** 0: pointing to the right
*** all triangles have a line passing through the center of the rectangle
***********/
void CalcVertices(Point *vertices, Rect *ctlRect, short value)
{
	short	halfWidth, halfHeight;
	Point	centerPoint;
	long	delta;
	Rect	theRect;
	theRect.top = ctlRect->top +1;
	theRect.left = ctlRect->left +1;
	theRect.bottom = ctlRect->bottom -1;
	theRect.right = ctlRect->right - 1;
	
	halfWidth = (theRect.right - theRect.left) /2; // don't want to go outside rect
	halfHeight = (theRect.bottom - theRect.top) /2;
	if (value == 2) // flag is pointing diagonally during animation
	{
		delta = (halfWidth > halfHeight)?halfHeight:halfWidth; // which is shorter
		
		delta = (delta * 707L) / 1000L; // multiply by the square root of 2 over 2
		// delta is change in both the x and y coordinates from the center of 
		// the rectangle
		centerPoint.h = theRect.left + halfWidth;
		centerPoint.v = theRect.top + halfHeight;
		
		vertices[0].h = centerPoint.h + delta;
		vertices[0].v = centerPoint.v - delta; // top, right corner
		vertices[1].h = centerPoint.h - delta;
		vertices[1].v = centerPoint.v + delta; // bottom, left corner
		vertices[2].h = vertices[0].h;
		vertices[2].v = vertices[1].v; // bottom, right corner
	}
	else if (value) // flag is pointing down
	{
		vertices[0].h = theRect.left;
		vertices[0].v = theRect.top + halfHeight;
		vertices[1].h = theRect.right;
		vertices[1].v = theRect.top + halfHeight;
		vertices[2].h = theRect.left + halfWidth;
		vertices[2].v = theRect.bottom;
	}
	else // flag is pointing to the right
	{
		vertices[0].h = theRect.left + halfWidth;
		vertices[0].v = theRect.top;
		vertices[1].h = theRect.left  + halfWidth;
		vertices[1].v = theRect.bottom;
		vertices[2].h = theRect.right;
		vertices[2].v = theRect.top + halfHeight;
	}
}
