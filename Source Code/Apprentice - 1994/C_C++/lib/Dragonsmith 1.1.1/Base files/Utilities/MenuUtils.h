/*
	MenuUtils.h
*/

#pragma once

#include	<Menus.h>

#define	iLastItem		(-1)			// For use in CheckOne

void ShowMenuAction (void);
void CheckOne (MenuHandle menu, short first, short last, short itemToCheck);
Boolean ToggleMenuItem (MenuHandle menu, short item);
Boolean ItemIsChecked (MenuHandle menu, short item);

