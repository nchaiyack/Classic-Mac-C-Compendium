/*
 *	File:		CWASTEStyleTask.h
 *	System:		Mark's Class Library
 *	Evironment:	MacOS 7.0/THINK C 6.0/TCL 1.1.3
 *	Author:		Mark Alldritt
 *
 *
 *		Copyright © 1994    All Rights Reserved
 *		Mark Alldritt
 *		1571 Deep Cove Road
 *		N. Vancouver, B.C. CANADA
 *
 *	
 *	Notes:
 *
 *	This source file depends on the "Scriptable TCL+MacTraps" pre-compiled header for all TCL
 *	definitions.
 *
 *
 *	Edit History:
 *
 *	V1-0.0	Mark Alldritt	10-May-1994
 *	- Initial version of module.
 *
 */

#pragma once

#include "CTextStyleTask.h"

class CWASTEText;

class CWASTEStyleTask : public CTextStyleTask
{
public:

				 CWASTEStyleTask( CWASTEText *aTextPane, long aStyleCmd, short firstTaskIndex);
				 ~CWASTEStyleTask(void);
				 
				 
	virtual void Do(void);    //mf to allow cmdJustify
	virtual void Undo( void);	

protected:

	StScrpHandle	oldStyles;
	long			selStart, selEnd;
	
	virtual void SaveStyle( void);
	virtual void RestoreStyle( void);
	
};