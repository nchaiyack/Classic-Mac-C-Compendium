/*
	MenuUtils.c
	
	Created	24 May 1992	Extracted from DFilePaths.c
	Modified	11 Jul 1992	Added ShowMenuAction
			15 Jul 1992	Got rid of call to Delay in ShowMenuAction � Delay hogs the CPU
			06 Sep 1992	Added check for last == iLastItem (-1) in CheckOne to allow for an arbitrarily large item
							group at the end of a menu

	Copyright � 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"MenuUtils.h"
#include	"EventUtils.h"

void ShowMenuAction (void)
{
	long		ticks;
	
	YieldCPUTime (2);		/* Wait a little to let the user see the hilited menu title */
	HiliteMenu (0);			/* Then clear the hilited menu (whatever it was) */
}

void CheckOne (MenuHandle menu, register short first, register short last, register short itemToCheck)
{
	register short		i;
	short			mark;
	
	if (last == iLastItem)
		last = CountMItems (menu);
	if (itemToCheck < first || itemToCheck > last)
		return;
	for (i = first; i < itemToCheck; i++)
		SetItemMark (menu, i, noMark);
	SetItemMark (menu, itemToCheck, checkMark);
	for (i = itemToCheck + 1; i <= last; i++)
		SetItemMark (menu, i, noMark);
}

Boolean ItemIsChecked (MenuHandle menu, short item)
{
	short		mark;
	
	GetItemMark (menu, item, &mark);
	return (mark == checkMark);
}

Boolean ToggleMenuItem (MenuHandle menu, short item)
{
	Boolean	newSetting;
	
	newSetting = ! ItemIsChecked (menu, item);
	CheckItem (menu, item, newSetting);
	return newSetting;
}
