/*
	Paths.h
	
	Paths routines for Graphic Elements
	
	Copyright 1993 by Al Evans
	
	11/10/93
	
*/

//Load precompiled headers if in MPW
#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

/*
	Another quick hack -- showing how frames and positions of Graphic Elements
	might be changed by a path interpreter.
*/

/*
	A simple path system for programmed animation. A PathRec contains a
	pointer to a sequence of PathEntries and fields to hold the results
	of processing those PathEntries.
	
	The FrameSeqGraphic which uses paths calls InitPath to initialize
	the PathRec, then calls GetNextStep during its AutoChangeProc update
	its PathRec. On return from GetNextStep, it changes frames if 
	PathRec.currFrame is non-zero and moves if PathRec.currXMove or
	PathRec.currYMove is non-zero.
	
	Each PathEntry is one step in the path. Each PathEntry has a command,
	a parameter, and x value, and a y value. Their effects on the fields
	of the PathRec depend on the command:
		
		absMotionCmd: 
				PathRec.currFrame = param;
				PathRec.currXMove = xVal;
				PathRec.currYMove = yVal;
		
		relMotionCmd:
				PathRec.currFrame = param;
				PathRec.currXMove += xVal;
				PathRec.currYMove += yVal;
				
	Paths can also contain control commands:
	
		gotoCmd:
				Reset the current step in the path to (param) and
				process the PathEntry found there.
		resetCmd:
				Reset the current step in the path to zero. Must
				be included as the last step in each path.
		repeatCmd:
				Only meaningful before a relMotionCmd. Causes the
				next command to be repeated (param) times. If this
				command includes a frame change, the frame is changed
				only on the first execution.
*/

typedef struct {
	signed char		command;
	unsigned char	param;
	signed char		xVal;
	signed char		yVal;
}	PathEntry, *PathEntryPtr;

typedef struct {
	short			currStep;
	short			currFrame;
	short			currXMove;
	short			currYMove;
	short			count;
	short			sp;
	short			stack[16];
	PathEntryPtr	path;
} PathRec, *PathRecPtr;

typedef enum {
		absMotionCmd	= 	(signed char) 0x20,
		relMotionCmd	=	(signed char) 0x21,
		//commands < 0 manipulate path pointer
		repeatCmd		=	(signed char) 0xD1,
		goToCmd			=	(signed char) 0xE0,
		goSubCmd		=	(signed char) 0xE1,
		returnCmd		=	(signed char) 0xE2,
		resetCmd		=	(signed char) 0xFF
} PathCommand;
	
#ifdef __cplusplus
extern "C" {
#endif

void InitPath(PathRecPtr path);

void GetNextStep(PathRecPtr path);

void DoPathGoTo(PathRecPtr path, short gotoStep);

void DoPathGoSub(PathRecPtr path, short subRtnStep);

#ifdef __cplusplus
}
#endif
