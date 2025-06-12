void DrawMenuItem(MenuHandle whichMenu, Rect *menuRect, short whichItem,
					MDEFconstants hiliteState, MDEFcolors *menuColors) {
	Str255 itemText;
	Rect itemRect;
	Boolean itemDisabled;
	short itemMarkOffset = 0;

	// Get text of menu item.
	GetItem(whichMenu, whichItem, itemText);

	// Determine menu item's rect.
	GetMenuItemRect(menuRect, &itemRect, whichItem);
	// Do some rect clipping & fudging
	InsetRect(&itemRect, rectPadding, rectPadding);
	itemRect.right--;
	itemRect.bottom--;

	// Catch special menu item cases, such as the dividing line "(-"
	if (itemText[1] == '-') {
		// Find height of menu item, divided by two
		short halfWay = (itemRect.bottom - itemRect.top) >> 1;
		// Draw top line (shadow)
		RGBForeColor(&menuColors->dkGray);
		MoveTo(itemRect.left  + 1, itemRect.top + halfWay);
		LineTo(itemRect.right - 2, itemRect.top + halfWay);
		// Draw bottom line (highlight)
		RGBForeColor(&menuColors->white);
		MoveTo(itemRect.left + 1, itemRect.top + halfWay + 1);
		LineTo(itemRect.right - 2, itemRect.top + halfWay + 1);
		// Restore colors
		RGBForeColor(&menuColors->black);
		return; // No more to draw, so exit
	}

	itemDisabled = IsItemDisabled(whichMenu, whichItem);

	if (!itemDisabled) {
		// If not disabled, draw the "3D box" surrounding menu item text...
		if (hiliteState == unhilite)
			RGBForeColor(&menuColors->white);
		else
			RGBForeColor(&menuColors->dkGray);
		// Draw topleft shadow or highlight
		MoveTo(itemRect.left, itemRect.top);
		LineTo(itemRect.left, itemRect.bottom);
		MoveTo(itemRect.left, itemRect.top);
		LineTo(itemRect.right, itemRect.top);
		if (hiliteState == unhilite)
			RGBForeColor(&menuColors->dkGray);
		else
			RGBForeColor(&menuColors->white);
		// Draw botRight shadow or highlight
		MoveTo(itemRect.right, itemRect.top + 1);
		LineTo(itemRect.right, itemRect.bottom);
		MoveTo(itemRect.left + 1, itemRect.bottom);
		LineTo(itemRect.right, itemRect.bottom);
	}
	RGBForeColor(&menuColors->ltGray);
	InsetRect(&itemRect, 1, 1);
	PaintRect(&itemRect);
	InsetRect(&itemRect, -1, -1);

	// Draw item mark, if any. Remember to note if menu item is disabled or not
	itemMarkOffset = StringWidth("\pÃ") + 4;
	short theMark, theChar;
	GetItemMark(whichMenu, whichItem, &theMark);
	GetItemCmd(whichMenu, whichItem, &theChar);
	if (theMark != noMark && theChar != hMenuCmd) {
		// Not a submenu, but an actual item mark, so we have to draw it
		if (hiliteState == hilite)
			MoveTo(menuRect->left + widthPad, itemRect.bottom - heightPad + 1 + textHtPad);
		else
			MoveTo(menuRect->left + widthPad - 1, itemRect.bottom - heightPad + textHtPad);
		RGBForeColor(&menuColors->black);
		TextFont(systemFont);
		if (itemDisabled)
			TextMode(grayishTextOr);
		DrawChar(theMark);
		if (changeFont)
			TextFont(menuFont);
		if (itemDisabled)
			TextMode(sysTextMode);
	}

	// Now time to draw the menu item text itself...
	if (itemDisabled) {
		// If item is disabled, draw text that is grayed-out, via
		// the grayishTextOr mode (available in System 7 only)
		MoveTo(menuRect->left + widthPad - 1 + itemMarkOffset, itemRect.bottom - heightPad);
		RGBForeColor(&menuColors->black);
		// Set the gray-out mode
		TextMode(grayishTextOr);
		DrawString(itemText);
		// Restore old text mode
		TextMode(sysTextMode);
	}
	else {
		// Draw normal menu item text...
		RGBForeColor(&menuColors->black);
		if (hiliteState == hilite) {
			// Offset down & right by one pixel
			MoveTo(menuRect->left + widthPad + itemMarkOffset, itemRect.bottom - heightPad + 1 + textHtPad);
		}
		else {
			// Move to actual text position
			MoveTo(menuRect->left + widthPad - 1 + itemMarkOffset, itemRect.bottom - heightPad + textHtPad);
		}
		DrawString(itemText);
	}
	
	// Alright, time to draw Cmd-keys and/or submenu triangles
	if (theChar == hMenuCmd) {
		// Draw submenu symbol
		Rect subMenuRect;
		SetRect(&subMenuRect, 0, 0, subMenuWd, subMenuHt);
		subMenuRect.top = (itemRect.top + ((itemRect.bottom - itemRect.top) >> 1)) - (subMenuHt >> 1);
		subMenuRect.right = itemRect.right - rectPadding - 4;
		subMenuRect.left = subMenuRect.right - subMenuWd;
		subMenuRect.bottom = subMenuRect.top + subMenuHt;

		if (hiliteState == hilite)
			OffsetRect(&subMenuRect, 1, 1);
		short halfHeight = subMenuRect.top + ((subMenuRect.bottom - subMenuRect.top) >> 1);

		RGBForeColor(&menuColors->dkGray);
		MoveTo(subMenuRect.left, subMenuRect.top);
		LineTo(subMenuRect.right, halfHeight);
		MoveTo(subMenuRect.left, subMenuRect.top);
		LineTo(subMenuRect.left, subMenuRect.bottom);
		RGBForeColor(&menuColors->white);
		MoveTo(subMenuRect.left, subMenuRect.bottom);
		LineTo(subMenuRect.right, halfHeight);
		RGBForeColor(&menuColors->black);
	}


	// Draw cmd-key
	else if (theChar != 0) {
		short cmdWidth = StringWidth("\pW");	// "W" is widest character (I think)
		cmdWidth += CharWidth(17);				// "Cmd" key character
		if (hiliteState == hilite && !itemDisabled)
			MoveTo(itemRect.right - (cmdKeyPad + cmdWidth) + 1, itemRect.bottom - heightPad + textHtPad + 1);
		else
			MoveTo(itemRect.right - (cmdKeyPad + cmdWidth), itemRect.bottom - heightPad + textHtPad);
		TextFont(systemFont);
		if (itemDisabled)
			TextMode(grayishTextOr);
		DrawChar(17);						// ASCII = 17; command-key clover character
		if (changeFont)
			TextFont(menuFont);
		DrawChar(theChar);
		if (itemDisabled)
			TextMode(sysTextMode);
	}

} // END DrawMenuItem