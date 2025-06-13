/*****
 *
 *	IconUtil.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include <Icons.h>

#include "IconUtil.h"


/***  FUNCTIONS  ***/

/* IM-MoreMTB: 5-10 */
void
IconDrawFromFamily ( short resID, Rect *destRect, Handle *iconSuiteHdl )
{
	IconSelectorValue	iconType;
	IconAlignmentType	align;
	IconTransformType	transform;
	OSErr				theErr;
	
	iconType	= svAllAvailableData;
	theErr		= GetIconSuite ( iconSuiteHdl, resID, iconType );
	
	if ( iconSuiteHdl != nil )
	{
		align		= atAbsoluteCenter;
		transform	= ttNone;
		theErr		= PlotIconSuite ( destRect, align, transform, *iconSuiteHdl );
	}
} /* IconDrawFromFamily */


/*****  EOF  *****/
