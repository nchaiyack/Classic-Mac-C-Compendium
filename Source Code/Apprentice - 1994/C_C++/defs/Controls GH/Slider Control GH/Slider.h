/************************ Slider.h ********************/
#pragma once

typedef struct ThumbInfo
{
	Rect	limitRect;
	Rect	slopRect;
	short	axis;
}ThumbInfo;
typedef struct Private
{
	Boolean		useColorQD;
	Boolean		devicesAvailable;
	short		oldValue;
	Point		lastPoint;
	Point		offsetPoint; // offset from entry point to top-left of thumb
	Rect		lastRect; // used by autotrack
	unsigned long	lastTime;
	RGBColor	thumbColor;
}Private, *PrivatePtr, **PrivateHandle; 

/************* prototypes *****************/
void	CalcGrayRect(ControlHandle me, Rect *grayRect, Rect *whiteRect);
long CalcRegion(ControlHandle me, RgnHandle theRegion, char calcThumb);
short CalcRounding(Rect *ctlRect);
long CalcStripRegion(ControlHandle me, RgnHandle theRegion);
short CalcThumbLen(Rect *ctlRect);
void CalcThumbRects(ControlHandle me, short value,	Rect *roundThumb);
void CleanOldThumb(ControlHandle me, Rect *thumbRect);
void CopyRect(Rect *src, Rect *dest);
void DrawMe(ControlHandle me, short part);
void DrawMyFrame(ControlHandle me, Rect *grayRect);
void DrawMyIndicator(ControlHandle me, Rect *thumbRect);
void InitMe(ControlHandle me);
void MapPt2Value(ControlHandle me, Point *aPoint, short *value);
void MapValue2Point(ControlHandle me, short value, Point *aPoint);
void PositionMe(ControlHandle me, short deltaV, short deltaH);
void ProvideDragInfo(ControlHandle me, ThumbInfo *param);
Boolean TestForColor(void);
long TestMe(ControlHandle me, short v, short h);
Boolean UseColorQD(ControlHandle me, Rect *theRect);
Boolean Using32Bit(void);
void DragMe(ControlHandle me, short part);