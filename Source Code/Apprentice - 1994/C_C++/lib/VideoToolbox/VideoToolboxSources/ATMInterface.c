/*
 * ATMInterface.c
 *
 * Version 1.01
 *
 * Adobe Type Manager is a trademark 
 * of Adobe Systems Incorporated.
 */
#include "ATMInterface.h"

static int16 open;
static ATMProcs3 procs;

int16 initATM()
	{
	CntrlParam c;
	
	if (OpenDriver("\p.ATM",&c.ioCRefNum))
		return 0;

	c.csCode = ATMProcsStatusCode;
	*(ATMProcs3 **) c.csParam = &procs;
	procs.version = ATMProcs3Version;

	if (PBStatus((ParmBlkPtr)&c,0))
		return 0;

	return open = 1;
	}

int16 fontAvailableATM(family,style)
	int16 family;
	int16 style;
	{
	return open ? (*procs.fontAvailable)(family,style) : 0;
	}

int16 showTextATM(text,length,matrix)
	char *text;
	int16 length;
	FixedMatrix *matrix;
	{
	return open ? (*procs.showText)(text,length,matrix) : length;
	}

int16 xyshowTextATM(text,length,matrix,displacements)
	char *text;
	int16 length;
	FixedMatrix *matrix;
	Fixed *displacements;
	{
	return open ? (*procs.xyshowText)(text,length,matrix,displacements) : length;
	}
