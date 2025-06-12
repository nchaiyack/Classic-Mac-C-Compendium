//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the implementation of the RotateMouseTask class.
//|_____________________________________________________________________

#include "CControlsDirector.h"
#include "CRotateMouseTask.h"
#include "CHyperCuberPane.h"

//========================== Globals =========================\\

extern Boolean drawing_disabled;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CRotateMouseTask::IRotateMouseTask
//|
//| Purpose: Initialize the mouse task.
//|
//| Parameters: h_mouse_tasks: the horizontal mouse controls
//|             v_mouse_tasks: the vertical mouse controls
//|             num_h_tasks:   number of horizontal controls
//|             num_v_tasks:   number of vertical controls
//|_______________________________________________________________________________

void CRotateMouseTask::IRotateMouseTask(CHyperCuberPane *the_pane, mouse_task_struct *h_mouse_tasks,
										mouse_task_struct *v_mouse_tasks,
										short num_h_tasks, short num_v_tasks)
{

	CMouseTask::IMouseTask(0);				//  Initialize superclass

	pane = the_pane;

	h_tasks = h_mouse_tasks;
	v_tasks = v_mouse_tasks;
	num_h = num_h_tasks;
	num_v = num_v_tasks;

}	//==== CRotateMouseTask::IRotateMouseTask() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CRotateMouseTask::KeepTracking
//|
//| Purpose: This gets called regularly as long as the mouse button is down.
//|
//| Parameters: start_point:    point where button went down
//|             previous_point: last registered location of cursor
//|             current_point:  current location of cursor
//|__________________________________________________________________________

void CRotateMouseTask::KeepTracking(LongPt *current_point, LongPt *previous_point,
										LongPt *start_point)
{

	long horiz_delta = current_point->h -
						previous_point->h;				//  Find mouse movement since last time
	long vert_delta = current_point->v -
						previous_point->v;

	drawing_disabled = TRUE;							//  Don't draw until all changes made

	long i;
	for (i = 0; i < num_h; i++)
		h_tasks[i].controls_director->
				OffsetScrollBar(h_tasks[i].angle,
					horiz_delta*h_tasks[i].multiplier);	//  Nudge the scroll bar

	for (i = 0; i < num_v; i++)
		v_tasks[i].controls_director->
				OffsetScrollBar(v_tasks[i].angle,
					vert_delta*v_tasks[i].multiplier);	//  Nudge the scroll bar

	drawing_disabled = FALSE;							//  Redraw the object
	pane->Prepare();
	pane->Draw((Rect *) NULL);


}	//==== CRotateMouseTask::KeepTracking() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CRotateMouseTask::EndTracking
//|
//| Purpose: This gets called when the mouse button is released.
//|
//| Parameters: none
//|______________________________________________________________

void CRotateMouseTask::EndTracking(LongPt *current_point, LongPt *previous_point,
										LongPt *start_point)
{

	KeepTracking(current_point, previous_point, start_point);

}	//==== CRotateMouseTask::EndTracking() ====\\



