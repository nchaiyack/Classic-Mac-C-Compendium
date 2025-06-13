/*
	File:		DinkUtils.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		fixing minor typeo
		 <3>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <2>	 9/20/92	MTG		Bringing the C++ verion of DinkClass up to date with the THINKC
									version

	To Do:
*/

// this file has a number of utility fuctions
// for The DinkClass
#include <DinkUtils.h>
#include "DApplication.h"
#include "DEventHandler.h"
#include "DDocument.h"
#include <Editions.h>
#include <Traps.h>
#include <GestaltEqu.h>
#ifdef THINK_C
	//NOTHING!, this trap is in the THINK headder files
#else
	//for MPW
	#define _GestaltDispatch 0xA0AD
#endif

// commented out for MPW extern DApplication *DEventHandler::gApplication;

void InitToolBox(int numMoreMasers)
{
	int i;
	EventRecord event;
	
	MaxApplZone();
	for (i=0; i<numMoreMasers; i++)
		MoreMasters();

	FlushEvents(everyEvent, 0);

	InitGraf((Ptr) &qd.thePort);
//	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( NULL);
	InitCursor();
	
	InitEditionPack();
	
	for (i = 1; i <= 3; i++)
		EventAvail(everyEvent, &event);
}




OSErr RequiredCheck( AppleEvent *theAppleEvent)
{
	OSErr myErr;
	DescType typeCode;
	Size actualSize;
	
	myErr = AEGetAttributePtr( theAppleEvent, keyMissedKeywordAttr, typeWildCard,
								&typeCode, 0L, 0, &actualSize);
	
	if (myErr == errAEDescNotFound ) 
		return noErr;
	
	if (myErr == noErr )
		return errAEEventNotHandled ;
	
	return myErr;
}

Boolean System7Available(void)
{
	long sysVersion;
	
	if( !TrapExists(_GestaltDispatch) ) 
		return false;
	
	if( !Gestalt( gestaltSystemVersion, &sysVersion) )
	{
		if(sysVersion >= 0x0700) 
			return true;
	}
	return false;
}


Boolean TrapExists(short theTrap)
{
	TrapType	theTrapType;

	theTrapType = GetTrapType(theTrap);
	if ((theTrapType == ToolTrap) && ((theTrap &= 0x07FF) >= NumToolboxTraps()))
		theTrap = _Unimplemented;

	return (NGetTrapAddress(_Unimplemented, ToolTrap) != NGetTrapAddress(theTrap,
				  theTrapType));
}

TrapType	GetTrapType(short theTrap)
{
	/* OS traps start with A0, Tool with A8 or AA. */
	if ((theTrap & 0x0800) == 0)					/* per D.A. */
		return (OSTrap);
	else
		return (ToolTrap);
}



short	NumToolboxTraps(void)
{
	if (NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		return (0x200);
	else
		return (0x400);
}




void	pstrcat(Str255 dest, Str255 src)
{
	short	i;

	for (i = 0; i < src[0];)
		dest[++dest[0]] = src[++i];
}


void	pstrcpy(Str255 dest, Str255 src)
{
	BlockMove (src, dest, (*(char *)(src))+1);
}


