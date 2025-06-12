/* NumberDialog.c */
/*****************************************************************************/
/*                                                                           */
/*    Stupid Fred's Text Editor                                              */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This software is Public Domain; it may be used for any purpose         */
/*    whatsoever without restriction.                                        */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
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
