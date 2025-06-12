/*
	Paths.c
	
	Paths routines for Graphic Elements
	
	Copyright 1993 by Al Evans
	
	11/10/93
	
*/

#include "Paths.h"

void InitPath(PathRecPtr path)
{
	path->currStep = 0;
	path->currFrame = 0;
	path->currXMove = 0;
	path->currYMove = 0;
	path->count = 0;
	path->sp = 0;
}

void GetNextStep(PathRecPtr pathRec)
{
	PathEntryPtr	thisStep;
	
	thisStep = pathRec->path + pathRec->currStep;
	if (pathRec->count > 0) {
		pathRec->currXMove += thisStep->xVal;
		pathRec->currYMove += thisStep->yVal;
		pathRec->currFrame = 0;			//Any frame change has already been done
		pathRec->count--;
		return;
	}
	//Do path control command(s)
	while (thisStep->command < 0) {
		switch (thisStep->command) {
			case repeatCmd:
				pathRec->count = thisStep->param;
				pathRec->currStep++;
				break;
			case goToCmd:
				pathRec->currStep = thisStep->param;
				break;
			case goSubCmd:
				pathRec->stack[pathRec->sp++] = pathRec->currStep;
				pathRec->currStep = thisStep->param;
				break;
			case returnCmd:
				pathRec->currStep = pathRec->stack[--pathRec->sp] + 1;
				break;
			case resetCmd:
				pathRec->currStep = 0;
				break;
		}
		thisStep = pathRec->path + pathRec->currStep;
	}
	switch (thisStep->command) {
		case absMotionCmd:
			pathRec->currXMove = thisStep->xVal;
			pathRec->currYMove = thisStep->yVal;
			break;
		case relMotionCmd:
			pathRec->currXMove += thisStep->xVal;
			pathRec->currYMove += thisStep->yVal;
			break;
	}
	pathRec->currFrame = thisStep->param;
	if (pathRec->count)
		pathRec->count--;
	else
		pathRec->currStep++;
}

void DoPathGoTo(PathRecPtr path, short gotoStep)
{
	path->currStep = gotoStep;
}

void DoPathGoSub(PathRecPtr path, short subRtnStep)
{
	path->stack[path->sp++] = path->currStep;
	path->currStep = subRtnStep;
}
