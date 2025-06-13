/******************************************************************************
 COptionsDialog.h

		Interface for COptionsDialog class.
		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_COptionsDialog

#include "CDLOGDirector.h"

class COptionsDialog : public CDLOGDirector
{
public:

	void IOptionsDialog( int theID, CDirectorOwner *aSupervisor);
	
	virtual void DoCommand( long);
	
};