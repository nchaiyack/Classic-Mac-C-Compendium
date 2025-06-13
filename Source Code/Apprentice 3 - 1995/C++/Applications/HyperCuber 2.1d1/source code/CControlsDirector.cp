//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector.cp
//|
//| This file contains the implementation of the controls window director.
//|___________________________________________________________________________

#include "CControlsDirector.h"
#include "CControlsWindow.h"
#include "CGraphic.h"
#include "CHyperCuberPane.h"
#include "CHyperCuberPrefs.h"
#include "CHyperScrollBar.h"
#include "CScrollNumPane.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"
#include "HyperCuber Messages.h"

#include <stdio.h>
#include <string.h>



//============================ Constants ============================\\

#define	Pi	3.14159265358979323846


//================================ Globals =======================\\

Boolean fCreatingBars;				//  TRUE if we are creating the scroll bars.  This variable
									//    is necessary so the program doesn't respond to the
									//    SetValue in the initialization as though the user had
									//    moved the bar.


//================================ External Globals =======================\\

extern CDesktop			*gDesktop;
extern CBartender		*gBartender;
extern CHyperCuberPrefs	*gPrefs;

extern Boolean			drawing_disabled;


//============================ Procedure Prototypes =======================\\

pascal void	scroll_action_proc(ControlHandle control, short part);
void		scroll_thumb_proc(CScrollBar *bar, short delta);



//================================== Types ================================\\

typedef struct
	{
	CHyperScrollBar	*bar;
	long			dimension;
	long			angle_num;	
	} RefStruct;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::IControlsDirector
//|
//| Purpose: Initialize the director
//|
//| Parameters: aSupervisor:   the supervisor
//|             pane:          the pane containing the graphic
//|             the_dimension: the dimension it controls
//|______________________________________________________________

void CControlsDirector::IControlsDirector(CDirectorOwner *aSupervisor, CHyperCuberPane *pane,
											long the_dimension)
{

#define CONTROLS_WINDOW_ID 130

	CDirector::IDirector(aSupervisor);					//  Initialize superclass

	graphic_pane = pane;								//  Remember the pane the graphic's in
	dimension = the_dimension;							//  Remember the dimension to control

	BuildWindow();										//  Build the controls window
	
	if (gPrefs->prefs.
			controls_window_visible[the_dimension])
		itsWindow->Select();							//  Show the window, if it's visible
	
}	//==== CControlsDirector::IControlsDirector() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::BuildWindow
//|
//| Purpose: This builds the controls window and adds the controls
//|
//| Parameters: none
//|___________________________________________________________________________

void CControlsDirector::BuildWindow(void)
{

	itsWindow = new(CControlsWindow);
	((CControlsWindow *) itsWindow)->
			IControlsWindow(CONTROLS_WINDOW_ID,
					TRUE, gDesktop, this, dimension);	//  Create the window
	itsWindow->helpResID = WINDOW_HELP_RES;				//  Link in Balloon help

	short window_height = dimension*20 + 5;
	Rect size_rect;										//  Limit size of window to size of
	gDesktop->GetBounds(&size_rect);					//  desktop and make sure it's always
	size_rect.left = 200;								//  large enough to use the scroll bars
	size_rect.top = window_height;
	size_rect.bottom = window_height;
	itsWindow->SetSizeRect(&size_rect);

	((CControlsWindow *) itsWindow)->PlaceWithVerify(
			&gPrefs->prefs.
				controls_window_position[dimension]);	//  Place window in default position
														//   (but not offscreen)

	Rect window_rect;
	((CControlsWindow *) itsWindow)->
							GetRect(&window_rect);		//  Find the window position
	Point float_loc; 
	float_loc.h = window_rect.left;
	float_loc.v = window_rect.top;
	itsWindow->SetShowFloatLoc(float_loc);				//  Set floating location

	const char left_brace[2] = "[";
	const char right_brace[2] = "]";
	const char colon[2] = ":";
	const char Pchar[2] = "P";
	char dimension_string[10];
	NumToString(dimension, 
				(unsigned char *) dimension_string);	//  Convert dimension to a string
	PtoCstr((unsigned char *) dimension_string);
	
	char bar_title[30];
	strcpy(bar_title, left_brace);						//  Create the perspective bar title
	strcat(bar_title, dimension_string);
	strcat(bar_title, colon);
	strcat(bar_title, Pchar);
	strcat(bar_title, right_brace);
	
	fCreatingBars = TRUE;								//  Remember that we're creating bars now
	
	long vert = 5;
	SetupScrollBar(vert, bar_title, &perspective_bar, 	//  Set up the perspective scroll bar
					0,
					0, 10, (dimension <= 4) ? 7 : 3,	
					TRUE);

	long i;												//  Set up the angle scroll bars
	for (i = 1; i <= dimension-1; i++)					
		{
		vert += 20;										//  Go down to next scroll bar
		
		char angle_num_string[10];
		NumToString(i,
					(unsigned char*) angle_num_string);	//  Convert angle number to a string
		PtoCstr((unsigned char *) angle_num_string);
		
		strcpy(bar_title, left_brace);					//  Create the angle bar title
		strcat(bar_title, dimension_string);
		strcat(bar_title, colon);
		strcat(bar_title, angle_num_string);
		strcat(bar_title, right_brace);

		short value = 0;
		if (dimension == 3)
			value = 150;
		
		CHyperScrollBar	*angle_bar = new(CHyperScrollBar);
		SetupScrollBar(vert, bar_title, angle_bar, 		//  Set up the angle scroll bar
						i,
						0, 360, value,
						FALSE);
		
		angle_bars.Append(angle_bar);					//  Add this bar to the list
		
		}

	fCreatingBars = FALSE;								//  We're creating the bars anymore

}	//==== CControlsDirector::BuildWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::SetupScrollBar
//|
//| Purpose: This procedure adds a scroll bar and its title to the controls pane
//|
//| Parameters: voffset:       the vertical offset of the scroll bar from the top
//|             title:         the title to put next to the scroll bar
//|             bar:           the scroll bar to create and initialize
//|             angle_num:     angle number of the angle controlled by this bar
//|             min,max,value: minimum, maximum, and starting value of scroll bar
//|             fPerspective:  TRUE if this is a perspective bar
//|_______________________________________________________________________________

void CControlsDirector::SetupScrollBar(short voffset, char *title, CHyperScrollBar *bar,
										long angle_num,
										short min, short max, short value,
										Boolean fPerspective)
{

	LongRect interior_rect;								//  Get the width of the window
	itsWindow->GetInterior(&interior_rect);
	long window_width = interior_rect.right -
						interior_rect.left;

	CEditText *text = new(CEditText);
	text->IEditText(itsWindow, this,					//  Set up the scroll bar text
					47, 24, 0, voffset,
					sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextPtr(title, strlen(title));
	text->SetAlignCmd(cmdAlignRight);
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);

	char num_string[10];
	text = new(CEditText);								//  Set up the lower bound text
	sprintf(num_string, "%d", min);
	text->IEditText(itsWindow, this, 30, 20,
					68, voffset + 2,
					sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextPtr(num_string, strlen(num_string));
	text->SetAlignCmd(cmdAlignRight);
	text->SetFontNumber(geneva);
	text->SetFontSize(9);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	
	text = new(CEditText);								//  Set up the upper bound text
	sprintf(num_string, "%d", max);
	text->IEditText(itsWindow, this, 25, 15,
					window_width - 25, voffset + 2,
					sizFIXEDRIGHT, sizFIXEDTOP, -1);
	text->SetTextPtr(num_string, strlen(num_string));
	text->SetAlignCmd(cmdAlignLeft);
	text->SetFontNumber(geneva);
	text->SetFontSize(9);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	
	bar->IHyperScrollBar(itsWindow, this,			//  Set up the scroll bar
						HORIZONTAL, window_width - 127,
						100, voffset, !fPerspective);
	bar->hSizing = sizELASTIC;
	bar->vSizing = sizFIXEDTOP;
	bar->SetMinValue(0);
	bar->SetMaxValue(max - min);
	bar->SetValue(value - min);
	bar->SetActionProc(scroll_action_proc);
	bar->SetThumbFunc(scroll_thumb_proc);
	bar->helpResIndex = fPerspective ?
			kPerspectiveScrollBar : kAngleScrollBar;	//  Link to Balloon Help

	RefStruct **ref =
			(RefStruct **) NewHandle(sizeof(RefStruct));//  Create a new references structure
	
	(*ref)->bar = bar;									//  Remember the CHyperScrollBar
	(*ref)->dimension = dimension;						//  Remember the dimension of this bar
	(*ref)->angle_num = angle_num;						//  Remember the angle number of this bar
	
	SetCRefCon(bar->macControl, (long) ref);			//  Remember the CHyperScrollBar handle

	CScrollNumPane *num_pane = new(CScrollNumPane);		//  Create a number pane to show the 
	num_pane->IScrollNumPane(itsWindow, this,			//    scroll bar's setting
								28, 20,
								50, voffset + 2,
								sizFIXEDLEFT, sizFIXEDTOP,
								-1, bar, min);
	num_pane->SetAlignCmd(cmdAlignCenter);
	
}	//==== CControlsDirector::SetupScrollBar() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::ShowWindow
//|
//| Purpose: Show one of the controls windows
//|
//| Parameters: dimension: dimension controlled by the window to show
//|___________________________________________________________________________

void CControlsDirector::ShowWindow(void)
{

	itsWindow->Show();

}	//==== CControlsDirector::ShowWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::HideWindow
//|
//| Purpose: Hide one of the controls windows
//|
//| Parameters: dimension: dimension controlled by the window to hide
//|___________________________________________________________________________

void CControlsDirector::HideWindow(void)
{

	itsWindow->Hide();

}	//==== CControlsDirector::HideWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: scroll_action_proc
//|
//| Purpose: This procedure is called whenever a scroll bar is clicked anywhere
//|          but on the thumb.
//|
//| Parameters: control: the scroll bar which was moved
//|             part:    where the bar was clicked
//|___________________________________________________________________________

pascal void	scroll_action_proc(ControlHandle control, short part)
{

	static short increment;
	
	RefStruct **ref =
				(RefStruct **) GetCRefCon(control);	//  Get the reference structure
	
	CHyperScrollBar *bar = (*ref)->bar;				//  Get the CHyperScrollBar handle
	
	switch(part)									//  Find the distance moved
		{
		case inUpButton: increment = -1; break;
		case inDownButton: increment = 1; break;
		case inPageUp: increment = -10; break;
		case inPageDown: increment = 10; break;
		}

	short value = bar->GetValue();					//  Change the scroll bar's value.  Note that
	value += increment;								//    this also sends a controlValueChanged
	bar->SetValue(value);							//    message to the controls window director.
			
}	//==== scroll_action_proc() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure: scroll_thumb_proc
//|
//| Purpose: This procedure is called whenever a scroll bar thumb is moved
//|
//| Parameters: bar:   the scroll bar which was moved
//|             delta: amount the thumb was moved
//|___________________________________________________________________________

void	scroll_thumb_proc(CScrollBar *bar, short delta)
{

	short value = bar->GetValue();	//  Change the scroll bar's value.  Note that this also sends					
	bar->SetValue(value);			//  a controlValueChanged message to the graphics pane

}	//==== scroll_thumb_proc() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::OffsetScrollBar
//|
//| Purpose: Offset the value of a scroll bar by some increment
//|
//| Parameters: angle:  the angle corresponding to the scroll bar to offset
//|             offset: how much to offset the scroll bar
//|__________________________________________________________________

void CControlsDirector::OffsetScrollBar(long angle, long offset)
{

	CHyperScrollBar *bar = (angle == 0) ? &perspective_bar :
									angle_bars.NthItem(angle);	//  Get the scroll bar
	
	bar->SetValue(bar->GetValue() + offset);					//  Offset the value

}	//==== CControlsDirector::OffsetScrollBar() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsDirector::ProviderChanged
//|
//| Purpose: This is called every time a scroll bar changes value
//|
//| Parameters: provider: the control which changed
//|             reason:   the reason we were called (controlValueChanged)
//|             info:     unused
//|__________________________________________________________________

void CControlsDirector::ProviderChanged(CCollaborator *provider, long reason, void *info)
{

	if ((reason == controlValueChanged) && (!fCreatingBars))
		{
		CHyperScrollBar *bar = (CHyperScrollBar *) provider;	//  Get the scroll bar handle
	
		RefStruct **ref;
		ref = (RefStruct **) GetCRefCon(bar->macControl);		//  Get the reference structure

		long angle_num = (*ref)->angle_num;						//  Find the angle number
	
		short bar_value = bar->GetValue();						//  Get the angle from the bar

		if (angle_num == 0)
			graphic_pane->graphic->
					ChangePerspective(dimension, bar_value);	//  Change the amount of perspect.
		
		else
			graphic_pane->graphic->ChangeAngle(dimension,
								angle_num, bar_value*Pi/180);	//  Change the angle
		
		if (!drawing_disabled)	
			graphic_pane->Draw((Rect *) NULL);					//  Redraw the graphic
		
		}
		
}	//==== CControlsDirector::ProviderChanged() ====\\



