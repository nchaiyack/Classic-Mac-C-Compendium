/******************************************************************************
 CShellDialog.h

		
	SUPERCLASS = CDLOGDirector
	
	Copyright � 1993 Johns Hopkins University. All rights reserved.
	
 ******************************************************************************/

#pragma once

#include "CDLOGDirector.h"

CLASS CMovieController;

class CShellDialog : public CDLOGDirector
{

public:

	CMovieController		*itsMC;
	
	void IShellDialog( short ShellDialogID);
	
	virtual void DoCommand( long aCmd);
	virtual void ProviderChanged( CCollaborator *aProvider, long reason, void *info);
	
	virtual void Dispose( void);

};