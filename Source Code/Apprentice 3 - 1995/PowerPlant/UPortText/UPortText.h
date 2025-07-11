/******************************************************************************

	UPortText.h					Requires Metrowerks PowerPlant.
		
	by Ron Davis
	7/21/94
	
	Anyone on the planet may use this code, but don't blame me for problems with it.
	People off planet and Joe Zobkiw must get special permission from the author to 
	use it.
	
	This is a utility object that saves the current text info in the constructor,
	sets it to what is passsed in and then restores it in the destructor.
	
	There are two constructors, one for manual entry and one for Txtr resources.
	
	WARNING: Beware of FocusDraw().  FocusDraw does something to change the
	graphics port and this causes UPortText to not be able to reset the TextSize.
	Don't ask me why, I didn't look into it that far.
	
	See UPortText.cp for examples of usage.
	
******************************************************************************/

#pragma once

#include	<UTextTraits.h>

class UPortText {

public:
	short	iOldFont;
	Style	iOldFace;
	short	iOldSize;
	short	iOldMode;
	RGBColor	iOldColor;

			UPortText(short fontNum, short fontSize, Style fontStyle, 
				short fontMode,RGBColor fontColor);
			UPortText(ResIDT theID);
			~UPortText();
	void	GetCurrent();
};