/******************************************************************
	FILE:			ArrowCDEF.c
	CREATED:		March 16, 1994
	AUTHOR:		David Hay
	
	Copyright (C) 1994  David Hay
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Comments and questions are welcome:
	E-mail:	hay@cs.colorado.edu
	US Mail:	David Hay
			117 Piedra Loop
			Los Alamos, NM 87544
	
	Thanks to Eddy J. Gurney for helping a novice to Macintosh programming (me) and for
	showing me that Geneva 9 pt is a really cool font to program in!
******************************************************************/

#include <SetUpA4.h>

#include "ArrowCDEF.h"

#define	kInactive		255		/*	part code indicating the control is inactive		*/

const unsigned char copyright[] = "Arrow CDEF 1.0 �1994 David Hay.";

struct ControlResData			/*	The data we get from the resource	*/
{
	short	plainPictID;		/*	The PICT to draw for an unhilited arrow			*/
	short	upPictID;			/*	The PICT to draw when the up arrow is pressed		*/
	short	downPictID;		/*	The PICT to draw then the down arrow is pressed	*/
	short	inactivePictID;		/*	The PICT to draw when the arrow is inactive		*/
};

typedef struct ControlResData ControlResData;
typedef ControlResData **CntlResDataHndl;


struct ControlData			/*	the data we store in the contrlData field of the control	*/
{
	ControlResData		picts;	/*	The PICTs to draw 							*/
	short			resFile;	/*	The resource file to use, since it might change on us	*/
};

typedef struct ControlData ControlData;
typedef ControlData **ControlDataHandle;



pascal long main( short variation, ControlHandle theControl, short msg, long param )
{
	long					result;		/*	the result to return		*/
	ControlDataHandle		control;		/*	the control data			*/

	RememberA0();
	SetUpA4();

	result = 0;

	switch ( msg )
	{
		case initCntl:		/*	initialize the arrow control data		*/
		{
			short				arrowResID;	/*	res ID of the resource holding the pict info	*/
			CntlResDataHndl		picts;		/*	res handle to hold which picts to use		*/
			
			arrowResID = GetCRefCon( theControl );
			picts = (CntlResDataHndl) Get1Resource( kArrowResType, arrowResID );
			control = (ControlDataHandle)NewHandleClear( sizeof( ControlData ) );
			BlockMove( *picts, *control, sizeof( ControlResData ) );
			(*control)->resFile = CurResFile();		/*	save the res file in case it changes	*/
			(*theControl)->contrlData = (Handle)control;
			break;
		}

		case drawCntl:		/*	Draw the arrow control				*/
		{
			PicHandle			thePict;		/*	the PICT to draw			*/
			unsigned char		hilite;		/*	the current control hilite state	*/
			Rect				drawRect;		/*	the rect to draw the PICT in	*/
			short			pictID;		/*	the resource ID of the PICT	*/
			short			curRes;		/*	the current resource file		*/
			
			if ( (*theControl)->contrlVis )
			{
				thePict = NULL;
				hilite = (*theControl)->contrlHilite;
				control = (ControlDataHandle)(*theControl)->contrlData;
				drawRect = (*theControl)->contrlRect;
				switch( hilite )
				{
					case 0:			/*	no hiliting			 */
						pictID = (*control)->picts.plainPictID;
						break;
					case inUpButton:	/*	up arrow hilited	*/
						pictID = (*control)->picts.upPictID;
						break;
					case inDownButton:	/*	down arrow hilited	*/
						pictID = (*control)->picts.downPictID;
						break;
					case kInactive:		/*	inactive control		*/
						pictID = (*control)->picts.inactivePictID;
						break;
					default:			/* any other part codes don't apply */
						pictID = -1;
						break;
				}
				if ( pictID >= 0 )	/*	will probably always be true, but better safe than sorry!	*/
				{
					curRes = CurResFile();			/*	save the current res file		*/
					UseResFile( (*control)->resFile );	/*	change to the saved res file	*/
					thePict = GetPicture( pictID );		/*	Get the picture, draw it, if...	*/
					if ( thePict )					/*	...possible, then release it.	*/
					{
						if ( hilite == kInactive )		/*	Erase the arrow only when...	*/
							EraseRect( &drawRect );	/*	...it needs to be deactivated	*/
						DrawPicture( thePict, &drawRect );
						ReleaseResource( thePict );
					}
					UseResFile( curRes );			/*	resore the old res file		*/
				}
			}
			break;
		}

		case testCntl:		/*	Determine which part of the arrow the mouse is in	*/
		{
			Point			mousePoint;	/*	where the mouse is				*/
			Rect			upRect;		/*	rect comprimising the up arrow	*/
			Rect			downRect;		/*	rect comprimising the down arrow	*/
			Rect			plainRect;		/*	the entire arrow rect			*/
						
			plainRect = (*theControl)->contrlRect;
			mousePoint.v = HiWord( param );
			mousePoint.h = LoWord( param );

			/*	If the mouse point is in the control and it is active,
				determine which part the mouse went down in.			*/
			if ( (*theControl)->contrlHilite != kInactive && PtInRect( mousePoint, &plainRect ) )
			{
				/*	Assume the up arrow and down arrow each take half the control	*/
				upRect = plainRect;
				upRect.bottom -= (plainRect.bottom - plainRect.top) / 2;
				downRect = plainRect;
				downRect.top = upRect.bottom - 1;

				if ( PtInRect( mousePoint, &upRect ) )			/*	up arrow pressed	*/
				{
					result = inUpButton;
				}
				else if ( PtInRect( mousePoint, &downRect ) )		/*	down arrow pressed	*/
				{
					result = inDownButton;
				}
			}
			break;
		}
		
		case calcCRgns:	/*	This is the message sent if using 24-bit addressing	*/
			param = (long)StripAddress((Ptr)param);		/* Mask off the high byte if necessary */
			/*	Fall through on purpose!	*/

		case calcCntlRgn:	/*	Under 32-bit addressing we get this message		*/
			RectRgn((RgnHandle)param, &(*theControl)->contrlRect);
			break;
		
		case dispCntl:		/*	Free the data stored in the control				*/
			if ( (*theControl)->contrlData );
			{
				DisposeHandle( (*theControl)->contrlData );
				(*theControl)->contrlData = NULL;
			}
			break;

	#if 0	
		case posCntl:
		case thumbCntl:
		case dragCntl:
		case autoTrack:
		case undoDev:
		case cutDev:
		case copyDev:
		case pasteDev:
		case clearDev:
		case cursorDev:
		case calcThumbRgn:
			break;	/*	We don't respond to any of these messages	*/
	#endif

		default:	/*	Any other messages we don't handle or which aren't defined yet	*/
			break;
	}

	RestoreA4();

	return( result );
}
