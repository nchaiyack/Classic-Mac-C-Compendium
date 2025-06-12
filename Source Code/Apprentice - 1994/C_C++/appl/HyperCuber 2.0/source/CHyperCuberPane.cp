//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CHyperCuberPane.c
//
// This file contains the implementation of the pane for the graphic window.
//___________________________________________________________________________

#include "CControlsDirector.h"
#include "CGraphic.h"
#include "CHyperCuberApp.h"
#include "CHyperCuberDoc.h"
#include "CHyperCuberPane.h"
#include "CHyperCuberPrefs.h"
#include "CHyperScrollBar.h"
#include "CRotateMouseTask.h"
#include "HyperCuber Commands.h"
#include "HyperCuber Messages.h"

#include <LongQD.h>


//============================ Constants ============================\\

#define	Pi	3.14159265358979323846


//============================ Globals ============================\\

Boolean				drawing_disabled = FALSE;	//  TRUE if drawing is disabled

extern CHyperCuberPrefs	*gPrefs;

extern short		menubar_height;				//  The height of the menubar (if any)


//============================ Prototypes ============================\\

extern void create_offscreen_pixmap(CGrafPort *color_port, Rect *bounds);
extern void adjust_offscreen_pixmap(CGrafPort *color_port, Rect *bounds);
extern void dispose_offscreen_pixmap(CGrafPort *color_port);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPane::IHyperCuberPane
//|
//| Purpose: Initialize a HyperCuberPane
//|
//| Parameters: same as Pane
//|______________________________________________________________

void CHyperCuberPane::IHyperCuberPane(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing)
{

//	char	axes_3D_string[] = "0,0,1.3 0,0,-1.3 0,1.3,0 0,-1.3,0 1.3,0,0 -1.3,0,0\n1#1 2#1\n3#1 4#1\n5#1 6#1";
//	char	axes_4D_string[] = "1.3,0,0,0 -1.3,0,0,0 0,1.3,0,0 0,-1.3,0,0 0,0,1.3,0 0,0,-1.3,0 0,0,0,1.3 0,0,0,-1.3\n1#1 2#1\n3#1 4#1\n5#1 6#1\n7#1 8#1";

	CPane::IPane(anEnclosure, aSupervisor, aWidth, aHeight,			//  Initialize as a Pane
							aHEncl, aVEncl, aHSizing, aVSizing);

	Rect port_bounds = {0, 0, aHeight, aWidth};						//  Create the offscreen port
	create_offscreen_pixmap(&OffscreenPort, &port_bounds);
	
	StereoMode = mono;									//  Don't draw in stereo
	fAddOver = FALSE;									//  Don't use addOver mode
	fAntialias = FALSE;									//  Don't antialias
	fUseNativeColors = TRUE;							//  Use the object's native colors
	fDrawOffscreen = TRUE;								//  Draw offscreen

}	//==== CHyperCuberPane::IHyperCuberPane() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPane::Dispose
//|
//| Purpose: Dispose of a CHyperCuberPane
//|
//| Parameters: none
//|_________________________________________________________

void CHyperCuberPane::Dispose()
{

	dispose_offscreen_pixmap(&OffscreenPort);	//  Dispose of the associated offscreen port

	inherited::Dispose();						//  Dispose of the pane

}	//==== CHyperCuberPane::Dispose() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPane::Draw
//|
//| Purpose: Refresh the graphics pane
//|
//| Parameters: area: the part of the pane to draw
//|______________________________________________________________

void CHyperCuberPane::Draw(Rect *area)
{

	RGBColor rgb_black = {0, 0, 0};

	if (fAntialias)
		graphic->ClearDrawingArea(&rgb_black);	//  Background must be black for antialiasing

	else
		graphic->ClearDrawingArea(
			&gPrefs->prefs.background_color);	//  No antialiasing; use selected background color

	graphic->Draw();							//  Draw the object

}	//==== CHyperCuberPane::Draw() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPane::AdjustToEnclosure
//|
//| Purpose: This is called when the window is resized.  It keeps  the
//|          graphics panes square.
//|
//| Parameters: passed on to superclass
//|______________________________________________________________________

void	CHyperCuberPane::AdjustToEnclosure(Rect *deltaEncl)
{

	drawing_disabled = FALSE;					//  Assume that this adjustment will make the
												//    panes small enough to fit in memory

	CPane::AdjustToEnclosure(deltaEncl);		//  Adjust this and all subpanes

	FitToEnclosure(TRUE, TRUE);					//  Fill the enclosure
	Refresh();									//  Force this window to be redrawn (eventually)
	
	CHyperCuberDoc *doc =
			(CHyperCuberDoc *) itsSupervisor;	//  Get the document

	short aspect_ratio =						//  Use 2:1 aspect ratio for two-image stereo, 1:1 for normal
		(StereoMode == two_image_stereo) ? 2 : 1;

	short	width, height;
	GetLengths(&width, &height);				//  Find the new size of the pane
	
	Rect change_rect = {0, 0, 0, 0};			//  Make pane fit the aspect ratio
	short size;
	if (width > aspect_ratio * height)
		{
		size = aspect_ratio * height;
		change_rect.right = size - width;
		}
	else
		{
		size = width;
		change_rect.bottom = size / aspect_ratio - height;
		}
		
	ChangeSize(&change_rect, FALSE);
	CenterWithinEnclosure(TRUE, TRUE);	

	Rect bounds;
	LongToQDRect(&frame, &bounds);				//  Find size of pane
	
	adjust_offscreen_pixmap(&OffscreenPort,		//  Make offscreen bitmap large enough
								&bounds);
	graphic->FitToPane();							//  Remap the graphic to the new pane size

}	//==== CHyperCuberPane::AdjustToEnclosure() ====\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Procedure CHyperCuberPane::UpdateGraphicsPanes
//
// Purpose: Recompute and redraw the graphics pane
//
// Parameters: main_pane: the main pane
//_________________________________________________________

void CHyperCuberPane::UpdateGraphicsPanes(void)
{

	LongRect graphics_rect;
	Prepare();								//  Erase the pane
	GetFrame(&graphics_rect);
	LEraseRect(&graphics_rect);					

	Rect no_movement = {0, 0, 0, 0};
	AdjustToEnclosure(&no_movement);		//  Recalc positions of graphics panes

	Refresh();								//  Redraw the object

}	//==== CHyperCuberPane::UpdateGraphicsPanes() ====\\


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberPane::DoClick
//|
//| Purpose: This is called when the user clicks in the pane
//|
//| Parameters: hit_point: the point where the click occurred
//|             modifiers: modifiers from mouseDown event
//|             when:      when the click occurred
//|______________________________________________________________

void CHyperCuberPane::DoClick(Point hit_point, short modifiers, long when)
{

	Point global_point = hit_point;
	LocalToGlobal(&global_point);						//  Find click's global coordinates
	if (global_point.v < menubar_height)
		{
		DoCommand(cmdToggleMenuBar);					//  If the click was in the (hidden) menubar,
		return;											//    show the menubar.
		}

	mouse_task_struct	h_mouse_tasks[MAX_MOUSE_CONTROLS];
	mouse_task_struct	v_mouse_tasks[MAX_MOUSE_CONTROLS];
	short num_h_tasks = 0;
	short num_v_tasks = 0;

	CHyperCuberDoc *doc = (CHyperCuberDoc *) itsSupervisor;	//  Get the document

	long graphic_dimension = doc->graphic->dimension;		//  Find dimension of graphic

	long i;
	for (i = 0; i < gPrefs->prefs.num_mouse_controls; i++)	//  Loop through all mouse controls
		{
		mouse_control_struct mouse_control =
					gPrefs->prefs.mouse_controls[i];		//  Get point to mouse controls
		if ((mouse_control.modifiers == modifiers) &&
			(mouse_control.dimension <= graphic_dimension))	//  Use this task if the modifier keys
															//   are right and the dimension is
															//   no more than the graphic dimension
			{
			
			mouse_task_struct mouse_task;
			
			mouse_task.controls_director =
				(CControlsDirector *)
					doc->controls_directors->
						NthItem(mouse_control.dimension);	//  Find controls for this dimension
			
			mouse_task.angle = mouse_control.angle;			//  Get angle to change
			mouse_task.multiplier =
								mouse_control.multiplier;	//  Get multiplier
			
			if (mouse_control.horiz)						
				h_mouse_tasks[num_h_tasks++] = mouse_task;	//  Add task to horizontal tasks array
			
			else
				v_mouse_tasks[num_v_tasks++] = mouse_task;	//  Add task to vertical tasks array
			}
		}

	CRotateMouseTask *mouse_task = new (CRotateMouseTask);
	mouse_task->IRotateMouseTask(this, h_mouse_tasks,
				v_mouse_tasks, num_h_tasks, num_v_tasks);	//  Initialize mouse task

	LongRect pin_rect = {-10000, -10000, 10000, 10000};
	LongPt long_hit_point;
	QDToLongPt(hit_point, &long_hit_point);	
	TrackMouse(mouse_task, &long_hit_point, &pin_rect);

}	//==== CHyperCuberPane::DoClick() ====\\

