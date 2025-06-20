/******************************************************************************
 CProjectInfoDialog.h

		Interface for CProjectInfoDialog class.
		
	SUPERCLASS = CDLOGDirector
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CProjectInfoDialog

#include "CDLOGDirector.h"

class CProjectInfoDialog : public CDLOGDirector
{
public:

	void IProjectInfoDialog( int theID, CDirectorOwner *aSupervisor);
	
	virtual void DoCommand( long);
	
};
