//
//	cicnButton CDEF
//	version 1.0.0
//	20 March 1994
//	
//	Written by: Paul Celestin
//	
//	This CDEF displays a cicn whose resource ID is derived from the
//	value field of the CNTL. The min, max, and refcon fields are not
//	used and should therefore not be set.
//	
//	940320 - 1.0.0 - initial release
//	950505 - 1.0.1 - Reinder Verlinde
//						adapted to Universal headers,
//						converted to C++
//						converted to personal coding standards
//						added variation code for title to the right of cicn (+0x0001)
//						added variation code for use of window font (+0x0008)
//
// Known problems: should get the icons when 'initCntl' message is received,
// and release them at dispCntl time. Currently we end up creating and disposing a
// fresh handle everytime through main. (950611: did not change this because things
// may be more efficient this way (second icon is not retrieved until needed,
// resource manager may cache the icons for us)
//
// May not work on every Mac, since GetCIcon is called.
//
#include <Controls.h>
#include <QuickDraw.h>
#include <Resources.h>
#include <Icons.h>

const int offset_of_inverted_icon = 1;

pascal long main( short variationCode, ControlHandle theControl, short message, long param);

void drawIt( ControlHandle theControl, short variationCode);

long testIt( ControlHandle theControl, Point pointToTest);

pascal long main( short variationCode, ControlHandle theControl, short message, long param)
{
	long result = 0;

	const char state = HGetState( (Handle)theControl);

	switch( message)
	{
		case drawCntl:
			drawIt( theControl, variationCode);
			break;

		case testCntl:
			result = testIt( theControl, *(Point *) &param);
			break;
#if 0
		case calcCRgns:	// just to show that I know of these messages
	  	case initCntl:
		case dispCntl:
		case posCntl:
		case thumbCntl:
		case dragCntl:
		case autoTrack:
		case calcCntlRgn:
		case calcThumbRgn:
#endif
		default:
			break;
	}

	HSetState( (Handle)theControl, state);

	return result;
}
//
// drawIt - here is where we actually draw the control
//
void drawIt( ControlHandle theControl, short variationCode)
{
	//
	// Only do something if the control is visible
	//
	if( (*theControl)->contrlVis)
	{
		const char handleState = HGetState( (Handle)theControl);
		HLock( (Handle) theControl);
		//
		// the base ID of the 'cicn' resource is stored in the value field; get it
		//
		short myICONID = GetControlValue( theControl);

		if( (*theControl)->contrlHilite == kInButtonControlPart)
		{
			myICONID += offset_of_inverted_icon;
		}
		const Rect myRect = (*theControl)->contrlRect;
		
		CIconHandle myICON = GetCIcon( myICONID);
			
		if( myICON != 0)
		{
			EraseRect( &myRect);
			PlotCIcon( &myRect, myICON);
		
			const Str255 *myTitle = &(*theControl)->contrlTitle;

			if( StrLength( *myTitle) != 0)
			{
				GrafPtr savedPort;
				GetPort( &savedPort);
			
				const short savedFont = savedPort->txFont;
				const short savedSize = savedPort->txSize;
				const short savedMode = savedPort->txMode;
			
				if( (variationCode & useWFont) == 0)
				{
					TextFont( geneva);
					TextSize( 9);
				}
				TextMode( srcOr);

					short stringHPos;
					short stringVPos;
					if( (variationCode & 1) != 0)
					{
						stringHPos = (myRect.right + myRect.left - StringWidth( *myTitle)) >> 1;
						stringVPos = myRect.bottom + 12;
					} else {
						stringHPos = (myRect.right + 6);
						stringVPos = ((myRect.top + myRect.bottom) >> 1) + 5;
					}							
					MoveTo( stringHPos, stringVPos);
					DrawString( *myTitle);

				TextMode( savedMode);
				if( (variationCode & useWFont) == 0)
				{
					TextSize( savedSize);
					TextFont( savedFont);
				}
			}
			if ((*theControl)->contrlHilite == 255)
			{
				//
				// Gray out the control:
				//
				static const Pattern myGray = { {0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55}};
				PenState oldPenState;
				GetPenState(&oldPenState);
					PenPat( &myGray);
					PenMode( srcBic);
					PaintRect( &myRect);
				SetPenState( &oldPenState);
			}
			DisposeCIcon( myICON);
		}
		HSetState( (Handle) theControl, handleState);
	}
	return;
}
//
// testIt - test for mouse hits within control
//
long testIt( ControlHandle theControl, Point pointToTest)
{
	const Rect myRect = (*theControl)->contrlRect;
	long result = 0;

	if( ((*theControl)->contrlVis != 0)
		&& ((*theControl)->contrlHilite != 255)
		&& (PtInRect( pointToTest, &myRect)))
	{
		result = kInButtonControlPart;
	}
	return result;
}
