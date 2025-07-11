/******************************************************************************
 CDemoDialog.cp

		
	SUPERCLASS = CDLOGDirector
	
	Copyright � 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/5/94					by:		mrw			TCL Version:	1.1.3

 ******************************************************************************/

#pragma once

#include "CDLOGDirector.h"

CLASS CIconPane;
CLASS CHyperText;

class CDemoDialog : public CDLOGDirector{

public:
	
	CHyperText	*itsHyperText;
	CIconPane	*itsASIcon;
	CIconPane	*itsPaintIcon;
	CIconPane	*itsCommIcon;
	CIconPane	*itsMailIcon;
	
	void IDemoDialog( void);
	
	virtual void DoCommand( long aCmd);
	virtual void ProviderChanged( CCollaborator *aProvider, long reason, void *info);
	
	virtual void Dispose( void);

};