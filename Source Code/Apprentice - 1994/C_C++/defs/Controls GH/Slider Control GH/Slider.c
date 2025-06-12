#define __FOR__CDEV__

typedef pascal void (*IndicatorAction) (void);

/************** Slider.c v1.1 *********
******* code for a slider type control CDEF
*******©1993 Glenn R. Howes
******* all rights reserved *****/

#include "utilities.h"
pascal long main(short variation, ControlHandle me,
				short msg, long param)
{
	long result = 0L;


	switch (msg)
	{
		case testCntl: // determine if mouse down is in control
			result = TestMe(me, HiWord(param), LoWord(param));
		break;
		case calcCRgns: // 24-bit means of requesting shape of control or part
			result = CalcStripRegion(me, (RgnHandle)param);
		break;
		case initCntl: // 1st message, allocate private data
			InitMe(me);
		break;
		case dispCntl: // last message, dispose private data
			if ((*me)->contrlData)
				DisposeHandle((*me)->contrlData);
		break;
		case posCntl: // given new point (and old point from the thumbCntl msg)
					// set the controls new value
			PositionMe(me, HiWord(param), LoWord(param));
		break;
		case thumbCntl: // request for info for use in dragging thumb outline
						// also gives point at which drag starts
			ProvideDragInfo(me, (ThumbInfo*)param);
		break;
		case dragCntl:
			if (param)
			{
				DragMe(me, LoWord(param));
				
/************
**	if we are being used by a CDEV or other modal dialog, we have to fool
**  the dialog manager into sending the item hit message through to the application
**  so, we return a 0 in those cases. We play around with other routines so that 
**  the default dragging when it is called is invisible to the user.
**  If we are being used by an application which can manually call TrackControl, we
**  can do things the right way and just check the slider's value after the call
**  to TrackControl. I suppose I ought to just have one version of this code and
**  use variation codes, but that is neither here nor there.
*************/


#ifndef __FOR__CDEV__
				result = 1L;
#endif
			}
		break;

		break;
		case calcCntlRgn: // 32-bit clean message asking for whole cntl region
			CalcRegion( me, (RgnHandle)param, FALSE);
			result = 1L;
		break;
		case calcThumbRgn: // 32-bit clean message asking for shape of thumb
			result =  CalcRegion( me, (RgnHandle)param, TRUE);
			result =  1L;
		break;
		case drawCntl: // draw control or part of control
			DrawMe(me, LoWord(param));
		break;
	}
	
	
	return (result);
}
/********************** DragMe ******************
** Here's the situation, there has been a mouse down in my
** slider's thumb. I will move the thumb back & forth or
** up and down until the mouse is released. If there is a
** action proc, I will update the control's value and call it.
** If there is a proc stored in the low memory global DragHook,
** I'll call that too.
** 
** I've really violated the zeroth rule of programming
** "Never write the same code twice" here and I apologize. If I had
** the time I would modularize all the part drawing
**************************************************/
void DragMe(ControlHandle me, short part)
{
	Point			newPoint, lastPoint, offsetPoint;
	unsigned long	currTime, lastTime, pinnedPoint;
	Point			entry;
	Rect			ctlRect, mouseRect, updateRect, newRect, lastRect;
	Boolean			erase, fill,frame, vertical;
	short			thumbLen, halfThumb;
	short			diameter;
	Pattern			grayPattern;
	RgnHandle		savedClip = 0L, thumbRegion =0L, nonThumbRegion =0L;
	Rect			bigRect;
	IndicatorAction	action = (*me)->contrlAction;

	CopyRect(&(*me)->contrlRect, &ctlRect);
	thumbLen = CalcThumbLen(&ctlRect);
	halfThumb = thumbLen >> 1;
	diameter = CalcRounding(&ctlRect);
	GetMouse(&lastPoint);
	CalcThumbRects(me, (*me)->contrlValue, &lastRect);
	offsetPoint.h = lastPoint.h - lastRect.left;
	offsetPoint.v = lastPoint.v - lastRect.top;
	GetIndPattern(grayPattern, 0, 1);// standard black for frame and top region
	vertical = ((ctlRect.bottom - ctlRect.top) // if we are a vertical control
			>= (ctlRect.right - ctlRect.left));
			
	savedClip = NewRgn();
	thumbRegion = NewRgn();
	nonThumbRegion = NewRgn();
	if (savedClip && thumbRegion && nonThumbRegion)
	{
		GetClip(savedClip);
		OpenRgn();
		FrameRoundRect(&lastRect, diameter, diameter);
		CloseRgn(thumbRegion);
		SetRect(&bigRect, -16000,-16000,16000,16000);
		OpenRgn();
		FrameRect(&bigRect);
		CloseRgn(nonThumbRegion);
		XorRgn(nonThumbRegion,thumbRegion,nonThumbRegion);
		DisposeRgn(thumbRegion);
		thumbRegion = 0L;
	}
	
	while(WaitMouseUp())
	{
		erase = fill = frame = FALSE;
		CalcThumbLen(&ctlRect);
		GetMouse(&newPoint);
		
		
		CopyRect(&ctlRect, &mouseRect);
		CopyRect(&ctlRect, &updateRect);
		CopyRect(&ctlRect, &newRect);

		if (vertical)
		{
			updateRect.left += 1;
			updateRect.right -=1;
			
			mouseRect.top += offsetPoint.v;
			mouseRect.bottom -= (thumbLen - offsetPoint.v);
			*((long *)(&newPoint)) = PinRect(&mouseRect,newPoint);

			newRect.top = newPoint.v - offsetPoint.v;
			newRect.bottom = newRect.top + thumbLen;

			if (newPoint.v == lastPoint.v) // haven't moved
			{
				continue; // go back to WaitMouseUp
			}
			if (nonThumbRegion) // shift the mask to cover the new thumb rectangle
				OffsetRgn(nonThumbRegion,0,newPoint.v - lastPoint.v);
				
			if (newPoint.v > lastPoint.v) // moved down
			{
				fill = TRUE;
				updateRect.top = lastRect.top;
				updateRect.bottom = newRect.top + halfThumb;
			}
			else // moved up
			{
				erase = TRUE;
				updateRect.bottom = lastRect.bottom;
				updateRect.top = newRect.bottom - halfThumb;
				if (lastRect.bottom >= ctlRect.bottom -4)
				{
					frame = TRUE;
				}
			}
			
		}
		else
		{
			updateRect.top += 1;
			updateRect.bottom -=1;
			
			mouseRect.left += offsetPoint.h;
			mouseRect.right -= (thumbLen - offsetPoint.h -1);
			*((long *)(&newPoint))  = PinRect(&mouseRect,newPoint);

			newRect.left = newPoint.h - offsetPoint.h;
			newRect.right = newRect.left + thumbLen;

			if (newPoint.h == lastPoint.h) // haven't moved
			{
				continue; // go back to WaitMouseUp
			}
			
			if (nonThumbRegion)
				OffsetRgn(nonThumbRegion,newPoint.h - lastPoint.h,0);
			if (newPoint.h < lastPoint.h) // moved left
			{
				fill = TRUE;
				updateRect.right = lastPoint.h - offsetPoint.h
									+ thumbLen;
				updateRect.left = newRect.left + halfThumb;
			}
			else // moved right
			{
				erase = TRUE;
				updateRect.left = lastPoint.h - offsetPoint.h;
				updateRect.right = newRect.left + halfThumb;
				if (lastRect.left <= ctlRect.left + 4)
				{
					frame = TRUE;
				}
			}
		}
		lastPoint.v = newPoint.v;
		lastPoint.h = newPoint.h;
		
		lastTime = TickCount();
		if (nonThumbRegion)
			SetClip(nonThumbRegion);
		do
		{
			currTime = TickCount();
		}while (currTime == lastTime); // to avoid flicker drawing done at most once a tick
		
		if (erase)
		{
			EraseRect(&updateRect);
		}
		else if (fill)
		{
			
			FillRoundRect(&updateRect,diameter, diameter, grayPattern);
		}
		
		if (frame)
		{
			FrameRoundRect(&ctlRect, diameter,diameter);
		}
		if (savedClip)
			SetClip(savedClip);
		DrawMyIndicator(me, &newRect);
		CopyRect(&newRect, &lastRect);
		
		
		if (action && action != (ProcPtr) -1L)
		{
			newPoint.h = lastRect.left + halfThumb;
			newPoint.v = lastRect.top + halfThumb;
			MapPt2Value(me, &newPoint, &(*me)->contrlValue);
			action();
		}
		
		if (DragHook && DragHook != (ProcPtr) -1L) // DragHook is a low memory global
		{
			((IndicatorAction) DragHook)();
		}
		
	} 
	// the user has let up on the mouse
	// time to clean up
	newPoint.h = lastRect.left + halfThumb;
	newPoint.v = lastRect.top + halfThumb;
	MapPt2Value(me, &newPoint, &(*me)->contrlValue);
	if(savedClip)
	{
		SetClip(savedClip);
		DisposeRgn(savedClip);
	}
	if (nonThumbRegion)
		DisposeRgn(nonThumbRegion);
	
	DrawMe(me, 0); // snap the thumb into the nearest quantum
}
/******************* ProvideDragInfo ***********
**** a mouse down has occurred in the thumb and we are
**** asked to proide information used by the toolbox routine
**** DragGrayRgn to confine the drag to a given rectangle 
****  the limit rect is the rect in which the region will move
****  the slop rect is the rect in which the mouse can be and the
****  gray outline will still be visible.
****  Notice that I'm taking into account the position within the
****  thumb so that the region doesn't go beyond the control****/
void ProvideDragInfo(ControlHandle me, ThumbInfo *param)
{ 
	Rect			ctlRect, thumbRect;
	PrivateHandle	privData;
	Point			entry;
	privData = (PrivateHandle)(*me)->contrlData;
	
	// the original mousedown point is stored in the top,left corner of
	// the thumbinfo's limitRect field
	
	entry.h = param->limitRect.left;
	entry.v = param->limitRect.top;

	CopyRect(&(*me)->contrlRect, &ctlRect);
	

	CalcThumbRects(me, (*me)->contrlValue, &thumbRect);
	
	if (privData)
	{
		(*privData)->lastPoint.h = entry.h;
		(*privData)->lastPoint.v = entry.v;
		(*privData)->offsetPoint.h = entry.h - thumbRect.left;
		(*privData)->offsetPoint.v = entry.v - thumbRect.top;
		CopyRect(&thumbRect, &(*privData)->lastRect);
		(*privData)->lastTime = TickCount();
	}
	
	
	if ((ctlRect.bottom - ctlRect.top) // if we are a vertical control
		>= (ctlRect.right - ctlRect.left))
	{
		ctlRect.bottom -= (thumbRect.bottom - entry.v);
		ctlRect.top += (entry.v - thumbRect.top);
		
		CopyRect(&ctlRect, &param->limitRect);
	
		param->axis = 2; // confine to vertical
	}
	else // if we are a horizontal control
	{
		ctlRect.right -= (thumbRect.right - entry.h) -1;
		ctlRect.left += (entry.h - thumbRect.left);
		
		CopyRect(&ctlRect, &param->limitRect);
	
		param->axis = 1; // confine to horizontal

	}
/*******
** here's the scoop, if we are being used by a CDEV, or a modal dialog
** we have to force the dialog manager into recognizing that the thumb
** has been moved
***************/
	#ifdef __FOR__CDEV__
	SetRect(&param->slopRect,-32000,-32000,32000,32000);
	CopyRect(&(*me)->contrlRect, &param->limitRect);
	return;
	#endif
	InsetRect(&ctlRect, -5, -5); // make slop bigger than ctl rect
	CopyRect(&ctlRect, &param->slopRect);
}
/************ PositionMe *************
******** I've been dragged, with the sequence 
****** TestMe, ProvideDragInfo, and now PositionMe
****** so the delta values are changes from the original mousedown *****/
void PositionMe(ControlHandle me, short deltaV, short deltaH)
{
	Point	newPoint;
/******** 
** if we are using our own drag routines, we have already set the value of
** of the control, so we don't need to set it now.
********/
#ifdef __FOR__CDEV__
	return;
#endif
	MapValue2Point(me, (*me)->contrlValue, &newPoint);
	newPoint.v += deltaV;
	newPoint.h += deltaH;
	
	MapPt2Value(me, &newPoint, &(*me)->contrlValue);
	DrawMe(me, 0);
}
/************ MapPt2Value *****************
***** given a point within the control, map it to a control value
******/
void MapPt2Value(ControlHandle me, Point *aPoint, short *value)
{
	Rect	ctlRect;
	short	width, height, thumbLen, halfThumb;
	long	min, max, locValue;
	
	CopyRect(&(*me)->contrlRect, &ctlRect);
	min = (*me)->contrlMin;
	max = (*me)->contrlMax;
	
	width = ctlRect.right - ctlRect.left;
	height = ctlRect.bottom - ctlRect.top;
	
	thumbLen = CalcThumbLen(&ctlRect);
	halfThumb = thumbLen /2;
	
	if (width > height)
	{
		ctlRect.right -= halfThumb;
		ctlRect.left += halfThumb;
		width -= thumbLen;
		if (ctlRect.right < aPoint->h) aPoint->h = ctlRect.right;
		else if (ctlRect.left > aPoint->h) aPoint->h = ctlRect.left;
	
		locValue = min + ((max-min)*(aPoint->h-ctlRect.left))/width;
	}
	else
	{
		ctlRect.bottom -= halfThumb;
		ctlRect.top += halfThumb;
		height -= thumbLen;
		if (ctlRect.bottom < aPoint->v) aPoint->v = ctlRect.bottom;
		else if (ctlRect.top > aPoint->v) aPoint->v = ctlRect.top;
		
		locValue = min + ((max-min)*(ctlRect.bottom-aPoint->v))/height;
	}
	*value = locValue;
}
/*********** MapValue2Point *********
******* given a control value, convert it to a point within
******* the control corresponding to where the center of the thumb
******* must be **********/
void MapValue2Point(ControlHandle me, short value, Point *aPoint)
{
	Rect	ctlRect;
	long	width, height;
	long	min, max;
	short	thumbLen, halfThumb;
	CopyRect(&(*me)->contrlRect, &ctlRect);
	min = (*me)->contrlMin;
	max = (*me)->contrlMax;
	
	thumbLen = CalcThumbLen(&ctlRect);
	halfThumb = thumbLen /2;
	
	width = ctlRect.right - ctlRect.left;
	height = ctlRect.bottom - ctlRect.top;
	
	if (value > max)
		value = max;
	else if (value < min) 
		value = min;

	if (width > height) // horizontal control
	{
		width -= thumbLen;
		aPoint->h = ctlRect.left + halfThumb + (width*value)/(max-min);
		aPoint->v = ctlRect.top + height /2;
	}
	else
	{
		height -= thumbLen;
		aPoint->v = ctlRect.bottom - halfThumb -(height * value)/(max-min);
		aPoint->h = ctlRect.left + width /2;
	}
}

/************* CalcStripRegion *******
******* when in 24-bit mode, we will get a region handle whose
******* 31st bit is used to indicate if the app wants the region
******* of the thumb (on) or the whole control (off).
******* We are supposed to strip off the bit and return the 
******* calculated region ******/
long CalcStripRegion(ControlHandle me, RgnHandle theRegion)
{
	char	calcThumb;
	if (Using32Bit()) // WHY IS THIS GETTING CALLEDÀ
	{
		return (CalcRegion(me, theRegion, FALSE));
	}
	else
	{
		calcThumb = (long)theRegion >> 31L; // getting high bit, on = calc thumb
		theRegion = (RgnHandle)(0x7FFFFFFFL & (long)theRegion); 
		// stripping out high bit only see tech note Òjoy of 32 bit cleanÓ
		return (CalcRegion(me, theRegion, calcThumb)); // calc the region
	}
}
/*************** CalcRegion ******
******* figure the region of the control, or the thumb if the 
******* calcThumb parameter is non-zero **********/
long CalcRegion(ControlHandle me, RgnHandle theRegion, char calcThumb)
{
	RgnHandle	myRegion, curClip;
	Rect		ctlRect, thumbRect;
	short		diameter;
	
#ifdef __FOR__CDEV__
	return ((long)theRegion);
#endif
	
	CopyRect(&(*me)->contrlRect, &ctlRect);
	
	myRegion = NewRgn();
	if(curClip = NewRgn())
	{
		GetClip(curClip);
		ClipRect(&ctlRect);
	}
	
	if (myRegion)
	{
		OpenRgn();
		
		diameter = CalcRounding(&ctlRect);
		if (calcThumb)
		{
			CalcThumbRects(me, (*me)->contrlValue, &thumbRect);
			FrameRoundRect(&thumbRect, diameter,diameter);
		}
		else
		{
			FrameRoundRect(&ctlRect, diameter, diameter);
		}

		CloseRgn(myRegion);
		InsetRgn(myRegion, -1, -1); // make it a little bigger to take into account
									// width of lines
		UnionRgn(theRegion, myRegion, theRegion);
		DisposeRgn(myRegion);
	}
	if (curClip) 
	{
		SetClip(curClip);
		DisposeRgn(curClip);
	}
	return ((long)theRegion);
}
/************ TestMe ****************
******* is the point within the control, and if so,
******* what part, there are 3 parts: thumb, pageup, 
pagedown **********/
long TestMe(ControlHandle me, short v, short h)
{
	Point			testPoint;
	long			result = 0L;
	Rect			ctlRect, whiteRect, grayRect, roundThumb;
	
	testPoint.h = h; testPoint.v = v;
	
	if(PtInRect(testPoint, &(*me)->contrlRect) && (*me)->contrlHilite < 255) 
	// quick determination of (mouse in rect and control undimmed)
	{
		CopyRect(&(*me)->contrlRect, &ctlRect);
		CalcGrayRect(me, &grayRect, &whiteRect);
		CalcThumbRects(me,(*me)->contrlValue, &roundThumb);
		if (PtInRect(testPoint, &roundThumb))
		{
			result = inThumb;
		}
		/*
		else if (PtInRect(testPoint, &grayRect))
		{
			result = inPageUp;
		}
		else
		{
			result = inPageDown;
		} */
	}
	return (result);
}
/*************** DrawMe **********
******* Draw the control or the control's thumb *****/
void DrawMe(ControlHandle me, short part)
{
	Rect			grayRect, whiteRect, thumbRect, tempRect;
	PenState		oldState;
	PrivateHandle	privData;
	RgnHandle		oldRegion = 0L, thumbRegion = 0L, ctlRegion =0L;
	short			diameter;
	
	GetPenState(&oldState);
	PenNormal();
	privData = (PrivateHandle)(*me)->contrlData;
	CalcGrayRect(me, &grayRect, &whiteRect);
	CalcThumbRects(me, (*me)->contrlValue, &thumbRect);

	switch (part)
	{
		case 255: // change in dimming
			EraseRect(&grayRect);
		case 0: // draw whole thing
			EraseRect(&whiteRect);
			DrawMyFrame(me,&grayRect);
			DrawMyIndicator(me, &thumbRect);
		break;
		case 129: // redraw generic indicator
				// I'm using the regions to stop the flashing of drawing 
				// the gray rect and then drawing the white thumb over it
			oldRegion = NewRgn();
			thumbRegion = NewRgn();
			ctlRegion = NewRgn();
			if (ctlRegion && thumbRegion && oldRegion && privData)
			{
				diameter = CalcRounding(&(*me)->contrlRect);
				GetClip(oldRegion);
				CopyRect(&thumbRect, &tempRect);
				// InsetRect(&tempRect, -2,-2);
				OpenRgn();
					FrameRoundRect(&(*me)->contrlRect, diameter, diameter);
				CloseRgn(ctlRegion);
				OpenRgn();
					FrameRoundRect(&thumbRect, diameter, diameter);
				CloseRgn(thumbRegion);
				XorRgn(ctlRegion, thumbRegion, thumbRegion);
				SetClip(thumbRegion);
				CleanOldThumb(me, &thumbRect);
				SetClip(ctlRegion);
				DrawMyIndicator(me, &thumbRect);
				SetClip(oldRegion);
			}
			else // we had a memory allocation problem, draw whole thing
			{
				EraseRect(&whiteRect);
				DrawMyFrame(me,&grayRect);
				DrawMyIndicator(me, &thumbRect);
			}
			if (thumbRegion) DisposeRgn(thumbRegion);
			if (ctlRegion) DisposeRgn(ctlRegion);
			if (oldRegion) DisposeRgn(oldRegion);
			break;
		default:
		case inPageUp:
		case inPageDown:
		break;
	}
	if (privData)
		(*privData)->oldValue = (*me)->contrlValue;
	SetPenState(&oldState);
}
/*************** CleanOldThumb *************
***** We've gotten a message to draw the thumb, which
***** implies we have to eradicate the old thumb *******/
void CleanOldThumb(ControlHandle me, Rect *thumbRect)
{
	PrivateHandle	privData;
	short			oldValue, currValue;
	short			diameter;
	Rect			grayRect, ctlRect, oldThumb, whiteRect;
	Boolean 		upAndDown;
	Pattern			grayPattern;
	
	privData = (PrivateHandle)(*me)->contrlData; // privData has already been tested
	currValue = (*me)->contrlValue;
	oldValue = (*privData)->oldValue;
	
	//if (currValue == oldValue) return;
	
	(*me)->contrlValue = oldValue;
	CalcGrayRect(me, &grayRect, &whiteRect);
	(*me)->contrlValue = currValue;
	
	CopyRect(&(*me)->contrlRect, &ctlRect);
	diameter = CalcRounding(&ctlRect);
	upAndDown = ((ctlRect.right - ctlRect.left) <= (ctlRect.bottom - ctlRect.top));
	
	CalcThumbRects(me, oldValue, &oldThumb);
	GetIndPattern(grayPattern, 0, 1);// black
	PenPat(grayPattern);

	if (oldValue > currValue)
	{
		if (upAndDown)
		{
			oldThumb.bottom = thumbRect->bottom;
		}
		else
		{
			oldThumb.left = thumbRect->left;
		}
		FillRoundRect(&oldThumb, diameter, diameter, grayPattern);
	}
	else
	{
		if (upAndDown)
		{
			oldThumb.top = thumbRect->top;
		}
		else
		{
			oldThumb.right = thumbRect->right;
		}
		EraseRoundRect(&oldThumb, diameter, diameter);
	}
	FrameRoundRect(&ctlRect, diameter, diameter);
}
/********** DrawMyIndicator ************
******* Draw the thumb, the thumbRect must be previously
******* calculated ******/
void DrawMyIndicator(ControlHandle me, Rect *thumbRect)
{
	Rect		ctlRect;
	Boolean 	useColorQD, dimmed = FALSE;
	RGBColor	newColor, oldColor;
	Pattern		grayPattern;
	Boolean		upAndDown;
	short		diameter;
	PrivateHandle	privData;
	
	privData = (PrivateHandle)(*me)->contrlData;
	
	if ((*me)->contrlMin >= (*me)->contrlMax) dimmed = TRUE;
	if ((*me)->contrlHilite == 255) dimmed = TRUE;
	
	
	if (!dimmed)
	{
		CopyRect(&(*me)->contrlRect, &ctlRect);
		diameter = CalcRounding (&ctlRect);
		
	
		// calculate the rectangles used in drawing
		useColorQD = UseColorQD(me, thumbRect);
		// start actual drawing
		if(useColorQD && privData)
		{
			GetForeColor(&oldColor);
			RGBForeColor(&(*privData)->thumbColor);	// real medium gray
			GetIndPattern(grayPattern, 0, 1);// standard black for fill
			FillRoundRect(thumbRect, diameter, diameter, grayPattern);
			RGBForeColor(&oldColor);
		}
		else
		{
			GetIndPattern(grayPattern, 0, 4);// standard gray for fill
			FillRoundRect(thumbRect, diameter, diameter, grayPattern);
		}
		FrameRoundRect(thumbRect, diameter, diameter);
	}
}
/***************** CalcThumbRects ******
**** should be CalcThumbRect, just figure what the bounding rect of
*** the thumb should be given the value *******/
void CalcThumbRects(ControlHandle me, short value, 
		Rect *roundThumb)
{
	long	width, height, thumbLen;
	Point	valuePoint;
	Rect	ctlRect;
	
	CopyRect(&(*me)->contrlRect, &ctlRect);
	CopyRect(&ctlRect, roundThumb);
	
	height = ctlRect.bottom - ctlRect.top;
	width = ctlRect.right - ctlRect.left;
	
	MapValue2Point(me, value, &valuePoint);
	thumbLen = CalcThumbLen(&ctlRect);
	// calculate the rectangles used in drawing
	if (height >= width)
	{
		roundThumb->top = valuePoint.v - thumbLen/2;
		roundThumb->bottom = roundThumb->top+thumbLen;
		
	}
	else
	{
		roundThumb->right = valuePoint.h + thumbLen/2;
		roundThumb->left = roundThumb->right - thumbLen;
	}
}
/********** CalcThumbLen *********
**** figure how high or wide the thumb should be (given
**** that some moron might have made the control too small ***/
short CalcThumbLen(Rect *ctlRect)
{
	short	height, width;
	short	result;
	height = ctlRect->bottom - ctlRect->top;
	width = ctlRect->right - ctlRect->left;
	if (width > height)
	{
		result = (width > 20)?20:width;
	}
	else
	{
		result = (height > 20)?20:height;
	}
	return (result);
}
/********* DrawMyFrame **********
******* Draw the black portion of the control (pageup)
******* and frame the whole thing ********/
void DrawMyFrame(ControlHandle me, Rect *grayRect)
{	
	Rect		ctlRect;
	Boolean 	useColorQD, dimmed = FALSE;
	RGBColor	newColor, oldColor;
	Pattern		grayPattern;
	PrivateHandle	privData;
	short 		min, max, diameter;
	privData = (PrivateHandle)(*me)->contrlData;
	
	CopyRect(&(*me)->contrlRect, &ctlRect);
	useColorQD = UseColorQD(me, &ctlRect);
	min = (*me)->contrlMin;
	max = (*me)->contrlMax;
	diameter = CalcRounding (&ctlRect);
	if (min >= max) 
	{
		dimmed = TRUE;
	}
	if ((*me)->contrlHilite == 255) dimmed = TRUE;
	
	if (!dimmed)
	{
		GetIndPattern(grayPattern, 0, 1);// standard black for frame and top region
		FillRoundRect(grayRect,diameter, diameter, grayPattern);
		PenPat(grayPattern);
	}

	FrameRoundRect(&ctlRect, diameter, diameter);
}
/*********** CalcGrayRect
***** figure out the rectangle which enclose the pageup and pagedown
***** parts of the control (they meet in the middle of the thumb) ***/
void	CalcGrayRect(ControlHandle me, Rect *grayRect, Rect *whiteRect)
{
	long	height, width, value, max, min;
	Point	valuePoint;  
	short	thumbLen, halfThumb;
	
	
	value = (*me)->contrlValue;
	min = (*me)->contrlMin;
	max = (*me)->contrlMax;
	if (value > max) // too big, who let this in?
		value = max;
	else if (value < min) // too small
		value = min;
	MapValue2Point(me,value, &valuePoint);
	
	CopyRect(&(*me)->contrlRect, grayRect);
	CopyRect(grayRect, whiteRect);
	thumbLen = CalcThumbLen(grayRect);
	halfThumb = thumbLen /2;
	width = grayRect->right - grayRect->left;
	height = grayRect->bottom - grayRect->top;
	if (width > height) // we are going side to side
	{
		grayRect->left = valuePoint.h - halfThumb;
		whiteRect->right = valuePoint.h;
	}
	else
	{ // halfThumb is because quickdraw sort of bales out
		// when round rects are small 
		grayRect->bottom = valuePoint.v + halfThumb;
		whiteRect->top = valuePoint.v;
	}
}
/******* CalcRounding *******
****** are we big enough to use round rectangles 
if so use 16 diameter rounding, if not use square corners ****/
short CalcRounding(Rect *ctlRect)
{
	short	width, height;
	width = ctlRect->right - ctlRect->left;
	height = ctlRect->bottom - ctlRect->top;
	if (width > height)
	{
		if (width > 64)
			return (16);
	}
	else
	{
		if (height > 64)
			return (16);
	}
	return (0);
}

/******** InitMe ********
****** allocate my global memory, test for color and load color table
****** information ****/
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

			tempColor = &(*privDataH)->thumbColor; 
			tempColor->blue = tempColor->green = tempColor->red = 30583; // gray
			GetAuxCtl(me, &acHndl);
			if (acHndl)
			{
				tableH = (*acHndl)->acCTable;
				tabLen = (*tableH)->ctSize;
				while(tabLen >= 0)
				{
				/*	if ((*tableH)->ctTable[tabLen].value == cThumbColor)
					{
						BlockMove(&(*tableH)->ctTable[tabLen].rgb, 
							&(*privDataH)->thumbColor, sizeof(RGBColor));
					} */
					tabLen--;
				}
			}
		}
		// we need to know if we can call GetDeviceList to tell whether 
		// or not our control is on a deep color screen
		(*privDataH)->devicesAvailable = TrapAvailable(0xAA29);
		
		(*privDataH)->oldValue = (*me)->contrlValue;
	}
	(*me)->contrlData = (Handle)privDataH;
}