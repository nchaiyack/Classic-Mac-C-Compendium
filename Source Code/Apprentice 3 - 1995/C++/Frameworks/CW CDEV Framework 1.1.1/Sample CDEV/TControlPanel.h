#pragma once

#ifndef	_H_TControlPanel
#define	_H_TControlPanel

// Description:	A C++ class for writing control panel's (CDEV's)
// 				in Metrowerks CodeWarrior.
// Author: 		Matthew E. Axsom
// Version: 	1.1.1 - 05/16/95
// Copyright:	©1994-95, Matthew E. Axsom, All Rights Reserved

// internal error code for cdevMemErr.  Use instead of cdevMemErr since cdevMemErr
// is 0 and framework uses a return code of zero to indicate success.
#define	cdevFWMemErr	memFullErr

class TControlPanel {
public:
	short		fLastItem;				// last system item in control panel.

	DialogPtr	fDialog;				// control panel dialog box
	EventRecord	*fEvent;				// event
	
			TControlPanel(short numItems,DialogPtr cp);
	virtual	~TControlPanel(void);
	
	//  processes all events 'cept macDev,initDev,closeDev
	long actions(short message, short itemHit);	
	long CommandKey(short);				// handles command key's for non sys7 machines
	
	// you'll need to override these methods w/your own.
	
	// these 2 are provided to ease porting from Symantec's CDEV framework.
	// Initilization and clean up should be done from constructor/destructor
	virtual long Init(void);			// called as a result of a initDev message
	virtual long Close(void);			// called as a result of a closeDev message
	
	virtual long ItemHit(short itemHit);	// handles a mouse hit in the control panel
	virtual long Idle(void);			// handling of nulDev message
	virtual long Update(void);			// update user items
	virtual long Activate(void);		// activate user items
	virtual long Deactivate(void);		// deactivate user items
	virtual long KeyDown(short c);		// key down was pressed
	virtual long Undo(void);			// undo from edit menu or cmd-z
	virtual long Cut(void);				// cut from edit menu or cmd-x
	virtual long Copy(void);			// copy from edit menu or cmd-c
	virtual long Paste(void);			// paste from edit menu or cmd-v
	virtual long Clear(void);			// clear from edit menu
};

#endif	//_H_TControlPanel