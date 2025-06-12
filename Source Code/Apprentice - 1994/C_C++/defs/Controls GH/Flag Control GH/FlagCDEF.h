/************ FlagCDEF.h ***********
****** prototypes and typedefs for the flag.c ****/
#pragma once
typedef struct Private
{
	Boolean		useColorQD;
	Boolean		devicesAvailable;
	short		oldValue;
	RGBColor	frameColor;
	RGBColor	fillColor;
}Private, *PrivatePtr, **PrivateHandle;

long CalcRegion(ControlHandle me, RgnHandle theRegion);
long CalcStripRegion(ControlHandle me, RgnHandle theRegion);
void CalcVertices(Point *vertices, Rect *ctlRect, short value);
void DrawLines(Point *vertices);
void DrawMe(ControlHandle me, short parm);
void DrawVertices(ControlHandle me, Point *vertices);
void InitMe(ControlHandle me);
Boolean TestForColor(void);
long TestMe(ControlHandle me, short v, short h);
long TrackMe(ControlHandle me, short part);

