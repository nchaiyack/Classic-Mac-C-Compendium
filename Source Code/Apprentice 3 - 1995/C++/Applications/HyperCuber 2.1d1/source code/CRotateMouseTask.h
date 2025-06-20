//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interfact to the CRotateMouseTask class.
//|_________________________________________________________________
#pragma once

#include "Mouse.h"
#include <CMouseTask.h>

class CHyperCuberPane;

class CRotateMouseTask : CMouseTask
	{
	
	MouseTaskStruct *h_tasks;		//  Horizontal tasks
	MouseTaskStruct *v_tasks;		//  Vertical tasks

	short num_h, num_v;				//  Number of horizontal and vertical tasks
		
	CHyperCuberPane		*pane;		//  The pane containing graphic we're rotating
	
  public:
  
	void	IRotateMouseTask(CHyperCuberPane *the_pane, MouseTaskStruct *h_mouse_tasks,
										MouseTaskStruct *v_mouse_tasks,
										short num_h_tasks, short num_v_tasks);
	void	KeepTracking(LongPt *start_point, LongPt *previous_point, LongPt *current_point);
	void	EndTracking(LongPt *start_point, LongPt *previous_point, LongPt *current_point);

	};