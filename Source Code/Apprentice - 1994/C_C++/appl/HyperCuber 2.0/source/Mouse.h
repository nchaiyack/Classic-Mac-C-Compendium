//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Mouses.h
//|
//| This contains header information relating to the use
//| of mouse controls in HyperCuber.
//|_________________________________________________________

#pragma once

CLASS CControlsDirector;

typedef struct
	{
	char	dimension;
	char	angle;
	Boolean	horiz;
	char	multiplier;
	short	modifiers;
	} mouse_control_struct;

typedef struct
	{
	CControlsDirector	*controls_director;
	long				angle;
	char				multiplier;
	} mouse_task_struct;

