//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow.cp
//|
//| This file contains the implementation of a controls window.
//|___________________________________________________________________________

#include "CControlsWindow.h"
#include "CHyperCuberPrefs.h"



//=============================== Globals ===============================\\

extern CHyperCuberPrefs *gPrefs;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow::IControlsWindow
//|
//| Purpose: Initialize the window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CControlsWindow::IControlsWindow(short WINDid, Boolean aFloating,
						CDesktop *anEnclosure, CDirector *aSupervisor, short dim)
{

	IEnhancedWindow(WINDid, aFloating, anEnclosure, aSupervisor);
	
	dimension = dim;
	title_bar_height = 10;

}	//==== CControlsWindow::IControlsWindow() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow::Move
//|
//| Purpose: Move the window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CControlsWindow::Move(short h, short v)
{
	
	inherited::Move(h, v);
	
	GetRect(&gPrefs->prefs.controls_window_position[dimension]);	//  Save new window position

}	//==== CControlsWindow::Move() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow::Drag
//|
//| Purpose: Drag the window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CControlsWindow::Drag(EventRecord *macEvent)
{
	
	inherited::Drag(macEvent);

	GetRect(&gPrefs->prefs.controls_window_position[dimension]);	//  Save new window position
	
}	//==== CControlsWindow::Move() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow::HideFloat
//|
//| Purpose: Hide the floating window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CControlsWindow::HideFloat(void)
{

//	GetRect(&gPrefs->prefs.controls_window_position[dimension]);//  Save new window position
//	gPrefs->prefs.controls_window_visible[dimension] = FALSE;	//  Window is no longer visible
	
	inherited::HideFloat();										//  Hide the window	

}	//==== CControlsWindow::HideFloat() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow::ShowFloat
//|
//| Purpose: Show the floating window
//|
//| Parameters: passed to superclass
//|______________________________________________________________

void CControlsWindow::ShowFloat(void)
{

//	Rect window_rect =
//			gPrefs->prefs.controls_window_position[dimension];	//  Get correct position

//	hiding.h = window_rect.left;								//  Show window at correct place
//	hiding.v = window_rect.top;

	inherited::ShowFloat();										//  Show the window

//	Place(&window_rect);										//  Move to correct position

//	gPrefs->prefs.controls_window_visible[dimension] = TRUE;	//  Window is now visible

}	//==== CControlsWindow::ShowFloat() ====\\
