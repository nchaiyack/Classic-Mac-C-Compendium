/******************************************************************************

	UPortText.cp					Requires Metrowerks PowerPlant.
		
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
	
	Example of usage:
	
	CMyWowDraw::Drawit()
	{
	
		// If you are going to draw outside of the DrawSelf routine you must call this
		FocusDraw();

		UPortText	*theText = new UPortText(4,9,0,srcOr,blackColor);
		<or>
		UPortText	*theText = new UPortText(130);

		...do some drawing here...
	
		delete theText;		// or you could just depend on the fact that theText is local
							// var its distructor will get called.	
	}	
	
******************************************************************************/
#include	"UPortText.h"
#include	<UEnvironment.h>
#include	<UTextTraits.h>

// Manual building Constructor
UPortText::UPortText(short fontNum, short fontSize, Style fontStyle,
 short fontMode,RGBColor fontColor)
{
	GetCurrent();
	
	TextFont(fontNum);
	TextFace(fontStyle);
	TextSize(fontSize);
	TextMode(fontMode);
	
	if (UEnvironment::HasFeature(env_SupportsColor)) {
		RGBForeColor(&fontColor);
	}
}

// Build from txtr resource constructor.
UPortText::	UPortText(ResIDT theID)
{
	GetCurrent();
	UTextTraits::SetPortTextTraits(theID);
}

// Destructor, set everything back the way it was.
UPortText::~UPortText()
{
	TextFont(iOldFont);
	TextFace(iOldFace);
	TextSize(iOldSize);
	TextMode(iOldMode);
	
	if (UEnvironment::HasFeature(env_SupportsColor)) {
		RGBForeColor(&iOldColor);
	}
}
// Handles saving the current port settings.
void UPortText::GetCurrent()
{
	iOldFont = qd.thePort->txFont;
	iOldFace = qd.thePort->txFace;
	iOldSize = qd.thePort->txSize;
	iOldMode = qd.thePort->txMode;
	
	if (UEnvironment::HasFeature(env_SupportsColor)) {
		GetForeColor(&iOldColor);
	}
}