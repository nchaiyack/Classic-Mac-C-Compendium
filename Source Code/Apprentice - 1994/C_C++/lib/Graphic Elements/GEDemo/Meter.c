/*
	Meter.c
	
	A "meter" type Graphic Element
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/5/93
*/

#include "Meter.h"

/*
	Look at the meter's PICT resources to see how it works. The basic idea is that
	the meter "bezel" has an opening 10 X 100 pixels for the indicator.
	The indicator is 110 X 100 pixels in size. It is pure white for the top 10
	pixels of its height. Below that, it is divided diagonally into red and white
	areas. This indicator is scrolled up and down behind the bezel to give a
	reading between 0 and 100&.
*/

Boolean LoadUsageMeterScene(GEWorldPtr world)
{
	GrafElPtr		meterBkg, thisElement;
	
	//Get meter bkg picture
	meterBkg = NewBasicPICT(world, meterBkgID, meterPlane, rMeterBkg,
								transparent, meterLeft, meterTop);
	if (meterBkg == nil) return false;
	
	//Get Indicator picture
	thisElement = NewScrollingGraphic(world, meterIndID, indPlane, rMeterInd,
								srcCopy, indLeft, indTop);
	if (thisElement == nil) return false;
	
	//Show only top 10 pixels of indicator
	thisElement->animationRect.bottom = thisElement->animationRect.top + 10;
	//Connect indicator to bkg
	meterBkg->slaveGrafEl = thisElement;
	return true;
}

pascal void SetMeterReading(GEWorldPtr world, short percent)
{
	if (percent < 0) percent = 0;
	if (percent > 100) percent = 100;
	
	SetScroll(world, meterIndID, 0, percent);
}
