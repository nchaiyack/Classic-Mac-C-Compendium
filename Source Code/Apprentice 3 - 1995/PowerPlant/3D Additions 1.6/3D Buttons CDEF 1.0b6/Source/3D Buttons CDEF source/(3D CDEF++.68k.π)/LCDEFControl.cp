/*
	Public domain by Zig Zichterman.
*/
/*
	LCDEFControl
	
	Base class for 3D Buttons controls. Most of the
	code lives in LCDEFControl, with selected stuff 
	overridden in derived classes.
	
	12/29/94	zz	h	add mButtonKind
	12/20/94	zz	h	initial write
*/
#include "LCDEFControl.h"

#include "IsColorQDPresent.h"
#include "LCDEFCheckbox.h"
#include "LCDEFIconButton.h"
#include "LCDEFPushButton.h"
#include "LCDEFRadioButton.h"
#include "LColorTable.h"
#include "LDeviceLoop.h"
#include <Memory.h>
#include "RectT.h"
#include "RgnHandleT.h"
#include "StOffscreen.h"
#include "StSaveEm.h"
#include <TextEdit.h>
#include <ToolUtils.h>

/**************************************************************************
	Main()											[static, public]
	
	entry point for LCDEFControl. Create an object of the appropriate
	LCDEFControl derived class, and have it dispatch the message.
**************************************************************************/ 
long
LCDEFControl::Main(
	short			inVariation,
	ControlHandle	ioControl,
	short			inMsg,
	long			ioParam)
{
	// figure out what kind of control to create
	ButtonKindT		buttonKind				= ButtonKind_Unknown;
	Boolean			indentWhenHighlighted	= false;
	Boolean			useWindowFont			= false;
	UnpackVariant(inVariation,buttonKind,
			indentWhenHighlighted,useWindowFont);
	
	// perform ONE single check to see if the control handle
	// is sane. The rest of the CDEF can assume that the
	// handle isn't NULL or purged
	if ((ioControl == NULL) || (*ioControl == NULL)) {
		return 0;
	}
	
	// lock the control across the rest of the CDEF's work. We
	// dereference the ControlHandle in a few places, and it's
	// simpler to take care of the lock in one place
	char	saveLock	= ::HGetState((Handle) ioControl);
	::HLock((Handle) ioControl);
	
	// create the appropriate control kind, let it handle the
	// rest of the work
	long	result	= 0;
	switch (buttonKind) {
		case ButtonKind_RadioButton	:
			{
				LCDEFRadioButton	control(ioControl,buttonKind,
					indentWhenHighlighted,useWindowFont);
				result = control.Dispatch(inMsg,ioParam);
			}
			break;
			
		case ButtonKind_Checkbox	:
			{
				LCDEFCheckbox	control(ioControl,buttonKind,
					indentWhenHighlighted,useWindowFont);
				result = control.Dispatch(inMsg,ioParam);
			}
			break;
			
		case ButtonKind_IconButton	:
			{
				LCDEFIconButton	control(ioControl,buttonKind,
					indentWhenHighlighted,useWindowFont);
				result = control.Dispatch(inMsg,ioParam);
			}
			break;
			
		case ButtonKind_PushButton	:
			{
				LCDEFPushButton	control(ioControl,buttonKind,
					indentWhenHighlighted,useWindowFont);
				result = control.Dispatch(inMsg,ioParam);
			}
			break;
	}
	
	// restore the control's lock state
	::HSetState((Handle) ioControl,saveLock);
	
	// return result
	return result;
}

/**************************************************************************
	LCDEFControl()									[protected]
	
	Create a control. The ControlHandle should be valid and locked.
**************************************************************************/
LCDEFControl::LCDEFControl(
	ControlHandle	inControl,
	ButtonKindT		inButtonKind,
	Boolean			inIndentWhenHighlighted,
	Boolean			inUseWindowFont)
{
	mControl					= inControl;
	mButtonKind					= inButtonKind;
	mIndentWhenHighlighted		= inIndentWhenHighlighted,
	mUseWindowFont				= inUseWindowFont;
	mBackgroundColor.red		=
	mBackgroundColor.green		=
	mBackgroundColor.blue		= Grey_White;
	mInactiveFrameColor.red		=
	mInactiveFrameColor.green	=
	mInactiveFrameColor.blue	= Grey_Black;
}

/**************************************************************************
	~LCDEFControl()									[protected, virtual]
	
	Destroy a control. Nothing to dispose or tear down, so do nothing.
**************************************************************************/
LCDEFControl::~LCDEFControl()
{
}

/**************************************************************************
	Dispatch()										[protected]
	
	Depending on the incoming message do something. Because simple
	button CDEFs don't need to track the mouse, we only pay attention
	to a few commands, and return 0 for all others
**************************************************************************/
long
LCDEFControl::Dispatch(
	short	inMsg,
	long	ioParam)
{
	// assume we'll return 0
	long	result	= 0;
	
	switch (inMsg) {
		case drawCntl	:
			// draw the control in whatever state that
			// the control handle says. ioParam is the
			// part code (inButton, inThumb, and so on),
			// but we ignore it
			Draw();
			break;
		
		case testCntl	:
			// hit test a point against the control.
			// ioParam has the point's local coords,
			// h in the lower 16 bits, v in the upper 16 bits.
			{
				Point	hitPoint	= {0,0};
				hitPoint.h	= LoWord(ioParam);
				hitPoint.v	= HiWord(ioParam);
				result = HitTest(hitPoint);
			}
			break;
			
		case calcCRgns	:
			// calculate the button region. This is the
			// 24-bit addressing version of calcCntlRgn,
			// so this should only get called when 32-bit
			// addressing is off. ioParam is a region handle
			// that we must change to our control region.
			{
				// get the region handle
				// do the 32-bit clean thing
				RgnHandle	controlRgn = (RgnHandle)
					::StripAddress((Ptr) ioParam);
				
				// stuff our control region into the region
				GetControlRgn(controlRgn);
			}
			break;
		
		case calcCntlRgn	:
			// calculate the button region. This is the
			// 32-bit clean versino of calcCRgns, so this
			// should only get called when 32-bit addressing
			// is on. ioParam is a region handle that we must
			// change to our control region.
			{
				// get the region handle
				RgnHandle	controlRgn = (RgnHandle) ioParam;
				
				// stuff our control region into the region
				GetControlRgn(controlRgn);
			}
			break;
			
	}
	
	// return result
	return result;
}
	
/**************************************************************************
	UnpackVariant()									[protected, static]
	
	Strip the variation code down to 2 bits of button kinds,
	one bit of useWFont, and one bit of indentWhenHighlighted
**************************************************************************/
void
LCDEFControl::UnpackVariant(
	short			inVariation,
	ButtonKindT &	outButtonKind,
	Boolean &		outIndentWhenHighlighted,
	Boolean &		outUseWindowFont)
{
	// button kind is in the lowest 2 bits:
	// 0 = pushbutton, 1 = checkbox, 2 = radio button, 3 = icon button
	outButtonKind = (ButtonKindT) (inVariation & 0x03);
	
	// bit 3 (mask 0x04) is set if you want the name and icon
	// to slide down & right by one pixel when highlighted. The
	// flag is ignored for checkboxes and radio buttons. Leave
	// the bit cleared to match the _develop_ 15 interface.
	outIndentWhenHighlighted
		= (inVariation & IndentWhenHighlightedVariantFlag) != 0;
	
	// bit 4 (mask 0x08) is set if you want the button title
	// to draw in whatever the font is on entry (the window font).
	// Leave the bit cleared for system font, default size, 
	// plain, srcCopy.
	outUseWindowFont	= (inVariation & useWFont) != 0;
}

/**************************************************************************
	Draw()											[protected]
	
	Draw the control. Makes heavy use of virtual functions to let
	derived classes do the work
**************************************************************************/
void
LCDEFControl::Draw(void)
{
	// if we're invisible, don't bother
	if (IsVisible() == false) {
		return;
	}
	
	// save the font, clip region, pen, color, whatever
	StSaveEm	saveAll;

	// calculate colors
	CalcColors();
	
	// calculate any rect for parts such as checkboxes, radio buttons, etc
	CalcPartRects();
	
	// clip to our turf
	ClipToControlRgn();
	
	// loop through all the devices (screens)
	RectT			controlRect;
	GetControlRect(controlRect);
	LDeviceLoop		devices(controlRect);
	Boolean			is3D			= false;
	Boolean			isGreyCapable	= false;
	RectT			clippedRect;
	while (devices.Next(is3D,isGreyCapable,clippedRect)) {
		// set up an offscreen GWorld for the intersection
		// of the device and the original clip region
		StOffscreen 	offscreen(clippedRect);
		
		// offscreen GWorld doesn't work right for icons--something
		// to do with ::PlotIconID() I think. 
		if (mButtonKind == ButtonKind_IconButton) {
			offscreen.PostDraw();
		}
		
		// set up the font--if we're supposed to use the window font
		if (mUseWindowFont) {
			// use it, even on offscreen GWorlds
			saveAll.mText.Restore();
		} else {
			// use system font
			saveAll.mText.SystemFont();
		}
		
		// set up the pen. We like normal, boring, default pens.
		::PenNormal();
		
		// do the drawing
		DrawLoop((Is3DFlagT) is3D,(IsGreyCapableFlagT) isGreyCapable);
		
		// [copy offscreen buffer to screen through ~offscreen]
	}
	
	// [restore font, clip region, pen, color, whatever through ~saveAll]
}

/**************************************************************************
	HitTest()										[protected]
	
	Test if a point is in the control. Uses the virtual function
	GetControlRgn() and tests the point for inclusion in that region
**************************************************************************/
Boolean
LCDEFControl::HitTest(
	Point	inHitPoint) const
{
	// assume the point's not in the control
	Boolean	isPointInControl	= false;
	
	// allocate a region
	RgnHandle	controlRgn	= ::NewRgn();
	
	// if allocation succeeded
	if (controlRgn != NULL) {
		// stuff the control region into it
		GetControlRgn(controlRgn);
		
		// test for inclusion
		isPointInControl = ::PtInRgn(inHitPoint,controlRgn);
		
		// dump the region
		::DisposeRgn(controlRgn);
		controlRgn = NULL;
	} else {
		// not enough memory for region calculation
		// just use the control rect
		RectT	controlRect;
		GetControlRect(controlRect);
		isPointInControl = ::PtInRect(inHitPoint,&controlRect);
	}
	
	// return result
	return isPointInControl;
}

/**************************************************************************
	GetControlRgn()									[protected, virtual]
													[not an override   ]
													
	Calculate a region for the control. This default routine just
	uses the control's rectangle. Derived classes can do something
	fancier like framing a round rect.
**************************************************************************/
void
LCDEFControl::GetControlRgn(
	RgnHandle	outControlRgn) const
{
	// watch out for bogus region handles
	if ((outControlRgn == NULL) || (*outControlRgn == NULL)) {
		return;
	}
	
	// get the control rect
	RectT	controlRect;
	GetControlRect(controlRect);
	
	// apply it to the output region
	::RectRgn(outControlRgn,&controlRect);
}

/**************************************************************************
	GetControlRect()								[protected]
	
	Get the boudning rectangle for this control. Returns the
	contrlRect field of the ControlHandle
**************************************************************************/
void
LCDEFControl::GetControlRect(
	Rect &	outControlRect) const
{
	outControlRect = (**mControl).contrlRect;
}

/**************************************************************************
	GetControlValue()								[protected]
	
	Get the value for this control
**************************************************************************/
short
LCDEFControl::GetControlValue(void) const
{
	return (**mControl).contrlValue;
}

/**************************************************************************
	GetControlTitle()								[protected]
	
	Get the title for this control. Promise not to change it.
**************************************************************************/
StringPtr
LCDEFControl::GetControlTitle(void) const
{
	return (**mControl).contrlTitle;
}

/**************************************************************************
	IsVisible()										[protected]
	
	Is this control handle visible (true) or hidden (false)
**************************************************************************/
Boolean
LCDEFControl::IsVisible(void) const
{
	return ((**mControl).contrlVis != false);
}

/**************************************************************************
	IsEnabled()										[protected]
	
	Return true if the control is enabled, false if not (highlight == 255)
**************************************************************************/
LCDEFControl::IsEnabledFlagT
LCDEFControl::IsEnabled(void) const
{
	if ((**mControl).contrlHilite != 255) {
		return Flag_IsEnabled;
	} else {
		return Flag_IsNotEnabled;
	}
}

/**************************************************************************
	IsHighlighted()									[protected]
	
	Return true if the control is highlighted (1 < highlight < 255)
**************************************************************************/
LCDEFControl::IsHighlightedFlagT
LCDEFControl::IsHighlighted(void) const
{
	if ((1 <= (**mControl).contrlHilite) 
				&& ((**mControl).contrlHilite <= 254)) {
		return Flag_IsHighlighted;
	} else {
		return Flag_IsNotHighlighted;
	}
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
// drawing routines
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

/**************************************************************************
	CalcColors()									[protected]
	
	Calculate all the colors this control will need for drawing.
	If no color quickdraw, do nothing
**************************************************************************/
void
LCDEFControl::CalcColors(void)
{
	// if we lack color quickdraw
	if (IsColorQDPresent() == false) {
		// do nothing. Nobody will use RGB colors such as mBackgroundColor
		return;
	}
		
	// do our own stuff
	CalcBackgroundColor();
	CalcInactiveFrameColor();
	
	// do our derived class' stuff
	CalcColorsSelf();
}

/**************************************************************************
	CalcBackgroundColor()							[protected]
	
	Calculate the correct color to use for backgrounds. This is
	the same color as the window's background/content color, unless
	this control has an auxilliary custom color table that defines
	a different background color.
**************************************************************************/
void
LCDEFControl::CalcBackgroundColor(void)
{
	// do we have a custom color for our control body?
	{
		LColorTable	controlColors;
		controlColors.GetControlColors(mControl);
		// if have color for body
		if (controlColors.GetColorForPart(
				LColorTable::Part_ControlBody,mBackgroundColor)) {
			// look no further
			return;
		}
	}
	
	// do we have a custom color for our window content?
	{
		WindowPtr	window	= NULL;
		::GetPort(&window);
		LColorTable	windowColors;
		windowColors.GetWindowColors(window);
		// if have color for content
		if (windowColors.GetColorForPart(
				LColorTable::Part_WindowContent,mBackgroundColor)) {
			// look no further
			return;
		}
	}
	
	// no custom colors. Use default grey
	mBackgroundColor.red	=
	mBackgroundColor.green	=
	mBackgroundColor.blue	= Grey_Background;
}

/**************************************************************************
	CalcDimColor()									[protected]
	
	Figure out a color partway between black and the background color
**************************************************************************/
void
LCDEFControl::CalcInactiveFrameColor(void)
{
	Blend(Grey_Black,mBackgroundColor,0.45,mInactiveFrameColor);	
}


/**************************************************************************
	ClipToControlRgn()								[protected]
	
	Clip to the intersection of the current clip region and the
	control's region.
**************************************************************************/
void
LCDEFControl::ClipToControlRgn(void) const
{
	RgnHandleT	controlRgn;
	GetControlRgn(controlRgn);
	RgnHandleT	clipRgn;
	clipRgn.CopyFromClip();
	clipRgn.SectWith(controlRgn);
	clipRgn.CopyToClip();
}

/**************************************************************************
	DrawLoop()										[protected]
	
	Draw control. Any offscreen or device loop stuff is already
	taken care of by Draw().
**************************************************************************/
void
LCDEFControl::DrawLoop(
	Is3DFlagT			inIs3D,					// can we draw 3D effects?
	IsGreyCapableFlagT	inIsGreyCapable) const	// can we use true grey for dimmed stuff?
{
	// cache a couple booleans
	IsEnabledFlagT		isEnabled		= IsEnabled();
	IsHighlightedFlagT	isHighlighted	= IsHighlighted();
	
	// set the pen for erase
	SetBGColorToErase(inIs3D,isEnabled,isHighlighted);
	
	// erase whatever needs erasin'
	Erase();
	
	// set the pen for drawing frames & text
	SetFGColorToFrame(inIs3D,inIsGreyCapable,isEnabled);
	
	// draw text
	RectT	titleRect;
	DrawTitle(inIs3D,isHighlighted,titleRect);
	
	// grey out text if necessary
	DimTitleIfNecessary(inIsGreyCapable,isEnabled,titleRect);
	
	// invert 1-bit highlighted controls
	if ((inIs3D == false) && (isHighlighted)) {
		InvertHighlight();
	}

	// draw any checkmark, radio button, or icon button value
	DrawValue(inIs3D, isEnabled, isHighlighted);

	// draw frame
	DrawFrame();
	
	// draw 3D effects
	if (inIs3D && isEnabled) {
		Draw3DEffects();
	}
}

/**************************************************************************
	SetFGColorToFrame()								[protected]
	
	Set the foreground color to black (if active), or grey (if
	inactive and grey capable)
**************************************************************************/
void 
LCDEFControl::SetFGColorToFrame(
	Is3DFlagT			inIs3D,
	IsGreyCapableFlagT	inIsGreyCapable,
	IsEnabledFlagT		inIsEnabled) const
{
	// if we're enabled, use black. Period.
	if (inIsEnabled) {
		::ForeColor(blackColor);
		return;
	}
	
	// we're not enabled (aka inactive). We want grey. Well, greyish...
	
	// if we can use 3D effects,
	if (inIs3D) {
		// we can use the inactive frame color
		::RGBForeColor(&mInactiveFrameColor);
	} else if (inIsGreyCapable) {
		// we can at least use a light grey
		ForeGrey(Grey_2BitInactiveFrame);
	} else {
		// we're stuck with black
		::ForeColor(blackColor);
	}
}

/**************************************************************************
	DimTitleIfNecessary()							[protected]
	
	If the device in incapable of drawing grey, and the control
	is inactive, then blot out the control with a 50% dither
**************************************************************************/
void
LCDEFControl::DimTitleIfNecessary(
	IsGreyCapableFlagT	inIsGreyCapable,
	IsEnabledFlagT		inIsEnabled,
	const Rect &		inTitleRect) const
{	
	if ((inIsGreyCapable == false) && (inIsEnabled == false)) {
		DimRect(inTitleRect);
	}
}


//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ
// drawing utilities
//ΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡΡ

/**************************************************************************
	ForeGrey()										[protected, static]
	
	Change the pen's foreground color to a grey with the given
	value for all three RGB values
**************************************************************************/
void
LCDEFControl::ForeGrey(
	unsigned short	inGrey)
{
	RGBColor	foreColor;
	foreColor.red = foreColor.green = foreColor.blue = inGrey;
	::RGBForeColor(&foreColor);
}

/**************************************************************************
	BackGrey()										[protected, static]
	
	Change the pen's background color to a grey with the given
	value for all three RGB values
**************************************************************************/
void
LCDEFControl::BackGrey(
	unsigned short	inGrey)
{
	RGBColor	backColor;
	backColor.red = backColor.green = backColor.blue = inGrey;
	::RGBBackColor(&backColor);
}

/**************************************************************************
	Blend()											[protected, static]
	
	Blend a grey with a color. Excuse the floating point math, it's 
	pokey on 68k, but I doubt you'll notice
**************************************************************************/
void
LCDEFControl::Blend(
	unsigned short	inGreyLevel,	// 0x0000 to 0xFFFF
	const RGBColor	&inColor,		// color to mix with inGreyLevel
	float			inPercentGrey,	// 1.0 = pure grey, 0.0 = pure color
	RGBColor		&outBlend)
{
	// pin ratio to sensible values
	float percentGrey	= inPercentGrey;
	if (percentGrey < 0.0) {
		percentGrey = 0.0;
	} else if (1.0 < percentGrey) {
		percentGrey = 1.0;
	}
	
	// convert colors to floats
	float floatGrey		= (float) inGreyLevel;
	float colorRed		= (float) inColor.red;
	float colorGreen	= (float) inColor.green;
	float colorBlue		= (float) inColor.blue;
	
	// calc the difference between grey and color
	float diffRed	= floatGrey - colorRed;
	float diffGreen	= floatGrey - colorGreen;
	float diffBlue	= floatGrey - colorBlue;

	// calc the amount of the difference to add to the color
	float addRed	= percentGrey * diffRed;
	float addGreen	= percentGrey * diffGreen;
	float addBlue	= percentGrey * diffBlue;
	
	// calc the blended color
	float blendRed		= colorRed		+ addRed;
	float blendGreen	= colorGreen	+ addGreen;
	float blendBlue		= colorBlue		+ addBlue;
	
	// convert back to RGBColor
	outBlend.red	= blendRed;
	outBlend.green	= blendGreen;
	outBlend.blue	= blendBlue;
}

/**************************************************************************
	StringBox()										[protected, static]
	
	Draw a string in the given box. The string is vertically centered
	in the given box, so if the string is too long, the top and bottom
	lines will get cut off. Yo
**************************************************************************/
void
LCDEFControl::StringBox(
	const StringPtr	inString,
	const Rect &	inBox,
	short			inAlign)
{
	// use the same horizontal coords as the supplied box
	RectT	textBox;
	textBox.left	= inBox.left;
	textBox.right	= inBox.right;
	
	// how tall is the string?
	const short	TitleHeight	= StringHeight(inString) + 0;
	
	// vertically center the string in the box
	const short	CenterV	= (inBox.top + inBox.bottom)/2;
	textBox.top		= CenterV - (TitleHeight/2);
	textBox.bottom	= textBox.top + TitleHeight;
	
	// draw it
	::TETextBox(inString + 1,*inString,&textBox,inAlign);
}

/**************************************************************************
	CountLines()									[protected, static]
	
	Count the number of lines (carriage returns) in a string
**************************************************************************/
short
LCDEFControl::CountLines(
	const StringPtr	inString)
{
	// start with one line
	short			lineCount		= 1;
	
	// iterate through all the characters
	short				charsRemaining	= *inString;
	unsigned char *		charPtr			= inString + 1;
	const unsigned char	Return			= '\r';
	for (;charsRemaining--;charPtr++) {
		// each carriage return implies another line
		if (*charPtr == Return) lineCount++;
	}
	
	// return the number of lines we counted
	return lineCount;
}

/**************************************************************************
	StringHeight()									[protected, static]
	
	Return the height of a string in pixels. Takes into account 
	the number of lines
**************************************************************************/
short
LCDEFControl::StringHeight(
	const StringPtr	inString)
{
	// get the height of one line of text
	FontInfo	fontInfo;
	::GetFontInfo(&fontInfo);
	const short	LineHeightPixels	= fontInfo.ascent + fontInfo.descent;
	
	// get the number of lines in the string
	const short	LineCount	= CountLines(inString);
	
	// return the product, plus first-line leading
	return LineCount * LineHeightPixels + fontInfo.leading;
}

/**************************************************************************
	DimRect()										[protected, static]
	
	clear out a dither of 50% grey from the given rect. Useful for
	dimming inactive controls on shallow screens
**************************************************************************/
void
LCDEFControl::DimRect(
	const Rect &	inRect)
{
	// fabricate a 50% grey dither through code.
	// this way we don't rely on QuickDraw globals
	unsigned char grey[8];
	grey[0] = grey[2] = grey[4] = grey[6] = 0xAA;
	grey[1] = grey[3] = grey[5] = grey[7] = 0x55;
	
	::PenPat((PatPtr) grey);
	::PenMode(patBic);
	::PaintRect(&inRect);
	::PenNormal();
}
