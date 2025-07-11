/* CDividerLine.c */

#include "CDividerLine.h"
#include "LocationConstants.h"
#include "CWindow.h"


void			CDividerLine::IDividerLine(CWindow* TheWindow)
	{
		LongPoint			Start,Extent;

		GetRect(DividerLineID,&Start,&Extent);
		IViewRect(Start,Extent,TheWindow,TheWindow);
	}


void			CDividerLine::DoUpdate(void)
	{
		SetUpPort();
		Window->LPaintRect(ZeroPoint,Extent);
	}
