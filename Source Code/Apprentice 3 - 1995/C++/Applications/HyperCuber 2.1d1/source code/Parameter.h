//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Parameter.h
//|
//| This contains header information relating to the use
//| of parameters in HyperCuber.
//|_________________________________________________________

#pragma once

typedef struct
	{
	char	name[10];				//  Name of this parameter
	double	start;					//  Starting value of this parameter
	double	end;					//  Ending value of this parameter
	double	step;					//  Step size to use when varying parameter from min to max
	} Parameter;
		
