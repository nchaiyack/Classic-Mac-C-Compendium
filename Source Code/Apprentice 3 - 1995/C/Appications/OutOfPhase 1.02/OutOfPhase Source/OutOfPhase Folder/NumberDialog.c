/* NumberDialog.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "NumberDialog.h"
#include "StringDialog.h"
#include "Numbers.h"
#include "Memory.h"


/* present a dialog displaying InitialValue and asking for a new value.  the */
/* new value is returned.  if the dialog couldn't be presented or the user cancelled, */
/* then the original number is returned */
long					DoNumberDialog(char* Prompt, long InitialValue, struct MenuItemType* MCut,
								struct MenuItemType* MPaste, struct MenuItemType* MCopy,
								struct MenuItemType* MUndo, struct MenuItemType* MSelectAll,
								struct MenuItemType* MClear)
	{
		char*								String;

		String = IntegerToString(InitialValue);
		if (String != NIL)
			{
				if (DoStringDialog(Prompt,&String,MCut,MPaste,MCopy,MUndo,MSelectAll,MClear))
					{
						InitialValue = StringToInteger(String,PtrSize(String));
					}
				ReleasePtr(String);
			}
		return InitialValue;
	}
