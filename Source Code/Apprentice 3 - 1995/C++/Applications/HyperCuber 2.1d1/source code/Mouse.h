//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Mouses.h
//|
//| This contains header information relating to the use
//| of mouse controls in HyperCuber.
//|_________________________________________________________

#pragma once

class CControlsDirector;

typedef struct
	{
	char	dimension;
	char	angle;
	Boolean	horiz;
	char	multiplier;
	short	modifiers;
	} MouseControl;

typedef struct
	{
	CControlsDirector	*controls_director;
	long				angle;
	char				multiplier;
	} MouseTaskStruct;
