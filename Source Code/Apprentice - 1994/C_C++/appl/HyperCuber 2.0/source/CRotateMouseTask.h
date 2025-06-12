//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interfact to the CRotateMouseTask class.
//|_________________________________________________________________
#pragma once

#include "Mouse.h"
#include <CMouseTask.h>

CLASS CHyperCuberPane;

class CRotateMouseTask : CMouseTask
	{
	
	mouse_task_struct *h_tasks;		//  Horizontal tasks
	mouse_task_struct *v_tasks;		//  Vertical tasks

	short num_h, num_v;				//  Number of horizontal and vertical tasks
		
	CHyperCuberPane		*pane;		//  The pane containing graphic we're rotating
	
  public:
  
	void	IRotateMouseTask(CHyperCuberPane *the_pane, mouse_task_struct *h_mouse_tasks,
										mouse_task_struct *v_mouse_tasks,
										short num_h_tasks, short num_v_tasks);
	void	KeepTracking(LongPt *start_point, LongPt *previous_point, LongPt *current_point);
	void	EndTracking(LongPt *start_point, LongPt *previous_point, LongPt *current_point);

	};