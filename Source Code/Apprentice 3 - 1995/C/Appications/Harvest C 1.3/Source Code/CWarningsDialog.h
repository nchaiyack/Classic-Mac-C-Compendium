/******************************************************************************
 CWarningsDialog.h

		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#define _H_CWarningsDialog

#include "CDLOGDirector.h"
#include "CWarningsPane.h"
#include "CWarningsArray.h"
#include "CCheckBox.h"
#include "CRadioControl.h"
#include "CRadioGroupPane.h"


class CWarningsDialog : public CDLOGDirector
{

public:

	CWarningsArray		*itsWarnings;
	CWarningsPane		*itsWarningsPane;
	CCheckBox *warningOn;
	CRadioControl *allOn;
	CRadioControl *allOff;
	CRadioControl *indiv;
	CRadioGroupPane *radioPane;
	CHarvestOptions *itsOptions;

	
	void IWarningsDialog(  CHarvestOptions *theOpts,CWarningsArray *theWarnings);
	
	virtual void SetupItems( void);
	virtual void DoCommand( long aCmd);
	virtual void ProviderChanged( CCollaborator *aProvider, long reason, void *info);
	
	virtual void Dispose( void);
	
};