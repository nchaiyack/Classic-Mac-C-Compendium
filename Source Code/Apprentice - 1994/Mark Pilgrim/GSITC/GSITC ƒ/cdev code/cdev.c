/**********************************************************************\

File:		cdev.c

Purpose:	This module handles the actual cdev; check the switch-case
			statement in main(...) for all the messages it does and
			doesn't handle.
			

Go Sit In The Corner -=- not you, just the cursor
Copyright �1994, Mark Pilgrim

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "cdev globals.h"
#include "cdev prefs.h"
#include "cdev.h"
#include "cdev meat.h"
#include "cdev gui.h"
#include "cdev error.h"
#include "Retrace.h"

pascal long main(int message, int item, int numItems, int CPanelID,
	EventRecord *theEvent, PrefHandle cdevStorage, DialogPtr theDlog)
{
	int				resultCode;
	
	if (message==macDev)
		return 1L;											/* we work on every machine */
	else if (cdevStorage!=0L)
	{
		switch (message) 
		{
			case initDev:									/* initialize cdev */
				cdevStorage=NewHandle(sizeof(PrefStruct));	/* create private storage */
				if (cdevStorage==0L)
					return 0L;

				resultCode=PreferencesInit(cdevStorage);
				if ((resultCode!=prefs_allsWell) && (resultCode!=prefs_virginErr))
				{
					HandleError(resultCode);
					return cdevGenErr;
				}
				
				RemoveTheVBL(cdevStorage);
				
				CheckOnOff(theDlog, numItems, (**cdevStorage).isOn);
				CheckShowIcon(theDlog, numItems, (**cdevStorage).showIcon);
				CheckTime(theDlog, numItems, (**cdevStorage).numTicks);
				CheckCorner(theDlog, numItems, (**cdevStorage).whichCorner);
				CheckAlways(theDlog, numItems, (**cdevStorage).always);
				break;
			case hitDev:									/* handle hit on item */
				switch (item-numItems)
				{
					case kButtonOn:
					case kTitleOn:
						(**cdevStorage).isOn=0xFF;
						CheckOnOff(theDlog, numItems, TRUE);
						break;
					case kButtonOff:
					case kTitleOff:
						(**cdevStorage).isOn=0x00;
						CheckOnOff(theDlog, numItems, FALSE);
						break;
					case kButtonShowIcon:
					case kTitleShowIcon:
						(**cdevStorage).showIcon=!((**cdevStorage).showIcon);
						CheckShowIcon(theDlog, numItems, (**cdevStorage).showIcon);
						break;
					case kButtonAlways:
					case kTitleAlways:
						(**cdevStorage).always=0xFF;
						CheckAlways(theDlog, numItems, TRUE);
						break;
					case kButtonOnlyIf:
					case kTitleOnlyIf:
						(**cdevStorage).always=0x00;
						CheckAlways(theDlog, numItems, FALSE);
						break;
					case kButtonTopLeft:
					case kButtonTopRight:
					case kButtonBottomRight:
					case kButtonBottomLeft:
						(**cdevStorage).whichCorner=item-numItems-kButtonTopLeft;
						CheckCorner(theDlog, numItems, (**cdevStorage).whichCorner);
						break;
				}
				if ((item-numItems>=kTitleFirstTime) && (item-numItems<=kTitleLastTime))
					item-=5;
				if ((item-numItems>=kButtonFirstTime) && (item-numItems<=kButtonLastTime))
				{
					switch (item-numItems-kButtonFirstTime)
					{
						case 0:	(**cdevStorage).numTicks=900;	break;
						case 1:	(**cdevStorage).numTicks=1800;	break;
						case 2:	(**cdevStorage).numTicks=3600;	break;
						case 3:	(**cdevStorage).numTicks=7200;	break;
						case 4:	(**cdevStorage).numTicks=18000;	break;
					}
					CheckTime(theDlog, numItems, (**cdevStorage).numTicks);
				}
				break;
			case closeDev:									/* clean up and dispose */
				if ((**cdevStorage).isOn)
				{
					resultCode=InstallTheVBL(cdevStorage);
					if (resultCode!=allsWell)
					{
						HandleError(resultCode);
						return cdevGenErr;
					}
				}
				SaveThePrefs(cdevStorage);
				break;
			case nulDev:
			case updateDev:									/* handle any update drawing */
			case activDev:									/* activate any needed items */
			case deactivDev:								/* deactivate any needed items */
			case keyEvtDev:									/* respond to keydown */
			case undoDev:
			case cutDev:
			case copyDev:
			case pasteDev:
			case clearDev:
				break;
		}

		return (long)cdevStorage;
	}
	
	return 0L;
}
