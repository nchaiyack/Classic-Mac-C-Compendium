#include <GestaltEqu.h>
#include "utilities.h"
/********* CopyRect ********
**** take one rect and copy it to another
**** as this is a useful routine, is small, and
**** is used in a lot of places, I've hand assembled it
**** hopefully, it's as fast as possible *****/
void CopyRect(Rect *src, Rect *dest)
{
	asm 
	{
		MOVE.L	src, A0 // move pointer to src into A0
		MOVE.L	dest, A1 // move pointer to dest into A1
		MOVE.L 	(A0)+, (A1)+ // move first point into dest, increment both
		MOVE.L	(A0), (A1) // move second point into dest, no increment
	}
}
/************* Using32Bit *******
******* calls gestalt to answer the question "are we in 32-bit mode"?
****** hopefully, the gestalt glue will return an error in systems
****** before gestalt was available (6.0.4?) ******/
Boolean Using32Bit(void)
{
	long	response;
	OSErr	iErr;
	Boolean	result = FALSE;
	
	iErr = Gestalt(gestaltAddressingModeAttr, &response);
	if (!iErr && (response & 1L))
		result = TRUE;
	return (result);
}
/********** TestForColor *******
***** answer question "Do we have color quickdraw?" ***/
Boolean TestForColor(void)
{
	SysEnvRec	theSysEnv;
	OSErr		errCode;
	Boolean		result = FALSE;
	errCode = SysEnvirons(curSysEnvVers, &theSysEnv);
	if (!errCode)
	{
		result = theSysEnv.hasColorQD;
	}
	return (result);
} 
/******** UseColorQD *******
**** answer the question, "Is the rectangle completely on
a deep color screen?" *******/
Boolean UseColorQD(ControlHandle me, Rect *theRect)
{
	PrivateHandle	privData;
	Boolean			result = FALSE;
	Rect			myRect, ctlRect;
	Point			origin;
	GDHandle 		gd;
	Rect			intersection;
	short			newDepth, flatest = 32;

	privData = (PrivateHandle)(*me)->contrlData;
	
	// first check to see if I can check the devices list for the depth of my control
	if (!privData)
		return (FALSE);
	if (!(*privData)->useColorQD)
		return (FALSE);
	if (!(*privData)->devicesAvailable)
		return (FALSE);
		
	CopyRect(theRect, &ctlRect);
	origin.h = ctlRect.left;
	origin.v = ctlRect.top;
	LocalToGlobal(&origin);
	OffsetRect(& ctlRect, origin.h, origin.v);
/******** code provided by Leonard Roesenthol **********/
	gd = GetDeviceList();
	while(gd)
	{	if(SectRect(& ctlRect,&(*gd)->gdRect,&intersection) &&
			TestDeviceAttribute(gd,13) && 	/* screen */
			TestDeviceAttribute(gd,15))		/* active */
		{	newDepth = (*(*gd)->gdPMap)->pixelSize;
			if(newDepth < flatest)
				flatest = newDepth;
		}
		gd = GetNextDevice(gd);
	}
/****************************************************/
	if (flatest > 4) return (TRUE);
	else return (FALSE);
} 
/* CODE EXAMPLE #1  these trap availables algorithms are from Think Reference*/
#include<Traps.h>
#include<OSUtils.h>

#define TrapMask 0x0800

short NumToolboxTraps( void )
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) ==
			NGetTrapAddress(0xAA6E, ToolTrap))
		return(0x0200);
	else
		return(0x0400);
}

TrapType GetTrapType(short theTrap)
{

	if ((theTrap & TrapMask) > 0)
		return(ToolTrap);
	else
		return(OSTrap);

}

Boolean TrapAvailable(short theTrap)
{

	TrapType	tType;

	tType = GetTrapType(theTrap);
	if (tType == ToolTrap)
	theTrap = theTrap & 0x07FF;
	if (theTrap >= NumToolboxTraps())
		theTrap = _Unimplemented;

	return (NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap));
}