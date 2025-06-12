//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CScrollNumPane
//|
//| This implements a pane containing a number which reflects the setting of a scroll bar
//|_______________________________________________________________________________________

#include "CColorPaneBorder.h"
#include "CScrollNumPane.h"


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CScrollNumPane::IScrollNumPane
//|
//| Purpose: Initialize the scroll number pane.
//|
//| Parameters: scroll_bar: scroll bar to track
//|             rest same as superclass
//|_________________________________________________________

void CScrollNumPane::IScrollNumPane(CView *anEnclosure, CBureaucrat *aSupervisor, short aWidth,
									short aHeight, short aHEncl, short aVEncl, SizingOption	aHSizing,
									SizingOption aVSizing, short line, CScrollBar *scroll_bar,
									short offset)
{

	IEditText(anEnclosure, aSupervisor,				//  Call superclass' initialization routine
				aWidth, aHeight, aHEncl, aVEncl,
					aHSizing, aVSizing, line);
	SetAlignCmd(cmdAlignLeft);
	SetFontNumber(geneva);
	SetFontSize(9);
	SetWantsClicks(FALSE);							//  No editing of this text
	Specify(kNotEditable, kNotSelectable,
				kNotStylable);

	RGBColor border_color = {0xB000, 0xB000, 0xB000};
	CColorPaneBorder *border =						//  Attach a colored shadow to this pane
						new(CColorPaneBorder);
	border->IColorPaneBorder(
					kBorderRight + kBorderBottom,
					&border_color);
	SetBorder(border);

	num_offset = offset;							//  Save number to add to everything

	DependUpon(scroll_bar);							//  The number depends on the scroll bar

	ProviderChanged(scroll_bar, 0, NULL);			//  Set number to the current scroll bar value

}	//==== CScrollNumPane::IScrollNumPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CScrollNumPane::Draw
//|
//| Purpose: Draw the scroll number pane.
//|
//| Parameters: the_rect: rectangle to draw
//|_________________________________________________________

void CScrollNumPane::Draw(Rect *the_rect)
{

	Prepare();

	RGBColor back_color;							//  Remember current background color
	GetBackColor(&back_color);

	RGBColor rect_color = {0xE000, 0xE000, 0xE000};	//  Change background color
	RGBBackColor(&rect_color);						

	CEditText::Draw(the_rect);						//  Draw the text (with colored background)

	RGBBackColor(&back_color);						//  Restore background color

}	//==== CScrollNumPane::Draw() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CScrollNumPane::ProviderChanged
//|
//| Purpose: This is called every time the scroll bar moves
//|
//| Parameters: provider: the scroll bar
//|             reason:   the reason we were called (controlValueChanged)
//|             info:     unused
//|_______________________________________________________________________

void CScrollNumPane::ProviderChanged(CCollaborator *provider, long reason, void *info)
{

	long new_value = ((CScrollBar *) provider)->GetValue();	//  Get new number
	
	Str255 num_string;										//  Update the number in the pane
	NumToString(new_value + num_offset, num_string);
	SetTextPtr((char *) (num_string+1), num_string[0]);

	Rect frame_rect;
	LongToQDRect(&frame, &frame_rect);
	Draw(&frame_rect);

}	//==== CScrollNumPane::ProviderChanged() ====\\
