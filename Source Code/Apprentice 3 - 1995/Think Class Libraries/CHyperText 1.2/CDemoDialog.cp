/******************************************************************************
 CDemoDialog.cp

		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1994 Johns Hopkins University. All rights reserved.
	
	Original Author: 	Martin R. Wachter		email:	mrw@welchgate.welch.jhu.edu
	Created:			4/4/94					by:		mrw			TCL Version:	1.1.3
	Modified:			4/5/94					by:		mrw			TCL Version:	1.1.3

 ******************************************************************************/

#include "CDemoDialog.h"
#include "CDialog.h"
#include "HyperDemoCommands.h"
#include <CIconPane.h>
#include <CHyperText.h>

#define	kDemoDialogID			1024

enum		/* window item numbers	*/
{
	kOKBtn = 1,
	hyText,
	asIcon,
	paintIcon,
	commIcon,
	mailIcon
};

extern CApplication	*gApplication;

/******************************************************************************
 IDemoDialog
******************************************************************************/

void CDemoDialog::IDemoDialog( void)
{
	
	CDLOGDirector::IDLOGDirector( kDemoDialogID, gApplication);
	
	((CDialog*)itsWindow)->SetDefaultCmd( cmdOK);

	itsHyperText = (CHyperText*) itsWindow->FindViewByID( hyText);
	itsHyperText->SetClickCmd(cmdHyperHit);
	
	itsASIcon = (CIconPane*) itsWindow->FindViewByID( asIcon);
	itsPaintIcon = (CIconPane*) itsWindow->FindViewByID( paintIcon);
	itsCommIcon = (CIconPane*) itsWindow->FindViewByID( commIcon);
	itsMailIcon = (CIconPane*) itsWindow->FindViewByID( mailIcon);
	
	// disable cHaveIconDispatch because of a BUG in CIconPane in TCL 1.1.3
	itsASIcon->cHaveIconDispatch = FALSE;
	itsPaintIcon->cHaveIconDispatch = FALSE;
	itsCommIcon->cHaveIconDispatch = FALSE;
	itsMailIcon->cHaveIconDispatch = FALSE;
	
}	/* CDemoDialog::IDemoDialog */

/******************************************************************************
 DoCommand
******************************************************************************/

void CDemoDialog::DoCommand( long aCmd)
{
Str255	sTemp;
Rect	r;
long	fticks;

	CopyPString(itsHyperText->lastHyperword, sTemp);
	
	switch (aCmd){
		case cmdHyperHit:
			if(EqualString(sTemp,"\papplescript",FALSE,FALSE)){
				itsASIcon->Prepare();
				itsASIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsASIcon->DrawIcon(FALSE);
				Delay(5,&fticks);
				itsASIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsASIcon->DrawIcon(FALSE);
			}
			else if(EqualString(sTemp,"\ppainting",FALSE,FALSE)){
				itsPaintIcon->Prepare();
				itsPaintIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsPaintIcon->DrawIcon(FALSE);
				Delay(5,&fticks);
				itsPaintIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsPaintIcon->DrawIcon(FALSE);
			}
			else if(EqualString(sTemp,"\pcommunication",FALSE,FALSE)){
				itsCommIcon->Prepare();
				itsCommIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsCommIcon->DrawIcon(FALSE);
				Delay(5,&fticks);
				itsCommIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsCommIcon->DrawIcon(FALSE);
			}
			else if(EqualString(sTemp,"\pmail",FALSE,FALSE)){
				itsMailIcon->Prepare();
				itsMailIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsMailIcon->DrawIcon(FALSE);
				Delay(5,&fticks);
				itsMailIcon->DrawIcon(TRUE);
				Delay(5,&fticks);
				itsMailIcon->DrawIcon(FALSE);
			}
		break;
			
		default: 
				inherited::DoCommand( aCmd);
				break;
	}
				
}	/* CDemoDialog::DoCommand */

/******************************************************************************
 ProviderChanged
******************************************************************************/

void CDemoDialog::ProviderChanged( CCollaborator *aProvider, long reason,
										void *info)
{
	inherited::ProviderChanged( aProvider, reason, info);

}	/* CDemoDialog::ProviderChanged */

/******************************************************************************
 Dispose
******************************************************************************/

void CDemoDialog::Dispose( void)
{
	inherited::Dispose();
}

