/*--------------------------------------------------------------------------------------------
Module Name:	CDEV.h
Author:			Matthew E. Axsom
Description:	Header file for CDEV.cp.  C++ object for Simple Sample w/Stub cdev
Copyright:		(c) Copyright 1995, Matthew E. Axsom
				All rights reserved.

Date		Who	Ver.	Description
--------	---	-----	---------------------------------------------------
03/08/95	MEA	1.00 	Created
--------------------------------------------------------------------------------------------*/
#pragma once

#include "TControlPanel.h"

class myCDEV : TControlPanel {
public:
			myCDEV(short numItems,DialogPtr cp);
	virtual	~myCDEV(void);
	
	long ItemHit(short itemHit);	// handles a mouse hit in the control panel
	long Idle(void);			// handling of nulDev message
	long Update(void);			// update user items
	long Activate(void);		// activate user items
	long Deactivate(void);		// deactivate user items
	long Undo(void);			// undo from edit menu or cmd-z
	
private:
	Boolean			activated,
					showSeconds;
	unsigned long	lastTime;
	Str255			timeString;
	
	Boolean getTime(void);
	void setCheckBox(void);
};
